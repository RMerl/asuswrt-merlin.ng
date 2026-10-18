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
/**
 * \file device_mqtt.c
 *
 * Implements the Device.MQTT data model object
 *
 */
#ifdef ENABLE_MQTT
#include <time.h>
#include <string.h>
#include <limits.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "dm_access.h"
#include "dm_trans.h"
#include "kv_vector.h"
#include "mtp_exec.h"
#include "device.h"
#include "text_utils.h"
#include "mqtt.h"
#include "iso8601.h"

typedef struct
{
    mqtt_conn_params_t conn_params;
    mqtt_subs_config_t subscriptions[MAX_MQTT_SUBSCRIPTIONS];
} client_t;


// Table used to convert from a textual representation of protocol version to an enumeration
const enum_entry_t mqtt_protocolver[kMqttProtocol_Max] =
{
    { kMqttProtocol_3_1,  "3.1" },           // Use v3.1 MQTT
    { kMqttProtocol_3_1_1, "3.1.1" },        // Use v3.1.1 MQTT
    { kMqttProtocol_5_0, "5.0" },          // Use v5.0 MQTT (recommended)
};

// Table used to convert from a textual representation of Transport protocol to an enumeration
const enum_entry_t mqtt_tsprotocol[kMqttTSprotocol_Max] =
{
    { kMqttTSprotocol_tcpip,  "TCP/IP" },
    { kMqttTSprotocol_tls, "TLS" },
};
//------------------------------------------------------------------------------
// Location of the MQTT Client table within the data model
#define DEVICE_MQTT_CLIENT "Device.MQTT.Client"
static const char device_mqtt_client_root[] = DEVICE_MQTT_CLIENT;

//------------------------------------------------------------------------------
// Cache of the parameters in the Device.MQTT.Client table
static client_t mqtt_client_params[MAX_MQTT_CLIENTS];

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
//mqtt client
int ValidateAdd_Mqttclients(dm_req_t *req);
int Notify_MQTTClientAdded(dm_req_t *req);
int Notify_MqttClientDeleted(dm_req_t *req);
int NotifyChange_MQTTEnable(dm_req_t *req, char *value);
int NotifyChange_MQTTBrokerAddress(dm_req_t *req, char *value);
int Validate_MQTTBrokerPort(dm_req_t *req, char *value);
int NotifyChange_MQTTBrokerPort(dm_req_t *req, char *value);
int NotifyChange_MQTTUsername(dm_req_t *req, char *value);
int NotifyChange_MQTTPassword(dm_req_t *req, char *value);
int Validate_MQTTProtocolVersion(dm_req_t *req, char *value);
int NotifyChange_MQTTProtocolVersion(dm_req_t *req, char *value);
int Validate_MQTTKeepAliveTime(dm_req_t *req, char *value);
int NotifyChange_MQTTKeepAliveTime(dm_req_t *req, char *value);
int NotifyChange_MQTTClientId(dm_req_t *req, char *value);
int NotifyChange_MQTTName(dm_req_t *req, char *value);
int Validate_MQTTTransportProtocol(dm_req_t *req, char *value);
int NotifyChange_MQTTTransportProtocol(dm_req_t *req, char *value);
int NotifyChange_MQTTCleanSession(dm_req_t *req, char *value);
int NotifyChange_MQTTCleanStart(dm_req_t *req, char *value);
int NotifyChange_MQTTRequestResponseInfo(dm_req_t *req, char *value);
int Get_MqttResponseInformation(dm_req_t *req, char *buf, int len);
int Validate_MQTTConnectRetryTime(dm_req_t *req, char *value);
int NotifyChange_MQTTConnectRetryTime(dm_req_t *req, char *value);
int Validate_MQTTConnectRetryIntervalMultiplier(dm_req_t *req, char *value);
int NotifyChange_MQTTConnectRetryIntervalMultiplier(dm_req_t *req, char *value);
int Validate_MQTTConnectRetryMaxInterval(dm_req_t *req, char *value);
int NotifyChange_MQTTConnectRetryMaxInterval(dm_req_t *req, char *value);
int Get_MqttClientStatus(dm_req_t *req, char *buf, int len);
int AutoPopulate_MqttName(dm_req_t *req, char *buf, int len);
int Validate_MqttName(dm_req_t *req, char *value);
int NotifyChange_MQTT_ALPN(dm_req_t *req, char *value);

/*MQTT Subscriptions*/
int ValidateAdd_MqttClientSubscriptions(dm_req_t *req);
int Notify_MqttClientSubcriptionsAdded(dm_req_t *req);
int Notify_MqttClientSubscriptionsDeleted(dm_req_t *req);
int Validate_MQTTSubscriptionEnable(dm_req_t *req, char *value);
int NotifyChange_MQTTSubscriptionEnable(dm_req_t *req, char *value);
int NotifyChange_MQTTSubscriptionTopic(dm_req_t *req, char *value);
int Validate_MQTTSubscriptionQoS(dm_req_t *req, char *value);
int NotifyChange_MQTTSubscriptionQoS(dm_req_t *req, char *value);
int Validate_MQTTSubscriptionTopic(dm_req_t *req, char *value);

mqtt_conn_params_t *FindMqttParamsByInstance(int instance);
mqtt_conn_params_t *FindUnusedMqttParams(void);
mqtt_subs_config_t* FindUnusedSubscriptionInMqttClient(client_t* client);
client_t *FindUnusedMqttClient(void);
client_t *FindDevMqttClientByInstance(int instance);
void DestroyMQTTClient(client_t *client);
int ProcessMqttClientAdded(int instance);
int ProcessMqttSubscriptionAdded(int instance, int sub_instance, mqtt_subs_config_t **mqtt_sub);
int DEVICE_MQTT_StartAllClients(void);
int EnableMQTTClient(client_t *mqttclient);
void ScheduleMqttReconnect(mqtt_conn_params_t *mp);
void ScheduleMQTTResubscribe(client_t *mqttclient, mqtt_subs_config_t *sub);
int ClientNumberOfEntries(void);
int SubscriptionNumberofEntries(int instance);
int Operate_MQTTForceReconnect(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);

/*********************************************************************//**
**
** DEVICE_MQTT_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_MQTT_Init(void)
{
    int err = USP_ERR_OK;
    int i, j;
    mqtt_conn_params_t *mp;
    mqtt_subs_config_t *subs;

    // Exit if unable to initialise the lower level MQTT component
    err = MQTT_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Initialise all of the client parameters
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        mp = &mqtt_client_params[i].conn_params;
        MQTT_InitConnParams(mp);

        // Initialise the subsciption mappings
        memset(&mqtt_client_params[i].subscriptions, 0, sizeof(mqtt_client_params->subscriptions));
        for (j=0; j<MAX_MQTT_SUBSCRIPTIONS; j++)
        {
            subs = &mqtt_client_params[i].subscriptions[j];
            subs->instance = INVALID;
        }
    }

    // Register parameters implemented by this component
    err |= USP_REGISTER_Param_SupportedList("Device.MQTT.Capabilities.ProtocolVersionsSupported", mqtt_protocolver, NUM_ELEM(mqtt_protocolver));
    err |= USP_REGISTER_Param_SupportedList("Device.MQTT.Capabilities.TransportProtocolSupported", mqtt_tsprotocol, NUM_ELEM(mqtt_tsprotocol));
    err |= USP_REGISTER_Param_Constant("Device.MQTT.Capabilities.MaxNumberOfClientSubscriptions", TO_STR(MAX_MQTT_SUBSCRIPTIONS), DM_UINT);

    err |= USP_REGISTER_Object(DEVICE_MQTT_CLIENT ".{i}", ValidateAdd_Mqttclients, NULL,
                               Notify_MQTTClientAdded, NULL, NULL, Notify_MqttClientDeleted);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("MQTT object registration failed");
        return err;
    }

    err |= USP_REGISTER_Param_NumEntries("Device.MQTT.ClientNumberOfEntries", DEVICE_MQTT_CLIENT ".{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_MQTT_CLIENT ".{i}.Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.Enable", "false", NULL, NotifyChange_MQTTEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.BrokerAddress", NULL, NULL, NotifyChange_MQTTBrokerAddress, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.BrokerPort", "1883", Validate_MQTTBrokerPort, NotifyChange_MQTTBrokerPort, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.Username", NULL, NULL, NotifyChange_MQTTUsername, DM_STRING);
    err |= USP_REGISTER_DBParam_Secure(DEVICE_MQTT_CLIENT ".{i}.Password", "", NULL, NotifyChange_MQTTPassword);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.KeepAliveTime", "60",  Validate_MQTTKeepAliveTime, NotifyChange_MQTTKeepAliveTime, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ProtocolVersion", "5.0", Validate_MQTTProtocolVersion, NotifyChange_MQTTProtocolVersion , DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ClientID", "", NULL, NotifyChange_MQTTClientId, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWriteAuto(DEVICE_MQTT_CLIENT ".{i}.Name", AutoPopulate_MqttName, Validate_MqttName, NotifyChange_MQTTName, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.TransportProtocol", "TCP/IP", Validate_MQTTTransportProtocol, NotifyChange_MQTTTransportProtocol , DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.CleanSession", "true", NULL, NotifyChange_MQTTCleanSession , DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.CleanStart", "true", NULL, NotifyChange_MQTTCleanStart , DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.RequestResponseInfo", "false", NULL, NotifyChange_MQTTRequestResponseInfo , DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ALPN", "", DEVICE_SECURITY_ValidateALPN, NotifyChange_MQTT_ALPN , DM_STRING);

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ConnectRetryTime", "5", Validate_MQTTConnectRetryTime, NotifyChange_MQTTConnectRetryTime , DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ConnectRetryIntervalMultiplier", "2000", Validate_MQTTConnectRetryIntervalMultiplier, NotifyChange_MQTTConnectRetryIntervalMultiplier , DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.ConnectRetryMaxInterval", "30720", Validate_MQTTConnectRetryMaxInterval, NotifyChange_MQTTConnectRetryMaxInterval , DM_UINT);

    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_MQTT_CLIENT ".{i}.ResponseInformation", Get_MqttResponseInformation, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_MQTT_CLIENT ".{i}.Status", Get_MqttClientStatus, DM_STRING);

    err |= USP_REGISTER_Object(DEVICE_MQTT_CLIENT ".{i}.Subscription.{i}.", ValidateAdd_MqttClientSubscriptions, NULL, Notify_MqttClientSubcriptionsAdded,
                                                              NULL, NULL, Notify_MqttClientSubscriptionsDeleted);
    err |= USP_REGISTER_Param_NumEntries(DEVICE_MQTT_CLIENT ".{i}.SubscriptionNumberOfEntries", DEVICE_MQTT_CLIENT".{i}.Subscription.{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_MQTT_CLIENT ".{i}.Subscription.{i}.Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.Subscription.{i}.Enable", "false", Validate_MQTTSubscriptionEnable, NotifyChange_MQTTSubscriptionEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.Subscription.{i}.Topic", NULL, Validate_MQTTSubscriptionTopic, NotifyChange_MQTTSubscriptionTopic, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_MQTT_CLIENT ".{i}.Subscription.{i}.QoS", TO_STR(MQTT_FALLBACK_QOS), Validate_MQTTSubscriptionQoS, NotifyChange_MQTTSubscriptionQoS, DM_UINT);

    char *name_unique_keys[] = { "Name" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_MQTT_CLIENT ".{i}", name_unique_keys, NUM_ELEM(name_unique_keys));

    err |= USP_REGISTER_SyncOperation(DEVICE_MQTT_CLIENT ".{i}.ForceReconnect()", Operate_MQTTForceReconnect);

    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to Register MQTT", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_MQTT_Start
**
** Initialises the mqtt connection array from the DB
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_MQTT_Start(void)
{
    int i;
    int err;
    int_vector_t iv;
    int instance;
    char path[MAX_DM_PATH];

    // Exit if unable to get the object instance numbers present in the mqtt client table
    err = DATA_MODEL_GetInstances(DEVICE_MQTT_CLIENT, &iv);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to start Device MQTT", __FUNCTION__);
        return err;
    }

    // Add all MQTT clients to the MQTT client array
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];
        err = ProcessMqttClientAdded(instance);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a MQTT connection with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_mqtt_client_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    err = MQTT_Start();
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to start MQTT", __FUNCTION__);
        return err;
    }
    err = USP_ERR_OK;

exit:
    // Destroy the vector of instance numbers for the table
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_MQTT_Stop
**
** Frees up all memory associated with this module
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_MQTT_Stop(void)
{
    int i;

    // Destroy all clients
    for (i = 0; i < MAX_MQTT_CLIENTS; i++)
    {
        DestroyMQTTClient(&mqtt_client_params[i]);
    }

    // NOTE: No need to call MQTT_Destroy() from here, as the MQTT MTP thread will already have been destroyed before this function is called
}

/*********************************************************************//**
**
** DEVICE_MQTT_StartAllClients
**
** Starts all MQTT clients
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_MQTT_StartAllClients(void)
{
    client_t *mqttclient;
    int err;
    int i;

    // Iterate over all MQTT clients, starting the ones that are enabled
    for (i = 0; i<MAX_MQTT_CLIENTS; i++)
    {
        mqttclient = &mqtt_client_params[i];

        if ((mqttclient->conn_params.instance != INVALID) && (mqttclient->conn_params.enable == true))
        {
            // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
            err = EnableMQTTClient(mqttclient);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_MQTT_ScheduleReconnect
**
** Schedules a reconnect to the specified MQTT client, once that connection has finished sending any response
**
** \param   instance - instance number of the connection in Device.MQTT.Client.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_MQTT_ScheduleReconnect(int instance)
{
    mqtt_conn_params_t *mp;

    // Exit if unable to find the specified MQTT connection
    mp = FindMqttParamsByInstance(instance);
    if (mp == NULL)
    {
        return;
    }

    // Schedule a reconnect if this MQTT client enabled
    if (mp->enable)
    {
        ScheduleMqttReconnect(mp);
    }
}

/*********************************************************************//**
**
** DEVICE_MQTT_UpdateClientId
**
** Called to update the ClientID saved in Device.MQTT.Client.{i}.ClientID
** This may occur for MQTTv5 connections whose ClientID is assigned by the MQTT Broker in the CONNACK. See R-MQTT.9)
**
** \param   instance - instance number of the connection in Device.MQTT.Client.{i}
** \param   client_id - instance - instance number of the connection in Device.MQTT.Client.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_MQTT_UpdateClientId(int instance, char *client_id)
{
    int err;
    mqtt_conn_params_t *mp;
    char path[MAX_DM_PATH];
    dm_trans_vector_t trans;

    // Exit if unable to determine mqtt client to be updated
    mp = FindMqttParamsByInstance(instance);
    if (mp == NULL)
    {
        return;
    }

    // Exit if client_id hasn't changed
    if (strcmp(mp->client_id, client_id) == 0)
    {
        return;
    }

    // Set the new value persistently in the USP DB
    err = DM_TRANS_Start(&trans);
    if (err == USP_ERR_OK)
    {
        USP_SNPRINTF(path, sizeof(path), "Device.MQTT.Client.%d.ClientID", instance);
        DATA_MODEL_SetParameterValue(path, client_id, 0);

        // The commit will cascade to update our local copy
        DM_TRANS_Commit();
    }
}

/*********************************************************************//**
**
** DEVICE_MQTT_GetMtpStatus
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.Status for an MQTT client
**
** \param   instance - instance number of the connection in Device.MQTT.Client.{i}
**
** \return  Status of the MQTT client
**
**************************************************************************/
mtp_status_t DEVICE_MQTT_GetMtpStatus(int instance)
{
    mqtt_conn_params_t *mp;

    // Exit if unable to find the specified MQTT client
    // NOTE: This could occur if the connection was disabled, or the connection reference was incorrect
    mp = FindMqttParamsByInstance(instance);
    if ((mp == NULL) || (mp->enable == false))
    {
        return kMtpStatus_Down;
    }

    return MQTT_GetMtpStatus(mp->instance);
}

/*********************************************************************//**
**
** DEVICE_MQTT_CountEnabledConnections
**
** Returns a count of the number of enabled MQTT clients
**
** \param   None
**
** \return  returns a count of the number of enabled MQTT clients
**
**************************************************************************/
int DEVICE_MQTT_CountEnabledConnections(void)
{
    int i;
    int count = 0;
    client_t *mqttclient;

    // Iterate over all MQTT connections
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        // Increase the count if found an enabled connection
        mqttclient = &mqtt_client_params[i];
        if ((mqttclient->conn_params.instance != INVALID) && (mqttclient->conn_params.enable))
        {
            count++;
        }
    }

    return count;
}

/*********************************************************************//**
**
** DEVICE_MQTT_UpdateControllerTopics
**
** Called to update all MTP clients with the set of Controllers connected to them
** (in order that USP Connect/Disconnect records can be sent to the Controllers)
** This function is called whenever there are changes to Controller MTP parameters
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DEVICE_MQTT_UpdateControllerTopics(void)
{
    int i;
    kv_vector_t controller_topics;
    mqtt_conn_params_t *mp;

    // Iterate over all MQTT connections, updating which controllers are connected to them
    for (i=0; i<NUM_ELEM(mqtt_client_params); i++)
    {
        mp = &mqtt_client_params[i].conn_params;
        if ((mp->instance != INVALID) && (mp->enable))
        {
            DEVICE_CONTROLLER_GetMqttControllerTopics(mp->instance, &controller_topics);
            MQTT_ModifyConnectedControllers(mp->instance, &controller_topics); // NOTE: ownership of controller_topics passes to MQTT MTP if it contains anything
        }
    }
}

/*********************************************************************//**
**
** Get_MqttClientStatus
**
** Gets the value of Device.MQTT.Client.{i}.Status
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_MqttClientStatus(dm_req_t *req, char *buf, int len)
{
    mqtt_conn_params_t* conn_params;
    const char *status;

    // Determine MQTT connection to be read
    conn_params = FindMqttParamsByInstance(inst1);
    USP_ASSERT(conn_params != NULL);

    if (conn_params->enable == false)
    {
        status = "Disabled";
    }
    else
    {
        status = MQTT_GetClientStatus(conn_params->instance);
    }

    USP_STRNCPY(buf, status, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_MqttResponseInformation
**
** Gets the value of Device.MQTT.Client.{i}.ResponseInformation
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_MqttResponseInformation(dm_req_t *req, char *buf, int len)
{
    return MQTT_GetAgentResponseTopicDiscovered(inst1, buf, len);
}

/*********************************************************************//**
**
** ProcessMqttClientAdded
*
** Reads the parameters for the specified Mqtt Client from the database and processes them
**
** \param   instance - instance number of the Mqtt client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessMqttClientAdded(int instance)
{
    int err;
    char path[MAX_DM_PATH];
    int_vector_t iv;
    client_t *mqttclient;
    int i;
    int mqtt_subs_inst;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_username_cb_t   get_mtp_username_cb;
    dm_vendor_get_mtp_password_cb_t   get_mtp_password_cb;

    // Initialise to defaults
    INT_VECTOR_Init(&iv);
    mqttclient = FindUnusedMqttClient();
    if (mqttclient == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }
    mqttclient->conn_params.instance = instance;

    // Exit if unable to get the enable for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetBool(path, &mqttclient->conn_params.enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the host for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.BrokerAddress", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.host);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the port for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.BrokerPort", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &mqttclient->conn_params.port);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the protocol version for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ProtocolVersion", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetEnum(path, &mqttclient->conn_params.version, mqtt_protocolver, NUM_ELEM(mqtt_protocolver));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the keepalive for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.KeepAliveTime", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetInteger(path, &mqttclient->conn_params.keepalive);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the username for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Username", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.username);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Override a blank username in the database with that provided by a core vendor hook
    if (mqttclient->conn_params.username[0] == '\0')
    {
        get_mtp_username_cb = vendor_hook_callbacks.get_mtp_username_cb;
        if (get_mtp_username_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_username_cb(instance, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Replace the blank password from the database with the password retrieved via core vendor hook
            USP_SAFE_FREE(mqttclient->conn_params.username);
            mqttclient->conn_params.username = USP_STRDUP(buf);
        }
    }

    // Exit if unable to get the password for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Password", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetPassword(path, &mqttclient->conn_params.password);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Override a blank password in the database with that provided by a core vendor hook
    if (mqttclient->conn_params.password[0] == '\0')
    {
        get_mtp_password_cb = vendor_hook_callbacks.get_mtp_password_cb;
        if (get_mtp_password_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_password_cb(instance, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Replace the blank password from the database with the password retrieved via core vendor hook
            USP_SAFE_FREE(mqttclient->conn_params.password);
            mqttclient->conn_params.password = USP_STRDUP(buf);
        }
    }

    // Exit if unable to get the clientid for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ClientID", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.client_id);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the name for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Name", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.name);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the ALPN options for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ALPN", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.alpn);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the transport protocol for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.TransportProtocol", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetEnum(path, &mqttclient->conn_params.ts_protocol, mqtt_tsprotocol, NUM_ELEM(mqtt_tsprotocol));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the CleanSession for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.CleanSession", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetBool(path, &mqttclient->conn_params.clean_session);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the Cleanstart for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.CleanStart", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetBool(path, &mqttclient->conn_params.clean_start);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the RequestResponseInfo for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.RequestResponseInfo", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetBool(path, &mqttclient->conn_params.request_response_info);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the Response Information for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ResponseInformation", device_mqtt_client_root, instance);
    USP_SAFE_FREE(mqttclient->conn_params.response_information);
    err = DM_ACCESS_GetString(path, &mqttclient->conn_params.response_information);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the ConnectRetryTime for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ConnectRetryTime", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &mqttclient->conn_params.retry.connect_retrytime);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the ConnectRetryIntervalMultiplier for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ConnectRetryIntervalMultiplier", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &mqttclient->conn_params.retry.interval_multiplier);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the ConnectRetryMaxInterval for this MQTT client
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ConnectRetryMaxInterval", device_mqtt_client_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &mqttclient->conn_params.retry.max_interval);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the object instance numbers present in the mqtt client subscription table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Subscription.", device_mqtt_client_root, instance);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all MQTT subscriptions
    for (i=0; i < iv.num_entries; i++)
    {
        mqtt_subs_inst = iv.vector[i];
        err = ProcessMqttSubscriptionAdded(instance, mqtt_subs_inst, NULL);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a MQTT subscription with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d.Subscription.%d", device_mqtt_client_root, instance, mqtt_subs_inst);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    // If the code gets here, then we successfully retrieved all data about the MQTT client
    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);

    if (err != USP_ERR_OK)
    {
        DestroyMQTTClient(mqttclient);
    }
    return err;
}

/*********************************************************************//**
**
** EnableMQTTClient
**
** Wrapper function to start the specified MQTT client connecting
**
** \param   mqttclient - MQTT client parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int EnableMQTTClient(client_t *mqttclient)
{
    int err;
    mqtt_conn_params_t *mp;
    kv_vector_t controller_topics;

    // Update agent topic. NOTE: It could be NULL if Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured is empty
    mp = &mqttclient->conn_params;
    USP_SAFE_FREE(mp->response_topic);
    mp->response_topic = USP_STRDUP(DEVICE_MTP_GetAgentMqttResponseTopic(mp->instance));
    mp->publish_qos = DEVICE_MTP_GetAgentMqttPublishQos(mp->instance);

    // Exit if unable to start the MQTT client
    // NOTE: Ownership of controller_topics passes to MQTT_EnableClient
    DEVICE_CONTROLLER_GetMqttControllerTopics(mqttclient->conn_params.instance, &controller_topics);
    err = MQTT_EnableClient(mp, mqttclient->subscriptions, &controller_topics);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s MQTT failed to enable the client", __FUNCTION__);
        return err;
    }

    return err;
}

/*********************************************************************//**
**
** ProcessMqttSubscriptionAdded
*
** Reads the parameters for the specified MQTT client subscription from the
** database and caches them in the local mqtt_client_params[]
** NOTE: Does not propagate the change to the underlying MTP (this must be performed by the caller by calling MQTT_AddSubscription)
**
** \param   instance - instance number of the MQTT client
** \param   sub_instance - instance number of the MQTT Subscription
** \param   mqtt_sub - pointer to variable in which to return a pointer to the MQTT subscription entry allocated by this function
**                     NOTE: This parameter may be NULL, if the caller is not interested in this value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessMqttSubscriptionAdded(int instance, int sub_instance, mqtt_subs_config_t **mqtt_sub)
{
    int err = USP_ERR_OK;
    char path[MAX_DM_PATH];
    client_t *client = NULL;
    mqtt_subs_config_t *sub;

    // Default return parameters
    if (mqtt_sub != NULL)
    {
        *mqtt_sub = NULL;
    }

    // Exit if unable to allocate a subscription entry for this MQTT client
    client = FindDevMqttClientByInstance(instance);
    USP_ASSERT(client != NULL);
    sub = FindUnusedSubscriptionInMqttClient(client);
    if (sub == NULL)
    {
        USP_LOG_Error("%s: Failed to find empty subscription.", __FUNCTION__);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Exit if unable to retrieve the Enable parameter for this MQTT subscription
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Subscription.%d.Enable", device_mqtt_client_root, instance, sub_instance);
    err = DM_ACCESS_GetBool(path, &sub->enabled);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to retrieve the Topic parameter for this MQTT subscription
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Subscription.%d.Topic", device_mqtt_client_root, instance, sub_instance);
    USP_SAFE_FREE(sub->topic);
    err = DM_ACCESS_GetString(path, &sub->topic);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the subscription is enabled, but the topic is empty
    USP_ASSERT(sub->topic != NULL);
    if ((sub->enabled) && (sub->topic[0] == '\0'))
    {
        USP_LOG_Error("%s: Device.MQTT.Client.%d.Subscription.%d is enabled, but topic is empty", __FUNCTION__, instance, sub_instance);
        USP_SAFE_FREE(sub->topic);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to retrieve the QoS parameter for this MQTT subscription
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Subscription.%d.QoS", device_mqtt_client_root, instance, sub_instance);
    err = DM_ACCESS_GetUnsigned(path, &sub->qos);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Since the subscription was retrieved successfully, mark the subscription as 'in-use' and return the subscription entry
    sub->instance = sub_instance;
    if (mqtt_sub != NULL)
    {
        *mqtt_sub = sub;
    }

    return err;
}

/*********************************************************************//**
**
** AutoPopulate_MqttName
**
** Called to get an auto-populated parameter value for the Device.MQTT.Client.{i}.Name parameter
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer in which to store the value to use to auto-populate the parameter's value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if auto assigned ID was unique
**
**************************************************************************/
int AutoPopulate_MqttName(dm_req_t *req, char *buf, int len)
{
    int err;

    USP_SNPRINTF(buf, len, "cpe-%d", inst1);
    err = Validate_MqttName(req, buf);

    return err;
}

/*********************************************************************//**
**
** Validate_MqttName
**
** Validates that the Name parameter being added is unique for the recipient
**
** \param   req - pointer to structure identifying the instance
** \param   value - value that the controller would like to set the Name to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MqttName(dm_req_t *req, char *value)
{
    int i;
    client_t *mqttclient;

    // Exit if no value set for Name (i.e. set to an empty string)
    if (*value == '\0')
    {
        USP_ERR_SetMessage("%s: Name must be set to a non-empty string", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Iterate over all MQTT clients
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        // Skip if this is an unused slot
        mqttclient = &mqtt_client_params[i];
        if (mqttclient->conn_params.instance == INVALID)
        {
            continue;
        }

        // Exit if the instance already exists
        if (mqttclient->conn_params.instance == inst1)
        {
            // Allow the new value if no value was previously set, or the new value is the same as the old value
            if ((*(mqttclient->conn_params.name) == '\0') || (strcmp(mqttclient->conn_params.name, value) == 0))
            {
                return USP_ERR_OK;
            }

            // Otherwise, the new value is different, so don't allow it
            USP_ERR_SetMessage("%s: Name cannot be changed from '%s'", __FUNCTION__, mqttclient->conn_params.name);
            return USP_ERR_INVALID_VALUE;
        }


        // Exit if this entry matches the value that the controller is trying to set
        if (strcmp(mqttclient->conn_params.name, value)==0)
        {
            USP_ERR_SetMessage("%s: Name (%s) is already in use by %s.%d", __FUNCTION__, value, device_mqtt_client_root, mqttclient->conn_params.instance);
            return USP_ERR_INVALID_VALUE;
        }
    }

    // If the code gets here, then the new Subscription ID value is unique for this controller
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTBrokerPort
**
** Validates Device.MQTT.Client.{i}.BrokerPort by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTBrokerPort(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_MQTTKeepAliveTime
**
** Validates Device.MQTT.Client.{i}.KeepAliveTime by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTKeepAliveTime(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 0, 65535);
}

/*********************************************************************//**
**
** Validate_MQTTProtocolVersion
**
** Validates Device.MQTT.Client.{i}.ProtocolVersion
** by checking that it matches the mqtt protocol version we support
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTProtocolVersion(dm_req_t *req, char *value)
{
    mqtt_protocolver_t protocol;

    // Exit if the protocol was invalid
    protocol = TEXT_UTILS_StringToEnum(value, mqtt_protocolver, NUM_ELEM(mqtt_protocolver));
    if (protocol == INVALID)
    {
        USP_ERR_SetMessage("%s: Invalid or unsupported protocol %s", __FUNCTION__, value);
        return USP_ERR_INVALID_VALUE;
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTTransportProtocol
**
** Validates Device.MQTT.Client.{i}.TransportProtocol
** by checking that it matches the mqtt Transport protocol we support
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTTransportProtocol(dm_req_t *req, char *value)
{
    mqtt_tsprotocol_t tsprotocol;

    // Exit if the protocol was invalid
    tsprotocol = TEXT_UTILS_StringToEnum(value, mqtt_tsprotocol, NUM_ELEM(mqtt_tsprotocol));
    if (tsprotocol == INVALID)
    {
        USP_ERR_SetMessage("%s: Invalid or unsupported transport protocol %s", __FUNCTION__, value);
        return USP_ERR_INVALID_VALUE;
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTConnectRetryTime
**
** Validates Device.MQTT.Client.{i}.ConnectRetryTime by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTConnectRetryTime(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_MQTTConnectRetryIntervalMultiplier
**
** Validates Device.MQTT.Client.{i}.ConnectRetryIntervalMultiplier by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTConnectRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1000, 65535);
}

/*********************************************************************//**
**
** Validate_MQTTConnectRetryMaxInterval
**
** Validates Device.MQTT.Client.{i}.ConnectRetryMaxInterval by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTConnectRetryMaxInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** NotifyChange_MQTTEnable
**
** Function called when Device.MQTT.Client.{i}.Enable is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTEnable(dm_req_t *req, char *value)
{
    client_t *mqttclient;
    bool old_value;
    int err;

    // Determine mqtt client to be updated
    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL);
    old_value = mqttclient->conn_params.enable;

    // Stop the connection if it has been disabled
    // NOTE: This code does not support sending a response back to a controller that disables it's own MQTT connection
    // However, as it is unlikely to be the case that a controller would ever do this, I have not added extra code to support this
    if ((old_value == true) && (val_bool == false))
    {
        MQTT_DisableClient(mqttclient->conn_params.instance);
    }

    // Set the new value, we do this inbetween stopping and starting the connection because both must have the enable set to true
    mqttclient->conn_params.enable = val_bool;

    // Start the connection if it has been enabled
    if ((old_value == false) && (val_bool == true))
    {
        // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
        err = EnableMQTTClient(mqttclient);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTBrokerAddress
**
** Function called when Device.MQTT.Client.{i}.BrokerAddress is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTBrokerAddress(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    bool schedule_reconnect = false;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Determine whether to schedule a reconnect
    if ((strcmp(mp->host, value) != 0) && (mp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(mp->host);
    mp->host = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleMqttReconnect(mp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTBrokerPort
**
** Function called when Device.MQTT.Client.{i}.BrokerPort is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTBrokerPort(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    bool schedule_reconnect = false;

    // Determine mqtt connection to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Determine whether to schedule a reconnect
    if ((mp->port != val_uint) && (mp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    mp->port = val_uint;

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleMqttReconnect(mp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTUsername
**
** Function called when Device.MQTT.Client.{i}.Username is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTUsername(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    bool schedule_reconnect = false;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_username_cb_t   get_mtp_username_cb;
    int err;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Override a blank username in the database with that provided by a core vendor hook
    if (*value == '\0')
    {
        get_mtp_username_cb = vendor_hook_callbacks.get_mtp_username_cb;
        if (get_mtp_username_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_username_cb(inst1, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                return err;
            }

            value = buf;
        }
    }

    // Determine whether to schedule a reconnect
    if ((strcmp(mp->username, value) != 0) && (mp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(mp->username);
    mp->username = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleMqttReconnect(mp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTPassword
**
** Function called when Device.MQTT.Client.{i}.Password is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTPassword(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    bool schedule_reconnect = false;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_password_cb_t   get_mtp_password_cb;
    int err;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Override a blank password in the database with that provided by a core vendor hook
    if (*value == '\0')
    {
        get_mtp_password_cb = vendor_hook_callbacks.get_mtp_password_cb;
        if (get_mtp_password_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_password_cb(inst1, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                return err;
            }

            value = buf;
        }
    }

    // Determine whether to schedule a reconnect
    if ((strcmp(mp->password, value) != 0) && (mp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(mp->password);
    mp->password = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleMqttReconnect(mp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTKeepAliveTime
**
** Function called when Device.MQTT.Client.{i}.KeepAliveTime is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTKeepAliveTime(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if value has not changed
    if (mp->keepalive == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mp->keepalive = val_uint;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTT_ALPN
**
** Function called when Device.MQTT.Client.{i}.ALPN is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTT_ALPN(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if value has not changed
    if (strcmp(mp->alpn, value)==0)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    USP_SAFE_FREE(mp->alpn);
    mp->alpn = USP_STRDUP(value);

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*************************************************************************
**
** NotifyChange_MQTTProtocolVersion
**
** Function called when Device.MQTT.Client.{i}.ProtocolVersion is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTProtocolVersion(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    mqtt_protocolver_t new_protocol;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Extract the new value
    new_protocol = TEXT_UTILS_StringToEnum(value, mqtt_protocolver, NUM_ELEM(mqtt_protocolver));
    USP_ASSERT(new_protocol != INVALID); // Value must already have validated to have got here

    // Exit if protocol has not changed
    if (new_protocol == mp->version)
    {
        return USP_ERR_OK;
    }

    // Store new protocol
    mp->version = new_protocol;

    if ((mp->enable) && (mp->instance != INVALID))
    {
        ScheduleMqttReconnect(mp);
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTClientID
**
** Function called when Device.MQTT.Client.{i}.ClientID is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTClientId(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if the value has not changed
    if (strcmp(mp->client_id, value) == 0)
    {
        return USP_ERR_OK;
    }

    // Set the new value in the local structure
    USP_SAFE_FREE(mp->client_id);
    mp->client_id = USP_STRDUP(value);

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect. Writing to ClientID must not force a reconnect
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTName
**
** Function called when Device.MQTT.Client.{i}.Name is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTName(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if the value has not changed
    if (strcmp(mp->name, value) == 0)
    {
        return USP_ERR_OK;
    }

    // Set the new value in the local structure
    USP_SAFE_FREE(mp->name);
    mp->name = USP_STRDUP(value);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTTransportProtocol
**
** Function called when Device.MQTT.Client.{i}.TransportProtocol is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTTransportProtocol(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;
    mqtt_tsprotocol_t new_tsprotocol;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Extract the new value
    new_tsprotocol = TEXT_UTILS_StringToEnum(value, mqtt_tsprotocol, NUM_ELEM(mqtt_tsprotocol));
    USP_ASSERT(new_tsprotocol != INVALID); // Value must already have validated to have got here

    // Exit if protocol has not changed
    if (new_tsprotocol == mp->ts_protocol)
    {
        return USP_ERR_OK;
    }

    // Store new protocol
    mp->ts_protocol = new_tsprotocol;

    if ((mp->enable) && (mp->instance != INVALID))
    {
        ScheduleMqttReconnect(mp);
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTCleanSession
**
** Function called when Device.MQTT.Client.{i}.CleanSession is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTCleanSession(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if value has not changed
    if (mp->clean_session == val_bool)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mp->clean_session = val_bool;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTCleanStart
**
** Function called when Device.MQTT.Client.{i}.CleanStart is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTCleanStart(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if value has not changed
    if (mp->clean_start == val_bool)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mp->clean_start = val_bool;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTRequestResponseInfo
**
** Function called when Device.MQTT.Client.{i}.RequestResponseInfo is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTRequestResponseInfo(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if value has not changed
    if (mp->request_response_info == val_bool)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mp->request_response_info = val_bool;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTConnectRetryTime
**
** Function called when Device.MQTT.Client.{i}.ConnectRetryTime is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTConnectRetryTime(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if the value has not changed
    if (mp->retry.connect_retrytime == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value.
    mp->retry.connect_retrytime = val_uint;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTConnectRetryIntervalMultiplier
**
** Function called when Device.MQTT.Client.{i}.ConnectRetryIntervalMultiplier is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTConnectRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if the value has not changed
    if (mp->retry.interval_multiplier == val_int)
    {
        return USP_ERR_OK;
    }

    // Set the new value.
    mp->retry.interval_multiplier = val_int;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTConnectRetryMaxInterval
**
** Function called when Device.MQTT.Client.{i}.ConnectRetryMaxInterval is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTConnectRetryMaxInterval(dm_req_t *req, char *value)
{
    mqtt_conn_params_t *mp;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if the value has not changed
    if (mp->retry.max_interval == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value.
    mp->retry.max_interval = val_uint;

    // Inform the MQTT MTP thread of the new value. This will take effect at the next reconnect.
    MQTT_UpdateConnectionParams(mp, false);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Operate_MQTTForceReconnect
**
** Implements sync command Device.MQTT.Client.{i}.ForceReconnect()
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   command_key - pointer to string containing the command key for this operation
** \param   input_args - vector containing input arguments and their values
** \param   output_args - vector to return output arguments in
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Operate_MQTTForceReconnect(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    mqtt_conn_params_t *mp;
    char *status;

    // Determine mqtt client to be updated
    mp = FindMqttParamsByInstance(inst1);
    USP_ASSERT(mp != NULL);

    // Exit if connection is disabled (reconnect only if the MQTT client is currently connected to the MQTT broker)
    if (mp->enable == false)
    {
        return USP_ERR_OK;
    }

    // Exit if MQTT client is not currently connected to the MQTT broker (do nothing in this case)
    status = (char *) MQTT_GetClientStatus(inst1);
    if (strcmp(status, "Connected") != 0)
    {
        return USP_ERR_OK;
    }

    // Schedule a reconnect
    ScheduleMqttReconnect(mp);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ScheduleMQTTReconnect
**
** Wrapper function to schedule a MQTT reconnect with the current connection parameters
**
** \param   mp - MQTT connection parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ScheduleMqttReconnect(mqtt_conn_params_t *mp)
{
    USP_SAFE_FREE(mp->response_topic);
    mp->response_topic = USP_STRDUP(DEVICE_MTP_GetAgentMqttResponseTopic(mp->instance));
    mp->publish_qos = DEVICE_MTP_GetAgentMqttPublishQos(mp->instance);

    // NOTE: If the agent or controller topics are NULL, this is handled by the MTP layer

    MQTT_UpdateConnectionParams(mp, true);
}

/*********************************************************************//**
**
** ScheduleMQTTResubscribe
**
** Wrapper function to schedule a MQTT resubscribe of the specified MQTT subscription
**
** \param   mqttclient - MQTT client to modify the subscription on
** \param   sub - subscription owned by the MQTT client to change
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ScheduleMQTTResubscribe(client_t *mqttclient, mqtt_subs_config_t *sub)
{
    // Update controller and agent topics. Note: Both could possibly be set to NULL
    USP_SAFE_FREE(mqttclient->conn_params.response_topic);
    mqttclient->conn_params.response_topic = USP_STRDUP(DEVICE_MTP_GetAgentMqttResponseTopic(mqttclient->conn_params.instance));
    mqttclient->conn_params.publish_qos = DEVICE_MTP_GetAgentMqttPublishQos(mqttclient->conn_params.instance);

    if (mqttclient->conn_params.response_topic == NULL)
    {
        USP_LOG_Error("%s response topic not found", __FUNCTION__);
        return;
    }
    if ((mqttclient->conn_params.instance != INVALID) && (sub->instance != INVALID))
    {
    	MQTT_ScheduleResubscription(mqttclient->conn_params.instance, sub);
    }
}


/*********************************************************************//**
**
** ValidateAdd_Mqttclients
**
** Function called to determine whether a new MQTT client may be added
**
** \param   req - pointer to structure identifying the Mqtt client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_Mqttclients(dm_req_t *req)
{
    mqtt_conn_params_t *mp;

    // Exit if unable to find a free MQTT Client
    mp = FindUnusedMqttParams();
    if (mp == NULL)
    {
        USP_LOG_Error("Resources exceeded error");
        return USP_ERR_RESOURCES_EXCEEDED;
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_MQTTClientAdded
**
** Function called when a Mqtt client has been added to Device.MQTT.Client.{i}
**
** \param   req - pointer to structure identifying the MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_MQTTClientAdded(dm_req_t *req)
{
    int err;
    client_t *mqttclient;

    // Exit if failed to copy from DB into mqtt client array
    err = ProcessMqttClientAdded(inst1);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("Process MQTT client add failed");
        return err;
    }

    // Start the connection (if enabled)
    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL);         // As we had just successfully added it

    // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
    err = EnableMQTTClient(mqttclient);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("Enable client failed");
        return err;
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindMQTTParamsByInstance
**
** Finds the mqtt params slot by it's data model instance number
**
** \param   instance - instance number of the MQTT client in the data model
**
** \return  pointer to slot, or NULL if slot was not found
**
**************************************************************************/
mqtt_conn_params_t *FindMqttParamsByInstance(int instance)
{
    int i = 0;
    mqtt_conn_params_t *mp = NULL;

    // Iterate over all MQTT connections
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        // Exit if found a mqtt connection that matches the instance number
        mp = &mqtt_client_params[i].conn_params;
        if (mp->instance == instance)
        {
            return mp;
        }
    }

    USP_LOG_Error("%s: failed", __FUNCTION__);
    // If the code gets here, then no matching slot was found
    return NULL;
}

/*********************************************************************//**
**
** FindDevMqttClientByInstance
**
** Finds the mqtt client by it's connect params instance number
**
** \param   instance - instance number of the MQTT client in the data model
**
** \return  pointer to slot, or NULL if slot was not found
**
**************************************************************************/
client_t *FindDevMqttClientByInstance(int instance)
{
    int i;
    client_t *mqttclient;

    // Iterate over all MQTT connections
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        mqttclient = &mqtt_client_params[i];
        // Exit if found a mqtt connection that matches the instance number
        if (mqttclient->conn_params.instance == instance)
        {
            return mqttclient;
        }
    }

    // If the code gets here, then no matching slot was found
    return NULL;
}
/*************************************************************************
**
** Notify_MqttClientDeleted
**
** Function called when a MQTT Client has been deleted from Device.MQTT.Client.{i}
**
** \param   req - pointer to structure identifying the connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_MqttClientDeleted(dm_req_t *req)
{
    client_t* client;
    client = FindDevMqttClientByInstance(inst1);

    // If we couldn't find it, it's probably ok - must have been deleted?
    if (client == NULL)
    {
        return USP_ERR_OK;
    }

    DestroyMQTTClient(client);

    // Unpick references to this connection
    DEVICE_CONTROLLER_NotifyMqttConnDeleted(inst1);
    DEVICE_MTP_NotifyMqttConnDeleted(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DestroyMQTTClient
**
** Frees all memory associated with the specified MQTT client
**
** \param   mp - pointer to MQTT client to free
**
** \return  None
**
**************************************************************************/
void DestroyMQTTClient(client_t *client)
{
    int i;

    mqtt_conn_params_t* mp = &client->conn_params;

    // Disable the lower level connection
    MQTT_DisableClient(mp->instance);

    // Free and DeInitialise the slot
    MQTT_DestroyConnParams(mp);

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        USP_SAFE_FREE(client->subscriptions[i].topic);
    }
}

/*********************************************************************//**
**
** FindUnusedMqttParams
**
** Finds the first free mqtt params slot
**
** \param   None
**
** \return  Pointer to first free slot, or NULL if no slot was found
**
**************************************************************************/
mqtt_conn_params_t *FindUnusedMqttParams(void)
{
    int i;
    mqtt_conn_params_t *mp;

    // Iterate over all MQTT clients
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        // Exit if found an unused slot
        mp = &mqtt_client_params[i].conn_params;
        if (mp->instance == INVALID)
        {
            return mp;
        }
    }

    // If the code gets here, then no free slot has been found
    USP_ERR_SetMessage("%s: Only %d MQTT clients are supported.", __FUNCTION__, MAX_MQTT_CLIENTS);
    return NULL;
}

/*********************************************************************//**
**
** FindUnusedMqttClient
**
** Finds the first free mqtt client slot
**
** \param   None
**
** \return  Pointer to first free slot, or NULL if no slot was found
**
**************************************************************************/
client_t *FindUnusedMqttClient(void)
{
    int i;
    client_t *mqttclient;

    // Iterate over all MQTT clients
    for (i=0; i<MAX_MQTT_CLIENTS; i++)
    {
        // Exit if found an unused slot
        mqttclient = &mqtt_client_params[i];
        if (mqttclient->conn_params.instance == INVALID)
        {
            return mqttclient;
        }
    }

    // If the code gets here, then no free slot has been found
    USP_ERR_SetMessage("%s: Only %d MQTT clients are supported.", __FUNCTION__, MAX_MQTT_CLIENTS);
    return NULL;
}

/*********************************************************************//**
**
** FindUnusedSubscriptionInMqttClient
**
** Finds the first free mqtt subscription slot within a client
**
** \param   client_t  - pointer to client object to check
**
** \return  Pointer to first free subscription slot, or NULL if no slot was found
**
**************************************************************************/
mqtt_subs_config_t* FindSubscriptionInMqttClient(client_t* client, int instance)
{
    int i;
    mqtt_subs_config_t* sub = NULL;

    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        sub = &client->subscriptions[i];
        if (sub->instance == instance)
        {
            return sub;
        }
    }

    return NULL;
}


/*********************************************************************//**
**
** FindUnusedSubscriptionInMqttClient
**
** Finds the first free mqtt subscription slot within a client
**
** \param   client_t  - pointer to client object to check
**
** \return  Pointer to first free subscription slot, or NULL if no slot was found
**
**************************************************************************/
mqtt_subs_config_t* FindUnusedSubscriptionInMqttClient(client_t* client)
{
    // Just find a subscription with an INVALID instance
    mqtt_subs_config_t* sub = FindSubscriptionInMqttClient(client, INVALID);

    if (sub != NULL)
    {
        // Success - return early
        return sub;
    }

    // If the code gets here, then no free slot has been found
    USP_ERR_SetMessage("%s: Only %d MQTT subscriptions are supported.", __FUNCTION__, MAX_MQTT_SUBSCRIPTIONS);
    return NULL;
}

/*********************************************************************//**
**
** ValidateAdd_MqttClientSubscriptions
**
** Function called to determine whether a new MQTT client Subs may be added
**
** \param   req - pointer to structure identifying the Mqtt client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_MqttClientSubscriptions(dm_req_t *req)
{
    client_t *mqttclient;

    // Exit if unable to find a free MQTT subscription
    mqttclient = FindDevMqttClientByInstance(inst1);
    if (mqttclient == NULL)
    {
        USP_LOG_Error("No matching MQTT client for instance: %d", inst1);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    mqtt_subs_config_t* sub = FindUnusedSubscriptionInMqttClient(mqttclient);
    if (sub == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_MqttClientSubcriptionsAdded
**
** Function called when a Mqtt client subs has been added to
** Device.MQTT.Client.{i}.Subscription.{i}
**
** \param   req - pointer to structure identifying the MQTT client
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_MqttClientSubcriptionsAdded(dm_req_t *req)
{
    int err;
    client_t *mqttclient;
    mqtt_subs_config_t *sub;

    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL); // As we had just successfully added it

    // Exit if failed to copy from DB into mqtt client array
    err = ProcessMqttSubscriptionAdded(inst1, inst2, &sub);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage(" %s: Process MQTT client added failed", __FUNCTION__);
        return err;
    }

    // Exit if unable to propagate the subscription to the MQTT MTP
    err = MQTT_AddSubscription(inst1, sub);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: client subscribe failed", __FUNCTION__);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_MqttClientSubscriptionsDeleted
**
** Function called when a MQTT Client subs has been deleted from
** Device.MQTT.Client.{i}.Subscriptions.{i}.
**
** \param   req - pointer to structure identifying the connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_MqttClientSubscriptionsDeleted(dm_req_t *req)
{
    client_t *mqttclient;
    int err = USP_ERR_OK;
    mqtt_subs_config_t *sub;

    // Exit if unable to find the MQTT client owning the subscription which has been deleted
    mqttclient = FindDevMqttClientByInstance(inst1);
    if (mqttclient == NULL)
    {
        return USP_ERR_OK;
    }

    // Exit if unable to find the subscription which has been deleted
    sub = FindSubscriptionInMqttClient(mqttclient, inst2);
    if (sub == NULL)
    {
        USP_ERR_SetMessage("%s: Delete subscription failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Delete the subscription from device_mqtt
    USP_SAFE_FREE(sub->topic);
    memset(sub, 0, sizeof(mqtt_subscription_t));
    sub->instance = INVALID;

    // Delete the subscription from core mqtt
    err = MQTT_DeleteSubscription(inst1, inst2);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Delete subscription failed", __FUNCTION__);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTSubscriptionEnable
**
** Validates Device.MQTT.Client.{i}.Subscription.{i}.Enable
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTSubscriptionEnable(dm_req_t *req, char *value)
{
    client_t *mqttclient;
    mqtt_subs_config_t *sub;

    // Exit if the MQTT client or subscription does not exist yet in our local data structure
    // NOTE: this could occur if this validate is being called as part of a USP Add, before the notify vendor hook.
    //       In this case, the validate for the topic will prevent an empty topic, so we don't need to prevent an enable of a pre-existing empty topic
    mqttclient = FindDevMqttClientByInstance(inst1);
    if (mqttclient == NULL)
    {
        return USP_ERR_OK;
    }

    sub = FindSubscriptionInMqttClient(mqttclient, inst2);
    if (sub == NULL)
    {
        return USP_ERR_OK;
    }

    // Exit if trying to enable a subscription which has an empty topic. This is not allowed.
    if ((val_bool == true) &&
        ((sub->topic == NULL) || (sub->topic[0] == '\0')) )
    {
        USP_ERR_SetMessage("%s: Cannot enable subscription with empty topic", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_MQTTSubscriptionEnable
**
** Function called when Device.MQTT.Connection.{i}.Subscription.{j}.
** Enable is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTSubscriptionEnable(dm_req_t *req, char *value)
{
    client_t *mqttclient;
    mqtt_subs_config_t *sub;
    bool old_value;

    // Initialise to defaults
    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL);

    sub = FindSubscriptionInMqttClient(mqttclient, inst2);
    if (sub == NULL)
    {
        USP_ERR_SetMessage("%s: Subscription enable change failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    old_value = sub->enabled;

    // Exit early if enable has not changed
    if (old_value == val_bool)
    {
        return USP_ERR_OK;
    }

    // Store new subscription state
    sub->enabled = val_bool;

    if ((mqttclient->conn_params.instance != INVALID) && (sub->instance != INVALID))
    {
        ScheduleMQTTResubscribe(mqttclient, sub);
    }
    return USP_ERR_OK;
}

/*************************************************************************
**
** NotifyChange_MQTTSubscriptionTopic
**
** Function called when Device.MQTT.Client.{i}.Subscriptions.{j}.Topic
** is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTSubscriptionTopic(dm_req_t *req, char *value)
{
    client_t *mqttclient;
    mqtt_subs_config_t *sub;
    bool schedule_reconnect = false;

    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL);

    // inst2 is the subscriptions starting from 1, not 0
    // So we need -1
    sub = FindSubscriptionInMqttClient(mqttclient, inst2);
    if (sub == NULL)
    {
        USP_ERR_SetMessage("%s: Subscription topic change failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    if((strcmp(sub->topic, value) != 0) && (sub->enabled))
    {
        schedule_reconnect = true;
    }

    // Set the new value.
    // This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(sub->topic);
    sub->topic = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect && (sub->instance != INVALID))
    {
        ScheduleMQTTResubscribe(mqttclient, sub);
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTSubscriptionQoS
**
** Validates Device.MQTT.Client.{i}.Subscriptions.{j}.QoS by checking if valid number
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTSubscriptionQoS(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, kMqttQos_MostOnce, kMqttQos_ExactlyOnce);
}
/*************************************************************************
**
** NotifyChange_MQTTSubscriptionQoS
**
** Function called when Device.MQTT.Client.{i}.Subscriptions.{j}.QoS
** is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_MQTTSubscriptionQoS(dm_req_t *req, char *value)
{
    client_t *mqttclient;
    mqtt_subs_config_t *sub;
    bool schedule_reconnect = false;

    // Initialise to defaults
    mqttclient = FindDevMqttClientByInstance(inst1);
    USP_ASSERT(mqttclient != NULL);

    // inst2 is the subscription index starting from 1
    // Not 0, so we need to -1 to the array index
    sub = FindSubscriptionInMqttClient(mqttclient, inst2);
    if (sub == NULL)
    {
        USP_ERR_SetMessage("%s: Subscription QoS change failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    if (sub->enabled && (sub->qos != val_uint))
    {
        schedule_reconnect = true;
    }

    sub->qos = val_uint;

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect && (sub->instance != INVALID))
    {
        ScheduleMQTTResubscribe(mqttclient, sub);
    }
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_MQTTSubscriptionTopic
**
** Validates that a new Device.MQTT.Client.{i}.Subscriptions.{i}.Topic is unique in the table
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_MQTTSubscriptionTopic(dm_req_t *req, char *value)
{
    int i;
    client_t *client;
    mqtt_subs_config_t *sub;

    // Exit if no value set for Topic (i.e. set to an empty string)
    if (*value == '\0')
    {
        USP_ERR_SetMessage("%s: Topic must be set to a non-empty string", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if the MQTT client does not exist yet in our local data structure
    // NOTE: this could occur if this validate is being called as part of a USP Add of both client and subs, before the notify vendor hook.
    //       In this rare case (adding MQTT client and multiple child subscriptions using a single USP Add messsage), the code does not prevent duplicate topics.
    //       To fix this, the code in this function needs to get the values of the other topics from the USP DB, rather than using our local data structure
    client = FindDevMqttClientByInstance(inst1);
    if (client == NULL)
    {
        return USP_ERR_OK;
    }

    // Iterate over all subscriptions for this MQTT client
    for (i=0; i<MAX_MQTT_SUBSCRIPTIONS; i++)
    {
        // Exit if the topic already exists in another subscription
        sub = &client->subscriptions[i];
        if ((sub->instance != INVALID) && (sub->instance != inst2) &&
            (sub->topic != NULL) && (strcmp(sub->topic, value)==0))
        {
            USP_ERR_SetMessage("%s: Topic (%s) is already in use by Subscription.%d", __FUNCTION__, value, sub->instance);
            return USP_ERR_UNIQUE_KEY_CONFLICT;
        }
    }

    // If the code gets here, then the new topic is unique for this MQTT client
    return USP_ERR_OK;
}

#endif
