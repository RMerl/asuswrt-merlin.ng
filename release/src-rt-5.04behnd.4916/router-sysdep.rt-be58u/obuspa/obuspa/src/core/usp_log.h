/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
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
 * \file usp_log.h
 *
 * Header file for USP low leel logging functionality
 *
 */
#ifndef USP_LOG_H
#define USP_LOG_H

//------------------------------------------------------------------------------------
// Enumeration for type of information being logged
typedef enum
{
    kLogLevel_Off = 0,
    kLogLevel_Error = 1,
    kLogLevel_Warning = 2,
    kLogLevel_Info = 3,
    kLogLevel_Debug = 4,

    kLogLevel_Max       // This should always be the last value in the enum
} log_level_t;

//------------------------------------------------------------------------------------
// Enumeration for type of information being logged
typedef enum
{
    kLogType_Debug,
    kLogType_Dump,
    kLogType_Protocol
} log_type_t;

//------------------------------------------------------------------------------------
// API
void USP_LOG_Init(void);
int USP_LOG_SetFile(const char *file);
void USP_LOG_Callstack(void);
void USP_LOG_HexBuffer(const char *title, const unsigned char *buf, int len);
void USP_LOG_String(log_level_t log_level, log_type_t log_type, char *str);
void USP_LOG_Printf(log_level_t log_level, log_type_t log_type, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void USP_LOG_Puts(log_level_t log_level, log_type_t log_type, const char *str);
void USP_LOG_ErrorSSL(const char *func_name, const char *failure_string, int ret, int err);

//------------------------------------------------------------------------------------
// Macros used for normal debug logging
extern log_level_t usp_log_level;
extern bool enable_protocol_trace;
extern bool enable_callstack_debug;

#define USP_LOG_Error(...)       if (usp_log_level >= kLogLevel_Error)   { USP_LOG_Printf(kLogLevel_Error, kLogType_Debug, __VA_ARGS__); if (enable_callstack_debug) { USP_LOG_Callstack(); } }
#define USP_LOG_Warning(...)     if (usp_log_level >= kLogLevel_Warning) { USP_LOG_Printf(kLogLevel_Warning, kLogType_Debug, __VA_ARGS__); }
#define USP_LOG_Info(...)        if (usp_log_level >= kLogLevel_Info)    { USP_LOG_Printf(kLogLevel_Info, kLogType_Debug, __VA_ARGS__); }
#define USP_LOG_Debug(...)       if (usp_log_level >= kLogLevel_Debug)   { USP_LOG_Printf(kLogLevel_Debug, kLogType_Debug, __VA_ARGS__); }

// Macro used to dump out the data model/database etc
#define USP_DUMP(...)       USP_LOG_Printf(kLogLevel_Debug, kLogType_Dump, __VA_ARGS__)

// Macro used to print out STOMP frames
#define USP_PROTOCOL(...)   USP_LOG_Printf(kLogLevel_Debug, kLogType_Protocol, __VA_ARGS__)

// Maximum number of characters in a single log statement
#define USP_LOG_MAXLEN  (10*1024)
#endif
