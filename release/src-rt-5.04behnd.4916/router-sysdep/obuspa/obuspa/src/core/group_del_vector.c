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
 * \file group_del_vector.c
 *
 * Performs deletion of a set of objects across different data model provider components
 *
 */

#include <string.h>

#include "common_defs.h"
#include "group_del_vector.h"
#include "data_model.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_Init
**
** Initialises a group delete vector structure
**
** \param   gdv - Pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void GROUP_DEL_VECTOR_Init(group_del_vector_t *gdv)
{
    gdv->vector = NULL;
    gdv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_Destroy
**
** Frees all memory used by the group del vector
**
** \param   gdv - pointer to vector to destroy
**
** \return  None
**
**************************************************************************/
void GROUP_DEL_VECTOR_Destroy(group_del_vector_t *gdv)
{
    int i;
    group_del_entry_t *gde;

    for (i=0; i < gdv->num_entries; i++)
    {
        gde = &gdv->vector[i];
        USP_SAFE_FREE(gde->path);
        USP_SAFE_FREE(gde->err_msg);
    }

    USP_SAFE_FREE(gdv->vector);
    gdv->num_entries = 0;
}

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_AddObjectsToDelete
**
** Adds a set of objects to delete to the vector
**
** \param   gdv - pointer to vector to add to
** \param   obj_paths - string vector of resolved objects to delete
** \param   group_ids - int vector containing the group_ids of the resolved objects to delete
**
** \return  None
**
**************************************************************************/
void GROUP_DEL_VECTOR_AddObjectsToDelete(group_del_vector_t *gdv, str_vector_t *obj_paths, int_vector_t *group_ids)
{
    int i;
    int new_num_entries;
    int num_to_add;
    group_del_entry_t *gde;

    // Exit if no objects to add
    USP_ASSERT(obj_paths->num_entries == group_ids->num_entries);
    if (obj_paths->num_entries == 0)
    {
        return;
    }

    // Increase the vector size
    num_to_add = obj_paths->num_entries;
    new_num_entries = gdv->num_entries + num_to_add;
    gdv->vector = USP_REALLOC(gdv->vector, new_num_entries*sizeof(group_del_entry_t));
    gde = &gdv->vector[ gdv->num_entries ];
    gdv->num_entries = new_num_entries;

    // Fill in the entries from the input vectors
    for (i=0; i<num_to_add; i++)
    {
        // Move the resolved path string from obj_paths vector to group del vector
        gde->path = obj_paths->vector[i];
        obj_paths->vector[i] = NULL;                // This ensures that the string is not freed when obj_paths is deleted
        gde->group_id = group_ids->vector[i];
        gde->err_code = USP_ERR_OK;
        gde->err_msg = NULL;
        gde++;
    }
}

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_AddObjectNotDeleted
**
** Adds an entry to the group del vector containing a path which failed to resolve
**
** \param   gdv - pointer to vector to add to
** \param   obj_path - path expression which failed to resolve
** \param   err_code - cause of failure to delete the requested objects
** \param   err_msg - textual cause of failure to delete the requested objects
**
** \return  None
**
**************************************************************************/
void GROUP_DEL_VECTOR_AddObjectNotDeleted(group_del_vector_t *gdv, char *obj_path, int err_code, char *err_msg)
{
    int new_num_entries;
    group_del_entry_t *gde;

    // Increase the vector size
    new_num_entries = gdv->num_entries + 1;
    gdv->vector = USP_REALLOC(gdv->vector, new_num_entries*sizeof(group_del_entry_t));
    gde = &gdv->vector[ gdv->num_entries ];
    gdv->num_entries = new_num_entries;

    // Fill in the entry
    gde->path = USP_STRDUP(obj_path);
    gde->group_id = NON_GROUPED;
    gde->err_code = err_code;
    gde->err_msg = USP_STRDUP(err_msg);
}

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_AreAllPathsTheSameGroupId
**
** Determines whether all resolved paths in the group del vector are targetted at the same data model provider component
** This test is necessary for allow_partial=false, because if any object fails to delete
** it is not possible to re-create objects that have previously been deleted successfully in other data model providers
**
** \param   gdv - pointer to vector containing the objects that were deleted successfully or unsuccessfully
** \param   single_group_id - pointer to variable in which to return the group_id identifying the data model provider component for all resolved paths
**                            NOTE: This variable may be returned as NON_GROUPED, if all of the paths were owned by the intenal data model
**
** \return  None
**
**************************************************************************/
bool GROUP_DEL_VECTOR_AreAllPathsTheSameGroupId(group_del_vector_t *gdv, int *single_group_id)
{
    int i;
    int group_id;
    int first_group_id = NON_GROUPED;
    group_del_entry_t *gde;

    for (i=0; i < gdv->num_entries; i++)
    {
        gde = &gdv->vector[i];
        group_id = gde->group_id;
        if (group_id != NON_GROUPED)
        {
            if (first_group_id == NON_GROUPED)
            {
                first_group_id = group_id;
            }
            else
            {
                if (group_id != first_group_id)
                {
                    return false;
                }
            }
        }
    }

    *single_group_id = first_group_id;
    return true;
}

/*********************************************************************//**
**
** GROUP_DEL_VECTOR_FindFirstFailureIfAllFailed
**
** Determines whether all of the objects in the specified slice of the group del vector failed to delete
** and if so, returns the first object which failed to delete
**
**
** \param   gdv - pointer to vector containing the objects that were deleted successfully or unsuccessfully
** \param   index - start index of objects in the slice of the group del vector to consider
** \param   num_objects - number of objects to consider in the slice
**
**
** \return  First object that failed to delete (if all failed to delete in the slice)
**          or NULL if all objects in the slice deleted successfully
**
**************************************************************************/
group_del_entry_t *GROUP_DEL_VECTOR_FindFirstFailureIfAllFailed(group_del_vector_t *gdv, int index, int num_objects)
{
    int i;
    group_del_entry_t *gde;
    group_del_entry_t *first_failure = NULL;

    // Exit if there were no objects to delete in this slice, indicating success
    // This can occur if the requested path resolves to zero objects (because the object has already been deleted)
    if (num_objects == 0)
    {
        return NULL;
    }

    for (i=index; i < index+num_objects; i++)
    {
        gde = &gdv->vector[i];
        if (gde->err_code == USP_ERR_OK)
        {
            return NULL;    // Success is indicated if any of the objects were deleted successfully
        }

        // If the code gets here, then the object failed to create
        if (first_failure == NULL)
        {
            first_failure = gde;
        }
    }

    // Only if all of the objects failed to delete should a failure be indicated
    return first_failure;
}
