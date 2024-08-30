/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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
 * \file stomp.c
 *
 * Called from the ProtocolHandler to implement the STOMP protocol.
 *
 */

#ifndef DISABLE_STOMP
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <math.h>
#include <net/if.h>

#include <protobuf-c/protobuf-c.h>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>


#include "common_defs.h"
#include "stomp.h"
#include "usp-msg.pb-c.h"
#include "mtp_exec.h"
#include "msg_handler.h"
#include "proto_trace.h"
#include "data_model.h"
#include "iso8601.h"
#include "text_utils.h"
#include "device.h"
#include "nu_ipaddr.h"
#include "os_utils.h"
#include "dm_exec.h"
#include "nu_macaddr.h"
#include "retry_wait.h"


//------------------------------------------------------------------------------
// General definitions used in code
#define EMPTY_BODY ""

#define BBF_STOMP_CONTENT_TYPE        "application/vnd.bbf.usp.msg"

//------------------------------------------------------------------------------
// State of a STOMP connection
typedef enum
{
    kStompState_Idle,                       // Not yet connected
    kStompState_SendingStompFrame,          // TCP connected to the STOMP server and currently sending the initial STOMP frame
    kStompState_AwaitingConnectedFrame,     // Awaiting the response to the STOMP frame, the CONNECTED frame
    kStompState_SendingSubscribeFrame,      // Sending the subscribe frame, to subscribe to this Agent's queue
    kStompState_Running,                    // Normal steady state: Connection is ready to send and receive USP messages
    kStompState_Retrying,                   // An error has occurred. We have dropped the TCP connection and will attempt a reconnect at some time in the future

    kStompState_Max
} stomp_state_t;

//------------------------------------------------------------------------------------
// Array used by debug to print out the current STOMP connection state
char *state_names[kStompState_Max] =
{
    "Idle",                     // kStompState_Idle
    "SendingStompFrame",        // kStompState_SendingStompFrame
    "AwaitingConnectedFrame",   // kStompState_AwaitingConnectedFrame
    "SendingSubscribeFrame",    // kStompState_SendingSubscribeFrame
    "Running",                  // kStompState_Running
    "Retrying"                  // kStompState_Retrying
};

//------------------------------------------------------------------------------
// Table to convert from cause of STOMP failure to the value used by Device.STOMP.Connection{i}.Status
enum_entry_t stomp_failure_strings[] =
{
    { kStompFailure_None, "No Error" },
    { kStompFailure_ServerDNS, "ServerNotPresent" },
    { kStompFailure_Authentication, "Error_AuthenticationFailure" },
    { kStompFailure_Connect, "ServerNotPresent" },
    { kStompFailure_ReadWrite, "ServerNotPresent" },
    { kStompFailure_Timeout, "ServerNotPresent"},
    { kStompFailure_Misconfigured, "Error_Misconfigured"},
    { kStompFailure_OtherError, "Error"},
};

//------------------------------------------------------------------------------
// Definition of flags for perform_resubscribe
#define SCHEDULE_UNSUBSCRIBE  0x00000001
#define SCHEDULE_SUBSCRIBE    0x00000002

//------------------------------------------------------------------------------
// Parameters for each stomp connection
typedef struct
{
    // Setup parameters - these cache the parameters in Device.STOMP.Connection.{i}
    int instance;          // instance of this connection in the Device.STOMP.Connection.{i} table. Set to INVALID, if this entry is not used.
    char *host;
    unsigned port;
    char *username;
    char *password;
    bool enable_encryption;
    char *virtual_host;
    bool enable_heartbeats;
    unsigned incoming_heartbeat_period;  // in ms. NOTE: the negotiated heartbeat_period is stored in seconds
    unsigned outgoing_heartbeat_period;  // in ms
    stomp_retry_params_t retry;         // Parameters associated with retrying the connection
    char *provisionned_queue;           // Name of stomp queue to subscribe to (in Device.LocalAgent.MTP.{i}.STOMP.Destination)
                                        // NOTE This may be NULL or blank, because the queue may be provisionned by the controller in the CONNECTED frame

    // State variables
    stomp_state_t state;    // current state of this STOMP connection
    time_t last_status_change; // Time at which the status of the connection changed (as seen by Device.STOMP.Connection.{i}.LastChangeDate
    time_t  stomp_handshake_timeout;   // Absolute Time by which the STOMP connection should have performed initial STOMP handshake (ie STOMP, CONNECTED, SUBSCRIBE frame sequence)
    int retry_count;        // Number of times that the connection has been tried, and has failed. Starts from 0.
    time_t retry_time;      // If state is kStompState_Retrying, then this is the unix time at which the retry should be attempted
    stomp_failure_t failure_code; // If the STOMP connection fails, this gets set to the last cause of failure
    scheduled_action_t  schedule_reconnect;  // Sets whether a STOMP reconnect is scheduled after the send queue has cleared

    scheduled_action_t  schedule_resubscribe;// Sets whether a STOMP resubscribe is scheduled after the send queue has cleared
    unsigned  perform_resubscribe;  // Set to perform a STOMP UNSUBSCRIBE/SUBSCRIBE pair, after the current STOMP frame has been sent

    int socket_fd;          // socket used for this STOMP connection (this is actually part of the bio, but duplicated here to make it easier to access)
    SSL *ssl;               // SSL used for this STOMP connection
    STACK_OF(X509) *cert_chain; // Full SSL certificate chain for the STOMP connection, collected in the SSL verify callback

    ctrust_role_t role;     // role granted by the CA cert in the chain of trust with the STOMP broker

    char *subscribe_dest;   // STOMP destination to subscribe to (received from the STOMP server in the CONNECTED frame).
                            // This overrides Device.LocalAgent.MTP.{i}.STOMP.Destination.
    int agent_heartbeat_period;   // Negotiated number of seconds between sending out heartbeats (if no other message has been sent in the meantime)
                                  // Or zero if heartbeats should not be sent
    int server_heartbeat_period;   // Negotiated number of seconds between receiving heartbeats (if no other message has been sent in the meantime)
                                  // Or zero if heartbeats are not expected to be received from the server
    time_t next_heartbeat_time;  // Absolute time at which next agent heartbeat should be sent, or INVALID_TIME if heartbeats are not being sent

    time_t last_received_time; // Last time at which a heartbeat or a USP message was received from the server, or INVALID_TIME if nothing received yet (eg connection is in retrying state)

    unsigned char *rxframe;   // pointer to buffer, used to concatenate message fragments until a complete message has been received
    int rxframe_msglen;       // number of message bytes copied into rxframe
    int rxframe_maxlen;       // size of rxframe allocated
    int rxframe_frame_len;    // Total number of bytes for the entire message (calculated using content-length: header and bytes received in message headers)
    int rxframe_header_len;   // Number of bytes in the STOP header. This is all bytes before the body, including COMMAND and the blank line separating the header from the body

    unsigned char *txframe;   // Variables representing the current STOMP frame being transmitted
    int txframe_len;
    int txframe_sent_count;
    bool txframe_contains_usp_record; // Set if the current frame being transmitted contains the USP record at the head of the send queue

    double_linked_list_t usp_record_send_queue;    // Queue of USP records to send on this STOMP connection

    stomp_conn_params_t next_conn_params;  // Connection parameters to use, the next time that a reconnect occurs
    char *next_provisionned_queue;         // Agent queue name to use, the next time that a reconnect or resubscribe occurs

    char mgmt_ip_addr[NU_IPADDRSTRLEN]; // IP address of device's source address providing this STOMP connection
    char mgmt_if_name[IFNAMSIZ];        // Name of network interface providing this STOMP connection


} stomp_connection_t;

//------------------------------------------------------------------------------
// Array of enabled (ie active) STOMP connections
static stomp_connection_t stomp_connections[MAX_STOMP_CONNECTIONS];

//------------------------------------------------------------------------------
// Payload to send in STOMP queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    mtp_send_item_t item;   // Information about the content to send
    char *controller_queue; // Name of the STOMP queue to send this message to
    char *agent_queue;      // Name of the STOMP queue used by this agent
    time_t expiry_time;     // Time at which this USP record should be removed from the queue
} stomp_send_item_t;

//------------------------------------------------------------------------------
// Variables associated with determining whether the Management IP address has changed (used by UpdateMgmtInterface)
static time_t next_mgmt_if_poll_time = 0;   // Absolute time at which to next poll for IP address change
#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
char last_mgmt_ip_addr[NU_IPADDRSTRLEN] = { 0 };
#endif

//------------------------------------------------------------------------------------
// The SSL context for STOMP (created for use with TLS)
SSL_CTX *stomp_ssl_ctx = NULL;

//------------------------------------------------------------------------------------
// Mutex used to protect access to this component
static pthread_mutex_t stomp_access_mutex;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void UpdateStompConnectionSockSet(stomp_connection_t *sc, socket_set_t *set);
void ProcessStompConnectionSocketActivity(stomp_connection_t *sc, socket_set_t *set);
int CalcTimeoutToStompHandshakeFailure(stomp_connection_t *sc);
void UpdateAgentHeartbeat(stomp_connection_t *sc);
int TransmitStompMessage(stomp_connection_t *sc);
void ReceiveStompMessage(stomp_connection_t *sc);
int ReceiveStompMessageInner(stomp_connection_t *sc, unsigned char *buf, int num_bytes);
int StompWrite(stomp_connection_t *sc, unsigned char *buf, int bytes_to_attempt);
int IsStompMsgComplete(stomp_connection_t *sc, int *msg_size);
int ParseStompHeaders(stomp_connection_t *sc, int *header_size);
void RemoveReceivedHeartBeats(stomp_connection_t *sc);
int ParseContentLengthHeader(stomp_connection_t *sc, int *content_length);
void HandleStompMessage(stomp_connection_t *sc, int msg_size);
void HandleRxMsg_AwaitingConnectedFrameState(stomp_connection_t *sc, int msg_size);
void HandleRxMsg_RunningState(stomp_connection_t *sc, int msg_size);
void RemoveMessageFromRxBuf(stomp_connection_t *sc, int msg_size);
bool IsFrame(char *frame_name, unsigned char *msg, int msg_len);
void ParseConnectedFrame(stomp_connection_t *sc, unsigned char *msg, int msg_len);
bool GetStompHeaderValue(char *header, unsigned char *msg, int msg_len, char *buf, int len);
void HandleStompSocketError(stomp_connection_t *sc, stomp_failure_t failure_code);
unsigned CalculateStompRetryWaitTime(unsigned retry_count, double interval, double multiplier);
int StartSendingFrame_STOMP(stomp_connection_t *sc);
int StartSendingFrame_SUBSCRIBE(stomp_connection_t *sc);
int StartSendingFrame_SEND(stomp_connection_t *sc, char *controller_queue, char *agent_queue, mtp_send_item_t *msi);
int StartSendingFrame_UNSUBSCRIBE(stomp_connection_t *sc);
int StartSendingFrame_DISCONNECT(stomp_connection_t *sc);
char *AddrInfoToStr(struct addrinfo *addr, char *buf, int len);
void UpdateNextHeartbeatTime(stomp_connection_t *sc);
int UpdateMgmtInterface(void);
void UpdateWANInterface(bool is_first_time);
stomp_connection_t *FindStompConnByInst(int instance);
void StartStompConnection(stomp_connection_t *sc);
void StopStompConnection(stomp_connection_t *sc, bool purge_queued_messages);
void InitStompConnection(stomp_connection_t *sc);
int PerformStompSslConnect(stomp_connection_t *sc);
int PerformStompSslHandshake(stomp_connection_t *sc);
stomp_connection_t *FindUnusedStompConn(void);
void CopyStompConnParamsToNext(stomp_connection_t *sc, stomp_conn_params_t *sp, char *stomp_queue);
void CopyStompConnParamsFromNext(stomp_connection_t *sc);
char *AllocateStringIfChanged(char *cur_str, char *new_str);
void LogNoPasswordWarning(stomp_connection_t *sc);
void EscapeStompHeader(char *src, char *dest, int dest_len);
void HandleStompSourceIPAddrChanges(void);
bool IsUspRecordInStompQueue(stomp_connection_t *sc, unsigned char *pbuf, int pbuf_len);
void RemoveExpiredStompMessages(stomp_connection_t *sc);
void RemoveStompQueueItem(stomp_connection_t *sc, stomp_send_item_t *queued_msg);
void QueueUspConnectRecord_STOMP(stomp_connection_t *sc, mtp_send_item_t *msi, char *controller_queue, char *agent_queue, time_t expiry_time);
int HandleStompRunningState(stomp_connection_t *sc, socket_set_t *set);
int GetNextStompMsgToSend(stomp_connection_t *sc);

/*********************************************************************//**
**
** STOMP_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int STOMP_Init(void)
{
    int i;
    int err;
    stomp_connection_t *sc;

    // Mark all stomp connection slots as unused
    memset(stomp_connections, 0, sizeof(stomp_connections));
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        sc->instance = INVALID;
        sc->schedule_reconnect = kScheduledAction_Off;
        sc->schedule_resubscribe = kScheduledAction_Off;
    }

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&stomp_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** STOMP_Destroy
**
** Frees all memory associated with this component and closes all sockets
**
** \param   None
**
** \return  None
**
**************************************************************************/
void STOMP_Destroy(void)
{
    int i;
    stomp_connection_t *sc;

    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if (sc->instance != INVALID)
        {
            STOMP_DisableConnection(sc->instance, PURGE_QUEUED_MESSAGES);
        }
    }

    // Free the OpenSSL context
    if (stomp_ssl_ctx != NULL)
    {
        SSL_CTX_free(stomp_ssl_ctx);
    }
}

/*********************************************************************//**
**
** STOMP_Start
**
** Called before starting all STOMP connections
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int STOMP_Start(void)
{
    int err;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Store the initial IP address for the management interface
    UpdateMgmtInterface();

    // Create the SSL context with trust store and client cert loaded
    stomp_ssl_ctx = DEVICE_SECURITY_CreateSSLContext(SSLv23_client_method(), SSL_VERIFY_PEER, DEVICE_SECURITY_TrustCertVerifyCallback);
    if (stomp_ssl_ctx == NULL)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // If code gets here then it was successful
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);
    return err;
}

/*********************************************************************//**
**
** STOMP_UpdateAllSockSet
**
** Updates the set of all STOMP socket fds to read/write from
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void STOMP_UpdateAllSockSet(socket_set_t *set)
{
    int i;
    stomp_connection_t *sc;
    bool responses_sent;
    int timeout;
    time_t cur_time;
    time_t expected_heartbeat_time;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly ncessary, as only the MTP thread should be calling this function
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Determine whether IP address has changed (if time to poll it)
    timeout = UpdateMgmtInterface();
    SOCKET_SET_UpdateTimeout(timeout*SECONDS, set);

    // Iterate over all STOMP connections, updating the ones that are enabled
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if (sc->instance != INVALID)
        {
            // Determine if all responses have been sent on this connection, and update whether they have been sent on all connections
            // NOTE: For the receive buffer, Rabbit MQ adds a redundant newline padding at the end of each stomp frame.
            // Therefore a single line feed in the receive buffer is still an empty buffer
            responses_sent = ((sc->usp_record_send_queue.head == NULL) &&
                              (sc->txframe == NULL) &&
                              ( (sc->rxframe_msglen==0) || ((sc->rxframe_msglen==1) && (sc->rxframe[0] == '\n')) )
                             );

            // If a reconnect is scheduled...
            if (sc->schedule_reconnect == kScheduledAction_Activated)
            {
                // Perform a reconnect when all responses have been sent (and there are no incoming messages)
                if (responses_sent)
                {
                    USP_LOG_Info("Connection parameters changed. Reconnecting to (host=%s, port=%d)", sc->host, sc->port);

                    StopStompConnection(sc, PURGE_QUEUED_MESSAGES);  // NOTE: All messages in queue should already have been removed
                    sc->schedule_reconnect = kScheduledAction_Off;
                    StartStompConnection(sc);
                }
                else
                {
                    // Ensure that this function will be called at least once every second until all responses have been sent
                    // (without this, after the last message is sent, the socket select timeout will be large, preventing this code from running for a long time)
                    SOCKET_SET_UpdateTimeout(1*SECONDS, set);
                }
            }

            // If a resubscribe is scheduled...
            if (sc->schedule_resubscribe == kScheduledAction_Activated)
            {
                // Perform a resubscribe when all responses have been sent (and there are no incoming messages)
                if (responses_sent)
                {
                    USP_LOG_Info("Agent Destination queue changed. Resubscribing to queue=%s", sc->next_provisionned_queue);
                    sc->provisionned_queue = AllocateStringIfChanged(sc->provisionned_queue, sc->next_provisionned_queue);

                    sc->schedule_resubscribe = kScheduledAction_Off;
                    sc->perform_resubscribe = SCHEDULE_UNSUBSCRIBE | SCHEDULE_SUBSCRIBE;
                }
                else
                {
                    // Ensure that this function will be called at least once every second until all responses have been sent
                    // (without this, after the last message is sent, the socket select timeout will be large, preventing this code from running for a long time)
                    SOCKET_SET_UpdateTimeout(1*SECONDS, set);
                }
            }

            // Handle STOMP server heartbeat timeouts
            if ((sc->server_heartbeat_period != 0) && (sc->last_received_time != INVALID_TIME))
            {
                // If a STOMP server heartbeat timeout has occurred...
                cur_time = time(NULL);
                expected_heartbeat_time = sc->last_received_time + sc->server_heartbeat_period + STOMP_SERVER_HEARTBEAT_GRACE_PERIOD;
                if (cur_time >= expected_heartbeat_time)
                {
                    // Cause a retry to occur
                    USP_LOG_Error("ERROR: STOMP server heartbeat not received (cur_time=%d, last_received_time=%d)", (int)cur_time, (int)sc->last_received_time);
                    HandleStompSocketError(sc, kStompFailure_Timeout);
                }
                else
                {
                    // Otherwise, update timeout, so that it at least occurs when the STOMP server heartbeat timeout would fire
                    timeout = (int)(expected_heartbeat_time - cur_time);
                    SOCKET_SET_UpdateTimeout(timeout*SECONDS, set);
                }
            }

            // Update the socket set with the socket and timeout for this connection
            UpdateStompConnectionSockSet(sc, set);
        }
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);
}

/*********************************************************************//**
**
** STOMP_AreAllResponsesSent
**
** Determines whether all responses have been sent, and that there are no outstanding incoming messages
**
** \param   None
**
** \return  true if all responses have been sent
**
**************************************************************************/
bool STOMP_AreAllResponsesSent(void)
{
    int i;
    stomp_connection_t *sc;
    bool responses_sent;
    bool all_responses_sent = true;  // Assume that all responses have been sent on all connections

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly ncessary, as only the MTP thread should be calling this function
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return true;
    }

    // Iterate over all STOMP connections,
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if (sc->instance != INVALID)
        {
            // Determine if all responses have been sent on this connection, and update whether they have been sent on all connections
            // NOTE: For the receive buffer, Rabbit MQ adds a redundant newline padding at the end of each stomp frame.
            // Therefore a single line feed in the receive buffer is still an empty buffer
            responses_sent = ((sc->usp_record_send_queue.head == NULL) &&
                              (sc->txframe == NULL) &&
                              ( (sc->rxframe_msglen==0) || ((sc->rxframe_msglen==1) && (sc->rxframe[0] == '\n')) )
                             );
            if (responses_sent == false)
            {
                all_responses_sent = false;
            }
        }
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    return all_responses_sent;
}

/*********************************************************************//**
**
** STOMP_ProcessAllSocketActivity
**
** Processes the socket for the specified controller
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  Nothing
**
**************************************************************************/
void STOMP_ProcessAllSocketActivity(socket_set_t *set)
{
    int i;
    stomp_connection_t *sc;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly ncessary, as only the MTP thread should be calling this function
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Iterate over all STOMP connections, processing activity on the ones that are enabled
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if ((sc->instance != INVALID) && (sc->socket_fd != INVALID))
        {
            ProcessStompConnectionSocketActivity(sc, set);
        }
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);
}

/*********************************************************************//**
**
** STOMP_QueueBinaryMessage
**
** Function called to queue a USP record on the specified STOMP connection
**
** \param   msi - pointer to content to send
**                NOTE: Ownership of the payload buffer passes to this function, unless an error is returned
** \param   instance - instance number of the stomp connection in Device.STOMP.Connection.{i}
** \param   controller_queue - name of STOMP queue to send this message to
** \param   agent_queue - name of agent's STOMP queue configured for this connection in the data model
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int STOMP_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char *controller_queue, char *agent_queue, time_t expiry_time)
{
    stomp_connection_t *sc;
    stomp_send_item_t *send_item;
    int err = USP_ERR_GENERAL_FAILURE;
    bool is_duplicate;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        USP_FREE(msi->pbuf);
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return USP_ERR_OK;
    }

    // Exit if unable to find the specified STOMP connection
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        USP_LOG_Error("%s: No internal STOMP connection matching Device.STOMP.Connection.%d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if this is a connect record, adding it at the front of the queue
    if (msi->content_type == kMtpContentType_ConnectRecord)
    {
        QueueUspConnectRecord_STOMP(sc, msi, controller_queue, agent_queue, expiry_time);
        err = USP_ERR_OK;
        goto exit;
    }

    // Do not add this message to the queue, if it is already present in the queue
    // This situation could occur if a notify is being retried to be sent, but is already held up in the queue pending sending
    is_duplicate = IsUspRecordInStompQueue(sc, msi->pbuf, msi->pbuf_len);
    if (is_duplicate)
    {
        USP_FREE(msi->pbuf);
        err = USP_ERR_OK;
        goto exit;
    }

    // If not currently transmitting a frame, then first remove any queued messages that have expired
    if (sc->txframe == NULL)
    {
        RemoveExpiredStompMessages(sc);
    }

    // Add the item to the queue
    send_item = USP_MALLOC(sizeof(stomp_send_item_t));
    send_item->item = *msi;  // NOTE: Ownership of the payload buffer passes to the STOMP message queue
    send_item->controller_queue = USP_STRDUP(controller_queue);
    send_item->agent_queue = USP_STRDUP(agent_queue);
    send_item->expiry_time = expiry_time;

    DLLIST_LinkToTail(&sc->usp_record_send_queue, send_item);
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_StompWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** STOMP_EnableConnection
**
** TCP Connects to the specified STOMP connection
** On exit, the state will be either kStompState_SendingStompFrame (success) or kStompState_Retrying (failure)
**
** \param   sp - pointer to data model parameters specifying the STOMP connection
** \param   stomp_queue - destination queue to use for this device (ie the agent's queue)
**                        NOTE: stomp_queue is allowed to be NULL, as this is valid for the case of the broker provisioning the queue in the CONNECTED frame
**
** \return  USP_ERR_OK if connection was enabled. Note: if the connection failed, it will be retried later
**
**************************************************************************/
int STOMP_EnableConnection(stomp_conn_params_t *sp, char *stomp_queue)
{
    stomp_connection_t *sc;
    int err;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return USP_ERR_OK;
    }

    // Create this STOMP connection, if not already started
    // NOTE: If the code is correct, then the STOMP connection for the specified instance should never exist when this function is called
    sc = FindStompConnByInst(sp->instance);
    if (sc == NULL)
    {
        // Exit if run out of stomp connection slots
        // NOTE: Caller should have already ensured this
        sc = FindUnusedStompConn();
        if (sc == NULL)
        {
            USP_LOG_Error("%s: No more STOMP connections allowed", __FUNCTION__);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }
    }

    // Copy across the connection parameters to use when starting the connection
    CopyStompConnParamsToNext(sc, sp, stomp_queue);


    sc->retry_count = 0;
    sc->failure_code = kStompFailure_None;

    StartStompConnection(sc);
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_StompWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** STOMP_DisableConnection
**
** Disconnects from the specified STOMP connection and frees the specified connection
** This is called from DEVICE_STOMP, if the connection has been disabled
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
** \param   purge_queued_messages - set if the message queue should be purged. This would normally be the case unless we are retrying a connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int STOMP_DisableConnection(int instance, bool purge_queued_messages)
{
    stomp_connection_t *sc;
    stomp_conn_params_t *np;
    int err;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return USP_ERR_OK;
    }

    // Exit if unable to find this connection
    // NOTE: This could occur if the connection has already been disabled
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        USP_LOG_Error("%s: Unable to find STOMP connection for instance=%d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Stop this connection, freeing all state variables
    StopStompConnection(sc, purge_queued_messages);

    // Free the parameters describing the current connection
    USP_SAFE_FREE(sc->host);
    USP_SAFE_FREE(sc->username);
    USP_SAFE_FREE(sc->password);
    USP_SAFE_FREE(sc->virtual_host);
    USP_SAFE_FREE(sc->provisionned_queue);

    // Free the parameters describing the next time the connection is retried
    np = &sc->next_conn_params;
    USP_SAFE_FREE(np->host);
    USP_SAFE_FREE(np->username);
    USP_SAFE_FREE(np->password);
    USP_SAFE_FREE(np->virtual_host);
    USP_SAFE_FREE(sc->next_provisionned_queue);

    // The following code is not strictly necessary, but leaves the structure cleaner
    sc->port = 0;
    sc->enable_encryption = false;
    sc->enable_heartbeats = false;
    sc->incoming_heartbeat_period = 0;
    sc->outgoing_heartbeat_period = 0;
    memset(&sc->retry, 0, sizeof(sc->retry));

    np->port = 0;
    np->enable_encryption = false;
    np->enable_heartbeats = false;
    np->incoming_heartbeat_period = 0;
    np->outgoing_heartbeat_period = 0;
    memset(&np->retry, 0, sizeof(np->retry));


    // Mark this slot as not in use
    sc->instance = INVALID;
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_StompWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** STOMP_ScheduleReconnect
**
** Signals that a STOMP reconnect occurs when all queued messages have been sent
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work, and why
**
** \param   sp - pointer to data model parameters specifying the STOMP connection
** \param   stomp_queue - destination queue to use for this device (ie the agent's queue)
**                        NOTE: stomp_queue is allowed to be NULL, as this is valid for the case of the broker provisioning the queue in the CONNECTED frame
**
** \return  None
**
**************************************************************************/
void STOMP_ScheduleReconnect(stomp_conn_params_t *sp, char *stomp_queue)
{
    stomp_connection_t *sc = NULL;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Exit if unable to find the specified STOMP connection
    sc = FindStompConnByInst(sp->instance);
    if (sc == NULL)
    {
        goto exit;
    }

    // Copy across the connection parameters to use after the reconnect
    CopyStompConnParamsToNext(sc, sp, stomp_queue);


    // Signal a reconnect (which overrides any scheduled resubscribe)
    sc->schedule_reconnect = kScheduledAction_Signalled;
    sc->schedule_resubscribe = kScheduledAction_Off;

    // No need to perform a resubscribe, if we're reconnecting anyway
    // NOTE: The resubscribe flags could have been set if the agent queue was changed in the same USP set transaction as the other STOMP parameters
    sc->perform_resubscribe = 0;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (sc != NULL)
    {
        MTP_EXEC_StompWakeup();
    }
}

/*********************************************************************//**
**
** STOMP_ActivateScheduledActions
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
void STOMP_ActivateScheduledActions(void)
{
    int i;
    stomp_connection_t *sc;
    bool wakeup = false;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Iterate over all STOMP connections, activating all reconnects and resubscribes which have been signalled
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if (sc->schedule_reconnect == kScheduledAction_Signalled)
        {
            sc->schedule_reconnect = kScheduledAction_Activated;
            wakeup = true;
        }

        if (sc->schedule_resubscribe == kScheduledAction_Signalled)
        {
            sc->schedule_resubscribe = kScheduledAction_Activated;
            wakeup = true;
        }
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // Wakeup the STOMP MTP thread, so that it can process the reconnect or resubscribe
    // (This is done outside of the mutex protection, as a slight optimization to avoid unnecessary task switches)
    if (wakeup)
    {
        MTP_EXEC_StompWakeup();
    }
}

/*********************************************************************//**
**
** STOMP_ScheduleResubscribe
**
** Called to schedule an UNSUBSCRIBE frame, then SUBSCRIBE frame with the new STOMP destination
**
** \param   instance - instance number of the stomp connection in Device.STOMP.Connection.{i}
** \param   stomp_queue - destination queue to use for this device (ie the agent's queue)
**                        NOTE: stomp_queue is allowed to be NULL, as this is valid for the case of the broker provisioning the queue in the CONNECTED frame
**
** \return  USP_ERR_OK if connection was enabled. Note: if the connection failed, it will be retried later
**
**************************************************************************/
void STOMP_ScheduleResubscribe(int instance, char *stomp_queue)
{
    stomp_connection_t *sc;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Exit if this stomp connection is not currently enabled
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Store the new agent queue to use
    sc->next_provisionned_queue = AllocateStringIfChanged(sc->next_provisionned_queue, stomp_queue);

    // Schedule a re-subscribe if this connection is already subscribed
    if ((sc->state == kStompState_SendingSubscribeFrame) || (sc->state == kStompState_Running))
    {
        // and the agent's queue is not overridden by the STOMP server, and a reconnect is not pending
        if ((sc->subscribe_dest == NULL) && (sc->schedule_reconnect == kScheduledAction_Off))
        {
            sc->schedule_resubscribe = kScheduledAction_Signalled;
        }
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);

    // Since successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    MTP_EXEC_StompWakeup();
}

/*********************************************************************//**
**
** STOMP_UpdateRetryParams
**
** Called by DEVICE_STOMP, if any of the retry parameters are changed
**
** \param   instance - Instance number of the STOMP connection in Device.STOMP.Connection.{i}
** \param   retry_params - pointer to structure containing parameters controlling a retry
**
** \return  None
**
**************************************************************************/
void STOMP_UpdateRetryParams(int instance, stomp_retry_params_t *retry_params)
{
    stomp_connection_t *sc;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return;
    }

    // Exit if unable to find the specified STOMP connection
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        goto exit;
    }

    // Copy across the connection parameters to use during a retry
    memcpy(&sc->retry, retry_params, sizeof(stomp_retry_params_t));

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);
}

/*********************************************************************//**
**
** STOMP_GetMtpStatus
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.Status for a STOMP connection
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
**
** \return  Status of the STOMP connection
**
**************************************************************************/
mtp_status_t STOMP_GetMtpStatus(int instance)
{
    stomp_connection_t *sc;
    mtp_status_t status;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return kMtpStatus_Down;
    }

    // Exit if unable to find the specified STOMP connection
    // NOTE: This could occur if Device.STOMP.Connection.{i} is disabled
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        status = kMtpStatus_Down;
        goto exit;
    }

    // Exit if connection is not yet up and running
    if (sc->state != kStompState_Running)
    {
        status = kMtpStatus_Down;
        goto exit;
    }

    // Connection is up and running
    status = kMtpStatus_Up;

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);
    return status;
}

/*********************************************************************//**
**
** STOMP_GetConnectionStatus
**
** Function called to get the value of Device.STOMP.Connection.{i}.Status,
** and Device.STOMP.Connection.{i}.LastChangeDate for a STOMP connection
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
** \param   last_change_date - pointer to variable in which to return the time at which
**                             the STOMP connection status changed, or NULL if this parameter is not required
**
** \return  Status of the STOMP connection
**
**************************************************************************/
char *STOMP_GetConnectionStatus(int instance, time_t *last_change_date)
{
    char *status;
    time_t last_change = 0;
    stomp_connection_t *sc;

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&stomp_access_mutex);
        return "Connecting";
    }

    // Exit if unable to find the specified STOMP connection
    // NOTE: This could occur if Device.STOMP.Connection.{i} is disabled
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        status = "Disabled";
        goto exit;
    }

    last_change = sc->last_status_change;
    switch(sc->state)
    {
        case kStompState_Running:
            status = "Enabled";
            break;

        default:
        case kStompState_Idle:
        case kStompState_SendingStompFrame:
        case kStompState_AwaitingConnectedFrame:
        case kStompState_SendingSubscribeFrame:
            status = "Connecting";
            break;
    }

    // If an error condition has occured and hasn't corrected itself yet, then override the status with the error
    if (sc->failure_code != kStompFailure_None)
    {
        USP_ASSERT(sc->state != kStompState_Running);
        status = TEXT_UTILS_EnumToString(sc->failure_code, stomp_failure_strings, NUM_ELEM(stomp_failure_strings));
    }

exit:
    // Save last change date, if required
    if (last_change_date != NULL)
    {
        *last_change_date = last_change;
    }

    OS_UTILS_UnlockMutex(&stomp_access_mutex);
    return status;
}


/*********************************************************************//**
**
** STOMP_GetDestinationFromServer
**
** Function called to get the subscribe-to destination, which is sent from the server in the 'subscribe-dest' STOMP header
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
** \param   buf - pointer to buffer in which to return the subscribe-to destination
** \param   len - length of buffer
**
** \return  None
**
**************************************************************************/
void STOMP_GetDestinationFromServer(int instance, char *buf, int len)
{
    stomp_connection_t *sc;

    // Set default return value
    *buf = '\0';

    OS_UTILS_LockMutex(&stomp_access_mutex);

    // Exit if MTP thread has exited
    if (is_stomp_mtp_thread_exited)
    {
        goto exit;
    }

    // Exit if unable to find the specified STOMP connection
    sc = FindStompConnByInst(instance);
    if (sc == NULL)
    {
        goto exit;
    }

    // Determine the name of the queue to subscribe to
    if (sc->subscribe_dest != NULL)
    {
        USP_STRNCPY(buf, sc->subscribe_dest, len);
    }

exit:
    OS_UTILS_UnlockMutex(&stomp_access_mutex);
}

/*********************************************************************//**
**
** StartStompConnection
**
** TCP Connects to the specified STOMP connection
** On exit, the state will be either kStompState_SendingStompFrame (success) or kStompState_Retrying (failure)
**
** \param   sc - pointer to STOMP connection
**
** \return  None. If the connection failed, it will be retried later
**
**************************************************************************/
void StartStompConnection(stomp_connection_t *sc)
{
    int err;
    char buf[NU_IPADDRSTRLEN];
    bool prefer_ipv6;
    nu_ipaddr_t dst;
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    sa_family_t family;
    fd_set writefds;
    struct timeval timeout;
    int num_sockets;
    int so_err;
    socklen_t so_len = sizeof(so_err);
    nu_ipaddr_t local_mgmt_addr;
    stomp_failure_t stomp_err = kStompFailure_OtherError;
    char *mgmt_interface = "any";   // Used only for debug purposes

    // Copy across the next connection parameters to use into the working state
    CopyStompConnParamsFromNext(sc);

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
    mgmt_interface = nu_macaddr_wan_ifname();
#endif

    USP_LOG_Info("Attempting to connect to host=%s (port=%d, %s) from interface=%s", sc->host, sc->port,
                    (sc->enable_encryption) ? "encrypted" : "unencrypted",
                    mgmt_interface);

    // Initialise state
    InitStompConnection(sc);

    // Get the preference for IPv4 or IPv6, if dual stack
    prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
    // Exit if no WAN address available yet
    if (*last_mgmt_ip_addr == '\0')
    {
        USP_LOG_Warning("%s: Cannot connect, WAN interface is down, or has no IP address", __FUNCTION__);
        goto exit;
    }

    // Exit if unable to convert the WAN address
    err = nu_ipaddr_from_str(last_mgmt_ip_addr, &local_mgmt_addr);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to convert IP address (%s)", __FUNCTION__, last_mgmt_ip_addr);
        goto exit;
    }
#else
    // Set local_mgmt_addr to zero IP address - this denotes that there is no restriction on which local interface connects to the controller
    nu_ipaddr_set_zero(&local_mgmt_addr);
#endif

    // Exit if unable to determine the IP address of the STOMP server
    err = tw_ulib_diags_lookup_host(sc->host, AF_UNSPEC, prefer_ipv6, &local_mgmt_addr, &dst);
    if (err != USP_ERR_OK)
    {
        stomp_err = kStompFailure_ServerDNS;
        goto exit;
    }

    // Exit if unable to make a socket address structure to contact the STOMP server
    err = nu_ipaddr_to_sockaddr(&dst, sc->port, &saddr, &saddr_len);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to determine which address family to use to contact the STOMP server
    // NOTE: This shouldn't fail if tw_ulib_diags_lookup_host() is correct
    err = nu_ipaddr_get_family(&dst, &family);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to create the socket
    sc->socket_fd = socket(family, SOCK_STREAM, 0);
    if (sc->socket_fd == -1)
    {
        USP_ERR_ERRNO("socket", errno);
        goto exit;
    }

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
{
    struct sockaddr_storage waddr;
    socklen_t waddr_len;

    // Create a sockaddr structure containing our local WAN interface that we want to bind to
    err = nu_ipaddr_to_sockaddr(&local_mgmt_addr, 0, &waddr, &waddr_len);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to bind to our local WAN interface
    err = bind(sc->socket_fd, (struct sockaddr *)&waddr, waddr_len);
    if (err == -1)
    {
        USP_ERR_ERRNO("bind", errno);
        goto exit;
    }
}
#endif

    // Exit if unable to set the socket as non blocking
    // We do this before connecting so that we can timeout on connect taking too long
    err = fcntl(sc->socket_fd, F_SETFL, O_NONBLOCK);
    if (err == -1)
    {
        USP_ERR_ERRNO("fcntl", errno);
        goto exit;
    }

    // Exit if unable to connect to the STOMP server
    // NOTE: The connect is performed in non-blocking mode
    err = connect(sc->socket_fd, (struct sockaddr *) &saddr, saddr_len);
    if ((err == -1) && (errno != EINPROGRESS))
    {
        USP_ERR_ERRNO("connect", errno);
        stomp_err = kStompFailure_Connect;
        goto exit;
    }

    // Set up arguments for the select() call
    FD_ZERO(&writefds);
    FD_SET(sc->socket_fd, &writefds);
    timeout.tv_sec = STOMP_CONNECT_TIMEOUT;
    timeout.tv_usec = 0;

    // Exit if the connect timed out
    num_sockets = select(sc->socket_fd + 1, NULL, &writefds, NULL, &timeout);
    if (num_sockets == 0)
    {
        USP_LOG_Error("%s: connect timed out", __FUNCTION__);
        stomp_err = kStompFailure_Connect;
        goto exit;
    }

    // Exit if unable to determine whether the connect was successful or not
    err = getsockopt(sc->socket_fd, SOL_SOCKET, SO_ERROR, &so_err, &so_len);
    if (err == -1)
    {
        USP_ERR_ERRNO("getsockopt", errno);
        stomp_err = kStompFailure_Connect;
        goto exit;
    }

    // Exit if connect was not successful
    if (so_err != 0)
    {
        USP_LOG_Error("%s: async connect failed", __FUNCTION__);
        stomp_err = kStompFailure_Connect;
        goto exit;
    }

    // Perform the SSL handshake (if required), determining the role to use when processing USP messages
    if (sc->enable_encryption)
    {
        err = PerformStompSslConnect(sc);
        if (err != USP_ERR_OK)
        {
            stomp_err = kStompFailure_Authentication;
            goto exit;
        }
    }
    else
    {
        // If encryption is off, then use the Non SSL role
        sc->role = ROLE_NON_SSL;
    }

    // Update the address used to connect to the controller
    err = nu_ipaddr_get_interface_addr_from_sock_fd(sc->socket_fd, sc->mgmt_ip_addr, sizeof(sc->mgmt_ip_addr));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Update the interface used to connect to the controller
    err = nu_ipaddr_get_interface_name_from_src_addr(sc->mgmt_ip_addr, sc->mgmt_if_name, sizeof(sc->mgmt_if_name));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

#ifndef CONNECT_ONLY_OVER_WAN_INTERFACE
#endif

    USP_LOG_Info("Connected to %s (host=%s, port=%d) from interface=%s", nu_ipaddr_str(&dst, buf, sizeof(buf)), sc->host, sc->port, sc->mgmt_if_name);

    // Exit if unable to queue the initial STOMP frame for sending
    err = StartSendingFrame_STOMP(sc);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // If the code gets here, we have successfully set up state to start sending initial frame
    sc->state = kStompState_SendingStompFrame;
    stomp_err = kStompFailure_None;

exit:
    // Wind back state
    if (stomp_err != kStompFailure_None)
    {
        USP_LOG_Error("ERROR: STOMP failed whilst attempting to connect to (host=%s, port=%d)", sc->host, sc->port);
        HandleStompSocketError(sc, stomp_err);
    }
}

/*********************************************************************//**
**
** StopStompConnection
**
** Disconnects from the specified STOMP connection
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
** \param   purge_queued_messages - set if the message queue should be purged. This would normally be the case unless we are retrying a connection
**
** \return  None
**
**************************************************************************/
void StopStompConnection(stomp_connection_t *sc, bool purge_queued_messages)
{
    USP_LOG_Info("Disconnecting from (host=%s, port=%d)", sc->host, sc->port);


    // Free the SSL connection and any saved certificate chain
    if (sc->enable_encryption)
    {
        if (sc->cert_chain != NULL)
        {
            sk_X509_pop_free(sc->cert_chain, X509_free);
            sc->cert_chain = NULL;
        }

        if (sc->ssl != NULL)
        {
            SSL_free(sc->ssl);
            sc->ssl = NULL;
        }
    }

    // Close the socket
    if (sc->socket_fd != -1)
    {
        close(sc->socket_fd);
    }

    sc->socket_fd = -1;
    sc->ssl = NULL;
    sc->cert_chain = NULL;
    sc->role = ROLE_DEFAULT;
    USP_SAFE_FREE(sc->subscribe_dest);
    sc->agent_heartbeat_period = 0;
    sc->server_heartbeat_period = 0;
    sc->next_heartbeat_time = INVALID_TIME;
    sc->last_received_time = INVALID_TIME;
    sc->mgmt_ip_addr[0] = '\0';
    sc->mgmt_if_name[0] = '\0';

    // Free any partially received message
    USP_SAFE_FREE(sc->rxframe);
    sc->rxframe_maxlen = 0;
    sc->rxframe_msglen = 0;
    sc->rxframe_frame_len = 0;
    sc->rxframe_header_len = INVALID;

    // Free any partially transmitted frame
    USP_SAFE_FREE(sc->txframe);
    sc->txframe_len = 0;
    sc->txframe_sent_count = 0;

    // Purge all queued USP messages if required
    if (purge_queued_messages)
    {
        while (sc->usp_record_send_queue.head != NULL)
        {
            RemoveStompQueueItem(sc, (stomp_send_item_t *) sc->usp_record_send_queue.head);
        }
    }

    sc->state = kStompState_Idle;
}

/*********************************************************************//**
**
** InitStompConnection
**
** Called to initialize the instance variables associated with the connection, before starting to connect
** (the data model instance number and connection setup parameters are untouched)
**
** \param   sc - pointer to STOMP connection
**
** \return  None
**
**************************************************************************/
void InitStompConnection(stomp_connection_t *sc)
{
    time_t cur_time;

    cur_time = time(NULL);
    sc->state = kStompState_Idle;
    sc->retry_time = 0;
    #define STOMP_HANDSHAKE_TIMEOUT 10 // Total time allowed to perform the STOMP handshake sequence (ie STOMP, CONNECTED, SUBSCRIBE frames)
    sc->stomp_handshake_timeout = cur_time + STOMP_HANDSHAKE_TIMEOUT;

    sc->schedule_reconnect = kScheduledAction_Off;
    sc->schedule_resubscribe = kScheduledAction_Off;
    sc->perform_resubscribe = 0;

    sc->socket_fd = -1;
    sc->ssl = NULL;
    sc->cert_chain = NULL;
    sc->role = ROLE_DEFAULT;
    sc->subscribe_dest = NULL;

    sc->agent_heartbeat_period = 0;
    sc->server_heartbeat_period = 0;
    sc->next_heartbeat_time = INVALID_TIME;
    sc->last_received_time = INVALID_TIME;

    sc->rxframe = NULL;
    sc->rxframe_msglen = 0;
    sc->rxframe_maxlen = 0;
    sc->rxframe_frame_len = 0;
    sc->rxframe_header_len = INVALID;

    sc->txframe = NULL;
    sc->txframe_len = 0;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = false;

    // Store the time at which we started connecting, unless we want to preserve the time at which an error first occurred
    if (sc->failure_code == kStompFailure_None)
    {
        sc->last_status_change = cur_time;
    }

}

/*********************************************************************//**
**
** PerformStompSslConnect
**
** Perform an SSL connect on the specified socket (which is already connected to the server)
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PerformStompSslConnect(stomp_connection_t *sc)
{
    int err;
    X509 *server_cert;

    // Exit if unable to create a new SSL connection
    sc->ssl = SSL_new(stomp_ssl_ctx);
    if (sc->ssl == NULL)
    {
        USP_LOG_Error("%s: SSL_new() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the pointer to the variable in which to point to the certificate chain collected in the verify callback
    SSL_set_app_data(sc->ssl, &sc->cert_chain);

    // Exit if failed to setup hostname validation with SSL
    err = DEVICE_SECURITY_AddCertHostnameValidation(sc->ssl, sc->host, strlen(sc->host));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Host validation failed.", __FUNCTION__);
        return err;
    }

    // Exit if unable to attach the socket to our SSL connection
    err = SSL_set_fd(sc->ssl, sc->socket_fd);
    if (err != 1)
    {
        USP_LOG_Error("%s: SSL_set_fd() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to successfully perform the SSL handshake
    err = PerformStompSslHandshake(sc);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the handshake was successful, but the server did not provide a certificate
    // This might occur if an insecure anonymous cipher suite is being used
    server_cert = SSL_get_peer_certificate(sc->ssl);
    if (server_cert == NULL)
    {
        USP_LOG_Error("%s: SSL_get_peer_certificate() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }


    X509_free(server_cert);

    // If we have a certificate chain, then determine which role to allow for controllers on this STOMP connection
    if (sc->cert_chain != NULL)
    {
        // Exit if unable to determine the role associated with the trusted root cert
        err = DEVICE_SECURITY_GetControllerTrust(sc->cert_chain, &sc->role);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    // Allow SSL_write() to write a partial message ie not block if it cannot write the full message
    SSL_set_mode(sc->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

    // If the code gets here, then the SSL connection was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PerformStompSslHandshake
**
** Perform an SSL handshake with timeout
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PerformStompSslHandshake(stomp_connection_t *sc)
{
    int err;
    int result;
    time_t timeout_time;
    int timeout_ms;
    socket_set_t set;
    int num_sockets;

    #define STOMP_SSL_HANDSHAKE_TIMEOUT 10  // in seconds
    timeout_time = time(NULL) + STOMP_SSL_HANDSHAKE_TIMEOUT;

    while(true)
    {
        // Exit if connect timed out (calculating the amount of time left)
        timeout_ms = (timeout_time - time(NULL)) * 1000;
        if (timeout_ms < 0)
        {
            USP_LOG_Error("%s: SSL handshake timed out", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Attempt to perform the next step of the SSL handshake
        // NOTE: This function is non-blocking, and will return an eror if it needs to read
        result = SSL_connect(sc->ssl);
        err = SSL_get_error(sc->ssl, result);

        switch(result)
        {
            case 0:
                // SSL Handshake failed gracefully (eg due to mismatch in ciphers supported)
                USP_LOG_ErrorSSL(__FUNCTION__, "SSL_connect() failed", result, err);
                return USP_ERR_INTERNAL_ERROR;
                break;

            case 1:
                // SSL Handshake was successful
                return USP_ERR_OK;
                break;

            default:
                if ((err != SSL_ERROR_WANT_READ) && (err != SSL_ERROR_WANT_WRITE))
                {
                    // SSL Handshake failed for some other reason
                    USP_LOG_ErrorSSL(__FUNCTION__, "SSL_connect() failed", result, err);
                    return USP_ERR_INTERNAL_ERROR;
                }
                break;
        }

        // If the code gets here, then SSL wanted a read or a write, so setup to wait for one on the socket
        SOCKET_SET_Clear(&set);
        if (err == SSL_ERROR_WANT_READ)
        {
            SOCKET_SET_AddSocketToReceiveFrom(sc->socket_fd, timeout_ms, &set);
        }

        if (err == SSL_ERROR_WANT_WRITE)
        {
            SOCKET_SET_AddSocketToSendTo(sc->socket_fd, timeout_ms, &set);
        }

        // Exit if an error occurred whilst waiting for the read/write or timeout
        num_sockets = SOCKET_SET_Select(&set);
        if (num_sockets == -1)
        {
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Code should never get here, as all exits are from the while loop
    return USP_ERR_INTERNAL_ERROR;
}

/*********************************************************************//**
**
** UpdateStompConnectionSockSet
**
** Updates the set of socket fds to read/write from, based on the specific STOMP connection
**
** \param   sc - pointer to STOMP connection
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void UpdateStompConnectionSockSet(stomp_connection_t *sc, socket_set_t *set)
{
    int err;
    time_t cur_time;
    time_t timeout;

    // If we have timed out whilst attempting to perform the initial STOMP handshake (STOMP+CONNECTED+SUBSCRIBE frames)
    // then abort and retry the connection. This probably means the server is down
    if ((sc->state==kStompState_SendingStompFrame) ||
        (sc->state==kStompState_AwaitingConnectedFrame) ||
        (sc->state==kStompState_SendingSubscribeFrame))
    {
        cur_time = time(NULL);
        if (cur_time >= sc->stomp_handshake_timeout)
        {
            USP_LOG_Error("%s: STOMP timed out (in state=%s) whilst performing initial STOMP handshake to (host=%s, port=%d)", __FUNCTION__, state_names[sc->state], sc->host, sc->port);
            HandleStompSocketError(sc, kStompFailure_Timeout);
        }
    }

    // Determine what to do based on the state of the STOMP connection state machine
    switch(sc->state)
    {
        case kStompState_Idle:
            // Do nothing
            break;

        case kStompState_SendingStompFrame:
            timeout = CalcTimeoutToStompHandshakeFailure(sc);
            SOCKET_SET_AddSocketToSendTo(sc->socket_fd, timeout*SECONDS, set);
            break;

        case kStompState_AwaitingConnectedFrame:
            timeout = CalcTimeoutToStompHandshakeFailure(sc);
            SOCKET_SET_AddSocketToReceiveFrom(sc->socket_fd, timeout*SECONDS, set);
            break;

        case kStompState_SendingSubscribeFrame:
            timeout = CalcTimeoutToStompHandshakeFailure(sc);
            SOCKET_SET_AddSocketToSendTo(sc->socket_fd, timeout*SECONDS, set);
            break;

        case kStompState_Running:
            err = HandleStompRunningState(sc, set);
            if (err != USP_ERR_OK)
            {
                return;
            }
            break;

        case kStompState_Retrying:
            cur_time = time(NULL);
            timeout = sc->retry_time - cur_time;
            if (timeout <= 0)
            {
                // It's time to retry
                StartStompConnection(sc);
                timeout = 0;

                // Add this socket, if the connection has started successfully
                if (sc->state == kStompState_SendingStompFrame)
                {
                    SOCKET_SET_AddSocketToSendTo(sc->socket_fd, timeout*SECONDS, set);
                }
            }
            else
            {
                // Wait until it's time to retry
                SOCKET_SET_UpdateTimeout(timeout*SECONDS, set);   // Retry in 5 seconds time
            }
            break;


        default:
            // Code should never get here
            TERMINATE_BAD_CASE(sc->state);
            break;
    }
}

/*********************************************************************//**
**
** HandleStompRunningState
**
** Updates the set of socket fds to read/write from, when in kStompState_Running
**
** \param   sc - pointer to STOMP connection
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  USP_ERR_OK if the socket was not closed down
**
**************************************************************************/
int HandleStompRunningState(stomp_connection_t *sc, socket_set_t *set)
{
    int err;
    time_t timeout;
    time_t cur_time;

    // If not currently transmitting a frame, then see if there are any more to send
    if (sc->txframe == NULL)
    {
        // Exit if unable to form the next message because unable to get agent or controller queue name
        err = GetNextStompMsgToSend(sc);
        if (err != USP_ERR_OK)
        {
            HandleStompSocketError(sc, kStompFailure_Misconfigured);
            return err;
        }
    }

    // Calculate timeout to next heartbeat
    timeout = 3600;                 // Default timeout with no heartbeats
    if (sc->next_heartbeat_time != INVALID_TIME)
    {
        cur_time = time(NULL);
        timeout = sc->next_heartbeat_time - cur_time;
        if (timeout < 0)
        {
            timeout = 0;    // This is only necessary for the case of message processing taking longer than the heartbeat time
        }
    }

    // Always listening, in this state
    SOCKET_SET_AddSocketToReceiveFrom(sc->socket_fd, timeout*SECONDS, set);

    // Want to transmit message (or heartbeat) if one is pending
    if ((sc->txframe != NULL) || (timeout == 0))
    {
        SOCKET_SET_AddSocketToSendTo(sc->socket_fd, timeout*SECONDS, set);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetNextStompMsgToSend
**
** Forms the next stomp frame to send and changes state to start sending it
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successfully formed next STOMP frame to send, USP_ERR_INTERNAL_ERROR if unable to get agent or controller queue name
**
**************************************************************************/
int GetNextStompMsgToSend(stomp_connection_t *sc)
{
    int err = USP_ERR_OK;
    stomp_send_item_t *queued_msg;

    if (sc->perform_resubscribe & SCHEDULE_UNSUBSCRIBE)
    {
        // Start sending an UNSUBSCRIBE frame, if scheduled
        sc->perform_resubscribe &= (~SCHEDULE_UNSUBSCRIBE);
        err = StartSendingFrame_UNSUBSCRIBE(sc);
    }
    else if (sc->perform_resubscribe & SCHEDULE_SUBSCRIBE)
    {
        // Start sending a SUBSCRIBE frame (with new destination), if scheduled
        sc->perform_resubscribe &= (~SCHEDULE_SUBSCRIBE);
        err = StartSendingFrame_SUBSCRIBE(sc);
    }
    else
    {
        // First remove all expired messages from the queue
        RemoveExpiredStompMessages(sc);

        // Start sending the message at the head of the send queue, if ready to accept a new message to send
        // NOTE: Message will be removed from send queue when it has been sent out successfully
        queued_msg = (stomp_send_item_t *) sc->usp_record_send_queue.head;
        if (queued_msg != NULL)
        {
            err = StartSendingFrame_SEND(sc, queued_msg->controller_queue, queued_msg->agent_queue, &queued_msg->item);
        }
    }

    return err;
}

/*********************************************************************//**
**
** ProcessStompConnectionSocketActivity
**
** Processes the sockets that we are waiting on for a controller
**
** \param   sc - pointer to STOMP connection
** \param   set - pointer to socket set structure containing sockets with activity on them
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void ProcessStompConnectionSocketActivity(stomp_connection_t *sc, socket_set_t *set)
{
    // Service sockets which are ready for activity
    switch(sc->state)
    {
        case kStompState_Idle:
            // Do nothing
            break;

        case kStompState_SendingStompFrame:
            if (SOCKET_SET_IsReadyToWrite(sc->socket_fd, set))
            {
                TransmitStompMessage(sc);
            }
            break;

        case kStompState_AwaitingConnectedFrame:
            // Read the (hopefully) CONNECTED frame
            if (SOCKET_SET_IsReadyToRead(sc->socket_fd, set))
            {
                ReceiveStompMessage(sc);
            }
            break;

        case kStompState_SendingSubscribeFrame:
            if (SOCKET_SET_IsReadyToWrite(sc->socket_fd, set))
            {
                USP_ASSERT(sc->txframe != NULL);
                TransmitStompMessage(sc);
            }
            break;

        case kStompState_Running:
            if (SOCKET_SET_IsReadyToRead(sc->socket_fd, set))
            {
                ReceiveStompMessage(sc);
            }

            // Exit if the socket has been closed (eg if a STOMP ERROR frame was received)
            if (sc->socket_fd == INVALID)
            {
                return;
            }

            if (SOCKET_SET_IsReadyToWrite(sc->socket_fd, set))
            {
                if (sc->txframe != NULL)
                {
                    // Send a message (if we have one to send)
                    TransmitStompMessage(sc);
                }
                else
                {
                    // Send a heartbeat (if time to send one)
                    UpdateAgentHeartbeat(sc);
                }
            }
            break;

        case kStompState_Retrying:
            // We would not expect any socket activity whilst in this state
            // Code implementing the retry mechanism is present in UpdateStompConnectionSockSet()
            break;

        default:
            // Code should never get here
            TERMINATE_BAD_CASE(sc->state);
            break;
    }
}

/*********************************************************************//**
**
** CalcTimeoutToStompHandshakeFailure
**
** Calculates the delay (in seconds) left until the initial STOMP handshake has timed out
** The initial STOMP handshake is the sequence with frames STOMP, CONNECTED & SUBSCRIBE
**
** \param   sc - pointer to STOMP connection
**
** \return  Number of seconds left of initial STOMP handshake timeout
**
**************************************************************************/
int CalcTimeoutToStompHandshakeFailure(stomp_connection_t *sc)
{
    time_t cur_time;
    int timeout;

    cur_time = time(NULL);
    timeout = sc->stomp_handshake_timeout - cur_time;
    if (timeout < 0)
    {
        timeout = 0;
    }

    return timeout;
}

/*********************************************************************//**
**
** UpdateAgentHeartbeat
**
** Attempts to send a heartbeat message to the STOMP server if it's time to send one
** If this fails, then the STOMP connection is closed and enters the retrying state
** NOTE: If we are sending or receiving a message, but have not had any communication for a long time,
**       then this function also closes the connection and enters the retrying state
**
** \param   sc - pointer to STOMP connection
**
** \return  None
**
**************************************************************************/
void UpdateAgentHeartbeat(stomp_connection_t *sc)
{
    time_t cur_time;
    time_t delta_time;
    int num_bytes_sent;

    // Exit if heartbeats not enabled yet
    if (sc->next_heartbeat_time == INVALID_TIME)
    {
        return;
    }

    // Exit if it's not yet time to send a heartbeat
    cur_time = time(NULL);
    delta_time = sc->next_heartbeat_time - cur_time;
    if (delta_time > 0)
    {
        return;
    }

    // Attempt to send the heartbeat
    USP_LOG_Debug("Sending heartbeat at time %d", (int)time(NULL));
    #define HEARTBEAT_STR "\n"
    num_bytes_sent = StompWrite(sc, (unsigned char *)HEARTBEAT_STR, sizeof(HEARTBEAT_STR)-1);

    // Exit if an error occurred
    if (num_bytes_sent < 0)
    {
        USP_LOG_Error("%s: STOMP Server write error (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_ReadWrite);
        return;
    }

    // Exit if unable to send the heartbeat. This must be because the STOMP server is down.
    // Exit if 0 bytes were sent. This denotes that the STOMP server has gone down.
    if (num_bytes_sent == 0)
    {
        USP_LOG_Error("%s: STOMP Server disconnected (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_ReadWrite);
        return;
    }

    // Since the heartbeat was successfully sent, update the time at which to send the next heartbeat
    UpdateNextHeartbeatTime(sc);
}

/*********************************************************************//**
**
** TransmitStompMessage
**
** Deal with sending out the message in multiple packets
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int TransmitStompMessage(stomp_connection_t *sc)
{
    int num_bytes_sent;
    unsigned char *buf;
    int bytes_to_attempt;
    char *agent_queue;
    stomp_send_item_t *cur_msg;
    mtp_content_type_t type;

    // Determine what to send
    buf = &sc->txframe[ sc->txframe_sent_count ];
    bytes_to_attempt = sc->txframe_len - sc->txframe_sent_count;

    // Attempt to send the rest of the frame
    num_bytes_sent = StompWrite(sc, buf, bytes_to_attempt);

    // Exit if an error occurred
    if (num_bytes_sent < 0)
    {
        // The USP Record has not been removed from the send queue, and so will be re-sent after connection to the STOMP server has been re-established
        USP_LOG_Error("%s: STOMP Server write error (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_ReadWrite);
        return USP_ERR_OK;
    }

    // Exit if 0 bytes were sent. This denotes that the STOMP server has gone down.
    if (num_bytes_sent == 0)
    {
        USP_LOG_Error("%s: STOMP Server disconnected (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_ReadWrite);
        return USP_ERR_OK;
    }

    // If something was sent, we don't need to send out a heartbeat for some time to come
    if (num_bytes_sent > 0)
    {
        UpdateNextHeartbeatTime(sc);
    }

    // Exit if the frame has not been sent out entirely
    if (sc->txframe_sent_count + num_bytes_sent < sc->txframe_len)
    {
        sc->txframe_sent_count += num_bytes_sent;
        return USP_ERR_OK;
    }

    // The frame has been sent out entirely, so remove the frame
    USP_FREE(sc->txframe);
    sc->txframe = NULL;
    sc->txframe_len = 0;

    // Also, if it contained an embedded USP message, then remove that from the send queue
    if (sc->txframe_contains_usp_record)
    {
        // Remove the USP message from the send queue
        cur_msg = (stomp_send_item_t *) sc->usp_record_send_queue.head;
        type = cur_msg->item.content_type;  // Save content_type, before freeing the message
        RemoveStompQueueItem(sc, cur_msg);

        // If the USP message was a Disconnect record, then close the socket and go into the retrying state
        // NOTE: USP Disconnect records closing an E2E session (kMtpContentType_E2E_SessTermination) do not close the connection
        if (type == kMtpContentType_DisconnectRecord)
        {
            StartSendingFrame_DISCONNECT(sc);
            TransmitStompMessage(sc);
            HandleStompSocketError(sc, kStompFailure_OtherError);
        }
    }

    // Move to next state (if required)
    switch(sc->state)
    {
        case kStompState_SendingStompFrame:
            sc->state = kStompState_AwaitingConnectedFrame;
            break;

        case kStompState_SendingSubscribeFrame:
            sc->state = kStompState_Running;
            sc->failure_code = kStompFailure_None;  // The reason this is set here is that we don't want to change the previous Connection.{i}.Status until it becomes successful
            sc->last_status_change = time(NULL);
            sc->retry_count = 0;        // Since successful, reset the retry count

            // Notify the data model of the role to use for controllers connected to this STOMP connection
            // This will also unblock the Boot! event, subscriptions, and restarting of operations
            agent_queue = (sc->subscribe_dest != NULL) ? sc->subscribe_dest : sc->provisionned_queue;
            DM_EXEC_PostStompHandshakeComplete(sc->instance, agent_queue, sc->role);
            break;

        default:
        case kStompState_Idle:
        case kStompState_AwaitingConnectedFrame:
        case kStompState_Running:
        case kStompState_Retrying:
            // No change in state
            break;
    }

    // NOTE: An optimisation here, would be that if there are other messages pending, to attempt to send them here also
    // However the code is much simpler if we just send one message at a time, and anyway it is unlikely we will have many
    // messages queued to send at the same time.
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ReceiveStompMessage
**
** Deal with concatenating the received packets until we have a full message
**
** \param   sc - pointer to STOMP connection
**
** \return  None (errors are handled internally by this function)
**
**************************************************************************/
void ReceiveStompMessage(stomp_connection_t *sc)
{
    unsigned char buf[1024];
    int num_bytes;
    int bytes_pending;
    int err;
    int ssl_err;

    // Perform a simple recv() if connection is not encrypted
    if (sc->enable_encryption == false)
    {
        num_bytes = recv(sc->socket_fd, buf, sizeof(buf), 0);

        // Exit if an error occurred
        if (num_bytes < 0)
        {
            USP_LOG_Error("%s: STOMP Server read error (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
            HandleStompSocketError(sc, kStompFailure_ReadWrite);
            return;
        }

        // Exit if 0 bytes were received. This denotes that the STOMP server has gone down.
        if (num_bytes == 0)
        {
            USP_LOG_Error("%s: STOMP Server disconnected (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
            HandleStompSocketError(sc, kStompFailure_ReadWrite);
            return;
        }

        ReceiveStompMessageInner(sc, buf, num_bytes);
        return;
    }

    // Otherwise the connection is encrypted
    // Keep reading until all bytes have been read - this is necessary because OpenSSL has a larger read buffer than us
    // and so OpenSSL may consume all the bytes from the socket.
    bytes_pending = 1;  // Assume at least 1 byte is available to read, because the select() call indicated there was data to read
    while (bytes_pending > 0)
    {
        // Read from SSL
        num_bytes = SSL_read(sc->ssl, buf, sizeof(buf));

        // Determine if there was any error
        ssl_err = SSL_get_error(sc->ssl, num_bytes);
        switch(ssl_err)
        {
            case SSL_ERROR_NONE:
                // Exit if an error occurred
                // NOTE: I don't think this case can occur in practice
                if (num_bytes < 0)
                {
                    USP_LOG_Error("%s: STOMP Server read error (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
                    HandleStompSocketError(sc, kStompFailure_ReadWrite);
                    return;
                }

                // If there was no SSL error, but no bytes to read, then let the SSL_pending() indicate whether to exit the loop
                // NOTE: I don't think this case can occur in practice
                if (num_bytes == 0)
                {
                    break;
                }

                // Exit if an error occurred when attempting to concatenate the bytes read to the end of the receive buffer
                err = ReceiveStompMessageInner(sc, buf, num_bytes);
                if (err != USP_ERR_OK)
                {
                    return;
                }
                break;

            case SSL_ERROR_ZERO_RETURN:
                // Exit if the STOMP server has gone down
                USP_LOG_Error("%s: STOMP Server disconnected (host %s, port %d). Retrying.", __FUNCTION__, sc->host, sc->port);
                HandleStompSocketError(sc, kStompFailure_ReadWrite);
                return;
                break;

            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                // Exit if a renegotiation is being initiated by the server - we let the select drive this
                return;
                break;

            default:
            case SSL_ERROR_SYSCALL:
                // Exit if any other error occurred
                USP_LOG_Error("%s: SSL error %d occurred on (host %s, port %d). Retrying.", __FUNCTION__, ssl_err, sc->host, sc->port);
                HandleStompSocketError(sc, kStompFailure_ReadWrite);
                return;
                break;
        }

        // Exit if the socket has been closed down due to an error on the connection (eg received a STOMP ERROR frame)
        if (sc->socket_fd == INVALID)
        {
            return;
        }

        // See if any more data is pending
        bytes_pending = SSL_pending(sc->ssl);
    }

}

/*********************************************************************//**
**
** ReceiveStompMessageInner
**
** Called for each message fragment received from the socket or SSL.
** This function concatenates the fragments together into a receive buffer,
** then detects STOMP frames in the receive buffer and removes STOMP frames from it
**
** \param   sc - pointer to STOMP connection
** \param   buf - pointer to buffer containing the message fragment
** \param   num_bytes - number of bytes in the message fragment
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
int ReceiveStompMessageInner(stomp_connection_t *sc, unsigned char *buf, int num_bytes)
{
    int new_len;
    int msg_size;
    int err;

    // Exit if no bytes to concatenate
    if (num_bytes <= 0)
    {
        return USP_ERR_OK;
    }

    // Log the time at which the last message fragment was received (this is an alternative to receiving the STOMP server heartbeat)
    sc->last_received_time = time(NULL);

    // Increase size of rx buffer, if required
    new_len = sc->rxframe_msglen + num_bytes;

    // Prevent rogue controllers from crashing agent by setting an arbitrary message size limit
    if (new_len > MAX_USP_MSG_LEN)
    {
        USP_LOG_Error("ERROR: STOMP Connection to (host %s, port %d) receiving a message >%d bytes long. Closing connection.", sc->host, sc->port, MAX_USP_MSG_LEN);
        HandleStompSocketError(sc, kStompFailure_OtherError);
        return USP_ERR_INTERNAL_ERROR;
    }

    if (new_len > sc->rxframe_maxlen)
    {
        // Increase receive buffer size
        sc->rxframe = USP_REALLOC(sc->rxframe, new_len);
        sc->rxframe_maxlen = new_len;
    }

    // Copy into the receive buffer
    memcpy(&sc->rxframe[sc->rxframe_msglen], buf, num_bytes);
    sc->rxframe_msglen = new_len;

    // Exit if an error occurred whilst parsing the STOMP header
    err = IsStompMsgComplete(sc, &msg_size);   // NOTE: rxframe can contain more than one message, hence the need to use msg_size rather than rxframe_msglen
    if (err != USP_ERR_OK)
    {
        HandleStompSocketError(sc, kStompFailure_OtherError);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Keep processing messages received in the buffer, until there are no more complete messages in the buffer
    while (msg_size > 0)
    {
        // Process the message
        HandleStompMessage(sc, msg_size);

        // Determine whether there is another complete message in the buffer
        // Exit if an error occurred whilst parsing the STOMP header
        err = IsStompMsgComplete(sc, &msg_size);
        if (err != USP_ERR_OK)
        {
            HandleStompSocketError(sc, kStompFailure_OtherError);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StompWrite
**
** Attempt to send the specified data to the STOMP server
**
** \param   sc - pointer to STOMP connection
** \param   buf - pointer to buffer containing data to send
** \param   bytes_to_attempt - number of bytes of data to attempt to send
**
** \return  >0  Number of bytes sent (which might be less than the number to attempt)
**          0   indicates that the STOMP server has disconnected
**          <0  indicates that another error has occurred
**
**************************************************************************/
int StompWrite(stomp_connection_t *sc, unsigned char *buf, int bytes_to_attempt)
{
    int num_bytes_sent = 0;
    int err;
    int retry_count;

    // Perform a simple send() if connection is not encrypted
    if (sc->enable_encryption == false)
    {
        num_bytes_sent = send(sc->socket_fd, buf, bytes_to_attempt, 0);
        goto exit;
    }

    // Code below is complex because a renegotiation could occur, and open SSL requires that we retry the EXACT same SSL call
    // We cope with this by retrying the SSL call until the retry has completed (or failed)
    // This code blocks until the retry has completed, or the retry has timed out
    #define ONE_SECOND_IN_MICROSECONDS (1000000)
    #define SSL_RETRY_SLEEP (ONE_SECOND_IN_MICROSECONDS/20)             // Retry 20 times a second
    #define SSL_RETRY_TIMEOUT  (5*ONE_SECOND_IN_MICROSECONDS)           // Retry for upto 5 seconds
    #define MAX_SSL_RETRY_COUNT  (SSL_RETRY_TIMEOUT/SSL_RETRY_SLEEP)
    err = SSL_ERROR_WANT_WRITE;
    retry_count = 0;
    while ( (retry_count < MAX_SSL_RETRY_COUNT) &&
            ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE)) )
    {
        // Try sending
        num_bytes_sent = SSL_write(sc->ssl, buf, bytes_to_attempt);
        if (num_bytes_sent > 0)
        {
            break;
        }

        // Determine whether to retry this call until the write has occurred - this is needed if a renegotiation occurs
        err = SSL_get_error(sc->ssl, num_bytes_sent);
        USP_LOG_ErrorSSL(__FUNCTION__, "SSL_write() failed", num_bytes_sent, err);
        usleep(SSL_RETRY_SLEEP);
        retry_count++;

        // Exit if STOMP server has disconnected (after logging failure codes)
        if (num_bytes_sent==0)
        {
            break;
        }
    }

    // Log an error if retry timed out
    if (retry_count == MAX_SSL_RETRY_COUNT)
    {
        USP_LOG_Error("%s: SSL Renegotiation timed out", __FUNCTION__);
    }

exit:
    return num_bytes_sent;
}

/*********************************************************************//**
**
** IsStompMsgComplete
**
** Determine whether we have received a complete STOMP message
** NOTE: This function also removes all heartbeat messages
** Messages have the format:-
**          COMMAND
**          header1:value
**          header2:value
**            (blank line)
**          Body
**          NULL terminator (NOTE: NULL terminator may be present in the body if a content-length: header is present)
**
** \param   sc - pointer to STOMP connection
** \param   msg_size - pointer to variable in which to return the size of the message received, or 0 if no complete message to process received yet
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
int IsStompMsgComplete(stomp_connection_t *sc, int *msg_size)
{
    unsigned char *p;
    int i;
    int len = sc->rxframe_msglen;   // Convenience variable and optimisation
    int err;

    // Default to returning 'message not complete yet'
    *msg_size = 0;

    // Remove any received heartbeat messages (we need to do this here as heartbeat messages may be interleaved between STOMP frames)
    RemoveReceivedHeartBeats(sc);

    // Exit if no receive buffer left after removing heartbeat messages
    if ((sc->rxframe == NULL) || (sc->rxframe_msglen == 0))
    {
        *msg_size = 0;
        return USP_ERR_OK;
    }

    // Wait for all of the stomp headers to be retrieved
    if (sc->rxframe_header_len == INVALID)
    {
        // Exit if an error occurred when parsing the stomp headers
        err = ParseStompHeaders(sc, &sc->rxframe_header_len);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if headers not fully received yet
        if (sc->rxframe_header_len == INVALID)
        {
            return USP_ERR_OK;
        }
    }

    // From this point on, all of the stomp headers for this frame have been retrieved,
    // and we have determined whether the "content-length:" header is present

    // Exit if a 'content-length:' header has been received, and we have received enough bytes to make a full frame
    if (sc->rxframe_frame_len != 0)
    {
        if (len >= sc->rxframe_frame_len)
        {
            *msg_size = sc->rxframe_frame_len;
            return USP_ERR_OK;
        }

        // If the code gets here, then we're still waiting to receive 'content-length:' bytes of payload
        *msg_size = 0;
        return USP_ERR_OK;
    }

    // Otherwise, if the "content-length:" header was not received, then the frame is terminated by NULL
    p = sc->rxframe;
    for (i=0; i<len; i++)
    {
        if (*p++ == '\0')
        {
            *msg_size = i+1;     // Plus 1 to include NULL terminator
            return USP_ERR_OK;
        }
    }

    // If the code gets here, then no full frame has been received
    *msg_size = 0;
    return USP_ERR_OK;
}


/*********************************************************************//**
**
** RemoveReceivedHeartBeats
**
** Removes all leading heartbeat messages from the receive buffer
** NOTE: This may result in the receive buffer becoming empty - the caller must check for this
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
void RemoveReceivedHeartBeats(stomp_connection_t *sc)
{
    unsigned char *p;
    int heartbeat_bytes;
    int len = sc->rxframe_msglen;   // Convenience variable and optimisation

    // Exit if no receive buffer left
    if ((sc->rxframe == NULL) || (sc->rxframe_msglen == 0))
    {
        return;
    }

    // Determine how many bytes are heartbeat messages
    p = sc->rxframe;
    heartbeat_bytes = 0;
    while ((*p == '\n') && (heartbeat_bytes < len))
    {
        heartbeat_bytes++;
        p++;
    }

    // Remove all heartbeat messages (skip leading '\n')
    if (heartbeat_bytes > 0)
    {
        USP_LOG_Debug("Received %d heartbeats at time %d", heartbeat_bytes, (int)time(NULL));
        RemoveMessageFromRxBuf(sc, heartbeat_bytes);
        sc->last_received_time = time(NULL);  // NOTE: Not strictly necessary as it will already have been set in ReceiveStompMessageInner()
    }
}

/*********************************************************************//**
**
** ParseStompHeaders
**
** Parses the Stomp Headers in the frame, if they have not already been parsed
**
** \param   sc - pointer to STOMP connection
** \param   header_size - pointer to variable in which to return the size of the headers
**                        in the fames, or INVALID if all headers have not been received yet
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
int ParseStompHeaders(stomp_connection_t *sc, int *header_size)
{
    int i;
    unsigned char *p;
    int header_len;
    int content_len;
    int len = sc->rxframe_msglen;   // Convenience variable and optimisation
    int err;

    // Determine if we have read all stomp headers
    header_len = INVALID;
    p = sc->rxframe;
    for (i=0; i<len; i++)
    {
        // Detect the end of all stomp headers (denoted by a blank line)
        // Code is complicated by the fact we have to deal with optional carriage return character
        if ( (*p == '\n') && ( ((i >= 1) && (p[-1] == '\n')) ||                 // LF case
                               ((i >= 2) && (p[-1] == '\r') && (p[-2] == '\n')) // CR-LF case
                             )
           )
        {
            header_len = i + 1;     // Plus 1 to include this '\n' character
            break;
        }

        // Move to next character
        p++;
    }

    // Exit if we do not have all of the stomp headers for this frame yet
    if (header_len == INVALID)
    {
        *header_size = INVALID;
        return USP_ERR_OK;
    }

    // Since we have all stomp headers, see if any of them is "content-length:"
    *header_size = header_len;
    err = ParseContentLengthHeader(sc, &content_len);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    if (content_len == 0)
    {
        sc->rxframe_frame_len = 0;    // "content-length:" header not found
    }
    else
    {
        // "content-length:" header found, so calculate the total frame size
        sc->rxframe_frame_len = header_len + content_len + 1; // Plus 1 to include NULL terminator at the end of the frame

        // Exit if the parsed content length is too long
        if (sc->rxframe_frame_len > MAX_USP_MSG_LEN)
        {
            USP_LOG_Error("%s: Parsed STOMP content length (%d) would take frame length over %d bytes", __FUNCTION__, content_len, MAX_USP_MSG_LEN);
            return USP_ERR_RESOURCES_EXCEEDED;
        }
    }

    // NOTE: We do not check that the destination header matches the queue that we subscribed to because
    // this function is called for all STOMP frames received (and the CONNECTED frame does not include the destination header)

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseContentLengthHeader
**
** Parses value of the "content-length:" header, if present in the frame
** NOTE: When this function is called, we have already validated that we have all headers
**
** \param   sc - pointer to STOMP connection
** \param   content_length - pointer to variable in which to return the parsed "content-length:" header
**                           If the header is not present, this is set to 0
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
int ParseContentLengthHeader(stomp_connection_t *sc, int *content_length)
{
    char buf[12];
    bool is_present;
    int err;

    // Set default, if failed to parse header
    *content_length = 0;

    // Exit if no "content-length:" header was found
    is_present = GetStompHeaderValue("content-length:", sc->rxframe, sc->rxframe_msglen, buf, sizeof(buf));
    if (is_present == false)
    {
        return USP_ERR_OK;
    }

    // Exit if failed to convert the content length value
    err = TEXT_UTILS_StringToUnsigned(buf, (unsigned *)content_length);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then the content length header was present and converted successfully
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** HandleStompMessage
**
** Handle a received STOMP message
**
** \param   sc - pointer to STOMP connection
** \param   msg_size - size of message (including any terminator)
**
** \return  None - this function handles errors that it encounters
**
**************************************************************************/
void HandleStompMessage(stomp_connection_t *sc, int msg_size)
{
    switch(sc->state)
    {
        case kStompState_AwaitingConnectedFrame:
            HandleRxMsg_AwaitingConnectedFrameState(sc, msg_size);
            break;

        case kStompState_Running:
            HandleRxMsg_RunningState(sc, msg_size);
            break;

        case kStompState_Idle:
        case kStompState_SendingStompFrame:
        case kStompState_SendingSubscribeFrame:
            // Code should never get here
            USP_LOG_Error("WARNING: Ignoring unexpected message whilst STOMP connection was in state %d\n", sc->state);
            break;

        default:
            // Code should never get here
            TERMINATE_BAD_CASE(sc->state);
            break;
    }

    // Remove this message from the head of the buffer, now that we have processed it
    // NOTE: Rx Buffer could have been removed by HandleRxMsg_AwaitingConnectedFrameState, if no agent queue name setup to subscribe to
    if (sc->rxframe != NULL)
    {
        RemoveMessageFromRxBuf(sc, msg_size);

        sc->rxframe_frame_len = 0;
        sc->rxframe_header_len = INVALID;
    }
}

/*********************************************************************//**
**
** HandleRxMsg_AwaitingConnectedFrameState
**
** Handle a STOMP message received when in the AwaitingConnectedFrame state
**
** \param   sc - pointer to STOMP connection
** \param   msg_size - size of message (including any terminator)
**
** \return  None - this function handles errors that it encounters
**
**************************************************************************/
void HandleRxMsg_AwaitingConnectedFrameState(stomp_connection_t *sc, int msg_size)
{
    int err;

    // Exit if this is not the expected CONNECTED frame
    if (IsFrame("CONNECTED", sc->rxframe, msg_size) == false)
    {
        USP_LOG_Error("%s: Received unexpected STOMP frame on connection to (host %s, port %d): Expected CONNECTED.", __FUNCTION__, sc->host, sc->port);
        USP_LOG_Info("Got frame:- %s", sc->rxframe);
        HandleStompSocketError(sc, kStompFailure_Authentication);
        return;
    }

    USP_LOG_Info("Received CONNECTED frame from (host=%s, port=%d)", sc->host, sc->port);
    USP_PROTOCOL("%s", sc->rxframe);

    // Extract data from the STOMP headers contained in the CONNECTED frame
    ParseConnectedFrame(sc, sc->rxframe, msg_size);

    // Exit if unable to create a subscribe frame. If this fails, it is because we don't know which queue to subscribe to
    err = StartSendingFrame_SUBSCRIBE(sc);
    if (err != USP_ERR_OK)
    {
        // Attempt to send disconnect frame before closing the socket
        StartSendingFrame_DISCONNECT(sc);
        TransmitStompMessage(sc);
        HandleStompSocketError(sc, kStompFailure_Misconfigured);
        return;
    }


    // Move to the SendingSubscribeFrame state
    sc->state = kStompState_SendingSubscribeFrame;
}

/*********************************************************************//**
**
** HandleRxMsg_RunningState
**
** Handle a STOMP message received when in the Running state
**
** \param   sc - pointer to STOMP connection
** \param   msg_size - size of message (including any terminator)
**
** \return  None - this function handles errors that it encounters
**
**************************************************************************/
void HandleRxMsg_RunningState(stomp_connection_t *sc, int msg_size)
{
    #define MAX_STOMP_HEADER_VALUE_LEN  256
    int offset;
    unsigned char *pbuf;
    int pbuf_len;
    char reply_to_dest[MAX_STOMP_HEADER_VALUE_LEN];
    char content_type[64];
    bool is_present;
    char time_buf[MAX_ISO8601_LEN];
    mtp_reply_to_t mtp_reply_to = {0};

    // Exit if this is not the expected MESSAGE frame
    if (IsFrame("MESSAGE", sc->rxframe, msg_size) == false)
    {
        // Ignore RECEIPT frames NOTE: We should not receive these because we never request them
        if (IsFrame("RECEIPT", sc->rxframe, msg_size) == true)
        {
            USP_LOG_Warning("%s: Ignoring STOMP RECEIPT frame (as not requested on host %s, port %d)", __FUNCTION__, sc->host, sc->port);
            return;
        }

        USP_LOG_Error("%s: Received frame other than MESSAGE from (host %s, port %d): Scheduling reconnect.", __FUNCTION__, sc->host, sc->port);
        USP_LOG_Info("Got frame:- %s", sc->rxframe);
        HandleStompSocketError(sc, kStompFailure_OtherError);
        return;
    }

    // Fill In the mtp_reply_to_t structure, based on whether we have a 'reply-to' field or not
    mtp_reply_to.protocol = kMtpProtocol_STOMP;
    mtp_reply_to.stomp_instance = sc->instance;
    is_present = GetStompHeaderValue("reply-to-dest:", sc->rxframe, msg_size, reply_to_dest, sizeof(reply_to_dest));
    if ((is_present) && (reply_to_dest[0] != '\0'))
    {
        mtp_reply_to.is_reply_to_specified = true;
        mtp_reply_to.stomp_dest = reply_to_dest;
    }

    // Check the content-type
    is_present = GetStompHeaderValue("content-type:", sc->rxframe, msg_size, content_type, sizeof(content_type));
    if (is_present)
    {
        // Only allow "application/vnd.bbf.usp.msg" fames
        if (strcmp(content_type, BBF_STOMP_CONTENT_TYPE) != 0)
        {
            USP_LOG_Error("%s: Received STOMP frame with incorrect content-type (=%s) on connection to (host %s, port %d)", __FUNCTION__, content_type, sc->host, sc->port);
            HandleStompSocketError(sc, kStompFailure_OtherError);
            return;
        }
    }
    else
    {
        USP_LOG_Error("%s: Received STOMP frame with missing content-type header on connection to (host %s, port %d)", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_OtherError);
        return;
    }

    // Calculate payload start and size
    pbuf = &sc->rxframe[sc->rxframe_header_len];
    pbuf_len = msg_size - sc->rxframe_header_len - 1;     // Minus 1 to not include STOMP frame NULL terminator
    if (pbuf_len == 0)
    {
        USP_LOG_Error("%s: Received STOMP frame with no payload on connection to (host %s, port %d)", __FUNCTION__, sc->host, sc->port);
        HandleStompSocketError(sc, kStompFailure_OtherError);
        return;
    }
    USP_ASSERT(pbuf[pbuf_len] == '\0');

    // Make STOMP header into a NULL terminated string
    USP_ASSERT(pbuf[-2]=='\n');
    pbuf[-2] = '\0';

    // Skip leading LF character when printing the STOMP header
    offset = (sc->rxframe[0]=='\n') ? 1 : 0;

    // Log received message
    iso8601_cur_time(time_buf, sizeof(time_buf));
    USP_PROTOCOL("\n");
    USP_LOG_Info("Message received at time %s, from host %s over STOMP", time_buf, sc->host);
    USP_PROTOCOL("%s", &sc->rxframe[offset]);

    // Send the USP Record to the data model thread for processing
    DM_EXEC_PostUspRecord(pbuf, pbuf_len, sc->role, &mtp_reply_to);
}

/*********************************************************************//**
**
** RemoveMessageFromRxBuf
**
** Removes the specified number of bytes from the beginning of the STOMP connection's receive buffer
**
** \param   sc - pointer to STOMP connection
** \param   msg_size - size of message (including any terminator)
**
** \return  None
**
**************************************************************************/
void RemoveMessageFromRxBuf(stomp_connection_t *sc, int msg_size)
{
    int new_size;

    USP_ASSERT(sc->rxframe != NULL);
    USP_ASSERT(msg_size > 0);
    USP_ASSERT(sc->rxframe_msglen >= msg_size);

    // Remove this message from the head of the buffer, now that we have processed it
    new_size = sc->rxframe_msglen - msg_size;
    if (new_size == 0)
    {
        // No other messages in the buffer, so just free it
        USP_FREE(sc->rxframe);
        sc->rxframe = NULL;
        sc->rxframe_msglen = 0;
        sc->rxframe_maxlen = 0;
    }
    else
    {
        // Move the next message in the buffer down to the start of the buffer
        memmove(&sc->rxframe[0], &sc->rxframe[msg_size], new_size);
        sc->rxframe_msglen = new_size;
    }
}

/*********************************************************************//**
**
** IsFrame
**
** Determines if the received STOMP message is the specified frame
**
** \param   frame_name - STOMP frame message type to match against
** \param   msg - pointer to message to parse
** \param   msg_len - size of message (including any terminator)
**
** \return  true if the frame is a connected frame containing all the right header values.
**          false otherwise
**
**************************************************************************/
bool IsFrame(char *frame_name, unsigned char *msg, int msg_len)
{
    unsigned char c;
    int name_len;

    // Skip leading CR LF
    c = *msg;
    while ((c=='\n') || (c=='\r'))   // NOTE: Do not have to guard against msg_len going less than zero, because msg_len is already terminated by a NULL character
    {
        msg++;
        msg_len--;
        c = *msg;
    }

    // Exit if message does not match the specified frame name
    name_len = strlen(frame_name);
    if (strncmp((char *)msg, frame_name, name_len) != 0)
    {
        return false;
    }

    // If the code gets here, then the message is the specified frame name
    return true;
}

/*********************************************************************//**
**
** ParseConnectedFrame
**
** Extracts values from the heartbeat and subscribe-dest STOMP headers in the Connected frame
**
** \param   sc - pointer to STOMP connection
** \param   msg - pointer to message to parse
** \param   msg_len - size of message (including any terminator)
**
** \return  None
**
**************************************************************************/
void ParseConnectedFrame(stomp_connection_t *sc, unsigned char *msg, int msg_len)
{
    char buf[256];
    bool is_present;
    int num_parsed;
    int sx, sy;     // NOTE: naming of these variables comes from https://stomp.github.io/stomp-specification-1.2.html#Heart-beating
    int period_ms;

    // Extract the heartbeat STOMP header
    is_present = GetStompHeaderValue("heart-beat:", msg, msg_len, buf, sizeof(buf));
    if (is_present)
    {
        num_parsed = sscanf(buf, "%d,%d", &sx, &sy);
        if (num_parsed == 2)
        {
            // Handle negotiated agent heartbeat period
            if ((sc->enable_heartbeats == false) || (sc->outgoing_heartbeat_period == 0) || (sy == 0))
            {
                // Case of outgoing heartbeats disabled (either by our data model, or the STOMP server)
                sc->agent_heartbeat_period = 0;
            }
            else
            {
                // Case of outgoing heartbeats enabled
                // Convert outgoing heartbeat period to nearest second (rounded down)
                period_ms = MAX(sc->outgoing_heartbeat_period, sy);
                sc->agent_heartbeat_period = (period_ms >= 1000) ? period_ms/1000 : 1;
            }

            // Handle negotiated server heartbeat period
            if ((sc->enable_heartbeats == false) || (sc->incoming_heartbeat_period == 0) || (sx == 0))
            {
                // Case of incoming heartbeats disabled (either by our data model, or the STOMP server)
                sc->server_heartbeat_period = 0;
            }
            else
            {
                // Case of incoming heartbeats enabled
                // Convert incoming heartbeat period to nearest second (rounded down)
                period_ms = MAX(sc->incoming_heartbeat_period, sx);
                sc->server_heartbeat_period = (period_ms >= 1000) ? period_ms/1000 : 1;
            }
        }
        else
        {
            USP_LOG_Warning("%s: Received heart-beat STOMP header ('%s') is incorrectly formatted", __FUNCTION__, buf);
        }
    }

    // Extract the subscribe-dest STOMP header
    is_present = GetStompHeaderValue("subscribe-dest:", msg, msg_len, buf, sizeof(buf));
    if (is_present)
    {
        sc->subscribe_dest = USP_STRDUP(buf);
    }
}

/*********************************************************************//**
**
** GetStompHeaderValue
**
** Copies the value associated with the specified STOMP header into a return buffer
**
** \param   header - pointer to string containing header to search for
** \param   msg - pointer to message to parse
** \param   msg_len - size of message (including any terminator)
** \param   buf - pointer to buffer in which to return the value associated with the header
** \param   len - length of return buffer
**
** \return  true if the header is present, false otherwise
**
**************************************************************************/
bool GetStompHeaderValue(char *header, unsigned char *msg, int msg_len, char *buf, int len)
{
    int header_len;

    header_len = strlen(header);
    while (msg_len > header_len)
    {
        // After a newline...
        if (*msg == '\n')
        {
            // Skip newline
            msg++;
            msg_len--;

            // ...see if the header matches
            if (memcmp(msg, header, header_len)==0)
            {
             // ...if so, skip to the header's value
                msg += header_len;
                msg_len -= header_len;

                // and copy the value into the return buffer
                while ((msg_len > 0) && (len > 1) && (*msg != '\n') && (*msg != '\r') && (*msg != '\0'))
                {
                    *buf = *msg;

                    msg++;   msg_len--;
                    buf++;   len--;
                }
                *buf = '\0';
                return true;
            }
        }
        else
        {
            // Move to next character
            msg++;
            msg_len--;
        }
    }

    // If the code gets here, then the header was not found
    return false;
}

/*********************************************************************//**
**
** HandleStompSocketError
**
** Undo all state associated with the connection, and go to state kStompState_Retrying
**
** \param   sc - pointer to STOMP connection
** \param   failure_code - cause of STOMP connection failure
**
** \return  Nothing
**
**************************************************************************/
void HandleStompSocketError(stomp_connection_t *sc, stomp_failure_t failure_code)
{
    unsigned wait_time;


    // Save cause of failure
    // Update the time at which an error occured, if it is a different error than last time (or the first time the error has occurred)
    if (sc->failure_code != failure_code)
    {
        sc->last_status_change = time(NULL);
        sc->failure_code = failure_code;
    }

    // Undo transient state associated with the connection
    USP_LOG_Error("Error on STOMP connection to (host %s, port %d). Closing connection.", sc->host, sc->port);
    StopStompConnection(sc, DONT_PURGE_QUEUED_MESSAGES);

    // Start retrying this connection
    sc->state = kStompState_Retrying;
    sc->retry_count++;

    // Calculate time until next retry
    wait_time = CalculateStompRetryWaitTime(sc->retry_count, sc->retry.initial_interval, sc->retry.interval_multiplier);

    // Limit the retry time to the maximum
    if (wait_time > sc->retry.max_interval)
    {
        wait_time = sc->retry.max_interval;
    }

    USP_LOG_Info("Retrying STOMP connection to (host %s, port %d) in %d seconds (retry_count=%d).", sc->host, sc->port, wait_time, sc->retry_count);
    sc->retry_time = time(NULL) + wait_time;
}

/*********************************************************************//**
**
**  CalculateStompRetryWaitTime
**
**  Determines the number of seconds until the specified retry should occur
**  NOTE: This algorithm is different than the standard TR069 algorithm - it is a copy of the XMPP retry algorithm
**
** \param   retry_count - Number specifying the retry attempt that we want to calculate the delta time to. Counts from 1.
** \param   interval - The retry interval
** \param   multiplier - The interval multiplier
**
** \return  Number of seconds until the next retry
**
**************************************************************************/
unsigned CalculateStompRetryWaitTime(unsigned retry_count, double interval, double multiplier)
{
    unsigned range;

    // This function should not be called with a retry_count of 0
    // However, if it is, just treat it the same as a retry count of 1
    if (retry_count <= 0)
    {
        retry_count = 1;
    }

    // Limit retry count to avoid overflows in range calculation
    if (retry_count > 10)
    {
        retry_count = 10;
    }

    range = interval * pow(multiplier/1000, retry_count-1);

    return rand_r(&mtp_thread_random_seed) % range;
}

/*********************************************************************//**
**
** StartSendingFrame_STOMP
**
** Creates the STOMP message frame, and sets up state to transmit it
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartSendingFrame_STOMP(stomp_connection_t *sc)
{
    unsigned char *buf;
    int len;
    char heartbeat_args[64];
    char password_args[256];
    char debug_pw_args[256];
    char escaped_endpoint_id[256];
    char escaped_username[256];
    char escaped_password[256];
    char *endpoint_id;

    // Write the heartbeat header arguments into a buffer (if enabled)
    heartbeat_args[0] = '\0';
    if (sc->enable_heartbeats)
    {
        USP_SNPRINTF(heartbeat_args, sizeof(heartbeat_args), "heart-beat:%d,%d\n", sc->outgoing_heartbeat_period, sc->incoming_heartbeat_period);
    }

    // Get the endpoint_id, and escape any special characters in it
    endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    EscapeStompHeader(endpoint_id, escaped_endpoint_id, sizeof(escaped_endpoint_id));

    // Write the password arguments into a buffer (if they exist)
    password_args[0] = '\0';
    if ((sc->username != NULL) && (sc->username[0] != '\0') && (sc->password != NULL) && (sc->password[0] != '\0'))
    {
        EscapeStompHeader(sc->username, escaped_username, sizeof(escaped_username));
        EscapeStompHeader(sc->password, escaped_password, sizeof(escaped_password));
        USP_SNPRINTF(password_args, sizeof(password_args), "login:%s\npasscode:%s\n", escaped_username, escaped_password);
        USP_SNPRINTF(debug_pw_args, sizeof(debug_pw_args), "login:%s\npasscode:\n", escaped_username);
    }
    else
    {
        // Print a warning if no STOMP password is set, and a client certificate cannot alternatively be used for authentication
        LogNoPasswordWarning(sc);
    }

    #define STOMP_FRAME_FORMAT  "STOMP\n" \
                                "accept-version:1.2\n" \
                                "host:%s\n"  \
                                "%s"  \
                                "endpoint-id:%s\n"  \
                                "%s"        \
                                "\n"        \
                                EMPTY_BODY

    // Allocate a buffer to store the frame in
    // NOTE: The code assumes that none of the strings (host, login, passcode) contain embedded NULLs or CR/LF
    len = sizeof(STOMP_FRAME_FORMAT) + strlen(sc->virtual_host) + strlen(heartbeat_args)
                                     + strlen(escaped_endpoint_id) + strlen(password_args) - 8; // Minus 8 to remove all "%s" from the frame
    buf = USP_MALLOC(len);

    // Print the STOMP frame for debug (does not contain password)
    USP_LOG_Info("Sending STOMP frame to (host=%s, port=%d)", sc->host, sc->port);
    if (enable_protocol_trace)
    {
        USP_SNPRINTF(((char *)buf), len, STOMP_FRAME_FORMAT, sc->virtual_host, heartbeat_args, escaped_endpoint_id, debug_pw_args);
        USP_PROTOCOL("%s", buf);
    }

    // Form the real STOMP frame
    USP_SNPRINTF(((char *)buf), len, STOMP_FRAME_FORMAT, sc->virtual_host, heartbeat_args, escaped_endpoint_id, password_args);

    // Save the frame to transmit
    USP_ASSERT(sc->txframe == NULL);
    sc->txframe = buf;
    sc->txframe_len = len;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = false;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StartSendingFrame_SUBSCRIBE
**
** Creates the SUBSCRIBE message frame, and sets up state to transmit it
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartSendingFrame_SUBSCRIBE(stomp_connection_t *sc)
{
    char *agent_queue_name;
    unsigned char *buf;
    int len;

    USP_LOG_Info("Sending SUBSCRIBE frame to (host=%s, port=%d)", sc->host, sc->port);

    // NOTE: We do not open multiple subscriptions with the server, hence the "id:" header can be hardcoded
    // NOTE: We do not support sending ACK frames, hence the "ack:" header is set to "auto"
    #define SUBSCRIBE_FRAME_FORMAT  "SUBSCRIBE\n" \
                                    "id:0\n" \
                                    "destination:%s\n"  \
                                    "ack:auto\n"  \
                                    "\n"        \
                                    EMPTY_BODY

    // Determine the name of the queue to subscribe to
    if (sc->subscribe_dest != NULL)
    {
        agent_queue_name = sc->subscribe_dest;
    }
    else
    {
        agent_queue_name = sc->provisionned_queue;
    }

    // Exit if unable to get the name of the queue to subscribe to (ie this agent's queue)
    if ((agent_queue_name == NULL) || (*agent_queue_name == '\0'))
    {
        USP_LOG_Error("%s: Unable to get agent queue name for Device.STOMP.Connection.%d. Retrying", __FUNCTION__, sc->instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Allocate buffer to store the frame in
    // 2DO RH: The code assumes that none of the strings (destination, ack) contain embedded NULLs or CR/LF
    len = sizeof(SUBSCRIBE_FRAME_FORMAT) + strlen(agent_queue_name) - 2; // Minus 2 to remove all "%s" from the frame
    buf = USP_MALLOC(len);

    // Form the SUBSCRIBE frame
    USP_SNPRINTF(((char *)buf), len, SUBSCRIBE_FRAME_FORMAT, agent_queue_name);
    USP_PROTOCOL("%s", buf);

    // Save the frame to transmit
    USP_ASSERT(sc->txframe == NULL);
    sc->txframe = buf;
    sc->txframe_len = len;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = false;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StartSendingFrame_SEND
**
** Creates the SEND message frame, and sets up state to transmit it
**
** \param   sc - pointer to STOMP connection
** \param   controller_queue - name of STOMP queue to send this message to
** \param   agent_queue - name of agent's STOMP queue configured for this connection in the data model
** \param   msi - Information about the content to send. The ownership of
**                          the payload buffer is not passed to this function and stays with the caller.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartSendingFrame_SEND(stomp_connection_t *sc, char *controller_queue, char *agent_queue, mtp_send_item_t *msi)
{
    unsigned char *buf;
    int len;                    // Total number of bytes in the entire STOMP frame including NULL terminator
    int body_offset;            // Offset from the start of the STOMP message (in bytes) to the message's body (which will contain the google protocol buf encoded USP message)
    char content_length[16];    // Temporary string containing the content length digits
    char *content_type_str;
    USP_ASSERT(msi != NULL);

    // Exit if unable to get the name of the controller's queue on this connection
    if ((controller_queue == NULL) || (*controller_queue == '\0'))
    {
        USP_LOG_Error("%s: Unable to get controller queue name for Device.STOMP.Connection.%d. Retrying", __FUNCTION__, sc->instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Determine the name of this agent's STOMP queue
    if (sc->subscribe_dest != NULL)
    {
        // Override the queue configured in the data model with the queue given in the subscribe-dest STOMP header
        agent_queue = sc->subscribe_dest;
    }

    // Exit if unable to get the name of this agent's queue
    if ((agent_queue == NULL) || (*agent_queue == '\0'))
    {
        USP_LOG_Error("%s: Unable to get agent queue name for Device.STOMP.Connection.%d. Retrying", __FUNCTION__, sc->instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    content_type_str = BBF_STOMP_CONTENT_TYPE;

    // Determine the size of the USP message
    USP_SNPRINTF(content_length, sizeof(content_length), "%d", msi->pbuf_len);
    #define SEND_FRAME_FORMAT   "SEND\n" \
                                "content-length:%s\n" \
                                "content-type:%s\n"   \
                                "reply-to-dest:%s\n"  \
                                "destination:%s"

    // Allocate buffer to store the frame in
    #define STOMP_BODY_SEPARATOR "\n\n"
    len = sizeof(SEND_FRAME_FORMAT) +
          strlen(content_length) +
          strlen(content_type_str) +
          strlen(agent_queue) +
          strlen(controller_queue) - 8 + // Minus 8 to remove all "%s" from the frame
          sizeof(STOMP_BODY_SEPARATOR)-1 + // Minus 1 to not include NULL terminator in STOMP_BODY_SEPARATOR
          msi->pbuf_len;
    buf = USP_MALLOC(len);

    // Form the STOMP headers
    body_offset = USP_SNPRINTF((char *)buf, len, SEND_FRAME_FORMAT, content_length, content_type_str, agent_queue, controller_queue);

    MSG_HANDLER_LogMessageToSend(msi, kMtpProtocol_STOMP, sc->host, buf);

    // Form the STOMP body
    memcpy(&buf[body_offset], STOMP_BODY_SEPARATOR, sizeof(STOMP_BODY_SEPARATOR)-1);
    body_offset += 2;
    memcpy(&buf[body_offset], msi->pbuf, msi->pbuf_len);

    // Terminate the STOMP message
    buf[len-1] = '\0';

    // Save the frame to transmit
    USP_ASSERT(sc->txframe == NULL);
    sc->txframe = buf;
    sc->txframe_len = len;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = true;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StartSendingFrame_UNSUBSCRIBE
**
** Creates an UNSUBSCRIBE frame, and sets up state to transmit it
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartSendingFrame_UNSUBSCRIBE(stomp_connection_t *sc)
{
    unsigned char *buf;
    int len;

    USP_LOG_Info("Sending UNSUBSCRIBE frame to (host=%s, port=%d)", sc->host, sc->port);

    // NOTE: We do not open multiple subscriptions with the server, hence the "id:" header can be hardcoded
    #define UNSUBSCRIBE_FRAME_FORMAT  "UNSUBSCRIBE\n" \
                                      "id:0\n" \
                                      "\n"        \
                                      EMPTY_BODY

    // Allocate buffer to store the frame in
    len = sizeof(UNSUBSCRIBE_FRAME_FORMAT);
    buf = USP_MALLOC(len);

    // Form the UNSUBSCRIBE frame
    USP_SNPRINTF(((char *)buf), len, UNSUBSCRIBE_FRAME_FORMAT);
    USP_PROTOCOL("%s", buf);

    // Save the frame to transmit
    USP_ASSERT(sc->txframe == NULL);
    sc->txframe = buf;
    sc->txframe_len = len;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = false;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StartSendingFrame_DISCONNECT
**
** Creates a DISCONNECT frame, and sets up state to transmit it
**
** \param   sc - pointer to STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartSendingFrame_DISCONNECT(stomp_connection_t *sc)
{
    unsigned char *buf;
    int len;

    USP_LOG_Info("Sending DISCONNECT frame to (host=%s, port=%d)", sc->host, sc->port);

    // NOTE: We do not open multiple subscriptions with the server, hence the "id:" header can be hardcoded
    #define DISCONNECT_FRAME_FORMAT  "DISCONNECT\n" \
                                      "\n"        \
                                      EMPTY_BODY

    // Allocate buffer to store the frame in
    len = sizeof(DISCONNECT_FRAME_FORMAT);
    buf = USP_MALLOC(len);

    // Form the DISCONNECT frame
    USP_SNPRINTF(((char *)buf), len, DISCONNECT_FRAME_FORMAT);
    USP_PROTOCOL("%s", buf);

    // Save the frame to transmit
    USP_ASSERT(sc->txframe == NULL);
    sc->txframe = buf;
    sc->txframe_len = len;
    sc->txframe_sent_count = 0;
    sc->txframe_contains_usp_record = false;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** LogNoPasswordWarning
**
** Called in the case of no STOMP password setyup to log a warning, if no client cert is setup for authentication instead
**
** \param   sc - pointer to STOMP connection
**
** \return  None
**
**************************************************************************/
void LogNoPasswordWarning(stomp_connection_t *sc)
{
    bool available;
    bool matches_endpoint;

    DEVICE_SECURITY_GetClientCertStatus(&available, &matches_endpoint);

    // Exit if we are using a client cert to authenticate instead
    if ((sc->enable_encryption==true) && (available == true) && (matches_endpoint==true))
    {
        USP_LOG_Info("Using Client Certificate with SubjectAltName==EndpointID to authenticate (No username/password set)");
        return;
    }

    USP_LOG_Warning("%s: WARNING: No username/password set for connection to (host=%s, port=%d)", __FUNCTION__, sc->host, sc->port);

    // Exit if no encryption is enabled
    if (sc->enable_encryption==false)
    {
        USP_LOG_Warning("%s: WARNING: No TLS set for connection to (host=%s, port=%d)", __FUNCTION__, sc->host, sc->port);
        return;
    }

    // Exit if no client certificate being used
    if (available == false)
    {
        USP_LOG_Warning("%s: WARNING: No client certificate set for connection to (host=%s, port=%d)", __FUNCTION__, sc->host, sc->port);
        return;
    }

    // Exit if client certificate does not match the endpoint ID
    if (matches_endpoint == false)
    {
        USP_LOG_Warning("%s: WARNING: Client certificate does not contain Device's EndpointID in SubjectAltName", __FUNCTION__);
        return;
    }

    // NOTE: The code should never get here, as we've already tested for all cases.
}

/*********************************************************************//**
**
** EscapeStompHeader
**
** Escapes any special characters which are present in a STOMP header value string
** See https://stomp.github.io/stomp-specification-1.2.html#Value_Encoding
** NOTE: This function assumes that the source string does not contain carriage returns or line feeds
**
** \param   src - pointer to string to convert
** \param   dest - pointer to buffer in which to store escaped string
** \param   dest_len - size of buffer in which to store escaped string
**
** \return  None
**
**************************************************************************/
void EscapeStompHeader(char *src, char *dest, int dest_len)
{
    char c;

    c = *src++;
    while ((c != '\0') && (dest_len > 2))   // dest_len > 2 to allow for NULL terminator at end of dest buffer (2 rather than 1 because the last character might need escaping)
    {
        // Escape colons and back slashes
        if (c == ':')
        {
            *dest++ = '\\';
            *dest++ = 'c';
            dest_len -= 2;
        }
        else if (c == '\\')
        {
            *dest++ = '\\';
            *dest++ = '\\';
            dest_len -= 2;
        }
        else if (c == '\r')
        {
            *dest++ = '\\';
            *dest++ = 'r';
            dest_len -= 2;
        }
        else if (c == '\n')
        {
            *dest++ = '\\';
            *dest++ = 'n';
            dest_len -= 2;
        }
        else
        {
            // Other characters copy over unescaped
            *dest++ = c;
            dest_len--;
        }

        c = *src++;
    }
    *dest = '\0';
}

/*********************************************************************//**
**
** AddrInfoToStr
**
** Returns the specified address in an ASCII form
**
** \param   add - pointer to structure containing address to convert
** \param   buf - pointer to buffer to store the address in (must be INET6_ADDRSTRLEN bytes long)
** \param   buf - size of buffer (must be at least INET6_ADDRSTRLEN bytes long)
**
** \return  USP_ERR_OK if successful
**          USP_ERR_RESOURCES_EXCEEDED if unable to allocate a buffer to store the frame in
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
char *AddrInfoToStr(struct addrinfo *addr, char *buf, int len)
{
    struct sockaddr_in *in4;
    struct sockaddr_in6 *in6;
    void *sin_addr = NULL;
    char *server_str;

    USP_ASSERT(len >= INET6_ADDRSTRLEN);

    switch (addr->ai_family)
    {
        case AF_INET:
            in4 = (struct sockaddr_in *) addr->ai_addr;
            sin_addr = &(in4->sin_addr);
            break;

        case AF_INET6:
            in6 = (struct sockaddr_in6 *) addr->ai_addr;
            sin_addr = &(in6->sin6_addr);
            break;

        default:
            TERMINATE_BAD_CASE(addr->ai_family);
            break;
    }

    server_str = (char *) inet_ntop(addr->ai_family, sin_addr, buf, len);
    if (server_str == NULL)
    {
        USP_ERR_ERRNO("inet_ntop", errno);
    }

    return server_str;
}

/*********************************************************************//**
**
** UpdateNextHeartbeatTime
**
** Called whenever any data has been successfully sent to the STOMP server,
** to update the time at which we next need to send out a heartbeat to the STOMP server
** Agent Heartbeats will only be sent if the agent does not send anything else in the meantime
**
** \param   sc - pointer to STOMP connection
**
** \return  None
**
**************************************************************************/
void UpdateNextHeartbeatTime(stomp_connection_t *sc)
{
    time_t cur_time;

    cur_time = time(NULL);

    // Update the next time to perform a heartbeat
    if (sc->agent_heartbeat_period != 0)
    {
        // Outgoing heartbeats enabled
        sc->next_heartbeat_time = cur_time + sc->agent_heartbeat_period;
    }
    else
    {
        // Outgoing heartbeats disabled
        sc->next_heartbeat_time = INVALID_TIME;
    }
}

/*********************************************************************//**
**
** UpdateMgmtInterface
**
** Called to determine whether the IP address used for any of the STOMP connections has changed
** NOTE: This function only checks the IP address periodically
**
** \param   None
**
** \return  Number of seconds remaining until next time to poll the WAN interface for IP address change
**
**************************************************************************/
int UpdateMgmtInterface(void)
{
    time_t cur_time;
    int timeout;
    static bool is_first_time = true; // The first time this function is called, it just sets up the IP address and next_mgmt_if_poll_time

    // Exit if it's not yet time to poll the IP address
    cur_time = time(NULL);
    if (is_first_time == false)
    {
        timeout = next_mgmt_if_poll_time - cur_time;
        if (timeout > 0)
        {
            goto exit;
        }
    }

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
    UpdateWANInterface(is_first_time);
#else
    HandleStompSourceIPAddrChanges();
#endif

    // Set next time to poll for IP address change
    #define MGMT_IP_ADDR_POLL_PERIOD 5
    timeout = MGMT_IP_ADDR_POLL_PERIOD;
    next_mgmt_if_poll_time = cur_time + timeout;
    is_first_time = false;

exit:
    return timeout;
}


#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
/*********************************************************************//**
**
** UpdateWANInterface
**
** Called to determine whether the IP address of the WAN interface has changed
**
** \param   is_first_time - Set if it is the first time this function is called.
**                          The first time the function is called, it just updates the state of the system, it doesn't log that the IP address has changed
**
** \return  None
**
**************************************************************************/
void UpdateWANInterface(bool is_first_time)
{
    int i;
    stomp_connection_t *sc;
    char cur_mgmt_ip_addr[NU_IPADDRSTRLEN];

    // Get the current IP address
    tw_ulib_dev_get_live_wan_address(cur_mgmt_ip_addr, sizeof(cur_mgmt_ip_addr));

    // If this is the first time, then just update the state of the system with the IP address found, then exit
    if (is_first_time)
    {
        USP_STRNCPY(last_mgmt_ip_addr, cur_mgmt_ip_addr, sizeof(last_mgmt_ip_addr));
        return;
    }

    // Exit if the IP address has not changed, subsequently to the first time
    if (strcmp(last_mgmt_ip_addr, cur_mgmt_ip_addr) == 0)
    {
        return;
    }

    // Store off the new IP address, this is needed for StartStompConnection()
    USP_STRNCPY(last_mgmt_ip_addr, cur_mgmt_ip_addr, sizeof(last_mgmt_ip_addr));


    // Iterate over all STOMP connections, stopping and restarting the ones that are enabled
    USP_LOG_Warning("Mgmt IP Address changed to %s. Restarting all STOMP connections.", cur_mgmt_ip_addr);
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if (sc->instance != INVALID)
        {
            StopStompConnection(sc, DONT_PURGE_QUEUED_MESSAGES);
            StartStompConnection(sc);
        }
    }
}
#endif // CONNECT_ONLY_OVER_WAN_INTERFACE

#ifndef CONNECT_ONLY_OVER_WAN_INTERFACE
/*********************************************************************//**
**
** HandleStompSourceIPAddrChanges
**
** Restarts all STOMP connections whose IP address has changed
**
** \param   None
**
** \return  None
**
**************************************************************************/
void HandleStompSourceIPAddrChanges(void)
{
    int i;
    stomp_connection_t *sc;
    bool has_changed;
    bool has_addr = false;

    // Iterate over all STOMP connections, restarting any whose IP address has changed
    // NOTE: If the STOMP connection failed, then it will be retried by the retry mechanism.
    //       This code does NOT detect interfaces going up and then retrying the connection
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sc = &stomp_connections[i];
        if ((sc->instance != INVALID) && (sc->mgmt_if_name[0] != '\0') && (sc->mgmt_ip_addr[0] != '\0'))
        {
            has_changed = nu_ipaddr_has_interface_addr_changed(sc->mgmt_if_name, sc->mgmt_ip_addr, &has_addr);
            if (has_changed)
            {
                // Stop, then restart the STOMP connection
                USP_LOG_Warning("Mgmt IP Address for interface=%s changed. Restarting STOMP connection %d.", sc->mgmt_if_name, sc->instance);
                StopStompConnection(sc, DONT_PURGE_QUEUED_MESSAGES);
                StartStompConnection(sc);
            }
        }
    }
}
#endif

/*********************************************************************//**
**
** CopyStompConnParamsToNext
**
** Copies the supplied data model connection parameters into the slot's copy of the next parameters
**
** \param   sc - pointer to stomp connection into which we want to copy the parameters (destination)
** \param   sp - pointer to data model parameters to copy (source)
** \param   stomp_queue - destination queue to use for this device (ie the agent's queue)
**                        NOTE: stomp_queue is allowed to be NULL, as this is valid for the case of the broker provisioning the queue in the CONNECTED frame
**
** \return  None
**
**************************************************************************/
void CopyStompConnParamsToNext(stomp_connection_t *sc, stomp_conn_params_t *sp, char *stomp_queue)
{
    stomp_conn_params_t *np;

    // Copy across connection parameters into the next parameters to use when the connection is started
    np = &sc->next_conn_params;
    np->instance = sp->instance;
    np->port = sp->port;
    np->enable_encryption = sp->enable_encryption;
    np->enable_heartbeats = sp->enable_heartbeats;
    np->incoming_heartbeat_period = sp->incoming_heartbeat_period;
    np->outgoing_heartbeat_period = sp->outgoing_heartbeat_period;

    np->host = AllocateStringIfChanged(np->host, sp->host);
    np->username = AllocateStringIfChanged(np->username, sp->username);
    np->password = AllocateStringIfChanged(np->password, sp->password);
    np->virtual_host = AllocateStringIfChanged(np->virtual_host, sp->virtual_host);
    sc->next_provisionned_queue = AllocateStringIfChanged(sc->next_provisionned_queue, stomp_queue);

    np->retry.initial_interval = sp->retry.initial_interval;
    np->retry.interval_multiplier = sp->retry.interval_multiplier;
    np->retry.max_interval = sp->retry.max_interval;
}

/*********************************************************************//**
**
** CopyStompConnParamsFromNext
**
** Copies the next stomp connection parameters into the set of parameters used when starting the connection
**
** \param   sc - pointer to stomp connection to update the stomp connection parameters of
**
** \return  None
**
**************************************************************************/
void CopyStompConnParamsFromNext(stomp_connection_t *sc)
{
    stomp_conn_params_t *np;

    // Copy across the next connection parameters into the parameters to use when the connection is started
    np = &sc->next_conn_params;
    sc->instance = np->instance;
    sc->port = np->port;
    sc->enable_encryption = np->enable_encryption;
    sc->enable_heartbeats = np->enable_heartbeats;
    sc->incoming_heartbeat_period = np->incoming_heartbeat_period;
    sc->outgoing_heartbeat_period = np->outgoing_heartbeat_period;

    sc->host = AllocateStringIfChanged(sc->host, np->host);
    sc->username = AllocateStringIfChanged(sc->username, np->username);
    sc->password = AllocateStringIfChanged(sc->password, np->password);
    sc->virtual_host = AllocateStringIfChanged(sc->virtual_host, np->virtual_host);
    sc->provisionned_queue = AllocateStringIfChanged(sc->provisionned_queue, sc->next_provisionned_queue);

    sc->retry.initial_interval = np->retry.initial_interval;
    sc->retry.interval_multiplier = np->retry.interval_multiplier;
    sc->retry.max_interval = np->retry.max_interval;

}

/*********************************************************************//**
**
** AllocateStringIfChanged
**
** Allocates a copy of the new string, if it has changed from the current string
**
** \param   cur_str - pointer to current string
** \param   new_str - pointer to new string
**
** \return  Pointer to copy of new string, or existing string if no change
**
**************************************************************************/
char *AllocateStringIfChanged(char *cur_str, char *new_str)
{
    // Exit if the string has not changed - nothing to allocate/deallocate
    if ((cur_str != NULL) && (new_str != NULL))
    {
        if (strcmp(cur_str, new_str) == 0)
        {
            return cur_str;
        }
    }

    // Free the current string
    USP_SAFE_FREE(cur_str);

    // Exit if new string does not need allocating
    if (new_str == NULL)
    {
        return NULL;
    }

    // Allocate a copy of the new string
    return USP_STRDUP(new_str);
}

/*********************************************************************//**
**
** FindStompConnByInst
**
** Finds a STOMP connection by it's data model instance number
** NOTE: It isssible for this function to return NULL under normal circumstances if the connection is disabled
**
** \param   instance - instance number of the STOMP connection in the data model
**
** \return  pointer to slot, or NULL if slot was not found
**
**************************************************************************/
stomp_connection_t *FindStompConnByInst(int instance)
{
    int i;
    stomp_connection_t *sc;

    // Iterate over all STOMP connections
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        // Exit if found a stomp connection that matches the instance number
        sc = &stomp_connections[i];
        if (sc->instance == instance)
        {
            return sc;
        }
    }

    // If the code gets here, then no matching slot was found
    return NULL;
}

/*********************************************************************//**
**
** FindUnusedStompConn
**
** Finds the first free stomp connection slot
**
** \param   None
**
** \return  Pointer to first free slot, or NULL if no slot was found
**
**************************************************************************/
stomp_connection_t *FindUnusedStompConn(void)
{
    int i;
    stomp_connection_t *sc;

    // Iterate over all STOMP connections
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        // Exit if found an unused slot
        sc = &stomp_connections[i];
        if (sc->instance == INVALID)
        {
            return sc;
        }
    }

    // If the code gets here, then no free slot has been found
    USP_LOG_Error("%s: Only %d STOMP connections are supported.", __FUNCTION__, MAX_STOMP_CONNECTIONS);
    return NULL;
}

/*********************************************************************//**
**
** RemoveExpiredStompMessages
**
** Removes all expired messages from the queue of messages to send
** NOTE: This mechanism can be used to prevent the queue from filling up needlessly if the controller is offine
**
** \param   sc - pointer to STOMP connection
**
** \return  None
**
**************************************************************************/
void RemoveExpiredStompMessages(stomp_connection_t *sc)
{
    time_t cur_time;
    stomp_send_item_t *queued_msg;
    stomp_send_item_t *next_msg;

    USP_ASSERT(sc->txframe == NULL);    // This function must not remove the current frame being transmitted whilst it is being transmitted

    cur_time = time(NULL);
    queued_msg = (stomp_send_item_t *) sc->usp_record_send_queue.head;
    while (queued_msg != NULL)
    {
        next_msg = (stomp_send_item_t *) queued_msg->link.next;
        if (cur_time > queued_msg->expiry_time)
        {
            RemoveStompQueueItem(sc, queued_msg);
        }

        queued_msg = next_msg;
    }
}

/*********************************************************************//**
**
** RemoveStompQueueItem
**
** Frees the specified item in the send queue of the specified controller
**
** \param   sc - pointer to STOMP connection
** \param   queued_msg - message to remove from the queue
**
** \return  None
**
**************************************************************************/
void RemoveStompQueueItem(stomp_connection_t *sc, stomp_send_item_t *queued_msg)
{
    USP_ASSERT(queued_msg != NULL);

    // Free all dynamically allocated member variables
    USP_FREE(queued_msg->controller_queue);
    USP_FREE(queued_msg->agent_queue);
    USP_FREE(queued_msg->item.pbuf);

    // Remove the specified item from the queue, and free the item itself
    DLLIST_Unlink(&sc->usp_record_send_queue, queued_msg);
    USP_FREE(queued_msg);
}

/*********************************************************************//**
**
** IsUspRecordInStompQueue
**
** Determines whether the specified USP record is already queued, waiting to be sent
** This is used to avoid duplicate records being placed in the queue, which could occur under notification retry conditions
**
** \param   sc - stomp connection which has USP records queued to send
** \param   pbuf - pointer to buffer containing USP Record to match against
** \param   pbuf_len - length of buffer containing USP Record to match against
**
** \return  true if the message is already queued
**
**************************************************************************/
bool IsUspRecordInStompQueue(stomp_connection_t *sc, unsigned char *pbuf, int pbuf_len)
{
    stomp_send_item_t *queued_msg;

    // Iterate over USP Records in the STOMP queue
    queued_msg = (stomp_send_item_t *) sc->usp_record_send_queue.head;
    while (queued_msg != NULL)
    {
        // Exit if the USP record is already in the queue
        if ((queued_msg->item.pbuf_len == pbuf_len) && (memcmp(queued_msg->item.pbuf, pbuf, pbuf_len)==0))
        {
             return true;
        }

        // Move to next message in the queue
        queued_msg = (stomp_send_item_t *) queued_msg->link.next;
    }

    // If the code gets here, then the USP record is not in the queue
    return false;
}

/*********************************************************************//**
**
** QueueUspConnectRecord_STOMP
**
** Adds the USP connect record at the front of the queue, ensuring that there is only one connect record in the queue
**
** \param   sc - stomp connection which has USP records queued to send
** \param   msi - pointer to content to send
**                NOTE: Ownership of the payload buffer passes to this function
** \param   controller_queue - name of STOMP queue to send this message to
** \param   agent_queue - name of agent's STOMP queue configured for this connection in the data model
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  None
**
**************************************************************************/
void QueueUspConnectRecord_STOMP(stomp_connection_t *sc, mtp_send_item_t *msi, char *controller_queue, char *agent_queue, time_t expiry_time)
{
    stomp_send_item_t *cur_msg;
    stomp_send_item_t *next_msg;
    stomp_send_item_t *send_item;
    mtp_content_type_t type;

    // Iterate over USP Records in the queue, removing all stale connect and disconnect records
    // A connect or disconnect record may still be in the queue if the connection failed before the record was fully sent
    cur_msg = (stomp_send_item_t *) sc->usp_record_send_queue.head;
    while (cur_msg != NULL)
    {
        // Save pointer to next message, as we may remove the current message
        next_msg = (stomp_send_item_t *) cur_msg->link.next;

        // Remove current message if it is a connect or disconnect record
        type = cur_msg->item.content_type;
        if (IsUspConnectOrDisconnectRecord(type))
        {
            RemoveStompQueueItem(sc, cur_msg);
        }

        // Move to next message in the queue
        cur_msg = next_msg;
    }

    // Add the new connect record to the queue
    send_item = USP_MALLOC(sizeof(stomp_send_item_t));
    send_item->item = *msi;  // NOTE: Ownership of the payload buffer passes to the STOMP message queue
    send_item->controller_queue = USP_STRDUP(controller_queue);
    send_item->agent_queue = USP_STRDUP(agent_queue);
    send_item->expiry_time = expiry_time;

    DLLIST_LinkToHead(&sc->usp_record_send_queue, send_item);
}

#endif // DISABLE_STOMP
