/*
 *
 * Copyright (C) 2020, Broadband Forum
 * Copyright (C) 2020  CommScope, Inc
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
 * \file group_get_vector.c
 *
 * Performs getting of a mixed group of parameters
 * The parameters are sorted by group, then each group is get en-masse by a single vendor hook for that group
 * The single vendor hook contacts the software component owning the parameters
 * Having a single call to get multiple parameters is more efficient,
 * especially if DBus/RPC messaging is needed to reach the software component owning the parameters
 *
 */

#include <string.h>

#include "common_defs.h"
#include "group_get_vector.h"
#include "int_vector.h"
#include "data_model.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void GetParameterGroup(int group_id, group_get_vector_t *ggv, int_vector_t *iv);
void GetParametersIndividually(group_get_vector_t *ggv);

/*********************************************************************//**
**
** GROUP_GET_VECTOR_Init
**
** Initialises a group get vector structure
**
** \param   ggv - Pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void GROUP_GET_VECTOR_Init(group_get_vector_t *ggv)
{
    ggv->vector = NULL;
    ggv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_Destroy
**
** Frees all memory used by the group get vector
**
** \param   ggv - pointer to vector to destroy
**
** \return  None
**
**************************************************************************/
void GROUP_GET_VECTOR_Destroy(group_get_vector_t *ggv)
{
    int i;
    group_get_entry_t *gge;

    for (i=0; i < ggv->num_entries; i++)
    {
        gge = &ggv->vector[i];
        USP_FREE(gge->path);
        USP_SAFE_FREE(gge->value);
        USP_SAFE_FREE(gge->err_msg);
    }
    USP_FREE(ggv->vector);

    ggv->vector = NULL;     // Not strictly necessary
    ggv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_Add
**
** Adds the specified parameter (and associated group_id) to the specified group get vector
**
** \param   ggv - pointer to group get vector to add to
** \param   path - data model path of parameter to get. This is copied into the group get vector.
** \param   group_id - group_id associated with parameter to get
**
** \return  None
**
**************************************************************************/
void GROUP_GET_VECTOR_Add(group_get_vector_t *ggv, char *path, int group_id)
{
    int new_num_entries;
    group_get_entry_t *gge;

    // Increase the number of entries in the group get vector
    new_num_entries = ggv->num_entries + 1;
    ggv->vector = USP_REALLOC(ggv->vector, new_num_entries*sizeof(group_get_entry_t));
    gge = &ggv->vector[ ggv->num_entries ];
    ggv->num_entries = new_num_entries;

    // Fill in this new entry
    gge->path = USP_STRDUP(path);
    gge->group_id = group_id;
    gge->value = NULL;
    gge->err_code = USP_ERR_OK;
    gge->err_msg = NULL;
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_FindParam
**
** Finds the index of the specified parameter in the specified group get vector
**
** \param   ggv - pointer to group get vector to search
** \param   path - path of parameter to find
**
** \return  None
**
**************************************************************************/
int GROUP_GET_VECTOR_FindParam(group_get_vector_t *ggv, char *path)
{
    int i;
    group_get_entry_t *gge;

    // Iterate over all entries in the group get vector searching for a match
    for (i=0; i < ggv->num_entries; i++)
    {
        gge = &ggv->vector[i];
        if (strcmp(gge->path, path)==0)
        {
            return i;
        }
    }

    // If the code gets here, then no match was found
    return INVALID;
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_AddParams
**
** Adds the specified set of parameters (and associated group_id) to the specified group get vector
** IMPORTANT: Ownership of the strings in the params vector transfers to the group get vector
**
** \param   ggv - pointer to vector to expand
** \param   params - string vector containing the list of parameters to add
**                   NOTE: Ownership of the strings in the params vector transfers to the group get vector
** \param   group_ids - group_ids associated with each parameter
**
** \return  None
**
**************************************************************************/
void GROUP_GET_VECTOR_AddParams(group_get_vector_t *ggv, str_vector_t *params, int_vector_t *group_ids)
{
    int i;
    int start_index;
    group_get_entry_t *gge;

    USP_ASSERT(params->num_entries == group_ids->num_entries)

    start_index = ggv->num_entries;
    ggv->num_entries += params->num_entries;
    ggv->vector = USP_REALLOC(ggv->vector, (ggv->num_entries)*sizeof(group_get_entry_t));

    // Transfer the parameter path strings and group_ids to the group get vector
    for (i=0; i < params->num_entries; i++)
    {
        gge = &ggv->vector[start_index + i];
        gge->path = params->vector[i];
        gge->group_id = group_ids->vector[i];
        gge->value = NULL;
        gge->err_code = USP_ERR_OK;
        gge->err_msg = NULL;
    }
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_ConvertToKeyValueVector
**
** Converts the group get vector into a key value vector, destroying the group get vector in the process,
** avoiding as much memory allocation and copying as possible.
**
** \param   ggv - pointer to group get vector to convert
** \param   kvv - pointer to key-value pair vector destination structure
**
** \return  None
**
**************************************************************************/
void GROUP_GET_VECTOR_ConvertToKeyValueVector(group_get_vector_t *ggv, kv_vector_t *kvv)
{
    int i;
    int num_entries;
    group_get_entry_t *gge;
    kv_pair_t *pair;

    // Allocate a key value vector to store the results
    num_entries = ggv->num_entries;
    kvv->vector = USP_MALLOC(num_entries*sizeof(kv_pair_t));
    kvv->num_entries = num_entries;

    // Copy across path and value into key value vector
    for (i=0; i < num_entries; i++)
    {
        gge = &ggv->vector[i];
        pair = &kvv->vector[i];

        pair->key = gge->path;
        if ((gge->value != NULL) && (gge->err_code == USP_ERR_OK) && (gge->err_msg == NULL))
        {
            pair->value = gge->value;
        }
        else
        {
            // Intentionally ignoring errors that occurred whilst getting the value by returning an empty string if they occur
            pair->value = USP_STRDUP("");
        }

        USP_SAFE_FREE(gge->err_msg);
    }

    // Empty the group get vector
    USP_SAFE_FREE(ggv->vector);
    ggv->vector = NULL;
    ggv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_GET_VECTOR_GetValues
**
** Gets the value of all parameters in the vector
**
** \param   ggv - Contains the list of parameters to get and (after getting) stores the values
**
** \return  None - This function does not return an error code because error codes for each parameter are returned in the group get vector itself
**
**************************************************************************/
void GROUP_GET_VECTOR_GetValues(group_get_vector_t *ggv)
{
    int i;
    group_get_entry_t *gge;
    char buf[MAX_DM_VALUE_LEN];
    int_vector_t ggv_indexes[MAX_VENDOR_PARAM_GROUPS]; // Array of vectors
    int_vector_t *iv;

#ifdef GET_GROUPED_PARAMETERS_INDIVIDUALLY
    // Get each parameter in the list individually, stopping at the first required parameter which fails
    GetParametersIndividually(ggv);
    return;
#endif

    // Iterate over all parameters, getting them if non grouped, otherwise adding them to the relevant group to get
    memset(ggv_indexes, 0, sizeof(ggv_indexes));
    for (i=0; i < ggv->num_entries; i++)
    {
        gge = &ggv->vector[i];
        if (gge->group_id == NON_GROUPED)
        {
            // If the parameter is not grouped, then get its value now.
            gge->err_code = DATA_MODEL_GetParameterValue(gge->path, buf, sizeof(buf), 0);
            if (gge->err_code != USP_ERR_OK)
            {
                gge->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
            }
            else
            {
                gge->value = USP_STRDUP(buf);
            }
        }
        else
        {
            // If the parameter is grouped, then defer it for later with the rest of the parameters in its group
            iv = &ggv_indexes[gge->group_id];
            INT_VECTOR_Add(iv, i);
        }
    }

    // Get the parameters for each group
    for (i=0; i<MAX_VENDOR_PARAM_GROUPS; i++)
    {
        iv = &ggv_indexes[i];
        if (iv->num_entries > 0)
        {
            GetParameterGroup(i, ggv, iv);
        }
    }

    // Clean up
    for (i=0; i<MAX_VENDOR_PARAM_GROUPS; i++)
    {
        INT_VECTOR_Destroy(&ggv_indexes[i]);
    }
}

/*********************************************************************//**
**
** GetParameterGroup
**
** Gets the value of a group of parameters specified by the indexes in the iv vector
**
** \param   group_id - GroupID of the parameters to get
** \param   ggv - Contains the list of parameters to get and (after getting) stores the values
** \param   iv - pointer to vector containing the index of each parameter to get for this group (index in group get vector)
**
** \return  None
**
**************************************************************************/
void GetParameterGroup(int group_id, group_get_vector_t *ggv, int_vector_t *iv)
{
    int i;
    kv_vector_t params;
    int index;
    dm_get_group_cb_t get_group_cb;
    kv_pair_t *kv;
    char err_msg[USP_ERR_MAXLEN];
    int err;
    group_get_entry_t *gge;
    char *usp_err_msg;

    // Exit if there is no callback defined for this group
    get_group_cb = group_vendor_hooks[group_id].get_group_cb;
    if (get_group_cb == NULL)
    {
        // Mark all results for params in this group with an error
        for (i=0; i < iv->num_entries; i++)
        {
            index = iv->vector[i];
            gge = &ggv->vector[index];
            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: No registered group callback to get param %s", __FUNCTION__, gge->path);
            gge->err_code = USP_ERR_INTERNAL_ERROR;
            gge->err_msg = USP_STRDUP(err_msg);
        }
        return;
    }

    // Add all parameters to get in this group to a key value vector
    // NOTE: We form the key value vector manually to avoid copying the param paths.
    //       Ownership of the param paths stay with the group get vector
    params.num_entries = iv->num_entries;
    params.vector = USP_MALLOC(sizeof(kv_pair_t)*(iv->num_entries));
    for (i=0; i < iv->num_entries; i++)
    {
        index = iv->vector[i];
        gge = &ggv->vector[index];
        USP_ASSERT(gge->path != NULL);

        kv = &params.vector[i];
        kv->key = gge->path;
        kv->value = NULL;
    }

    // Exit if group callback fails
    USP_ERR_ClearMessage();
    err = get_group_cb(group_id, &params);
    if (err != USP_ERR_OK)
    {
        // Mark all results for params in this group with an error
        usp_err_msg = USP_ERR_GetMessage();
        for (i=0; i < iv->num_entries; i++)
        {
            index = iv->vector[i];
            gge = &ggv->vector[index];
            gge->err_code = USP_ERR_INTERNAL_ERROR;

            // Assign an error message to this param
            if (usp_err_msg[0] != '\0')
            {
                gge->err_msg = USP_STRDUP(usp_err_msg);
            }
            else
            {
                // Form an error message if none was provided
                USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Get group callback failed for param %s", __FUNCTION__, gge->path);
                gge->err_msg = USP_STRDUP(err_msg);
            }

            // NOTE: The group get might have populated a value for some params, so free these values
            USP_SAFE_FREE(params.vector[i].value);
        }
        goto exit;
    }

    // Move all parameter values obtained to the group get vector
    // NOTE: Ownership of the value string transfers from the params vector to the group get vector
    for (i=0; i < iv->num_entries; i++)
    {
        kv = &params.vector[i];
        index = iv->vector[i];
        gge = &ggv->vector[index];

        if (kv->value != NULL)
        {
            gge->value = kv->value;
        }
        else
        {
            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Get group callback did not provide a value for param %s", __FUNCTION__, gge->path);
            gge->err_code = USP_ERR_INTERNAL_ERROR;
            gge->err_msg = USP_STRDUP(err_msg);
        }
    }

exit:
    // Destroy the key-value vector.
    // As ownership of all strings in it have transferred to the group get vector, we only have to free the array itself
    USP_FREE(params.vector);
}

/*********************************************************************//**
**
** GetParametersIndividually
**
** Gets each parameter in the specified list individually
**
** \param   ggv - Contains the list of parameters to get and (after getting) stores the values
**
** \return  None - This function does not return an error code because error codes for each parameter are returned in the group get vector itself
**
**************************************************************************/
void GetParametersIndividually(group_get_vector_t *ggv)
{
    int i;
    group_get_entry_t *gge;
    char buf[MAX_DM_VALUE_LEN];
    dm_get_group_cb_t get_group_cb;
    kv_vector_t pv;
    kv_pair_t param;

    // Iterate over all parameters, getting them in the order they occur in the array
    for (i=0; i < ggv->num_entries; i++)
    {
        gge = &ggv->vector[i];
        if (gge->group_id == NON_GROUPED)
        {
            // Non-grouped parameters can directly call DATA_MODEL_GetParameterValue()
            gge->err_code = DATA_MODEL_GetParameterValue(gge->path, buf, sizeof(buf), 0);
            if (gge->err_code == USP_ERR_OK)
            {
                gge->value = USP_STRDUP(buf);
            }
        }
        else
        {
            // Grouped parameters cannot call DATA_MODEL_GetParameterValue(), as that would cause infinite recursion
            get_group_cb = group_vendor_hooks[gge->group_id].get_group_cb;
            if (get_group_cb == NULL)
            {
                // Set an error message, if no group callback registered for this parameter
                USP_ERR_SetMessage("%s: No registered group callback to get param %s", __FUNCTION__, gge->path);
                gge->err_code = USP_ERR_INTERNAL_ERROR;
            }
            else
            {
                // Get this grouped parameter individually using the group get callback
                pv.num_entries = 1;
                pv.vector = &param;
                param.key = gge->path;
                param.value = NULL;

                USP_ERR_ClearMessage();
                gge->err_code = get_group_cb(gge->group_id, &pv);
                if (gge->err_code != USP_ERR_OK)
                {
                    USP_ERR_ReplaceEmptyMessage("%s: group get failed for '%s' (%s)", __FUNCTION__, gge->path, USP_ERR_UspErrToString(gge->err_code));
                    USP_SAFE_FREE(param.value)
                }
                else
                {
                    if (param.value != NULL)
                    {
                        // Move ownership of the returned string from param.value to gge->value
                        gge->value = param.value;
                        param.value = NULL;  // not strictly necessary
                    }
                    else
                    {
                        // If no value was returned, then this is also reported as an error in the group get array
                        USP_ERR_ReplaceEmptyMessage("%s: Get group callback did not provide a value for param %s", __FUNCTION__, gge->path);
                        gge->err_code = USP_ERR_INTERNAL_ERROR;
                    }
                }
            }
        }

        // Save the error message
        if (gge->err_code != USP_ERR_OK)
        {
            gge->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
        }
    }
}
