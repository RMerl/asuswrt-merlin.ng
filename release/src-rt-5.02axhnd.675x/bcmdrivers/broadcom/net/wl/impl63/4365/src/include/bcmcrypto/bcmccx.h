#ifdef BCMCCX

/*
 *   bcmccx.h - Prototypes for BCM CCX utility functions
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
 * $Id: bcmccx.h 451682 2014-01-27 20:30:17Z $
 */

#ifndef _BCM_CCX_H_
#define _BCM_CCX_H_

#include <proto/eap.h>

#define CCX_PW_HASH_LEN		16 /* CCX Password Hash length */
#define CCX_SESSION_KEY_LEN	16 /* CCX session Key lenght */

#define CKIP_LLC_SNAP_LEN	8 /* SKIP LLC SNAP header length */

#define	BCM_CKIP_RXSEQ_WIN	32	/* rx sequence number window buffer size */

/* Pick a target position in the RXSEQ window for the latest sequence number
 * when the window needs to be moved. Below is 3/4 of the way through the window.
 */
#define BCM_CKIP_RXSEQ_WIN_TARGET	(BCM_CKIP_RXSEQ_WIN - BCM_CKIP_RXSEQ_WIN/4 - 1)

/* bcm_ckip_rxseq_check() uses the 32 bit sequence number shifed down 1 bit */
#define BCM_CKIP_RXSEQ_MASK		0x7fffffff /* Mask for CKIP RxSEQ */

/* bcm_ckip_rxseq_check() uses a uint32 for the BCM_CKIP_RXSEQ_WIN bitmap,
 * so make sure the window fits in the bitmap
 */
#if (BCM_CKIP_RXSEQ_WIN > 32)
#error "BCM_CKIP_RXSEQ_WIN does not fit in 32 bits"
#endif /* (BCM_CKIP_RXSEQ_WIN > 32) */

/* Get the MD4 hash and hash-hash of a password. */
extern void bcm_ccx_hashpwd(uint8 *pwd, size_t pwdlen,
                                      uint8 hash[CCX_PW_HASH_LEN],
                                      uint8 hashhash[CCX_PW_HASH_LEN]);

/* Apply MD5 to get the CCX session key */
extern void bcm_ccx_session_key(uint8 *inbuf, size_t in_len,
                                          uint8 outbuf[CCX_SESSION_KEY_LEN]);

/* Derive LEAP response from LEAP challenge and password hash */
extern void bcm_ccx_leap_response(uint8 pwhash[CCX_PW_HASH_LEN],
                                            uint8 challenge[LEAP_CHALLENGE_LEN],
                                            uint8 response[LEAP_RESPONSE_LEN]);

/* allow out-of-order SEQ due to QoS, seq number increase/reset per key */
extern bool bcm_ckip_rxseq_check(uint32 seq_odd, uint32 *rxseq_base,
                                           uint32 *rxseq_bitmap);

#endif /* _BCM_CCX_H_ */

#endif /* BCMCCX */
