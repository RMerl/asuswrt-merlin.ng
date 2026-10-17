// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_SYSREG_SOFT_RESET		((unsigned int *)0x20002088)
#define MPFS_SYS_SERVICE_CR		((unsigned int *)0x37020050)
#define MPFS_SYS_SERVICE_SR		((unsigned int *)0x37020054)
#define MPFS_SYS_SERVICE_MAILBOX	((unsigned char *)0x37020800)

#define PERIPH_RESET_VALUE		0x1e8u
#define SERVICE_CR_REQ			0x1u
#define SERVICE_SR_BUSY			0x2u

static void read_device_serial_number(u8 *response, u8 response_size)
{
	u8 idx;
	u8 *response_buf;
	unsigned int val;

	response_buf = (u8 *)response;

	writel(SERVICE_CR_REQ, MPFS_SYS_SERVICE_CR);
	/*
	 * REQ bit will remain set till the system controller starts
	 * processing.
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_CR);
	} while (SERVICE_CR_REQ == (val & SERVICE_CR_REQ));

	/*
	 * Once system controller starts processing the busy bit will
	 * go high and service is completed when busy bit is gone low
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_SR);
	} while (SERVICE_SR_BUSY == (val & SERVICE_SR_BUSY));

	for (idx = 0; idx < response_size; idx++)
		response_buf[idx] = readb(MPFS_SYS_SERVICE_MAILBOX + idx);
}

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}

int board_early_init_f(void)
{
	unsigned int val;

	/* Reset uart, mmc peripheral */
	val = readl(MPFS_SYSREG_SOFT_RESET);
	val = (val & ~(PERIPH_RESET_VALUE));
	writel(val, MPFS_SYSREG_SOFT_RESET);

	return 0;
}

int board_late_init(void)
{
	u32 ret;
	u32 node;
	u8 idx;
	u8 device_serial_number[16] = { 0 };
	unsigned char mac_addr[6];
	char icicle_mac_addr[20];
	void *blob = (void *)gd->fdt_blob;

	node = fdt_path_offset(blob, "/soc/ethernet@20112000");
	if (node < 0) {
		printf("No ethernet0 path offset\n");
		return -ENODEV;
	}

	ret = fdtdec_get_byte_array(blob, node, "local-mac-address", mac_addr, 6);
	if (ret) {
		printf("No local-mac-address property for ethernet@20112000\n");
		return -EINVAL;
	}

	read_device_serial_number(device_serial_number, 16);

	/* Update MAC address with device serial number */
	mac_addr[0] = 0x00;
	mac_addr[1] = 0x04;
	mac_addr[2] = 0xA3;
	mac_addr[3] = device_serial_number[2];
	mac_addr[4] = device_serial_number[1];
	mac_addr[5] = device_serial_number[0];

	ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
	if (ret) {
		printf("Error setting local-mac-address property for ethernet@20112000\n");
		return -ENODEV;
	}

	icicle_mac_addr[0] = '[';

	sprintf(&icicle_mac_addr[1], "%pM", mac_addr);

	icicle_mac_addr[18] = ']';
	icicle_mac_addr[19] = '\0';

	for (idx = 0; idx < 20; idx++) {
		if (icicle_mac_addr[idx] == ':')
			icicle_mac_addr[idx] = ' ';
	}
	env_set("icicle_mac_addr0", icicle_mac_addr);

	mac_addr[5] = device_serial_number[0] + 1;

	node = fdt_path_offset(blob, "/soc/ethernet@20110000");
	if (node >= 0) {
		ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
		if (ret) {
			printf("Error setting local-mac-address property for ethernet@20110000\n");
			return -ENODEV;
		}
	}

	icicle_mac_addr[0] = '[';

	sprintf(&icicle_mac_addr[1], "%pM", mac_addr);

	icicle_mac_addr[18] = ']';
	icicle_mac_addr[19] = '\0';

	for (idx = 0; idx < 20; idx++) {
		if (icicle_mac_addr[idx] == ':')
			icicle_mac_addr[idx] = ' ';
	}
	env_set("icicle_mac_addr1", icicle_mac_addr);

	return 0;
}
