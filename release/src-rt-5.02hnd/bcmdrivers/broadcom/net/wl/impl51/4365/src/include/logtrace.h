/*
 * Trace log blocks sent over HBUS
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: logtrace.h 333856 2012-05-17 23:43:07Z $
 */

#ifndef	_LOGTRACE_H
#define	_LOGTRACE_H

#include <msgtrace.h>
#include <osl_decl.h>
extern void logtrace_start(void);
extern void logtrace_stop(void);
extern int logtrace_sent(void);
extern void logtrace_trigger(void);
extern void logtrace_init(void *hdl1, void *hdl2, msgtrace_func_send_t func_send);
extern bool logtrace_event_enabled(void);

#endif	/* _LOGTRACE_H */
