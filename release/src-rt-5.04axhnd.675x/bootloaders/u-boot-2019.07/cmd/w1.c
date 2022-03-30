/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2018
 * Microchip Technology, Inc.
 * Eugen Hristev <eugen.hristev@microchip.com>
 */
#include <common.h>
#include <command.h>
#include <w1.h>
#include <w1-eeprom.h>
#include <dm/device-internal.h>

static int w1_bus(void)
{
	struct udevice *bus, *dev;
	int ret;

	ret = w1_get_bus(0, &bus);
	if (ret) {
		printf("one wire interface not found\n");
		return CMD_RET_FAILURE;
	}
	printf("Bus %d:\t%s", bus->seq, bus->name);
	if (device_active(bus))
		printf("  (active)");
	printf("\n");

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		ret = device_probe(dev);

		printf("\t%s (%d) uclass %s : ", dev->name, dev->seq,
		       dev->uclass->uc_drv->name);

		if (ret)
			printf("device error\n");
		else
			printf("family 0x%x\n", w1_get_device_family(dev));
	}
	return CMD_RET_SUCCESS;
}

static int w1_read(int argc, char *const argv[])
{
	int bus_n = 0, dev_n = 0, offset = 0, len = 512;
	int i;
	struct udevice *bus, *dev;
	int ret;
	u8 buf[512];

	if (argc > 2)
		bus_n = simple_strtoul(argv[2], NULL, 10);

	if (argc > 3)
		dev_n = simple_strtoul(argv[3], NULL, 10);

	if (argc > 4)
		offset = simple_strtoul(argv[4], NULL, 10);

	if (argc > 5)
		len = simple_strtoul(argv[5], NULL, 10);

	if (len > 512) {
		printf("len needs to be <= 512\n");
		return CMD_RET_FAILURE;
	}

	ret = w1_get_bus(bus_n, &bus);
	if (ret) {
		printf("one wire interface not found\n");
		return CMD_RET_FAILURE;
	}

	for (device_find_first_child(bus, &dev), i = 0;
	   dev && i <= dev_n;
	   device_find_next_child(&dev), i++) {
		ret = device_probe(dev);
		if (!ret && i == dev_n)
			break;
	}

	if (i != dev_n || ret || !dev) {
		printf("invalid dev\n");
		return CMD_RET_FAILURE;
	}

	if (strcmp(dev->uclass->uc_drv->name, "w1_eeprom")) {
		printf("the device present on the interface is of unknown device class\n");
		return CMD_RET_FAILURE;
	}

	ret = w1_eeprom_read_buf(dev, offset, (u8 *)buf, len);
	if (ret) {
		printf("error reading device %s\n", dev->name);
		return CMD_RET_FAILURE;
	}

	for (i = 0; i < len; i++)
		printf("%x", buf[i]);
	printf("\n");

	return CMD_RET_SUCCESS;
}

int do_w1(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "bus"))
		return w1_bus();

	if (!strcmp(argv[1], "read"))
		return w1_read(argc, argv);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(w1, 6, 0, do_w1,
	   "onewire interface utility commands",
	   "bus - show onewire bus info (all)\n"
	   "w1 read [<bus> [<dev> [offset [length]]]]"
	   "    - read from onewire device 'dev' on onewire bus 'bus'"
	   " starting from offset 'offset' and length 'length'\n"
	   "      defaults: bus 0, dev 0, offset 0, length 512 bytes.");
