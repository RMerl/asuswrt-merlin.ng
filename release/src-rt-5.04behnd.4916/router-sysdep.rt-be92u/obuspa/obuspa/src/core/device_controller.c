/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020,  BT PLC
 * Copyright (C) 2022, Snom Technology GmbH
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
 * \file device_controller.c
 *
 * Implements the Device.LocalAgent.Controller data model object
 *
 */
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#include "common_defs.h"
#include "device.h"
#include "data_model.h"
#include "usp_api.h"
#include "dm_access.h"
#include "dm_trans.h"
#include "dm_exec.h"
#include "mtp_exec.h"
#include "msg_handler.h"
#include "text_utils.h"
#include "iso8601.h"
#include "retry_wait.h"
#include "usp_record.h"
#include "subs_retry.h"
#include "usp_broker.h"
#include "os_utils.h"

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include "e2e_defs.h"
#include "e2e_context.h"
#endif

#ifndef DISABLE_STOMP
#include "stomp.h"
#endif

#ifdef ENABLE_COAP
#include "usp_coap.h"
#endif

#ifdef ENABLE_MQTT
#include "mqtt.h"
#endif

#ifdef ENABLE_WEBSOCKETS
#include "wsclient.h"
#include "wsserver.h"
#endif

#ifdef ENABLE_UDS
#include "uds.h"
#endif

//------------------------------------------------------------------------------
// Location of the controller table within the data model
#define DEVICE_CONT_ROOT "Device.LocalAgent.Controller"
static const char device_cont_root[] = DEVICE_CONT_ROOT;

//------------------------------------------------------------------------------
// Time at which next periodic notification should fire
static time_t first_periodic_notification_time = END_OF_TIME;

//------------------------------------------------------------------------------------
// Mutex used to protect access to can_all_mtp_start variable
static pthread_mutex_t can_mtp_connect_mutex;

//------------------------------------------------------------------------------
// Variable used to determine whether MTP client connections for STOMP, CoAP, MQTT and Websockets are allowed to connect
static volatile bool can_mtp_connect = false;

// Timeout for can_mtp_connect vendor hook
static char pre_connect_timeout_param[] = "Device.LocalAgent.X_VANTIVA-COM_PreConnectTimeout";
static time_t pre_connect_timeout = 0;

//------------------------------------------------------------------------------
// Structure representing entries in the Device.LocalAgent.Controller.{i}.MTP.{i} table
typedef struct
{
    int instance;         // instance of the MTP in the Device.LocalAgent.Controller.{i}.MTP.{i} table
                          // This value will be marked as INVALID, if the entry is not currently being used
    bool enable;
    mtp_protocol_t protocol;

    // NOTE: The following is not a union, because the data model would allow both MTP.{i}.STOMP and MTP.{i}.CoAP objects to be seeded at the same time - with protocol choosing which one is active
#ifndef DISABLE_STOMP
    int stomp_connection_instance;
    char *stomp_controller_queue;
#endif

#ifdef ENABLE_COAP
    char *coap_controller_host;
    coap_config_t coap;
#endif

#ifdef ENABLE_MQTT
    int mqtt_connection_instance;
    char *mqtt_controller_topic;
#endif

#ifdef ENABLE_WEBSOCKETS
    wsclient_config_t websock;
#endif

#ifdef ENABLE_UDS
    int uds_connection_instance;
#endif

} controller_mtp_t;

//------------------------------------------------------------------------------
// Define for default value of Device.LocalAgent.Controller.{i}.MTP.{i}.Protocol
// We attempt to use WebSockets if present (TR181-2-15-1), falling back to an enabled MTP
#if defined(ENABLE_WEBSOCKETS)
    #define DEFAULT_CONTROLLER_MTP "WebSocket"
#elif !defined(DISABLE_STOMP)
    #define DEFAULT_CONTROLLER_MTP "STOMP"
#elif defined(ENABLE_MQTT)
    #define DEFAULT_CONTROLLER_MTP "MQTT"
#elif defined(ENABLE_COAP)
    #define DEFAULT_CONTROLLER_MTP "CoAP"
#else
    #define DEFAULT_CONTROLLER_MTP ""
#endif

//------------------------------------------------------------------------------
// Structure representing entries in the Device.LocalAgent.Controller.{i} table
typedef struct
{
    int instance;      // instance of the controller in the Device.LocalAgent.Controller.{i} table
                       // This value will be marked as INVALID, if the entry is not currently being used
    bool enable;
    char *endpoint_id;
    controller_mtp_t mtps[MAX_CONTROLLER_MTPS];  // Array of controller MTPs

    time_t periodic_base;
    unsigned periodic_interval;
    time_t next_time_to_fire;   // Absolute time at which periodic notification should fire for this controller
    int inherited_instance;     // Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
    int assigned_instance;      // Assigned role instance in Device.LocalAgent.ControllerTrust.Role.{i}

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    e2e_session_t e2e_session;
#endif
    unsigned subs_retry_min_wait_interval;
    unsigned subs_retry_interval_multiplier;

} controller_t;

// Array of controllers
static controller_t controllers[MAX_CONTROLLERS];

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int SendOnBoardRequest(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);
int ExecuteSendOnBoardRequest(controller_t* controller);
void SendOnBoardRequestNotify(Usp__Msg *req, controller_t* controller);
void PeriodicNotificationExec(int id);
int ValidateAdd_Controller(dm_req_t *req);
int ValidateAdd_ControllerMtp(dm_req_t *req);
int Notify_ControllerAdded(dm_req_t *req);
int Notify_ControllerDeleted(dm_req_t *req);
int Notify_ControllerMtpAdded(dm_req_t *req);
int Notify_ControllerMtpDeleted(dm_req_t *req);
int Validate_ControllerEndpointID(dm_req_t *req, char *value);
int Validate_ControllerMtpEnable(dm_req_t *req, char *value);
int Validate_ControllerMtpProtocol(dm_req_t *req, char *value);
int Validate_PeriodicNotifInterval(dm_req_t *req, char *value);
int Validate_ControllerRetryMinimumWaitInterval(dm_req_t *req, char *value);
int Validate_ControllerRetryIntervalMultiplier(dm_req_t *req, char *value);
int Validate_SessionRetryInterval(dm_req_t *req, char *value);
int Validate_SessionRetryMultiplier(dm_req_t *req, char *value);
int Validate_PeriodicNotifInterval(dm_req_t *req, char *value);
int Notify_ControllerEnable(dm_req_t *req, char *value);
int Notify_ControllerEndpointID(dm_req_t *req, char *value);
int Notify_ControllerMtpEnable(dm_req_t *req, char *value);
int Notify_ControllerMtpProtocol(dm_req_t *req, char *value);
int Notify_PeriodicNotifInterval(dm_req_t *req, char *value);
int Notify_PeriodicNotifTime(dm_req_t *req, char *value);
int Notify_ControllerRetryMinimumWaitInterval(dm_req_t *req, char *value);
int Notify_ControllerRetryIntervalMultiplier(dm_req_t *req, char *value);
int Get_ControllerInheritedRole(dm_req_t *req, char *buf, int len);
int ProcessControllerAdded(int cont_instance);
int ProcessControllerMtpAdded(controller_t *cont, int mtp_instance);
controller_t *FindUnusedController(void);
controller_mtp_t *FindUnusedControllerMtp(controller_t *cont);
controller_mtp_t *FindControllerMtpFromReq(dm_req_t *req, controller_t **p_cont);
controller_t *FindControllerByInstance(int cont_instance);
controller_t *FindControllerByEndpointId(char *endpoint_id);
controller_t *FindEnabledControllerByEndpointId(char *endpoint_id);
controller_mtp_t *FindFirstEnabledMtp(controller_t *cont, mtp_protocol_t preferred_protocol);
controller_mtp_t *FindControllerMtpByInstance(controller_t *cont, int mtp_instance);
controller_mtp_t *FindControllerMtpByProtocol(controller_t *cont, mtp_protocol_t protocol);
void DestroyController(controller_t *cont);
void DestroyControllerMtp(controller_mtp_t *mtp);
int ValidateEndpointIdUniqueness(char *endpoint_id, int instance);
time_t CalcNextPeriodicTime(time_t cur_time, time_t periodic_base, int periodic_interval);
void UpdateFirstPeriodicNotificationTime(void);
int Validate_ControllerAssignedRole(dm_req_t *req, char *value);
int Notify_ControllerAssignedRole(dm_req_t *req, char *value);
int UpdateAssignedRole(controller_t *cont, char *reference);
int ValidateMtpUniqueness(mtp_protocol_t protocol, int cont_inst, int mtp_inst);
int ValidateMtpResourceAvailable(mtp_protocol_t protocol, int cont_inst, int mtp_inst);
int CalcNotifyDest(char *endpoint_id, controller_t *cont, controller_mtp_t *mtp, mtp_conn_t *dest);
int QueueBinaryMessageOnMtp(mtp_send_item_t *msi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtpc, time_t expiry_time);
int CopyNotifyDestForEndpoint(char *endpoint_id, mtp_protocol_t protocol, Usp__Header__MsgType usp_msg_type, mtp_conn_t *dest);
void StartAllMtpClients(void);
void PollCanMtpConnect(int id);
bool UpdateCanMtpConnect(void);
void StartAllWebsocketClients(void);
void StartAllCoAPClients(void);
void AllowConnectOnAllMtpClients(void);
void UpdateNextPeriodicTime(void);

#ifndef DISABLE_STOMP
int Notify_ControllerMtpStompReference(dm_req_t *req, char *value);
int Notify_ControllerMtpStompDestination(dm_req_t *req, char *value);
#endif

#ifdef ENABLE_COAP
int Notify_ControllerMtpCoapHost(dm_req_t *req, char *value);
int Notify_ControllerMtpCoapPort(dm_req_t *req, char *value);
int Notify_ControllerMtpCoapPath(dm_req_t *req, char *value);
int Notify_ControllerMtpCoapEncryption(dm_req_t *req, char *value);
#endif

#ifdef ENABLE_MQTT
int Notify_ControllerMtpMqttReference(dm_req_t *req, char *value);
int Notify_ControllerMtpMqttTopic(dm_req_t *req, char *value);
#endif

#ifdef ENABLE_WEBSOCKETS
int Validate_ControllerMtpWebsockKeepAlive(dm_req_t *req, char *value);
int Get_WebsockRetryCount(dm_req_t *req, char *buf, int len);
int Notify_ControllerMtpWebsockHost(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockPort(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockPath(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockEncryption(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockKeepAlive(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockRetryInterval(dm_req_t *req, char *value);
int Notify_ControllerMtpWebsockRetryMultiplier(dm_req_t *req, char *value);
void SwitchMtpDestIfNotConnected(char *endpoint_id, mtp_conn_t *mtpc);
#endif

#ifdef ENABLE_UDS
int Notify_ControllerMtpUdsReference(dm_req_t *req, char *value);
int Get_ControllerMtpUspServiceRef(dm_req_t *req, char *buf, int len);
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
int ProcessControllerE2ESessionAdded(controller_t *cont);
int Validate_E2ESessionMode(dm_req_t *req, char *value);
int Validate_E2ESessionMaxUSPRecordSize(dm_req_t *req, char *value);
int Notify_E2ESessionMode(dm_req_t *req, char *value);
int Notify_E2ESessionMaxUSPRecordSize(dm_req_t *req, char *value);
int Get_E2ESessionStatus(dm_req_t *req, char *buf, int len);
int Async_E2ESessionReset(dm_req_t *req, kv_vector_t *input_args, int request);

extern const enum_entry_t e2e_session_modes[kE2EMode_Max];
#endif

/*********************************************************************//**
**
** DEVICE_CONTROLLER_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CONTROLLER_Init(void)
{
    int err = USP_ERR_OK;
    int i, j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Add timer to be called back when first periodic notification fires
    first_periodic_notification_time = END_OF_TIME;
    SYNC_TIMER_Add(PeriodicNotificationExec, 0, first_periodic_notification_time);

    // Mark all controller and mtp slots as unused
    memset(controllers, 0, sizeof(controllers));
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        cont->instance = INVALID;

        for (j=0; j<MAX_CONTROLLER_MTPS; j++)
        {
            mtp = &cont->mtps[j];
            mtp->instance = INVALID;
        }
    }

    // Register parameters implemented by this component
    err |= USP_REGISTER_Object(DEVICE_CONT_ROOT ".{i}", ValidateAdd_Controller, NULL, Notify_ControllerAdded,
                                                        NULL, NULL, Notify_ControllerDeleted);
    err |= USP_REGISTER_Object(DEVICE_CONT_ROOT ".{i}.MTP.{i}", ValidateAdd_ControllerMtp, NULL, Notify_ControllerMtpAdded,
                                                                NULL, NULL, Notify_ControllerMtpDeleted);
    err |= USP_REGISTER_DBParam_Alias(DEVICE_CONT_ROOT ".{i}.Alias", NULL);
    err |= USP_REGISTER_DBParam_Alias(DEVICE_CONT_ROOT ".{i}.MTP.{i}.Alias", NULL);

    err |= USP_REGISTER_Param_NumEntries("Device.LocalAgent.ControllerNumberOfEntries", DEVICE_CONT_ROOT ".{i}");
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.Enable", "false", NULL, Notify_ControllerEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.EndpointID", "", Validate_ControllerEndpointID, Notify_ControllerEndpointID, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CONT_ROOT ".{i}.InheritedRole", Get_ControllerInheritedRole, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.AssignedRole", "", Validate_ControllerAssignedRole, Notify_ControllerAssignedRole, DM_STRING);

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.PeriodicNotifInterval", "86400", Validate_PeriodicNotifInterval, Notify_PeriodicNotifInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.PeriodicNotifTime", UNKNOWN_TIME_STR, NULL, Notify_PeriodicNotifTime, DM_DATETIME);
    err |= USP_REGISTER_Event("Device.LocalAgent.Periodic!");

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.USPNotifRetryMinimumWaitInterval", "5", Validate_ControllerRetryMinimumWaitInterval, Notify_ControllerRetryMinimumWaitInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.USPNotifRetryIntervalMultiplier", "2000", Validate_ControllerRetryIntervalMultiplier, Notify_ControllerRetryIntervalMultiplier, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.ControllerCode", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.ProvisioningCode", "", NULL, NULL, DM_STRING);

    err |= USP_REGISTER_SyncOperation(DEVICE_CONT_ROOT ".{i}.SendOnBoardRequest()", SendOnBoardRequest);

    err |= USP_REGISTER_Param_NumEntries(DEVICE_CONT_ROOT ".{i}.MTPNumberOfEntries", "Device.LocalAgent.Controller.{i}.MTP.{i}");
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.Enable", "false", Validate_ControllerMtpEnable, Notify_ControllerMtpEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.Protocol", DEFAULT_CONTROLLER_MTP, Validate_ControllerMtpProtocol, Notify_ControllerMtpProtocol, DM_STRING);

#ifndef DISABLE_STOMP
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.STOMP.Reference", "", DEVICE_MTP_ValidateStompReference, Notify_ControllerMtpStompReference, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.STOMP.Destination", "", NULL, Notify_ControllerMtpStompDestination, DM_STRING);
#endif

#ifdef ENABLE_COAP
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.CoAP.Host", "", NULL, Notify_ControllerMtpCoapHost, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.CoAP.Port", "5683", DM_ACCESS_ValidatePort, Notify_ControllerMtpCoapPort, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.CoAP.Path", "", NULL, Notify_ControllerMtpCoapPath, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.CoAP.EnableEncryption", "true", NULL, Notify_ControllerMtpCoapEncryption, DM_BOOL);
#endif

#ifdef ENABLE_MQTT
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.MQTT.Reference", "", DEVICE_MTP_ValidateMqttReference, Notify_ControllerMtpMqttReference, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.MQTT.Topic", "", NULL, Notify_ControllerMtpMqttTopic, DM_STRING);
#endif

#ifdef ENABLE_WEBSOCKETS
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.Host", "", NULL, Notify_ControllerMtpWebsockHost, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.Port", "80", DM_ACCESS_ValidatePort, Notify_ControllerMtpWebsockPort, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.Path", "", NULL, Notify_ControllerMtpWebsockPath, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.EnableEncryption", "true", NULL, Notify_ControllerMtpWebsockEncryption, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.KeepAliveInterval", "30", Validate_ControllerMtpWebsockKeepAlive, Notify_ControllerMtpWebsockKeepAlive, DM_UINT);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.CurrentRetryCount", Get_WebsockRetryCount, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.SessionRetryMinimumWaitInterval", "5", Validate_SessionRetryInterval, Notify_ControllerMtpWebsockRetryInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.WebSocket.SessionRetryIntervalMultiplier", "2000", Validate_SessionRetryMultiplier, Notify_ControllerMtpWebsockRetryMultiplier, DM_UINT);
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.E2ESession.SessionMode", "Allow", Validate_E2ESessionMode, Notify_E2ESessionMode, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.E2ESession.MaxUSPRecordSize", "0", Validate_E2ESessionMaxUSPRecordSize, Notify_E2ESessionMaxUSPRecordSize, DM_UINT);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CONT_ROOT ".{i}.E2ESession.Status", Get_E2ESessionStatus, DM_STRING);
    err |= USP_REGISTER_AsyncOperation(DEVICE_CONT_ROOT ".{i}.E2ESession.Reset()", Async_E2ESessionReset, NULL);
#endif

#ifdef ENABLE_UDS
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_CONT_ROOT ".{i}.MTP.{i}.UDS.UnixDomainSocketRef", "", NULL, Notify_ControllerMtpUdsReference, DM_STRING);
#ifndef REMOVE_USP_BROKER
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CONT_ROOT ".{i}.MTP.{i}.UDS.USPServiceRef", Get_ControllerMtpUspServiceRef, DM_STRING);
#endif
#endif

    err |= USP_REGISTER_DBParam_ReadWrite(pre_connect_timeout_param, "60", NULL, NULL, DM_UINT);

    // Register unique keys for all tables
    char *cont_unique_keys[] = { "EndpointID" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_CONT_ROOT ".{i}", cont_unique_keys, NUM_ELEM(cont_unique_keys));

    char *mtp_unique_keys[] = { "Protocol" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_CONT_ROOT ".{i}.MTP.{i}", mtp_unique_keys, NUM_ELEM(mtp_unique_keys));

    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to create mutex protecting access to the 'can_all_mtp_connect' flag
    err = OS_UTILS_InitMutex(&can_mtp_connect_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_Start
**
** Initialises the controllers array with the values of all controllers from the DB
** NOTE: If the database contains invalid data, then entries will be deleted
**       We need to do this otherwise it would be possible to set bad DB values to good,
**       but our code would not pick them up because they were not in the internal data structure
**       This function ensures that the database and the internal controller data structure it populates always match
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CONTROLLER_Start(void)
{
    int i, j;
    int err;
    int_vector_t iv;
    int cont_instance;
    controller_t *cont;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];
    int count;
    unsigned timeout;

    // Exit if unable to calculate the MTP connect timeout
    err = DM_ACCESS_GetUnsigned(pre_connect_timeout_param, &timeout);
    if (err != USP_ERR_OK)
    {
        return err;
    }
    pre_connect_timeout = OS_UTILS_TimeNow() + timeout;

    // Exit if unable to get the object instance numbers present in the controllers table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(DEVICE_CONT_ROOT, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit, issuing a warning, if no controllers are present in database
    if (iv.num_entries == 0)
    {
        USP_LOG_Warning("%s: WARNING: No instances in %s", __FUNCTION__, device_cont_root);
        err = USP_ERR_OK;
        goto exit;
    }

    // Add all controllers from the controllers table to the controllers array
    for (i=0; i < iv.num_entries; i++)
    {
        cont_instance = iv.vector[i];
        err = ProcessControllerAdded(cont_instance);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a controller with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_cont_root, cont_instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    // Count number of enabled MTPs
    count = 0;
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable))
                {
                    count++;
                }
            }
        }
    }

    // Display a warning of no controller MTPs are enabled
    if (count==0)
    {
        USP_LOG_Warning("%s: WARNING: No enabled MTPs in %s.{i}.MTP. USP Agent may only be usable via the CLI", __FUNCTION__, device_cont_root);
    }

    err = USP_ERR_OK;

exit:
    // Destroy the vector of instance numbers for the table
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_Stop
**
** Frees up all memory associated with this module
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_Stop(void)
{
    int i;
    controller_t *cont;

    // Iterate over all controllers, freeing all memory used by them
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            DestroyController(cont);
        }
    }
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindInstanceByEndpointId
**
** Gets the instance number of the enabled controller (in Device.LocalAgent.Controller.{i}) based on the specified endpoint_id
**
** \param   endpoint_id - controller that we want to find the instance number of
**
** \return  instance number of controller, or INVALID if unable to find the enabled controller
**
**************************************************************************/
int DEVICE_CONTROLLER_FindInstanceByEndpointId(char *endpoint_id)
{
    controller_t *cont;

    // Exit if no endpoint_id set by caller
    if (endpoint_id == NULL)
    {
        return INVALID;
    }

    // Exit if unable to find a matching, enabled controller
    cont = FindEnabledControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        return INVALID;
    }

    // Found the matching instance
    return cont->instance;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindEndpointIdByInstance
**
** Gets the endpoint_id of the specified enabled controller
**
** \param   instance - instance number of the controller in the Device.LocalAgent.Controller.{i} table
**
** \return  pointer to endpoint_id of the controller, or NULL if no controller found, or controller was disabled
**
**************************************************************************/
char *DEVICE_CONTROLLER_FindEndpointIdByInstance(int instance)
{
    controller_t *cont;

    // Exit if unable to find a matching, enabled controller
    cont = FindControllerByInstance(instance);
    if ((cont == NULL) || (cont->enable == false))
    {
        return NULL;
    }

    // Found the matching instance
    return cont->endpoint_id;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindEndpointByMTP
**
** Finds the endpoint_id of the first controller connected to the MTP on which we received a packet
**
** \param   mtpc - pointer to structure specifying which protocol (and MTP instance) a packet was received on
**
** \return  endpoint_id or NULL if unable to determine one
**
**************************************************************************/
char *DEVICE_CONTROLLER_FindEndpointByMTP(mtp_conn_t *mtpc)
{
    int i, j;
    controller_t *cont;
    controller_mtp_t *mtp;

    USP_ASSERT(mtpc->protocol != kMtpProtocol_None);

#ifdef ENABLE_WEBSOCKETS
    // Exit if packet was received on agent's websocket server and endpoint_id was not provided to the MTP in the
    // Sec-WebSocket-Extensions. We can't use the data model configuration to determine the Controller's endpoint_id in this case
    if ((mtpc->protocol == kMtpProtocol_WebSockets) && (mtpc->ws.serv_conn_id != INVALID))
    {
        return NULL;
    }
#endif

#ifdef ENABLE_COAP
    // Exit if packet was received on agent's CoAP server. In this case, it's not possible to determine endpoint_id from the data model configuration
    if (mtpc->protocol == kMtpProtocol_CoAP)
    {
        return NULL;
    }
#endif

    // Iterate over all enabled controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable == true))
        {
            // Iterate over all enabled MTPs which match the protocol that the packet was received on
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable == true) && (mtp->protocol == mtpc->protocol))
                {
                    // Return this controller's endpoint_id, if this MTP matches the MTP that the packet was received on
                    switch(mtp->protocol)
                    {
#ifndef DISABLE_STOMP
                        case kMtpProtocol_STOMP:
                            if (mtp->stomp_connection_instance == mtpc->stomp.instance)
                            {
                                return cont->endpoint_id;
                            }
                            break;
#endif

#ifdef ENABLE_MQTT
                        case kMtpProtocol_MQTT:
                            if (mtp->mqtt_connection_instance == mtpc->mqtt.instance)
                            {
                                return cont->endpoint_id;
                            }
                            break;
#endif

#ifdef ENABLE_WEBSOCKETS
                        case kMtpProtocol_WebSockets:
                            // NOTE: This is Websocket client case. Websocket server case already handled earlier in this function
                            USP_ASSERT(mtpc->ws.serv_conn_id == INVALID);
                            if ((mtpc->ws.client_cont_instance == cont->instance) && (mtpc->ws.client_mtp_instance == mtp->instance))
                            {
                                return cont->endpoint_id;
                            }
                            break;
#endif

                        default:
                            TERMINATE_BAD_CASE(mtp->protocol);
                            break;
                    }
                }
            }
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_GetSubsRetryParams
**
** Gets the subscription retry parameters for the specified endpoint_id
**
** \param   endpoint_id - controller that we want to get the retry parameters for
** \param   min_wait_interval - pointer to variable in which to return the minimum wait interval
** \param   interval_multiplier - pointer to variable in which to return the interval multiplier
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CONTROLLER_GetSubsRetryParams(char *endpoint_id, unsigned *min_wait_interval, unsigned *interval_multiplier)
{
    controller_t *cont;

    // Exit if unable to find a matching, enabled controller
    cont = FindEnabledControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        USP_LOG_Warning("%s: Unable to find enabled controller with endpoint_id=%s", __FUNCTION__, endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Found the matching instance, so copy the retry params
    *min_wait_interval = cont->subs_retry_min_wait_interval;
    *interval_multiplier = cont->subs_retry_interval_multiplier;
    return USP_ERR_OK;
}

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindE2ESessionByInstance
**
** Find the E2E Session to use with specified enabled controller
**
** \param   instance - instance number of the controller in Device.LocalAgent.Controller.{i}
**
** \return  pointer to E2ESession instance, or NULL
**
**************************************************************************/
e2e_session_t *DEVICE_CONTROLLER_FindE2ESessionByInstance(int instance)
{
    // Exit if unable to find a matching enabled controller
    controller_t *cont = FindControllerByInstance(instance);
    if ((cont == NULL) || (cont->enable == false))
    {
        return NULL;
    }

    // Return pointer to the E2ESession instance
    return &cont->e2e_session;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindE2ESessionByEndpointId
**
** Find the E2E Session to use with the controller matching the specified endpoint_id
**
** \param   endpoint_id - name of the controller to find
**
** \return  pointer to E2ESession instance, or NULL
**
**************************************************************************/
e2e_session_t *DEVICE_CONTROLLER_FindE2ESessionByEndpointId(char *endpoint_id)
{
    // Exit if unable to find a matching enabled controller
    controller_t *cont = FindControllerByEndpointId(endpoint_id);
    if ((cont == NULL) || (cont->enable == false))
    {
        return NULL;
    }

    // Return pointer to the E2ESession instance
    return &cont->e2e_session;
}
#endif

/*********************************************************************//**
**
** DEVICE_CONTROLLER_GetCombinedRoleByInstance
**
** Gets the inherited and assigned role to use for the specified controller instance
** This is used when resolving paths used by subscriptions
**
** \param   instance - instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   combined_role - pointer to variable in which to return the combined role
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CONTROLLER_GetCombinedRoleByInstance(int instance, combined_role_t *combined_role)
{
    controller_t *cont;

    // Exit if unable to find a matching enabled controller
    cont = FindControllerByInstance(instance);
    if ((cont == NULL) || (cont->enable == false))
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Calculate the role indexes to be used for Assigned and Inherited roles
    combined_role->assigned_index = DEVICE_CTRUST_RoleInstanceToIndex(cont->assigned_instance);
    combined_role->inherited_index = DEVICE_CTRUST_RoleInstanceToIndex(cont->inherited_instance);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_GetCombinedRoleByEndpointId
**
** Gets the combined role to use for the specified controller endpoint_id, when
** processing request messages from that controller
**
** \param   endpoint_id - endpoint_id of the controller that has sent the current message being processed
** \param   role_instance - Inherited role in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   protocol - protocol that the message was received on
** \param   combined_role - pointer to structure to return combined role in
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_GetCombinedRoleByEndpointId(char *endpoint_id, int role_instance, mtp_protocol_t protocol, combined_role_t *combined_role)
{
    controller_t *cont;

    // Exit if unable to find a matching enabled controller
    cont = FindEnabledControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        // If this is an unknown controller, then grant it a limited set of permissions
        combined_role->inherited_index = DEVICE_CTRUST_RoleInstanceToIndex(ROLE_UNTRUSTED);
        combined_role->assigned_index = INVALID;
        return;
    }

    // Save the inherited role that the MTP cert has granted the controller sending this message (if necessary)
    switch(protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            // No need to save for STOMP - Already setup by DEVICE_CONTROLLER_SetRolesFromStomp
            break;
#endif
#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            // No need to save for MQTT - Already setup by DEVICE_CONTROLLER_SetRolesFromMqtt
            break;
#endif

        default:
            cont->inherited_instance = role_instance;
            break;
    }

    // Calculate the role indexes to be used for Assigned and Inherited roles
    combined_role->assigned_index = DEVICE_CTRUST_RoleInstanceToIndex(cont->assigned_instance);
    combined_role->inherited_index = DEVICE_CTRUST_RoleInstanceToIndex(cont->inherited_instance);
}

#ifndef DISABLE_STOMP
/*********************************************************************//**
**
** DEVICE_CONTROLLER_SetRolesFromStomp
**
** Sets the controller trust role to use for all controllers connected to the specified STOMP controller
**
** \param   stomp_instance - STOMP instance (in Device.STOMP.Connection table)
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_SetRolesFromStomp(int stomp_instance, int role_instance)
{
    int i, j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Iterate over all enabled controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            // Iterate over all enabled MTP slots for this controller
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable))
                {
                    // If this controller is connected to the specified STOMP connection, then set its inherited role
                    if ((mtp->protocol == kMtpProtocol_STOMP) && (mtp->stomp_connection_instance == stomp_instance))
                    {
                        cont->inherited_instance = role_instance;
                    }
                }
            }
        }
    }
}
#endif

/*********************************************************************//**
**
** DEVICE_CONTROLLER_QueueBinaryMessage
**
** Queues a binary message to be sent to a controller
** NOTE: This function determines the destination MTP, if the message is a USP notification
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is passed to this function, unless an error is returned.
** \param   endpoint_id - controller to send the message to
** \param   usp_msg_id - pointer to string containing the msg_id of the serialized USP Message
** \param   mtpc - details of where this USP response message should be sent. NOTE: This may not be specified if the message is a notification
** \param   expiry_time - time at which the USP message should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_CONTROLLER_QueueBinaryMessage(mtp_send_item_t *msi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtpc, time_t expiry_time)
{
    int err = USP_ERR_GENERAL_FAILURE;
    mtp_conn_t dest;
    USP_ASSERT(msi != NULL);

    // Take a copy of the MTP destination parameters we've been given
    // because we may modify it (and we don't want the caller to free anything we put in it, as they are owned by the data model)
    memcpy(&dest, mtpc, sizeof(dest));

    // If 'reply-to' was not specified, then use the configured MTP to fill in where the response should be sent
    // This is always the case for notifications, since they are not a response to any incoming USP message
    if (mtpc->is_reply_to_specified == false)
    {
        err = CopyNotifyDestForEndpoint(endpoint_id, mtpc->protocol, msi->usp_msg_type, &dest);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    // Send the response
    // NOTE: Ownership of msi->pbuf passes to the MTP, if successful
    err = QueueBinaryMessageOnMtp(msi, endpoint_id, usp_msg_id, &dest, expiry_time);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // NOTE: member variables in dest must NOT be freed as they are owned elsewhere
    // They are either owned by the caller (in mtpc) or owned by the data model (in controllers[].mtps[])

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_IsMTPConfigured
**
** Determines whether an MTP has been configured to send messages to the specified controller
** This function is used by ValidateUspRecord() to determine whether to process a received USP message
**
** \param   endpoint_id - Endpoint ID of controller that send a USP message
** \param   protocol - protocol on which the USP message was received
**
** \return  true if a 'reply-to' MTP is specified, false otherwise
**
**************************************************************************/
bool DEVICE_CONTROLLER_IsMTPConfigured(char *endpoint_id, mtp_protocol_t protocol)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Exit if unable to find the specified controller
    cont = FindEnabledControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        return false;
    }

    // Exit if unable to find a configured MTP for this controller
    mtp = FindFirstEnabledMtp(cont, protocol);
    if (mtp == NULL)
    {
#ifdef ENABLE_WEBSOCKETS
{
        // Exit if the controller is connected to the agent's websocket server
        int err;
        err = WSSERVER_GetMTPForEndpointId(endpoint_id, NULL);
        if (err == USP_ERR_OK)
        {
            return true;
        }
}
#endif

        // If the code gets here, then there was no MTP configured for this controller,
        // and the controller is not connected via the agent's websocket server
        return false;
    }

    // Check that all MTP parameters are configured for the specified protocol
    switch(protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            if ((mtp->stomp_connection_instance == INVALID) || (mtp->stomp_controller_queue[0] == '\0'))
            {
                return false;
            }
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            if ((mtp->coap_controller_host[0] == '\0') || (mtp->coap.resource[0] == '\0'))
            {
                return false;
            }
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            if ((mtp->mqtt_connection_instance == INVALID) || (mtp->mqtt_controller_topic[0] == '\0'))
            {
                return false;
            }
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            if ((cont->instance == INVALID) || (mtp->instance == INVALID))
            {
                return false;
            }
            break;
#endif

#ifdef ENABLE_UDS
        case kMtpProtocol_UDS:
            if ((cont->instance == INVALID) || (mtp->instance == INVALID))
            {
                return false;
            }
            break;
#endif

        default:
            TERMINATE_BAD_CASE(mtp->protocol);
            break;
    }

    // If the code gets here, then a valid MTP has been found for the specified controller
    return true;
}

#ifndef DISABLE_STOMP
/*********************************************************************//**
**
** DEVICE_CONTROLLER_QueueStompConnectRecord
**
** Queues a connect record for all Controllers attached to the specified STOMP connection
** The connect record is sent immediately after a successful connection to a STOMP broker to
** inform the attached Controllers of the presence of the agent, and the STOMP queue on which the agent can be contacted
**
** \param   stomp_instance - Instance number of the STOMP connection in Device.STOMP.Connection.{i}
** \param   agent_queue - STOMP destination which the agent has actually subscribed to
**                        NOTE: This may have been set by Device.LocalAgent.MTP.{i}.STOMP.Destination, or it may have been set by the subscribe-dest STOMP header in the CONNECTED frame
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_QueueStompConnectRecord(int stomp_instance, char *agent_queue)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller, sending a USP connect record on each one
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_STOMP) && (mtp->stomp_connection_instance == stomp_instance))
                {
                    if (mtp->stomp_controller_queue[0] != '\0')
                    {
                        mtp_send_item_t msi;
                        USPREC_StompConnect_Create(cont->endpoint_id, agent_queue, &msi);
                        DEVICE_STOMP_QueueBinaryMessage(&msi, stomp_instance, mtp->stomp_controller_queue, agent_queue, END_OF_TIME);
                        // NOTE: No need to free any members of the msi structure, since ownership of the payload buffer has passed to STOMP MTP layer
                    }
                    else
                    {
                        USP_LOG_Error("%s: Unable to send USP Connect record to %s because it has no STOMP destination queue", __FUNCTION__, cont->endpoint_id);
                    }
                }
            }
        }
    }

}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_NotifyStompConnDeleted
**
** Called when a STOMP connection is deleted
** This code unpicks all references to the STOMP connection existing in the Controller MTP table
**
** \param   stomp_instance - instance in Device.STOMP.Connection which has been deleted
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_NotifyStompConnDeleted(int stomp_instance)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller, clearing out all references to the deleted STOMP connection
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_STOMP) && (mtp->stomp_connection_instance == stomp_instance))
                {
                    USP_SNPRINTF(path, sizeof(path), "Device.LocalAgent.Controller.%d.MTP.%d.STOMP.Reference", cont->instance, mtp->instance);
                    DATA_MODEL_SetParameterValue(path, "", 0);
                }
            }
        }
    }
}
#endif

#ifdef ENABLE_MQTT
/*********************************************************************//**
**
** DEVICE_CONTROLLER_GetMqttControllerTopics
**
** Creates a list of controllers (and their associated topics) which are connected to the specified MQTT client
** The returned list is used later on to send a USP Connect record to each controller in the list
**
** \param   mqtt_instance - Instance number of the connection in Device.MQTT.Client.{i}
** \param   controller_topics - key value vector in which to return the list of controlelrs and their assocaiated topics
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_GetMqttControllerTopics(int mqtt_instance, kv_vector_t *controller_topics)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;

    KV_VECTOR_Init(controller_topics);

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_MQTT) && (mtp->enable) && (mtp->mqtt_connection_instance == mqtt_instance))
                {
                    USP_ASSERT(mtp->mqtt_controller_topic != NULL);
                    if (mtp->mqtt_controller_topic[0] != '\0')
                    {
                        KV_VECTOR_Add(controller_topics, cont->endpoint_id, mtp->mqtt_controller_topic);
                    }
                    else
                    {
                        USP_LOG_Error("%s: Unable to send USP Connect record to %s because it has no MQTT topic", __FUNCTION__, cont->endpoint_id);
                    }
                }
            }
        }
    }
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_SetRolesFromMqtt
**
** Sets the controller trust role to use for all controllers connected to the specified MQTT Client
**
** \param   mqtt_instance - MQTT instance (in Device.MQTT.Client table)
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_SetRolesFromMqtt(int mqtt_instance, int role_instance)
{
    int i, j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Iterate over all enabled controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            // Iterate over all enabled MTP slots for this controller
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable))
                {
                    // If this controller is connected to the specified MQTT connection, then set its inherited role
                    if ((mtp->protocol == kMtpProtocol_MQTT) && (mtp->mqtt_connection_instance == mqtt_instance))
                    {
                        cont->inherited_instance = role_instance;
                    }
                }
            }
        }
    }
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_NotifyMqttConnDeleted
**
** Called when a MQTT connection is deleted
** This code unpicks all references to the MQTT client existing in the Controller MTP table
**
** \param   mqtt_instance - instance in Device.MQTT.Client which has been deleted
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_NotifyMqttConnDeleted(int mqtt_instance)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller, clearing out all references to the deleted MQTT client
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_MQTT) && (mtp->mqtt_connection_instance == mqtt_instance))
                {
                    USP_SNPRINTF(path, sizeof(path), "Device.LocalAgent.Controller.%d.MTP.%d.MQTT.Reference", cont->instance, mtp->instance);
                    DATA_MODEL_SetParameterValue(path, "", 0);
                }
            }
        }
    }
}
#endif

/*********************************************************************//**
**
** DEVICE_CONTROLLER_SetInheritedRole
**
** Sets the controller trust role to use for the specified controller
**
** \param   cont_instance - Instance of controller in Device.LocalAgent.Controller.{i}
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_SetInheritedRole(int cont_instance, int role_instance)
{
    int i;
    controller_t *cont;

    // Iterate over all enabled controllers, setting the inherited role of the matching controller
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance == cont_instance) && (cont->instance != INVALID))
        {
            cont->inherited_instance = role_instance;
            break;
        }
    }
}

#ifdef ENABLE_WEBSOCKETS
/*********************************************************************//**
**
** DEVICE_CONTROLLER_CountEnabledWebsockClientConnections
**
** Determines the number of websocket client connections to controllers
**
** \param   None
**
** \return  Count of the number of enabled websocket client connections to controllers
**
**************************************************************************/
int DEVICE_CONTROLLER_CountEnabledWebsockClientConnections(void)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;
    int count = 0;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            // Iterate over all MTP slots for this controller
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable) && (mtp->protocol == kMtpProtocol_WebSockets))
                {
                    count++;
                }
            }
        }
    }

    return count;
}
#endif

#if defined(ENABLE_UDS) && !(defined(REMOVE_USP_BROKER) && defined(REMOVE_USP_SERVICE))
/*********************************************************************//**
**
** DEVICE_CONTROLLER_AddController_UDS
**
** Adds the specified endpoint as a controller into the Controller table of this device, connected to the specified UDS instance
** Called in a USP Broker, after a USP Service has connected to the Broker's agent Unix domain socket
** Also called in a USP Service, after connecting to a USP Broker on the Broker's controller Unix domain socket
**
** \param   endpoint_id - Endpoint_id of the USP Service connecting as a controller to this broker
** \param   uds_instance - instance number in Device.UnixDomainSockets.UnixDomainSocket.{i} which the USP Service is connected to
**                         (and which is the Broker's agent socket)
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_AddController_UDS(char *endpoint_id, int uds_instance)
{
    controller_t *cont = NULL;
    controller_mtp_t *mtp = NULL;
    int err;
    int cont_instance;
    int mtp_instance;
    dm_trans_vector_t trans;
    char path[MAX_DM_PATH];
    char value[MAX_DM_PATH];

    // Determine whether the external endpoint exists in the Controller table already
    cont = FindControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        goto update;
    }
    else
    {
        cont_instance = cont->instance;
    }

    // Determine whether the external endpoint has a UDS entry in the Controller MTP table already
    mtp = FindControllerMtpByProtocol(cont, kMtpProtocol_UDS);
    if (mtp == NULL)
    {
        goto update;
    }
    else
    {
        mtp_instance = mtp->instance;
    }

    // Exit if the external endpoint is already connected to the specified UDS instance (and enabled)
    if ((cont->enable==true) && (mtp->enable==true) && (mtp->uds_connection_instance == uds_instance))
    {
        return;
    }

update:
    // If the code gets here, then either the external endpoint needs adding to the controller table
    // or a UDS MTP needs adding, or the entries need enabling or the UDS reference needs updating

    //-------------------------------------------------------
    // ADD / UPDATE CONTROLLER
    if ((cont == NULL) || (cont->enable == false))
    {
        // Exit if unable to start a transaction for adding/updating the controller
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            return;
        }

        // Add a Controller, if not already present
        if (cont == NULL)
        {
            // Exit if failed to add a new instance to Device.LocalAgent.Controller.{i}
            err = DATA_MODEL_AddInstance((char *)device_cont_root, &cont_instance, CHECK_CREATABLE);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Exit if failed to update Device.LocalAgent.Controller.{i}.EndpointID
            USP_SNPRINTF(path, sizeof(path), "%s.%d.EndpointID", device_cont_root, cont_instance);
            err = DATA_MODEL_SetParameterValue(path, endpoint_id, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        // Exit if failed to update Device.LocalAgent.Controller.{i}.Enable to true
        USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_cont_root, cont_instance);
        err = DATA_MODEL_SetParameterValue(path, "true", 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Commit the transaction for adding/updating the controller
        // NOTE: Ths will chain to updating the controllers[] data structure, and allow the MTP to be added
        DM_TRANS_Commit();

        // Ensure that cont points to the controller whcih we jsut added
        cont = FindControllerByInstance(cont_instance);
        USP_ASSERT(cont != NULL);           // Because we just added it (when the commit chained to updating controllers[])
    }

    // Controllers that are connected over UDS have implicit full access
    cont->inherited_instance = ROLE_UDS;

    //-------------------------------------------------------
    // ADD / UPDATE MTP
    if ((mtp == NULL) || (mtp->enable != true) || (mtp->uds_connection_instance != uds_instance))
    {
        // Exit if unable to start a transaction for adding/updating the controller MTP
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            return;
        }

        // Add a Controller MTP, if one has not been added already...
        if (mtp == NULL)
        {
            // Exit if failed to add a new instance to Device.LocalAgent.Controller.{i}.MTP.{i}
            USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP", device_cont_root, cont_instance);
            err = DATA_MODEL_AddInstance(path, &mtp_instance, CHECK_CREATABLE);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Exit if failed to update Device.LocalAgent.Controller.{i}.MTP.{i}.Protocol to UDS
            USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Protocol", device_cont_root, cont_instance, mtp_instance);
            err = DATA_MODEL_SetParameterValue(path, "UDS", 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        // Exit if failed to update Device.LocalAgent.Controller.{i}.MTP.{i}.UDS.UnixDomainSocketRef to the specified instance number
        if ((mtp == NULL) || (mtp->uds_connection_instance != uds_instance))
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.UDS.UnixDomainSocketRef", device_cont_root, cont_instance, mtp_instance);
            USP_SNPRINTF(value, sizeof(value), "Device.UnixDomainSockets.UnixDomainSocket.%d", uds_instance);
            err = DATA_MODEL_SetParameterValue(path, value, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        // Exit if failed to update Device.LocalAgent.Controller.{i}.MTP.{i}.Enable to true
        if ((mtp == NULL) || (mtp->enable != true))
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Enable", device_cont_root, cont_instance, mtp_instance);
            err = DATA_MODEL_SetParameterValue(path, "true", 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        // Commit the transaction for adding/updating the controller MTP
        // NOTE: Ths will chain to updating the controllers[] data structure
        DM_TRANS_Commit();
    }

    // If the code gets here, everything was successful
    err = USP_ERR_OK;


exit:
    // Abort the transaction if not successful
    if (err != USP_ERR_OK)
    {
        DM_TRANS_Abort();
    }
}
#endif

#ifndef REMOVE_USP_SERVICE
/*********************************************************************//**
**
** DEVICE_CONTROLLER_FindFirstControllerEndpoint
**
** Returns the endpoint_id of the first controller found (lowest instance number) in Device.LocalAgent.Controller
**
** \param   None
**
** \return  endpoint_id of the first controller or NULL if no controller setup
**          NOTE: Ownership of the endpoint_id stays with this file
**
**************************************************************************/
char *DEVICE_CONTROLLER_FindFirstControllerEndpoint(void)
{
    int i;
    controller_t *cont;
    int instance = INT_MAX;
    char *endpoint_id = NULL;

    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            if (cont->instance < instance)
            {
                instance = cont->instance;
                endpoint_id = cont->endpoint_id;
            }
        }
    }

    return endpoint_id;
}
#endif

/*********************************************************************//**
**
** DEVICE_CONTROLLER_CanMtpConnect
**
** Called from an MTP thread to determine whether the MTP can start connecting
**
** \param   None
**
** \return  true if the MTP can start. flase if the MTP should not start connecting
**
**************************************************************************/
bool DEVICE_CONTROLLER_CanMtpConnect(void)
{
    bool allowed;       // Intermediate variable to contain saved copy of can_all_mtp_start, within the mutex protection

    OS_UTILS_LockMutex(&can_mtp_connect_mutex);
    allowed = can_mtp_connect;
    OS_UTILS_UnlockMutex(&can_mtp_connect_mutex);

    return allowed;
}

/*********************************************************************//**
**
** DEVICE_CONTROLLER_StartAllMtpClients
**
** Starts all MTP clients (in a state of either being allowed to connect, or held off in a retrying state)
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_CONTROLLER_StartAllMtpClients(void)
{
    bool allowed;
    time_t next_time;

    // Determine whether the MTPs can be allowed to connect
    // NOTE: They might not be allowed to connect because we're waiting for NTP time or some critical USP Service to register
    allowed = UpdateCanMtpConnect();

    // Start all MTP clients. They will either connect (if can_connect==true) or they will be held in a retrying state (if can_connect==false)
    StartAllMtpClients();

    // If allowed to connect, then we assume that NTP time has been obtained, so allow Periodic! timers to start
    // and MTP servers to start
    if (allowed)
    {
        UpdateNextPeriodicTime();
        DEVICE_MTP_StartMtpServers();
    }

    // If not allowed to connect, then start a sync timer to periodically determine when they can connect
    if (allowed == false)
    {
        #define CAN_MTP_CONNECT_POLL_ID 0
        #define CAN_MTP_CONNECT_POLL_INTERVAL 10
        next_time = time(NULL) + CAN_MTP_CONNECT_POLL_INTERVAL;
        SYNC_TIMER_Add(PollCanMtpConnect, CAN_MTP_CONNECT_POLL_ID, next_time);
    }
}

/*********************************************************************//**
**
** StartAllMtpClients
**
** Starts all STOMP, CoAP, MQTT and Websockets clients
** NOTE: Ultimately the clients may go straight to a retrying state (instead of connecting) if 'can_mtp_connect' is false
**
** \param   None
**
** \return  None
**
**************************************************************************/
void StartAllMtpClients(void)
{
#ifndef DISABLE_STOMP
    DEVICE_STOMP_StartAllConnections();
#endif

#ifdef ENABLE_COAP
    StartAllCoAPClients();
#endif

#ifdef ENABLE_MQTT
    DEVICE_MQTT_StartAllClients();
#endif

#ifdef ENABLE_WEBSOCKETS
    StartAllWebsocketClients();
#endif
}

#ifdef ENABLE_COAP
/*********************************************************************//**
**
** StartAllCoAPClients
**
** Called to start all enabled CoAP clients in the Controller MTP table
**
** \param   None
**
** \return  None
**
**************************************************************************/
void StartAllCoAPClients(void)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable) && (mtp->protocol == kMtpProtocol_CoAP))
                {
                    COAP_CLIENT_Start(cont->instance, mtp->instance, cont->endpoint_id);
                    // NOTE: Intentionally ignoring error, as there's nothing we can do, and we'd like to continue starting any other CoAP clients
                }
            }
        }
    }
}
#endif

#ifdef ENABLE_WEBSOCKETS
/*********************************************************************//**
**
** StartAllWebsocketClients
**
** Called to start all enabled Websocket clients in the Controller MTP table
**
** \param   None
**
** \return  None
**
**************************************************************************/
void StartAllWebsocketClients(void)
{
    int i;
    int j;
    controller_t *cont;
    controller_mtp_t *mtp;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Iterate over all MTP slots for this controller
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable) && (mtp->protocol == kMtpProtocol_WebSockets))
                {
                    WSCLIENT_StartClient(cont->instance, mtp->instance, cont->endpoint_id, &mtp->websock);
                    // NOTE: Intentionally ignoring error, as there's nothing we can do, and we'd like to continue starting any other CoAP clients
                }
            }
        }
    }
}
#endif

/*********************************************************************//**
**
** PollCanMtpConnect
**
** Periodically called at startup to determine whether the MTP clients (STOMP, CoAP, MQTT, Websockets)
** are allowed to start connecting to the USP Controllers. If so, then allow them to.
**
** \param   id - unused
**
** \return  None
**
**************************************************************************/
void PollCanMtpConnect(int id)
{
    bool allowed;
    time_t next_time;

    allowed = UpdateCanMtpConnect();

    // Exit if allowed to connect
    if (allowed)
    {
        // Release all MTPs waiting to connect in a retrying state
        AllowConnectOnAllMtpClients();

        // Start all MTP servers (Websockets and CoAP)
        DEVICE_MTP_StartMtpServers();

        // Allow Periodic! events to be sent
        UpdateNextPeriodicTime();

        // Remove the sync timer
        SYNC_TIMER_Remove(PollCanMtpConnect, CAN_MTP_CONNECT_POLL_ID);
        return;
    }

    // If still not allowed to connect, schedule this function to be called again in the future
    next_time = time(NULL) + CAN_MTP_CONNECT_POLL_INTERVAL;
    SYNC_TIMER_Reload(PollCanMtpConnect, CAN_MTP_CONNECT_POLL_ID, next_time);
}

/*********************************************************************//**
**
** AllowConnectOnAllMtpClients
**
** Called to release the MTP clients from their holding state of retrying, so that they can then connect
** NOTE: They may not have been allowed to connect because NTP time may not have been obtained, or because a critical USP Service had not been registered
**
** \param   None
**
** \return  None
**
**************************************************************************/
void AllowConnectOnAllMtpClients(void)
{
#ifndef DISABLE_STOMP
    STOMP_AllowConnect();
#endif

#ifdef ENABLE_COAP
    COAP_CLIENT_AllowConnect();
#endif

#ifdef ENABLE_MQTT
    MQTT_AllowConnect();
#endif

#ifdef ENABLE_WEBSOCKETS
    WSCLIENT_AllowConnect();
#endif
}

/*********************************************************************//**
**
** UpdateCanMtpConnect
**
** Updates the 'can_mtp_connect' variable, which determines whether the MTPs are allowed to connect or whether they must be held in a retrying state
**
** \param   None
**
** \return  true if the MTPs are allowed to connect, false otherwise
**
**************************************************************************/
bool UpdateCanMtpConnect(void)
{
    bool allowed;
    can_mtp_connect_cb_t  can_mtp_connect_cb;
    time_t time_now;

    // Determine whether the vendor hook allows the MTPs to start
    can_mtp_connect_cb = vendor_hook_callbacks.can_mtp_connect_cb;
    if (can_mtp_connect_cb != NULL)
    {
        allowed = can_mtp_connect_cb();
    }
    else
    {
        // Default (if no vendor hook is registered) is to immediately start all MTP clients connecting
        allowed = true;
    }

    // Handle the case of timed out waiting for permission to connect
    if (allowed == false)
    {
        time_now = OS_UTILS_TimeNow();
        if (time_now >= pre_connect_timeout)
        {
            USP_LOG_Warning("%s: WARNING: Timed out waiting for permission to connect. Connecting anyway. Some criteria may not be met.", __FUNCTION__);
            allowed = true;
        }
    }

    // Save off whether connection is allowed
    OS_UTILS_LockMutex(&can_mtp_connect_mutex);
    can_mtp_connect = allowed;
    OS_UTILS_UnlockMutex(&can_mtp_connect_mutex);

    return allowed;
}

/*********************************************************************//**
**
** UpdateNextPeriodicTime
**
** Updates the periodic time for all controllers, then updates the sync timer for the next Periodic! event to be sent
** This function is called after we believe that NTP time has been obtained
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UpdateNextPeriodicTime(void)
{
    int i;
    controller_t *cont;
    time_t cur_time;

    // Iterate over all controllers, recalculating the next time their Periodic! event should fire
    cur_time = time(NULL);
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable))
        {
            cont->next_time_to_fire = CalcNextPeriodicTime(cur_time, cont->periodic_base, cont->periodic_interval);
        }
    }

    // Update the time at which the next periodic notification should fire
    UpdateFirstPeriodicNotificationTime();
}

/*********************************************************************//**
**
** CopyNotifyDestForEndpoint
**
** Determines a destination MTP to send a USP Record to based on the endpoint to send to
** This function is usually used to determine the destination MTP for USP notifications
**
** \param   endpoint_id - endpoint to send the message to
** \param   protocol - MTP protocol to use when sending the message
** \param   usp_msg_type - type of the USP message to send
** \param   dest - pointer to mtp_conn structure in which to copy the destination info
**                 NOTE: Ownership of the info copied into dest stays with the source of the copy (shallow copy), so dest does not have to be freed (deep copy)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CopyNotifyDestForEndpoint(char *endpoint_id, mtp_protocol_t protocol, Usp__Header__MsgType usp_msg_type, mtp_conn_t *dest)
{
    int err;
    controller_t *cont;
    controller_mtp_t *mtp;

#ifndef REMOVE_USP_BROKER
{
    // Exit if the destination endpoint is a USP service, copying the destination MTP based on the message type
    // NOTE: If the destination endpoint is a USP Broker, and this agent is a USP Service, then the USP Broker is a Controller, so its destination MTP is not calculated here
    // Message type determines whether the message is sent from the Broker's controller or the Broker's Agent socket (if UDS)
    mtp_conn_t *src;
    src = USP_BROKER_GetNotifyDestForEndpoint(endpoint_id, usp_msg_type);
    if (src != NULL)
    {
        memcpy(dest, src, sizeof(mtp_conn_t));
        return USP_ERR_OK;
    }
}
#endif

    // Exit if unable to find the specified controller
    cont = FindEnabledControllerByEndpointId(endpoint_id);
    if (cont == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to find an enabled controller to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Find the configured MTP in the Controller.MTP table to send the USP Record to
    // NOTE: This may be NULL if no enabled MTP is configured for the controller
    mtp = FindFirstEnabledMtp(cont, protocol);

    err = CalcNotifyDest(endpoint_id, cont, mtp, dest);
    if (err != USP_ERR_OK)
    {
        return err;
    }

#ifdef ENABLE_WEBSOCKETS
    // Switch the MTP destination, if the endpoint is not currently connected via
    // the configured connection, but is connected via the agent's websocket server
    SwitchMtpDestIfNotConnected(endpoint_id, dest);
#endif
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CalcNotifyDest
**
** Calculates the MTP destination to send a notification to
** This would normally be the specified MTP, however if the MTP is not currently connected
** and the endpoint is reachable via the agent's websocket server, then the websocket server is used instead
**
** \param   endpoint_id - USP Controller endpoint to send the notification to
** \param   cont - pointer to controller instance to send the message to (only used by CoAP)
** \param   mtp - configured MTP to send the notification to, or NULL if no MTP configured for the endpoint
** \param   mtpc - pointer to structure to update with the calculated MTP destination.
**                NOTE: the mtpc structure should be updated, not initialised.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcNotifyDest(char *endpoint_id, controller_t *cont, controller_mtp_t *mtp, mtp_conn_t *mtpc)
{
    // If no MTP was configured to send notifications to this controller
    if (mtp == NULL)
    {
#ifdef ENABLE_WEBSOCKETS
        // Exit if the controller is connected to the agent's websocket server
        int err;
        err = WSSERVER_GetMTPForEndpointId(endpoint_id, mtpc);
        if (err == USP_ERR_OK)
        {
            return err;
        }
#endif

        // If the code gets here, no MTP was found to send this message on
        USP_ERR_SetMessage("%s: Unable to find a valid controller MTP to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    switch(mtp->protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            USP_ASSERT(mtp->stomp_controller_queue != NULL);
            if ((mtp->stomp_connection_instance == INVALID) || (mtp->stomp_controller_queue[0] == '\0'))
            {
                USP_ERR_SetMessage("%s: No Stomp connection or destination in controller MTP to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
                return USP_ERR_INTERNAL_ERROR;
            }

            mtpc->protocol = kMtpProtocol_STOMP;
            mtpc->stomp.instance = mtp->stomp_connection_instance;
            mtpc->stomp.dest = mtp->stomp_controller_queue;
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            if ((mtp->coap_controller_host[0] == '\0') || (mtp->coap.resource[0] == '\0'))
            {
                USP_ERR_SetMessage("%s: No CoAP host or resource in controller MTP to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
                return USP_ERR_INTERNAL_ERROR;
            }

            mtpc->protocol = kMtpProtocol_CoAP;
            mtpc->coap.host = mtp->coap_controller_host;
            mtpc->coap.port = mtp->coap.port;
            mtpc->coap.resource = mtp->coap.resource;
            mtpc->coap.encryption = mtp->coap.enable_encryption;
            mtpc->coap.reset_session_hint = false;
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            USP_ASSERT(mtp->mqtt_controller_topic != NULL);
            if ((mtp->mqtt_connection_instance == INVALID) || (mtp->mqtt_controller_topic[0] == '\0'))
            {
                USP_ERR_SetMessage("%s: No MQTT client or topic in controller MTP to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
                return USP_ERR_INTERNAL_ERROR;
            }

            mtpc->protocol = kMtpProtocol_MQTT;
            mtpc->mqtt.instance = mtp->mqtt_connection_instance;
            mtpc->mqtt.topic = mtp->mqtt_controller_topic;
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            mtpc->protocol = kMtpProtocol_WebSockets;
            mtpc->ws.client_cont_instance = cont->instance;
            mtpc->ws.client_mtp_instance = mtp->instance;
            mtpc->ws.serv_conn_id = INVALID;
            break;
#endif

#ifdef ENABLE_UDS
        case kMtpProtocol_UDS:
{
            int err;
            err = UDS_GetMTPForEndpointId(endpoint_id, mtpc);
            if (err != USP_ERR_OK)
            {
                return err;
            }
}
            break;
#endif

        default:
            TERMINATE_BAD_CASE(mtp->protocol);
            break;
    }

    return USP_ERR_OK;
}


#ifdef ENABLE_WEBSOCKETS
/*********************************************************************//**
**
** SwitchMtpDestIfNotConnected
**
** Switches the MTP destination, if the endpoint is not currently connected via
** the configured connection, but is connected via the agent's websocket server
**
** \param   endpoint_id - USP Controller endpoint to send the notification to
** \param   mtpc - pointer to structure in which to return the calculated MTP destination
**
** \return  None
**
**************************************************************************/
void SwitchMtpDestIfNotConnected(char *endpoint_id, mtp_conn_t *mtpc)
{
    int err;
    mtp_conn_t wsserv_dest;

    // Exit if controller is not connected to agent's websocket server
    err = WSSERVER_GetMTPForEndpointId(endpoint_id, &wsserv_dest);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Since controller is connected to agent's websocket server,
    // use the websocket server if the configured MTP is not actually connected
    switch(mtpc->protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
        {
            char *status;
            status = STOMP_GetConnectionStatus(mtpc->stomp.instance, NULL);
            if (strcmp(status, "Enabled") != 0)
            {
                memcpy(mtpc, &wsserv_dest, sizeof(mtp_conn_t));
            }
        }
        break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            // Since CoAP uses UDP, we don't know if the controller is connected or not, so just send on the configured MTP (CoAP)
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
        {
            char *status;
            status = (char *) MQTT_GetClientStatus(mtpc->mqtt.instance);
            if (strcmp(status, "Connected") != 0)
            {
                memcpy(mtpc, &wsserv_dest, sizeof(mtp_conn_t));
            }
        }
        break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            if (mtpc->ws.serv_conn_id == INVALID)
            {
                // If configured to connect via the websocket client, then if it isn't connected, then send via agent's websocket server
                if (WSCLIENT_IsEndpointConnected(endpoint_id) == false)
                {
                   memcpy(mtpc, &wsserv_dest, sizeof(mtp_conn_t));
                }
            }
            else
            {
                // USP Record is already routed to be sent via agent's websocket server. Nothing to do.
            }
            break;
#endif

        default:
            TERMINATE_BAD_CASE(mtpc->protocol);
            break;
    }
}
#endif

/*********************************************************************//**
**
** QueueBinaryMessageOnMtp
**
** Queues a binary message on the specified MTP
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is passed to this function, unless an error is returned.
** \param   endpoint_id - controller to send the message to
** \param   usp_msg_id - pointer to string containing the msg_id of the serialized USP Message
** \param   mtpc - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP message should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int QueueBinaryMessageOnMtp(mtp_send_item_t *msi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtpc, time_t expiry_time)
{
    int err = USP_ERR_OK;
    USP_ASSERT(msi != NULL);

    switch(mtpc->protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
        {
            char *agent_queue = DEVICE_MTP_GetAgentStompQueue(mtpc->stomp.instance);

            err = DEVICE_STOMP_QueueBinaryMessage(msi, mtpc->stomp.instance, mtpc->stomp.dest, agent_queue, expiry_time);
        }
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
        {
            controller_t *cont;
            controller_mtp_t *mtp;
            cont = FindEnabledControllerByEndpointId(endpoint_id);
            mtp = FindFirstEnabledMtp(cont, mtpc->protocol);  // NOTE: This may be NULL if no enabled MTP is configured for the controller
            if ((cont == NULL) || (mtp==NULL))
            {
                USP_ERR_SetMessage("%s: Unable to find an enabled MTP to send to endpoint_id=%s", __FUNCTION__, endpoint_id);
                return USP_ERR_INTERNAL_ERROR;
            }

            err = COAP_CLIENT_QueueBinaryMessage(msi, cont->instance, mtp->instance, mtpc, expiry_time);
        }
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
        {
            err = MQTT_QueueBinaryMessage(msi, mtpc->mqtt.instance, mtpc->mqtt.topic, expiry_time);
        }
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            if (mtpc->ws.serv_conn_id == INVALID)
            {
                WSCLIENT_QueueBinaryMessage(msi, mtpc->ws.client_cont_instance, mtpc->ws.client_mtp_instance, expiry_time);
            }
            else
            {
                WSSERVER_QueueBinaryMessage(msi, mtpc->ws.serv_conn_id, expiry_time);
            }
            err = USP_ERR_OK;
            break;
#endif

#ifdef ENABLE_UDS
        case kMtpProtocol_UDS:
            {
               err = UDS_QueueBinaryMessage(msi, mtpc, expiry_time, kUdsFrameType_UspRecord);
            }
            break;
#endif

        default:
            TERMINATE_BAD_CASE(mtpc->protocol);
            break;
    }

    return err;
}

/*********************************************************************//**
**
** SendOnBoardRequest
**
** Called when sync command Device.LocalAgent.Controller.{i}.SendOnBoardRequest() is executed
**
** \param   req - pointer to structure identifying the command
** \param   command_key - not used, OnBoardRequest notification doesn't have a command key field
** \param   input_args - not used, the command doesn't receive parameters
** \param   output_args - not used, the command doesn't return values
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendOnBoardRequest(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    int err = USP_ERR_OK;

    controller_t* controller = FindControllerByInstance(inst1);
    if (controller == NULL)
    {
        USP_ERR_SetMessage("%s: Controller instance %d not found", __FUNCTION__, inst1);
        err = USP_ERR_INVALID_ARGUMENTS;
        goto exit;
    }

    // Execute operation
    err = ExecuteSendOnBoardRequest(controller);
    if (err != USP_ERR_OK)
    {
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

exit:
    // Log output results
    USP_LOG_Info("=== SendOnBoardRequest Operation completed with result=%d ===", err);

    return err;
}

/*********************************************************************//**
**
** ExecuteSendOnBoardRequest
**
** Creates OnBoardRequest Notify request
**
** \param   controller - point to the controller responsible for sending the OnBoardRequest notification
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ExecuteSendOnBoardRequest(controller_t* controller)
{
    Usp__Msg *req;

    // Only send the OnBoardRequest if the controller is enabled, according to R-NOT.5
    if (controller->enable)
    {
        char oui[MAX_DM_SHORT_VALUE_LEN];
        char product_class[MAX_DM_SHORT_VALUE_LEN];
        char serial_number[MAX_DM_SHORT_VALUE_LEN];

        int err = USP_ERR_OK;
        err = DATA_MODEL_GetParameterValue("Device.DeviceInfo.ManufacturerOUI", oui, sizeof(oui), 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        err = DATA_MODEL_GetParameterValue("Device.DeviceInfo.ProductClass", product_class, sizeof(product_class), 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        err = DATA_MODEL_GetParameterValue("Device.DeviceInfo.SerialNumber", serial_number, sizeof(serial_number), 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Create the notify message
        req = MSG_HANDLER_CreateNotifyReq_OnBoard(oui, product_class, serial_number, true);

        // Send the Notify Request
        SendOnBoardRequestNotify(req, controller);
        usp__msg__free_unpacked(req, pbuf_allocator);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SendOnBoardRequestNotify
**
** Sends OnBoardRequest Notify
**
** \param   req - USP OnBoardRequest notify message
** \param   controller - point to the controller responsible for sending the OnBoardRequest notification
**
** \return  None
**
**************************************************************************/
void SendOnBoardRequestNotify(Usp__Msg *req, controller_t* controller)
{
    unsigned char *pbuf;
    int pbuf_len;
    int size;
    time_t retry_expiry_time;
    char *dest_endpoint;
    mtp_conn_t mtp_conn = {0};  // Ensures mtp_conn.is_reply_to_specified=false
    usp_send_item_t usp_send_item;
    char *msg_id;

    // Exit if unable to determine the endpoint of the controller
    // This could occur if the controller had been deleted
    dest_endpoint = DEVICE_CONTROLLER_FindEndpointIdByInstance(controller->instance);
    if (dest_endpoint == NULL)
    {
        USP_LOG_Error("%s: SendOnBoardRequest dest_endpoint is NULL", __FUNCTION__);
        return;
    }
    USP_LOG_Debug("SendOnBoardRequest dest_endpoint=%s", dest_endpoint);

    // Serialize the protobuf structure into a binary format buffer
    pbuf_len = usp__msg__get_packed_size(req);
    pbuf = USP_MALLOC(pbuf_len);
    size = usp__msg__pack(req, pbuf);
    USP_ASSERT(size == pbuf_len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Determine the time at which we should give up retrying, or expire the message in the MTP's send queue
    retry_expiry_time = END_OF_TIME;       // default to never expire

    // Marshal parameters to pass to MSG_HANDLER_QueueUspRecord()
    MSG_HANDLER_UspSendItem_Init(&usp_send_item);
    usp_send_item.usp_msg_type = USP__HEADER__MSG_TYPE__NOTIFY;
    usp_send_item.msg_packed = pbuf;
    usp_send_item.msg_packed_size = pbuf_len;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    usp_send_item.curr_e2e_session = &controller->e2e_session;
    usp_send_item.usp_msg = req;
#endif

    // Send the message
    // NOTE: Intentionally ignoring error here.
    msg_id = req->header->msg_id;
    MSG_HANDLER_QueueUspRecord(&usp_send_item, dest_endpoint, msg_id, &mtp_conn, retry_expiry_time);

    // Ensure the message is retried until a NotifyResponse is received
    // NOTE: Ownership of the serialized USP message passes to the subs retry module
    SUBS_RETRY_Add(ON_BOARD_REQUEST_SUBS_INSTANCE, msg_id, "", dest_endpoint, "", pbuf, pbuf_len, END_OF_TIME);
}

/*********************************************************************//**
**
** PeriodicNotificationExec
**
** Sends out periodic notifications (that have fired) for all controllers
** This function is called back from a timer when it is time for a periodic notification to fire
**
** \param   id - (unused) identifier of the sync timer which caused this callback
**
** \return  None
**
**************************************************************************/
void PeriodicNotificationExec(int id)
{
    int i;
    controller_t *cont;
    time_t cur_time;

    cur_time = time(NULL);

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Skip this entry if it is unused
        cont = &controllers[i];
        if (cont->instance == INVALID)
        {
            continue;
        }

        // Send this notification, if it's time to fire
        if (cur_time >= cont->next_time_to_fire)
        {
            // Send a notification event to the controller (if there are any periodic events subscribed to)
            DEVICE_SUBSCRIPTION_SendPeriodicEvent(cont->instance); // Intentionally ignoring any errors

            // Update the time at which this notification next fires
            cont->next_time_to_fire = CalcNextPeriodicTime(cur_time, cont->periodic_base, cont->periodic_interval);
        }
    }

    // Update the time at which the next periodic notification should fire
    UpdateFirstPeriodicNotificationTime();
}

/*********************************************************************//**
**
** ValidateAdd_Controller
**
** Function called to determine whether a controller may be added
**
** \param   req - pointer to structure identifying the request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_Controller(dm_req_t *req)
{
    controller_t *cont;

    // Exit if unable to find a free controller slot
    cont = FindUnusedController();
    if (cont == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateAdd_ControllerMtp
**
** Function called to determine whether an MTP may be added to a controller
**
** \param   req - pointer to structure identifying the controller MTP
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_ControllerMtp(dm_req_t *req)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Exit if unable to find the parent controller
    // NOTE: This occurs in the case of a USP Add message containing a nested add (of Controller and MTP)
    cont = FindControllerByInstance(inst1);
    if (cont == NULL)
    {
        USP_ERR_SetMessage("%s: %s.%d does not exist. Adding both Controller and MTP using a single USP message are not supported", __FUNCTION__, device_cont_root, inst1);
        return USP_ERR_CREATION_FAILURE;
    }

    // Exit if unable to find a free MTP slot
    mtp = FindUnusedControllerMtp(cont);
    if (mtp == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerAdded
**
** Function called when a controller has been added to Device.LocalAgent.Controller.{i}
**
** \param   req - pointer to structure identifying the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerAdded(dm_req_t *req)
{
    int err;

    err = ProcessControllerAdded(inst1);

    return err;
}

/*********************************************************************//**
**
** Notify_ControllerDeleted
**
** Function called when a controller has been deleted from Device.LocalAgent.Controller.{i}
**
** \param   req - pointer to structure identifying the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerDeleted(dm_req_t *req)
{
    controller_t *cont;

    // Exit if we cannot find the controller
    // NOTE: We might not find it if it was never added. This could occur if deleting from the DB at startup when we detected that the database params were invalid
    cont = FindControllerByInstance(inst1);
    if (cont == NULL)
    {
        return USP_ERR_OK;
    }

    // Delete the controller from the array
    DestroyController(cont);

    // Delete all subscriptions owned by this controller
    DEVICE_SUBSCRIPTION_NotifyControllerDeleted(inst1);

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpAdded
**
** Function called when an MTP has been added to Device.LocalAgent.Controller.{i}.MTP.{i}
**
** \param   req - pointer to structure identifying the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpAdded(dm_req_t *req)
{
    int err;
    controller_t *cont;

    // Exit if the specified controller is not in the controller array - this could occur on startup if the controller entry in the DB was incorrect
    cont = FindControllerByInstance(inst1);
    if (cont == NULL)
    {
        USP_ERR_SetMessage("%s: Controller instance %d does not exist in internal data structure", __FUNCTION__, inst1);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if an error occurred in processing the MTP
    err = ProcessControllerMtpAdded(cont, inst2);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpDeleted
**
** Function called when an MTP has been deleted from Device.LocalAgent.Controller.{i}.MTP.{i}
**
** \param   req - pointer to structure identifying the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpDeleted(dm_req_t *req)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Exit if we cannot find the controller MTP
    // NOTE: We might not find it, if it was never added. This could occur if deleting from the DB
    // at startup, if we detected that the database params were invalid
    mtp = FindControllerMtpFromReq(req, &cont);
    if (mtp == NULL)
    {
        return USP_ERR_OK;
    }

#ifdef ENABLE_COAP
    // Stop this MTP, if it is CoAP
    // (We don't need to do anything if this MTP is STOMP, because all we are deleting is an
    //  address to send to, the STOMP connection itself is separate)
    if ((mtp->protocol == kMtpProtocol_CoAP) && (mtp->enable) && (cont->enable))
    {
        COAP_CLIENT_Stop(cont->instance, mtp->instance);
    }
#endif

#ifdef ENABLE_WEBSOCKETS
    // Stop this MTP, if it is WebSockets
    if ((mtp->protocol == kMtpProtocol_WebSockets) && (mtp->enable) && (cont->enable))
    {
        WSCLIENT_StopClient(cont->instance, mtp->instance);
    }
#endif

    // Delete the controller MTP from the array
    DestroyControllerMtp(mtp);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_ControllerEndpointID
**
** Validates that the EndpointID is unique across all registered controllers
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerEndpointID(dm_req_t *req, char *value)
{
    int err;

    // Exit if endpoint_id is not unique
    err = ValidateEndpointIdUniqueness(value, inst1);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_ControllerMtpEnable
**
** Validates Device.LocalAgent.Controller.{i}.MTP.{i}.Enable
** by checking that it is a boolean
** and also checking that setting it to true would not enable more than one STOMP MTP
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerMtpEnable(dm_req_t *req, char *value)
{
    int err;
    mtp_protocol_t protocol;
    char path[MAX_DM_PATH];

    // Exit if we are disabling this controller MTP. In this case we do not have to perform the uniqueness aand resource available checks
    if (val_bool == false)
    {
        return USP_ERR_OK;
    }

    // Exit if unable to get the protocol configured for this MTP
    // NOTE: We look the value up in the database because this function may be called before the controller MTP has actually been added to the internal data structure
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Protocol", device_cont_root, inst1, inst2);
    err = DM_ACCESS_GetEnum(path, &protocol, mtp_protocols, NUM_ELEM(mtp_protocols));
    if (err != USP_ERR_OK)
    {
        // NOTE: Ignoring any error because the setting of enable may be done before protocol, when performing an AddInstance
        return USP_ERR_OK;
    }

    // Exit if trying to enable more than one MTP with the same protocol for this controller
    err = ValidateMtpUniqueness(protocol, inst1, inst2);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if trying to enable more MTPs than the lower levels support for this protocol
    err = ValidateMtpResourceAvailable(protocol, inst1, inst2);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return err;
}

/*********************************************************************//**
**
** Validate_ControllerMtpProtocol
**
** Validates Device.LocalAgent.Controller.{i}.MTP.{i}.Protocol
** by checking that it matches the protocols we support
** and also checking that there is not another enabled MTP for this controller with the same protocol assigned
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerMtpProtocol(dm_req_t *req, char *value)
{
    int err;
    int index;
    mtp_protocol_t protocol;
    bool enable;
    char path[MAX_DM_PATH];

    // Exit if the protocol was invalid
    index = TEXT_UTILS_StringToEnum(value, mtp_protocols, NUM_ELEM(mtp_protocols));
    if (index == INVALID)
    {
        USP_ERR_SetMessage("%s: Invalid or unsupported protocol %s", __FUNCTION__, value);
        return USP_ERR_INVALID_VALUE;
    }
    protocol = (mtp_protocol_t) index;

    // Exit if this controller MTP is not enabled
    // NOTE: We look the value up in the database because this function may be called before the controller MTP has actually been added to the internal data structure
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Enable", device_cont_root, inst1, inst2);
    err = DM_ACCESS_GetBool(path, &enable);
    if ((err != USP_ERR_OK) || (enable == false))
    {
        // NOTE: Ignoring any error because the setting of protocol may be done before enable, when performing an AddInstance
        return USP_ERR_OK;
    }

    // Exit if trying to enable more than one MTP with the same protocol for this controller
    err = ValidateMtpUniqueness(protocol, inst1, inst2);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if trying to enable more MTPs than the lower levels support for this protocol
    err = ValidateMtpResourceAvailable(protocol, inst1, inst2);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_ControllerRetryMinimumWaitInterval
**
** Validates Device.LocalAgent.Controller.{i}.USPNotifRetryMinimumWaitInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerRetryMinimumWaitInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_ControllerRetryIntervalMultiplier
**
** Validates Device.LocalAgent.Controller.{i}.USPNotifRetryIntervalMultiplier
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1000, 65535);
}

/*********************************************************************//**
**
** Validate_SessionRetryInterval
**
** Validates Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.SessionRetryMinimumWaitInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_SessionRetryInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_SessionRetryMultiplier
**
** Validates Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.SessionRetryIntervalMultiplier
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_SessionRetryMultiplier(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1000, 65535);
}

/*********************************************************************//**
**
** Validate_ControllerAssignedRole
**
** Validates Device.LocalAgent.Controller.{i}.AssignedRole
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerAssignedRole(dm_req_t *req, char *value)
{
    int err;
    int instance;

    // Empty String is an allowed value for Assigned Role
    if (*value == '\0')
    {
        return USP_ERR_OK;
    }

    err = DM_ACCESS_ValidateReference(value, "Device.LocalAgent.ControllerTrust.Role.{i}", &instance);

    return err;
}

/*********************************************************************//**
**
** Validate_ControllerMtpWebsockKeepAlive
**
** Validates Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.KeepAliveInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_ControllerMtpWebsockKeepAlive(dm_req_t *req, char *value)
{
    // NOTE: Disallow 0 for keep alive period (0 is NOT a special case for off)
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, UINT_MAX);
}


/*********************************************************************//**
**
** Validate_PeriodicNotifInterval
**
** Validates Device.LocalAgent.Controller.{i}.PeriodicNotifInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_PeriodicNotifInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, UINT_MAX);
}

/*********************************************************************//**
**
** Notify_ControllerEnable
**
** Function called when Device.LocalAgent.Controller.{i}.Enable is modified
** This function updates the value of the enable stored in the controllers array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerEnable(dm_req_t *req, char *value)
{
    controller_t *cont;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Exit if the value has not changed
    if (val_bool == cont->enable)
    {
        return USP_ERR_OK;
    }

    // Save the new value
    cont->enable = val_bool;

#ifdef ENABLE_COAP
{
    // Iterate over all MTPs for this controller, starting or stopping its associated CoAP MTPs
    int i;
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        int err;
        controller_mtp_t *mtp;

        mtp = &cont->mtps[i];
        if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_CoAP))
        {
            if ((mtp->enable) && (cont->enable))
            {
                // Exit if unable to start client
                err = COAP_CLIENT_Start(cont->instance, mtp->instance, cont->endpoint_id);
                if (err != USP_ERR_OK)
                {
                    return err;
                }
            }
            else
            {
                COAP_CLIENT_Stop(cont->instance, mtp->instance);
            }
        }
    }
}
#endif

#ifdef ENABLE_WEBSOCKETS
{
    // Iterate over all MTPs for this controller, starting or stopping its associated Websocket MTPs
    int i;
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        controller_mtp_t *mtp;

        mtp = &cont->mtps[i];
        if ((mtp->instance != INVALID) && (mtp->protocol == kMtpProtocol_WebSockets))
        {
            if ((mtp->enable) && (cont->enable))
            {
                WSCLIENT_StartClient(cont->instance, mtp->instance, cont->endpoint_id, &mtp->websock);
            }
            else
            {
                WSCLIENT_StopClient(cont->instance, mtp->instance);
            }
        }
    }
}
#endif

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerEndpointID
**
** Function called when Device.LocalAgent.Controller.{i}.EndpointID is modified
** This function updates the value of the endpoint_id stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerEndpointID(dm_req_t *req, char *value)
{
    controller_t *cont;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Set the new value
    USP_SAFE_FREE(cont->endpoint_id);
    cont->endpoint_id = USP_STRDUP(value);

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerAssignedRole
**
** Function called when Device.LocalAgent.Controller.{i}.AssignedRole is modified
** This function updates the value of the assigned_role stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerAssignedRole(dm_req_t *req, char *value)
{
    int err;
    controller_t *cont;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    err = UpdateAssignedRole(cont, value);

    return err;
}

/*********************************************************************//**
**
** Notify_ControllerMtpEnable
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.Enable is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpEnable(dm_req_t *req, char *value)
{
    controller_t *cont = NULL;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value has not changed
    if (val_bool == mtp->enable)
    {
        return USP_ERR_OK;
    }

    // Save the new value
    mtp->enable = val_bool;

#ifdef ENABLE_COAP
{
    // Start or stop CoAP client based on new value
    int err;
    if (mtp->protocol == kMtpProtocol_CoAP)
    {
        if ((mtp->enable) && (cont->enable))
        {
            // Exit if unable to start client
            err = COAP_CLIENT_Start(cont->instance, mtp->instance, cont->endpoint_id);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
        else
        {
            COAP_CLIENT_Stop(cont->instance, mtp->instance);
        }
    }
}
#endif

#ifdef ENABLE_WEBSOCKETS
{
    // Start or stop WebSockets client based on new value
    if (mtp->protocol == kMtpProtocol_WebSockets)
    {
        if ((mtp->enable) && (cont->enable))
        {
            WSCLIENT_StartClient(cont->instance, mtp->instance, cont->endpoint_id, &mtp->websock);
        }
        else
        {
            WSCLIENT_StopClient(cont->instance, mtp->instance);
        }
    }
}
#endif

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

    // NOTE: We do not have to do anything for STOMP, as these parameters are only searched when we send

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpProtocol
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.Protocol is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpProtocol(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;
    mtp_protocol_t new_protocol;
    int index;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

#if defined(ENABLE_COAP) || defined(ENABLE_WEBSOCKETS)
    mtp_protocol_t old_protocol;
    old_protocol = mtp->protocol;
#endif

    // Extract the new value
    index = TEXT_UTILS_StringToEnum(value, mtp_protocols, NUM_ELEM(mtp_protocols));
    USP_ASSERT(index != INVALID); // Value must already have validated to have got here
    new_protocol = (mtp_protocol_t) index;

    // Exit if protocol has not changed
    if (new_protocol == mtp->protocol)
    {
        return USP_ERR_OK;
    }

    // Store new protocol
    mtp->protocol = new_protocol;

    // Exit if the MTP is not enabled - nothing more to do
    if ((mtp->enable == false) || (cont->enable == false))
    {
        return USP_ERR_OK;
    }

#ifdef ENABLE_COAP
{
    int err;

    // Stop the old CoAP server, if we've moved from CoAP
    if (old_protocol == kMtpProtocol_CoAP)
    {
        COAP_CLIENT_Stop(cont->instance, mtp->instance);
    }

    // Start the new CoAP server, if we've moved to CoAP, exiting if an error occurred
    if (new_protocol == kMtpProtocol_CoAP)
    {
        err = COAP_CLIENT_Start(cont->instance, mtp->instance, cont->endpoint_id);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }
}
#endif

#ifdef ENABLE_WEBSOCKETS
{
    // Stop the old connection, if we've moved from WebSockets to a different protocol
    if (old_protocol == kMtpProtocol_WebSockets)
    {
        WSCLIENT_StopClient(cont->instance, mtp->instance);
    }

    // Start a new connection, if we've moved to WebSockets from a different protocol
    if (new_protocol == kMtpProtocol_WebSockets)
    {
        WSCLIENT_StartClient(cont->instance, mtp->instance, cont->endpoint_id, &mtp->websock);
    }
}
#endif

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif
    // NOTE: We don't need to do anything explicitly for STOMP

    return USP_ERR_OK;
}

#ifndef DISABLE_STOMP
/*********************************************************************//**
**
** Notify_ControllerMtpStompReference
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.STOMP.Reference is modified
** This function updates the value of the stomp_connection_instance stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpStompReference(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];
    int err;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.STOMP.Reference", device_cont_root, cont->instance, mtp->instance);

    err = DEVICE_MTP_GetStompReference(path, &mtp->stomp_connection_instance);

    return err;
}

/*********************************************************************//**
**
** Notify_ControllerMtpStompDestination
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.STOMP.Destination is modified
** This function updates the value of the stomp_controller_queue stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpStompDestination(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SAFE_FREE(mtp->stomp_controller_queue);
    mtp->stomp_controller_queue = USP_STRDUP(value);

    return USP_ERR_OK;
}
#endif

#ifdef ENABLE_COAP
/*********************************************************************//**
**
** Notify_ControllerMtpCoapHost
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.CoAP.Host is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpCoapHost(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SAFE_FREE(mtp->coap_controller_host);
    mtp->coap_controller_host = USP_STRDUP(value);

    // NOTE: We do not need to explicitly propagate this value to the COAP module here,
    // as each USP message that is queued includes this information
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpCoapPort
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.CoAP.Port is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpCoapPort(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    mtp->coap.port = val_uint;

    // NOTE: We do not need to explicitly propagate this value to the COAP module here,
    // as each USP message that is queued includes this information

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpCoapPath
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.CoAP.Path is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpCoapPath(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SAFE_FREE(mtp->coap.resource);
    mtp->coap.resource = USP_STRDUP(value);

    // NOTE: We do not need to explicitly propagate this value to the COAP module here,
    // as each USP message that is queued includes this information

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpCoapEncryption
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.CoAP.EnableEncryption is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpCoapEncryption(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SAFE_FREE(mtp->coap.resource);
    mtp->coap.enable_encryption = val_bool;

    // NOTE: We do not need to explicitly propagate this value to the COAP module here,
    // as each USP message that is queued includes this information

    return USP_ERR_OK;
}
#endif

#ifdef ENABLE_MQTT
/*********************************************************************//**
**
** Notify_ControllerMtpMqttReference
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.MQTT.Reference is modified
** This function updates the value of the mqtt_client_instance stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpMqttReference(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];
    bool schedule_reconnect = false;
    int instance, err;

    // Exit if reference is a blank string
    if (*value == '\0')
    {
        return USP_ERR_OK;
    }

    // Exif if the controller trust role instance number does not exist
    err = DM_ACCESS_ValidateReference(value, "Device.MQTT.Client.{i}", &instance);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: instance (%d) is not found", __FUNCTION__, instance);
        return err;
    }

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Set the new value
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.MQTT.Reference", device_cont_root, cont->instance, mtp->instance);

    err = DEVICE_MTP_GetMqttReference(path, &mtp->mqtt_connection_instance);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s controller instance is invalid\n", __FUNCTION__);
        return err;
    }

    if ((mtp->enable == true) && (mtp->protocol == kMtpProtocol_MQTT) &&
        (mtp->mqtt_connection_instance != instance))
    {
        if (instance != INVALID)
        {
            schedule_reconnect = true;
        }
    }

    DEVICE_MQTT_UpdateControllerTopics();

    // Set the new value
    mtp->mqtt_connection_instance = instance;

    if (schedule_reconnect)
    {
        DEVICE_MQTT_ScheduleReconnect(mtp->mqtt_connection_instance);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpMqttTopic
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.MQTT.Topic is modified
** This function updates the value of the mqtt_controller_topic stored in the controller array
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpMqttTopic(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if topic has not changed
    if (strcmp(mtp->mqtt_controller_topic, value)==0)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    USP_SAFE_FREE(mtp->mqtt_controller_topic);
    mtp->mqtt_controller_topic = USP_STRDUP(value);

    DEVICE_MQTT_UpdateControllerTopics();

    return USP_ERR_OK;
}
#endif

#ifdef ENABLE_WEBSOCKETS
/*********************************************************************//**
**
** Notify_ControllerMtpWebsockHost
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.Host is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockHost(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (strcmp(mtp->websock.host, value)==0)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    USP_SAFE_FREE(mtp->websock.host);
    mtp->websock.host = USP_STRDUP(value);

    // Schedule a reconnect, if the connection is currently enabled
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockPort
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.Port is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockPort(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (mtp->websock.port == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->websock.port = val_uint;

    // Schedule a reconnect, if the connection is currently enabled
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockPath
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.Path is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockPath(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (strcmp(mtp->websock.path, value)==0)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    USP_SAFE_FREE(mtp->websock.path);
    mtp->websock.path = USP_STRDUP(value);

    // Schedule a reconnect, if the connection is currently enabled
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockEncryption
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.EnableEncryption is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockEncryption(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (mtp->websock.enable_encryption == val_bool)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->websock.enable_encryption = val_bool;

    // Schedule a reconnect, if the connection is currently enabled
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockKeepAlive
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.KeepAliveInterval is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockKeepAlive(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (mtp->websock.keep_alive_interval == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->websock.keep_alive_interval = val_uint;

    // Inform the MTP of the change (NOTE: This will not result in a reconnect)
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockRetryInterval
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.SessionRetryMinimumWaitInterval is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockRetryInterval(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (mtp->websock.retry_interval == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->websock.retry_interval = val_uint;

    // Inform the MTP of the change (NOTE: This will not result in a reconnect)
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerMtpWebsockRetryMultiplier
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.SessionRetryIntervalMultiplier is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpWebsockRetryMultiplier(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Exit if the value hasn't changed
    if (mtp->websock.retry_multiplier == val_uint)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->websock.retry_multiplier = val_uint;

    // Inform the MTP of the change (NOTE: This will not result in a reconnect)
    if (mtp->enable)
    {
        WSCLIENT_StartClient(inst1, inst2, cont->endpoint_id, &mtp->websock);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_WebsockRetryCount
**
** Gets the value of Device.LocalAgent.Controller.{i}.MTP.{i}.WebSocket.CurrentRetryCount
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_WebsockRetryCount(dm_req_t *req, char *buf, int len)
{
    val_uint = WSCLIENT_GetRetryCount(inst1, inst2);

    return USP_ERR_OK;
}
#endif

#ifdef ENABLE_UDS
/*********************************************************************//**
**
** Notify_ControllerMtpUdsReference
**
** Function called when Device.LocalAgent.Controller.{i}.MTP.{i}.UDS.UnixDomainSocketRef is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerMtpUdsReference(dm_req_t *req, char *value)
{
    controller_t *cont;
    controller_mtp_t *mtp;
    int uds_instance;
    int err;

    // Determine MTP to be updated
    mtp = FindControllerMtpFromReq(req, &cont);
    USP_ASSERT(mtp != NULL);

    // Determine the instance number in the UDS table that the reference points to
    err = DEVICE_MTP_GetUdsReference(req->path, &uds_instance);
    USP_ASSERT(err == USP_ERR_OK)

    // Exit if the value hasn't changed
    if (mtp->uds_connection_instance == uds_instance)
    {
        return USP_ERR_OK;
    }

    // Set the new value
    mtp->uds_connection_instance = uds_instance;

    // NOTE: No need to start a UDS Client/Server based on this reference - they are started based on being present in the UnixDomainSockets table

    return USP_ERR_OK;
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** Get_ControllerMtpUspServiceRef
**
** Gets the value of Device.LocalAgent.Controller.{i}.MTP.{i}.UDS.USPServiceRef
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_ControllerMtpUspServiceRef(dm_req_t *req, char *buf, int len)
{
    controller_t *cont;
    int instance;

    // Determine Controller
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Exit if unable to find the connected USP Service matching this Controller
    instance = USP_BROKER_GetUspServiceInstance(cont->endpoint_id, ONLY_CONTROLLER_CONNECTIONS);
    if (instance == INVALID)
    {
        *buf = '\0';
        return USP_ERR_OK;
    }

    USP_SNPRINTF(buf, len, "Device.USPServices.USPService.%d", instance);

    return USP_ERR_OK;
}
#endif
#endif

/*********************************************************************//**
**
** Notify_PeriodicNotifInterval
**
** Function called when Device.LocalAgent.Controller.{i}.PeriodicNotifInterval
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_PeriodicNotifInterval(dm_req_t *req, char *value)
{
    controller_t *cont;
    time_t cur_time;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Set the new value
    cont->periodic_interval = val_uint;

    // Calculate the new next time that this notification should fire
    cur_time = time(NULL);
    cont->next_time_to_fire = CalcNextPeriodicTime(cur_time, cont->periodic_base, cont->periodic_interval);

    // Update the time at which the first periodic notification fires
    UpdateFirstPeriodicNotificationTime();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_PeriodicNotifTime
**
** Function called when Device.LocalAgent.Controller.{i}.PeriodicNotifTime is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_PeriodicNotifTime(dm_req_t *req, char *value)
{
    controller_t *cont;
    time_t cur_time;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Set the new value
    cont->periodic_base = RETRY_WAIT_UseRandomBaseIfUnknownTime(val_datetime);

    // Calculate the new next time that this notification should fire
    cur_time = time(NULL);
    cont->next_time_to_fire = CalcNextPeriodicTime(cur_time, cont->periodic_base, cont->periodic_interval);

    // Update the time at which the first periodic notification fires
    UpdateFirstPeriodicNotificationTime();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerRetryMinimumWaitInterval
**
** Called when Device.LocalAgent.Controller.{i}.USPNotifRetryMinimumWaitInterval is modified
**
** \param   req - pointer to structure identifying the parameter
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerRetryMinimumWaitInterval(dm_req_t *req, char *value)
{
    controller_t *cont;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Update cached value
    cont->subs_retry_min_wait_interval = val_uint;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_ControllerRetryIntervalMultiplier
**
** Called when Device.LocalAgent.Controller.{i}.USPNotifRetryIntervalMultiplier is modified
**
** \param   req - pointer to structure identifying the parameter
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_ControllerRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    controller_t *cont;

    // Determine controller to be updated
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Update cached value
    cont->subs_retry_interval_multiplier = val_uint;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_ControllerInheritedRole
**
** Gets the value of Device.LocalAgent.Controller.{i}.InheritedRole
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_ControllerInheritedRole(dm_req_t *req, char *buf, int len)
{
    controller_t *cont;

    // Set default inherited role
    *buf = '\0';

    // Exit if unable to find a matching enabled controller
    cont = FindControllerByInstance(inst1);
    if ((cont == NULL) || (cont->enable == false))
    {
        return USP_ERR_OK;
    }

    // If the code gets here, then we have determined which instance of the Role table is associated with the controller's role
    if (cont->inherited_instance != INVALID)
    {
        USP_SNPRINTF(buf, len, "Device.LocalAgent.ControllerTrust.Role.%d", cont->inherited_instance);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessControllerAdded
**
** Reads the parameters for the specified controller from the database and processes them
**
** \param   cont_instance - instance number of the controller in the controller table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessControllerAdded(int cont_instance)
{
    controller_t *cont;
    int err;
    int i;
    int_vector_t iv;
    int mtp_instance;
    time_t cur_time;
    time_t base;
    char path[MAX_DM_PATH];
    char reference[MAX_DM_PATH];

    // Exit if unable to add another controller
    cont = FindUnusedController();
    if (cont == NULL)
    {
        USP_LOG_Error("%s: Attempted to add too many controllers. Only %d supported", __FUNCTION__, MAX_CONTROLLERS);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Initialise to defaults
    INT_VECTOR_Init(&iv);
    memset(cont, 0, sizeof(controller_t));
    cont->instance = cont_instance;
    cont->inherited_instance = INVALID;
    cont->assigned_instance = INVALID;

    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        cont->mtps[i].instance = INVALID;
    }

    // Exit if unable to determine whether this controller was enabled or not
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_cont_root, cont_instance);
    err = DM_ACCESS_GetBool(path, &cont->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the periodic base time for this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.PeriodicNotifTime", device_cont_root, cont_instance);
    err = DM_ACCESS_GetDateTime(path, &base);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
    cont->periodic_base = RETRY_WAIT_UseRandomBaseIfUnknownTime(base);


    // Exit if unable to get the periodic interval for this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.PeriodicNotifInterval", device_cont_root, cont_instance);
    err = DM_ACCESS_GetUnsigned(path, &cont->periodic_interval);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Calculate the time at which this notification next fires
    cur_time = time(NULL);
    cont->next_time_to_fire = CalcNextPeriodicTime(cur_time, cont->periodic_base, cont->periodic_interval);

    // Update the time at which the next periodic notification should fire
    UpdateFirstPeriodicNotificationTime();

    // Exit if unable to get the minimum subs retry interval for this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.USPNotifRetryMinimumWaitInterval", device_cont_root, cont_instance);
    err = DM_ACCESS_GetUnsigned(path, &cont->subs_retry_min_wait_interval);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the subs retry interval multiplier for this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.USPNotifRetryIntervalMultiplier", device_cont_root, cont_instance);
    err = DM_ACCESS_GetUnsigned(path, &cont->subs_retry_interval_multiplier);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the endpoint ID of this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.EndpointID", device_cont_root, cont_instance);
    err = DM_ACCESS_GetString(path, &cont->endpoint_id);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if the endpoint ID of this controller is not unique
    err = ValidateEndpointIdUniqueness(cont->endpoint_id, cont_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the assigned role of this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.AssignedRole", device_cont_root, cont_instance);
    err = DATA_MODEL_GetParameterValue(path, reference, sizeof(reference), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the value was incorrectly set
    err = UpdateAssignedRole(cont, reference);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the object instance numbers present in this controller's MTP table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP", device_cont_root, cont_instance);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit, issuing a warning, if no MTPs for this controller are present in database
    if (iv.num_entries == 0)
    {
        USP_LOG_Warning("%s: WARNING: No MTP instances for %s.%d", __FUNCTION__, device_cont_root, cont_instance);
        err = USP_ERR_OK;
        goto exit;
    }

    // Iterate over all MTPs, getting their parameters into the controller structure
    // Or deleting them from the database, if they contain invalid parameters
    // NOTE: We need to delete them to prevent them being modified to good values, which then this code does not pickup (because they are not in our internal array)
    for (i=0; i < iv.num_entries; i++)
    {
        mtp_instance = iv.vector[i];

        err = ProcessControllerMtpAdded(cont, mtp_instance);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a controller MTP with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d", device_cont_root, cont_instance, mtp_instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    err = ProcessControllerE2ESessionAdded(cont);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

    // If the code gets here, then we successfully retrieved all data about the controller (even if some of the MTPs were not added)
    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        DestroyController(cont);
    }

    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** ProcessControllerMtpAdded
**
** Reads the parameters for the specified MTP from the database and processes them
**
** \param   cont - pointer to controller in the controller array, which this MTP is associated with
** \param   mtp_instance - instance number of the MTP for the specified controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessControllerMtpAdded(controller_t *cont, int mtp_instance)
{
    int err;
    controller_mtp_t *mtp;
    char path[MAX_DM_PATH];

    // Exit if unable to find a free MTP slot
    mtp = FindUnusedControllerMtp(cont);
    if (mtp == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Initialise to defaults
    memset(mtp, 0, sizeof(controller_mtp_t));
    mtp->instance = mtp_instance;

    // Exit if unable to get the protocol for this MTP
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Protocol", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetEnum(path, &mtp->protocol, mtp_protocols, NUM_ELEM(mtp_protocols));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the enable for this MTP
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Enable", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetBool(path, &mtp->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    if (mtp->enable)
    {
        // Exit if this MTP is not the only enabled MTP with the same protocol for this controller
        err = ValidateMtpUniqueness(mtp->protocol, cont->instance, mtp_instance);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if trying to enable more MTPs than the lower levels support for this protocol
        err = ValidateMtpResourceAvailable(mtp->protocol, cont->instance, mtp_instance);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

#ifndef DISABLE_STOMP
    // Exit if there was an error in the reference to the entry in the STOMP connection table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.STOMP.Reference", device_cont_root, cont->instance, mtp_instance);
    err = DEVICE_MTP_GetStompReference(path, &mtp->stomp_connection_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the name of the controller's STOMP queue
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.STOMP.Destination", device_cont_root, cont->instance, mtp_instance);
    USP_ASSERT(mtp->stomp_controller_queue == NULL);
    err = DM_ACCESS_GetString(path, &mtp->stomp_controller_queue);
    if (err != USP_ERR_OK)
    {
        return err;
    }
#endif

#ifdef ENABLE_COAP
    // Exit if unable to get the name of the controller's CoAP host name
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.CoAP.Host", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetString(path, &mtp->coap_controller_host);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the name of the controller's CoAP resource name
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.CoAP.Path", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetString(path, &mtp->coap.resource);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the name of the controller's CoAP port
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.CoAP.Port", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetUnsigned(path, &mtp->coap.port);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to determine whether to send to this controller using encryption
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.CoAP.EnableEncryption", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetBool(path, &mtp->coap.enable_encryption);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start a CoAP client to this controller (if required)
    if ((can_mtp_connect) && (mtp->protocol == kMtpProtocol_CoAP) && (mtp->enable) && (cont->enable))
    {
        err = COAP_CLIENT_Start(cont->instance, mtp_instance, cont->endpoint_id);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }
#endif

#ifdef ENABLE_MQTT
    // Exit if unable to get the enable for this MTP
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.Enable", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetBool(path, &mtp->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if there was an error in the reference to the entry in the MQTT client table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.MQTT.Reference", device_cont_root, cont->instance, mtp_instance);
    err = DEVICE_MTP_GetMqttReference(path, &mtp->mqtt_connection_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the name of the controller's MQTT queue
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.MQTT.Topic", device_cont_root, cont->instance, mtp_instance);
    USP_ASSERT(mtp->mqtt_controller_topic == NULL);
    err = DM_ACCESS_GetString(path, &mtp->mqtt_controller_topic);
    if (err != USP_ERR_OK)
    {
        return err;
    }
#endif

#ifdef ENABLE_WEBSOCKETS
    // Exit if unable to get the name of the controller's websocket server host name
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.Host", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetString(path, &mtp->websock.host);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the name of the controller's websocket path
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.Path", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetString(path, &mtp->websock.path);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the name of the controller's websocket port
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.Port", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetUnsigned(path, &mtp->websock.port);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the name of the controller's websocket port
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.EnableEncryption", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetBool(path, &mtp->websock.enable_encryption);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the websocket keep alive interval
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.KeepAliveInterval", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetUnsigned(path, &mtp->websock.keep_alive_interval);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the retry interval when connecting to the controller over websockets
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.SessionRetryMinimumWaitInterval", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetUnsigned(path, &mtp->websock.retry_interval);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the retry multiplier when connecting to the controller over websockets
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.WebSocket.SessionRetryIntervalMultiplier", device_cont_root, cont->instance, mtp_instance);
    err = DM_ACCESS_GetUnsigned(path, &mtp->websock.retry_multiplier);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start a WebSocket client to connect to this controller (if required)
    if ((can_mtp_connect) && (mtp->protocol == kMtpProtocol_WebSockets) && (mtp->enable) && (cont->enable))
    {
        WSCLIENT_StartClient(cont->instance, mtp_instance, cont->endpoint_id, &mtp->websock);
    }
#endif

#ifdef ENABLE_UDS
    // Exit if unable to get the UDS reference
    USP_SNPRINTF(path, sizeof(path), "%s.%d.MTP.%d.UDS.UnixDomainSocketRef", device_cont_root, cont->instance, mtp_instance);
    err = DEVICE_MTP_GetUdsReference(path, &mtp->uds_connection_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // The inherited role when using UDS is known before connection, so set it now, so that permissions are correct
    // when generating any USP events before UDS has connected
    if (mtp->protocol == kMtpProtocol_UDS)
    {
        cont->inherited_instance = ROLE_UDS;
    }

    // NOTE: No need to start a UDS Client/Server based on this reference - they are started based on being present in the UnixDomainSockets table
#endif

#ifdef ENABLE_MQTT
    DEVICE_MQTT_UpdateControllerTopics();
#endif

    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        DestroyControllerMtp(mtp);
    }

    return err;
}

/*********************************************************************//**
**
** UpdateAssignedRole
**
** Given a reference value, sets the assigned_role stored in the controller array
**
** \param   cont - pointer to controller in the controller array, whose role we are updating
** \param   reference - path to instance in Device.LocalAgent.ControllerTrust.Role table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UpdateAssignedRole(controller_t *cont, char *reference)
{
    int err;
    int role_instance;

    // Exit if reference is a blank string
    if (*reference == '\0')
    {
        cont->assigned_instance = INVALID;
        return USP_ERR_OK;
    }

    // Exif if the controller trust role instance number does not exist
    err = DM_ACCESS_ValidateReference(reference, "Device.LocalAgent.ControllerTrust.Role.{i}", &role_instance);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    cont->assigned_instance = role_instance;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindUnusedController
**
** Finds the first free controller slot
**
** \param   None
**
** \return  Pointer to first free controller, or NULL if no controller found
**
**************************************************************************/
controller_t *FindUnusedController(void)
{
    int i;
    controller_t *cont;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Exit if found an unused controller
        cont = &controllers[i];
        if (cont->instance == INVALID)
        {
            return cont;
        }
    }

    // If the code gets here, then no free controller slot has been found
    USP_ERR_SetMessage("%s: Only %d controllers are supported.", __FUNCTION__, MAX_CONTROLLERS);
    return NULL;
}

/*********************************************************************//**
**
** FindUnusedControllerMtp
**
** Finds the first free MTP instance for the specified controller
**
** \param   cont - pointer to controller
**
** \return  Pointer to first free MTP instance, or NULL if no MTP instance found
**
**************************************************************************/
controller_mtp_t *FindUnusedControllerMtp(controller_t *cont)
{
    int i;
    controller_mtp_t *mtp;

    // Iterate over all MTP slots for this controller
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        // Exit if found an unused controller MTP
        mtp = &cont->mtps[i];
        if (mtp->instance == INVALID)
        {
            return mtp;
        }
    }

    // If the code gets here, then no free MTP slot has been found for this controller
    USP_ERR_SetMessage("%s: Only %d MTPs are supported per controller.", __FUNCTION__, MAX_CONTROLLER_MTPS);
    return NULL;
}

/*********************************************************************//**
**
** FindControllerMtpFromReq
**
** Gets a pointer to a controller MTP entry in the controllers array
** based on the specified instance numbers
**
** \param   req - pointer to structure identifying the path
** \param   cont - pointer to variable in which to return a pointer to the controller
**
** \return  pointer to MTP entry
**
**************************************************************************/
controller_mtp_t *FindControllerMtpFromReq(dm_req_t *req, controller_t **p_cont)
{
    controller_t *cont;
    controller_mtp_t *mtp;

    // Determine Controller
    // NOTE: We might not find it if it was never added. This could occur if deleting from the DB at startup when we detected that the database params were invalid
    cont = FindControllerByInstance(inst1);
    if (cont == NULL)
    {
        return NULL;
    }

    // Determine Controller MTP
    // NOTE: We might not find it if it was never added. This could occur if deleting from the DB at startup when we detected that the database params were invalid
    mtp = FindControllerMtpByInstance(cont, inst2);
    if (mtp == NULL)
    {
        return NULL;
    }

    // Return the controller and controller MTP referred to by the instance numbers
    *p_cont = cont;
    return mtp;
}

/*********************************************************************//**
**
** FindControllerByInstance
**
** Finds a controller entry by it's data model instance number
**
** \param   cont_instance - instance number of the controller in the data model
**
** \return  pointer to controller entry within the controllers array, or NULL if controller was not found
**
**************************************************************************/
controller_t *FindControllerByInstance(int cont_instance)
{
    int i;
    controller_t *cont;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Exit if found a controller that matches the instance number
        cont = &controllers[i];
        if (cont->instance == cont_instance)
        {
            return cont;
        }
    }

    // If the code gets here, then no matching controller was found
    return NULL;
}

/*********************************************************************//**
**
** FindControllerByEndpointId
**
** Finds the controller matching the specified endpoint_id
**
** \param   endpoint_id - name of the controller to find
**
** \return  pointer to controller entry within the controllers array, or NULL if controller was not found
**
**************************************************************************/
controller_t *FindControllerByEndpointId(char *endpoint_id)
{
    int i;
    controller_t *cont;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Exit if found an enabled controller that matches the endpoint_id
        cont = &controllers[i];
        if ((cont->instance != INVALID) &&
            (strcmp(cont->endpoint_id, endpoint_id)==0))
        {
            return cont;
        }
    }

    // If the code gets here, then no matching controller was found
    return NULL;
}

/*********************************************************************//**
**
** FindEnabledControllerByEndpointId
**
** Finds the enabled controller matching the specified endpoint_id
**
** \param   endpoint_id - name of the controller to find
**
** \return  pointer to controller entry within the controllers array, or NULL if controller was not found
**
**************************************************************************/
controller_t *FindEnabledControllerByEndpointId(char *endpoint_id)
{
    int i;
    controller_t *cont;

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Exit if found an enabled controller that matches the endpoint_id
        cont = &controllers[i];
        if ((cont->instance != INVALID) && (cont->enable == true) &&
            (strcmp(cont->endpoint_id, endpoint_id)==0))
        {
            return cont;
        }
    }

    // If the code gets here, then no matching controller was found
    return NULL;
}

/*********************************************************************//**
**
** FindFirstEnabledMtp
**
** Finds the first enabled MTP for the specified controller, if possible matching the preferred MTP protocol
**
** \param   cont - pointer to controller in the controller array, which this MTP is associated with
** \param   preferred_protocol - preferred protocol to use (NOTE: this is unknown for notification messages and will be set to kMtpProtocol_None)
**
** \return  pointer to controller MTP found, or NULL if none was found
**
**************************************************************************/
controller_mtp_t *FindFirstEnabledMtp(controller_t *cont, mtp_protocol_t preferred_protocol)
{
    int i;
    controller_mtp_t *mtp;
    controller_mtp_t *first_mtp = NULL;

    // Iterate over all enabled MTPs for this controller, finding the first enabled MTP for this controller
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        mtp = &cont->mtps[i];

        if ((mtp->instance != INVALID) && (mtp->enable == true))
        {
            // Exit if found a matching protocol
            if ((preferred_protocol == kMtpProtocol_None) || (preferred_protocol == mtp->protocol))
            {
                return mtp;
            }

            // Save the first MTP found, which we'll use if no matching protocol found
            if (first_mtp == NULL)
            {
                first_mtp = mtp;
            }
        }
    }

    return first_mtp;
}

/*********************************************************************//**
**
** FindControllerMtpByInstance
**
** Finds an MTP entry by it's data model instance number, for the specified controller
**
** \param   cont - pointer to controller that has this MTP
** \param   mtp_instance - instance number of the MTP in the data model
**
** \return  pointer to controller entry within the controllers array, or NULL if controller was not found
**
**************************************************************************/
controller_mtp_t *FindControllerMtpByInstance(controller_t *cont, int mtp_instance)
{
    int i;
    controller_mtp_t *mtp;

    // Iterate over all MTPs for this controller
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        // Exit if found an MTP that matches the instance number
        mtp = &cont->mtps[i];
        if (mtp->instance == mtp_instance)
        {
            return mtp;
        }
    }

    // If the code gets here, then no matching MTP was found
    return NULL;
}

/*********************************************************************//**
**
** FindControllerMtpByProtocol
**
** Finds an MTP entry by it's data model instance number, for the specified controller
**
** \param   cont - pointer to controller that has this MTP
** \param   mtp_instance - instance number of the MTP in the data model
**
** \return  pointer to controller entry within the controllers array, or NULL if controller was not found
**
**************************************************************************/
controller_mtp_t *FindControllerMtpByProtocol(controller_t *cont, mtp_protocol_t protocol)
{
    int i;
    controller_mtp_t *mtp;

    // Iterate over all MTPs for this controller
    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        // Exit if found an MTP that matches the instance number
        mtp = &cont->mtps[i];
        if ((mtp->instance != INVALID) && (mtp->protocol == protocol))
        {
            return mtp;
        }
    }

    // If the code gets here, then no matching MTP was found
    return NULL;
}

/*********************************************************************//**
**
** DestroyController
**
** Frees all memory associated with the specified controller slot
**
** \param   cont - pointer to controller to free
**
** \return  None
**
**************************************************************************/
void DestroyController(controller_t *cont)
{
    int i;
    controller_mtp_t *mtp;

    cont->instance = INVALID;      // Mark controller slot as free
    cont->enable = false;
    USP_SAFE_FREE(cont->endpoint_id);

    for (i=0; i<MAX_CONTROLLER_MTPS; i++)
    {
        mtp = &cont->mtps[i];
        DestroyControllerMtp(mtp);
    }
}

/*********************************************************************//**
**
** DestroyControllerMtp
**
** Frees all memory associated with the specified controller mtp slot
**
** \param   cont - pointer to controller mtp to free
**
** \return  None
**
**************************************************************************/
void DestroyControllerMtp(controller_mtp_t *mtp)
{
    mtp->instance = INVALID;      // Mark controller slot as free
    mtp->protocol = kMtpProtocol_None;
    mtp->enable = false;

#ifndef DISABLE_STOMP
    mtp->stomp_connection_instance = INVALID;
    USP_SAFE_FREE(mtp->stomp_controller_queue);
#endif

#ifdef ENABLE_COAP
    USP_SAFE_FREE(mtp->coap_controller_host);
    USP_SAFE_FREE(mtp->coap.resource);
    mtp->coap.port = 0;
#endif

#ifdef ENABLE_MQTT
    mtp->mqtt_connection_instance = INVALID;
    USP_SAFE_FREE(mtp->mqtt_controller_topic);
#endif

#ifdef ENABLE_WEBSOCKETS
    USP_SAFE_FREE(mtp->websock.host);
    USP_SAFE_FREE(mtp->websock.path);
#endif
}

/*********************************************************************//**
**
** ValidateMtpUniqueness
**
** Validates that only one MTP of the specified protocol is enabled at any one time for the specified controller
** NOTE: This function disregards whether the controller itself is enabled, otherwise this check would also need to be performed on controller enablement
**
** \param   protocol - MTP Protocol that we want to check only one is enabled at a time
** \param   cont_inst - instance number of controller in Device.LocalAgent.Controller.{i} table
** \param   mtp_inst - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.{i} table
**                     NOTE: This is also the instance number which the caller wants to set to the specified protocol
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateMtpUniqueness(mtp_protocol_t protocol, int cont_inst, int mtp_inst)
{
    int i;
    controller_t *cont;
    controller_mtp_t *mtp;
    char *protocol_str;

    // Determine the controller entry
    cont = FindControllerByInstance(cont_inst);
    USP_ASSERT(cont != NULL);

    // Iterate over all MTPs, seeing if any (other than the one currently being set) is enabled and set the the same protocol
    for (i=0; i < MAX_CONTROLLER_MTPS; i++)
    {
        mtp = &cont->mtps[i];

        // Skip this entry if not in use
        if (mtp->instance == INVALID)
        {
            continue;
        }

        // Skip the instance currently being validated - we allow the current MTP to have its protocol set to the same value again !
        if (mtp->instance == mtp_inst)
        {
            continue;
        }

        // Exit if another MTP is enabled, and uses the same protocol
        if ((mtp->enable == true) && (mtp->protocol == protocol))
        {
            protocol_str = TEXT_UTILS_EnumToString(protocol, mtp_protocols, NUM_ELEM(mtp_protocols));
            USP_ERR_SetMessage("%s: Controller can only have one enabled %s MTP (matches %s.%d.MTP.%d)", __FUNCTION__, protocol_str, device_cont_root, cont_inst, mtp->instance);
            return USP_ERR_VALUE_CONFLICT;
        }
    }

    // If the code gets here, then only the instance being validated is set the the specified protocol and enabled
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateMtpResourceAvailable
**
** Validates that we have enough MTP resources available of the specified protocol
** NOTE: This function disregards whether the controller itself is enabled, otherwise this check would also need to be performed on controller enablement
**
** \param   protocol - MTP Protocol that we want to check resource availability of
** \param   cont_inst - instance number of controller in Device.LocalAgent.Controller.{i} table
** \param   mtp_inst - instance number of MTP in Device.LocalAgent.Controller.{i}.MTP.{i} table
**                     NOTE: This is also the instance number which the caller wants to set to the specified protocol
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateMtpResourceAvailable(mtp_protocol_t protocol, int cont_inst, int mtp_inst)
{
    int i, j;
    int max_count;
    int count;
    controller_t *cont;
    controller_mtp_t *mtp;
    char *protocol_str;

    // Determine the maximum number of MTP resources for the specified protocol, exiting if there are no constraints
    switch(protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            max_count = MAX_STOMP_CONNECTIONS;
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            max_count = MAX_COAP_CLIENTS;
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            max_count = MAX_MQTT_CLIENTS;
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            max_count = MAX_WEBSOCKET_CLIENTS;
            break;
#endif
        default:
        case kMtpProtocol_None:
        case kMtpProtocol_Max:
            // All other MTPs have no resource constraints, so just return
            return USP_ERR_OK;
            break;
    }

    // Count the number of currently enabled MTPs (across all controllers) which use this protocol
    count = 1;      // Account for the MTP slot which we want to activate the MTP resource on
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        cont = &controllers[i];
        if (cont->instance != INVALID)
        {
            // Iterate over all MTP slots for this controller
            for (j=0; j<MAX_CONTROLLER_MTPS; j++)
            {
                mtp = &cont->mtps[j];
                if ((mtp->instance != INVALID) && (mtp->enable) && (mtp->protocol == protocol))
                {
                    if ((cont->instance != cont_inst) || (mtp->instance != mtp_inst)) // Only increment if not the MTP slot we want to activate (as we've already accounted for it)
                    {
                        count++;
                    }
                }
            }
        }
    }

    // Exit if activating the specified MTP would exceed the resources available for the specified protocol
    if (count > max_count)
    {
        protocol_str = TEXT_UTILS_EnumToString(protocol, mtp_protocols, NUM_ELEM(mtp_protocols));
        USP_ERR_SetMessage("%s: Resources exceeded: Too many enabled %s MTPs already (maximum=%d) to activate %s.%d.MTP.%d", __FUNCTION__, protocol_str, max_count, device_cont_root, cont_inst, mtp_inst);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // If the code gets here, then the specified MTP can be activated - there are enough resources for it
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateEndpointIdUniqueness
**
** Validates that the EndpointID is unique across all registered controllers
**
** \param   endpoint_id - endpoint_id to determine if it is unique
** \param   cont_instance - Instance number which is expected to match the endpoint_id
**                          This instance is skipped when searching.
**                          It is necessary to allow you to set an endpoint id to be the same value again.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateEndpointIdUniqueness(char *endpoint_id, int cont_instance)
{
    int i;
    controller_t *cont;

    // Interate over all controllers, checking that none match the new EndpointID
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Skip unused controller slots
        cont = &controllers[i];
        if (cont->instance == INVALID)
        {
            continue;
        }

        // Skip the instance which is having it's EndpointID altered
        if (cont->instance == cont_instance)
        {
            continue;
        }

        // Exit if the specified endpointID is already used by another controller
        if (strcmp(cont->endpoint_id, endpoint_id)==0)
        {
            USP_ERR_SetMessage("%s: EndpointID is not unique (matches %s.%d)", __FUNCTION__, device_cont_root, cont->instance);
            return USP_ERR_UNIQUE_KEY_CONFLICT;
        }
    }

    // If the code gets here, then the specified endpointID is unique among all controllers
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CalcNextPeriodicTime
**
** Calculates the next absolute time at which a periodic notification event should be sent
**
** \param   cur_time - current time
** \param   periodic_base - time reference which intervals are relative to
** \param   periodic_interval - time interval between each periodic notification event
**
** \return  next absolute time to fire the periodic notification
**
**************************************************************************/
time_t CalcNextPeriodicTime(time_t cur_time, time_t periodic_base, int periodic_interval)
{
    time_t diff;
    time_t offset;

    if (periodic_base <= cur_time)
    {
        // periodic_base is in the past
        offset = (cur_time - periodic_base) % periodic_interval; // This is the delta to the time of the last inform interval period
        diff = periodic_interval - offset;
    }
    else
    {
        // periodic_base is in the future
        diff = (periodic_base - cur_time) % periodic_interval;
    }

    // Correct for case of currently at a periodic inform interval time
    if (diff == 0)
    {
        diff = periodic_interval;
    }

    return cur_time + diff;
}

/*********************************************************************//**
**
** UpdateFirstPeriodicNotificationTime
**
** Updates the absolute time at which the next periodic notification event should be sent
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UpdateFirstPeriodicNotificationTime(void)
{
    int i;
    controller_t *cont;
    time_t first = END_OF_TIME;
    bool allowed;

    // Exit, if not allowed to connect yet. The sync timer will be left unchanged (scheduled to fire at END_OF_TIME)
    // This is done to prevent Periodic! being queued when NTP time is obtained (the huge jump forward in time causes the sync timer to fire immediately)
    allowed = DEVICE_CONTROLLER_CanMtpConnect();
    if (allowed == false)
    {
        return;
    }

    // Iterate over all controllers
    for (i=0; i<MAX_CONTROLLERS; i++)
    {
        // Skip this entry if it is unused
        cont = &controllers[i];
        if (cont->instance == INVALID)
        {
            continue;
        }

        // Update time of the first periodic notification
        if (cont->next_time_to_fire < first)
        {
            first = cont->next_time_to_fire;
        }
    }

    // Update the timer. We do this every time because we always want the timer to be reactivated
    first_periodic_notification_time = first;
    SYNC_TIMER_Reload(PeriodicNotificationExec, 0, first_periodic_notification_time);
}

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
/*********************************************************************//**
**
** ProcessControllerE2ESessionAdded
**
** Function called when a E2ESession object has been added to Device.LocalAgent.Controller.{i}
**
** \param   cont - pointer to structure identifying the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessControllerE2ESessionAdded(controller_t *cont)
{
    int err = USP_ERR_OK;
    char path[MAX_DM_PATH];

    cont->e2e_session.current_session_id = (E2E_FIRST_VALID_SESS_ID - 1);
    cont->e2e_session.last_sent_sequence_id = 0;
    cont->e2e_session.last_recv_sequence_id = 0;
    SAR_VECTOR_Init(&(cont->e2e_session.received_payloads));

    // Exit if unable to get the End-to-End Session mode of this controller
    USP_SNPRINTF(path, sizeof(path), "%s.%d.E2ESession.SessionMode", device_cont_root, cont->instance);
    err = DM_ACCESS_GetEnum(path, &cont->e2e_session.mode, e2e_session_modes, kE2EMode_Max);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // By default, the current E2ESession.SessionMode value
    cont->e2e_session.mode_buffered = cont->e2e_session.mode;

    USP_SNPRINTF(path, sizeof(path), "%s.%d.E2ESession.MaxUSPRecordSize", device_cont_root, cont->instance);
    err = DM_ACCESS_GetUnsigned(path, &cont->e2e_session.max_record_size);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    cont->e2e_session.status = kE2EStatus_Down;

exit:
    return err;
}

/*********************************************************************//**
**
** Validate_E2ESessionMode
**
** Validates Device.LocalAgent.Controller.{i}.E2ESession.SessionMode
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_E2ESessionMode(dm_req_t *req, char *value)
{
    int err = USP_ERR_OK;
    int result = E2E_CONTEXT_E2eSessionModeToEnum(value);

    if (result == INVALID)
    {
        USP_ERR_SetMessage("%s: Invalid or unsupported E2ESession mode %s", __FUNCTION__, value);
        err = USP_ERR_INVALID_VALUE;
    }

    return err;
}

/*********************************************************************//**
**
** Validate_E2ESessionMaxUSPRecordSize
**
** Validates Device.LocalAgent.Controller.{i}.E2ESession.MaxUSPRecordSize
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_E2ESessionMaxUSPRecordSize(dm_req_t *req, char *value)
{
    if (val_uint == 0)
    {
        return USP_ERR_OK;
    }
    return DM_ACCESS_ValidateRange_Unsigned(req, 512, UINT_MAX);
}

/*********************************************************************//**
**
** Notify_E2ESessionMode
**
** Called when Device.LocalAgent.Controller.{i}.E2ESession.SessionMode is modified
**
** \param   req - pointer to structure identifying the parameter
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_E2ESessionMode(dm_req_t *req, char *value)
{
    static const e2e_event_t kE2E_EVENT_MAP[kE2EMode_Max][kE2EStatus_Max] = {
    //  kE2EStatus_Up,         kE2EStatus_Negotiating, kE2EStatus_Down
       {kE2EEvent_None,        kE2EEvent_None,         kE2EEvent_Establishment}, // kE2EMode_Require
       {kE2EEvent_None,        kE2EEvent_None,         kE2EEvent_None},          // kE2EMode_Allow
       {kE2EEvent_Termination, kE2EEvent_Termination,  kE2EEvent_None},          // kE2EMode_Forbid
    };

    e2e_session_mode_t mode = INVALID;
    e2e_event_t event = kE2EEvent_None;
    int err = USP_ERR_OK;

    // Determine controller to be updated
    controller_t *cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    mode = E2E_CONTEXT_E2eSessionModeToEnum(value);
    USP_ASSERT(mode != INVALID); // Value must already have validated to have got here

    // Store the given SessionMode value; will be applied during E2E_CONTEXT_E2eSessionEvent()
    cont->e2e_session.mode_buffered = mode;

    USP_ASSERT(cont->e2e_session.status < kE2EStatus_Max);
    event = kE2E_EVENT_MAP[mode][cont->e2e_session.status];

    // If an E2E event is needed or the mode changed
    if (event != kE2EEvent_None ||
        cont->e2e_session.mode != mode)
    {
        // Log the input for the event
        USP_LOG_Info("=== E2ESession.SessionMode SET operation on Controller instance %d ===", inst1);
        USP_LOG_Info("E2E Event: %s",E2E_CONTEXT_E2eSessionEventToString(event));
        USP_LOG_Info("E2E Mode: %s => %s", E2E_CONTEXT_E2eSessionModeToString(cont->e2e_session.mode), E2E_CONTEXT_E2eSessionModeToString(mode));

        // This async call will end in E2E_CONTEXT_E2eSessionEvent() function.
        err = DM_EXEC_PostE2eEvent(event, INVALID, inst1);
    }

    return err;
}

/*********************************************************************//**
**
** Notify_E2ESessionMaxUSPRecordSize
**
** Called when Device.LocalAgent.Controller.{i}.E2ESession.MaxUSPRecordSize is modified
**
** \param   req - pointer to structure identifying the parameter
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_E2ESessionMaxUSPRecordSize(dm_req_t *req, char *value)
{
    // Determine controller to be updated
    controller_t *cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Update cached value
    cont->e2e_session.max_record_size = val_uint;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_E2ESessionStatus
**
** Gets the value of Device.LocalAgent.Controller.{i}.E2ESession.Status
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_E2ESessionStatus(dm_req_t *req, char *buf, int len)
{
    controller_t *cont = NULL;

    // Determine controller to be read
    cont = FindControllerByInstance(inst1);
    USP_ASSERT(cont != NULL);

    // Get the E2ESession status of this controller
    USP_STRNCPY(buf, E2E_CONTEXT_E2eSessionStatusToString(cont->e2e_session.status), len);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Async_E2ESessionReset
**
** Starts the asynchronous Reset operation for the E2ESession context.
** Post a signal to perform the operation on the DataModel thread.
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   input_args - not used
** \param   request - instance number of this operation in the Device.LocalAgent.Request table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Async_E2ESessionReset(dm_req_t *req, kv_vector_t *input_args, int request)
{
    static const e2e_event_t kE2E_EVENT_MAP[kE2EMode_Max][kE2EStatus_Max] = {
    //  kE2EStatus_Up,         kE2EStatus_Negotiating, kE2EStatus_Down
       {kE2EEvent_Restart,     kE2EEvent_Restart,      kE2EEvent_Establishment}, // kE2EMode_Require
       {kE2EEvent_Termination, kE2EEvent_Termination,  kE2EEvent_None},          // kE2EMode_Allow
       {kE2EEvent_Termination, kE2EEvent_Termination,  kE2EEvent_None},          // kE2EMode_Forbid
    };

    int err = USP_ERR_OK;
    e2e_session_t* curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(inst1);
    e2e_event_t event = kE2EEvent_None;

    if (curr_e2e_session == NULL)
    {
        USP_ERR_SetMessage("%s: Device.LocalAgent.Controller.%d is not enabled", __FUNCTION__, inst1);
        return USP_ERR_COMMAND_FAILURE;
    }

    USP_ASSERT(curr_e2e_session->mode < kE2EMode_Max);
    USP_ASSERT(curr_e2e_session->status < kE2EStatus_Max);
    event = kE2E_EVENT_MAP[curr_e2e_session->mode][curr_e2e_session->status];

    if (event == kE2EEvent_None)
    {
        USP_ERR_SetMessage("%s: Nothing to do", __FUNCTION__);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if unable to signal that this operation is active
    err = USP_SIGNAL_OperationStatus(request, "Active");
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Log the input conditions for the operation
    USP_LOG_Info("=== E2ESession.Reset async operation on Controller instance %d ===", inst1);
    USP_LOG_Info("E2E Event: %s",E2E_CONTEXT_E2eSessionEventToString(event));

    // This async call will end in E2E_CONTEXT_E2eSessionEvent() function.
    err = DM_EXEC_PostE2eEvent(event, request, inst1);

    if (err != USP_ERR_OK)
    {
        return USP_ERR_COMMAND_FAILURE;
    }

    return USP_ERR_OK;
}
#endif

//------------------------------------------------------------------------------------------
// Code to test the CalcNextPeriodicTime() function
// NOTE: In test cases below, the periodic_interval is assumed to be 5 seconds
#if 0
time_t calc_next_periodic_time_test_cases[] =
{
    // cur_time // periodic_base    // next_time
    0,          0,                  10,
    0,          0,                  5,
    1,          0,                  5,
    4,          0,                  5,
    5,          0,                  10,

    4,          5,                  5,
    5,          5,                  10,

    6,          5,                  10,
    9,          5,                  10,
    10,         5,                  15,

    50,         100,                55,
    51,         100,                55,
    52,         100,                55,
    200,        100,                205,
    201,        100,                205,
    204,        100,                205,
    205,        100,                210,
};

void TestCalcNextPeriodicTime(void)
{
    int i;
    time_t *p;
    time_t result;

    p = calc_next_periodic_time_test_cases;
    for (i=0; i < NUM_ELEM(calc_next_periodic_time_test_cases); i+=3)
    {

        result = CalcNextPeriodicTime(p[0], p[1], 5);
        if (result != p[2])
        {
            printf("ERROR: [cur_time=%d, periodic_base=%d] Expected %d (got %d)\n", (int)p[0], (int)p[1], (int)p[2], (int)result);
        }
        p += 3;
    }
}
#endif
