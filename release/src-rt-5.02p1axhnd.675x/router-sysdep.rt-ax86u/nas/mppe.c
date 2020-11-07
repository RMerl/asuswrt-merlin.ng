/*
 * Microsoft Point-to-Point Encryption Protocol (MPPE)
 *
 * Copyright 2019 Broadcom
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
 * $Id: mppe.c 685074 2017-02-15 09:43:36Z $
 */

#include <typedefs.h>
#include <string.h>
#include <md5.h>

#include "mppe.h"

#define bcopy(src, dst, len) memcpy((dst), (src), (len))
#define bzero(b, len) memset((b), '\0', (len))

/* Encrypt or decrypt a MPPE message in place */
void
mppe_crypt(unsigned char salt[2],		/* 2 bytes Salt */
	   unsigned char *text,	int text_len,	/* Multiple of 16 bytes String */
	   unsigned char *key, int key_len,	/* Shared secret */
	   unsigned char vector[16],		/* 16 bytes Request Authenticator vector */
	   int encrypt)				/* Encrypt if 1 */
{
	unsigned char b[16], c[16], *p;
	MD5_CTX md5;
	int i;

	/* Initial cipher block is Request Authenticator vector */
	bcopy(vector, c, 16);
	for (p = text; &p[15] < &text[text_len]; p += 16) {
		MD5Init(&md5);
		/* Add shared secret */
		MD5Update(&md5, key, key_len);
		/* Add last cipher block */
		MD5Update(&md5, c, 16);
		/* Add salt */
		if (p == text)
			MD5Update(&md5, salt, 2);
		MD5Final(b, &md5);
		if (encrypt) {
			for (i = 0; i < 16; i++) {
				p[i] ^= b[i];
				c[i] = p[i];
			}
		} else {
			for (i = 0; i < 16; i++) {
				c[i] = p[i];
				p[i] ^= b[i];
			}
		}
	}
}
