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
 * \file usp_api.c
 *
 * Implements the API exposed to the vendor
 * In many cases these functions are just a facade around internal core functionality
 *
 */

#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "iso8601.h"
#include "os_utils.h"
#include "device.h"
#include "dm_inst_vector.h"
#include "dm_trans.h"
#include "str_vector.h"

/*********************************************************************//**
**
** USP_DM_GetParameterValue
**
** Gets a single named parameter from the data model
** Wrapper function around data model API, that ensures the function is only called from the data model thread
**
** \param   path - pointer to string containing complete data model path to the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_GetParameterValue(char *path, char *buf, int len)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return DATA_MODEL_GetParameterValue(path, buf, len, 0);
}

/*********************************************************************//**
**
** USP_DM_SetParameterValue
**
** Sets a single named parameter in the data model
** Wrapper function around data model API, that ensures the function is only called from the data model thread
**
** \param   path - pointer to string containing complete path
** \param   new_value - pointer to buffer containing value to set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_SetParameterValue(char *path, char *new_value)
{
    int err;
    dm_trans_vector_t trans;

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit, setting the value, if this function is called from within a transaction
    if (DM_TRANS_IsWithinTransaction() == true)
    {
        return DATA_MODEL_SetParameterValue(path, new_value, 0);
    }

    // If the code gets here, then a transaction has not been started, so start one in this function
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the set parameter value failed
    err = DATA_MODEL_SetParameterValue(path, new_value, 0);
    if (err != USP_ERR_OK)
    {
        DM_TRANS_Abort();
        return err;
    }

    // Commit the transaction
    DM_TRANS_Commit();
    return err;
}

/*********************************************************************//**
**
** USP_DM_DeleteInstance
**
** Deletes the specified instance from the data model
** Wrapper function around data model API, that ensures the function is only called from the data model thread
**
** \param   path - path of the object instance to delete
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_DeleteInstance(char *path)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return DATA_MODEL_DeleteInstance(path, IGNORE_NO_INSTANCE);
}

/*********************************************************************//**
**
** USP_DM_InformInstance
**
** Synchronously notifies USP Agent core that a vendor controlled object is present
** This function should only be called from the data model thread.
** Typically this function is called at startup from VENDOR_Start() to seed the data model with vendor object instances
**
** \param   path - path of the object instance to delete
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_InformInstance(char *path)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return DATA_MODEL_NotifyInstanceAdded(path);
}

/*********************************************************************//**
**
** USP_DM_RefreshInstance
**
** Adds the specified object instance into the instance vector
** NOTE: This function may only be called by the vendor within the context of the get_instances_cb call
**       It must contain only instances of the object specified in the get_instances_cb (and that object's children)
**
** \param   path - data model path of the multi-instance object to add
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_RefreshInstance(char *path)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return DM_INST_VECTOR_RefreshInstance(path);
}

/*********************************************************************//**
**
** USP_DM_GetInstances
**
** Gets an array of instance numbers for the specified object
** Wrapper function around data model API, that ensures the function is only called from the data model thread
**
** \param   path - path of the object
** \param   vector - pointer to array of integers to populate
** \param   max_entries - Maximum number of entries to populate in the vector
** \param   num_entries - pointer to variable in which to return the number of entries written to the return vector
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_GetInstances(char *path, int *vector, int max_entries, int *num_entries)
{
    int_vector_t iv;
    int err;
    int num_to_copy;

    // Exit if this function has invalid arguments
    if ((path == NULL) || (vector == NULL) | (num_entries == NULL))
    {
        USP_ERR_SetMessage("%s: Invalid arguments (some are NULL)", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to get the instances of this object
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Copy as many instance numbers into the return array as it has capacity for
    num_to_copy = MIN(max_entries, iv.num_entries);
    memcpy(vector, iv.vector, num_to_copy*sizeof(int));
    *num_entries = num_to_copy;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}


/*********************************************************************//**
**
** USP_DM_IsRegistered
**
** Determines whether the specified data model schema path is registered in the supported data model
** The path should use either the '{i}' or '*' notation instead of instance numbers. Any instance numbers in the path are treated as '{i}'
** IMPORTANT:  This function checks for presence in the supported data model, not the instantiated data model.
**
** \param   path - supported data model path of the data model element
**
** \return  true if the specified data model element exists in the supported data model, false otherwise
**
**************************************************************************/
bool USP_DM_IsRegistered(char *path)
{
    dm_node_t *node;

    // Exit if this function has invalid arguments
    if (path == NULL)
    {
        USP_ERR_SetMessage("%s: Invalid argument (path is NULL)", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exiit if the path does not exist in the data model
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
    if (node == NULL)
    {
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** USP_DM_InformDataModelEvent
**
** Synchronously notifies USP Agent core of a USP Event
** This function should only be called from the data model thread.
** Typically this function is called at startup from the async operation restart vendor hook,
** to ensure a particular USP event (e.g. TransferComplete!) is sent before the corresponding Operation Complete notification
** NOTE: Ownership of the output_args stays with the caller, so the caller must destroy them after calling this function
**
** \param   event_name - name of the event
** \param   output_args - arguments for the event
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_InformDataModelEvent(char *event_name, kv_vector_t *output_args)
{
    // Exit if this function is not being called from the data model thread
    if (OS_UTILS_IsDataModelThread(__FUNCTION__, PRINT_WARNING)==false)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions(event_name, output_args);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_ARG_Create
**
** Dynamically allocates a key-value pair structure and initialises it
**
** \param   kvv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
kv_vector_t *USP_ARG_Create(void)
{
    kv_vector_t *kvv;
    kvv = USP_MALLOC(sizeof(kv_vector_t));
    KV_VECTOR_Init(kvv);

    return kvv;
}

/*********************************************************************//**
**
** USP_ARG_Init
**
** Initialises a key-value pair vector structure
**
** \param   kvv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void USP_ARG_Init(kv_vector_t *kvv)
{
    KV_VECTOR_Init(kvv);
}

/*********************************************************************//**
**
** USP_ARG_Add
**
** Adds a key value pair into the vector, where the value is specified as a string
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - pointer to string to copy
**
** \return  None
**
**************************************************************************/
void USP_ARG_Add(kv_vector_t *kvv, char *key, char *value)
{
    KV_VECTOR_Add(kvv, key, value);
}

/*********************************************************************//**
**
** USP_ARG_Replace
**
** Replaces the value associated with the specified key
**
** \param   kvv - pointer to structure to replace the value in
** \param   key - pointer to key, whose value we want to replace
** \param   value - pointer to replacement value
**
** \return  true if the value was replaced, false if the key does not exist in the vector
**
**************************************************************************/
bool USP_ARG_Replace(kv_vector_t *kvv, char *key, char *value)
{
    return KV_VECTOR_Replace(kvv, key, value);
}

/*********************************************************************//**
**
** USP_ARG_ReplaceWithHint
**
** Replaces the value associated with the specified key, given a hint index to find the key
**
** \param   kvv - pointer to structure to replace the value in
** \param   key - pointer to key, whose value we want to replace
** \param   value - pointer to replacement value
** \param   hint - index of entry in key value vector at which the key is expected to be located
**
** \return  true if the value was replaced, false if the key does not exist in the vector
**
**************************************************************************/
bool USP_ARG_ReplaceWithHint(kv_vector_t *kvv, char *key, char *value, int hint)
{
    return KV_VECTOR_ReplaceWithHint(kvv, key, value, hint);
}

/*********************************************************************//**
**
** USP_ARG_AddUnsigned
**
** Adds a key value pair into the vector, where the value is specified as an unsigned number
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void USP_ARG_AddUnsigned(kv_vector_t *kvv, char *key, unsigned value)
{
    KV_VECTOR_AddUnsigned(kvv, key, value);
}

/*********************************************************************//**
**
** USP_ARG_AddBool
**
** Adds a key value pair into the vector, where the value is specified as a boolean
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void USP_ARG_AddBool(kv_vector_t *kvv, char *key, bool value)
{
    KV_VECTOR_AddBool(kvv, key, value);
}

/*********************************************************************//**
**
** USP_ARG_AddDateTime
**
** Adds a key value pair into the vector, where the value is specified as a date-time
**
** \param   kvv - pointer to structure to add the string to
** \param   key - pointer to string to copy
** \param   value - value to convert to a string and add to the vector
**
** \return  None
**
**************************************************************************/
void USP_ARG_AddDateTime(kv_vector_t *kvv, char *key, time_t value)
{
    KV_VECTOR_AddDateTime(kvv, key, value);
}

/*********************************************************************//**
**
** USP_ARG_Get
**
** Returns a pointer to the value associated with the specified key or the specified default value
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - pointer to default value to return (this could be NULL if we want a return indicator that the key did not exist)
**
** \return  pointer to value
**
**************************************************************************/
char *USP_ARG_Get(kv_vector_t *kvv, char *key, char *default_value)
{
    return KV_VECTOR_Get(kvv, key, default_value, 0);
}

/*********************************************************************//**
**
** USP_ARG_GetUnsigned
**
** Gets the value of the specified parameter from the vector as an unsigned integer
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an unsigned integer
**
**************************************************************************/
int USP_ARG_GetUnsigned(kv_vector_t *kvv, char *key, unsigned default_value, unsigned *value)
{
    return KV_VECTOR_GetUnsigned(kvv, key, default_value, value);
}

/*********************************************************************//**
**
** USP_ARG_GetUnsignedWihinRange
**
** Gets the value of the specified parameter from the vector as an unsigned integer,
** checking that it is within the specified range
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   min - minimum allowed value
** \param   min - maximum allowed value
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an unsigned integer
**          USP_ERR_INVALID_VALUE if value is out of range
**
**************************************************************************/
int USP_ARG_GetUnsignedWithinRange(kv_vector_t *kvv, char *key, unsigned default_value, unsigned min, unsigned max, unsigned *value)
{
    return KV_VECTOR_GetUnsignedWithinRange(kvv, key, default_value, min, max, value);
}

/*********************************************************************//**
**
** USP_ARG_GetInt
**
** Gets the value of the specified parameter from the vector as an integer
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an integer
**
**************************************************************************/
int USP_ARG_GetInt(kv_vector_t *kvv, char *key, int default_value, int *value)
{
    return KV_VECTOR_GetInt(kvv, key, default_value, value);
}

/*********************************************************************//**
**
** USP_ARG_GetIntWithinRange
**
** Gets the value of the specified parameter from the vector as an integer,
** checking that it is within the specified range
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   min - minimum allowed value
** \param   min - maximum allowed value
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INVALID_TYPE if unable to convert the key's value (given in the vector) to an integer
**          USP_ERR_INVALID_VALUE if value is out of range
**
**************************************************************************/
int USP_ARG_GetIntWithinRange(kv_vector_t *kvv, char *key, int default_value, int min, int max, int *value)
{
    return KV_VECTOR_GetIntWithinRange(kvv, key, default_value, min, max, value);
}

/*********************************************************************//**
**
** USP_ARG_GetBool
**
** Gets the value of the specified parameter from the vector as a bool
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_ARG_GetBool(kv_vector_t *kvv, char *key, bool default_value, bool *value)
{
    return KV_VECTOR_GetBool(kvv, key, default_value, value);
}

/*********************************************************************//**
**
** USP_ARG_GetDateTime
**
** Gets the value of the specified parameter from the vector as a time_t
**
** \param   kvv - pointer to key-value pair vector structure
** \param   key - pointer to name of key to get the value of
** \param   default_value - default value, if not present in the vector
** \param   value - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_ARG_GetDateTime(kv_vector_t *kvv, char *key, char *default_value, time_t *value)
{
    return KV_VECTOR_GetDateTime(kvv, key, default_value, value);
}

/*********************************************************************//**
**
** USP_ARG_Destroy
**
** Deallocates all memory associated with the key-value pair vector, apart from the kv_vector_t structure itself
** This is the opposite of USP_ARG_Init()
**
** \param   kvv - pointer to structure to destroy all dynmically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void USP_ARG_Destroy(kv_vector_t *kvv)
{
    KV_VECTOR_Destroy(kvv);
}

/*********************************************************************//**
**
** USP_ARG_Delete
**
** Deallocates all memory associated with the key-value pair vector, including the kv_vector_t structure itself
** This is the opposite of USP_ARG_Create()
**
** \param   kvv - pointer to structure to destroy all dynmically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void USP_ARG_Delete(kv_vector_t *kvv)
{
    KV_VECTOR_Destroy(kvv);
    USP_FREE(kvv);
}

/*********************************************************************//**
**
** USP_STR_VEC_Init
**
** Initialises a string vector structure
**
** \param   sv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void USP_STR_VEC_Init(str_vector_t *sv)
{
    STR_VECTOR_Init(sv);
}

/*********************************************************************//**
**
** USP_STR_VEC_Add
**
** Copies a string into the vector of strings
**
** \param   sv - pointer to structure to add the string to
** \param   str - pointer to string to copy
**
** \return  None
**
**************************************************************************/
void USP_STR_VEC_Add(str_vector_t *sv, char *str)
{
    STR_VECTOR_Add(sv, str);
}

/*********************************************************************//**
**
** USP_STR_VEC_Destroy
**
** Deallocates all memory associated with the string vector, apart from the str_vector_t structure itself
** This is the opposite of USP_STR_VEC_Init()
**
** \param   sv - pointer to structure to destroy all dynmically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void USP_STR_VEC_Destroy(str_vector_t *sv)
{
    STR_VECTOR_Destroy(sv);
}

/*********************************************************************//**
**
** USP_CONVERT_DateTimeToUnixTime
**
** Converts an ISO8601 string time into a UTC-based unix time
**
** \param   date - pointer to ISO8601 string to convert
**
** \return  Number of seconds since the UTC unix epoch, or INVALID_TIME if the conversion failed
**
**************************************************************************/
time_t USP_CONVERT_DateTimeToUnixTime(char *date)
{
    return iso8601_to_unix_time(date);
}

/*********************************************************************//**
**
** USP_CONVERT_UnixTimeToDateTime
**
** Given a time_t converts it to an ISO8601 string
**
** \param   unix_time - time in seconds since the epoch (UTC)
** \param   buf - pointer to buffer in which to return the string
** \param   len - length of buffer. Must be at least MAX_ISO8601_LEN bytes long.
**
** \return  pointer to string (i.e. supplied buffer). This is useful if this function is called within a printf()
**
**************************************************************************/
char *USP_CONVERT_UnixTimeToDateTime(time_t unix_time, char *buf, int len)
{
    return iso8601_from_unix_time(unix_time, buf, len);
}

/*********************************************************************//**
**
** USP_DM_RegisterRoleName
**
** Sets an entry in the Device.LocalAgent.ControllerTrust.Role.{i} table
**
** \param   role_instance - Instance in Device.LocalAgent.ControllerTrust.Role.{i} to which we want to assign a name
** \param   name - name of the role
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_RegisterRoleName(int role_instance, char *name)
{
    int err;
    char buf[MAX_DM_PATH];

    // Exit if role is out of bounds
    // NOTE: This check is necessary because USP_DM_AddControllerTrustPermission() sizes an array for only MAX_CTRUST_ROLES instances
    if ((role_instance < 1) || (role_instance >= MAX_CTRUST_ROLES))
    {
        USP_ERR_SetMessage("%s: Supplied role (%d) is out of bounds (expected < %d)", __FUNCTION__, role_instance, MAX_CTRUST_ROLES);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set this role in the database
    USP_SNPRINTF(buf, sizeof(buf), "cpe-%d", role_instance);
    err  = DEVICE_CTRUST_SetRoleParameter(role_instance, "Alias", buf);
    err |= DEVICE_CTRUST_SetRoleParameter(role_instance, "Name", name);
    err |= DEVICE_CTRUST_SetRoleParameter(role_instance, "Enable", "true");
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to set Role.%d", __FUNCTION__, role_instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_DM_AddControllerTrustPermission
**
** Adds an entry to the Device.LocalAgent.ControllerTrust.Rol.{i}.Permission.{i} table
**
** \param   role_instance - Instance in Device.LocalAgent.ControllerTrust.Role.{i} to which we want to add a permission
** \param   path - pointer to path expression specifying which data model nodes are modified
**                 Currently this only supports partial paths, full paths and wildcards. Instance numbers in the path are not supported.
** \param   permission_bitmask - bitmask of permissions to apply to the data model nodes
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_DM_AddControllerTrustPermission(int role_instance, char *path, unsigned short permission_bitmask)
{
    int err;
    dm_node_t *node;
    static int permission_count[MAX_CTRUST_ROLES] = {0};
    char buf[MAX_DM_SHORT_VALUE_LEN];
    int perm_instance;

    // Exit if role is out of bounds
    // NOTE: This check is necessary because permission_count[] is sized based on instance numbers in the range 1..MAX_CTRUST_ROLES
    if ((role_instance < 1) || (role_instance >= MAX_CTRUST_ROLES))
    {
        USP_ERR_SetMessage("%s: Supplied role (%d) is out of bounds (expected < %d)", __FUNCTION__, role_instance, MAX_CTRUST_ROLES);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if path is not a data model path
    // This may occur if the path is owned by a USP Service, and the USP Service has not registered yet.
    // In this case, when the USP Service registers the path, the permissions will be set then, instead of now
    node =  DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
    if (node == NULL)
    {
        return USP_ERR_OK;
    }

    // Increment the permission instance counter for this role
    permission_count[role_instance-1]++;
    perm_instance = permission_count[role_instance-1];

    // Set this permission in the database
    USP_SNPRINTF(buf, sizeof(buf), "cpe-%d", perm_instance);
    err  = DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Alias", buf);
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Enable", "true");

    USP_SNPRINTF(buf, sizeof(buf), "%d", perm_instance);
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Order", buf);
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Targets", path);

    // Set Device.ControllerTrust.Role.{i}.Permission.{i}.Param
    #define PERM_CHAR(perm_char, mask) (permission_bitmask & mask) ? perm_char : '-';
    buf[0] = PERM_CHAR('r', PERMIT_GET);
    buf[1] = PERM_CHAR('w', PERMIT_SET);
    buf[2] = '-';
    buf[3] = PERM_CHAR('n', PERMIT_SUBS_VAL_CHANGE);
    buf[4] = '\0';
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Param", buf);

    // Set Device.ControllerTrust.Role.{i}.Permission.{i}.Obj
    buf[0] = PERM_CHAR('r', PERMIT_OBJ_INFO);
    buf[1] = PERM_CHAR('w', PERMIT_ADD);
    buf[2] = '-';
    buf[3] = PERM_CHAR('n', PERMIT_SUBS_OBJ_ADD);
    buf[4] = '\0';
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "Obj", buf);

    // Set Device.ControllerTrust.Role.{i}.Permission.{i}.InstantiatedObj
    buf[0] = PERM_CHAR('r', PERMIT_GET_INST);
    buf[1] = PERM_CHAR('w', PERMIT_DEL);
    buf[2] = '-';
    buf[3] = PERM_CHAR('n', PERMIT_SUBS_OBJ_DEL);
    buf[4] = '\0';
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "InstantiatedObj", buf);

    // Set Device.ControllerTrust.Role.{i}.Permission.{i}.CommandEvent
    buf[0] = PERM_CHAR('r', PERMIT_CMD_INFO);
    buf[1] = '-';
    buf[2] = PERM_CHAR('x', PERMIT_OPER);
    buf[3] = PERM_CHAR('n', PERMIT_SUBS_EVT_OPER_COMP);
    buf[4] = '\0';
    err |= DEVICE_CTRUST_SetPermissionParameter(role_instance, perm_instance, "CommandEvent", buf);

    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to set Role.%d.Permission.%d", __FUNCTION__, role_instance, perm_instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_HOOK_DenyAddInstance
**
** Callback that must be registered for read only tables, to prevent a controller from creating entries in the table
** NOTE: If this exact callback is not registered, then GetSupportedDM may incorrectly report that the object is read-write
**
** \param   req - pointer to structure containing path information
**
** \return  USP_ERR_OBJECT_NOT_CREATABLE always
**
**************************************************************************/
int USP_HOOK_DenyAddInstance(dm_req_t *req)
{
    USP_ERR_SetMessage("%s: Cannot add instances to a read only table", __FUNCTION__);
    return USP_ERR_OBJECT_NOT_CREATABLE;
}

/*********************************************************************//**
**
** USP_HOOK_DenyDeleteInstance
**
** Callback that must be registered for read only tables, to prevent a controller from deleting entries from the table
** NOTE: If this exact callback is not registered, then GetSupportedDM may incorrectly report that the object is read-write
**
** \param   req - pointer to structure containing path information
**
** \return  USP_EUSP_ERR_OBJECT_NOT_DELETABLE always
**
**************************************************************************/
int USP_HOOK_DenyDeleteInstance(dm_req_t *req)
{
    USP_ERR_SetMessage("%s: Cannot delete instances from a read only table", __FUNCTION__);
    return USP_ERR_OBJECT_NOT_DELETABLE;
}

