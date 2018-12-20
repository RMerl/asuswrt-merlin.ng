/*
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
 * $Id: dpsta_linux.h 402535 2013-05-16 02:04:53Z $
 */

#ifndef _DPSTA_LINUX_H_
#define _DPSTA_LINUX_H_

#define DPSTACVAR 1

/* dpsta_ioctl cmd */
#define DPSTA_CMD_ENABLE	SIOCDEVPRIVATE
#define DPSTA_CMD_SETGETVAR	(SIOCDEVPRIVATE + DPSTACVAR)

/* dpsta iovar */
#define DPSTA_IOV_UIF 1
#define DPSTA_IOV_MSGLEVEL 2
#define DPSTA_IOV_DPINFO 3

/* dpsta msglevel */
#define DPSTA_ERROR_VAL 0x00000001
#define DPSTA_TRACE_VAL 0x00000002
#define DPSTA_PKT_VAL   0x00000004

#define DPSTA_POLICY_AUTO	0
#define DPSTA_POLICY_SAMEBAND	1
#define DPSTA_POLICY_CROSSBAND	2
#define DPSTA_POLICY_AUTO_5G    3

#define DPSTA_LAN_UIF_AUTO	0	/* UIF is not assigned/fixed for LAN traffic (default 2G)*/
#define DPSTA_LAN_UIF_2G	1	/* Use 2G as UIF for LAN traffic */
#define DPSTA_LAN_UIF_5G	2	/* Use 5G as UIF for LAN traffic */
#define DPSTA_LAN_UIF_AUTO_5G	3	/* UIF is not assigned/fixed for LAN traffic (default 5G) */

#define DPSTA_NUM_UPSTREAM_IF	3

typedef struct dpsta_enable_info {
	bool	enable;			/* Enable/Disable Dualband PSTA mode */
	uint32	policy;			/* Inband or crossband repeating */
	uint32	lan_uif;		/* Upstream interface for lan traffic */
					/* Upstream interfaces managed by DPSTA */
	uint8	upstream_if[DPSTA_NUM_UPSTREAM_IF][IFNAMSIZ];
} dpsta_enable_info_t;

/* structure to send a generic var set/get */
typedef struct dpsta_var_s {
	uint cmd;
	uint set;
	uint len;
	union {
		int arg;
		char uif[IFNAMSIZ];
		dpsta_enable_info_t dpinfo;
	};
} dpsta_var_t;
#endif /* _DPSTA_LINUX_H_ */
