/*
 * Wireless Application Event Service
 * shared header file
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _appeventd_h_
#define _appeventd_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <appevent_hdr.h>

extern int appeventd_debug_level;

#define APPEVENTD_DEBUG_ERROR	0x0001
#define APPEVENTD_DEBUG_WARNING	0x0002
#define APPEVENTD_DEBUG_INFO	0x0004
#define APPEVENTD_DEBUG_DETAIL	0x0008

#define APPEVENTD_ERROR(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_ERROR) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_WARNING(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_WARNING) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_INFO(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_INFO) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_DEBUG(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_DETAIL) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_BUFSIZE	2048

#define APPEVENTD_OK	0
#define APPEVENTD_FAIL -1

/* WiFi Application Event ID */
#define APP_E_BSD_STEER_START 1  /* status: STEERING */
#define APP_E_BSD_STEER_END   2  /* status: SUCC/FAIL */
#define APP_E_BSD_STATS_QUERY 3  /* status: STA/RADIO */
#define APP_E_WBD_SLAVE_WEAK_CLIENT 4  /* status: SUCC */
#define APP_E_WBD_SLAVE_STEER_START 5  /* status: SUCC */
#define APP_E_WBD_SLAVE_STEER_RESP  6  /* status: SUCC */
#define APP_E_WBD_MASTER_WEAK_CLIENT 7  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_START 8  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_RESP  9  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_END   10  /* status: SUCC */
#define APP_E_WBD_MASTER_MAP_INIT_START   11  /* status: SUCC */
#define APP_E_WBD_MASTER_MAP_INIT_END   12  /* status: SUCC */
#define APP_E_WBD_SLAVE_MAP_INIT_END 13  /* status: SUCC */
#endif /* _appeventd_h_ */
