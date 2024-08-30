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
 * \file wsclient.c
 *
 * Implements a websocket client connection from agent to controller
 *
 */

#ifdef ENABLE_WEBSOCKETS
#include <libwebsockets.h>
#include <unistd.h>

#include "common_defs.h"
#include "dllist.h"
#include "wsclient.h"
#include "wsserver.h"
#include "msg_handler.h"
#include "os_utils.h"
#include "iso8601.h"
#include "dm_exec.h"
#include "retry_wait.h"
#include "nu_ipaddr.h"
#include "nu_macaddr.h"
#include "text_utils.h"

//------------------------------------------------------------------------------
// State of a websocket client connection
typedef enum
{
    kWebsockState_Idle,                     // Used to mark the entry in websock_clients[] as unused
    kWebsockState_AwaitingConnected,        // Awaiting successful connection
    kWebsockState_Running,                  // Normal steady state: Connection is ready to send and receive USP messages
                                            // NOTE: When closing the connection, the state stays as 'Running' until closed
    kWebsockState_Retrying,                 // An error has occurred. We have dropped the TCP connection and will attempt a reconnect at some time in the future

    kWebsockState_Max
} wsclient_state_t;

//------------------------------------------------------------------------------
// Payload to send in WebSocket queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    mtp_send_item_t item;   // Information about the content to send
    time_t expiry_time;     // Time at which this USP record should be removed from the queue
} wsclient_send_item_t;

//------------------------------------------------------------------------------
// Used to record the reason for receiving a LWS_CALLBACK_WSI_DESTROY event
// This is necessary because we need to handle the LWS_CALLBACK_WSI_DESTROY event differently depending on why it occurred
// and libwebsockets provides no other way for us to pass a reason to the LWS_CALLBACK_WSI_DESTROY handler
typedef enum
{
    kWebSockCloseReason_Unknown,                 // Unknown reason why connection was closed
    kWebSockCloseReason_ServerDisconnect,        // Remote server closed the connection
    kWebSockCloseReason_LibWebSocketsError,      // Libwebsockets had an error. This maybe protocol related, or OS related
    kWebSockCloseReason_InternalError,           // We forced the closure because an error was detected.
    kWebSockCloseReason_GracefulClose,           // We forced the closure because the MTP was being disabled (or a stop/reboot was scheduled).
    kWebSockCloseReason_ConnParamsChanged,       // We forced the closure because the connection parameters changed
    kWebSockCloseReason_BadUspRecord,            // We forced the closure, because we received a badly formed USP record
    kWebSockCloseReason_MissingPong,             // We forced the closure, because too many pong heartbeat frames were missing

    kWebSockCloseReason_Max
} wsclient_close_reason_t;

//------------------------------------------------------------------------------
// Structure representing a websocket client
typedef struct
{
    wsclient_state_t state;         // State of the connection. kWebsockState_Idle indicates that this entry in the array is unused

    // Configuration info
    int cont_instance;              // instance number of controller in Device.LocalAgent.Controller.{i}
    int mtp_instance;               // mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
    char *cont_endpoint_id;         // endpoint_id of the controller being connected to
    char *host;                     // host to send USP messages to the controller on
    unsigned port;                  // port to send USP messages to the controller on
    char *path;                     // Path to send USP messages to the controller on
    bool enable_encryption;         // Set if TLS must be used on the connection

    unsigned keep_alive_interval;   // Number of seconds between transmitting Websock PING frames
    unsigned retry_interval;        // Interval constant to use when calculating the exponential backoff period, when retrying a connection
    unsigned retry_multiplier;      // Multiplier constant to use when calculating the exponential backoff period, when retrying a connection
    int retry_count;                // Number of times that the connection has been tried, and has failed. Starts from 0.

    // State variables
    double_linked_list_t usp_record_send_queue;  // Queue of USP Records to send
    struct lws *ws_handle;        // pointer to structure allocated by libwebsockets for this client connection
    lws_sorted_usec_list_t retry_timer;     // structure used by libwebsockets for scheduled timer, used for connection retries
    wsclient_close_reason_t close_reason; // Used to record the reason for receiving a LWS_CALLBACK_WSI_DESTROY event
    bool send_ping;                 // Set if the next LWS_CALLBACK_CLIENT_WRITEABLE event should send a ping frame (rather than servicing the USP record send queue)
    int ping_count;                 // Number of websocket ping frames sent without corresponding pong responses

    unsigned char *rx_buf;          // Buffer used to build up the USP record as chunks are received
    int rx_buf_len;                 // Current size of received USP Record in rx_buf
    int rx_buf_max_len;             // Current allocated size of rx_buf. This will hold the current websocket fragment. If the USP Record is contained in multiple websocket fragments, then the buffer is reallocated to include the new fragment
    int tx_index;                   // Counts the number of bytes sent of the current USP Record to tx (i.e. at the head of the usp_record_send_queue)

    bool disconnect_sent;           // Set if a USP Disconnect record is being sent. Causes a reconnect after the record has been transmitted.
    scheduled_action_t  schedule_reconnect;  // Sets whether a reconnect is scheduled after the send queue has cleared
    scheduled_action_t  schedule_close;      // Sets whether a close is scheduled after the send queue has cleared

    STACK_OF(X509) *cert_chain; // Full SSL certificate chain for the Websocket connection, collected in the SSL verify callback
    ctrust_role_t role;         // role granted by the CA cert in the chain of trust with the websockets server

} wsclient_t;

//------------------------------------------------------------------------------
// Array of active websocket connections
static wsclient_t wsclients[MAX_WEBSOCKET_CLIENTS];

//------------------------------------------------------------------------------
// Messages on the websocket client thread's message queue
// Message to start connection to the specified server, or restart connection with changed parameters
typedef struct
{
    int cont_instance;              // instance number of controller in Device.LocalAgent.Controller.{i}
    int mtp_instance;               // mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
    char *cont_endpoint_id;         // endpoint_id of the controller being connected to
    char *host;                     // Host to send USP messages to the controller on
    unsigned port;                  // Port to send USP messages to the controller on
    char *path;                     // Path to send USP messages to the controller on
    bool enable_encryption;         // Set if the connection should use TLS
    unsigned keep_alive_interval;   // Number of seconds between transmitting Websock PING frames
    unsigned retry_interval;        // Interval constant to use when calculating the exponential backoff period, when retrying a connection
    unsigned retry_multiplier;      // Multiplier constant to use when calculating the exponential backoff period, when retrying a connection
} start_client_msg_t;

// Message to stop connection to the specified server
typedef struct
{
    int cont_instance;              // instance number of controller in Device.LocalAgent.Controller.{i}
    int mtp_instance;               // mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
} stop_client_msg_t;

// Message to send a USP record to the specified server
typedef struct
{
    int cont_instance;              // instance number of controller in Device.LocalAgent.Controller.{i}
    int mtp_instance;               // mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)

    mtp_send_item_t item;           // Information about the content to send
    time_t expiry_time;             // time at which the USP record should be removed from the MTP send queue
} queue_client_usp_record_msg_t;

//------------------------------------------------------------------------------
// Enumeration of message type on the websocket client thread queue
typedef enum
{
    kWsclientMsgType_StartClient,
    kWsclientMsgType_StopClient,
    kWsclientMsgType_ActivateScheduledActions,
    kWsclientMsgType_QueueUspRecord
} wsclient_msg_type_t;

//------------------------------------------------------------------------------
// Message on the websocket client thread queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure

    wsclient_msg_type_t   msg_type;  // Type of websocket message contained in the union (below)
    union
    {
        start_client_msg_t       start_client_msg;      // Message to start/restart connection to the specified server
        stop_client_msg_t        stop_client_msg;      // Message to stop connection to the specified server
        queue_client_usp_record_msg_t   queue_usp_record_msg;  // Message to queue a USP record to send to the specified server
    } inner;

} wsclient_msg_t;

//------------------------------------------------------------------------------
// Message queue containing messages to send to websockets thread
// NOTE: libwebsockets is not threadsafe, so all calls to it must occur within the websockets thread
//       This message queue is serviced in the websockets thread
static double_linked_list_t wsclient_msg_queue;

//------------------------------------------------------------------------------------
// Mutex used to protect access to this component
static pthread_mutex_t wsc_access_mutex;

//------------------------------------------------------------------------------------
// libwebsockets context for this component
static struct lws_context *wsc_ctx = NULL;

//------------------------------------------------------------------------------------
// Structure passed to libwebsockets defining the websocket USP sub-protocol
static struct lws_protocols wsc_subprotocol[2];      // last entry is NULL terminator

//------------------------------------------------------------------------------
// Flag set to true if the MTP thread has exited
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_wsclient_mtp_thread_exited = false;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ServiceWsclientMessageQueue(void);
void HandleWsclient_StartClient(start_client_msg_t *scm);
void HandleWsclient_ConnParamsChanged(wsclient_t *wc, start_client_msg_t *scm);
void HandleWsclient_StopClient(stop_client_msg_t *tcm);
void AttemptWsclientConnect(wsclient_t *wc);
void HandleWsclient_QueueUspRecord(queue_client_usp_record_msg_t *qur);
void ServiceWsclientSendQueue(void);
int HandleAllWscEvents(struct lws *handle, enum lws_callback_reasons event, void *per_session_data, void *event_args, size_t event_args_len);
int AddWsclientUspExtension(struct lws *handle, unsigned char **ppHeaders, int headers_len);
int ValidateReceivedWsclientSubProtocol(struct lws *handle);
int HandleWscEvent_LoadCerts(SSL_CTX *ssl_ctx);
int HandleWscEvent_VerifyCerts(struct lws *handle, SSL *ssl, int preverify_ok, X509_STORE_CTX *x509_ctx);
int HandleWscEvent_Error(struct lws *handle, char *err_msg);
int HandleWscEvent_Closed(struct lws *handle);
int HandleWscEvent_Destroyed(struct lws *handle);
int HandleWscEvent_Connected(struct lws *handle);
int HandleWscEvent_Receive(struct lws *handle, unsigned char *chunk, int chunk_len);
int HandleWscEvent_Transmit(struct lws *handle);
int HandleWscEvent_PingTimer(struct lws *handle);
int HandleWscEvent_PongReceived(struct lws *handle);
void ScheduleWsclientRetry(wsclient_t *wc);
wsclient_t *FindWsclient_ByInstance(int cont_instance, int mtp_instance);
wsclient_t *FindFreeWsclient(void);
void HandleWscEvent_RetryTimer(lws_sorted_usec_list_t *sul);
void DestroyWsclient(wsclient_t *wc, bool is_libwebsockets_destroyed);
void LogWsclientDebug(int level, const char *line);
void AssertRetryCallbackNotInUse(lws_sorted_usec_list_t *sul);
void ForceWsclientClose(wsclient_t *wc, wsclient_close_reason_t reason);
void GracefullyCloseWsclient(wsclient_t *wc, wsclient_close_reason_t reason);
void HandleWsclient_ActivateScheduledActions(void);
int SendWsclientPing(wsclient_t *wc);
int CloseWsclientConnection(wsclient_t *wc, wsclient_close_reason_t close_reason, int close_status, char *fmt, ...) __attribute__((format(printf, 4, 5)));
bool IsUspRecordInWsclientQueue(wsclient_t *wc, unsigned char *pbuf, int pbuf_len);
void RemoveExpiredWsclientMessages(wsclient_t *wc);
void RemoveWsclientSendItem(wsclient_t *wc, wsclient_send_item_t *si);
bool AreAllWsclientResponsesSent(void);
void PurgeWsclientMessageQueue(void);
void QueueUspConnectRecord_Wsclient(wsclient_t *wc);

// Code to help debug mutex issues
//static int mtx_count = 0;
//void mtx_fail_breakpoint(void)
//{
//}
//
//#define OS_UTILS_LockMutex(x)  USP_LOG_Info("MTX_DGB: %s(%d) waiting to take", __FUNCTION__, __LINE__); OS_UTILS_LockMutex(x); mtx_count++; USP_LOG_Info("MTX_DGB: %s(%d) taken", __FUNCTION__, __LINE__);
//#define OS_UTILS_UnlockMutex(x)   if (mtx_count==0) { mtx_fail_breakpoint(); }  mtx_count--; USP_LOG_Info("MTX_DGB: %s(%d) released", __FUNCTION__, __LINE__); OS_UTILS_UnlockMutex(x);


/*********************************************************************//**
**
** WSCLIENT_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int WSCLIENT_Init(void)
{
    int i;
    int err;
    wsclient_t *wc;

    // Initialise websocket client connection array
    memset(wsclients, 0, sizeof(wsclients));
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        wc->state = kWebsockState_Idle;
    }

    // Set log mask and function to re-direct libwebsockets log output
    #define LIBWEBSOCKETS_LOG_MASK  0  // (LLL_ERR | LLL_WARN | LLL_NOTICE)
    lws_set_log_level(LIBWEBSOCKETS_LOG_MASK, LogWsclientDebug);

    // Initialise message queue to control websocket client thread
    DLLIST_Init(&wsclient_msg_queue);

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&wsc_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WSCLIENT_Destroy
**
** Frees all memory associated with this component and closes all sockets
**
** \param   None
**
** \return  None
**
**************************************************************************/
void WSCLIENT_Destroy(void)
{
    int i;
    wsclient_t *wc;

    // First cancel all retry timers, to prevent our structure containing references to memory which is freed when the libwebsocket context is destroyed
    // NOTE: This is not strictly necessary for WSCLIENT, as this function is only called when shutting down anyway
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if (wc->state == kWebsockState_Retrying)
        {
            lws_sul_cancel(&wc->retry_timer);
        }
    }

    // Destroy the libwebsocket context
    // NOTE: This also frees libwebsockets' SSL context and associated certs
    lws_context_destroy(wsc_ctx);
    wsc_ctx = NULL;

    // Iterate over all websock clients, freeing any memory they're using
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if (wc->state != kWebsockState_Idle)
        {
            DestroyWsclient(wc, true);
        }
    }

    // Remove all messages on the websocket client thread's message queue
    PurgeWsclientMessageQueue();
}

/*********************************************************************//**
**
** WSCLIENT_Start
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
int WSCLIENT_Start(void)
{
    struct lws_protocols *p;
	struct lws_context_creation_info info;

    // Setup USP subprotocol to use
    #define WEBSOCKET_PROTOCOL_STR "v1.usp"
    memset(wsc_subprotocol, 0, sizeof(wsc_subprotocol));
    p = &wsc_subprotocol[0];
    p->name = WEBSOCKET_PROTOCOL_STR;
    p->callback = HandleAllWscEvents;
    p->per_session_data_size = 0;

    #define RX_CHUNK_SIZE MAX_USP_MSG_LEN  // Maximum size of chunks that we want to receive from libwebsockets
    #define TX_CHUNK_SIZE 2*1024           // Maximum size of chunks that we want to send to libwebsockets.
                                           //  NOTE: This needs to be small enough to ensure that large messages are fully sent when shutting down gracefully
    p->rx_buffer_size = RX_CHUNK_SIZE;
    p->tx_packet_size = TX_CHUNK_SIZE;

    // Setup websock context
    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = wsc_subprotocol;
    info.pvo = NULL;
    info.fd_limit_per_thread = 1 + 2*MAX_WEBSOCKET_CLIENTS;  // Multiply by 2 to include extra http2 fds. Plus 1 for libwebsockets internal fds

    // NOTE: The following option must be set for libwebsockets to support SSL.
    // It not only initialises libSSL, but it also enables SSL support in libwebsockets.
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    // Create the websocket context
    // NOTE: libwebsockets calls HandleAllWscEvents() from within lws_create_context() to load the
    // trust store and client certs into libwebsockets SSL context
    wsc_ctx = lws_create_context(&info);
    if (wsc_ctx == NULL)
    {
        USP_LOG_Error("%s: lws_create_context() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WSCLIENT_StartClient
**
** Called to start a WebSocket client connection to the specified server
** NOTE: This function may also be called to schedule a reconnect with changed connection parameters
** NOTE: Caller will have already checked that there are enough websocket client slots for this to succeed
**
** \param   cont_instance - instance number of controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
** \param   cont_endpoint_id - endpoint_id of the controller
** \param   config - connection details of the websockets server to connect to
**
** \return  None
**
**************************************************************************/
void WSCLIENT_StartClient(int cont_instance, int mtp_instance, char *cont_endpoint_id, wsclient_config_t *config)
{
    wsclient_msg_t *msg;
    start_client_msg_t *scm;

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        return;
    }

    // Setup the message to post to the websock thread
    msg = USP_MALLOC(sizeof(wsclient_msg_t));
    memset(msg, 0, sizeof(wsclient_msg_t));
    msg->msg_type = kWsclientMsgType_StartClient;

    scm = &msg->inner.start_client_msg;
    scm->cont_instance = cont_instance;
    scm->mtp_instance = mtp_instance;
    scm->cont_endpoint_id = USP_STRDUP(cont_endpoint_id);
    scm->host = USP_STRDUP(config->host);
    scm->port = config->port;
    scm->path = USP_STRDUP(config->path);
    scm->enable_encryption = config->enable_encryption;
    scm->keep_alive_interval = config->keep_alive_interval;
    scm->retry_interval = config->retry_interval;
    scm->retry_multiplier = config->retry_multiplier;

    // Add the message to the end of the message queue
    OS_UTILS_LockMutex(&wsc_access_mutex);
    DLLIST_LinkToTail(&wsclient_msg_queue, msg);
    OS_UTILS_UnlockMutex(&wsc_access_mutex);

    // Cause the libwebsocket poll() to exit and then read this message
    if (wsc_ctx != NULL)
    {
        lws_cancel_service(wsc_ctx);
    }
}

/*********************************************************************//**
**
** WSCLIENT_StopClient
**
** Stops a currently active WebSocket client connection.
** Called because the MTP has been disabled or deleted
**
** \param   cont_instance - instance number of controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
**
** \return  None
**
**************************************************************************/
void WSCLIENT_StopClient(int cont_instance, int mtp_instance)
{
    wsclient_msg_t *msg;
    stop_client_msg_t *tcm;

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        return;
    }

    // Setup the message to post to the websock thread
    msg = USP_MALLOC(sizeof(wsclient_msg_t));
    memset(msg, 0, sizeof(wsclient_msg_t));
    msg->msg_type = kWsclientMsgType_StopClient;

    tcm = &msg->inner.stop_client_msg;
    tcm->cont_instance = cont_instance;
    tcm->mtp_instance = mtp_instance;

    // Add the message to the end of the message queue
    OS_UTILS_LockMutex(&wsc_access_mutex);
    DLLIST_LinkToTail(&wsclient_msg_queue, msg);
    OS_UTILS_UnlockMutex(&wsc_access_mutex);

    // Cause the libwebsocket poll() to exit and then read this message
    if (wsc_ctx != NULL)
    {
        lws_cancel_service(wsc_ctx);
    }
}

/*********************************************************************//**
**
** WSCLIENT_ActivateScheduledActions
**
** Activates all scheduled actions. If a reconnect or close is scheduled, all USP messages
** that are queued to send will be sent before performing the action
**
** \param   None
**
** \return  None
**
**************************************************************************/
void WSCLIENT_ActivateScheduledActions(void)
{
    wsclient_msg_t *msg;

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        return;
    }

    // Setup the message to post to the websock thread
    msg = USP_MALLOC(sizeof(wsclient_msg_t));
    memset(msg, 0, sizeof(wsclient_msg_t));
    msg->msg_type = kWsclientMsgType_ActivateScheduledActions;

    // Add the message to the end of the message queue
    OS_UTILS_LockMutex(&wsc_access_mutex);
    DLLIST_LinkToTail(&wsclient_msg_queue, msg);

    // Cause the libwebsocket poll() to exit and then read this message
    if (wsc_ctx != NULL)
    {
        lws_cancel_service(wsc_ctx);
    }
    OS_UTILS_UnlockMutex(&wsc_access_mutex);
}

/*********************************************************************//**
**
** WSCLIENT_QueueBinaryMessage
**
** Function called to queue a USP record on the specified WebSocket connection
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is always passed to this function.
** \param   cont_instance - instance number of controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  None
**
**************************************************************************/
void WSCLIENT_QueueBinaryMessage(mtp_send_item_t *msi, int cont_instance, int mtp_instance, time_t expiry_time)
{
    wsclient_msg_t *msg;
    queue_client_usp_record_msg_t *qur;
    USP_ASSERT(msi != NULL);

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        USP_FREE(msi->pbuf);
        return;
    }

    // Setup the message to post to the websock thread
    msg = USP_MALLOC(sizeof(wsclient_msg_t));
    memset(msg, 0, sizeof(wsclient_msg_t));
    msg->msg_type = kWsclientMsgType_QueueUspRecord;

    qur = &msg->inner.queue_usp_record_msg;
    qur->cont_instance = cont_instance;
    qur->mtp_instance = mtp_instance;
    qur->item = *msi;  // NOTE: Ownership of the payload buffer passes to the websock message queue
    qur->expiry_time = expiry_time;

    // Add the message to the end of the message queue
    OS_UTILS_LockMutex(&wsc_access_mutex);
    DLLIST_LinkToTail(&wsclient_msg_queue, msg);
    OS_UTILS_UnlockMutex(&wsc_access_mutex);

    // Cause the libwebsocket poll() to exit and then read this message
    if (wsc_ctx != NULL)
    {
        lws_cancel_service(wsc_ctx);
    }
}

/*********************************************************************//**
**
** WSCLIENT_GetRetryCount
**
** Function called to obtain the retry count for a particular WebSocket connection
**
** \param   cont_instance - instance number of controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
**
** \return  retry count or 0 if no websock connection setup for the specified instances
**
**************************************************************************/
unsigned WSCLIENT_GetRetryCount(int cont_instance, int mtp_instance)
{
    unsigned retry_count;
    wsclient_t *wc;

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        return 0;
    }

    OS_UTILS_LockMutex(&wsc_access_mutex);

    // Exit if unable to determine which connection
    wc = FindWsclient_ByInstance(cont_instance, mtp_instance);
    if (wc == NULL)
    {
        retry_count = 0;
        goto exit;
    }

    retry_count = (unsigned) wc->retry_count;

exit:
    OS_UTILS_UnlockMutex(&wsc_access_mutex);
    return retry_count;
}

/*********************************************************************//**
**
** WSCLIENT_IsEndpointConnected
**
** Determines whether the specified endpoint is currently connected to the Agent's websocket client
**
** \param   endpoint_id - Endpoint ID of the controller which we want to determine if it is connected via the agent's websocket client
**
** \return  true if the endpoint is connected
**
**************************************************************************/
bool WSCLIENT_IsEndpointConnected(char *endpoint_id)
{
    int i;
    wsclient_t *wc;
    bool is_connected;

    // Exit if MTP thread has exited
    if (is_wsclient_mtp_thread_exited)
    {
        return false;
    }

    OS_UTILS_LockMutex(&wsc_access_mutex);

    // Iterate over all websock client connections, finding the one with matching endpoint_id and determining whether it is running
    is_connected = false;
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if ((wc->state == kWebsockState_Running) && (strcmp(wc->cont_endpoint_id, endpoint_id)==0))
        {
            is_connected = true;
            goto exit;
        }
    }

exit:
    OS_UTILS_UnlockMutex(&wsc_access_mutex);
    return is_connected;
}

/*********************************************************************//**
**
** WSCLIENT_Main
**
** Main loop of MTP thread for agent acting as WebSocket client
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *WSCLIENT_Main(void *args)
{
    int err;

    while (FOREVER)
    {
        // Service the websocket client thread message queue
        OS_UTILS_LockMutex(&wsc_access_mutex);
        ServiceWsclientMessageQueue();
        ServiceWsclientSendQueue();

        // Exit this thread, if an exit is scheduled and all responses have been sent
        if (mtp_exit_scheduled == kScheduledAction_Activated)
        {
            if (AreAllWsclientResponsesSent())
            {
                // Free all memory associated with MTP layer and the libwebsockets context
                WSCLIENT_Destroy();

                // Prevent the data model from making any other changes to the MTP thread
                is_wsclient_mtp_thread_exited = true;

                // Signal the data model thread that this thread has exited
                OS_UTILS_UnlockMutex(&wsc_access_mutex);
                DM_EXEC_PostMtpThreadExited(WSCLIENT_EXITED);
                goto exit;
            }
        }

        OS_UTILS_UnlockMutex(&wsc_access_mutex);

        // Allow libwebsockets to wait for socket activity
        // NOTE: If there is any activity, HandleAllWscEvents() will be called to process it
        err = lws_service(wsc_ctx, 0);
        if (err != 0)
        {
            // NOTE: The code should never get here, but if it does, handle it by logging the error and sleeping for a while (to prevent the thread hogging the processor)
            #define MILLISECONDS 1000  // number of micro seconds (us) in a millisecond
            USP_LOG_Warning("%s: lws_service() returned %d", __FUNCTION__, err);
            usleep(100*MILLISECONDS);
        }
    }

exit:
    return NULL;
}

/*********************************************************************//**
**
** ServiceWsclientMessageQueue
**
** Services the websocket client thread's message queue
**
** \param   None
**
** \return  None
**
**************************************************************************/
void ServiceWsclientMessageQueue(void)
{
    wsclient_msg_t *msg;
    start_client_msg_t *scm;
    stop_client_msg_t *tcm;
    queue_client_usp_record_msg_t *qur;

    // Exit if no messages to process
    msg = (wsclient_msg_t *) wsclient_msg_queue.head;
    if (msg == NULL)
    {
        return;
    }

    // Process the message
    switch(msg->msg_type)
    {
        case kWsclientMsgType_StartClient:
            scm = &msg->inner.start_client_msg;
            HandleWsclient_StartClient(scm);
            break;

        case kWsclientMsgType_StopClient:
            tcm = &msg->inner.stop_client_msg;
            HandleWsclient_StopClient(tcm);
            break;

        case kWsclientMsgType_ActivateScheduledActions:
            HandleWsclient_ActivateScheduledActions();
            break;

        case kWsclientMsgType_QueueUspRecord:
            qur = &msg->inner.queue_usp_record_msg;
            HandleWsclient_QueueUspRecord(qur);
            break;

        default:
            break;
    }

    // Remove the message from the queue and free it, since we have processed it
    // NOTE: No need to free message structure members. They are consumed by the relevant handler
    DLLIST_Unlink(&wsclient_msg_queue, msg);
    USP_FREE(msg);
}

/*********************************************************************//**
**
** HandleWsclient_StartClient
**
** Processes a kWsclientMsgType_StartClient message, by starting a websocket connection
** NOTE: This function takes ownership (or frees) all dynamically allocated members of the message
**
** \param   scm - pointer to structure containing web socket client connection parameters
**
** \return  None
**
**************************************************************************/
void HandleWsclient_StartClient(start_client_msg_t *scm)
{
    wsclient_t *wc;

    wc = FindWsclient_ByInstance(scm->cont_instance, scm->mtp_instance);
    if (wc != NULL)
    {
        // Handle change in connection parameters then exit
        // NOTE: HandleWsclient_ConnParamsChanged() takes ownership of host, path and cont_endpoint_id from the message
        HandleWsclient_ConnParamsChanged(wc, scm);
        return;
    }

    // Exit if no free clients. In this case, the start client message is ignored
    wc = FindFreeWsclient();
    if (wc == NULL)
    {
        USP_LOG_Error("%s: No free WebSocket client slots", __FUNCTION__);
        USP_SAFE_FREE(scm->cont_endpoint_id);
        USP_SAFE_FREE(scm->host);
        USP_SAFE_FREE(scm->path);
        return;
    }

    // Fill in the websocket client structure
    // NOTE: This function takes ownership of host, path and cont_endpoint_id from the message
    memset(wc, 0, sizeof(wsclient_t));
    wc->state = kWebsockState_AwaitingConnected;
    wc->cont_instance = scm->cont_instance;
    wc->mtp_instance = scm->mtp_instance;
    wc->cont_endpoint_id = scm->cont_endpoint_id;
    wc->host = scm->host;
    wc->port = scm->port;
    wc->path = scm->path;
    wc->enable_encryption = scm->enable_encryption;
    wc->keep_alive_interval = scm->keep_alive_interval;
    wc->retry_interval = scm->retry_interval;
    wc->retry_multiplier = scm->retry_multiplier;
    wc->retry_count = 0;
    wc->send_ping = false;
    wc->ping_count = 0;
    DLLIST_Init(&wc->usp_record_send_queue);
    wc->ws_handle = NULL;
    memset(&wc->retry_timer, 0, sizeof(wc->retry_timer));
    wc->close_reason = kWebSockCloseReason_Unknown;
    wc->disconnect_sent = false;
    wc->schedule_reconnect = kScheduledAction_Off;
    wc->schedule_close = kScheduledAction_Off;
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;
    wc->tx_index = 0;
    wc->cert_chain = NULL;
    wc->role = (scm->enable_encryption) ? ROLE_NON_SSL : ROLE_DEFAULT;

    // Start the connection. If an error occurs, handle by putting into retry state
    AttemptWsclientConnect(wc);
}

/*********************************************************************//**
**
** HandleWsclient_StopClient
**
** Processes a kWsclientMsgType_StopClient message, by starting a websocket connection
** NOTE: This function takes ownership (or frees) all dynamically allocated members of the message
**
** \param   tcm - pointer to structure identifying the client to stop
**
** \return  None
**
**************************************************************************/
void HandleWsclient_StopClient(stop_client_msg_t *tcm)
{
    wsclient_t *wc;

    // Exit if unable to find the client
    // NOTE: This should never occur
    wc = FindWsclient_ByInstance(tcm->cont_instance, tcm->mtp_instance);
    if (wc == NULL)
    {
        return;
    }

    // Determine whether to close immediately, or whether we have to wait for enqueued messages
    // to be sent or libwebsockets to close
    switch(wc->state)
    {
        case kWebsockState_AwaitingConnected:
            // If currently connecting, we can ask libwebsockets to close the connection immediately
            ForceWsclientClose(wc, kWebSockCloseReason_GracefulClose);
            break;

        case kWebsockState_Running:
            // If currently running, then we need to wait for the action to be scheduled,
            // as there may be responses that need to be queued and sent before closing the connection
            wc->schedule_close = kScheduledAction_Signalled;
            break;

        case kWebsockState_Retrying:
            // If currently retrying, just cancel the retry callback. The connection is already closed.
            lws_sul_schedule(wsc_ctx, 0, &wc->retry_timer, HandleWscEvent_RetryTimer, LWS_SET_TIMER_USEC_CANCEL);
            DestroyWsclient(wc, false);
            break;

        default:
            TERMINATE_BAD_CASE(wc->state);
            break;
    }
}

/*********************************************************************//**
**
** HandleWsclient_ActivateScheduledActions
**
** Called after enqueueing all USP response messages to inform the MTP that it can perform the
** scheduled action once all enqueued USP messages have been sent
**
** \param   None
**
** \return  None
**
**************************************************************************/
void HandleWsclient_ActivateScheduledActions(void)
{
    int i;
    wsclient_t *wc;

    // Mark all signalled actions as activated. This will wait for all queued messages to be sent before performing the action
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if (wc->state != kWebsockState_Idle)
        {
            if (wc->schedule_reconnect == kScheduledAction_Signalled)
            {
                wc->schedule_reconnect = kScheduledAction_Activated;
            }

            if (wc->schedule_close == kScheduledAction_Signalled)
            {
                wc->schedule_close = kScheduledAction_Activated;
            }
        }
    }
}

/*********************************************************************//**
**
** HandleWsclient_QueueUspRecord
**
** Processes a kWsclientMsgType_QueueUspRecord, by adding the record to the send queue
** NOTE: This function takes ownership (or frees) all dynamically allocated members of the message
**
** \param   qur - pointer to structure containing USP record to queue for sending
**
** \return  None
**
**************************************************************************/
void HandleWsclient_QueueUspRecord(queue_client_usp_record_msg_t *qur)
{
    wsclient_t *wc;
    wsclient_send_item_t *si;
    int err;
    bool is_duplicate;

    // Exit if no client connection started
    wc = FindWsclient_ByInstance(qur->cont_instance, qur->mtp_instance);
    if (wc == NULL)
    {
        USP_LOG_Error("%s: Discarding USP message to send to controller.%d.MTP.%d", __FUNCTION__, qur->cont_instance, qur->mtp_instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Remove all USP notifications from the send queue, that have expired
    RemoveExpiredWsclientMessages(wc);

    // Do not add this message to the queue, if it is already present in the queue
    // This situation could occur if a notify is being retried to be sent, but is already held up in the queue pending sending
    is_duplicate = IsUspRecordInWsclientQueue(wc, qur->item.pbuf, qur->item.pbuf_len);
    if (is_duplicate)
    {
        err = USP_ERR_OK;
        USP_FREE(qur->item.pbuf);
        goto exit;
    }

    // Add USP Record to queue
    si = USP_MALLOC(sizeof(wsclient_send_item_t));
    memset(si, 0, sizeof(wsclient_send_item_t));
    si->item = qur->item;
    si->expiry_time = qur->expiry_time;
    DLLIST_LinkToTail(&wc->usp_record_send_queue, si);
    err = USP_ERR_OK;

exit:
    // Free dynamically allocated members of the message, if their ownership was not transferred
    if (err != USP_ERR_OK)
    {
        USP_FREE(qur->item.pbuf);
    }
}

/*********************************************************************//**
**
** HandleWsclient_ConnParamsChanged
**
** Handles the case of the websock connection parameters have changed for a connection which is already active
** NOTE: This function takes ownership (or frees) all dynamically allocated members of the message
**
** \param   wc - pointer to websocket client connection to restart the connection on
** \param   scm - pointer to structure containing web socket client connection parameters
**
** \return  None
**
**************************************************************************/
void HandleWsclient_ConnParamsChanged(wsclient_t *wc, start_client_msg_t *scm)
{
    USP_LOG_Info("%s: Client connection parameters changed for controller.%d.MTP.%d", __FUNCTION__, scm->cont_instance, scm->mtp_instance);

    // Copy new retry algorithm parameters. They will be used, the next time a retry is triggered
    wc->retry_interval = scm->retry_interval;
    wc->retry_multiplier = scm->retry_multiplier;

    // If keep alive interval has changed, then cause the websock ping frame to be sent immediately (so that the new interval is respected immediately)
    if (scm->keep_alive_interval != wc->keep_alive_interval)
    {
        wc->keep_alive_interval = scm->keep_alive_interval;
        lws_set_timer_usecs(wc->ws_handle, 0);
    }

    // Copy a changed endpoint_id. This will be used in subsequent debug messages
    // NOTE: This function takes ownership of cont_endpoint_id from the message
    if (strcmp(scm->cont_endpoint_id, wc->cont_endpoint_id)!=0)
    {
        USP_SAFE_FREE(wc->cont_endpoint_id);
        wc->cont_endpoint_id = scm->cont_endpoint_id;
    }
    else
    {
        USP_SAFE_FREE(scm->cont_endpoint_id);
    }

    // Exit if none of the connection parameters have changed
    if ((strcmp(scm->host, wc->host)==0) && (scm->port == wc->port) && (strcmp(scm->path, wc->path)==0))
    {
        USP_SAFE_FREE(scm->host);
        USP_SAFE_FREE(scm->path);
        return;
    }

    // If the code gets here, then the connection parameters have changed, so copy the new params over
    // NOTE: This function takes ownership of host and path from the message
    USP_SAFE_FREE(wc->host);
    USP_SAFE_FREE(wc->path);
    wc->host = scm->host;
    wc->path = scm->path;
    wc->port = scm->port;

    // Determine whether to reconnect immediately, or whether we have to wait for enqueued messages
    // to be sent or libwebsockets to close
    switch(wc->state)
    {
        case kWebsockState_AwaitingConnected:
            // If currently connecting, we need to wait for libwebsockets to close the connection first
            ForceWsclientClose(wc, kWebSockCloseReason_ConnParamsChanged);
            break;

        case kWebsockState_Running:
            // If currently running, then we need to wait for the action to be scheduled,
            // as there may be responses that need to be queued and sent before restarting the connection
            wc->schedule_reconnect = kScheduledAction_Signalled;
            break;

        case kWebsockState_Retrying:
            // If currently retrying, immediately retry with the new connection params, ensuring we cancel the retry callback first
            lws_sul_schedule(wsc_ctx, 0, &wc->retry_timer, HandleWscEvent_RetryTimer, LWS_SET_TIMER_USEC_CANCEL);
            wc->retry_count = 0;
            AttemptWsclientConnect(wc);
            break;

        default:
            TERMINATE_BAD_CASE(wc->state);
            break;
    }
}

/*********************************************************************//**
**
** AttemptWsclientConnect
**
** Attempt to connect on the specified websock client
** NOTE: This function handles any errors by scheduling a retry
**
** \param   wc - pointer to websocket client connection to start the connection on
**
** \return  None
**
**************************************************************************/
void AttemptWsclientConnect(wsclient_t *wc)
{
    struct lws_client_connect_info info;
    char *encrypt_str;
    char *interface = "any";
    char wan_addr[NU_IPADDRSTRLEN];     // scope of wan_addr[] needs to exist until call to lws_client_connect_via_info()
    struct lws *handle;

    // Check that structure is not part of a linked list owned by libwebsockets anymore
    AssertRetryCallbackNotInUse(&wc->retry_timer);
    memset(&wc->retry_timer, 0, sizeof(wc->retry_timer));

    // Initialize connection parameters passed to libwebsockets
    memset(&info, 0, sizeof(info));
    info.context = wsc_ctx;
    info.port = wc->port;
    info.path = wc->path;
    info.address = wc->host;
    info.host = wc->host;       // Needed for TLS
    info.origin = wc->host;     // Needed for TLS
    info.pwsi = &wc->ws_handle;     // Needed for TLS
    info.ssl_connection = (wc->enable_encryption) ? LCCSCF_USE_SSL : 0;
    info.protocol = WEBSOCKET_PROTOCOL_STR;
    info.opaque_user_data = wc;

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
{
    int err;
    bool prefer_ipv6;

    // Get preference for IPv4 or IPv6 WAN address (in case of Dual Stack CPE)
    prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

    // Exit if unable to get current IP address for WAN interface
    interface = nu_macaddr_wan_ifname();
    err = tw_ulib_get_dev_ipaddr(interface, wan_addr, sizeof(wan_addr), prefer_ipv6);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to get IP address of WAN interface (%s). Retrying later", __FUNCTION__, interface);
        ScheduleWsclientRetry(wc);
        return;
    }

    // NOTE: Even though libwebsockets documentation states that you can use an interface name here,
    // this does not work when libwebsockets is compiled with IPv6 support. It needs the IP address of the interface instead.
    info.iface = wan_addr;
}
#else
    (void)wan_addr;
#endif

    // Store state of connection in websock client structure
    wc->state = kWebsockState_AwaitingConnected;
    wc->close_reason = kWebSockCloseReason_Unknown;
    wc->disconnect_sent = false;
    wc->schedule_reconnect = kScheduledAction_Off;
    wc->schedule_close = kScheduledAction_Off;
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;

    encrypt_str = (wc->enable_encryption) ? "encrypted" : "unencrypted";
    USP_LOG_Info("Attempting to connect to host=%s (port=%d, %s, path=%s) from interface=%s", wc->host, wc->port, encrypt_str, wc->path, interface);

    // Since lws_client_connect_via_info() can block, release the mutex, so that the data model can post
    // messages to this thread's message queue whilst the connect is taking place
    OS_UTILS_UnlockMutex(&wsc_access_mutex);
    handle = lws_client_connect_via_info(&info);
    OS_UTILS_LockMutex(&wsc_access_mutex);

    // Exit (retrying later) if libwebsockets refused to attempt to connect to the specified server
    // NOTE: There is no need to specifically deallocate this handle when retrying, as libwebsockets will have already done it automatically
    wc->ws_handle = handle;
    if (wc->ws_handle == NULL)
    {
        // NOTE: No need to call ScheduleWsclientRetry() here, as it will have been handled by the LWS_CALLBACK_WSI_DESTROY event within the lws_client_connect_via_info() call
        USP_LOG_Error("%s: lws_client_connect_via_info() failed", __FUNCTION__);
        USP_ASSERT(wc->state == kWebsockState_Retrying);
        return;
    }
}

/*********************************************************************//**
**
** ForceWsclientClose
**
** Schedules the specified connection being closed without sending a WebSockets close frame (just closing the TCP connection)
**
** \param   wc - pointer to websocket client connection to close
** \param   reason - reason for closing the connection
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
void ForceWsclientClose(wsclient_t *wc, wsclient_close_reason_t reason)
{
    // Exit if we're already closing for another reason (nothing to do)
    if (wc->close_reason != kWebSockCloseReason_Unknown)
    {
        return;
    }

    // Save the reason for closing the connection.
    // This is needed so that we can take the appropriate action after the connection has closed
    wc->close_reason = reason;

    // Libwebsockets doesn't allow us to synchronously force close the TCP connection
    // We have to emulate it by forcing a timeout and waiting for the LWS_CALLBACK_WSI_DESTROY event
    lws_set_timeout(wc->ws_handle, PENDING_TIMEOUT_USER_OK, LWS_TO_KILL_ASYNC);
}

/*********************************************************************//**
**
** GracefullyCloseWsclient
**
** Schedules the specified connection being closed gracefully by sending a WebSockets close frame
**
** \param   wc - pointer to websocket client connection to close
** \param   reason - reason for closing the connection
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
void GracefullyCloseWsclient(wsclient_t *wc, wsclient_close_reason_t reason)
{
    int err;

    // Exit if we're already closing for another reason (nothing to do)
    if (wc->close_reason != kWebSockCloseReason_Unknown)
    {
        return;
    }

    // Save the reason for closing the connection.
    // This is needed so that we can take the appropriate action after the connection has closed
    wc->close_reason = reason;

    // Libwebsockets doesn't allow us to synchronously send a close frame on the connection
    // Instead we have to call lws_close_reason() when attempting to write to the connection, and return an error from LWS_CALLBACK_CLIENT_WRITEABLE
    err = lws_callback_on_writable(wc->ws_handle);
    if (err != 1)
    {
        // If an error occurred, then force close the TCP connection
        USP_LOG_Error("%s: lws_callback_on_writable() returned %d", __FUNCTION__, err);
        lws_set_timeout(wc->ws_handle, PENDING_TIMEOUT_USER_OK, LWS_TO_KILL_ASYNC);
    }
}

/*********************************************************************//**
**
** ServiceWsclientSendQueue
**
** Informs libwebsockets of clients which have USP Record data to send
**
** \param   None
**
** \return
**
**************************************************************************/
void ServiceWsclientSendQueue(void)
{
    int i;
    wsclient_t *wc;
    int err;

    // Iterate over all clients, finding those which have data to send
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if (wc->state == kWebsockState_Running)
        {
            // Start closing the connection if it was scheduled to happen once the USP message queue was exhausted
            if (wc->usp_record_send_queue.head == NULL)
            {
                if (wc->close_reason == kWebSockCloseReason_Unknown)
                {
                    if (wc->disconnect_sent)
                    {
                        GracefullyCloseWsclient(wc, kWebSockCloseReason_BadUspRecord);
                    }
                    else if (wc->schedule_reconnect == kScheduledAction_Activated)
                    {
                        GracefullyCloseWsclient(wc, kWebSockCloseReason_ConnParamsChanged);
                    }
                    else if ((wc->schedule_close == kScheduledAction_Activated) || (mtp_exit_scheduled == kScheduledAction_Activated))
                    {
                        GracefullyCloseWsclient(wc, kWebSockCloseReason_GracefulClose);
                    }

                }
            }
            else
            {
                // If the code gets here, we have data to send, so ask libwebsockets for permission to send
                USP_ASSERT(wc->ws_handle != NULL);
                err = lws_callback_on_writable(wc->ws_handle);
                if (err != 1)
                {
                    // If an error occurred, then close the TCP connection and retry
                    USP_LOG_Error("%s: lws_callback_on_writable() returned %d", __FUNCTION__, err);
                    ForceWsclientClose(wc, kWebSockCloseReason_InternalError);
                }
            }
        }
    }
}

/*********************************************************************//**
**
** HandleAllWscEvents
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
int HandleAllWscEvents(struct lws *handle, enum lws_callback_reasons event, void *per_session_data, void *event_args, size_t event_args_len)
{
    int result = 0;

    OS_UTILS_LockMutex(&wsc_access_mutex);

    // Process the event
    #define tr_event(...)    //USP_LOG_Debug(__VA_ARGS__) // uncomment the macro definition to get event debug
    switch (event)
    {
        case LWS_CALLBACK_WSI_DESTROY:
            tr_event("WS client: LWS_CALLBACK_WSI_DESTROY");
            result = HandleWscEvent_Destroyed(handle);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            tr_event("WS client: LWS_CALLBACK_CLIENT_CONNECTION_ERROR");
            result = HandleWscEvent_Error(handle, event_args);
            break;

        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            tr_event("WS client: LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER");
            result = AddWsclientUspExtension(handle, (unsigned char **)event_args, event_args_len);
		    break;

        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
            tr_event("WS client: LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH");
            result = ValidateReceivedWsclientSubProtocol(handle);
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            tr_event("WS client: LWS_CALLBACK_CLIENT_ESTABLISHED");
            result = HandleWscEvent_Connected(handle);
		    break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            tr_event("WS client: LWS_CALLBACK_CLIENT_CLOSED");
            result = HandleWscEvent_Closed(handle);
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            tr_event("WS client: LWS_CALLBACK_CLIENT_RECEIVE");
            result = HandleWscEvent_Receive(handle, event_args, event_args_len);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            tr_event("WS client: LWS_CALLBACK_CLIENT_WRITEABLE");
            result = HandleWscEvent_Transmit(handle);
            break;

        case LWS_CALLBACK_TIMER:
            tr_event("WS client: LWS_CALLBACK_TIMER");
            result = HandleWscEvent_PingTimer(handle);
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
            tr_event("WS client: LWS_CALLBACK_CLIENT_RECEIVE_PONG");
            result = HandleWscEvent_PongReceived(handle);
            break;

        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
            tr_event("WS client: LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS");
            result = HandleWscEvent_LoadCerts(per_session_data);
            break;

        case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
            tr_event("WS client: LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION");
            result = HandleWscEvent_VerifyCerts(handle, event_args, event_args_len, per_session_data);
            break;

        default:
            tr_event("WS client: event=%d", __FUNCTION__, event);
            break;
    }

    OS_UTILS_UnlockMutex(&wsc_access_mutex);

    return result;
}

/*********************************************************************//**
**
** HandleWscEvent_LoadCerts
**
** Called from the LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS event
** to load the trust store and client certs into libwebsockets SSL context
** when libwebsockets is creating a libwebsockets context in lws_create_context()
**
** \param   ssl_ctx - libwebsockets handle identifying the connection with activity on it
** \param   err_msg - pointer to string identifying the error, or NULL if no error message provided
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWscEvent_LoadCerts(SSL_CTX *ssl_ctx)
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
** HandleWscEvent_VerifyCerts
**
** Called from the LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION event
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
int HandleWscEvent_VerifyCerts(struct lws *handle, SSL *ssl, int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    int verify_ok;
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Validate this CA cert, and save the cert chain (if not already saved by a previous invocation to this function for another cert in the chain)
    verify_ok = DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(preverify_ok, x509_ctx, &wc->cert_chain);
    if (verify_ok==0)
    {
        return 1;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_Error
**
** Called from the LWS_CALLBACK_CLIENT_CONNECTION_ERROR event
** when libwebsockets has encountered an error
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   err_msg - pointer to string identifying the error, or NULL if no error message provided
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWscEvent_Error(struct lws *handle, char *err_msg)
{
    wsclient_t *wc;

    // Handle case of error message being unknown
    if (err_msg == NULL)
    {
        err_msg = "Unknown";
    }

    // Log the cause of connection failure
    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);
    USP_LOG_Error("%s: Error (%s) occurred on websocket connection to endpoint_id=%s (host=%s)", __FUNCTION__, err_msg, wc->cont_endpoint_id, wc->host);

    // Set the root cause of error, if none set yet
    if (wc->close_reason == kWebSockCloseReason_Unknown)
    {
        wc->close_reason = kWebSockCloseReason_LibWebSocketsError;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_Closed
**
** Called from the LWS_CALLBACK_CLIENT_CLOSED event
** after the connection has been closed.
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWscEvent_Closed(struct lws *handle)
{
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Set the root cause of error, if none set yet
    if (wc->close_reason == kWebSockCloseReason_Unknown)
    {
        USP_LOG_Error("%s: Server closed websocket connection to endpoint_id=%s (host=%s)", __FUNCTION__, wc->cont_endpoint_id, wc->host);
        wc->close_reason = kWebSockCloseReason_ServerDisconnect;
    }

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_Destroyed
**
** Called from the LWS_CALLBACK_WSI_DESTROY event
** after the remote server closes the connection, or we have force closed the connection
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
int HandleWscEvent_Destroyed(struct lws *handle)
{
    wsclient_t *wc;

    // Exit if nothing to free. This can occur if the connection timed out during SSL handshake
    wc = lws_get_opaque_user_data(handle);
    if (wc == NULL)
    {
        return 0;
    }

    USP_LOG_Info("%s: Closed connection to %s", __FUNCTION__, wc->cont_endpoint_id);

    // Free any partially received USP Record
    USP_SAFE_FREE(wc->rx_buf);
    wc->rx_buf = NULL;
    wc->rx_buf_len = 0;
    wc->rx_buf_max_len = 0;

    // Free any saved cert chain
    if (wc->cert_chain != NULL)
    {
        sk_X509_pop_free(wc->cert_chain, X509_free);
        wc->cert_chain = NULL;
    }

    // Handle the client being destroyed
    switch(wc->close_reason)
    {
        case kWebSockCloseReason_GracefulClose:
            DestroyWsclient(wc, false);
            break;

        case kWebSockCloseReason_ConnParamsChanged:
            AttemptWsclientConnect(wc);
            break;

        default:
        case kWebSockCloseReason_Unknown:
        case kWebSockCloseReason_ServerDisconnect:
        case kWebSockCloseReason_LibWebSocketsError:
        case kWebSockCloseReason_InternalError:
        case kWebSockCloseReason_BadUspRecord:
        case kWebSockCloseReason_MissingPong:
            ScheduleWsclientRetry(wc);
            break;

    }

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_RetryTimer
**
** Called directly from libwebsockets as part of scheduled timer
** when it is time to retry connecting to the server
**
** \param   sul - pointer to stucture containing the scheduled timer
**
** \return  None
**
**************************************************************************/
void HandleWscEvent_RetryTimer(lws_sorted_usec_list_t *sul)
{
    wsclient_t *wc;

    OS_UTILS_LockMutex(&wsc_access_mutex);
    // Check that structure is not part of a linked list owned by libwebsockets anymore
    AssertRetryCallbackNotInUse(sul);

    // Calculate the websock client containing the specified sul structure
    wc = lws_container_of(sul, wsclient_t, retry_timer);

    // Attempt the reconnect
    AttemptWsclientConnect(wc);

    OS_UTILS_UnlockMutex(&wsc_access_mutex);
}

/*********************************************************************//**
**
** HandleWscEvent_PingTimer
**
** Called from the LWS_CALLBACK_TIMER event
** when it is time to send a websocket ping frame
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWscEvent_PingTimer(struct lws *handle)
{
    int err;
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit if failed to ask libwebsockets for permission to send the ping
    err = lws_callback_on_writable(handle);
    if (err != 1)
    {
        // If an error occurred, then close the TCP connection and retry
        USP_LOG_Error("%s: lws_callback_on_writable() returned %d", __FUNCTION__, err);
        ForceWsclientClose(wc, kWebSockCloseReason_InternalError);
        return -1;
    }

    // Indicate that the next LWS_CALLBACK_CLIENT_WRITEABLE event should send a ping frame
    wc->send_ping = true;

    // Rearm the timer for sending the next websocket PING frame
    lws_set_timer_usecs(handle, wc->keep_alive_interval * LWS_USEC_PER_SEC);

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_PongReceived
**
** Called from the LWS_CALLBACK_CLIENT_RECEIVE_PONG event
** to process a received PONG record
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWscEvent_PongReceived(struct lws *handle)
{
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Reset the count of consecutive pings sent without any pongs
    USP_LOG_Debug("%s: Received PONG at time %d", __FUNCTION__, (int)time(NULL));
    wc->ping_count =0;

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_Connected
**
** Called from the LWS_CALLBACK_CLIENT_ESTABLISHED event
** when the websocket handshake has completed successfully
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWscEvent_Connected(struct lws *handle)
{
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);
    USP_ASSERT(wc->state == kWebsockState_AwaitingConnected);
    wc->state = kWebsockState_Running;
    wc->retry_count = 0;

    USP_LOG_Info("%s: Connected to %s", __FUNCTION__, wc->cont_endpoint_id);

    // If we have a certificate chain, then determine which role to allow for the controller on the Websocket connection
    if (wc->cert_chain != NULL)
    {
        // NOTE: Ignoring any error returned by DEVICE_SECURITY_GetControllerTrust() - just leave the role to the default set in HandleWsclient_StartClient
        DEVICE_SECURITY_GetControllerTrust(wc->cert_chain, &wc->role);

        // Free the saved cert chain as we don't need it anymore
        sk_X509_pop_free(wc->cert_chain, X509_free);
        wc->cert_chain = NULL;
    }

    // Set a timer to expire when it is time to send the websocket PING frame
    lws_set_timer_usecs(handle, wc->keep_alive_interval * LWS_USEC_PER_SEC);

    // Ensure a USP Connect record is sent
    QueueUspConnectRecord_Wsclient(wc);

    return 0;
}

/*********************************************************************//**
**
** HandleWscEvent_Receive
**
** Called from the LWS_CALLBACK_CLIENT_RECEIVE event
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
int HandleWscEvent_Receive(struct lws *handle, unsigned char *chunk, int chunk_len)
{
    mtp_reply_to_t mtp_reply_to;
    char buf[MAX_ISO8601_LEN];
    wsclient_t *wc;
    int new_len;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // No need to receive a PONG packet, if we received some other packet signalling keep alive
    wc->ping_count = 0;

    // Exit, sending a close frame if we received a text frame, instead of a binary frame
    if (!lws_frame_is_binary(handle))
    {
        return CloseWsclientConnection(wc, kWebSockCloseReason_BadUspRecord, LWS_CLOSE_STATUS_INVALID_PAYLOAD,
                                      "%s: Text frame received from %s. Expecting binary frame.", __FUNCTION__, wc->cont_endpoint_id);
    }

    // Calculate the total size needed for all websocket fragments received so far (including this one)
    // NOTE: We may only have received a chunk of the current websocket fragment - lws_remaining_packet_payload() returns the number of bytes remaining in the current websocket fragment
    new_len = wc->rx_buf_len + chunk_len + lws_remaining_packet_payload(handle);

    // Exit, sending a close frame, if the size of the USP Record is larger than we allow
    if (new_len > MAX_USP_MSG_LEN)
    {
        return CloseWsclientConnection(wc, kWebSockCloseReason_BadUspRecord, LWS_CLOSE_STATUS_MESSAGE_TOO_LARGE,
                                      "%s: %s sent a message >%d bytes long (%d bytes). Closing connection.", __FUNCTION__, wc->cont_endpoint_id, MAX_USP_MSG_LEN, new_len);
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
    USP_LOG_Info("Message received at time %s, from host %s over WebSockets", buf, wc->host);

    // Send the USP Record to the data model thread for processing
    // NOTE: Ownership of receive buffer stays with this thread
    memset(&mtp_reply_to, 0, sizeof(mtp_reply_to));
    mtp_reply_to.is_reply_to_specified = true;
    mtp_reply_to.protocol = kMtpProtocol_WebSockets;
    mtp_reply_to.wsclient_cont_instance = wc->cont_instance;
    mtp_reply_to.wsclient_mtp_instance = wc->mtp_instance;
    mtp_reply_to.wsserv_conn_id = INVALID;
    mtp_reply_to.cont_endpoint_id = wc->cont_endpoint_id;
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
** HandleWscEvent_Transmit
**
** Called from the LWS_CALLBACK_CLIENT_WRITEABLE event
** to send a USP record (if one is queued to send) or send a PING frame (if it is time to do so)
** or to gracefully close the connection
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int HandleWscEvent_Transmit(struct lws *handle)
{
    wsclient_t *wc;
    wsclient_send_item_t *si;
    int bytes_written;
    int bytes_remaining;
    int chunk_len;
    unsigned char tx_buf[LWS_PRE + TX_CHUNK_SIZE];
    unsigned write_flags;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit, sending a Ping frame, if is time to do so
    // NOTE: Its is safe to send a ping frame whilst in the middle of sending the chunks of a USP Record
    // (because each chunk is sent as a separate Binary or Continuation frame)
    if (wc->send_ping)
    {
        return SendWsclientPing(wc);
    }

    // Exit if no item to send
    // NOTE: It is possible for this to occur under normal circumstances
    si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    if (si == NULL)
    {
        // Exit, sending a websocket Close frame if gracefully closing the websocket connection after all responses have been sent
        if (wc->close_reason != kWebSockCloseReason_Unknown)
        {
            lws_close_reason(wc->ws_handle, LWS_CLOSE_STATUS_GOINGAWAY, NULL, 0);
            return -1;      // Returning an error is what forces libwebsockets to send the close frame
        }

        return 0;
    }

    // Exit if all of the USP records in the send queue were notifications that have expired
    RemoveExpiredWsclientMessages(wc);
    si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    if (si == NULL)
    {
        return 0;
    }

    // If the code gets here, then we have a USP Record which we'd like to send

    // Determine whether we are sending the first chunk
    if (wc->tx_index == 0)
    {
        // Log the USP Record before we send the first chunk
        MSG_HANDLER_LogMessageToSend(&si->item, kMtpProtocol_WebSockets, wc->host, NULL);
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

    // Exit if unable to send the chunk, restarting the connection
    // NOTE: We do not expect this call to fail, as libwebsockets buffers the data if you try to send too much
    bytes_written = lws_write(handle, &tx_buf[LWS_PRE], chunk_len, write_flags);
    if (bytes_written < chunk_len)
    {
        int len;
        char buf[128];

        if (wc->close_reason == kWebSockCloseReason_Unknown)
        {
            wc->close_reason = kWebSockCloseReason_InternalError;
        }
        len = USP_SNPRINTF(buf, sizeof(buf), "%s: lws_write wrote only %d bytes (wanted %d bytes)", __FUNCTION__, bytes_written, si->item.pbuf_len);
        USP_LOG_Error("%s", buf);
        lws_close_reason(wc->ws_handle, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, (unsigned char *)buf, len);
        return -1;
    }

    // If this is a disconnect record, schedule the websocket close and reconnect after the disconnect record has been sent
    // NOTE: USP Disconnect records closing an E2E session (kMtpContentType_E2E_SessTermination) do not close the connection
    if (si->item.content_type == kMtpContentType_DisconnectRecord)
    {
        wc->disconnect_sent = true;
    }

    // Free the USP Record from the queue, if all chunks of it have been sent
    wc->tx_index += chunk_len;
    if (wc->tx_index >= si->item.pbuf_len)
    {
        RemoveWsclientSendItem(wc, si);
        wc->tx_index = 0;
    }

    return 0;
}

/*********************************************************************//**
**
** QueueUspConnectRecord_Wsclient
**
** Adds the USP connect record at the front of the queue, ensuring that there is only one connect record in the queue
**
** \param   wc - pointer to websocket client connection to send the connect record to
**
** \return  None
**
**************************************************************************/
void QueueUspConnectRecord_Wsclient(wsclient_t *wc)
{
    wsclient_send_item_t *cur_msg;
    wsclient_send_item_t *next_msg;
    wsclient_send_item_t *send_item;
    mtp_content_type_t type;

    // Iterate over USP Records in the queue, removing all stale connect and disconnect records
    // A connect or disconnect record may still be in the queue if the connection failed before the record was fully sent
    cur_msg = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    while (cur_msg != NULL)
    {
        // Save pointer to next message, as we may remove the current message
        next_msg = (wsclient_send_item_t *) cur_msg->link.next;

        // Remove current message if it is a connect or disconnect record
        type = cur_msg->item.content_type;
        if (IsUspConnectOrDisconnectRecord(type))
        {
            RemoveWsclientSendItem(wc, cur_msg);
        }

        // Move to next message in the queue
        cur_msg = next_msg;
    }

    // Add the new connect record to the queue
    send_item = USP_MALLOC(sizeof(wsclient_send_item_t));
    USPREC_WebSocketConnect_Create(wc->cont_endpoint_id, &send_item->item);
    send_item->expiry_time = END_OF_TIME;

    DLLIST_LinkToHead(&wc->usp_record_send_queue, send_item);
}

/*********************************************************************//**
**
** CloseWsclientConnection
**
** Called to close the connection when something is wrong
**
** \param   wc - pointer to websocket client connection to restart the connection on
** \param   close_reason -  internal reason for closing the connection
** \param   close_status -  websockets reason for closing the connection
** \param   fmt - printf style format
**
** \return  Always -1 (to close the connection and retry)
**
**************************************************************************/
int CloseWsclientConnection(wsclient_t *wc, wsclient_close_reason_t close_reason, int close_status, char *fmt, ...)
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

    // Store the reasons for closure
    if (wc->close_reason == kWebSockCloseReason_Unknown)
    {
        wc->close_reason = close_reason;
    }
    lws_close_reason(wc->ws_handle, close_status, (unsigned char *)buf, chars_written);

    return -1;      // Returning an error is what forces libwebsockets to send the close frame
}

/*********************************************************************//**
**
** SendWsclientPing
**
** Called to send a websocket ping frame from the agent's websocket client
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int SendWsclientPing(wsclient_t *wc)
{
    unsigned char buf[LWS_PRE+1];       // Plus 1 to allow for a single data byte in the ping frame
    int bytes_written;

    // Exit if we've sent out too many pings without any pong response
    #define ALLOWED_MISSING_PONGS 2  // 0=all pings must have a pong, 1=one ping can be missing a pong etc
    if (wc->ping_count > ALLOWED_MISSING_PONGS)
    {
        USP_LOG_Error("%s: Closing connection to endpoint_id=%s because missing %d websocket PONG responses", __FUNCTION__, wc->cont_endpoint_id, wc->ping_count);
        wc->close_reason = kWebSockCloseReason_MissingPong;
        return -1;
    }
    wc->ping_count++;

    // Reset the flag which causes this function to be called, as we are going to send the ping now
    wc->send_ping = false;

    // Write PING frame
    USP_LOG_Debug("%s: Sending PING at time %d", __FUNCTION__, (int)time(NULL));
    buf[LWS_PRE] = wc->ping_count;
    bytes_written = lws_write(wc->ws_handle, &buf[LWS_PRE], 1, LWS_WRITE_PING);
    if (bytes_written < 0)
    {
        USP_LOG_Error("%s: lws_write() failed", __FUNCTION__);
        wc->close_reason = kWebSockCloseReason_InternalError;
        return -1;
    }

    return 0;
}

/*********************************************************************//**
**
** ScheduleWsclientRetry
**
** Called when it is time to retry connecting to the server
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 otherwise
**
**************************************************************************/
void ScheduleWsclientRetry(wsclient_t *wc)
{
    unsigned wait_time;

    // Start retrying this connection
    // NOTE: libwebsockets will have automatically deallocated its handle, in the cases where this function is called
    wc->state = kWebsockState_Retrying;
    wc->ws_handle = NULL;
    wc->retry_count++;

    // Calculate time until next retry
    wait_time = RETRY_WAIT_Calculate(wc->retry_count, wc->retry_interval, wc->retry_multiplier);
    USP_LOG_Info("%s: Retrying connection in %d seconds (retry_count=%d)", __FUNCTION__, wait_time, wc->retry_count);

    // Check that structure is not part of a linked list owned by libwebsockets anymore
    AssertRetryCallbackNotInUse(&wc->retry_timer);

    // Schedule the retry callback at the reconnection time
    lws_sul_schedule(wsc_ctx, 0, &wc->retry_timer, HandleWscEvent_RetryTimer, wait_time*LWS_US_PER_SEC);
}

/*********************************************************************//**
**
** AddWsclientUspExtension
**
** Called from the LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER event
** to add the 'Sec-WebSocket-Extensions' header with the 'bbf-usp-protocol' WebSocket Extension
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
** \param   ppHeaders - pointer to variable containing a pointer to a buffer containing the headers
** \param   headers_len - total size of the libwebsockets buffer containing the headers
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int AddWsclientUspExtension(struct lws *handle, unsigned char **ppHeaders, int headers_len)
{
    unsigned char *pHeadersEnd;
    char buf[128];
    int len;
    int err;
    wsclient_t *wc;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // If unable to add the header, then return an error, which will cause the connection to be retried
    pHeadersEnd = (*ppHeaders) + headers_len;
    len = USP_SNPRINTF(buf, sizeof(buf), "bbf-usp-protocol; eid=\"%s\"", DEVICE_LOCAL_AGENT_GetEndpointID());
    err = lws_add_http_header_by_token(handle, WSI_TOKEN_EXTENSIONS, (unsigned char *)buf, len, ppHeaders, pHeadersEnd);
    if (err != 0)
    {
        USP_LOG_Error("%s: lws_add_http_header_by_token() returned %d", __FUNCTION__, err);
        wc->close_reason = kWebSockCloseReason_InternalError;
        return -1;
    }

    return 0;
}

/*********************************************************************//**
**
** ValidateReceivedWsclientSubProtocol
**
** Called from the LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH event
** to validate that the websock handshake response contained the USP sub-protocol
** NOTE: If this function returns an error, then the connection will be reconnected
**
** \param   handle - libwebsockets handle identifying the connection with activity on it
**
** \return  0 if successful, -1 to close the connection and retry
**
**************************************************************************/
int ValidateReceivedWsclientSubProtocol(struct lws *handle)
{
    int num_bytes;
    char buf[256];
    wsclient_t *wc;
    char *endpoint_id;

    wc = lws_get_opaque_user_data(handle);
    USP_ASSERT(wc != NULL);

    // Exit if Sec-WebSocket-Protocol header is present, but too large for the buffer. In this case we just ignore the header.
    num_bytes = lws_hdr_copy(handle, buf, sizeof(buf)-1, WSI_TOKEN_PROTOCOL);
    if (num_bytes == -1)
    {
        USP_LOG_Error("%s: lws_hdr_copy() failed (%d)", __FUNCTION__, num_bytes);
        wc->close_reason = kWebSockCloseReason_InternalError;
        return -1;
    }

    // Exit if the server did not provide a Sec-WebSocket-Protocol header in its response
    if ((num_bytes == 0) || (buf[0] == '\0'))
    {
        USP_LOG_Error("%s: Server did not specify any websocket subprotocol in handshake response", __FUNCTION__);
        wc->close_reason = kWebSockCloseReason_InternalError;
        return -1;
    }

    // Exit if subprotocol chosen by server does not match the one we requested
    // NOTE: Libwebsockets should have already handled this case by sending a LWS_CALLBACK_CLIENT_CONNECTION_ERROR event
    if (strcmp(buf, WEBSOCKET_PROTOCOL_STR) !=0)
    {
        USP_LOG_Error("%s: Server specified incorrect websocket subprotocol in handshake response (got '%s', expected '%s')", __FUNCTION__, buf, WEBSOCKET_PROTOCOL_STR);
        wc->close_reason = kWebSockCloseReason_InternalError;
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

    // Exit if the server did not provide a Sec-WebSocket-Extensions header in its response
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

    // Exit if we connected to a different controller than configured.
    // We disconnect in this case, because otherwise notifications aren't sent on the right MTP, and the agent code gets complicated having to cope with this inconsistent state
    if (strcmp(endpoint_id, wc->cont_endpoint_id) != 0)
    {
        USP_LOG_Error("%s: Disconnecting as server is wrong controller (got '%s', expected '%s')", __FUNCTION__, endpoint_id, wc->cont_endpoint_id);
        wc->close_reason = kWebSockCloseReason_InternalError;
        return -1;
    }

    // Disconnect this endpoint from the websocket server (if already connected to it).
    // Websocket client connection to a controller takes precedence over websocket server connection
    WSSERVER_DisconnectEndpoint(endpoint_id);

    return 0;
}

/*********************************************************************//**
**
** IsUspRecordInWsclientQueue
**
** Determines whether the specified USP record is already queued, waiting to be sent
** This is used to avoid duplicate records being placed in the queue, which could occur under notification retry conditions
**
** \param   wc - pointer to websocket client connection which has queued USP records to send
** \param   pbuf - pointer to buffer containing USP Record to match against
** \param   pbuf_len - length of buffer containing USP Record to match against
**
** \return  true if the message is already queued
**
**************************************************************************/
bool IsUspRecordInWsclientQueue(wsclient_t *wc, unsigned char *pbuf, int pbuf_len)
{
    wsclient_send_item_t *si;

    // Iterate over USP Records in the queue
    si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    while (si != NULL)
    {
        // Exit if the USP record is already in the queue
        if ((si->item.pbuf_len == pbuf_len) && (memcmp(si->item.pbuf, pbuf, pbuf_len)==0))
        {
             return true;
        }

        // Move to next message in the queue
        si = (wsclient_send_item_t *) si->link.next;
    }

    // If the code gets here, then the USP record is not in the queue
    return false;
}

/*********************************************************************//**
**
** RemoveExpiredWsclientMessages
**
** Removes all expired messages from the queue of messages to send
** NOTE: This mechanism can be used to prevent the queue from filling up needlessly if the controller is offine
**
** \param   wc - pointer to websocket client connection which has queued USP records to send
**
** \return  None
**
**************************************************************************/
void RemoveExpiredWsclientMessages(wsclient_t *wc)
{
    time_t cur_time;
    wsclient_send_item_t *si;
    wsclient_send_item_t *next_msg;

    cur_time = time(NULL);
    si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;

    // Skip the first USP Record, if we're currently in the process of transmitting it
    if ((si != NULL) && (wc->tx_index != 0))
    {
        si = (wsclient_send_item_t *) si->link.next;
    }

    while (si != NULL)
    {
        next_msg = (wsclient_send_item_t *) si->link.next;
        if (cur_time > si->expiry_time)
        {
            RemoveWsclientSendItem(wc, si);
        }

        si = next_msg;
    }
}

/*********************************************************************//**
**
** AreAllWsclientResponsesSent
**
** Determines whether all webcocket USP Record queues are empty
** This function determines whether the websocket client thread can exit due to a scheduled exit (caused by reboot, factory reset or '-c stop')
**
** \param   None
**
** \return  true if there are no pending USP Records to send to any controllers over websockets
**
**************************************************************************/
bool AreAllWsclientResponsesSent(void)
{
    int i;
    wsclient_t *wc;

    // Iterate over all websock clients, exiting if any of them still have USP Records to send
    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if ((wc->state != kWebsockState_Idle) && (wc->usp_record_send_queue.head != NULL))
        {
            return false;
        }
    }

    // If the code gets here, then there are no pending USP records to send
    return true;
}

/*********************************************************************//**
**
** PurgeWsclientMessageQueue
**
** Removes all messages from the websocket client thread's message queue
**
** \param   None
**
** \return  None
**
**************************************************************************/
void PurgeWsclientMessageQueue(void)
{
    wsclient_msg_t *msg;
    start_client_msg_t *scm;
    queue_client_usp_record_msg_t *qur;

    // Exit if no messages to process
    msg = (wsclient_msg_t *) wsclient_msg_queue.head;
    while (msg != NULL)
    {
        // Process the message
        switch(msg->msg_type)
        {
            case kWsclientMsgType_StartClient:
                scm = &msg->inner.start_client_msg;
                USP_FREE(scm->cont_endpoint_id);
                USP_FREE(scm->host);
                USP_FREE(scm->path);
                break;

            case kWsclientMsgType_QueueUspRecord:
                qur = &msg->inner.queue_usp_record_msg;
                USP_FREE(qur->item.pbuf);
                break;

            default:
            case kWsclientMsgType_StopClient:
            case kWsclientMsgType_ActivateScheduledActions:
                // Nothing to free in these messages
                break;
        }

        // Remove the message from the queue and free it
        DLLIST_Unlink(&wsclient_msg_queue, msg);
        USP_FREE(msg);

        // Move to next message
        msg = (wsclient_msg_t *) wsclient_msg_queue.head;
    }
}

/*********************************************************************//**
**
** FindWsclient_ByInstance
**
** Finds the matching websocket client entry in wsclients[]
**
** \param   cont_instance - instance number of controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.(i)
**
** \return  pointer to matching entry in wsclients[] or NULL, if no match found
**
**************************************************************************/
wsclient_t *FindWsclient_ByInstance(int cont_instance, int mtp_instance)
{
    int i;
    wsclient_t *wc;

    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if ((wc->state != kWebsockState_Idle) && (wc->cont_instance == cont_instance) && (wc->mtp_instance == mtp_instance))
        {
            return wc;
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** FindFreeWsclient
**
** Finds the first unused entry in wsclients[]
**
** \param   None
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
wsclient_t *FindFreeWsclient(void)
{
    int i;
    wsclient_t *wc;

    for (i=0; i<NUM_ELEM(wsclients); i++)
    {
        wc = &wsclients[i];
        if (wc->state == kWebsockState_Idle)
        {
            return wc;
        }
    }

    // If the code gets here, then no free entry was found
    return NULL;
}

/*********************************************************************//**
**
** DestroyWsclient
**
** Frees all dynamically allocated memory (including queued USP Records)
** in the websock client structure and marks the slot as not in use
** NOTE: This function must only be called after you know that libwebsockets has deallocated its data structures
**
** \param   wc - pointer to websocket client connection to destroy
** \param   is_libwebsockets_destroyed - set if libwebsockets has already been destroyed
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
void DestroyWsclient(wsclient_t *wc, bool is_libwebsockets_destroyed)
{
    wsclient_send_item_t *si;

    // Only perform the following if libwebsockets hasn't already been destroyed
    if (is_libwebsockets_destroyed == false)
    {
        // Check that retry timer is not part of a linked list owned by libwebsockets anymore
        AssertRetryCallbackNotInUse(&wc->retry_timer);

        // Disable the ping timer if one setup (there might not be any ws_handle, if currently retrying)
        if (wc->ws_handle != 0)
        {
            lws_set_timer_usecs(wc->ws_handle, -1);
        }
    }

    // Free configuration parameters
    USP_SAFE_FREE(wc->cont_endpoint_id);
    USP_SAFE_FREE(wc->host);
    USP_SAFE_FREE(wc->path);

    // Free all queued USP messages
    si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    while (si != NULL)
    {
        RemoveWsclientSendItem(wc, si);
        si = (wsclient_send_item_t *) wc->usp_record_send_queue.head;
    }

    // Free any received parts of a USP Record
    USP_SAFE_FREE(wc->rx_buf);

    // Free any saved cert chain
    if (wc->cert_chain != NULL)
    {
        sk_X509_pop_free(wc->cert_chain, X509_free);
        wc->cert_chain = NULL;
    }

    // Mark the slot as not in use
    wc->state = kWebsockState_Idle;
}

/*********************************************************************//**
**
** RemoveWsclientSendItem
**
** Removes the specified item from the send queues linked list, and frees it and all it's components
**
** \param   wc - pointer to websocket client connection on which the send queue resides
** \param   si - send item to remove from the queue
**
** \return  None
**
**************************************************************************/
void RemoveWsclientSendItem(wsclient_t *wc, wsclient_send_item_t *si)
{
    DLLIST_Unlink(&wc->usp_record_send_queue, si);
    USP_FREE(si->item.pbuf);
    USP_FREE(si);
}

/*********************************************************************//**
**
** AssertRetryCallbackNotInUse
**
** Checks that the retry callback is present in any linked lists in use by libwebsockets
**
** \param   sul - pointer to structure associated with libwebsocket scheduled callbacks
**
** \return  None
**
**************************************************************************/
void AssertRetryCallbackNotInUse(lws_sorted_usec_list_t *sul)
{
    // Check that structure is not part of a linked list owned by libwebsockets anymore
    USP_ASSERT(sul->list.prev == NULL);
    USP_ASSERT(sul->list.next == NULL);
    USP_ASSERT(sul->list.owner == NULL);
}

/*********************************************************************//**
**
** LogWsclientDebug
**
** Logs debug from libwebsocket to USP Agent's configured output
**
** \param   level - bitmask of type of debug to emit (eg. LLL_ERR)
** \param   line - pointer to buffer containing log message to emit
**
** \return  pointer to unused entry in wsclients[] or NULL, if no free entries left
**
**************************************************************************/
void LogWsclientDebug(int level, const char *line)
{
    USP_LOG_Debug("%s: %s", __FUNCTION__, line);
}

#endif
