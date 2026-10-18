/*
 *
 * Copyright (C) 2022-2024, Broadband Forum
 * Copyright (C) 2022, Snom Technology GmbH
 * Copyright (C) 2023  CommScope, Inc
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
 * \file e2e_context.h
 *
 * Header file containing helpers and functions for End-to-End exchanges.
 *
 */
#ifndef E2E_CONTEXT_H
#define E2E_CONTEXT_H

#include "vendor_defs.h"  // For E2ESESSION_EXPERIMENTAL_USP_V_1_2
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)

#include "usp-record.pb-c.h"

#include "e2e_defs.h"
#include "sar_vector.h"
#include "device.h"
#include "usp_record.h"

// Utility functions
int E2E_CONTEXT_QueueUspSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id,
                                      mtp_conn_t *mtpc, time_t expiry_time);
int E2E_CONTEXT_HandleUspRecord(UspRecord__Record *rec, int role_instance, mtp_conn_t *mtpc);
char *E2E_CONTEXT_SarStateToString(int state);
char *E2E_CONTEXT_E2eSessionEventToString(int event);
char *E2E_CONTEXT_E2eSessionStatusToString(int status);
char *E2E_CONTEXT_E2eSessionModeToString(int mode);
int E2E_CONTEXT_E2eSessionModeToEnum(char *str);
void E2E_CONTEXT_E2eSessionEvent(e2e_event_t event, int request, int controller);
int E2E_CONTEXT_ValidateSessionContextRecord(UspRecord__SessionContextRecord *ctx);
bool E2E_CONTEXT_IsToSendThroughSessionContext(e2e_session_t *e2e);
#endif  // #if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)

#endif
