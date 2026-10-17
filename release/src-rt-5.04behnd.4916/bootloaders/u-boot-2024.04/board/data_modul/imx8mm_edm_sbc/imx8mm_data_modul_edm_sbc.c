// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/bitfield.h>
#include <malloc.h>
#include <spl.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	dmo_setup_boot_device();
	dmo_setup_mac_address();

	ret = uclass_get_device_by_name(UCLASS_MISC, "usb-hub@2c", &dev);
	if (ret)
		printf("Error bringing up USB hub (%d)\n", ret);

	return 0;
}

int fdtdec_board_setup(const void *fdt_blob)
{
	const void __iomem *mux = (void __iomem *)IOMUXC_BASE_ADDR +
		FIELD_GET(MUX_CTRL_OFS_MASK, IMX8MM_PAD_ENET_MDC_GPIO1_IO16);
	const char *phy_compat = "ethernet-phy-ieee802.3-c22";
	bool is_bcmphy;
	int phy_node;
	int ret;

	/* Do nothing if not i.MX8MM eDM SBC */
	ret = fdt_node_check_compatible(fdt_blob, 0, "dmo,imx8mm-data-modul-edm-sbc");
	if (ret)
		return 0;

	/*
	 * If GPIO1_16 RGMII_MDC is HIGH, then R530 is populated.
	 * R530 is populated only on boards with AR8031 PHY.
	 *
	 * If GPIO1_16 RGMII_MDC is LOW, then the in-SoM pull down
	 * is the dominant pull resistor. This is the case on boards
	 * with BCM54213PE PHY.
	 */
	setbits_le32(mux, IOMUX_CONFIG_SION);
	is_bcmphy = !(readl(GPIO1_BASE_ADDR) & BIT(16));
	clrbits_le32(mux, IOMUX_CONFIG_SION);

	phy_node = fdt_node_offset_by_compatible(fdt_blob, -1, phy_compat);
	if (phy_node < 0)
		return 0;

	/*
	 * Update PHY MDC address in control DT based on the populated
	 * PHY type. AR8031 is at address 0, BCM54213PE is at address 1.
	 */
	fdt_setprop_inplace_u32((void *)fdt_blob, phy_node,
				"reg", is_bcmphy ? 1 : 0);

	return 0;
}
