#ifdef BCMWAPI_WPI
/*
 * sms4.h
 * SMS-4 block cipher
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
 * $Id: sms4.h 781105 2019-11-11 21:40:54Z $
 */

#ifndef _SMS4_H_
#define _SMS4_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif // endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

#define SMS4_BLOCK_SZ		16U
#define SMS4_WORD_SZ		sizeof(uint32)
#define SMS4_RK_WORDS		32U
#define SMS4_BLOCK_WORDS	(SMS4_BLOCK_SZ/SMS4_WORD_SZ)

#define SMS4_WPI_PN_LEN		SMS4_BLOCK_SZ
#define SMS4_WPI_HEADER_LEN	(SMS4_WPI_PN_LEN + 2)
#define SMS4_WPI_MAX_MPDU_LEN	2278U

#define SMS4_WPI_MIN_AAD_LEN		32U
#define SMS4_WPI_MAX_AAD_LEN		34U
#define SMS4_OLD_KEY_MAXVALIDTIME	60U

/* WPI IV, not to be confused with CBC-MAC and OFB IVs which are really just
 * the PN
 */
BWL_PRE_PACKED_STRUCT struct wpi_iv {
	uint8	key_idx;
	uint8	reserved;
	uint8	PN[SMS4_WPI_PN_LEN];
} BWL_POST_PACKED_STRUCT;

#define SMS4_WPI_IV_LEN	sizeof(struct wpi_iv)

void sms4_enc(uint32 *Y, uint32 *X, const uint32 *RK);

void sms4_dec(uint32 *Y, uint32 *X, uint32 *RK);

void sms4_key_exp(uint32 *MK, uint32 *RK);

int sms4_wpi_cbc_mac(const uint8 *ick,
	const uint8 *iv,
	const size_t aad_len,
	const uint8 *aad,
	uint8 *ptxt);

int sms4_ofb_crypt(const uint8 *ek,
	const uint8 *iv,
	const size_t data_len,
	uint8 *ptxt);

#define SMS4_WPI_SUCCESS		0
#define SMS4_WPI_ENCRYPT_ERROR		-1
#define SMS4_WPI_DECRYPT_ERROR		-2
#define SMS4_WPI_CBC_MAC_ERROR		-3
#define SMS4_WPI_DECRYPT_MIC_FAIL	SMS4_WPI_CBC_MAC_ERROR
#define SMS4_WPI_OFB_ERROR		-4

#define SMS4_WPI_SUBTYPE_LOW_MASK	0x0070
#define	SMS4_WPI_FC_MASK 		~(SMS4_WPI_SUBTYPE_LOW_MASK | \
					FC_RETRY | FC_PM | FC_MOREDATA)

int sms4_wpi_pkt_encrypt(const uint8 *ek,
	const uint8 *ick,
	const size_t data_len,
	uint8 *p);

int sms4_wpi_pkt_decrypt(const uint8 *ek,
	const uint8 *ick,
	const size_t data_len,
	uint8 *p);

void sxor_128bit_block(const uint8 *src1, const uint8 *src2, uint8 *dst);

#ifdef BCMSMS4_TEST
int sms4_test_enc_dec(void);
int sms4_test_cbc_mac(void);
int sms4_test_ofb_crypt(void);
int sms4_test_wpi_pkt_encrypt_decrypt_timing(int *t);
int sms4_test_wpi_pkt_encrypt(void);
int sms4_test_wpi_pkt_decrypt(void);
int sms4_test_wpi_pkt_micfail(void);
int sms4_test(int *t);
#endif /* BCMSMS4_TEST */

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _SMS4_H_ */
#endif /* BCMWAPI_WPI */
