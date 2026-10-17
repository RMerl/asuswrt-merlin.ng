// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-2023 NXP
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <fdt_support.h>
#include <i2c.h>
#include <image.h>
#include <init.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <clock_legacy.h>
#include <fm_eth.h>
#include "t102xrdb.h"
#ifdef CONFIG_TARGET_T1024RDB
#include "cpld.h"
#elif defined(CONFIG_TARGET_T1023RDB)
#include <i2c.h>
#include <mmc.h>
#endif
#include "../common/sleep.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TARGET_T1023RDB
enum {
	GPIO1_SD_SEL    = 0x00020000, /* GPIO1_14, 0: eMMC, 1:SD/MMC */
	GPIO1_EMMC_SEL,
	GPIO3_GET_VERSION,	       /* GPIO3_4/5, 00:RevB, 01: RevC */
	GPIO3_BRD_VER_MASK = 0x0c000000,
	GPIO3_OFFSET = 0x2000,
	I2C_GET_BANK,
	I2C_SET_BANK0,
	I2C_SET_BANK4,
};
#endif

#if CONFIG_IS_ENABLED(DM_SERIAL)
int get_serial_clock(void)
{
	return get_bus_freq(0) / 2;
}
#endif

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *freq[3] = {"100.00MHZ", "125.00MHz", "156.25MHZ"};
	ccsr_gur_t __iomem *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) & FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	printf("Board: %sRDB, ", cpu->name);
#if defined(CONFIG_TARGET_T1024RDB)
	printf("Board rev: 0x%02x CPLD ver: 0x%02x, ",
	       CPLD_READ(hw_ver), CPLD_READ(sw_ver));
#elif defined(CONFIG_TARGET_T1023RDB)
	printf("Rev%c, ", t1023rdb_ctrl(GPIO3_GET_VERSION) + 'B');
#endif
	printf("boot from ");

#ifdef CONFIG_SDCARD
	puts("SD/MMC\n");
#elif CONFIG_SPIFLASH
	puts("SPI\n");
#elif defined(CONFIG_TARGET_T1024RDB)
	u8 reg;

	reg = CPLD_READ(flash_csr);

	if (reg & CPLD_BOOT_SEL) {
		puts("NAND\n");
	} else {
		reg = ((reg & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);
		printf("NOR vBank%d\n", reg);
	}
#elif defined(CONFIG_TARGET_T1023RDB)
#ifdef CONFIG_MTD_RAW_NAND
	puts("NAND\n");
#else
	printf("NOR vBank%d\n", t1023rdb_ctrl(I2C_GET_BANK));
#endif
#endif

	puts("SERDES Reference Clocks:\n");
	if (srds_s1 == 0x95)
		printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[2], freq[0]);
	else
		printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[0], freq[1]);

	return 0;
}

#ifdef CONFIG_TARGET_T1024RDB
static void board_mux_lane(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1;
	u8 reg = CPLD_READ(misc_ctl_status);

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	if (srds_prtcl_s1 == 0x95) {
		/* Route Lane B to PCIE */
		CPLD_WRITE(misc_ctl_status, reg & ~CPLD_PCIE_SGMII_MUX);
	} else {
		/* Route Lane B to SGMII */
		CPLD_WRITE(misc_ctl_status, reg | CPLD_PCIE_SGMII_MUX);
	}
	CPLD_WRITE(boot_override, CPLD_OVERRIDE_MUX_EN);
}
#endif

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	return 0;
}

int board_early_init_r(void)
{
#ifdef CFG_SYS_FLASH_BASE
	const unsigned int flashbase = CFG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);
	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();
	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CFG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);
#endif

#ifdef CONFIG_TARGET_T1024RDB
	board_mux_lane();
#endif

	pci_init();

	return 0;
}

#ifdef CONFIG_TARGET_T1024RDB
void board_reset(void)
{
	CPLD_WRITE(reset_ctl1, CPLD_LBMAP_RESET);
}
#endif

int misc_init_r(void)
{
	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
	fsl_fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
#ifndef CONFIG_DM_ETH
	fdt_fixup_fman_ethernet(blob);
#endif
	fdt_fixup_board_enet(blob);
#endif

#ifdef CONFIG_TARGET_T1023RDB
	if (t1023rdb_ctrl(GPIO3_GET_VERSION) > 0)
		fdt_enable_nor(blob);
#endif

	return 0;
}

#ifdef CONFIG_TARGET_T1023RDB
/* Enable NOR flash for RevC */
static void fdt_enable_nor(void *blob)
{
	int nodeoff = fdt_node_offset_by_compatible(blob, 0, "cfi-flash");

	if (nodeoff >= 0)
		fdt_status_okay(blob, nodeoff);
	else
		printf("WARNING unable to set status for NOR\n");
}

int board_mmc_getcd(struct mmc *mmc)
{
	ccsr_gpio_t __iomem *pgpio = (void *)(CFG_SYS_MPC85xx_GPIO_ADDR);
	u32 val = in_be32(&pgpio->gpdat);

	/* GPIO1_14, 0: eMMC, 1: SD/MMC */
	val &= GPIO1_SD_SEL;

	return val ? -1 : 1;
}

int board_mmc_getwp(struct mmc *mmc)
{
	ccsr_gpio_t __iomem *pgpio = (void *)(CFG_SYS_MPC85xx_GPIO_ADDR);
	u32 val = in_be32(&pgpio->gpdat);

	val &= GPIO1_SD_SEL;

	return val ? -1 : 0;
}

static u32 t1023rdb_ctrl(u32 ctrl_type)
{
	ccsr_gpio_t __iomem *pgpio = (void *)(CFG_SYS_MPC85xx_GPIO_ADDR);
	ccsr_gur_t __iomem  *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 val;
	u8 tmp;
	int bus_num = I2C_PCA6408_BUS_NUM;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_PCA6408_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return ret;
	}
	switch (ctrl_type) {
	case GPIO1_SD_SEL:
		val = in_be32(&pgpio->gpdat);
		val |= GPIO1_SD_SEL;
		out_be32(&pgpio->gpdat, val);
		setbits_be32(&pgpio->gpdir, GPIO1_SD_SEL);
		break;
	case GPIO1_EMMC_SEL:
		val = in_be32(&pgpio->gpdat);
		val &= ~GPIO1_SD_SEL;
		out_be32(&pgpio->gpdat, val);
		setbits_be32(&pgpio->gpdir, GPIO1_SD_SEL);
		break;
	case GPIO3_GET_VERSION:
		pgpio = (ccsr_gpio_t *)(CFG_SYS_MPC85xx_GPIO_ADDR
			 + GPIO3_OFFSET);
		val = in_be32(&pgpio->gpdat);
		val = ((val & GPIO3_BRD_VER_MASK) >> 26) & 0x3;
		if (val == 0x3) /* GPIO3_4/5 not used on RevB */
			val = 0;
		return val;
	case I2C_GET_BANK:
		dm_i2c_read(dev, 0, &tmp, 1);
		tmp &= 0x7;
		tmp = ((tmp & 1) << 2) | (tmp & 2) | ((tmp & 4) >> 2);
		return tmp;
	case I2C_SET_BANK0:
		tmp = 0x0;
		dm_i2c_write(dev, 1, &tmp, 1);
		tmp = 0xf8;
		dm_i2c_write(dev, 3, &tmp, 1);
		/* asserting HRESET_REQ */
		out_be32(&gur->rstcr, 0x2);
		break;
	case I2C_SET_BANK4:
		tmp = 0x1;
		dm_i2c_write(dev, 1, &tmp, 1);
		tmp = 0xf8;
		dm_i2c_write(dev, 3, &tmp, 1);
		out_be32(&gur->rstcr, 0x2);
		break;
	default:
		break;
	}
#else
	u32 orig_bus;

	orig_bus = i2c_get_bus_num();

	switch (ctrl_type) {
	case GPIO1_SD_SEL:
		val = in_be32(&pgpio->gpdat);
		val |= GPIO1_SD_SEL;
		out_be32(&pgpio->gpdat, val);
		setbits_be32(&pgpio->gpdir, GPIO1_SD_SEL);
		break;
	case GPIO1_EMMC_SEL:
		val = in_be32(&pgpio->gpdat);
		val &= ~GPIO1_SD_SEL;
		out_be32(&pgpio->gpdat, val);
		setbits_be32(&pgpio->gpdir, GPIO1_SD_SEL);
		break;
	case GPIO3_GET_VERSION:
		pgpio = (ccsr_gpio_t *)(CFG_SYS_MPC85xx_GPIO_ADDR
			 + GPIO3_OFFSET);
		val = in_be32(&pgpio->gpdat);
		val = ((val & GPIO3_BRD_VER_MASK) >> 26) & 0x3;
		if (val == 0x3) /* GPIO3_4/5 not used on RevB */
			val = 0;
		return val;
	case I2C_GET_BANK:
		i2c_set_bus_num(bus_num);
		i2c_read(I2C_PCA6408_ADDR, 0, 1, &tmp, 1);
		tmp &= 0x7;
		tmp = ((tmp & 1) << 2) | (tmp & 2) | ((tmp & 4) >> 2);
		i2c_set_bus_num(orig_bus);
		return tmp;
	case I2C_SET_BANK0:
		i2c_set_bus_num(bus_num);
		tmp = 0x0;
		i2c_write(I2C_PCA6408_ADDR, 1, 1, &tmp, 1);
		tmp = 0xf8;
		i2c_write(I2C_PCA6408_ADDR, 3, 1, &tmp, 1);
		/* asserting HRESET_REQ */
		out_be32(&gur->rstcr, 0x2);
		break;
	case I2C_SET_BANK4:
		i2c_set_bus_num(bus_num);
		tmp = 0x1;
		i2c_write(I2C_PCA6408_ADDR, 1, 1, &tmp, 1);
		tmp = 0xf8;
		i2c_write(I2C_PCA6408_ADDR, 3, 1, &tmp, 1);
		out_be32(&gur->rstcr, 0x2);
		break;
	default:
		break;
	}
#endif
	return 0;
}

static int switch_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (!strcmp(argv[1], "bank0"))
		t1023rdb_ctrl(I2C_SET_BANK0);
	else if (!strcmp(argv[1], "bank4") || !strcmp(argv[1], "altbank"))
		t1023rdb_ctrl(I2C_SET_BANK4);
	else if (!strcmp(argv[1], "sd"))
		t1023rdb_ctrl(GPIO1_SD_SEL);
	else if (!strcmp(argv[1], "emmc"))
		t1023rdb_ctrl(GPIO1_EMMC_SEL);
	else
		return CMD_RET_USAGE;
	return 0;
}

U_BOOT_CMD(
	switch, 2, 0, switch_cmd,
	"for bank0/bank4/sd/emmc switch control in runtime",
	"command (e.g. switch bank4)"
);
#endif
