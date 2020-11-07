/*
 * Encode base functions which provides encoding of basic data types
 * and provides bounds checking on the buffer to be encoded.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "trace.h"
#include "bcm_encode.h"

static int isLengthValid(bcm_encode_t *pkt, int length)
{
	assert(pkt != 0);

	if (pkt == 0 || length < 0)
		return FALSE;

	if (pkt->buf == 0)
		return FALSE;

	if (pkt->length + length > pkt->maxLength) {
		WL_ERROR(("length %d exceeds remaining buffer %d\n",
			length, pkt->maxLength - pkt->length));
		return FALSE;
	}

	return TRUE;
}

/* initialize pkt encode buffer */
int bcm_encode_init(bcm_encode_t *pkt, int maxLength, uint8 *buf)
{
	assert(pkt != 0);

	if (buf == 0)
		return FALSE;

	memset(buf, 0, maxLength);
	pkt->maxLength = maxLength;
	pkt->length = 0;
	pkt->buf = buf;

	return TRUE;
}

/* encode byte */
int bcm_encode_byte(bcm_encode_t *pkt, uint8 byte)
{
	assert(pkt != 0);

	if (!isLengthValid(pkt, 1))
		return 0;

	pkt->buf[pkt->length++] = byte;
	return 1;
}

/* encode 16-bit big endian */
int bcm_encode_be16(bcm_encode_t *pkt, uint16 value)
{
	assert(pkt != 0);

	if (!isLengthValid(pkt, 2))
		return 0;

	pkt->buf[pkt->length++] = value >> 8;
	pkt->buf[pkt->length++] = value;
	return 2;
}

/* encode 32-bit big endian */
int bcm_encode_be32(bcm_encode_t *pkt, uint32 value)
{
	assert(pkt != 0);

	if (!isLengthValid(pkt, 4))
		return 0;

	pkt->buf[pkt->length++] = value >> 24;
	pkt->buf[pkt->length++] = value >> 16;
	pkt->buf[pkt->length++] = value >> 8;
	pkt->buf[pkt->length++] = value;
	return 4;
}

/* encode 16-bit little endian */
int bcm_encode_le16(bcm_encode_t *pkt, uint16 value)
{
	assert(pkt != 0);

	if (!isLengthValid(pkt, 2))
		return 0;

	pkt->buf[pkt->length++] = value;
	pkt->buf[pkt->length++] = value >> 8;
	return 2;
}

/* encode 32-bit little endian */
int bcm_encode_le32(bcm_encode_t *pkt, uint32 value)
{
	assert(pkt != 0);

	if (!isLengthValid(pkt, 4))
		return 0;

	pkt->buf[pkt->length++] = value;
	pkt->buf[pkt->length++] = value >> 8;
	pkt->buf[pkt->length++] = value >> 16;
	pkt->buf[pkt->length++] = value >> 24;
	return 4;
}

/* encode bytes */
int bcm_encode_bytes(bcm_encode_t *pkt, int length, uint8 *bytes)
{
	assert(pkt != 0);
	assert(bytes != 0);

	if (!isLengthValid(pkt, length))
		return 0;

	memcpy(&pkt->buf[pkt->length], bytes, length);
	pkt->length += length;
	return length;
}
