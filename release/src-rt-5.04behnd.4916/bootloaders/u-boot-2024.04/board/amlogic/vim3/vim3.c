// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <env_internal.h>
#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/boot.h>
#include <asm/arch/eth.h>
#include <asm/arch/sm.h>
#include <asm/global_data.h>
#include <i2c.h>
#include "khadas-mcu.h"

int mmc_get_env_dev(void)
{
	switch (meson_get_boot_device()) {
	case BOOT_DEVICE_EMMC:
		return 2;
	case BOOT_DEVICE_SD:
		return 1;
	default:
		/* boot device is not EMMC|SD */
		return -1;
	}
}

/*
 * The VIM3 on-board  MCU can mux the PCIe/USB3.0 shared differential
 * lines using a FUSB340TMX USB 3.1 SuperSpeed Data Switch between
 * an USB3.0 Type A connector and a M.2 Key M slot.
 * The PHY driving these differential lines is shared between
 * the USB3.0 controller and the PCIe Controller, thus only
 * a single controller can use it.
 */
int meson_ft_board_setup(void *blob, struct bd_info *bd)
{
	struct udevice *bus, *dev;
	int node, i2c_node, ret;
	unsigned int i2c_addr;
	u32 *val;

	/* Find I2C device */
	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1, "khadas,mcu");
	if (node < 0) {
		printf("vim3: cannot find khadas,mcu node\n");
		return 0;
	}

	/* Get addr */
	val = (u32 *)fdt_getprop(gd->fdt_blob, node, "reg", NULL);
	if (!val) {
		printf("vim3: cannot find khadas,mcu node i2c addr\n");
		return 0;
	}
	i2c_addr = fdt32_to_cpu(*val);

	/* Get i2c device */
	i2c_node = fdt_parent_offset(gd->fdt_blob, node);
	if (node < 0) {
		printf("vim3: cannot find khadas,mcu i2c node\n");
		return 0;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C, i2c_node, &bus);
	if (ret < 0) {
		printf("vim3: cannot find i2c bus (%d)\n", ret);
		return 0;
	}

	ret = i2c_get_chip(bus, i2c_addr, 1, &dev);
	if (ret < 0) {
		printf("vim3: cannot find i2c chip (%d)\n", ret);
		return 0;
	}

	/* Read USB_PCIE_SWITCH_REG */
	ret = dm_i2c_reg_read(dev, KHADAS_MCU_USB_PCIE_SWITCH_REG);
	if (ret < 0) {
		printf("vim3: failed to read i2c reg (%d)\n", ret);
		return 0;
	}
	debug("MCU_USB_PCIE_SWITCH_REG: %d\n", ret);

	/*
	 * If in PCIe mode, alter DT
	 * 0：Enable USB3.0，Disable PCIE, 1：Disable USB3.0, Enable PCIE
	 */
	if (ret > 0) {
		static char data[32] __aligned(4);
		const void *ptmp;
		int len;

		/* Find USB node */
		node = fdt_node_offset_by_compatible(blob, -1, "amlogic,meson-g12a-usb-ctrl");
		if (node < 0) {
			printf("vim3: cannot find amlogic,meson-g12a-usb-ctrl node\n");
			return 0;
		}

		/* Update PHY names (mandatory to disable USB3.0) */
		len = strlcpy(data, "usb2-phy0", 32) + 1;
		len += strlcpy(&data[len], "usb2-phy1", 32 - len) + 1;
		ret = fdt_setprop(blob, node, "phy-names", data, len);
		if (ret < 0) {
			printf("vim3: failed to update usb phy names property (%d)\n", ret);
			return 0;
		}

		/* Update PHY list, by keeping the 2 first entries (optional) */
		ptmp = fdt_getprop(blob, node, "phys", &len);
		if (ptmp) {
			memcpy(data, ptmp, min_t(unsigned int, 2 * sizeof(u32), len));

			ret = fdt_setprop(blob, node, "phys", data,
					  min_t(unsigned int, 2 * sizeof(u32), len));
			if (ret < 0)
				printf("vim3: failed to update usb phys property (%d)\n", ret);
		} else
			printf("vim3: cannot find usb node phys property\n");

		/* Find PCIe node */
		node = fdt_node_offset_by_compatible(blob, -1, "amlogic,g12a-pcie");
		if (node < 0) {
			printf("vim3: cannot find amlogic,g12a-pcie node\n");
			return 0;
		}

		/* Enable PCIe */
		len = strlcpy(data, "okay", 32) + 1;
		ret = fdt_setprop(blob, node, "status", data, len);
		if (ret < 0) {
			printf("vim3: failed to enable pcie node (%d)\n", ret);
			return 0;
		}

		printf("vim3: successfully enabled PCIe\n");
	}

	return 0;
}

#define EFUSE_MAC_OFFSET	0
#define EFUSE_MAC_SIZE		12
#define MAC_ADDR_LEN		6

int misc_init_r(void)
{
	u8 mac_addr[MAC_ADDR_LEN + 1];
	char efuse_mac_addr[EFUSE_MAC_SIZE], tmp[3];
	char serial_string[EFUSE_MAC_SIZE + 1];
	ssize_t len;

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  efuse_mac_addr, EFUSE_MAC_SIZE);
		if (len != EFUSE_MAC_SIZE)
			return 0;

		/* MAC is stored in ASCII format, 1bytes = 2characters */
		for (int i = 0; i < 6; i++) {
			tmp[0] = efuse_mac_addr[i * 2];
			tmp[1] = efuse_mac_addr[i * 2 + 1];
			tmp[2] = '\0';
			mac_addr[i] = hextoul(tmp, NULL);
		}
		mac_addr[MAC_ADDR_LEN] = '\0';

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else
			meson_generate_serial_ethaddr();

		eth_env_get_enetaddr("ethaddr", mac_addr);
	}

	if (!env_get("serial#")) {
		eth_env_get_enetaddr("ethaddr", mac_addr);
		sprintf(serial_string, "%02X%02X%02X%02X%02X%02X",
			mac_addr[0], mac_addr[1], mac_addr[2],
			mac_addr[3], mac_addr[4], mac_addr[5]);
		env_set("serial#", serial_string);
	}

	return 0;
}
