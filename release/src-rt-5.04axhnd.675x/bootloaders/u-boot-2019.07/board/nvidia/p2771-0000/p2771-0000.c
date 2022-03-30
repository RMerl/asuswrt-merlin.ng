// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION
 */

#include <common.h>
#include <environment.h>
#include <fdtdec.h>
#include <i2c.h>
#include <linux/libfdt.h>
#include <asm/arch-tegra/cboot.h>
#include "../p2571/max77620_init.h"

void pin_mux_mmc(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO3 to 3.3V for SD card power */
	debug("%s: Set LDO3 for VDDIO_SDMMC_AP power to 3.3V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return;
	}
	/* 0xF2 for 3.3v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xF2;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L3_REG, &val, 1);
	if (ret) {
		printf("i2c_write 0 0x3c 0x27 failed: %d\n", ret);
		return;
	}
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO7 to 1.05V for PEX power */
	debug("%s: Set LDO7 for PEX power to 1.05V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return -1;
	}
	/* 0xC5 for 1.05v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xC5;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L7_REG, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x31 failed: %d\n", ret);

	return 0;
}
#endif

static void ft_mac_address_setup(void *fdt)
{
	const void *cboot_fdt = (const void *)cboot_boot_x0;
	uint8_t mac[ETH_ALEN], local_mac[ETH_ALEN];
	const char *path;
	int offset, err;

	err = cboot_get_ethaddr(cboot_fdt, local_mac);
	if (err < 0)
		memset(local_mac, 0, ETH_ALEN);

	path = fdt_get_alias(fdt, "ethernet");
	if (!path)
		return;

	debug("ethernet alias found: %s\n", path);

	offset = fdt_path_offset(fdt, path);
	if (offset < 0) {
		printf("ethernet alias points to absent node %s\n", path);
		return;
	}

	if (is_valid_ethaddr(local_mac)) {
		err = fdt_setprop(fdt, offset, "local-mac-address", local_mac,
				  ETH_ALEN);
		if (!err)
			debug("Local MAC address set: %pM\n", local_mac);
	}

	if (eth_env_get_enetaddr("ethaddr", mac)) {
		if (memcmp(local_mac, mac, ETH_ALEN) != 0) {
			err = fdt_setprop(fdt, offset, "mac-address", mac,
					  ETH_ALEN);
			if (!err)
				debug("MAC address set: %pM\n", mac);
		}
	}
}

static int ft_copy_carveout(void *dst, const void *src, const char *node)
{
	struct fdt_memory fb;
	int err;

	err = fdtdec_get_carveout(src, node, "memory-region", 0, &fb);
	if (err < 0) {
		if (err != -FDT_ERR_NOTFOUND)
			printf("failed to get carveout for %s: %d\n", node,
			       err);

		return err;
	}

	err = fdtdec_set_carveout(dst, node, "memory-region", 0, "framebuffer",
				  &fb);
	if (err < 0) {
		printf("failed to set carveout for %s: %d\n", node, err);
		return err;
	}

	return 0;
}

static void ft_carveout_setup(void *fdt)
{
	const void *cboot_fdt = (const void *)cboot_boot_x0;
	static const char * const nodes[] = {
		"/host1x@13e00000/display-hub@15200000/display@15200000",
		"/host1x@13e00000/display-hub@15200000/display@15210000",
		"/host1x@13e00000/display-hub@15200000/display@15220000",
	};
	unsigned int i;
	int err;

	for (i = 0; i < ARRAY_SIZE(nodes); i++) {
		printf("copying carveout for %s...\n", nodes[i]);

		err = ft_copy_carveout(fdt, cboot_fdt, nodes[i]);
		if (err < 0) {
			if (err != -FDT_ERR_NOTFOUND)
				printf("failed to copy carveout for %s: %d\n",
				       nodes[i], err);

			continue;
		}
	}
}

int ft_board_setup(void *fdt, bd_t *bd)
{
	ft_mac_address_setup(fdt);
	ft_carveout_setup(fdt);

	return 0;
}
