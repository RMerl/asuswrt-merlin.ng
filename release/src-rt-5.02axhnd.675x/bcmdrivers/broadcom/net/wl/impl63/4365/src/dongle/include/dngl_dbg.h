/*
 * DONGLE debug macros
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
 * $Id: dngl_dbg.h 464743 2014-03-25 21:04:32Z $
 */

#ifndef _dngl_dbg_h_
#define _dngl_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define DNGL_ERROR	0x00000001	/* enables err() printf statements */
#define DNGL_TRACE	0x00000002	/* enables trace() printf statements */
#define DNGL_PRPKT	0x00000008	/* enables hex() printf statements */
#define DNGL_INFORM	0x00000010	/* enables dbg() printf statements */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
extern int dngl_msglevel;

#define dngl_dbg(bit, fmt, args...) do { \
	if (dngl_msglevel & (bit)) \
		printf("%s: " fmt "\n", __FUNCTION__ , ## args); \
} while (0)

#define dngl_hex(msg, buf, len) do { \
	if (dngl_msglevel & DNGL_PRPKT) \
		prhex(msg, buf, len); \
} while (0)

/* Debug functions */
#define err(fmt, args...) dngl_dbg(DNGL_ERROR, fmt , ## args)
#else
#define err(fmt, args...)
#endif	/* BCMDBG || BCMDBG_ERR */

#ifdef BCMDBG
#define trace(fmt, args...) dngl_dbg(DNGL_TRACE, fmt , ## args)
#define dbg(fmt, args...) dngl_dbg(DNGL_INFORM, fmt , ## args)
#define hex(msg, buf, len) dngl_hex(msg, buf, len)
#else
#define trace(fmt, args...)
#define dbg(fmt, args...)
#define hex(msg, buf, len)
#endif // endif

#endif /* _dngl_dbg_h_ */
