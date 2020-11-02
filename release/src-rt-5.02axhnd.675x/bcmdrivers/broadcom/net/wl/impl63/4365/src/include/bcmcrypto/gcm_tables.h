/*
 * gcm_tables.h
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
 * $Id: gcm_tables.h 349803 2012-08-09 18:49:41Z $
 */

#ifndef _GCM_TABLES_H_
#define _GCM_TABLES_H_

/* Support for various per-key tables.
 *  Shoup's 4 bit tables         - 256 bytes/key - Supported
 *  Shoup's 8 bit tables         - 4096 bytes/key - Supported
 *  Wei Dai (crypto-optimization)- 2048 bytes/key - NOT Supported
 *  Simple 4 bit tables          - 8192 bytes/key - NOT Supported
 *  Simple 8 bit tables          - 65536 bytes/key - NOT Supported
 *
 * Rationale: 4 bit tables are reasonable in terms of memory. 8 bit
 * tables would provide performance comparable to CCM. Wei Dai
 * claims performance equivalent to 8k bytes/key - i.e simple 4 bit and
 * within a factor of two compared to 64k bytes/key - i.e. simple 8 bit
 * Based on comments in openssl 1.0.1c implementation, simple tables
 * seem to be vulnerable to timing attacks...
 */

#if GCM_TABLE_SZ != 256 && GCM_TABLE_SZ != 4096
#error "Unsupported table size"
#endif // endif

#define GCM_TABLE_NUM_ENTRIES (GCM_TABLE_SZ / sizeof(gcm_block_t))

struct gcm_table {
	gcm_block_t M0[GCM_TABLE_NUM_ENTRIES];
};

typedef struct gcm_table gcm_table_t;

void gcm_init_table(gcm_table_t *t, gcm_block_t hk);
void gcm_mul_with_table(const gcm_table_t *t, const gcm_block_t y,
	gcm_block_t out);

#endif /* _GCM_TABLES_H_ */
