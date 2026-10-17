/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6858_RNG_H
#define _6858_RNG_H

#define RNG_BASE    0xff800b80

typedef struct Rng {
	uint32_t ctrl0;		/* 0x00 */
	uint32_t rngSoftReset;	/* 0x04 */
	uint32_t rbgSoftReset;	/* 0x08 */
	uint32_t totalBitCnt;	/* 0x0c */
	uint32_t totalBitCntThreshold;	/* 0x10 */
	uint32_t revId;		/* 0x14 */
	uint32_t intStatus;	/* 0x18 */
#define RNG_INT_STATUS_NIST_FAIL       (0x1<<5)
#define RNG_INT_STATUS_FIFO_FULL       (0x1<<2)
	uint32_t intEn;		/* 0x1c */
	uint32_t rngFifoData;	/* 0x20 */
	uint32_t fifoCnt;	/* 0x24 */
#define RNG_PERM_ALLOW_SECURE_ACCESS           0xCC
#define RNG_PERM_ALLOW_NONSEC_ACCESS           0x33
	uint32_t perm;		/* 0x28 */
} Rng;

#define RNG_PERM_SHIFT 		0x0
#define RNG_PERM_BLK_SHIFT	0x4

#define RNG_PERM_SEC_W 		0x8
#define RNG_PERM_SEC_R 		0x4
#define RNG_PERM_NSEC_W 	0x2
#define RNG_PERM_NSEC_R 	0x1

#define RNG_PERM_BLK_SEC_W 	0x8
#define RNG_PERM_BLK_SEC_R 	0x4
#define RNG_PERM_BLK_NSEC_W 	0x2
#define RNG_PERM_BLK_NSEC_R 	0x1

#define RNG_PERM_NSEC_ENABLE   (((RNG_PERM_BLK_NSEC_R|RNG_PERM_BLK_NSEC_W)<<RNG_PERM_BLK_SHIFT)|  (RNG_PERM_SEC_W|RNG_PERM_SEC_R)<<RNG_PERM_SHIFT)
#define RNG_PERM_SEC_ENABLE   (((RNG_PERM_BLK_SEC_R|RNG_PERM_BLK_SEC_W)<<RNG_PERM_BLK_SHIFT)|  (RNG_PERM_SEC_W|RNG_PERM_SEC_R)<<RNG_PERM_SHIFT)
#define RNG_PERM_DISABLE_ALL	0x0   

#define RNG ((volatile Rng * const) RNG_BASE)

#endif
