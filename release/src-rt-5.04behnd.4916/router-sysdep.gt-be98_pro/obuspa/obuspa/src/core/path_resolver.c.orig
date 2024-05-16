/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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
 * \file path_resolver.c
 *
 * Resolves path expressions into individual parameters or objects which exist in the data model
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common_defs.h"
#include "data_model.h"
#include "dm_inst_vector.h"
#include "path_resolver.h"
#include "dm_access.h"
#include "kv_vector.h"
#include "expr_vector.h"
#include "text_utils.h"
#include "group_get_vector.h"

//-------------------------------------------------------------------------
// State variable associated with the resolver. This is passed to all recursive resolver functions
typedef struct
{
    str_vector_t *sv;       // pointer to string vector to return the resolved paths in
                            // or NULL if we are only interested in whether the expression exists in the schema
    int_vector_t *gv;       // pointer to integer vector in which to return the group_id of the resolved parameters
                            // or NULL if we are not interesetd in group_id (eg if the expression describes objects not parameters)
    resolve_op_t op;        // operation being performed that requires path resolution
    int depth;              // Number of hierarchical levels to traverse in the data model when performing partial path resolution
    combined_role_t *combined_role;  // pointer to role to use when performing the path resolution.
                            // If the search path resolves to an object or param which there is no permission for,
                            // then a error will be generated (or the path forgivingly ignored in the case of a get)
    unsigned flags;         // flags controlling resolving of the path eg GET_ALL_INSTANCES
} resolver_state_t;

// Structure containing unique key search variables
// The indexes in keys[] and ggv_indexes[] refer to the same key
// The indexes in ggv and key_types[] refer to the same key (for keys containing references)
// ggv_indexes maps a key in keys to it's entry in ggv. This is needed because some keys are not in the ggv due to lack of permissions or contain an empty reference
typedef struct
{
    expr_vector_t keys;     // expression keys used for unique key search
    int_vector_t ggv_indexes; // group get index vector, maps to each {instance, key} pair, points to ggv index
                            // or INVALID if controller does not have read permission for that {instance, key} pair
    group_get_vector_t ggv; // group get vector for the valid parameters
    int_vector_t key_types; // integer vector for valid key types
} search_param_t;

//--------------------------------------------------------------------
// Typedef for the compare callback
typedef int (*dm_cmp_cb_t)(char *lhs, expr_op_t op, char *rhs, bool *result);

//-------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int ExpandPath(char *resolved, char *unresolved, resolver_state_t *state);
int ExpandWildcard(char *resolved, char *unresolved, resolver_state_t *state);
int ResolveReferenceFollow(char *resolved, char *unresolved, resolver_state_t *state);
int ResolveUniqueKey(char *resolved, char *unresolved, resolver_state_t *state);
int DoesInstanceMatchUniqueKey(char *object, int instance, expr_vector_t *keys, bool *is_match, resolver_state_t *state);
int ResolvePartialPath(char *path, resolver_state_t *state);
int GetChildParams(char *path, int path_len, dm_node_t *node, dm_instances_t *inst, resolver_state_t *state, int depth_remaining);
int GetChildParams_MultiInstanceObject(char *path, int path_len, dm_node_t *node, dm_instances_t *inst, resolver_state_t *state, int depth_remaining);
int AddPathFound(char *path, resolver_state_t *state);
int CountPathSeparator(char *path);
int CheckPathProperties(char *path, resolver_state_t *state, bool *add_to_vector, unsigned *path_properties, int *group_id);
int GetGroupIdForUniqueKeys(char *object, expr_vector_t *keys, resolver_state_t *state, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm);
void ExpandUniqueKeysOverAllInstances(char *object, int_vector_t *instances, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm, search_param_t *sp);
void ExpandUniqueKeysOverSingleInstance(char *object, int instance, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm, search_param_t *sp);
int DoUniqueKeysMatch(int index, search_param_t *sp, bool *is_match);
int SplitReferenceKeysFromkeys(expr_vector_t *all_keys, expr_vector_t *keys, expr_vector_t *ref_keys);
int ExpandsNonReferencedKeys(char *resolved, resolver_state_t *state, int_vector_t *instances, search_param_t *sp);
int ResolveReferencedKeys(char *resolved, resolver_state_t *state, int_vector_t *instances, search_param_t *sp);
int ResolveIntermediateReferences(str_vector_t *params, resolver_state_t *state, int_vector_t *perm);
bool GroupReferencedParameters(str_vector_t *params, resolver_state_t *state, int_vector_t *perm, group_get_vector_t *ggv, int *err);
void InitSearchParam(search_param_t *sp);
void DestroySearchParam(search_param_t *sp);
void RefreshInstances_LifecycleSubscriptionEndingInPartialPath(char *path);

/*********************************************************************//**
**
** PATH_RESOLVER_ResolveDevicePath
**
** Wrapper around PATH_RESOLVER_ResolvePath() which ensures that the path starts with 'Device.'
** This function therefore does not allow querying of 'Internal.' database parameters
** This function should be used by all USP protocol message handlers (since 'Internal.' is not exposed to controllers)
** However CLI commands can directly use PATH_RESOLVER_ResolvePath()
**
** \param   path - pointer to path expression identifying parameters in the data model
** \param   sv - pointer to string vector to return the resolved paths in
**               or NULL if we are only interested in whether the expression exists in the schema
**               NOTE: As this function can be used to append to a string vector, it does not initialise
**                     the vector, so the caller must initialise the vector.
**                     Also, the caller must destroy the vector, even if an error is returned
** \param   gv - pointer to vector in which to return the group_id of the parameters
**               or NULL if the caller is not interested in this
**               NOTE: values in sv and gv relate by index
** \param   op - operation being performed that requires path resolution
** \param   depth - Number of hierarchical levels to traverse in the data model when performing partial path resolution
** \param   combined_role - role to use when performing the resolution. If set to INTERNAL_ROLE, then permissions are ignored (used internally)
** \param   flags - flags controlling resolving of the path eg GET_ALL_INSTANCES
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int PATH_RESOLVER_ResolveDevicePath(char *path, str_vector_t *sv, int_vector_t *gv, resolve_op_t op, int depth, combined_role_t *combined_role, unsigned flags)
{
    int err;
    int len;

    // Exit if the path does not begin with "Device."
    #define DEVICE_ROOT_STR "Device."
    if (strncmp(path, DEVICE_ROOT_STR, sizeof(DEVICE_ROOT_STR)-1) != 0)
    {
        USP_ERR_SetMessage("%s: Expression does not start in '%s' (path='%s')", __FUNCTION__, DEVICE_ROOT_STR, path);
        return USP_ERR_INVALID_PATH;
    }

    // Perform checks on whether the path is terminated correctly (by '.' or not)
    len = strlen(path);
    if (path[len-1] == '.')
    {
        // Path ends in '.'
        // Exit if the path should not end in '.'
        if ((op==kResolveOp_Oper) || (op==kResolveOp_Event))
        {
            USP_ERR_SetMessage("%s: Path should not end in '.'", __FUNCTION__);
            return USP_ERR_INVALID_PATH_SYNTAX;
        }
    }
    else
    {
        // Path does not end in '.'
        // Exit if the path should end in '.'
        if ((op==kResolveOp_Add) || (op==kResolveOp_Del) || (op==kResolveOp_Instances))
        {
            USP_ERR_SetMessage("%s: Path must end in '.'", __FUNCTION__);
            return USP_ERR_INVALID_PATH_SYNTAX;
        }
    }

    err = PATH_RESOLVER_ResolvePath(path, sv, gv, op, depth, combined_role, flags);
    return err;
}

/*********************************************************************//**
**
** PATH_RESOLVER_ResolvePath
**
** Resolves the specified path expression into a vector of parameter paths which exist in the data model
** Resolution involves resolving wildcarded, key based addressing, reference following and expression based searching based expressions
** Resolution takes account of permissions, potentially failing the resolution if sufficient permissions are not available for the specified role
** NOTE: The string vector is assumed to be initialised before this function is called, allowing this function to append to the list
**
** \param   path - pointer to path expression identifying parameters in the data model
** \param   sv - pointer to string vector to return the resolved paths in
**               or NULL if we are only interested in whether the expression exists in the schema
**               NOTE: As this function can be used to append to a string vector, it does not initialise
**                     the vector, so the caller must initialise the vector.
**                     Also, the caller must destroy the vector, even if an error is returned
** \param   gv - pointer to vector in which to return the group_id of the parameters
**               or NULL if the caller is not interested in this
**               NOTE: values in sv and gv relate by index
** \param   op - operation being performed that requires path resolution
** \param   depth - Number of hierarchical levels to traverse in the data model when performing partial path resolution
** \param   combined_role - role to use when performing the resolution
*  \param   flags - flags controlling resolving of the path eg GET_ALL_INSTANCES
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int PATH_RESOLVER_ResolvePath(char *path, str_vector_t *sv, int_vector_t *gv, resolve_op_t op, int depth, combined_role_t *combined_role, unsigned flags)
{
    char resolved[MAX_DM_PATH];
    char unresolved[MAX_DM_PATH];
    int err;
    resolver_state_t state;

    // Use of the gv argument is only valid for paths that describe parameters
    USP_ASSERT((gv==NULL) || (op==kResolveOp_Get) || (op==kResolveOp_Set) || (op==kResolveOp_SubsValChange) || (op==kResolveOp_GetBulkData));

    // Exit if path contains any path separators with no intervening objects
    if (strstr(path, "..") != NULL)
    {
        USP_ERR_SetMessage("%s: Path should not contain '..'", __FUNCTION__);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Take a copy of the path expression, so that the code below may alter the unresolved buffer
    USP_STRNCPY(unresolved, path, sizeof(unresolved));

    // Set up state variables for resolving the path, then resolve it
    resolved[0] = '\0';  // Start from an empty string for the resolved portion of the path
    state.sv = sv;
    state.gv = gv;
    state.op = op;
    state.depth = depth;
    state.combined_role = combined_role;
    state.flags = flags;

    err = ExpandPath(resolved, unresolved, &state);

    return err;
}

/*********************************************************************//**
**
** ExpandPath
**
** Iterates over all unresolved aspects of the path, resolving them into a path
** NOTE: This function is recursive
**
** \param   resolved - pointer to buffer containing data model path that has been resolved so far
** \param   unresolved - pointer to rest of search path to resolve
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ExpandPath(char *resolved, char *unresolved, resolver_state_t *state)
{
    int len;
    int err;
    char c;
    bool check_refresh_instances = false;

    // Exit if path is too long
    len = strlen(resolved);
    if (len >= MAX_DM_PATH-1)
    {
        USP_ERR_SetMessage("%s(%d): path expansion too long. Aborting at %s", __FUNCTION__, __LINE__, resolved);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Then iterate over 'unresolved', appending to the buffer in 'resolved', until we hit one of the addressing operators (ie '*', '[', or '+')
    c = *unresolved;
    while (c != '\0')
    {
        // If hit a wildcard, handle it (and rest of unresolved), then exit
        if (c == '*')
        {
            resolved[len] = '\0';
            err = ExpandWildcard(resolved, &unresolved[1], state);
            return err;
        }

        // If hit a reference follow, handle it (and rest of unresolved), then exit
        if (c == '+')
        {
            resolved[len] = '\0';
            err = ResolveReferenceFollow(resolved, &unresolved[1], state);
            return err;
        }

        // If hit a unique key address, handle it (and rest of unresolved), then exit
        if (c == '[')
        {
            resolved[len] = '\0';
            err = ResolveUniqueKey(resolved, &unresolved[1], state);
            return err;
        }

        // Exit if unable to append any more characters to 'resolved'
        if (len >= MAX_DM_PATH-1)
        {
            resolved[len] = '\0';
            USP_ERR_SetMessage("%s(%d): path expansion too long. Aborting at %s", __FUNCTION__, __LINE__, resolved);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Append this character to the path
        resolved[len++] = c;

        // Move to the next character
        unresolved++;
        c = *unresolved;
    }

    // If the code gets here, then we have finished parsing the search path
    // So turn it into a string
    resolved[len] = '\0';

    // Remove trailing '.' from the path
    if (resolved[len-1] == '.')
    {
        switch(state->op)
        {
            case kResolveOp_Get:
            case kResolveOp_SubsValChange:
            case kResolveOp_GetBulkData:
            case kResolveOp_SubsOper:
            case kResolveOp_SubsEvent:
                // These cases allow a partial path for parameters
                resolved[len-1] = '\0';
                err = ResolvePartialPath(resolved, state);
                return err;
                break;

            case kResolveOp_SubsAdd:
            case kResolveOp_SubsDel:
                // Remove any trailing '.'  The partial path may potentially call a refresh instances vendor hook to be called
                resolved[len-1] = '\0';
                check_refresh_instances = true;
                break;

            case kResolveOp_Add:
            case kResolveOp_Del:
            case kResolveOp_Set:
            case kResolveOp_Instances:
            case kResolveOp_Any:
            case kResolveOp_StrictRef:
            case kResolveOp_ForgivingRef:
                // These cases do not process a partial path - just remove any trailing '.'
                resolved[len-1] = '\0';
                break;

            default:
            case kResolveOp_Oper:
            case kResolveOp_Event:
                // These cases should never occur (as code in PATH_RESOLVER_ResolveDevicePath prevents this case)
                USP_ASSERT(false);
                break;

        }
    }

    // Exit if an error occurred with this path, which halts further path resolution
    err = AddPathFound(resolved, state);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Partial path for add/delete object subscriptions must ensure that object instances are refreshed
    // Do this by getting the instances for this object (all sub objects are also refreshed in the process)
    if (check_refresh_instances)
    {
        RefreshInstances_LifecycleSubscriptionEndingInPartialPath(resolved);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** RefreshInstances_LifecycleSubscriptionEndingInPartialPath
**
** Refreshes the instance numbers of a top level object referenced by an object lifetime subscription
**
** The code in RefreshInstancesForObjLifetimeSubscriptions() periodically
** refreshes all instances which have object lifetime subscriptions on them
** in order to determine whether the subscription should fire.
** This function is called if the ReferenceList of the subscription is a partial path.
** It ensures that the refresh instances vendor hook is called, if it wouldn't have been
** already during path resolution. The only time it wouldn't have been called is if the
** path resolver resolves to a partial path of a top level multi-instance object
**
** \param   path - path of the object to potentially refresh
**
** \return  None
**
**************************************************************************/
void RefreshInstances_LifecycleSubscriptionEndingInPartialPath(char *path)
{
    dm_node_t *node;
    bool is_qualified_instance;
    dm_object_info_t *info;
    dm_instances_t inst;

    // Exit if unable to find node representing this object. NOTE: This should never occur, as caller should have ensured path exists in schema
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance);
    if (node == NULL)
    {
        return;
    }

    // Exit if this is not a top level multi-instance object with a refresh instances vendor hook
    // NOTE: If path is to a child object whose parent has a refresh instances vendor hook,
    //       then the vendor hook will already have been called as part of resolving the path, so no need to refresh here
    // NOTE: The path resolver disallows object lifecycle subscriptions on partial paths that are not multi-instance objects
    //       so this code does not have to cope with calling the refresh instances vendor hook for a child object of the given path.
    info = &node->registered.object_info;
    if ((node->type != kDMNodeType_Object_MultiInstance) || (node->order != 1) || (info->refresh_instances_cb == NULL))
    {
        return;
    }

    // Exit if this object is already a fully qualified instance
    // NOTE: This may be the case if the subscription ReferenceList terminated in wildcard or instance number before the partial path dot character
    // If so, the refresh instances vendor hook would already have been called
    if (is_qualified_instance)
    {
        return;
    }

    // NOTE: This function may be called recursively if it is time to call the refresh instances vendor hook
    // The first time DM_INST_VECTOR_RefreshTopLevelObjectInstances() is called, if it calls the refresh instances vendor hook,
    // then afterwards it will determine if any of the instances caused the subscription to fire.
    // It does this by calling the path resolver, which will end up in this function again.
    // The second time that DM_INST_VECTOR_RefreshTopLevelObjectInstances() is called, the instances cache
    // will not need refreshing, hence DM_INST_VECTOR_RefreshTopLevelObjectInstances() will return the second time it is called.
    DM_INST_VECTOR_RefreshTopLevelObjectInstances(node);
}

/*********************************************************************//**
**
** ExpandWildcard
**
** Expands the wildcard that exists inbetween 'resolved' and 'unresolved' parts of the path
** then reurses to resolve the rest of the path
** NOTE: This function is recursive
**
** \param   resolved - pointer to buffer containing object that we need to search all instances of
** \param   unresolved - pointer to rest of search path to resolve
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ExpandWildcard(char *resolved, char *unresolved, resolver_state_t *state)
{
    int_vector_t iv;
    int i;
    int err;
    int len;
    int len_left;
    char *p;

    // Exit if unable to get the instances of this object
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(resolved, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if there are no instances of this object
    if (iv.num_entries == 0)
    {
        goto exit;
    }

    // Exit if no space left in the buffer to append the instance number
    len = strlen(resolved);
    len_left = MAX_DM_PATH - len;
    if (len_left < 2)       // 2 to include a single digit and NULL terminator
    {
        resolved[len] = '\0';
        USP_ERR_SetMessage("%s(%d): path expansion too long. Aborting at %s", __FUNCTION__, __LINE__, resolved);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Expand the wildcard and recurse to expand the unresolved part of the path
    p = &resolved[len];
    for (i=0; i < iv.num_entries; i++)
    {
        USP_SNPRINTF(p, len_left, "%d", iv.vector[i]);
        err = ExpandPath(resolved, unresolved, state);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** ResolveReferenceFollow
**
** De-references the specified data model path, then recurses to resolve the rest of the path
** NOTE: This function is recursive
**
** \param   resolved - pointer to buffer containing data model path to de-reference
** \param   unresolved - pointer to rest of search path to resolve
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ResolveReferenceFollow(char *resolved, char *unresolved, resolver_state_t *state)
{
    char dereferenced[MAX_DM_PATH];
    int err;
    unsigned flags;
    unsigned short permission_bitmask;
    char *p;

    // Exit if this is a Bulk Data collection operation, which does not allow reference following
    // (because the alt-name reduction rules in TR-157 do not support it)
    if (state->op == kResolveOp_GetBulkData)
    {
        USP_ERR_SetMessage("%s: Bulk Data collection does not allow reference following in search expressions", __FUNCTION__);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Exit if unable to determine whether we are allowed to read the reference
    err = DATA_MODEL_GetPermissions(resolved, state->combined_role, &permission_bitmask);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if not permitted to read the reference
    if ((permission_bitmask & PERMIT_GET) == 0)
    {
        // Get operations are forgiving of permissions, so just give up further resolution here
        #define IS_FORGIVING(op) ((op == kResolveOp_Get) || (op == kResolveOp_SubsValChange) || (op == kResolveOp_ForgivingRef))
        #define IS_STRICT(op)    ((op != kResolveOp_Get) && (op != kResolveOp_SubsValChange) && (op != kResolveOp_ForgivingRef))
        if (IS_FORGIVING(state->op))
        {
            return USP_ERR_OK;
        }

        // Other operations are not forgiving, so return an error
        USP_ERR_SetMessage("%s: Not permitted to read reference follow %s", __FUNCTION__, resolved);
        return USP_ERR_PERMISSION_DENIED;
    }

    // Exit if unable to get the path for the dereferenced object
    err = DATA_MODEL_GetParameterValue(resolved, dereferenced, sizeof(dereferenced), 0);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Unable to get the value of the dereferenced path contained in %s", __FUNCTION__, resolved);
        return err;
    }

    // Exit if the reference was empty
    // NOTE: A get parameter value is forgiving in this case, whilst a set fails
    if (dereferenced[0] == '\0')
    {
        if (IS_STRICT(state->op))
        {
            USP_ERR_SetMessage("%s: The dereferenced path contained in %s was empty", __FUNCTION__, resolved);
            return USP_ERR_OBJECT_DOES_NOT_EXIST;
        }
        return USP_ERR_OK;
    }

    // Truncate string to just the first reference, if the reference contains a list of references
    // The USP Spec says that only the first reference should be used if the '#' operator is omitted before the '+' operator
    p = strchr(dereferenced, ',');
    if (p != NULL)
    {
        *p = '\0';
    }

    // Resolve the reference if it contains a search expression, reference following or wildcard
    if (strpbrk(dereferenced, "[+#*]") != NULL)
    {
        str_vector_t sv;
        resolve_op_t op;

        // Determine resolve operation to use when resolving the reference
        op = IS_FORGIVING(state->op) ? kResolveOp_ForgivingRef : kResolveOp_StrictRef;

        // Exit if unable to resolve any search expressions contained in the reference
        STR_VECTOR_Init(&sv);
        err = PATH_RESOLVER_ResolvePath(dereferenced, &sv, NULL, op, FULL_DEPTH, state->combined_role, 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if the reference resolved to zero paths
        // NOTE: This may be the case. For example, if the reference contains unique key based addressing and the role does not have permissions to read them
        if (sv.num_entries == 0)
        {
            // NOTE: No need to destroy sv, as we already know that number of entries is zero
            if (IS_STRICT(state->op))
            {
                USP_ERR_SetMessage("%s: The dereferenced path contained in %s (%s) resolved to empty", __FUNCTION__, resolved, dereferenced);
                return USP_ERR_OBJECT_DOES_NOT_EXIST;
            }
            return USP_ERR_OK;
        }

        // Replace the value of the reference parameter with its first resolved path
        USP_STRNCPY(dereferenced, sv.vector[0], sizeof(dereferenced));
        STR_VECTOR_Destroy(&sv);
    }

    // Exit if the dereferenced path is not a fully qualified object
    // NOTE: We do not check permissions here, since there may be further parts of the path to resolve after this reference follow
    flags = DATA_MODEL_GetPathProperties(dereferenced, INTERNAL_ROLE, NULL, NULL, NULL);
    if ( ((flags & PP_IS_OBJECT) == 0) || ((flags & PP_IS_OBJECT_INSTANCE) ==0) )
    {
        USP_ERR_SetMessage("%s: The dereferenced path contained in %s was not an object instance (got the value '%s')", __FUNCTION__, resolved, dereferenced);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Exit if the dereferenced path does not have instance numbers that exist
    // NOTE: A get parameter value is forgiving in this case, whilst a set fails
    if ((flags & PP_INSTANCE_NUMBERS_EXIST) == 0)
    {
        if (IS_STRICT(state->op))
        {
            USP_ERR_SetMessage("%s: The dereferenced object %s does not exist", __FUNCTION__, dereferenced);
            return USP_ERR_OBJECT_DOES_NOT_EXIST;
        }
        return USP_ERR_OK;
    }

    // If the code gets here then the resolved path has been successfully dereferenced,
    // so continue resolving the path, using the dereferenced path
    err = ExpandPath(dereferenced, unresolved, state);

    return err;
}

/*********************************************************************//**
**
** SplitReferenceKeysFromkeys
**
** Split expression keys in all_keys with reference parameters and non-referenced parameters
**
** \param   all_keys - Pointer to all expression keys
** \param   keys - pointer to expression keys which does not have a referenced parameter
** \param   ref_keys - pointer to expression keys with referenced parameter
**
** \return  USP_ERR_OK if split succeed, or err if reference keys doesn't have an object
**
**************************************************************************/
int SplitReferenceKeysFromkeys(expr_vector_t *all_keys, expr_vector_t *keys, expr_vector_t *ref_keys)
{
    int i;
    expr_comp_t *ec;
    char *is_ref;
    size_t param_len;

    EXPR_VECTOR_Init(keys);
    EXPR_VECTOR_Init(ref_keys);

    for (i=0; i < all_keys->num_entries; i++)
    {
        ec = &all_keys->vector[i];
        is_ref = strchr(ec->param, '+');

        if (is_ref)
        {
            param_len = strlen(ec->param);
            // Error if reference follow does not have a key after the reference object
            if (ec->param[param_len - 1] == '+')
            {
                USP_ERR_SetMessage("%s: Key (%s) does not terminate in a parameter name. References must be to objects.", __FUNCTION__, ec->param);
                return USP_ERR_INVALID_PATH_SYNTAX;
            }

            EXPR_VECTOR_Add(ref_keys, ec->param, ec->op, ec->value);
        }
        else
        {
            EXPR_VECTOR_Add(keys, ec->param, ec->op, ec->value);
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CheckPathPermission
**
** Checks permission of the datamodel object path for uniqe key search
**
** \param   path - Datamodel object path
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   gid - pointer to group_id variable to get the group_id of path object
** \param   param_type - pointer to type_flags to get the parameter type of path object
** \param   has_permission - pointer to permission flag to get the read permission of the path object
**
** \return  USP_ERR_OK if controller has permission of parameters
**
**************************************************************************/
int CheckPathPermission(char *path, resolver_state_t *state, int *gid, int *param_type, bool *has_permission)
{
    unsigned flags;
    unsigned short permission_bitmask;
    unsigned param_type_flags;
    int param_group_id;

    // Initialize with default values
    *has_permission = true;

    // Exit if the path is not a parameter
    flags = DATA_MODEL_GetPathProperties(path, state->combined_role, &permission_bitmask, &param_group_id, &param_type_flags);
    if ((flags & PP_IS_PARAMETER) == 0)
    {
        USP_ERR_SetMessage("%s: Search key '%s' is not a parameter", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    if ((permission_bitmask & PERMIT_GET) == 0)
    {
        *has_permission = false;
        // Get operations are forgiving of permissions, so just indicate that none of the instances match
        // NOTE: BulkData get operations are not forgiving of permissions, so will return an error
        if (IS_STRICT(state->op))
        {
            USP_ERR_SetMessage("%s: Not permitted to read unique key %s", __FUNCTION__, path);
            return USP_ERR_PERMISSION_DENIED;
        }
    }

    *gid = param_group_id;
    if (param_type)
    {
        *param_type = (int)param_type_flags;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GroupReferencedParameters
**
** Create group get vector with referenced path present in params vector,
** and check permission for the considered parameters.
**
** \param   params - pointer to path vectors with referenced parameters
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   perm - pointer to permission vector, containing permissions of the params
** \param   ggv - pointer to group get vector to populate with the intermediate referenced key
** \param   err - pointer to error variable, err in case controller does not have permission on referenced keys
**
** \return  True if params vector still have path with references
**
**************************************************************************/
bool GroupReferencedParameters(str_vector_t *params, resolver_state_t *state, int_vector_t *perm, group_get_vector_t *ggv, int *err)
{
    int i;
    char *path;
    char *ref;
    bool is_ref_found;
    int param_group_id;
    bool has_permission;

    // Initialise with default values
    is_ref_found = false;
    *err = USP_ERR_OK;

    // Form a vector containing the paths to the next references to resolve
    for (i = 0; i < params->num_entries; i++)
    {
        path = params->vector[i];

        // No need to resolve this path further as controller does not have permission
        if (perm->vector[i] == false)
        {
            continue;
        }

        ref = strchr(path, '+');
        if (ref)
        {
            ref[0] = '\0';
            *err = CheckPathPermission(path, state, &param_group_id, NULL, &has_permission);
            if (*err != USP_ERR_OK)
            {
                ref[0] = '+';
                goto exit;
            }

            // Only add the path to ggv, if the current object+key has permission
            // along with previous object+key pair
            if (has_permission == true)
            {
                is_ref_found = true;
                GROUP_GET_VECTOR_Add(ggv, path, param_group_id);
            }
            else
            {
                perm->vector[i] = false;
            }
            ref[0] = '+';
        }
    }

exit:
    return is_ref_found;
}

/*********************************************************************//**
**
** ResolveIntermediateReferences
**
** De-references paths of params vector in place
**
** \param   params - pointer to path vectors with referenced parameters
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   perm - pointer to permission vector, containing permissions of the params
**
** \return  USP_ERR_OK if successful, or no prams with referenced keys
**
**************************************************************************/
int ResolveIntermediateReferences(str_vector_t *params, resolver_state_t *state, int_vector_t *perm)
{
    int i;
    int index;
    int err;
    char temp[MAX_DM_PATH];
    char *ref;
    group_get_vector_t ggv;
    group_get_entry_t *gge;

    // Initialise with default values
    GROUP_GET_VECTOR_Init(&ggv);

    // Loop until all path references(+) of params vector resolved, or error occurred
    while (GroupReferencedParameters(params, state, perm, &ggv, &err))
    {
        index = 0;
        // exit if error occurred in GroupReferencedParameters call
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Get the values of referenced paths only
        GROUP_GET_VECTOR_GetValues(&ggv);
        for (i = 0; i < params->num_entries; i++)
        {
            // Update the params vector in-place with the new referenced path value and rest of the key
            // if the index has get permission
            ref = strchr(params->vector[i], '+');
            if (ref && (perm->vector[i] == true))
            {
                gge = &ggv.vector[index];
                index++;

                // Exit if an error occurred when getting the unique key param, or no parameter was provided
                if (gge->err_code != USP_ERR_OK)
                {
                    USP_ERR_SetMessage("%s: Failed when defererencing %s (%s)", __FUNCTION__, gge->path, gge->err_msg);
                    err = gge->err_code;
                    goto exit;
                }
                USP_ASSERT(gge->value != NULL);     // GROUP_GET_VECTOR_GetValues() should have set an error message if the vendor hook didn't set a value for the parameter
                ref[0] = '\0';

                // If the dereferenced path is not a fully qualified object, mark permission as false for this instance
                // to avoid further processing.
                // NOTE: This applies to both gets and sets. Sets are forgiving if ANY instance matches, when using keys containing references
                if(strlen(gge->value) == 0)
                {
                    if (IS_STRICT(state->op))
                    {
                        USP_ERR_SetMessage("%s: The dereferenced path contained in '%s' was not an object instance (got the value '%s')", __FUNCTION__, gge->path, gge->value);
                    }
                    perm->vector[i] = false;
                    continue;
                }
                USP_SNPRINTF(temp, sizeof(temp), "%s%s", gge->value, &ref[1]);

                // Free the previous value and update the new resolved path
                USP_FREE(params->vector[i]);
                params->vector[i] = USP_STRDUP(temp);
            }
        }
        GROUP_GET_VECTOR_Destroy(&ggv);
    }

exit:
    GROUP_GET_VECTOR_Destroy(&ggv);
    return err;
}

/*********************************************************************//**
**
** ResolveReferencedKeys
**
** Resolves the intermediate references to form a group get vector of final parameters to get
**
** \param   resolved - pointer to data model object that we want to lookup by unique key
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   instances - instances of base object to get
** \param   sp - pointer to search parameter structure to return ggv_index, ggv and key_types for each valid {instance, key} pair
**
** \return  USP_ERR_OK if successful, or no referenced unique keys present
**
**************************************************************************/
int ResolveReferencedKeys(char *resolved, resolver_state_t *state, int_vector_t *instances, search_param_t *sp)
{
    char temp[MAX_DM_PATH];
    int err, i, j;
    expr_comp_t *ec;
    str_vector_t params;
    int_vector_t perm;
    int ggv_index;
    int group_id;
    int param_type;
    bool has_permission;

    // It is possible to have only non-referenced unique keys
    if (sp->keys.num_entries == 0)
    {
        return USP_ERR_OK;
    }

    // Exit if unable to get a list of all parameters referenced by the expression
    STR_VECTOR_Init(&params);
    INT_VECTOR_Init(&perm);

    // Create a STR vector of the parameters path for further resolution
    for (i = 0; i < instances->num_entries; i++)
    {
        for (j = 0; j < sp->keys.num_entries; j++)
        {
            ec = &sp->keys.vector[j];
            USP_SNPRINTF(temp, sizeof(temp), "%s%d.%s", resolved, instances->vector[i], ec->param);
            STR_VECTOR_Add(&params, temp);
            // Initialise each {instance, key} pair with valid permission, latter on if any reference doesn't
            // have read permission, it will be updated to Invalid
            INT_VECTOR_Add(&perm, true);
        }
    }

    // Resolve all references within params vector inline, to leave only final parameter to get
    // this will only have list of parameter for which controller has read permission
    err = ResolveIntermediateReferences(&params, state, &perm);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // This checks permission on the fully resolved parameter list and also gets
    // group_ids and key_types of the parameters.
    ggv_index = 0;
    for (i = 0; i < params.num_entries; i++)
    {
        // mark permission vector as invalid if intermediate nodes does not have permissions
        if (perm.vector[i] == false)
        {
            INT_VECTOR_Add(&sp->ggv_indexes, INVALID);
        }
        else
        {
            // Check permission of final parameter list, mark ggv_index as invalid if controller
            // does not have read permission of that parameter,
            // or it should point to the valid ggv index which shall be used to do compare in DoUniqueKeysMatch
            err = CheckPathPermission(params.vector[i], state, &group_id, &param_type, &has_permission);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            if (has_permission == true)
            {
                GROUP_GET_VECTOR_Add(&sp->ggv, params.vector[i], group_id);
                INT_VECTOR_Add(&sp->ggv_indexes, ggv_index);
                INT_VECTOR_Add(&sp->key_types, param_type);
                ggv_index++;
            }
            else
            {
                INT_VECTOR_Add(&sp->ggv_indexes, INVALID);
            }
        }
    }

exit:
    STR_VECTOR_Destroy(&params);
    INT_VECTOR_Destroy(&perm);

    return err;
}

/*********************************************************************//**
**
** ExpandsNonReferencedKeys
**
** Expands keys over instances to create a group get vector for the non
** referenced keys present in unique key search
**
** \param   resolved - pointer to data model object that we want to lookup by unique key
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   instances - instances of base object to get
** \param   sp - pointer to search parameter structure to return ggv_index, ggv and key_types for each valid {instance, key} pair
**
** \return  USP_ERR_OK if successful, or no non-referenced keys present
**
**************************************************************************/
int ExpandsNonReferencedKeys(char *resolved, resolver_state_t *state, int_vector_t *instances, search_param_t *sp)
{
    int err;
    int_vector_t group_ids;
    int_vector_t perm;
    int_vector_t key_types;

    // It is possible to have non-referenced unique keys
    if (sp->keys.num_entries == 0)
        return USP_ERR_OK;

    INT_VECTOR_Init(&group_ids);
    INT_VECTOR_Init(&perm);
    INT_VECTOR_Init(&key_types);

    // Get the group IDs of all unique key parameters, this also checks that we have permissions to read the parameters
    // If we don't have permissions, then the path resolution may fail either with an error (eg for SET) or silently (eg for GET)
    err = GetGroupIdForUniqueKeys(resolved, &sp->keys, state, &group_ids, &key_types, &perm);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Populate the search parameter structure with unique keys for all instances
    ExpandUniqueKeysOverAllInstances(resolved, instances, &group_ids, &key_types, &perm, sp);

exit:
    INT_VECTOR_Destroy(&group_ids);
    INT_VECTOR_Destroy(&perm);
    INT_VECTOR_Destroy(&key_types);
    return err;
}

/*********************************************************************//**
**
** ResolveUniqueKey
**
** Resolves the unique key
** NOTE: This function is recursive
**
** \param   resolved - pointer to data model object that we want to lookup by unique key
** \param   unresolved - pointer to unique key and rest of search path to resolve
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ResolveUniqueKey(char *resolved, char *unresolved, resolver_state_t *state)
{
    expr_vector_t keys;
    int i;
    int err;
    char *p;
    int len;
    int_vector_t instances;
    search_param_t sp;
    search_param_t ref_sp;
    char temp[MAX_DM_PATH];
    bool is_match;
    bool is_ref_match;
    expr_op_t valid_ops[] = {kExprOp_Equal, kExprOp_NotEqual, kExprOp_LessThanOrEqual, kExprOp_GreaterThanOrEqual, kExprOp_LessThan, kExprOp_GreaterThan};

    // Exit if unable to find the end of the unique key
    p = strchr(unresolved, ']');
    if (p == NULL)
    {
        USP_ERR_SetMessage("%s: Unterminated Unique Key (%s) in search path", __FUNCTION__, unresolved);
        return USP_ERR_INVALID_PATH_SYNTAX;
    }

    // Initialise vectors used by this function
    EXPR_VECTOR_Init(&keys);
    INT_VECTOR_Init(&instances);
    InitSearchParam(&sp);
    InitSearchParam(&ref_sp);

    // Exit if unable to get the instances of this object
    err = DATA_MODEL_GetInstances(resolved, &instances);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if there are no instances of this object
    if (instances.num_entries == 0)
    {
        err = USP_ERR_OK;
        goto exit;
    }

    // Exit if the unique key is too long
    len = p - unresolved;
    if (len > MAX_DM_PATH-1)
    {
        USP_ERR_SetMessage("%s: Unique Key too long (%s) in search path", __FUNCTION__, unresolved);
        err = USP_ERR_INVALID_PATH_SYNTAX;
        goto exit;
    }

    // Copy the unique key expressions (ie the expression within []) into temp
    memcpy(temp, unresolved, len);
    temp[len] = '\0';
    unresolved = &p[1];

    // If the code gets here, unresolved points to the character after ']', and temp contains the unique key expression

    // Exit if an error occurred whilst parsing the key expressions
    err = EXPR_VECTOR_SplitExpressions(temp, &keys, "&&", valid_ops, NUM_ELEM(valid_ops), EXPR_FROM_USP);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if no key expressions were found
    if (keys.num_entries == 0)
    {
        USP_ERR_SetMessage("%s: No unique key found in search path before %s", __FUNCTION__, unresolved);
        err = USP_ERR_INVALID_PATH_SYNTAX;
        goto exit;
    }

    // split the keys in referenced unique keys and non-referenced unique keys, this is required as the permissions info
    // for the non-referenced unique keys available in schema but for referenced keys, it depends on the value of
    // referenced parameter keys
    err = SplitReferenceKeysFromkeys(&keys, &sp.keys, &ref_sp.keys);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // once the keys are divided in non-referenced keys and referenced keys, keys not required for further
    // operations
    EXPR_VECTOR_Destroy(&keys);

    // Update the group get vector for non-referenced unique key parameters, this also checks that we have permissions to read the parameters
    // If we don't have permissions, then the path resolution may fail either with an error (eg for SET) or silently (eg for GET)
    err = ExpandsNonReferencedKeys(resolved, state, &instances, &sp);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Update the group get vector for referenced unique key parameters, this also checks that we have permissions to
    // read the referenced parameters, If we don't have permissions, then the path resolution may fail either with an
    // error (eg for SET) or silently (eg for GET)
    err = ResolveReferencedKeys(resolved, state, &instances, &ref_sp);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // For optimisation call the group gets on non-referenced keys and referenced-keys only after permission validation
    // has been done
    GROUP_GET_VECTOR_GetValues(&sp.ggv);
    GROUP_GET_VECTOR_GetValues(&ref_sp.ggv);

    // Iterate over all instances of the object present in the data model
    for (i=0; i < instances.num_entries; i++)
    {
        // Exit if an error occurred whilst trying to determine whether this instance matched the unique key
        err = DoUniqueKeysMatch(i, &sp, &is_match);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if an error occurred whilst trying to determine whether this instance matched the unique key
        // it is okay to have no ref_keys present in query, in case of that is_ref_match will be marked as true
        err = DoUniqueKeysMatch(i, &ref_sp, &is_ref_match);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // If found an instance which matches, continue resolving the path recursively, selecting this instance
        if (is_match & is_ref_match)
        {
            USP_SNPRINTF(temp, sizeof(temp), "%s%d", resolved, instances.vector[i]);
            err = ExpandPath(temp, unresolved, state);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    // If the code gets here, then no matching unique key has been found
    // It is not a parse error to find no instances of an object.
    // The caller (USP message handler) will deal with the case of no objects found appropriately.
    err = USP_ERR_OK;

exit:
    // Ensure that the key expressions and key-values are deleted
    // NOTE: This is safe to do again here, even if they have already been deleted in the body of the function
    INT_VECTOR_Destroy(&instances);
    EXPR_VECTOR_Destroy(&keys);
    DestroySearchParam(&sp);
    DestroySearchParam(&ref_sp);
    return err;
}

/*********************************************************************//**
**
** GetGroupIdForUniqueKeys
**
** Gets the GroupIds of all parameters in the specified unique key
** NOTE: For efficiency reasons, this function also checks that we have permission to read the unique keys
**
** \param   object - data model path of object to see if it matches the unique key
** \param   keys - vector of key expressions that specify the unique key
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   group_ids - pointer to vector in which to return the group_id of the parameters
** \param   key_types - pointer to vector in which to return the type_flags of the parameters
** \param   perm - pointer to vector in which to return the permission of the parameters
**
** \return  USP_ERR_OK if no errors occurred
**
**************************************************************************/
int GetGroupIdForUniqueKeys(char *object, expr_vector_t *keys, resolver_state_t *state, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm)
{
    int i;
    expr_comp_t *ec;
    char path[MAX_DM_PATH];
    int param_group_id;
    int param_type_flags;
    int err;
    bool has_permission;

    // Iterate over all unique keys, checking their permissions and getting their group_id
    for (i=0; i < keys->num_entries; i++)
    {
        // Form parameter path of the unique key to check
        // NOTE: DATA_MODEL_GetPathProperties() requires a non-schema path, so just choose 1 for the instance number (the path does not have to be instantiated to get the path's properties)
        ec = &keys->vector[i];
        USP_SNPRINTF(path, sizeof(path), "%s1.%s", object, ec->param);

        err = CheckPathPermission(path, state, &param_group_id, &param_type_flags, &has_permission);
        if (err != USP_ERR_OK)
        {
            return err;
        }
        INT_VECTOR_Add(perm, has_permission);
        INT_VECTOR_Add(group_ids, param_group_id);
        INT_VECTOR_Add(key_types, param_type_flags);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ExpandUniqueKeysOverAllInstances
**
** Fills in the group get vector with all unique key parameters to get (for all instances of an object)
**
** \param   object - data model path of base object
** \param   instances - instances of base object to get
** \param   group_ids - pointer to vector in which to return the group_id of the parameters
** \param   key_types - pointer to vector in which to return the type_flags of the parameters
** \param   perm - pointer to vector in which to return the permission of the parameters
** \param   sp - pointer to search parameter structure to return ggv_index, ggv and key_types for each valid {instance, key} pair
**
** \return  None
**
**************************************************************************/
void ExpandUniqueKeysOverAllInstances(char *object, int_vector_t *instances, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm, search_param_t *sp)
{
    int i;

    // Iterate over all instances of the object present in the data model
    for (i = 0; i < instances->num_entries; i++)
    {
        ExpandUniqueKeysOverSingleInstance(object, instances->vector[i], group_ids, key_types, perm, sp);
    }
}

/*********************************************************************//**
**
** ExpandUniqueKeysOverSingleInstance
**
** Calculates the full paths of the unique keys for a specific object instance, returning them in the params vector
**
** \param   object - data model path of object to see if it matches the unique key
** \param   instance - instance number of the object to see if it matches the unique key
** \param   group_ids - int vector containing group_id of each datamodel object
** \param   key_types - int vector containing key_types of each datamodel object
** \param   perm - int vector containing read permission of each datamodel object
** \param   sp - pointer to search parameter structure to return ggv_index, ggv and key_types for each valid {instance, key} pair
**
** \return  None
**
**************************************************************************/
void ExpandUniqueKeysOverSingleInstance(char *object, int instance, int_vector_t *group_ids, int_vector_t *key_types, int_vector_t *perm, search_param_t *sp)
{
    int i;
    expr_comp_t *ec;
    char path[MAX_DM_PATH];
    int ggv_index;

    ggv_index = sp->ggv.num_entries;
    // Form vector of unique key params for this instance
    for (i=0; i < sp->keys.num_entries; i++)
    {
        // In case controller does not have read permission for the parameter,
        // point ggv_index to INVALID, or a valid ggv index if it does.
        if (perm->vector[i] == false)
        {
            INT_VECTOR_Add(&sp->ggv_indexes, INVALID);
        }
        else
        {
            INT_VECTOR_Add(&sp->ggv_indexes, ggv_index);
            ec = &sp->keys.vector[i];
            USP_SNPRINTF(path, sizeof(path), "%s%d.%s", object, instance, ec->param);
            GROUP_GET_VECTOR_Add(&sp->ggv, path, group_ids->vector[i]);
            INT_VECTOR_Add(&sp->key_types, key_types->vector[i]);
            ggv_index++;
        }
    }
}

/*********************************************************************//**
**
** DoUniqueKeysMatch
**
** Determines whether a set of unique keys match
**
** \param   index - current instance index number for which unique key match is performed
** \param   sp - pointer to search parameter structure containing ggv_index, ggv and key_types for each valid {instance, key} pair
** \param   is_match - pointer to variable in which to return whether the unique keys match
**
** \return  USP_ERR_OK if no errors occurred
**
**************************************************************************/
int DoUniqueKeysMatch(int index, search_param_t *sp, bool *is_match)
{
    int i;
    int err;
    expr_comp_t *ec;
    group_get_entry_t *gge;
    bool result;
    unsigned type_flags;
    dm_cmp_cb_t cmp_cb;
    int perm_index;
    int ggv_index;

    // Assume that this instance does not match
    *is_match = false;
    perm_index = index * sp->keys.num_entries;

    // Iterate over all key expressions to match, exiting on the first one which isn't true
    for (i=0; i < sp->keys.num_entries; i++)
    {
        ggv_index = sp->ggv_indexes.vector[perm_index + i];

        // Exit if we previously determined that this instantiated key could not be used to match the instance because
        // either the controller did not have permissions, or the key pointed to an empty reference
        if (ggv_index == INVALID)
        {
            return USP_ERR_OK;
        }

        ec = &sp->keys.vector[i];
        gge = &sp->ggv.vector[ggv_index];
        type_flags = (unsigned) sp->key_types.vector[ggv_index];

        // Exit if an error occurred when getting the unique key param, or no parameter was provided
        if (gge->err_code != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s", gge->err_msg);
            return gge->err_code;
        }
        USP_ASSERT(gge->value != NULL);     // GROUP_GET_VECTOR_GetValues() should have set an error message if the vendor hook didn't set a value for the parameter

        // Determine the function to call to perform the comparison
        if (type_flags & (DM_INT | DM_UINT | DM_ULONG | DM_LONG | DM_DECIMAL))
        {
            cmp_cb = DM_ACCESS_CompareNumber;
        }
        else if (type_flags & DM_BOOL)
        {
            cmp_cb = DM_ACCESS_CompareBool;
        }
        else if (type_flags & DM_DATETIME)
        {
            cmp_cb = DM_ACCESS_CompareDateTime;
        }
        else
        {
            // Default, and also for DM_STRING, DM_BASE64, DM_HEXBIN
            cmp_cb = DM_ACCESS_CompareString;
        }

        // Exit if an error occurred when comparing the values
        // This could occur if the operator was invalid for the specified type, or type conversion failed
        err = cmp_cb(gge->value, ec->op, ec->value, &result);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if the unique key did not match the value we want
        if (result != true)
        {
            return USP_ERR_OK;
        }
    }

    // If the code gets here, then the instance matches all key expressions in the compound unique key
    *is_match = true;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ResolvePartialPath
**
** Gets a vector of the full path names of all the child parameters of the specified object
** that are instantiated in the data model (ie partial path->param list)
** NOTE: This function does not take 'op' as a parameter (unlike the other resolve functions) because it is only applicable to get operations
**
** \param   path - path of the root object. NOTE: Must not include trailing '.' !
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if successful, or no instances found
**
**************************************************************************/
int ResolvePartialPath(char *path, resolver_state_t *state)
{
    dm_instances_t inst;
    dm_node_t *node;
    bool exists;
    char child_path[MAX_DM_PATH];
    int len;
    int err;
    bool is_qualified_instance;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance);
    if (node == NULL)
    {
        return USP_ERR_INVALID_PATH;
    }

    // Exit if this is not an object
    if (IsObject(node) == false)
    {
        USP_ERR_SetMessage("%s: Partial Path %s is not an object", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
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
        return USP_ERR_OK;
    }

    len = strlen(path);
    USP_STRNCPY(child_path, path, sizeof(child_path));

    if (is_qualified_instance)
    {
        // Object is specified with trailing instance number or is a single instance object
        err = GetChildParams(child_path, len, node, &inst, state, state->depth);
    }
    else
    {
        // Object is specified without trailing instance number
        USP_ASSERT(node->type == kDMNodeType_Object_MultiInstance); // SingleInstance objects should have (is_qualified_instance==true), and hence shouldn't have got here
        err = GetChildParams_MultiInstanceObject(child_path, len, node, &inst, state, state->depth);
    }

    return err;
}

/*********************************************************************//**
**
** GetChildParams
**
** Adds the names of all instantiated child parameters of the specified node to the vector
** This function is called when processing a get using a partial path
** NOTE: This function is recursive
**
** \param   path - path of the object instance to get the child parameters of
** \param   path_len - length of path (position to append child node names)
** \param   node - Node to get children of
** \param   inst - pointer to instance structure locating the parent node
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   depth_remaining - number of hierarchical levels to continue to traverse in the data model
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetChildParams(char *path, int path_len, dm_node_t *node, dm_instances_t *inst, resolver_state_t *state, int depth_remaining)
{
    int err;
    dm_node_t *child;
    unsigned short permission_bitmask;
    bool add_to_vector;

    // Exit if we should abort recursing any further into the data model
    if (depth_remaining <= 0)
    {
        return USP_ERR_OK;
    }

    // Iterate over list of children
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        add_to_vector = false;
        switch(child->type)
        {
            // For single instance child object nodes, recurse to find all child parameters
            case kDMNodeType_Object_SingleInstance:
                {
                    int len;
                    len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                    err = GetChildParams(path, path_len+len, child, inst, state, depth_remaining-1);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            // For multi-instance child objects, ensure that all instances of all of their children are recursed into
            case kDMNodeType_Object_MultiInstance:
                {
                    int len;
                    len = USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                    err = GetChildParams_MultiInstanceObject(path, path_len+len, child, inst, state, depth_remaining-1);
                    if (err != USP_ERR_OK)
                    {
                        return err;
                    }
                }
                break;

            case kDMNodeType_DBParam_ReadOnly:
            case kDMNodeType_DBParam_ReadOnlyAuto:
            case kDMNodeType_DBParam_ReadWriteAuto:
            case kDMNodeType_VendorParam_ReadOnly:
            case kDMNodeType_VendorParam_ReadWrite:
            case kDMNodeType_Param_ConstantValue:
            case kDMNodeType_Param_NumEntries:
            case kDMNodeType_DBParam_ReadWrite:
            case kDMNodeType_DBParam_Secure:
                {
                    // Deal with GetBulkData operations
                    permission_bitmask = DM_PRIV_GetPermissions(child, state->combined_role);
                    if (state->op == kResolveOp_GetBulkData)
                    {
                        USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                        if (permission_bitmask & PERMIT_GET)
                        {
                            add_to_vector = true;
                        }
                        else
                        {
                            // Exit if permissions do not allow a bulk data get of this parameter
                            USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                            USP_ERR_SetMessage("%s: Controller's role permissions do not allow a bulk data read of %s", __FUNCTION__, path);
                            return USP_ERR_PERMISSION_DENIED;
                        }
                    }

                    // If permissions allow it, append the name of this parameter to the parent path and add to the vector
                    // NOTE: If permissions don't allow it, then just forgivingly leave the path out of the vector
                    if ( ((state->op == kResolveOp_Get) && (permission_bitmask & PERMIT_GET)) ||
                         ((state->op == kResolveOp_SubsValChange) && (permission_bitmask & PERMIT_SUBS_VAL_CHANGE)) )
                    {
                        add_to_vector = true;
                    }
                }
                break;

            case kDMNodeType_AsyncOperation:
                permission_bitmask = DM_PRIV_GetPermissions(child, state->combined_role);
                if ((state->op == kResolveOp_SubsOper) && (permission_bitmask & PERMIT_SUBS_EVT_OPER_COMP))
                {
                    add_to_vector = true;
                }
                break;


            case kDMNodeType_Event:
                permission_bitmask = DM_PRIV_GetPermissions(child, state->combined_role);
                if ((state->op == kResolveOp_SubsEvent) && (permission_bitmask & PERMIT_SUBS_EVT_OPER_COMP))
                {
                    add_to_vector = true;
                }
                break;

            case kDMNodeType_SyncOperation:
                // Cannot subscribe to synchronous operations
                break;

            default:
                TERMINATE_BAD_CASE(child->type);
                break;
        }


        // Add this node, if permissions have allowed it and we are returning a vector
        if (add_to_vector)
        {
            if (state->sv != NULL)
            {
                USP_SNPRINTF(&path[path_len], MAX_DM_PATH-path_len, ".%s", child->name);
                STR_VECTOR_Add(state->sv, path);
            }

            if (state->gv != NULL)
            {
                dm_param_info_t *info;
                info = &child->registered.param_info;
                INT_VECTOR_Add(state->gv, info->group_id);
            }
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetChildParams_MultiInstanceObject
**
** Iterates over all instances of the specified object, adding the names of all instantiated child
** parameters of the specified node to the vector
** This function is called when processing a get using a partial path
** NOTE: This function is recursive
**
** \param   path - path of the object instance to get the child parameters of
** \param   path_len - length of path (position to append child node names)
** \param   node - Node to get children of
** \param   inst - pointer to instance structure locating the parent node
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   depth_remaining - number of hierarchical levels to continue to traverse in the data model
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetChildParams_MultiInstanceObject(char *path, int path_len, dm_node_t *node, dm_instances_t *inst, resolver_state_t *state, int depth_remaining)
{
    int_vector_t iv;
    int instance;
    int len;
    int order;
    int i;
    int err;

    // Exit if we should abort recursing any further into the data model
    if (depth_remaining <= 0)
    {
        return USP_ERR_OK;
    }

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

        // Get all child parameters of this object
        inst->instances[order] = instance;
        err = GetChildParams(path, path_len+len, node, inst, state, depth_remaining);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
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
** AddPathFound
**
** Adds the path to the vector of resolved parameters, after checking that
** the path meets the criteria for inclusion for the specified operation being performed by this USP message
**
** \param   path - pointer to path expression identifying objects in the data model
** \param   state - pointer to structure containing state variables to use with this resolution
**
** \return  USP_ERR_OK if path resolution should continue
**
**          NOTE: With forgiving operations such as get and delete, path resolution
**                continues, even if this path is not suitable for inclusion in the result vector
**
**************************************************************************/
int AddPathFound(char *path, resolver_state_t *state)
{
    int index;
    int err;
    bool add_to_vector;
    unsigned path_properties;
    int group_id;

    // Exit if the path did not match the properties we expected of it
    err = CheckPathProperties(path, state, &add_to_vector, &path_properties, &group_id);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if we are gracefully ignoring this path (eg for a get of a parameter which the controller does not have permission for)
    if (add_to_vector==false)
    {
        return USP_ERR_OK;
    }

    // Exit if we are just validating the search path, and don't actually want to add the path to the returned vector
    if (state->sv == NULL)
    {
        return USP_ERR_OK;
    }

    // Handle a Subscription ReferenceList which just references the name of the multi-instance object (unqualified)
    // NOTE: If it references a single specific object instance then the normal code at the end of the function is run instead
    if ( ((state->op == kResolveOp_SubsAdd) || (state->op == kResolveOp_SubsDel)) &&
         ((path_properties & PP_IS_OBJECT_INSTANCE) == 0) )
    {
        USP_ASSERT(path_properties & PP_IS_MULTI_INSTANCE_OBJECT);
        err = DATA_MODEL_GetInstancePaths(path, state->sv, INTERNAL_ROLE);  // NOTE: We can use internal role because we've already checked permissions on this object
                                                                            //       and we don't want it to check get object instance permissions anyway for subscription add/delete paths
        return err;
    }

    // Handle resolving GetInstances
    if (state->op == kResolveOp_Instances)
    {
        if (state->flags & GET_ALL_INSTANCES)
        {
            err = DATA_MODEL_GetAllInstancePaths(path, state->sv, state->combined_role);
        }
        else
        {
            err = DATA_MODEL_GetInstancePaths(path, state->sv, state->combined_role);
        }
        return err;
    }

    // Normal execution path below
    // Exit if the path already exists in the vector
    index = STR_VECTOR_Find(state->sv, path);
    if (index != INVALID)
    {
        return USP_ERR_OK;
    }

    // Finally add the single path to the vector
    STR_VECTOR_Add(state->sv, path);

    // And add the group_id (if required)
    if (state->gv != NULL)
    {
        INT_VECTOR_Add(state->gv, group_id);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CheckPathProperties
**
** Check that the resolved path has the properties which we expect of it
**
** \param   path - pointer to path expression identifying objects in the data model
** \param   state - pointer to structure containing state variables to use with this resolution
** \param   add_to_vector - pointer to variable in which to return if the path should be added to the vector of resolved objects/parameters
** \param   path_properties - pointer to variable in which to return the properties of the resolved object/parameter
** \param   group_id - pointer to variable in which to return the group_id, or NULL if this is not required. NOTE: Only applicable for parameters
**
** \return  USP_ERR_OK if path resolution should continue
**
**          NOTE: With forgiving operations such as get and delete, path resolution
**                continues, even if this path is not suitable for inclusion in the result vector
**
**************************************************************************/
int CheckPathProperties(char *path, resolver_state_t *state, bool *add_to_vector, unsigned *path_properties, int *group_id)
{
    unsigned flags;
    int err;
    unsigned short permission_bitmask;

    // Assume that the path should be added to the vector
    *add_to_vector = false;

    // Exit if the path does not exist in the schema
    flags = DATA_MODEL_GetPathProperties(path, state->combined_role, &permission_bitmask, group_id, NULL);
    *path_properties = flags;
    if ((flags & PP_EXISTS_IN_SCHEMA)==0)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not exist in the schema", __FUNCTION__, path);
        return USP_ERR_INVALID_PATH;
    }

    // ===
    // Check that path represents the type of node we are expecting for this operation
    switch(state->op)
    {
        case kResolveOp_Get:
        case kResolveOp_SubsValChange:
        case kResolveOp_GetBulkData:
            // Exit if the path does not represent a parameter
            if ((flags & PP_IS_PARAMETER)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) is not a parameter", __FUNCTION__, path);
                return USP_ERR_INVALID_PATH;
            }
            break;

        case kResolveOp_Set:
        case kResolveOp_Add:
        case kResolveOp_Del:
        case kResolveOp_Instances:
        case kResolveOp_SubsAdd:
        case kResolveOp_SubsDel:
            // Exit if the path does not represent an object
            if ((flags & PP_IS_OBJECT)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) is not an object", __FUNCTION__, path);
                err = (state->op == kResolveOp_Add) ? USP_ERR_OBJECT_NOT_CREATABLE : USP_ERR_NOT_A_TABLE;
                return err;
            }
            break;

        case kResolveOp_Oper:
        case kResolveOp_SubsOper:
            // Exit if the path does not represent an operation
            if ((flags & PP_IS_OPERATION)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) is not an operation", __FUNCTION__, path);
                err = USP_ERR_COMMAND_FAILURE;
                return err;
            }
            break;

        case kResolveOp_Event:
        case kResolveOp_SubsEvent:
            // Exit if the path does not represent an event
            if ((flags & PP_IS_EVENT)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) is not an event", __FUNCTION__, path);
                return USP_ERR_INVALID_PATH;
                return err;
            }
            break;

        case kResolveOp_Any:
        case kResolveOp_StrictRef:
        case kResolveOp_ForgivingRef:
            // Not applicable, as this operation just validates the expression
            break;

        default:
            TERMINATE_BAD_CASE(state->op);
            break;
    }

    // ===
    // Check that path contains (or does not contain) a fully qualified object
    // Check that the path is to (or is not to) a multi-instance object
    switch(state->op)
    {
        case kResolveOp_Get:
        case kResolveOp_Set:
        case kResolveOp_Oper:
        case kResolveOp_Event:
        case kResolveOp_SubsValChange:
        case kResolveOp_SubsOper:
        case kResolveOp_SubsEvent:
        case kResolveOp_GetBulkData:
            // Not applicable
            break;

        case kResolveOp_Del:
            // Exit if the path is not a fully qualified object instance
            if ((flags & PP_IS_OBJECT_INSTANCE)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) should contain instance number of object", __FUNCTION__, path);
                return USP_ERR_OBJECT_DOES_NOT_EXIST;
            }
            break;

        case kResolveOp_Instances:
            // Whilst they are treated differently, the code allows for a GetInstances on a single instance object,
            // and a GetInstances on a specific, qualified multi instance object - in both recursive and non-recursive cases
            // So nothing to check further here
            break;

        case kResolveOp_SubsAdd:
        case kResolveOp_SubsDel:
            // Exit if the path is not a multi-instance object
            if ((flags & PP_IS_MULTI_INSTANCE_OBJECT)==0)
            {
                USP_ERR_SetMessage("%s: Path (%s) is not a multi-instance object", __FUNCTION__, path);
                return USP_ERR_NOT_A_TABLE;
            }
            break;

        case kResolveOp_Add:
            // Exit if the path is a fully qualified object instance
            if (flags & PP_IS_OBJECT_INSTANCE)
            {
                if (flags & PP_IS_MULTI_INSTANCE_OBJECT)
                {
                    USP_ERR_SetMessage("%s: Path (%s) should not end in an instance number", __FUNCTION__, path);
                    err = USP_ERR_CREATION_FAILURE;
                }
                else
                {
                    USP_ERR_SetMessage("%s: Path (%s) is not a multi-instance object", __FUNCTION__, path);
                    err = USP_ERR_NOT_A_TABLE;
                }
                return err;
            }
            break;

        case kResolveOp_Any:
        case kResolveOp_StrictRef:
        case kResolveOp_ForgivingRef:
            // Not applicable, as this operation just validates the expression
            break;

        default:
            TERMINATE_BAD_CASE(state->op);
            break;
    }



    // ===
    // Exit if the role associated with the USP operation invoking path resolution does not have permission to perform the required operation
    switch(state->op)
    {
        case kResolveOp_Get:
            // It is not an error to not have permissions for a get operation.
            // It is forgiving, so just exit here, without adding the path to the vector
            if ((permission_bitmask & PERMIT_GET)==0)
            {
                return USP_ERR_OK;
            }
            break;

        case kResolveOp_Set:
            // kResolveOp_Set resolves to objects, not parameters
            // So checking for permission to write is performed later in GROUP_SET_VECTOR_Add() when the parameter to set is known
            break;

        case kResolveOp_Add:
            if ((permission_bitmask & PERMIT_ADD)==0)
            {
                USP_ERR_SetMessage("%s: No permission to add to %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_Del:
            if ((permission_bitmask & PERMIT_DEL)==0)
            {
                USP_ERR_SetMessage("%s: No permission to delete %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_Instances:
            // Checking for permission to read instances of this object
            // is performed later by DATA_MODEL_GetAllInstancePaths() or DATA_MODEL_GetInstancePaths()
            break;

        case kResolveOp_Oper:
            if ((permission_bitmask & PERMIT_OPER)==0)
            {
                USP_ERR_SetMessage("%s: No permission to perform operation %s", __FUNCTION__, path);
                return USP_ERR_COMMAND_FAILURE;
            }
            break;

        case kResolveOp_Event:
        case kResolveOp_SubsEvent:
            if ((permission_bitmask & PERMIT_SUBS_EVT_OPER_COMP)==0)
            {
                USP_ERR_SetMessage("%s: No permission to subscribe to event %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_SubsOper:
            if ((permission_bitmask & PERMIT_SUBS_EVT_OPER_COMP)==0)
            {
                USP_ERR_SetMessage("%s: No permission to subscribe to operation %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_SubsAdd:
            if ((permission_bitmask & PERMIT_SUBS_OBJ_ADD)==0)
            {
                USP_ERR_SetMessage("%s: No permission to subscribe to object creation on %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_SubsDel:
            if ((permission_bitmask & PERMIT_SUBS_OBJ_DEL)==0)
            {
                USP_ERR_SetMessage("%s: No permission to subscribe to object deletion on %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_SubsValChange:
            if ((permission_bitmask & PERMIT_SUBS_VAL_CHANGE)==0)
            {
                USP_ERR_SetMessage("%s: No permission to subscribe to value change on %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_GetBulkData:
            if ((permission_bitmask & PERMIT_GET)==0)
            {
                USP_ERR_SetMessage("%s: No permission to get bulk data on %s", __FUNCTION__, path);
                return USP_ERR_PERMISSION_DENIED;
            }
            break;

        case kResolveOp_Any:
        case kResolveOp_StrictRef:
        case kResolveOp_ForgivingRef:
            // Not applicable, as this operation just validates the expression
            break;

        default:
            TERMINATE_BAD_CASE(state->op);
            break;
    }

    // ===
    // Exit if the instance numbers in the path are not instantiated (do not currently exist in the data model)
    switch(state->op)
    {
        case kResolveOp_Get:
        case kResolveOp_Del:
        case kResolveOp_SubsValChange:
        case kResolveOp_SubsAdd:
        case kResolveOp_SubsDel:
        case kResolveOp_SubsOper:
        case kResolveOp_SubsEvent:
        case kResolveOp_GetBulkData:
            // It is not an error for instance numbers to not be instantiated for a get parameter value
            // or a delete or a subscription reference list
            // Both are forgiving, so just exit here, without adding the path to the vector
            if ((flags & PP_INSTANCE_NUMBERS_EXIST)==0)
            {
                return USP_ERR_OK;
            }
            break;

        case kResolveOp_Set:
        case kResolveOp_Add:
        case kResolveOp_Instances:
        case kResolveOp_Oper:
        case kResolveOp_Event:
            // Instance numbers must be instantiated (exist in data model)
            if ((flags & PP_INSTANCE_NUMBERS_EXIST)==0)
            {
                USP_ERR_SetMessage("%s: Object exists in schema, but instances are invalid: %s", __FUNCTION__, path);
                return USP_ERR_OBJECT_DOES_NOT_EXIST;
            }
            break;

        case kResolveOp_Any:
        case kResolveOp_StrictRef:
        case kResolveOp_ForgivingRef:
            // Not applicable, as this operation just validates the expression
            break;

        default:
            TERMINATE_BAD_CASE(state->op);
            break;
    }

    // If the code gets here, then the path should be added to the vector of resolved objects/parameters
    *add_to_vector = true;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CountPathSeparator
**
** Counts the number of path separators ('.') in the specified data model path
**
** \param   path - pointer to string containing data model path
**
** \return  Number of separators in the specified path
**
**************************************************************************/
int CountPathSeparator(char *path)
{
    char *p = path;
    int count = 0;

    // Iterate over all characters in the path, counting the number of '.' characters in the string
    while (*p != '\0')
    {
        if (*p == '.')
        {
            count++;
        }
        p++;
    }

    return count;
}

/*********************************************************************//**
**
** InitSearchParam
**
** Initialize search parameter set
**
** \param   sp - pointer to search parameter sets
**
** \return  None
**
**************************************************************************/
void InitSearchParam(search_param_t *sp)
{
    EXPR_VECTOR_Init(&sp->keys);
    INT_VECTOR_Init(&sp->ggv_indexes);
    GROUP_GET_VECTOR_Init(&sp->ggv);
    INT_VECTOR_Init(&sp->key_types);
}

/*********************************************************************//**
**
** DestroySearchParam
**
** Destroy search parameter set
**
** \param   sp - pointer to search parameter sets
**
** \return  None
**
**************************************************************************/
void DestroySearchParam(search_param_t *sp)
{
    EXPR_VECTOR_Destroy(&sp->keys);
    INT_VECTOR_Destroy(&sp->ggv_indexes);
    GROUP_GET_VECTOR_Destroy(&sp->ggv);
    INT_VECTOR_Destroy(&sp->key_types);
}
