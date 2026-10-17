/*
 *
 * Copyright (C) 2023-2024, Broadband Forum
 * Copyright (C) 2023-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file uds.c
 *
 * Implements the Unix Domain Socket MTP (both server and client modes)
 *
 */

#ifdef ENABLE_UDS

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <string.h>

#include "common_defs.h"
#include "uds.h"
#include "device.h"
#include "dm_exec.h"
#include "vendor_defs.h"
#include "dllist.h"
#include "msg_handler.h"
#include "mtp_exec.h"
#include "os_utils.h"
#include "iso8601.h"

//------------------------------------------------------------------------------
// R-UDS.5 - When a UNIX domain socket connection is closed or fails to be established, the USP Endpoint acting as a client MUST attempt to re-establish
// the UNIX domain socket within a random amount of time between 1 and 5 seconds
#define MIN_RETRY_INTERVAL 1
#define MAX_RETRY_INTERVAL 5
#define DONT_RETRY false
#define RETRY_LATER true
#define HANDSHAKE_TIMEOUT 30
#define TLV_HEADER_SIZE 5

//------------------------------------------------------------------------------
// one of these for each instantiated UDS server
typedef struct
{
    int instance;          // Instance number in Device.UnixDomainSockets.UnixDomainSocket.{i}. INVALID denotes that this entry in the array isn't used
    int listen_sock;       // if this is an MTP (service) then listen on this socket for connections from controller
    uds_path_t path_type;  // Specifies whether this server is listening on the USP Broker's agent or controller path
    scheduled_action_t schedule_reconnect;  // Sets whether a UDS reconnect is scheduled after the send queue has cleared
    uds_conn_params_t next_conn_params;     // Connection parameters to use, the next time that a reconnect occurs
    char *socket_path;                      // Store the socket path to remove the socket file when closing the listening socket
} uds_server_t;

// Array of listening servers
static uds_server_t uds_servers[MAX_UDS_SERVERS];

//------------------------------------------------------------------------------
// Payload to send in UDS queue
typedef struct
{
    double_link_t link;        // Doubly linked list pointers. These must always be first in this structure
    uds_frame_t type;          // Identifies as either UDS error, handshake or message
    mtp_send_item_t item;      // Information about the content to send
    time_t expiry_time;        // Time at which this USP record should be removed from the queue
} uds_send_item_t;

//------------------------------------------------------------------------------
// Structure representing each client connection
typedef struct
{
    int instance;                                // Instance number in Device.UnixDomainSockets.UnixDomainSocket.{i}. INVALID denotes that this entry in the array isn't used
    uds_connection_type_t type;                  // Is this the server or client end of the connection
    uds_path_t path_type;                        // Specifies whether this connection is on the USP Broker's agent or controller path
    int socket;                                  // socket connection to a server or client or INVALID if this UDS connection entry is not used
    int hdr_bytes_rxed;                          // number of bytes of sync header received on the socket
    int len_bytes_rxed;                          // number of bytes of length received on the socket
    unsigned char length_bytes[4];               // buffer to read length bytes into
    int payload_bytes_rxed;                      // number of bytes of payload received on the socket
    int payload_length;                          // calculated payload length from bitshifted bytes
    unsigned char *rx_buf;                       // transient buffer containing the USP message frame currently being received
    char *endpoint_id;                           // Endpoint ID at the other end of this connection
    double_linked_list_t usp_record_send_queue;  // Queue of USP Records to send to the endpoint
    unsigned char *tx_buf;                       // transient buffer containing the USP message frame currently being sent
    int tx_bytes_sent;                           // Counts the number of bytes sent of the current USP Record to tx (i.e. at the head of the usp_record_send_queue)
    int tx_buf_len;                              // the length of the currently transmitting packet
    uds_frame_t tx_buf_type;                     // type of the uds frame being transmitted
    bool is_disconnect_record;                   // Is the transmitted usp message of disconnect record type
    unsigned conn_id;                            // Unique identifier for this connection. Used to ensure that USP responses are placed on the correct queue (or discarded if the connection has dropped)
    char *socket_path;                           // Store the socket path in case we need to reconnect
    time_t retry_timeout;                        // If we lose the connection and have to reconnect after a random timeout
    scheduled_action_t schedule_reconnect;       // Sets whether a UDS reconnect is scheduled after the send queue has cleared
    uds_conn_params_t next_conn_params;          // Connection parameters to use, the next time that a reconnect occurs
    time_t handshake_timeout;                    // Close the connection if we don't receive a handshake response after timeout
} uds_connection_t;

// Array of connections
static uds_connection_t uds_connections[MAX_UDS_SERVERS*MAX_USP_SERVICES];

//------------------------------------------------------------------------------
// Sync bytes identifying the start of a UDS frame
// If these are not present, then something is wrong and the connection should be dropped
static const char uds_frame_sync_bytes[4] = { 0x5F, 0x55, 0x53, 0x50 };

//------------------------------------------------------------------------------
// Mutex used to protect access to this component (MTP exec thread vs message handler)
// Should lock around UDS_xxx API functions to protect connections and item queues.  Private functions should not lock
static pthread_mutex_t uds_access_mutex;

//------------------------------------------------------------------------------
// Counter used to ensure that each uds connection has a unique identifier
static unsigned uds_conn_id_counter = 1;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ProcessUdsFrame(uds_connection_t *uc);
void ProcessUdsRecord(uds_connection_t *uc, uds_frame_t frame_type, unsigned char *record, unsigned record_length);
uds_connection_t *FindFreeUdsConnection(void);
uds_server_t *FindFreeUdsServer(void);
uds_server_t *FindUdsServerByInstanceId(int instance);
uds_connection_t *FindUdsClientByInstanceId(int instance);
void CloseUdsConnection(uds_connection_t *uc, bool retry);
int CreateUdsClient(uds_conn_params_t *ucp);
int StartUdsClient(uds_connection_t *uc);
int EnableUdsServer(uds_conn_params_t *ucp);
char *ValidateUdsEndpointID(char *endpointID, uds_path_t path_type);
void SendUdsErrorFrame(uds_connection_t *uc, char* errorString);
void SendUdsFrames(uds_connection_t *uc);
void ReadUdsFrames(uds_connection_t *uc);
int HandleUdsListeningSocketConnection(uds_server_t *us);
uds_connection_t *FindUdsConnectionByConnId(unsigned conn_id);
void PopUdsSendItem(uds_connection_t *uc);
unsigned CalcNextUdsConnectionId(void);
void RemoveUdsQueueItem(uds_connection_t *uc, uds_send_item_t *queued_msg);
void RemoveExpiredUdsMessages(uds_connection_t *uc);
bool IsUspRecordInUdsQueue(uds_connection_t *uc, unsigned char *pbuf, int pbuf_len);
void InitialiseUdsConnection(uds_connection_t *uc);
void InitialiseUdsServer(uds_server_t *us);
char *EndpointIdForLog(uds_connection_t *uc);
void QueueUspConnectRecord_Uds(uds_connection_t *uc);

/*********************************************************************//**
**
** UDS_Init
**
** Called by DEVICE_UDS_Init data model during initialisation
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UDS_Init(void)
{
    uds_connection_t *uc;
    uds_server_t *us;
    int i;
    int err = USP_ERR_OK;

    // Mark all UDS connection slots as unused
    memset(uds_connections, 0, sizeof(uds_connections));

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        InitialiseUdsConnection(uc);
    }

    // Mark all UDS server slots as unused
    memset(uds_servers, 0, sizeof(uds_servers));
    for (i=0; i < NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        InitialiseUdsServer(us);
    }

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&uds_access_mutex);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

exit:
    return err;
}

/*********************************************************************//**
**
** UDS_UpdateAllSockSet
**
** Updates the set of all UDS socket fds to read/write from
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void UDS_UpdateAllSockSet(socket_set_t *set)
{
    int i;
    uds_connection_t *uc;
    uds_server_t *us;
    bool responses_sent = false;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly ncessary, as only the MTP thread should be calling this function
    if (is_uds_mtp_thread_exited)
    {
        goto exit;
    }

    // Add all server listening sockets
    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if (us->instance != INVALID)
        {
           if (us->schedule_reconnect == kScheduledAction_Activated)
           {
               // Its possible a listening server could be scheduled to restart when there are no valid connections
               // Default responses_sent to true to ensure the connection gets restarted in this case
               responses_sent = true;

               // we should only disable the listening server connection (and any connections associated with it) if there
               // are no pending messages being received or transmitted by any connections associated with the listening socket
               for (i=0; i<NUM_ELEM(uds_connections); i++)
               {
                   uc = &uds_connections[i];
                   if (uc->instance != INVALID)
                   {
                       // if the connection is a server connection belonging to the server socket instance
                       if ((uc->type == kUdsConnType_Server) && (uc->instance == us->instance))
                       {
                          // Determine if all responses have been sent on this connection, and update whether they have been sent on all connections
                          // Also check that we've not started to receive a UDS frame
                          responses_sent = ((uc->usp_record_send_queue.head == NULL) &&
                                            (uc->tx_buf == NULL) &&
                                            (uc->hdr_bytes_rxed==0));

                          if (responses_sent == false)
                          {
                             // there are pending messages in the message queue / rx buffer
                             // Break out and ignore this server instance until messages have been handled
                             USP_LOG_Info("Pending messages in the UDS server connection queue - cannot reconnect yet");
                             break;
                          }
                       }
                   }
               }

               if (responses_sent == true)
               {
                   uds_conn_params_t conn_params;

                   // if all messages pertaining to this server socket have been sent then disable the connection
                   // this will close all active connections associated with the listening server socket
                   USP_LOG_Info("UDS server connection parameters changed. Reconnecting instance %d", us->instance);

                   // make a deep copy of us->next_conn_params as these get freed in UDS_DisableConnection
                   conn_params.instance = us->next_conn_params.instance;
                   conn_params.path = USP_STRDUP(us->next_conn_params.path);
                   conn_params.path_type = us->next_conn_params.path_type;
                   conn_params.mode = us->next_conn_params.mode;

                   UDS_DisableConnection(us->instance);
                   // start a new connection (which may be a client or a server)
                   UDS_EnableConnection(&conn_params);
                   USP_SAFE_FREE(conn_params.path);
               }
           }

           if (us->listen_sock != INVALID)
           {
               SOCKET_SET_AddSocketToReceiveFrom(us->listen_sock, MAX_SOCKET_TIMEOUT, set);
           }
       }
    }

    time_t next_wakeup_time = INVALID;

    // Check all connected client sockets
    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance != INVALID)
        {
            // Connecting clients (either connected or retrying) are checked for scheduled reconnect
            if (uc->type == kUdsConnType_Client)
            {
                // Determine if all responses have been sent on this connection, and update whether they have been sent on all connections
                // Also check that we've not started to receive a UDS frame
                responses_sent = ((uc->usp_record_send_queue.head == NULL) &&
                                  (uc->tx_buf == NULL) &&
                                  (uc->hdr_bytes_rxed==0));

                // If a reconnect is scheduled...
                if (uc->schedule_reconnect == kScheduledAction_Activated)
                {
                    // Perform a reconnect when all responses have been sent (and there are no incoming messages)
                    if (responses_sent)
                    {
                        uds_conn_params_t conn_params;

                        USP_LOG_Info("UDS client connection parameters changed. Reconnecting instance %d", uc->instance);

                        // make a deep copy of us->next_conn_params as these get freed in UDS_DisableConnection
                        conn_params.instance = uc->next_conn_params.instance;
                        conn_params.path = USP_STRDUP(uc->next_conn_params.path);
                        conn_params.path_type = uc->next_conn_params.path_type;
                        conn_params.mode = uc->next_conn_params.mode;

                        UDS_DisableConnection(uc->instance);
                        // start a new connection (which may be a client or a server)
                        UDS_EnableConnection(&conn_params);
                        USP_SAFE_FREE(conn_params.path);
                    }
                    else
                    {
                        USP_LOG_Info("Pending messages in the client connection queue - cannot reconnect yet");
                    }
                }
            }

            if (uc->socket != INVALID)
            {
                SOCKET_SET_AddSocketToReceiveFrom(uc->socket, MAX_SOCKET_TIMEOUT, set);

                // Only interested in writing if there are USP records in the queue/outgoing buffer
                if ((uc->usp_record_send_queue.head != NULL) || (uc->tx_buf != NULL))
                {
                    SOCKET_SET_AddSocketToSendTo(uc->socket, MAX_SOCKET_TIMEOUT, set);
                }
            }

            // If any connections are retrying or waiting for a handshake response then set a socket timeout
            if (uc->type == kUdsConnType_Client)
            {
                if (uc->retry_timeout != INVALID)
                {
                    if ((next_wakeup_time == INVALID) || (next_wakeup_time > uc->retry_timeout))
                    {
                       next_wakeup_time = uc->retry_timeout;
                    }
                }

                if (uc->handshake_timeout != INVALID)
                {
                    if ((next_wakeup_time == INVALID) || (next_wakeup_time > uc->handshake_timeout))
                    {
                       next_wakeup_time = uc->handshake_timeout;
                    }
                }
            }
        }
    }

    // we need to calculate when the socket next needs to wake and call processActivity
    // This is either on the next connection retry or when a handshake timeout occurs
    if (next_wakeup_time != INVALID)
    {
        time_t cur_time;
        time_t socket_timeout;
        cur_time = time(NULL);
        socket_timeout = next_wakeup_time - cur_time;

        // time_t is signed so it is possible for timeout to be negative if we overshot for some reason
        // Set a minimum timeout of 1 second to avoid spinning
        if (socket_timeout <= 1)
        {
            socket_timeout = 1;
        }

        SOCKET_SET_UpdateTimeout(socket_timeout*SECONDS, set);
    }

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
}

/*********************************************************************//**
**
** UDS_GetMTPForEndpointId
**
** Determines if the specified endpoint is connected to an active connection
** and if so returns the parameters specifying the MTP to use
**
** \param   endpoint_id - Endpoint ID of controller that maybe connected to agent's socket
** \param   mtpc - structure to update with MTP details, if the specified controller is connected to the agent's socket
**                or NULL if the caller is just trying to determine whether the controller is connected to the agent's socket
**                NOTE: the mtpc structure should be updated, not initialised.
**
** \return  USP_ERR_OK if specified endpoint is connected to this endpoint via UDS
**
**************************************************************************/
int UDS_GetMTPForEndpointId(char *endpoint_id, mtp_conn_t *mtpc)
{
    int i;
    int ret = USP_ERR_OK;
    uds_connection_t *uc;

    USP_ASSERT(mtpc != NULL);

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        ret = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance != INVALID)
        {
            if ((uc->type != kUdsConnType_Invalid) && (uc->endpoint_id != NULL))
            {
                if (strcmp(uc->endpoint_id, endpoint_id)==0)
                {
                    mtpc->protocol = kMtpProtocol_UDS;
                    mtpc->uds.conn_id = uc->conn_id;
                    mtpc->uds.path_type = uc->path_type;
                    ret = USP_ERR_OK;
                    goto exit;
                }
            }
        }
    }

    ret = USP_ERR_INTERNAL_ERROR;

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
    return ret;
}

/*********************************************************************//**
**
** UDS_GetInstanceForConnection
**
** Determines the instance number in Device.UnixDomainSockets.UnixDomainSocket.{i} used by the specified connection
**
** \param   conn_id - uniquely identifies the connection
**
** \return  instance number in Device.UnixDomainSockets.UnixDomainSocket.{i} or INVALID if the connection had died
**
**************************************************************************/
int UDS_GetInstanceForConnection(unsigned conn_id)
{
    uds_connection_t *uc;
    int instance = INVALID;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        goto exit;
    }

    // Exit if the cnnection is not alive anymore
    uc = FindUdsConnectionByConnId(conn_id);
    if (uc == NULL)
    {
        goto exit;
    }

    // Connection is still connected
    instance = uc->instance;

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
    return instance;
}

/*********************************************************************//**
**
** UDS_EnableConnection
**
** Starts a new Unix Domain Socket as either a server or client
**
** \param   ucp - UDS connection parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UDS_EnableConnection(uds_conn_params_t *ucp)
{
    int err = USP_ERR_OK;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    switch(ucp->mode)
    {
        case kUdsConnType_Client:
            err = CreateUdsClient(ucp);
            break;

        case kUdsConnType_Server:
            err = EnableUdsServer(ucp);
            break;

        default:
        case kUdsConnType_Invalid:
            TERMINATE_BAD_CASE(ucp->mode);
            break;
    }

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
    return err;
}

/*********************************************************************//**
**
** UDS_DisableConnection
**
** Disconnects from the specified UDS connection and frees the specified connection
** This is called from DEVICE_UDS, if the connection has been disabled
**
** \param   instance - instance number in Device.UnixDomainSocket.UnixDomainSockets.{i}
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UDS_DisableConnection(int instance)
{
    int i;
    int err = USP_ERR_OK;
    uds_server_t *us;
    uds_connection_t *uc;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // first check the server instances and close any matching listening sockets
    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if ((us->instance != INVALID) && (us->instance == instance))
        {
            // and lastly close the server socket
            if (us->listen_sock != INVALID)
            {
                close(us->listen_sock);
                us->listen_sock = INVALID;
            }

            // delete any UDS file from the rootfs
            USP_ASSERT(us->socket_path);
            unlink(us->socket_path);
            USP_SAFE_FREE(us->socket_path);

            USP_SAFE_FREE(us->next_conn_params.path);

            InitialiseUdsServer(us);
            // there can only be one listening socket for a given instance so we can break out
            break;
        }
    }

    // The instance of a listening socket may be shared by any open connections on that socket
    // Iterate across all connections and close any that match the passed in instance that
    // either pertains to a listening socket connection or a client connection
    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if ((uc->instance != INVALID) && (uc->instance == instance))
        {
            CloseUdsConnection(uc, DONT_RETRY);
        }
    }

    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
    return err;
}

/*********************************************************************//**
**
** UDS_ScheduleReconnect
**
** Signals that a UDS reconnect occurs when all queued messages have been sent
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work, and why
**
** \param   ucp - pointer to data model parameters specifying the UDS connection
**
** \return  None
**
**************************************************************************/
void UDS_ScheduleReconnect(uds_conn_params_t *ucp)
{
    uds_connection_t *uc = NULL;
    uds_server_t *us = NULL;

    USP_LOG_Info("%s: scheduling reconnect on instance=%d path=%s, mode=%d", __FUNCTION__,ucp->instance, ucp->path, ucp->mode );

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&uds_access_mutex);
        return;
    }

    // ucp->instance could pertain to either a connecting client or a server listening socket
    us = FindUdsServerByInstanceId(ucp->instance);
    if (us != NULL)
    {
        // instance pertains to a listening server.  We need to disconnect the server and create
        // a new connection instance for the next connection params (which may now be a client).
        us->next_conn_params.instance = ucp->instance;
        USP_SAFE_FREE(us->next_conn_params.path);
        us->next_conn_params.path = USP_STRDUP(ucp->path);
        us->next_conn_params.path_type = ucp->path_type;
        us->next_conn_params.mode = ucp->mode;
        us->schedule_reconnect = kScheduledAction_Signalled;
        mtp_reconnect_scheduled = true;     // Set flag to ensure that data model thread subsequently calls UDS_ActivateScheduledActions()
        goto exit;
    }

    // Exit if unable to find the specified UDS connection
    uc = FindUdsClientByInstanceId(ucp->instance);
    if (uc != NULL)
    {
        // instance pertains to a client.  We need to disconnect the client and create
        // a new connection instance for the next connection params (which may now be a server)
        uc->next_conn_params.instance = ucp->instance;
        USP_SAFE_FREE(uc->next_conn_params.path);
        uc->next_conn_params.path = USP_STRDUP(ucp->path);
        uc->next_conn_params.path_type = ucp->path_type;
        uc->next_conn_params.mode = ucp->mode;
        uc->schedule_reconnect = kScheduledAction_Signalled;
        mtp_reconnect_scheduled = true;     // Set flag to ensure that data model thread subsequently calls UDS_ActivateScheduledActions()
        goto exit;
    }

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if ((uc != NULL) || (us != NULL))
    {
        MTP_EXEC_UdsWakeup();
    }
}

/*********************************************************************//**
**
** UDS_ProcessAllSocketActivity
**
** Processes the socket for the specified controller
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  Nothing
**
**************************************************************************/
void UDS_ProcessAllSocketActivity(socket_set_t *set)
{
    int i;
    uds_server_t *us;
    uds_connection_t *uc;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly ncessary, as only the MTP thread should be calling this function
    if (is_uds_mtp_thread_exited)
    {
        goto exit;
    }

    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if ((us->instance != INVALID) && (us->listen_sock != INVALID))
        {
            // server is running - are there any connection attempts
            if (SOCKET_SET_IsReadyToRead(us->listen_sock, set))
            {
                HandleUdsListeningSocketConnection(us);
            }
        }
    }

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance != INVALID)
        {
            if (uc->socket != INVALID)
            {
               // all active connections should attempt to read from the socket if data is available
               if (SOCKET_SET_IsReadyToRead(uc->socket, set))
               {
                   ReadUdsFrames(uc);
               }
            }

            // re-check uc->socket in case the connection was closed by ReadUdsFrames()
            if (uc->socket != INVALID)
            {
                if (SOCKET_SET_IsReadyToWrite(uc->socket, set))
                {
                    // Data in the queue/buffer and socket is ready to be written to
                    SendUdsFrames(uc);
                }
            }

            // re-check uc->socket in case the connection was closed by SendUdsFrames()
            if (uc->type == kUdsConnType_Client)
            {
                if (uc->retry_timeout != INVALID)
                {
                    // This is a valid client instance with an active retry timeout
                    if (uc->retry_timeout <= time(NULL))
                    {
                        uc->retry_timeout = INVALID;
                        USP_LOG_Info("%s: Retrying client socket connection now %s", __FUNCTION__, uc->socket_path);
                        StartUdsClient(uc);
                    }
                }

                if (uc->handshake_timeout != INVALID)
                {
                    // This is a valid client instance with an active retry timeout
                    if (uc->handshake_timeout <= time(NULL))
                    {
                        uc->handshake_timeout = INVALID;
                        USP_LOG_Info("%s: Retrying client socket handshake now %s", __FUNCTION__, uc->socket_path);
                        CloseUdsConnection(uc, RETRY_LATER);
                    }
                }
            }
        }
    }

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);
}

/*********************************************************************//**
**
** UDS_QueueBinaryMessage
**
** Function called to queue a USP record on the specified UDS connection
**
** \param   msi - pointer to content to send
**                NOTE: Ownership of the payload buffer passes to this function, unless an error is returned
** \param   mtpc - pointer to structure containing details of where to send this message
** \param   expiry_time - time duration in which the message should expire
** \param   frame_type - type of the UDS frame being sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UDS_QueueBinaryMessage(mtp_send_item_t *msi, mtp_conn_t *mtpc, time_t expiry_time, uds_frame_t frame_type)
{
    uds_connection_t *uc;
    uds_send_item_t *send_item;
    int err = USP_ERR_GENERAL_FAILURE;
    bool is_duplicate = false;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to determine which UDS connection to put the record on.
    // This could happen if the UDS connection dropped or was deleted in the data model whilst processing the USP Request (that this message is the associated USP response)
    uc = FindUdsConnectionByConnId(mtpc->uds.conn_id);
    if (uc == NULL)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Remove any queued messages that have expired
    RemoveExpiredUdsMessages(uc);

    // Do not add this message to the queue, if it is already present in the queue
    // This situation could occur if a notify is being retried to be sent, but is already held up in the queue pending sending
    if(frame_type == kUdsFrameType_UspRecord)
    {
       is_duplicate = IsUspRecordInUdsQueue(uc, msi->pbuf, msi->pbuf_len);
       if (is_duplicate)
       {
           USP_FREE(msi->pbuf);
           err = USP_ERR_OK;
           goto exit;
       }
    }

    // Add USP Record to queue
    send_item = USP_MALLOC(sizeof(uds_send_item_t));
    memset(send_item, 0, sizeof(uds_send_item_t));
    send_item->item = *msi;  // NOTE: Ownership of the payload buffer passes to the UDS message queue
    send_item->expiry_time = expiry_time;
    send_item->type = frame_type;
    if(send_item->type == kUdsFrameType_Error || msi->content_type == kMtpContentType_ConnectRecord)
    {
       DLLIST_LinkToHead(&uc->usp_record_send_queue, send_item);
    }
    else
    {
       DLLIST_LinkToTail(&uc->usp_record_send_queue, send_item);
    }
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);

    // kick the MTP thread to check the write status of the sending socket
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_UdsWakeup();
    }

    return err;
}
/*********************************************************************//**
**
** UDS_ActivateScheduledActions
**
** Called when all USP response messages have been queued.
** This function activates all scheduled actions which have been signalled
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work, and why
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UDS_ActivateScheduledActions(void)
{
    int i;
    uds_connection_t *uc;
    uds_server_t *us;
    bool wakeup = false;

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&uds_access_mutex);
        return;
    }

    // Iterate over all UDS connections, activating all reconnects and resubscribes which have been signalled
    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance != INVALID)
        {
           if (uc->schedule_reconnect == kScheduledAction_Signalled)
           {
               uc->schedule_reconnect = kScheduledAction_Activated;
               wakeup = true;
           }
        }
    }

    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if (us->instance != INVALID)
        {
            if (us->schedule_reconnect == kScheduledAction_Signalled)
            {
                us->schedule_reconnect = kScheduledAction_Activated;
                wakeup = true;
            }
        }
    }

    OS_UTILS_UnlockMutex(&uds_access_mutex);

    // Wakeup the UDS MTP thread, so that it can process the reconnect
    // (This is done outside of the mutex protection, as a slight optimization to avoid unnecessary task switches)
    if (wakeup)
    {
        MTP_EXEC_UdsWakeup();
    }
}

/*********************************************************************//**
**
** UDS_AreAllResponsesSent
**
** Determines whether all responses have been sent, and that there are no outstanding incoming messages
**
** \param   None
**
** \return  true if all responses have been sent
**
**************************************************************************/
bool UDS_AreAllResponsesSent(void)
{
    int i;
    uds_connection_t *uc;
    bool responses_sent;
    bool all_responses_sent = true;  // Assume that all responses have been sent on all connections

    OS_UTILS_LockMutex(&uds_access_mutex);

    // Exit if MTP thread has exited
    if (is_uds_mtp_thread_exited)
    {
        all_responses_sent = true;
        goto exit;
    }

    // Add all connected client sockets
    for (i=0; i< NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        // include ALL connections.  Connected or retrying clients, and also connections pertaining to server sockets
        // Note we may want to modify this to ignore retrying clients as potentially this could deadlock if a connection is stuck retrying
        if (uc->instance != INVALID)
        {
            // Determine if all responses have been sent on this connection, and update whether they have been sent on all connections
            // Also check that we've not started to receive a UDS frame
            responses_sent = ((uc->usp_record_send_queue.head == NULL) &&
                              (uc->tx_buf == NULL) &&
                              (uc->hdr_bytes_rxed==0));

            if (responses_sent == false)
            {
                all_responses_sent = false;
                break;
            }
        }
    }

exit:
    OS_UTILS_UnlockMutex(&uds_access_mutex);

    return all_responses_sent;
}

/*********************************************************************//**
**
** UDS_Destroy
**
** Frees all memory associated with this component and closes all sockets
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UDS_Destroy(void)
{
    int i;
    uds_connection_t *uc;
    uds_server_t *us;

    OS_UTILS_LockMutex(&uds_access_mutex);  // Ensure that the data model is held off accessing this module's data structures until after we have destroyed them

    // disable any "Connect"ing connections and client connections from listening servers
    for (i=0; i< NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance != INVALID)
        {
            UDS_DisableConnection(uc->instance);
        }
    }

    // disable any "Listen"ing server connections
    for (i=0; i < NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if (us->instance != INVALID)
        {
            UDS_DisableConnection(us->instance);
        }
    }

    // Prevent the data model from making any other changes to the MTP thread
    is_uds_mtp_thread_exited = true;

    OS_UTILS_UnlockMutex(&uds_access_mutex);
}

/*********************************************************************//**
**
** UDS_PathTypeToString
**
** Returns a textual description of the specified type of path, for use by debug
** NOTE: This function may be called from any thread
**
** \param   path_type - type of path
**
** \return  None
**
**************************************************************************/
char *UDS_PathTypeToString(uds_path_t path_type)
{
    char *str;

    switch(path_type)
    {
        case kUdsPathType_BrokersAgent:
            str = "Broker's Agent path";
            break;

        case kUdsPathType_BrokersController:
            str = "Broker's Controller path";
            break;

        default:
        case kUdsPathType_Invalid:
            str = "Unknown";
            break;
    }

    return str;
}


/*********************************************************************//**
**
** EnableUdsServer
**
** Private internal function to start a "Listening" mode domain socket
**
** \param   ucp - pointer to data model parameters specifying the connection to disable
**
** \return  None
**
**************************************************************************/
int EnableUdsServer(uds_conn_params_t *ucp)
{
    int err = USP_ERR_INTERNAL_ERROR;
    int result;
    struct sockaddr_un addr;
    uds_server_t *us;
    mode_t current_mask;

    // Exit if unable to find an unused server slot
    us = FindFreeUdsServer();
    if (us == NULL)
    {
        USP_LOG_Error("%s: No free server elements available", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    InitialiseUdsServer(us);

    // keep a copy of the UDS DM instance
    us->instance = ucp->instance;
    us->path_type = ucp->path_type;
    USP_ASSERT(us->socket_path == NULL);
    us->socket_path = USP_STRDUP(ucp->path);

    // Create a new server socket with domain: AF_UNIX, type: SOCK_STREAM, protocol: 0
    us->listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (us->listen_sock == INVALID)
    {
        USP_ERR_ERRNO("socket", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to ensure that all directories used by the UDS path have been created
    err = OS_UTILS_CreateDirFromFilename(ucp->path);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Note normally we would expect any socket files to be cleaned up when the socket was gracefully closed
    // however - it is possible the file exists due to ungraceful exit of the previous execution
    unlink(ucp->path);

    // Fill in the unix domain socket path
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    USP_STRNCPY(addr.sun_path, ucp->path, sizeof(addr.sun_path) - 1);

    // Exit if unable to bind the socket to the required path
    // NOTE: Temporarily change file creation permissions so that this process (running as root) creates a socket that can be accessed by non-root users
    current_mask = umask(0);
    result = bind(us->listen_sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (result == -1)
    {
        USP_ERR_ERRNO("bind", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }
    umask(current_mask);

    // this is a passive socket that creates active sockets connections
    #define MAX_PENDING_CONNECTIONS     5
    result = listen(us->listen_sock, MAX_PENDING_CONNECTIONS);
    if (result == -1)
    {
        USP_ERR_ERRNO("listen", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // we now have a passive socket listening for connections from services at SERVER_SOCKET_PATH
    err = USP_ERR_OK;

exit:
    // clean up any resources allocated
    if ((err != USP_ERR_OK) && (us->listen_sock != INVALID))
    {
        close(us->listen_sock);
        us->listen_sock = INVALID;      // Mark the slot as unused
    }

    return err;
}

/*********************************************************************//**
**
** CreateUdsClient
**
** Private internal function to create a "Connect" client connection instance
**
** \param   ucp - pointer to data model parameters specifying the domain socket path
**
** \return  None
**
**************************************************************************/
int CreateUdsClient(uds_conn_params_t *ucp)
{
    int err = USP_ERR_OK;
    uds_connection_t *uc = NULL;

    // Exit if unable to find an unused connection slot
    uc = FindFreeUdsConnection();
    if (uc == NULL)
    {
        USP_LOG_Error("%s: Too many connections - ignoring connect request", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    memset(uc, 0, sizeof(uds_connection_t));

    InitialiseUdsConnection(uc);

    uc->instance = ucp->instance;
    uc->conn_id = CalcNextUdsConnectionId();
    uc->path_type = ucp->path_type;
    uc->socket_path = USP_STRDUP(ucp->path);

    err = StartUdsClient(uc);

exit:
    return err;
}

/*********************************************************************//**
**
** StartUdsClient
**
** Private internal function to start a "Connect" mode domain socket
** Called when creating a new client connection, or retrying an existing connection
**
** \param   ucp - pointer to data model parameters specifying the domain socket path
**
** \return  USP_ERR_OK if the connection is established or a retry triggered
**
**************************************************************************/
int StartUdsClient(uds_connection_t *uc)
{
    int err = USP_ERR_INTERNAL_ERROR;
    int result;
    struct sockaddr_un addr;
    uds_send_item_t *send_item = NULL;

    // type might be kUdsConnType_Retry if retrying after being disconnected
    uc->type = kUdsConnType_Client;

    // Create a new client connecting socket with domain: AF_UNIX, type: SOCK_STREAM, protocol: 0
    uc->socket = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (uc->socket == -1)
    {
        USP_ERR_ERRNO("socket", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Fill in the unix domain socket path
    USP_ASSERT(uc->socket_path != NULL);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    USP_STRNCPY(addr.sun_path, uc->socket_path, sizeof(addr.sun_path));

    // Exit if unable to connect
    result = connect(uc->socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (result == -1)
    {
        // This may mean that the service we're connecting to isn't available yet
        USP_LOG_Warning("%s: cannot connect to socket path %s", __FUNCTION__, uc->socket_path);
        // close connection, set retry = true to re-attempt connection automatically
        CloseUdsConnection(uc, RETRY_LATER);
        // Because we are retrying the connection its okay to return success here
        err = USP_ERR_OK;
        goto exit;
    }

    // Add USP Record to end of queue
    send_item = USP_MALLOC(sizeof(uds_send_item_t));
    memset(send_item, 0, sizeof(uds_send_item_t));
    send_item->expiry_time = END_OF_TIME;
    send_item->type = kUdsFrameType_Handshake;
    DLLIST_LinkToHead(&uc->usp_record_send_queue, send_item);

    // UDS-18 if we don't receive a handshake within 30 seconds then we need to close the connection
    time_t cur_time;
    cur_time = time(NULL);
    uc->handshake_timeout = cur_time + HANDSHAKE_TIMEOUT;
    // kick the MTP thread here to trigger recalc of the handshake timeout in UDS_UpdateAllSockSet
    MTP_EXEC_UdsWakeup();

    err = USP_ERR_OK;

exit:
    // If a non-recoverable error occurred connecting to the server then clean up and return an error
    if (err != USP_ERR_OK)
    {
        CloseUdsConnection(uc, DONT_RETRY);
    }

    return err;
}

/*********************************************************************//**
**
** FindFreeUdsConnection
**
** Private internal function to find a free UDS connection element in the array
**
** \param   None
**
** \return  a pointer to the connection, or NULL if none are found
**
**************************************************************************/
uds_connection_t *FindFreeUdsConnection(void)
{
    int i;
    uds_connection_t *uc;

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if (uc->instance == INVALID)
        {
            return uc;
        }
    }

    // If the code gets here, then no free entry was found
    return NULL;
}

/*********************************************************************//**
**
** FindUdsConnectionByConnId
**
** Finds the UDS connection matching the specified connection_id
**
** \param   conn_id - unique identifier for each connection
**
** \return  a pointer to the connection, or NULL if none are found
**
**************************************************************************/
uds_connection_t *FindUdsConnectionByConnId(unsigned conn_id)
{
    int i;
    uds_connection_t *uc;

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if ((uc->instance != INVALID) && (uc->conn_id == conn_id))
        {
            return uc;
        }
    }

    // If the code gets here, then no free entry was found
    return NULL;
}

/*********************************************************************//**
**
** FindFreeUdsServer
**
** Private internal function to find a free UDS server element in the array
**
** \param   None
**
** \return  a pointer to the server, or NULL if none are found
**
**************************************************************************/
uds_server_t *FindFreeUdsServer(void)
{
    int i;
    uds_server_t *us;

    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if (us->instance == INVALID)
        {
            return us;
        }
    }

    // If the code gets here, then no free entry was found
    return NULL;
}

/*********************************************************************//**
**
** QueueUspConnectRecord_Uds
**
** Adds the USP connect record at the front of the queue, ensuring that there is only one connect record in the queue
**
** \param   uc - Pointer to uds connection to send the connect record to
**
** \return  None
**
**************************************************************************/
void QueueUspConnectRecord_Uds(uds_connection_t *uc)
{
    uds_send_item_t *cur_msg;
    uds_send_item_t *next_msg;
    mtp_content_type_t type;
    uds_send_item_t *send_item;

    // Iterate over USP Records in the queue, removing all stale connect and disconnect records
    // A connect or disconnect record may still be in the queue if the connection failed before the record was fully sent
    cur_msg = (uds_send_item_t *) uc->usp_record_send_queue.head;
    while (cur_msg != NULL)
    {
        // Save pointer to next message, as we may remove the current message
        next_msg = (uds_send_item_t *) cur_msg->link.next;

        // Remove current message if it is a connect or disconnect record
        type = cur_msg->item.content_type;
        if (IsUspConnectOrDisconnectRecord(type))
        {
            RemoveUdsQueueItem(uc, cur_msg);
        }

        // Move to next message in the queue
        cur_msg = next_msg;
    }

    // Create the UDS USP Connect record
    send_item = USP_MALLOC(sizeof(uds_send_item_t));
    memset(send_item, 0, sizeof(uds_send_item_t));
    send_item->expiry_time = END_OF_TIME;
    send_item->type = kUdsFrameType_UspRecord;
    USPREC_UdsConnect_Create(uc->endpoint_id, &send_item->item);

    // Add the new connect record to the queue
    DLLIST_LinkToHead(&uc->usp_record_send_queue, send_item);
}

/*********************************************************************//**
**
** CloseUdsConnection
**
** Private internal function to close a connection
**
** \param   uc - Pointer to the connection to close
** \param   retry - whether to attempt to retry connecting (only relevant for client connections, ignored for server connections)
**
** \return  None
**
**************************************************************************/
void CloseUdsConnection(uds_connection_t *uc, bool retry)
{
    // Close the socket
    if (uc->socket != INVALID)
    {
        close(uc->socket);
        uc->socket = INVALID;
    }

    // Inform the rest of the system that an endpoint has disconnected, unless this was a graceful shutdown
    // We don't post the message for graceful shutdown, because we don't want any active requests to be removed from
    // the request table, as the operation complete (indicating failure) will not be sent because UDS has shutdown
    // Instead the operation complete is sent the next time that we start up
    // Also don't post if the connection was closed before successful UDS handshake (and hence remote endpoint_id is unknown)
    if ((mtp_exit_scheduled != kScheduledAction_Activated) && (uc->endpoint_id != NULL))
    {
        DM_EXEC_PostUdsDisconnected(uc->endpoint_id, uc->path_type);
    }

    // flush any rx/tx buffers
    USP_SAFE_FREE(uc->rx_buf);
    USP_SAFE_FREE(uc->tx_buf);
    USP_SAFE_FREE(uc->endpoint_id);

    // Flush all queued outgoing USP messages
    // Unlike other MTPs, For UDS we purge the list regardless of retrying or not.  Unlike other MTPS, for
    // UDS it is highly likely that loss of connection is due to either the client or server crashing and
    // potentially restarting.  On loss of connection any registered UDS services should be removed from
    // the datamodel and reregistered on successful reconnection.
    while (uc->usp_record_send_queue.head != NULL)
    {
       RemoveUdsQueueItem(uc, (uds_send_item_t *) uc->usp_record_send_queue.head);
    }

    // only attempt to retry/reconnect if we are the client side of the connection
    // Server side connection should be invalidated and a new connection established on a retry from a client
    if ((uc->type == kUdsConnType_Client) && (retry == true))
    {
        unsigned int delay;
        // retry state is special and indicates a valid client connection instance that is currently disconnected
        // No packets will be sent or received in this state or until the state returns to kUdsConnType_Client
        // which can only happen after a successful reconnection to a server.
        // delay must be betwee 1 and 5 seconds
        delay = (rand() % (MAX_RETRY_INTERVAL-MIN_RETRY_INTERVAL)) + MIN_RETRY_INTERVAL;
        uc->retry_timeout =  time(NULL) + delay;
        USP_LOG_Info("%s: Retrying connection in %d seconds for %s", __FUNCTION__, delay, UDS_PathTypeToString(uc->path_type));
        // kick the MTP thread here to trigger recalc of the socket timeout in UDS_UpdateAllSockSet
        MTP_EXEC_UdsWakeup();
    }
    else
    {
        // This is a server type connection, or a client connection that we don't want to reconnect
        USP_SAFE_FREE(uc->socket_path);
        USP_SAFE_FREE(uc->next_conn_params.path);
        InitialiseUdsConnection(uc);
    }

    return;
}

/*********************************************************************//**
**
** ReadUdsFrames
**
** Private internal function read data from any UDS connections with data available
**
** \param   uc - UDS connection which received some data
**
** \return  None
**
**************************************************************************/
void ReadUdsFrames(uds_connection_t *uc)
{
    int i;
    int num_bytes = 0;

    USP_ASSERT(uc->socket != INVALID);

    // the following implementation breaks the message down into header, length and payload to make
    // it easier to follow.  It could potentially be optimised to read a larger block of data into
    // a temporary buffer and then process that buffer in one iteration - though we cannot know the
    // length of the payload up front, so there's always the possibility of having to iterate several
    // times in order to pull in the entire frame.  This implementation takes a minimum of 3 iterations.


    // All UDS messages contain an 8 byte Header including a sync word followed by remaining bytes
    if (uc->hdr_bytes_rxed < 4)
    {
        int bytes_outstanding;
        char buf[4];

        bytes_outstanding = sizeof(uds_frame_sync_bytes) - uc->hdr_bytes_rxed;
        num_bytes = recv(uc->socket, buf, bytes_outstanding, 0);
        if (num_bytes <= 0) // -1 = error, 0 = other end closed
        {
            USP_LOG_Warning("%s: Endpoint (%s) disconnected from %s", __FUNCTION__, EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            CloseUdsConnection(uc, RETRY_LATER);
            return;
        }

        // Count the number of correct sync bytes received
        // If any of the sync bytes are incorrect, then disconnect
        for (i=0; i < num_bytes && (uc->hdr_bytes_rxed < 4) ; i++ )
        {
            if (buf[i] == uds_frame_sync_bytes[uc->hdr_bytes_rxed])
            {
                uc->hdr_bytes_rxed++;
            }
            else
            {
                USP_LOG_Error("%s: UDS Sync bytes incorrect. Disconnecting %s from %s", __FUNCTION__, EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
                CloseUdsConnection(uc, RETRY_LATER);
                return;
            }
        }
    }

    if ((uc->hdr_bytes_rxed == 4) && (uc->len_bytes_rxed < 4))
    {
        unsigned char *buf;
        int len;

        // great - we've got our 4 byte header so now we need to read the length 4 bytes
        buf = &uc->length_bytes[uc->len_bytes_rxed];
        len = 4 - uc->len_bytes_rxed;
        num_bytes = recv(uc->socket, buf, len, 0);
        if (num_bytes <= 0) // -1 = error, 0 = other end closed
        {
            USP_LOG_Warning("%s: Endpoint (%s) disconnected from %s", __FUNCTION__, EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            CloseUdsConnection(uc, RETRY_LATER);
            return;
        }

        uc->len_bytes_rxed += num_bytes;

        if (uc->len_bytes_rxed == 4)
        {
            // convert from the byte stream using correct endian
            uc->payload_length = CONVERT_4_BYTES(uc->length_bytes);
            if(uc->payload_length < TLV_HEADER_SIZE)
            {
                // The minimum valid frame payload length is at least one record which contains a type (1 byte) and a length (4 bytes)
                USP_LOG_Error("%s: Failed to parse incoming UDS Frame, as TLV is missing", __FUNCTION__);
                CloseUdsConnection(uc, RETRY_LATER);
                return;
            }

            if (uc->payload_length > MAX_UDS_FRAME_PAYLOAD_LEN)
            {
                // If a frame contains more than MAX_UDS_PAYLOAD_LEN of data then ignore it to limit memory usage
                USP_LOG_Error("%s: Incoming UDS frame is too large (%u bytes)", __FUNCTION__, uc->payload_length);
                CloseUdsConnection(uc, RETRY_LATER);
                return;
            }

            uc->rx_buf =  USP_MALLOC(uc->payload_length);
        }
    }

    if ((uc->hdr_bytes_rxed == 4) && (uc->len_bytes_rxed == 4))
    {
        // we have the header and the length, now read the payload
        num_bytes = recv(uc->socket, uc->rx_buf + uc->payload_bytes_rxed, uc->payload_length - uc->payload_bytes_rxed, 0);
        if (num_bytes <= 0) // -1 = error, 0 = other end closed
        {
            USP_LOG_Warning("%s: Endpoint (%s) disconnected from %s", __FUNCTION__, EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            CloseUdsConnection(uc, RETRY_LATER);
            return;
        }
        uc->payload_bytes_rxed += num_bytes;

        if (uc->payload_bytes_rxed == uc->payload_length)
        {
            ProcessUdsFrame(uc);

            // reset xxxBytesReceived to start parsing the next frame
            uc->hdr_bytes_rxed = 0;
            uc->len_bytes_rxed = 0;
            uc->payload_bytes_rxed = 0;
            USP_FREE(uc->rx_buf);
            uc->rx_buf = NULL;
        }
    }
}

/*********************************************************************//**
**
** ProcessUdsFrame
**
** Private internal function to process a raw UDS frame
**
** \param   uc - UDS connection which received some data
**
** \return  None
**
**************************************************************************/
void ProcessUdsFrame(uds_connection_t *uc)
{
    uds_frame_t frame_type;
    int record_length;

    // R-UDS.6 - A Frame sent across a UNIX domain socket that is being used as an MTP MUST have a Header field and one or more TLV fields.
    // Iterate through all the TLV fields in the payload processing them individually
    int record_offset = 0;

    // There must be at least TLV_HEADER_SIZE (5 bytes) following the current UDS record start offset to process the TLV field
    while ((record_offset + TLV_HEADER_SIZE) < uc->payload_length)
    {
       frame_type = uc->rx_buf[record_offset];
       record_length = CONVERT_4_BYTES((&uc->rx_buf[record_offset+1]));

       if (record_length < 0)
       {
           USP_LOG_Error("%s: Failed to parse incoming USP record length", __FUNCTION__);
           SendUdsErrorFrame(uc,"Failed to parse incoming USP record");
           return;
       }

       // If the TLV UDS record payload crosses the end of the UDS frame payload buffer then this is a malformed frame
       if ((record_offset + record_length + TLV_HEADER_SIZE) > uc->payload_length)
       {
           // R-UDS.23 UDS client or server must send error frame if incoming UDS Frame containing USP record cannot be parsed
           // R-UDS.24 UDS client or server must send error frame if it cannot parse incoming UDS Frame
           USP_LOG_Error("%s: Failed to parse incoming USP record", __FUNCTION__);
           SendUdsErrorFrame(uc,"Failed to parse incoming USP record");
           return;
       }
       ProcessUdsRecord(uc, frame_type, &uc->rx_buf[record_offset + TLV_HEADER_SIZE], record_length);
       // increment offset to point to next TLV entry 1 byte (Type) + 4 bytes (Len) + Record length
       record_offset += (TLV_HEADER_SIZE + record_length);
    }
}

/*********************************************************************//**
**
** ProcessUdsRecord
**
** Private internal function to process a raw UDS record extracted from a UDS frame
**
** \param   uc - UDS connection which received some data
** \param   frame_type - The type of record (handshake, error or USP record)
** \param   record - Pointer to the start of the record
** \param   record_length - The length of the record
**
** \return  None
**
**************************************************************************/
void ProcessUdsRecord(uds_connection_t *uc, uds_frame_t frame_type, unsigned char *record, unsigned record_length)
{
    char buf[128];
    unsigned len;
    char *err_msg = NULL;
    bool drop_connection = false;
    char time_buf[MAX_ISO8601_LEN];
    char *validate_endpoint = NULL;
    mtp_conn_t mtp_conn;

    switch(frame_type)
    {
        case kUdsFrameType_Handshake:
            // Exit if we've already received a handshake (in which case, we just ignore this handshake packet [R-UDS.19])
            if (uc->endpoint_id != NULL)
            {
                USP_LOG_Warning("%s: Ignoring extraneous UDS MTP handshake", __FUNCTION__);
                break;
            }

            if(record_length == 0)
            {
                // R-UDS.21 : UDS client or server must send error frame if it cannot parse handshake frame
                err_msg = "Invalid EndpointID, Failed to process Handshake Frame";
                USP_LOG_Error("%s: %s", __FUNCTION__, err_msg);
                SendUdsErrorFrame(uc, err_msg);
                break;
            }

            //get the endpointID for validation
            validate_endpoint = USP_MALLOC(record_length+1);   // Plus 1 to include NULL terminator
            memcpy(validate_endpoint, record, record_length);
            validate_endpoint[record_length] = '\0';

            //check for endpointID validity
            err_msg = ValidateUdsEndpointID(validate_endpoint, uc->path_type);
            if (err_msg != NULL)
            {
                // R-UDS.21 : UDS client or server must send error frame if it cannot parse handshake frame
                USP_LOG_Error("%s: %s", __FUNCTION__, err_msg);
                SendUdsErrorFrame(uc, err_msg);
                USP_SAFE_FREE(validate_endpoint);
                break;
            }

            // Save the endpoint_id of the connecting client
            uc->endpoint_id = validate_endpoint;
            USP_LOG_Info("Received UDS HANDSHAKE from endpoint_id=%s on %s", EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));

            if ( ((RUNNING_AS_USP_SERVICE()==true) && (uc->path_type == kUdsPathType_BrokersController)) ||
                 ((RUNNING_AS_USP_SERVICE()==false) && (uc->path_type == kUdsPathType_BrokersAgent)) )
            {
                QueueUspConnectRecord_Uds(uc);
            }

            // cancel the handshake timeout during handshake exchange
            // Only the client sets a handshake timeout as only the client expects a handshake response
            // doing this on a server connection is benign
            uc->handshake_timeout = INVALID;

            // Notify the data model of the endpoint which has connected
            DM_EXEC_PostUdsHandshakeComplete(uc->endpoint_id, uc->path_type, uc->conn_id);

            // If acting as a server, then queue our UDS handshake message now, in response to the handshake message received from the connecting client [R-UDS.17]
            if (uc->type == kUdsConnType_Server)
            {
                uds_send_item_t *send_item;

                send_item = USP_MALLOC(sizeof(uds_send_item_t));
                memset(send_item, 0, sizeof(uds_send_item_t));
                send_item->expiry_time = END_OF_TIME;
                send_item->type = kUdsFrameType_Handshake;
                DLLIST_LinkToHead(&uc->usp_record_send_queue, send_item);
            }
            break;

        case kUdsFrameType_Error:
            // R-UDS.25 : USP record with type error received. Close the uds socket connection
            len = MIN(record_length, (sizeof(buf)-1));
            USP_STRNCPY(buf, (char*)record, len);
            buf[len] = '\0'; // NULL terminate the error string
            USP_LOG_Error("Received UDS ERROR from endpoint_id=%s on %s", EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            USP_LOG_Error("UDS ERROR is '%s'", buf);
            drop_connection = true;
            break;

        case kUdsFrameType_UspRecord:
            // Discard received frame if we received a USP Record before the handshake process has completed [R-UDS.20]
            if (uc->endpoint_id == NULL)
            {
                USP_LOG_Warning("%s: Ignoring USP frame received before handshake completed", __FUNCTION__);
                break;
            }

            iso8601_cur_time(time_buf, sizeof(time_buf));
            USP_LOG_Info("USP Record received at time %s, from endpoint_id=%s over UDS (%s)", time_buf, EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));

            memset(&mtp_conn, 0, sizeof(mtp_conn));
            mtp_conn.is_reply_to_specified = true;
            mtp_conn.protocol = kMtpProtocol_UDS;
            mtp_conn.uds.conn_id = uc->conn_id;
            mtp_conn.uds.path_type = uc->path_type;
            DM_EXEC_PostUspRecord(record, record_length, EndpointIdForLog(uc), ROLE_UDS, &mtp_conn);
            break;



        default:
            // Unexpected type in the USP TLV, ignoring the frame as per R-UDS.15
            USP_LOG_Error("%s: Unsupported UDS frame type (0x%02x). Ignoring packet", __FUNCTION__, frame_type);
            break;
    }


    if (drop_connection)
    {
        USP_LOG_Info("Closing connection_id =%u", uc->conn_id);
        CloseUdsConnection(uc, RETRY_LATER);
    }
}


/*********************************************************************//**
**
** SendUdsErrorFrame
**
** Private internal function to send a UDS error frame
**
** \param   uc - Pointer to uds_connection_t instance structure
**
** \param   errorString - error message describing the failure
**
** \return  None
**
**************************************************************************/
void SendUdsErrorFrame(uds_connection_t *uc, char* errorString)
{

    mtp_conn_t mtp_conn;
    memset(&mtp_conn, 0, sizeof(mtp_conn));
    mtp_conn.uds.conn_id = uc->conn_id;
    mtp_conn.protocol = kMtpProtocol_UDS;

    mtp_send_item_t msi;
    msi.usp_msg_type = INVALID_USP_MSG_TYPE; //USP message type is for logging purpose only
    msi.content_type = kMtpContentType_UspMessage;
    msi.pbuf_len = strlen(errorString);
    msi.pbuf = USP_MALLOC(msi.pbuf_len);
    memcpy(msi.pbuf, errorString, msi.pbuf_len);

    UDS_QueueBinaryMessage(&msi, &mtp_conn, END_OF_TIME, kUdsFrameType_Error);

    return;
}

/*********************************************************************//**
**
** ValidateUdsEndpointID
**
** Private internal function to validate EndpointID from a received UDS handshake
**
** \param   endpointID - endpointID string
**
** \param   path_type - whether the endpoint is connected to the Broker's Controller or the Broker's Agent socket
**
** \return  a NULL if endpointID is valid or error string if endpointID is not valid
**
**************************************************************************/
char *ValidateUdsEndpointID(char* endpointID, uds_path_t path_type)
{
    int i;
    int count;
    uds_connection_t *uc;
    char *our_endpoint_id;

    if((strcmp(endpointID, "") == 0) || (strcmp(endpointID, " ") == 0))
    {
        return "NULL or empty string in EndpointID, Failed to process Handshake Frame";
    }

    // Determine if EndpointID contains two colons
    count = 0;
    for (i=0; endpointID[i] != '\0'; i++)
    {
        if(endpointID[i] ==':')
        {
            count++;
        }
    }

    // Exit if EndpointID does not contain two colons
    if(count!=2)
    {
        return "Incorrect format of EndpointID, Failed to process Handshake Frame";
    }

    //Iterate over existing UDS connections, to check if we have case of Duplicate EndpointID connecting on same UDS path
    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if ((uc->instance != INVALID) && (uc->endpoint_id != NULL) && (uc->path_type == path_type) && (strcmp(endpointID, uc->endpoint_id) == 0))
        {
            USP_LOG_Info("%s: Found matching path type %s and endpoint ID %s in existing connections", __FUNCTION__, UDS_PathTypeToString(path_type), endpointID);
            return "Duplicate EndpointID connecting on same UDS path, Failed to process Handshake Frame";
        }
    }

    // Disallow connections between endpoints with the same Endpoint ID
    our_endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    if (strcmp(endpointID, our_endpoint_id)==0)
    {
        return "Connecting EndpointID is the same as this Endpoint, Failed to process Handshake Frame";
    }

    return NULL;
}

/*********************************************************************//**
**
** SendUdsFrames
**
** Checks the connection message queue and writes any data available to the socket
**
** \param   uc - Pointer to uds_connection_t instance structure
**
** \return  None
**
**************************************************************************/
void SendUdsFrames(uds_connection_t *uc)
{
    int bytes_sent = 0;

    if (uc->socket == INVALID)
    {
        // bad socket - shouldn't really get here unless the socket was closed
        USP_LOG_Error("%s: transmit data on invalid socket", __FUNCTION__);
        return;
    }

    // Get the next frame, if not currently transmitting one
    if (uc->tx_buf == NULL)
    {
        PopUdsSendItem(uc);

        // Exit if there's no more frames to send
        if (uc->tx_buf == NULL)
        {
            return;
        }
    }

    // if we get here there has to be data in uc->tx_buf (either partial remaining frame, or a newly constructed frame)
    USP_ASSERT(uc->tx_buf != NULL);

    // Try sending the remaining data in the UDS frame
    bytes_sent = send(uc->socket, &uc->tx_buf[uc->tx_bytes_sent], (uc->tx_buf_len - uc->tx_bytes_sent), 0);
    if (bytes_sent == -1)
    {
        USP_ERR_ERRNO("send", errno);
        CloseUdsConnection(uc, RETRY_LATER);
        return;
    }

    uc->tx_bytes_sent += bytes_sent;
    if (uc->tx_bytes_sent == uc->tx_buf_len)
    {
        // if we sent all the data then free the buffer.
        // The next record in the queue will be popped on the next iteration
        // If the UDS frame sent was of Error Type, close the connection if the error was encountered after successful handshake.
        // Close the uds connection, If the UDS frame sent was for usp disconnect record.
        if(((uc->tx_buf_type == kUdsFrameType_Error) && (uc->endpoint_id != NULL)) || uc->is_disconnect_record)
        {
            CloseUdsConnection(uc, RETRY_LATER);
        }
        USP_FREE(uc->tx_buf);
        uc->tx_buf = NULL;
        uc->is_disconnect_record = false;
    }
}

/*********************************************************************//**
**
** PopUdsSendItem
**
** Removes the first item, forms a UDS frame with it, and puts the frame into the outgoing buffer, pending sending
**
** \param   uc - Pointer to uds_connection_t instance structure
**
** \return  None
**
**************************************************************************/
void PopUdsSendItem(uds_connection_t *uc)
{
    int tlv_len = 0;
    char *endpoint_id = "";  // NOTE: This is initialised to an empty string to prevent Clang static analyser from generating a false positive if it was initialised to NULL
    int endpoint_len = 0;
    unsigned char *p;

    USP_ASSERT(uc->tx_buf == NULL);

    //Remove any queued messages that have expired
    RemoveExpiredUdsMessages(uc);

    // Exit if no more USP records left to send
    uds_send_item_t *send_item = (uds_send_item_t *) uc->usp_record_send_queue.head;
    if (send_item == NULL)
    {
        return;
    }

    // Calculate the length of the TLV payload
    switch (send_item->type)
    {
        case kUdsFrameType_UspRecord:
            // Log the USP Record before we send the first chunk
            MSG_HANDLER_LogMessageToSend(&send_item->item, kMtpProtocol_UDS, uc->endpoint_id, NULL);
            tlv_len = 1 + 4 + send_item->item.pbuf_len;
            break;

        case kUdsFrameType_Error:
            tlv_len = 1 + 4 + send_item->item.pbuf_len;
            break;

        case kUdsFrameType_Handshake:
            endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
            USP_ASSERT(endpoint_id != NULL);
            endpoint_len = strlen(endpoint_id);
            // handshake record consists of TLV (type is 1 byte, length is 4)
            tlv_len = 1 + 4 + endpoint_len;
            break;


        default:
            TERMINATE_BAD_CASE(send_item->type);
            break;
    }

    // frame consists of 4 sync bytes + 4 length bytes + payload
    uc->tx_buf_len = 4 + 4 + tlv_len;
    uc->tx_buf = USP_MALLOC(uc->tx_buf_len);
    uc->tx_bytes_sent = 0;

    // Construct frame header
    p = uc->tx_buf;
    WRITE_N_BYTES(p, uds_frame_sync_bytes, sizeof(uds_frame_sync_bytes));
    WRITE_4_BYTES(p, tlv_len);

    // construct the USP type/len/value
    switch (send_item->type)
    {
        case kUdsFrameType_UspRecord:
            USP_LOG_Info("Sending USP RECORD to endpoint_id=%s on %s", EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            WRITE_BYTE(p, kUdsFrameType_UspRecord);
            WRITE_4_BYTES(p, send_item->item.pbuf_len);
            WRITE_N_BYTES(p, send_item->item.pbuf, send_item->item.pbuf_len);
            USP_FREE(send_item->item.pbuf);
            break;

        case kUdsFrameType_Error:
            USP_LOG_Info("Sending UDS ERROR to endpoint_id=%s on %s", EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            WRITE_BYTE(p, kUdsFrameType_Error);
            WRITE_4_BYTES(p, send_item->item.pbuf_len);
            WRITE_N_BYTES(p, send_item->item.pbuf, send_item->item.pbuf_len);
            USP_FREE(send_item->item.pbuf);
            break;

        case kUdsFrameType_Handshake:
            USP_LOG_Info("Sending UDS HANDSHAKE to endpoint_id=%s on %s", EndpointIdForLog(uc), UDS_PathTypeToString(uc->path_type));
            WRITE_BYTE(p, kUdsFrameType_Handshake);
            WRITE_4_BYTES(p, endpoint_len);
            WRITE_N_BYTES(p, endpoint_id, endpoint_len);
            break;


        default:
            TERMINATE_BAD_CASE(send_item->type);
            break;
    }

    //Update the type of frame being sent. This will be needed to close connection, if the frame being sent is of type Error
    uc->tx_buf_type = send_item->type;
    //Update if the usp message being sent is of disconnect record type. This will be needed to close connection, if disconnect record is being sent
    uc->is_disconnect_record = (send_item->item.content_type == kMtpContentType_DisconnectRecord) ? true : false;

    // now we've converted the send item into a UDS frame
    // free up the record and remove it from the tx queue
    DLLIST_Unlink(&uc->usp_record_send_queue, send_item);
    USP_FREE(send_item);
}

/*********************************************************************//**
**
** HandleUdsListeningSocketConnection
**
** Called when a client connects to one of our UDS listening sockets
**
** \param   us - UDS server which a client has connected to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int HandleUdsListeningSocketConnection(uds_server_t *us)
{
    USP_ASSERT(us);

    int socket;
    uds_connection_t *uc = NULL;
    struct sockaddr sa;
    socklen_t sa_len;

    // Exit if unable to accept the connection
    sa_len = sizeof(sa);
    socket = accept4(us->listen_sock, &sa, &sa_len, SOCK_NONBLOCK);
    if (socket == -1)
    {
        // If an error occurred, just log it
        USP_ERR_ERRNO("accept", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if no free internal UDS connection entries
    uc = FindFreeUdsConnection();
    if (uc == NULL)
    {
        USP_LOG_Error("%s: Too many connections - ignoring connect request", __FUNCTION__);
        close (socket);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Fill in the UDS connection structure
    InitialiseUdsConnection(uc);
    uc->type = kUdsConnType_Server;
    uc->socket = socket;
    uc->conn_id = CalcNextUdsConnectionId();
    uc->instance = us->instance;
    uc->path_type = us->path_type;
    uc->socket_path = NULL; // we don't need to remember socket path for server side connections - only clients attempt to reconnect after a disconnection

    DLLIST_Init(&uc->usp_record_send_queue);

    // NOTE: The UDS handshake record is sent in response to the handshake message received from the client [R-UDS.17], rather than here

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CalcNextUdsConnectionId
**
** Assigns a unique identifier for a UDS connection
**
** \param   None
**
** \return  unique identifier
**
**************************************************************************/
unsigned CalcNextUdsConnectionId(void)
{
    uds_conn_id_counter++;

    // Ensure that counter is unique, after unsigned wrap around
    while (FindUdsConnectionByConnId(uds_conn_id_counter) != NULL)
    {
        uds_conn_id_counter++;
    }

    return uds_conn_id_counter;
}

/*********************************************************************//**
**
** RemoveExpiredUdsMessages
**
** Removes all expired messages from the queue of messages to send
** NOTE: This mechanism can be used to prevent the queue from filling up needlessly if the controller is offine
**
** \param   uc - pointer to UDS connection
**
** \return  None
**
**************************************************************************/
void RemoveExpiredUdsMessages(uds_connection_t *uc)
{
    time_t cur_time;
    uds_send_item_t *queued_msg;
    uds_send_item_t *next_msg;
    char cur_time_buf[MAX_ISO8601_LEN], exp_time_buf[MAX_ISO8601_LEN];

    cur_time = time(NULL);
    iso8601_from_unix_time(cur_time, cur_time_buf, sizeof(cur_time_buf));
    queued_msg = (uds_send_item_t *) uc->usp_record_send_queue.head;
    while (queued_msg != NULL)
    {
        next_msg = (uds_send_item_t *) queued_msg->link.next;
        if (cur_time > queued_msg->expiry_time)
        {
            iso8601_from_unix_time(queued_msg->expiry_time, exp_time_buf, sizeof(exp_time_buf));
            USP_LOG_Warning("Removing message from queue with expiry time = %s, current time = %s", exp_time_buf, cur_time_buf);
            RemoveUdsQueueItem(uc, queued_msg);
        }

        queued_msg = next_msg;
    }
}


/*********************************************************************//**
**
** RemoveUdsQueueItem
**
** Frees the specified item in the send queue of the specified controller
**
** \param   uc - pointer to UDS connection
** \param   queued_msg - message to remove from the queue
**
** \return  None
**
**************************************************************************/
void RemoveUdsQueueItem(uds_connection_t *uc, uds_send_item_t *queued_msg)
{
    USP_ASSERT(queued_msg != NULL);

    // Free all dynamically allocated member variables
    USP_SAFE_FREE(queued_msg->item.pbuf);

    // Remove the specified item from the queue, and free the item itself
    DLLIST_Unlink(&uc->usp_record_send_queue, queued_msg);
    USP_FREE(queued_msg);
}

/*********************************************************************//**
**
** IsUspRecordInUdsQueue
**
** Determines whether the specified USP record is already queued, waiting to be sent
** This is used to avoid duplicate records being placed in the queue, which could occur under notification retry conditions
**
** \param   uc - uds connection which has USP records queued to send
** \param   pbuf - pointer to buffer containing USP Record to match against
** \param   pbuf_len - length of buffer containing USP Record to match against
**
** \return  true if the message is already queued
**
**************************************************************************/
bool IsUspRecordInUdsQueue(uds_connection_t *uc, unsigned char *pbuf, int pbuf_len)
{
    uds_send_item_t *queued_msg;

    // Iterate over USP Records in the STOMP queue
    queued_msg = (uds_send_item_t *) uc->usp_record_send_queue.head;
    while (queued_msg != NULL)
    {
        // Exit if the USP record is already in the queue
        if ((queued_msg->item.pbuf_len == pbuf_len) && (memcmp(queued_msg->item.pbuf, pbuf, pbuf_len)==0))
        {
             return true;
        }

        // Move to next message in the queue
        queued_msg = (uds_send_item_t *) queued_msg->link.next;
    }

    // If the code gets here, then the USP record is not in the queue
    return false;
}

/*********************************************************************//**
**
** FindUdsServerByInstanceId
**
** Private internal function to find a UDS server matching the datamodel instance
**
** \param   instance - the instance ID as specified in the datamodel
**
** \return  a pointer to the server, or NULL if none are found
**
**************************************************************************/
uds_server_t *FindUdsServerByInstanceId(int instance)
{
    int i;
    uds_server_t *us = NULL;

    for (i=0; i<NUM_ELEM(uds_servers); i++)
    {
        us = &uds_servers[i];
        if ((us->instance != INVALID) && (us->instance == instance))
        {
            return us;
        }
    }
    return NULL;
}

/*********************************************************************//**
**
** FindUdsClientByInstanceId
**
** Private internal function to find a UDS client connection matching the datamodel instance
**
** \param   instance - the instance ID as specified in the datamodel
**
** \return  a pointer to the connection, or NULL if none are found
**
**************************************************************************/
uds_connection_t *FindUdsClientByInstanceId(int instance)
{
    int i;
    uds_connection_t *uc = NULL;

    for (i=0; i<NUM_ELEM(uds_connections); i++)
    {
        uc = &uds_connections[i];
        if ((uc->instance != INVALID) && (uc->instance == instance))
        {
            // this is a client connection or a client connection pending a retry
            return uc;
        }
    }
    return NULL;
}

/*********************************************************************//**
**
** InitialiseUdsConnection
**
** Initialises uds_connection_t struct to default (invalid) values
**
** \param   pointer to a uds_connection_t structure
**
** \return  none
**
**************************************************************************/
void InitialiseUdsConnection(uds_connection_t *uc)
{
    memset(uc, 0, sizeof(uds_connection_t));
    uc->instance = INVALID;
    uc->type = kUdsConnType_Invalid;
    uc->path_type = kUdsPathType_Invalid;
    uc->socket = INVALID;
    uc->hdr_bytes_rxed = 0;
    uc->len_bytes_rxed = 0;
    uc->payload_bytes_rxed = 0;
    uc->payload_length = 0;
    uc->rx_buf = NULL;
    uc->endpoint_id = NULL;
    uc->usp_record_send_queue.head = NULL;
    uc->usp_record_send_queue.tail = NULL;
    uc->tx_buf = NULL;
    uc->tx_bytes_sent = 0;
    uc->tx_buf_len = 0;
    uc->tx_buf_type = kUdsFrameType_Invalid;
    uc->is_disconnect_record = false;
    uc->conn_id = INVALID;
    uc->socket_path = NULL;
    uc->retry_timeout = INVALID;
    uc->schedule_reconnect = kScheduledAction_Off;
    uc->next_conn_params.mode = kUdsConnType_Invalid;
    uc->next_conn_params.path_type = kUdsPathType_Invalid;
    uc->next_conn_params.instance = INVALID;
    uc->next_conn_params.path = NULL;
}

/*********************************************************************//**
**
** InitialiseUdsServer
**
** Initialises uds_server_t struct to default (invalid) values
**
** \param   pointer to a uds_server_t structure
**
** \return  none
**
**************************************************************************/
void InitialiseUdsServer(uds_server_t *us)
{
    memset(us, 0, sizeof(uds_server_t));
    us->listen_sock = INVALID;
    us->instance = INVALID;
    us->path_type = kUdsPathType_Invalid;
    us->schedule_reconnect = kScheduledAction_Off;
    us->next_conn_params.mode = kUdsConnType_Invalid;
    us->next_conn_params.path_type = kUdsPathType_Invalid;
    us->next_conn_params.instance = INVALID;
    us->next_conn_params.path = NULL;
    us->socket_path = NULL;
}

/*********************************************************************//**
**
** EndpointIdForLog
**
** Returns string of EndpointId to report in logs
** This function is necessary because the remote endpoint_id may be unknown at the time that the log statement needs to be printed
**
** \param   pointer to a uds_server_t structure
**
** \return  none
**
**************************************************************************/
char *EndpointIdForLog(uds_connection_t *uc)
{
    return (uc->endpoint_id == NULL) ? "UNKNOWN" : uc->endpoint_id;
}

#endif // ENABLE_UDS
