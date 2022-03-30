/*
 * Freescale SGMII Riser Card
 *
 * This driver supports the SGMII Riser card found on the
 * "DS" style of development board from Freescale.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <linux/libfdt.h>
#include <tsec.h>
#include <fdt_support.h>

void fsl_sgmii_riser_init(struct tsec_info_struct *tsec_info, int num)
{
	int i;

	for (i = 0; i < num; i++)
		if (tsec_info[i].flags & TSEC_SGMII)
			tsec_info[i].phyaddr += SGMII_RISER_PHY_OFFSET;
}

void fsl_sgmii_riser_fdt_fixup(void *fdt)
{
	struct eth_device *dev;
	int node;
	int mdio_node;
	int i = -1;
	int etsec_num = 0;

	node = fdt_path_offset(fdt, "/aliases");
	if (node < 0)
		return;

	while ((dev = eth_get_dev_by_index(++i)) != NULL) {
		struct tsec_private *priv;
		int phy_node;
		int enet_node;
		uint32_t ph;
		char sgmii_phy[16];
		char enet[16];
		const u32 *phyh;
		const char *model;
		const char *path;

		if (!strstr(dev->name, "eTSEC"))
			continue;

		priv = dev->priv;
		if (!(priv->flags & TSEC_SGMII)) {
			etsec_num++;
			continue;
		}

		mdio_node = fdt_node_offset_by_compatible(fdt, -1,
				"fsl,gianfar-mdio");
		if (mdio_node < 0)
			return;

		sprintf(sgmii_phy, "sgmii-phy@%d", etsec_num);
		phy_node = fdt_subnode_offset(fdt, mdio_node, sgmii_phy);
		if (phy_node > 0) {
			fdt_increase_size(fdt, 32);
			ph = fdt_create_phandle(fdt, phy_node);
			if (!ph)
				continue;
		}

		sprintf(enet, "ethernet%d", etsec_num++);
		path = fdt_getprop(fdt, node, enet, NULL);
		if (!path) {
			debug("No alias for %s\n", enet);
			continue;
		}

		enet_node = fdt_path_offset(fdt, path);
		if (enet_node < 0)
			continue;

		model = fdt_getprop(fdt, enet_node, "model", NULL);

		/*
		 * We only want to do this to eTSECs.  On some platforms
		 * there are more than one type of gianfar-style ethernet
		 * controller, and as we are creating an implicit connection
		 * between ethernet nodes and eTSEC devices, it is best to
		 * make the connection use as much explicit information
		 * as exists.
		 */
		if (!strstr(model, "TSEC"))
			continue;

		if (phy_node < 0) {
			/*
			 * This part is only for old device tree without
			 * sgmii_phy nodes. It's kept just for compatible
			 * reason. Soon to be deprecated if all device tree
			 * get updated.
			 */
			phyh = fdt_getprop(fdt, enet_node, "phy-handle", NULL);
			if (!phyh)
				continue;

			phy_node = fdt_node_offset_by_phandle(fdt,
					fdt32_to_cpu(*phyh));

			priv = dev->priv;

			if (priv->flags & TSEC_SGMII)
				fdt_setprop_cell(fdt, phy_node, "reg",
						priv->phyaddr);
		} else {
			fdt_setprop(fdt, enet_node, "phy-handle", &ph,
					sizeof(ph));
			fdt_setprop_string(fdt, enet_node,
					"phy-connection-type",
					phy_string_for_interface(
						PHY_INTERFACE_MODE_SGMII));
		}
	}
}
