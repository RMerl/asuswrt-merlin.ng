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
 * \file msg_utils.h
 *
 * Header file for functions implementing common USP message handling utility functions
 * (currently only used when running as a USP Service or a USP Broker)
 *
 */
#ifndef MSG_UTILS_H
#define MSG_UTILS_H

#include "common_defs.h"
#include "usp-record.pb-c.h"
#include "usp-msg.pb-c.h"
#include "kv_vector.h"

//------------------------------------------------------------------------------
// API
Usp__Msg *MSG_UTILS_Create_GetReq(char *msg_id, kv_vector_t *kvv);
Usp__Msg *MSG_UTILS_Create_SetReq(char *msg_id, kv_vector_t *kvv);
Usp__Msg *MSG_UTILS_Create_AddReq(char *msg_id, char *path, group_add_param_t *params, int num_params);
Usp__Msg *MSG_UTILS_Create_DeleteReq(char *msg_id, str_vector_t *paths, bool allow_partial);
Usp__Msg *MSG_UTILS_Create_GetSupportedDMReq(char *msg_id, str_vector_t *sv);
Usp__Msg *MSG_UTILS_Create_GetInstancesReq(char *msg_id, str_vector_t *sv);
Usp__Msg *MSG_UTILS_Create_OperateReq(char *msg_id, char *path, char *command_key, kv_vector_t *input_args);
int MSG_UTILS_ValidateUspResponse(Usp__Msg *resp, Usp__Response__RespTypeCase response_type, char **param_errs_path);
int MSG_UTILS_ProcessUspService_GetResponse(Usp__Msg *resp, kv_vector_t *kvv);

#endif
