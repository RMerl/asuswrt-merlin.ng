/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2020-2021, BT PLC
 * Copyright (C) 2021-2024  CommScope, Inc
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



#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>

#include "vendor_defs.h"  // For MAX_MQTT_SUBSCRIPTIONS
#include "usp-msg.pb-c.h"
#include "mtp_exec.h"
#include "socket_set.h"
#include "kv_vector.h"


typedef struct
{
    unsigned connect_retrytime;
    unsigned interval_multiplier;
    unsigned max_interval;
} mqtt_retry_params_t;

typedef enum
{
    kMqttQos_MostOnce = 0,       // TCP Fire and forget, the worst QOS
    kMqttQos_AtLeastOnce = 1,    // Acknowledged Message, can be sent more than once
    kMqttQos_ExactlyOnce = 2,    // Fully ackd message, always received once
    kMqttQos_Default = MQTT_FALLBACK_QOS,
} mqtt_qos_t;

typedef enum
{
    kMqttProtocol_3_1,           // Use v3.1 MQTT
    kMqttProtocol_3_1_1,         // Use v3.1.1 MQTT
    kMqttProtocol_5_0,           // Use v5.0 MQTT (recommended)
    kMqttProtocol_Default = kMqttProtocol_5_0,
    kMqttProtocol_Max,
} mqtt_protocolver_t;

typedef enum
{
    kMqttSubState_Unsubscribed = 0, // Not currently subscribed
    kMqttSubState_Subscribing,      // MQTT SUBSCRIBE message is being sent, waiting for SUBACK
    kMqttSubState_Subscribed,       // Subscribed (MQTT SUBACK has been received)
    kMqttSubState_Failed,           // Attempted to subscribe, but SUBACK indicated that subscription failed
    kMqttSubState_Unsubscribing,    // MQTT UNSUBSCRIBE is being sent. When UNSUBACK is received, the subscription will move to the unsubscribed state
    kMqttSubState_Resubscribing,    // MQTT UNSUBSCRIBE is being sent. When UNSUBACK is received, a subscribe will be sent for the new topic.
                                    // Subscription stays in this state until SUBACK is received
} mqtt_substate_t;

typedef enum
{
    kMqttTSprotocol_tcpip = 0,
    kMqttTSprotocol_tls,
    kMqttTSprotocol_Max,
} mqtt_tsprotocol_t;

// Structure used by device_mqtt.c, containing the configuration of an MQTT subscription
typedef struct
{
    int instance;
    mqtt_qos_t qos;
    char* topic;
    bool enabled;
} mqtt_subs_config_t;

// Structure used by mqtt.c, containing the configuration and current state of an MQTT subscription
typedef struct
{
    // Configuration
    int instance;
    mqtt_qos_t qos;
    char* topic;
    bool enabled;

    // State
    int mid; // Last mid for subscribe or unsubscribe message - to identify the SUBACK/UNSUBACK
    mqtt_substate_t state;
} mqtt_subscription_t;

typedef struct
{
    char* host;                   // Hostname of the broker
    unsigned int port;            // Port for broker
    int keepalive;                // Keepalive setting for broker connection
    char* username;               // Username to connect to broker
    char* password;               // Password to connect to broker
    char *alpn;                   // Application Layer Protocol Negotiation options to send in SSL handshake (comma separated list)
    int instance;                 // Client instance (Device.MQTT.Client.{i})
    bool enable;

    mqtt_protocolver_t version;   // MQTT protocol version to use

    char* response_topic;         // Agent's topic: Topic which agent subscribes to, and Controller publishes to
                                  // NOTE: If not configured in Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured, then this variable may be set to NULL
    mqtt_qos_t publish_qos;       // Agent's PublishQos: used to set mqtt_send_item_t.qos variable, when building a msg to send.
                                  // NOTE: If not configured in Device.LocalAgent.MTP.{i}.MQTT.PublishQoS, then its value is kMqttQos_Default

    // V5 Params
    char* client_id;

    // mqtt parameters
    char* name;
    mqtt_tsprotocol_t ts_protocol;
    bool clean_session;
    bool clean_start;
    bool request_response_info;
    char* response_information;

    mqtt_retry_params_t retry;
} mqtt_conn_params_t;

//------------------------------------------------------------------------------
// API
int MQTT_Init(void);
void MQTT_Destroy(void);
int MQTT_Start(void);
void MQTT_Stop(void);
void MQTT_ModifyConnectedControllers(int instance, kv_vector_t *controller_topics);
int MQTT_EnableClient(mqtt_conn_params_t *mqtt_params, mqtt_subs_config_t subscriptions[MAX_MQTT_SUBSCRIPTIONS], kv_vector_t *controller_topics);
int MQTT_DisableClient(int instance);
int MQTT_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char *topic, time_t expiry_time);
void MQTT_UpdateConnectionParams(mqtt_conn_params_t *mqtt_params, bool schedule_reconnect);
void MQTT_ActivateScheduledActions(void);
mtp_status_t MQTT_GetMtpStatus(int instance);
const char* MQTT_GetClientStatus(int instance);
void MQTT_UpdateRetryParams(int instance, mqtt_retry_params_t *retry_params);
bool MQTT_AreAllResponsesSent(void);
void MQTT_ProcessAllActivity(void);
int MQTT_AddSubscription(int instance, mqtt_subs_config_t *subscription);
int MQTT_DeleteSubscription(int instance, int subinstance);
int MQTT_ScheduleResubscription(int instance, mqtt_subs_config_t *new_sub);
void MQTT_UpdateAllSockSet(socket_set_t *set);
void MQTT_ProcessAllSocketActivity(socket_set_t* set);
void MQTT_InitConnParams(mqtt_conn_params_t* params);
void MQTT_DestroyConnParams(mqtt_conn_params_t* params);
int MQTT_GetAgentResponseTopicDiscovered(int instance, char *buf, int len);
void MQTT_AllowConnect(void);
#ifdef BDK_USP
mqtt_subscription_t *MQTT_FindSubscriptionByInstances(int instance, int subinstance);
#endif   /* BDK_USP */

#endif
