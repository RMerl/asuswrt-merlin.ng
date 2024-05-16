/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file iso8601.h
 *
 * ISO8601 Time Format string utilities
 *
 */

#ifndef _ISO8601_H_
#define _ISO8601_H_

#include <time.h>
#include <sys/time.h>
#include <limits.h>

//------------------------------------------------------------------
// Definition for Unknown Time value (from TR-106a7 section A.2.3.5)
#define UNKNOWN_TIME_STR  "0001-01-01T00:00:00Z"
#define UNKNOWN_TIME      ((time_t)(INT_MIN))  // This is a special case, represented in 32 bits, so that the code works with 32 bit Linux kernels

//------------------------------------------------------------------
// Functions
char *iso8601_cur_time(char *buf, int len);
char *iso8601_from_unix_time(time_t unix_time, char *buf, int len);
size_t iso8601_strftime(char *buf, size_t buflen, const struct tm *tm);
size_t iso8601_us_strftime(char *buf, size_t bufsiz, const struct timeval *tv);
bool iso8601_is_valid(const char *date);
time_t iso8601_to_unix_time(const char *date);
size_t uptime_strftime(char *buf, size_t buflen, unsigned uptime);

#endif
