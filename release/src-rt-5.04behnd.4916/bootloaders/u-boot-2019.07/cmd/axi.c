// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * (C) Copyright 2017, 2018
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <axi.h>
#include <command.h>
#include <console.h>
#include <dm.h>

/* Currently selected AXI bus device */
static struct udevice *axi_cur_bus;
/* Transmission size from last command */
static uint dp_last_size;
/* Address from last command */
static uint dp_last_addr;
/* Number of bytes to display from last command; default = 64 */
static uint dp_last_length = 0x40;

/**
 * show_bus() - Show devices on a single AXI bus
 * @bus: The AXI bus device to printt information for
 */
static void show_bus(struct udevice *bus)
{
	struct udevice *dev;

	printf("Bus %d:\t%s", bus->req_seq, bus->name);
	if (device_active(bus))
		printf("  (active %d)", bus->seq);
	printf("\n");
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev))
		printf("  %s\n", dev->name);
}

/**
 * axi_set_cur_bus() - Set the currently active AXI bus
 * @busnum: The number of the bus (i.e. its sequence number) that should be
 *	    made active
 *
 * The operations supplied by this command operate only on the currently active
 * bus.
 *
 * Return: 0 if OK, -ve on error
 */
static int axi_set_cur_bus(unsigned int busnum)
{
	struct udevice *bus;
	struct udevice *dummy;
	int ret;

	/* Make sure that all sequence numbers are initialized */
	for (uclass_first_device(UCLASS_AXI, &dummy);
	     dummy;
	     uclass_next_device(&dummy))
		;

	ret = uclass_get_device_by_seq(UCLASS_AXI, busnum, &bus);
	if (ret) {
		debug("%s: No bus %d\n", __func__, busnum);
		return ret;
	}
	axi_cur_bus = bus;

	return 0;
}

/**
 * axi_get_cur_bus() - Retrieve the currently active AXI bus device
 * @busp: Pointer to a struct udevice that receives the currently active bus
 *	  device
 *
 * Return: 0 if OK, -ve on error
 */
static int axi_get_cur_bus(struct udevice **busp)
{
	if (!axi_cur_bus) {
		puts("No AXI bus selected\n");
		return -ENODEV;
	}
	*busp = axi_cur_bus;

	return 0;
}

/*
 * Command handlers
 */

static int do_axi_show_bus(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	struct udevice *dummy;

	/* Make sure that all sequence numbers are initialized */
	for (uclass_first_device(UCLASS_AXI, &dummy);
	     dummy;
	     uclass_next_device(&dummy))
		;

	if (argc == 1) {
		/* show all busses */
		struct udevice *bus;

		for (uclass_first_device(UCLASS_AXI, &bus);
		     bus;
		     uclass_next_device(&bus))
			show_bus(bus);
	} else {
		int i;

		/* show specific bus */
		i = simple_strtoul(argv[1], NULL, 10);

		struct udevice *bus;
		int ret;

		ret = uclass_get_device_by_seq(UCLASS_AXI, i, &bus);
		if (ret) {
			printf("Invalid bus %d: err=%d\n", i, ret);
			return CMD_RET_FAILURE;
		}
		show_bus(bus);
	}

	return 0;
}

static int do_axi_bus_num(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	int ret = 0;
	int bus_no;

	if (argc == 1) {
		/* querying current setting */
		struct udevice *bus;

		if (!axi_get_cur_bus(&bus))
			bus_no = bus->seq;
		else
			bus_no = -1;

		printf("Current bus is %d\n", bus_no);
	} else {
		bus_no = simple_strtoul(argv[1], NULL, 10);
		printf("Setting bus to %d\n", bus_no);

		ret = axi_set_cur_bus(bus_no);
		if (ret)
			printf("Failure changing bus number (%d)\n", ret);
	}

	return ret ? CMD_RET_FAILURE : 0;
}

static int do_axi_md(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* Print that many bytes per line */
	const uint DISP_LINE_LEN = 16;
	u8 linebuf[DISP_LINE_LEN];
	unsigned int k;
	ulong addr, length, size;
	ulong nbytes;
	enum axi_size_t axisize;
	int unitsize;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	size = dp_last_size;
	addr = dp_last_addr;
	length = dp_last_length;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (!axi_cur_bus) {
		puts("No AXI bus selected\n");
		return CMD_RET_FAILURE;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		size = simple_strtoul(argv[1], NULL, 10);

		/*
		 * Address is specified since argc >= 3
		 */
		addr = simple_strtoul(argv[2], NULL, 16);

		/*
		 * If there's another parameter, it is the length to display;
		 * length is the number of objects, not number of bytes
		 */
		if (argc > 3)
			length = simple_strtoul(argv[3], NULL, 16);
	}

	switch (size) {
	case 8:
		axisize = AXI_SIZE_8;
		unitsize = 1;
		break;
	case 16:
		axisize = AXI_SIZE_16;
		unitsize = 2;
		break;
	case 32:
		axisize = AXI_SIZE_32;
		unitsize = 4;
		break;
	default:
		printf("Unknown read size '%lu'\n", size);
		return CMD_RET_USAGE;
	};

	nbytes = length * unitsize;
	do {
		ulong linebytes = (nbytes > DISP_LINE_LEN) ?
				  DISP_LINE_LEN : nbytes;

		for (k = 0; k < linebytes / unitsize; ++k) {
			int ret = axi_read(axi_cur_bus, addr + k * unitsize,
					   linebuf + k * unitsize, axisize);

			if (!ret) /* Continue if axi_read was successful */
				continue;

			if (ret == -ENOSYS)
				printf("axi_read failed; read size not supported?\n");
			else
				printf("axi_read failed: err = %d\n", ret);

			return CMD_RET_FAILURE;
		}
		print_buffer(addr, (void *)linebuf, unitsize,
			     linebytes / unitsize,
			     DISP_LINE_LEN / unitsize);

		nbytes -= max(linebytes, 1UL);
		addr += linebytes;

		if (ctrlc())
			break;
	} while (nbytes > 0);

	dp_last_size = size;
	dp_last_addr = addr;
	dp_last_length = length;

	return 0;
}

static int do_axi_mw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 writeval;
	ulong addr, count, size;
	enum axi_size_t axisize;

	if (argc <= 3 || argc >= 6)
		return CMD_RET_USAGE;

	size = simple_strtoul(argv[1], NULL, 10);

	switch (size) {
	case 8:
		axisize = AXI_SIZE_8;
		break;
	case 16:
		axisize = AXI_SIZE_16;
		break;
	case 32:
		axisize = AXI_SIZE_32;
		break;
	default:
		printf("Unknown write size '%lu'\n", size);
		return CMD_RET_USAGE;
	};

	/* Address is specified since argc > 4 */
	addr = simple_strtoul(argv[2], NULL, 16);

	/* Get the value to write */
	writeval = simple_strtoul(argv[3], NULL, 16);

	/* Count ? */
	if (argc == 5)
		count = simple_strtoul(argv[4], NULL, 16);
	else
		count = 1;

	while (count-- > 0) {
		int ret = axi_write(axi_cur_bus, addr + count * sizeof(u32),
				    &writeval, axisize);

		if (ret) {
			printf("axi_write failed: err = %d\n", ret);
			return CMD_RET_FAILURE;
		}
	}

	return 0;
}

static cmd_tbl_t cmd_axi_sub[] = {
	U_BOOT_CMD_MKENT(bus, 1, 1, do_axi_show_bus, "", ""),
	U_BOOT_CMD_MKENT(dev, 1, 1, do_axi_bus_num, "", ""),
	U_BOOT_CMD_MKENT(md, 4, 1, do_axi_md, "", ""),
	U_BOOT_CMD_MKENT(mw, 5, 1, do_axi_mw, "", ""),
};

static int do_ihs_axi(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'axi' command argument */
	argc--;
	argv++;

	/* Hand off rest of command line to sub-commands */
	c = find_cmd_tbl(argv[0], &cmd_axi_sub[0], ARRAY_SIZE(cmd_axi_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

static char axi_help_text[] =
	"bus  - show AXI bus info\n"
	"axi dev [bus] - show or set current AXI bus to bus number [bus]\n"
	"axi md size addr [# of objects] - read from AXI device at address [addr] and data width [size] (one of 8, 16, 32)\n"
	"axi mw size addr value [count] - write data [value] to AXI device at address [addr] and data width [size] (one of 8, 16, 32)\n";

U_BOOT_CMD(axi, 7, 1, do_ihs_axi,
	   "AXI sub-system",
	   axi_help_text
);
