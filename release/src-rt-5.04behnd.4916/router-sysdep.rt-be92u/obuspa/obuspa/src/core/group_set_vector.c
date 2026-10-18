/*
 *
 * Copyright (C) 2020-2024, Broadband Forum
 * Copyright (C) 2020-2024  CommScope, Inc
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
 * \file group_set_vector.c
 *
 * Performs setting of a mixed group of parameters
 * The parameters are sorted by group, then each group is set en-masse by a single vendor hook for that group
 * The single vendor hook contacts the software component owning the parameters
 * Having a single call to set multiple parameters is more efficient,
 * especially if DBus/RPC messaging is needed to reach the software component owning the parameters
 *
 */

#include <string.h>

#include "common_defs.h"
#include "group_set_vector.h"
#include "int_vector.h"
#include "data_model.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int SetParameterGroup(int group_id, group_set_vector_t *gsv, int_vector_t *iv, bool is_required);
void SetParametersIndividually(group_set_vector_t *gsv, int index, int num_entries);

/*********************************************************************//**
**
** GROUP_SET_VECTOR_Init
**
** Initialises a group get vector structure
**
** \param   gsv - Pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void GROUP_SET_VECTOR_Init(group_set_vector_t *gsv)
{
    gsv->vector = NULL;
    gsv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_SET_VECTOR_Destroy
**
** Frees all memory used by the group set vector
**
** \param   gsv - pointer to vector to destroy
**
** \return  None
**
**************************************************************************/
void GROUP_SET_VECTOR_Destroy(group_set_vector_t *gsv)
{
    int i;
    group_set_entry_t *gse;

    for (i=0; i < gsv->num_entries; i++)
    {
        gse = &gsv->vector[i];
        USP_FREE(gse->path);
        USP_SAFE_FREE(gse->err_msg);
        // NOTE: value does not have to be freed, as ownership of it stays with the USP request message
    }
    USP_FREE(gsv->vector);

    gsv->vector = NULL;     // Not strictly necessary
    gsv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_SET_VECTOR_Add
**
** Adds the parameter to the group set vector, determining it's group_id and checking permissions
**
** \param   gsv - pointer to group set vector to add the parameter to
** \param   path - full data model path to the parameter
** \param   value - value that the parameter is going to be set to
** \param   is_required - whether the parameter is required to be set or not
** \param   combined_role - pointer to role used to access this path. If set to INTERNAL_ROLE(=NULL), then full permissions are always returned
**
** \return  None - errors are stored in the group set vector for processing later
**
**************************************************************************/
void GROUP_SET_VECTOR_Add(group_set_vector_t *gsv, char *path, char *value, bool is_required, combined_role_t *combined_role)
{
    char buf[128];
    unsigned path_properties;
    unsigned short permission_bitmask;
    int group_id = NON_GROUPED;  // This is the default, if the path is not in the schema
    int err_code = USP_ERR_OK;
    char *err_msg = NULL;
    int new_num_entries;
    group_set_entry_t *gse;
    unsigned type_flags;

    // Exit if path does not exist in the schema
    path_properties = DATA_MODEL_GetPathProperties(path, combined_role, &permission_bitmask, &group_id, &type_flags, 0);
    if ((path_properties & PP_EXISTS_IN_SCHEMA) == 0)
    {
        err_code = USP_ERR_UNSUPPORTED_PARAM;
        err_msg = USP_ERR_GetMessage();
        goto exit;
    }

    // Exit if path is an object. This could occur when performing an AddObject, if the name of a child parameter was empty
    if (path_properties & PP_IS_OBJECT)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: Parameter name is empty for child parameter of %s", __FUNCTION__, path);
        err_code = USP_ERR_INVALID_ARGUMENTS;
        err_msg = buf;
        goto exit;
    }

    // Exit if path is not a parameter. This could occur when performing an AddObject, if the name of the child parameter is actually a child object, event or command
    if ((path_properties & PP_IS_PARAMETER) == 0)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: %s is not a parameter", __FUNCTION__, path);
        err_code = USP_ERR_INTERNAL_ERROR;
        err_msg = buf;
        goto exit;
    }

    // Exit if parameter is read only
    if ((path_properties & PP_IS_WRITABLE)==0)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: Trying to perform a parameter set on read-only parameter %s", __FUNCTION__, path);
        err_code = USP_ERR_PARAM_READ_ONLY;
        err_msg = buf;
        goto exit;
    }

    // Exit if no permission to write to parameter
    if ((permission_bitmask & PERMIT_SET)==0)
    {
        USP_SNPRINTF(buf, sizeof(buf), "%s: No permission to write to %s", __FUNCTION__, path);
        err_code = USP_ERR_PERMISSION_DENIED;
        err_msg = buf;
        goto exit;
    }

    // If the code gets here, then there were no errors detected which would prevent the parameter from being set later

exit:
    // All cases result in adding the path to the group set vector
    // Increase the number of entries in the group set vector
    new_num_entries = gsv->num_entries + 1;
    gsv->vector = USP_REALLOC(gsv->vector, new_num_entries*sizeof(group_set_entry_t));
    gse = &gsv->vector[ gsv->num_entries ];
    gsv->num_entries = new_num_entries;

    // Fill in this new entry
    gse->path = USP_STRDUP(path);
    gse->group_id = group_id;
    gse->type_flags = type_flags;
    gse->value = value;                 // NOTE: Ownership of this stays with the USP Request message
    gse->is_required = is_required;
    gse->err_code = err_code;
    gse->err_msg = USP_STRDUP(err_msg);
}

/*********************************************************************//**
**
** GROUP_SET_VECTOR_GetFailureIndex
**
** Finds the first required parameter which failed in the specified slice of the group set vector
**
** \param   gsv - pointer to group set vector
** \param   index - index of first parameter in the group set vector to test
** \param   num_entries - number of parameters in the group set vector to test
**
** \return  index of the first required parameter which failed to set, in the group set vector
**
**************************************************************************/
int GROUP_SET_VECTOR_GetFailureIndex(group_set_vector_t *gsv, int index, int num_entries)
{
    int i;
    int param_index;
    group_set_entry_t *gse;

    for (i=0; i < num_entries; i++)
    {
        param_index = index + i;
        gse = &gsv->vector[param_index];
        if ((gse->err_code != USP_ERR_OK) && (gse->is_required))
        {
            return param_index;
        }
    }

    return INVALID;
}

/*********************************************************************//**
**
** GROUP_SET_VECTOR_SetValues
**
** Sets the value of the specified slice of parameters in the vector
**
** \param   gsv - Contains the list of parameters to set
** \param   index - index of first parameter to set in the vector
** \param   num_entries - number of parameters to set (from index onwards)
**
** \return  None - This function does not return an error code because error codes for each parameter are returned in the group set vector itself
**
**************************************************************************/
void GROUP_SET_VECTOR_SetValues(group_set_vector_t *gsv, int index, int num_entries)
{
    // Exit if any of the required parameters failed to set already (lack of permissions or invalid parameter name)
    int first_failure = GROUP_SET_VECTOR_GetFailureIndex(gsv, index, num_entries);
    if (first_failure != INVALID)
    {
        return;
    }

#ifdef SET_GROUPED_PARAMETERS_INDIVIDUALLY
    // Set each parameter in the list individually, stopping at the first required parameter which fails
    SetParametersIndividually(gsv, index, num_entries);
    return;
#endif

    // Set each parameter in the list by sorting into buckets based on group_id and required/non-required status
    // then perfoming group sets on each bucket
    int i;
    group_set_entry_t *gse;
    int_vector_t required_indexes[MAX_VENDOR_PARAM_GROUPS]; // Each entry in the array is a vector of indexes into the group set vector for required parameters of a software component group
    int_vector_t non_required_indexes[MAX_VENDOR_PARAM_GROUPS]; // Each entry in the array is a vector of indexes into the group set vector for non-required parameters of a software component group
    int_vector_t *iv;
    int param_index;
    int err;

    // Iterate over all parameters, setting them if non grouped, otherwise adding them to the relevant group to get
    memset(required_indexes, 0, sizeof(required_indexes));
    memset(non_required_indexes, 0, sizeof(non_required_indexes));
    for (i=0; i < num_entries; i++)
    {
        param_index = index + i;
        gse = &gsv->vector[param_index];
        if (gse->err_msg == NULL)           // Some parameters might already have failed (invalid path or permissions)
        {
            USP_ASSERT(gse->err_code == USP_ERR_OK);
            if (gse->group_id == NON_GROUPED)
            {
                // If the parameter is not grouped, then set its value now.
                gse->err_code = DATA_MODEL_SetParameterValue(gse->path, gse->value, CHECK_WRITABLE);
                if (gse->err_code != USP_ERR_OK)
                {
                    gse->err_msg = USP_STRDUP( USP_ERR_GetMessage() );

                    // Exit on the first required parameter that failed to set
                    if (gse->is_required)
                    {
                        goto exit;
                    }
                }
            }
            else
            {
                // If the parameter is grouped, then defer it for later with the rest of the parameters in its group
                iv = (gse->is_required) ? &required_indexes[gse->group_id] : &non_required_indexes[gse->group_id];
                INT_VECTOR_Add(iv, param_index);
            }
        }
    }

    // Set the parameters for each group
    for (i=0; i<MAX_VENDOR_PARAM_GROUPS; i++)
    {
        // Set the required parameters for this group (exiting if a required parameter failed to set)
        iv = &required_indexes[i];
        if (iv->num_entries > 0)
        {
            err = SetParameterGroup(i, gsv, iv, true);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        // Set the non-required parameters for this group (ignoring errors)
        iv = &non_required_indexes[i];
        if (iv->num_entries > 0)
        {
            SetParameterGroup(i, gsv, iv, false);
        }
    }

exit:
    // Clean up
    for (i=0; i<MAX_VENDOR_PARAM_GROUPS; i++)
    {
        INT_VECTOR_Destroy(&required_indexes[i]);
        INT_VECTOR_Destroy(&non_required_indexes[i]);
    }
}

/*********************************************************************//**
**
** SetParameterGroup
**
** Sets the value of a group of parameters specified by the indexes in the iv vector
**
** \param   group_id - GroupID of the parameters to set
** \param   ggv - Contains the list of parameters to set
** \param   iv - pointer to vector containing the index of each parameter to set for this group (index in group set vector)
** \param   is_required - determines the action to apply if the group set fails, based on whether the parameters are required or non-required
**
** \return  USP_ERR_OK if the subset of parametersall the specified parameter
**
**************************************************************************/
int SetParameterGroup(int group_id, group_set_vector_t *gsv, int_vector_t *iv, bool is_required)
{
    int i;
    kv_vector_t params;
    int index;
    dm_set_group_cb_t set_group_cb;
    kv_pair_t *kv;
    char err_msg[USP_ERR_MAXLEN];
    int err;
    group_set_entry_t *gse;
    unsigned *param_types = NULL;
    char *group_err_msg;
    int failure_index;
    char *failure_path;
    char *saved_err_msg;
    char *non_required_err_msg;

    // Exit if there is no callback defined for this group
    set_group_cb = group_vendor_hooks[group_id].set_group_cb;
    if (set_group_cb == NULL)
    {
        // Mark all results for params in this group with an error
        for (i=0; i < iv->num_entries; i++)
        {
            index = iv->vector[i];
            gse = &gsv->vector[index];

            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: No registered group callback to set param %s", __FUNCTION__, gse->path);
            gse->err_code = USP_ERR_INTERNAL_ERROR;
            gse->err_msg = USP_STRDUP(err_msg);
        }
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add all parameters to set in this group to a key value vector
    // NOTE: We form the key value vector manually to avoid copying the param paths and values
    //       Ownership of the param paths stay with the group set vector, and ownership of the values stay with the USP request message
    params.num_entries = iv->num_entries;
    params.vector = USP_MALLOC((iv->num_entries) * sizeof(kv_pair_t));
    param_types = USP_MALLOC((iv->num_entries) * sizeof(unsigned));
    for (i=0; i < iv->num_entries; i++)
    {
        index = iv->vector[i];
        gse = &gsv->vector[index];
        kv = &params.vector[i];

        kv->key = gse->path;
        kv->value = gse->value;
        param_types[i] = gse->type_flags;
    }

    // Exit if group set callback succeeded
    USP_ERR_ClearMessage();
    failure_index = INVALID;
    err = set_group_cb(group_id, &params, param_types, &failure_index);
    if (err == USP_ERR_OK)
    {
        goto exit;
    }

    // If the code gets here, an error occurred
    // Exit if no specific param failed. This could occur if (say) messaging failed between the agent and
    // the software component implementing this group of parameters
    if ((failure_index == INVALID) || (failure_index >= params.num_entries))
    {
        USP_ERR_ReplaceEmptyMessage("%s: Set group callback failed", __FUNCTION__);
        group_err_msg = USP_ERR_GetMessage();

        // Mark all results for params in this group with an error
        for (i=0; i < iv->num_entries; i++)
        {
            index = iv->vector[i];
            gse = &gsv->vector[index];

            gse->err_code = err;
            gse->err_msg = USP_STRDUP( group_err_msg );
        }
        goto exit;
    }

    // If the code gets here, we know the first parameter that failed
    failure_path = params.vector[failure_index].key;
    USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: group set failed at '%s' (%s)", __FUNCTION__, failure_path, USP_ERR_UspErrToString(err));
    saved_err_msg = USP_ERR_GetMessage();
    group_err_msg = (*saved_err_msg != '\0') ? saved_err_msg : err_msg;

    // If we were attempting to set required parameters, then just mark the first parameter causing the failure
    // (They all will have been wound back)
    if (is_required)
    {
        index = iv->vector[failure_index];
        gse = &gsv->vector[index];
        gse->err_code = err;
        gse->err_msg = USP_STRDUP( group_err_msg );
        goto exit;
    }

    // If the code gets here, then we were attempting to set non-required parameters
    // Since one of the parameters failed, all of them will have been wound back, so mark all of them as failing
    for (i=0; i < iv->num_entries; i++)
    {
        index = iv->vector[i];
        gse = &gsv->vector[index];

        gse->err_code = err;
        non_required_err_msg = (i == failure_index) ? group_err_msg : err_msg;
        gse->err_msg = USP_STRDUP( non_required_err_msg );
    }

exit:
    // Destroy the key-value vector.
    // As it never owned any of the strings in it, we only have to free the array itself
    USP_FREE(params.vector);
    USP_FREE(param_types);
    return err;
}

/*********************************************************************//**
**
** SetParametersIndividually
**
** Sets each parameter in the specified list individually
**
** \param   group_id - GroupID of the parameters to set
** \param   ggv - Contains the list of parameters to set
** \param   index - index of first parameter to set in the vector
** \param   num_entries - number of parameters to set (from index onwards)
**
** \return  None - This function does not return an error code because error codes for each parameter are returned in the group set vector itself
**
**************************************************************************/
void SetParametersIndividually(group_set_vector_t *gsv, int index, int num_entries)
{
    int i;
    int param_index;
    group_set_entry_t *gse;
    dm_set_group_cb_t set_group_cb;
    kv_vector_t pv;
    kv_pair_t param;
    int failure_index;

    // Iterate over all parameters, setting them in the order that they occur in the array
    for (i=0; i < num_entries; i++)
    {
        param_index = index + i;
        gse = &gsv->vector[param_index];
        if (gse->err_msg == NULL)           // Some parameters might already have failed (invalid path or permissions)
        {
            USP_ASSERT(gse->err_code == USP_ERR_OK);
            if (gse->group_id == NON_GROUPED)
            {
                // Non-grouped parameters can directly call DATA_MODEL_SetParameterValue()
                gse->err_code = DATA_MODEL_SetParameterValue(gse->path, gse->value, CHECK_WRITABLE);
            }
            else
            {
                // Grouped parameters cannot call DATA_MODEL_SetParameterValue(), as that would cause infinite recursion
                set_group_cb = group_vendor_hooks[gse->group_id].set_group_cb;
                if (set_group_cb == NULL)
                {
                    // Set an error message, if no group callback registered for this parameter
                    USP_ERR_SetMessage("%s: No registered group callback to set param %s", __FUNCTION__, gse->path);
                    gse->err_code = USP_ERR_INTERNAL_ERROR;
                }
                else
                {
                    // Set this grouped parameter individually using the group set callback
                    pv.num_entries = 1;
                    pv.vector = &param;
                    param.key = gse->path;
                    param.value = gse->value;

                    USP_ERR_ClearMessage();
                    failure_index = INVALID;
                    gse->err_code = set_group_cb(gse->group_id, &pv, &gse->type_flags, &failure_index);
                    if (gse->err_code != USP_ERR_OK)
                    {
                        USP_ERR_ReplaceEmptyMessage("%s: group set failed for '%s' (%s)", __FUNCTION__, gse->path, USP_ERR_UspErrToString(gse->err_code));
                    }
                }
            }

            // Exit on the first required parameter that failed to set
            if (gse->err_code != USP_ERR_OK)
            {
                gse->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
                if (gse->is_required)
                {
                    return;
                }
            }
        }
    }
}
