/*
 * gcm.h
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
 * $Id: gcm.h 349803 2012-08-09 18:49:41Z $
 */

#ifndef _GCM_H_
#define _GCM_H_

/* GCM is an authenticated encryption mechanism standardized by
 * NIST -  http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf
 * based on submission http://siswg.net/docs/gcm_spec.pdf
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif // endif

#define GCM_BLOCK_SZ 16
typedef uint8 gcm_block_t[GCM_BLOCK_SZ];

#ifndef GCM_TABLE_SZ
#define GCM_TABLE_SZ 0
#endif // endif

#if GCM_TABLE_SZ
#include <bcmcrypto/gcm_tables.h>
#endif // endif

/* GCM works with block ciphers (e.g. AES) with block length of 128 bits, but
 * is independent of the cipher. We abstract the encryption functionlity
 * note: block cipher decryption function not needed  and that in and out
 * may overlap
 */
typedef void (*gcm_encr_fn_t)(void *encr_ctx, gcm_block_t in, gcm_block_t out);

enum gcm_op_type {
	GCM_ENCRYPT = 1,    /* plain txt -> cipher text, mac */
	GCM_DECRYPT,		/* cipher text -> plain text, mac */
	GCM_MAC				/* plain text -> mac */
};

typedef enum gcm_op_type gcm_op_type_t;

struct gcm_ctx {
	gcm_op_type_t op_type; 	/* selected operation */
	gcm_encr_fn_t encr_cb;	/* encryption callback */
	void *encr_cb_ctx;		/* encryption callback context */
	gcm_block_t ej0;		/* encrypted j0 - see spec */
	gcm_block_t counter;	/* gcm counter block - initial = encr(j0+1) */
	gcm_block_t hk;			/* hash key */
	gcm_block_t mac;		/* incremental mac */
	size_t		aad_len;	/* length of aad */
	size_t		data_len;	/* processed data length */

#if GCM_TABLE_SZ
	/* Support for per-key table-based optimization. */
	gcm_table_t hk_table;
#endif // endif
};

typedef struct gcm_ctx gcm_ctx_t;

void gcm_init(gcm_ctx_t *gcm_ctx, gcm_op_type_t op_type,
	gcm_encr_fn_t encr, void *encr_ctx,
	const uint8 *nonce, size_t nonce_len,
	const uint8 *aad, size_t aad_len);

/* update gcm with data. depending on the operation, data is modified
 * in place except GCM_VERIFY where the data is not changed.
 * data_len MUST be a multiple of GCM_BLOCK_SZ
 */
void gcm_update(gcm_ctx_t *gcm_ctx, uint8 *data, size_t data_len);

/* finalize gcm - data length may be 0, and need not be a multiple of
 * GCM_BLOCK_SZ
 */
void gcm_final(gcm_ctx_t *gcm_ctx, uint8 *data, size_t data_len,
	uint8 *mac, size_t mac_len);

/* gcm multiply operation - non-table version */
void gcm_mul_block(const gcm_block_t x, const gcm_block_t y, gcm_block_t out);

#endif /* _GCM_H_ */
