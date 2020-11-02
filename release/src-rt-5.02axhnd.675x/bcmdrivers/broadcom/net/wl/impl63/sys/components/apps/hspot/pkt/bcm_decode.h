/*
 * Decode base functions which provides decoding of basic data types
 * and provides bounds checking on the buffer to be decoded.
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

#ifndef _BCM_DECODE_H_
#define _BCM_DECODE_H_

#include "typedefs.h"

typedef struct
{
	int maxLength;
	int offset;
	uint8 *buf;
} bcm_decode_t;

/* get decode buffer length */
#define bcm_decode_buf_length(pkt)	\
	((pkt)->maxLength)

/* get decode buffer */
#define bcm_decode_buf(pkt)		\
	((pkt)->buf)

/* get decode offset */
#define bcm_decode_offset(pkt)		\
	((pkt)->offset)

/* set decode offset */
#define bcm_decode_offset_set(pkt, value)	\
	((pkt)->offset = (value))

/* get decode remaining count */
#define bcm_decode_remaining(pkt)	\
	((pkt)->maxLength > (pkt)->offset ? (pkt)->maxLength - (pkt)->offset : 0)

/* get decode current pointer */
#define bcm_decode_current_ptr(pkt)	\
	(&(pkt)->buf[(pkt)->offset])

/* is zero length decode */
#define bcm_decode_is_zero_length(pkt)	\
	(bcm_decode_buf(pkt) != 0 && bcm_decode_buf_length(pkt) == 0)

/* is packet valid to decode */
#define bcm_decode_is_pkt_valid(pkt) 	\
	(bcm_decode_buf(pkt) != 0)

/* initialize pkt decode with decode buffer */
int bcm_decode_init(bcm_decode_t *pkt, int maxLength, uint8 *data);

/* decode byte */
int bcm_decode_byte(bcm_decode_t *pkt, uint8 *byte);

/* decode 16-bit big endian */
int bcm_decode_be16(bcm_decode_t *pkt, uint16 *value);

/* decode 32-bit big endian */
int bcm_decode_be32(bcm_decode_t *pkt, uint32 *value);

/* decode 16-bit little endian */
int bcm_decode_le16(bcm_decode_t *pkt, uint16 *value);

/* decode 32-bit little endian */
int bcm_decode_le32(bcm_decode_t *pkt, uint32 *value);

/* decode bytes */
int bcm_decode_bytes(bcm_decode_t *pkt, int length, uint8 *bytes);

#endif /* _BCM_DECODE_H_ */
