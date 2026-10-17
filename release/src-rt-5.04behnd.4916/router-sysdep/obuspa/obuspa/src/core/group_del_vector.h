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
 * \file group_del_vector.h
 *
 * Header file for an intermediate data structure which helps with handling an Delete Request
 *
 */

#ifndef GROUP_DEL_VECTOR_H
#define GROUP_DEL_VECTOR_H

#include "device.h"
#include "str_vector.h"
#include "int_vector.h"

//------------------------------------------------------------------------------
// Structure containing info for resolved object to delete
typedef struct
{
    char *path;         // The resolved object to delete
    int group_id;       // Group which the object belongs to

    int err_code;       // Indicates whether this object was deleted successfully or not
    char *err_msg;      // Error message, if failed to delete this object. Only set if err_code indicates an error
} group_del_entry_t;

//------------------------------------------------------------------------------
// Vector of all objects to delete (for all groups)
typedef struct
{
    group_del_entry_t *vector;
    int num_entries;
} group_del_vector_t;

//------------------------------------------------------------------------------
// API
void GROUP_DEL_VECTOR_Init(group_del_vector_t *gdv);
void GROUP_DEL_VECTOR_Destroy(group_del_vector_t *gdv);
void GROUP_DEL_VECTOR_AddObjectsToDelete(group_del_vector_t *gdv, str_vector_t *obj_paths, int_vector_t *group_ids);
void GROUP_DEL_VECTOR_AddObjectNotDeleted(group_del_vector_t *gdv, char *obj_path, int err_code, char *err_msg);
bool GROUP_DEL_VECTOR_AreAllPathsTheSameGroupId(group_del_vector_t *gdv, int *group_id);
group_del_entry_t *GROUP_DEL_VECTOR_FindFirstFailureIfAllFailed(group_del_vector_t *gdv, int start_index, int end_index);

#endif
