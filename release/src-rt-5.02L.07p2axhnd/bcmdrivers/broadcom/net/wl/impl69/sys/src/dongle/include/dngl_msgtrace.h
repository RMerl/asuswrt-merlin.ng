/*
 * Trace messages sent over HBUS
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dngl_msgtrace.h 542902 2015-03-22 23:29:48Z $
 */

#ifndef _dngl_msgtrace_h_
#define _dngl_msgtrace_h_

#include <typedefs.h>
#include <osl_decl.h>

typedef struct msgtrace msgtrace_t;
msgtrace_t *msgtrace_get_addr(void);

/* The hbus driver generates traces when sending a trace message. This causes endless traces.
 * This flag must be set to TRUE in any hbus traces. The flag is reset in the function msgtrace_put.
 * This prevents endless traces but generates hasardous lost of traces only in bus device code.
 * It is recommendat to set this flag in macro SD_TRACE but not in SD_ERROR for avoiding missing
 * hbus error traces. hbus error trace should not generates endless traces.
 */
extern bool msgtrace_hbus_trace;

/* setup trace sendup func */
typedef void (*msgtrace_sendup_trace_fn_t)(void *ctx, uint8 *hdr, uint16 hdrlen,
	uint8 *buf, uint16 buflen);
void msgtrace_set_sendup_trace_fn(msgtrace_sendup_trace_fn_t fn, void *ctx);

extern void msgtrace_start(void);
extern void msgtrace_stop(void);
extern int msgtrace_sent(void);
extern void msgtrace_init(osl_t *osh);
extern bool msgtrace_event_enabled(void);

#endif /* _dngl_msgtrace_h_ */
