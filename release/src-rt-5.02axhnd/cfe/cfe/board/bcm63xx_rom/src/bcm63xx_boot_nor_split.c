/*  *********************************************************************
 *
 <:copyright-BRCM:2017:proprietary:standard

 Copyright (c) 2017 Broadcom
 All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
 constitutes the valuable trade secrets of Broadcom, and you shall use
 all reasonable efforts to protect the confidentiality thereof, and to
 use this information only in connection with your use of Broadcom
 integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 LIMITED REMEDY.
 :>
 ********************************************************************* */
#include "rom_main.h"
#include "btrm_if.h"
#include "bcm_btrm_gen3_common.h"
#include "bcm_otp.h"
#include "flash_api.h"

#define NOR_MAPPED_SZ		0x100000 /* direct-mapped */
#define RESERVED_SZ		0x20000  /* for CFEROM (variable) + metadata (12K) */
#define UBOOT_BASE		(BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI + RESERVED_SZ)
#define UBOOT_LZ_HDR_SZ		20
#define UBOOT_CMP_HDR_SZ	4
#define UBOOT_LZ_SZ_OFF		8
#define UBOOT_APPEND_BYTE_LEN	12
#define UBOOT_DECR_ADDR		(DRAM_BASE + 0x3000000)
#define UBOOT_COPY_ADDR		(DRAM_BASE + 0x2000000)
#define UBOOT_TEXT_BASE		(DRAM_BASE + 0x1000000)
#define SECTOR_SIZE		0x1000 /* 4K sectors */
#define UBOOT_ACTIVE_IMG_OFFSET	(UBOOT_BASE - 3 * SECTOR_SIZE)
#define UBOOT_IMGx_DATA_OFFSET	(UBOOT_BASE - 2 * SECTOR_SIZE)

static uint32_t get_uboot_sz(uintptr_t base)
{
	return *((uint32_t *)(base + UBOOT_LZ_SZ_OFF));
}

static uint8_t get_active_image(void)
{
	return (*(uint32_t *)(UBOOT_ACTIVE_IMG_OFFSET) == 1);
}

static uint32_t get_active_image_offset(uint8_t active)
{
	return active * (NOR_MAPPED_SZ - RESERVED_SZ) / 2 + UBOOT_BASE;
}
static bool boot_is_secure(void)
{
	return bcm_otp_is_boot_secure() || bcm_otp_is_boot_mfg_secure();
}

static uint32_t get_image_sz(uint8_t active)
{
	return *(uint32_t *)((uintptr_t)UBOOT_IMGx_DATA_OFFSET + active * SECTOR_SIZE);
}

static uint8_t *verify_signature(uintptr_t base, uint32_t sz)
{
	Booter1Args sec_args;
	memcpy((void *)&sec_args, (void *)BTRM_INT_MEM_CREDENTIALS_ADDR, sizeof(Booter1Args));
	return (uint8_t *) authenticate((uint8_t *) UBOOT_COPY_ADDR, sz, sec_args.authArgs.manu);
}

static void img_decrypt(uintptr_t dst, uint8_t *img, uint32_t sz)
{
	Booter1Args sec_args;
	memcpy((void *)&sec_args, (void *)BTRM_INT_MEM_CREDENTIALS_ADDR, sizeof(Booter1Args));
	unsigned char origIv[CIPHER_IV_LEN];
	memcpy((void *)origIv, (void *) sec_args.encrArgs.biv, CIPHER_IV_LEN);
	decryptWithEk((unsigned char *) dst, img, sec_args.encrArgs.bek,
			(uint32_t)(sz-SEC_S_SIGNATURE), origIv);
	ek_iv_cleanup(&sec_args.encrArgs);
	memset((void *)origIv, 0, CIPHER_IV_LEN);
}

#define CODE_START_BOOT_NOR	0x53424E52	/* SBNR */
#define CODE_SECURE_BOOT	0x53454355	/* SECU */
#define CODE_UNSECURE		0x554E5345	/* UNSE */
#define CODE_NEW_WAY		0x4E455757	/* NEWW */
#define CODE_AUTH_DONE		0x4155544F	/* AUTO */
#define CODE_DECRYPT_DONE	0x4352594F	/* CRYO */
#define CODE_DECOMPRESS_START	0x4C5A5354	/* LZST */
#define CODE_DECOMPRESS_DONE	0x4C5A4F4B	/* LZOK */

void bootNor(void)
{
	uint8_t active = get_active_image();
	uint32_t sz = get_image_sz(active);
	board_setleds(CODE_START_BOOT_NOR);
	uintptr_t current_location;
	unsigned char *pucSrc, *pucDst;

	uintptr_t active_img_offset = get_active_image_offset(active);
	board_setleds(CODE_NEW_WAY);
	/* Copy the entire secondary bootloader image to DDR to make it simpler */
	memcpy((void *) UBOOT_COPY_ADDR,
			(void *) active_img_offset, sz);
	current_location = UBOOT_COPY_ADDR;

	if (boot_is_secure()) {
		board_setleds(CODE_SECURE_BOOT);
		uint8_t *authenticated_img = verify_signature(UBOOT_COPY_ADDR, sz);
		board_setleds(CODE_AUTH_DONE);
		img_decrypt(UBOOT_DECR_ADDR, authenticated_img, sz);
		board_setleds(CODE_DECRYPT_DONE);
		current_location = UBOOT_DECR_ADDR;
		pucSrc = (unsigned char *) (current_location + UBOOT_APPEND_BYTE_LEN);
	} else
		pucSrc = (unsigned char *) (current_location + UBOOT_LZ_HDR_SZ);

	uint32_t dataLen = get_uboot_sz(current_location);
	pucDst = (unsigned char *) UBOOT_TEXT_BASE;
	board_setleds(CODE_DECOMPRESS_START);
	decompressLZMA((unsigned char *) pucSrc, dataLen, pucDst, 23*1024*1024);
	board_setleds(CODE_DECOMPRESS_DONE);
	cfe_launch((unsigned long) pucDst);
}
