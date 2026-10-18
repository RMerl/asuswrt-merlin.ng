/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020,  BT PLC
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
 * \file data_model.c
 *
 * Implements the API to the USP data model
 *
 */

#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "device.h"
#include "database.h"
#include "int_vector.h"
#include "dm_inst_vector.h"
#include "dm_trans.h"
#include "dm_access.h"
#include "cli.h"
#include "vendor_api.h"
#include "text_utils.h"
#include "iso8601.h"
#include "group_get_vector.h"
#include "plugin.h"

#ifdef ENABLE_COAP
#include "usp_coap.h"
#endif

#ifdef ENABLE_WEBSOCKETS
#include "wsclient.h"
#include "wsserver.h"
#endif

#ifdef ENABLE_UDS
#include "uds.h"
#endif

#ifndef REMOVE_USP_BROKER
#include "usp_broker.h"
#endif

#ifndef REMOVE_USP_SERVICE
#include "usp_service.h"
#endif

//--------------------------------------------------------------------
// Boolean that allows us to control which scope the USP_REGISTER_XXX() functions can be called in
bool is_executing_within_dm_init = false;

//--------------------------------------------------------------------
// Segment of a data model path e.g. "Device" or "LocalAgent"
typedef struct
{
    char *name;
    dm_node_type_t type;
} dm_path_segment;

//--------------------------------------------------------------------
// Array to convert from enumeration to string
char *dm_node_type_to_str[kDMNodeType_Max] =
{
    "MultiInstanceObject",         // kDMNodeType_Object_MultiInstance
    "SingleInstanceObject",        // kDMNodeType_Object_SingleInstance
    "ConstantValueParameter",      // kDMNodeType_Param_ConstantValue
    "NumEntriesParameter",         // kDMNodeType_Param_NumEntries
    "ReadWriteDBParameter",        // kDMNodeType_DBParam_ReadWrite
    "ReadOnlyDBParameter",         // kDMNodeType_DBParam_ReadOnly
    "ReadOnlyAutoDBParameter",     // kDMNodeType_DBParam_ReadOnlyAuto
    "ReadWriteAutoDBParameter",    // kDMNodeType_DBParam_ReadWriteAuto
    "SecureDBParameter",           // kDMNodeType_DBParam_Secure
    "ReadOnlyVendorParameter",     // kDMNodeType_VendorParam_ReadOnly
    "ReadWriteVendorParameter",    // kDMNodeType_VendorParam_ReadWrite
    "SynchronousOperation",        // kDMNodeType_SyncOperation
    "AsynchronousOperation",       // kDMNodeType_AsyncOperation
    "Event"                        // kDMNodeType_Event
};

//--------------------------------------------------------------------
// Separator used in schema path to denote the position of instance numbers in the path
#define MULTI_SEPARATOR "{i}"

//--------------------------------------------------------------------
// Pointer to the root data model nodes
static dm_node_t *root_device_node;
static dm_node_t *root_internal_node;

//--------------------------------------------------------------------
// Map containing all data model nodes, indexed by squashed hash value
dm_node_t *dm_node_map[MAX_NODE_MAP_BUCKETS] = { 0 };

//------------------------------------------------------------------------------
// Convenience variables to prevent the proliferation of the string 'Device.' everywhere
#define DM_ROOT "Device."
char *dm_root = DM_ROOT;
int dm_root_len = sizeof(DM_ROOT)-1;

//--------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void SerializeNativeValue(dm_req_t *req, dm_node_t *node, char *buf, int len);
void FormInstanceString(dm_instances_t *inst, char *buf, int len);
dm_node_t *CreateNode(char *name, dm_node_type_t type, char *schema_path);
int LinkToNodeMap(dm_node_t *node, char *schema_path);
void UnlinkFromNodeMap(dm_node_t *node);
int ParseSchemaPath(char *path, char *path_segments, int path_segment_len, dm_node_type_t type, dm_path_segment *segments, int max_segments);
dm_node_t *FindNodeFromHash(dm_hash_t hash);
char *ParseInstanceInteger(char *p, int *p_value);
int AddChildParamsDefaultValues(char *path, int path_len, dm_node_t *node, dm_instances_t *inst);
int DeleteChildParams(char *path, int path_len, dm_node_t *node, dm_instances_t *inst);
int DeleteChildParams_MultiInstanceObject(char *path, int path_len, dm_node_t *node, dm_instances_t *inst);
int strncpy_path_segments(char *dst, char *src, int maxlen);
void DumpSchemaFromRoot(dm_node_t *root, char *name);
void AddChildNodes(dm_node_t *parent, str_vector_t *sv);
void AddChildArgs(str_vector_t *sv, char *path, str_vector_t *args, char *arg_type);
int SortSchemaPath(const void *p1, const void *p2);
int RegisterDefaultControllerTrust(void);
void DestroySchemaRecursive(dm_node_t *node);
void DestroyNode(dm_node_t *node);
void DestroyInstanceVectorRecursive(dm_node_t *parent);
void DumpInstanceVectorRecursive(dm_node_t *parent);
int GetAllInstancePathsRecursive(dm_node_t *node, dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role);
void DumpDataModelNodeMap(void);
int GetVendorParam(dm_node_t *node, char *path, dm_instances_t *inst, char *buf, int len, dm_req_t *req);
int SetVendorParam(dm_node_t *node, char *path, dm_instances_t *inst, char *value, dm_req_t *req);
double_link_t *FindLinkToFirstObject(double_linked_list_t *list);
int OverrideNodeType(dm_node_t *node, dm_node_type_t type, char *schema_path, dm_instances_t *inst);

/*********************************************************************//**
**
** DATA_MODEL_Init
**
** Initialise the data model and register all nodes in the data model schema
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_Init(void)
{
    int err;

    // Allocate the root nodes for the data model
    #define DEVICE_NODE_NAME "Device"
    root_device_node = CreateNode(DEVICE_NODE_NAME, kDMNodeType_Object_SingleInstance, DEVICE_NODE_NAME);

    #define INTERNAL_NODE_NAME "Internal"
    root_internal_node = CreateNode(INTERNAL_NODE_NAME, kDMNodeType_Object_SingleInstance, INTERNAL_NODE_NAME);

#ifdef ENABLE_COAP
    // Initialise CoAP protocol layer
    COAP_Init();
#endif

#ifdef ENABLE_WEBSOCKETS
    // Initialise WebSockets protocol layer
    WSCLIENT_Init();
    WSSERVER_Init();
#endif

    // Register core implemented nodes in the schema
    is_executing_within_dm_init = true;
    err = USP_ERR_OK;
    err |= DEVICE_LOCAL_AGENT_Init();
#ifndef REMOVE_DEVICE_SECURITY
    err |= DEVICE_SECURITY_Init();
#endif

#ifndef REMOVE_DEVICE_TIME
    err |= DEVICE_TIME_Init();
#endif
    err |= DEVICE_CONTROLLER_Init();
    err |= DEVICE_MTP_Init();

#ifndef DISABLE_STOMP
    err |= DEVICE_STOMP_Init();
#endif

#ifdef ENABLE_UDS
    err |= DEVICE_UDS_Init();
#endif

#ifdef ENABLE_MQTT
    err |= DEVICE_MQTT_Init();
#endif
    err |= DEVICE_SUBSCRIPTION_Init();
    err |= DEVICE_CTRUST_Init();
    err |= DEVICE_REQUEST_Init();
#ifndef REMOVE_DEVICE_BULKDATA
    err |= DEVICE_BULKDATA_Init();
#endif




#ifndef REMOVE_SELF_TEST_DIAG_EXAMPLE
    // Register data model parameters used by the Self Test Diagnostics example code
    err |= DEVICE_SELF_TEST_Init();
#endif


#ifndef REMOVE_USP_BROKER
    err |= USP_BROKER_Init();
#endif

    // Exit if an error has occurred
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Register vendor nodes in the schema
    err = VENDOR_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Register vendor nodes from all plugins in the schema
    err = PLUGIN_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to potentially perform a programmatic factory reset of the parameters in the database
    // NOTE: This must be performed before DEVICE_LOCAL_AGENT_SetDefaults(), but after VENDOR_Init()
    err = DATABASE_Start();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Set the default values of OUI, Serial Number and (LocalAgent) EndpointID, and cache EndpointID
    err = DEVICE_LOCAL_AGENT_SetDefaults();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    is_executing_within_dm_init = false;

    // Exit if unable to register all nodes in the schema
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then all of the data model components initialised successfully
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_Start
**
** Instantiate all instances in the data model
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_Start(void)
{
    int err;
    dm_trans_vector_t trans;
    register_controller_trust_cb_t   register_controller_trust_cb;

    // Determine function to call to register controller trust and data model permissions
    register_controller_trust_cb = vendor_hook_callbacks.register_controller_trust_cb;
    if (register_controller_trust_cb == NULL)
    {
        register_controller_trust_cb = RegisterDefaultControllerTrust;
    }

    // Set all roles and permissions
    // NOTE: This must be done before any transaction is started otherwise object deletion notifications are not sent
    // (because we are unable to generate the list of objects in a deletion subscription because of lack of permissions)
    err = register_controller_trust_cb();
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: register_controller_trust_cb() failed", __FUNCTION__);
        return err;
    }

    // Seed data model with instance numbers from the database
    // NOTE: This is called after the register controller trust vendor hook has been called, as it may seed the DB with roles and permissions
    if (is_running_cli_local_command == false)
    {
        err = DATABASE_ReadDataModelInstanceNumbers(false);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    // As most start routines also clean the database, start a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to start all nodes in the schema (that require a separate start)
    // Typically these functions seed the data model with instance numbers or require the
    // data model to be running to access database parameters (seeded from the database - above)
    err = USP_ERR_OK;
    err |= DEVICE_LOCAL_AGENT_Start();
#ifndef REMOVE_DEVICE_TIME
    err |= DEVICE_TIME_Start();
#endif
    err |= DEVICE_CTRUST_Start();
    err |= DEVICE_CONTROLLER_Start();      // NOTE: This must come after DEVICE_CTRUST_Start() because it determines the role of the controller

    // Load trust store and client certs into USP Agent's cache
    // NOTE: This call does not leave any dynamic allocations owned by SSL (which is necessary, since libwebsockets is going to re-initialise SSL)
#ifndef REMOVE_DEVICE_SECURITY
    err |= DEVICE_SECURITY_Start();
#endif

#ifdef ENABLE_WEBSOCKETS
    // IMPORTANT: libwebsockets re-initialises libssl here, then loads the trust store and client cert from USP Agent's cache
    err |= WSCLIENT_Start();
    err |= WSSERVER_Start();
#endif

#ifndef DISABLE_STOMP
    err |= DEVICE_STOMP_Start();          // NOTE: This must come after DEVICE_SECURITY_Start(), as it assumes the trust store and client certs have been locally cached
#endif

#ifdef ENABLE_UDS
    err |= DEVICE_UDS_Start();
#endif

#ifdef ENABLE_COAP
    err |= COAP_Start();                  // NOTE: This must come after DEVICE_SECURITY_Start(), as it assumes the trust store and client certs have been locally cached
#endif

#ifdef ENABLE_MQTT
    err |= DEVICE_MQTT_Start();
#endif
    err |= DEVICE_MTP_Start();            // NOTE: This must come after COAP_Start, as it assumes that the CoAP SSL contexts have been created
    err |= DEVICE_SUBSCRIPTION_Start();   // NOTE: This must come after DEVICE_LOCAL_AGENT_Start(), as it calls DEVICE_LOCAL_AGENT_GetRebootInfo()
#ifndef REMOVE_DEVICE_BULKDATA
    err |= DEVICE_BULKDATA_Start();
#endif





    // Always start the vendor last
    err |= VENDOR_Start();
    err |= PLUGIN_Start();

    // Refresh all objects which use the refresh instances vendor hook
    // This provides the baseline after which object/additions deletions are notified (if relevant subscriptions exist)
    DM_INST_VECTOR_RefreshBaselineInstances(root_device_node);

    // Ensure that if the Boot! event is generated a long time after startup, that it refreshes the instance numbers if they have expired
    DM_INST_VECTOR_NextLockPeriod();

    // Commit all database changes
    if (err == USP_ERR_OK)
    {
        err = DM_TRANS_Commit();
    }
    else
    {
        DM_TRANS_Abort(); // Ignore error from this - we want to return the error from the body of this function instead
    }

    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_Stop
**
** Frees all memory used by the data model
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATA_MODEL_Stop(void)
{
    VENDOR_Stop();
    PLUGIN_Stop();

    DEVICE_SUBSCRIPTION_Stop();
    DEVICE_CONTROLLER_Stop();
    DEVICE_MTP_Stop();
#ifndef DISABLE_STOMP
    DEVICE_STOMP_Stop();
#endif
#ifdef ENABLE_MQTT
    DEVICE_MQTT_Stop();
#endif
#ifdef ENABLE_UDS
    DEVICE_UDS_Stop();
#endif
#ifndef REMOVE_DEVICE_BULKDATA
    DEVICE_BULKDATA_Stop();
#endif
    DEVICE_CTRUST_Stop();
#ifndef REMOVE_DEVICE_SECURITY
    DEVICE_SECURITY_Stop();
#endif
    DEVICE_LOCAL_AGENT_Stop();

#ifndef REMOVE_USP_BROKER
    USP_BROKER_Stop();
#endif

#ifndef REMOVE_USP_SERVICE
    USP_SERVICE_Stop();
#endif

    // Free the instance vectors here, so that they are not reported as a memory leak
    // NOTE: Whilst this is also done in DestroySchemaRecursive(), we do it earlier here as this memory is logged with checking turned on, whilst the schema itself is not
    DestroyInstanceVectorRecursive(root_device_node);
    DestroyInstanceVectorRecursive(root_internal_node);

    // Stop all checking of memory allocations
    // This is necessary because the data model schema was allocated before memory checking was turned on.
    // And memory check will generate an error if it doesn't know about the memory block being freed
    USP_MEM_StopCollection();

    // Free all allocations that occurred before mem info collection was turned on
    DestroySchemaRecursive(root_device_node);
    DestroySchemaRecursive(root_internal_node);

    // If logging memory usage, print out all memory still in use, after attempting to free all known references
    USP_MEM_PrintLeakReport();
}

/*********************************************************************//**
**
** DATA_MODEL_GetParameterValue
**
** Gets a single named parameter from the data model
**
** \param   path - pointer to string containing complete data model path to the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
** \param   flags - options to control execution of this function (eg SHOW_PASSWORD)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetParameterValue(char *path, char *buf, int len, unsigned flags)
{
    dm_node_t *node;
    dm_node_t *table_node;
    int err;
    dm_instances_t inst;
    char instances[MAX_DM_PATH];
    bool exists;
    dm_req_t req;
    int num_instances;
    char *default_value;
    unsigned db_flags = 0;          // Default to database not unobfuscating values. NOTE Only secure nodes are obfuscated

    // Exit if unable to get node associated with parameter
    // This could occur if the parameter is not present in the schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // NOTE: We do not check 'is_qualified_instance' here, because the only time it would be unqualified, is if the
    //       path represented a multi-instance object. If path does represent this, then it will be caught below (switch statement)

    // Validate that the parsed object instance numbers exist in the data model (if parameter contains multi-instance objects in it's path)
    if (inst.order > 0)
    {
        err = DM_INST_VECTOR_IsExist(&inst, &exists);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        if (exists == false)
        {
            if ((flags & DONT_LOG_NO_INSTANCE_ERROR) == 0)
            {
                USP_ERR_SetMessage("%s: Path %s: Instance numbers do not exist", __FUNCTION__, path);
            }
            return USP_ERR_OBJECT_DOES_NOT_EXIST;
        }
    }

    // Get value based on the type of the node
    switch(node->type)
    {
        case kDMNodeType_Param_ConstantValue:
            USP_STRNCPY(buf, node->registered.param_info.default_value, len);
            break;

        case kDMNodeType_DBParam_Secure:
            // Return an empty string, if special flag is not set
            if ((flags & SHOW_PASSWORD)==0)
            {
                *buf = '\0';
                break;
            }
            // Intentional fall through to code below
            db_flags = OBFUSCATED_VALUE;

        case kDMNodeType_DBParam_ReadWrite:
        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_ReadOnlyAuto:
        case kDMNodeType_DBParam_ReadWriteAuto:
            FormInstanceString(&inst, instances, sizeof(instances));
            err = DATABASE_GetParameterValue(path, node->hash, instances, buf, len, db_flags);
            if (err == USP_ERR_OBJECT_DOES_NOT_EXIST)
            {
                // No entry present in the database, use the default value
                default_value = node->registered.param_info.default_value;
                if (default_value != NULL)
                {
                    USP_STRNCPY(buf, default_value, len);
                }
                else
                {
                    *buf = '\0';
                }
            }
            else if (err != USP_ERR_OK)
            {
                // An error occurred
                return err;
            }
            break;

        case kDMNodeType_Param_NumEntries:
            table_node = node->registered.param_info.table_node;
            err = DM_INST_VECTOR_GetNumInstances(table_node, &inst, &num_instances);
            if (err != USP_ERR_OK)
            {
                return err;
            }

            USP_SNPRINTF(buf, len, "%d", num_instances);
            break;


        case kDMNodeType_VendorParam_ReadOnly:
        case kDMNodeType_VendorParam_ReadWrite:
            err = GetVendorParam(node, path, &inst, buf, len, &req);
            if (err != USP_ERR_OK)
            {
                return err;
            }
            break;

        case kDMNodeType_Object_MultiInstance:
        case kDMNodeType_Object_SingleInstance:
            // Exit if path was to an object
            USP_ERR_SetMessage("%s: Trying to perform a parameter get operation on an object (%s)", __FUNCTION__, path);
            return USP_ERR_INVALID_PATH;
            break;

        case kDMNodeType_SyncOperation:
        case kDMNodeType_AsyncOperation:
            // Exit if path was to an operation
            USP_ERR_SetMessage("%s: Trying to perform a parameter get on an operation (%s)", __FUNCTION__, path);
            return USP_ERR_INVALID_PATH;
            break;

        case kDMNodeType_Event:
            // Exit if path was to an event
            USP_ERR_SetMessage("%s: Trying to perform a parameter get on an event (%s)", __FUNCTION__, path);
            return USP_ERR_INVALID_PATH;
            break;

        default:
            TERMINATE_BAD_CASE(node->type);
            break;
    }

    // If code gets here, then value was retrieved successfully
    buf[len -1] = '\0';         // Ensure that buffer is always zero terminated (eg vendor may not do this)

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_SetParameterValue
**
** Sets a single named parameter in the data model
**
** \param   path - pointer to string containing complete path
** \param   new_value - pointer to buffer containing value to set
** \param   flags - options to control execution of this function (eg CHECK_WRITABLE)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_SetParameterValue(char *path, char *new_value, unsigned flags)
{
    dm_node_t *node;
    int err;
    dm_instances_t inst;
    char instances[MAX_DM_PATH];
    dm_validate_value_cb_t validate_cb;
    dm_req_t req;
    bool exists;
    unsigned db_flags = 0;          // Default to database not unobfuscating values. NOTE Only secure nodes are obfuscated

    USP_ASSERT(DM_TRANS_IsWithinTransaction()==true);

    // Exit if unable to get node associated with parameter
    // This could occur if the parameter is not present in the schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_UNSUPPORTED_PARAM;
    }

    // NOTE: We do not check 'is_qualified_instance' here, because the only time it would be unqualified, is if the
    //       path represented a multi-instance object. If path does represent this, then it will be caught below


    // Exit if trying to set the value of something which is not a parameter
    if (IsParam(node) == false)
    {
        USP_ERR_SetMessage("%s: Trying to perform a parameter set on something which is not a parameter (%s)", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    // Validate that the parsed object instance numbers exist in the data model (if parameter contains multi-instance objects in its path)
    if (inst.order > 0)
    {
        err = DM_INST_VECTOR_IsExist(&inst, &exists);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        if (exists == false)
        {
            USP_ERR_SetMessage("%s: Path %s: Instance numbers do not exist", __FUNCTION__, path);
            return USP_ERR_INVALID_PATH;
        }
    }

    // Exit if the parameter is read only
    switch(node->type)
    {
        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_ReadOnlyAuto:
            if (flags & CHECK_WRITABLE)
            {
                // Read-only parameters may be written internally by USP Agent when seeding read only tables
                // but writes initiated by a controller should not be allowed
                USP_ERR_SetMessage("%s: Trying to perform a parameter set on read-only parameter %s", __FUNCTION__, path);
                return USP_ERR_PARAM_READ_ONLY;
            }
            break;

        case kDMNodeType_Param_ConstantValue:
        case kDMNodeType_Param_NumEntries:
        case kDMNodeType_VendorParam_ReadOnly:
            USP_ERR_SetMessage("%s: Trying to perform a parameter set on read-only parameter %s", __FUNCTION__, path);
            return USP_ERR_PARAM_READ_ONLY;
            break;

        default:
            // Parameter is writable - nothing to do
            break;
    }

    // Exit if unable to convert the new value to its correct native type
    // NOTE: This also initialises the request structure, passed to the set vendor hook
    err = DM_PRIV_InitSetRequest(&req, node, path, &inst, new_value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Perform the set
    switch(node->type)
    {
        case kDMNodeType_VendorParam_ReadWrite:
            // Exit if unable to set the vendor parameter, aborting the transaction
            err = SetVendorParam(node, path, &inst, new_value, &req);
            if (err != USP_ERR_OK)
            {
                return err;
            }
            break;

        case kDMNodeType_DBParam_Secure:
            // Intentional fall through to code below
            db_flags = OBFUSCATED_VALUE;

        case kDMNodeType_DBParam_ReadWrite:
        case kDMNodeType_DBParam_ReadWriteAuto:
            // Exit if new value fails additional validation
            validate_cb = node->registered.param_info.validator_cb;
            if (validate_cb != NULL)
            {
                USP_ERR_ClearMessage();
                err = validate_cb(&req, new_value);
                if (err != USP_ERR_OK)
                {
                    USP_ERR_ReplaceEmptyMessage("%s: Failed to validate (new value=%s) on (path=%s)", __FUNCTION__, new_value, path);
                    return err;
                }
            }

            // Set the parameter to the new value in the database
            FormInstanceString(&inst, instances, sizeof(instances));
            err = DATABASE_SetParameterValue(path, node->hash, instances, new_value, db_flags);
            if (err != USP_ERR_OK)
            {
                return err;
            }
            break;

        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_ReadOnlyAuto:
            // Set the parameter to the new value in the database
            // Read-only parameters may be written internally by USP Agent when seeding read only tables
            // but writes initiated by a controller should never reach here
            FormInstanceString(&inst, instances, sizeof(instances));
            err = DATABASE_SetParameterValue(path, node->hash, instances, new_value, 0);
            if (err != USP_ERR_OK)
            {
                return err;
            }
            break;

        case kDMNodeType_Param_ConstantValue:
        case kDMNodeType_Param_NumEntries:
        case kDMNodeType_VendorParam_ReadOnly:
            // The code should not have reached here, as these parameters are read only
            USP_ASSERT(false);
            break;

        default:
            TERMINATE_BAD_CASE(node->type);
            break;
    }

    // Add this instance to the dm_instances_vector vector
    err = DM_INST_VECTOR_Add(&inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Add this parameter to the list of parameters which are pending notification to the vendor
    // They will be notified once the whole transaction has been completed successfully
    // (or they will be forgotten if the transaction was aborted)
    DM_TRANS_Add(kDMOp_Set, path, new_value, &req.val_union, node, &inst);

    // If code gets here, then value was set successfully
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_AddInstance
**
** Adds an instance to the data model
** This function may be called with either the agent or the controller allocating the instance number
**   - If this code allocates the instance number, then 'instance' is non NULL,
**     and the path is an object without trailing instance number (e.g. "Device.LocalAgent.Controller")
**   - If the caller allocates the instance number, then 'instance' is NULL,
**     and the path is an object with trailing instance number (e.g. "Device.LocalAgent.Controller.1")
** NOTE: If the path is specified with a trailing '.', then internally this is removed, to be consistent for notify callbacks etc
**
** \param   path - path of the object to add an instance to
** \param   instance - pointer to variable in which to return instance number in,
**                     or NULL to denote controller allocated instance number
** \param   flags - options to control execution of this function (eg CHECK_CREATABLE, IGNORE_INSTANCE_EXISTS)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_AddInstance(char *path, int *instance, unsigned flags)
{
    dm_instances_t inst;
    dm_node_t *node;
    int err;
    int new_instance;
    int order;
    char internal_path[MAX_DM_PATH];
    dm_validate_add_cb_t validate_add;
    dm_add_cb_t add;
    dm_add_group_cb_t group_add;
    dm_object_info_t *info;
    dm_req_t req;
    bool exists;
    bool is_qualified_instance;
    int group_id;
    int len;
    char *p;

    USP_ASSERT(DM_TRANS_IsWithinTransaction()==true);

    // Exit if path is too long
    len = strlen(path);
    if (len >= MAX_DM_PATH)
    {
        USP_ERR_SetMessage("%s: Path too long (%d chars in %s).", __FUNCTION__, len, path);
        return USP_ERR_RESOURCES_EXCEEDED;
    }
    USP_STRNCPY(internal_path, path, sizeof(internal_path));

    // Fixup path (if necessary) so that it does not include a trailing '.'
    // The reason for this is so that strings in debug and notify callbacks are always consistent
    if (internal_path[len-1] == '.')
    {
        internal_path[len-1] = '\0';
    }

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(internal_path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if we cannot add instances of this object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Cannot create instances of %s. Not a multi-instance object.", __FUNCTION__, internal_path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if the table is grouped but non-writable
    info = &node->registered.object_info;
    group_id = node->group_id;
    if ((group_id != NON_GROUPED) && (info->group_writable == false))
    {
        USP_ERR_SetMessage("%s: Cannot add instances to a read only table", __FUNCTION__);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if the path contains a trailing instance number and shouldn't, or vice versa
    if ((instance == NULL) && (group_id == NON_GROUPED))
    {
        // Path should contain an instance number
        if (is_qualified_instance == false)
        {
            USP_ERR_SetMessage("%s: Path (%s) should contain instance number of object to add", __FUNCTION__, internal_path);
            return USP_ERR_OBJECT_NOT_CREATABLE;
        }
    }
    else
    {
        // Path shouldn't contain an instance number
        if (is_qualified_instance)
        {
            USP_ERR_SetMessage("%s: Path (%s) should not contain instance number of object to add", __FUNCTION__, internal_path);
            return USP_ERR_OBJECT_NOT_CREATABLE;
        }
    }

    // Determine the instance number to add
    // NOTE: For grouped objects, the instance number is returned by the group_add callback
    if (group_id == NON_GROUPED)
    {
        if (instance != NULL)
        {
            // Data model allocates instance number
            err = DM_INST_VECTOR_GetNextInstance(node, &inst, &new_instance);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
        else
        {
            // Exit if unable to determine whether instance exists
            err = DM_INST_VECTOR_IsExist(&inst, &exists);
            if (err != USP_ERR_OK)
            {
                return err;
            }

            // Exit if specified instance already exists
            if (exists)
            {
                // Exit if we don't care if the instance has already been created
                if (flags & IGNORE_INSTANCE_EXISTS)
                {
                    return USP_ERR_OK;
                }

                USP_ERR_SetMessage("%s: Cannot add %s. Instance already exists", __FUNCTION__, path);
                return USP_ERR_REQUEST_DENIED;
            }

            // Fixup internal_path, so that it does not include the trailing instance number
            USP_ASSERT(inst.order > 0);        // since a fully qualified instance
            inst.order--;
            new_instance = inst.instances[ inst.order ];
            p = strrchr(internal_path, '.');

            USP_ASSERT(p != NULL);             // since a fully qualified instance
            *p = '\0';
        }
    }

    // Exit if unable to determine whether parent object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the parent object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, internal_path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    if (group_id == NON_GROUPED)
    {
        // USE NON GROUPED API
        // Populate request structure passed to vendor hook functions
        DM_PRIV_RequestInit(&req, node, internal_path, &inst);

        // Exit if vendor hook is not allowing any more instances of this object to be created
        // Typically this will be the case if the agent has a fixed number of resources representing the object,
        // or if the agent owns creation/deletion of the object
        // NOTE: Read-only tables may be added to internally by USP Agent when seeding them
        // but adds initiated by a controller should always check whether the table is read only
        if (flags & CHECK_CREATABLE)
        {
            validate_add = info->validate_add_cb;
            if (validate_add != NULL)
            {
                USP_ERR_ClearMessage();

                err = validate_add(&req);
                if (err != USP_ERR_OK)
                {
                    USP_ERR_ReplaceEmptyMessage("%s: Cannot add any more instances to path=%s", __FUNCTION__, internal_path);
                    return err;
                }
            }
        }

        // Add this instance and node to the instances structure
        // NOTE: This implicitly updates the DM request structure, as that structure just points to the instance structure
        order = inst.order;
        inst.nodes[order] = node;
        inst.instances[order] = new_instance;
        inst.order = order+1;

        // Exit if add vendor hook fails
        add = info->add_cb;
        if (add != NULL)
        {
            USP_ERR_ClearMessage();

            err = add(&req);
            if (err != USP_ERR_OK)
            {
                USP_ERR_ReplaceEmptyMessage("%s: add vendor hook failed for path=%s", __FUNCTION__, internal_path);
                return err;
            }
        }
    }
    else
    {
        // USE GROUPED API
        group_add = group_vendor_hooks[group_id].add_group_cb;
        if (group_add == NULL)
        {
            USP_ERR_SetMessage("%s: No grouped Add vendor hook registered to add to %s", __FUNCTION__, internal_path);
            return USP_ERR_OBJECT_NOT_CREATABLE;
        }

        USP_ERR_ClearMessage();
        err = group_add(group_id, internal_path, &new_instance);
        if (err != USP_ERR_OK)
        {
            USP_ERR_ReplaceEmptyMessage("%s: group add vendor hook failed for path=%s", __FUNCTION__, internal_path);
            return err;
        }

        order = inst.order;
        inst.nodes[order] = node;
        inst.instances[order] = new_instance;
        inst.order = order+1;
    }

    // Save the instance number to return
    if (instance != NULL)
    {
        *instance = new_instance;
    }

    // Register the new instance number with the data model
    err = DM_INST_VECTOR_Add(&inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Form the path to this instance in internal_path[]
    len = strlen(internal_path);
    len += USP_SNPRINTF(&internal_path[len], sizeof(internal_path)-len, ".%d", new_instance);

    // Now add default values for all child parameters that exist in the USP database
    err = AddChildParamsDefaultValues(internal_path, len, node, &inst);
    if (err != USP_ERR_OK)
    {
        DM_INST_VECTOR_Remove(&inst);
        return err;
    }
    internal_path[len] = '\0';      // Child path contains instance number of object just created

    // Add this object instance to the list of instances which are pending notification to the vendor
    // They will be notified once the whole transaction has been completed successfully
    // (or they will be forgotten if the transaction was aborted)
    DM_TRANS_Add(kDMOp_Add, internal_path, NULL, NULL, node, &inst);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_DeleteInstance
**
** Deletes the specified instance from the data model
**
** \param   path - path of the object instance to delete
** \param   flags - options to control execution of this function (eg CHECK_DELETABLE, IGNORE_NO_INSTANCE)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_DeleteInstance(char *path, unsigned flags)
{
    dm_instances_t inst;
    dm_node_t *node;
    int err;
    char child_path[MAX_DM_PATH];
    int group_id;
    dm_object_info_t *info;
    dm_validate_del_cb_t validate_del;
    dm_del_cb_t del;
    dm_del_group_cb_t group_del;
    dm_req_t req;
    bool exists;
    bool is_qualified_instance;

    USP_ASSERT(DM_TRANS_IsWithinTransaction()==true);

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not contain instance number of object to delete", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        // Exit if we should silently ignore objects that have already been deleted
        if (flags & IGNORE_NO_INSTANCE)
        {
            return USP_ERR_OK;
        }

        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if we cannot delete instances of this object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Cannot delete instances of %s. Not a multi-instance object.", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    info = &node->registered.object_info;
    group_id = node->group_id;
    if (group_id == NON_GROUPED)
    {
        // USE NON GROUPED API
        // Populate request structure passed to vendor hook functions
        DM_PRIV_RequestInit(&req, node, path, &inst);

        // Exit if vendor hook is not allowing this instance to be deleted
        // Typically this will be the case if the agent owns creation/deletion of the object
        // NOTE: Read-only tables may be deleted internally by USP Agent
        // but deletes initiated by a controller should always check whether the table is read only
        if (flags & CHECK_DELETABLE)
        {
            validate_del = info->validate_del_cb;
            if (validate_del != NULL)
            {
                USP_ERR_ClearMessage();

                err = validate_del(&req);
                if (err != USP_ERR_OK)
                {
                    USP_ERR_ReplaceEmptyMessage("%s: Cannot delete object=%s", __FUNCTION__, path);
                    return err;
                }
            }
        }

        // Exit if delete vendor hook fails
        // This vendor hook typically is used to delete a row in a vendor DB
        // NOTE: The delete vendor hook needs to be separate from the validate and notify vendor hooks
        // It needs to be separate from validate, because validate is not called for internal data model instance deletion
        // It needs to be separate from notify because vendor DB changes need to be done in same transaction as USP DB changes
        del = info->del_cb;
        if (del != NULL)
        {
            USP_ERR_ClearMessage();

            err = del(&req);
            if (err != USP_ERR_OK)
            {
                USP_ERR_ReplaceEmptyMessage("%s: del vendor hook failed for path=%s", __FUNCTION__, path);
                return err;
            }
        }
    }
    else
    {
        // USE GROUPED API
        // Exit if the table is non-writable
        if (info->group_writable == false)
        {
            USP_ERR_SetMessage("%s: Cannot delete instances from a read only table", __FUNCTION__);
            return USP_ERR_OBJECT_NOT_DELETABLE;
        }

        group_del = group_vendor_hooks[group_id].del_group_cb;
        if (group_del == NULL)
        {
            USP_ERR_SetMessage("%s: No grouped Delete vendor hook registered to delete from %s", __FUNCTION__, path);
            return USP_ERR_OBJECT_NOT_DELETABLE;
        }

        USP_ERR_ClearMessage();
        err = group_del(group_id, path);
        if (err != USP_ERR_OK)
        {
            USP_ERR_ReplaceEmptyMessage("%s: group delete vendor hook failed for path=%s", __FUNCTION__, path);
            return err;
        }
    }

    // Add this object instance to the list of instances which are pending notification to the vendor (ie calling the notify vendor hook)
    // They will be notified once the whole transaction has been completed successfully
    // (or they will be forgotten if the transaction was aborted)
    // NOTE: This must be performed before the object is actually deleted from the data model, because
    // it also determines the list of objects which will send ObjectDeletion notifies based on the objects currently in the data model
    DM_TRANS_Add(kDMOp_Del, path, NULL, NULL, node, &inst);

    // Now delete all child parameters and instances
    USP_STRNCPY(child_path, path, sizeof(child_path));
    err = DeleteChildParams(child_path, strlen(child_path), node, &inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // DeRegister the instance number with the data model
    // NOTE: This must be performed after DeleteChildParams(), otherwise that function will not be aware of the child instance numbers to delete
    DM_INST_VECTOR_Remove(&inst);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetPermissions
**
** Gets the permissions associated with a data model object or parameter, for the specified role
**
** \param   path - path of the object or parameter to get the permissions of
** \param   combined_role - role used to access this path. If set to INTERNAL_ROLE, then full permissions are always returned
** \param   perm - pointer to variable in which to return permission bitmask
** \param   flags - bitmask of options controlling execution (eg DONT_LOG_ERRORS)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetPermissions(char *path, combined_role_t *combined_role, unsigned short *perm, unsigned flags)
{
    dm_node_t *node;

    // Exit if unable to get node associated with object or parameter
    // This could occur if the parameter is not present in the schema, or if the specified instance does not exist
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, flags);
    if (node == NULL)
    {
        *perm = 0;
        return USP_ERR_INVALID_PATH;
    }

    // Get the permissions associated with the role
    *perm = DM_PRIV_GetPermissions(node, combined_role);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_NotifyInstanceAdded
**
** Called if a vendor thread signals that an instance has been added
** (the vendor will already have assigned the instance number of the object being added)
** NOTE: This function does not have to be called within a transaction
**
** \param   path - path of the object instance that has been added by the vendor
**
** \return  USP_ERR_OK if object was added to data model
**          NOTE: The return code decides whether subscriptions should be consulted
**
**************************************************************************/
int DATA_MODEL_NotifyInstanceAdded(char *path)
{
    int err;
    dm_node_t *node;
    dm_instances_t inst;
    bool is_qualified_instance;
    bool exists;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if the object the vendor signalled was not a multi-instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Path (%s) is not a multi-instance object.", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) should contain instance number of object that was added", __FUNCTION__, path);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to determine whether instance already exists
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if instance already exists - nothing to do
    if (exists)
    {
        USP_ERR_SetMessage("%s: Object (%s) already exists in the data model", __FUNCTION__, path);
        return USP_ERR_CREATION_FAILURE;
    }

    // Exit if the parent object instances in the path do not exist
    if (inst.order > 1)
    {
        inst.order--;           // Temporarily remove the instance number of the object that was added,
                                // so that structure indicates only parent instance numbers
        err = DM_INST_VECTOR_IsExist(&inst, &exists);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        if (exists == false)
        {
            USP_ERR_SetMessage("%s: Parent objects in path (%s) do not exist", __FUNCTION__, path);
            return USP_ERR_OBJECT_DOES_NOT_EXIST;
        }
        inst.order++;           // Restore the structure, so that it indicates the instance number of the object that was added
    }

    // Register this new instance with the data model
    err = DM_INST_VECTOR_Add(&inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_NotifyInstanceDeleted
**
** Updates the instance cache after an object has been deleted
** Called by the data model thread after a vendor thread signals that an instance has been deleted
** NOTE: This function does not have to be called within a transaction
**
** \param   path - path of the object instance that has been deleted by the vendor
**
** \return  USP_ERR_OK if object was deleted from data model.
**          NOTE: The return code decides whether subscriptions should be consulted
**
**************************************************************************/
int DATA_MODEL_NotifyInstanceDeleted(char *path)
{
    dm_node_t *node;
    dm_instances_t inst;
    bool is_qualified_instance;
    bool exists;
    int err;
    int i;
    str_vector_t child_objs;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if the object the vendor signalled was not a multi-instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Cannot signal deleting instances of %s. Not a multi-instance object.", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) should contain instance number of object that was deleted", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if unable to determine whether instance exists
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if instance does not exist - nothing to do
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object (%s) does not exist in the data model", __FUNCTION__, path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Ensure that object is present in list of objects to match against for deletion
    DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths();

    // Queue object life events for this object and all child objects
    STR_VECTOR_Init(&child_objs);
    err = DATA_MODEL_GetAllInstancePaths(path, &child_objs, INTERNAL_ROLE);
    if (err == USP_ERR_OK)
    {
        for (i=0; i < child_objs.num_entries; i++)
        {
            DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(child_objs.vector[i], kSubNotifyType_ObjectDeletion);
        }
    }
    STR_VECTOR_Destroy(&child_objs);

    // Remove this instance (and all children) from the data model cache
    DM_INST_VECTOR_Remove(&inst);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_Operate
**
** Performs a synchronous or asynchronous operation
**
** \param   path - path of the operation
** \param   input_args - input arguments for the operation
** \param   output_args - output arguments for the operation (if it was a synchronous operation and has completed)
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   instance - pointer to variable in which to return the instance number of an entry in the request table
**                     (if the operation was sync, then the variable will be set to invalid)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_Operate(char *path, kv_vector_t *input_args, kv_vector_t *output_args, char *command_key, int *instance)
{
    dm_instances_t inst;
    dm_node_t *node;
    int err;
    dm_sync_oper_cb_t sync_oper_cb;
    dm_async_oper_cb_t async_oper_cb;
    dm_req_t req;
    bool exists;
    bool is_qualified_instance;
    dm_oper_info_t *info;

    // Setup default return values
    KV_VECTOR_Init(output_args);
    *instance = INVALID;
    USP_ASSERT(DM_TRANS_IsWithinTransaction()==true);

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }
    info = &node->registered.oper_info;

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not contain instance number of object to operate on", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if input arguments do not match those registered in the data model for this operation
    err = KV_VECTOR_ValidateArguments(input_args, &info->input_args, IGNORE_UNKNOWN_ARGS);
    if (err != USP_ERR_OK)
    {
        return USP_ERR_COMMAND_FAILURE;
    }

    // Populate request structure passed to vendor hook functions
    DM_PRIV_RequestInit(&req, node, path, &inst);

    switch(node->type)
    {
        case kDMNodeType_SyncOperation:
            // Perform the synchronous operation
            sync_oper_cb = info->sync_oper_cb;
            USP_ASSERT(sync_oper_cb != NULL);

            USP_ERR_ClearMessage();
            err = sync_oper_cb(&req, command_key, input_args, output_args);
            if (err != USP_ERR_OK)
            {
                USP_ERR_ReplaceEmptyMessage("%s: Synchronous operation (%s) failed", __FUNCTION__, path);
                goto exit;
            }

            #ifdef VALIDATE_OUTPUT_ARG_NAMES
            // Validate the names of the output arguments
            err = KV_VECTOR_ValidateArguments(output_args, &info->output_args, NO_FLAGS);
            if (err != USP_ERR_OK)
            {
                USP_LOG_Warning("%s: Output arguments names do not match those registered (%s). Please check code.", __FUNCTION__, path);
                err = USP_ERR_OK;
            }
            #endif
            break;

        case kDMNodeType_AsyncOperation:
            // Exit if we've reached the limit of concurrent operations for this command
            if (info->max_concurrency < INT_MAX)
            {
                int count = DEVICE_REQUEST_CountMatchingRequests(node->path);
                if (count == INVALID)
                {
                    err = USP_ERR_INTERNAL_ERROR;
                    goto exit;
                }

                if (count >= info->max_concurrency)
                {
                    USP_ERR_ReplaceEmptyMessage("%s: Limit of %d %s in progress reached", __FUNCTION__, info->max_concurrency, node->name);
                    err = USP_ERR_RESOURCES_EXCEEDED;
                    goto exit;
                }
            }

            // Create an entry in the Request Table
            err = DEVICE_REQUEST_Add(path, command_key, instance);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Add a time reference to the input args for the time at which this operation was issued
            // This is necessary so that when restarting an interrupted operation after a reboot
            // the correct absolute times are calculated, if the input args contained relative time args (eg 'delay')
            USP_ARG_AddDateTime(input_args, SAVED_TIME_REF_ARG_NAME, time(NULL));

            // Persist the input arguments (if this operation might be restarted at bootup)
            if (info->restart_cb != NULL)
            {
                err = DEVICE_REQUEST_PersistOperationArgs(*instance, input_args, "Input");
                if (err != USP_ERR_OK)
                {
                    goto exit;
                }
            }

            // Start the async operation
            async_oper_cb = info->async_oper_cb;
            USP_ASSERT(async_oper_cb != NULL);

            USP_ERR_ClearMessage();
            err = async_oper_cb(&req, input_args, *instance);
            if (err != USP_ERR_OK)
            {
                USP_ERR_ReplaceEmptyMessage("%s: Asynchronous operation (%s) failed to start", __FUNCTION__, path);
            }
            break;

        default:
            // Exit if we cannot operate on this path
            USP_ERR_SetMessage("%s: Path '%s' is not an operation", __FUNCTION__, path);
            err = USP_ERR_COMMAND_FAILURE;
            break;
    }

exit:
    return err;
}


/*********************************************************************//**
**
** DATA_MODEL_ShouldOperationRestart
**
** This function is called for all asynchronous requests in the request table at boot up
** to determine whether to restart the Async Operation
**
** \param   path - path of this operation in the data model
** \param   instance - instance number of this operation in the Request table
** \param   is_restart - pointer to variable in which to return whether the operation should be restarted or not
**
**                     The following parameters are only used if the operation should not be restarted
**                     They determine the values placed in the operation complete message
** \param   err_code - pointer to variable in which to return an error code
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
** \param   output_args - pointer to structure in which to return output arguments for the operation
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DATA_MODEL_ShouldOperationRestart(char *path, int instance, bool *is_restart,
                                      int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args)
{
    dm_instances_t inst;
    dm_node_t *node;
    int err;
    dm_req_t req;
    bool exists;
    bool is_qualified_instance;
    dm_oper_info_t *info;
    dm_async_restart_cb_t restart_cb;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not contain instance number of object to operate on", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Populate request structure passed to vendor hook functions
    DM_PRIV_RequestInit(&req, node, path, &inst);

    // If no restart callback has been setup, then use the default callback (which is for the operation to not be restarted)
    info = &node->registered.oper_info;
    USP_ASSERT(node->type == kDMNodeType_AsyncOperation);
    restart_cb = info->restart_cb;
    if (restart_cb == NULL)
    {
        restart_cb = DM_ACCESS_DontRestartAsyncOperation;
    }

    // Determine whether to restart this operation
    err = restart_cb(&req, instance, is_restart, err_code, err_msg, err_msg_len, output_args);

    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_RestartAsyncOperation
**
** Restarts an asynchronous operation
** This function is called on reboot, if a power cycle interrupted an operation from completing
**
** \param   path - path of the operation
** \param   input_args - input arguments for the operation
** \param   instance - instance number of the entry in the request table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_RestartAsyncOperation(char *path, kv_vector_t *input_args, int instance)
{
    dm_instances_t inst;
    dm_node_t *node;
    int err;
    dm_async_oper_cb_t async_oper_cb;
    dm_req_t req;
    bool exists;
    bool is_qualified_instance;
    dm_oper_info_t *info;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if this object is not a fully qualified instance
    if (is_qualified_instance == false)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not contain instance number of object to operate on", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Populate request structure passed to vendor hook functions
    DM_PRIV_RequestInit(&req, node, path, &inst);

    // Start the async operation
    info = &node->registered.oper_info;
    USP_ASSERT(node->type == kDMNodeType_AsyncOperation);
    async_oper_cb = info->async_oper_cb;
    USP_ASSERT(async_oper_cb != NULL);
    err = async_oper_cb(&req, input_args, instance);

    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_GetPathProperties
**
** Determines whether the specified path exists in the schema, and if it does
** whether it's instance numbers match, and whether it is an object or a parameter.
** If it is a multi-instance object, then determines whether the path contains the instance number of the object or not
**
** \param   path - pointer to string containing complete path to object
** \param   combined_role - pointer to role used to access this path. If set to INTERNAL_ROLE(=NULL), then full permissions are always returned
** \param   permission_bitmask - pointer to variable in which to return the permissions associated with this path
**                               If this parameter is NULL, then the caller is not interested in the permissions for this node,
**                               and the role argument is ignored
** \param   group_id - pointer to variable in which to return the group_id, or NULL if this is not required
** \param   type_flags - pointer to variable in which to return the type of the parameter, or NULL if this is not required. NOTE: Only applicable for parameters
** \param   exec_flags - bitmask of options controling execution (eg DONT_LOG_ERRORS)
**
** \return  flag variable containing the path's properties
**          NOTE: Sets USP error message if returning flags that would constitute an error
**
**************************************************************************/
unsigned DATA_MODEL_GetPathProperties(char *path, combined_role_t *combined_role, unsigned short *permission_bitmask, int *group_id, unsigned *type_flags, unsigned exec_flags)
{
    dm_node_t *node;
    dm_instances_t inst;
    bool exists;
    bool is_qualified_instance;
    unsigned flags = 0;               // default return value
    dm_param_info_t *info;
    int err;

    // Default return value for permissions
    if (permission_bitmask != NULL)
    {
        *permission_bitmask = PERMIT_NONE;
    }

    // Exit if path does not exist in the schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, exec_flags);
    if (node == NULL)
    {
        return flags;
    }
    flags |= PP_EXISTS_IN_SCHEMA;

    // Setup permissions to return
    if (permission_bitmask != NULL)
    {
        *permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
    }

    // Determine whether path is to a parameter or an object, and also whether the parameter is writable
    switch(node->type)
    {
        case kDMNodeType_Object_MultiInstance:
            flags |= PP_IS_MULTI_INSTANCE_OBJECT;   // Intentional fall through

        case kDMNodeType_Object_SingleInstance:
            flags |= PP_IS_OBJECT;
            if (is_qualified_instance)
            {
                flags |= PP_IS_OBJECT_INSTANCE;
            }
            break;

        case kDMNodeType_DBParam_Secure:
            flags |= PP_IS_SECURE_PARAM;
            // Intentional fall through (kDMNodeType_DBParam_Secure is writable and denotes a parameter)

        case kDMNodeType_DBParam_ReadWrite:
        case kDMNodeType_DBParam_ReadWriteAuto:
        case kDMNodeType_VendorParam_ReadWrite:
            flags |= PP_IS_WRITABLE;
            // Intentional fall through (these types are parameters)

        case kDMNodeType_Param_ConstantValue:
        case kDMNodeType_Param_NumEntries:
        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_ReadOnlyAuto:
        case kDMNodeType_VendorParam_ReadOnly:
            flags |= PP_IS_PARAMETER;
            USP_ASSERT(is_qualified_instance == true);     // If it's a parameter then path must be a qualified instance, otherwise DM_PRIV_GetNodeFromPath() would have returned NULL

            // Determine if this parameter should be ignored by value change subscriptions
            if (node->registered.param_info.type_flags & DM_VALUE_CHANGE_WILL_IGNORE)
            {
                flags |= PP_VALUE_CHANGE_WILL_IGNORE;
            }
            break;

        case kDMNodeType_SyncOperation:
        case kDMNodeType_AsyncOperation:
            flags |= PP_IS_OPERATION;
            USP_ASSERT(is_qualified_instance == true);     // If it's an operation then path must be a qualified instance, otherwise DM_PRIV_GetNodeFromPath() would have returned NULL
            break;

        case kDMNodeType_Event:
            flags |= PP_IS_EVENT;
            USP_ASSERT(is_qualified_instance == true);     // If it's an event then path must be a qualified instance, otherwise DM_PRIV_GetNodeFromPath() would have returned NULL
            break;

        default:
            TERMINATE_BAD_CASE(node->type);
            break;
    }

    // Store the group_id
    if (group_id != NULL)
    {
        *group_id = node->group_id;
    }

    // Store the type_flags, if this is a parameter
    if (type_flags != NULL)
    {
        if (flags & PP_IS_PARAMETER)
        {
            info = &node->registered.param_info;
            *type_flags = info->type_flags;
        }
        else
        {
            *type_flags = 0;
        }
    }

    // Exit if unable to determine if the specified object instances of the parameter/object exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Determine if the specified object instances of the parameter/object exist
    // NOTE: If the path is to an unqualified multi-instance object, then this checks the parent instances of the object
    if (exists)
    {
        flags |= (PP_INSTANCE_NUMBERS_EXIST | PP_PARENT_INSTANCE_NUMBERS_EXIST);
    }
    else
    {
        // Not all instance numbers in the path exist, but do parent instances exist ?
        // (This flag is used by SetRequest:auto-create objects)
        if (is_qualified_instance)
        {
            if (inst.order == 1)
            {
                flags |= PP_PARENT_INSTANCE_NUMBERS_EXIST;
            }
            else if (inst.order > 1)
            {
                inst.order--;
                err = DM_INST_VECTOR_IsExist(&inst, &exists);
                if (err != USP_ERR_OK)
                {
                    return err;
                }

                if (exists)
                {
                    flags |= PP_PARENT_INSTANCE_NUMBERS_EXIST;
                }
            }
        }
    }

    return flags;
}

/*********************************************************************//**
**
** DATA_MODEL_SplitPath
**
** Splits the given path into a schema path and the instance numbers
** This function also determines if the instance numbers exist in the data model
** NOTE: This function only generates an error if the path is not present in the schema
**
** \param   path - pointer to string containing complete path to object
** \param   schema_path - pointer to variable in which to return the schema path
** \param   instances - pointer to structure in which to return instance numbers
** \param   instances_exist - pointer to boolean in which to return whether the returned instance numbers exist in the data model
**                            or NULL, if the caller is not interested in this information
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_SplitPath(char *path, char **schema_path, dm_req_instances_t *instances, bool *instances_exist)
{
    dm_node_t *node;
    dm_instances_t inst;
    int err;

    // Exit if path does not exist in the schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Determine if the parsed instances exist in the data model
    if (instances_exist != NULL)
    {
        err = DM_INST_VECTOR_IsExist(&inst, instances_exist);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    // Finally copy the schema_path and instance numbers into the return arguments
    *schema_path = node->path;
    memcpy(instances, &inst, sizeof(dm_req_instances_t));
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_InformInstance
**
** Informs the data model that an instance of the specified object exists
** NOTE: This function is called by vendor owned objects
**
** \param   path - path to instance which exists
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_InformInstance(char *path)
{
    int err;
    dm_node_t *node;
    dm_instances_t inst;
    bool is_qualified_instance;

    // Exit if unable to get node associated with parameter
    // This could occur if the parameter is not present in the schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // Exit if the specified path does not represent a qualified multi-instance object
    if ((is_qualified_instance == false) || (inst.order == 0))
    {
        USP_ERR_SetMessage("%s: path %s does not represent a fully qualified multi-instance object", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    // Exit if unable to add this instance to the data model
    err = DM_INST_VECTOR_Add(&inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_AddParameterInstances
**
** Adds the specified object instances for the parameter to the internal data model instance arrays
** NOTE: This function is called at bootup by the database startup code to seed the model with the
**       object instances contained in the database
** NOTE: The instance is not added again, if it already exists
**
** \param   hash - hash identifying data model parameter
** \param   instances - string containing the instance numbers of the multi-instance objects in the path of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if the parameter does not exist in the data model or the
**          instance numbers are not correct (invalid, too many or not enough for the object's path)
**
**************************************************************************/
int DATA_MODEL_AddParameterInstances(dm_hash_t hash, char *instances)
{
    dm_node_t *node;
    dm_instances_t inst;
    int err;

    // Exit if parameter does not exist in the data model
    node = FindNodeFromHash(hash);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: WARNING: Parameter (hash=%d) does not exist in the data model schema", __FUNCTION__, hash);
        return USP_ERR_INVALID_PATH;
    }

    // Exit if unable to parse the instance numbers from the string
    memset(&inst, 0, sizeof(inst));
    err = DM_PRIV_ParseInstanceString(instances, &inst);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Instance numbers ('%s') for hash=%d are invalid", __FUNCTION__, instances, hash);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the number of object instances in this string do not match the data model schema
    if (inst.order != node->order)
    {
        USP_ERR_SetMessage("%s: Number of instance numbers ('%s') for hash=%d does not match the number expected (%d)", __FUNCTION__, instances, hash, node->order);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Since they match, copy across the instance nodes which are the data model objects associated with the parsed instance numbers
    memcpy(inst.nodes, node->instance_nodes, inst.order*sizeof(dm_node_t *));

    // Finally add this instance to the dm_instances_vector vector
    err = DM_INST_VECTOR_Add(&inst);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetUniqueKeys
**
** Gets all the unique keys for the specified multi-instance object
**
** \param   path - pointer to string containing complete path to multi-instance object
** \param   ukv - pointer to variable in which to return a vector containing the unique keys
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetUniqueKeys(char *path, dm_unique_key_vector_t *ukv)
{
    dm_node_t *node;

    // Exit if path does not exist in the schema
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if node is not a multi-instance object (only multi-instance objects have unique keys)
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Path '%s' does not represent a multi-instance object", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    *ukv = node->registered.object_info.unique_keys;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetUniqueKeyParams
**
** Gets the values of all parameters which are registered as unique keys or part of compound unique keys
**
** \param   obj_path - pointer to string containing complete path to multi-instance object
** \param   params - pointer to key-value vector in which to return the name and values of all unique keys.
** \param   combined_role - role used to access the unique keys. If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
int DATA_MODEL_GetUniqueKeyParams(char *obj_path, kv_vector_t *params, combined_role_t *combined_role)
{
    dm_node_t *node;
    dm_unique_key_vector_t *ukv;
    char *name;
    int i, j;
    char param_path[MAX_DM_PATH];
    unsigned short permission_bitmask;
    group_get_vector_t ggv;
    group_get_entry_t *gge;
    kv_pair_t *pair;
    str_vector_t nv;              // Contains the names of each parameter (ie not the full path)
    int group_id;
    int len;
    int index;
    int err;

    KV_VECTOR_Init(params);
    STR_VECTOR_Init(&nv);
    GROUP_GET_VECTOR_Init(&ggv);

    // Exit if path does not exist in the schema
    node = DM_PRIV_GetNodeFromPath(obj_path, NULL, NULL, 0);
    if (node == NULL)
    {
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if node is not a multi-instance object (only multi-instance objects have unique keys)
    // Handle this case gracefully, so that GetInstances can just return no unique keys when invoked on a single instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        err = USP_ERR_OK;
        goto exit;
    }

    // Pre=calculate base path to avoid unnecessary copying in the loop
    len = USP_SNPRINTF(param_path, sizeof(param_path), "%s.", obj_path);

    // Iterate over all parameters of all unique keys, adding them (if not already added) to the group get vector
    ukv = &node->registered.object_info.unique_keys;
    for (i=0; i < ukv->num_entries; i++)
    {
        for (j=0; j<MAX_COMPOUND_KEY_PARAMS; j++)
        {
            // Move to compound unique key, if we've reached the end of the params for this compound key
            name = ukv->vector[i].param[j];
            if (name == NULL)
            {
                break;
            }

            // Move to next param in the compound unique key, if this param has already been added to the return array
            // This could occur if the object has more than one compound unique key, and the same param exists in two compound unique keys
            index = GROUP_GET_VECTOR_FindParam(&ggv, name);
            if (index != INVALID)
            {
                continue;
            }

            // Form the full path to the parameter
            USP_STRNCPY(&param_path[len], name, sizeof(param_path)-len);

            // Move to next param in the compound unique key if role does not have permission to read this parameter
            DATA_MODEL_GetPathProperties(param_path, combined_role, &permission_bitmask, &group_id, NULL, 0);
            if ((permission_bitmask & PERMIT_GET)==0)
            {
                continue;
            }

            // Add this parameter to the names and group get vectors
            STR_VECTOR_Add(&nv, name);
            GROUP_GET_VECTOR_Add(&ggv, param_path, group_id);
        }
    }

    // Exit if there aren't any unique key parameters associated with this object
    if (ggv.num_entries == 0)
    {
        err = USP_ERR_OK;
        goto exit;
    }

    // Get all unique key parameters
    GROUP_GET_VECTOR_GetValues(&ggv);

    // Exit if any of the parameters previously failed to get, returning the reason for error
    for (i=0; i < ggv.num_entries; i++)
    {
        gge = &ggv.vector[i];
        if ((gge->err_code != USP_ERR_OK) || (gge->value == NULL))
        {
            USP_ERR_SetMessage("%s", gge->err_msg);
            err = gge->err_code;
            goto exit;
        }
    }

    // Allocate memory to store the array of the key-value pair vector
    params->vector = USP_MALLOC(ggv.num_entries*sizeof(kv_pair_t));
    params->num_entries = ggv.num_entries;

    // Move all parameter names in the names vector, and values in the group get vector to the key-value pair vector
    // Freeing the contents of the group get vector whilst going along
    for (i=0; i < ggv.num_entries; i++)
    {
        gge = &ggv.vector[i];
        pair = &params->vector[i];
        pair->key = nv.vector[i];
        pair->value = gge->value;

        USP_SAFE_FREE(gge->path);
        USP_SAFE_FREE(gge->err_msg);
    }

    // Finally destroy the group get vector and name vectors
    // (all memory referenced by them has been moved to the key-value pair vector or freed)
    USP_FREE(ggv.vector);
    ggv.vector = NULL;
    ggv.num_entries = 0;
    USP_FREE(nv.vector);
    nv.vector = NULL;
    nv.num_entries = 0;

    // If the code gets here, all parameters in unique keys have been obtained
    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&nv);
    GROUP_GET_VECTOR_Destroy(&ggv);
    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_ValidateDefaultedUniqueKeys
**
** Validates that the unique keys that have been read back are unique
** by attempting to set any that haven't previously been set
** NOTE: This code is necessary because it is possible for the USP Add message to not contain
** values for some writable unique keys. If a unique key is not explicitly set,
** then it will end up with its default value. The default value might not be unique.
** By explicitly setting the parameter here, we force the set vendor hook to check that the key is unique
**
** \param   obj_path - pointer to string containing complete path to multi-instance object
** \param   params - pointer to key-value vector containing the values of all unique keys.
**                   Some of these values might be defaults, rather than having been explicitly set.
** \param   gsv - group set vector containing the parameters which were explicitly set in the Add message, and their result
**                or NULL, if no parameters were explicitly set
**
** \return  USP_ERR_OK if the unique keys are unique
**
**************************************************************************/
int DATA_MODEL_ValidateDefaultedUniqueKeys(char *obj_path, kv_vector_t *unique_key_params, group_set_vector_t *gsv)
{
    int i;
    int index;
    int err;
    char buf[MAX_DM_PATH];
    char *path;
    char *value;
    group_set_entry_t *gse;
    dm_node_t *node;
    str_vector_t uk_paths;    // vector containing the full paths of the unique keys which we're going to set
                              // the index of a parameter in this vector matches that in the unique_key_params vector

    // First, form the full path to all unique keys
    STR_VECTOR_Init(&uk_paths);
    for (i=0; i < unique_key_params->num_entries; i++)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s.%s", obj_path, unique_key_params->vector[i].key);
        STR_VECTOR_Add(&uk_paths, buf);
    }

    // Iterate over all parameters which were explicitly set
    if (gsv != NULL)
    {
        for (i=0; i < gsv->num_entries; i++)
        {
            // Determine if this parameter was a unique key
            gse = &gsv->vector[i];
            index = STR_VECTOR_Find(&uk_paths, gse->path);
            if (index != INVALID)
            {
                // Exit if the unique key failed to set, irrespective of whether it was marked as not-required
                // This prevents controllers from getting around the uniqueness check by marking the parameter as 'not-required'
                // (which would normally cause the failure to be ignored)
                if (gse->err_code != USP_ERR_OK)
                {
                    err = gse->err_code;
                    USP_ERR_SetMessage("%s", gse->err_msg);
                    goto exit;
                }

                // Remove this unique key as it has already been explicitly set successfully
                USP_FREE(uk_paths.vector[index]);
                uk_paths.vector[index] = NULL;
            }
        }
    }

    // Then remove all unique keys that are read only, or would have been set automatically
    // NOTE: We assume that if a key has been set automatically, that the chosen value is already unique (ensured by the automatic setter)
    for (i=0; i < uk_paths.num_entries; i++)
    {
        path = uk_paths.vector[i];
        if (path != NULL)
        {
            // Determine the data model type of the node
            node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
            USP_ASSERT(node != NULL);       // node should never be NULL, we've already got the value of all unique key parameters, so it must exist in the data model

            // Remove the node, if it is a read only parameter (in which case it can't be set to a unique value anyway)
            // or if the key has automatically been set to a unique value (kDMNodeType_DBParam_ReadWriteAuto)
            switch(node->type)
            {
                case kDMNodeType_DBParam_ReadOnly:
                case kDMNodeType_DBParam_ReadOnlyAuto:
                case kDMNodeType_DBParam_ReadWriteAuto:
                case kDMNodeType_VendorParam_ReadOnly:
                    USP_FREE(path);
                    uk_paths.vector[i] = NULL;
                    break;

                default:
                    break;
            }
        }
    }

    // Finally, attempt to set all of the unique keys that are left,
    // these will be parameters which we cannot yet guarantee their uniqueness (until they have been explicitly set)
    for (i=0; i < uk_paths.num_entries; i++)
    {
        path = uk_paths.vector[i];
        if (path != NULL)
        {
            value = unique_key_params->vector[i].value;
            USP_ASSERT(value != NULL);
            err = DATA_MODEL_SetParameterValue(path, value, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&uk_paths);
    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_DumpSchema
**
** Logs the data model schema which has been built up using the registration interface
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATA_MODEL_DumpSchema(void)
{
    DumpSchemaFromRoot(root_device_node, "DataModel");
    DumpSchemaFromRoot(root_internal_node, "Internal");
}

/*********************************************************************//**
**
** DATA_MODEL_DumpInstances
**
** Logs the instances in the data model instances vector
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DATA_MODEL_DumpInstances(void)
{
    USP_DUMP("Dumping DataModel Instance Vector...");
    DumpInstanceVectorRecursive(root_device_node);
    DumpInstanceVectorRecursive(root_internal_node);
}

/*********************************************************************//**
**
** DATA_MODEL_GetNumInstances
**
** Gets the number of instances of the specified object
** NOTE: You almost certainly want to call DATA_MODEL_GetInstances() instead,
**       unless you know the instances are contiguous and count from 1
**
** \param   path - path of the object. NOTE: This is not a schema path (ie no '{i}' in the path. Use a partial path).
** \param   num_instances - pointer to variable in which to store the number of instances
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetNumInstances(char *path, int *num_instances)
{
    dm_instances_t inst;
    dm_node_t *node;
    bool exists;
    bool is_qualified_instance;
    int err;

    // Exit if input params are invalid
    // This is necessary because this function (unlike some others in the data model)
    // is available to be called by the vendor hook functions
    if ((path==NULL) || (num_instances==NULL))
    {
        USP_ERR_SetMessage("ERROR: Invalid parameters passed into %s()", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if this is not a multi-instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Not a multi-instance object: %s", __FUNCTION__, path);
        return USP_ERR_OBJECT_NOT_CREATABLE;
    }

    // Exit if this object is a fully qualified instance
    if (is_qualified_instance)
    {
        USP_ERR_SetMessage("%s: Path (%s) should not contain instance number of object to get the number of instances of", __FUNCTION__, path);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Get the number of instances of the object represented by node
    err = DM_INST_VECTOR_GetNumInstances(node, &inst, num_instances);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetInstances
**
** Gets a vector of instances numbers for the specified object
**
** \param   path - path of the object. NOTE: This is not a schema path (ie no '{i}' in the path)
** \param   iv - pointer to structure in which to return the instance numbers
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetInstances(char *path, int_vector_t *iv)
{
    int err;
    dm_instances_t inst;
    dm_node_t *node;
    bool exists;
    bool is_qualified_instance;

    INT_VECTOR_Init(iv);

    // Exit if input params are invalid
    // This is necessary because this function (unlike some others in the data model)
    // is available to be called by the vendor hook functions
    if ((path==NULL) || (iv==NULL))
    {
        USP_ERR_SetMessage("ERROR: Invalid parameters passed into %s()", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if this is not a multi-instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_ERR_SetMessage("%s: Not a multi-instance object: %s", __FUNCTION__, path);
        return USP_ERR_NOT_A_TABLE;
    }

    // Exit if this object is already a fully qualified instance
    // This can occur if the path given to the path resolver has an instance number immediately followed by '*' or '[]' (ie path syntax error)
    if (is_qualified_instance)
    {
        USP_ERR_SetMessage("%s: Path (%s) should not contain an instance number followed by '*' or '[]'", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Exit if unable to determine whether the object instances in the path exist
    err = DM_INST_VECTOR_IsExist(&inst, &exists);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the object instances in the path do not exist
    if (exists == false)
    {
        USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Get the instances of the object represented by node
    err = DM_INST_VECTOR_GetInstances(node, &inst, iv);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetInstancePaths
**
** Returns a string vector containing the paths of all instances of the specified object
** NOTE: This code copes with being given a single qualified object instance (rather than a table)
**       by just returning that specific object instance in the string vector
**
** \param   path - path of the object. NOTE: This is not a schema path (ie no '{i}' in the path. Use a partial path).
** \param   sv - pointer to structure in which to return the paths to the instances
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetInstancePaths(char *path, str_vector_t *sv, combined_role_t *combined_role)
{
    int_vector_t iv;
    dm_instances_t inst;
    bool is_qualified_instance;
    dm_node_t *node;
    int err;
    int i;
    int instance;
    char buf[MAX_DM_PATH];
    unsigned short permission_bitmask;

    INT_VECTOR_Init(&iv);

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // Exit if there is no permission to read the instance numbers of this object
    permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
    if ((permission_bitmask & PERMIT_GET_INST)==0)
    {
        return USP_ERR_OK;
    }

    // Exit if this is not a multi-instance object, putting the single instance object in the returned string vector
    // NOTE: This case is not used when resolving add/delete object subscriptions, but is used for GetInstances
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        STR_VECTOR_Add_IfNotExist(sv, path);
        return USP_ERR_OK;
    }

    // Exit if this object is a fully qualified instance, putting it in the returned string vector
    if (is_qualified_instance)
    {
        STR_VECTOR_Add_IfNotExist(sv, path);
    }

    // Get an array of instances for this specific object
    err = DM_INST_VECTOR_GetInstances(node, &inst, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over all instances of this object
    for (i=0; i < iv.num_entries; i++)
    {
        // Form the path to this instance
        instance = iv.vector[i];
        USP_SNPRINTF(buf, sizeof(buf), "%s.%d", path, instance);
        STR_VECTOR_Add_IfNotExist(sv, buf);
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DATA_MODEL_GetAllInstancePaths
**
** Returns a string vector containing the paths of all instances of the specified object and recursively all child instances
**
** \param   path - path of the object. NOTE: This is not a schema path (ie no '{i}' in the path. Use a partial path).
** \param   sv - pointer to structure in which to return the paths to the instances
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_GetAllInstancePaths(char *path, str_vector_t *sv, combined_role_t *combined_role)
{
    dm_instances_t inst;
    dm_node_t *node;
    bool is_qualified_instance;
    int err;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    USP_ASSERT(IsObject(node));

    // If this is a qualified multi-instance node (ie ending in an instance number), then
    // return this object and the child objects that match this instance
    if ((node->type==kDMNodeType_Object_MultiInstance) && (is_qualified_instance))
    {
        err = DM_INST_VECTOR_GetAllInstancePaths_Qualified(&inst, sv, combined_role);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        return USP_ERR_OK;
    }

    // Find all child instances starting at this node
    err = GetAllInstancePathsRecursive(node, &inst, sv, combined_role);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_GetJSONParameterType
**
** Obtains the type of the specified parameter
** The type is denoted by a letter code: 'S'=strings and datetime, 'N'=number, 'B'=boolean
** NOTE: This function MUST only ever be called on parameter (not object) paths that have already been validated
**
** \param   path - full data model path of the parameter
**
** \return  type of the parameter as a character code
**
**************************************************************************/
char DATA_MODEL_GetJSONParameterType(char *path)
{
    dm_node_t *node;
    unsigned type_flags;
    char type;

    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    USP_ASSERT(node != NULL);  // because the path we queried was generated by the path resolver, so we expect it to exist
    USP_ASSERT( ((node->type != kDMNodeType_Object_MultiInstance) &&
                 (node->type != kDMNodeType_Object_SingleInstance) &&
                 (node->type != kDMNodeType_SyncOperation) &&
                 (node->type != kDMNodeType_AsyncOperation) &&
                 (node->type != kDMNodeType_Event)) );

    // Calculate the type of this parameter
    type_flags = node->registered.param_info.type_flags;

#ifdef REPRESENT_JSON_NUMBERS_WITH_FULL_PRECISION
    if (type_flags & (DM_INT | DM_LONG))
    {
        type = 'L';
    }
    else if (type_flags & (DM_UINT | DM_ULONG))
    {
        type = 'U';
    }
    else if (type_flags & DM_DECIMAL)
    {
        type = 'N';
    }
#else
    if (type_flags & (DM_INT | DM_UINT | DM_ULONG | DM_DECIMAL | DM_LONG))
    {
        type = 'N';
    }
#endif
    else if (type_flags & DM_BOOL)
    {
        type = 'B';
    }
    else
    {
        // Default, and also for DM_STRING, DM_DATETIME, DM_BASE64, DM_HEXBIN
        type = 'S';
    }

    return type;
}

/*********************************************************************//**
**
** DATA_MODEL_SetParameterInDatabase
**
** Function to set a parameter directly in the database, ignoring all vendor hooks
** This function is used by the dbset CLI command and also when resetting the database programatically
**
** \param   path - data model path of the parameter
** \param   value - value to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_SetParameterInDatabase(char *path, char *value)
{
    int err;
    dm_hash_t hash;
    char instances[MAX_DM_PATH];
    unsigned path_flags;
    unsigned db_flags;

    // Exit if parameter path is incorrect
    err = DM_PRIV_FormDB_FromPath(path, &hash, instances, sizeof(instances));
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Determine if this parameter is secure and hence whether the database needs to obfuscate the value
    path_flags = DATA_MODEL_GetPathProperties(path, INTERNAL_ROLE, NULL, NULL, NULL, 0);
    db_flags = (path_flags & PP_IS_SECURE_PARAM) ? OBFUSCATED_VALUE : 0;

    // Exit if unable to set value of parameter in DB
    err = DATABASE_SetParameterValue(path, hash, instances, value, db_flags);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DATA_MODEL_FindUnusedGroupId
**
** Returns the first unregistered group_id
**
** \param   None
**
** \return  First group_id that has not been registered yet, or INVALID if no more free group_ids
**
**************************************************************************/
int DATA_MODEL_FindUnusedGroupId(void)
{
    int i;
    group_vendor_hook_t *gvh;

    for (i=0; i<MAX_VENDOR_PARAM_GROUPS; i++)
    {
        gvh = &group_vendor_hooks[i];
        if (gvh->get_group_cb == NULL)
        {
            return i;
        }
    }

    return INVALID;
}

/*********************************************************************//**
**
** DATA_MODEL_DeRegisterPath
**
** Removes the specified schema path and all child nodes from the data model schema,
** freeing all memory and unlinking from internal data structures
**
** \param   schema_path - data model schema path to de-register
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DATA_MODEL_DeRegisterPath(char *schema_path)
{
    dm_node_t *node;
    dm_node_t *parent;

    // Exit if node is not known
    node = DM_PRIV_GetNodeFromPath(schema_path, NULL, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if node is nested more than one instance deep
    // These cannot be removed by calling DestroySchemaRecursive because this would not remove the top level node in the nest
    // (containing the instance vector) and hence the instance vector would still contain pointers to this node
    if (node->order > 1)
    {
        USP_ERR_SetMessage("%s: Cannot remove '%s' as it is nested multi-instance", __FUNCTION__, schema_path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Remove the specified node and all nodes underneath it in the data model
    parent = node->parent_node;
    DestroySchemaRecursive(node);

    // Remove any parent nodes of the node which has been deleted, if they now have no children
    // This is needed to cope correctly with the case of ServiceA registering Device.Common.ObjectA and ServiceB registering Device.Common.ObjectB
    // When both services disconnect, the Common object needs to also be deleted from the data model
    node = parent;
    while (node->child_nodes.head == NULL)
    {
        parent = node->parent_node;

        DestroyNode(node);
        node = parent;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_InitSetRequest
**
** Fills in the dm_req_t structure for a Parameter set, converting the new_value from a string to it's native type
**
** \param   req - pointer to structure identifying the parameter
** \param   node - pointer to node representing the parameter or object
** \param   path - pointer to string containing complete data model path to the parameter or object
** \param   inst - pointer to instances structure
**
** \return  USP_ERR_OK if new_value converted successfully
**
**************************************************************************/
int DM_PRIV_InitSetRequest(dm_req_t *req, dm_node_t *node, char *path, dm_instances_t *inst, char *new_value)
{
    int err = USP_ERR_OK;   // Default for DM_STRING
    unsigned type_flags;
    char *type_name;
    char *range = "";

    DM_PRIV_RequestInit(req, node, path, inst);

    // If the value is not a string, then convert it to its native type
    // NOTE: DM_HEXBIN and DM_BASE64 are treated as strings (carried in buf)
    type_flags = node->registered.param_info.type_flags;
    if (type_flags & DM_DATETIME)
    {
        type_name = "DateTime";
        err = TEXT_UTILS_StringToDateTime(new_value, &req->val_union.value_datetime);
        range = " within the range 1970-2037 (or the UNKNOWN_TIME)";
    }
    else if (type_flags & DM_BOOL)
    {
        type_name = "boolean";
        err = TEXT_UTILS_StringToBool(new_value, &req->val_union.value_bool);
    }
    else if (type_flags & DM_INT)
    {
        type_name = "int32";
        err = TEXT_UTILS_StringToInteger(new_value, &req->val_union.value_int);
    }
    else if (type_flags & DM_UINT)
    {
        type_name = "uint32";
        err = TEXT_UTILS_StringToUnsigned(new_value, &req->val_union.value_uint);
    }
    else if (type_flags & DM_ULONG)
    {
        type_name = "ulong";
        err = TEXT_UTILS_StringToUnsignedLongLong(new_value, &req->val_union.value_ulong);
    }
    else if (type_flags & DM_DECIMAL)
    {
        type_name = "decimal";
        err = TEXT_UTILS_StringToDouble(new_value, &req->val_union.value_decimal);
    }
    else if (type_flags & DM_LONG)
    {
        type_name = "long";
        err = TEXT_UTILS_StringToLongLong(new_value, &req->val_union.value_long);
    }

    // Set a more useful error message containing the name of the parameter in error
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Expected a %s%s for param=%s (got '%s')", __FUNCTION__, type_name, range, req->path, new_value);
    }

    return err;
}

/*********************************************************************//**
**
** DM_PRIV_RequestInit
**
** Fills in the dm_req_t structure which is passed to all vendor hooks
**
** \param   req - pointer to structure identifying the parameter
** \param   node - pointer to node representing the parameter or object
** \param   path - pointer to string containing complete data model path to the parameter or object
** \param   inst - pointer to instances structure
**
** \return  None
**
**************************************************************************/
void DM_PRIV_RequestInit(dm_req_t *req, dm_node_t *node, char *path, dm_instances_t *inst)
{
    req->path = path;
    req->schema_path = node->path;
    req->inst = (dm_req_instances_t *) inst;
    memset(&req->val_union, 0, sizeof(req->val_union));
    req->group_id = node->group_id;
}

/*********************************************************************//**
**
** DM_PRIV_CalcHashFromPath
**
** Returns the hash associated with the given instantiated data model path and extracts instance numbers
** NOTE: Does not check that the path exists in the supported data model, so this function allows the hash
**       of legacy paths that are not now present in the data model to be calculated
**
** \param   path - full data model path of the parameter or object to calculate the hash of
**                 This may be an instantiated or schema path or may use wildcards instead of instance numbers (if so, inst and is_qualified_instance must be NULL).
** \param   inst - pointer to instances structure, filled in from parsing the path
**                 NOTE: This parameter may be NULL if instances are not required
**
** \param   p_hash - pointer to variable in which to return the calculated hash
** \param   flags - bitmask of options controlling execution (eg DONT_LOG_ERRORS)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_PRIV_CalcHashFromPath(char *path, dm_instances_t *inst, dm_hash_t *p_hash, unsigned flags)
{
    dm_hash_t hash = OFFSET_BASIS;
    char c;
    char *p;
    char *p_num;
    char t;
    int num_digits;
    unsigned number;

    // Setup default return values
    // NOTE: It is important that this structure does not contain uninitialised values, otherwise more than one entry may be added to the DM_INST_VECTOR
    if (inst != NULL)
    {
        memset(inst, 0, sizeof(dm_instances_t));
    }

    // Iterate over all characters in the path, calculating the hash of the schema version of the instantiated path,
    // and extracting all instance numbers
    p = path;
    c = *p++;
    while (c != '\0')
    {
        // If hit a path segment separator...
        if (c == '.')
        {
            // Ignore consecutive dots in the path
            // This makes it easier to cope with pathnames which have been formed by concatenating an object and parameter name
            // where the object name, may or may not contain a trailing dot. For example: Subscription Recipient
            p_num = p;
            t = *p_num++;
            if (t == '.')
            {
                while (t == '.')    // On exiting this loop, t contains the character after the last consecutive dot
                {
                    t = *p_num++;
                }
                p = p_num - 1;      // Leave p pointing to the character after the last consecutive dot
            }

            // Exit loop if path contains a trailing path segment separator (these are not included in the hash)
            if (t == '\0')
            {
                break;
            }

            // Determine the number of digits, if the following path segment is a number
            // Also calculate the instance number in the path segment
            num_digits = 0;
            number = 0;
            while (IS_NUMERIC(t))
            {
                number = 10*number + t - '0';
                num_digits++;
                t = *p_num++;
            }

            // If the path segment is purely an instance number...
            if ((num_digits > 0) && ((t == '.') || (t == '\0')))
            {
                // Store the instance number in the dm_instances_t structure
                if (inst != NULL)
                {
                    if (inst->order == MAX_DM_INSTANCE_ORDER)
                    {
                        if ((flags & DONT_LOG_ERRORS)==0)
                        {
                            USP_ERR_SetMessage("%s: More than %d instance numbers in path", __FUNCTION__, MAX_DM_INSTANCE_ORDER);
                        }
                        return USP_ERR_INTERNAL_ERROR;
                    }
                    inst->instances[ inst->order ] = number;
                    inst->order++;
                }

                // Add schema instance separator to hash, instead of the instance number in the path segment
                ADD_TO_HASH('.', hash);
                ADD_TO_HASH('{', hash);
                ADD_TO_HASH('i', hash);
                ADD_TO_HASH('}', hash);

                // Skip to after instance number
                p += num_digits;
                c = *p++;
                continue;
            }
        }
        else if (c == '*')
        {
            // Add schema instance separator to hash, instead of the wildcard in the path segment
            ADD_TO_HASH('{', hash);
            ADD_TO_HASH('i', hash);
            ADD_TO_HASH('}', hash);
            c = *p++;
            continue;
        }

        // Otherwise add the current character to the hash
        ADD_TO_HASH(c, hash);
        c = *p++;
    }

    *p_hash = hash;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_GetNodeFromPath
**
** Returns the node associated with the given instantiated data model path
** NOTE: Checks that path is specified with the correct number of {i} instance numbers in it
** NOTE: Does not check that the instance numbers are present in the data model
**
** \param   path - full data model path of the parameter or object to return the node of
**                 This may be an instantiated or schema path or may use wildcards instead of instance numbers (if so, inst and is_qualified_instance must be NULL).
** \param   inst - pointer to instances structure, filled in from parsing the path
**                 NOTE: This parameter may be NULL if instances are not required, but only if is_qualified_instance is NULL too
**
** \param   is_qualified_instance - Pointer to boolean in which to return whether the instances
**                                  structure contains all instance numbers for the parameter/object being addressed
**                                  It will be false only if the path represents a multi-instance object
**                                  without instance number (unqualified)
**                                  NOTE: This parameter may be NULL if checking is not required
** \param   flags - bitmask of options controling execution (eg DONT_LOG_ERRORS)
**
** \return  pointer to node, or NULL if matching node not found or specified object instance is not present
**          NOTE: Sets USP error message if path is in error
**
**************************************************************************/
dm_node_t *DM_PRIV_GetNodeFromPath(char *path, dm_instances_t *inst, bool *is_qualified_instance, unsigned flags)
{
    dm_hash_t hash;
    dm_node_t *node;
    int err;

    // Exit if unable to calculate the hash for the path
    err = DM_PRIV_CalcHashFromPath(path, inst, &hash, flags);
    if (err != USP_ERR_OK)
    {
        return NULL;
    }

    // If unable to find the node using the given path...
    node = FindNodeFromHash(hash);
    if (node == NULL)
    {
        // Then try again using a qualified instance schema path
        // NOTE: This is necessary because this function is sometimes called with a path representing an
        // unqualified multi-instance object. But the hash must be of a qualified multi-instance schema path
        ADD_TO_HASH('.', hash);
        ADD_TO_HASH('{', hash);
        ADD_TO_HASH('i', hash);
        ADD_TO_HASH('}', hash);

        // Exit if unable to find the node
        node = FindNodeFromHash(hash);
        if (node == NULL)
        {
            if ((flags & DONT_LOG_ERRORS)==0)
            {
                USP_ERR_SetMessage("%s: Path is invalid: %s", __FUNCTION__, path);
            }
            return NULL;
        }
    }

    // Calculate 'is_qualified_instance' (if required)
    // Check that the object instance order in the path is correct
    // This is complicated by the fact that MultiInstanceObjects may be specified without the trailing instance (ie. unqualified) for some operations
    if (is_qualified_instance != NULL)
    {
        if (inst->order == node->order)
        {
            *is_qualified_instance = true;
        }
        else
        {
            // Only multi-instance objects are allowed to be specified unqualified
            if (node->type != kDMNodeType_Object_MultiInstance)
            {
                if ((flags & DONT_LOG_ERRORS)==0)
                {
                    USP_ERR_SetMessage("%s: Path %s does not have the right number of '{i}' instances (got %d, expected %d)", __FUNCTION__, path, inst->order, node->order);
                }
                return NULL;
            }
            else
            {
                if (inst->order == node->order-1)
                {
                    *is_qualified_instance = false;
                }
                else
                {
                    if ((flags & DONT_LOG_ERRORS)==0)
                    {
                        USP_ERR_SetMessage("%s: Path %s does not have the right number of '{i}' instances (got %d, expected %d)", __FUNCTION__, path, inst->order, node->order);
                    }
                    return NULL;
                }
            }
        }
    }

    // Copy the nodes associated with each multi-instance object into the instances structure (if required)
    if (inst != NULL)
    {
        if (inst->order > 0)
        {
            memcpy(inst->nodes, node->instance_nodes, (inst->order)*sizeof(dm_node_t *));
        }
    }

    // NOTE: We do not validate that the parsed instances actually exist in the data model in this function
    //       This is because for a SetParameterValues and AddObject, the instance might not yet exist

    return node;
}

/*********************************************************************//**
**
** DM_PRIV_AddSchemaPath
**
** Allocates and initialises all data model nodes in the given path
** Returning a pointer to the last node in the path
** NOTE: If the given path already exists in the data model, this function will return an error
**       unless flags==SUPPRESS_PRE_EXISTANCE_ERR, which instead can be used to check whether the node exists in the schema or not
**
** \param   path - full data model path of the parameter or object to create
** \param   type - type of the last node in the path (eg object or parameter)
** \param   flags - options to control execution of this function (eg SUPPRESS_PRE_EXISTANCE_ERR)
**
** \return  pointer to created node, or NULL if out of memory
**
**************************************************************************/
dm_node_t *DM_PRIV_AddSchemaPath(char *path, dm_node_type_t type, unsigned flags)
{
    dm_node_t *parent = NULL; // This pointer walks through the data model tree
    dm_node_t *child;         // This pointer walks through the children of the parent node
    dm_path_segment segments[MAX_PATH_SEGMENTS];
    int num_segments;
    char path_segments[MAX_DM_PATH];
    char schema_path[MAX_DM_PATH];  // This string is built up to traverse schema path to each node
    dm_instances_t inst;        // NOTE: This function only makes use of the node aspect of this structure
    dm_path_segment *seg;
    int i;
    bool check_node_type = true;    // Used only with the last node in the path if the last node was previously registered. Determines whether to check the type of the last node
    bool override_node_type = false;// Used only with the last node in the path if the last node was previously registered. Determines whether to override the type and instance node array of the last node
    bool is_object;

    // Exit if there were too many or not enough segments in the path
    num_segments = ParseSchemaPath(path, path_segments, sizeof(path_segments), type, segments, MAX_PATH_SEGMENTS);
    if (num_segments < 1)
    {
        return NULL;
    }

    // Exit if first segment was not one of the root data model nodes
    if (strcmp(segments[0].name, root_device_node->name) == 0)
    {
        parent = root_device_node;
    }
    else if (strcmp(segments[0].name, root_internal_node->name) == 0)
    {
        parent = root_internal_node;
    }
    else
    {
        USP_ERR_SetMessage("%s: Invalid path %s", __FUNCTION__, path);
        return NULL;
    }
    strcpy(schema_path, segments[0].name);

    // Iterate over segments, using them to traverse the data model tree
    inst.order = 0;
    for (i=1; i<num_segments; i++)
    {
        seg = &segments[i];
        check_node_type = true;

        // Update the schema path to this node
        strcat(schema_path, ".");
        strcat(schema_path, seg->name);
        if (seg->type == kDMNodeType_Object_MultiInstance)
        {
            strcat(schema_path, ".{i}");
        }

        // See if child exists in the data model
        child = DM_PRIV_FindMatchingChild(parent, seg->name);
        if (child == NULL)
        {
            // Do not allow tables to be registered implicitly by a parameter. Only allow them to be registered explicitly.
            // Only non-table objects are registered implicitly
            if ((seg->type == kDMNodeType_Object_MultiInstance) && (i != num_segments-1))
            {
                USP_ERR_SetMessage("%s: %s must be registered before %s", __FUNCTION__, schema_path, path);
                return NULL;
            }

            // Node has not yet been added, so add it
            child = CreateNode(seg->name, seg->type, schema_path);
            if (child == NULL)
            {
                return NULL;
            }

            // Add the node to it's parent, ensuring that all child parameters are placed before all child objects
            // This prevents the parameters being separated in different resolved_path results (of a GetResponse), if RESOLVED_PATH_SEARCH_LIMIT is exceeded
            // This code also ensures that the order of registering the nodes is respected for each type in the child list
            child->parent_node = parent;
            is_object = IsObject(child);
            if (is_object)
            {
                DLLIST_LinkToTail(&parent->child_nodes, child);
            }
            else
            {
                double_link_t *insert_point;
                insert_point = FindLinkToFirstObject(&parent->child_nodes);
                if (insert_point != NULL)
                {
                    DLLIST_InsertLinkBefore(insert_point, &parent->child_nodes, child);
                }
                else
                {
                    DLLIST_LinkToTail(&parent->child_nodes, child);
                }
            }

            // Add this node to the instance node array, if it is a multi-instance object
            if (seg->type == kDMNodeType_Object_MultiInstance)
            {
                inst.nodes[ inst.order ] = child;
                inst.order++;
            }

            // Save the instance nodes for this object
            memcpy(child->instance_nodes, &inst.nodes, inst.order*sizeof(dm_node_t *));
            child->order = inst.order;
        }
        else
        {
            // If code gets here, the current child node already exists in the data model

            // Special considerations for the last node in the path
            if (i == num_segments-1)
            {
                // Exit with an error, if we're not expecting the last node to exist
                if ((flags & (SUPPRESS_PRE_EXISTANCE_ERR | OVERRIDE_LAST_TYPE)) == 0)
                {
                    USP_ERR_SetMessage("%s: Path %s already exists in schema", __FUNCTION__, path);
                    return NULL;
                }

                // Determine whether to type check the last node in the path
                if (flags & (SUPPRESS_LAST_TYPE_CHECK | OVERRIDE_LAST_TYPE))
                {
                    check_node_type = false;
                }

                // Determine whether to override the type of the last node in the path
                if (flags & OVERRIDE_LAST_TYPE)
                {
                    override_node_type = true;
                }
            }

            // Check that it's type (from the path) matches that expected
            if ((check_node_type) && (child->type != seg->type))
            {
                USP_ERR_SetMessage("%s: Path segment '%s' expected type %s in path %s", __FUNCTION__, child->name, dm_node_type_to_str[child->type], path);
                return NULL;
            }

            // Add this node to the instance node array, if it is a multi-instance object
            if (seg->type == kDMNodeType_Object_MultiInstance)
            {
                inst.nodes[ inst.order ] = child;
                inst.order++;
            }

            if (override_node_type)
            {
                OverrideNodeType(child, type, schema_path, &inst);
            }
            else
            {
                // Check that the number of instance separators in the path to the node matches that expected
                if (child->order != inst.order)
                {
                    USP_ERR_SetMessage("%s: Path segment '%s' expected order of %d in path %s", __FUNCTION__, child->name, inst.order, path);
                    return NULL;
                }
            }

        }

        // Found the child matching the segment, so move to the child, and search for next segment
        parent = child;
    }

    // If the code gets here, then all segments have been traversed in the data model
    return parent;
}

/*********************************************************************//**
**
** DM_PRIV_FormInstantiatedPath
**
** Forms a data model instantiated path string from data model schema path and parsed instance array
** This function replaces each '{i}' in the schema_path with the relevant instance number from the inst structure
**
** \param   schema_path - supported data model path to interpolate with instance numbers
** \param   inst - pointer to instance structure specifying the instances in the path
**                 NOTE: Only the instance numbers are used in this structure, the nodes are not
** \param   buf - pointer to buffer in which to store the parameter/object
** \param   len - length of buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_PRIV_FormInstantiatedPath(char *schema_path, dm_instances_t *inst, char *buf, int len)
{
    char *src;
    char *dest;
    int i;      // iterates through the dm_instances array when '{i}' is encountered in the schema path
    int num_chars_written;

    // Iterate over the schema path, copying it to the buffer, but replacing '{i}' with instance numbers
    i = 0;
    *buf = '\0';
    src = schema_path;
    dest = buf;
    while (*src != '\0')
    {
        if (strncmp(src, MULTI_SEPARATOR, sizeof(MULTI_SEPARATOR)-1) == 0)
        {
            // Exit if the number of '{i}' separators in this string exceed the instance numbers in the array
            if (i == inst->order)
            {
                USP_ERR_SetMessage("%s: Cannot form instantiated path for %s as only %d instance numbers provided", __FUNCTION__, schema_path, inst->order);
                *buf = '\0';
                return USP_ERR_INTERNAL_ERROR;
            }

            // Replace the {i} separator with the instance number from the array
            num_chars_written = USP_SNPRINTF(dest, len, "%d", inst->instances[i]);
            dest += num_chars_written;
            len -= num_chars_written;
            src += sizeof(MULTI_SEPARATOR)-1;
            i++;
        }
        else
        {
            // Copy the source path to the destination
            if (len > 1)
            {
                *dest++ = *src++;
                len--;
            }
        }
    }

    // Exit if not all instance numbers have not been consumed
    if (i != inst->order)
    {
        *buf = '\0';
        USP_ERR_SetMessage("%s: Cannot form instantiated path for %s as too many instance numbers provided (%d)", __FUNCTION__, schema_path, inst->order);
        return USP_ERR_INTERNAL_ERROR;
    }


    *dest = '\0';   // ensures that buffer is always zero terminated
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_FormDB_FromPath
**
** Forms the hash and instance string of the specified parameter path
** This function is called by the 'dbset' and 'dbget' CLI commands
** NOTE: This function is not intended to support objects, as they are not represented in the database directly)
**
** \param   path - path to parameter in the data model
** \param   hash - pointer to variable in which to store the hash identifying the data model parameter
** \param   instances - pointer to buffer to return a string containing the instance numbers of the multi-instance objects in the path
** \param   len - length of the buffer
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if the parameter does not exist in the data model or the
**          instance numbers are not correct (invalid, too many or not enough for the object's path)
**
**************************************************************************/
int DM_PRIV_FormDB_FromPath(char *path, dm_hash_t *hash, char *instances, int len)
{
    dm_node_t *node;
    dm_instances_t inst;
    dm_hash_t migrated_hash;
    dm_node_t *migrated_node;

    // Exit if parameter does not exist in the data model
    // or parameter is specified with incorrect instance order
    node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // If the path has been migrated to a new path, then determine the new node to use in the database
    // This code ensures that factory reset databases set using dbset commands or '-r' option seed the new parameters in the USP DB
    migrated_hash = (dm_hash_t) DATABASE_GetMigratedHash((db_hash_t) node->hash);
    if (migrated_hash != node->hash)
    {
        // Exit if parameter does not exist in the data model
        migrated_node = FindNodeFromHash(migrated_hash);
        if (migrated_node == NULL)
        {
            USP_ERR_SetMessage("%s: Migrated Parameter (hash=%d) does not exist in the data model schema", __FUNCTION__, migrated_hash);
            return USP_ERR_INVALID_PATH;
        }
        USP_LOG_Warning("%s: WARNING: '%s' is deprecated, please use '%s' instead", __FUNCTION__, node->path, migrated_node->path);
        node = migrated_node;
    }

    // Exit if path is not to a parameter
    if (IsParam(node)==false)
    {
        USP_ERR_SetMessage("%s: Path (%s) is an object. Expecting a parameter.", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    // Exit if path is not to a database parameter
    if (IsDbParam(node)==false)
    {
        USP_ERR_SetMessage("%s: Parameter (%s) is not stored in the database.", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    *hash = node->hash;
    FormInstanceString(&inst, instances, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_FormPath_FromDB
**
** Forms a data model path string from hash and instance string
** This function is called by the database code to dump the contents of the database
**
** \param   hash - hash identifying data model parameter
** \param   instances - string containing the instance numbers of the multi-instance objects in the path of the parameter
** \param   buf - pointer to buffer in which to store the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if the parameter does not exist in the data model or the
**          instance numbers are not correct (invalid, too many or not enough for the object's path)
**
**************************************************************************/
int DM_PRIV_FormPath_FromDB(dm_hash_t hash, char *instances, char *buf, int len)
{
    dm_node_t *node;
    dm_instances_t inst;
    int err;

    // Exit if parameter does not exist in the data model
    node = FindNodeFromHash(hash);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: Parameter (hash=%d) does not exist in the data model schema", __FUNCTION__, hash);
        return USP_ERR_INVALID_PATH;
    }

    // Exit if unable to parse the instance numbers from the string
    memset(&inst, 0, sizeof(inst));
    err = DM_PRIV_ParseInstanceString(instances, &inst);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Instance numbers ('%s') for hash=%d are invalid", __FUNCTION__, instances, hash);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the number of object instances in this string do not match the data model schema
    if (inst.order != node->order)
    {
        USP_ERR_SetMessage("%s: Number of instance numbers ('%s') for hash=%d does not match the number expected (%d)", __FUNCTION__, instances, hash, node->order);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to form the instantiated path
    err = DM_PRIV_FormInstantiatedPath(node->path, &inst, buf, len);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_FindMatchingChild
**
** Finds the data model child node matching the specified name, given a parent node
**
** \param   parent - pointer to data model node to find child node for
** \param   name - name of child node
**
** \return  pointer to matching child node, or NULL if no match was found
**
**************************************************************************/
dm_node_t *DM_PRIV_FindMatchingChild(dm_node_t *parent, char *name)
{
    dm_node_t *child;

    // Iterate over list of children, seeing if any match
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        if (strcmp(child->name, name)==0)
        {
            // Found a match
            return child;
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** DM_PRIV_AddUniqueKey
**
** Adds a unique key to the vector of unique keys for a table
**
** \param   node - pointer to multi-instance object in data model to register the unique key with
** \param   unique_key - pointer to structure representing the unique key to add
**
** \return  None
**
**************************************************************************/
void DM_PRIV_AddUniqueKey(dm_node_t *node, dm_unique_key_t *unique_key)
{
    int new_num_entries;
    dm_unique_key_vector_t *ukv;

    USP_ASSERT(node->type == kDMNodeType_Object_MultiInstance);
    ukv = &node->registered.object_info.unique_keys;
    new_num_entries = ukv->num_entries + 1;
    ukv->vector = USP_REALLOC(ukv->vector, new_num_entries*sizeof(dm_unique_key_t));

    memcpy(&ukv->vector[ ukv->num_entries ], unique_key, sizeof(dm_unique_key_t));
    ukv->num_entries = new_num_entries;
}

/*********************************************************************//**
**
** DM_PRIV_ApplyPermissions
**
** Applies the specified permission to this node and all of it's children
** NOTE: This function is recursive
**
** \param   node - Node to apply permissions to. If set to NULL, this imples that permissions must be set from the root device node
** \param   role_index - role to apply permissions to
** \param   permission_bitmask - bitmask of permissions to apply
**
** \return  None
**
**************************************************************************/
void DM_PRIV_ApplyPermissions(dm_node_t *node, int role_index, unsigned short permission_bitmask)
{
    dm_node_t *child;

    // Apply special case of passing in NULL, to denote the root_device_node
    if (node == NULL)
    {
        node = root_device_node;
    }

    // Apply permissions to this node
    node->permissions[role_index] = permission_bitmask;

    // Iterate over list of children
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        // Apply permissions to child node
        child->permissions[role_index] = permission_bitmask;

        // Apply permissions to all children of the child node
        if (child->child_nodes.head != NULL)
        {
            DM_PRIV_ApplyPermissions(child, role_index, permission_bitmask);
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** DM_PRIV_ReRegister_DBParam_Default
**
** Registers a new default value for a USP DB parameter
**
** \param   path - full data model path for the parameter
** \param   value - new default value for the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_PRIV_ReRegister_DBParam_Default(char *path, char *value)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if parameter does not exist in the data model
    // or parameter is specified with incorrect instance order
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // Exit if path is not to a parameter
    if (IsParam(node)==false)
    {
        USP_ERR_SetMessage("%s: Path (%s) is an object. Expecting a parameter.", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    // Exit if path is not to a database parameter
    if (IsDbParam(node)==false)
    {
        USP_ERR_SetMessage("%s: Parameter (%s) is not stored in the database.", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Free the old default value and replace it with the new one
    info = &node->registered.param_info;
    USP_SAFE_FREE(info->default_value);
    info->default_value = USP_STRDUP(value);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_GetPermissions
**
** Returns the permissions for the specified data model node, given the specified role
**
** \param   node - Node to get permissions for
** \param   combined_role - role to get permissions for.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  Permissions bitmask associated with the specified node and role
**
**************************************************************************/
unsigned short DM_PRIV_GetPermissions(dm_node_t *node, combined_role_t *combined_role)
{
    unsigned short permissions = 0;
    int role_index;

    // If using the internal role, then this overrides all permissions setup and permits all
    // This is necessary because at startup the permission bitmask in the data model is not setup, but we still need to ensure that we can do everything
    if (combined_role == INTERNAL_ROLE)
    {
        return PERMIT_ALL;
    }

    // Add permissions from inherited role
    role_index = combined_role->inherited_index;
    if ((role_index < MAX_CTRUST_ROLES) && (role_index != INVALID))
    {
        permissions |= node->permissions[ role_index ];
    }

    // Add permissions from assigned role
    role_index = combined_role->assigned_index;
    if ((role_index < MAX_CTRUST_ROLES) && (role_index != INVALID))
    {
        permissions |= node->permissions[ role_index ];
    }

    return permissions;
}

/*********************************************************************//**
**
** DM_PRIV_ParseInstanceString
**
** Parses a string containing the instance numbers into the inst structure
** eg An instance string of "1.5" would return instances 1 and 5 in the inst array
**
** \param   instances - pointer to string containing instances to parse
** \param   inst - pointer to instances structure in which to return the parsed object instances
**
** \return  USP_ERR_OK if successful, USP_ERR_INTERNAL_ERROR otherwise
**
**************************************************************************/
int DM_PRIV_ParseInstanceString(char *instances, dm_instances_t *inst)
{
    char *p;
    int value;

    // Clear instances structure
    memset(inst, 0, sizeof(dm_instances_t));

    // Exit if instance string is empty
    if ((instances == NULL) || (*instances == '\0'))
    {
        return USP_ERR_OK;
    }

    // Iterate over all instance numbers in the string
    p = instances;
    while (*p != '\0')
    {
        // Exit if an error in parsing the next integer in the instance string
        p = ParseInstanceInteger(p, &value);
        if (p == NULL)
        {
            return USP_ERR_INTERNAL_ERROR;
        }

        // Store this instance number in the array
        inst->instances[ inst->order++ ] = value;
    }

    // If the code gets here, the instances string was parsed successfully
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_PRIV_IsChildOf
**
** Determines whether the specified path is a child of the specified parent node
**
** \param   path - data model path to determine whether it is a child
** \param   parent_node - data model node of parent to test against
**
** \return  true if the specified path is a child of the specified parent node, false otherwise
**
**************************************************************************/
bool DM_PRIV_IsChildOf(char *path, dm_node_t *parent_node)
{
    dm_node_t *node;

    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    if (node == NULL)
    {
        return false;
    }

    return DM_PRIV_IsChildNodeOf(node, parent_node);
}

/*********************************************************************//**
**
** DM_PRIV_IsChildNodeOf
**
** Determines whether the specified node is a child of the specified parent node
**
** \param   node - data model node to test whether it's a child
** \param   parent_node - data model node of parent to test against
**
** \return  true if the specified path is a child of the specified parent node, false otherwise
**
**************************************************************************/
bool DM_PRIV_IsChildNodeOf(dm_node_t *node, dm_node_t *parent_node)
{
    // Traverse the data model tree upwards, seeing if any parents match the specified parent
    while (node->parent_node != NULL)
    {
        // Exit if the path had the specified parent node as a parent
        if (node->parent_node == parent_node)
        {
            return true;
        }

        // Move up the data model tree to the next parent
        node = node->parent_node;
    }

    return false;
}

/*********************************************************************//**
**
** DM_PRIV_AreAllChildrenGroupId
**
** Recursively determines whether all child nodes have the specified group_id (ie are owned by the same data model provider component)
** NOTE: This function intentionally does not check that the node itself has the specified group_id (as it is used for cases when this is not the case)
**
** \param   parent - data model node to check all children of
** \param   group_id - group_id to check that all children match
**
** \return  true all child nodes match the specified group_id
**
**************************************************************************/
bool DM_PRIV_AreAllChildrenGroupId(dm_node_t *parent, int group_id)
{
    dm_node_t *child;

    // Iterate over list of children
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        // Exit if this child does not match the specified group_id
        if (child->group_id != group_id)
        {
            return false;
        }

        // Exit if all children of this child do not match the specified group_id
        if (DM_PRIV_AreAllChildrenGroupId(child, group_id) == false)
        {
            return false;
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }

    // If the code gets here, then all children matched the specified group_id, or the node did not have any children
    return true;
}

/*********************************************************************//**
**
** GetAllInstancePathsRecursive
**
** Finds all child object instances from the specified node
**
** \param   node - pointer to node to recursively find all child object instances of
** \param   inst - pointer to instance structure specifying the object's parents and their instance numbers
** \param   sv - pointer to structure in which to return the paths to the instances
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetAllInstancePathsRecursive(dm_node_t *node, dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role)
{
    dm_node_t *child;
    int err;

    // Exit if this is a multi-instance node, adding all child instances below this node to the results
    if (node->type == kDMNodeType_Object_MultiInstance)
    {
        err = DM_INST_VECTOR_GetAllInstancePaths_Unqualified(node, inst, sv, combined_role);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        return USP_ERR_OK;
    }

    // Recurse over all child nodes, trying to find all next level multi-instance nodes
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        err = GetAllInstancePathsRecursive(child, inst, sv, combined_role);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        child = (dm_node_t *) child->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SerializeNativeValue
**
** If val_union was modified by the Get vendor hook (instead of the buffer), then write the returned value into the buffer
** This function facilitates the vendor hook returning a parameter's value in native format seamlessly
**
** \param   req - pointer to structure identifying the parameter
** \param   node - pointer to node representing the parameter or object
** \param   buf - pointer to buffer into which to return the value contained in req->val_union (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  Permissions bitmask associated with the specified node and role
**
**************************************************************************/
void SerializeNativeValue(dm_req_t *req, dm_node_t *node, char *buf, int len)
{
    unsigned type_flags;

    type_flags = node->registered.param_info.type_flags;

    // Exit if the type of the parameter is a string. Strings are always returned using the buffer.
    if (type_flags & DM_STRING)
    {
        return;
    }

    // Exit if the vendor has written a value to the buffer. We will use the value in the buffer, not the native value.
    if (*buf != '\0')
    {
        return;
    }

    // Convert the string to its native type
    if (type_flags & DM_DATETIME)
    {
        iso8601_from_unix_time(req->val_union.value_datetime, buf, len);
    }
    else if (type_flags & DM_BOOL)
    {
        USP_SNPRINTF(buf, len, "%s", TEXT_UTILS_BoolToString(req->val_union.value_bool) );
    }
    else if (type_flags & DM_INT)
    {
        USP_SNPRINTF(buf, len, "%d", req->val_union.value_int);
    }
    else if (type_flags & DM_UINT)
    {
        USP_SNPRINTF(buf, len, "%u", req->val_union.value_uint);
    }
    else if (type_flags & DM_ULONG)
    {
        USP_SNPRINTF(buf, len, "%llu", req->val_union.value_ulong);
    }
    else if (type_flags & DM_DECIMAL)
    {
        USP_SNPRINTF(buf, len, "%lf", req->val_union.value_decimal);
    }
    else if (type_flags & DM_LONG)
    {
        USP_SNPRINTF(buf, len, "%lld", req->val_union.value_long);
    }
}

/*********************************************************************//**
**
** FormInstanceString
**
** Forms a string containing the instance numbers which have previously been parsed into the inst structure
** eg Device.WiFi.EndPoint.1.Profile.5.Enable would have an instance string of "1.5"
**
** \param   inst - pointer to instances structure
** \param   buf - pointer to buffer into which to return the instnace string
** \param   len - length of buffer in which to return the instance string
**
** \return  None
**
**************************************************************************/
void FormInstanceString(dm_instances_t *inst, char *buf, int len)
{
    int i;
    int offset;
    int count;

    offset = 0;
    *buf = '\0';        // If no instances, make the buffer NULL terminated

    // Iterate over all instance numbers in the instance structure, building up the instance staing
    for (i=0; i < inst->order; i++)
    {
        // Append the instance number to the string
        if (i==0)
        {
            count = USP_SNPRINTF(&buf[offset], len, "%d", inst->instances[i]);  // First instance number, no separator
        }
        else
        {
            count = USP_SNPRINTF(&buf[offset], len, ".%d", inst->instances[i]); // Additional instance numbers need separator
        }

        // Exit USP Agent if USP_SNPRINTF failed
        if (count <= 0)
        {
            USP_ERR_Terminate("%s(%d): USP_SNPRINTF failed", __FUNCTION__, __LINE__);
        }

        // Move to where we write the next instance number
        offset += count;
        len -= count;
    }
}

/*********************************************************************//**
**
** ParseInstanceInteger
**
** Parses the next integer in the instance string
** eg Device.WiFi.EndPoint.1.Profile.5.Enable would have an instance string of "1.5"
**
** \param   p - pointer to string containing the next integer
** \param   p_value - pointer to variable in which to store the parsed value
**
** \return  pointer to next integer to parse in the string (after this one), or NULL if an error occurred in conversion
**
**************************************************************************/
char *ParseInstanceInteger(char *p, int *p_value)
{
    int value = 0;
    char c;

    // Iterate over all characters in the integer, converting them and building up the integer value
    c = *p;
    while ((c != '\0') && (c != '.'))
    {
        // Exit if number contains characters other than digits
        if ((c < '0') || (c > '9'))
        {
            return NULL;
        }

        value = value*10 + (c - '0');

        // Move to next character in the string
        p++;
        c = *p;
    }

    // If the code gets here, then it has reached the end of the string, or a '.' separator
    // Skip the '.' separator
    if (c == '.')
    {
        p++;
    }

    *p_value = value;
    return p;
}

/*********************************************************************//**
**
** CreateNode
**
** Allocates and initialises a data model node, and adds it to dm_node_map[]
**
** \param   name - portion of the data model path that this node represents
** \param   type - type of node (eg object or parameter)
** \param   schema_path - path in the data model to this node. Used to calculate the hash for parameter nodes
**                      eg 'Device.LocalAgent.Controller.{i}.Enable'
**
** \return  pointer to created node
**
**************************************************************************/
dm_node_t *CreateNode(char *name, dm_node_type_t type, char *schema_path)
{
    dm_node_t *node;
    int err;

    // Allocate memory for the node
    node = USP_MALLOC(sizeof(dm_node_t));
    memset(node, 0, sizeof(dm_node_t));     // NOTE: All roles start from zero permissions

    node->link.next = NULL;
    node->link.prev = NULL;
    node->next_node_map_link = NULL;
    node->type = type;
    node->name = USP_STRDUP(name);
    node->path = USP_STRDUP(schema_path);
    DLLIST_Init(&node->child_nodes);
    node->parent_node = NULL;

    // Default the group_id
    // NOTE: For objects which are non multi-instance, the group_id is effectively 'don't care' as non-table objects are not accessible via the grouped vendor hook APIs
    // NOTE: For non multi-instance objects in a USP Broker owned by USP Services, the group_id is overridden later to ensure that all nodes underneath a registered DM object are the same group_id
    node->group_id = NON_GROUPED;

    // Add the node into the node map, addressable by schema path
    err = LinkToNodeMap(node, schema_path);
    if (err != USP_ERR_OK)
    {
        USP_FREE(node);
        return NULL;
    }

    return node;
}

/*********************************************************************//**
**
** OverrideNodeType
**
** Modifies the specified data model node from one type to another
** This is called when it is known whether the path registered by a USP Service is a single or multi-instance object
** (this is only known after the GSDM response. The path is first registered with possibly the wrong type when the Register message is received)
**
** \param   node - pointer to data model schema node to modify
** \param   type - new type of the node
** \param   schema_path - path of the node in the schema
**                        NOTE: the schema path is different depending on whether the node is a single or multi-instance object
** \param   inst - pointer to the parent object nodes for each instance in the path to this node
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int OverrideNodeType(dm_node_t *node, dm_node_type_t type, char *schema_path, dm_instances_t *inst)
{
    dm_hash_t new_hash;
    dm_node_t *n;
    int err;

    // If the schema_path hasn't changed, then we don't need to re-home the node in dm_node_map[]
    if (strcmp(node->path, schema_path)==0)
    {
        goto exit;
    }

    // Exit if the changed schema_path would cause a hash collision
    // In this case, we do not change the node's type (or anything else)
    new_hash = TEXT_UTILS_CalcHash(schema_path);
    n = FindNodeFromHash(new_hash);
    if (n != NULL)
    {
        USP_ERR_SetMessage("%s: Failed to modify the type of node %s (to %s) because it's node hash conflicted with %s", __FUNCTION__, node->path, schema_path, n->name);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Re-home the node in the dm_node_map[], if the schema_path for the node has changed
    // Remove the node from it's existing position in the dm_node_map[]
    UnlinkFromNodeMap(node);

    // Add the node again using the new schema path
    err = LinkToNodeMap(node, schema_path);
    USP_ASSERT(err == USP_ERR_OK);      // This should be the case because we already checked for hash collisions earlier in this function

    // Replace schema path
    USP_FREE(node->path);
    node->path = USP_STRDUP(schema_path);

exit:
    // Override instance nodes and type
    node->type = type;
    memcpy(node->instance_nodes, inst->nodes, inst->order*sizeof(dm_node_t *));
    node->order = inst->order;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** LinkToNodeMap
**
** Adds the data model schema node into dm_node_map[]
**
** \param   node - pointer to data model schema node to add
** \aram    schema_path - pointer to string containing the schema_path (which is the key to use for the map)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int LinkToNodeMap(dm_node_t *node, char *schema_path)
{
    unsigned squashed_hash;
    dm_node_t *existing_node;
    dm_node_t *n;

    // Calculate hash of schema path
    node->hash = TEXT_UTILS_CalcHash(schema_path);

    // Exit if we have a hash collision
    n = FindNodeFromHash(node->hash);
    if (n != NULL)
    {
        USP_ERR_SetMessage("%s: Failed to add node %s because it's node hash conflicted with %s", __FUNCTION__, schema_path, n->name);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Push this node at the front of the linked list of nodes matching the squashed hash
    squashed_hash = ((unsigned)node->hash) % MAX_NODE_MAP_BUCKETS;
    existing_node = dm_node_map[squashed_hash];
    if (existing_node != NULL)
    {
        node->next_node_map_link = existing_node;
    }
    dm_node_map[squashed_hash] = node;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** UnlinkFromNodeMap
**
** Removes the data model schema node from dm_node_map[]
**
** \param   node - pointer to data model schema node to remove
**
** \return  None
**
**************************************************************************/
void UnlinkFromNodeMap(dm_node_t *node)
{
    unsigned squashed_hash;
    dm_node_t *cur_node;
    dm_node_t *node_before;

    squashed_hash = ((unsigned)node->hash) % MAX_NODE_MAP_BUCKETS;
    if (dm_node_map[squashed_hash] == node)
    {
        // Node is at the beginning of the chain, so just attach the rest of the chain to the node map
        dm_node_map[squashed_hash] = node->next_node_map_link;
    }
    else
    {
        // Node is in the middle or end of the chain, or missing from the chain
        cur_node = dm_node_map[squashed_hash];
        node_before = cur_node;
        cur_node = cur_node->next_node_map_link;
        while (cur_node != node)
        {
            // Exit if the node is missing from the node map, in which case there's nothing more to do
            if (cur_node == NULL)
            {
                USP_ASSERT(node->next_node_map_link == NULL);
                return;
            }

            node_before = cur_node;
            cur_node = cur_node->next_node_map_link;
        }

        // Attach the node before it in the chain to the one after it in the chain
        node_before->next_node_map_link = node->next_node_map_link;
    }

    // Ensure that node does not point to chain, as we've removed it from the chain now
    node->next_node_map_link = NULL;
}

/*********************************************************************//**
**
** ParseSchemaPath
**
** Splits the given data model schema path into path segments which have a 1-to-1 correspondence with nodes in the data model tree
** This function works on paths containing '{i}' or instance numbers
** NOTE: This function ignores duplicate '.' separators and also trailing '.' (for partial paths)
**
** \param   path - full data model path to split (not altered by this function)
** \param   path_segments - pointer to buffer in which to store the path segment strings
** \param   path_segment_len - length of buffer in which to store path segment strings
** \param   type - type of the last node in the path (eg object or parameter)
** \param   segment - pointer to array containing the segments
** \param   max_segments - maximum number of segments allowed in the array
**
** \return  number of segments in the path, or -1 if array was not large enough
**
**************************************************************************/
int ParseSchemaPath(char *path, char *path_segments, int path_segment_len, dm_node_type_t type, dm_path_segment *segments, int max_segments)
{
    int num_segments = 0;
    char *segment_start;
    dm_path_segment *seg;
    int len;
    char last_char;
    int i;

    // Setup default return values
    memset(segments, 0, sizeof(segments[0])*max_segments);

    // Create path segment strings
    len = strncpy_path_segments(path_segments, path, path_segment_len);

    // Scan the path, storing each segment found
    num_segments = 0;
    last_char = '\0';
    for (i=0; i<len; i++)
    {
        // See if found the start of a new segment
        if ((last_char == '\0') && (path_segments[i] != '\0'))
        {
            segment_start = &path_segments[i];

            if ( (strcmp(segment_start, MULTI_SEPARATOR)==0) || ((*segment_start>='0') && (*segment_start<='9')) )
            {
                // Special case of last separator was a multi instance object
                segments[num_segments-1].type = kDMNodeType_Object_MultiInstance;
            }
            else
            {
                // Normal case, add a segment to the end of the array
                if (*segment_start != '\0')             // Ignore empty segments (these are ones that use more than on '.' as separator)
                {
                    if (num_segments == max_segments)
                    {
                        USP_ERR_SetMessage("%s: More than %d path segments in path=%s", __FUNCTION__, max_segments, path);
                        return -1;
                    }

                    seg = &segments[num_segments];
                    seg->name = segment_start;
                    seg->type = kDMNodeType_Object_SingleInstance;  // Assume a single instance object until overridden by trailing '{i}'

                    num_segments++;
                }
            }
        }

        last_char = path_segments[i];
    }

    // If this path represents a parameter, then override the last segment's type
    if ((type != kDMNodeType_Object_MultiInstance) && (num_segments > 0))
    {
        seg = &segments[num_segments-1];
        seg->type = type;
    }

    return num_segments;
}

/*********************************************************************//**
**
** strncpy_path_segments
**
** Copy from src to dst, replacing all '.' characters with NULL terminators, and counting the total number of characters copied
** NOTE: If the src ends in a '.', then it is discarded
**
** \param   src - pointer to source path to copy
** \param   dst - buffer to contain destination path segment strings
** \param   maxlen - maximum number of characters in the destination buffer
**
** \return  number of characters copied into the destination
**
**************************************************************************/
int strncpy_path_segments(char *dst, char *src, int maxlen)
{
    int len;
    char c;

    len = 0;
    c = *src++;
    while ((c != '\0') && (len < maxlen-1))
    {
        // Convert '.' to NULL terminator
        if (c == '.')
        {
            c = '\0';
        }

        // Copy from src to destination
        *dst++ = c;
        len++;

        c = *src++;
    }

    // Ensure destination is always NULL terminated
    *dst = '\0';

    // Discard a trailing '.' from the length returned (the '.' will have already been changed into a '\0')
    if ((len > 0) && (dst[-1] == '\0'))
    {
        len--;
    }

    return len;
}

/*********************************************************************//**
**
** AddChildParamsDefaultValues
**
** Adds the default values for all children of the specified node into the database
** NOTE: This function is recursive
**
** \param   path - path of the object instance to add children to. This code will modify the buffer pointed to by this path
**                 NOTE: path is only actually needed for debug and error reporting purposes
** \param   path_len - length of path (position to append child node names)
** \param   node - Node to add defaulted children to
** \param   inst - pointer to instance structure locating the parent node
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int AddChildParamsDefaultValues(char *path, int path_len, dm_node_t *node, dm_instances_t *inst)
{
    int err;
    dm_node_t *child;

    // Iterate over list of children
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        switch(child->type)
        {
            case kDMNodeType_DBParam_ReadOnly:
            case kDMNodeType_DBParam_ReadWrite:
            case kDMNodeType_DBParam_Secure:
                {
                    char instances[MAX_DM_PATH];
                    char *new_value;

                    // Append the name of this parameter to the parent path
                    USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);

                    // Set the parameter to the new value in the database
                    new_value = child->registered.param_info.default_value;
                    FormInstanceString(inst, instances, sizeof(instances));
                    err = DATABASE_SetParameterValue(path, child->hash, instances, new_value, 0);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }

                    // Intentionally not intending to notify vendor of params which are defaulted,
                    // as we have a notify for when the whole instance has been added anyway
                    // So there is no need here to add the parameters to the pending notify queue
                    // (and we couldn't anyway, as this function is called during an add operation, not a set operation)
                }
                break;

            case kDMNodeType_DBParam_ReadOnlyAuto:
            case kDMNodeType_DBParam_ReadWriteAuto:
                {
                    char instances[MAX_DM_PATH];
                    char new_value[MAX_DM_VALUE_LEN];
                    dm_get_value_cb_t get_cb;
                    dm_req_t req;

                    get_cb = child->registered.param_info.get_cb;
                    USP_ASSERT(get_cb != NULL)

                    // Append the name of this parameter to the parent path
                    USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);

                    // Exit if unable to get the automatically populated value from the vendor code
                    DM_PRIV_RequestInit(&req, child, path, inst);
                    USP_ERR_ClearMessage();
                    new_value[0] = '\0';

                    err = get_cb(&req, new_value, sizeof(new_value));
                    if (err != USP_ERR_OK)
                    {
                        USP_ERR_ReplaceEmptyMessage("%s: GetAuto callback for path %s returned error %d", __FUNCTION__, path, err);
                        return err;
                    }

                    // If the parameter value was returned as a native value (in val_union), then convert it to a string
                    SerializeNativeValue(&req, child, new_value, sizeof(new_value));

                    // Set the parameter to the new value in the database
                    FormInstanceString(inst, instances, sizeof(instances));
                    err = DATABASE_SetParameterValue(path, child->hash, instances, new_value, 0);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }

                }
                break;

            // For single instance child object nodes, ensure that all of their children have their default value set
            case kDMNodeType_Object_SingleInstance:
                {
                    int len;
                    len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                    err = AddChildParamsDefaultValues(path, path_len+len, child, inst);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            // Nothing to do for non database writable parameters
            default:
            case kDMNodeType_VendorParam_ReadOnly:
            case kDMNodeType_VendorParam_ReadWrite:
            case kDMNodeType_Param_ConstantValue:
            case kDMNodeType_Param_NumEntries:
            case kDMNodeType_SyncOperation:
            case kDMNodeType_AsyncOperation:
            case kDMNodeType_Event:
            case kDMNodeType_Object_MultiInstance:  // A child Multi-instance object needs an instance creating before we can populate it
                break;
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DeleteChildParams
**
** Deletes all child parameters and object instances of the specified node
** NOTE: This function is recursive
**
** \param   path - path of the object instance to delete children from. This code will modify the buffer pointed to by this path
** \param   path_len - length of path (position to append child node names)
** \param   node - Node to delete children of
** \param   inst - pointer to instance structure locating the parent node
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeleteChildParams(char *path, int path_len, dm_node_t *node, dm_instances_t *inst)
{
    int err;
    dm_node_t *child;

    // Iterate over list of children
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        switch(child->type)
        {
            case kDMNodeType_DBParam_ReadOnly:
            case kDMNodeType_DBParam_ReadWrite:
            case kDMNodeType_DBParam_ReadOnlyAuto:
            case kDMNodeType_DBParam_ReadWriteAuto:
            case kDMNodeType_DBParam_Secure:
                {
                    char instances[MAX_DM_PATH];

                    // Append the name of this parameter to the parent path
                    USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);

                    FormInstanceString(inst, instances, sizeof(instances));
                    err = DATABASE_DeleteParameter(path, child->hash, instances);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            // For single instance child object nodes, ensure that all of their children are deleted
            case kDMNodeType_Object_SingleInstance:
                {
                    int len;
                    len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                    err = DeleteChildParams(path, path_len+len, child, inst);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            // For multi-instance child objects, ensure that all instances of all of their children are deleted
            case kDMNodeType_Object_MultiInstance:
                {
                    int len;
                    len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                    err = DeleteChildParams_MultiInstanceObject(path, path_len+len, child, inst);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            // Nothing to do for non database parameters
            case kDMNodeType_VendorParam_ReadOnly:
            case kDMNodeType_VendorParam_ReadWrite:
            case kDMNodeType_Param_ConstantValue:
            case kDMNodeType_Param_NumEntries:
            case kDMNodeType_SyncOperation:
            case kDMNodeType_AsyncOperation:
            case kDMNodeType_Event:
                break;

            default:
                TERMINATE_BAD_CASE(child->type);
                break;
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DeleteChildParams_MultiInstanceObject
**
** Iterates over all instances of a multi-instance object, deleting Deletes all child parameters and child object instances
** NOTE: This function is recursive
**
** \param   path - path of the object to delete children from. This code will modify the buffer pointed to by this path
** \param   path_len - length of path (position to append child node names)
** \param   node - Node to delete children of
** \param   inst - pointer to instance structure locating the parent node
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeleteChildParams_MultiInstanceObject(char *path, int path_len, dm_node_t *node, dm_instances_t *inst)
{
    int_vector_t iv;
    int instance;
    int len;
    int order;
    int i;
    int err;

    // Get an array of instances for this specific object
    INT_VECTOR_Init(&iv);
    err = DM_INST_VECTOR_GetInstances(node, inst, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Update instance structure in readiness to populate it with the instance number
    order = inst->order;
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);
    inst->nodes[order] = node;
    inst->order = order+1;

    // Iterate over all instances of this object
    for (i=0; i < iv.num_entries; i++)
    {
        // Form the path to this instance
        instance = iv.vector[i];
        len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%d", instance);

        // Delete all child parameters of this object
        inst->instances[order] = instance;
        err = DeleteChildParams(path, path_len+len, node, inst);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // De-register this object from the data model
        DM_INST_VECTOR_Remove(inst);

        // Add this object instance to the list of instances which are pending notification to the vendor
        path[path_len+len] = '\0';
        DM_TRANS_Add(kDMOp_Del, path, NULL, NULL, node, inst);
    }

    // Put the instance structure back to the way it was
    inst->nodes[order] = NULL;
    inst->instances[order] = 0;
    inst->order = order;
    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DestroySchemaRecursive
**
** Recursively frees all memory associated with the specified data model node
**
** \param   node - pointer to node to recursively destroy
**
** \return  None
**
**************************************************************************/
void DestroySchemaRecursive(dm_node_t *node)
{
    dm_node_t *child;

    // First destroy all child nodes
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        DestroySchemaRecursive(child);
        child = (dm_node_t *) node->child_nodes.head;
    }

    // Then destroy this node
    DestroyNode(node);
}

/*********************************************************************//**
**
** DestroyNode
**
** Unlinks the specified node from all internal data structures, then frees it
**
** \param   node - pointer to node to recursively destroy
**
** \return  None
**
**************************************************************************/
void DestroyNode(dm_node_t *node)
{
    dm_node_t *parent;

    // Remove it from the NodeMap
    UnlinkFromNodeMap(node);

    // Remove this node from its parent's child list
    parent = node->parent_node;
    if (parent != NULL)
    {
        DLLIST_Unlink(&parent->child_nodes, node);
    }

    // Free extra information contained in the 'registered' union
    switch (node->type)
    {
        case kDMNodeType_Object_MultiInstance:
            // NOTE: No need to free the unique keys in the vector - the pointers are to the child nodes' param names, so are going to be freed
            USP_SAFE_FREE(node->registered.object_info.unique_keys.vector);
            DM_INST_VECTOR_Destroy(&node->registered.object_info.inst_vector);
            break;

        case kDMNodeType_Param_ConstantValue:
        case kDMNodeType_DBParam_ReadWrite:
        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_Secure:
        case kDMNodeType_DBParam_ReadOnlyAuto:
        case kDMNodeType_DBParam_ReadWriteAuto:
            USP_FREE(node->registered.param_info.default_value);
            break;

        default:
        case kDMNodeType_Param_NumEntries:
        case kDMNodeType_Object_SingleInstance:
        case kDMNodeType_VendorParam_ReadOnly:
        case kDMNodeType_VendorParam_ReadWrite:
            // These types of nodes do not allocate anything extra in the 'registered' union
            break;

        case kDMNodeType_Event:
            {
                dm_event_info_t *info;

                info = &node->registered.event_info;
                STR_VECTOR_Destroy(&info->event_args);
            }
            break;

        case kDMNodeType_SyncOperation:
        case kDMNodeType_AsyncOperation:
            {
                dm_oper_info_t *info;

                info = &node->registered.oper_info;
                STR_VECTOR_Destroy(&info->input_args);
                STR_VECTOR_Destroy(&info->output_args);
            }
            break;
    }

    // Finally free this node itself
    USP_FREE(node->path);
    USP_FREE(node->name);
    USP_FREE(node);
}

/*********************************************************************//**
**
** DestroyInstanceVectorRecursive
**
** Recursively frees all memory containing the instance vectors of all top level multi-instance objects
**
** \param   parent - pointer to node to recursively free all instance vectors of
**
** \return  None
**
**************************************************************************/
void DestroyInstanceVectorRecursive(dm_node_t *parent)
{
    dm_node_t *child;

    if (parent->type == kDMNodeType_Object_MultiInstance)
    {
        // This node is a top level multi instance node, storing it's instances and all instances of its children
        // So free the instance vector it holds, then exit
        // NOTE: we do not have to recurse to its children because their instances are stored here
        DM_INST_VECTOR_Destroy(&parent->registered.object_info.inst_vector);
        return;
    }

    // Recurse to free all child node instance vectors
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        DestroyInstanceVectorRecursive(child);

        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** DumpInstanceVectorRecursive
**
** Recursively prints out instances stored in the data model
**
** \param   parent - pointer to node to recursively dump
**
** \return  None
**
**************************************************************************/
void DumpInstanceVectorRecursive(dm_node_t *parent)
{
    dm_node_t *child;

    if (parent->type == kDMNodeType_Object_MultiInstance)
    {
        // This node is a top level multi instance node, storing it's instances and all instances of its children
        // So print the instances it holds, then exit
        // NOTE: we do not have to recurse to its children because their instances are stored here
        DM_INST_VECTOR_Dump(&parent->registered.object_info.inst_vector);
        return;
    }

    // Recurse to dump all child node instance vectors
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        DumpInstanceVectorRecursive(child);

        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** DumpSchemaFromRoot
**
** Dumps the schema, starting at the specified root node
**
** \param   root - pointer to root data model node to dump from
** \param   name - name of the data model root node
**
** \return  None
**
**************************************************************************/
void DumpSchemaFromRoot(dm_node_t *root, char *name)
{
    int i;
    str_vector_t sv;

    USP_DUMP("\nDumping %s Schema...", name);
    STR_VECTOR_Init(&sv);

    // Build up the paths of all elements in the schema in a string vector
    AddChildNodes(root, &sv);

    // Sort the paths into alphabetical order
    qsort(sv.vector, sv.num_entries, sizeof(char *), SortSchemaPath);

    // Print the sorted schema paths
    for (i=0; i < sv.num_entries; i++)
    {
        USP_DUMP("%s", sv.vector[i]);
    }

    STR_VECTOR_Destroy(&sv);
}

/*********************************************************************//**
**
** DumpDataModelNodeMap
**
** Dumps the names of all nodes stored in the node map
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DumpDataModelNodeMap(void)
{
    int i;
    dm_node_t *node;
    int num_links;
    int max_num_links = 0;
    int index_with_max_links = 0;

    USP_DUMP("\nDumping Data Model Node Map...");
    for (i=0; i<MAX_NODE_MAP_BUCKETS; i++)
    {
        node = dm_node_map[i];
        num_links = 0;
        while (node != NULL)
        {
            USP_DUMP("[%d] %s (hash=0x%08x)", i, node->path, node->hash);
            node = node->next_node_map_link;
            num_links++;
        }

        // Update the maximum number of nodes that had the same squashed_hash value
        if (num_links > max_num_links)
        {
            max_num_links = num_links;
            index_with_max_links = i;
        }
    }

    USP_DUMP("\nMaximum number of nodes mapping into same bucket=%d (at [%d])", max_num_links, index_with_max_links);

}

/*********************************************************************//**
**
** SortSchemaPath
**
** Function used by quicksort to sort a list of schema paths
**
** \param   p1 - pointer to first element in a string array to compare
** \param   p1 - pointer to second element in a string array to compare
**
** \return  None
**
**************************************************************************/
int SortSchemaPath(const void *p1, const void *p2)
{
    // qsort passes in pointers to the elements in the array to be compared
    // Hence we need to dereference the elements to get to the strings they point to
    char *s1 = * (char **) p1;
    char *s2 = * (char **) p2;

    return strcmp(s1, s2);
}

/*********************************************************************//**
**
** AddChildNodes
**
** Function called recursively to add the schema paths of all nodes to a string vector
**
** \param   parent - pointer to data model node to add
** \param   sv - pointer to string vector in which to add the schema paths
**
** \return  None
**
**************************************************************************/
void AddChildNodes(dm_node_t *parent, str_vector_t *sv)
{
    dm_node_t *child;
    char obj_path[MAX_DM_PATH];
    char *path;

    // Add this node to the string vector
    USP_SNPRINTF(obj_path, sizeof(obj_path), "%s.", parent->path);
    path = (IsObject(parent)) ? obj_path : parent->path;
    STR_VECTOR_Add(sv, path);

    // Add arguments (if applicable) to string vector
    if (IsOperation(parent))
    {
        AddChildArgs(sv, parent->path, &parent->registered.oper_info.input_args, "input");
        AddChildArgs(sv, parent->path, &parent->registered.oper_info.output_args, "output");
    }

    if (parent->type == kDMNodeType_Event)
    {
        AddChildArgs(sv, parent->path, &parent->registered.event_info.event_args, "event_arg");
    }

    // Iterate over list of children
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        AddChildNodes(child, sv);

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** AddChildArgs
**
** Function called to add recursively to add the schema paths of all nodes to a string vector
**
** \param   sv - pointer to string vector in which to add the schema paths
** \param   path - data model path of the USP command or event
** \param   args - pointer to string vector containing arguments to add to the schema path vector
** \param   arg_type - pointer to string describing type of argument (input, output, or event_arg)
**
** \return  None
**
**************************************************************************/
void AddChildArgs(str_vector_t *sv, char *path, str_vector_t *args, char *arg_type)
{
    int i;
    char buf[MAX_DM_PATH];

    for (i=0; i < args->num_entries; i++)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s %s:%s", path, arg_type, args->vector[i]);
        STR_VECTOR_Add(sv, buf);
    }
}

/*********************************************************************//**
**
** FindNodeFromHash
**
** Finds the node that matches the specified hash of its data model schema path
**
** \param   hash - hash value of the node path of the node to find
**
** \return  pointer to node matching hash in the data model, or NULL if no match
**
**************************************************************************/
dm_node_t *FindNodeFromHash(dm_hash_t hash)
{
    unsigned squashed_hash;
    dm_node_t *node;

    squashed_hash = ((unsigned)hash) % MAX_NODE_MAP_BUCKETS;

    // Find the node in the linked list of nodes which match the squashed hash
    node = dm_node_map[squashed_hash];
    while ((node != NULL) && (node->hash != hash))
    {
        node = node->next_node_map_link;
    }

    return node;
}

/*********************************************************************//**
**
** FindLinkToFirstObject
**
** Finds the first object in the list of child nodes
**
** \param   list - pointer to the list containing child nodes
**
** \return  pointer to first object found in the list of child nodes or NULL if no objects are found
**
**************************************************************************/
double_link_t *FindLinkToFirstObject(double_linked_list_t *list)
{
    double_link_t *item;
    dm_node_t *child;

    item = list->head;
    while (item != NULL)
    {
        child = (dm_node_t *) item;
        if (IsObject(child))
        {
            return item;
        }

        // move to next item in linked list
        item = item->next;
    }

    // Got to end of list and no objects found
    return NULL;
}

/*********************************************************************//**
**
** RegisterDefaultControllerTrust
**
** This function is called if no vendor hook overrides it
** Registers all controller trust roles and permissions
** This function is called inbetween VENDOR_Init() and VENDOR_Start()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int RegisterDefaultControllerTrust(void)
{
    int err = USP_ERR_OK;

    // Register 'Full Access' role
    err |= USP_DM_RegisterRoleName(ROLE_FULL_ACCESS, "Full Access");
    err |= USP_DM_AddControllerTrustPermission(ROLE_FULL_ACCESS, dm_root, PERMIT_ALL);

    // Register 'Untrusted' role
    err |= USP_DM_RegisterRoleName(ROLE_UNTRUSTED,  "Untrusted");
    err |= USP_DM_AddControllerTrustPermission(ROLE_UNTRUSTED, dm_root, PERMIT_NONE);
    err |= USP_DM_AddControllerTrustPermission(ROLE_UNTRUSTED, "Device.DeviceInfo.", PERMIT_GET | PERMIT_OBJ_INFO);
    err |= USP_DM_AddControllerTrustPermission(ROLE_UNTRUSTED, "Device.LocalAgent.ControllerTrust.RequestChallenge()", PERMIT_OPER);
    err |= USP_DM_AddControllerTrustPermission(ROLE_UNTRUSTED, "Device.LocalAgent.ControllerTrust.ChallengeResponse()", PERMIT_OPER);

    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetVendorParam
**
** Gets the value of a single vendor parameter, which might need to be obtained by the group get mechanism
**
** \param   node - pointer to node representing the parameter to get
** \param   path - pointer to string containing complete data model path to the parameter
** \param   inst - pointer to instances structure
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
** \param   req - pointer to structure identifying the parameter
**
** \return  None
**
**************************************************************************/
int GetVendorParam(dm_node_t *node, char *path, dm_instances_t *inst, char *buf, int len, dm_req_t *req)
{
    int err;
    dm_get_value_cb_t get_cb;
    dm_get_group_cb_t get_group_cb;
    kv_vector_t params;
    kv_pair_t pair;

    // Exit (getting the value) if the vendor parameter was not grouped with any other parameters
    if (node->group_id == NON_GROUPED)
    {
        get_cb = node->registered.param_info.get_cb;
        USP_ASSERT(get_cb != NULL)

        // Exit if unable to get the value from the vendor code
        DM_PRIV_RequestInit(req, node, path, inst);
        USP_ERR_ClearMessage();
        buf[0] = '\0';

        err = get_cb(req, buf, len);
        if (err != USP_ERR_OK)
        {
            USP_ERR_ReplaceEmptyMessage("%s: Get callback for path %s returned error %d", __FUNCTION__, path, err);
            return err;
        }

        // If the parameter value was returned as a native value (in val_union), then convert it to a string
        SerializeNativeValue(req, node, buf, len);

        return USP_ERR_OK;
    }

    // If the code gets here, then the parameter is grouped with other parameters

    // Exit if there is no callback defined for this group
    get_group_cb = group_vendor_hooks[node->group_id].get_group_cb;
    if (get_group_cb == NULL)
    {
        USP_ERR_SetMessage("%s: No registered group callback to get param %s", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Statically create a kv vector, on the stack for this single parameter
    // The only item that will need freeing is the value string (if the group callback filled it in)
    pair.key = path;
    pair.value = NULL;
    params.vector = &pair;
    params.num_entries = 1;

    // Exit if group callback fails
    err = get_group_cb(node->group_id, &params);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Get group callback failed for param %s", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if group callback did not fill in a value for the parameter
    if (pair.value == NULL)
    {
        USP_ERR_SetMessage("%s: Get group callback did not provide a value for param %s", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy the value of the parameter into the return buffer, and then free the dynamically allocated parameter value
    USP_STRNCPY(buf, pair.value, len);
    USP_FREE(pair.value);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SetVendorParam
**
** Sets the value of a single vendor parameter, which might need to be obtained by the group set mechanism
**
** \param   node - pointer to node representing the parameter to set
** \param   path - pointer to string containing complete data model path to the parameter
** \param   inst - pointer to instances structure
** \param   value - value to set the parameter to
** \param   req - pointer to structure identifying the parameter
**
** \return  None
**
**************************************************************************/
int SetVendorParam(dm_node_t *node, char *path, dm_instances_t *inst, char *value, dm_req_t *req)
{
    int err;
    dm_set_value_cb_t set_cb;
    dm_set_group_cb_t set_group_cb;
    dm_param_info_t *info;
    kv_vector_t params;
    kv_pair_t pair;
    int failure_index = INVALID;  // not actually used by this function

    // Exit (setting the value) if the vendor parameter was not grouped with any other parameters
    info = &node->registered.param_info;
    if (node->group_id == NON_GROUPED)
    {
        set_cb = info->set_cb;
        if (set_cb == NULL)
        {
            USP_ERR_SetMessage("%s: No registered callback to set param %s", __FUNCTION__, path);
            return USP_ERR_INTERNAL_ERROR;
        }

        USP_ERR_ClearMessage();
        err = set_cb(req, value);
        if (err != USP_ERR_OK)
        {
            USP_ERR_ReplaceEmptyMessage("%s: Failed to set (new value=%s) on (path=%s)", __FUNCTION__, value, path);
            return err;
        }

        return USP_ERR_OK;
    }

    // If the code gets here, then the parameter is grouped with other parameters

    // Exit if there is no callback defined for this group
    set_group_cb = group_vendor_hooks[node->group_id].set_group_cb;
    if (set_group_cb == NULL)
    {
        USP_ERR_SetMessage("%s: No registered group callback to set param %s", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Statically create a kv vector, on the stack for this single parameter
    // Ownership of the key-value pair in the vector stays with the caller of this function
    pair.key = path;
    pair.value = value;
    params.vector = &pair;
    params.num_entries = 1;

    // Exit if group callback fails
    err = set_group_cb(node->group_id, &params, &info->type_flags, &failure_index);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Set group callback failed for param %s (err_code=%d)", __FUNCTION__, path, err);
        return err;
    }

    return USP_ERR_OK;
}
