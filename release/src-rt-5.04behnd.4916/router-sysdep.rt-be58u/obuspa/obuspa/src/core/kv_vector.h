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
 * \file kv_vector.h
 *
 * Implements a vector of key-value pairs
 *
 */

#ifndef KV_VECTOR_H
#define KV_VECTOR_H

#include <time.h>
#include <limits.h>         // included as a convenience because calls to KV_VECTOR_GetUnsignedWithinRange() often reference UINT_MAX

#include "usp_api.h"
#include "str_vector.h"

//-----------------------------------------------------------------------------------------
// Key-value pair Vector API
void KV_VECTOR_Init(kv_vector_t *kvv);
void KV_VECTOR_Add(kv_vector_t *kvv, char *key, char *value);
bool KV_VECTOR_Replace(kv_vector_t *kvv, char *key, char *value);
bool KV_VECTOR_ReplaceWithHint(kv_vector_t *kvv, char *key, char *value, int hint);
void KV_VECTOR_AddUnsigned(kv_vector_t *kvv, char *key, unsigned value);
void KV_VECTOR_AddBool(kv_vector_t *kvv, char *key, bool value);
void KV_VECTOR_AddDateTime(kv_vector_t *kvv, char *key, time_t value);
void KV_VECTOR_AddEnum(kv_vector_t *kvv, char *key, int value, const enum_entry_t *enums, int num_enums);
void KV_VECTOR_AddHexNumber(kv_vector_t *kvv, char *key, unsigned char *buf, int len);
void KV_VECTOR_Destroy(kv_vector_t *kvv);
void KV_VECTOR_Dump(kv_vector_t *kvv);
int  KV_VECTOR_FindKey(kv_vector_t *kvv, char *key, int start_index);
int KV_VECTOR_ValidateArguments(kv_vector_t *args, str_vector_t *expected_schema, unsigned flags);

char *KV_VECTOR_Get(kv_vector_t *kvv, char *key, char *default_value, int start_index);
int KV_VECTOR_GetUnsigned(kv_vector_t *kvv, char *key, unsigned default_value, unsigned *value);
int KV_VECTOR_GetUnsignedWithinRange(kv_vector_t *kvv, char *key, unsigned default_value, unsigned min, unsigned max, unsigned *value);
int KV_VECTOR_GetInt(kv_vector_t *kvv, char *key, int default_value, int *value);
int KV_VECTOR_GetIntWithinRange(kv_vector_t *kvv, char *key, int default_value, int min, int max, int *value);
int KV_VECTOR_GetBool(kv_vector_t *kvv, char *key, bool default_value, bool *value);
int KV_VECTOR_GetDateTime(kv_vector_t *kvv, char *key, char *default_value, time_t *value);
int KV_VECTOR_GetHexNumber(kv_vector_t *kvv, char *key, unsigned char *buf, int len, int *bytes_copied);
int KV_VECTOR_GetEnum(kv_vector_t *kvv, char *key, void *value, int default_value, const enum_entry_t *enums, int num_enums);

//-----------------------------------------------------------------------------------------
// Defines for flags parameter of KV_VECTOR_ValidateArguments()
#define IGNORE_UNKNOWN_ARGS 0x00000001

#endif
