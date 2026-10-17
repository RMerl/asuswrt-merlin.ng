// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <mmc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "../common/tlv_data.h"

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-15t1-clearfog"
 */
#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

static struct tlv_data cf_tlv_data = { 0 };

static void cf_read_tlv_data(void)
{
	static bool read_once;

	if (read_once)
		return;
	read_once = true;

	read_tlv_data(&cf_tlv_data);
}

/* The starting board_serdes_map reflects original Clearfog Pro usage */
static struct serdes_map board_serdes_map[] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
};

void config_cfbase_serdes_map(void)
{
	board_serdes_map[4].serdes_type = USB3_HOST0;
	board_serdes_map[4].serdes_speed = SERDES_SPEED_5_GBPS;
	board_serdes_map[4].serdes_mode = SERDES_DEFAULT_MODE;
}

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	cf_read_tlv_data();

	/* Apply build configuration options before runtime configuration */
	if (IS_ENABLED(CONFIG_CLEARFOG_SFP_25GB))
		board_serdes_map[5].serdes_speed = SERDES_SPEED_3_125_GBPS;

	if (IS_ENABLED(CONFIG_CLEARFOG_CON2_SATA)) {
		board_serdes_map[4].serdes_type = SATA2;
		board_serdes_map[4].serdes_speed = SERDES_SPEED_3_GBPS;
		board_serdes_map[4].serdes_mode = SERDES_DEFAULT_MODE;
		board_serdes_map[4].swap_rx = 1;
	}

	if (IS_ENABLED(CONFIG_CLEARFOG_CON3_SATA)) {
		board_serdes_map[2].serdes_type = SATA1;
		board_serdes_map[2].serdes_speed = SERDES_SPEED_3_GBPS;
		board_serdes_map[2].serdes_mode = SERDES_DEFAULT_MODE;
		board_serdes_map[2].swap_rx = 1;
	}

	/* Apply runtime detection changes */
	if (sr_product_is(&cf_tlv_data, "Clearfog GTR")) {
		if (IS_ENABLED(CONFIG_CLEARFOG_GTR_SERDES0_SATA)) {
			/* serdes 0 is sata (like clearfog pro) */
		} else if (IS_ENABLED(CONFIG_CLEARFOG_GTR_SERDES0_PCIE)) {
			/* serdes 0 is pci */
			board_serdes_map[0].serdes_type = PEX0;
			board_serdes_map[0].serdes_speed = SERDES_SPEED_5_GBPS;
			board_serdes_map[0].serdes_mode = PEX_ROOT_COMPLEX_X1;
		}
		/* serdes 1 is 2.5Gbps fixed link to ethernet switch */
		board_serdes_map[1].serdes_type = SGMII1;
		board_serdes_map[1].serdes_speed = SERDES_SPEED_3_125_GBPS;
		board_serdes_map[1].serdes_mode = SERDES_DEFAULT_MODE;
		/* serdes 2 is pci (like clearfog pro) */
		/* serdes 3 is usb-3 (like clearfog pro) */
		/* serdes 4 is pci (like clearfog pro) */
		/* serdes 5 is sfp connector (like clearfog pro) */
	} else if (sr_product_is(&cf_tlv_data, "Clearfog Pro")) {
		/* handle recognized product as noop, no adjustment required */
	} else if (sr_product_is(&cf_tlv_data, "Clearfog Base")) {
		config_cfbase_serdes_map();
	} else {
		/*
		 * Fallback to static default. EEPROM TLV support is not
		 * enabled, runtime detection failed, hardware support is not
		 * present, EEPROM is corrupt, or an unrecognized product name
		 * is present.
		 */
		if (IS_ENABLED(CONFIG_SPL_CMD_TLV_EEPROM))
			puts("EEPROM TLV detection failed: ");
		puts("Using static config for ");
		if (IS_ENABLED(CONFIG_TARGET_CLEARFOG_BASE)) {
			puts("Clearfog Base.\n");
			config_cfbase_serdes_map();
		} else {
			puts("Clearfog Pro.\n");
		}
	}

	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct mv_ddr_topology_map board_topology_map = {
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
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_DEFAULT} },	/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	NOT_COMBINED,			/* ddr twin-die combined */
	{ {0} },			/* raw spd data */
	{0},				/* timing parameters */
	{ {0} },			/* electrical configuration */
	{0,},				/* electrical parameters */
	0,				/* ODT configuration */
	0x3,				/* clock enable mask */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	struct if_params *ifp = &board_topology_map.interface_params[0];

	cf_read_tlv_data();

	switch (cf_tlv_data.ram_size) {
	case 2:
		ifp->memory_size = MV_DDR_DIE_CAP_2GBIT;
		break;
	case 4:
	default:
		ifp->memory_size = MV_DDR_DIE_CAP_4GBIT;
		break;
	case 8:
		ifp->memory_size = MV_DDR_DIE_CAP_8GBIT;
		break;
	}

	switch (cf_tlv_data.ram_channels) {
	default:
	case 1:
		for (uint8_t i = 0; i < 5; i++)
			ifp->as_bus_params[i].cs_bitmask = 0x1;
		break;
	case 2:
		for (uint8_t i = 0; i < 5; i++)
			ifp->as_bus_params[i].cs_bitmask = 0x3;
		break;
	}

	/* Return the board topology as defined in the board code */
	return &board_topology_map;
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x10400011, MVEBU_MPP_BASE + 0x08);
	writel(0x22043333, MVEBU_MPP_BASE + 0x0c);
	writel(0x44400002, MVEBU_MPP_BASE + 0x10);
	writel(0x41144004, MVEBU_MPP_BASE + 0x14);
	writel(0x40333333, MVEBU_MPP_BASE + 0x18);
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(BOARD_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(BOARD_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(BOARD_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(BOARD_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(BOARD_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(BOARD_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/* Toggle GPIO41 to reset onboard switch and phy */
	clrbits_le32(MVEBU_GPIO1_BASE + 0x0, BIT(9));
	clrbits_le32(MVEBU_GPIO1_BASE + 0x4, BIT(9));
	/* GPIO 19 on ClearFog rev 2.1 controls the uSOM onboard phy reset */
	clrbits_le32(MVEBU_GPIO0_BASE + 0x0, BIT(19));
	clrbits_le32(MVEBU_GPIO0_BASE + 0x4, BIT(19));
	mdelay(1);
	setbits_le32(MVEBU_GPIO1_BASE + 0x0, BIT(9));
	setbits_le32(MVEBU_GPIO0_BASE + 0x0, BIT(19));
	mdelay(10);

	return 0;
}

int checkboard(void)
{
	char *board = "Clearfog Pro";
	if (IS_ENABLED(CONFIG_TARGET_CLEARFOG_BASE))
		board = "Clearfog Base";

	cf_read_tlv_data();
	if (strlen(cf_tlv_data.tlv_product_name[0]) > 0)
		board = cf_tlv_data.tlv_product_name[0];

	printf("Board: SolidRun %s", board);
	if (strlen(cf_tlv_data.tlv_product_name[1]) > 0)
		printf(", %s", cf_tlv_data.tlv_product_name[1]);
	puts("\n");

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}

int board_late_init(void)
{
	if (env_get("fdtfile"))
		return 0;

	cf_read_tlv_data();

	if (sr_product_is(&cf_tlv_data, "Clearfog Base"))
		env_set("fdtfile", "armada-388-clearfog-base.dtb");
	else if (sr_product_is(&cf_tlv_data, "Clearfog GTR S4"))
		env_set("fdtfile", "armada-385-clearfog-gtr-s4.dtb");
	else if (sr_product_is(&cf_tlv_data, "Clearfog GTR L8"))
		env_set("fdtfile", "armada-385-clearfog-gtr-l8.dtb");
	else if (IS_ENABLED(CONFIG_TARGET_CLEARFOG_BASE))
		env_set("fdtfile", "armada-388-clearfog-base.dtb");
	else
		env_set("fdtfile", "armada-388-clearfog-pro.dtb");

	return 0;
}

static bool has_emmc(void)
{
	struct mmc *mmc;

	mmc = find_mmc_device(0);
	if (!mmc)
		return 0;
	return (!mmc_init(mmc) && IS_MMC(mmc)) ? true : false;
}

/*
 * The Clearfog devices have only one SDHC device. This is either eMMC
 * if it is populated on the SOM or SDHC if not. The Linux device tree
 * assumes the SDHC case. Detect if the device is an eMMC and fixup the
 * device-tree, so that it will be detected by Linux.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int node;

	if (has_emmc()) {
		node = fdt_node_offset_by_compatible(blob, -1, "marvell,armada-380-sdhci");
		if (node < 0)
			return 0; /* Unexpected eMMC device; patching not supported */

		puts("Patching FDT so that eMMC is detected by OS\n");
		return fdt_setprop_empty(blob, node, "non-removable");
	}

	return 0;
}
