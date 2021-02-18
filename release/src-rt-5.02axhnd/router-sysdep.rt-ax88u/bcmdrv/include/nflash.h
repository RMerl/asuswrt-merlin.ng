/*
 * Broadcom chipcommon NAND flash interface
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: nflash.h 667654 2016-10-28 02:10:45Z $
 */

#ifndef _nflash_h_
#define _nflash_h_

/* nand_cmd_start commands */
#define NCMD_NULL			0
#define NCMD_PAGE_RD			1
#define NCMD_SPARE_RD			2
#define NCMD_STATUS_RD			3
#define NCMD_PAGE_PROG			4
#define NCMD_SPARE_PROG			5
#define NCMD_COPY_BACK			6
#define NCMD_ID_RD			7
#define NCMD_BLOCK_ERASE		8
#define NCMD_FLASH_RESET		9
#define NCMD_LOCK			0xa
#define NCMD_LOCK_DOWN			0xb
#define NCMD_UNLOCK			0xc
#define NCMD_LOCK_STATUS		0xd

/* nand_acc_control */
#define	NAC_RD_ECC_EN			0x80000000
#define	NAC_WR_ECC_EN			0x40000000
#define	NAC_RD_ECC_BLK0_EN		0x20000000
#define	NAC_FAST_PGM_RDIN		0x10000000
#define	NAC_RD_ERASED_ECC_EN		0x08000000
#define	NAC_PARTIAL_PAGE_EN		0x04000000
#define	NAC_PAGE_HIT_EN			0x01000000
#define	NAC_ECC_LEVEL0_MASK		0x00f00000
#define	NAC_ECC_LEVEL0_SHIFT		20
#define	NAC_ECC_LEVEL_MASK		0x000f0000
#define	NAC_ECC_LEVEL_SHIFT		16
#define	NAC_SPARE_SIZE0			0x00003f00
#define	NAC_SPARE_SIZE			0x0000003f

/* nand_config */
#define	NCF_CONFIG_LOCK			0x80000000
#define	NCF_BLOCK_SIZE_MASK		0x70000000
#define	NCF_BLOCK_SIZE_SHIFT		28
#define	NCF_DEVICE_SIZE_MASK		0x0f000000
#define	NCF_DEVICE_SIZE_SHIFT		24
#define	NCF_DEVICE_WIDTH		0x00800000
#define	NCF_PAGE_SIZE_MASK		0x00300000
#define	NCF_PAGE_SIZE_SHIFT		20
#define	NCF_FULL_ADDR_BYTES_MASK	0x00070000
#define	NCF_FULL_ADDR_BYTES_SHIFT	16
#define	NCF_COL_ADDR_BYTES_MASK		0x00007000
#define	NCF_COL_ADDR_BYTES_SHIFT	12
#define	NCF_BLK_ADDR_BYTES_MASK		0x00000700
#define	NCF_BLK_ADDR_BYTES_SHIFT	8

/* nand_intfc_status */
#define	NIST_CTRL_READY			0x80000000
#define	NIST_FLASH_READY		0x40000000
#define	NIST_CACHE_VALID		0x20000000
#define	NIST_SPARE_VALID		0x10000000
#define	NIST_ERASED			0x08000000
#define	NIST_STATUS			0x000000ff

#ifndef _LANGUAGE_ASSEMBLY
#include <typedefs.h>
#include <sbchipc.h>
#include <hndnand.h>
#endif /* _LANGUAGE_ASSEMBLY */

#endif /* _nflash_h_ */
