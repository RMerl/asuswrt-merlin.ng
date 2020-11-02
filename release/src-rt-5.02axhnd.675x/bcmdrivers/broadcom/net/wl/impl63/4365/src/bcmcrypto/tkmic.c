/*
 *   tkmic.c - TKIP Message Integrity Check (MIC) functions
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
 * $Id: tkmic.c 451682 2014-01-27 20:30:17Z $
 */

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmcrypto/tkmic.h>

/*
 * "Michael" Messge Integrity Check (MIC) algorithm
 */
static INLINE void
tkip_micblock(uint32 *left, uint32 *right)
{
	uint32 l = *left;
	uint32 r = *right;

	/*
	 * Per Henry, we replaced the ROTL with ROTR
	 */
	r ^= ROTR32(l, 15);
	l += r; /* mod 2^32 */
	r ^= XSWAP32(l);
	l += r; /* mod 2^32 */
	r ^= ROTR32(l, 29);
	l += r; /* mod 2^32 */
	r ^= ROTR32(l, 2);
	l += r; /* mod 2^32 */

	*left = l;
	*right = r;
}

/* compute mic across message */
/* buffer must already have terminator and padding appended */
/* buffer length (n) specified in bytes */
void
tkip_mic(uint32 k0, uint32 k1, int n, uint8 *m, uint32 *left, uint32 *right)
{
	uint32 l = k0;
	uint32 r = k1;

	if (((uintptr)m & 3) == 0) {
		for (; n > 0; n -= 4) {
			l ^= ltoh32(*(uint *)m);
			m += 4;
			tkip_micblock(&l, &r);
		}
	} else {
		for (; n > 0; n -= 4) {
			l ^= ltoh32_ua(m);
			m += 4;
			tkip_micblock(&l, &r);
		}
	}
	*left = l;
	*right = r;
}

/* append the MIC terminator to the data buffer */
/* terminator is 0x5a followed by 4-7 bytes of 0 */
/* param 'o' is the current frag's offset in the frame */
/* returns length of message plus terminator in bytes */
int
tkip_mic_eom(uint8 *m, uint n, uint o)
{
	uint8 *mend = m + n;
	uint t = n + o;
	mend[0] = 0x5a;
	mend[1] = 0;
	mend[2] = 0;
	mend[3] = 0;
	mend[4] = 0;
	mend += 5;
	o += n + 5;
	while (o++%4) {
		*mend++ = 0;
	}
	return (n+o-1-t);
}
