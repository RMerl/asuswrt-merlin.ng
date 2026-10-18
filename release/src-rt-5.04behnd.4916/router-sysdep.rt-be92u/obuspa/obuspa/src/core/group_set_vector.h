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
 * Header file for functions performing setting of a mixed group of parameters
 *
 */

#ifndef GROUP_SET_VECTOR_H
#define GROUP_SET_VECTOR_H

#include "str_vector.h"
#include "device.h"      // for combined_role_t

//------------------------------------------------------------------------------
// Structure containing info for parameter to set, and its value or error
typedef struct
{
    char *path;         // Full data model path to the parameter
    int group_id;       // Group which the parameter belongs to. All parameters in the same group can be obtained at the same time.
    unsigned type_flags;// Type of the parameter eg DM_STRING. Some low level (eg RDK) APIs require this additional info
    char *value;        // NOTE: ownership of the value string stays with the USP request message
    bool is_required;   // Whether the USP message marked the parameter as being required or not
    int err_code;       // Set if an error occured when trying to set the parameter
    char *err_msg;      // Set with an error string if an error occured when trying to set the parameter
} group_set_entry_t;

//------------------------------------------------------------------------------
// Vector of all parameters to set (for all groups)
typedef struct
{
    group_set_entry_t *vector;
    int num_entries;
} group_set_vector_t;

//------------------------------------------------------------------------------
// API
void GROUP_SET_VECTOR_Init(group_set_vector_t *gsv);
void GROUP_SET_VECTOR_Destroy(group_set_vector_t *gsv);
void GROUP_SET_VECTOR_Add(group_set_vector_t *gsv, char *path, char *value, bool is_required, combined_role_t *combined_role);
int GROUP_SET_VECTOR_GetFailureIndex(group_set_vector_t *gsv, int index, int num_entries);
void GROUP_SET_VECTOR_SetValues(group_set_vector_t *gsv, int index, int num_entries);

#endif
