/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6813_TRNG_H
#define _6813_TRNG_H

#define TRNG_BASE	0xFF803800

typedef struct trng {
	uint32_t spare;			/* 0x000 */
	uint32_t scratch;		/* 0x004 */
	uint32_t dummy1[2];
#define TRNG_CMD_MASK			0x0000000F
	uint32_t cmd;			/* 0x010 */
	uint32_t dummy2[3];
#define TRNG_STATUS_CMD_MASK		0x0000000F
#define TRNG_STATUS_RDY			0x00000010
#define TRNG_STATUS_ERR			0x00000020
#define TRNG_STATUS_DATA_CNT		0xFF000000
#define TRNG_STATUS_DATA_CNT_SHIFT	24
	uint32_t sts;			/* 0x020 */
	uint32_t dummy3[3];
	uint32_t pool_sts;		/* 0x030 */
	uint32_t dummy4[3];
	uint32_t dout;			/* 0x040 */
	uint32_t dummy5[3];
	uint32_t rosc_out;		/* 0x050 */
	uint32_t rosc_fifo_out;		/* 0x054 */
	uint32_t rosc_fifo_ov_cnt;	/* 0x058 */
	uint32_t dummy6;
#define TRNG_REQ_LEN_MAX_VAL		4095		/* Generate 4095 X 128bit blocks */
	uint32_t req_len;		/* 0x060 */
	uint32_t dummy7[3];
	uint32_t user_in_len;		/* 0x070 */
	uint32_t dummy8[3];
#define TRNG_CTRL_USE_RO		0x00000001
#define TRNG_CTRL_PRED_RES_ON		0x00000002
	uint32_t ctrl;			/* 0x080 */
	uint32_t reserved;		/* 0x084 */
	uint32_t selftest_ctrl;		/* 0x088 */
	uint32_t dummy9;
	uint32_t min_entr;		/* 0x090 */
	uint32_t dummy10[27];
	uint32_t entropy_0;		/* 0x100 */
	uint32_t entropy_1;		/* 0x104 */
	uint32_t entropy_2;		/* 0x108 */
	uint32_t entropy_3;		/* 0x10c */
	uint32_t entropy_4;		/* 0x110 */
	uint32_t entropy_5;		/* 0x114 */
	uint32_t entropy_6;		/* 0x118 */
	uint32_t entropy_7;		/* 0x11c */
	uint32_t nonce_0;		/* 0x120 */
	uint32_t nonce_1;		/* 0x124 */
	uint32_t nonce_2;		/* 0x128 */
	uint32_t nonce_3;		/* 0x12c */
	uint32_t dummy11[52];
	uint32_t user_in_0;		/* 0x200 */
	uint32_t user_in_1;		/* 0x204 */
	uint32_t user_in_2;		/* 0x208 */
	uint32_t user_in_3;		/* 0x20c */
	uint32_t user_in_4;		/* 0x210 */
	uint32_t user_in_5;		/* 0x214 */
	uint32_t user_in_6;		/* 0x218 */
	uint32_t user_in_7;		/* 0x21c */
	uint32_t user_in_8;		/* 0x220 */
	uint32_t user_in_9;		/* 0x224 */
	uint32_t user_in_10;		/* 0x228 */
	uint32_t user_in_11;		/* 0x22c */
	uint32_t dummy12[52];
	uint32_t isr;			/* 0x300 */
	uint32_t ier;			/* 0x304 */
	uint32_t ifr;			/* 0x308 */
	uint32_t bist_test;		/* 0x30c */
	uint32_t dummy13[16];
	uint32_t char_ctrl;		/* 0x350 */
	uint32_t dummy14[235];

#define TRNG_PERM_ALLOW_SECURE_ACCESS	0xCC
#define TRNG_PERM_ALLOW_NONSEC_ACCESS	0x33
	uint32_t perm;			/* 0x700 */
	uint32_t misc_ctrl;		/* 0x704 */
} TRNG;

#define TRNG_PERM_SHIFT 	0x0
#define TRNG_PERM_BLK_SHIFT	0x4

#define TRNG_PERM_SEC_W 	0x8
#define TRNG_PERM_SEC_R 	0x4
#define TRNG_PERM_NSEC_W 	0x2
#define TRNG_PERM_NSEC_R 	0x1

#define TRNG_PERM_BLK_SEC_W 	0x8
#define TRNG_PERM_BLK_SEC_R 	0x4
#define TRNG_PERM_BLK_NSEC_W 	0x2
#define TRNG_PERM_BLK_NSEC_R 	0x1

#define TRNG_PERM_NSEC_ENABLE   (((TRNG_PERM_BLK_NSEC_R|TRNG_PERM_BLK_NSEC_W)<<TRNG_PERM_BLK_SHIFT)|  (TRNG_PERM_SEC_W|TRNG_PERM_SEC_R)<<TRNG_PERM_SHIFT)
#define TRNG_PERM_SEC_ENABLE   (((TRNG_PERM_BLK_SEC_R|TRNG_PERM_BLK_SEC_W)<<TRNG_PERM_BLK_SHIFT)|  (TRNG_PERM_SEC_W|TRNG_PERM_SEC_R)<<TRNG_PERM_SHIFT)
#define TRNG_PERM_DISABLE_ALL	0x0   

#define TRNG ((volatile TRNG * const) TRNG_BASE)

#ifndef ENABLE_DUAL_NONSEC_RNG
#define ENABLE_DUAL_NONSEC_RNG  1
#endif

#endif
