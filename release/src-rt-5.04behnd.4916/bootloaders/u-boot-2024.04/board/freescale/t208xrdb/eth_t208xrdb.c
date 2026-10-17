// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <net.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <asm/fsl_serdes.h>

extern u8 get_hw_revision(void);

/* Disable the MAC5 and MAC6 "fsl,fman-memac" nodes and the two
 * "fsl,dpa-ethernet" nodes that reference them.
 */
void fdt_fixup_board_fman_ethernet(void *fdt)
{
	int mac_off, eth_off, i;
	char mac_path[2][42] = {
		"/soc@ffe000000/fman@400000/ethernet@e8000",
		"/soc@ffe000000/fman@400000/ethernet@ea000",
	};
	u32 eth_ph;

	for (i = 0; i < 2; i++) {
		/* Disable the MAC node */
		mac_off = fdt_path_offset(fdt, mac_path[i]);
		if (mac_off < 0)
			continue;
		fdt_status_disabled(fdt, mac_off);

		/* Disable the fsl,dpa-ethernet node that points to the MAC.
		 * The fsl,fman-mac property refers to the MAC's phandle.
		 */
		eth_ph = fdt_get_phandle(fdt, mac_off);
		if (eth_ph <= 0)
			continue;

		eth_off = fdt_node_offset_by_prop_value(fdt, -1, "fsl,fman-mac",
							&eth_ph,
							sizeof(eth_ph));
		if (eth_off >= 0)
			fdt_status_disabled(fdt, eth_off);
	}
}

/* Update the address of the second Aquantia PHY on boards revision D and up.
 * Also rename the PHY node to align with the address change.
 */
void fdt_fixup_board_phy(void *fdt)
{
	const char phy_path[] =
		"/soc@ffe000000/fman@400000/mdio@fd000/ethernet-phy@1";
	int ret, offset, new_addr = AQR113C_PHY_ADDR2;
	char new_name[] = "ethernet-phy@00";

	if (get_hw_revision() == 'C')
		return;

	offset = fdt_path_offset(fdt, phy_path);
	if (offset < 0) {
		printf("ethernet-phy@1 node not found in the dts\n");
		return;
	}

	ret = fdt_setprop(fdt, offset, "reg", &new_addr, sizeof(new_addr));
	if (ret < 0) {
		printf("Unable to set 'reg' for node ethernet-phy@1: %s\n",
		       fdt_strerror(ret));
		return;
	}

	sprintf(new_name, "ethernet-phy@%x", new_addr);
	ret = fdt_set_name(fdt, offset, new_name);
	if (ret < 0)
		printf("Unable to rename node ethernet-phy@1: %s\n",
		       fdt_strerror(ret));
}

void fdt_fixup_board_enet(void *fdt)
{
	return;
}
