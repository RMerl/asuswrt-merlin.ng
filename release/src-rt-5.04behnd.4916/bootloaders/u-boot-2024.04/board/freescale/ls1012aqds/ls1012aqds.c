// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/cache.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/fdt.h>
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <ahci.h>
#include <hwconfig.h>
#include <mmc.h>
#include <env_internal.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_esdhc.h>
#include <fsl_mmdc.h>
#include <spl.h>
#include <netdev.h>
#include "../common/qixis.h"
#include "ls1012aqds_qixis.h"
#include "ls1012aqds_pfe.h"
#include <net/pfe_eth/pfe/pfe_hw.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	u8 sw;

	sw = QIXIS_READ(arch);
	printf("Board Arch: V%d, ", sw >> 4);
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A' - 1);

	sw = QIXIS_READ(brdcfg[QIXIS_LBMAP_BRDCFG_REG]);

	if (sw & QIXIS_LBMAP_ALTBANK)
		printf("flash: 2\n");
	else
		printf("flash: 1\n");

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());

	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));
	return 0;
}

#ifdef CONFIG_TFABOOT
int dram_init(void)
{
	gd->ram_size = tfa_get_dram_size();
	if (!gd->ram_size)
		gd->ram_size = CFG_SYS_SDRAM_SIZE;

	return 0;
}
#else
int dram_init(void)
{
	static const struct fsl_mmdc_info mparam = {
		0x05180000,	/* mdctl */
		0x00030035,	/* mdpdc */
		0x12554000,	/* mdotc */
		0xbabf7954,	/* mdcfg0 */
		0xdb328f64,	/* mdcfg1 */
		0x01ff00db,	/* mdcfg2 */
		0x00001680,	/* mdmisc */
		0x0f3c8000,	/* mdref */
		0x00002000,	/* mdrwd */
		0x00bf1023,	/* mdor */
		0x0000003f,	/* mdasp */
		0x0000022a,	/* mpodtctrl */
		0xa1390003,	/* mpzqhwctrl */
	};

	mmdc_init(&mparam);
	gd->ram_size = CFG_SYS_SDRAM_SIZE;
#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}
#endif

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	u8 mux_sdhc_cd = 0x80;
	int bus_num = 0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(bus_num, CFG_SYS_I2C_FPGA_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return ret;
	}
	dm_i2c_write(dev, 0x5a, &mux_sdhc_cd, 1);
#else
	i2c_set_bus_num(bus_num);

	i2c_write(CFG_SYS_I2C_FPGA_ADDR, 0x5a, 1, &mux_sdhc_cd, 1);
#endif

	return 0;
}
#endif

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
				   CONFIG_SYS_CCI400_OFFSET);

	/* Set CCI-400 control override register to enable barrier
	 * transaction */
	if (current_el() == 3)
		out_le32(&cci->ctrl_ord,
			 CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

	return 0;
}

#ifdef CONFIG_FSL_PFE
void board_quiesce_devices(void)
{
	pfe_command_stop(0, NULL);
}
#endif

int esdhc_status_fixup(void *blob, const char *compat)
{
	char esdhc0_path[] = "/soc/esdhc@1560000";
	char esdhc1_path[] = "/soc/esdhc@1580000";
	u8 card_id;

	do_fixup_by_path(blob, esdhc0_path, "status", "okay",
			 sizeof("okay"), 1);

	/*
	 * The Presence Detect 2 register detects the installation
	 * of cards in various PCI Express or SGMII slots.
	 *
	 * STAT_PRS2[7:5]: Specifies the type of card installed in the
	 * SDHC2 Adapter slot. 0b111 indicates no adapter is installed.
	 */
	card_id = (QIXIS_READ(present2) & 0xe0) >> 5;

	/* If no adapter is installed in SDHC2, disable SDHC2 */
	if (card_id == 0x7)
		do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	else
		do_fixup_by_path(blob, esdhc1_path, "status", "okay",
				 sizeof("okay"), 1);
	return 0;
}

static int pfe_set_properties(void *set_blob, struct pfe_prop_val prop_val,
			      char *enet_path, char *mdio_path)
{
	do_fixup_by_path(set_blob, enet_path, "fsl,gemac-bus-id",
			 &prop_val.busid, PFE_PROP_LEN, 1);
	do_fixup_by_path(set_blob, enet_path, "fsl,gemac-phy-id",
			 &prop_val.phyid, PFE_PROP_LEN, 1);
	do_fixup_by_path(set_blob, enet_path, "fsl,mdio-mux-val",
			 &prop_val.mux_val, PFE_PROP_LEN, 1);
	do_fixup_by_path(set_blob, enet_path, "phy-mode",
			 prop_val.phy_mode, strlen(prop_val.phy_mode) + 1, 1);
	do_fixup_by_path(set_blob, mdio_path, "fsl,mdio-phy-mask",
			 &prop_val.phy_mask, PFE_PROP_LEN, 1);
	return 0;
}

static void fdt_fsl_fixup_of_pfe(void *blob)
{
	int i = 0;
	struct pfe_prop_val prop_val;
	void *l_blob = blob;

	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;
	unsigned int srds_s1 = in_be32(&gur->rcwsr[4]) &
		FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	for (i = 0; i < NUM_ETH_NODE; i++) {
		switch (srds_s1) {
		case SERDES_1_G_PROTOCOL:
			if (i == 0) {
				prop_val.busid = cpu_to_fdt32(
						ETH_1_1G_BUS_ID);
				prop_val.phyid = cpu_to_fdt32(
						ETH_1_1G_PHY_ID);
				prop_val.mux_val = cpu_to_fdt32(
						ETH_1_1G_MDIO_MUX);
				prop_val.phy_mask = cpu_to_fdt32(
						ETH_1G_MDIO_PHY_MASK);
				prop_val.phy_mode = "sgmii";
				pfe_set_properties(l_blob, prop_val, ETH_1_PATH,
						   ETH_1_MDIO);
			} else {
				prop_val.busid = cpu_to_fdt32(
						ETH_2_1G_BUS_ID);
				prop_val.phyid = cpu_to_fdt32(
						ETH_2_1G_PHY_ID);
				prop_val.mux_val = cpu_to_fdt32(
						ETH_2_1G_MDIO_MUX);
				prop_val.phy_mask = cpu_to_fdt32(
						ETH_1G_MDIO_PHY_MASK);
				prop_val.phy_mode = "rgmii";
				pfe_set_properties(l_blob, prop_val, ETH_2_PATH,
						   ETH_2_MDIO);
			}
		break;
		case SERDES_2_5_G_PROTOCOL:
			if (i == 0) {
				prop_val.busid = cpu_to_fdt32(
						ETH_1_2_5G_BUS_ID);
				prop_val.phyid = cpu_to_fdt32(
						ETH_1_2_5G_PHY_ID);
				prop_val.mux_val = cpu_to_fdt32(
						ETH_1_2_5G_MDIO_MUX);
				prop_val.phy_mask = cpu_to_fdt32(
						ETH_2_5G_MDIO_PHY_MASK);
				prop_val.phy_mode = "2500base-x";
				pfe_set_properties(l_blob, prop_val, ETH_1_PATH,
						   ETH_1_MDIO);
			} else {
				prop_val.busid = cpu_to_fdt32(
						ETH_2_2_5G_BUS_ID);
				prop_val.phyid = cpu_to_fdt32(
						ETH_2_2_5G_PHY_ID);
				prop_val.mux_val = cpu_to_fdt32(
						ETH_2_2_5G_MDIO_MUX);
				prop_val.phy_mask = cpu_to_fdt32(
						ETH_2_5G_MDIO_PHY_MASK);
				prop_val.phy_mode = "2500base-x";
				pfe_set_properties(l_blob, prop_val, ETH_2_PATH,
						   ETH_2_MDIO);
			}
		break;
		default:
			printf("serdes:[%d]\n", srds_s1);
		}
	}
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);
	fdt_fsl_fixup_of_pfe(blob);

	return 0;
}
#endif
