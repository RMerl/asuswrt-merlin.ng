// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
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

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	int i, interface;
	struct memac_mdio_info mdio_info;
	struct mii_dev *dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct memac_mdio_controller *reg;
	u32 srds_s1, cfg;

	cfg = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
				FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	cfg >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;

	srds_s1 = serdes_get_number(FSL_SRDS_1, cfg);

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

	switch (srds_s1) {
	case 0x1D:
		/*
		 * XFI does not need a PHY to work, but to avoid U-boot use
		 * default PHY address which is zero to a MAC when it found
		 * a MAC has no PHY address, we give a PHY address to XFI
		 * MAC error.
		 */
		wriop_set_phy_address(WRIOP1_DPMAC1, 0, 0x0a);
		wriop_set_phy_address(WRIOP1_DPMAC2, 0, AQ_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC3, 0, QSGMII1_PORT1_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC4, 0, QSGMII1_PORT2_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC5, 0, QSGMII1_PORT3_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC6, 0, QSGMII1_PORT4_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC7, 0, QSGMII2_PORT1_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC8, 0, QSGMII2_PORT2_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC9, 0, QSGMII2_PORT3_PHY_ADDR);
		wriop_set_phy_address(WRIOP1_DPMAC10, 0,
				      QSGMII2_PORT4_PHY_ADDR);

		break;
	default:
		printf("SerDes1 protocol 0x%x is not supported on LS1088ARDB\n",
		       srds_s1);
		break;
	}

	for (i = WRIOP1_DPMAC3; i <= WRIOP1_DPMAC10; i++) {
		interface = wriop_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_QSGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

	dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
	wriop_set_mdio(WRIOP1_DPMAC2, dev);

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */
