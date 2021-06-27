/*
 * Copyright (c) 2020, NLNet Labs, Sinodun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

#include <getdns/getdns.h>
#include <getdns/getdns_extra.h>

typedef void(*stubby_verror_t)(getdns_loglevel_type level, const char *fmt, va_list ap);
typedef void(*stubby_vlog_t)(void *userarg, uint64_t system,
                             getdns_loglevel_type level,
                             const char *fmt, va_list ap);

void stubby_set_verror(stubby_verror_t err);
void stubby_set_vlog(stubby_vlog_t log);

void stubby_log(void *userarg, uint64_t system,
                getdns_loglevel_type level, const char *fmt, ...);

void stubby_error(const char *fmt, ...);
void stubby_warning(const char *fmt, ...);
void stubby_debug(const char *fmt, ...);

void stubby_set_log_funcs(stubby_verror_t errfunc, stubby_vlog_t logfunc);

void stubby_set_getdns_logging(getdns_context *context, int loglevel);

#endif
