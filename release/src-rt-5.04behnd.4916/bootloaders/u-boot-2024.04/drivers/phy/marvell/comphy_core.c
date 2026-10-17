// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 *
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/libfdt.h>

#include "comphy_core.h"

#define COMPHY_MAX_CHIP 4

DECLARE_GLOBAL_DATA_PTR;

static const char *get_speed_string(u32 speed)
{
	static const char * const speed_strings[] = {
		"1.25 Gbps", "2.5 Gbps", "3.125 Gbps",
		"5 Gbps", "5.125 Gpbs", "6 Gbps",
		"10.3125 Gbps"
	};

	if (speed < 0 || speed > COMPHY_SPEED_MAX)
		return "invalid";

	return speed_strings[speed];
}

static const char *get_type_string(u32 type)
{
	static const char * const type_strings[] = {
		"UNCONNECTED", "PEX0", "PEX1", "PEX2", "PEX3",
		"SATA0", "SATA1", "SGMII0", "SGMII1", "SGMII2",
		"USB3", "USB3_HOST0", "USB3_HOST1",
		"USB3_DEVICE", "RXAUI0", "RXAUI1", "SFI0", "SFI1", "AP",
		"IGNORE"
	};

	if (type < 0 || type > COMPHY_TYPE_MAX)
		return "invalid";

	return type_strings[type];
}

void comphy_print(struct chip_serdes_phy_config *chip_cfg,
		  struct comphy_map *comphy_map_data)
{
	u32 lane;

	for (lane = 0; lane < chip_cfg->comphy_lanes_count;
	     lane++, comphy_map_data++) {
		if (comphy_map_data->speed == COMPHY_SPEED_INVALID) {
			printf("Comphy-%d: %-13s\n", lane,
			       get_type_string(comphy_map_data->type));
		} else {
			printf("Comphy-%d: %-13s %-10s\n", lane,
			       get_type_string(comphy_map_data->type),
			       get_speed_string(comphy_map_data->speed));
		}
	}
}

int comphy_rx_training(struct udevice *dev, u32 lane)
{
	struct chip_serdes_phy_config *chip_cfg = dev_get_priv(dev);

	if (chip_cfg->rx_training)
		return chip_cfg->rx_training(chip_cfg, lane);

	return 0;
}

static int comphy_probe(struct udevice *dev)
{
	int node = dev_of_offset(dev);
	struct chip_serdes_phy_config *chip_cfg = dev_get_priv(dev);
	int last_idx = 0;
	static int current_idx;
	int res;

	/* Save base addresses for later use */
	chip_cfg->comphy_base_addr = devfdt_get_addr_index_ptr(dev, 0);
	if (!chip_cfg->comphy_base_addr)
		return -EINVAL;

	chip_cfg->hpipe3_base_addr = devfdt_get_addr_index_ptr(dev, 1);
	if (!chip_cfg->hpipe3_base_addr)
		return -EINVAL;

	if (device_is_compatible(dev, "marvell,comphy-a3700")) {
		chip_cfg->comphy_init_map = comphy_a3700_init_serdes_map;
		chip_cfg->ptr_comphy_chip_init = comphy_a3700_init;
		chip_cfg->rx_training = NULL;
	}

	if (device_is_compatible(dev, "marvell,comphy-cp110")) {
		chip_cfg->comphy_init_map = comphy_cp110_init_serdes_map;
		chip_cfg->ptr_comphy_chip_init = comphy_cp110_init;
		chip_cfg->rx_training = comphy_cp110_sfi_rx_training;
	}

	/*
	 * Bail out if no chip_init function is defined, e.g. no
	 * compatible node is found
	 */
	if (!chip_cfg->ptr_comphy_chip_init) {
		dev_err(dev, "comphy: No compatible DT node found\n");
		return -ENODEV;
	}

	res = chip_cfg->comphy_init_map(node, chip_cfg);
	if (res < 0)
		return res;

	/* Save CP index for MultiCP devices (A8K) */
	chip_cfg->cp_index = current_idx++;
	/* PHY power UP sequence */
	chip_cfg->ptr_comphy_chip_init(chip_cfg, chip_cfg->comphy_map_data);
	/* PHY print SerDes status */
	printf("Comphy chip #%d:\n", chip_cfg->cp_index);
	comphy_print(chip_cfg, chip_cfg->comphy_map_data);

	/*
	 * Only run the dedicated PHY init code once, in the last PHY init call
	 */
	if (of_machine_is_compatible("marvell,armada8040"))
		last_idx = 1;

	if (chip_cfg->cp_index == last_idx) {
		/* Initialize dedicated PHYs (not muxed SerDes lanes) */
		comphy_dedicated_phys_init();
	}

	return 0;
}

static const struct udevice_id comphy_ids[] = {
	{ .compatible = "marvell,mvebu-comphy" },
	{ .compatible = "marvell,comphy-a3700" },
	{ }
};

U_BOOT_DRIVER(mvebu_comphy) = {
	.name	= "mvebu_comphy",
	.id	= UCLASS_MISC,
	.of_match = comphy_ids,
	.probe	= comphy_probe,
	.priv_auto	= sizeof(struct chip_serdes_phy_config),
};
