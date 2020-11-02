/*
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dpsta_linux.h 773108 2019-03-13 07:14:59Z $
 */

#ifndef _DPSTA_LINUX_H_
#define _DPSTA_LINUX_H_

#define DPSTA_CMD_ENABLE	SIOCDEVPRIVATE

/* dpsta msglevel */
#define DPSTA_ERROR_VAL 0x00000001
#define DPSTA_TRACE_VAL 0x00000002
#define DPSTA_PKT_VAL   0x00000004

/* upstream policy for wireless client */
enum dpsta_policy {
	DPSTA_POLICY_AUTO = 0,  /* If same radio is not connected, go to cross radio */
	DPSTA_POLICY_SAMEBAND,  /* Force to same radio */
	DPSTA_POLICY_CROSSBAND, /* Force to cross radio */
	DPSTA_POLICY_AUTO_0,    /* If radio-0 is not connected, go to radio-1 */
	DPSTA_POLICY_AUTO_1,    /* If radio-1 is not connected, go to radio-0 */
	DPSTA_POLICY_LAST
};

/* upstream policy for wired client (host/ethernet) */
enum dpsta_lan_uif {
	DPSTA_LAN_UIF_AUTO_0 = 0, /* If both upstream radios are connnected, prefer radio-0 */
	DPSTA_LAN_UIF_0,          /* Use radio-0 as UIF for LAN traffic */
	DPSTA_LAN_UIF_1,          /* Use radio-1 as UIF for LAN traffic */
	DPSTA_LAN_UIF_AUTO_1,     /* If both upstream radios are connnected, prefer radio-1 */
	DPSTA_LAN_UIF_LAST
};

#define DPSTA_MAX_UPSTREAM_IF 3 /* Total number of wifi radios that could be used for upstream */
#define DPSTA_USED_UPSTREAM_IF 2 /* Only use two upstreams even we have more than two radios */

typedef struct dpsta_enable_info {
	bool	enable;			/* Enable/Disable Dualband PSTA mode */
	uint32	policy;			/* Inband or crossband repeating */
	uint32	lan_uif;		/* Upstream interface for lan traffic */
					/* Upstream interfaces managed by DPSTA */
	uint8	upstream_if[DPSTA_MAX_UPSTREAM_IF][IFNAMSIZ];
} dpsta_enable_info_t;

typedef struct dpsta_cnt_s {
	uint32  tx[DPSTA_MAX_UPSTREAM_IF];	/**< tx data for wired/host clients */
	uint32  wtx[DPSTA_MAX_UPSTREAM_IF];	/**< tx data for wireless clients */
	uint32  rx;	/**< dpsta rx data */
} dpsta_cnt_t;
#endif /* _DPSTA_LINUX_H_ */
