/*
 *
 * Copyright (C) 2021-2022, Broadband Forum
 * Copyright (C) 2021-2022  CommScope, Inc
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
 * \file wsserver.c
 *
 * Implements a websocket server on the agent, which a USP Controller can connect to
 *
 */

#ifdef ENABLE_WEBSOCKETS
#include <libwebsockets.h>
#include <unistd.h>

#include "common_defs.h"
#include "dllist.h"
#include "wsserver.h"
#include "wsclient.h"
#include "msg_handler.h"
#include "os_utils.h"
#include "iso8601.h"
#include "dm_exec.h"
#include "retry_wait.h"
#include "nu_ipaddr.h"
#include "nu_macaddr.h"
#include "text_utils.h"

//------------------------------------------------------------------------------
// Server context structure
typedef struct
{
    struct lws_context  *ctx;                // libwebsockets context. Set to NULL if the server is not running
    wsserv_config_t     cur_config;          // Contains current configuration params
    wsserv_config_t     new_config;          // Contains configuration params to use when restarting the server. NOTE: These params might be the same as cur_config, if the configuration has not changed

    scheduled_action_t  schedule_restart;    // Sets whether a WebSocket server restart (with new config) is scheduled after the send queue has cleared
                                             // This flag is also used to indicate that a start (with new config) is still pending (e.g. if a WebSocket server start had failed previously, this flag would still be set)
    scheduled_action_t  schedule_stop;       // Sets whether the server should be disabled after the send queue has cleared
    char wan_addr[NU_IPADDRSTRLEN];          // IP address on which the websocket server is listening. Set to "any" if listening on all device IP addresses

    lws_sorted_usec_list_t wan_poll_timer;   // structure used by libwebsockets for scheduled timer, used for polling the WAN interface for IP address changes
} wsserv_t;

static wsserv_t wsserv;

//------------------------------------------------------------------------------
// Payload to send in WebSocket queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    mtp_send_item_t item;   // Information about the content to send
    time_t expiry_time;     // Time at which this USP record should be removed from the queue
} wsserv_send_item_t;

//------------------------------------------------------------------------------
// Structure representing a connection from the agent's websocket server to a controller
typedef struct
{
    struct lws *ws_handle;  // pointer to structure allocated by libwebsockets for this connection. NULL marks the entry as unused
    int conn_id;            // unique identifier for the connection

    // State variables
    double_linked_list_t usp_record_send_queue;  // Queue of USP Records to send to this controller
    bool disconnect_sent;           // Set if a USP Disconnect record is being sent. Causes a close after the record has been transmitted.

    unsigned char *rx_buf;  // Buffer used to build up the USP record as chunks are received
    int rx_buf_len;         // Current size of received USP Record in rx_buf
    int rx_buf_max_len;     // Current allocated size of rx_buf. This will hold the current websocket fragment. If the USP Record is contained in multiple websocket fragments, then the buffer is reallocated to include the new fragment
    int tx_index;           // Counts the number of bytes sent of the current USP Record to tx (i.e. at the head of the usp_record_send_queue)

    ctrust_role_t role;     // role granted by the CA cert in the chain of trust with the websockets client

    bool send_ping;         // Set if the next LWS_CALLBACK_SERVER_WRITEABLE event should send a ping frame (rather than servicing the USP record send queue)
    int ping_count;         // Number of websocket ping frames sent without corresponding pong responses

    char peer[64];          // Name of peer, either endpoint_id (if available in Sec-WebSocket-Extensions) or an IP address.
    bool is_peer_an_eid;    // Set if the above peer string is an endpoint_id. False if it is an IP address.

} wsconn_t;

//------------------------------------------------------------------------------
// Array of active websocket server connections
static wsconn_t ws_connections[MAX_WEBSOCKET_CLIENTS];

//------------------------------------------------------------------------------------
// Count of number of connections. This is used to ensure that each connection is uniquely
// identified so that responses are sent back on the connection the request was received on
static int conn_id_counter = 0;

//------------------------------------------------------------------------------------
// Mutex used to protect access to this component
static pthread_mutex_t wss_access_mutex;

//------------------------------------------------------------------------------------
// Structure passed to libwebsockets defining the websocket USP sub-protocol
static struct lws_protocols wss_subprotocol[2];      // last entry is NULL terminator

//------------------------------------------------------------------------------
// Flag set to true if the websocket server MTP has been shutdown
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_wsserv_mtp_shutdown = false;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int HandleAllWssEvents(struct lws *handle, enum lws_callback_reasons event, void *per_session_data, void *event_args, size_t event_args_len);
int HandleWssEvent_NewClient(struct lws *handle);
int HandleWssEvent_Established(struct lws *handle);
int HandleWssEvent_Receive(struct lws *handle, unsigned char *chunk, int chunk_len);
int HandleWssEvent_Transmit(struct lws *handle);
wsconn_t *FindFreeWsConnection(void);
wsconn_t *FindWsConnectionByConnId(int conn_id);
wsconn_t *FindWsConnectionByEndpointId(char *endpoint_id);
void RemoveWsservSendItem(wsconn_t *wc, wsserv_send_item_t *si);
bool AreAllWebsockServerMessagesSent(void);
int StartWebsockServer(wsserv_config_t *config);
void StopWebsockServer(void);
void FreeWebsockServerConnection(wsconn_t *wc);
void CopyWssConfig(wsserv_config_t *dest, wsserv_config_t *src);
void DestroyWssConfig(wsserv_config_t *config);
void LogWsserverDebug(int level, const char *line);
int AddWsservUspExtension(struct lws *handle, char **ppHeaders, int headers_len);
void RemoveExpiredWsservMessages(wsconn_t *wc);
bool IsUspRecordInWsservQueue(wsconn_t *wc, unsigned char *pbuf, int pbuf_len);
int HandleWssEvent_LoadCerts(SSL_CTX *ssl_ctx);
int HandleWssEvent_VerifyCerts(struct lws *handle, SSL *ssl, int preverify_ok, X509_STORE_CTX *x509_ctx);
int HandleWssEvent_Destroyed(struct lws *handle);
int ValidateReceivedWsservProtocolExtension(struct lws *handle);
int CountWsConnections(void);
bool WantingToStopWsServer(void);
int HandleWssEvent_PingTimer(struct lws *handle);
int HandleWssEvent_PongReceived(struct lws *handle);
int SendWsservPing(wsconn_t *wc);
int CloseWsservConnection(struct lws *handle, int close_status, char *fmt, ...);
void QueueUspConnectRecord_Wsserv(wsconn_t *wc);
bool ServiceActiveWebsockServer(void);
bool ServiceInactiveWebsockServer(void);
void HandleWssEvent_WanPollTimer(lws_sorted_usec_list_t *sul);
void AssertWanPollCallbackNotInUse(lws_sorted_usec_list_t *sul);
int CalcWsservIpAddr(char *buf, int len);

/*********************************************************************//**
**
** WSSERVER_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int WSSERVER_Init(void)
{
    int i;
    int err;
    wsconn_t *wc;

    // Initialise websocket connection array
    memset(ws_connections, 0, sizeof(ws_connections));
    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        wc->ws_handle = NULL;
    }

    // Initialise the server context
    memset(&wsserv, 0, sizeof(wsserv));
    wsserv.schedule_restart = kScheduledAction_Off;
    wsserv.schedule_stop = kScheduledAction_Off;

    // Set log mask and function to re-direct libwebsockets log output
    #define LIBWEBSOCKETS_LOG_MASK  0  // (LLL_ERR | LLL_WARN | LLL_NOTICE)
    lws_set_log_level(LIBWEBSOCKETS_LOG_MASK, LogWsserverDebug);

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&wss_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WSSERVER_Start
**
** Called before starting all WebSockets connections to create the libwebsockets context
** IMPORTANT: Libwebsockets insists on re-initialising libssl when lws_create_context() is called
**       Also lws_create_context() loads the trust store and client cert into libwebsockets SSL context
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int WSSERVER_Start(void)
{
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WSSERVER_EnableServer
**
** Starts (or restarts) the agent's websocket server
**
** \param   config - pointer to structure containing configuration parameters
**                   NOTE: Ownership of strings within the config parameters stay with the caller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int WSSERVER_EnableServer(wsserv_config_t *config)
{
    int i;
    wsconn_t *wc;
    int err = USP_ERR_OK;

    // Exit if websocket server MTP has shutdown
    if (is_wsserv_mtp_shutdown)
    {
        return USP_ERR_OK;
    }

    OS_UTILS_LockMutex(&wss_access_mutex);

    // If the only change is to the keep alive interval whilst the server is running, then
    // cause the websock ping frame to be sent immediately (so that the new interval is respected immediately)
    if ((wsserv.ctx != NULL) &&
        (wsserv.cur_config.port == config->port) &&
        (strcmp(wsserv.cur_config.path, config->path)==0) &&
        (wsserv.cur_config.enable_encryption == config->enable_encryption) &&
        (wsserv.cur_config.keep_alive != config->keep_alive))
    {
        // Save new configuration
        wsserv.cur_config.keep_alive = config->keep_alive;

        // Cause the websock ping frame to be sent immediately on all active connections
        for (i=0; i<NUM_ELEM(ws_connections); i++)
        {
            wc = &ws_connections[i];
            if (wc->ws_handle != NULL)
            {
                lws_set_timer_usecs(wc->ws_handle, 0);
            }
        }
        goto exit;
    }

    // Otherwise, save the new config and schedule a restart
    CopyWssConfig(&wsserv.new_config, config);
    wsserv.schedule_restart = kScheduledAction_Signalled;

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return err;
}

/*********************************************************************//**
**
** WSSERVER_DisableServer
**
** Stops the agent's websocket server
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int WSSERVER_DisableServer(void)
{
    // Exit if websocket server MTP has shutdown
    if (is_wsserv_mtp_shutdown)
    {
        return USP_ERR_OK;
    }

    OS_UTILS_LockMutex(&wss_access_mutex);

    if (wsserv.ctx != NULL)
    {
        // If currently running, then we need to wait for all messages to be sent before stopping
        wsserv.schedule_stop = kScheduledAction_Signalled;
    }

    // NOTE: Nothing to do if already stopped

    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WSSERVER_ActivateScheduledActions
**
** Activates all scheduled actions. If a reconnect or close is scheduled, all USP messages
** that are queued to send will be sent before performing the action
**
** \param   None
**
** \return  None
**
**************************************************************************/
void WSSERVER_ActivateScheduledActions(void)
{
    bool wake_thread = false;

    // Exit if websocket server MTP has shutdown
    if (is_wsserv_mtp_shutdown)
    {
        return;
    }

    OS_UTILS_LockMutex(&wss_access_mutex);

    if (mtp_exit_scheduled == kScheduledAction_Activated)
    {
        wake_thread = true;
    }

    if (wsserv.schedule_restart == kScheduledAction_Signalled)
    {
        wsserv.schedule_restart = kScheduledAction_Activated;
        wake_thread = true;
    }

    if (wsserv.schedule_stop == kScheduledAction_Signalled)
    {
        wsserv.schedule_stop = kScheduledAction_Activated;
        wake_thread = true;
    }

    if ((wake_thread) && (wsserv.ctx != NULL))
    {
        // Cause the libwebsocket thread to wake up and process the scheduled action
        lws_cancel_service(wsserv.ctx);
    }

    OS_UTILS_UnlockMutex(&wss_access_mutex);
}

/*********************************************************************//**
**
** WSSERVER_QueueBinaryMessage
**
** Function called to queue a USP record on the specified WebSocket connection
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is always passed to this function.
** \param   conn_id - handle used to lookup the connection to send the USP message to
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  None
**
**************************************************************************/
void WSSERVER_QueueBinaryMessage(mtp_send_item_t *msi, int conn_id, time_t expiry_time)
{
    wsconn_t *wc;
    wsserv_send_item_t *si;
    bool is_duplicate;
    USP_ASSERT(msi != NULL);

    // Exit if websocket server MTP has shutdown
    if (is_wsserv_mtp_shutdown)
    {
        USP_FREE(msi->pbuf);
        return;
    }

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Exit if the connection to send to does not exist anymore
    wc = FindWsConnectionByConnId(conn_id);
    if (wc == NULL)
    {
        USP_FREE(msi->pbuf);
        goto exit;
    }

    // Remove all USP notifications from the send queue, that have expired
    RemoveExpiredWsservMessages(wc);

    // Do not add this message to the queue, if it is already present in the queue
    // This situation could occur if a notify is being retried to be sent, but is already held up in the queue pending sending
    is_duplicate = IsUspRecordInWsservQueue(wc, msi->pbuf, msi->pbuf_len);
    if (is_duplicate)
    {
        USP_FREE(msi->pbuf);
        return;
    }

    // Add USP Record to queue
    si = USP_MALLOC(sizeof(wsserv_send_item_t));
    memset(si, 0, sizeof(wsserv_send_item_t));
    si->item = *msi;  // NOTE: Ownership of the payload buffer passes to the websock message queue
    si->expiry_time = expiry_time;
    DLLIST_LinkToTail(&wc->usp_record_send_queue, si);

    // Cause the libwebsocket thread to wake up and start sending the record
    USP_ASSERT(wsserv.ctx != NULL);
    lws_cancel_service(wsserv.ctx);

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);
}

/*********************************************************************//**
**
** WSSERVER_GetMtpStatus
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.Status for a Websocket server connection
**
** \param   None
**
** \return  Status of the server
**
**************************************************************************/
mtp_status_t WSSERVER_GetMtpStatus(void)
{
    mtp_status_t status;

    OS_UTILS_LockMutex(&wss_access_mutex);
    status = (wsserv.ctx != NULL) ? kMtpStatus_Up : kMtpStatus_Down;
    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return status;
}

/*********************************************************************//**
**
** WSSERVER_DisconnectEndpoint
**
** Called to gracefully disconnect the specified endpoint (if it is connected)
** If an endpoint is connected to both the websocket client and the websocket server, then
** this function is used to give preference to connections via the websocket client
**
** \param   endpoint_id - endpoint_id of the controller which we want to disconnect, if it is connected via the agent's websocket server
**
** \return  Status of the server
**
**************************************************************************/
void WSSERVER_DisconnectEndpoint(char *endpoint_id)
{
    wsconn_t *wc;
    mtp_send_item_t mtp_send_item;

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Exit if the specified endpoint is not connected to the agent's websocket server
    wc = FindWsConnectionByEndpointId(endpoint_id);
    if (wc == NULL)
    {
        goto exit;
    }

    // Send a disconnect record, which will result in the controller connected to the websocket server being disconnected
    USPREC_Disconnect_Create(kMtpContentType_DisconnectRecord, endpoint_id, USP_ERR_PERMISSION_DENIED,
                             "Agent's websocket client has connected, so closing this connection", &mtp_send_item);
    WSSERVER_QueueBinaryMessage(&mtp_send_item, wc->conn_id, END_OF_TIME);

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);
}

/*********************************************************************//**
**
** WSSERVER_GetMTPForEndpointId
**
** Determines if the specified endpoint is connected to the agent's websocket server
** and if so returns the parameters specifying the MTP to use
**
** \param   endpoint_id - Endpoint ID of controller that maybe connected to agent's webserver
** \param   mrt - structure to fill in with MTP details, if the specified controller is connected to the agent's websocket server
**                or NULL if the caller is just trying to determine whether the controller is connected to the agent's websocket server
**
** \return  USP_ERR_OK if specified endpoint is connected to the agent's websocket server
**
**************************************************************************/
int WSSERVER_GetMTPForEndpointId(char *endpoint_id, mtp_reply_to_t *mrt)
{
    int err;
    wsconn_t *wc;

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Exit if the specified endpoint is not connected to the agent's websocket server
    wc = FindWsConnectionByEndpointId(endpoint_id);
    if (wc == NULL)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Fill in the MTP to use, in order to send to this endpoint via the agent's websocket server
    if (mrt != NULL)
    {
        memset(mrt, 0, sizeof(mtp_reply_to_t));
        mrt->is_reply_to_specified = true;
        mrt->protocol = kMtpProtocol_WebSockets;
        mrt->wsclient_cont_instance = INVALID;
        mrt->wsclient_mtp_instance = INVALID;
        mrt->wsserv_conn_id = wc->conn_id;
    }
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);
    return err;
}

/*********************************************************************//**
**
** WSSERVER_Main
**
** Main loop of MTP thread for agent acting as WebSocket server
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *WSSERVER_Main(void *args)
{
    bool run_thread = true;

    while (run_thread)
    {
        if (wsserv.ctx == NULL)
        {
            // WebSocket server is not currently running, so see if it should be started
            run_thread = ServiceInactiveWebsockServer();
        }
        else
        {
            // WebSocket server is currently running
            run_thread = ServiceActiveWebsockServer();
        }
    }

    // Deal with the case of exiting this thread because we are shutting down (due to Reboot() or FactoryReset() USP commands)
    OS_UTILS_LockMutex(&wss_access_mutex);
    if (mtp_exit_scheduled == kScheduledAction_Activated)
    {
        // Free configurations
        USP_ASSERT(wsserv.ctx == NULL);     // Websocket server context should already have been freed by call to StopWebsock
        DestroyWssConfig(&wsserv.cur_config);
        DestroyWssConfig(&wsserv.new_config);

        // Prevent the data model from making any other changes to the MTP thread
        is_wsserv_mtp_shutdown = true;

        // Signal the data model thread that this MTP has shutdown
        DM_EXEC_PostMtpThreadExited(WSSERVER_EXITED);
    }
    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return NULL;
}

/*********************************************************************//**
**
** ServiceActiveWebsockServer
**
** Services a running WebSocket server
**
** \param   None
**
** \return  false if WebSocketServer thread should shut down (due to Reboot() or FactoryReset() USP commands)
**          true if WebSocketServer thread should continue running
**
**************************************************************************/
bool ServiceActiveWebsockServer(void)
{
    int err;
    int i;
    wsconn_t *wc;
    bool run_thread = true;
    bool wanting_to_stop;

    // Allow libwebsockets to wait for socket activity
    // NOTE: If there is any activity, HandleAllWssEvents() will be called to process it
    err = lws_service(wsserv.ctx, 0);
    if (err != 0)
    {
        // NOTE: The code should never get here, but if it does, handle it by logging the error and sleeping for a while (to prevent the thread hogging the processor)
        #define MILLISECONDS 1000  // number of micro seconds (us) in a millisecond
        USP_LOG_Warning("%s: lws_service() returned %d", __FUNCTION__, err);
        usleep(100*MILLISECONDS);
    }

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Deal with stopping or restarting the server
    wanting_to_stop = WantingToStopWsServer();
    if (wanting_to_stop)
    {
        // Wait for all responses to be sent before performing the scheduled action
        if (AreAllWebsockServerMessagesSent())
        {
            // Wait for all clients to have been gracefully disconnected (a close frame must have been sent to them, to cause them to disconnect)
            if (CountWsConnections() == 0)
            {
                // Stop the current websocket server
                StopWebsockServer();

                // Cause the WebSocket server thread to exit, if this is the scheduled action
                if ((wsserv.schedule_stop == kScheduledAction_Activated) || (mtp_exit_scheduled == kScheduledAction_Activated))
                {
                    run_thread = false;
                    wsserv.schedule_stop = kScheduledAction_Off;
                    goto exit;
                }

                // Restart the server with new config, if this is the scheduled action
                if (wsserv.schedule_restart == kScheduledAction_Activated)
                {
                    // Exit if unable to start the server with the new config
                    // Since the current server has been stopped, ServiceInactiveWsserver() will periodically retry starting the server
                    err = StartWebsockServer(&wsserv.new_config);
                    if (err != USP_ERR_OK)
                    {
                        goto exit;
                    }

                    // Server was started successfully, so clear the schedule_restart_flag
                    wsserv.schedule_restart = kScheduledAction_Off;
                    goto exit;
                }
            }
        }
    }

    // Iterate over all active connections, asking libwebsockets for permission to send
    // NOTE: If we are shutting down gracefully, then this code ensures that each client is sent a close frame
    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if (wc->ws_handle != NULL)
        {
            if ((wc->usp_record_send_queue.head != NULL) || (wanting_to_stop) || (wc->disconnect_sent))
            {
                err = lws_callback_on_writable(wc->ws_handle);
                if (err != 1)
                {
                    // If an error occurred, then close the TCP connection immediately
                    USP_LOG_Error("%s: lws_callback_on_writable() returned %d", __FUNCTION__, err);
                    lws_set_timeout(wc->ws_handle, PENDING_TIMEOUT_USER_OK, LWS_TO_KILL_ASYNC);
                }
            }
        }
    }

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return run_thread;
}

/*********************************************************************//**
**
** ServiceInactiveWebsockServer
**
** Services the condition of no WebSocket server currently running
** - Checks whether we are shutting down or whether to attempt to start the websocket server
**
** \param   None
**
** \return  false if WebSocketServer thread should shut down (due to Reboot() or FactoryReset() USP commands)
**          true if WebSocketServer thread should continue running
**
**************************************************************************/
bool ServiceInactiveWebsockServer(void)
{
    bool run_thread = true;
    int delay = 0;
    int err;

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Cause thread to exit, if we are shutting down (due to Reboot() or FactoryReset() USP commands)
    if ((wsserv.schedule_stop == kScheduledAction_Activated) || (mtp_exit_scheduled == kScheduledAction_Activated))
    {
        run_thread = false;
        goto exit;
    }

    // Exit if a restart of the server is not scheduled. In this case, we just wait a second before seeing again if we should start the server or shut down the thread
    if (wsserv.schedule_restart != kScheduledAction_Activated)
    {
        delay = 1;
        goto exit;
    }

    // Attempt to start the server
    err = StartWebsockServer(&wsserv.new_config);
    if (err != USP_ERR_OK)
    {
        #define WS_SERVER_RETRY_TIME 5
        USP_LOG_Warning("%s: Failed to restart WebSocket server. Retrying in %d seconds", __FUNCTION__, WS_SERVER_RETRY_TIME);
        delay = WS_SERVER_RETRY_TIME;
        goto exit;
    }

    // Since server has been started successfully, clear the schedule_restart flag
    wsserv.schedule_restart = kScheduledAction_Off;

exit:
    OS_UTILS_UnlockMutex(&wss_access_mutex);

    // Ensure that the websocket server thread does not keep CPU busy all the time
    // NOTE: The sleep is performed outside of the mutex, so that WSSERVER_XXX API functions do not block
    if (delay > 0)
    {
        sleep(delay);
    }

    return run_thread;
}

/*********************************************************************//**
**
** StartWebsockServer
**
** Starts the agent's websocket server
**
** \param   config - pointer to structure containing configuration parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartWebsockServer(wsserv_config_t *config)
{
    int err;
    struct lws_protocols *p;
    struct lws_context_creation_info info;

    // Setup USP subprotocol to use
    #define WEBSOCKET_PROTOCOL_STR "v1.usp"
    memset(wss_subprotocol, 0, sizeof(wss_subprotocol));
    p = &wss_subprotocol[0];
    p->name = WEBSOCKET_PROTOCOL_STR;
    p->callback = HandleAllWssEvents;
    p->per_session_data_size = 0;

    #define RX_CHUNK_SIZE MAX_USP_MSG_LEN  // Maximum size of chunks that we want to receive from libwebsockets
    #define TX_CHUNK_SIZE 2*1024           // Maximum size of chunks that we want to send to libwebsockets.
                                           //  NOTE: This needs to be small enough to ensure that large messages are fully sent when shutting down gracefully
    p->rx_buffer_size = RX_CHUNK_SIZE;
    p->tx_packet_size = TX_CHUNK_SIZE;

    // Setup websock context
    memset(&info, 0, sizeof info);
    info.port = config->port;
    info.protocols = wss_subprotocol;
    info.pvo = NULL;
    info.fd_limit_per_thread = 1 + 2*MAX_WEBSOCKET_CLIENTS;  // Multiply by 2 to include extra http2 fds. Plus 1 for libwebsockets internal fds
    info.simultaneous_ssl_restriction = MAX_WEBSOCKET_CLIENTS;

    // NOTE: The following option must be set for libwebsockets to support SSL.
    // It not only initialises libSSL, but it also enables SSL support in libwebsockets.
    if (config->enable_encryption)
    {
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info.options |= LWS_SERVER_OPTION_CREATE_VHOST_SSL_CTX;
        info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
    }

    // Exit if unable to determine IP address of interface to listen on
    err = CalcWsservIpAddr(wsserv.wan_addr, sizeof(wsserv.wan_addr));
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Create the websocket context
    // NOTE: libwebsockets calls HandleAllWssEvents() from within lws_create_context() to load the
    // trust store and client certs into libwebsockets SSL context
    info.iface = (strcmp(wsserv.wan_addr, "any")==0) ? NULL : wsserv.wan_addr;
    wsserv.ctx = lws_create_context(&info);
    if (wsserv.ctx == NULL)
    {
        USP_LOG_Error("%s: lws_create_context() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }
    USP_LOG_Info("%s: Started Websocket server listening on interface=%s", __FUNCTION__, wsserv.wan_addr);

    // Indicate that the server is now started, and initialise rest of fields
    CopyWssConfig(&wsserv.cur_config, config);
    wsserv.schedule_restart = kScheduledAction_Off;
    wsserv.schedule_stop = kScheduledAction_Off;

    // Schedule the WAN poll callback
    #define WAN_IP_ADDR_POLL_PERIOD 5
    lws_sul_schedule(wsserv.ctx, 0, &wsserv.wan_poll_timer, HandleWssEvent_WanPollTimer, WAN_IP_ADDR_POLL_PERIOD*LWS_US_PER_SEC);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** StopWebsockServer
**
** Stops the agent's websocket server, freeing all memory associated with it
**
** \param   Mone
**
** \return  None
**
**************************************************************************/
void StopWebsockServer(void)
{
    int i;
    wsconn_t *wc;

    // First cancel our libwebsockets timer, we need to do this otherwise our structure contains references
    // to memory which is freed when the libwebsocket context is destroyed
    lws_sul_cancel(&wsserv.wan_poll_timer);

    // Destroy the libwebsocket context
    // NOTE: This also frees libwebsockets' SSL context and associated certs
    lws_context_destroy(wsserv.ctx);
    wsserv.ctx = NULL;

    // Iterate over all websock connections, freeing any memory they're using
    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if (wc->ws_handle != NULL)
        {
            FreeWebsockServerConnection(wc);
        }
    }

    // Free cur_config
    USP_SAFE_FREE(wsserv.cur_config.path);

    // NOTE: Do not free new_config, as we will need it if restarting the server with a different config
    // NOTE: Do not modify schedule_restart or schedule_stop. We want their state to persist so that we can decide whether to restart or not
}

/*********************************************************************//**
**
** FreeWebsockServerConnection
**
** Stops the agent's websocket server, freeing all memory associated with it
** NOTE: It is assumed that the caller closes the associated libwebsockets ws_handle
**
** \param   wc - pointer to websocket server connection to free
**
** \return  None
**
**************************************************************************/
void FreeWebsockServerConnection(wsconn_t *wc)
{
    wsserv_send_item_t *si;

    // Free all queued messages
    // NOTE: The code should have already ensured this
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    while (si != NULL)
    {
        RemoveWsservSendItem(wc, si);
        si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    }

    // Free the receive buf
    USP_SAFE_FREE(wc->rx_buf);
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;
    wc->tx_index = 0;

    // Mark the entry as not in use
    wc->conn_id = INVALID;
    wc->ws_handle = NULL;
}

/*********************************************************************//**
**
** HandleAllWssEvents
**
** Called back by libwebsockets from within lws_service() to inform this code of activity on the websocket connection
** NOTE: Argument names matching the libwebsockets documentation are shown in brackets below
**
** \param   handle           (wsi) - libwebsockets handle identifying the connection with activity on it
** \param   event            (reason)- event for activity on the connection
** \param   per_session_data (user) - pointer to per-session data allocated by libwebsockets for this connection
** \param   event_args       (in) - pointer to structure/buffer containing arguments associated with the event
** \param   event_args_len   (len) - size of structure/buffer containing arguments associated with the event
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleAllWssEvents(struct lws *handle, enum lws_callback_reasons event, void *per_session_data, void *event_args, size_t event_args_len)
{
    struct lws_process_html_args *args;
    int result = 0;

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Process the event
    #define tr_event(...)    //USP_LOG_Debug(__VA_ARGS__) // uncomment the macro definition to get event debug
    switch (event)
    {
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
            tr_event("WS server: LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS");
            result = HandleWssEvent_LoadCerts(per_session_data);
            break;

        case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
            tr_event("WS server: LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION");
            result = HandleWssEvent_VerifyCerts(handle, event_args, event_args_len, per_session_data);
            break;


        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
            tr_event("WS server: LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED");
            result = HandleWssEvent_NewClient(handle);
            break;


        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            tr_event("WS server: LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
            result = ValidateReceivedWsservProtocolExtension(handle);
            break;


        case LWS_CALLBACK_ADD_HEADERS:
            tr_event("WS server: LWS_CALLBACK_ADD_HEADERS");
            args = (struct lws_process_html_args *)event_args;
            result = AddWsservUspExtension(handle, &args->p, args->max_len);
            break;

        case LWS_CALLBACK_ESTABLISHED:
            tr_event("WS server: LWS_CALLBACK_ESTABLISHED");
            result = HandleWssEvent_Established(handle);
            break;

        case LWS_CALLBACK_RECEIVE:
            tr_event("WS server: LWS_CALLBACK_RECEIVE");
            result = HandleWssEvent_Receive(handle, event_args, event_args_len);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            tr_event("WS server: LWS_CALLBACK_SERVER_WRITEABLE");
            result = HandleWssEvent_Transmit(handle);
		    break;

        case LWS_CALLBACK_WSI_DESTROY:
            tr_event("WS server: LWS_CALLBACK_WSI_DESTROY");
            result = HandleWssEvent_Destroyed(handle);
            break;


        case LWS_CALLBACK_TIMER:
            tr_event("WS server: LWS_CALLBACK_TIMER");
            result = HandleWssEvent_PingTimer(handle);
            break;

        case LWS_CALLBACK_RECEIVE_PONG:
            tr_event("WS server: LWS_CALLBACK_RECEIVE_PONG");
            result = HandleWssEvent_PongReceived(handle);
            break;

        default:
            tr_event("WS server: event=%d", event);
            break;
    }

    OS_UTILS_UnlockMutex(&wss_access_mutex);

    return result;
}

/*********************************************************************//**
**
** HandleWssEvent_Destroyed
**
** Called from the LWS_CALLBACK_WSI_DESTROY event
** after the remote server closes the connection, or we have force closed the connection
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWssEvent_Destroyed(struct lws *handle)
{
    wsconn_t *wc;
    wsserv_send_item_t *si;

    // Exit if nothing to free
    wc = lws_get_opaque_user_data(handle);
    if (wc == NULL)
    {
        return 0;
    }

    USP_LOG_Info("%s: Closed connection from %s", __FUNCTION__, wc->peer);

    // Free any partially received USP Record
    USP_SAFE_FREE(wc->rx_buf);
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;

    // Remove all items pending to send to the controller
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    while (si != NULL)
    {
        RemoveWsservSendItem(wc, si);
        si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    }

    // Mark the slot as not in use
    wc->ws_handle = NULL;

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_NewClient
**
** Called from the LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED event
** in response to a controller connecting to the agent's websocket server
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWssEvent_NewClient(struct lws *handle)
{
    wsconn_t *wc;

    // Exit if unable to allocate a websocket connection for this controller
    wc = FindFreeWsConnection();
    if (wc == NULL)
    {
        USP_LOG_Error("%s: Disallowing new connection as only %d concurrent ws client connections allowed", __FUNCTION__, (int)NUM_ELEM(ws_connections));
        return -1;
    }

    // Initialise the websocket connection
    wc->ws_handle = handle;
    DLLIST_Init(&wc->usp_record_send_queue);
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;
    wc->tx_index = 0;
    wc->role = (wsserv.cur_config.enable_encryption) ? ROLE_NON_SSL : ROLE_DEFAULT;
    wc->send_ping = false;
    wc->ping_count = 0;
    wc->role = ROLE_DEFAULT;
    wc->disconnect_sent = false;

    // Assign a unique server handle for this connection
    wc->conn_id = conn_id_counter++;
    if (conn_id_counter == INVALID)
    {
        conn_id_counter = 0;
    }

    // Determine the IP address of the peer to use in debug. This will be overridden with endpoint_id if present in Sec_WebSocket_Extensions header
    // NOTE: libwebsockets insists on using a IPv4 mapped IPv6 address to represent IPv4 addresses
    wc->is_peer_an_eid = false;
    lws_get_peer_simple(handle, wc->peer, sizeof(wc->peer));
    USP_LOG_Info("%s: %s is attempting to connect", __FUNCTION__, wc->peer);

    // Associate the websocket connection handle with our websocket connection structure
    lws_set_opaque_user_data(handle, wc);

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_LoadCerts
**
** Called from the LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS event
** to load the trust store and client certs into libwebsockets SSL context
** when libwebsockets is creating a libwebsockets context in lws_create_context()
**
** \param   ssl_ctx - libwebsockets handle identifying the connection with activity on it
** \param   err_msg - pointer to string identifying the error, or NULL if no error message provided
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWssEvent_LoadCerts(SSL_CTX *ssl_ctx)
{
    int err;

    // Exit if unable to load trust store and client cert into libwebsocket's SSL context
    err = DEVICE_SECURITY_LoadTrustStore(ssl_ctx, 0, NULL);
    if (err != USP_ERR_OK)
    {
        return -1;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_VerifyCerts
**
** Called from the LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION event
** Called back from OpenSSL for each certificate in the received server certificate chain of trust
** This function saves the certificate chain into the Websocket client connection structure
** This function is used to ignore certificate validation errors caused by system time being incorrect
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   ssl - pointer to SSL structure for this connection
** \param   preverify_ok - set to 1, if the current certificate passed, set to 0 if it did not
** \param   x509_ctx - pointer to context for certificate chain verification
**
** \return  0 if certificate chain should be trusted
**          1 if certificate chain should not be trusted, and connection dropped
**
**************************************************************************/
int HandleWssEvent_VerifyCerts(struct lws *handle, SSL *ssl, int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    wsconn_t *wc;
    int verify_ok;
    STACK_OF(X509) *cert_chain = NULL;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // If we have a certificate chain, then determine which role to allow for the controller on the Websocket connection
    verify_ok = DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(preverify_ok, x509_ctx, &cert_chain);
    if (cert_chain != NULL)
    {
        // NOTE: Ignoring any error returned by DEVICE_SECURITY_GetControllerTrust() - just leave the role to the default setup in HandleWssEvent_NewClient()
        DEVICE_SECURITY_GetControllerTrust(cert_chain, &wc->role);

        // Free the saved cert chain as we don't need it anymore
        sk_X509_pop_free(cert_chain, X509_free);
    }

    // Exit if cert chain could not be trusted
    if (verify_ok==0)
    {
        return 1;
    }

    return 0;
}

/*********************************************************************//**
**
** ValidateReceivedWsservProtocolExtension
**
** Called from the LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION event
** to validate that the endpoint specified in the Sec-WebSocket-Extensions header
** is not already connected to the agent's websocket client or already connected
** via another websocket client connection to this server
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection
**
**************************************************************************/
int ValidateReceivedWsservProtocolExtension(struct lws *handle)
{
    int num_bytes;
    char buf[256];
    char *endpoint_id;
    wsconn_t *wc;

    // Exit if Sec-WebSocket-Protocol header is present, but too large for the buffer. In this case we just ignore the header.
    num_bytes = lws_hdr_copy(handle, buf, sizeof(buf)-1, WSI_TOKEN_PROTOCOL);
    if (num_bytes == -1)
    {
        USP_LOG_Error("%s: lws_hdr_copy() failed (%d)", __FUNCTION__, num_bytes);
        return -1;
    }

    // Exit if the client did not provide a Sec-WebSocket-Protocol header in its request
    // NOTE: libwebsockets allows this case, so we have to prevent it explicitly
    if ((num_bytes == 0) || (buf[0] == '\0'))
    {
        USP_LOG_Error("%s: Client did not specify any websocket subprotocol in handshake request", __FUNCTION__);
        return -1;
    }

    // Exit if subprotocol requested by client does not match the one we support
    if (strcmp(buf, WEBSOCKET_PROTOCOL_STR) !=0)
    {
        USP_LOG_Error("%s: Client specified incorrect websocket subprotocol in handshake request (got '%s', expected '%s')", __FUNCTION__, buf, WEBSOCKET_PROTOCOL_STR);
        return -1;
    }

    //---------------------------------------
    // Exit if Sec-WebSocket-Extensions header is present, but too large for the buffer. In this case we just ignore the header.
    // NOTE: If the header is not present, then this call returns num_bytes=0
    num_bytes = lws_hdr_copy(handle, buf, sizeof(buf)-1, WSI_TOKEN_EXTENSIONS);
    if (num_bytes == -1)
    {
        USP_LOG_Error("%s: lws_hdr_copy(Sec-WebSocket-Extensions) failed (%d)", __FUNCTION__, num_bytes);
        return 0;
    }

    // Exit if the client did not provide a Sec-WebSocket-Extensions header in its response
    // NOTE: This is not an error
    if ((num_bytes == 0) || (buf[0] == '\0'))
    {
        return 0;
    }

    // Exit if unable to parse the endpoint ID. NOTE: This is not an error
    #define EID_SEPARATOR  "eid="
    endpoint_id = strstr(buf, EID_SEPARATOR);
    if (endpoint_id == NULL)
    {
        return 0;
    }
    endpoint_id += sizeof(EID_SEPARATOR)-1;   // Skip the separator to point to the endpoint_id

    // Strip whitespace and speech marks
    endpoint_id = TEXT_UTILS_TrimDelimitedBuffer(endpoint_id, "\"\"");

    // Exit if endpoint_id is empty. NOTE: This is not an error
    if (*endpoint_id == '\0')
    {
        return 0;
    }

    // Exit if already connected to this controller via the agent's websocket client. Abort the connection to the websocket server
    if (WSCLIENT_IsEndpointConnected(endpoint_id))
    {
        USP_LOG_Error("%s: Not permitting controller eid='%s' to connect. Already connected via agent's websocket client", __FUNCTION__, endpoint_id);
        return -1;
    }

    // Exit if already connected to this controller via another websocket client connection to this server
    wc = FindWsConnectionByEndpointId(endpoint_id);
    if (wc != NULL)
    {
        USP_LOG_Error("%s: Not permitting controller eid='%s' to connect. Already connected via agent's websocket server", __FUNCTION__, endpoint_id);
        return -1;
    }

    // Save the endpoint_id
    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);
    USP_STRNCPY(wc->peer, endpoint_id, sizeof(wc->peer));
    wc->is_peer_an_eid = true;

    // NOTE: No need to validate the Sec-WebSocket-Protocol header provided by the client in the session initiation request
    // libwebsockets ensures that the list of websocket sub-protocols provided in the header contains the sub-protocol that we support

    return 0;
}

/*********************************************************************//**
**
** AddWsservUspExtension
**
** Called from the LWS_CALLBACK_ADD_HEADERS event
** to add the 'Sec-WebSocket-Extensions' header with the 'bbf-usp-protocol' WebSocket Extension
** in the websocket initiation handshake response
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   ppHeaders - pointer to variable containing a pointer to a buffer containing the headers
** \param   headers_len - total size of the libwebsockets buffer containing the headers
**
** \return  0 if successful, -1 to close the connection
**
**************************************************************************/
int AddWsservUspExtension(struct lws *handle, char **ppHeaders, int headers_len)
{
    char *pHeadersEnd;
    char buf[128];
    int len;
    int err;

    // Exit, disallowing all new connectins whilst trying to stop gracefully
    if (WantingToStopWsServer())
    {
        USP_LOG_Error("%s: Disallowing new connection, since stopping the websocket server.", __FUNCTION__)
        return -1;
    }

    // If unable to add the header, then return an error, which will cause the connection to be dropped
    pHeadersEnd = (*ppHeaders) + headers_len;
    len = USP_SNPRINTF(buf, sizeof(buf), "bbf-usp-protocol; eid=\"%s\"", DEVICE_LOCAL_AGENT_GetEndpointID());
    err = lws_add_http_header_by_token(handle, WSI_TOKEN_EXTENSIONS, (unsigned char *)buf, len, (unsigned char **)ppHeaders, (unsigned char *)pHeadersEnd);
    if (err != 0)
    {
        USP_LOG_Error("%s: lws_add_http_header_by_token() returned %d", __FUNCTION__, err);
        return -1;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_Established
**
** Called from the LWS_CALLBACK_ESTABLISHED event
** in response to a controller connecting to the agent's websocket server
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWssEvent_Established(struct lws *handle)
{
    int hdr_len;
    char buf[256];
    char *path;
    wsconn_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit if unable to get the path from the websocket URL that the controller was connecting to
    hdr_len = lws_hdr_copy(handle, buf, sizeof(buf), WSI_TOKEN_GET_URI);
    if (hdr_len == -1)
    {
        USP_LOG_Error("%s: lws_hdr_copy() returned an error. Closing connection.", __FUNCTION__)
        return -1;
    }

    // Calculate the path which we are listening on - an empty config path should be treated the same as '/'
    path = (wsserv.cur_config.path[0] == '\0') ? "/" : wsserv.cur_config.path;

    // Exit if the controller was trying to connect to the wrong URL path
    if (strcmp(buf, path) != 0)
    {
        USP_LOG_Error("%s: Controller was trying to connect to path '%s', but agent is listening on '%s'. Closing Connection.", __FUNCTION__, buf, path);
        return -1;
    }

    USP_LOG_Info("%s: Accepted connection from %s", __FUNCTION__, wc->peer);

    // Ensure a USP Connect record is sent
    QueueUspConnectRecord_Wsserv(wc);

    // Set a timer to expire when it is time to send the websocket PING frame
    lws_set_timer_usecs(handle, wsserv.cur_config.keep_alive * LWS_USEC_PER_SEC);

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_Receive
**
** Called from the LWS_CALLBACK_RECEIVE event
** to process a received USP record
** NOTE: This function is called for each chunk of (upto) size RX_CHUNK_SIZE
**       There are multiple chunks in a websocket fragment, and (possibly) multiple websocket
**       fragments in a websocket message (which contains a single USP Record)
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   chunk - pointer to buffer containing a chunk of the received USP record
** \param   chunk_len - length of received USP record chunk
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWssEvent_Receive(struct lws *handle, unsigned char *chunk, int chunk_len)
{
    mtp_reply_to_t mtp_reply_to;
    char buf[MAX_ISO8601_LEN];
    wsconn_t *wc;
    int new_len;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // No need to receive a PONG packet, if we received some other packet signalling keep alive
    wc->ping_count = 0;

    // Exit, sending a close frame if we received a text frame, instead of a binary frame
    if (lws_frame_is_binary(handle)==false)
    {
        return CloseWsservConnection(handle, LWS_CLOSE_STATUS_INVALID_PAYLOAD, "%s: Text frame received from %s. Expecting binary frame.", __FUNCTION__, wc->peer);
    }

    // Calculate the total size needed for all websocket fragments received so far (including this one)
    // NOTE: We may only have received a chunk of the current websocket fragment - lws_remaining_packet_payload() returns the number of bytes remaining in the current websocket fragment
    new_len = wc->rx_buf_len + chunk_len + lws_remaining_packet_payload(handle);

    // Exit, sending a close frame, if the size of the USP Record is larger than we allow
    if (new_len > MAX_USP_MSG_LEN)
    {
        return CloseWsservConnection(handle, LWS_CLOSE_STATUS_MESSAGE_TOO_LARGE, "%s: %s sent a message >%d bytes long (%d bytes). Closing connection.", __FUNCTION__, wc->peer, MAX_USP_MSG_LEN, new_len);
    }

    // Increase the size of the chunk buffer to receive this websocket fragment
    if (new_len > wc->rx_buf_max_len)
    {
        wc->rx_buf = USP_REALLOC(wc->rx_buf, new_len);
        wc->rx_buf_max_len = new_len;
    }

    // Add the chunk to the buffer, building up the received USP Record
    memcpy(&wc->rx_buf[wc->rx_buf_len], chunk, chunk_len);
    wc->rx_buf_len += chunk_len;

    // Exit if not all chunks of the USP Record have been received yet
    if (!lws_is_final_fragment(handle))
    {
        return 0;
    }

    // Log time at which message was received
    iso8601_cur_time(buf, sizeof(buf));
    USP_PROTOCOL("\n");
    USP_LOG_Info("Websocket server received message at time %s, from %s", buf, wc->peer);

    // Send the USP Record to the data model thread for processing
    // NOTE: Ownership of receive buffer stays with this thread
    memset(&mtp_reply_to, 0, sizeof(mtp_reply_to));
    mtp_reply_to.is_reply_to_specified = true;
    mtp_reply_to.protocol = kMtpProtocol_WebSockets;
    mtp_reply_to.wsclient_cont_instance = INVALID;
    mtp_reply_to.wsclient_mtp_instance = INVALID;
    mtp_reply_to.wsserv_conn_id = wc->conn_id;
    mtp_reply_to.cont_endpoint_id = (wc->is_peer_an_eid) ? wc->peer : NULL;
    DM_EXEC_PostUspRecord(wc->rx_buf, wc->rx_buf_len, wc->role, &mtp_reply_to);

    // Free receive buffer
    USP_FREE(wc->rx_buf);
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_Transmit
**
** Called from the LWS_CALLBACK_SERVER_WRITEABLE event
** to send a USP record (if one is queued to send) or send a PING frame (if it is time to do so)
** or to gracefully close the connection
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWssEvent_Transmit(struct lws *handle)
{
    wsconn_t *wc;
    wsserv_send_item_t *si;
    int bytes_written;
    int bytes_remaining;
    int chunk_len;
    unsigned char tx_buf[LWS_PRE + TX_CHUNK_SIZE];
    unsigned write_flags;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit, causing the websocket to close if a disconnect frame has just been sent
    if (wc->disconnect_sent)
    {
        wc->disconnect_sent = false;    // Ensure that idf this function is called again before closing that it doesn't repeat this action
        return CloseWsservConnection(handle, LWS_CLOSE_STATUS_POLICY_VIOLATION, "%s: Disconnect Record sent", __FUNCTION__);
    }

    // Exit, sending a Ping frame, if is time to do so
    // NOTE: Its is safe to send a ping frame whilst in the middle of sending the chunks of a USP Record
    // (because each chunk is sent as a separate Binary or Continuation frame)
    if (wc->send_ping)
    {
        return SendWsservPing(wc);
    }

    // Exit if no item to send
    // NOTE: It is possible for this to occur under normal circumstances
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    if (si == NULL)
    {
        // Exit, sending a websocket Close frame if gracefully closing the websocket connection after all responses have been sent
        if (WantingToStopWsServer())
        {
            return CloseWsservConnection(handle, LWS_CLOSE_STATUS_GOINGAWAY, "%s: Disabling websocket server", __FUNCTION__);
        }

        return 0;
    }

    // Exit if all of the USP records in the send queue were notifications that have expired
    RemoveExpiredWsservMessages(wc);
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    if (si == NULL)
    {
        return 0;
    }

    // If the code gets here, then we have a USP Record which we'd like to send

    // Determine whether we are sending the first chunk
    if (wc->tx_index == 0)
    {
        // Log the USP Record before we send the first chunk
        MSG_HANDLER_LogMessageToSend(&si->item, kMtpProtocol_WebSockets, wc->peer, NULL);
        write_flags = LWS_WRITE_BINARY;
    }
    else
    {
        // This isn't the first chunk
        write_flags = LWS_WRITE_CONTINUATION;
    }

    // Determine whether we are sending the last chunk
    bytes_remaining = si->item.pbuf_len - wc->tx_index;
    if (bytes_remaining <= TX_CHUNK_SIZE)
    {
        // This is the last chunk
        chunk_len = bytes_remaining;
    }
    else
    {
        // This isn't the last chunk
        chunk_len = TX_CHUNK_SIZE;
        write_flags |= LWS_WRITE_NO_FIN;
    }

    // Copy a chunk into our local buffer
    USP_ASSERT(bytes_remaining > 0);
    memcpy(&tx_buf[LWS_PRE], &si->item.pbuf[wc->tx_index], chunk_len);

    // Exit if unable to send the chunk
    // NOTE: We do not expect this call to fail, as libwebsockets buffers the data if you try to send too much
    bytes_written = lws_write(handle, &tx_buf[LWS_PRE], chunk_len, write_flags);
    if (bytes_written < chunk_len)
    {
        return CloseWsservConnection(handle, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, "%s: lws_write wrote only %d bytes (wanted %d bytes)", __FUNCTION__, bytes_written, si->item.pbuf_len);
    }

    // If this is a disconnect record, schedule the websocket close after the disconnect record has been sent
    // NOTE: USP Disconnect records closing an E2E session (kMtpContentType_E2E_SessTermination) do not close the connection
    if (si->item.content_type == kMtpContentType_DisconnectRecord)
    {
        wc->disconnect_sent = true;
    }

    // Free the USP Record from the queue, if all chunks of it have been sent
    wc->tx_index += chunk_len;
    if (wc->tx_index >= si->item.pbuf_len)
    {
        RemoveWsservSendItem(wc, si);
        wc->tx_index = 0;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_PingTimer
**
** Called from the LWS_CALLBACK_TIMER event
** when it is time to send a websocket ping frame
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWssEvent_PingTimer(struct lws *handle)
{
    int err;
    wsconn_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit if failed to ask libwebsockets for permission to send the ping
    err = lws_callback_on_writable(handle);
    if (err != 1)
    {
        // If an error occurred, then close the TCP connection
        USP_LOG_Error("%s: lws_callback_on_writable() returned %d", __FUNCTION__, err);
        return -1;
    }

    // Indicate that the next LWS_CALLBACK_CLIENT_WRITEABLE event should send a ping frame
    wc->send_ping = true;

    // Rearm the timer for sending the next websocket PING frame
    lws_set_timer_usecs(handle, wsserv.cur_config.keep_alive * LWS_USEC_PER_SEC);

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_PongReceived
**
** Called from the LWS_CALLBACK_RECEIVE_PONG event
** to process a received PONG record
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWssEvent_PongReceived(struct lws *handle)
{
    wsconn_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Reset the count of consecutive pings sent without any pongs
    USP_LOG_Debug("%s: Received PONG at time %d from %s", __FUNCTION__, (int)time(NULL), wc->peer);
    wc->ping_count =0;

    return 0;
}

/*********************************************************************//**
**
** HandleWssEvent_WanPollTimer
**
** Called directly from libwebsockets as part of scheduled timer
** when it is time to poll the WAN interface for IP address changes
**
** \param   sul - pointer to stucture containing the scheduled timer
**
** \return  None
**
**************************************************************************/
void HandleWssEvent_WanPollTimer(lws_sorted_usec_list_t *sul)
{
    int err;
    char new_wan_addr[NU_IPADDRSTRLEN];

    OS_UTILS_LockMutex(&wss_access_mutex);

    // Check that structure is not part of a linked list owned by libwebsockets anymore
    AssertWanPollCallbackNotInUse(sul);

    // Exit if unable to determine IP address of WAN interface
    // In this case, we keep the current websocket server running, as it's just that the interface is down
    new_wan_addr[0] = '\0';
    err = CalcWsservIpAddr(new_wan_addr, sizeof(new_wan_addr));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if IP address hasn't changed
    USP_ASSERT(new_wan_addr[0] != '\0');    // Above code should have ensured that the WAN address is not an empty string
    if (strcmp(new_wan_addr, wsserv.wan_addr)==0)
    {
        goto exit;
    }

    // If the code gets here, then the IP address on the WAN interface has changed, so schedule a restart of the server
    USP_LOG_Info("%s: Restarting WebSocket server as IP address has changed to %s (from %s)", __FUNCTION__, new_wan_addr, wsserv.wan_addr);
    wsserv.schedule_restart = kScheduledAction_Activated;

exit:
    // Rearm timer to poll the address again
    lws_sul_schedule(wsserv.ctx, 0, &wsserv.wan_poll_timer, HandleWssEvent_WanPollTimer, WAN_IP_ADDR_POLL_PERIOD*LWS_US_PER_SEC);

    OS_UTILS_UnlockMutex(&wss_access_mutex);
}

/*********************************************************************//**
**
** AssertWanPollCallbackNotInUse
**
** Checks that the wan poll callback is present in any linked lists in use by libwebsockets
**
** \param   sul - pointer to structure associated with libwebsocket scheduled callbacks
**
** \return  None
**
**************************************************************************/
void AssertWanPollCallbackNotInUse(lws_sorted_usec_list_t *sul)
{
    // Check that structure is not part of a linked list owned by libwebsockets anymore
    USP_ASSERT(sul->list.prev == NULL);
    USP_ASSERT(sul->list.next == NULL);
    USP_ASSERT(sul->list.owner == NULL);
}

/*********************************************************************//**
**
** CalcWsservIpAddr
**
** Calculates the IP address of the interface that the websocket server listens
** NOTE: "any" is returned if all IP addresses should be used
** NOTE: empty string is returned if no IP address found for the specified interface
**
** \param   buf - buffer in which to write the address in string form
** \param   len - length of buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcWsservIpAddr(char *buf, int len)
{
    int err;
    char *interface;
    bool prefer_ipv6;

    // Exit if websocket server should listen on all interfaces
    interface = (usp_interface != NULL) ? usp_interface : WEBSOCKET_LISTEN_INTERFACE;
    if ((*interface == '\0') || (strcmp(interface, "any")==0))
    {
        USP_STRNCPY(buf, "any", len);
        return USP_ERR_OK;
    }

    // Get preference for IPv4 or IPv6 address (in case of Dual Stack CPE)
    prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

    // Exit if unable to get current IP address for listening on
    err = tw_ulib_get_dev_ipaddr(interface, buf, len, prefer_ipv6);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to get IP address of WAN interface (%s). Retrying later", __FUNCTION__, interface);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SendWsservPing
**
** Called to send a websocket ping frame from the agent's websocket server
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int SendWsservPing(wsconn_t *wc)
{
    unsigned char buf[LWS_PRE+1];       // Plus 1 to allow for a single data byte in the ping frame
    int bytes_written;

    // Exit if we've sent out too many pings without any pong response
    #define ALLOWED_MISSING_PONGS 2  // 0=all pings must have a pong, 1=one ping can be missing a pong etc
    if (wc->ping_count > ALLOWED_MISSING_PONGS)
    {
        USP_LOG_Error("%s: Closing connection to endpoint_id=%s because missing %d websocket PONG responses", __FUNCTION__, wc->peer, wc->ping_count);
        return -1;      // NOTE: Not sending close frame as unlikely to get any ACK
    }
    wc->ping_count++;

    // Reset the flag which causes this function to be called, as we are going to send the ping now
    wc->send_ping = false;

    // Write PING frame
    // NOTE: The ping frame must not be empty, otherwise the pong frame will be empty
    // and this causes libwebsockets to not send the LWS_CALLBACK_RECEIVE_PONG event
    USP_LOG_Debug("%s: Sending PING at time %d to %s", __FUNCTION__, (int)time(NULL), wc->peer);
    buf[LWS_PRE] = wc->ping_count;
    bytes_written = lws_write(wc->ws_handle, &buf[LWS_PRE], 1, LWS_WRITE_PING);
    if (bytes_written < 0)
    {
        USP_LOG_Error("%s: lws_write() failed", __FUNCTION__);
        return -1;   // NOTE: Not sending close frame as unlikely to get any ACK
    }

    return 0;
}

/*********************************************************************//**
**
** QueueUspConnectRecord_Wsserv
**
** Adds the USP connect record at the front of the queue, ensuring that there is only one connect record in the queue
**
** \param   wc - pointer to websocket server connection to send the connect record to
**
** \return  None
**
**************************************************************************/
void QueueUspConnectRecord_Wsserv(wsconn_t *wc)
{
    wsserv_send_item_t *cur_msg;
    wsserv_send_item_t *next_msg;
    wsserv_send_item_t *send_item;
    mtp_content_type_t type;

    // Iterate over USP Records in the queue, removing all stale connect and disconnect records
    // A connect or disconnect record may still be in the queue if the connection failed before the record was fully sent
    cur_msg = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    while (cur_msg != NULL)
    {
        // Save pointer to next message, as we may remove the current message
        next_msg = (wsserv_send_item_t *) cur_msg->link.next;

        // Remove current message if it is a connect or disconnect record
        type = cur_msg->item.content_type;
        if (IsUspConnectOrDisconnectRecord(type))
        {
            RemoveWsservSendItem(wc, cur_msg);
        }

        // Move to next message in the queue
        cur_msg = next_msg;
    }

    // Exit if we don't have a endpoint_id for the controller that has connected
    // (because we can't send a record if we don't know which controller to address it to)
    if (wc->is_peer_an_eid == false)
    {
        return;
    }

    // Add the new connect record to the queue
    send_item = USP_MALLOC(sizeof(wsserv_send_item_t));
    USPREC_WebSocketConnect_Create(wc->peer, &send_item->item);
    send_item->expiry_time = END_OF_TIME;

    DLLIST_LinkToHead(&wc->usp_record_send_queue, send_item);
}

/*********************************************************************//**
**
** CountWsConnections
**
** Counts the number of active connections to this websocket server
**
** \param   None
**
** \return  Number of active connections
**
**************************************************************************/
int CountWsConnections(void)
{
    int i;
    wsconn_t *wc;
    int count = 0;

    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if (wc->ws_handle != NULL)
        {
            count++;
        }
    }

    return count;
}

/*********************************************************************//**
**
** CloseWsservConnection
**
** Called to close the connection when something is wrong
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   close_status -  websockets reason for closing the connection
** \param   fmt - printf style format
**
** \return  Always -1 (to cause libwebsockets to send the close fame and close the connection)
**
**************************************************************************/
int CloseWsservConnection(struct lws *handle, int close_status, char *fmt, ...)
{
    va_list ap;
    char buf[256];
    int chars_written;

    // Print the message to the buffer
    va_start(ap, fmt);
    chars_written = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    // Log the message
    USP_LOG_Error("%s", buf);

    lws_close_reason(handle, close_status, (unsigned char *)buf, chars_written);

    return -1;      // Returning an error is what forces libwebsockets to send the close frame
}

/*********************************************************************//**
**
** FindFreeWsConnection
**
** Finds the first unused entry in ws_connections[]
**
** \param   None
**
** \return  pointer to unused entry in ws_connections[] or NULL, if no free entries left
**
**************************************************************************/
wsconn_t *FindFreeWsConnection(void)
{
    int i;
    wsconn_t *wc;

    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if (wc->ws_handle == NULL)
        {
            return wc;
        }
    }

    // If the code gets here, then no free entry was found
    return NULL;
}

/*********************************************************************//**
**
** FindWsConnectionByConnId
**
** Finds the connection matching the specified conn_id
**
** \param   conn_id - handle uniquely identifying connection
**
** \return  pointer to entry in ws_connections[] or NULL, if no free entries left
**
**************************************************************************/
wsconn_t *FindWsConnectionByConnId(int conn_id)
{
    int i;
    wsconn_t *wc;

    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if ((wc->ws_handle != NULL) && (wc->conn_id == conn_id))
        {
            return wc;
        }
    }

    // If the code gets here, then no matching entry was found
    return NULL;
}

/*********************************************************************//**
**
** FindWsConnectionByEndpointId
**
** Finds the connection matching the specified endpoint_id (parsed from Sec-WebSocket-Extensions header)
**
** \param   conn_id - handle uniquely identifying connection
**
** \return  pointer to entry in ws_connections[] or NULL, if no free entries left
**
**************************************************************************/
wsconn_t *FindWsConnectionByEndpointId(char *endpoint_id)
{
    int i;
    wsconn_t *wc;

    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if ((wc->ws_handle != NULL) && (strcmp(wc->peer, endpoint_id)==0))
        {
            return wc;
        }
    }

    // If the code gets here, then no matching entry was found
    return NULL;
}

/*********************************************************************//**
**
** AreAllWebsockServerMessagesSent
**
** Determines whether all outgoing USP Records have been sent
** This function is used to ensure that all responses have been sent before
** changing server config or shutting down (due to Reboot() or FactoryReset() USP commands)
**
** \param   None
**
** \return  true if there are no pending USP Records to send to any controllers over websockets
**
**************************************************************************/
bool AreAllWebsockServerMessagesSent(void)
{
    int i;
    wsconn_t *wc;

    // Iterate over all active connections, exiting if any have messages left to send
    for (i=0; i<NUM_ELEM(ws_connections); i++)
    {
        wc = &ws_connections[i];
        if ((wc->ws_handle != NULL) && (wc->usp_record_send_queue.head != NULL))
        {
            return false;
        }
    }

    // If the code gets here, then no messages are left to send
    return true;
}

/*********************************************************************//**
**
** IsUspRecordInWsservQueue
**
** Determines whether the specified USP record is already queued, waiting to be sent
** This is used to avoid duplicate records being placed in the queue, which could occur under notification retry conditions
**
** \param   wc - pointer to websocket connection which has queued USP records to send
** \param   pbuf - pointer to buffer containing USP Record to match against
** \param   pbuf_len - length of buffer containing USP Record to match against
**
** \return  true if the message is already queued
**
**************************************************************************/
bool IsUspRecordInWsservQueue(wsconn_t *wc, unsigned char *pbuf, int pbuf_len)
{
    wsserv_send_item_t *si;

    // Iterate over USP Records in the queue
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;
    while (si != NULL)
    {
        // Exit if the USP record is already in the queue
        if ((si->item.pbuf_len == pbuf_len) && (memcmp(si->item.pbuf, pbuf, pbuf_len)==0))
        {
             return true;
        }

        // Move to next message in the queue
        si = (wsserv_send_item_t *) si->link.next;
    }

    // If the code gets here, then the USP record is not in the queue
    return false;
}

/*********************************************************************//**
**
** RemoveExpiredWsservMessages
**
** Removes all expired messages from the queue of messages to send
** NOTE: This mechanism can be used to prevent the queue from filling up needlessly if the controller is offine
**
** \param   wc - pointer to websocket connection which has queued USP records to send
**
** \return  None
**
**************************************************************************/
void RemoveExpiredWsservMessages(wsconn_t *wc)
{
    time_t cur_time;
    wsserv_send_item_t *si;
    wsserv_send_item_t *next_msg;

    cur_time = time(NULL);
    si = (wsserv_send_item_t *) wc->usp_record_send_queue.head;

    // Skip the first USP Record, if we're currently in the process of transmitting it
    if ((si != NULL) && (wc->tx_index != 0))
    {
        si = (wsserv_send_item_t *) si->link.next;
    }

    while (si != NULL)
    {
        next_msg = (wsserv_send_item_t *) si->link.next;
        if (cur_time > si->expiry_time)
        {
            RemoveWsservSendItem(wc, si);
        }

        si = next_msg;
    }
}

/*********************************************************************//**
**
** RemoveWsservSendItem
**
** Removes the specified item from the send queues linked list, and frees it and all it's components
**
** \param   wc - pointer to websocket client connection on which the send queue resides
** \param   si - sned irem to remove from the queue
**
** \return  None
**
**************************************************************************/
void RemoveWsservSendItem(wsconn_t *wc, wsserv_send_item_t *si)
{
    DLLIST_Unlink(&wc->usp_record_send_queue, si);
    USP_FREE(si->item.pbuf);
    USP_FREE(si);
}

/*********************************************************************//**
**
** CopyWssConfig
**
** Copies the websocket server configuration
**
** \param   dest - pointer to structure in which to deep copy the configuration parameters
** \param   src - pointer to structure containing configuration parameters to deep copy
**
** \return  None
**
**************************************************************************/
void CopyWssConfig(wsserv_config_t *dest, wsserv_config_t *src)
{
    dest->port = src->port;
    dest->enable_encryption = src->enable_encryption;
    USP_SAFE_FREE(dest->path);
    dest->path = USP_STRDUP(src->path);
    dest->keep_alive = src->keep_alive;
}

/*********************************************************************//**
**
** DestroyWssConfig
**
** Frees the websocket server configuration
**
** \param   config - pointer to structure containing configuration parameters to free
**
** \return  None
**
**************************************************************************/
void DestroyWssConfig(wsserv_config_t *config)
{
    USP_SAFE_FREE(config->path);
    memset(config, 0, sizeof(wsserv_config_t));
}

/*********************************************************************//**
**
** WantingToStopWsServer
**
** Determines whether the USP Agent is wanting to stop the websocket server
**
** \param   None
**
** \return  true if the USP Agent is wanting to stop the websocket server
**
**************************************************************************/
bool WantingToStopWsServer(void)
{
    return (wsserv.schedule_restart == kScheduledAction_Activated) ||
           (wsserv.schedule_stop == kScheduledAction_Activated) ||
           (mtp_exit_scheduled == kScheduledAction_Activated);
}

/*********************************************************************//**
**
** LogWsserverDebug
**
** Logs debug from libwebsocket to USP Agent's configured output
**
** \param   level - bitmask of type of debug to emit (eg. LLL_ERR)
** \param   line - pointer to buffer containing log message to emit
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
void LogWsserverDebug(int level, const char *line)
{
    USP_LOG_Debug("%s: %s", __FUNCTION__, line);
}

#endif
