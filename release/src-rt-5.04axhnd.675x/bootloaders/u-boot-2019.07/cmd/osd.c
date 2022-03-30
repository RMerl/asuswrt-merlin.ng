// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on the gdsys osd driver, which is
 *
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 */

#include <common.h>
#include <dm.h>
#include <hexdump.h>
#include <video_osd.h>
#include <malloc.h>

/* Container for selected OSD device */
static struct udevice *osd_cur;

/**
 * cmd_osd_set_osd_num() - Set the OSD selected for operation
 *
 * Set the OSD device, which will be used by all subsequent OSD commands.
 *
 * Devices are identified by their uclass sequence number (as listed by 'osd
 * show').
 *
 * @osdnum: The OSD device to be selected, identified by its sequence number.
 * Return: 0 if OK, -ve on error
 */
static int cmd_osd_set_osd_num(unsigned int osdnum)
{
	struct udevice *osd;
	int res;

	res = uclass_get_device_by_seq(UCLASS_VIDEO_OSD, osdnum, &osd);
	if (res) {
		printf("%s: No OSD %u (err = %d)\n", __func__, osdnum, res);
		return res;
	}
	osd_cur = osd;

	return 0;
}

/**
 * osd_get_osd_cur() - Get the selected OSD device
 *
 * Get the OSD device that is used by all OSD commands.
 *
 * @osdp: Pointer to structure that will receive the currently selected OSD
 *	  device.
 * Return: 0 if OK, -ve on error
 */
static int osd_get_osd_cur(struct udevice **osdp)
{
	if (!osd_cur) {
		puts("No osd selected\n");
		return -ENODEV;
	}
	*osdp = osd_cur;

	return 0;
}

/**
 * show_osd() - Display information about a OSD device
 *
 * Display a device's ID (sequence number), and whether it is active (i.e.
 * probed) or not.
 *
 * @osd: OSD device to print information for
 */
static void show_osd(struct udevice *osd)
{
	printf("OSD %d:\t%s", osd->req_seq, osd->name);
	if (device_active(osd))
		printf("  (active %d)", osd->seq);
	printf("\n");
}

static int do_osd_write(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	uint x, y;
	uint count;
	char *hexstr;
	u8 *buffer;
	size_t buflen;
	int res;

	if (argc < 4 || (strlen(argv[3]) % 2))
		return CMD_RET_USAGE;

	if (!osd_cur) {
		puts("No osd selected\n");
		return CMD_RET_FAILURE;
	}

	x = simple_strtoul(argv[1], NULL, 16);
	y = simple_strtoul(argv[2], NULL, 16);
	hexstr = argv[3];
	count = (argc > 4) ? simple_strtoul(argv[4], NULL, 16) : 1;

	buflen = strlen(hexstr) / 2;

	buffer = malloc(buflen);
	if (!buffer) {
		puts("Memory allocation failure\n");
		return CMD_RET_FAILURE;
	}

	res = hex2bin(buffer, hexstr, buflen);
	if (res) {
		free(buffer);
		puts("Hexadecimal input contained invalid characters\n");
		return CMD_RET_FAILURE;
	}

	res = video_osd_set_mem(osd_cur, x, y, buffer, buflen, count);
	if (res) {
		free(buffer);
		printf("%s: Could not write to video mem\n",
		       osd_cur->name);
		return CMD_RET_FAILURE;
	}

	free(buffer);

	return CMD_RET_SUCCESS;
}

static int do_osd_print(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	uint x, y;
	u8 color;
	char *text;
	int res;

	if (argc < 5)
		return CMD_RET_USAGE;

	if (!osd_cur) {
		puts("No osd selected\n");
		return CMD_RET_FAILURE;
	}

	x = simple_strtoul(argv[1], NULL, 16);
	y = simple_strtoul(argv[2], NULL, 16);
	color = simple_strtoul(argv[3], NULL, 16);
	text = argv[4];

	res = video_osd_print(osd_cur, x, y, color, text);
	if (res) {
		printf("Could not print string to osd %s\n", osd_cur->name);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_osd_size(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	uint x, y;
	int res;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (!osd_cur) {
		puts("No osd selected\n");
		return CMD_RET_FAILURE;
	}

	x = simple_strtoul(argv[1], NULL, 16);
	y = simple_strtoul(argv[2], NULL, 16);

	res = video_osd_set_size(osd_cur, x, y);
	if (res) {
		printf("Could not set size on osd %s\n", osd_cur->name);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_show_osd(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct udevice *osd;

	if (argc == 1) {
		/* show all OSDs */
		struct uclass *uc;
		int res;

		res = uclass_get(UCLASS_VIDEO_OSD, &uc);
		if (res) {
			printf("Error while getting OSD uclass (err=%d)\n",
			       res);
			return CMD_RET_FAILURE;
		}

		uclass_foreach_dev(osd, uc)
			show_osd(osd);
	} else {
		int i, res;

		/* show specific OSD */
		i = simple_strtoul(argv[1], NULL, 10);

		res = uclass_get_device_by_seq(UCLASS_VIDEO_OSD, i, &osd);
		if (res) {
			printf("Invalid osd %d: err=%d\n", i, res);
			return CMD_RET_FAILURE;
		}
		show_osd(osd);
	}

	return CMD_RET_SUCCESS;
}

static int do_osd_num(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	int osd_no;
	int res = 0;

	if (argc == 1) {
		/* querying current setting */
		struct udevice *osd;

		if (!osd_get_osd_cur(&osd))
			osd_no = osd->seq;
		else
			osd_no = -1;
		printf("Current osd is %d\n", osd_no);
	} else {
		osd_no = simple_strtoul(argv[1], NULL, 10);
		printf("Setting osd to %d\n", osd_no);

		res = cmd_osd_set_osd_num(osd_no);
		if (res)
			printf("Failure changing osd number (err = %d)\n", res);
	}

	return res ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_osd_sub[] = {
	U_BOOT_CMD_MKENT(show, 1, 1, do_show_osd, "", ""),
	U_BOOT_CMD_MKENT(dev, 1, 1, do_osd_num, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 1, do_osd_write, "", ""),
	U_BOOT_CMD_MKENT(print, 4, 1, do_osd_print, "", ""),
	U_BOOT_CMD_MKENT(size, 2, 1, do_osd_size, "", ""),
};

static int do_osd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'osd' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_osd_sub[0], ARRAY_SIZE(cmd_osd_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

static char osd_help_text[] =
	"show  - show OSD info\n"
	"osd dev [dev] - show or set current OSD\n"
	"write [pos_x] [pos_y] [buffer] [count] - write 8-bit hex encoded buffer to osd memory at a given position\n"
	"print [pos_x] [pos_y] [color] [text] - write ASCII buffer (given by text data and driver-specific color information) to osd memory\n"
	"size [size_x] [size_y] - set OSD XY size in characters\n";

U_BOOT_CMD(
	osd, 6, 1, do_osd,
	"OSD sub-system",
	osd_help_text
);
