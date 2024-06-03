/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX23/i.MX28 specific functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef __MXS_SYS_PROTO_H__
#define __MXS_SYS_PROTO_H__

#include <asm/mach-imx/sys_proto.h>

int mxsmmc_initialize(bd_t *bis, int id, int (*wp)(int), int (*cd)(int));

#ifdef CONFIG_SPL_BUILD

#if defined(CONFIG_MX23)
#include <asm/arch/iomux-mx23.h>
#elif defined(CONFIG_MX28)
#include <asm/arch/iomux-mx28.h>
#endif

void mxs_common_spl_init(const uint32_t arg, const uint32_t *resptr,
			 const iomux_cfg_t *iomux_setup,
			 const unsigned int iomux_size);

void mxs_power_switch_dcdc_clocksource(uint32_t freqsel);
#endif

struct mxs_pair {
	uint8_t	boot_pads;
	uint8_t boot_mask;
	const char *mode;
};

static const struct mxs_pair mxs_boot_modes[] = {
#if defined(CONFIG_MX23)
	{ 0x00, 0x0f, "USB" },
	{ 0x01, 0x1f, "I2C, master" },
	{ 0x02, 0x1f, "SSP SPI #1, master, NOR" },
	{ 0x03, 0x1f, "SSP SPI #2, master, NOR" },
	{ 0x04, 0x1f, "NAND" },
	{ 0x06, 0x1f, "JTAG" },
	{ 0x08, 0x1f, "SSP SPI #3, master, EEPROM" },
	{ 0x09, 0x1f, "SSP SD/MMC #0" },
	{ 0x0a, 0x1f, "SSP SD/MMC #1" },
	{ 0x00, 0x00, "Reserved/Unknown/Wrong" },
#elif defined(CONFIG_MX28)
	{ 0x00, 0x0f, "USB #0" },
	{ 0x01, 0x1f, "I2C #0, master, 3V3" },
	{ 0x11, 0x1f, "I2C #0, master, 1V8" },
	{ 0x02, 0x1f, "SSP SPI #2, master, 3V3 NOR" },
	{ 0x12, 0x1f, "SSP SPI #2, master, 1V8 NOR" },
	{ 0x03, 0x1f, "SSP SPI #3, master, 3V3 NOR" },
	{ 0x13, 0x1f, "SSP SPI #3, master, 1V8 NOR" },
	{ 0x04, 0x1f, "NAND, 3V3" },
	{ 0x14, 0x1f, "NAND, 1V8" },
	{ 0x06, 0x1f, "JTAG" },
	{ 0x08, 0x1f, "SSP SPI #3, master, 3V3 EEPROM" },
	{ 0x18, 0x1f, "SSP SPI #3, master, 1V8 EEPROM" },
	{ 0x09, 0x1f, "SSP SD/MMC #0, 3V3" },
	{ 0x19, 0x1f, "SSP SD/MMC #0, 1V8" },
	{ 0x0a, 0x1f, "SSP SD/MMC #1, 3V3" },
	{ 0x1a, 0x1f, "SSP SD/MMC #1, 1V8" },
	{ 0x00, 0x00, "Reserved/Unknown/Wrong" },
#endif
};

#define MXS_BM_USB			0x00
#define MXS_BM_I2C_MASTER_3V3		0x01
#define MXS_BM_I2C_MASTER_1V8		0x11
#define MXS_BM_SPI2_MASTER_3V3_NOR	0x02
#define MXS_BM_SPI2_MASTER_1V8_NOR	0x12
#define MXS_BM_SPI3_MASTER_3V3_NOR	0x03
#define MXS_BM_SPI3_MASTER_1V8_NOR	0x13
#define MXS_BM_NAND_3V3			0x04
#define MXS_BM_NAND_1V8			0x14
#define MXS_BM_JTAG			0x06
#define MXS_BM_SPI3_MASTER_3V3_EEPROM	0x08
#define MXS_BM_SPI3_MASTER_1V8_EEPROM	0x18
#define MXS_BM_SDMMC0_3V3		0x09
#define MXS_BM_SDMMC0_1V8		0x19
#define MXS_BM_SDMMC1_3V3		0x0a
#define MXS_BM_SDMMC1_1V8		0x1a

#define MXS_SPL_DATA ((struct mxs_spl_data *)(CONFIG_SYS_TEXT_BASE - 0x200))

struct mxs_spl_data {
	uint8_t		boot_mode_idx;
	uint32_t	mem_dram_size;
};

int mxs_dram_init(void);

#endif	/* __SYS_PROTO_H__ */
