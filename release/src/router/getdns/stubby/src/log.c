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

#include "config.h"

#include <stdarg.h>
#include <stdio.h>

#if defined(STUBBY_ON_WINDOWS)
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#include "log.h"

static void default_stubby_verror(getdns_loglevel_type level, const char *fmt, va_list ap)
{
        (void) level;
        (void) vfprintf(stderr, fmt, ap);
        (void) fputc('\n', stderr);
}

static void default_stubby_vlog(void *userarg, uint64_t system,
                                getdns_loglevel_type level,
                                const char *fmt, va_list ap)
{
        struct timeval tv;
        struct tm tm;
        char buf[10];
#if defined(STUBBY_ON_WINDOWS)
        struct _timeb timeb;
        time_t tsec;

        _ftime_s(&timeb);
        tsec = (time_t)timeb.time;
        tv.tv_usec = timeb.millitm * 1000;
        gmtime_s(&tm, &tsec);
#else
        gettimeofday(&tv, NULL);
        gmtime_r(&tv.tv_sec, &tm);
#endif
        strftime(buf, 10, "%H:%M:%S", &tm);
        (void)userarg; (void)system; (void)level;
        (void) fprintf(stderr, "[%s.%.6d] STUBBY: ", buf, (int)tv.tv_usec);
        (void) vfprintf(stderr, fmt, ap);
        (void) fputc('\n', stderr);
}

static stubby_verror_t stubby_verror = default_stubby_verror;
static stubby_vlog_t stubby_vlog = default_stubby_vlog;

void stubby_error(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        stubby_verror(GETDNS_LOG_ERR, fmt, args);
        va_end(args);
}

void stubby_warning(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        stubby_verror(GETDNS_LOG_WARNING, fmt, args);
        va_end(args);
}

void stubby_debug(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        stubby_verror(GETDNS_LOG_DEBUG, fmt, args);
        va_end(args);
}

void stubby_log(void *userarg, uint64_t system,
                getdns_loglevel_type level, const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        stubby_vlog(userarg, system, level, fmt, args);
        va_end(args);
}

void stubby_set_log_funcs(stubby_verror_t errfunc, stubby_vlog_t logfunc)
{
        stubby_verror = errfunc;
        stubby_vlog = logfunc;
}

void stubby_set_getdns_logging(getdns_context *context, int loglevel)
{
        (void) getdns_context_set_logfunc(context, NULL, GETDNS_LOG_UPSTREAM_STATS, loglevel, stubby_vlog);
}
