/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 OCOTP Register Definitions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __MX28_REGS_OCOTP_H__
#define __MX28_REGS_OCOTP_H__

#include <asm/mach-imx/regs-common.h>

#ifndef	__ASSEMBLY__
struct mxs_ocotp_regs {
	mxs_reg_32(hw_ocotp_ctrl)	/* 0x0 */
	mxs_reg_32(hw_ocotp_data)	/* 0x10 */
	mxs_reg_32(hw_ocotp_cust0)	/* 0x20 */
	mxs_reg_32(hw_ocotp_cust1)	/* 0x30 */
	mxs_reg_32(hw_ocotp_cust2)	/* 0x40 */
	mxs_reg_32(hw_ocotp_cust3)	/* 0x50 */
	mxs_reg_32(hw_ocotp_crypto0)	/* 0x60 */
	mxs_reg_32(hw_ocotp_crypto1)	/* 0x70 */
	mxs_reg_32(hw_ocotp_crypto2)	/* 0x80 */
	mxs_reg_32(hw_ocotp_crypto3)	/* 0x90 */
	mxs_reg_32(hw_ocotp_hwcap0)	/* 0xa0 */
	mxs_reg_32(hw_ocotp_hwcap1)	/* 0xb0 */
	mxs_reg_32(hw_ocotp_hwcap2)	/* 0xc0 */
	mxs_reg_32(hw_ocotp_hwcap3)	/* 0xd0 */
	mxs_reg_32(hw_ocotp_hwcap4)	/* 0xe0 */
	mxs_reg_32(hw_ocotp_hwcap5)	/* 0xf0 */
	mxs_reg_32(hw_ocotp_swcap)	/* 0x100 */
	mxs_reg_32(hw_ocotp_custcap)	/* 0x110 */
	mxs_reg_32(hw_ocotp_lock)	/* 0x120 */
	mxs_reg_32(hw_ocotp_ops0)	/* 0x130 */
	mxs_reg_32(hw_ocotp_ops1)	/* 0x140 */
	mxs_reg_32(hw_ocotp_ops2)	/* 0x150 */
	mxs_reg_32(hw_ocotp_ops3)	/* 0x160 */
	mxs_reg_32(hw_ocotp_un0)	/* 0x170 */
	mxs_reg_32(hw_ocotp_un1)	/* 0x180 */
	mxs_reg_32(hw_ocotp_un2)	/* 0x190 */
	mxs_reg_32(hw_ocotp_rom0)	/* 0x1a0 */
	mxs_reg_32(hw_ocotp_rom1)	/* 0x1b0 */
	mxs_reg_32(hw_ocotp_rom2)	/* 0x1c0 */
	mxs_reg_32(hw_ocotp_rom3)	/* 0x1d0 */
	mxs_reg_32(hw_ocotp_rom4)	/* 0x1e0 */
	mxs_reg_32(hw_ocotp_rom5)	/* 0x1f0 */
	mxs_reg_32(hw_ocotp_rom6)	/* 0x200 */
	mxs_reg_32(hw_ocotp_rom7)	/* 0x210 */
	mxs_reg_32(hw_ocotp_srk0)	/* 0x220 */
	mxs_reg_32(hw_ocotp_srk1)	/* 0x230 */
	mxs_reg_32(hw_ocotp_srk2)	/* 0x240 */
	mxs_reg_32(hw_ocotp_srk3)	/* 0x250 */
	mxs_reg_32(hw_ocotp_srk4)	/* 0x260 */
	mxs_reg_32(hw_ocotp_srk5)	/* 0x270 */
	mxs_reg_32(hw_ocotp_srk6)	/* 0x280 */
	mxs_reg_32(hw_ocotp_srk7)	/* 0x290 */
	mxs_reg_32(hw_ocotp_version)	/* 0x2a0 */
};
#endif

#define	OCOTP_CTRL_WR_UNLOCK_MASK		(0xffff << 16)
#define	OCOTP_CTRL_WR_UNLOCK_OFFSET		16
#define	OCOTP_CTRL_WR_UNLOCK_KEY		(0x3e77 << 16)
#define	OCOTP_CTRL_RELOAD_SHADOWS		(1 << 13)
#define	OCOTP_CTRL_RD_BANK_OPEN			(1 << 12)
#define	OCOTP_CTRL_ERROR			(1 << 9)
#define	OCOTP_CTRL_BUSY				(1 << 8)
#define	OCOTP_CTRL_ADDR_MASK			0x3f
#define	OCOTP_CTRL_ADDR_OFFSET			0

#define	OCOTP_DATA_DATA_MASK			0xffffffff
#define	OCOTP_DATA_DATA_OFFSET			0

#define	OCOTP_CUST_BITS_MASK			0xffffffff
#define	OCOTP_CUST_BITS_OFFSET			0

#define	OCOTP_CRYPTO_BITS_MASK			0xffffffff
#define	OCOTP_CRYPTO_BITS_OFFSET		0

#define	OCOTP_HWCAP_BITS_MASK			0xffffffff
#define	OCOTP_HWCAP_BITS_OFFSET			0

#define	OCOTP_SWCAP_BITS_MASK			0xffffffff
#define	OCOTP_SWCAP_BITS_OFFSET			0

#define	OCOTP_CUSTCAP_RTC_XTAL_32768_PRESENT	(1 << 2)
#define	OCOTP_CUSTCAP_RTC_XTAL_32000_PRESENT	(1 << 1)

#define	OCOTP_LOCK_ROM7				(1 << 31)
#define	OCOTP_LOCK_ROM6				(1 << 30)
#define	OCOTP_LOCK_ROM5				(1 << 29)
#define	OCOTP_LOCK_ROM4				(1 << 28)
#define	OCOTP_LOCK_ROM3				(1 << 27)
#define	OCOTP_LOCK_ROM2				(1 << 26)
#define	OCOTP_LOCK_ROM1				(1 << 25)
#define	OCOTP_LOCK_ROM0				(1 << 24)
#define	OCOTP_LOCK_HWSW_SHADOW_ALT		(1 << 23)
#define	OCOTP_LOCK_CRYPTODCP_ALT		(1 << 22)
#define	OCOTP_LOCK_CRYPTOKEY_ALT		(1 << 21)
#define	OCOTP_LOCK_PIN				(1 << 20)
#define	OCOTP_LOCK_OPS				(1 << 19)
#define	OCOTP_LOCK_UN2				(1 << 18)
#define	OCOTP_LOCK_UN1				(1 << 17)
#define	OCOTP_LOCK_UN0				(1 << 16)
#define	OCOTP_LOCK_SRK				(1 << 15)
#define	OCOTP_LOCK_UNALLOCATED_MASK		(0x7 << 12)
#define	OCOTP_LOCK_UNALLOCATED_OFFSET		12
#define	OCOTP_LOCK_SRK_SHADOW			(1 << 11)
#define	OCOTP_LOCK_ROM_SHADOW			(1 << 10)
#define	OCOTP_LOCK_CUSTCAP			(1 << 9)
#define	OCOTP_LOCK_HWSW				(1 << 8)
#define	OCOTP_LOCK_CUSTCAP_SHADOW		(1 << 7)
#define	OCOTP_LOCK_HWSW_SHADOW			(1 << 6)
#define	OCOTP_LOCK_CRYPTODCP			(1 << 5)
#define	OCOTP_LOCK_CRYPTOKEY			(1 << 4)
#define	OCOTP_LOCK_CUST3			(1 << 3)
#define	OCOTP_LOCK_CUST2			(1 << 2)
#define	OCOTP_LOCK_CUST1			(1 << 1)
#define	OCOTP_LOCK_CUST0			(1 << 0)

#define	OCOTP_OPS_BITS_MASK			0xffffffff
#define	OCOTP_OPS_BITS_OFFSET			0

#define	OCOTP_UN_BITS_MASK			0xffffffff
#define	OCOTP_UN_BITS_OFFSET			0

#define	OCOTP_ROM_BOOT_MODE_MASK		(0xff << 24)
#define	OCOTP_ROM_BOOT_MODE_OFFSET		24
#define	OCOTP_ROM_SD_MMC_MODE_MASK		(0x3 << 22)
#define	OCOTP_ROM_SD_MMC_MODE_OFFSET		22
#define	OCOTP_ROM_SD_POWER_GATE_GPIO_MASK	(0x3 << 20)
#define	OCOTP_ROM_SD_POWER_GATE_GPIO_OFFSET	20
#define	OCOTP_ROM_SD_POWER_UP_DELAY_MASK	(0x3f << 14)
#define	OCOTP_ROM_SD_POWER_UP_DELAY_OFFSET	14
#define	OCOTP_ROM_SD_BUS_WIDTH_MASK		(0x3 << 12)
#define	OCOTP_ROM_SD_BUS_WIDTH_OFFSET		12
#define	OCOTP_ROM_SSP_SCK_INDEX_MASK		(0xf << 8)
#define	OCOTP_ROM_SSP_SCK_INDEX_OFFSET		8
#define	OCOTP_ROM_EMMC_USE_DDR			(1 << 7)
#define	OCOTP_ROM_DISABLE_SPI_NOR_FAST_READ	(1 << 6)
#define	OCOTP_ROM_ENABLE_USB_BOOT_SERIAL_NUM	(1 << 5)
#define	OCOTP_ROM_ENABLE_UNENCRYPTED_BOOT	(1 << 4)
#define	OCOTP_ROM_SD_MBR_BOOT			(1 << 3)

#define	OCOTP_SRK_BITS_MASK			0xffffffff
#define	OCOTP_SRK_BITS_OFFSET			0

#define	OCOTP_VERSION_MAJOR_MASK		(0xff << 24)
#define	OCOTP_VERSION_MAJOR_OFFSET		24
#define	OCOTP_VERSION_MINOR_MASK		(0xff << 16)
#define	OCOTP_VERSION_MINOR_OFFSET		16
#define	OCOTP_VERSION_STEP_MASK			0xffff
#define	OCOTP_VERSION_STEP_OFFSET		0

#endif /* __MX28_REGS_OCOTP_H__ */
