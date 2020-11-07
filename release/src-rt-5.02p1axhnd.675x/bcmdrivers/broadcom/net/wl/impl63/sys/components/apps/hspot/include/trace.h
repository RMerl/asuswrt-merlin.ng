/*
 * Tracing utility.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id:$
 */

#ifndef _TRACE_H_
#define _TRACE_H_

#include <stdio.h>
#include <ctype.h>
#include "typedefs.h"
#include "ethernet.h"
#ifdef BCMDRIVER
#include <wl_dbg.h>
#include <wlioctl.h>
#include <bcmutils.h>
#endif // endif

/* --------------------------------------------------------------- */

typedef enum {
	TRACE_NONE		= 0x0000,
	TRACE_ERROR		= 0x0001,
	TRACE_DEBUG		= 0x0002,
	TRACE_EVENT		= 0x0004,
	TRACE_PACKET	= 0x0008,
	TRACE_VERBOSE	= 0x0010,
	TRACE_ALL		= 0xffff,
	TRACE_PRINTF	= 0x10000	/* output same as printf */
} traceLevelE;

extern traceLevelE gTraceLevel;

#define TRACE_LEVEL_SET(level) 	gTraceLevel = level;

#if defined(BCMDBG)

#define TRACE(level, args...)	\
	trace(__FILE__, __LINE__, __FUNCTION__, level, args)

#define TRACE_MAC_ADDR(level, str, mac)	\
	traceMacAddr(__FILE__, __LINE__, __FUNCTION__, level, str, mac)

#define TRACE_HEX_DUMP(level, str, len, buf)	\
	traceHexDump(__FILE__, __LINE__, __FUNCTION__, level, str, len, buf)

#else

#define TRACE(level, args...)

#define TRACE_MAC_ADDR(level, str, mac)

#define TRACE_HEX_DUMP(level, str, len, buf)

#endif	/* BCMDBG */

void trace(const char *file, int line, const char *function,
	traceLevelE level, const char *format, ...);

void traceMacAddr(const char *file, int line, const char *function,
	traceLevelE level, const char *str, struct ether_addr *mac);

void traceHexDump(const char *file, int line, const char *function,
	traceLevelE level, const char *str, int len, uint8 *buf);

/* Compatibility with wl_dbg.h */

#if !defined(BCMDRIVER)

#define WL_ERROR(args)		WL_ERROR_ args
#define WL_ERROR_(args...)	TRACE(TRACE_ERROR, args)

#define WL_TRACE(args)		WL_TRACE_ args
#define WL_TRACE_(args...)	TRACE(TRACE_VERBOSE, args)

#define WL_PRINT(args)		printf args

#define TRACE_P2PO			(TRACE_PACKET)
#define WL_P2PO(args)		WL_P2PO_ args
#define WL_P2PO_(args...)	TRACE(TRACE_P2PO, args)

#define WL_PRPKT(m, b, n)	TRACE_HEX_DUMP(TRACE_PACKET, m, n, b)

#define WL_PRUSR(m, b, n)	TRACE_HEX_DUMP(TRACE_PRINTF, m, n, b)

#define WL_PRMAC(m, mac)	TRACE_MAC_ADDR(TRACE_DEBUG, m, mac)

#if defined(BCMDBG)
#define WL_P2PO_ON()		(gTraceLevel & TRACE_PACKET)
#else
#define WL_P2PO_ON()		(0)
#endif // endif

#else

#if defined(BCMDBG)
#define WL_PRMAC(m, mac)							\
	do {											\
		char eabuf[ETHER_ADDR_STR_LEN];				\
		(void)eabuf;								\
		WL_ERROR(("%s = %s\n", m,					\
			bcm_ether_ntoa((struct ether_addr*)mac,	\
			eabuf)));								\
	} while (0);
#else
#define WL_PRMAC(m, mac)
#endif // endif

#endif /* BCMDRIVER */

#define PRINT_MAC_ADDR(str, mac)	\
	traceMacAddr(__FILE__, __LINE__, __FUNCTION__, TRACE_PRINTF, str, mac)

#define PRINT_HEX_DUMP(str, len, buf)	\
	traceHexDump(__FILE__, __LINE__, __FUNCTION__, TRACE_PRINTF, str, len, buf)

#endif /* _TRACE_H_ */
