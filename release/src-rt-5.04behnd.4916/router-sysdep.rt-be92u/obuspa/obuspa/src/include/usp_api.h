/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file usp_api.h
 *
 * Header file containing the API functions provided by USP Agent core that may be called by a vendor
 *
 */
#ifndef USP_API_H
#define USP_API_H

#include "vendor_defs.h"  // For MAX_DM_INSTANCE_ORDER and REMOVE_DEVICE_SECURITY

#include <time.h>
#include <stdbool.h>

#ifndef REMOVE_DEVICE_SECURITY
#include <openssl/ssl.h>
#endif

//-----------------------------------------------------------------------------------------
// Magic values used to denote invalid
#define INVALID (-1)

//-----------------------------------------------------------------------------------------
// Key-value pair type
typedef struct
{
    char *key;
    char *value;
} kv_pair_t;

//-----------------------------------------------------------------------------------------
// Key-value pair vector type
typedef struct
{
    kv_pair_t *vector;
    int num_entries;
} kv_vector_t;

//-----------------------------------------------------------------------------------------
// Vector containing instance numbers
typedef struct
{
    int *vector;
    int num_entries;
} int_vector_t;

//-----------------------------------------------------------------------------------------
// Vector containing strings
typedef struct
{
    char **vector;
    int num_entries;
} str_vector_t;

//-------------------------------------------------------------------------
// Structure used for tables to convert from a string to an enumeration
typedef struct
{
    int value;
    char *name;
} enum_entry_t;

//-------------------------------------------------------------------------
// Enumeration of expression operators
typedef enum
{
    kExprOp_Equal = 0,              // '=='
    kExprOp_NotEqual,               // '!='
    kExprOp_LessThanOrEqual,        // '<='
    kExprOp_GreaterThanOrEqual,     // '>='
    kExprOp_LessThan,               // '<'
    kExprOp_GreaterThan,            // '>'
    kExprOp_Equals,                 // '='

    kExprOp_Max
} expr_op_t;

//-------------------------------------------------------------------------
// Union containing the native value for a parameter in a Get or Set ParameterValue operation
// NOTE: Strings (DM_STRING, DM_BASE64, DM_HEXBIN) are not transferred by this union.
//       Instead, they are transferred using the buf parameter of the vendor hook
typedef union
{
    time_t value_datetime;              // For DM_DATETIME parameters
    bool value_bool;                    // For DM_BOOL parameters
    int value_int;                      // For DM_INT parameters
    unsigned value_uint;                // For DM_UINT parameters
    unsigned long long value_ulong;     // For DM_ULONG parameters
    double value_decimal;               // For DM_DECIMAL parameters
    long long value_long;               // For DM_LONG parameters
} dm_val_union_t;

//-------------------------------------------------------------------------
// Structures passed to vendor hooks identifying the data model path
typedef struct
{
    // NOTE: Do not change the order of variables in this structure. They must match dm_instances_t
    int order;    // Number of instance numbers in this array, and hence number of instance separators in the path
    int instances[MAX_DM_INSTANCE_ORDER];
} dm_req_instances_t;

typedef struct
{
    char *path;         // Pointer to full path of the specific parameter or object
    char *schema_path;  // Pointer to schema path of the parameter or object
    dm_req_instances_t *inst;   // Pointer to instances information for the parameter or object
    dm_val_union_t val_union;   // When performing a Set Parameter Value, stores the new value converted to it's native type
    int group_id;       // group_id of the data model provider component implementing the data model path (or NON_GROUPED if implemented internally)
} dm_req_t;

//------------------------------------------------------------------------------
// Last cause of STOMP connection failure
typedef enum
{
    kStompFailure_None,                     // No failure. Connection may have been disabled/re-enabled, or connection params changed, or Agent's IP address changed, or Agent switched connection interface
    kStompFailure_ServerDNS,                // Failed to resolve the hostname
    kStompFailure_Authentication,           // Authentication with STOMP server failed (either password or SSL handshake failed).
    kStompFailure_Connect,                  // Failed to connect or network unreachable (CPE has no WAN IP address)
    kStompFailure_ReadWrite,                // Failed whilst sending/receiving
    kStompFailure_Timeout,                  // Failed due to STOMP handshake or server heartbeat timeout
    kStompFailure_Misconfigured,            // Agent or controller queue name not setup, or entry disabled
    kStompFailure_OtherError                // STOMP protocol error or internal error
} stomp_failure_t;

//------------------------------------------------------------------------------
// Enumeration for result of a BulkData Collection HTTP Post
typedef enum
{
    kBDCTransferResult_Success,
    kBDCTransferResult_Failure_DNS,         // DNS failed to lookup the BDC server
    kBDCTransferResult_Failure_Auth,        // Authentication failure - either password or SSL
    kBDCTransferResult_Failure_Connect,     // Failed to connect to the BDC server or network unreachable (CPE has no WAN IP address)
    kBDCTransferResult_Failure_ReadWrite,   // Failed whilst reading or writing to the BDC server
    kBDCTransferResult_Failure_Timeout,     // Timeout waiting for response from BDC server
    kBDCTransferResult_Failure_Other        // Other failures - typically protocol errors or unsuccessful HTTP response codes returned
} bdc_transfer_result_t;

//------------------------------------------------------------------------------------
// Convenience defines which alias variables within the dm_req_t structure
// to make vendor hook code easier to read (if a little more obscure)
// NOTE: By convention, these should only be used in Vendor Hooks
#define inst1   (req->inst->instances[0])
#define inst2   (req->inst->instances[1])
#define inst3   (req->inst->instances[2])
#define inst4   (req->inst->instances[3])
#define inst5   (req->inst->instances[4])
#define inst6   (req->inst->instances[5])

#define val_datetime  req->val_union.value_datetime
#define val_bool      req->val_union.value_bool
#define val_int       req->val_union.value_int
#define val_uint      req->val_union.value_uint
#define val_long      req->val_union.value_long
#define val_ulong     req->val_union.value_ulong
#define val_decimal   req->val_union.value_decimal


//------------------------------------------------------------------------------
// Bits in bitmask defining permissions. If the bit is set, then the permission is granted
// NOTE: The definitions of these bits have been ordered to match the bits in the data model parameters. This is for convenience, the code does not rely on this ordering
// Param rw-n
#define PERMIT_GET                0x8000 // Grants the capability to read the value of the Parameter via Get and read the meta-information of the Parameter via GetSupportedDM
#define PERMIT_SET                0x4000 // Grants the capability to update the value of the Parameter via Add or Set
#define PERMIT_SUBS_VAL_CHANGE    0x1000 // Grants the capability to receive Notify Messages of type ValueChange for this Parameter

// Obj rw-n
#define PERMIT_OBJ_INFO           0x0800 // Grants the capability to read the meta-information of the Object via GetSupportedDM
#define PERMIT_ADD                0x0400 // Grants the capability to create a new instance of a Multi-Instanced Object via Add
#define PERMIT_SUBS_OBJ_ADD       0x0100 // Grants the capability to receive Notify Messages of type ObjectCreation for this Object (multi-instance objects only)

// InstantiatedObj rw-n
#define PERMIT_GET_INST           0x0080 // Grants the capability to read the instance numbers and unique keys of the Instantiated Object via GetInstances and read the value of Parameters related to the Instantiated Object via a Get containing a search expression or wildcard in place of the instance identifier
#define PERMIT_DEL                0x0040 // Grants the capability to remove an existing instance of an Instantiated Object via Delet
#define PERMIT_SUBS_OBJ_DEL       0x0010 // Grants the capability to receive Notify Messages of type ObjectDeletion for this Instantiated Object

// CommandEvent r-xn
#define PERMIT_CMD_INFO           0x0008 // Grants the capability to read the meta-information of the Command (including input and output arguments) and Event (including arguments) via GetSupportedDM
#define PERMIT_OPER               0x0002 // Grants the capability to execute the Command via Operate, but grants no capabilities to an Event
#define PERMIT_SUBS_EVT_OPER_COMP 0x0001 // Grants the capability to receive Notify Messages of type OperationComplete for this Event or Command.

#define PERMIT_NONE               0x0000 // Grants no capabilities
#define PERMIT_ALL                0xFFFF // Grants all capabilities

//---------------------------------------------------------------------
// Structure for each element of trust store array
typedef struct
{
    const unsigned char *cert_data;
    int cert_len;
    int role_instance;        // Role in Device.LocalAgent.ControllerTrust.Role.{i} that this certificate permits
} trust_store_t;

//---------------------------------------------------------------------
// Structure describing the certificate and private key for this agent CPE
typedef struct
{
    unsigned char *cert_data;
    int cert_len;
    unsigned char *key_data;
    int key_len;
} agent_cert_info_t;


//-------------------------------------------------------------------------
// Enumeration of types of subscriptions
typedef enum
{
    kSubNotifyType_Invalid = INVALID,   // Used to mark a subscription for garbage collection and also as a return value if we failed to convert the textual representation of this enum
    kSubNotifyType_None = 0,            // This is the default value for the notification type parameter - indicates none setup yet
    kSubNotifyType_ValueChange = 1,
    kSubNotifyType_ObjectCreation,
    kSubNotifyType_ObjectDeletion,
    kSubNotifyType_OperationComplete,
    kSubNotifyType_Event,

    kSubNotifyType_Max                  // This should always be the last value in this enumeration. It is used to statically size arrays based on one entry for each active enumeration
} subs_notify_t;

//-------------------------------------------------------------------------
// Structure representing each parameter to set in the create object vendor hook
typedef struct
{
    char *param_name;   // IN: Name of parameter to set in the object. Set by caller of create object vendor hook
    char *value;        // IN: Value of parameter to set in the object. Set by caller of create object vendor hook
    bool is_required;   // IN: True if the parameter must be set in the object successfully. False if the USP request message doesn't care if the parameter is set successfully or not. Set by caller of create object vendor hook
    int err_code;       // OUT: Error code, if this parameter failed to set in the new object. Set by create object vendor hook
    char *err_msg;      // OUT: Error message, if this parameter failed to set in the new object. Set by create object vendor hook
} group_add_param_t;

//------------------------------------------------------------------------------------
// Enumeration for level of information being logged
typedef enum
{
    kLogLevel_Off = 0,
    kLogLevel_Error = 1,
    kLogLevel_Warning = 2,
    kLogLevel_Info = 3,
    kLogLevel_Debug = 4,
    kLogLevel_Trace = 5,

    kLogLevel_Max       // This should always be the last value in the enum
} log_level_t;

//------------------------------------------------------------------------------------
// Enumeration for type of information being logged
typedef enum
{
    kLogType_Debug,
    kLogType_Dump,
    kLogType_Protocol
} log_type_t;

//-------------------------------------------------------------------------
// Typedefs for data model callback functions (vendor hooks)
typedef int (*dm_get_value_cb_t)(dm_req_t *req, char *buf, int len);
typedef int (*dm_set_value_cb_t)(dm_req_t *req, char *buf);
typedef int (*dm_add_cb_t)(dm_req_t *req);
typedef int (*dm_del_cb_t)(dm_req_t *req);

typedef int (*dm_validate_value_cb_t)(dm_req_t *req, char *value);
typedef int (*dm_notify_set_cb_t)(dm_req_t *req, char *value);

typedef int (*dm_validate_add_cb_t)(dm_req_t *req);
typedef int (*dm_notify_add_cb_t)(dm_req_t *req);

typedef int (*dm_validate_del_cb_t)(dm_req_t *req);
typedef int (*dm_notify_del_cb_t)(dm_req_t *req);

typedef int (*dm_sync_oper_cb_t)(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);
typedef int (*dm_async_oper_cb_t)(dm_req_t *req, kv_vector_t *input_args, int instance);
typedef int (*dm_async_restart_cb_t)(dm_req_t *req, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args);

typedef int (*dm_get_group_cb_t)(int group_id, kv_vector_t *params);
typedef int (*dm_set_group_cb_t)(int group_id, kv_vector_t *params, unsigned *types, int *failure_index);
typedef int (*dm_add_group_cb_t)(int group_id, char *path, int *instance);
typedef int (*dm_del_group_cb_t)(int group_id, char *path);
typedef int (*dm_create_obj_cb_t)(int group_id, char *path, group_add_param_t *params, int num_params, int *instance, kv_vector_t *unique_keys);
typedef int (*dm_multi_del_cb_t)(int group_id, bool allow_partial, char **paths, int num_paths, int *failure_index);
typedef int (*dm_refresh_instances_cb_t)(int group_id, char *path, int *expiry_period);
typedef int (*dm_subscribe_cb_t)(int instance, int group_id, subs_notify_t notify_type, char *path, bool persistent);
typedef int (*dm_unsubscribe_cb_t)(int instance, int group_id, subs_notify_t notify_type, char *path);

//-------------------------------------------------------------------------
// Typedefs for core vendor hook callbacks

// Get the serial number of this device
// NOTE: This function returns a value which may be overridden by an entry in the USP database
typedef int (*dm_vendor_get_agent_serial_number_cb_t)(char *buf, int len);

// Get the endpoint_id of this device
// NOTE: This function returns a value which may be overridden by an entry in the USP database
typedef int (*dm_vendor_get_agent_endpoint_id_cb_t)(char *buf, int len);

// Called to signal to the vendor that the CPE should reboot
// By the time this function has been called, all communication channels to controllers will have been closed down
// This function would normally exit the USP Agent process
// However it doesn't have to, if it needs to wait until other actions running in the USP Agent process have completed
typedef int (*dm_vendor_reboot_cb_t)(void);

// Called to signal to the vendor that the CPE should reboot and perform a factory reset
typedef int (*dm_vendor_factory_reset_cb_t)(void);

// Vendor hooks associated with vendor database transactions
// NOTE: The start transaction vendor hook must only return when the start transaction has completed successfully.
//       It must never return an error. This is necessary because there are some cases where the agent cannot sensibly
//       handle success followed by failure as it would lead to data model inconsistencies.
//       e.g. when the result of one USP Set request cascades to other parts of the data model
typedef int (*dm_vendor_start_trans_cb_t)(void);
typedef int (*dm_vendor_commit_trans_cb_t)(void);
typedef int (*dm_vendor_abort_trans_cb_t)(void);

// Gets the current running software version of the firmware image
// This must match the software version of the active firmware image
typedef int (*get_active_software_version_cb_t)(char *buf, int len);


// Vendor hooks associated with certificates and controller trust
typedef int (*register_controller_trust_cb_t)(void);
typedef bool (*is_system_time_reliable_cb_t)(void);
typedef const trust_store_t *(*get_trust_store_cb_t)(int *num_trusted_certs);
typedef int (*get_agent_cert_cb_t)(agent_cert_info_t *info);


// Miscellaneous vendor hooks
typedef int (*get_hardware_version_cb_t)(char *buf, int len);
typedef int (*dm_vendor_get_mtp_username_cb_t)(int instance, char *buf, int len);
typedef int (*dm_vendor_get_mtp_password_cb_t)(int instance, char *buf, int len);
typedef void (*log_message_cb_t)(const char *buf);
typedef void (*modify_firmware_updated_cb_t)(bool *is_firmware_updated);
typedef bool (*can_mtp_connect_cb_t)(void);

#ifndef REMOVE_DEVICE_SECURITY
typedef int (*load_agent_cert_cb_t)(SSL_CTX *ctx);
#endif

//-------------------------------------------------------------------------
// Typedef for structure containing core vendor hook callbacks
// IMPORTANT:  DO NOT WRITE CODE THAT DEPENDS ON THE POSITION OF THE CALLBACK WITHIN THIS STRUCTURE !
typedef struct
{
    dm_vendor_get_agent_serial_number_cb_t  get_agent_serial_number_cb;
    dm_vendor_get_agent_endpoint_id_cb_t    get_agent_endpoint_id_cb;
    dm_vendor_reboot_cb_t                   reboot_cb;
    dm_vendor_factory_reset_cb_t            factory_reset_cb;

    // Vendor hooks associated with vendor database transactions
    dm_vendor_start_trans_cb_t              start_trans_cb;
    dm_vendor_commit_trans_cb_t             commit_trans_cb;
    dm_vendor_abort_trans_cb_t              abort_trans_cb;

    // Vendor hooks associated with certificates and controller trust
    register_controller_trust_cb_t          register_controller_trust_cb;
    is_system_time_reliable_cb_t            is_system_time_reliable_cb;
    get_trust_store_cb_t                    get_trust_store_cb;
    get_agent_cert_cb_t                     get_agent_cert_cb;

    // Miscellaneous vendor hooks
#ifndef REMOVE_DEVICE_INFO
    get_active_software_version_cb_t        get_active_software_version_cb;
    get_hardware_version_cb_t               get_hardware_version_cb;
#endif

    dm_vendor_get_mtp_username_cb_t         get_mtp_username_cb;
    dm_vendor_get_mtp_password_cb_t         get_mtp_password_cb;
#ifndef REMOVE_DEVICE_SECURITY
    load_agent_cert_cb_t                    load_agent_cert_cb;
#endif
    log_message_cb_t                        log_message_cb;
    modify_firmware_updated_cb_t            modify_firmware_updated_cb;
    can_mtp_connect_cb_t                    can_mtp_connect_cb;

} vendor_hook_cb_t;

//-------------------------------------------------------------------------
// Defines for type_flags argument of registration functions
#define DM_STRING       0x00000001      // String
#define DM_DATETIME     0x00000002      // ISO-8601 formatted time
#define DM_BOOL         0x00000004      // Boolean
#define DM_INT          0x00000008      // 32 bit signed integer (int)
#define DM_UINT         0x00000010      // 32 bit unsigned integer (unsigned int)
#define DM_ULONG        0x00000020      // 64 bit unsigned integer (unsigned long long)
#define DM_BASE64       0x00000040      // Base64 encoded binary (string)
#define DM_HEXBIN       0x00000080      // Hex encoded binary (string)
#define DM_DECIMAL      0x00000100      // 64 bit floating point number (double)
#define DM_LONG         0x00000200      // 64 bit signed integer (long long)
#define DM_VALUE_CHANGE_WILL_IGNORE  0x00000400 // Do not emit value change notifications for this parameter

//-------------------------------------------------------------------------
// Functions to register the data model
// These functions may only be called during startup (which for vendor code, means within VENDOR_Init())
int USP_REGISTER_Param_Constant(char *path, char *value, unsigned type_flags);
int USP_REGISTER_Param_SupportedList(char *path, const enum_entry_t *enums, int num_enums);
int USP_REGISTER_DBParam_ReadWrite(char *path, char *value, dm_validate_value_cb_t validator_cb, dm_notify_set_cb_t notify_set_cb, unsigned type_flags);
int USP_REGISTER_Param_NumEntries(char *path, char *table_path);
int USP_REGISTER_VendorParam_ReadOnly(char *path, dm_get_value_cb_t get_cb, unsigned type_flags);
int USP_REGISTER_VendorParam_ReadWrite(char *path, dm_get_value_cb_t get_cb, dm_set_value_cb_t set_cb, dm_notify_set_cb_t notify_set_cb, unsigned type_flags);
int USP_REGISTER_DBParam_ReadOnlyAuto(char *path, dm_get_value_cb_t get_cb, unsigned type_flags);
int USP_REGISTER_DBParam_ReadWriteAuto(char *path, dm_get_value_cb_t get_cb, dm_validate_value_cb_t validator_cb,
                                      dm_notify_set_cb_t notify_set_cb, unsigned type_flags);
int USP_REGISTER_DBParam_Alias(char *path, dm_notify_set_cb_t notify_set_cb);
int USP_REGISTER_DBParam_ReadOnly(char *path, char *value, unsigned type_flags);
int USP_REGISTER_DBParam_Secure(char *path, char *value, dm_validate_value_cb_t validator_cb, dm_notify_set_cb_t notify_set_cb);
int USP_REGISTER_DBParam_SecureWithType(char *path, char *value, dm_validate_value_cb_t validator_cb, dm_notify_set_cb_t notify_set_cb, unsigned type_flags);
int USP_REGISTER_Object(char *path, dm_validate_add_cb_t validate_add_cb, dm_add_cb_t add_cb, dm_notify_add_cb_t notify_add_cb,
                                   dm_validate_del_cb_t validate_del_cb, dm_del_cb_t del_cb, dm_notify_del_cb_t notify_del_cb);
int USP_REGISTER_Object_UniqueKey(char *path, char **params, int num_params);
int USP_REGISTER_SyncOperation(char *path, dm_sync_oper_cb_t sync_oper_cb);
int USP_REGISTER_AsyncOperation(char *path, dm_async_oper_cb_t async_oper_cb, dm_async_restart_cb_t restart_cb);
int USP_REGISTER_AsyncOperation_MaxConcurrency(char *path, int max_concurrency);
int USP_REGISTER_OperationArguments(char *path, char **input_arg_names, int num_input_arg_names, char **output_arg_names, int num_output_arg_names);
int USP_REGISTER_Event(char *path);
int USP_REGISTER_EventArguments(char *path, char **event_arg_names, int num_event_arg_names);
int USP_REGISTER_CoreVendorHooks(vendor_hook_cb_t *callbacks);

int USP_REGISTER_GroupId(char* path, int group_id);
int USP_REGISTER_GroupedObject(int group_id, char *path, bool is_writable);
int USP_REGISTER_GroupedVendorParam_ReadOnly(int group_id, char *path, unsigned type_flags);
int USP_REGISTER_GroupedVendorParam_ReadWrite(int group_id, char *path, unsigned type_flags);
int USP_REGISTER_GroupVendorHooks(int group_id, dm_get_group_cb_t get_group_cb, dm_set_group_cb_t set_group_cb,
                                                dm_add_group_cb_t add_group_cb, dm_del_group_cb_t del_group_cb);
int USP_REGISTER_SubscriptionVendorHooks(int group_id, dm_subscribe_cb_t subscribe_cb, dm_unsubscribe_cb_t unsubscribe_cb);
int USP_REGISTER_MultiDeleteVendorHook(int group_id, dm_multi_del_cb_t multi_del_cb);
int USP_REGISTER_CreateObjectVendorHook(int group_id, dm_create_obj_cb_t create_obj_cb);
int USP_REGISTER_Object_RefreshInstances(char *path, dm_refresh_instances_cb_t refresh_instances_cb);

//------------------------------------------------------------------------------
// Functions that may be called from vendor hooks to access the data model
// These functions must not be called from any thread other than the data model thread
int USP_DM_GetParameterValue(char *path, char *buf, int len);
int USP_DM_SetParameterValue(char *path, char *new_value);
int USP_DM_DeleteInstance(char *path);
int USP_DM_InformInstance(char *path);
int USP_DM_RefreshInstance(char *path);
int USP_DM_GetInstances(char *path, int *vector, int max_entries, int *num_entries);
int USP_DM_RegisterRoleName(int role_instance, char *name);
int USP_DM_AddControllerTrustPermission(int role_instance, char *path, unsigned short permission_bitmask);
int USP_DM_InformDataModelEvent(char *event_name, kv_vector_t *output_args);
bool USP_DM_IsRegistered(char *path);

//------------------------------------------------------------------------------
// Functions that may be called from a thread other than the data model thread to signal that some event occurred
int USP_SIGNAL_OperationComplete(int instance, int err_code, char *err_msg, kv_vector_t *output_args);
int USP_SIGNAL_DataModelEvent(char *event_name, kv_vector_t *output_args);
int USP_SIGNAL_OperationStatus(int instance, char *status);
int USP_SIGNAL_ObjectAdded(char *path);
int USP_SIGNAL_ObjectDeleted(char *path);

//------------------------------------------------------------------------------
// Function to perform work in the data model thread via a callback
// The callback may call any of the USP_DM_XXX set of functions
typedef void (*do_work_cb_t)(void *arg1, void *arg2);
int USP_PROCESS_DoWork(do_work_cb_t do_work_cb, void *arg1, void *arg2);

//------------------------------------------------------------------------------
// Functions for argument list data structure
#define SAVED_TIME_REF_ARG_NAME "Internal_TimeRef"
kv_vector_t * USP_ARG_Create(void);
void USP_ARG_Init(kv_vector_t *kvv);
void USP_ARG_Add(kv_vector_t *kvv, char *key, char *value);
bool USP_ARG_Replace(kv_vector_t *kvv, char *key, char *value);
bool USP_ARG_ReplaceWithHint(kv_vector_t *kvv, char *key, char *value, int hint);
void USP_ARG_AddUnsigned(kv_vector_t *kvv, char *key, unsigned value);
void USP_ARG_AddBool(kv_vector_t *kvv, char *key, bool value);
void USP_ARG_AddDateTime(kv_vector_t *kvv, char *key, time_t value);
char *USP_ARG_Get(kv_vector_t *kvv, char *key, char *default_value);
int USP_ARG_GetUnsigned(kv_vector_t *kvv, char *key, unsigned default_value, unsigned *value);
int USP_ARG_GetUnsignedWithinRange(kv_vector_t *kvv, char *key, unsigned default_value, unsigned min, unsigned max, unsigned *value);
int USP_ARG_GetInt(kv_vector_t *kvv, char *key, int default_value, int *value);
int USP_ARG_GetIntWithinRange(kv_vector_t *kvv, char *key, int default_value, int min, int max, int *value);
int USP_ARG_GetBool(kv_vector_t *kvv, char *key, bool default_value, bool *value);
int USP_ARG_GetDateTime(kv_vector_t *kvv, char *key, char *default_value, time_t *value);
void USP_ARG_Destroy(kv_vector_t *kvv);
void USP_ARG_Delete(kv_vector_t *kvv);

//------------------------------------------------------------------------------
// Functions for string vector data structure
void USP_STR_VEC_Init(str_vector_t *sv);
void USP_STR_VEC_Add(str_vector_t *sv, char *str);
void USP_STR_VEC_Destroy(str_vector_t *sv);

//------------------------------------------------------------------------------
// Functions converting data types
#define INVALID_TIME ((time_t)-1)
#define MAX_ISO8601_LEN  32   // 23 characters should be sufficient, so this is overkill
time_t USP_CONVERT_DateTimeToUnixTime(char *date);
char *USP_CONVERT_UnixTimeToDateTime(time_t unix_time, char *buf, int len);

//------------------------------------------------------------------------------
// Functions setting error messages in the data model thread
void USP_ERR_SetMessage(char *fmt, ...)   __attribute__((format(printf, 1, 2)));

//------------------------------------------------------------------------------
// Functions for debug logging
void USP_LOG_Printf(log_level_t log_level, log_type_t log_type, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void USP_LOG_String(log_level_t log_level, log_type_t log_type, char *str);
void USP_LOG_Puts(log_level_t log_level, log_type_t log_type, const char *str);
void USP_LOG_HexBuffer(const char *title, const unsigned char *buf, int len);
void USP_LOG_Callstack(void);

//------------------------------------------------------------------------------------
// Macros used for normal debug logging
extern log_level_t usp_log_level;

#define USP_LOG_Error(...)       if (usp_log_level >= kLogLevel_Error)   { USP_LOG_Printf(kLogLevel_Error, kLogType_Debug, __VA_ARGS__); if (enable_callstack_debug) { USP_LOG_Callstack(); } }
#define USP_LOG_Warning(...)     if (usp_log_level >= kLogLevel_Warning) { USP_LOG_Printf(kLogLevel_Warning, kLogType_Debug, __VA_ARGS__); }
#define USP_LOG_Info(...)        if (usp_log_level >= kLogLevel_Info)    { USP_LOG_Printf(kLogLevel_Info, kLogType_Debug, __VA_ARGS__); }
#define USP_LOG_Debug(...)       if (usp_log_level >= kLogLevel_Debug)   { USP_LOG_Printf(kLogLevel_Debug, kLogType_Debug, __VA_ARGS__); }
#define USP_LOG_Trace(...)       if (usp_log_level >= kLogLevel_Trace)   { USP_LOG_Printf(kLogLevel_Trace, kLogType_Debug, __VA_ARGS__); }

//-------------------------------------------------------------------------
// Functions used when registering validate_add and validate_delete vendor hooks, if the multi-instance object is read only
int USP_HOOK_DenyAddInstance(dm_req_t *req);
int USP_HOOK_DenyDeleteInstance(dm_req_t *req);

/*********************************************************************//**
**
** Callback function definition to be called when the USP Service receives a USP notification
**
** \param   subscription_id - instance ID of the subscription object generating the notification
**
** \param   path - One of the following depending on the type of notification:-
**                  For a changed value, the datamodel path of the parameter that has changed
**                  For add/delete, the path containing the instance of the object
**                  For an event, the datamodel path including the event name
**                  For an async operation, the path and name of the operation that has completed
**
** \param   args - One of the following depending on the type of notification:-
**                  For a changed value, the parameter path and new value
**                  For add/delete, the unique keys/values of the affected object
**                  For an event, the parameter keys/values pertaining to the event
**                  For an async operation, the output arguments of the command
**
** \param   cmd_key - (Operate only) The command key string returned from USP_SERVICE_Operate()
** \param   err_code - (Operate only) An error code indicating success or an error condition
** \param   err_msg - (Operate only) Human readable string indicating the cause of the error
**
** \return  void
**
**************************************************************************/
typedef void (*usp_service_notify_cb_t)(char *subscription_id, char *path, kv_vector_t *args, char *cmd_key, int err_code, char *err_msg);

//------------------------------------------------------------------------------
// Functions to query the whole device's data model, when running as a USP Service (and acting as a USP Controller)
int USP_SERVICE_Get(kv_vector_t *params, int timeout, char *err_msg, int err_msg_len);
int USP_SERVICE_Set(kv_vector_t *params, int timeout, char *err_msg, int err_msg_len);
int USP_SERVICE_Add(char *path, kv_vector_t *params, int timeout, int *instance, char *err_msg, int err_msg_len);
int USP_SERVICE_Delete(char *obj_path, int timeout, char *err_msg, int err_msg_len);
int USP_SERVICE_GetSupportedDM(char *obj_path, int timeout, kv_vector_t *paths, char *err_msg, int err_msg_len);
int USP_SERVICE_GetInstances(char *path, int timeout, str_vector_t *instances, char *err_msg, int err_msg_len);
int USP_SERVICE_Operate(char *cmd_path, kv_vector_t *args, int timeout, char **cmd_key, char *err_msg, int err_msg_len);
int USP_SERVICE_RegisterNotificationCallback(usp_service_notify_cb_t usp_service_notify_cb);



#endif
