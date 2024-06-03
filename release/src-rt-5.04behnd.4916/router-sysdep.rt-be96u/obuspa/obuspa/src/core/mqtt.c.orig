/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2020, BT PLC
 * Copyright (C) 2020-2022  CommScope, Inc
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
 *
 * \file mqtt.c
 *
 * Called from the ProtocolHandler to implement the MQTT protocol
 *
 */
#include "mqtt.h"
#include "common_defs.h"
#include "dllist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "device.h"
#include "dm_exec.h"
#include "os_utils.h"
#include <errno.h>
#include <math.h>
#include "retry_wait.h"
#include "text_utils.h"
#include "msg_handler.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#ifdef ENABLE_MQTT

#include <mosquitto.h>

// Defines for MQTT Property Values
#define PUBLISH 0x30
#define CONTENT_TYPE 3
#define RESPONSE_TOPIC 8
#define ASSIGNED_CLIENT_IDENTIFIER 18
#define REQUEST_RESPONSE_INFORMATION 25
#define RESPONSE_INFORMATION 26
#define USER_PROPERTY 38

//------------------------------------------------------------------------------
// Maximum length of a topic in characters (including NULL terminator)
#define MAX_TOPIC_LEN 128

//------------------------------------------------------------------------------
// Cause of failure of an MQTT Client Connection
typedef enum
{
    kMqttFailure_None,
    kMqttFailure_Connect,
    kMqttFailure_ReadWrite,
    kMqttFailure_Misconfigured,
    kMqttFailure_OtherError,
} mqtt_failure_t;

//------------------------------------------------------------------------------
// State of an MQTT Client Connection
typedef enum
{
    kMqttState_Idle,                 // Not yet connected
    kMqttState_SendingConnect,       // Transitory state. Set to initiate connecting and sending the connect message.
                                     // In this state the socket hasn't been connected yet
    kMqttState_AwaitingConnect,      // Waiting for the connect callback
    kMqttState_Running,              // Normal state
    kMqttState_AwaitingDisconnect,   // Waiting for the disconnect callback
    kMqttState_ErrorRetrying,        // Error in the connection (replace with retrying soon..)

    kMqttState_Max
} mqtt_state_t;

//------------------------------------------------------------------------------
// Structure for each MQTT client
typedef struct
{
    mqtt_conn_params_t conn_params; // If the Instance member of the structure is INVALID, then this mqtt_client_t structure is not in use
    mqtt_state_t state;
    struct mosquitto *mosq;
    mqtt_subscription_t subscriptions[MAX_MQTT_SUBSCRIPTIONS];
    double_linked_list_t usp_record_send_queue;

    // From the broker
    mqtt_subscription_t response_subscription; // Contains subscription for agent's topic
                                               // NOTE: The topic in here may be NULL if not set by either Device.LocalAgent.MTP.{i}.ResponseTopicConfigured or present in the CONNACK
    int retry_count;
    time_t retry_time;
    time_t last_status_change;
    mqtt_failure_t failure_code;

    ctrust_role_t role;

    // Scheduler
    mqtt_conn_params_t next_params;
    scheduled_action_t schedule_reconnect;   // Sets whether an MQTT reconnect is scheduled
    scheduled_action_t schedule_close;       // Sets whether an MQTT disable is scheduled

    STACK_OF(X509) *cert_chain; // Certificate chain saved during SSL cert verification, and used to determine the role for the controller

    // NOTE: verify_callback, are_certs_loaded and ssl_ctx are set once and are not freed after that, even if this client structure is marked as 'not-in-use'
    ssl_verify_callback_t *verify_callback;
    SSL_CTX *ssl_ctx;           // SSL context used by this MQTT client instead of the default libmosquitto SSL context
    bool are_certs_loaded;      // Flag indicating whether the above ssl_ctx has been loaded with the trust store certs
                                // It is used to ensure that the certs are loaded only once, rather than on every reconnect

    char *agent_topic_from_connack;  // Saved copy of agent's topic (if received in the CONNACK)
    int disconnect_mid;         // Contains a libmosquitto allocated message_id value if the current message being sent is a USP disconnect record.
                                // Used to force a disconnection after the USP disconnect record has been sent
    bool is_reconnect;          // Used to indicate whether the current disconnect is part of a reconnect sequence, or due to a disable
    bool is_disconnected;       // Set if the disconnect callback has been called, to cause ensuing actions to be taken
} mqtt_client_t;

#define INVALID_MOSQUITTO_MID  0   // Libmosquitto will never allocate a message_id of 0. Current code reserves that value for future use

//------------------------------------------------------------------------------------
// Array used by debug to print out the current MQTT client connection state
char *mqtt_state_names[kMqttState_Max] =
{
    "Idle",
    "SendingConnect",
    "AwaitingConnect",
    "Running",
    "AwaitingDisconnect",
    "Error/Retrying"
};

//------------------------------------------------------------------------------
// Payload to send in MQTT queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    mtp_send_item_t item;   // Information about the content to send
    char *topic;            // Name of the MQTT Topic to send to
    mqtt_qos_t qos;         // QOS to request when building PUBLISH message (obtained from mqtt_conn_params_t.publish_qos)
    int mid;                // MQTT message ID. This is filled in by libmosquitto, when we tell libmosquitto to send this message
} mqtt_send_item_t;

mqtt_client_t mqtt_clients[MAX_MQTT_CLIENTS];
static pthread_mutex_t mqtt_access_mutex;

//------------------------------------------------------------------------------------
// Forward declarations. These are not static, because we need them in the symbol table for USP_LOG_Callstack()
#define MoveState(state, to, event) MoveState_Private(state, to, event, __FUNCTION__)
void InitClient(mqtt_client_t *client, int index);
void InitRetry(mqtt_retry_params_t *retry);
void ResetRetryCount(mqtt_client_t* client);
void DestroyClient(mqtt_client_t *client);
void FreeMqttClientCertChain(mqtt_client_t *client);
void InitSubscription(mqtt_subscription_t *sub);
int EnableMqttClient(mqtt_client_t* client);
int EnableMosquitto(mqtt_client_t *client);
void SetupCallbacks(mqtt_client_t *client);
void QueueUspConnectRecord_MQTT(mqtt_client_t *client, mtp_send_item_t *msi, char *controller_topic);
int SendQueueHead(mqtt_client_t *client);
void Connect(mqtt_client_t *client);
int PerformMqttClientConnect(mqtt_client_t *client);
int ConnectSetEncryption(mqtt_client_t *client);
void ConnectCallback(struct mosquitto *mosq, void *userdata, int result);
void ConnectV5Callback(struct mosquitto *mosq, void *userdata, int result, int flags, const mosquitto_property *props);
void SaveAgentTopicFromConnack(mqtt_client_t *client, char *agent_topic);
void DisconnectClient(mqtt_client_t *client);
void DisconnectCallback(struct mosquitto *mosq, void *userdata, int rc);
int Subscribe(mqtt_client_t *client, mqtt_subscription_t *sub, bool is_agent_topic);
int SubscribeV5(mqtt_client_t *client, mqtt_subscription_t *sub);
void SubscribeToAll(mqtt_client_t *client);
void SubscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int* granted_qos);
void SubscribeV5Callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int* granted_qos, const mosquitto_property* props);
int Unsubscribe(mqtt_client_t *client, mqtt_subscription_t *sub);
int UnsubscribeV5(mqtt_client_t *client, mqtt_subscription_t *sub);
void UnsubscribeCallback(struct mosquitto *mosq, void *userdata, int mid );
void UnsubscribeV5Callback(struct mosquitto *mosq, void *userdata, int mid, const mosquitto_property* props);
int Publish(mqtt_client_t *client, mqtt_send_item_t *msg);
int PublishV5(mqtt_client_t *client, mqtt_send_item_t *msg);
void PublishCallback(struct mosquitto* mosq, void *userdata, int mid /*message id*/);
void PublishV5Callback(struct mosquitto *mosq, void *userdata, int mid, int reason_code, const mosquitto_property *props);
void MessageCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void MessageV5Callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message, const mosquitto_property *props);
void ReceiveMqttMessage(mqtt_client_t *client, const struct mosquitto_message *message, char *response_topic);
char *FindReplyToTopic(char *publish_topic, char *buf, int len);
void LogCallback(struct mosquitto *mosq, void *userdata, int level, const char *str);
void HandleMqttError(mqtt_client_t *client, mqtt_failure_t failure_code, const char* message);
void MoveState_Private(mqtt_state_t *state, mqtt_state_t to, const char *event, const char* func);
void ParamReplace(mqtt_conn_params_t *dest, mqtt_conn_params_t *src);
bool IsUspRecordInMqttQueue(mqtt_client_t *client, unsigned char *pbuf, int pbuf_len);
mqtt_client_t *FindUnusedMqttClient_Local();
mqtt_client_t *FindMqttClientByInstance(int instance);
mqtt_subscription_t *FindMqttSubscriptionByInstance(mqtt_client_t *client, int subinstance);
mqtt_subscription_t *FindSubscriptionByMid(mqtt_client_t *client, int mid, bool *is_agent_topic);
mqtt_client_t *FindMqttClientByMosquitto(struct mosquitto *mosq);
int AddUserProperties(mosquitto_property **props);
int AddConnectProperties(mosquitto_property **props);
int ConvertToMosquittoVersion(mqtt_protocolver_t version, int* mosquitto_version);
void PopClientUspQueue(mqtt_client_t *client);
void CleanMqttClient(mqtt_client_t *client, bool is_reconnect);
void HandleMqttClose(mqtt_client_t *client);
void HandleMqttReconnect(mqtt_client_t *client);
void HandleMqttReconnectAfterDisconnect(mqtt_client_t *client);
void HandleMqttDisconnect(mqtt_client_t *client);

//------------------------------------------------------------------------------------
#define DEFINE_MQTT_TrustCertVerifyCallbackIndex(index) \
int MQTT_TrustCertVerifyCallback_##index (int preverify_ok, X509_STORE_CTX *x509_ctx) \
{\
    return DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(preverify_ok, x509_ctx, &mqtt_clients[index].cert_chain);\
}

#define MQTT_TrustCertVerifyCallbackIndex(index) MQTT_TrustCertVerifyCallback_##index

DEFINE_MQTT_TrustCertVerifyCallbackIndex(0);
DEFINE_MQTT_TrustCertVerifyCallbackIndex(1);
DEFINE_MQTT_TrustCertVerifyCallbackIndex(2);
DEFINE_MQTT_TrustCertVerifyCallbackIndex(3);
DEFINE_MQTT_TrustCertVerifyCallbackIndex(4);
// Add more, with incrementing indexes here, if you change MAX_MQTT_CLIENTS

//------------------------------------------------------------------------------------
// Global variables
ssl_verify_callback_t* mqtt_verify_callbacks[] = {
    MQTT_TrustCertVerifyCallbackIndex(0),
    MQTT_TrustCertVerifyCallbackIndex(1),
    MQTT_TrustCertVerifyCallbackIndex(2),
    MQTT_TrustCertVerifyCallbackIndex(3),
    MQTT_TrustCertVerifyCallbackIndex(4),
    // Add more, with incrementing indexes here, if you change MAX_MQTT_CLIENTS
};

USP_COMPILEASSERT( ((sizeof(mqtt_verify_callbacks)/sizeof(ssl_verify_callback_t*)) == MAX_MQTT_CLIENTS),
        "There must be MAX_MQTT_CLIENTS callbacks defined");

/*********************************************************************//**
**
** MQTT_Init
**
** Initialise the MQTT component - basically a constructor
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_Init(void)
{
    int i;
    int err = USP_ERR_OK;
    mosquitto_lib_init();

    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        InitClient(&mqtt_clients[i], i);
    }

    err = OS_UTILS_InitMutex(&mqtt_access_mutex);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to initialise MQTT mutex", __FUNCTION__);
    }
    return err;
}

/*********************************************************************//**
**
** MQTT_Destroy
**
** Frees all memory associated with this component and closes all sockets
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MQTT_Destroy(void)
{
    int i;

    mqtt_client_t* client = NULL;
    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];
        DestroyClient(client);
    }

    memset(mqtt_clients, 0, sizeof(mqtt_clients));
    mosquitto_lib_cleanup();

}

/*********************************************************************//**
**
** MQTT_Start
**
** Called before starting all MQTT connections
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_Start(void)
{
    int i;
    int err = USP_ERR_OK;
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Initialise the SSL contexts for all of the clients
    // This cannot be done in MQTT_Init() because at that time in the initialisation the trust store certs haven't been locally cached
    // Also WSCLIENT_Start() is called after MQTT_Init(0, and it re-initialises OpenSSL (libwebsockets limitation)
    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        // Exit if unable to create an SSL context
        // NOTE: Trust store certs are only loaded into the context later, on demand, since most of these contexts will be unused
        client = &mqtt_clients[i];
        client->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
        client->are_certs_loaded = false;
        if (client->ssl_ctx == NULL)
        {
            USP_ERR_SetMessage("%s: SSL_CTX_new failed", __FUNCTION__);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }

        // Explicitly disallow SSLv2, as it is insecure. See https://arxiv.org/pdf/1407.2168.pdf
        // NOTE: Even without this, SSLv2 ciphers don't seem to appear in the cipher list. Just added in case someone is using an older version of OpenSSL.
        SSL_CTX_set_options(client->ssl_ctx, SSL_OP_NO_SSLv2);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    return err;
}

/*********************************************************************//**
**
** MQTT_EnableClient
**
** Enable the MQTT client connection to the broker with given params and topic
**
** \param   mqtt_params - pointer to data model parameters specifying the mqtt params
** \param   subscriptions[MAX_MQTT_SUBSCRIPTIONS] - subscriptions to use for this client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_EnableClient(mqtt_conn_params_t *mqtt_params, mqtt_subscription_t subscriptions[MAX_MQTT_SUBSCRIPTIONS])
{
    int i;
    int err = USP_ERR_OK;
    mqtt_client_t *client = NULL;
    mqtt_subscription_t *sub;

    // Exit if any of the subscriptions have an empty topic, but are enabled
    for (i=0; i<MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        sub = &subscriptions[i];
        if ((sub->instance != INVALID) && (sub->enabled==true) &&
            ((sub->topic==NULL) || (sub->topic[0] == '\0')) )
        {
            USP_LOG_Error("%s: Cannot enable client (instance=%d) as an enabled subscription's topic (instance=%d) is empty", __FUNCTION__, mqtt_params->instance, sub->instance);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }
    }

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // See if we're enabling an existing MQTT client for this instance
    client = FindMqttClientByInstance(mqtt_params->instance);
    if (client == NULL)
    {
        // If no pre-existing MQTT client for this instance, then attempt to allocate one
        client = FindUnusedMqttClient_Local();
        if (client == NULL)
        {
            USP_LOG_Error("%s: No internal MQTT client matching Device.MQTT.Connection.%d", __FUNCTION__, mqtt_params->instance);
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }

        // Mark the client as 'in-use'
        client->conn_params.instance = mqtt_params->instance;
    }

    // Exit if the caller needs to disable this MQTT client first (or wait for the disconnect to complete/timeout)
    if ((client->state != kMqttState_Idle) && (client->state != kMqttState_ErrorRetrying))
    {
        USP_LOG_Error("%s: Unexpected state: %s for client %d. Failing connection..",
            __FUNCTION__, mqtt_state_names[client->state], client->conn_params.instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    ParamReplace(&client->conn_params, mqtt_params);
    ParamReplace(&client->next_params, mqtt_params);

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        MQTT_SubscriptionReplace(&client->subscriptions[i], &subscriptions[i]);
    }

    ResetRetryCount(client);

    if (client->conn_params.enable)
    {
        err = EnableMqttClient(client);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    // Wakeup via the socket to handle the actual connect
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_MqttWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** MQTT_DisableClient
**
** Disables the specified MQTT client
**
** \param   instance - Instance number in Device.MQTT.Client.{i}
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_DisableClient(int instance)
{
    int err;
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Exit if no client exists with the specified instance number
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Schedule the disable to occur after it has been activated, and all queued messages sent out
    client->schedule_close = kScheduledAction_Signalled;
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    // Wakeup via the socket
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_MqttWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** MQTT_QueueBinaryMessage
**
** Queue a binary message onto an MQTT connection
**
** \param   msi - Information about the content to send. The ownership of
**              the payload buffer is passed to this function, unless an error is returned.
** \param   instance - instance number for the client in Device.MQTT.Client.{i}
** \param   topic - controller's topic to publish the message on
**
** \return  USP_ERR_OK on success, USP_ERR_XXX otherwise
**
**************************************************************************/
int MQTT_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char* topic)
{
    int err = USP_ERR_GENERAL_FAILURE;
    USP_ASSERT(msi != NULL);

    // Add the message to the back of the queue
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    if (is_mqtt_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&mqtt_access_mutex);
        USP_FREE(msi->pbuf);
        return USP_ERR_OK;
    }

    // Find suitable client to queue on
    mqtt_client_t *client = NULL;
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client %d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if this is a connect record, adding it at the front of the queue
    if (msi->content_type == kMtpContentType_ConnectRecord)
    {
        QueueUspConnectRecord_MQTT(client, msi, topic);
        err = USP_ERR_OK;
        goto exit;
    }

    // Find if this is a duplicate in the queue
    // May have been tried to be resent by the MTP_EXEC thread
    if (IsUspRecordInMqttQueue(client, msi->pbuf, msi->pbuf_len))
    {
        // No error, just return success
        USP_FREE(msi->pbuf);
        err = USP_ERR_OK;
        goto exit;
    }

    mqtt_send_item_t *send_item;
    send_item = USP_MALLOC(sizeof(mqtt_send_item_t));
    send_item->item = *msi;  // NOTE: Ownership of the payload buffer passes to the MQTT client

    // Determine Controller topic to publish to
    if (topic != NULL)
    {
        send_item->topic = USP_STRDUP(topic);
    }
    else
    {
        // NOTE: If Device.LocalAgent.Controller.{i}.MTP.{i}.MQTT.Topic is not configured, then send_item->topic will be set to NULL here
        send_item->topic = USP_STRDUP(client->conn_params.topic);
    }

    // Exit (discarding the USP record) if no controller topic to send the message to
    // NOTE: This should already have been ensured by the caller (in the function CalcNotifyDest)
    if ((send_item->topic == NULL) || (send_item->topic[0] == '\0'))
    {
        USP_LOG_Error("%s: Discarding USP Message (%s) as no controller topic to send to", __FUNCTION__, MSG_HANDLER_UspMsgTypeToString(send_item->item.usp_msg_type));
        USP_SAFE_FREE(send_item->item.pbuf);
        USP_SAFE_FREE(send_item->topic);
        USP_FREE(send_item);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    send_item->mid = INVALID;
    send_item->qos = client->conn_params.publish_qos;

    DLLIST_LinkToTail(&client->usp_record_send_queue, send_item);
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    if (err == USP_ERR_OK)
    {
        MTP_EXEC_MqttWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** MQTT_ScheduleReconnect
**
** Signals that an MQTT reconnect occurs when all queued message have been sent
** See comment header above definition of scheduled_action_t for an explanation of this and why
**
** \param   mqtt_params - pointer to data model parameters specifying the MQTT connection
**
** \return  None
**
**************************************************************************/
void MQTT_ScheduleReconnect(mqtt_conn_params_t *mqtt_params)
{
    mqtt_client_t *client = NULL;
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    if (is_mqtt_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&mqtt_access_mutex);
        return;
    }

    client = FindMqttClientByInstance(mqtt_params->instance);
    if (client == NULL)
    {
        goto exit;
    }

    ParamReplace(&client->next_params, mqtt_params);

    // Make sure we use the same instance
    client->next_params.instance = client->conn_params.instance;

    client->schedule_reconnect = kScheduledAction_Signalled;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    // Wakeup the MTP thread from select()
    if (client != NULL)
    {
        return MTP_EXEC_MqttWakeup();
    }
}

/*********************************************************************//**
**
** MQTT_ActivateScheduledActions
**
** Called when all USP response messages have been queued
** This function activates all scheduled actions which have been signalled
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work and why
**
** \param   None
**
** \return  None
**
**************************************************************************/
void MQTT_ActivateScheduledActions(void)
{
    int i;
    mqtt_client_t* client;
    bool wakeup = false;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    if (is_mqtt_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&mqtt_access_mutex);
        return;
    }

    for (i=0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];
        if (client->schedule_reconnect == kScheduledAction_Signalled)
        {
            client->schedule_reconnect = kScheduledAction_Activated;
            wakeup = true;
        }

        if (client->schedule_close == kScheduledAction_Signalled)
        {
            client->schedule_close = kScheduledAction_Activated;
            wakeup = true;
        }
    }

    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    if (wakeup)
    {
        MTP_EXEC_MqttWakeup();
    }
}

/*********************************************************************//**
**
** MQTT_GetMtpStatus
**
** Obtains the MTP status (Up/Down) of the specified MQTT Client instance
**
** \param   instance - the Device.MQTT.Client.{i} number
**
** \return  status of the specified MQTT Client instance
**
**************************************************************************/
mtp_status_t MQTT_GetMtpStatus(int instance)
{
    mtp_status_t status;
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Exit if not matching client found for this instance number
    client = FindMqttClientByInstance(instance);
    if (client==NULL)
    {
        status = kMtpStatus_Error;
        goto exit;
    }

    switch(client->state)
    {
        case kMqttState_Running:
            status = kMtpStatus_Up;
            break;

        case kMqttState_ErrorRetrying:
            status = kMtpStatus_Error;
            break;

        case kMqttState_Idle:
        case kMqttState_SendingConnect:
        case kMqttState_AwaitingConnect:
        case kMqttState_AwaitingDisconnect:
        default:
            status = kMtpStatus_Down;
            break;
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return status;
}

/*********************************************************************//**
**
** MQTT_GetClientStatus
**
** Get a string of the connection status for the device model items Device.MQTT.Client.{i}.Status
**
** \param   instance - Instance ID from Device.MQTT.Client.{i}
**
** \return  char string of connection status
**
**************************************************************************/
const char *MQTT_GetClientStatus(int instance)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    client = FindMqttClientByInstance(instance);
    const char *status;

    if (client)
    {
        switch (client->state)
        {
            case kMqttState_SendingConnect:
            case kMqttState_AwaitingConnect:
                status = "Connecting";
                break;

            case kMqttState_Running:
                status = "Connected";
                break;

            case kMqttState_ErrorRetrying:
                {
                    switch (client->failure_code)
                    {
                        case kMqttFailure_Misconfigured:
                            status = "Error_Misconfigured";
                            break;

                        case kMqttFailure_Connect:
                            status = "Error_BrokerUnreachable";
                            break;

                        default:
                            status = "Error";
                    }
                }
                break;

            case kMqttState_Idle: // Fallthrough, for completeness
            case kMqttState_AwaitingDisconnect:
            default:
                status = "Disabled";
                break;
        }
    }
    else
    {
        status = "Disabled";
    }

    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return status;
}

/*********************************************************************//**
**
** MQTT_AreAllResponsesSent
**
** Determines whether all responses have been sent, and that there are no outstanding incoming messages
**
** \param   None
**
** \return  true if all responses have been sent
**
**************************************************************************/
bool MQTT_AreAllResponsesSent(void)
{
    int i;
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    bool responses_sent = true;
    bool all_responses_sent = true;

    // Not strictly needed - but to protect against bad calling
    if (is_mqtt_mtp_thread_exited)
    {
        OS_UTILS_UnlockMutex(&mqtt_access_mutex);
        return true;
    }

    mqtt_client_t *client = NULL;

    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];
        if (client->conn_params.instance != INVALID)
        {
            // Check if the queue is empty
            responses_sent = (client->usp_record_send_queue.head == NULL);
        }
        if (!responses_sent)
        {
            all_responses_sent = false;
        }
    }


    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return all_responses_sent;
}

/*********************************************************************//**
**
** MQTT_AddSubscription
**
** Called when a new subscription instance has been added into Device.MQTT.Client.{i}.Subscription.{i}
**
** \param   instance - Device.MQTT.Client.{i} number for connection
** \param   subscription - pointer to subscription to add
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_AddSubscription(int instance, mqtt_subscription_t* subscription)
{
    int err = USP_ERR_OK;
    mqtt_client_t *client = NULL;
    mqtt_subscription_t *sub_dest = NULL;

    // Exit if subscription is enabled and topic is empty string. NOTE: This should have been prevented by the caller.
    USP_ASSERT(subscription->topic != NULL);
    if ((subscription->enabled) && (subscription->topic[0] == '\0'))
    {
        USP_LOG_Error("%s: Cannot subscribe to a topic of empty string", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT client matching Device.MQTT.Client.%d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    USP_ASSERT(subscription->instance != INVALID);
    USP_ASSERT(FindMqttSubscriptionByInstance(client, subscription->instance)==NULL); // This instance shouldn't exist, since we're adding it

    // Exit if unable to find a free subscription entry
    sub_dest = FindMqttSubscriptionByInstance(client, INVALID);
    if (sub_dest == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT client subscription remaining for %d", __FUNCTION__, subscription->instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Copy the subscription member variables into the new subscription entry
    MQTT_SubscriptionReplace(sub_dest, subscription);

    // Subscribe to the subscription only if currently connected (and subscription is enabled)
    if ((sub_dest->enabled == true) && (client->state == kMqttState_Running))
    {
        err = Subscribe(client, sub_dest, false);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    // If the code gets here, adding the subscription was successful
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return err;
}

/*********************************************************************//**
**
** MQTT_DeleteSubscription
**
** Deletes the specified subscription instance
**
** \param   instance - Client instance in Device.MQTT.Client.{i}
** \param   subinstance - Subscription instance in Device.MQTT.Client.{i}.Subscription.{i}
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_DeleteSubscription(int instance, int subinstance)
{
    int err = USP_ERR_OK;
    mqtt_client_t *client = NULL;
    mqtt_subscription_t *sub = NULL;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT client matching Device.MQTT.Client.%d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    sub = FindMqttSubscriptionByInstance(client, subinstance);
    if (sub == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT subscription matching Device.MQTT.Client.%d.Subscription.%d",
                      __FUNCTION__, instance, subinstance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    USP_ASSERT(sub->instance != INVALID);

    if ((sub->enabled) && (client->state == kMqttState_Running))
    {
        switch(sub->state)
        {
            case kMqttSubState_Resubscribing:
                // In the resubscribing state, we can't be sure whether we are waiting for the UNSUBACK of the old subscription
                // or the SUBACK of the new subscription. In the former case, the subscription is already unsubscribed or will be.
                // But in the latter case the new subscription is subscribed or will be. To err on the side of caution, assume
                // that we are in the latter case, and unsubscribe.
                // Intentional fall through

            case kMqttSubState_Subscribed:
                // Unsubscribe, ignoring any errors, since we are going to mark the subscription as not in use anyway
                Unsubscribe(client, sub);
                break;

            default:
            case kMqttSubState_Unsubscribed:
            case kMqttSubState_Subscribing:
            case kMqttSubState_Unsubscribing:
                // Nothing to do, the subscription is already unsubscribed, or will be
                break;
        }
    }

    // Mark the subscription as not in use.
    // Note: doing this, if just initiated an unsubscribe will lead to an error being logged in the unsubscribe callback (the error can be ignored)
    sub->instance = INVALID;

    // If the code gets here, deleting the subscription was successful
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return err;
}

/*********************************************************************//**
**
** MQTT_ScheduleResubscription
**
** Unsubscribes the specified subscription, then resubscribes to it with a different topic and QoS
**
** \param   instance - Client instance in Device.MQTT.Client.{i}
** \param   new_sub - pointer to new subscription parameters. Also contains subscription instance number, identifying the subscription to change
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MQTT_ScheduleResubscription(int instance, mqtt_subscription_t *new_sub)
{
    int err = USP_ERR_INTERNAL_ERROR;
    mqtt_client_t *client = NULL;
    mqtt_subscription_t *sub = NULL;
    bool is_unsubscribing;

    // Exit if subscription is enabled and topic is empty string. NOTE: This should have been prevented by the caller.
    USP_ASSERT(new_sub->topic != NULL);
    if ((new_sub->enabled) && (new_sub->topic[0] == '\0'))
    {
        USP_LOG_Error("%s: Cannot subscribe to a topic of empty string", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    USP_ASSERT(instance != INVALID);
    USP_ASSERT(new_sub->instance != INVALID);
    USP_ASSERT(new_sub->topic != NULL);   // Ensured by caller, as topic is retrieved from Device.MQTT.Client.Subscription.{i}.Topic

    // Exit if unable to find MQTT client matching the specified instance number
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT client matching Device.MQTT.Connection.%d", __FUNCTION__, instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to find the subscription matching the specified instance number for the client
    sub = FindMqttSubscriptionByInstance(client, new_sub->instance);
    if (sub == NULL)
    {
        USP_LOG_Error("%s: No internal MQTT client subscription matching %d", __FUNCTION__, new_sub->instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if not currently connected and subscribed - the resubscription will occur after receiving the connect callback
    if (client->state != kMqttState_Running)
    {
        MQTT_SubscriptionReplace(sub, new_sub);
        err = USP_ERR_OK;
        goto exit;
    }

    // First unsubscribe the existing subscription (if necessary)
    is_unsubscribing = (sub->enabled) && (sub->state != kMqttSubState_Unsubscribed) && (sub->state != kMqttSubState_Unsubscribing);
    if (is_unsubscribing)
    {
        USP_LOG_Debug("%s: Unsubscribe from topic=%s", __FUNCTION__, sub->topic);
        if (client->conn_params.version == kMqttProtocol_5_0)
        {
            err = UnsubscribeV5(client, sub);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
        else
        {
            if (mosquitto_unsubscribe(client->mosq, &sub->mid, sub->topic) != MOSQ_ERR_SUCCESS)
            {
                USP_LOG_Error("%s: Failed to unsubscribe from %s", __FUNCTION__, sub->topic);
                err = USP_ERR_INTERNAL_ERROR;
                goto exit;
            }
        }
    }

    // Change to new subscription
    // NOTE: Do not overwrite mid member, as we might need it to identify the UNSUBACK message if when we receive it (if unsubscribing)
    sub->qos = new_sub->qos;
    USP_SAFE_FREE(sub->topic);
    sub->topic = USP_STRDUP(new_sub->topic);
    sub->enabled = new_sub->enabled;

    // Update state
    if (is_unsubscribing)
    {
        if (sub->enabled)
        {
            // Set state to resubscribe to new topic on reception of UNSUBACK (initiated by the unsubscribe, above)
            sub->state = kMqttSubState_Resubscribing;
        }
        else
        {
            // Since new subscription is disabled, do not resubscribe on reception of UNSUBACK
            sub->state = kMqttSubState_Unsubscribing;
        }
    }
    else
    {
        if (sub->enabled)
        {
            if (client->state == kMqttState_Running)
            {
                // Since new subscription is enabled, and we don't have to wait for old subscription to unsubscribe first,
                // subscribe to new subscription here
                err = Subscribe(client, sub, false);
                if (err != USP_ERR_OK)
                {
                    goto exit;
                }
            }
        }
        else
        {
            // NOTE: Nothing to do, if not unsubscribing, and new subscription is disabled
            sub->state = kMqttSubState_Unsubscribed;
        }
    }

    // If the code gets here, the resubscription was successful
    err = USP_ERR_OK;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return err;
}

/*********************************************************************//**
**
** MQTT_SubscriptionReplace
**
** Replaces the specified subscription with a different subscription
** NOTE: This function does not need to be an API function, as it is only called from this file
**
** \param   dest - pointer to MQTT subscription to replace
** \param   src  - pointer to MQTT subscription to copy
**
** \return  None
**
**************************************************************************/
void MQTT_SubscriptionReplace(mqtt_subscription_t *dest, mqtt_subscription_t *src)
{
    MQTT_SubscriptionDestroy(dest);

    *dest = *src;
    dest->topic = USP_STRDUP(src->topic);
}

/*********************************************************************//**
**
** MQTT_SubscriptionDestroy
**
** Destroys the specified subscription and marks it as 'unused'
**
** \param   sub - pointer to subscription to destroy
**
** \return  None
**
**************************************************************************/
void MQTT_SubscriptionDestroy(mqtt_subscription_t *sub)
{
    USP_SAFE_FREE(sub->topic);
    memset(sub, 0, sizeof(mqtt_subscription_t));
    sub->instance = INVALID;
}

/*********************************************************************//**
**
** MQTT_UpdateAllSockSet
**
** \param set - socket set to update
**
** \return None
**
**************************************************************************/
void MQTT_UpdateAllSockSet(socket_set_t *set)
{
    int i;
    int sock;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    if (is_mqtt_mtp_thread_exited)
    {
        USP_LOG_Error("%s: mtp thread already exited", __FUNCTION__);
        OS_UTILS_UnlockMutex(&mqtt_access_mutex);
        return;
    }

    // Set a default timeout of 500ms
    // Makes sure we ping once per second.
    SOCKET_SET_UpdateTimeout(1*SECONDS, set);

    // Iterate over all mqtt clients currently enabled
    mqtt_client_t* client = NULL;
    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];
        if (client->conn_params.instance != INVALID)
        {
            switch (client->state)
            {
                case kMqttState_Running:
                    if (client->usp_record_send_queue.head)
                    {
                        if (SendQueueHead(client) == USP_ERR_OK)
                        {
                            // If this is a USP disconnect record, then save the (libmosquitto allocated) message id, so that
                            // when we get the publish callback, we can then shutdown the connection
                            // NOTE: USP Disconnect records terminating an E2E session (kMtpContentType_E2E_SessTermination) do not shutdown the connection
                            mqtt_send_item_t *q_msg;
                            q_msg = (mqtt_send_item_t *) client->usp_record_send_queue.head;
                            client->disconnect_mid = (q_msg->item.content_type == kMtpContentType_DisconnectRecord) ? q_msg->mid : INVALID_MOSQUITTO_MID;

                            // Remove item from send queue
                            PopClientUspQueue(client);
                        }
                        else
                        {
                            USP_LOG_Error("%s: Failed to send head of the queue, leaving there to try again", __FUNCTION__);
                        }
                    }
                    break;

                case kMqttState_ErrorRetrying:
                    {
                        time_t cur_time = time(NULL);
                        if (client->schedule_reconnect == kScheduledAction_Activated)
                        {
                            USP_LOG_Debug("%s: Scheduled reconnect in error due to reconfig.", __FUNCTION__);

                            // Use the correct configuration, and set retry to now
                            // Triggers reconnect straight away
                            ParamReplace(&client->conn_params, &client->next_params);
                            ResetRetryCount(client);
                            client->retry_time = cur_time;
                        }

                        // Retry connection - looking at the retry time
                        if (client->retry_time <= 0)
                        {
                            // failed - no retry time
                            USP_LOG_Error("%s: Retry time not set - failed", __FUNCTION__);
                            HandleMqttError(client, client->failure_code, "Retry error");
                        }
                        else if (cur_time >= client->retry_time)
                        {
                            USP_LOG_Debug("%s: Retrying connection", __FUNCTION__);
                            EnableMqttClient(client);
                        }
                        else
                        {
                            time_t diff = client->retry_time - cur_time;
                            USP_LOG_Debug("%s: Waiting for time to retry: remaining time: %lds retry_time: %ld time: %ld",
                                    __FUNCTION__, (long int)diff, (long int)client->retry_time, (long int)cur_time);
                        }
                    }
                    break;

                default:
                    // Nothing to do
                    break;
            }

            sock = mosquitto_socket(client->mosq);
            if (sock != -1)
            {
                // Add write socket if we want to write
                if (mosquitto_want_write(client->mosq))
                {
                    SOCKET_SET_AddSocketToSendTo(sock, SECONDS, set);
                }

                // Always try to read
                SOCKET_SET_AddSocketToReceiveFrom(sock, SECONDS, set);
            }
        }
    }

    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** MQTT_ProcessAllSocketActivity
**
** Processes activity on the specified sockets
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  None
**
**************************************************************************/
void MQTT_ProcessAllSocketActivity(socket_set_t* set)
{
    int i;
    int sock;
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    if (is_mqtt_mtp_thread_exited)
    {
        goto exit;
    }

    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];

        if (client->conn_params.instance != INVALID)
        {
            switch(client->state)
            {
                case kMqttState_SendingConnect:
                    // NOTE: the MQTT mutex is released temporarily around mosquitto_connect() in the following call
                    Connect(client);

                    // fall through
                default:
                    sock = mosquitto_socket(client->mosq);
                    if (sock != -1)
                    {
                        // Send all data on socket - if required
                        if (SOCKET_SET_IsReadyToWrite(sock, set) && mosquitto_want_write(client->mosq))
                        {
                            if (mosquitto_loop_write(client->mosq, 1) != MOSQ_ERR_SUCCESS)
                            {
                                USP_LOG_Error("%s: Failed to write to socket", __FUNCTION__);
                            }
                        }
                    }

                    sock = mosquitto_socket(client->mosq);
                    if (sock != -1)   // NOTE: Retesting, as connection may have been closed after writing
                    {
                        if (SOCKET_SET_IsReadyToRead(sock, set))
                        {
                            if (mosquitto_loop_read(client->mosq, 1) != MOSQ_ERR_SUCCESS)
                            {
                                USP_LOG_Error("%s: Failed to read from socket", __FUNCTION__);
                            }
                        }
                    }

                    sock = mosquitto_socket(client->mosq);
                    if (sock != -1)   // NOTE: Retesting, as connection may have been closed after reading
                    {
                        if (mosquitto_loop_misc(client->mosq) != MOSQ_ERR_SUCCESS)
                        {
                            USP_LOG_Error("%s: Failed to write misc", __FUNCTION__);
                        }
                    }
                    break;
            }

            // Deal with restarting the connection if all responses have been sent
            if ((client->schedule_reconnect == kScheduledAction_Activated) && (client->usp_record_send_queue.head == NULL))
            {
                HandleMqttReconnect(client);
            }

            // Deal with closing the connection
            if (client->schedule_close == kScheduledAction_Activated)
            {
                HandleMqttClose(client);
            }

            // If the client has disconnected, then handle the follow on actions here
            if (client->is_disconnected)
            {
                HandleMqttDisconnect(client);
            }
        }
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** MQTT_InitConnParams
**
** Initialise the conn params with the default data
**
** \param   params - pointer to connection parameters to initialise
**
** \return  None
**
**************************************************************************/
void MQTT_InitConnParams(mqtt_conn_params_t *params)
{
    memset(params, 0, sizeof(mqtt_conn_params_t));

    params->instance = INVALID;
    params->version = kMqttProtocol_Default;
    params->publish_qos = kMqttQos_Default;

    InitRetry(&params->retry);
}

/*********************************************************************//**
**
** MQTT_DestroyConnParams
**
** Free everything that has been allocated under *params.
** Destroys all data within the params. Will not destroy the actual data
** structure that holds the params.
**
** \param params - pointer to params to free the internal data from
**
** \return None
**
**************************************************************************/
void MQTT_DestroyConnParams(mqtt_conn_params_t *params)
{
    // Free all the items in the parameters
    USP_SAFE_FREE(params->host);
    USP_SAFE_FREE(params->username);
    USP_SAFE_FREE(params->password);
    USP_SAFE_FREE(params->topic);
    USP_SAFE_FREE(params->response_topic);
    USP_SAFE_FREE(params->client_id);
    USP_SAFE_FREE(params->name);
    USP_SAFE_FREE(params->response_information);

    memset(params, 0, sizeof(mqtt_conn_params_t));

    // Set to invalid as this is the default
    params->instance = INVALID;
}

/*********************************************************************//**
**
** MQTT_GetAgentResponseTopicDiscovered
**
** Reads the value of the CONNACK Response Information property supplied by a MQTT 5.0 broker
** If this is not available (for example not MQTT v5.0 or CONNACK not received yet) then an empty string is returned
**
** \param   instance - instance in Device.MQTT.Client.{i}
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  Always USP_ERR_OK - an empty string is returned if the value cannot be determined
**
**************************************************************************/
int MQTT_GetAgentResponseTopicDiscovered(int instance, char *buf, int len)
{
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Set default return value - an empty string
    *buf = '\0';

    // Exit if no client exists with the specified instance number
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        goto exit;
    }

    // Exit if client is not currently connected
    if (client->state != kMqttState_Running)
    {
        goto exit;
    }

    // Exit if client is not using MQTT v5 (earlier versions of MQTT do not allow for a response information property in the CONNACK)
    if (client->conn_params.version != kMqttProtocol_5_0)
    {
        goto exit;
    }

    // Copy the agent's discovered response topic into the return buffer
    if (client->agent_topic_from_connack != NULL)
    {
        USP_STRNCPY(buf, client->agent_topic_from_connack, len);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** HandleMqttReconnect
**
** Called to perform a scheduled reconnect, when all queued messages have been sent
**
** \param   client - pointer to MQTT client to perform the reconnect on
**
** \return  None
**
**************************************************************************/
void HandleMqttReconnect(mqtt_client_t *client)
{
    int sock;

    USP_LOG_Debug("%s: Schedule reconnect ready!", __FUNCTION__);

    switch(client->state)
    {
        case kMqttState_Idle:
        case kMqttState_ErrorRetrying:
            // In these states, we can start the connection immediately
            ParamReplace(&client->conn_params, &client->next_params);
            EnableMqttClient(client);
            break;

        case kMqttState_AwaitingDisconnect:
            // Already waiting for a disconnect, so just indicate that the disconnect should initiate the reconnect
            client->is_reconnect = true;
            break;

        default:
        case kMqttState_SendingConnect:
        case kMqttState_AwaitingConnect:
        case kMqttState_Running:
            // In these states, we need to first send a disconnect and wait for the disconnect callback.
            // The disconnect callback initiates the reconnect
            sock = mosquitto_socket(client->mosq);
            if (sock != -1)
            {
                client->is_reconnect = true;
                DisconnectClient(client);
            }
            else
            {
                // The socket should never be invalid in these states, but if it is, just start the connection immediately
                ParamReplace(&client->conn_params, &client->next_params);
                EnableMqttClient(client);
            }
            break;
    }
}

/*********************************************************************//**
**
** HandleMqttClose
**
** Called to perform a scheduled close
**
** \param   client - pointer to MQTT client to perform the close on
**
** \return  None
**
**************************************************************************/
void HandleMqttClose(mqtt_client_t *client)
{
    switch(client->state)
    {
        case kMqttState_AwaitingDisconnect:
            // If the client is already disconnecting, then just ensure it doesn't reconnect
            client->is_reconnect = false;
            break;

        case kMqttState_SendingConnect:
        case kMqttState_Idle:
        case kMqttState_ErrorRetrying:
            // In these states, the socket is already closed
            CleanMqttClient(client, false);
            break;

        default:
        case kMqttState_Running:
        case kMqttState_AwaitingConnect:
            if (client->usp_record_send_queue.head == NULL)
            {
                // Initiate disabling of the connection by sending a disconnect record
                USP_LOG_Debug("%s: Schedule close ready!", __FUNCTION__);
                client->is_reconnect = false;
                DisconnectClient(client);
            }
            break;
    }
}

/*********************************************************************//**
**
** InitClient
**
** Initialises the specified MQTT client structure
**
** \param   client - pointer to MQTT client to initialise
** \param   index - index of the mqtt_verify_callbacks[] used to identify the MQTT client during TLS negotiation
**
** \return  None
**
**************************************************************************/
void InitClient(mqtt_client_t *client, int index)
{
    int i;

    memset(client, 0, sizeof(mqtt_client_t));

    MQTT_InitConnParams(&client->conn_params);
    MQTT_InitConnParams(&client->next_params);

    client->state = kMqttState_Idle;
    client->mosq = NULL;
    client->role = ROLE_DEFAULT;
    client->schedule_reconnect = kScheduledAction_Off;
    client->schedule_close = kScheduledAction_Off;
    client->cert_chain = NULL;
    client->verify_callback = mqtt_verify_callbacks[index];
    client->ssl_ctx = NULL;   // NOTE: The SSL context is created in MQTT_Start()
    client->are_certs_loaded = false;
    client->agent_topic_from_connack = NULL;
    client->disconnect_mid = INVALID_MOSQUITTO_MID;
    client->is_reconnect = false;

    ResetRetryCount(client);

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        InitSubscription(&client->subscriptions[i]);
    }

    InitSubscription(&client->response_subscription);
}

/*********************************************************************//**
**
** InitRetry
**
** Initialise the retry state for an MQTT connection
**
** \param   retry - pointer to retry state of an MQTT connection
**
** \return  None
**
**************************************************************************/
void InitRetry(mqtt_retry_params_t *retry)
{
    memset(retry, 0, sizeof(mqtt_retry_params_t));
}

/*********************************************************************//**
**
** ResetRetryCount
**
** Resets the retry count for the specified MQTT client
**
** \param   client - pointer to MQTT client
**
** \return  None
**
**************************************************************************/
void ResetRetryCount(mqtt_client_t* client)
{
    if (client)
    {
        client->retry_time = 0;
        client->retry_count = 0;
    }
}

/*********************************************************************//**
**
** DestroyClient
**
** Frees all member variables in the specified MQTT client structure
** and free the associated libmosquitto context
** NOTE: This function is only called as part of graceful shutdown of the Agent
**
** \param   client - pointer to MQTT client whose cert chain is to be freed
**
** \return  None
**
**************************************************************************/
void DestroyClient(mqtt_client_t *client)
{
    int i;

    MQTT_DestroyConnParams(&client->conn_params);
    MQTT_DestroyConnParams(&client->next_params);

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        MQTT_SubscriptionDestroy(&client->subscriptions[i]);
    }

    MQTT_SubscriptionDestroy(&client->response_subscription);

    FreeMqttClientCertChain(client);

    // Free the mosquitto context
    if (client->mosq != NULL)
    {
        mosquitto_destroy(client->mosq);
        client->mosq = NULL;
    }

    // Free the SSL context
    if (client->ssl_ctx)
    {
        SSL_CTX_free(client->ssl_ctx);
        client->ssl_ctx = NULL;
        client->are_certs_loaded = false;
    }

    // NOTE: Following is not stricly necessary, and we do not have to set client->conn_params.instance to INVALID,
    // since this function is only called when shutting down the USP Agent
    memset(client, 0, sizeof(mqtt_client_t));
}

/*********************************************************************//**
**
** CleanMqttClient
**
** Frees all dynamically allocated member variables of the MQTT client structure
** (apart from those protected by the 'is_reconnect' flag)
**
** \param   client - pointer to MQTT client to free all member variables of
** \param   is_reconnect - Set if this function is called as part of a reconnect sequence
**                         (in which case the send queue is not purged and the next_params are not freed)
**
** \return  None
**
**************************************************************************/
void CleanMqttClient(mqtt_client_t *client, bool is_reconnect)
{
    int sock;

    // NOTE: It is expected that this function is called only when we know that no socket is open
    // If a socket is still open, it is not safe to forcibly close it here, as that creates a SIGPIPE exception when libmosquitto accesses the socket next
    // So just log the case. The socket will be closed when a reconnect happens on this mosquitto context
    sock = mosquitto_socket(client->mosq);
    if (sock != -1)
    {
        USP_LOG_Error("%s: Socket %d should have been closed by libmosquitto, but wasn't", __FUNCTION__, sock);
    }

    // Always ensure the cert chain and current connection params are freed
    FreeMqttClientCertChain(client);
    MQTT_DestroyConnParams(&client->conn_params);

    // Reset state variables which do not have to persist through a reconnect sequence
    client->schedule_reconnect = kScheduledAction_Off;
    client->schedule_close = kScheduledAction_Off;
    client->disconnect_mid = INVALID_MOSQUITTO_MID;
    USP_SAFE_FREE(client->agent_topic_from_connack);
    client->retry_time = 0;

    MoveState(&client->state, kMqttState_Idle, "Disable Client");

    // Exit if this function is not being called as part of a reconnect sequence
    if (is_reconnect)
    {
        return;
    }

    // If the code gets here, this function is being called as part of a reconnect sequence
    // So purge the send queue
    while (client->usp_record_send_queue.head)
    {
        PopClientUspQueue(client);
    }

    // Free the next_params
    MQTT_DestroyConnParams(&client->next_params);

    // Mark the structure as 'not-in-use'
    client->conn_params.instance = INVALID;
    client->is_reconnect = false;
}

/*********************************************************************//**
**
** FreeMqttClientCertChain
**
** Ensures that the cert chain is freed
**
** \param   client - pointer to MQTT client whose cert chain is to be freed
**
** \return  None
**
**************************************************************************/
void FreeMqttClientCertChain(mqtt_client_t *client)
{
    if (client->cert_chain != NULL)
    {
        sk_X509_pop_free(client->cert_chain, X509_free);
        client->cert_chain = NULL;
    }
}

/*********************************************************************//**
**
** InitSubscription
**
** Initialise the specified MQTT subscription
**
** \param   sub - pointer to MQTT subscription to initialise
**
** \return  None
**
**************************************************************************/
void InitSubscription(mqtt_subscription_t *sub)
{
    memset(sub, 0, sizeof(mqtt_subscription_t));
    sub->qos = kMqttQos_Default;
    sub->enabled = false;
    sub->mid = INVALID;
    sub->state = kMqttSubState_Unsubscribed;
}

/*********************************************************************//**
**
** EnableMqttClient
**
** Starts an MQTT client connection
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int EnableMqttClient(mqtt_client_t* client)
{
    mqtt_subscription_t *resp_sub;

    // Clear state variables that get reset each connection attempt
    // Note: Do not clear state variables that need to persist through retry
    USP_ASSERT(client != NULL);
    client->schedule_reconnect = kScheduledAction_Off;
    client->schedule_close = kScheduledAction_Off;
    FreeMqttClientCertChain(client);
    client->disconnect_mid = INVALID_MOSQUITTO_MID;
    USP_SAFE_FREE(client->agent_topic_from_connack);
    client->retry_time = 0;

    // Initialise the agent's response topic
    // NOTE: The agent's response topic stored in response_subscription may be NULL, if not configured in Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured
    resp_sub = &client->response_subscription;
    MQTT_SubscriptionDestroy(resp_sub);
    memset(resp_sub, 0, sizeof(mqtt_subscription_t));
    resp_sub->qos = kMqttQos_Default;
    resp_sub->enabled = true;
    resp_sub->topic = USP_STRDUP(client->conn_params.response_topic);

    int err = EnableMosquitto(client);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to enable client.", __FUNCTION__);
    }
    else
    {
        // Start the connection!
        MoveState(&client->state, kMqttState_SendingConnect, "Starting Connection");
    }

    return err;
}

/*********************************************************************//**
**
** EnableMosquitto
**
** Create a mosquitto context for the specified MQTT client
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int EnableMosquitto(mqtt_client_t *client)
{
    // Create a new mosquitto client instance
    // This takes the instance as the (void *) obj argument
    // Allowing us to use the instance number to identify any callbacks
    if (client->mosq != NULL)
    {
        // Destroy the mosquitto client
        // Will be regenerated with the mosquitto_new later
        mosquitto_destroy(client->mosq);
        client->mosq = NULL;
    }

    bool clean = client->conn_params.clean_session; // v3

    // Use clean_start (v5) flag instead
    if (client->conn_params.version == kMqttProtocol_5_0)
    {
        clean = client->conn_params.clean_start;
    }

    char* client_id = NULL;

    if (client->conn_params.client_id == NULL || strlen(client->conn_params.client_id) == 0)
    {
        if (client->conn_params.version != kMqttProtocol_5_0)
        {
            USP_LOG_Debug("%s: Client id is null or 0 length, overriding with endpoint", __FUNCTION__);
            USP_SAFE_FREE(client->conn_params.client_id);
            client->conn_params.client_id = USP_STRDUP(DEVICE_LOCAL_AGENT_GetEndpointID());

            // Got to make sure we do have a client id in v3
            USP_ASSERT(strlen(client->conn_params.client_id) > 0);

            client_id = client->conn_params.client_id;
        }
    }
    else
    {
        client_id = client->conn_params.client_id;
    }

    // If there's no client_id, we wil be requesting a new one,
    // Therefore, we must use a clean request.
    if (client_id == NULL)
    {
        clean = true;
    }

    client->mosq = mosquitto_new((const char*)client_id, clean, &client->conn_params.instance);

    if (client->mosq == NULL)
    {
        // check errno
        USP_LOG_Error("%s: Failed to allocate a new mosquitto client. errno %d", __FUNCTION__, errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Ensure we have the thread-safe implementation of mosquitto
    mosquitto_threaded_set(client->mosq, true);

    // Set the mosquitto version in use
    int mosquitto_version = 0;
    if (ConvertToMosquittoVersion(client->conn_params.version, &mosquitto_version) != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to get the mosquitto version from provided client version", __FUNCTION__);
        return USP_ERR_UNSUPPORTED_PARAM;
    }

    if (mosquitto_int_option(client->mosq, MOSQ_OPT_PROTOCOL_VERSION, mosquitto_version) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to set mosquitto version %d", __FUNCTION__, mosquitto_version);
        return USP_ERR_UNSUPPORTED_PARAM;
    }

    SetupCallbacks(client);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SetupCallbacks
**
** Registers the libmosquitto callbacks for an MQTT client
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void SetupCallbacks(mqtt_client_t *client)
{
    // Register all the generic callbacks
    // Bear in mind that these callbacks are used for every client
    // Need v5 and v3.1/3.1.1 callbacks registered where needed
    mosquitto_log_callback_set(client->mosq, LogCallback);
    mosquitto_disconnect_callback_set(client->mosq, DisconnectCallback);

    if (client->conn_params.version == kMqttProtocol_5_0)
    {
        mosquitto_connect_v5_callback_set(client->mosq, ConnectV5Callback);
        mosquitto_subscribe_v5_callback_set(client->mosq, SubscribeV5Callback);
        mosquitto_unsubscribe_v5_callback_set(client->mosq, UnsubscribeV5Callback);
        mosquitto_publish_v5_callback_set(client->mosq, PublishV5Callback);
        mosquitto_message_v5_callback_set(client->mosq, MessageV5Callback);
    }
    else
    {
        mosquitto_connect_callback_set(client->mosq, ConnectCallback);
        mosquitto_subscribe_callback_set(client->mosq, SubscribeCallback);
        mosquitto_unsubscribe_callback_set(client->mosq, UnsubscribeCallback);
        mosquitto_publish_callback_set(client->mosq, PublishCallback);
        mosquitto_message_callback_set(client->mosq, MessageCallback);
    }
}

/*********************************************************************//**
**
** QueueUspConnectRecord_MQTT
**
** Adds the USP connect record at the front of the queue, ensuring that there is only one connect record in the queue
**
** \param   client - pointer to MQTT client to send the connect record on
** \param   msi - pointer to content to send
**                NOTE: Ownership of the payload buffer passes to this function
** \param   controller_topic - topic to send the record to
**
** \return  None
**
**************************************************************************/
void QueueUspConnectRecord_MQTT(mqtt_client_t *client, mtp_send_item_t *msi, char *controller_topic)
{
    mqtt_send_item_t *cur_msg;
    mqtt_send_item_t *next_msg;
    mqtt_send_item_t *send_item;
    mtp_content_type_t type;

    // Iterate over USP Records in the queue, removing all stale connect and USP disconnect records
    // A USP connect or USP disconnect record may still be in the queue if the connection failed before the record was fully sent
    cur_msg = (mqtt_send_item_t *) client->usp_record_send_queue.head;
    while (cur_msg != NULL)
    {
        // Save pointer to next message, as we may remove the current message
        next_msg = (mqtt_send_item_t *) cur_msg->link.next;

        // Remove current message if it is a USP connect or USP disconnect record
        type = cur_msg->item.content_type;
        if (IsUspConnectOrDisconnectRecord(type))
        {
            DLLIST_Unlink(&client->usp_record_send_queue, cur_msg);
            USP_SAFE_FREE(cur_msg->item.pbuf);
            USP_SAFE_FREE(cur_msg->topic);
            USP_FREE(cur_msg);
        }

        // Move to next message in the queue
        cur_msg = next_msg;
    }

    // Add the new connect record to the queue
    send_item = USP_MALLOC(sizeof(mqtt_send_item_t));
    send_item->item = *msi;  // NOTE: Ownership of the payload buffer passes to the MQTT message queue

    send_item->topic = USP_STRDUP(controller_topic);
    send_item->qos = client->conn_params.publish_qos;
    send_item->mid = INVALID;

    DLLIST_LinkToHead(&client->usp_record_send_queue, send_item);
}

/*********************************************************************//**
**
** SendQueueHead
**
** Publishes the USP record at the front of the queue
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendQueueHead(mqtt_client_t *client)
{
    int err = USP_ERR_OK;

    // Can't be passed a NULL client
    USP_ASSERT(client != NULL);

    mqtt_state_t state = client->state;
    if (state == kMqttState_Running)
    {
        mqtt_send_item_t * q_msg = (mqtt_send_item_t *) client->usp_record_send_queue.head;

        // Check the queue head is ok
        if (q_msg == NULL)
        {
            USP_LOG_Error("%s: Can't send NULL head", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }

        err = Publish(client, q_msg);
    }
    else
    {
        USP_LOG_Error("%s: Incorrect state for sending messages %s", __FUNCTION__, mqtt_state_names[state]);
        err = USP_ERR_INTERNAL_ERROR;
    }

    return err;
}

/*********************************************************************//**
**
** Connect
**
** Kick off connecting the specified MQTT client to its configured broker
**
** \param   client - pointer to MQTT client to connect to the broker
**
** \return  None
**
**************************************************************************/
void Connect(mqtt_client_t *client)
{
    int err = USP_ERR_OK;

    // Start the MQTT Connect
    err = PerformMqttClientConnect(client);

    // Exit if failed to connect
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    err = USP_ERR_OK;

exit:
    // Move to next state, based on result of the connect
    if (err == USP_ERR_OK)
    {
        MoveState(&client->state, kMqttState_AwaitingConnect, "Connect sent");
    }
    else
    {
        HandleMqttError(client, kMqttFailure_Connect, "Failed to connect to client");
    }
}

/*********************************************************************//**
**
** PerformMqttClientConnect
**
** Attempt to TCP connect the specified client to its configured broker
**
** \param   client - pointer to MQTT client to connect to the broker
**
** \return  USP_ERR_INTERNAL_ERROR if failed to connect (and should retry)
**
**************************************************************************/
int PerformMqttClientConnect(mqtt_client_t *client)
{
    int version;
    mosquitto_property *proplist = NULL;
    int mosq_err = MOSQ_ERR_SUCCESS;
    int err = USP_ERR_OK;
    int keep_alive;

    // Exit if unable to configure username/password for this mosquitto context
    if (strlen(client->conn_params.username) > 0)
    {
        if (mosquitto_username_pw_set(client->mosq, client->conn_params.username, client->conn_params.password) != MOSQ_ERR_SUCCESS)
        {
            HandleMqttError(client, kMqttFailure_OtherError, "Failed to set username/password");
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }
    }
    else
    {
        USP_LOG_Warning("%s: No username found", __FUNCTION__);
    }

    // Exit if unable to configure encryption for this mosquitto context
    if (client->conn_params.ts_protocol == kMqttTSprotocol_tls)
    {
        USP_LOG_Debug("%s: Enabling encryption for MQTT client", __FUNCTION__);
        err = ConnectSetEncryption(client);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to set encryption when requested - terminating", __FUNCTION__);
            HandleMqttError(client, kMqttFailure_Misconfigured, "Failed to set SSL");
            goto exit;
        }
    }

    // Create all properties required for the connection (MQTTv5 only)
    version = client->conn_params.version;
    if (version == kMqttProtocol_5_0)
    {
        if (AddConnectProperties(&proplist) != USP_ERR_OK)
        {
            err = USP_ERR_INTERNAL_ERROR;
            goto exit;
        }
    }

    // Calculate the keep alive period to pass to libmosquitto. We might need to alter this, because libmosquitto does not support keep alive < 5 seconds
    keep_alive = client->conn_params.keepalive;
    if (keep_alive == 0)
    {
        keep_alive = 60*60*18;  // Set to 18 hours which is the largest that libmosquitto accepts (it truncates the arg to uint16 internally)
    }
    else if (keep_alive < 5)
    {
        keep_alive = 5;
    }

    // Release the access mutex temporarily whilst performing the connect call
    // We do this to prevent the data model thread from potentially being blocked, whilst the connect call is taking place
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

    // Perform the TCP connect
    // NOTE: TCP connect can block for around 2 minutes if the broker does not respond to the TCP handshake
    if (version == kMqttProtocol_5_0)
    {
        mosq_err = mosquitto_connect_bind_v5(client->mosq, client->conn_params.host, client->conn_params.port,
                                             keep_alive, NULL, proplist);
    }
    else
    {
        mosq_err = mosquitto_connect(client->mosq, client->conn_params.host, client->conn_params.port,
                                     keep_alive);
    }

    // Take the access mutex again
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Exit if failed to connect
    if (mosq_err != MOSQ_ERR_SUCCESS)
    {
        char *version_str = (version == kMqttProtocol_5_0) ? "v5" : "v3.1.1";
        USP_LOG_Error("%s: Failed to connect %s with %s (%d)", __FUNCTION__, version_str, mosquitto_strerror(mosq_err), mosq_err);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

exit:
    // Free the connect properties (MQTTv5 only)
    if (proplist != NULL)
    {
        mosquitto_property_free_all(&proplist);
    }

    return err;
}

/*********************************************************************//**
**
** ConnectSetEncryption
**
** Sets up the truststore and all SSL callbacks for the specified MQTT client
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ConnectSetEncryption(mqtt_client_t *client)
{
    USP_ASSERT(client->ssl_ctx != NULL);
    int err;

    // Load the trust store certs into the context. This is performed here, rather than in MQTT_start() in order
    // to minimise memory usage, since most of the MQTT client structures will typically be unused
    // NOTE: The 'are_certs_loaded' flag ensures that the certs are loaded only once, rather than every reconnect
    if (client->are_certs_loaded == false)
    {
        // Exit if unable to load the trust store
        err = DEVICE_SECURITY_LoadTrustStore(client->ssl_ctx, SSL_VERIFY_PEER, client->verify_callback);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to load the trust store", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }

        USP_LOG_Debug("%s: Loaded the trust store!", __FUNCTION__);
        client->are_certs_loaded = true;
    }

    // Enable hostname validation in the SSL context
    err = DEVICE_SECURITY_AddCertHostnameValidationCtx(client->ssl_ctx, client->conn_params.host,
                                                        strlen(client->conn_params.host));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Adding SSL hostname validation failed.", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // In libmosquitto 1.6.14 onwards, by default libmosquitto uses it's own SSL context.
    // So instruct libmosquitto to use SSL context owned by this MTP containing the right certs
#if LIBMOSQUITTO_VERSION_NUMBER >= 1006014

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error "Libmosquitto does not support MOSQ_OPT_SSL_CTX_WITH_DEFAULTS for OpenSSL revisions < 1.1"
#endif
    if (mosquitto_int_option(client->mosq, MOSQ_OPT_SSL_CTX_WITH_DEFAULTS, false) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to set mosquitto ssl default ctx as false", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

    // Set TLS using SSL_CTX in lib mosquitto
    if(mosquitto_opts_set(client->mosq, MOSQ_OPT_SSL_CTX, client->ssl_ctx) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to set ssl_ctx into mosquitto", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ConnectCallback
**
** Called by Libmosquitto when the CONNACK packet is received on an MQTTv3 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   result - reason code from the CONNACK
**
** \return  None
**
**************************************************************************/
void ConnectCallback(struct mosquitto *mosq, void *userdata, int result)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    int instance = *(int*)userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    if ((client->state != kMqttState_AwaitingConnect) && (client->state != kMqttState_ErrorRetrying))
    {
        USP_LOG_Error("%s: Wrong state: %s for client %d", __FUNCTION__, mqtt_state_names[client->state], instance);
    }
    else if (result != 0)
    {
        USP_LOG_Error("%s: Bad result (%d) in connect callback", __FUNCTION__, result);

        HandleMqttError(client, kMqttFailure_OtherError, "Error in connect callback");
    }
    else
    {
        if (client->cert_chain != NULL)
        {
            int err = DEVICE_SECURITY_GetControllerTrust(client->cert_chain, &client->role);
            if (err != USP_ERR_OK)
            {
                USP_LOG_Error("%s: Failed to get the controller trust with err: %d", __FUNCTION__, err);
            }
            else
            {
                USP_LOG_Debug("%s: Successfully got the cert chain!", __FUNCTION__);
            }

            // Free the cert chain, now that we've finished with it
            FreeMqttClientCertChain(client);
        }
        else
        {
            USP_LOG_Error("%s: No cert chain, so cannot get controller trust", __FUNCTION__);
        }


        ResetRetryCount(client);

        MoveState(&client->state, kMqttState_Running, "Connect Callback Received");
        SubscribeToAll(client);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** ConnectV5Callback
**
** Called by Libmosquitto when the CONNACK packet is received on an MQTTv5 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   result - reason code from the CONNACK
** \param   flags - unused
** \param   props - properties received in the CONNACK
**
** \return  None
**
**************************************************************************/
void ConnectV5Callback(struct mosquitto *mosq, void *userdata, int result, int flags, const mosquitto_property *props)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Receive the CONNACK here
    // Check the same stuff as the connect callback - but also look at the props values etc
    mqtt_client_t *client = NULL;
    int instance = *(int*)userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    if ((client->state != kMqttState_AwaitingConnect) && (client->state != kMqttState_ErrorRetrying))
    {
        USP_LOG_Error("%s: Wrong state: %s for client %d", __FUNCTION__, mqtt_state_names[client->state], instance);

        HandleMqttError(client, kMqttFailure_OtherError, "State error in connect v5 callback");
    }
    else if (result != 0)
    {
        // R-MQTT.10
        USP_LOG_Error("%s: Bad result (%d) in connect callback", __FUNCTION__, result);

        HandleMqttError(client, kMqttFailure_OtherError, "Error in connect v5 callback");
    }
    else
    {
        if (client->cert_chain != NULL)
        {
            int err = DEVICE_SECURITY_GetControllerTrust(client->cert_chain, &client->role);
            if (err != USP_ERR_OK)
            {
                USP_LOG_Error("%s: Failed to get the controller trust with err: %d", __FUNCTION__, err);
            }
            else
            {
                USP_LOG_Debug("%s: Successfully got the cert chain!", __FUNCTION__);
            }

            // Free the cert chain, now that we've finished with it
            FreeMqttClientCertChain(client);
        }
        else
        {
            USP_LOG_Error("%s: No cert chain, so cannot get controller trust", __FUNCTION__);
        }

        // Pick up client id, as per R-MQTT.9
        char *client_id_ptr = NULL;
        char *response_info_ptr = NULL;
        char *subscribe_topic_ptr = NULL;

        if (mosquitto_property_read_string(props, ASSIGNED_CLIENT_IDENTIFIER,
              &client_id_ptr, false /* skip first */) != NULL)
        {
            USP_LOG_Debug("%s: Received client_id: \"%s\"", __FUNCTION__, client_id_ptr);
            USP_SAFE_FREE(client->conn_params.client_id);
            client->conn_params.client_id = USP_STRDUP(client_id_ptr);
            free(client_id_ptr);
        }

        // Update the agent topic (if received in this CONNACK)
        USP_SAFE_FREE(client->agent_topic_from_connack);
        if (mosquitto_property_read_string(props, RESPONSE_INFORMATION,
              &response_info_ptr, false) != NULL)
        {
            // Then replace the response_topic in subscription with this
            SaveAgentTopicFromConnack(client, response_info_ptr);
            free(response_info_ptr);
        }
        else
        {
            // if no response information, check if it's in the subscribe-topic user prop
            char* userPropName;
            if (mosquitto_property_read_string_pair(props, USER_PROPERTY,
                  &userPropName, &subscribe_topic_ptr, false) != NULL)
            {
                // we only want subscribe-topic user property
                if (strcmp("subscribe-topic", userPropName) == 0)
                {
                    SaveAgentTopicFromConnack(client, subscribe_topic_ptr);
                    free(subscribe_topic_ptr);
                    free(userPropName);
                }
                else
                {
                    // it wasn't in the 1st one, try the next one, set skip 1st to true
                    free(subscribe_topic_ptr);
                    free(userPropName);
                    if (mosquitto_property_read_string_pair(props, USER_PROPERTY,
                       &userPropName, &subscribe_topic_ptr, true) != NULL)
                    {
                        // we only want subscribe-topic user property
                        if (strcmp("subscribe-topic", userPropName) == 0)
                        {
                            SaveAgentTopicFromConnack(client, subscribe_topic_ptr);
                        }
                        free(subscribe_topic_ptr);
                        free(userPropName);
                    }
                }
            }
        }

        USP_LOG_Debug("%s: Received client id \"%s\"", __FUNCTION__, client->conn_params.client_id);

        ResetRetryCount(client);

        MoveState(&client->state, kMqttState_Running, "Connect Callback Received");
        SubscribeToAll(client);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** SaveAgentTopicFromConnack
**
** Saves the agent topic received in the CONNACK into the MQTT client structure
**
** \param   client - pointer to MQTT client structure to update the agent topic in
** \param   agent_topic - value of agent response topic received in the CONNACK
**
** \return  None
**
**************************************************************************/
void SaveAgentTopicFromConnack(mqtt_client_t *client, char *agent_topic)
{
    USP_LOG_Debug("%s: Received agent-topic: \"%s\"", __FUNCTION__, agent_topic);

    // Override agent response topic configured in Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured
    USP_SAFE_FREE(client->response_subscription.topic);
    client->response_subscription.topic = USP_STRDUP(agent_topic);

    // Save the agent response topic received in the CONNACK into the MQTT client structure
    // (so it can be read by Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicDiscovered and Device.MQTT.Client.{i}.ResponseInformation)
    USP_SAFE_FREE(client->agent_topic_from_connack);
    client->agent_topic_from_connack = USP_STRDUP(agent_topic);
}

/*********************************************************************//**
**
** DisconnectClient
**
** Initiates an MQTT disconnect on the specified MQTT client (if still connected)
**
** \param   client - pointer to MQTT client
**
** \return  None
**
**************************************************************************/
void DisconnectClient(mqtt_client_t *client)
{
    int result;
    int sock;

    // Send a disconnect frame, if the socket is still connected
    sock = mosquitto_socket(client->mosq);
    if (sock != -1)
    {
        client->state = kMqttState_AwaitingDisconnect;

        // NOTE: mosquitto_disconnect() may return an error if already disconnected, or
        // if it cannot disconnect, because another thread is currently performing a mosquitto_connect()
        result = mosquitto_disconnect(client->mosq);
        if (result != MOSQ_ERR_SUCCESS)
        {
            USP_LOG_Warning("%s: mosquitto_disconnect() returned error=%d. Ignoring.", __FUNCTION__, result);
        }
    }
}

/*********************************************************************//**
**
** DisconnectCallback
**
** Called by Libmosquitto when the socket has been disconnected (for MQTTv5 and MQTTv3 connections)
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   rc - reason for disconnection
**
** \return  None
**
**************************************************************************/
void DisconnectCallback(struct mosquitto *mosq, void *userdata, int rc)
{
    int sock;
    int instance;
    mqtt_client_t *client;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    // Find client
    instance = *((int *)userdata);
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance", __FUNCTION__);
        goto exit;
    }

    // This function is called by libmosquitto, after the socket has been closed
    // Log an error if this was not the case
    sock = mosquitto_socket(client->mosq);
    if (sock != -1)
    {
        USP_LOG_Error("%s: Socket %d should have been closed by libmosquitto, but wasn't", __FUNCTION__, sock);
    }

    // Mark the MQTT client as disconnected. The actions to perform after thismust be performed in the MQTT MTP thread
    // because they might free the mosquitto context, and that must not be done from this callback (libmosquitto is still using it in the functions which called this)
    USP_LOG_Debug("%s: Disconnected (rc=%d, is_reconnect=%d)\n", __FUNCTION__, rc, client->is_reconnect);
    client->is_disconnected = true;

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** HandleMqttDisconnect
**
** Handles the actions caused by a disconnect occurring
** NOTE: This function must not be called from an MQTT callback, as it may free the libmosquitto context
**
** \param   client - pointer to MQTT client
**
** \return  None
**
**************************************************************************/
void HandleMqttDisconnect(mqtt_client_t *client)
{
    client->is_disconnected = false;

    switch(client->state)
    {
        case kMqttState_ErrorRetrying:
        case kMqttState_Idle:
            // Nothing to do, stay in the same state and ignore the disconnect
            break;

        case kMqttState_AwaitingDisconnect:
            // Reconnect, if this callback was received as part of a reconnect sequence
            HandleMqttReconnectAfterDisconnect(client);
            break;

        default:
        case kMqttState_SendingConnect:
        case kMqttState_AwaitingConnect:
        case kMqttState_Running:
            // Case of broker initiated disconnect, go to retrying
            HandleMqttError(client, kMqttFailure_OtherError, "Force disconnected from broker");
            break;
    }
}

/*********************************************************************//**
**
** HandleMqttReconnectAfterDisconnect
**
** Called to ensure that the MQTT client state indicates disconnected,
** then if the disconnect was part of a reconnect sequence, initiate the reconnect
**
** \param   client - pointer to MQTT client
**
** \return  None
**
**************************************************************************/
void HandleMqttReconnectAfterDisconnect(mqtt_client_t *client)
{
    // We have performed an agent-initiated disconnect from the broker
    // So reconnect, if this callback was received as part of a reconnect sequence
    CleanMqttClient(client, client->is_reconnect);
    if (client->is_reconnect)
    {
        client->is_reconnect = false;
        ParamReplace(&client->conn_params, &client->next_params);
        EnableMqttClient(client);
    }
}

/*********************************************************************//**
**
** Subscribe
**
** Initiates an MQTT SUBSCRIBE for the specified topic
**
** \param   client - pointer to MQTT client
** \param   sub - subscription to subscribe to
** \param   is_agent_topic - Set if the subscription is the agent's response topic
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Subscribe(mqtt_client_t *client, mqtt_subscription_t *sub, bool is_agent_topic)
{
    USP_ASSERT(client != NULL);
    USP_ASSERT(sub != NULL);

    if (sub->topic == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    int err = USP_ERR_OK;
    int version = client->conn_params.version;

    sub->state = kMqttSubState_Subscribing;
    if (version == kMqttProtocol_5_0)
    {
        USP_LOG_Debug("%s: Sending subscribe to %s %d %d", __FUNCTION__, sub->topic, sub->mid, sub->qos);
        err = SubscribeV5(client, sub);
    }
    else
    {
        char wildcarded_topic[MAX_TOPIC_LEN+2];  // Plus 2 to include the wildcard
        char *topic = sub->topic;

        // If this subscription is the agent's response topic, then for MQTTv3.x, we must subscribe to the wildcarded version
        // because the controller publishes to topics of the form 'agent-topic/reply-to=controller-topic' as per R-MQTT.24
        if (is_agent_topic)
        {
            USP_SNPRINTF(wildcarded_topic, sizeof(wildcarded_topic), "%s/#", sub->topic);
            topic = wildcarded_topic;
        }

        USP_LOG_Debug("%s: Sending subscribe to %s %d %d", __FUNCTION__, topic, sub->mid, sub->qos);
        if (mosquitto_subscribe(client->mosq, &sub->mid, topic, sub->qos) != MOSQ_ERR_SUCCESS)
        {
            USP_LOG_Error("%s: Failed to subscribe to %s", __FUNCTION__, sub->topic);
            err = USP_ERR_INTERNAL_ERROR;
        }
    }

    return err;
}

/*********************************************************************//**
**
** SubscribeV5
**
** Initiates an MQTTv5 SUBSCRIBE for the specified topic
**
** \param   client - pointer to MQTT client
** \param   sub - subscription to subscribe to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SubscribeV5(mqtt_client_t *client, mqtt_subscription_t *sub)
{
    int err = USP_ERR_OK;
    mosquitto_property *proplist = NULL;

    if (AddUserProperties(&proplist) != USP_ERR_OK)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

    if (mosquitto_subscribe_v5(client->mosq, &sub->mid, sub->topic, sub->qos,
                0 /*Options, default */, proplist) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to subscribe to %s with v5", __FUNCTION__, sub->topic);

        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

error:
    if (proplist)
    {
        // Free prop list now we're finished with it.
        mosquitto_property_free_all(&proplist);
    }
    return err;
}

/*********************************************************************//**
**
** SubscribeToAll
**
** Subscribes to all topics on the specified MQTT client connection
**
** \param   client - pointer to MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void SubscribeToAll(mqtt_client_t *client)
{
    int i;
    char buf[128];

    // Exit if no agent response topic configured (or set by the CONNACK)
    if ((client->response_subscription.topic==NULL) || (client->response_subscription.topic[0] == '\0'))
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: No response topic configured (or set by the CONNACK)", __FUNCTION__);
        HandleMqttError(client, kMqttFailure_Misconfigured, buf);
        return;
    }

    // Now we have the proplist, send the subscribe
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        mqtt_subscription_t *sub = &client->subscriptions[i];
        if (sub->enabled == true)
        {
            if (sub->topic == NULL)
            {
                USP_LOG_Error("%s: No to subscribe to, skipping sub %d", __FUNCTION__, i);
                continue;
            }

            if (Subscribe(client, sub, false) != USP_ERR_OK)
            {
                USP_SNPRINTF(buf, sizeof(buf), "%s: mosquitto_subscribe() failed for topic=%s", __FUNCTION__, sub->topic);
                HandleMqttError(client, kMqttFailure_OtherError, buf);
            }
        }
    }

    // Subscribe to response topic too
    if (Subscribe(client, &client->response_subscription, true) != USP_ERR_OK)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: mosquitto_subscribe() failed for agent's response topic=%s", __FUNCTION__, client->response_subscription.topic);
        HandleMqttError(client, kMqttFailure_OtherError, buf);
    }

    // Let the DM know we're ready for sending messages and instruct it to send a USP Connect record
    // NOTE: client->response_subscription.topic will contain either the value configured in Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured
    //       or the value received in the CONNACK (for MQTTv5)
    DM_EXEC_PostMqttHandshakeComplete(client->conn_params.instance, client->conn_params.version, client->response_subscription.topic, client->role);
}

/*********************************************************************//**
**
** SubscribeCallback
**
** Called by Libmosquitto when the SUBACK packet is received on an MQTTv3 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the topic that was subscribed to
** \param   qos_count - unused
** \param   granted_qos - unused
**
** \return  None
**
**************************************************************************/
void SubscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int* granted_qos)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    int instance = *(int*) userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    // Find the subscriber mid
    mqtt_subscription_t *sub = FindSubscriptionByMid(client, mid, NULL);

    if (sub == NULL)
    {
        USP_LOG_Error("%s: Failed to find subscription with mid %d", __FUNCTION__, mid);
        goto exit;
    }

    if (sub->state != kMqttSubState_Subscribing)
    {
        USP_LOG_Error("%s: Wrong state %d", __FUNCTION__, sub->state);
    }
    else
    {
        sub->state = kMqttSubState_Subscribed;
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

}

/*********************************************************************//**
**
** SubscribeV5Callback
**
** Called by Libmosquitto when the SUBACK packet is received in an MQTTv5 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the topic that was subscribed to
** \param   qos_count - unused
** \param   granted_qos - unused
** \param   props - unused
**
** \return  None
**
**************************************************************************/
void SubscribeV5Callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int* granted_qos,
        const mosquitto_property* props)
{
    // Basically the same as the 3.1.1 callback..
    // Mutex will be taken in the function below.
    SubscribeCallback(mosq, userdata, mid, qos_count, granted_qos);
}

/*********************************************************************//**
**
** Unsubscribe
**
** Unsubscribes from the specified topic
**
** \param   client - pointer to MQTT client
** \param   sub - subscription to unsubscribe from
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Unsubscribe(mqtt_client_t *client, mqtt_subscription_t *sub)
{
    USP_ASSERT(client != NULL);
    USP_ASSERT(sub != NULL);

    if (sub->topic == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    int version = client->conn_params.version;
    int err = USP_ERR_OK;

    sub->state = kMqttSubState_Unsubscribing;
    if (version == kMqttProtocol_5_0)
    {
        err = UnsubscribeV5(client, sub);
    }
    else
    {
        if (mosquitto_unsubscribe(client->mosq, &sub->mid, sub->topic) != MOSQ_ERR_SUCCESS)
        {
            USP_LOG_Error("%s: Failed to subscribe to %s", __FUNCTION__, sub->topic);
            err = USP_ERR_INTERNAL_ERROR;
        }
    }

    return err;
}

/*********************************************************************//**
**
** UnsubscribeV5
**
** Unsubscribes from the specified topic for an MQTTv5 connection
**
** \param   client - pointer to MQTT client
** \param   sub - subscription to unsubscribe from
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UnsubscribeV5(mqtt_client_t *client, mqtt_subscription_t *sub)
{
    mosquitto_property *proplist = NULL;
    int err = USP_ERR_OK;

    if (AddUserProperties(&proplist) != USP_ERR_OK)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

    if (mosquitto_unsubscribe_v5(client->mosq, &sub->mid, sub->topic, proplist) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to unsubscribe to %s with v5", __FUNCTION__, sub->topic);
        err = USP_ERR_INTERNAL_ERROR;
    }

error:
    if (proplist)
    {
        // Free all properties now that we're done with them.
        mosquitto_property_free_all(&proplist);
    }
    return err;
}

/*********************************************************************//**
**
** UnsubscribeCallback
**
** Called by Libmosquitto when the UNSUBACK packet is received
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the topic that was unsubscribed from
**
** \return  None
**
**************************************************************************/
void UnsubscribeCallback(struct mosquitto *mosq, void *userdata, int mid )
{
    bool is_agent_topic;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    int instance = *(int*) userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        return;
    }

    // Find the subscriber mid
    mqtt_subscription_t *sub = FindSubscriptionByMid(client, mid, &is_agent_topic);

    if (sub == NULL)
    {
        USP_LOG_Error("%s: Failed to find subscription with mid %d", __FUNCTION__, mid);
        goto exit;
    }

    if (sub->state == kMqttSubState_Unsubscribing)
    {
        sub->state = kMqttSubState_Unsubscribed;
    }
    else if(sub->state == kMqttSubState_Resubscribing)
    {
        if (Subscribe(client, sub, is_agent_topic) != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Re-Subscribe topic failed", __FUNCTION__);
        }
        else
        {
            USP_LOG_Debug("%s: Resubscribing", __FUNCTION__);
        }
    }
    else
    {
        USP_LOG_Error("%s: Wrong state %d", __FUNCTION__, sub->state);
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

}

/*********************************************************************//**
**
** UnsubscribeV5Callback
**
** Called by Libmosquitto when the UNSUBACK packet is received in an MQTTv5 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the topic that was unsubscribed from
** \param   props - unused
**
** \return  None
**
**************************************************************************/
void UnsubscribeV5Callback(struct mosquitto *mosq, void *userdata, int mid, const mosquitto_property* props)
{
    // Basically the same as the 3.1.1 callback..
    // Mutex will be taken in the function below.
    UnsubscribeCallback(mosq, userdata, mid);
}

/*********************************************************************//**
**
** Publish
**
** Initiates an MQTT PUBLISH for the specified USP record
**
** \param   client - pointer to MQTT client
** \param   msg - pointer to message to send
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Publish(mqtt_client_t *client, mqtt_send_item_t *msg)
{
    int err = USP_ERR_OK;

    USP_ASSERT(client != NULL);
    USP_ASSERT(msg != NULL);
    USP_ASSERT(msg->topic != NULL);

    MSG_HANDLER_LogMessageToSend(&msg->item, kMtpProtocol_MQTT, client->conn_params.host, NULL);

    int version = client->conn_params.version;
    if (version == kMqttProtocol_5_0)
    {
        err = PublishV5(client, msg);
    }
    else
    {
        char escaped_agent_topic[MAX_TOPIC_LEN];
        char topic[2*MAX_TOPIC_LEN+10];             // Plus 10 to allow for '/reply-to='
        USP_ASSERT((client->response_subscription.topic!= NULL) && (client->response_subscription.topic[0] != '\0')); // SubscribeToAll() should have prevented the code getting here
        TEXT_UTILS_ReplaceCharInString(client->response_subscription.topic, '/', "%2F", escaped_agent_topic, sizeof(escaped_agent_topic));
        USP_SNPRINTF(topic, sizeof(topic), "%s/reply-to=%s", msg->topic, escaped_agent_topic);

        if (mosquitto_publish(client->mosq, &msg->mid, topic, msg->item.pbuf_len, msg->item.pbuf, msg->qos, false /*retain*/) != MOSQ_ERR_SUCCESS)
        {
            USP_LOG_Error("%s: Failed to publish to v3.1.1. Params:\n MID:%d\n topic:%s\n msg->qos:%d\n", __FUNCTION__, msg->mid, msg->topic, msg->qos);
            err = USP_ERR_INTERNAL_ERROR;
        }
    }

    return err;
}

/*********************************************************************//**
**
** PublishV5
**
** Initiates an MQTTv5 PUBLISH for the specified USP record
**
** \param   client - pointer to MQTT client
** \param   msg - pointer to message to send
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PublishV5(mqtt_client_t *client, mqtt_send_item_t *msg)
{
    int err = USP_ERR_OK;
    mosquitto_property *proplist = NULL;

    // Setup proplist flags for v5
    if (mosquitto_property_add_string(&proplist, CONTENT_TYPE, "usp.msg") != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to add content type string", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

    USP_ASSERT((client->response_subscription.topic!= NULL) && (client->response_subscription.topic[0] != '\0')); // SubscribeToAll() should have prevented the code getting here
    if (mosquitto_property_add_string(&proplist, RESPONSE_TOPIC, client->response_subscription.topic) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to add response topic string", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

    // Check all properties
    if (mosquitto_property_check_all(PUBLISH, proplist) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: property check failed.", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

    int mosq_err = mosquitto_publish_v5(client->mosq, &msg->mid, msg->topic, msg->item.pbuf_len, msg->item.pbuf, msg->qos, false /* retain */, proplist);
    if (mosq_err != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to publish to v5 with error %d", __FUNCTION__, mosq_err);
        err = USP_ERR_INTERNAL_ERROR;
        goto error;
    }

error:
    // Free all properties now we're done with them.
    mosquitto_property_free_all(&proplist);
    return err;
}

/*********************************************************************//**
**
** PublishCallback
**
** Called by Libmosquitto when the publish has completed (according to QoS) on an MQTTv3 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the message that was sent
**
** \return  None
**
**************************************************************************/
void PublishCallback(struct mosquitto* mosq, void *userdata, int mid /*message id*/)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    int instance = *(int*) userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    if (client->state == kMqttState_Running)
    {
        USP_LOG_Debug("%s: Sent MID %d", __FUNCTION__, mid);
    }
    else
    {
        USP_LOG_Warning("%s: Received publish in wrong state: %s", __FUNCTION__, mqtt_state_names[client->state]);
    }

    // If libmosquitto has just sent a USP disconnect record, then force an actual disconnect
    if (mid == client->disconnect_mid)
    {
        HandleMqttError(client, kMqttFailure_OtherError, "USP Disconnect Record sent");
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

}

/*********************************************************************//**
**
** PublishV5Callback
**
** Called by Libmosquitto when the MQTTv5 publish has completed (according to QoS) on an MQTTv5 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   mid - libmosquitto message id identifying the message that was sent
** \param   reason_code - unused
** \param   props - unused
**
** \return  None
**
**************************************************************************/
void PublishV5Callback(struct mosquitto *mosq, void *userdata, int mid, int reason_code, const mosquitto_property *props)
{
    // Mutex taken in PublishCallback.
    PublishCallback(mosq, userdata, mid);
}

/*********************************************************************//**
**
** MessageCallback
**
** Called by Libmosquitto when a message has been received on an MQTTv3 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   message - pointer to libmosquitto message object
**
** \return  None
**
**************************************************************************/
void MessageCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    int instance;
    mqtt_client_t *client;
    char buf[MAX_TOPIC_LEN];
    char *reply_to_topic;

    OS_UTILS_LockMutex(&mqtt_access_mutex);

    instance = *(int*) userdata;
    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    if (message == NULL)
    {
        USP_LOG_Error("%s: NULL message", __FUNCTION__);
    }
    else if (!message->payloadlen)
    {
        USP_LOG_Warning("%s: Empty message on topic: %s", message->topic, __FUNCTION__);
    }
    else
    {
        USP_LOG_Info("%s: Received Message: Topic: '%s' PayloadLength: %d bytes", __FUNCTION__,
                     message->topic, message->payloadlen);

        if (client->state == kMqttState_Running)
        {
            // NOTE: If no reply-to topic is found, then reply_to_topic will be returned as NULL
            reply_to_topic = FindReplyToTopic(message->topic, buf, sizeof(buf));
            ReceiveMqttMessage(client, message, reply_to_topic);
        }
        else
        {
            USP_LOG_Warning("%s: Ignoring message received in incorrect state", __FUNCTION__);
        }
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);
}

/*********************************************************************//**
**
** MessageV5Callback
**
** Called by Libmosquitto when a message has been received on an MQTTv5 connection
**
** \param   mosq - libmosquitto context
** \param   userdata - pointer to Instance in Device.MQTT.Client.{i}
** \param   message - pointer to libmosquitto message object
** \param   props - pointer to MQTT properties carried with the message
**
** \return  None
**
**************************************************************************/
void MessageV5Callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message, const mosquitto_property *props)
{
    OS_UTILS_LockMutex(&mqtt_access_mutex);

    mqtt_client_t *client = NULL;
    int instance = *(int*) userdata;

    client = FindMqttClientByInstance(instance);
    if (client == NULL)
    {
        USP_LOG_Error("%s: Failed to find client instance by id %d", __FUNCTION__, instance);
        goto exit;
    }

    if (message == NULL)
    {
        USP_LOG_Error("%s: NULL message", __FUNCTION__);
    }
    else if (!message->payloadlen)
    {
        USP_LOG_Warning("%s: Empty message on topic: %s", message->topic, __FUNCTION__);
    }
    else
    {
        USP_LOG_Info("%s: Received Message: Topic: '%s' PayloadLength: %d bytes", __FUNCTION__,
                     message->topic, message->payloadlen);

        if (client->state == kMqttState_Running)
        {
            // Now we have a message from somewhere
            char *response_info_ptr = NULL;

            if (mosquitto_property_read_string(props, RESPONSE_TOPIC,
                    &response_info_ptr, false) == NULL)
            {
                USP_LOG_Debug("%s: Failed to read response topic in message info: \"%s\"\n", __FUNCTION__, response_info_ptr);
            }

            ReceiveMqttMessage(client, message, response_info_ptr);

            if (response_info_ptr != NULL)
            {
                free(response_info_ptr);
            }
        }
        else
        {
            USP_LOG_Warning("%s: Ignoring message received in incorrect state", __FUNCTION__);
        }
    }

exit:
    OS_UTILS_UnlockMutex(&mqtt_access_mutex);

}

/*********************************************************************//**
**
** ReceiveMqttMessage
**
** Called after receiving a message, to send it to the data model thread for processing
**
** \param   client - pointer to MQTT client
** \param   message - libmosquitto message object, whose payload is the USP record
** \param   response_topic - The topic to send the USP response to this message.
**                           NOTE: This may be NULL if none could be determined.
**
** \return  None
**
**************************************************************************/
void ReceiveMqttMessage(mqtt_client_t *client, const struct mosquitto_message *message, char *response_topic)
{
    mtp_reply_to_t mrt = {0};
    mrt.protocol = kMtpProtocol_MQTT;
    mrt.mqtt_instance = client->conn_params.instance;
    if (response_topic != NULL)
    {
        mrt.is_reply_to_specified = true;
        mrt.mqtt_topic = response_topic;
    }

    // Message may not be valid USP
    DM_EXEC_PostUspRecord(message->payload, message->payloadlen, client->role, &mrt);
}

/*********************************************************************//**
**
** FindReplyToTopic
**
** Parses the specified string, determining the topic to use when sending a reply
**
** \param   publish_topic - topic that the message was published to
** \param   buf - pointer to buffer in which to return the reply-to topic
** \param   len - length of buffer in which to return the reply-to topic
**
** \return  pointer to buffer containing reply-to topic or NULL if no reply-to topic
**          was parsed or the parsed reply-to topic was an empty string
**
**************************************************************************/
char *FindReplyToTopic(char *publish_topic, char *buf, int len)
{
    char *p;

    // Exit if the publish topic did not contain a reply-to topic
    #define REPLY_TO_QUALIFIER  "/reply-to="
    p = strstr(publish_topic, REPLY_TO_QUALIFIER);
    if (p == NULL)
    {
        return NULL;
    }

    // Skip the 'reply-to' qualifier
    p += sizeof(REPLY_TO_QUALIFIER) - 1;        // Minus 1 to exclude trailing NULL

    // Copy the reply-to topic into the return buffer, unescaping any topic slashes
    USP_STRNCPY(buf, p, len);
    TEXT_UTILS_PercentDecodeString(buf);

    // Exit if the reply-to topic is empty
    if (*buf == '\0')
    {
        return NULL;
    }

    return buf;
}

/*********************************************************************//**
**
** LogCallback
**
** Called by Libmosquitto to emit a log message
**
** \param   mosq - unused
** \param   userdata - unused
** \param   level - type of log message
** \param   str - string to log
**
** \return  None
**
**************************************************************************/
void LogCallback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    // Don't need a mutex as nothing is currently being accessed in the MQTT data
    // if anything is added that does, a mutex should be added here.
    switch(level)
    {
        case MOSQ_LOG_ERR:
            USP_LOG_Error("%s; MQTT Error: %s", __FUNCTION__, str);
            break;
        case MOSQ_LOG_WARNING:
            USP_LOG_Warning("%s: MQTT Warning: %s", __FUNCTION__, str);
        case MOSQ_LOG_INFO:
            USP_LOG_Info("%s: MQTT Info: %s", __FUNCTION__, str);
            break;
        case MOSQ_LOG_NOTICE:
        case MOSQ_LOG_DEBUG:
        default:
            USP_LOG_Debug("%s: MQTT Debug: %s", __FUNCTION__, str);
            break;

    }
}

/*********************************************************************//**
**
** HandleMqttError
**
** Called when an error is detected to enter the retry state
** NOTE:
**
** \param   client - pointer to MQTT client
** \param   failure_code - code for cause of error
** \param   message - textual cause of failure
**
** \return  None
**
**************************************************************************/
void HandleMqttError(mqtt_client_t *client, mqtt_failure_t failure_code, const char* message)
{
    time_t cur_time;

    USP_LOG_Debug("%s: Got error: %d, reason: %s, retry_count: %d", __FUNCTION__, failure_code, message, client->retry_count);

    // Exit if an error occurred whilst waiting for an MQTT disconnect to complete (as part of a reconnect sequence)
    // In this case, initiate the reconnect immediately, without waiting for the disconnect callback
    if ((client->state == kMqttState_AwaitingDisconnect) && (client->is_reconnect))
    {
        // NOTE: The socket will not have been closed by libmosquitto in this case, but libmosquitto will close the socket when performing the reconnect
        // NOTE: This case should never be allowed to be called from a libmosquitto callback
        HandleMqttReconnectAfterDisconnect(client);
        return;
    }

    // Flow is:
    // Set failure code - to something useful. Passed in as arg.
    cur_time = time(NULL);
    if (client->failure_code != failure_code)
    {
        client->last_status_change = cur_time;
        client->failure_code = failure_code;
    }

    // Initiate a disconnect, if still connected
    client->is_reconnect = false;
    DisconnectClient(client);

    // Move to retrying state
    MoveState(&client->state, kMqttState_ErrorRetrying, message);

    // Increment retry count
    client->retry_count++;

    // Calculate a wait time until retry
    mqtt_retry_params_t *retry = &client->conn_params.retry;
    time_t wait_time = RETRY_WAIT_Calculate(client->retry_count, retry->connect_retrytime, retry->interval_multiplier);

    if (wait_time > retry->max_interval)
    {
        wait_time = retry->max_interval;
    }

    // Set retry time as time + wait_time
    client->retry_time = cur_time + wait_time - 1;  // Minus 1 because the code adds an extra 1 second delay to the retry time,
                                                    // between setting the state to kMqttState_SendingConnect (in MQTT_UpdateAllSocketSet)
                                                    // and performing the connect (in MQTT_ProcessAllSocketActivity). Minus 1 pre-compensates for this.
}

/*********************************************************************//**
**
** MoveState_Private
**
** Called to change the state of an MQTT connection, logging the cause
**
** \param   state - pointer to variable containing the state to update
** \param   to - value of new state
** \param   event - textual cause for entering the new state
** \param   func - name of caller (for debug)
**
** \return  None
**
**************************************************************************/
void MoveState_Private(mqtt_state_t *state, mqtt_state_t to, const char *event, const char* func)
{
    USP_LOG_Debug("%s (%s): %s --> [[ %s ]] --> %s", func, __FUNCTION__, mqtt_state_names[*state], event, mqtt_state_names[to]);

    *state = to;
}

/*********************************************************************//**
**
** ParamReplace
**
** Called copy the MQTT connection parameters, from one structure to another
**
** \param   dest - destination to copy connection params into
** \param   src - source connection parameters to copy
**
** \return  None
**
**************************************************************************/
void ParamReplace(mqtt_conn_params_t *dest, mqtt_conn_params_t *src)
{
    if (dest == src)
    {
        // This shouldn't really happen,
        // but protect against it anyway
        return;
    }

    // Free the destination, just to simplify
    MQTT_DestroyConnParams(dest);

    *dest = *src;

    // Override all pointers with copies on the heap - must be freed
    dest->host = USP_STRDUP(src->host);
    dest->username = USP_STRDUP(src->username);
    dest->password = USP_STRDUP(src->password);
    dest->topic = USP_STRDUP(src->topic);
    dest->response_topic = USP_STRDUP(src->response_topic);
    dest->client_id = USP_STRDUP(src->client_id);
    dest->name = USP_STRDUP(src->name);
    dest->response_information = USP_STRDUP(src->response_information);
}

/*********************************************************************//**
**
** IsUspRecordInMqttQueue
**
** Determines whether the specified (binary) USP record is in the queue of messages to send
**
** \param   client - pointer to MQTT client
** \param   pbuf - pointer to USP record
** \param   pbuf_len - length of USP record
**
** \return  true if USP record found in queue
**
**************************************************************************/
bool IsUspRecordInMqttQueue(mqtt_client_t *client, unsigned char *pbuf, int pbuf_len)
{
    mqtt_send_item_t *q_msg;

    q_msg = (mqtt_send_item_t *) client->usp_record_send_queue.head;
    while (q_msg != NULL)
    {
        if ((q_msg->item.pbuf_len == pbuf_len) && (memcmp(q_msg->item.pbuf, pbuf, pbuf_len)==0))
        {
            return true;
        }

        q_msg = (mqtt_send_item_t *) q_msg->link.next;
    }

    return false;
}

/*********************************************************************//**
**
** FindUnusedMqttClient_Local
**
** Finds an unused MQTT client
**
** \param   None
**
** \return  pointer to unused MQTT client, or NULL if not found
**
**************************************************************************/
mqtt_client_t *FindUnusedMqttClient_Local()
{
    return FindMqttClientByInstance(INVALID);
}

/*********************************************************************//**
**
** FindMqttClientByInstance
**
** Finds the specified MQTT client, given the instance number
**
** \param   instance - Instance number in Device.MQTT.Client.{i}
**
** \return  pointer to MQTT client, or NULL if not found
**
**************************************************************************/
mqtt_client_t *FindMqttClientByInstance(int instance)
{
    int i;
    mqtt_client_t *client;

    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];

        if (client->conn_params.instance == instance)
        {
            return client;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindMqttSubscriptionByInstance
**
** Finds the specified MQTT subscription, given the client and subscription instance numbers
**
** \param   client - pointer to MQTT client
** \param   subinstance - Subscription instance in Device.MQTT.Client.{i}.Subscription.{i}
**                        or INVALID if we are trying to find an unused entry
**
** \return  pointer to MQTT subscription or NULL if no matching subscription found
**
**************************************************************************/
mqtt_subscription_t *FindMqttSubscriptionByInstance(mqtt_client_t *client, int subinstance)
{
    int i;
    mqtt_subscription_t *subs;

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        subs = &client->subscriptions[i];
        if (subs->instance == subinstance)
        {
            return subs;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindSubscriptionByMid
**
** Finds the mqtt subscription for the specified topic (on the specified client connection)
**
** \param   client - pointer to MQTT client
** \param   topic - topic to match
** \param   is_agent_topic - pointer to variable in which to return whether the returned subscription is the agent response topic subscription
**                           or NULL if the caller does not require this information
**
** \return  pointer to MQTT subscription or NULL if no matching subscription found
**
**************************************************************************/
mqtt_subscription_t *FindSubscriptionByMid(mqtt_client_t *client, int mid, bool *is_agent_topic)
{
    int i;

    if (client == NULL)
    {
        return NULL;
    }

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        if (client->subscriptions[i].mid == mid)
        {
            if (is_agent_topic != NULL)
            {
                *is_agent_topic = false;
            }
            return &client->subscriptions[i];
        }
    }

    if (client->response_subscription.mid == mid)
    {
        if (is_agent_topic != NULL)
        {
            *is_agent_topic = true;
        }
        return &client->response_subscription;
    }

    return NULL;
}

/*********************************************************************//**
**
** FindMqttClientByMosquitto
**
** Finds the mqtt client that uses the specified libmosquitto context
** This is used to identify the MQTT client in the SSL callbacks
**
** \param   mosq - pointer to libmosquitto context
**
** \return  pointer to MQTT client or NULL if no match was found
**
**************************************************************************/
mqtt_client_t *FindMqttClientByMosquitto(struct mosquitto *mosq)
{
    int i;
    mqtt_client_t *client;

    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        client = &mqtt_clients[i];
        if (client->mosq == mosq)
        {
            return client;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** AddUserProperties
**
** Creates a libmosquitto property object containing the 'usp-endpoint-id' user property
**
** \param   props - pointer to variable in which to return a pointer to the created libmosquitto property object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int AddUserProperties(mosquitto_property **props)
{
    char* endpoint = DEVICE_LOCAL_AGENT_GetEndpointID();

    if (mosquitto_property_add_string_pair(props, USER_PROPERTY, "usp-endpoint-id",
                endpoint) != MOSQ_ERR_SUCCESS)
    {
        USP_LOG_Error("%s: Failed to add user property string to properties", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** AddConnectProperties
**
** Adds all libmosquitto properties used in the CONNECT packet
**
** \param   props - pointer to variable in which to return a pointer to the created libmosquitto property object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int AddConnectProperties(mosquitto_property **props)
{
    if (AddUserProperties(props) != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    if (mosquitto_property_add_byte(props, REQUEST_RESPONSE_INFORMATION, (uint8_t)1) != MOSQ_ERR_SUCCESS)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ConvertToMosquittoVersion
**
** Converts from USP MQTT version enumeration to libmosquitto enumeration
**
** \param   version - USP MQTT version enumeration to convert
** \param   mosquitto_version - pointer to variable in which to return libmosquitto MQTT version enumeration
**
** \return  USP_ERR_OK if converted successfully
**
**************************************************************************/
int ConvertToMosquittoVersion(mqtt_protocolver_t version, int* mosquitto_version)
{
    if (mosquitto_version == NULL)
    {
        return USP_ERR_UNSUPPORTED_PARAM;
    }

    switch(version)
    {
        case kMqttProtocol_3_1:
            *mosquitto_version = MQTT_PROTOCOL_V31;
            break;
        case kMqttProtocol_3_1_1:
            *mosquitto_version = MQTT_PROTOCOL_V311;
            break;
        case kMqttProtocol_5_0:
            *mosquitto_version = MQTT_PROTOCOL_V5;
            break;
        default:
            return USP_ERR_UNSUPPORTED_PARAM;
            break;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PopClientUspQueue
**
** Removes the first USP Record from the queue of the specified MQTT client
**
** \param   client - pointer to MQTT client
**
** \return  None
**
**************************************************************************/
void PopClientUspQueue(mqtt_client_t *client)
{
    // Remove the head of the client usp queue
    // Passed the client, not queue to distinguish this from a generic queue pop
    // We make some assumptions as to the type of structure this is

    if (client)
    {
        mqtt_send_item_t *head = (mqtt_send_item_t *) client->usp_record_send_queue.head;
        if (head != NULL)
        {
            USP_SAFE_FREE(head->topic);
            USP_SAFE_FREE(head->item.pbuf);
            DLLIST_Unlink(&client->usp_record_send_queue, head);
            USP_SAFE_FREE(head);
        }
    }
}

#endif
