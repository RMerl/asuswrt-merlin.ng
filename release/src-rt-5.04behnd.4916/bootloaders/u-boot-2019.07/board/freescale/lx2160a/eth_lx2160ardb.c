// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

DECLARE_GLOBAL_DATA_PTR;

static bool get_inphi_phy_id(struct mii_dev *bus, int addr, int devad)
{
	int phy_reg;
	u32 phy_id;

	phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);
	phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);
	phy_id |= (phy_reg & 0xffff);

	if (phy_id == PHY_UID_IN112525_S03)
		return true;
	else
		return false;
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct memac_mdio_info mdio_info;
	struct memac_mdio_controller *reg;
	int i, interface;
	struct mii_dev *dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the EMI 1 */
	fm_memac_mdio_init(bis, &mdio_info);

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the EMI 2 */
	fm_memac_mdio_init(bis, &mdio_info);

	dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
	switch (srds_s1) {
	case 19:
		wriop_set_phy_address(WRIOP1_DPMAC2, 0,
				      CORTINA_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC3, 0,
				      AQR107_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC4, 0,
				      AQR107_PHY_ADDR2);
		if (get_inphi_phy_id(dev, INPHI_PHY_ADDR1, MDIO_MMD_VEND1)) {
			wriop_set_phy_address(WRIOP1_DPMAC5, 0,
					      INPHI_PHY_ADDR1);
			wriop_set_phy_address(WRIOP1_DPMAC6, 0,
					      INPHI_PHY_ADDR1);
		}
		wriop_set_phy_address(WRIOP1_DPMAC17, 0,
				      RGMII_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0,
				      RGMII_PHY_ADDR2);
		break;

	case 18:
		wriop_set_phy_address(WRIOP1_DPMAC7, 0,
				      CORTINA_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC8, 0,
				      CORTINA_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC9, 0,
				      CORTINA_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC10, 0,
				      CORTINA_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC3, 0,
				      AQR107_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC4, 0,
				      AQR107_PHY_ADDR2);
		if (get_inphi_phy_id(dev, INPHI_PHY_ADDR1, MDIO_MMD_VEND1)) {
			wriop_set_phy_address(WRIOP1_DPMAC5, 0,
					      INPHI_PHY_ADDR1);
			wriop_set_phy_address(WRIOP1_DPMAC6, 0,
					      INPHI_PHY_ADDR1);
		}
		wriop_set_phy_address(WRIOP1_DPMAC17, 0,
				      RGMII_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0,
				      RGMII_PHY_ADDR2);
		break;

	default:
		printf("SerDes1 protocol 0x%x is not supported on LX2160ARDB\n",
		       srds_s1);
		goto next;
	}

	for (i = WRIOP1_DPMAC2; i <= WRIOP1_DPMAC10; i++) {
		interface = wriop_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_XGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
		case PHY_INTERFACE_MODE_25G_AUI:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
			wriop_set_mdio(i, dev);
			break;
		case PHY_INTERFACE_MODE_XLAUI:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}
	for (i = WRIOP1_DPMAC17; i <= WRIOP1_DPMAC18; i++) {
		interface = wriop_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_ID:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

next:
	cpu_eth_init(bis);
#endif /* CONFIG_FSL_MC_ENET */

#ifdef CONFIG_PHY_AQUANTIA
	/*
	 * Export functions to be used by AQ firmware
	 * upload application
	 */
	gd->jt->strcpy = strcpy;
	gd->jt->mdelay = mdelay;
	gd->jt->mdio_get_current_dev = mdio_get_current_dev;
	gd->jt->phy_find_by_mask = phy_find_by_mask;
	gd->jt->mdio_phydev_for_ethname = mdio_phydev_for_ethname;
	gd->jt->miiphy_set_current_dev = miiphy_set_current_dev;
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

int fdt_fixup_board_phy(void *fdt)
{
	int mdio_offset;
	int ret;
	struct mii_dev *dev;

	ret = 0;

	dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
	if (!get_inphi_phy_id(dev, INPHI_PHY_ADDR1, MDIO_MMD_VEND1)) {
		mdio_offset = fdt_path_offset(fdt, "/soc/mdio@0x8B97000");

		if (mdio_offset < 0)
			mdio_offset = fdt_path_offset(fdt, "/mdio@0x8B97000");

		if (mdio_offset < 0) {
			printf("mdio@0x8B9700 node not found in dts\n");
			return mdio_offset;
		}

		ret = fdt_setprop_string(fdt, mdio_offset, "status",
					 "disabled");
		if (ret) {
			printf("Could not set disable mdio@0x8B97000 %s\n",
			       fdt_strerror(ret));
			return ret;
		}
	}

	return ret;
}
