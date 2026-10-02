/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file str_vector.h
 *
 * Implements a vector of strings
 *
 */

#ifndef STR_VECTOR_H
#define STR_VECTOR_H

//-----------------------------------------------------------------------------------------
// String vector type
typedef struct
{
    char **vector;
    int num_entries;
} str_vector_t;

//-----------------------------------------------------------------------------------------
#include "kv_vector.h"

//-----------------------------------------------------------------------------------------
// String Vector API
void STR_VECTOR_Init(str_vector_t *sv);
void STR_VECTOR_Clone(str_vector_t *sv, char **src_vector, int src_num_entries);
void STR_VECTOR_Add(str_vector_t *sv, char *str);
void STR_VECTOR_Add_IfNotExist(str_vector_t *sv, char *str);
int STR_VECTOR_Find(str_vector_t *sv, char *str);
void STR_VECTOR_Destroy(str_vector_t *sv);
void STR_VECTOR_Dump(str_vector_t *sv);
void STR_VECTOR_ConvertToKeyValueVector(str_vector_t *sv, kv_vector_t *kvv);
bool STR_VECTOR_Compare(str_vector_t *sv1, str_vector_t *sv2);
void STR_VECTOR_Sort(str_vector_t *sv);

#endif
