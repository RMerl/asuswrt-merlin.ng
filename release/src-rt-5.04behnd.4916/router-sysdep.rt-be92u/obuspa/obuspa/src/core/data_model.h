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
 * \file data_model.h
 *
 * Header file containing the API for the USP Data Model
 *
 */
#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "usp_api.h"
#include "dllist.h"
#include "str_vector.h"
#include "int_vector.h"
#include "sync_timer.h"
#include "subs_vector.h"
#include "device.h"
#include "group_set_vector.h"
#include "vendor_defs.h"  // For MAX_DM_INSTANCE_ORDER

//-----------------------------------------------------------------------------------------
// Type of each data model node
// NOTE: If extra nodes are added, please ensure that you update dm_node_type_to_str[]
typedef enum
{
    kDMNodeType_Object_MultiInstance,
    kDMNodeType_Object_SingleInstance,
    kDMNodeType_Param_ConstantValue,                // Read Only
    kDMNodeType_Param_NumEntries,                   // Read Only
    kDMNodeType_DBParam_ReadWrite,
    kDMNodeType_DBParam_ReadOnly,
    kDMNodeType_DBParam_ReadOnlyAuto,
    kDMNodeType_DBParam_ReadWriteAuto,
    kDMNodeType_DBParam_Secure,                     // Read Write
    kDMNodeType_VendorParam_ReadOnly,
    kDMNodeType_VendorParam_ReadWrite,
    kDMNodeType_SyncOperation,
    kDMNodeType_AsyncOperation,
    kDMNodeType_Event,

    // Always last enumeration, used to size arrays based on this enumeration
    kDMNodeType_Max
} dm_node_type_t;

//-----------------------------------------------------------------------------------------
// Structures associated with unique key addressing
// Structure representing a compound unique key. Points to the name of each parameter in the compound unique key.
// If not all entries in the array are needed (eg unique key is a single parameter), then the rest of the entries are NULL.
typedef struct
{
    char *param[MAX_COMPOUND_KEY_PARAMS];
} dm_unique_key_t;


// Vector of (potentially compound) unique keys
typedef struct
{
    dm_unique_key_t *vector;
    int num_entries;
} dm_unique_key_vector_t;

//--------------------------------------------------------------------
// Typedef for structure containing the instance numbers parsed from a data model path, and the data model nodes associated with each instance number
// e.g. 'Device.Wifi.1.Interface.2.Enable' would have count=2 and
// instances[0]=1, nodes[0]= ptr to 'Device.Wifi'
// instances[1]=2, nodes[1]= ptr to 'Device.Wifi.{i}.Interface'
typedef struct
{
    // NOTE: Do not change the order of variables in this structure. They must match dm_req_instances_t
    int order;    // Number of instance numbers in this array, and hence number of instance separators in the path
    int instances[MAX_DM_INSTANCE_ORDER];
    struct dm_node_tag *nodes[MAX_DM_INSTANCE_ORDER];
} dm_instances_t;

//-----------------------------------------------------------------------------------------
// Typedef for structure containing all object instances for a top level multi-instance node and its children
typedef struct
{
    dm_instances_t *vector;
    int num_entries;
} dm_instances_vector_t;

//-----------------------------------------------------------------------------------------
// Information registered in the data model for parameters
// NOTE: Not all of these parameters are relevant for each type of node. See USP_REGISTER_XXX() functions.
#define NON_GROUPED  (-1)       // Indicates that the parameter is not grouped with any other parameters during get/set

typedef struct
{
    char *default_value;
    dm_validate_value_cb_t validator_cb;
    dm_notify_set_cb_t notify_set_cb;
    dm_get_value_cb_t get_cb;
    dm_set_value_cb_t set_cb;
    unsigned type_flags;                  // type of the parameter
    struct dm_node_tag *table_node;       // database node representing the table which we need to get the number of entries in (for kDMNodeType_Param_NumEntries)
} dm_param_info_t;

// Information registered in the data model for objects
typedef struct
{
    dm_validate_add_cb_t validate_add_cb;
    dm_add_cb_t          add_cb;
    dm_notify_add_cb_t   notify_add_cb;
    dm_validate_del_cb_t validate_del_cb;
    dm_del_cb_t          del_cb;
    dm_notify_del_cb_t   notify_del_cb;
    dm_unique_key_vector_t unique_keys;

    bool group_writable;            // Set if this object can have instances added/deleted by a controller. Only used by grouped objects

    // The following are only used by top-level multi-instance objects
    dm_instances_vector_t inst_vector;          // vector of instances for this multi-instance object and all its children
    dm_refresh_instances_cb_t refresh_instances_cb; // (optional) callback to get the instances of this object and its children
    time_t refresh_instances_expiry_time;       // Absolute time at which the instances in the inst_vector are valid until. NOTE: Only used if refresh_instances_cb is non-NULL.
                                                // After this time, if the USP Agent needs to access the top-level multi-instance object or any of its children, then the callback will be invoked again. Unless locked (see usp_req_lock_count)
    unsigned int lock_period;       // Specifies the USP request during which the instances in the inst_vector are locked. This is used to prevent expiry during the processing of a USP request - which causes instance mismatch and errors
} dm_object_info_t;

// Information registered in the data model for operations
typedef struct
{
    dm_sync_oper_cb_t sync_oper_cb;
    dm_async_oper_cb_t async_oper_cb;
    dm_async_restart_cb_t restart_cb;   // Called only for async operations, to determine whether to restart them after a power-cycle
    int max_concurrency;                // Maximum number of concurrent invocations of this command
    str_vector_t input_args;            // String vector containing the names of all valid input arguments (in schema form)
    str_vector_t output_args;           // String vector containing the names of all valid output arguments (in schema form)
} dm_oper_info_t;

// Information registered in the data model for events
typedef struct
{
    str_vector_t event_args;            // String vector containing the names of all valid arguments carried in the event (in schema form)
} dm_event_info_t;

// Operations flags variable definitions
#define RESTART_ON_REBOOT 0x00000001        // Flag to signal that the operation must be restarted if the CPE reboots before it completes
                                            // If not set, a reboot will cause the operation complete to be sent (if subscribed to)

//-----------------------------------------------------------------------------------------
// Typedef for hash of schema path to data model parameter or object
typedef unsigned dm_hash_t;
typedef      int db_hash_t;     // This is different from dm_hash_t for historical reasons, to maintain backward compatability of databases

//-----------------------------------------------------------------------------------------
// Structure describing each data model node
typedef struct dm_node_tag
{
    double_link_t link;         // Link to siblings in the data model tree. This is a link in the linked list of the parent node's child_nodes linked list
    struct dm_node_tag *next_node_map_link;  // pointer to next node having the same squashed hash in dm_node_map[]
    char *path;                 // Schema path for this node. Used for debug, passed to the vendor hooks and with GetSupportedDM and Internal Services passthru

    char *name;                 // Part of the path that this node implements (name of path segment)
    dm_node_type_t type;
    double_linked_list_t child_nodes;   // Head and tail of the linked list containing child nodes. To traverse that list use the link to siblings
    struct dm_node_tag *parent_node;    // Pointer to parent of this node

    dm_hash_t hash;             // Contains hash of the data model schema path to this node. If this node is a multi-instance object, then schema path includes trailing '{i}'

    int order;                   // Number of instance separators in the path to this node
                                 // e.g. Device.Wifi.{i}.Interface.{i}.Enable would have an order of 2
                                 // And would contain pointers to the 2 nodes 'Device.Wifi.{i}' and
                                 // 'Device.Wifi.{i}.Interface.{i}' in the instance_nodes[] array
                                 // For nodes which are objects, if the node is a multi-instance object, then
                                 // it's instance separator is included e.g. Device.Wifi.{i}.Interface.{i} would have an order of 2
    struct dm_node_tag *instance_nodes[MAX_DM_INSTANCE_ORDER]; // See 'order' above

    unsigned short permissions[MAX_CTRUST_ROLES];    // Bitmask of permissions for each role

    int group_id;                   // Indicates the group_id of the software component implementing this object, or NON_GROUPED

    union
    {
        dm_param_info_t  param_info;                    // Parameters
        dm_object_info_t object_info;                   // Objects
                                                        // NOTE: kDMNodeType_Object_SingleInstance have no entry in this union
        dm_oper_info_t   oper_info;                     // Operations
        dm_event_info_t  event_info;                    // Events
    } registered;
} dm_node_t;

//------------------------------------------------------------------------------
// Structure containing the vendor hook callbacks
extern vendor_hook_cb_t vendor_hook_callbacks;

//------------------------------------------------------------------------------
// Array containing the get/set vendor hooks for each group of vendor parameters
typedef struct
{
    dm_get_group_cb_t get_group_cb;
    dm_set_group_cb_t set_group_cb;
    dm_add_group_cb_t add_group_cb;
    dm_del_group_cb_t del_group_cb;
    dm_subscribe_cb_t subscribe_cb;
    dm_unsubscribe_cb_t unsubscribe_cb;
    dm_create_obj_cb_t  create_obj_cb;
    dm_multi_del_cb_t   multi_del_cb;
} group_vendor_hook_t;

extern group_vendor_hook_t group_vendor_hooks[MAX_VENDOR_PARAM_GROUPS];

//------------------------------------------------------------------------------
// Boolean that allows us to control which scope the USP_REGISTER_XXX() functions can be called in
extern bool is_executing_within_dm_init;

//------------------------------------------------------------------------------
// Data model path to parameter recording the cause of the last reset (Internal.Reboot.Cause)
extern char *reboot_cause_path;

//------------------------------------------------------------------------------
// Convenience variables to prevent the proliferation of the string 'Device.' everywhere
extern char *dm_root;
extern int dm_root_len;

//------------------------------------------------------------------------------
// Defines for bits in flag variable returned by DATA_MODEL_GetPathProperties()
#define PP_EXISTS_IN_SCHEMA               0x00000001   // Object/Parameter exists in the schema
#define PP_IS_OBJECT                      0x00000002   // Path represents an object (eg single or multi-instance, qualified or unqualified)
#define PP_IS_PARAMETER                   0x00000004   // Path represents a parameter
#define PP_IS_OPERATION                   0x00000008   // Path represents an operation
#define PP_IS_EVENT                       0x00000010   // Path represents an event
#define PP_IS_OBJECT_INSTANCE             0x00000020   // Path represents an object which is either a single instance object, or a qualified multi-instance object
#define PP_INSTANCE_NUMBERS_EXIST         0x00000040   // Instance numbers given in the path exist. NOTE: Multi-instance objects specified without trailing instance numbers will check the parent instance numnbers in the path only
#define PP_PARENT_INSTANCE_NUMBERS_EXIST  0x00000080   // If the path represents a multi-instance object with a trailing instance number that does not exist, then if this bit is set, the parent instance numbers are instantiated in the model
#define PP_IS_MULTI_INSTANCE_OBJECT       0x00000100   // Set if the path represents a multi-instance object
#define PP_IS_SECURE_PARAM                0x00000200   // Set if the path represents a secure parameter
#define PP_IS_WRITABLE                    0x00000400   // Set if the path represents a writable parameter
#define PP_VALUE_CHANGE_WILL_IGNORE       0x00000800   // Set if the path represents a parameter that should be ignored by value change subscriptions

//------------------------------------------------------------------------------
// Convenience macros
#define IS_OBJECT(type)  ((type == kDMNodeType_Object_MultiInstance) || (type == kDMNodeType_Object_SingleInstance))
#define IsObject(node)  IS_OBJECT(node->type)

#define IsParam(node)   ((node->type != kDMNodeType_Object_MultiInstance) && \
                         (node->type != kDMNodeType_Object_SingleInstance) && \
                         (node->type != kDMNodeType_SyncOperation) && \
                         (node->type != kDMNodeType_AsyncOperation) && \
                         (node->type != kDMNodeType_Event))

#define IsDbParam(node)  ((node->type == kDMNodeType_DBParam_ReadWrite) || \
                          (node->type == kDMNodeType_DBParam_ReadOnly) || \
                          (node->type == kDMNodeType_DBParam_ReadOnlyAuto) || \
                          (node->type == kDMNodeType_DBParam_ReadWriteAuto) || \
                          (node->type == kDMNodeType_DBParam_Secure))

#define IsVendorParam(node)  ((node->type == kDMNodeType_VendorParam_ReadOnly) || \
                              (node->type == kDMNodeType_VendorParam_ReadWrite))

#define IsOperation(node)  ((node->type == kDMNodeType_SyncOperation) || (node->type == kDMNodeType_AsyncOperation))

#define IsOperationEvent(node)  ((node->type == kDMNodeType_SyncOperation) || (node->type == kDMNodeType_AsyncOperation) || (node->type == kDMNodeType_Event))

//------------------------------------------------------------------------------
// Definitions for flags in DATA_MODEL_GetParameterValue()
#define SHOW_PASSWORD 0x00000001        // Used internally by USP Agent to get the actual value of passwords (default behaviour is to return an empty string)
#define DONT_LOG_NO_INSTANCE_ERROR  0x00000002  // Suppresses logging of the no instance error. The error code is still returned, just not logged

//------------------------------------------------------------------------------
// Definitions for flags in DATA_MODEL_SetParameterValue()
#define CHECK_WRITABLE 0x00000001   // Prevents read only parameters being written by a controller

//------------------------------------------------------------------------------
// Definitions for flags in DATA_MODEL_AddInstance()
#define CHECK_CREATABLE        0x00000001   // Prevents read only instances being created by a controller
#define IGNORE_INSTANCE_EXISTS 0x00000002   // Does not generate an error if the specified instance already exists

//------------------------------------------------------------------------------
// Definitions for flags in DATA_MODEL_DeleteInstance()
#define CHECK_DELETABLE    0x00000001   // Prevents read only instances being deleted by a controller
#define IGNORE_NO_INSTANCE 0x00000002   // Does not generate an error if the instance has already been deleted

//------------------------------------------------------------------------------
// Definitions for flags in DM_PRIV_AddSchemaPath()
#define SUPPRESS_PRE_EXISTANCE_ERR  0x00000001 // Does not generate an error if the schema path already exists.
                                               // Using this flag allows the caller to check that the node exists
#define SUPPRESS_LAST_TYPE_CHECK    0x00000002 // Do not check the type of the last (ie rightmost) node in the path
                                               // This is used when we know that the node has already been added, but it could be more than one type.
                                               // When this is used, the caller must check the type of the last node
#define OVERRIDE_LAST_TYPE          0x00000004 // Ensures that the type of the last (ie rightmost) node in the path matches that specified
                                               // overriding (and ignoring) any previous registered type
                                               // This is used to override the type of a node registered after receiving a USP Register message
                                               // (at which point we don't know if it is a single or multi-instance object) with the correct
                                               // type received in the GSDM response
//-----------------------------------------------------------------------------
// Definitions for flags in DM_PRIV_GetNodeFromPath() and DM_PRIV_CalcHashFromPath()
#define DONT_LOG_ERRORS         0x00000001  // Suppresses logging of errors when calling the function - because errors may be expected

//-----------------------------------------------------------------------------
// API Functions
int DATA_MODEL_Init(void);
int DATA_MODEL_Start(void);
void DATA_MODEL_Stop(void);
int DATA_MODEL_GetNumInstances(char *path, int *num_instances);
int DATA_MODEL_GetInstances(char *path, int_vector_t *iv);
int DATA_MODEL_GetInstancePaths(char *path, str_vector_t *sv, combined_role_t *combined_role);
int DATA_MODEL_GetAllInstancePaths(char *path, str_vector_t *sv, combined_role_t *combined_role);
int DATA_MODEL_AddInstance(char *path, int *instance, unsigned flags);
int DATA_MODEL_DeleteInstance(char *path, unsigned flags);
int DATA_MODEL_GetPermissions(char *path, combined_role_t *combined_role, unsigned short *perm, unsigned flags);
int DATA_MODEL_NotifyInstanceAdded(char *path);
int DATA_MODEL_NotifyInstanceDeleted(char *path);
int DATA_MODEL_GetParameterValue(char *path, char *buf, int len, unsigned flags);
int DATA_MODEL_SetParameterValue(char *path, char *new_value, unsigned flags);
int DATA_MODEL_Operate(char *path, kv_vector_t *input_args, kv_vector_t *output_args, char *command_key, int *instance);
int DATA_MODEL_ShouldOperationRestart(char *path, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args);
int DATA_MODEL_RestartAsyncOperation(char *path, kv_vector_t *input_args, int instance);
unsigned DATA_MODEL_GetPathProperties(char *path, combined_role_t *combined_role, unsigned short *permission_bitmask, int *group_id, unsigned *type_flags, unsigned exec_flags);
int DATA_MODEL_SplitPath(char *path, char **schema_path, dm_req_instances_t *instances, bool *instances_exist);
int DATA_MODEL_InformInstance(char *path);
int DATA_MODEL_AddParameterInstances(dm_hash_t hash, char *instances);
int DATA_MODEL_GetUniqueKeys(char *path, dm_unique_key_vector_t *ukv);
int DATA_MODEL_GetUniqueKeyParams(char *obj_path, kv_vector_t *params, combined_role_t *combined_role);
int DATA_MODEL_ValidateDefaultedUniqueKeys(char *obj_path, kv_vector_t *unique_key_params, group_set_vector_t *gsv);
void DATA_MODEL_DumpSchema(void);
void DATA_MODEL_DumpInstances(void);
char DATA_MODEL_GetJSONParameterType(char *path);
int DATA_MODEL_SetParameterInDatabase(char *path, char *value);
int DATA_MODEL_FindUnusedGroupId(void);
int DATA_MODEL_DeRegisterPath(char *schema_path);

int DM_PRIV_InitSetRequest(dm_req_t *req, dm_node_t *node, char *path, dm_instances_t *inst, char *new_value);
void DM_PRIV_RequestInit(dm_req_t *req, dm_node_t *node, char *path, dm_instances_t *inst);
int DM_PRIV_ParseInstanceString(char *instances, dm_instances_t *inst);
int DM_PRIV_FormInstantiatedPath(char *schema_path, dm_instances_t *inst, char *buf, int len);
dm_node_t *DM_PRIV_AddSchemaPath(char *path, dm_node_type_t type, unsigned flags);
int DM_PRIV_FormDB_FromPath(char *path, dm_hash_t *hash, char *instances, int len);
int DM_PRIV_FormPath_FromDB(dm_hash_t hash, char *instances, char *buf, int len);
int DM_PRIV_CalcHashFromPath(char *path, dm_instances_t *inst, dm_hash_t *p_hash, unsigned flags);
dm_node_t *DM_PRIV_GetNodeFromPath(char *path, dm_instances_t *inst, bool *is_qualified_instance, unsigned flags);
dm_node_t *DM_PRIV_FindMatchingChild(dm_node_t *parent, char *name);
void DM_PRIV_AddUniqueKey(dm_node_t *node, dm_unique_key_t *unique_key);
void DM_PRIV_ApplyPermissions(dm_node_t *node, int role_index, unsigned short permission_bitmask);
unsigned short DM_PRIV_GetPermissions(dm_node_t *node, combined_role_t *combined_role);
int DM_PRIV_ReRegister_DBParam_Default(char *path, char *value);
bool DM_PRIV_IsChildOf(char *path, dm_node_t *parent_node);
bool DM_PRIV_IsChildNodeOf(dm_node_t *node, dm_node_t *parent_node);
bool DM_PRIV_AreAllChildrenGroupId(dm_node_t *parent, int group_id);

#endif

