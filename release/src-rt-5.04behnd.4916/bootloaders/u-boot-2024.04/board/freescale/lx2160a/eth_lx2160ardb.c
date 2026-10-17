// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2020 NXP
 *
 */

#include <common.h>
#include <netdev.h>
#include <exports.h>
#include <fsl-mc/fsl_mc.h>
#include "lx2160a.h"

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(struct bd_info *bis)
{
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

static int fdt_get_dpmac_node(void *fdt, int dpmac_id)
{
	char dpmac_str[11] = "dpmacs@00";
	int offset, dpmacs_offset;

	/* get the dpmac offset */
	dpmacs_offset = fdt_path_offset(fdt, "/soc/fsl-mc/dpmacs");
	if (dpmacs_offset < 0)
		dpmacs_offset = fdt_path_offset(fdt, "/fsl-mc/dpmacs");

	if (dpmacs_offset < 0) {
		printf("dpmacs node not found in device tree\n");
		return dpmacs_offset;
	}

	sprintf(dpmac_str, "dpmac@%x", dpmac_id);
	offset = fdt_subnode_offset(fdt, dpmacs_offset, dpmac_str);
	if (offset < 0) {
		sprintf(dpmac_str, "ethernet@%x", dpmac_id);
		offset = fdt_subnode_offset(fdt, dpmacs_offset, dpmac_str);
		if (offset < 0) {
			printf("dpmac@%x/ethernet@%x node not found in device tree\n",
			       dpmac_id, dpmac_id);
			return offset;
		}
	}

	return offset;
}

static int fdt_update_phy_addr(void *fdt, int dpmac_id, int phy_addr)
{
	char dpmac_str[] = "dpmacs@00";
	const u32 *phyhandle;
	int offset;
	int err;

	/* get the dpmac offset */
	offset = fdt_get_dpmac_node(fdt, dpmac_id);
	if (offset < 0)
		return offset;

	/* get dpmac phy-handle */
	sprintf(dpmac_str, "dpmac@%x", dpmac_id);
	phyhandle = (u32 *)fdt_getprop(fdt, offset, "phy-handle", NULL);
	if (!phyhandle) {
		printf("%s node not found in device tree\n", dpmac_str);
		return offset;
	}

	offset = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*phyhandle));
	if (offset < 0) {
		printf("Could not get the ph node offset for dpmac %d\n",
		       dpmac_id);
		return offset;
	}

	phy_addr = cpu_to_fdt32(phy_addr);
	err = fdt_setprop(fdt, offset, "reg", &phy_addr, sizeof(phy_addr));
	if (err < 0) {
		printf("Could not set phy node's reg for dpmac %d: %s.\n",
		       dpmac_id, fdt_strerror(err));
		return err;
	}

	return 0;
}

static int fdt_delete_phy_handle(void *fdt, int dpmac_id)
{
	const u32 *phyhandle;
	int offset;

	/* get the dpmac offset */
	offset = fdt_get_dpmac_node(fdt, dpmac_id);
	if (offset < 0)
		return offset;

	/* verify if the node has a phy-handle */
	phyhandle = (u32 *)fdt_getprop(fdt, offset, "phy-handle", NULL);
	if (!phyhandle)
		return 0;

	return fdt_delprop(fdt, offset, "phy-handle");
}

int fdt_fixup_board_phy_revc(void *fdt)
{
	int ret;

	if (get_board_rev() < 'C')
		return 0;

	/* DPMACs 3,4 have their Aquantia PHYs at new addresses */
	ret = fdt_update_phy_addr(fdt, 3, AQR113C_PHY_ADDR1);
	if (ret)
		return ret;

	ret = fdt_update_phy_addr(fdt, 4, AQR113C_PHY_ADDR2);
	if (ret)
		return ret;

	/* There is no PHY for the DPMAC2, so remove the phy-handle */
	return fdt_delete_phy_handle(fdt, 2);
}
