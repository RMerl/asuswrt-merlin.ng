/*
 *   wep.c - WEP functions
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
 * $Id: wep.c 451682 2014-01-27 20:30:17Z $
 */

/* XXX: Define bcm_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <bcm_cfg.h>
#include <typedefs.h>

/* include wl driver config file if this file is compiled for driver */
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <string.h>
#endif /* BCMDRIVER */

#include <bcmutils.h>
#include <bcmcrypto/rc4.h>
#include <bcmcrypto/wep.h>
#include <proto/802.11.h>

/* WEP-encrypt a buffer */
/* assumes a contiguous buffer, with IV prepended, and with enough space at
 * the end for the ICV
 */
/* FIXME: need incremental encrypt function, to handle fragmentation and
 * non-contiguous buffers
 */
void
wep_encrypt(uint buf_len, uint8 *buf, uint sec_len, uint8 *sec_data)
{
	uint8 key_data[16];
	uint32 ICV;
	rc4_ks_t ks;
	uint8 *body = buf + DOT11_IV_LEN;
	uint body_len = buf_len - (DOT11_IV_LEN + DOT11_ICV_LEN);
	uint8 *picv = body + body_len;

	memcpy(key_data, buf, 3);
	memcpy(&key_data[3], sec_data, sec_len);

	prepare_key(key_data, sec_len + 3, &ks);

	/* append ICV */
	ICV = ~hndcrc32(body, body_len, CRC32_INIT_VALUE);
	picv[0] = ICV & 0xff;
	picv[1] = (ICV >> 8) & 0xff;
	picv[2] = (ICV >> 16) & 0xff;
	picv[3] = (ICV >> 24) & 0xff;

	rc4(body, body_len + DOT11_ICV_LEN, &ks);
}

/* WEP-decrypt
 * Assumes a contigous buffer, with IV prepended.
 * Returns TRUE if ICV check passes, FALSE otherwise
 *
 */
bool
wep_decrypt(uint buf_len, uint8 *buf, uint sec_len, uint8 *sec_data)
{
	uint8 key_data[16];
	rc4_ks_t ks;

	memcpy(key_data, buf, 3);
	memcpy(&key_data[3], sec_data, sec_len);

	prepare_key(key_data, sec_len + 3, &ks);

	rc4(buf + DOT11_IV_LEN, buf_len - DOT11_IV_LEN, &ks);

	return (hndcrc32(buf + DOT11_IV_LEN, buf_len - DOT11_IV_LEN, CRC32_INIT_VALUE) ==
		CRC32_GOOD_VALUE);
}
