/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020, BT PLC
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
 * \file device.h
 *
 * Header file containing the APIs for the Data model implementation components
 *
 */
#ifndef DEVICE_H
#define DEVICE_H

#include "vendor_defs.h"  // For E2ESESSION_EXPERIMENTAL_USP_V_1_2 and REMOVE_DEVICE_SECURITY

#ifndef REMOVE_DEVICE_SECURITY
#include <openssl/ssl.h>
#endif

#include "kv_vector.h"
#include "usp_api.h"
#include "subs_vector.h"
#include "mtp_exec.h"
#include "mqtt.h"

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include "e2e_defs.h"
#endif

//------------------------------------------------------------------------------
// Enumeration of what to do when USP Agent exits gracefully (ie after all USP responses have been sent)
typedef enum
{
    kExitAction_Exit,
    kExitAction_Reboot,
    kExitAction_FactoryReset
} exit_action_t;

//------------------------------------------------------------------------------
// Combined controller trust role consisting of inherited and assigned role
typedef struct
{
    int inherited_index;      // index of the role in roles[] and in node->permissions[] for the inherited role
    int assigned_index;       // index of the role in roles[] and in node->permissions[] for the assigned role
} combined_role_t;

#define INTERNAL_ROLE             NULL   // Pointer to combined role used internally by Data Model. This always permits all operations (Even at bootup, when the permissions table has not been seeded yet)

//------------------------------------------------------------------------------
// Controller information to be used inside USP message processing
typedef struct
{
    char *endpoint_id;
} controller_info_t;

//------------------------------------------------------------------------------
// Structure containing cause of last boot cycle
typedef struct
{
    char *cause;                     // cause of the last reboot
    char *command_key;               // command_key associated with the last reboot
    char *cur_software_version;      // Current software version that is running
    char *last_software_version;     // Software version before the current boot period
    int request_instance;            // Instance number of the request that initiated the reboot, or INVALID if reboot was not initiated by an operation
    bool is_firmware_updated;       // whether the last reboot caused a different firmware image to run
} reboot_info_t;

//------------------------------------------------------------------------------
// Enumeration to specify whether the Unix domain socket path is used for communication to/from the Broker's Controller or Agent
typedef enum
{
    kUdsPathType_Invalid = INVALID,
    kUdsPathType_BrokersAgent,
    kUdsPathType_BrokersController,
} uds_path_t;

//------------------------------------------------------------------------------
// Structure specifying the STOMP destination that a response to a USP message must be sent
#ifndef DISABLE_STOMP
typedef struct
{
    int instance;       // Instance number in Device.STOMP.Connection.{i}
    char *dest;         // STOMP destination to send the response back to (only set if reply_to was specified in the received STOMP frame)
} from_stomp_t;
#endif
//------------------------------------------------------------------------------
// Structure specifying the CoAP destination that a response to a USP message must be sent
#ifdef ENABLE_COAP
typedef struct
{
    char *host;                    // Percent encoded hostname
    int port;
    char *resource;                // Percent encoded resource name
    bool encryption;
    bool reset_session_hint;       // Set if an existing DTLS session with this host should be reset.
                                   // If we know that the USP request came in on a new DTLS session, then it is likely
                                   // that the USP response must be sent back on a new DTLS session also. Without this,
                                   // the CoAP retry mechanism will cause the DTLS session to restart, but it is a while
                                   // before the retry is triggered, so this hint speeds up communications
} from_coap_t;
#endif

//------------------------------------------------------------------------------
// Structure specifying the MQTT destination that a response to a USP message must be sent
#ifdef ENABLE_MQTT
typedef struct
{
    int instance;
    char *topic;           // only set if reply_to was specified in the received MQTT packet
} from_mqtt_t;
#endif

//------------------------------------------------------------------------------
// Structure specifying the Websocket destination that a response to a USP message must be sent
#ifdef ENABLE_WEBSOCKETS
typedef struct
{
    int client_cont_instance;         // Controller instance number in Device.LocalAgent.Controller.{i}
    int client_mtp_instance;          // MTP instance number in Device.LocalAgent.Controller.{i}.MTP.{i}
    int serv_conn_id;                 // If USP record was received by agent's websockets server, then this uniquely identifies the connection.
                                      // It is set to INVALID for USP Records received by the agent's websocket client.
} from_ws_t;
#endif

//------------------------------------------------------------------------------
// Structure specifying the Unix Domain Socket (UDS) destination that a response to a USP message must be sent
#ifdef ENABLE_UDS
typedef struct
{
    unsigned conn_id;               // uniquely identifies the UDS connection on which this record was received
    uds_path_t path_type;           // identifies whether the UDS connection on which this record was received was the Broker's controller socket or the Broker's Agent socket
} from_uds_t;
#endif

//------------------------------------------------------------------------------
// Structure specifying the destination that a response to a USP message must be sent
typedef struct
{
    bool is_reply_to_specified; // Set if reply_to was specified in the received MTP frame
                                // If set to false, this is used by STOMP and MQTT MTPs to indicate that the destination topic was not provided in the MTP frame, and so should be determined from the data model
                                // Also if set to false (with no MTP protocol set) it is used with notifications to indicate that the datamodel should be used to determine the MTP destination
    mtp_protocol_t protocol;    // Protocol on which the USP message was received

    union
    {
#ifndef DISABLE_STOMP
        from_stomp_t stomp;
#endif
#ifdef ENABLE_COAP
        from_coap_t coap;
#endif
#ifdef ENABLE_MQTT
        from_mqtt_t mqtt;
#endif
#ifdef ENABLE_WEBSOCKETS
        from_ws_t ws;
#endif
#ifdef ENABLE_UDS
        from_uds_t uds;
#endif
    };
} mtp_conn_t;

//------------------------------------------------------------------------------
// Data model components API
#ifndef REMOVE_DEVICE_TIME
int DEVICE_TIME_Init(void);
int DEVICE_TIME_Start(void);
#endif
int DEVICE_LOCAL_AGENT_Init(void);
int DEVICE_LOCAL_AGENT_SetDefaults(void);
int DEVICE_LOCAL_AGENT_Start(void);
int DEVICE_LOCAL_AGENT_ScheduleReboot(exit_action_t exit_action, char *reboot_cause, char *command_key, int request_instance);
exit_action_t DEVICE_LOCAL_AGENT_GetExitAction(void);
int DEVICE_LOCAL_AGENT_SetDefaultRebootCause(void);
char *DEVICE_LOCAL_AGENT_GetEndpointID(void);
#ifndef REMOVE_DEVICE_BOOT_EVENT
void DEVICE_LOCAL_AGENT_GetRebootInfo(reboot_info_t *info);
#endif
bool DEVICE_LOCAL_AGENT_GetDualStackPreference(void);
void DEVICE_LOCAL_AGENT_Stop(void);
int DEVICE_CONTROLLER_Init(void);
int DEVICE_CONTROLLER_Start(void);
void DEVICE_CONTROLLER_Stop(void);
int DEVICE_CONTROLLER_FindInstanceByEndpointId(char *endpoint_id);
int DEVICE_CONTROLLER_QueueBinaryMessage(mtp_send_item_t *msi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtp_conn, time_t expiry_time);
bool DEVICE_CONTROLLER_IsMTPConfigured(char *endpoint_id, mtp_protocol_t protocol);
char *DEVICE_CONTROLLER_FindEndpointIdByInstance(int instance);
char *DEVICE_CONTROLLER_FindEndpointByMTP(mtp_conn_t *mtpc);
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
e2e_session_t *DEVICE_CONTROLLER_FindE2ESessionByInstance(int instance);
e2e_session_t *DEVICE_CONTROLLER_FindE2ESessionByEndpointId(char *endpoint_id);
#endif
int DEVICE_CONTROLLER_GetCombinedRoleByInstance(int instance, combined_role_t *combined_role);
void DEVICE_CONTROLLER_GetCombinedRoleByEndpointId(char *endpoint_id, int role_instance, mtp_protocol_t protocol, combined_role_t *combined_role);
void DEVICE_CONTROLLER_SetRolesFromStomp(int stomp_instance, int role_instance);
int DEVICE_CONTROLLER_GetSubsRetryParams(char *endpoint_id, unsigned *min_wait_interval, unsigned *interval_multiplier);
void DEVICE_CONTROLLER_GetMqttControllerTopics(int mqtt_instance, kv_vector_t *controller_topics);
void DEVICE_CONTROLLER_QueueStompConnectRecord(int stomp_instance, char *agent_queue);
void DEVICE_CONTROLLER_NotifyStompConnDeleted(int stomp_instance);
bool DEVICE_CONTROLLER_CanMtpConnect(void);
void DEVICE_CONTROLLER_StartAllMtpClients(void);
int DEVICE_MTP_Init(void);
int DEVICE_MTP_Start(void);
void DEVICE_MTP_Stop(void);
char *DEVICE_MTP_EnumToString(mtp_protocol_t protocol);
int DEVICE_MTP_ValidateStompReference(dm_req_t *req, char *value);
int DEVICE_MTP_GetStompReference(char *path, int *stomp_reference_instance);
char *DEVICE_MTP_GetAgentStompQueue(int instance);
void DEVICE_MTP_NotifyStompConnDeleted(int stomp_instance);
int DEVICE_STOMP_Init(void);
int DEVICE_STOMP_Start(void);
void DEVICE_STOMP_Stop(void);
int DEVICE_STOMP_StartAllConnections(void);
int DEVICE_STOMP_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char *controller_queue, char *agent_queue, time_t expiry_time);
void DEVICE_STOMP_ScheduleReconnect(int instance);
mtp_status_t DEVICE_STOMP_GetMtpStatus(int instance);
int DEVICE_STOMP_CountEnabledConnections(void);
void DEVICE_STOMP_GetDestinationFromServer(int instance, char *buf, int len);
int DEVICE_WEBSOCK_Init(void);
int DEVICE_WEBSOCK_Start(void);
void DEVICE_WEBSOCK_Stop(void);
int DEVICE_UDS_Init(void);
int DEVICE_UDS_Start(void);
void DEVICE_UDS_Stop(void);
int DEVICE_SUBSCRIPTION_Init(void);
int DEVICE_SUBSCRIPTION_Start(void);
void DEVICE_SUBSCRIPTION_Stop(void);
void DEVICE_SUBSCRIPTION_Update(int id);
void DEVICE_SUBSCRIPTION_ProcessAllOperationCompleteSubscriptions(char *command, char *command_key, int err_code, char *err_msg, kv_vector_t *output_args);
void DEVICE_SUBSCRIPTION_ResolveObjectCreationPaths(void);
void DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths(void);
void DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(char *obj_path, subs_notify_t notify_type);
void DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions(void);
void DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions(char *event_name, kv_vector_t *output_args);
void DEVICE_SUBSCRIPTION_SendPeriodicEvent(int cont_instance);
void DEVICE_SUBSCRIPTION_NotifyControllerDeleted(int cont_instance);
void DEVICE_SUBSCRIPTION_Dump(void);

#ifndef REMOVE_DEVICE_SECURITY
// Typedef for SSL verify callback
typedef int ssl_verify_callback_t(int preverify_ok, X509_STORE_CTX *x509_ctx);

int DEVICE_SECURITY_Init(void);
int DEVICE_SECURITY_Start(void);
void DEVICE_SECURITY_Stop(void);
int DEVICE_SECURITY_GetControllerTrust(STACK_OF(X509) *cert_chain, int *role_instance);
bool DEVICE_SECURITY_IsClientCertAvailable(void);
SSL_CTX *DEVICE_SECURITY_CreateSSLContext(const SSL_METHOD *method, int verify_mode, ssl_verify_callback_t verify_callback);
int DEVICE_SECURITY_LoadTrustStore(SSL_CTX *ssl_ctx, int verify_mode, ssl_verify_callback_t verify_callback);
int DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(int preverify_ok, X509_STORE_CTX *x509_ctx, STACK_OF(X509) **p_cert_chain);
void DEVICE_SECURITY_GetClientCertStatus(bool *available, bool *matches_endpoint);
int DEVICE_SECURITY_TrustCertVerifyCallback(int preverify_ok, X509_STORE_CTX *x509_ctx);
int DEVICE_SECURITY_NoSaveTrustCertVerifyCallback(int preverify_ok, X509_STORE_CTX *x509_ctx);
int DEVICE_SECURITY_AddCertHostnameValidation(SSL* ssl, const char* name, size_t length);
int DEVICE_SECURITY_AddCertHostnameValidationCtx(SSL_CTX* ssl_ctx, const char* name, size_t length);
int DEVICE_SECURITY_ValidateALPN(dm_req_t *req, char *value);
int DEVICE_SECURITY_SetALPN(SSL_CTX *ssl_ctx, char *alpn);
#endif

int DEVICE_CTRUST_Init(void);
int DEVICE_CTRUST_Start(void);
void DEVICE_CTRUST_Stop(void);
int DEVICE_CTRUST_AddCertRole(int cert_instance, int role_instance, bool signal_event);
int DEVICE_CTRUST_GetCertInheritedRole(int cert_instance);
int DEVICE_CTRUST_RoleInstanceToIndex(int role_instance);
int DEVICE_CTRUST_RoleIndexToInstance(int role_index);
void DEVICE_CTRUST_ApplyPermissionsToSubTree(char *path);
int DEVICE_CTRUST_SetRoleParameter(int instance, char *param_name, char *new_value);
int DEVICE_CTRUST_SetPermissionParameter(int instance1, int instance2, char *param_name, char *new_value);
int DEVICE_REQUEST_Init(void);
int DEVICE_REQUEST_Add(char *path, char *command_key, int *instance);
void DEVICE_REQUEST_OperationComplete(int instance, int err_code, char *err_msg, kv_vector_t *output_args);
void DEVICE_REQUEST_UpdateOperationStatus(int instance, char *status);
int DEVICE_REQUEST_RestartAsyncOperations(void);
int DEVICE_REQUEST_PersistOperationArgs(int instance, kv_vector_t *args, char *prefix);
int DEVICE_REQUEST_CountMatchingRequests(char *command_path);
void DEVICE_REQUEST_DeleteInstance(int instance);
int DEVICE_BULKDATA_Init(void);
int DEVICE_BULKDATA_Start(void);
void DEVICE_BULKDATA_Stop(void);
void DEVICE_BULKDATA_NotifyTransferResult(int profile_id, bdc_transfer_result_t transfer_result);
#ifndef REMOVE_SELF_TEST_DIAG_EXAMPLE
int DEVICE_SELF_TEST_Init(void);
#endif
int DEVICE_MQTT_Init(void);
int DEVICE_MQTT_Start(void);
void DEVICE_MQTT_Stop(void);
int DEVICE_MQTT_StartAllClients(void);
void DEVICE_MQTT_ScheduleReconnect(int instance);
void DEVICE_MQTT_UpdateClientId(int instance, char *client_id);
mtp_status_t DEVICE_MQTT_GetMtpStatus(int instance);
char *DEVICE_MTP_GetAgentMqttResponseTopic(int instance);
mqtt_qos_t DEVICE_MTP_GetAgentMqttPublishQos(int instance);
int DEVICE_MQTT_CountEnabledConnections(void);
void DEVICE_MQTT_UpdateControllerTopics(void);
int DEVICE_MTP_GetMqttReference(char *path, int *mqtt_connection_instance);
int DEVICE_MTP_GetUdsReference(char *path, int *uds_connection_instance);
void DEVICE_MTP_StartMtpServers(void);
void DEVICE_CONTROLLER_NotifyMqttConnDeleted(int mqtt_instance);
void DEVICE_MTP_NotifyMqttConnDeleted(int mqtt_instance);
int DEVICE_MTP_ValidateMqttReference(dm_req_t *req, char *value);
void DEVICE_CONTROLLER_SetRolesFromMqtt(int mqtt_instance, int role_instance);
void DEVICE_CONTROLLER_SetInheritedRole(int cont_instance, int role_instance);

#ifdef ENABLE_WEBSOCKETS
int DEVICE_CONTROLLER_CountEnabledWebsockClientConnections(void);
#endif

#ifndef REMOVE_USP_BROKER
int DEVICE_SUBSCRIPTION_RouteNotification(Usp__Msg *usp, int instance);
bool DEVICE_SUBSCRIPTION_MarkVendorLayerSubs(int broker_instance, subs_notify_t notify_type, char *path, int group_id);
void DEVICE_SUBSCRIPTION_StartAllVendorLayerSubsForGroup(int group_id);
void DEVICE_SUBSCRIPTION_FreeAllVendorLayerSubsForGroup(int group_id);
int DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs(int group_id, int broker_instance, int service_instance, char *path);
bool DEVICE_SUBSCRIPTION_IsMatch(int broker_instance, subs_notify_t notify_type, char *path);
#endif
#if defined(ENABLE_UDS) && !(defined(REMOVE_USP_BROKER) && defined(REMOVE_USP_SERVICE))
void DEVICE_CONTROLLER_AddController_UDS(char *endpoint_id, int uds_instance);
#endif
#ifndef REMOVE_USP_SERVICE
char *DEVICE_CONTROLLER_FindFirstControllerEndpoint(void);
#endif
//------------------------------------------------------------------------------
// Tables used to convert to/from an enumeration to/from a string
extern const enum_entry_t mtp_protocols[kMtpProtocol_Max];

extern const enum_entry_t notify_types[kSubNotifyType_Max];
extern const enum_entry_t mqtt_protocol[kMqttProtocol_Max];
//------------------------------------------------------------------------------
// Pointers to strings containing paths in the data model
extern char *device_req_root;

//-----------------------------------------------------------------------------------------------
// Global variables set by command line
extern char *auth_cert_file;
extern char *usp_trust_store_file;

//-----------------------------------------------------------------------------------------------
/*
 *  The maximum number of UDP Echo results that the platform will save
 *  This needs to be limited, because typically UDP echo will be run many thousands of times
 *  and there isn't enough memory to store all results.
 */
#define UDP_ECHO_MAX_RESULTS 25

#endif
