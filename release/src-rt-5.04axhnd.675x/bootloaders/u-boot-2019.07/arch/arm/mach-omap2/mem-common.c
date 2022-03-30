// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *     Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Initial Code from:
 *     Manikandan Pillai <mani.pillai@ti.com>
 *     Richard Woodruff <r-woodruff2@ti.com>
 *     Syed Mohammed Khasim <khasim@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <command.h>
#include <linux/mtd/omap_gpmc.h>
#include <jffs2/load_kernel.h>

const struct gpmc *gpmc_cfg = (struct gpmc *)GPMC_BASE;

#if defined(CONFIG_NOR)
char gpmc_cs0_flash = MTD_DEV_TYPE_NOR;
#elif defined(CONFIG_NAND) || defined(CONFIG_CMD_NAND)
char gpmc_cs0_flash = MTD_DEV_TYPE_NAND;
#elif defined(CONFIG_CMD_ONENAND)
char gpmc_cs0_flash = MTD_DEV_TYPE_ONENAND;
#else
char gpmc_cs0_flash = -1;
#endif

#if defined(CONFIG_OMAP34XX)
/********************************************************
 *  mem_ok() - test used to see if timings are correct
 *             for a part. Helps in guessing which part
 *             we are currently using.
 *******************************************************/
u32 mem_ok(u32 cs)
{
	u32 val1, val2, addr;
	u32 pattern = 0x12345678;

	addr = OMAP34XX_SDRC_CS0 + get_sdr_cs_offset(cs);

	writel(0x0, addr + 0x400);	/* clear pos A */
	writel(pattern, addr);		/* pattern to pos B */
	writel(0x0, addr + 4);		/* remove pattern off the bus */
	val1 = readl(addr + 0x400);	/* get pos A value */
	val2 = readl(addr);		/* get val2 */
	writel(0x0, addr + 0x400);	/* clear pos A */

	if ((val1 != 0) || (val2 != pattern))	/* see if pos A val changed */
		return 0;
	else
		return 1;
}
#endif

void enable_gpmc_cs_config(const u32 *gpmc_config, const struct gpmc_cs *cs,
				u32 base, u32 size)
{
	writel(0, &cs->config7);
	sdelay(1000);
	/* Delay for settling */
	writel(gpmc_config[0], &cs->config1);
	writel(gpmc_config[1], &cs->config2);
	writel(gpmc_config[2], &cs->config3);
	writel(gpmc_config[3], &cs->config4);
	writel(gpmc_config[4], &cs->config5);
	writel(gpmc_config[5], &cs->config6);
	/* Enable the config */
	writel((((size & 0xF) << 8) | ((base >> 24) & 0x3F) |
		(1 << 6)), &cs->config7);
	sdelay(2000);
}

void set_gpmc_cs0(int flash_type)
{
	const u32 *gpmc_regs;
	u32 base, size;
#if defined(CONFIG_NOR)
	const u32 gpmc_regs_nor[GPMC_MAX_REG] = {
		STNOR_GPMC_CONFIG1,
		STNOR_GPMC_CONFIG2,
		STNOR_GPMC_CONFIG3,
		STNOR_GPMC_CONFIG4,
		STNOR_GPMC_CONFIG5,
		STNOR_GPMC_CONFIG6,
		STNOR_GPMC_CONFIG7
	};
#endif
#if defined(CONFIG_NAND) || defined(CONFIG_CMD_NAND)
	const u32 gpmc_regs_nand[GPMC_MAX_REG] = {
		M_NAND_GPMC_CONFIG1,
		M_NAND_GPMC_CONFIG2,
		M_NAND_GPMC_CONFIG3,
		M_NAND_GPMC_CONFIG4,
		M_NAND_GPMC_CONFIG5,
		M_NAND_GPMC_CONFIG6,
		0
	};
#endif
#if defined(CONFIG_CMD_ONENAND)
	const u32 gpmc_regs_onenand[GPMC_MAX_REG] = {
		ONENAND_GPMC_CONFIG1,
		ONENAND_GPMC_CONFIG2,
		ONENAND_GPMC_CONFIG3,
		ONENAND_GPMC_CONFIG4,
		ONENAND_GPMC_CONFIG5,
		ONENAND_GPMC_CONFIG6,
		0
	};
#endif

	switch (flash_type) {
#if defined(CONFIG_NOR)
	case MTD_DEV_TYPE_NOR:
		gpmc_regs = gpmc_regs_nor;
		base = CONFIG_SYS_FLASH_BASE;
		size = (CONFIG_SYS_FLASH_SIZE > 0x08000000) ? GPMC_SIZE_256M :
		      ((CONFIG_SYS_FLASH_SIZE > 0x04000000) ? GPMC_SIZE_128M :
		      ((CONFIG_SYS_FLASH_SIZE > 0x02000000) ? GPMC_SIZE_64M  :
		      ((CONFIG_SYS_FLASH_SIZE > 0x01000000) ? GPMC_SIZE_32M  :
		                                              GPMC_SIZE_16M)));
		break;
#endif
#if defined(CONFIG_NAND) || defined(CONFIG_CMD_NAND)
	case MTD_DEV_TYPE_NAND:
		gpmc_regs = gpmc_regs_nand;
		base = CONFIG_SYS_NAND_BASE;
		size = GPMC_SIZE_16M;
		break;
#endif
#if defined(CONFIG_CMD_ONENAND)
	case MTD_DEV_TYPE_ONENAND:
		gpmc_regs = gpmc_regs_onenand;
		base = CONFIG_SYS_ONENAND_BASE;
		size = GPMC_SIZE_128M;
		break;
#endif
	default:
		/* disable the GPMC0 config set by ROM code */
		writel(0, &gpmc_cfg->cs[0].config7);
		sdelay(1000);
		return;
	}

	/* enable chip-select specific configurations */
	enable_gpmc_cs_config(gpmc_regs, &gpmc_cfg->cs[0], base, size);
}

/*****************************************************
 * gpmc_init(): init gpmc bus
 * Init GPMC for x16, MuxMode (SDRAM in x32).
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
	/* global settings */
	writel(0x00000008, &gpmc_cfg->sysconfig);
	writel(0x00000000, &gpmc_cfg->irqstatus);
	writel(0x00000000, &gpmc_cfg->irqenable);
	/* disable timeout, set a safe reset value */
	writel(0x00001ff0, &gpmc_cfg->timeout_control);
	writel(gpmc_cs0_flash == MTD_DEV_TYPE_NOR ?
		0x00000200 : 0x00000012, &gpmc_cfg->config);

	set_gpmc_cs0(gpmc_cs0_flash);
}
