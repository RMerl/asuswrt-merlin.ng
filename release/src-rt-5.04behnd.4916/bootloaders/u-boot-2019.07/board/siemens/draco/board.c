// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for TI AM335X based draco board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 *
 * Board functions for TI AM335X based boards
 * u-boot:/board/ti/am335x/board.c
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <watchdog.h>
#include "board.h"
#include "../common/factoryset.h"
#include <nand.h>

#ifdef CONFIG_SPL_BUILD
static struct draco_baseboard_id __attribute__((section(".data"))) settings;

#if DDR_PLL_FREQ == 303
#if !defined(CONFIG_TARGET_ETAMIN)
/* Default@303MHz-i0 */
const struct ddr3_data ddr3_default = {
	0x33524444, 0x56312e35, 0x0080, 0x0000, 0x003A, 0x003F, 0x009F,
	0x0079, 0x0888A39B, 0x26517FDA, 0x501F84EF, 0x00100206, 0x61A44A32,
	0x0000093B, 0x0000014A,
	"default name @303MHz           \0",
	"default marking                \0",
};
#else
/* etamin board */
const struct ddr3_data ddr3_default = {
	0x33524444, 0x56312e36, 0x0080, 0x0000, 0x003A, 0x0010, 0x009F,
	0x0050, 0x0888A39B, 0x266D7FDA, 0x501F86AF, 0x00100206, 0x61A44BB2,
	0x0000093B, 0x0000018A,
	"test-etamin                    \0",
	"generic-8Gbit                  \0",
};
#endif
#elif DDR_PLL_FREQ == 400
/* Default@400MHz-i0 */
const struct ddr3_data ddr3_default = {
	0x33524444, 0x56312e35, 0x0080, 0x0000, 0x0039, 0x0046, 0x00ab,
	0x0080, 0x0AAAA4DB, 0x26307FDA, 0x501F821F, 0x00100207, 0x61A45232,
	0x00000618, 0x0000014A,
	"default name @400MHz           \0",
	"default marking                \0",
};
#endif

static void set_default_ddr3_timings(void)
{
	printf("Set default DDR3 settings\n");
	settings.ddr3 = ddr3_default;
}

static void print_ddr3_timings(void)
{
	printf("\nDDR3\n");
	printf("clock:\t\t%d MHz\n", DDR_PLL_FREQ);
	printf("device:\t\t%s\n", settings.ddr3.manu_name);
	printf("marking:\t%s\n", settings.ddr3.manu_marking);
	printf("%-20s, %-8s, %-8s, %-4s\n", "timing parameters", "eeprom",
	       "default", "diff");
	PRINTARGS(magic);
	PRINTARGS(version);
	PRINTARGS(ddr3_sratio);
	PRINTARGS(iclkout);

	PRINTARGS(dt0rdsratio0);
	PRINTARGS(dt0wdsratio0);
	PRINTARGS(dt0fwsratio0);
	PRINTARGS(dt0wrsratio0);

	PRINTARGS(sdram_tim1);
	PRINTARGS(sdram_tim2);
	PRINTARGS(sdram_tim3);

	PRINTARGS(emif_ddr_phy_ctlr_1);

	PRINTARGS(sdram_config);
	PRINTARGS(ref_ctrl);
	PRINTARGS(ioctr_val);
}

static void print_chip_data(void)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	printf("\nCPU BOARD\n");
	printf("device: \t'%s'\n", settings.chip.sdevname);
	printf("hw version: \t'%s'\n", settings.chip.shwver);
	printf("max freq: \t%d MHz\n", dpll_mpu_opp100.m);
}
#endif /* CONFIG_SPL_BUILD */

#define AM335X_NAND_ECC_MASK 0x0f
#define AM335X_NAND_ECC_TYPE_16 0x02

static int ecc_type;

struct am335x_nand_geometry {
	u32 magic;
	u8 nand_geo_addr;
	u8 nand_geo_page;
	u8 nand_bus;
};

static int draco_read_nand_geometry(void)
{
	struct am335x_nand_geometry geo;

	/* Read NAND geometry */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0x80, 2,
		     (uchar *)&geo, sizeof(struct am335x_nand_geometry))) {
		printf("Could not read the NAND geomtery; something fundamentally wrong on the I2C bus.\n");
		return -EIO;
	}
	if (geo.magic != 0xa657b310) {
		printf("%s: bad magic: %x\n", __func__, geo.magic);
		return -EFAULT;
	}
	if ((geo.nand_bus & AM335X_NAND_ECC_MASK) == AM335X_NAND_ECC_TYPE_16)
		ecc_type = 16;
	else
		ecc_type = 8;

	return 0;
}

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(void)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		printf("Could not probe the EEPROM; something fundamentally wrong on the I2C bus.\n");
		return 1;
	}

#ifdef CONFIG_SPL_BUILD
	/* Read Siemens eeprom data (DDR3) */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, EEPROM_ADDR_DDR3, 2,
		     (uchar *)&settings.ddr3, sizeof(struct ddr3_data))) {
		printf("Could not read the EEPROM; something fundamentally wrong on the I2C bus.\nUse default DDR3 timings\n");
		set_default_ddr3_timings();
	}
	/* Read Siemens eeprom data (CHIP) */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, EEPROM_ADDR_CHIP, 2,
		     (uchar *)&settings.chip, sizeof(settings.chip)))
		printf("Could not read chip settings\n");

	if (ddr3_default.magic == settings.ddr3.magic &&
	    ddr3_default.version == settings.ddr3.version) {
		printf("Using DDR3 settings from EEPROM\n");
	} else {
		if (ddr3_default.magic != settings.ddr3.magic)
			printf("Warning: No valid DDR3 data in eeprom.\n");
		if (ddr3_default.version != settings.ddr3.version)
			printf("Warning: DDR3 data version does not match.\n");

		printf("Using default settings\n");
		set_default_ddr3_timings();
	}

	if (MAGIC_CHIP == settings.chip.magic)
		print_chip_data();
	else
		printf("Warning: No chip data in eeprom\n");

	print_ddr3_timings();

	return draco_read_nand_geometry();
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD
static void board_init_ddr(void)
{
struct emif_regs draco_ddr3_emif_reg_data = {
	.zq_config = 0x50074BE4,
};

struct ddr_data draco_ddr3_data = {
};

struct cmd_control draco_ddr3_cmd_ctrl_data = {
};

struct ctrl_ioregs draco_ddr3_ioregs = {
};

	/* pass values from eeprom */
	draco_ddr3_emif_reg_data.sdram_tim1 = settings.ddr3.sdram_tim1;
	draco_ddr3_emif_reg_data.sdram_tim2 = settings.ddr3.sdram_tim2;
	draco_ddr3_emif_reg_data.sdram_tim3 = settings.ddr3.sdram_tim3;
	draco_ddr3_emif_reg_data.emif_ddr_phy_ctlr_1 =
		settings.ddr3.emif_ddr_phy_ctlr_1;
	draco_ddr3_emif_reg_data.sdram_config = settings.ddr3.sdram_config;
	draco_ddr3_emif_reg_data.sdram_config2 = 0x08000000;
	draco_ddr3_emif_reg_data.ref_ctrl = settings.ddr3.ref_ctrl;

	draco_ddr3_data.datardsratio0 = settings.ddr3.dt0rdsratio0;
	draco_ddr3_data.datawdsratio0 = settings.ddr3.dt0wdsratio0;
	draco_ddr3_data.datafwsratio0 = settings.ddr3.dt0fwsratio0;
	draco_ddr3_data.datawrsratio0 = settings.ddr3.dt0wrsratio0;

	draco_ddr3_cmd_ctrl_data.cmd0csratio = settings.ddr3.ddr3_sratio;
	draco_ddr3_cmd_ctrl_data.cmd0iclkout = settings.ddr3.iclkout;
	draco_ddr3_cmd_ctrl_data.cmd1csratio = settings.ddr3.ddr3_sratio;
	draco_ddr3_cmd_ctrl_data.cmd1iclkout = settings.ddr3.iclkout;
	draco_ddr3_cmd_ctrl_data.cmd2csratio = settings.ddr3.ddr3_sratio;
	draco_ddr3_cmd_ctrl_data.cmd2iclkout = settings.ddr3.iclkout;

	draco_ddr3_ioregs.cm0ioctl = settings.ddr3.ioctr_val,
	draco_ddr3_ioregs.cm1ioctl = settings.ddr3.ioctr_val,
	draco_ddr3_ioregs.cm2ioctl = settings.ddr3.ioctr_val,
	draco_ddr3_ioregs.dt0ioctl = settings.ddr3.ioctr_val,
	draco_ddr3_ioregs.dt1ioctl = settings.ddr3.ioctr_val,

	config_ddr(DDR_PLL_FREQ, &draco_ddr3_ioregs, &draco_ddr3_data,
		   &draco_ddr3_cmd_ctrl_data, &draco_ddr3_emif_reg_data, 0);
}

static void spl_siemens_board_init(void)
{
	return;
}
#endif /* if def CONFIG_SPL_BUILD */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;

	ret = draco_read_nand_geometry();
	if (ret != 0)
		return ret;

	nand_curr_device = 0;
	omap_nand_switch_ecc(1, ecc_type);
#ifdef CONFIG_TARGET_ETAMIN
	nand_curr_device = 1;
	omap_nand_switch_ecc(1, ecc_type);
#endif
#ifdef CONFIG_FACTORYSET
	/* Set ASN in environment*/
	if (factory_dat.asn[0] != 0) {
		env_set("dtb_name", (char *)factory_dat.asn);
	} else {
		/* dtb suffix gets added in load script */
		env_set("dtb_name", "am335x-draco");
	}
#else
	env_set("dtb_name", "am335x-draco");
#endif

	return 0;
}
#endif

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_MII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 4,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	int n = 0;
	int rv;

	factoryset_env_set();

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel((RMII_MODE_ENABLE | RMII_CHIPCKL_ENABLE), &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
	return n;
}

static int do_switch_reset(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	/* Reset SMSC LAN9303 switch for default configuration */
	gpio_request(GPIO_LAN9303_NRST, "nRST");
	gpio_direction_output(GPIO_LAN9303_NRST, 0);
	/* assert active low reset for 200us */
	udelay(200);
	gpio_set_value(GPIO_LAN9303_NRST, 1);

	return 0;
};

U_BOOT_CMD(
	switch_rst, CONFIG_SYS_MAXARGS, 1,	do_switch_reset,
	"Reset LAN9303 switch via its reset pin",
	""
);
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#ifdef CONFIG_NAND_CS_INIT
/* GPMC definitions for second nand cs1 */
static const u32 gpmc_nand_config[] = {
	ETAMIN_NAND_GPMC_CONFIG1,
	ETAMIN_NAND_GPMC_CONFIG2,
	ETAMIN_NAND_GPMC_CONFIG3,
	ETAMIN_NAND_GPMC_CONFIG4,
	ETAMIN_NAND_GPMC_CONFIG5,
	ETAMIN_NAND_GPMC_CONFIG6,
	/*CONFIG7- computed as params */
};

static void board_nand_cs_init(void)
{
	enable_gpmc_cs_config(gpmc_nand_config, &gpmc_cfg->cs[1],
			      0x18000000, GPMC_SIZE_16M);
}
#endif

#include "../common/board.c"
