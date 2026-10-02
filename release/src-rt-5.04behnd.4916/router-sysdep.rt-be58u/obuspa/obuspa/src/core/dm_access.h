/*
 *
 * Copyright (C) 2019-2021, Broadband Forum
 * Copyright (C) 2016-2021  CommScope, Inc
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
 * \file dm_access.h
 *
 * Header file for API to access data model parameters by type
 *
 */
#ifndef DM_ACCESS_H
#define DM_ACCESS_H

#include <time.h>
#include "str_vector.h"
#include "nu_ipaddr.h"

//-------------------------------------------------------------------------
// The prefix to use when forming the default value of an Alias parameter
#define DEFAULT_ALIAS_PREFIX "cpe-"

//-------------------------------------------------------------------------
// API functions
int DM_ACCESS_GetString(char *path, char **p_str);
int DM_ACCESS_GetPassword(char *path, char **p_str);
int DM_ACCESS_GetInteger(char *path, int *value);
int DM_ACCESS_GetUnsigned(char *path, unsigned *value);
int DM_ACCESS_GetBool(char *path, bool *value);
int DM_ACCESS_GetEnum(char *path, void *value, const enum_entry_t *enums, int num_enums);
int DM_ACCESS_GetDateTime(char *path, time_t *value);
int DM_ACCESS_GetStringVector(char *path, str_vector_t *sv);
int DM_ACCESS_GetIpAddr(char *path, nu_ipaddr_t *ip_addr);

int DM_ACCESS_SetInteger(char *path, int value);

int DM_ACCESS_ValidateBool(dm_req_t *req, char *value);
int DM_ACCESS_ValidateBase64(dm_req_t *req, char *value);
int DM_ACCESS_ValidatePort(dm_req_t *req, char *value);
int DM_ACCESS_ValidateRange_Unsigned(dm_req_t *req, unsigned min_value, unsigned max_value);
int DM_ACCESS_ValidateRange_Signed(dm_req_t *req, int min_value, int max_value);
int DM_ACCESS_ValidateReference(char *reference, char *table, int *instance);
int DM_ACCESS_ValidateIpAddr(dm_req_t *req, char *value);

int DM_ACCESS_CompareString(char *lhs, expr_op_t op, char *rhs, bool *result);
int DM_ACCESS_CompareNumber(char *lhs, expr_op_t op, char *rhs, bool *result);
int DM_ACCESS_CompareBool(char *lhs, expr_op_t op, char *rhs, bool *result);
int DM_ACCESS_CompareDateTime(char *lhs, expr_op_t op, char *rhs, bool *result);
int DM_ACCESS_RestartAsyncOperation(dm_req_t *req, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args);
int DM_ACCESS_DontRestartAsyncOperation(dm_req_t *req, int instance, bool *is_restart, int *err_code, char *err_msg, int err_msg_len, kv_vector_t *output_args);
int DM_ACCESS_PopulateAliasParam(dm_req_t *req, char *buf, int len);



#endif

