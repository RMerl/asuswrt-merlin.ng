// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <fdt_support.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/fuse.h>

int nvidia_board_init(void)
{
	/* Set up panel bridge clocks */
	clock_start_periph_pll(PERIPH_ID_EXTPERIPH3, CLOCK_ID_PERIPH,
			       24 * 1000000);
	clock_external_output(3);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* First 3 bytes refer to LG vendor */
	u8 btmacaddr[6] = { 0x00, 0x00, 0x00, 0xD0, 0xC9, 0x88 };

	/* Generate device 3 bytes based on chip sd */
	u64 bt_device = tegra_chip_uid() >> 24ull;

	btmacaddr[0] =  (bt_device >> 1 & 0x0F) |
			(bt_device >> 5 & 0xF0);
	btmacaddr[1] =  (bt_device >> 11 & 0x0F) |
			(bt_device >> 17 & 0xF0);
	btmacaddr[2] =  (bt_device >> 23 & 0x0F) |
			(bt_device >> 29 & 0xF0);

	/* Set BT MAC address */
	fdt_find_and_setprop(blob, "/serial@70006200/bluetooth",
			     "local-bd-address", btmacaddr, 6, 1);

	/* Remove TrustZone nodes */
	fdt_del_node_and_alias(blob, "/firmware");
	fdt_del_node_and_alias(blob, "/reserved-memory/trustzone@bfe00000");

	return 0;
}
#endif
