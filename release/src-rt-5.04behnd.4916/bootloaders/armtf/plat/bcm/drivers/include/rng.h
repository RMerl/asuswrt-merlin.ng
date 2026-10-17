/*
 * <:copyright-BRCM:2023:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2023 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#ifndef __RNG_H__
#define __RNG_H__

#include <string.h>
#include <platform_def.h>

typedef struct Rng {
	uint32_t ctrl0;			/* 0x00 */
	uint32_t rngSoftReset;		/* 0x04 */
	uint32_t rbgSoftReset;		/* 0x08 */
	uint32_t totalBitCnt;		/* 0x0c */
	uint32_t totalBitCntThreshold;	/* 0x10 */
	uint32_t revId;			/* 0x14 */
	uint32_t intStatus;		/* 0x18 */
#define RNG_INT_STATUS_LOCK_FAIL	0x80000000
#define RNG_INT_STATUS_NIST_FAIL	(0x1<<5)
#define RNG_INT_STATUS_FIFO_FULL	(0x1<<2)
	uint32_t intEn;			/* 0x1c */
	uint32_t rngFifoData;		/* 0x20 */
	uint32_t fifoCnt;		/* 0x24 */
#define RNG_PERM_ALLOW_SECURE_ACCESS	0xCC
#define RNG_PERM_ALLOW_NONSEC_ACCESS	0x33
	uint32_t perm;			/* 0x28 */
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
#define RNG_PERM_SEC_ENABLE    (((RNG_PERM_BLK_SEC_R|RNG_PERM_BLK_SEC_W)<<RNG_PERM_BLK_SHIFT)|  (RNG_PERM_SEC_W|RNG_PERM_SEC_R)<<RNG_PERM_SHIFT)
#define RNG_PERM_DISABLE_ALL	0x0

#define RNG ((volatile Rng * const) (RNG_BASE + RNG_OFFSET))

#endif /* __RNG_H__ */
