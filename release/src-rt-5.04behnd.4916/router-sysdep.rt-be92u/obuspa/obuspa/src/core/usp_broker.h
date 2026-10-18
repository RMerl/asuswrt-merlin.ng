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
 * \file usp_broker.h
 *
 * Header file for functions implementing USP Broker functionality
 *
 */

#ifndef USP_BROKER_H
#define USP_BROKER_H

#include "uds.h"
#include "usp-record.pb-c.h"


//------------------------------------------------------------------------------
// Defines for flags argument of USP_BROKER_GetUspServiceInstance()
#define ONLY_CONTROLLER_CONNECTIONS 0x00000001      // Only return instance number if USP Service is connected as a Controller to the USP Broker's agent socket

//------------------------------------------------------------------------------
// API
int USP_BROKER_Init(void);
int USP_BROKER_Start(void);
void USP_BROKER_Stop(void);
int USP_BROKER_AddUspService(char *endpoint_id, mtp_conn_t *mtpc);
void USP_BROKER_HandleRegister(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void USP_BROKER_HandleDeRegister(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void USP_BROKER_HandleGetSupportedDMResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void USP_BROKER_HandleNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
int USP_BROKER_IsPathVendorSubscribable(subs_notify_t notify_type, char *path, bool *is_present);
int USP_BROKER_GetUspServiceInstance(char *endpoint_id, unsigned flags);
void USP_BROKER_GetAllRegisteredGroupIds(int_vector_t *iv);
mtp_conn_t *USP_BROKER_GetNotifyDestForEndpoint(char *endpoint_id, Usp__Header__MsgType usp_msg_type);
bool USP_BROKER_AttemptPassthru(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec);
void USP_BROKER_HandleUspServiceDisconnect(char *endpoint_id, uds_path_t path_type);
bool USP_BROKER_AttemptDirectGetForCli(char *path);

#endif
