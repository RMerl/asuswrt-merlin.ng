/*
 * Utilites for XDR encode and decode of data
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
 * $Id: bcm_xdr.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _BCM_XDR_H
#define _BCM_XDR_H

#define XDR_PACK_OPAQUE_VAR_SZ(len) (sizeof(uint32) + ROUNDUP(len, sizeof(uint32)))

/*
 * bcm_xdr_buf_t
 * Structure used for bookkeeping of a buffer being packed or unpacked.
 * Keeps a current read/write pointer and size as well as
 * the original buffer pointer and size.
 *
 */
typedef struct {
	uint8 *buf;	/* pointer to current position in origbuf */
	uint size;	/* current (residual) size in bytes */
	uint8 *origbuf;	/* unmodified pointer to orignal buffer */
	uint origsize;	/* unmodified orignal buffer size in bytes */
} bcm_xdr_buf_t;

void bcm_xdr_buf_init(bcm_xdr_buf_t *b, void *buf, size_t len);

int bcm_xdr_pack_uint32(bcm_xdr_buf_t *b, uint32 val);
int bcm_xdr_unpack_uint32(bcm_xdr_buf_t *b, uint32 *pval);
int bcm_xdr_pack_int32(bcm_xdr_buf_t *b, int32 val);
int bcm_xdr_unpack_int32(bcm_xdr_buf_t *b, int32 *pval);
int bcm_xdr_pack_int8(bcm_xdr_buf_t *b, int8 val);
int bcm_xdr_unpack_int8(bcm_xdr_buf_t *b, int8 *pval);
int bcm_xdr_pack_opaque(bcm_xdr_buf_t *b, uint len, const void *data);
int bcm_xdr_unpack_opaque(bcm_xdr_buf_t *b, uint len, void **pdata);
int bcm_xdr_unpack_opaque_cpy(bcm_xdr_buf_t *b, uint len, void *data);
int bcm_xdr_pack_opaque_varlen(bcm_xdr_buf_t *b, uint len, const void *data);
int bcm_xdr_unpack_opaque_varlen(bcm_xdr_buf_t *b, uint *plen, void **pdata);
int bcm_xdr_pack_string(bcm_xdr_buf_t *b, char *str);
int bcm_xdr_unpack_string(bcm_xdr_buf_t *b, uint *plen, char **pstr);

int bcm_xdr_pack_uint8_vec(bcm_xdr_buf_t *, uint8 *vec, uint32 elems);
int bcm_xdr_unpack_uint8_vec(bcm_xdr_buf_t *, uint8 *vec, uint32 elems);
int bcm_xdr_pack_uint16_vec(bcm_xdr_buf_t *b, uint len, void *vec);
int bcm_xdr_unpack_uint16_vec(bcm_xdr_buf_t *b, uint len, void *vec);
int bcm_xdr_pack_uint32_vec(bcm_xdr_buf_t *b, uint len, void *vec);
int bcm_xdr_unpack_uint32_vec(bcm_xdr_buf_t *b, uint len, void *vec);

int bcm_xdr_pack_opaque_raw(bcm_xdr_buf_t *b, uint len, void *data);
int bcm_xdr_pack_opaque_pad(bcm_xdr_buf_t *b);

int bcm_xdr_opaque_resrv(bcm_xdr_buf_t *b, uint len, void **pdata);
int bcm_xdr_opaque_resrv_varlen(bcm_xdr_buf_t *b, uint len, void **pdata);
#endif /* _BCM_XDR_H */
