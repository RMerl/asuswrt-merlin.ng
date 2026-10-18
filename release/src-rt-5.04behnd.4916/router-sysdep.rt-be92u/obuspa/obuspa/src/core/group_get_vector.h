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
 * \file group_get_vector.h
 *
 * Header file for functions performing getting of a mixed group of parameters
 *
 */

#ifndef GROUP_GET_VECTOR_H
#define GROUP_GET_VECTOR_H

#include "str_vector.h"
#include "kv_vector.h"

//------------------------------------------------------------------------------
// Structure containing info for parameter to get, and its value or error
typedef struct
{
    char *path;
    int group_id;
    char *value;
    int err_code;
    char *err_msg;
} group_get_entry_t;

//------------------------------------------------------------------------------
// Vector of all parameters to get (for all groups)
typedef struct
{
    group_get_entry_t *vector;
    int num_entries;
} group_get_vector_t;

//------------------------------------------------------------------------------
// API
void GROUP_GET_VECTOR_Init(group_get_vector_t *ggv);
void GROUP_GET_VECTOR_Destroy(group_get_vector_t *ggv);
void GROUP_GET_VECTOR_Add(group_get_vector_t *ggv, char *path, int group_id);
int GROUP_GET_VECTOR_FindParam(group_get_vector_t *ggv, char *path);
void GROUP_GET_VECTOR_AddParams(group_get_vector_t *ggv, str_vector_t *params, int_vector_t *group_ids);
void GROUP_GET_VECTOR_ConvertToKeyValueVector(group_get_vector_t *ggv, kv_vector_t *kvv);
void GROUP_GET_VECTOR_GetValues(group_get_vector_t *ggv);

#endif
