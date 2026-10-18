/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file usp_log.h
 *
 * Header file for USP low leel logging functionality
 * NOTE: The USP_LOG_XXX API functions have been moved to usp_api.h
 *
 */
#ifndef USP_LOG_H
#define USP_LOG_H

#include "usp_api.h"      // For backwards compatibility with this file declaring all USP_LOG_XXX API functions

//------------------------------------------------------------------------------------
// API
// NOTE: The USP_LOG_XXX API functions that are vendor callable have been moved to usp_api.h
void USP_LOG_Init(void);
int USP_LOG_SetFile(const char *file);

#ifndef REMOVE_DEVICE_SECURITY
void USP_LOG_ErrorSSL(const char *func_name, const char *failure_string, int ret, int err);
#endif
//------------------------------------------------------------------------------------
// Exported global variables
extern bool enable_protocol_trace;
extern bool enable_callstack_debug;

//------------------------------------------------------------------------------------
// Macro used to dump out the data model/database etc
#define USP_DUMP(...)       USP_LOG_Printf(kLogLevel_Debug, kLogType_Dump, __VA_ARGS__)

// Macro used to print out STOMP frames
#define USP_PROTOCOL(...)   USP_LOG_Printf(kLogLevel_Debug, kLogType_Protocol, __VA_ARGS__)

#endif
