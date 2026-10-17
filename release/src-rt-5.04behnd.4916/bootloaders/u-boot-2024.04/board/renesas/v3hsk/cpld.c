// SPDX-License-Identifier: GPL-2.0+
/*
 * V3HSK board CPLD access support
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Copyright (C) 2019 Cogent Embedded, Inc.
 *
 */

#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <linux/err.h>
#include <sysreset.h>
#include <command.h>

#define CPLD_ADDR_PRODUCT_0		0x0000 /* R */
#define CPLD_ADDR_PRODUCT_1		0x0001 /* R */
#define CPLD_ADDR_PRODUCT_2		0x0002 /* R */
#define CPLD_ADDR_PRODUCT_3		0x0003 /* R */
#define CPLD_ADDR_CPLD_VERSION_D	0x0004 /* R */
#define CPLD_ADDR_CPLD_VERSION_M	0x0005 /* R */
#define CPLD_ADDR_CPLD_VERSION_Y_0	0x0006 /* R */
#define CPLD_ADDR_CPLD_VERSION_Y_1	0x0007 /* R */
#define CPLD_ADDR_MODE_SET_0		0x0008 /* R */
#define CPLD_ADDR_MODE_SET_1		0x0009 /* R */
#define CPLD_ADDR_MODE_SET_2		0x000A /* R */
#define CPLD_ADDR_MODE_SET_3		0x000B /* R */
#define CPLD_ADDR_MODE_SET_4		0x000C /* R */
#define CPLD_ADDR_MODE_LAST_0		0x0018 /* R */
#define CPLD_ADDR_MODE_LAST_1		0x0019 /* R */
#define CPLD_ADDR_MODE_LAST_2		0x001A /* R */
#define CPLD_ADDR_MODE_LAST_3		0x001B /* R */
#define CPLD_ADDR_MODE_LAST_4		0x001C /* R */
#define CPLD_ADDR_DIPSW4		0x0020 /* R */
#define CPLD_ADDR_DIPSW5		0x0021 /* R */
#define CPLD_ADDR_RESET			0x0024 /* R/W */
#define CPLD_ADDR_POWER_CFG		0x0025 /* R/W */
#define CPLD_ADDR_PERI_CFG_0		0x0030 /* R/W */
#define CPLD_ADDR_PERI_CFG_1		0x0031 /* R/W */
#define CPLD_ADDR_PERI_CFG_2		0x0032 /* R/W */
#define CPLD_ADDR_PERI_CFG_3		0x0033 /* R/W */
#define CPLD_ADDR_LEDS			0x0034 /* R/W */
#define CPLD_ADDR_LEDS_CFG		0x0035 /* R/W */
#define CPLD_ADDR_UART_CFG		0x0036 /* R/W */
#define CPLD_ADDR_UART_STATUS		0x0037 /* R */

#define CPLD_ADDR_PCB_VERSION_0		0x1000 /* R */
#define CPLD_ADDR_PCB_VERSION_1		0x1001 /* R */
#define CPLD_ADDR_SOC_VERSION_0		0x1002 /* R */
#define CPLD_ADDR_SOC_VERSION_1		0x1003 /* R */
#define CPLD_ADDR_PCB_SN_0		0x1004 /* R */
#define CPLD_ADDR_PCB_SN_1		0x1005 /* R */

static u16 cpld_read(struct udevice *dev, u16 addr)
{
	u8 data[2];

	/* Random flash reads require 2 reads: first read is unreliable */
	if (addr >= CPLD_ADDR_PCB_VERSION_0)
		dm_i2c_read(dev, addr, data, 2);

	/* Only the second byte read is valid */
	dm_i2c_read(dev, addr, data, 2);
	return data[1];
}

static void cpld_write(struct udevice *dev, u16 addr, u8 data)
{
	dm_i2c_write(dev, addr, &data, 1);
}

static int do_cpld(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	u16 addr, val;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_SYSRESET,
					  DM_DRIVER_GET(sysreset_renesas_v3hsk),
					  &dev);
	if (ret)
		return ret;

	if (argc == 2 && strcmp(argv[1], "info") == 0) {
		printf("Product:                0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_PRODUCT_3) << 24) |
		       (cpld_read(dev, CPLD_ADDR_PRODUCT_2) << 16) |
		       (cpld_read(dev, CPLD_ADDR_PRODUCT_1) << 8) |
		       cpld_read(dev, CPLD_ADDR_PRODUCT_0));
		printf("CPLD version:           0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_CPLD_VERSION_Y_1) << 24) |
		       (cpld_read(dev, CPLD_ADDR_CPLD_VERSION_Y_0) << 16) |
		       (cpld_read(dev, CPLD_ADDR_CPLD_VERSION_M) << 8) |
		       cpld_read(dev, CPLD_ADDR_CPLD_VERSION_D));
		printf("Mode setting (MD0..26): 0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_MODE_LAST_3) << 24) |
		       (cpld_read(dev, CPLD_ADDR_MODE_LAST_2) << 16) |
		       (cpld_read(dev, CPLD_ADDR_MODE_LAST_1) << 8) |
		       cpld_read(dev, CPLD_ADDR_MODE_LAST_0));
		printf("DIPSW (SW4, SW5):       0x%02x, 0x%x\n",
		       cpld_read(dev, CPLD_ADDR_DIPSW4) ^ 0xff,
		       (cpld_read(dev, CPLD_ADDR_DIPSW5) ^ 0xff) & 0xf);
		printf("Power config:           0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_POWER_CFG));
		printf("Periferals config:      0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_PERI_CFG_3) << 24) |
		       (cpld_read(dev, CPLD_ADDR_PERI_CFG_2) << 16) |
		       (cpld_read(dev, CPLD_ADDR_PERI_CFG_1) << 8) |
		       cpld_read(dev, CPLD_ADDR_PERI_CFG_0));
		printf("PCB version:            %d.%d\n",
		       cpld_read(dev, CPLD_ADDR_PCB_VERSION_1),
		       cpld_read(dev, CPLD_ADDR_PCB_VERSION_0));
		printf("SOC version:            %d.%d\n",
		       cpld_read(dev, CPLD_ADDR_SOC_VERSION_1),
		       cpld_read(dev, CPLD_ADDR_SOC_VERSION_0));
		printf("PCB S/N:                %d\n",
		       (cpld_read(dev, CPLD_ADDR_PCB_SN_1) << 8) |
		       cpld_read(dev, CPLD_ADDR_PCB_SN_0));
		return 0;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], NULL, 16);
	if (!(addr >= CPLD_ADDR_PRODUCT_0 && addr <= CPLD_ADDR_UART_STATUS)) {
		printf("cpld invalid addr\n");
		return CMD_RET_USAGE;
	}

	if (argc == 3 && strcmp(argv[1], "read") == 0) {
		printf("0x%x\n", cpld_read(dev, addr));
	} else if (argc == 4 && strcmp(argv[1], "write") == 0) {
		val = simple_strtoul(argv[3], NULL, 16);
		cpld_write(dev, addr, val);
	}

	return 0;
}

U_BOOT_CMD(cpld, 4, 1, do_cpld,
	   "CPLD access",
	   "info\n"
	   "cpld read addr\n"
	   "cpld write addr val\n"
);

static int renesas_v3hsk_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	cpld_write(dev, CPLD_ADDR_RESET, 1);

	return -EINPROGRESS;
}

static int renesas_v3hsk_sysreset_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return 0;
}

static struct sysreset_ops renesas_v3hsk_sysreset = {
	.request	= renesas_v3hsk_sysreset_request,
};

static const struct udevice_id renesas_v3hsk_sysreset_ids[] = {
	{ .compatible = "renesas,v3hsk-cpld" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysreset_renesas_v3hsk) = {
	.name		= "renesas_v3hsk_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &renesas_v3hsk_sysreset,
	.probe		= renesas_v3hsk_sysreset_probe,
	.of_match	= renesas_v3hsk_sysreset_ids,
};
