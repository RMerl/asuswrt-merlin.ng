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
 * \file usp_service.h
 *
 * Header file for functions implementing USP Service functionality
 *
 */

#ifndef USP_SERVICE_H
#define USP_SERVICE_H
#include "usp_api.h"

//------------------------------------------------------------------------------
// API
void USP_SERVICE_Stop(void);
void USP_SERVICE_QueueRegisterRequest(char *endpoint_id, char *objects);
void USP_SERVICE_HandleRegisterResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void USP_SERVICE_QueueDeregisterRequest(char *endpoint_id, char *objects);
void USP_SERVICE_HandleDeRegisterResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
bool USP_SERVICE_AsController_IsExpectedResponse(Usp__Msg *usp);
int USP_SERVICE_SetBrokerAgent(char *endpoint_id, mtp_conn_t *mrt);

#endif
