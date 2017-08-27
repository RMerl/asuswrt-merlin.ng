/*
 * Linux-specific portion of
 * Broadcom 802.11 Networking Device Driver
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
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
 * $Id: wl_thread.h $
 */

#ifndef _wl_thread_h_
#define _wl_thread_h_

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#if defined(BCM_WFD) && defined(WL_ALL_PASSIVE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
#define WL_USE_L34_THREAD
#endif

struct wl_info;
struct wl_task;

extern int wl_thread_attach(struct wl_info *wl);
extern void wl_thread_detach(struct wl_info *wl);
extern void wl_thread_schedule_work(struct wl_info *wl);

#endif /* _wl_thread_h_ */
