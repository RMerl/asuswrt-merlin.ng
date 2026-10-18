/*
 *
 * Copyright (C) 2023-2024, Broadband Forum
 * Copyright (C) 2023-2024  CommScope, Inc
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
 * \file group_add_vector.c
 *
 * Performs addition of a set of objects across different data model provider components
 *
 */

#include <string.h>

#include "common_defs.h"
#include "group_add_vector.h"
#include "group_set_vector.h"
#include "data_model.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int CreateObject_WithCreateVendorHook(group_add_entry_t *gae, combined_role_t *combined_role);
int CreateObject_WithoutCreateVendorHook(group_add_entry_t *gae, combined_role_t *combined_role);
void RollbackGroupAddEntry(group_add_entry_t *gae);

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_Init
**
** Initialises a group add vector structure
**
** \param   gav - Pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void GROUP_ADD_VECTOR_Init(group_add_vector_t *gav)
{
    gav->vector = NULL;
    gav->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_Destroy
**
** Frees all memory used by the group add vector
**
** \param   gav - pointer to vector to destroy
**
** \return  None
**
**************************************************************************/
void GROUP_ADD_VECTOR_Destroy(group_add_vector_t *gav)
{
    int i, j;
    group_add_entry_t *gae;
    group_add_param_t *ps;

    for (i=0; i < gav->num_entries; i++)
    {
        gae = &gav->vector[i];
        USP_FREE(gae->req_path);
        USP_FREE(gae->res_path);
        USP_SAFE_FREE(gae->err_msg);
        KV_VECTOR_Destroy(&gae->unique_keys);

        for (j=0; j < gae->num_params; j++)
        {
            ps = &gae->params[j];
            USP_SAFE_FREE(ps->err_msg);
            // NOTE: We do not free param_name or value as they are owned by the parsed Add request protobuf structure
        }
        USP_SAFE_FREE(gae->params);
    }

    USP_SAFE_FREE(gav->vector);
    gav->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_AddObjectToCreate
**
** Adds an object to create to the vector
**
** \param   gav - pointer to vector to add to
** \param   req_path - requested data model path, that resolved to res_path
** \param   res_path - data model path of table to add an instance to
** \param   group_id - ID representing the data model provider component implementing this object or NON_GROUPED
**
** \return  None
**
**************************************************************************/
void GROUP_ADD_VECTOR_AddObjectToCreate(group_add_vector_t *gav, char *req_path, char *res_path, int group_id)
{
    int new_num_entries;
    group_add_entry_t *gae;

    // Increase the vector size
    new_num_entries = gav->num_entries + 1;
    gav->vector = USP_REALLOC(gav->vector, new_num_entries*sizeof(group_add_entry_t));
    gae = &gav->vector[ gav->num_entries ];
    gav->num_entries = new_num_entries;

    // Fill in the entry
    gae->req_path = USP_STRDUP(req_path);
    gae->res_path = USP_STRDUP(res_path);
    gae->instance = INVALID;
    gae->group_id = group_id;
    gae->params = NULL;
    gae->num_params = 0;
    gae->err_code = USP_ERR_OK;
    gae->err_msg = NULL;
    KV_VECTOR_Init(&gae->unique_keys);
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_AddParamSetting
**
** Adds a parameter to set to the last object in the vector
**
** \param   gav - pointer to vector to add to
** \param   param_name - Name of parameter to set (NOTE: Not full path, just leaf name)
** \param   value - value of parameter to set
** \param   is_required - set if an error should be generated if this parameter fails to set
**
** \return  None
**
**************************************************************************/
void GROUP_ADD_VECTOR_AddParamSetting(group_add_vector_t *gav, char *param_name, char *value, bool is_required)
{
    int new_num_params;
    group_add_entry_t *gae;
    group_add_param_t *ps;

    // Increase the vector size
    gae = &gav->vector[ gav->num_entries - 1];
    new_num_params = gae->num_params + 1;
    gae->params = USP_REALLOC(gae->params, new_num_params*sizeof(group_add_param_t));
    ps = &gae->params[ gae->num_params ];
    gae->num_params = new_num_params;

    // Fill in the entry
    ps->param_name = param_name;  // NOTE: Ownership of this string stays with the caller
    ps->value = value;
    ps->is_required = is_required;
    ps->err_code = USP_ERR_OK;
    ps->err_msg = NULL;
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_CreateObject
**
** Creates the specified object using the settings previously setup
** NOTE: If any of the required parameters fail to set, this function ensures that the object instance does not exist,
** when this function returns, if the object was owned by a data model provider component
**
** \param   gae - pointer to object to add and it's parameter settings
** \param   combined_role - permission roles to use when performing the add
**
** \return  USP_ERR_OK if object created successfully
**
**************************************************************************/
int GROUP_ADD_VECTOR_CreateObject(group_add_entry_t *gae, combined_role_t *combined_role)
{
    int err;
    group_vendor_hook_t *gvh;

    // Objects that are owned by the internal data model cannot use a create vendor hook
    if (gae->group_id == NON_GROUPED)
    {
        err = CreateObject_WithoutCreateVendorHook(gae, combined_role);
        goto exit;
    }

    // Use a create vendor hook, if one has been registered
    gvh = &group_vendor_hooks[gae->group_id];
    if (gvh->create_obj_cb == NULL)
    {
        err = CreateObject_WithoutCreateVendorHook(gae, combined_role);
    }
    else
    {
        err = CreateObject_WithCreateVendorHook(gae, combined_role);
    }

exit:
    return err;
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_AddObjectNotCreated
**
** Called if the requested path fails to resolve, to mark that path as a failure in the group add vector
**
** \param   gav - pointer to vector to add to
** \param   req_path - requested data model path that failed
** \param   err_code - cause of failure to resolve the requested path
** \param   err_msg - err_msg describing the cause of failing to resolve the requested path
**
** \return  None
**
**************************************************************************/
void GROUP_ADD_VECTOR_AddObjectNotCreated(group_add_vector_t *gav, char *req_path, int err_code, char *err_msg)
{
    int new_num_entries;
    group_add_entry_t *gae;

    // Increase the vector size
    new_num_entries = gav->num_entries + 1;
    gav->vector = USP_REALLOC(gav->vector, new_num_entries*sizeof(group_add_entry_t));
    gae = &gav->vector[ gav->num_entries ];
    gav->num_entries = new_num_entries;

    // Fill in the entry
    gae->req_path = USP_STRDUP(req_path);
    gae->res_path = USP_STRDUP(req_path);
    gae->instance = INVALID;
    gae->group_id = NON_GROUPED;
    gae->params = NULL;
    gae->num_params = 0;
    gae->err_code = err_code;
    gae->err_msg = USP_STRDUP(err_msg);
    KV_VECTOR_Init(&gae->unique_keys);
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_FindFirstFailedParam
**
** Determines the first parameter which failed after attempting to set a newly ceated object's parameters
**
** \param   gae - pointer to object to add and it's parameter settings
**
** \return  pointer to param setting entry or NULL if no parameter marked as failed
**
**************************************************************************/
group_add_param_t *GROUP_ADD_VECTOR_FindFirstFailedParam(group_add_entry_t *gae)
{
    int i;
    group_add_param_t *ps;

    // Iterate over all parameters, finding the first required parameter which failed to set
    for (i=0; i < gae->num_params; i++)
    {
        ps = &gae->params[i];
        if ((ps->err_code != USP_ERR_OK) && (ps->is_required))
        {
            return ps;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** GROUP_ADD_VECTOR_Rollback
**
** Rolls back (ie Deletes) all successfully created objects in the group add vector
** This function is called only when allow_partial=false after an object fails to create
**
** \param   gav - pointer to vector containing newly created objects that we want to delete
** \param   rollback_span - one more than the instance number of the objects in the group add vector that have been created successfully
**
** \return  pointer to param setting entry or NULL if no parameter marked as failed
**
**************************************************************************/
void GROUP_ADD_VECTOR_Rollback(group_add_vector_t *gav, int rollback_span)
{
    int i;
    group_add_entry_t *gae;

    USP_ASSERT(rollback_span <= gav->num_entries);
    for (i=0; i<rollback_span; i++)
    {
        gae = &gav->vector[i];

        // This function is only called for objects which have created successfully (with allow_partial=false)
        USP_ASSERT(gae->err_code == USP_ERR_OK);

        // Rollback this object instance, by explicity deleting it
        RollbackGroupAddEntry(gae);
    }
}

/*********************************************************************//**
**
** CreateObject_WithCreateVendorHook
**
** Creates the specified object using the settings previously setup in the group add vector
** This function is only called for Objects owned by data model provider components
** NOTE: Errors to set the object are returned in gae, whilst failure to set individual parameters are returned in gae->params
** NOTE: If any required parameter fails to set in the create vendor hook, then the create vendor hook ensures that the object is not created
**
** \param   gae - pointer to object to add and it's parameter settings
** \param   combined_role - permission roles to use when performing the add
**
** \return  USP_ERR_OK if object created successfully
**
**************************************************************************/
int CreateObject_WithCreateVendorHook(group_add_entry_t *gae, combined_role_t *combined_role)
{
    int i;
    int err;
    dm_create_obj_cb_t create_obj_cb;
    group_add_param_t *gap;
    char path[MAX_DM_PATH];
    char buf[128];
    unsigned permission_bitmask;
    dm_node_t *node;

    // Validate all parameters
    for (i=0; i < gae->num_params; i++)
    {
        // Skip if the parameter does not exist in the schema
        gap = &gae->params[i];
        USP_SNPRINTF(path, sizeof(path), "%s.{i}.%s", gae->res_path, gap->param_name);
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
        if (node == NULL)
        {
            gap->err_code = USP_ERR_UNSUPPORTED_PARAM;
            gap->err_msg = USP_STRDUP(USP_ERR_GetMessage());
            continue;
        }

        USP_ASSERT(node->group_id == gae->group_id);

        // Skip if path is an object. This could occur when performing an AddObject, if the name of a child parameter was empty
        if (IsObject(node))
        {
            USP_SNPRINTF(buf, sizeof(buf), "%s: Parameter name is empty for child parameter of %s", __FUNCTION__, path);
            gap->err_code = USP_ERR_INVALID_ARGUMENTS;
            gap->err_msg = USP_STRDUP(buf);
            continue;
        }

        // Skip if path is not a parameter. This could occur when performing an AddObject, if the name of the child parameter is actually a child object, event or command
        if (IsParam(node)==false)
        {
            USP_SNPRINTF(buf, sizeof(buf), "%s: %s is not a parameter", __FUNCTION__, path);
            gap->err_code = USP_ERR_INTERNAL_ERROR;
            gap->err_msg = USP_STRDUP(buf);
            continue;
        }

        // Skip if parameter is read only
        if (node->type == kDMNodeType_VendorParam_ReadOnly)
        {
            USP_SNPRINTF(buf, sizeof(buf), "%s: Trying to perform a parameter set on read-only parameter %s", __FUNCTION__, path);
            gap->err_code = USP_ERR_PARAM_READ_ONLY;
            gap->err_msg = USP_STRDUP(buf);
            continue;
        }
        USP_ASSERT(node->type == kDMNodeType_VendorParam_ReadWrite);

        // Skip if no permission to write to parameter
        permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
        if ((permission_bitmask & PERMIT_SET)==0)
        {
            USP_SNPRINTF(buf, sizeof(buf), "%s: No permission to write to %s", __FUNCTION__, path);
            gap->err_code = USP_ERR_PERMISSION_DENIED;
            gap->err_msg = USP_STRDUP(buf);
            continue;
        }
    }

    // Exit if any required parameters failed the above validation
    for (i=0; i < gae->num_params; i++)
    {
        gap = &gae->params[i];
        if ((gap->err_code != USP_ERR_OK) && (gap->is_required))
        {
            gae->err_code = gap->err_code;
            gae->err_msg = USP_STRDUP(gap->err_msg);
            err = gap->err_code;
            goto exit;
        }
    }

    // Attempt to create the object and set its parameters using a single IPC operation
    create_obj_cb = group_vendor_hooks[gae->group_id].create_obj_cb;
    USP_ASSERT(create_obj_cb != NULL);      // Caller (GROUP_ADD_VECTOR_CreateObject) ensured this
    KV_VECTOR_Init(&gae->unique_keys);
    err = create_obj_cb(gae->group_id, gae->res_path, gae->params, gae->num_params, &gae->instance, &gae->unique_keys);

    // If an error occurred, store the cause of failure
    if (err != USP_ERR_OK)
    {
        gae->err_code = err;
        gae->err_msg = USP_STRDUP(USP_ERR_GetMessage());
    }

    // NOTE: It's the responsibility of the data model provider component to ensure that there are no
    // unique keys which have been left with a default value which is not unique. So we don't check it here

exit:
    return err;
}

/*********************************************************************//**
**
** CreateObject_WithoutCreateVendorHook
**
** Creates the specified object using the settings previously setup in the group add vector
** This function is called for Objects owned by data model provider components that don't have a create vendor hook
** and for objects owned by the internal data model
** NOTE: If any of the required parameters fail to set, this function ensures that the object instance does not exist,
** when this function returns, if the object was owned by a data model provider component
**
** \param   gae - pointer to object to add and it's parameter settings
** \param   combined_role - permission roles to use when performing the add
**
** \return  USP_ERR_OK if object created successfully
**
**************************************************************************/
int CreateObject_WithoutCreateVendorHook(group_add_entry_t *gae, combined_role_t *combined_role)
{
    int err;
    int instance;
    char full_path[MAX_DM_PATH];
    int i;
    int len;
    group_set_vector_t gsv;
    group_set_entry_t *src;
    group_add_param_t *dest;
    group_add_param_t *ps;
#ifdef BDK_USP
    bool isParamUniqueKey(group_set_entry_t *src, dm_unique_key_vector_t *uniqueKeyVector);
    dm_unique_key_vector_t uniqueKeyVector = {0};
    bool isErrorUniqueKey = 0;
    int uniqueKeyErr = USP_ERR_OK;
#endif    

    GROUP_SET_VECTOR_Init(&gsv);

    // Exit if unable to add the specified object (and set the default values of all its child parameters)
    err = DATA_MODEL_AddInstance(gae->res_path, &instance, CHECK_CREATABLE);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Create the path to the object
    gae->instance = instance;
    len = USP_SNPRINTF(full_path, sizeof(full_path), "%s.%d", gae->res_path, instance);

    // Add all parameters to be set to a group set vector
    for (i=0; i < gae->num_params; i++)
    {
        ps = &gae->params[i];
        USP_SNPRINTF(&full_path[len], sizeof(full_path)-len, ".%s", ps->param_name);
        GROUP_SET_VECTOR_Add(&gsv, full_path, ps->value, ps->is_required, combined_role);
    }

#ifdef BDK_USP    
    // Get list of unique key for this fullpath
    DATA_MODEL_GetUniqueKeys(gae->res_path,&uniqueKeyVector);
#endif    
    
    // Perform the set of the parameters for this object
    GROUP_SET_VECTOR_SetValues(&gsv, 0, gsv.num_entries);

    // Copy the error for each parameter from the group set vector, to the params vector
    // Exiting if we've hit a required parameter, which failed to set
    for (i=0; i < gsv.num_entries; i++)
    {
        src = &gsv.vector[i];
        dest = &gae->params[i];
        if (src->err_code != USP_ERR_OK)
        {
            dest->err_code = src->err_code;
            dest->err_msg = USP_STRDUP(src->err_msg); // NOTE: We cannot just move the string between the structures, because the string needs to stay in the group set vector as it maybe used by DATA_MODEL_ValidateDefaultedUniqueKeys

#ifdef BDK_USP
            /* In BDK, a unique key could be autogenerated, so the value is not always NULL.
             * This will fail the check of uniqueKey being NULL.  We will intercept here first.
             */
            if (!isErrorUniqueKey)
            {
                isErrorUniqueKey = isParamUniqueKey(src,&uniqueKeyVector);
                uniqueKeyErr = dest->err_code;
            }
#endif
            if (dest->is_required)
            {
                RollbackGroupAddEntry(gae);
                err = dest->err_code;
                goto cleanup;
            }
        }
    }

    // If the code gets here, then all required parameters for this object have been successfully set
    // So now we need to get the values of all parameters used as unique keys

    // Exit if unable to get the parameter values of the unique keys for this object
    full_path[len] = '\0';

#ifdef BDK_USP
    /* In BDK, we autogen the unique key, so the unique key value may not be NULL, 
     * we intercept the error here */
    if (isErrorUniqueKey)
    {
        err = uniqueKeyErr;
    }
    else
    {
        err = DATA_MODEL_GetUniqueKeyParams(full_path, &gae->unique_keys, combined_role);        
    }
#else
    err = DATA_MODEL_GetUniqueKeyParams(full_path, &gae->unique_keys, combined_role);    
#endif

    if (err != USP_ERR_OK)        
    {
        RollbackGroupAddEntry(gae);
        goto exit;
    }

    // Exit if any unique keys have been left with a default value which is not unique
    // NOTE: This only applies to the internal data model. It's the responsibility of the data model provider component to ensure this for it's data model
    // NOTE: This applies, even if the parameter was marked as not required
    if (gae->group_id == NON_GROUPED)
    {
        err = DATA_MODEL_ValidateDefaultedUniqueKeys(full_path, &gae->unique_keys, &gsv);
        if (err != USP_ERR_OK)
        {
            RollbackGroupAddEntry(gae);
            goto exit;
        }
    }
    err = USP_ERR_OK;

exit:
    // Store off the cause of failure to create this object, if an error occurred
    if (err != USP_ERR_OK)
    {
        USP_ASSERT(gae->err_msg == NULL);
        gae->err_code = err;
        gae->err_msg = USP_STRDUP(USP_ERR_GetMessage());
    }

cleanup:
    // Clean up
    GROUP_SET_VECTOR_Destroy(&gsv);

    return err;
}

/*********************************************************************//**
**
** RollbackGroupAddEntry
**
** Rolls back (ie Deletes) the specified object instance if it is owned by a data model provider component
**
** \param   gae - pointer to structure identifying object instance to rollback
**
** \return  None
**
**************************************************************************/
void RollbackGroupAddEntry(group_add_entry_t *gae)
{
    char path[MAX_DM_PATH];

    // Exit if the object exists in the internal data model
    // In this case, the object doesn't need to be explicitly deleted, as the caller will use DM_TRANS_Abort() to perform the rollback
    if (gae->group_id == NON_GROUPED)
    {
        return;
    }

    // Delete the instance, ignoring any errors (since we can't sensibly recover from them, or report them for a rollback
    USP_ASSERT(gae->instance != INVALID);
    USP_SNPRINTF(path, sizeof(path), "%s.%d", gae->res_path, gae->instance);
    DATA_MODEL_DeleteInstance(path, 0);
}

#ifdef BDK_USP
bool isParamUniqueKey(group_set_entry_t *src, dm_unique_key_vector_t *uniqueKeyVector)
{
    int i = 0;
    bool isUniqueKey = 0;
    dm_unique_key_t *key;
    
    if (uniqueKeyVector != NULL)
    {
        key = uniqueKeyVector->vector;
        for (i=0; i < MAX_COMPOUND_KEY_PARAMS; i++)
        {
            if (key->param[i] == NULL)
            {
                break;
            }
            /* src->path is the full path of the parameter, the uniqueKeyVector contain the parameter name only */
            if (strstr(src->path,key->param[i]) != NULL)
            {
                isUniqueKey = 1;
                break;
            }
        }
    }
    return (isUniqueKey);
}
#endif
