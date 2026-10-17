// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */
#include <common.h>
#include <arm_ffa.h>
#include <command.h>
#include <dm.h>
#include <mapmem.h>
#include <stdlib.h>
#include <asm/io.h>

/* Select the right physical address formatting according to the platform */
#ifdef CONFIG_PHYS_64BIT
#define PhysAddrLength "ll"
#else
#define PhysAddrLength ""
#endif
#define PHYS_ADDR_LN "%" PhysAddrLength "x"

/**
 * ffa_get_dev() - Return the FF-A device
 * @devp:	pointer to the FF-A device
 *
 * Search for the FF-A device.
 *
 * Return:
 * 0 on success. Otherwise, failure
 */
static int ffa_get_dev(struct udevice **devp)
{
	int ret;

	ret = uclass_first_device_err(UCLASS_FFA, devp);
	if (ret) {
		log_err("Cannot find FF-A bus device\n");
		return ret;
	}

	return 0;
}

/**
 * do_ffa_getpart() - implementation of the getpart subcommand
 * @cmdtp:		Command Table
 * @flag:		flags
 * @argc:		number of arguments
 * @argv:		arguments
 *
 * Query a secure partition information. The secure partition UUID is provided
 * as an argument. The function uses the arm_ffa driver
 * partition_info_get operation which implements FFA_PARTITION_INFO_GET
 * ABI to retrieve the data. The input UUID string is expected to be in big
 * endian format.
 *
 * Return:
 *
 * CMD_RET_SUCCESS: on success, otherwise failure
 */
static int do_ffa_getpart(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	u32 count = 0;
	int ret;
	struct ffa_partition_desc *descs;
	u32 i;
	struct udevice *dev;

	if (argc != 2) {
		log_err("Missing argument\n");
		return CMD_RET_USAGE;
	}

	ret = ffa_get_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	/* Ask the driver to fill the buffer with the SPs info */

	ret = ffa_partition_info_get(dev, argv[1], &count, &descs);
	if (ret) {
		log_err("Failure in querying partition(s) info (error code: %d)\n", ret);
		return CMD_RET_FAILURE;
	}

	/* SPs found , show the partition information */
	for (i = 0; i < count ; i++) {
		log_info("Partition: id = %x , exec_ctxt %x , properties %x\n",
			 descs[i].info.id,
			 descs[i].info.exec_ctxt,
			 descs[i].info.properties);
	}

	return CMD_RET_SUCCESS;
}

/**
 * do_ffa_ping() - implementation of the ping subcommand
 * @cmdtp:		Command Table
 * @flag:		flags
 * @argc:		number of arguments
 * @argv:		arguments
 *
 * Send data to a secure partition. The secure partition UUID is provided
 * as an argument. Use the arm_ffa driver sync_send_receive operation
 * which implements FFA_MSG_SEND_DIRECT_{REQ,RESP} ABIs to send/receive data.
 *
 * Return:
 *
 * CMD_RET_SUCCESS: on success, otherwise failure
 */
static int do_ffa_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct ffa_send_direct_data msg = {
			.data0 = 0xaaaaaaaa,
			.data1 = 0xbbbbbbbb,
			.data2 = 0xcccccccc,
			.data3 = 0xdddddddd,
			.data4 = 0xeeeeeeee,
	};
	u16 part_id;
	int ret;
	struct udevice *dev;

	if (argc != 2) {
		log_err("Missing argument\n");
		return CMD_RET_USAGE;
	}

	part_id = strtoul(argv[1], NULL, 16);
	if (!part_id) {
		log_err("Partition ID can not be 0\n");
		return CMD_RET_USAGE;
	}

	ret = ffa_get_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	ret = ffa_sync_send_receive(dev, part_id, &msg, 1);
	if (!ret) {
		u8 cnt;

		log_info("SP response:\n[LSB]\n");
		for (cnt = 0;
		     cnt < sizeof(struct ffa_send_direct_data) / sizeof(u64);
		     cnt++)
			log_info("%llx\n", ((u64 *)&msg)[cnt]);
		return CMD_RET_SUCCESS;
	}

	log_err("Sending direct request error (%d)\n", ret);
	return CMD_RET_FAILURE;
}

/**
 *do_ffa_devlist() - implementation of the devlist subcommand
 * @cmdtp: [in]		Command Table
 * @flag:		flags
 * @argc:		number of arguments
 * @argv:		arguments
 *
 * Query the device belonging to the UCLASS_FFA
 * class.
 *
 * Return:
 *
 * CMD_RET_SUCCESS: on success, otherwise failure
 */
static int do_ffa_devlist(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	int ret;

	ret = ffa_get_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	log_info("device %s, addr " PHYS_ADDR_LN ", driver %s, ops " PHYS_ADDR_LN "\n",
		 dev->name,
		 map_to_sysmem(dev),
		 dev->driver->name,
		 map_to_sysmem(dev->driver->ops));

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(armffa,
	"getpart <partition UUID>\n"
	"       - lists the partition(s) info\n"
	"ping <partition ID>\n"
	"       - sends a data pattern to the specified partition\n"
	"devlist\n"
	"       - displays information about the FF-A device/driver\n");

U_BOOT_CMD_WITH_SUBCMDS(armffa, "Arm FF-A test command", armffa_help_text,
			U_BOOT_SUBCMD_MKENT(getpart, 2, 1, do_ffa_getpart),
			U_BOOT_SUBCMD_MKENT(ping, 2, 1, do_ffa_ping),
			U_BOOT_SUBCMD_MKENT(devlist, 1, 1, do_ffa_devlist));
