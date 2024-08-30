/*
 *
 * Copyright (C) 2019-2023, Broadband Forum
 * Copyright (C) 2016-2023  CommScope, Inc
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
 * \file usp_register.c
 *
 * Implements the functions which register the data model schema
 *
 */

#include <string.h>

#include "common_defs.h"
#include "dllist.h"
#include "data_model.h"
#include "dm_access.h"
#include "dm_inst_vector.h"

//------------------------------------------------------------------------------
// Structure containing vendor hook callback functions which are used by the core agent data model
// NOTE: As this structure is registered early in the bootup, it is safe to be indexed from multiple threads subsequently
vendor_hook_cb_t vendor_hook_callbacks = { NULL };

//------------------------------------------------------------------------------
// Array containing the get/set callbacks for each group of vendor parameters
group_vendor_hook_t group_vendor_hooks[MAX_VENDOR_PARAM_GROUPS];

//------------------------------------------------------------------------------
// Commonly used strings
static char *usp_err_invalid_param_str = "%s: Invalid parameters";
static char *usp_err_bad_scope_str = "ERROR: Function must be called from within context of VENDOR_Init(): %s(path=%s)";

//------------------------------------------------------------------------------
// Array used to register names of unique keys for a data model table
static char *alias_unique_keys[1] = { "Alias" };

//------------------------------------------------------------------------------
// Typedef for a generic callback, used to interpret the vendor_hook_cb_t structure as an array of generic callbacks
typedef int (*generic_cb_t)(void);

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int ValidateAliasParam(dm_req_t *req, char *value);
int ValidateParamUniqueness(dm_req_t *req, char *value);

/*********************************************************************//**
**
** USP_REGISTER_CoreVendorHooks
**
** Registers core vendor hook functions which the core agent can callback
** NOTE: This function may be called multiple times.
**       Set a callback to non-NULL in the structure if you want to override the currently stored callback
**
** \param   callbacks - pointer to structure containing callbacks to register
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_REGISTER_CoreVendorHooks(vendor_hook_cb_t *callbacks)
{
    int i;
    int num_vendor_hooks;
    generic_cb_t *src_callbacks;
    generic_cb_t *dst_callbacks;
    generic_cb_t cb;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, "undefined");
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are incorrect
    if (callbacks == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Override the current set of stored vendor hooks with these ones, if they are non-NULL
    num_vendor_hooks = sizeof(vendor_hook_cb_t) / sizeof(generic_cb_t);
    src_callbacks = (generic_cb_t *) callbacks;
    dst_callbacks = (generic_cb_t *) &vendor_hook_callbacks;
    for (i=0; i<num_vendor_hooks; i++)
    {
        cb = src_callbacks[i];
        if (cb != NULL)
        {
            dst_callbacks[i] = cb;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Param_Constant
**
** Registers a parameter which will never be changed
** This is useful for parameters which just state which options the agent supports, and for version numbers etc
**
** \param   path - full data model path for the parameter
** \param   value - constant value of the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Param_Constant(char *path, char *value, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (value == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Param_ConstantValue, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP(value);
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Param_NumEntries
**
** Registers a parameter which represents the number of entries in a data model table
** When the parameter is read, USP Agent will automatically calculate the number of entries in the specified table
** This function may be used for database and vendor controlled tables
**
** \param   path - full data model path for the parameter
** \param   table_path - data model path to the table which this parameter represents
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Param_NumEntries(char *path, char *table_path)
{
    dm_node_t *param_node;
    dm_node_t *table_node;
    int order;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (table_path == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add the path to the parameter to the data model
    param_node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Param_NumEntries, 0);
    if (param_node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add the path to the table to the data model (if not already added)
    table_node = DM_PRIV_AddSchemaPath(table_path, kDMNodeType_Object_MultiInstance, SUPPRESS_PRE_EXISTANCE_ERR);
    if (table_node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Check that the instances order in the table path is one more than the instances order in the parameter path
    // This must be the case for nested multi-instance tables
    if (param_node->order + 1 != table_node->order)
    {
        USP_ERR_SetMessage("%s(%d): table_path must have same parent as parameter path", __FUNCTION__, __LINE__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Check that the parameter path and table_path have the same parent, if the table is nested
    if (param_node->order > 0)
    {
        order = param_node->order - 1;
        if (param_node->instance_nodes[order] != table_node->instance_nodes[order])
        {
            USP_ERR_SetMessage("%s(%d): table_path must have same parent as parameter path", __FUNCTION__, __LINE__);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Save registered info into the data model
    info = &param_node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->table_node = table_node;
    info->group_id = NON_GROUPED;
    info->type_flags = DM_UINT;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Param_SupportedList
**
** Convenience function to register a parameter containing a comma separated list of enumerated values
** This function is typically used for parameters stating a fixed list of supported options
**
** \param   path - full data model path for the parameter
** \param   enums - pointer to conversion table a list of enumerations and their associated string representation
** \param   num_enums - number of enumerations in the table
**
** \return  pointer to converted string or "UNKNOWN" if unable to convert
**
**************************************************************************/
int USP_REGISTER_Param_SupportedList(char *path, const enum_entry_t *enums, int num_enums)
{
    int i;
    char *p;
    int chars_written;
    const enum_entry_t *e;
    char buf[MAX_DM_VALUE_LEN];
    int len;

    // Default to empty string, if no items
    p = buf;
    *p = '\0';
    len = sizeof(buf);

    // Iterate over all enums to add, forming a comma separated string
    for (i=0; i<num_enums; i++)
    {
        // Add comma before every enum (apart from the first)
        if (p != buf)
        {
            chars_written = USP_SNPRINTF(p, len, "%s", ", ");
            p += chars_written;
            len -= chars_written;
        }

        // Add the enum (if it's not empty)
        e = &enums[i];
        if (e->name[0] != '\0')
        {
            chars_written = USP_SNPRINTF(p, len, "%s", e->name);
            p += chars_written;
            len -= chars_written;
        }
    }

    // Register the parameter as a constant comma separated string
    return USP_REGISTER_Param_Constant(path, buf, DM_STRING);
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_ReadWrite
**
** Registers a parameter which will be persisted in the database
** This is useful for parameters which control configuration of the agent
**
** \param   path - full data model path for the parameter
** \param   value - default value of the parameter
** \param   validator_cb - callback called to validate a value, before allowing it to be set
** \param   notify_set_cb - callback called after the value has been changed
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_ReadWrite(char *path, char *value, dm_validate_value_cb_t validator_cb,
                                  dm_notify_set_cb_t notify_set_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If no default value is passed in, then use a default value of empty string
    if (value == NULL)
    {
        value = "";
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_DBParam_ReadWrite, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP(value);
    info->validator_cb = validator_cb;
    info->notify_set_cb = notify_set_cb;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_Secure
**
** Registers a parameter which may be written to, but when read back always returns an empty string
** This function should be used to register all parameters which are passwords
**
** \param   path - full data model path for the parameter
** \param   value - default value of the parameter
** \param   validator_cb - callback called to validate a value, before allowing it to be set
** \param   notify_set_cb - callback called after the value has been changed
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_Secure(char *path, char *value, dm_validate_value_cb_t validator_cb, dm_notify_set_cb_t notify_set_cb)
{
    return USP_REGISTER_DBParam_SecureWithType(path, value, validator_cb, notify_set_cb, DM_STRING);
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_SecureWithType
**
** Registers a parameter which may be written to, but when read back always returns an empty string
** This function should be used to register all parameters which are passwords
**
** \param   path - full data model path for the parameter
** \param   value - default value of the parameter
** \param   validator_cb - callback called to validate a value, before allowing it to be set
** \param   notify_set_cb - callback called after the value has been changed
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_SecureWithType(char *path, char *value, dm_validate_value_cb_t validator_cb, dm_notify_set_cb_t notify_set_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If no default value is passed in, then use a default value of empty string
    if (value == NULL)
    {
        value = "";
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_DBParam_Secure, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP(value);
    info->validator_cb = validator_cb;
    info->notify_set_cb = notify_set_cb;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_ReadOnly
**
** Registers a parameter which is persisted in the database and is read only
** This is useful for configuration parameters which are alterable by the agent eg in it's GUI, but not alterable by the controller
** NOTE: For parameters of this type, USP_REGISTER_VendorParam_ReadOnly() could be used, this function just makes the vendor implementation easier
**
** \param   path - full data model path for the parameter
** \param   value - default value of the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_ReadOnly(char *path, char *value, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If no default value is passed in, then use a default value of empty string
    if (value == NULL)
    {
        value = "";
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_DBParam_ReadOnly, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP(value);
    info->validator_cb = NULL;
    info->notify_set_cb = NULL;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_VendorParam_ReadOnly
**
** Registers a parameter which is not persisted in the database and is read only
** This is useful for parameters which are used for monitoring the current (transient) state of the agent
**
** \param   path - full data model path for the parameter
** \param   get_cb - callback called to get the value of the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_VendorParam_ReadOnly(char *path, dm_get_value_cb_t get_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (get_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_VendorParam_ReadOnly, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->get_cb = get_cb;
    info->set_cb = NULL;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_VendorParam_ReadWrite
**
** Registers a read-write parameter which is implemented by the vendor and is not persisted in the USP Agent database
**
** \param   path - full data model path for the parameter
** \param   get_cb - callback called to get the value of the parameter
** \param   set_cb - callback called to set the value of the parameter
** \param   notify_set_cb - callback called after the value has been changed
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_VendorParam_ReadWrite(char *path, dm_get_value_cb_t get_cb, dm_set_value_cb_t set_cb,
                                                   dm_notify_set_cb_t notify_set_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (get_cb == NULL) || (set_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_VendorParam_ReadWrite, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->get_cb = get_cb;
    info->set_cb = set_cb;
    info->notify_set_cb = notify_set_cb;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_GroupedVendorParam_ReadOnly
**
** Registers a grouped parameter which is not persisted in the database and is read only
** This is useful for parameters which are used for monitoring the current (transient) state of the agent
** The parameter is part of a group of parameters (with the same group_id) that can be more efficiently get using the group get value vendor hook
**
** \param   group_id - identifier of the group of parameters that this parameter belongs to
** \param   path - full data model path for the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_GroupedVendorParam_ReadOnly(int group_id, char *path, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (group_id == NON_GROUPED) || (group_id >= MAX_VENDOR_PARAM_GROUPS))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_VendorParam_ReadOnly, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->get_cb = NULL;
    info->set_cb = NULL;
    info->group_id = group_id;
    info->type_flags = type_flags;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_GroupedVendorParam_ReadWrite
**
** Registers a read-write parameter which is implemented by the vendor and is not persisted in the USP Agent database
** The parameter is part of a group of parameters (with the same group_id) that can be more efficiently get/set using the group get/set value vendor hook
**
** \param   group_id - identifier of the group of parameters that this parameter belongs to
** \param   path - full data model path for the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_GroupedVendorParam_ReadWrite(int group_id, char *path, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (group_id == NON_GROUPED) || (group_id >= MAX_VENDOR_PARAM_GROUPS))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_VendorParam_ReadWrite, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->group_id = group_id;
    info->type_flags = type_flags;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_ReadOnlyAuto
**
** Registers a parameter which is part of a table, and whose (read only) value is initialised dynamically at creation time
**
** \param   path - full data model path for the parameter
** \param   get_cb - callback called only once, to get the automatically assigned value of the parameter
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_ReadOnlyAuto(char *path, dm_get_value_cb_t get_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (get_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_DBParam_ReadOnlyAuto, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP("");
    info->get_cb = get_cb;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_ReadWriteAuto
**
** Registers a parameter which is part of a table, and whose (read write) value is initialised dynamically at creation time
**
** \param   path - full data model path for the parameter
** \param   get_cb - callback called only once, to get the automatically assigned value of the parameter
** \param   validator_cb - callback called to validate a value, before allowing it to be set
** \param   notify_set_cb - callback called after the value has been changed
** \param   type_flags - type of the parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_ReadWriteAuto(char *path, dm_get_value_cb_t get_cb, dm_validate_value_cb_t validator_cb,
                                      dm_notify_set_cb_t notify_set_cb, unsigned type_flags)
{
    dm_node_t *node;
    dm_param_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (get_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_DBParam_ReadWriteAuto, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.param_info;
    memset(info, 0, sizeof(dm_param_info_t));
    info->default_value = USP_STRDUP("");
    info->get_cb = get_cb;
    info->validator_cb = validator_cb;
    info->notify_set_cb = notify_set_cb;
    info->group_id = NON_GROUPED;
    info->type_flags = type_flags;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_DBParam_Alias
**
** Registers an 'Alias' style parameter and sets it to be one of the unique keys for the table
** This is a convenience function
**
** \param   path - full data model path for the 'Alias' style parameter
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_DBParam_Alias(char *path, dm_notify_set_cb_t notify_set_cb)
{
    int err;
    char table_path[MAX_DM_PATH];
    char *p;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to register the Alias parameter
    err =  USP_REGISTER_DBParam_ReadWriteAuto(path, DM_ACCESS_PopulateAliasParam, ValidateAliasParam, notify_set_cb, DM_STRING);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Create a string containing the name of the table
    USP_STRNCPY(table_path, path, sizeof(table_path));
    if (err != USP_ERR_OK)
    {
        return err;
    }
    p = strrchr(table_path, '.');
    *p = '\0';

    // Exit if unable to register the Alias parameter as a unique key for the table
    err = USP_REGISTER_Object_UniqueKey(table_path, alias_unique_keys, NUM_ELEM(alias_unique_keys));
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Object
**
** Registers that instances of an object (stored in DB) may be added and deleted by a controller
** This is useful for parameters which control configuration of the agent
**
** \param   path - full data model path for the object
** \param   validate_add_cb - callback called to validate whether an instance can be added by a controller.
**                            If set to NULL, then instances can always be added.
**                            If set to USP_HOOK_DenyAddInstance(), instances cannot be added to the table
** \param   add_cb - callback usually used by vendor to create the instance in the vendor DB, and set default vendor params
** \param   notify_add_cb - callback called after an instance has been added
** \param   validate_del_cb - callback called to validate whether the specified instance can be deleted by a controller.
**                            If set to NULL, then any instance can be deleted.
**                            If set to USP_HOOK_DenyDeleteInstance, instances cannot be deleted from the table
** \param   del_cb - callback usually used by vendor to delete the instance from the vendor DB
** \param   notify_del_cb - callback called after an instance has been deleted
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Object(char *path, dm_validate_add_cb_t validate_add_cb, dm_add_cb_t add_cb, dm_notify_add_cb_t notify_add_cb,
                                   dm_validate_del_cb_t validate_del_cb, dm_del_cb_t del_cb, dm_notify_del_cb_t notify_del_cb)
{
    dm_node_t *node;
    dm_object_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_MultiInstance, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.object_info;
    memset(info, 0, sizeof(dm_object_info_t));
    info->validate_add_cb = validate_add_cb;
    info->add_cb = add_cb;
    info->notify_add_cb = notify_add_cb;
    info->validate_del_cb = validate_del_cb;
    info->del_cb = del_cb;
    info->notify_del_cb = notify_del_cb;
    info->group_id = NON_GROUPED;
    DM_INST_VECTOR_Init(&info->inst_vector);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Object_UniqueKey
**
** Registers which parameters form a unique key (or compound unique key) for a multi-instance object
** More than one unique key/compound unique key may be registered per table
** NOTE: This function must be called after all parameters in the key have been registered with the data model
**
** \param   path - full data model path for the multi-instance object
** \param   params - pointer to array of strings. Each string is a parameter name in the unique key
** \param   num_params - number of parameters forming the unique key. If this is greter than 1, then the unique key is compound
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Object_UniqueKey(char *path, char **params, int num_params)
{
    int i;
    dm_node_t *node;
    dm_node_t *child;
    dm_unique_key_t unique_key;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if calling arguments are specified incorrectly
    if ((path==NULL) || (params==NULL) || (num_params < 1) || (num_params > MAX_COMPOUND_KEY_PARAMS))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if any of the params making up the key is NULL
    for (i=0; i<num_params; i++)
    {
        if (params[i] == NULL)
        {
            USP_ERR_SetMessage("%s: Parameter at position [%d] in params array is NULL", __FUNCTION__, i);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Exit if unable to find this multi-instance object in the data model
    // NOTE: This call will add the path if not already added, but unlike other DM_REGISTER functions will
    // not generate an error if this function is called after the node has been added by USP_REGISTER_Object()
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_MultiInstance, SUPPRESS_PRE_EXISTANCE_ERR);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if any of the params making up the key are not registered with the data model or are an object
    memset(&unique_key, 0, sizeof(unique_key));
    for (i=0; i<num_params; i++)
    {
        child = DM_PRIV_FindMatchingChild(node, params[i]);
        if (child == NULL)
        {
            USP_ERR_SetMessage("%s: Parameter '%s' in unique key is not a child of '%s'", __FUNCTION__, params[i], path);
            return USP_ERR_INTERNAL_ERROR;
        }

        if (IsParam(child)==false)
        {
            USP_ERR_SetMessage("%s: Parameter '%s.%s' in unique key is not a parameter", __FUNCTION__, path, params[i]);
            return USP_ERR_INTERNAL_ERROR;
        }

        unique_key.param[i] = child->name; // Using child->name instead of strdup(params[i]) saves memory
    }

    // Add this unique key to the data model
    DM_PRIV_AddUniqueKey(node, &unique_key);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Object_RefreshInstances
**
** Registers a function to call to get the instances of the top level multi-instance object (and all its descendants in the data model tree)
**
** \param   path - full data model path for the top-level multi-instance object
** \param   refresh_instances_cb - callback called to get all the instances of the specified object (and instances of all child objects)
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Object_RefreshInstances(char *path, dm_refresh_instances_cb_t refresh_instances_cb)
{
    dm_node_t *node;
    dm_object_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if calling arguments are specified incorrectly
    if ((path==NULL) || (refresh_instances_cb==NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to find this multi-instance object in the data model
    // NOTE: This call will add the path if not already added, but unlike other DM_REGISTER functions will
    // not generate an error if this function is called after the node has been added by USP_REGISTER_Object()
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_MultiInstance, SUPPRESS_PRE_EXISTANCE_ERR);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if node is not a top-level multi-instance node
    if (node->order != 1)
    {
        USP_ERR_SetMessage("%s: Path %s is not a top-level multi-instance object", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.object_info;
    info->refresh_instances_cb = refresh_instances_cb;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_GroupedObject
**
** Registers an object which uses the group add and delete vendor hooks
**
** \param   group_id - group_id to register for the object
** \param   path - full data model path for the top-level multi-instance object
** \param   is_writable - set if instances can be added/deleted, clear if instances are not controlled by USP controller
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_GroupedObject(int group_id, char *path, bool is_writable)
{
    dm_node_t *node;
    dm_object_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if calling arguments are specified incorrectly
    if ((path == NULL) || (group_id >= MAX_VENDOR_PARAM_GROUPS))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_MultiInstance, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Save registered info into the data model
    info = &node->registered.object_info;
    memset(info, 0, sizeof(dm_object_info_t));
    info->group_id = group_id;
    info->group_writable = is_writable;
    DM_INST_VECTOR_Init(&info->inst_vector);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_SyncOperation
**
** Registers a synchronous operation on an object with the data model
**
** \param   path - full data model path for the operation
** \param   sync_oper_cb - callback called to perform this operation on an object
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_SyncOperation(char *path, dm_sync_oper_cb_t sync_oper_cb)
{
    dm_node_t *node;
    dm_oper_info_t *info;
    int len;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (sync_oper_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if last two characters in the path are not '()'
    len = strlen(path);
    if ((len <2) || (strcmp(&path[len-2], "()") != 0))
    {
        USP_ERR_SetMessage("%s: '%s' is not an operation (missing '()')", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_SyncOperation, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    info = &node->registered.oper_info;
    memset(info, 0, sizeof(dm_oper_info_t));
    info->sync_oper_cb = sync_oper_cb;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_AsyncOperation
**
** Registers an asynchronous operation on an object with the data model
**
** \param   path - full data model path for the operation
** \param   async_oper_cb - callback called to start this operation on an object
** \param   restart_cb - called to determine whether to restart an operation, if it was interrupted by a power cycle
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_AsyncOperation(char *path, dm_async_oper_cb_t async_oper_cb, dm_async_restart_cb_t restart_cb)
{
    dm_node_t *node;
    dm_oper_info_t *info;
    int len;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if ((path == NULL) || (async_oper_cb == NULL))
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if last two characters in the path are not '()'
    len = strlen(path);
    if ((len <2) || (strcmp(&path[len-2], "()") != 0))
    {
        USP_ERR_SetMessage("%s: '%s' is not an operation (missing '()')", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_AsyncOperation, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    info = &node->registered.oper_info;
    memset(info, 0, sizeof(dm_oper_info_t));
    info->async_oper_cb = async_oper_cb;
    info->restart_cb = restart_cb;
    info->max_concurrency = INT_MAX;  // By default allow multiple concurrent invocations

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_AsyncOperation_MaxConcurrency
**
** Registers the maximum number of concurrently running operations of this type
**
** \param   path - full data model path for the operation
** \param   max_concurrency - Maximum number of concurrent callback called to start this operation on an object
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_AsyncOperation_MaxConcurrency(char *path, int max_concurrency)
{
    dm_node_t *node;
    dm_oper_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if this async command has not been registered yet
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: Async command %s must first be registered using USP_REGISTER_AsyncOperation()", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy the max_concurrency argument into the data model
    info = &node->registered.oper_info;
    info->max_concurrency = max_concurrency;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_OperationArguments
**
** Registers the names of the input and output arguments for an operation
** The information registered by this function is returned in the GetSupportedDM Response
**
** \param   path - full data model path for the operation
** \param   input_arg_names - pointer to array of strings containing the names of the input arguments
** \param   num_input_arg_names - number of input arguments
** \param   output_arg_names - pointer to array of strings containing the names of the output arguments
** \param   num_output_arg_names - number of output arguments
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_OperationArguments(char *path, char **input_arg_names, int num_input_arg_names, char **output_arg_names, int num_output_arg_names)
{
    int i;
    dm_node_t *node;
    dm_oper_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input arguments are inconsistently specified
    if ( ((num_input_arg_names > 0) && (input_arg_names == NULL)) ||
         ((num_output_arg_names > 0) && (output_arg_names == NULL)) )
    {
        USP_ERR_SetMessage("%s: Expecting Input or output arguments for %s (got NULL)", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if any of the input arguments are NULL
    for (i=0; i < num_input_arg_names; i++)
    {
        if (input_arg_names[i] == NULL)
        {
            USP_ERR_SetMessage("%s: Argument at position [%d] in input args array for %s is NULL", __FUNCTION__, i, path);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Exit if any of the output arguments are NULL
    for (i=0; i < num_output_arg_names; i++)
    {
        if (output_arg_names[i] == NULL)
        {
            USP_ERR_SetMessage("%s: Argument at position [%d] in output args array for %s is NULL", __FUNCTION__, i, path);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Exit if unable to find this operation in the data model
    // NOTE: This call will add the path if not already added, but unlike other DM_REGISTER functions will
    // not generate an error if this function is called after the node has been added by USP_REGISTER_Object()
    #define ASSUMED_TYPE  kDMNodeType_AsyncOperation
    node = DM_PRIV_AddSchemaPath(path, ASSUMED_TYPE, SUPPRESS_PRE_EXISTANCE_ERR | SUPPRESS_LAST_TYPE_CHECK);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Because we suppressed the type check of the last node, check here that it is one of the ones we expect
    if ((node->type != kDMNodeType_AsyncOperation) && (node->type != kDMNodeType_SyncOperation))
    {
        USP_ERR_SetMessage("%s: Expected %s to be an operation", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy the input and output arguments into the data model
    info = &node->registered.oper_info;
    if (input_arg_names != NULL)
    {
        STR_VECTOR_Clone(&info->input_args, input_arg_names, num_input_arg_names);
    }

    if (output_arg_names != NULL)
    {
        STR_VECTOR_Clone(&info->output_args, output_arg_names, num_output_arg_names);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_Event
**
** Registers an event with the data model
**
** \param   path - full data model path for the event
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_Event(char *path)
{
    dm_node_t *node;
    int len;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input parameters are not defined
    if (path == NULL)
    {
        USP_ERR_SetMessage(usp_err_invalid_param_str, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if last character in the path is not '!'
    len = strlen(path);
    if ((len < 1) || (path[len-1] != '!'))
    {
        USP_ERR_SetMessage("%s: '%s' is not an event (missing '!')", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this path to the data model
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Event, 0);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_EventArguments
**
** Registers the names of the arguments carried in an event
** The information registered by this function is returned in the GetSupportedDM Response
**
** \param   path - full data model path for the event
** \param   event_arg_names - pointer to array of strings containing the names of the arguments
** \param   num_event_arg_names - number of arguments
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_EventArguments(char *path, char **event_arg_names, int num_event_arg_names)
{
    int i;
    dm_node_t *node;
    dm_event_info_t *info;

    // Exit if this function is not being called from within VENDOR_Init()
    if (is_executing_within_dm_init == false)
    {
        USP_ERR_SetMessage(usp_err_bad_scope_str, __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if input arguments are inconsistently specified
    if ((num_event_arg_names > 0) && (event_arg_names == NULL))
    {
        USP_ERR_SetMessage("%s: Expecting event arguments for %s (got NULL)", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if any of the arguments are NULL
    for (i=0; i < num_event_arg_names; i++)
    {
        if (event_arg_names[i] == NULL)
        {
            USP_ERR_SetMessage("%s: Argument at position [%d] in event args array for %s is NULL", __FUNCTION__, i, path);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Exit if unable to find this event object in the data model
    // NOTE: This call will add the path if not already added, but unlike other DM_REGISTER functions will
    // not generate an error if this function is called after the node has been added by USP_REGISTER_Object()
    node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Event, SUPPRESS_PRE_EXISTANCE_ERR);
    if (node == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy the arguments into the data model
    info = &node->registered.event_info;
    if (event_arg_names != NULL)
    {
        STR_VECTOR_Clone(&info->event_args, event_arg_names, num_event_arg_names);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_REGISTER_GroupVendorHooks
**
** Registers the get and set vendor hooks for a group of vendor parameters
**
** \param   group_id - identifier of the group of parameters that this parameter belongs to
** \param   get_group_cb - callback called to get the values of an assortment of parameters from the group
** \param   set_group_cb - callback called to set the values of an assortment of parameters from the group
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int USP_REGISTER_GroupVendorHooks(int group_id, dm_get_group_cb_t get_group_cb, dm_set_group_cb_t set_group_cb,
                                                dm_add_group_cb_t add_group_cb, dm_del_group_cb_t del_group_cb)
{
    group_vendor_hook_t *gvh;

    if ((group_id == NON_GROUPED) || (group_id < 0) || (group_id >= MAX_VENDOR_PARAM_GROUPS))
    {
        USP_ERR_SetMessage("%s: Invalid Group ID (%d). Expected a value between 0 and %d", __FUNCTION__, group_id, MAX_VENDOR_PARAM_GROUPS-1);
        return USP_ERR_INTERNAL_ERROR;
    }

    gvh = &group_vendor_hooks[group_id];
    gvh->get_group_cb = get_group_cb;
    gvh->set_group_cb = set_group_cb;
    gvh->add_group_cb = add_group_cb;
    gvh->del_group_cb = del_group_cb;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateAliasParam
**
** Validates new values of an Alias parameter
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of the parameter for this instance which the controller would like to set
**
** \return  USP_ERR_OK if retrieved successfully
**
**************************************************************************/
int ValidateAliasParam(dm_req_t *req, char *value)
{
    char cur_value[MAX_DM_SHORT_VALUE_LEN];
    char c;
    int err;

    // Exit if value is empty. Alias values must not be empty according to the spec
    c = *value;
    if (c == '\0')
    {
        USP_ERR_SetMessage("%s: Alias parameter values must not be empty", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if value doesn't start with a letter. Alias values must start with a letter according to the spec
    if (IS_ALPHA(c)==false)
    {
        USP_ERR_SetMessage("%s: Alias parameter values must start with a letter (%s doesn't)", __FUNCTION__, value);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to get the current value of the parameter
    err = DATA_MODEL_GetParameterValue(req->path, cur_value, sizeof(cur_value), 0);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Failed to get the current value of an Alias parameter (%s)", __FUNCTION__, req->path);
        return err;
    }

    // Exit if the current value has already been changed from "cpe-"
    // We do not allow this, as the Alias parameter is only allowed to be set once (changed from the default value of 'cpe-')
    if ((*cur_value != '\0') && (strncmp(cur_value, DEFAULT_ALIAS_PREFIX, sizeof(DEFAULT_ALIAS_PREFIX)-1) != 0))
    {
        USP_ERR_SetMessage("%s: Alias parameter values must not be changed once assigned (current_value='%s')", __FUNCTION__, cur_value);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if the parameter is not unique within the table
    err = ValidateParamUniqueness(req, value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateParamUniqueness
**
** Convenience function to validate that the parameter is unique within the table
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of the parameter for this instance which the controller would like to set
**
** \return  USP_ERR_OK if retrieved successfully
**
**************************************************************************/
int ValidateParamUniqueness(dm_req_t *req, char *value)
{
    char *p;
    int_vector_t iv;
    int err;
    int i;
    int instance;
    int req_instance;
    char *param_name;
    char table_path[MAX_DM_PATH];
    char path[MAX_DM_PATH];
    char buf[MAX_DM_SHORT_VALUE_LEN];

    INT_VECTOR_Init(&iv);

    // Split off the param name
    USP_STRNCPY(table_path, req->path, sizeof(table_path));
    p = strrchr(table_path, '.');   // Skip back past param_name
    USP_ASSERT(p != NULL);
    *p = '\0';
    param_name = &p[1];

    // Split off the parent table instance
    p = strrchr(table_path, '.');   // Skip the instance number of this param
    USP_ASSERT(p != NULL);
    *p = '\0';

    // Determine the instance number for the param being set in the parent table
    USP_ASSERT(req->inst->order > 0);
    req_instance = req->inst->instances[ req->inst->order-1];

    // Exit if unable to get the instance numbers associated with the parent table
    err = DATA_MODEL_GetInstances(table_path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over all instances in the table, seeing if any match the specified param
    for (i=0; i < iv.num_entries; i++)
    {
        // Skip this entry if it is the param we're trying to set
        instance = iv.vector[i];
        if (instance == req_instance)
        {
            continue;
        }

        // Exit if unable to get the value of the parameter
        USP_SNPRINTF(path, sizeof(path), "%s.%d.%s", table_path, instance, param_name);
        err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if the parameter values match, and hence the value is not unique
        if (strcmp(buf, value) == 0)
        {
            USP_ERR_SetMessage("%s: The value for %s (%s) is not unique (already used by instance %d)", __FUNCTION__, req->path, value, instance);
            err = USP_ERR_INVALID_ARGUMENTS;
            goto exit;
        }
    }

    // If the code gets here, then no match was found
    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}


