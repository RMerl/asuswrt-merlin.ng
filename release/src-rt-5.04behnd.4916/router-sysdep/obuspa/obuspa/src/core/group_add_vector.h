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
 * \file group_add_vector.h
 *
 * Header file for an intermediate data structure which helps with handling an Add Request
 *
 */

#ifndef GROUP_ADD_VECTOR_H
#define GROUP_ADD_VECTOR_H

#include "device.h"
#include "kv_vector.h"

//------------------------------------------------------------------------------
// Structure containing info for object to set, and its value or error
typedef struct
{
    char *req_path;     // Path expression from Add request that resolved to res_path
    char *res_path;     // Resolved data model path to the object to add
    int instance;       // Instance number of the object that was added
    int group_id;       // Group which the object belongs to
    group_add_param_t *params;  // Pointer to array of parameters to set NOTE: ownership of this array stays with the caller (because the caller may use the same array for multiple entries in the group add vector)
    int num_params;

    int err_code;       // Indicates whether this object was created successfully or not
    char *err_msg;      // Error message, if failed to add this object. Only set if err_code indicates an error

    kv_vector_t unique_keys;  // Unique keys for the object, if added successfully

} group_add_entry_t;

//------------------------------------------------------------------------------
// Vector of all objects and child parameters to add (for all groups)
typedef struct
{
    group_add_entry_t *vector;
    int num_entries;
} group_add_vector_t;

//------------------------------------------------------------------------------
// API
void GROUP_ADD_VECTOR_Init(group_add_vector_t *gav);
void GROUP_ADD_VECTOR_Destroy(group_add_vector_t *gav);
void GROUP_ADD_VECTOR_AddObjectToCreate(group_add_vector_t *gav, char *req_path, char *res_path, int group_id);
void GROUP_ADD_VECTOR_AddParamSetting(group_add_vector_t *gav, char *param_name, char *value, bool is_required);
void GROUP_ADD_VECTOR_AddObjectNotCreated(group_add_vector_t *gav, char *req_path, int err_code, char *err_msg);
int GROUP_ADD_VECTOR_CreateObject(group_add_entry_t *gae, combined_role_t *combined_role);
group_add_param_t *GROUP_ADD_VECTOR_FindFirstFailedParam(group_add_entry_t *gae);
void GROUP_ADD_VECTOR_Rollback(group_add_vector_t *gav, int rollback_span);

#endif
