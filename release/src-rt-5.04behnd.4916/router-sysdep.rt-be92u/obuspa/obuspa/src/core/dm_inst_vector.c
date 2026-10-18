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
 * \file dm_inst_vector.c
 *
 * Implements a data structure containing a list of dm_inst structures
 * This is basically a list of all object instances instantiated in the data model
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "int_vector.h"
#include "dm_inst_vector.h"


//--------------------------------------------------------------------
// Top-level multi-instance node being refreshed
// This is used to ensure that within the refresh instances callback,
// the caller of USP_RefreshInstance() is only refreshing the instances that it has ben asked to refresh
// Outside of the refresh instances callback, this variable is set to NULL
static dm_node_t *refresh_instances_top_node = NULL;

//--------------------------------------------------------------------
// Vector, used to hold the new set of instances for the refresh_instances_top_node, within the refresh instances callback
static dm_instances_vector_t refreshed_instances_vector = { 0 };

//--------------------------------------------------------------------
// Counter that is incremented for each USP request (or item of internal work, such as BulkDataCollection or Value Change subscriptions)
// Its purpose is to ensure that during the processing of a USP request, the cached set of instances for a table do not expire
// (which may cause an instance number mismatch whilst performing the USP request, resulting in an internal error)
static unsigned int cur_lock_period = 1;      // Starts at 1, which is more than the starting value for lock_period in each object (0). This ensures that at startup the instance refresh is called in DM_INST_VECTOR_RefreshBaselineInstances().

//--------------------------------------------------------------------
// Flag, if set to true, the refresh instances vendor hook will be called, even if the instances cache hasn't expired
// This is needed to ensure that when periodically resolving object deletion notifications, that notifications aren't missed
// due to the subscription deletion watch objects being formed ahead (as empty - because of instance cache mismatches) of the
// instance cache expiring
static bool refresh_override = false;

//--------------------------------------------------------------------
// Boolean set after baseline instances have been retrieved. After being set, instance addition/deletion can cause notifications
static bool notify_subscriptions_allowed = false;

//--------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void AddObjectInstanceIfPermitted(dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role);
int RefreshInstVector(dm_node_t *node);
int RefreshInstVectorEntry(char *path);
bool IsExistInInstVector(dm_instances_t *match, dm_instances_vector_t *div);
void AddToInstVector(dm_instances_t *inst, dm_instances_vector_t *div);
void RefreshBaselineInstances(dm_node_t *parent);

/*********************************************************************//**
**
** DM_INST_VECTOR_Init
**
** Initialises a dm_inst vector
**
** \param   div - pointer to dm_instances vector structure to initialize
**
** \return  None
**
**************************************************************************/
void DM_INST_VECTOR_Init(dm_instances_vector_t *div)
{
    div->vector = NULL;
    div->num_entries = 0;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_Destroy
**
** Frees up all memory used by the specified dm_instances_vector structure
**
** \param   div - pointer to dm_instances vector structure
**
** \return  None
**
**************************************************************************/
void DM_INST_VECTOR_Destroy(dm_instances_vector_t *div)
{
    if (div->vector != NULL)
    {
        USP_FREE(div->vector);
    }

    div->vector = NULL;
    div->num_entries = 0;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_Add
**
** Adds the specified instance numbers and associated nodes to the dm_instances_vector vector
** NOTE: The instance is not added again, if it already exists
**
** \param   inst - pointer to instance structure to add to the dm_instances_vector vector
**                 contained within this structure is the top level multi-instance node which holds the dm_instances_vector
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_Add(dm_instances_t *inst)
{
    int i;
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    // Exit if there are no object instances to add
    // This is the case if this function is called for a parameter which does not have any object instances in it's path
    if (inst->order == 0)
    {
        return USP_ERR_OK;
    }

    // Determine which top level multi-instance node's DM instances array to add to
    USP_ASSERT(inst->order > 0);
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // See if this instance already exists
    for (i=0; i < div->num_entries; i++)
    {
        // If this instance of the object already exists then exit, nothing more to do
        oi = &div->vector[i];
        if (memcmp(oi, inst, sizeof(dm_instances_t)) == 0)
        {
            return USP_ERR_OK;
        }
    }

    // Add the instance to the correct top-level node instance vector
    AddToInstVector(inst, div);

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** DM_INST_VECTOR_Remove
**
** Deletes the specified instance numbers and associated nodes from the dm_instances_vector vector
** NOTE: This function deletes the instance number tree starting at the specified instance
** NOTE: The instance is not removed again, if it has already been removed
**
** \param   inst - pointer to instance structure to delete from the dm_instances_vector vector
**                 contained within this structure is the top level multi-instance node which holds the dm_instances_vector
**
** \return  None
**
**************************************************************************/
void DM_INST_VECTOR_Remove(dm_instances_t *inst)
{
    int i;
    int j;
    int order;
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    // Exit if there is no instance to remove
    if (inst->order == 0)
    {
        return;
    }

    // Determine which top level multi-instance node's DM instances array to remove from
    USP_ASSERT(inst->order > 0);
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Find this instance and all child nested instances and delete them
    j = 0;
    order = inst->order;
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order >= order) &&
            (memcmp(oi->nodes, inst->nodes, order*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            // Delete this node. Nothing to do in this iteration of the loop - this value will be overwritten by further
        }
        else
        {
            // Copy down later entries in the array, over ones which have been removed
            if (j < i)
            {
                memcpy(&div->vector[j], oi, sizeof(dm_instances_t));
            }

            j++;
        }
    }

    // NOTE: Don't bother reallocating the memory for the array (it could now be smaller).
    // It will be resized next time an instance is added.
    div->num_entries = j;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_IsExist
**
** Determines whether the specified object instance exists in the data model
**
** \param   match - pointer to instances structure describing the instances to match against
**                 contained within this structure is the top level multi-instance node which holds the dm_instances_vector
** \param   exists - pointer to boolean in which to return whether the object exists or not
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_IsExist(dm_instances_t *match, bool *exists)
{
    dm_node_t *top_node;
    dm_instances_vector_t *div;
    int err;

    // Exit if the object is a single instance object or an unqualified multi-instance object - these always exist
    if (match->order == 0)
    {
        *exists = true;
        return USP_ERR_OK;
    }

    // Determine which top level multi-instance node's DM instances array to search in
    USP_ASSERT(match->order > 0);
    top_node = match->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Exit if unable to refresh the cache of instances for this object (if necessary)
    err = RefreshInstVector(top_node);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    *exists = IsExistInInstVector(match, div);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_GetNextInstance
**
** Gets the next numbered instance for the specified object (given it's parent instance numbers)
** This function is called when allocating a new instance number for an object that is being added
**
** \param   node - pointer to object in data model
** \param   inst - pointer to instance structure specifying the object's parents and their instance numbers
**                 contained within this structure is the top level multi-instance node which holds the dm_instances_vector
** \param   next_instance - pointer to variable in which to return the next instance number
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_GetNextInstance(dm_node_t *node, dm_instances_t *inst, int *next_instance)
{
    int i;
    int order;
    int instance;
    int highest_instance=0;       // highest instance number encountered so far
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    order = inst->order;            // NOTE: This may be 0 for a top level multi-instance node
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);
    inst->nodes[order] = node;

    // Determine which top level multi-instance node's DM instances array to iterate over
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Iterate over the table of instance numbers, determining the highest instance number for the specified object
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order == order+1) &&
            (memcmp(oi->nodes, inst->nodes, (order+1)*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            instance = oi->instances[order];
            if (instance > highest_instance)
            {
                highest_instance = instance;
            }
        }
    }

    *next_instance = highest_instance+1;
    inst->nodes[order] = NULL;          // Undo the changes made by this function to the inst array

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_GetNumInstances
**
** Gets the number of instances of the specified object (given it's parent instance numbers)
**
** \param   node - pointer to object in data model
** \param   inst - pointer to instance structure specifying the object's parents and their instance numbers
** \param   num_instances - pointer to variable in which to return the number of instances of the specified object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_GetNumInstances(dm_node_t *node, dm_instances_t *inst, int *num_instances)
{
    int i;
    int order;
    int count;
    int err;
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    order = inst->order;           // NOTE: This may be 0 for a top level multi-instance node
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);
    inst->nodes[order] = node;

    // Determine which top level multi-instance node's DM instances array to iterate over
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Exit if unable to refresh the cache of instances for this object (if necessary)
    err = RefreshInstVector(top_node);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Iterate over the table of instance numbers, counting the number of instances which match the object and its parent instance numbers
    count = 0;
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order == order+1) &&
            (memcmp(oi->nodes, inst->nodes, (order+1)*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            count++;
        }
    }

    inst->nodes[order] = NULL;          // Undo the changes made by this function to the inst array

    *num_instances = count;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_GetInstances
**
** Gets a vector of the instance numbers for the specified object (given it's parent instance numbers)
**
** \param   node - pointer to object in data model
** \param   inst - pointer to instance structure specifying the object's parents and their instance numbers
** \param   iv - pointer to structure which will be populated with instance numbers by this function
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_GetInstances(dm_node_t *node, dm_instances_t *inst, int_vector_t *iv)
{
    int i;
    int order;
    int instance;
    dm_instances_t *oi;
    int index;
    int err;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    order = inst->order;          // NOTE: This may be 0 for a top level multi-instance node
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);
    inst->nodes[order] = node;
    INT_VECTOR_Init(iv);

    // Determine which top level multi-instance node's DM instances array to iterate over
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Exit if unable to refresh the cache of instances for this object (if necessary)
    err = RefreshInstVector(top_node);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over the instances array, finding the objects which match, and their instances
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order >= order+1) &&
            (memcmp(oi->nodes, inst->nodes, (order+1)*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            instance = oi->instances[order];

            // Add the instance to the array (if it has not been added already)
            index = INT_VECTOR_Find(iv, instance);
            if (index == INVALID)
            {
                INT_VECTOR_Add(iv, instance);
            }
        }
    }

    err = USP_ERR_OK;

exit:
    inst->nodes[order] = NULL;          // Undo the changes made by this function to the inst array
    return err;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_RefreshBaselineInstances
**
** Called at startup to determine all refreshed object instances, so that
** object creation and deletion after bootup can generate notification events if necessary
**
** \param   parent - node to get instances of (if it is a top level multi-instance object)
**
** \return  None - If instances could not be refreshed, then we will try again at the time of generating a USP message that needs them
**
**************************************************************************/
void DM_INST_VECTOR_RefreshBaselineInstances(dm_node_t *parent)
{
    RefreshBaselineInstances(parent);

    // Allow notifications to be sent, after this point
    notify_subscriptions_allowed = true;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_DumpTable
**
** Prints out the Object Instances array for the specified top level multi-instance object
**
** \param   path - data model path to top evel object
**
** \return  None
**
**************************************************************************/
void DM_INST_VECTOR_DumpTable(char *path)
{
    dm_node_t *node;

    // Exit if path was invalid
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    if ((node == NULL) || (node->type != kDMNodeType_Object_MultiInstance))
    {
        USP_LOG_Warning("%s: Unable to dump instance cache for %p (doesn't exist, or not a multi-instance object)", __FUNCTION__, path);
        return;
    }

    // Exit if no instances in the tabe
    if (node->registered.object_info.inst_vector.num_entries == 0)
    {
        USP_LOG_Info("Dumping instance cache for %s - No instances", path);
        return;
    }

    USP_LOG_Info("Dumping instance cache for %s", path);
    DM_INST_VECTOR_Dump(&node->registered.object_info.inst_vector);
    USP_LOG_Info("End");
}

/*********************************************************************//**
**
** DM_INST_VECTOR_Dump
**
** Prints out the Object Instances array
**
** \param   div - pointer to dm_instances vector structure
**
** \return  None
**
**************************************************************************/
void DM_INST_VECTOR_Dump(dm_instances_vector_t *div)
{
    int i;
    dm_instances_t *inst;
    dm_node_t *node;
    char path[MAX_DM_PATH];

    for (i=0; i < div->num_entries; i++)
    {
        inst = &div->vector[i];
        USP_ASSERT(inst->order >= 1);
        node = inst->nodes[inst->order - 1];
        DM_PRIV_FormInstantiatedPath(node->path, inst, path, sizeof(path));

        USP_DUMP("%s", path);
    }
}

/*********************************************************************//**
**
** DM_INST_VECTOR_GetAllInstancePaths_Unqualified
**
** Returns a string vector containing the paths of all instances to the specified
** unqualified multi-instance object and recursively all child instances
** This function expects the dm_instances_t structure to contain only the node's parents and parent instances
**
** \param   node - pointer to multi-instance object in data model that we want to get the instances of
** \param   inst - pointer to instance structure specifying the object's parents and their instance numbers
** \param   sv - pointer to structure which will be populated with paths to the instances of the object by this function
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_GetAllInstancePaths_Unqualified(dm_node_t *node, dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role)
{
    int i;
    int order;
    int err;
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    order = inst->order;          // NOTE: This may be 0 for a top level multi-instance node
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);
    inst->nodes[order] = node;

    // Determine which top level multi-instance node's DM instances array to iterate over
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Exit if unable to refresh the cache of instances for this object (if necessary)
    err = RefreshInstVector(top_node);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over the instances array, finding all objects which match, and their instances
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order >= order+1) &&
            (memcmp(oi->nodes, inst->nodes, (order+1)*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            AddObjectInstanceIfPermitted(oi, sv, combined_role);
        }
    }

    err = USP_ERR_OK;

exit:
    // Undo the changes made by this function to the inst array
    inst->nodes[order] = NULL;
    return err;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_GetAllInstancePaths_Qualified
**
** Returns a string vector containing the paths of all instances to the specified
** qualified multi-instance object and recursively all child instances
** This function expects the dm_instances_t structure to contain the object instances to match
**
** \param   inst - pointer to instance structure specifying the object and instance numbers to match
** \param   sv - pointer to structure which will be populated with paths to the instances of the object by this function
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_GetAllInstancePaths_Qualified(dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role)
{
    int i;
    int order;
    int err;
    dm_instances_t *oi;
    dm_node_t *top_node;
    dm_instances_vector_t *div;

    order = inst->order;
    USP_ASSERT(order > 0);
    USP_ASSERT(order < MAX_DM_INSTANCE_ORDER);

    // Determine which top level multi-instance node's DM instances array to iterate over
    top_node = inst->nodes[0];
    USP_ASSERT(top_node != NULL);
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    div = &top_node->registered.object_info.inst_vector;

    // Exit if unable to refresh the cache of instances for this object (if necessary)
    err = RefreshInstVector(top_node);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Iterate over the instances array, finding all objects which match, and their instances
    for (i=0; i < div->num_entries; i++)
    {
        oi = &div->vector[i];
        if ((oi->order >= order) &&
            (memcmp(oi->nodes, inst->nodes, order*sizeof(dm_node_t *)) == 0) &&
            (memcmp(oi->instances, inst->instances, order*sizeof(int)) == 0))
        {
            AddObjectInstanceIfPermitted(oi, sv, combined_role);
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_NextLockPeriod
**
** Signals that the current USP request has finished.
** This increments the count of USP requests, so that any cached instances which were previously prevented from
** expiring may now expire if they have reached their expiry time.
** Effectively this unlocks any previously locked cached instances in the inst vector
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DM_INST_VECTOR_NextLockPeriod(void)
{
    cur_lock_period++;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_SetRefreshOverride
**
** Sets a flag which enables or disables forcing the refresh instances vendor hook to be called,
** even if the instances cache haas not expired yet
**
** \param   force_override - set to true if the instances refresh vendor hook should be forced to be called
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DM_INST_VECTOR_SetRefreshOverride(bool force_override)
{
    refresh_override = force_override;
}

/*********************************************************************//**
**
** DM_INST_VECTOR_RefreshInstance
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
int DM_INST_VECTOR_RefreshInstance(char *path)
{
    dm_node_t *node;
    dm_node_t *top_node;
    dm_instances_t inst;
    bool is_qualified_instance;
    bool exists;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not exist in the schema", __FUNCTION__, path);
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

    // Exit if the top-level multi-instance object does not match the one we requested instances for
    // NOTE: We just emit a warning in this case, rather than returning an error
    // This allows clients to potentially be 'dumb' in refreshing instances - we filter the instances to add here
    top_node = inst.nodes[0];
    if (top_node != refresh_instances_top_node)
    {
        USP_LOG_Warning("%s: Ignoring USP_RefreshInstance(%s) as it was not for path %s (or its descendants)", __FUNCTION__, path, top_node->path);
        return USP_ERR_OK;
    }

    // Exit if instance already exists - nothing to do
    exists = IsExistInInstVector(&inst, &refreshed_instances_vector);
    if (exists)
    {
        return USP_ERR_OK;
    }

    // Exit if the parent object instances in the path do not exist
    if (inst.order > 1)
    {
        inst.order--;           // Temporarily remove the instance number of the object that was added,
                                // so that structure indicates only parent instance numbers
        exists = IsExistInInstVector(&inst, &refreshed_instances_vector);
        if (exists == false)
        {
            USP_ERR_SetMessage("%s: Parent objects in path (%s) do not exist", __FUNCTION__, path);
            return USP_ERR_OBJECT_DOES_NOT_EXIST;
        }
        inst.order++;           // Restore the structure, so that it indicates the instance number of the object that was added
    }

    // Add this to the refreshed instances vector
    AddToInstVector(&inst, &refreshed_instances_vector);

    return USP_ERR_OK;
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** DM_INST_VECTOR_SeedInstance
**
** Called to seed the specified object instance into the instance vector immediately after the USP Service has registered
** This is necessary to prevent any object creation/deletion notifications that use the refresh instances vendor hook from
** firing immediately after the USP Service has registered
** NOTE: This function does not create object lifecycle events, so will not trigger an object created/deleted notification
**
** \param   path - data model path of the multi-instance object to add
** \param   expiry_time - absolute time at which all of the instances for the top level object (specified by this path) expire
** \param   expected_group_id - group_id that the path must be owned by
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_SeedInstance(char *path, time_t expiry_time, int expected_group_id)
{
    dm_node_t *node;
    dm_node_t *top_node;
    dm_instances_t inst;
    bool is_qualified_instance;
    bool exists;
    dm_instances_vector_t *vector_to_refresh;

    // Exit if unable to find node representing this object
    node = DM_PRIV_GetNodeFromPath(path, &inst, &is_qualified_instance, 0);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: Path (%s) does not exist in the schema", __FUNCTION__, path);
        return USP_ERR_OBJECT_DOES_NOT_EXIST;
    }

    // Exit if the object was not a multi-instance object
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

    // Exit if this path is not owned by the expected USP Service
    if (node->group_id != expected_group_id)
    {
        USP_ERR_SetMessage("%s: Path (%s) is not owned by USP Service", __FUNCTION__, path);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Update the expiry time for this instance vector
    top_node = inst.nodes[0];
    USP_ASSERT(top_node->type == kDMNodeType_Object_MultiInstance);
    vector_to_refresh = &top_node->registered.object_info.inst_vector;
    top_node->registered.object_info.refresh_instances_expiry_time = expiry_time;

    // Exit if instance already exists - nothing to do
    exists = IsExistInInstVector(&inst, vector_to_refresh);
    if (exists)
    {
        return USP_ERR_OK;
    }

    // Add this to the relevant instances vector
    AddToInstVector(&inst, vector_to_refresh);

    return USP_ERR_OK;
}
#endif

/*********************************************************************//**
**
** DM_INST_VECTOR_RefreshTopLevelObjectInstances
**
** Refreshes the instances for the specified top level object and all children
** NOTE: This function may be called recursively, since it is called from the path resolver and it may call
**       the path resolver itself in order to resolve object lifetime event subscriptions
**
** \param   node - pointer to node of top level object to refresh
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_INST_VECTOR_RefreshTopLevelObjectInstances(dm_node_t *node)
{
    return RefreshInstVector(node);
}

/*********************************************************************//**
**
** AddObjectInstanceIfPermitted
**
** Adds the specified object instance, to the string vector, if the role permits its instance numbers to be read
**
** \param   inst - pointer to instance structure specifying the object and its instance numbers
** \param   sv - pointer to structure which will be populated with paths to the instances of the object by this function
**               NOTE: The caller must initialise this structure. This function adds to this structure, it does not initialise it.
** \param   combined_role - role to use to check that object instances may be returned.  If set to INTERNAL_ROLE, then full permissions are always returned
**
** \return  None
**
**************************************************************************/
void AddObjectInstanceIfPermitted(dm_instances_t *inst, str_vector_t *sv, combined_role_t *combined_role)
{
    dm_node_t *node;
    unsigned short permission_bitmask;
    char path[MAX_DM_PATH];
    int err;

    // Exit if the current role does not have permission to return this object instance in the string vector
    node = inst->nodes[inst->order-1];
    permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
    if ((permission_bitmask & PERMIT_GET_INST)==0)
    {
        return;
    }

    // Convert the dm_instances_t structure into a path
    err = DM_PRIV_FormInstantiatedPath(node->path, inst, path, sizeof(path));
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Add the path to the string vector
    STR_VECTOR_Add(sv, path);
}

/*********************************************************************//**
**
** RefreshBaselineInstances
**
** Called at startup to determine all refreshed object instances, so that
** object creation and deletion after bootup can generate notification events if necessary
** NOTE: This function is called recursively over the whole data model
**
** \param   parent - node to get instances of (if it is a top level multi-instance object)
**
** \return  None - If instances could not be refreshed, then we will try again at the time of generating a USP message that needs them
**
**************************************************************************/
void RefreshBaselineInstances(dm_node_t *parent)
{
    dm_node_t *child;

    // Stop recursing at all top level multi-instance object nodes of the data model tree
    if (parent->type == kDMNodeType_Object_MultiInstance)
    {
        dm_object_info_t *info;                   // Objects

        // Refresh the instances of this object (and all of its children) if required
        info = &parent->registered.object_info;
        if (info->refresh_instances_cb != NULL)
        {
            (void) RefreshInstVector(parent);
        }

        USP_ASSERT(parent->order == 1);
        return;
    }

    // Iterate over list of children
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        // Recurse through all objects to find first top level multi-instance object from here
        if (IsObject(child))
        {
            (void) RefreshBaselineInstances(child);
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** RefreshInstVector
**
** Called before querying the instances vector, to ensure that it is up to date
** If a refresh_instances_cb is registered, then the instances vector acts as a cache, with ageing of its content
** In this case, this function may call the refresh_instances_cb to obtain the instance numbers if the current ones in the cache have expired
** NOTE: This function creates object lifecycle events, so will trigger object created/deleted notifications
**
** \param   top_node - pointer to top-level multi-instance object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int RefreshInstVector(dm_node_t *top_node)
{
    dm_instances_vector_t *old_instances;
    dm_instances_vector_t *new_instances;
    dm_instances_t *inst;
    dm_node_t *node;
    char path[MAX_DM_PATH];
    dm_object_info_t *info;
    time_t cur_time;
    int expiry_period;
    int err;
    int len;
    int i;

    // Exit if this function is being called re-entrantly
    // This may be the case, as the refresh instances vendor callback ends up calling DM_INST_VECTOR_RefreshInstance()
    if (refresh_instances_top_node != NULL)
    {
        return USP_ERR_OK;
    }

    // Exit if this top-level multi-instance node is not maintained by the get instances callback
    // NOTE: Objects which don't have a refresh instances callback use USP_DM_InformInstance(), USP_SIGNAL_ObjectAdded() and USP_SIGNAL_ObjectDeleted() to maintain the instance vector
    info = &top_node->registered.object_info;
    if (info->refresh_instances_cb == NULL)
    {
        return USP_ERR_OK;
    }

    // Exit if the instances have already been locked for this USP request
    // This prevents instance numbers from changing mid-processing of an USP request and causing an error (if the instance numbers change)
    if (info->lock_period == cur_lock_period)
    {
        return USP_ERR_OK;
    }

    // Exit if it's not yet time to refresh the instance vector
    cur_time = time(NULL);
    if ((cur_time <= info->refresh_instances_expiry_time) && (refresh_override==false))
    {
        // Since we've determined that the instances have not expired, lock the cached instances for the rest of this USP request
        info->lock_period = cur_lock_period;
        return USP_ERR_OK;
    }

    // If the code gets here, then the instances must be refreshed

    // Truncate the schema path to form a partial path to the top-level multi-instance object
    #define INSTANCE_SEPARATOR "{i}"
    len = strlen(top_node->path) - (sizeof(INSTANCE_SEPARATOR)-1);
    USP_ASSERT(strcmp(&top_node->path[len], INSTANCE_SEPARATOR)==0);
    memcpy(path, top_node->path, len);
    path[len] = '\0';


    // Exit if unable to get the refreshed instances into the refreshed_instances_vector
    refresh_instances_top_node = top_node;      // Indicate to DM_INST_VECTOR_RefreshInstance() the top level node which is meant to be being refreshed
    DM_INST_VECTOR_Init(&refreshed_instances_vector);
    err = info->refresh_instances_cb(top_node->group_id, path, &expiry_period);
    if (err != USP_ERR_OK)
    {
        USP_ERR_ReplaceEmptyMessage("%s: Refresh Instances callback for %s failed", __FUNCTION__, top_node->path);
        DM_INST_VECTOR_Destroy(&refreshed_instances_vector);
        refresh_instances_top_node = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Update the expiry time and lock the instances for at least this USP request
    cur_time = time(NULL);
    info->refresh_instances_expiry_time = cur_time + expiry_period;
    info->lock_period = cur_lock_period;

    // Skip determining added/deleted instances, if we don't need to notify subscriptions because
    // we haven't got the baseline set of object instances at bootup yet
    if (notify_subscriptions_allowed == false)
    {
        goto exit;
    }

    // Determine all instances which have been added, by finding all instances in the new, which are not in the old instance vector
    old_instances = &info->inst_vector;
    new_instances = &refreshed_instances_vector;
    for (i=0; i < new_instances->num_entries; i++)
    {
        inst = &new_instances->vector[i];
        if (IsExistInInstVector(inst, old_instances)==false)
        {
            node = inst->nodes[inst->order-1];
            err = DM_PRIV_FormInstantiatedPath(node->path, inst, path, sizeof(path));
            if (err == USP_ERR_OK)
            {
                DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(path, kSubNotifyType_ObjectCreation);
            }
        }
    }

    // Determine all instances which have been deleted, by finding all instances in the old, which are not in the new instance vector
    for (i=0; i < old_instances->num_entries; i++)
    {
        inst = &old_instances->vector[i];
        if (IsExistInInstVector(inst, new_instances)==false)
        {
            node = inst->nodes[inst->order-1];
            err = DM_PRIV_FormInstantiatedPath(node->path, inst, path, sizeof(path));
            if (err == USP_ERR_OK)
            {
                DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(path, kSubNotifyType_ObjectDeletion);
            }
        }
    }

exit:
    // Replace the old instances with the new instances, deleting the old instances first
    DM_INST_VECTOR_Destroy(&info->inst_vector);
    memcpy(&info->inst_vector, &refreshed_instances_vector, sizeof(dm_instances_vector_t));
    refresh_instances_top_node = NULL;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** IsExistInInstVector
**
** Determines whether the specified object instance exists in the specified instances vector
**
** \param   match - pointer to instances structure describing the instances to match against
** \param   div - pointer to dm_instances vector structure to search in
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
bool IsExistInInstVector(dm_instances_t *match, dm_instances_vector_t *div)
{
    int i;
    dm_instances_t *inst;

    // Iterate over the array of object instances
    for (i=0; i < div->num_entries; i++)
    {
        inst = &div->vector[i];
        if (inst->order >= match->order)
        {
            if ( (memcmp(inst->nodes, match->nodes, (match->order)*sizeof(dm_node_t *)) == 0) &&
                 (memcmp(inst->instances, match->instances, (match->order)*sizeof(int)) == 0) )
            {
                // All specified object instances match
                return true;
            }
        }
    }

    // If the code gets here, then no instances matched
    return false;
}

/*********************************************************************//**
**
** AddToInstVector
**
** Adds the specified object instance into the specified instances vector
**
** \param   inst - pointer to instances structure describing the object instance
** \param   div - pointer to dm_instances vector structure to add to
**
** \return  None
**
**************************************************************************/
void AddToInstVector(dm_instances_t *inst, dm_instances_vector_t *div)
{
    int size;

    // Increase the size of the dm_instances_vector array
    size = (div->num_entries+1) * sizeof(dm_instances_t);
    div->vector = USP_REALLOC(div->vector, size);

    // And store this object instance
    memcpy(&div->vector[div->num_entries], inst, sizeof(dm_instances_t));
    div->num_entries++;
}

