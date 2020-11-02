/*
 * PATCH routines common hdr file
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
 * $Id: wlc_patch.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_patch_h_
#define _wlc_patch_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc.h>

#ifndef BCM_OL_DEV
#include <wlc_bsscfg.h>
#endif // endif

/* "Patch preambles" are assembly instructions corresponding to the first couple instructions
 * for each ROM function. These instructions are executed (in RAM) by manual patch functions prior
 * to branching to an offset within the patched ROM function. This avoids recursively hitting the
 * TCAM entry located at the beginning of the ROM function (in the absense of ROM function nop
 * preambles).
 */
#if defined(BCMROM_PATCH_PREAMBLE)
	#define CALLROM_ENTRY(a) a##__bcmromfn_preamble
#else
	#define CALLROM_ENTRY(a) a##__bcmromfn
#endif // endif

#ifdef WLC_PATCH_IOCTL

extern int wlc_ioctl_patchmod(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

#endif /* WLC_PATCH_IOCTL */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file. It must be
 * included after the prototypes above. The name of the included source file (WLC_PATCH_IOCTL_FILE)
 * is defined by the build environment.
 */
#if (defined(WLC_PATCH_IOCTL) || defined(WLC_PATCH_IOCTL_CHECKSUM))
	#if defined(WLC_PATCH_IOCTL_FILE)
		#include WLC_PATCH_IOCTL_FILE
	#endif
#endif /* WLC_PATCH_IOCTL || WLC_PATCH_IOCTL_CHECKSUM */

#endif /* _wlc_patch_h_ */
