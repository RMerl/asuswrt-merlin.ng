// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 * Copyright (C) 2016 Mario Six <mario.six@gdsys.cc>
 */

#include <common.h>
#include <dm.h>
#include <miiphy.h>
#include <tpm-v1.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm-generic/gpio.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include "../arch/arm/mach-mvebu/serdes/a38x/high_speed_env_spec.h"

#include "keyprogram.h"
#include "dt_helpers.h"
#include "hydra.h"
#include "ihs_phys.h"

DECLARE_GLOBAL_DATA_PTR;

#define DB_GP_88F68XX_GPP_OUT_ENA_LOW	0x7fffffff
#define DB_GP_88F68XX_GPP_OUT_ENA_MID	0xffffefff

#define DB_GP_88F68XX_GPP_OUT_VAL_LOW	0x0
#define DB_GP_88F68XX_GPP_OUT_VAL_MID	0x00001000
#define DB_GP_88F68XX_GPP_POL_LOW	0x0
#define DB_GP_88F68XX_GPP_POL_MID	0x0

static int get_tpm(struct udevice **devp)
{
	int rc;

	rc = uclass_first_device_err(UCLASS_TPM, devp);
	if (rc) {
		printf("Could not find TPM (ret=%d)\n", rc);
		return CMD_RET_FAILURE;
	}

	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct mv_ddr_topology_map ddr_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_4GBIT,	/* mem_size */
	    MV_DDR_FREQ_533,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_DEFAULT} },	/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */

};

static struct serdes_map serdes_topology_map[] = {
	{SGMII0, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	/* SATA tx polarity is inverted */
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 1},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0}
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = serdes_topology_map;
	*count = ARRAY_SIZE(serdes_topology_map);
	return 0;
}

void board_pex_config(void)
{
#ifdef CONFIG_SPL_BUILD
	uint k;
	struct gpio_desc gpio = {};

	if (!request_gpio_by_name(&gpio, "pca9698@22", 31, "fpga-program-gpio")) {
		/* prepare FPGA reconfiguration */
		dm_gpio_set_dir_flags(&gpio, GPIOD_IS_OUT);
		dm_gpio_set_value(&gpio, 0);

		/* give lunatic PCIe clock some time to stabilize */
		mdelay(500);

		/* start FPGA reconfiguration */
		dm_gpio_set_dir_flags(&gpio, GPIOD_IS_IN);
	}

	/* wait for FPGA done */
	if (!request_gpio_by_name(&gpio, "pca9698@22", 19, "fpga-done-gpio")) {
		for (k = 0; k < 20; ++k) {
			if (dm_gpio_get_value(&gpio)) {
				printf("FPGA done after %u rounds\n", k);
				break;
			}
			mdelay(100);
		}
	}

	/* disable FPGA reset */
	if (!request_gpio_by_name(&gpio, "gpio@18100", 6, "cpu-to-fpga-reset")) {
		dm_gpio_set_dir_flags(&gpio, GPIOD_IS_OUT);
		dm_gpio_set_value(&gpio, 1);
	}

	/* wait for FPGA ready */
	if (!request_gpio_by_name(&gpio, "pca9698@22", 27, "fpga-ready-gpio")) {
		for (k = 0; k < 2; ++k) {
			if (!dm_gpio_get_value(&gpio))
				break;
			mdelay(100);
		}
	}
#endif
}

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	return &ddr_topology_map;
}

int board_early_init_f(void)
{
#ifdef CONFIG_SPL_BUILD
	/* Configure MPP */
	writel(0x00111111, MVEBU_MPP_BASE + 0x00);
	writel(0x40040000, MVEBU_MPP_BASE + 0x04);
	writel(0x00466444, MVEBU_MPP_BASE + 0x08);
	writel(0x00043300, MVEBU_MPP_BASE + 0x0c);
	writel(0x44400000, MVEBU_MPP_BASE + 0x10);
	writel(0x20000334, MVEBU_MPP_BASE + 0x14);
	writel(0x40000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(DB_GP_88F68XX_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(DB_GP_88F68XX_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(DB_GP_88F68XX_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(DB_GP_88F68XX_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(DB_GP_88F68XX_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(DB_GP_88F68XX_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
#endif

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#ifndef CONFIG_SPL_BUILD
void init_host_phys(struct mii_dev *bus)
{
	uint k;

	for (k = 0; k < 2; ++k) {
		struct phy_device *phydev;

		phydev = phy_find_by_mask(bus, 1 << k,
					  PHY_INTERFACE_MODE_SGMII);

		if (phydev)
			phy_config(phydev);
	}
}

int ccdc_eth_init(void)
{
	uint k;
	uint octo_phy_mask = 0;
	int ret;
	struct mii_dev *bus;

	/* Init SoC's phys */
	bus = miiphy_get_dev_by_name("ethernet@34000");

	if (bus)
		init_host_phys(bus);

	bus = miiphy_get_dev_by_name("ethernet@70000");

	if (bus)
		init_host_phys(bus);

	/* Init octo phys */
	octo_phy_mask = calculate_octo_phy_mask();

	printf("IHS PHYS: %08x", octo_phy_mask);

	ret = init_octo_phys(octo_phy_mask);

	if (ret)
		return ret;

	printf("\n");

	if (!get_fpga()) {
		puts("fpga was NULL\n");
		return 1;
	}

	/* reset all FPGA-QSGMII instances */
	for (k = 0; k < 80; ++k)
		writel(1 << 31, get_fpga()->qsgmii_port_state[k]);

	udelay(100);

	for (k = 0; k < 80; ++k)
		writel(0, get_fpga()->qsgmii_port_state[k]);
	return 0;
}

#endif

int board_late_init(void)
{
#ifndef CONFIG_SPL_BUILD
	hydra_initialize();
#endif
	return 0;
}

int board_fix_fdt(void *rw_fdt_blob)
{
	struct udevice *bus = NULL;
	uint k;
	char name[64];
	int err;

	err = uclass_get_device_by_name(UCLASS_I2C, "i2c@11000", &bus);

	if (err) {
		printf("Could not get I2C bus.\n");
		return err;
	}

	for (k = 0x21; k <= 0x26; k++) {
		snprintf(name, 64,
			 "/soc/internal-regs/i2c@11000/pca9698@%02x", k);

		if (!dm_i2c_simple_probe(bus, k))
			fdt_disable_by_ofname(rw_fdt_blob, name);
	}

	return 0;
}

int last_stage_init(void)
{
	struct udevice *tpm;
	int ret;

#ifndef CONFIG_SPL_BUILD
	ccdc_eth_init();
#endif
	ret = get_tpm(&tpm);
	if (ret || tpm_init(tpm) || tpm_startup(tpm, TPM_ST_CLEAR) ||
	    tpm_continue_self_test(tpm)) {
		return 1;
	}

	mdelay(37);

	flush_keys(tpm);
	load_and_run_keyprog(tpm);

	return 0;
}
