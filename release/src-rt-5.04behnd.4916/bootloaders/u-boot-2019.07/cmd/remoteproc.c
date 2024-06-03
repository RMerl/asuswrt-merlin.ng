// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - http://www.ti.com/
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <remoteproc.h>

/**
 * print_remoteproc_list() - print all the remote processor devices
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int print_remoteproc_list(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;
	char *type;

	ret = uclass_get(UCLASS_REMOTEPROC, &uc);
	if (ret) {
		printf("Cannot find Remote processor class\n");
		return ret;
	}

	uclass_foreach_dev(dev, uc) {
		struct dm_rproc_uclass_pdata *uc_pdata;
		const struct dm_rproc_ops *ops = rproc_get_ops(dev);

		uc_pdata = dev_get_uclass_platdata(dev);

		switch (uc_pdata->mem_type) {
		case RPROC_INTERNAL_MEMORY_MAPPED:
			type = "internal memory mapped";
			break;
		default:
			type = "unknown";
			break;
		}
		printf("%d - Name:'%s' type:'%s' supports: %s%s%s%s%s%s\n",
		       dev->seq,
		       uc_pdata->name,
		       type,
		       ops->load ? "load " : "",
		       ops->start ? "start " : "",
		       ops->stop ? "stop " : "",
		       ops->reset ? "reset " : "",
		       ops->is_running ? "is_running " : "",
		       ops->ping ? "ping " : "");
	}
	return 0;
}

/**
 * do_rproc_init() - do basic initialization
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	unused
 * @argv:	unused
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_rproc_init(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (rproc_is_initialized()) {
		printf("\tRemote Processors are already initialized\n");
	} else {
		if (!rproc_init())
			return 0;
		printf("Few Remote Processors failed to be initalized\n");
	}

	return CMD_RET_FAILURE;
}

/**
 * do_remoteproc_list() - print list of remote proc devices.
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	unused
 * @argv:	unused
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_remoteproc_list(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	if (!rproc_is_initialized()) {
		printf("\t Remote Processors is not initialized\n");
		return CMD_RET_USAGE;
	}

	if (print_remoteproc_list())
		return CMD_RET_FAILURE;

	return 0;
}

/**
 * do_remoteproc_load() - Load a remote processor with binary image
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	argument count for the load function
 * @argv:	arguments for the load function
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_remoteproc_load(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	ulong addr, size;
	int id, ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	id = (int)simple_strtoul(argv[1], NULL, 10);
	addr = simple_strtoul(argv[2], NULL, 16);

	size = simple_strtoul(argv[3], NULL, 16);

	if (!size) {
		printf("\t Expect some size??\n");
		return CMD_RET_USAGE;
	}

	if (!rproc_is_initialized()) {
		printf("\tRemote Processors are not initialized\n");
		return CMD_RET_USAGE;
	}

	ret = rproc_load(id, addr, size);
	printf("Load Remote Processor %d with data@addr=0x%08lx %lu bytes:%s\n",
	       id, addr, size, ret ? " Failed!" : " Success!");

	return ret ? CMD_RET_FAILURE : 0;
}

/**
 * do_remoteproc_wrapper() - wrapper for various  rproc commands
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	argument count for the rproc command
 * @argv:	arguments for the rproc command
 *
 * Most of the commands just take id as a parameter andinvoke various
 * helper routines in remote processor core. by using a set of
 * common checks, we can reduce the amount of code used for this.
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_remoteproc_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
				 char *const argv[])
{
	int id, ret = CMD_RET_USAGE;

	if (argc != 2)
		return CMD_RET_USAGE;

	id = (int)simple_strtoul(argv[1], NULL, 10);

	if (!rproc_is_initialized()) {
		printf("\tRemote Processors are not initialized\n");
		return CMD_RET_USAGE;
	}

	if (!strcmp(argv[0], "start")) {
		ret = rproc_start(id);
	} else if (!strcmp(argv[0], "stop")) {
		ret = rproc_stop(id);
	} else if (!strcmp(argv[0], "reset")) {
		ret = rproc_reset(id);
	} else if (!strcmp(argv[0], "is_running")) {
		ret = rproc_is_running(id);
		if (!ret) {
			printf("Remote processor is Running\n");
		} else if (ret == 1) {
			printf("Remote processor is NOT Running\n");
			ret = 0;
		}
		/* Else error.. */
	} else if (!strcmp(argv[0], "ping")) {
		ret = rproc_ping(id);
		if (!ret) {
			printf("Remote processor responds 'Pong'\n");
		} else if (ret == 1) {
			printf("No response from Remote processor\n");
			ret = 0;
		}
		/* Else error.. */
	}

	if (ret < 0)
		printf("Operation Failed with error (%d)\n", ret);

	return ret ? CMD_RET_FAILURE : 0;
}

static cmd_tbl_t cmd_remoteproc_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_rproc_init,
			 "Enumerate and initialize all processors", ""),
	U_BOOT_CMD_MKENT(list, 0, 1, do_remoteproc_list,
			 "list remote processors", ""),
	U_BOOT_CMD_MKENT(load, 5, 1, do_remoteproc_load,
			 "Load remote processor with provided image",
			 "<id> [addr] [size]\n"
			 "- id: ID of the remote processor(see 'list' cmd)\n"
			 "- addr: Address in memory of the image to loadup\n"
			 "- size: Size of the image to loadup\n"),
	U_BOOT_CMD_MKENT(start, 1, 1, do_remoteproc_wrapper,
			 "Start remote processor",
			 "id - ID of the remote processor (see 'list' cmd)\n"),
	U_BOOT_CMD_MKENT(stop, 1, 1, do_remoteproc_wrapper,
			 "Stop remote processor",
			 "id - ID of the remote processor (see 'list' cmd)\n"),
	U_BOOT_CMD_MKENT(reset, 1, 1, do_remoteproc_wrapper,
			 "Reset remote processor",
			 "id - ID of the remote processor (see 'list' cmd)\n"),
	U_BOOT_CMD_MKENT(is_running, 1, 1, do_remoteproc_wrapper,
			 "Check to see if remote processor is running\n",
			 "id - ID of the remote processor (see 'list' cmd)\n"),
	U_BOOT_CMD_MKENT(ping, 1, 1, do_remoteproc_wrapper,
			 "Ping to communicate with remote processor\n",
			 "id - ID of the remote processor (see 'list' cmd)\n"),
};

/**
 * do_remoteproc() - (replace: short desc)
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	argument count
 * @argv:	argument list
 *
 * parses up the command table to invoke the correct command.
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_remoteproc(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	cmd_tbl_t *c = NULL;

	/* Strip off leading 'rproc' command argument */
	argc--;
	argv++;

	if (argc)
		c = find_cmd_tbl(argv[0], cmd_remoteproc_sub,
				 ARRAY_SIZE(cmd_remoteproc_sub));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(rproc, 5, 1, do_remoteproc,
	   "Control operation of remote processors in an SoC",
	   " [init|list|load|start|stop|reset|is_running|ping]\n"
	   "\t\t Where:\n"
	   "\t\t[addr] is a memory address\n"
	   "\t\t<id> is a numerical identifier for the remote processor\n"
	   "\t\t     provided by 'list' command.\n"
	   "\t\tNote: Remote processors must be initalized prior to usage\n"
	   "\t\tNote: Services are dependent on the driver capability\n"
	   "\t\t      'list' command shows the capability of each device\n"
	   "\n\tSubcommands:\n"
	   "\tinit   - Enumerate and initalize the remote processors\n"
	   "\tlist   - list available remote processors\n"
	   "\tload <id> [addr] [size]- Load the remote processor with binary\n"
	   "\t		  image stored at address [addr] in memory\n"
	   "\tstart <id>	- Start the remote processor(must be loaded)\n"
	   "\tstop <id>	- Stop the remote processor\n"
	   "\treset <id>	- Reset the remote processor\n"
	   "\tis_running <id> - Reports if the remote processor is running\n"
	   "\tping <id>	- Ping the remote processor for communication\n");
