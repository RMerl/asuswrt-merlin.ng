// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver
 *
 * Copyright (c) 2016 The Chromium OS Authors.
 * Copyright (c) 2016 National Instruments Corp
 */

#include <common.h>
#include <command.h>
#include <cros_ec.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

/* Note: depends on enum ec_current_image */
static const char * const ec_current_image_name[] = {"unknown", "RO", "RW"};

/**
 * Decode a flash region parameter
 *
 * @param argc Number of params remaining
 * @param argv List of remaining parameters
 * @return flash region (EC_FLASH_REGION_...) or -1 on error
 */
static int cros_ec_decode_region(int argc, char * const argv[])
{
	if (argc > 0) {
		if (0 == strcmp(*argv, "rw"))
			return EC_FLASH_REGION_ACTIVE;
		else if (0 == strcmp(*argv, "ro"))
			return EC_FLASH_REGION_RO;

		debug("%s: Invalid region '%s'\n", __func__, *argv);
	} else {
		debug("%s: Missing region parameter\n", __func__);
	}

	return -1;
}

/**
 * Perform a flash read or write command
 *
 * @param dev		CROS-EC device to read/write
 * @param is_write	1 do to a write, 0 to do a read
 * @param argc		Number of arguments
 * @param argv		Arguments (2 is region, 3 is address)
 * @return 0 for ok, 1 for a usage error or -ve for ec command error
 *	(negative EC_RES_...)
 */
static int do_read_write(struct udevice *dev, int is_write, int argc,
			 char * const argv[])
{
	uint32_t offset, size = -1U, region_size;
	unsigned long addr;
	char *endp;
	int region;
	int ret;

	region = cros_ec_decode_region(argc - 2, argv + 2);
	if (region == -1)
		return 1;
	if (argc < 4)
		return 1;
	addr = simple_strtoul(argv[3], &endp, 16);
	if (*argv[3] == 0 || *endp != 0)
		return 1;
	if (argc > 4) {
		size = simple_strtoul(argv[4], &endp, 16);
		if (*argv[4] == 0 || *endp != 0)
			return 1;
	}

	ret = cros_ec_flash_offset(dev, region, &offset, &region_size);
	if (ret) {
		debug("%s: Could not read region info\n", __func__);
		return ret;
	}
	if (size == -1U)
		size = region_size;

	ret = is_write ?
		cros_ec_flash_write(dev, (uint8_t *)addr, offset, size) :
		cros_ec_flash_read(dev, (uint8_t *)addr, offset, size);
	if (ret) {
		debug("%s: Could not %s region\n", __func__,
		      is_write ? "write" : "read");
		return ret;
	}

	return 0;
}

static int do_cros_ec(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	const char *cmd;
	int ret = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	if (0 == strcmp("init", cmd)) {
		/* Remove any existing device */
		ret = uclass_find_device(UCLASS_CROS_EC, 0, &dev);
		if (!ret)
			device_remove(dev, DM_REMOVE_NORMAL);
		ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
		if (ret) {
			printf("Could not init cros_ec device (err %d)\n", ret);
			return 1;
		}
		return 0;
	}

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	if (ret) {
		printf("Cannot get cros-ec device (err=%d)\n", ret);
		return 1;
	}
	if (0 == strcmp("id", cmd)) {
		char id[MSG_BYTES];

		if (cros_ec_read_id(dev, id, sizeof(id))) {
			debug("%s: Could not read KBC ID\n", __func__);
			return 1;
		}
		printf("%s\n", id);
	} else if (0 == strcmp("info", cmd)) {
		struct ec_response_mkbp_info info;

		if (cros_ec_info(dev, &info)) {
			debug("%s: Could not read KBC info\n", __func__);
			return 1;
		}
		printf("rows     = %u\n", info.rows);
		printf("cols     = %u\n", info.cols);
	} else if (0 == strcmp("curimage", cmd)) {
		enum ec_current_image image;

		if (cros_ec_read_current_image(dev, &image)) {
			debug("%s: Could not read KBC image\n", __func__);
			return 1;
		}
		printf("%d\n", image);
	} else if (0 == strcmp("hash", cmd)) {
		struct ec_response_vboot_hash hash;
		int i;

		if (cros_ec_read_hash(dev, EC_VBOOT_HASH_OFFSET_ACTIVE, &hash)) {
			debug("%s: Could not read KBC hash\n", __func__);
			return 1;
		}

		if (hash.hash_type == EC_VBOOT_HASH_TYPE_SHA256)
			printf("type:    SHA-256\n");
		else
			printf("type:    %d\n", hash.hash_type);

		printf("offset:  0x%08x\n", hash.offset);
		printf("size:    0x%08x\n", hash.size);

		printf("digest:  ");
		for (i = 0; i < hash.digest_size; i++)
			printf("%02x", hash.hash_digest[i]);
		printf("\n");
	} else if (0 == strcmp("reboot", cmd)) {
		int region;
		enum ec_reboot_cmd cmd;

		if (argc >= 3 && !strcmp(argv[2], "cold")) {
			cmd = EC_REBOOT_COLD;
		} else {
			region = cros_ec_decode_region(argc - 2, argv + 2);
			if (region == EC_FLASH_REGION_RO)
				cmd = EC_REBOOT_JUMP_RO;
			else if (region == EC_FLASH_REGION_ACTIVE)
				cmd = EC_REBOOT_JUMP_RW;
			else
				return CMD_RET_USAGE;
		}

		if (cros_ec_reboot(dev, cmd, 0)) {
			debug("%s: Could not reboot KBC\n", __func__);
			return 1;
		}
	} else if (0 == strcmp("events", cmd)) {
		uint32_t events;

		if (cros_ec_get_host_events(dev, &events)) {
			debug("%s: Could not read host events\n", __func__);
			return 1;
		}
		printf("0x%08x\n", events);
	} else if (0 == strcmp("clrevents", cmd)) {
		uint32_t events = 0x7fffffff;

		if (argc >= 3)
			events = simple_strtol(argv[2], NULL, 0);

		if (cros_ec_clear_host_events(dev, events)) {
			debug("%s: Could not clear host events\n", __func__);
			return 1;
		}
	} else if (0 == strcmp("read", cmd)) {
		ret = do_read_write(dev, 0, argc, argv);
		if (ret > 0)
			return CMD_RET_USAGE;
	} else if (0 == strcmp("write", cmd)) {
		ret = do_read_write(dev, 1, argc, argv);
		if (ret > 0)
			return CMD_RET_USAGE;
	} else if (0 == strcmp("erase", cmd)) {
		int region = cros_ec_decode_region(argc - 2, argv + 2);
		uint32_t offset, size;

		if (region == -1)
			return CMD_RET_USAGE;
		if (cros_ec_flash_offset(dev, region, &offset, &size)) {
			debug("%s: Could not read region info\n", __func__);
			ret = -1;
		} else {
			ret = cros_ec_flash_erase(dev, offset, size);
			if (ret) {
				debug("%s: Could not erase region\n",
				      __func__);
			}
		}
	} else if (0 == strcmp("regioninfo", cmd)) {
		int region = cros_ec_decode_region(argc - 2, argv + 2);
		uint32_t offset, size;

		if (region == -1)
			return CMD_RET_USAGE;
		ret = cros_ec_flash_offset(dev, region, &offset, &size);
		if (ret) {
			debug("%s: Could not read region info\n", __func__);
		} else {
			printf("Region: %s\n", region == EC_FLASH_REGION_RO ?
					"RO" : "RW");
			printf("Offset: %x\n", offset);
			printf("Size:   %x\n", size);
		}
	} else if (0 == strcmp("flashinfo", cmd)) {
		struct ec_response_flash_info p;

		ret = cros_ec_read_flashinfo(dev, &p);
		if (!ret) {
			printf("Flash size:         %u\n", p.flash_size);
			printf("Write block size:   %u\n", p.write_block_size);
			printf("Erase block size:   %u\n", p.erase_block_size);
		}
	} else if (0 == strcmp("vbnvcontext", cmd)) {
		uint8_t block[EC_VBNV_BLOCK_SIZE];
		char buf[3];
		int i, len;
		unsigned long result;

		if (argc <= 2) {
			ret = cros_ec_read_nvdata(dev, block,
						  EC_VBNV_BLOCK_SIZE);
			if (!ret) {
				printf("vbnv_block: ");
				for (i = 0; i < EC_VBNV_BLOCK_SIZE; i++)
					printf("%02x", block[i]);
				putc('\n');
			}
		} else {
			/*
			 * TODO(clchiou): Move this to a utility function as
			 * cmd_spi might want to call it.
			 */
			memset(block, 0, EC_VBNV_BLOCK_SIZE);
			len = strlen(argv[2]);
			buf[2] = '\0';
			for (i = 0; i < EC_VBNV_BLOCK_SIZE; i++) {
				if (i * 2 >= len)
					break;
				buf[0] = argv[2][i * 2];
				if (i * 2 + 1 >= len)
					buf[1] = '0';
				else
					buf[1] = argv[2][i * 2 + 1];
				strict_strtoul(buf, 16, &result);
				block[i] = result;
			}
			ret = cros_ec_write_nvdata(dev, block,
						   EC_VBNV_BLOCK_SIZE);
		}
		if (ret) {
			debug("%s: Could not %s VbNvContext\n", __func__,
			      argc <= 2 ?  "read" : "write");
		}
	} else if (0 == strcmp("test", cmd)) {
		int result = cros_ec_test(dev);

		if (result)
			printf("Test failed with error %d\n", result);
		else
			puts("Test passed\n");
	} else if (0 == strcmp("version", cmd)) {
		struct ec_response_get_version *p;
		char *build_string;

		ret = cros_ec_read_version(dev, &p);
		if (!ret) {
			/* Print versions */
			printf("RO version:    %1.*s\n",
			       (int)sizeof(p->version_string_ro),
			       p->version_string_ro);
			printf("RW version:    %1.*s\n",
			       (int)sizeof(p->version_string_rw),
			       p->version_string_rw);
			printf("Firmware copy: %s\n",
			       (p->current_image <
			       ARRAY_SIZE(ec_current_image_name) ?
			       ec_current_image_name[p->current_image] :
			       "?"));
			ret = cros_ec_read_build_info(dev, &build_string);
			if (!ret)
				printf("Build info:    %s\n", build_string);
		}
	} else if (0 == strcmp("ldo", cmd)) {
		uint8_t index, state;
		char *endp;

		if (argc < 3)
			return CMD_RET_USAGE;
		index = simple_strtoul(argv[2], &endp, 10);
		if (*argv[2] == 0 || *endp != 0)
			return CMD_RET_USAGE;
		if (argc > 3) {
			state = simple_strtoul(argv[3], &endp, 10);
			if (*argv[3] == 0 || *endp != 0)
				return CMD_RET_USAGE;
			ret = cros_ec_set_ldo(dev, index, state);
		} else {
			ret = cros_ec_get_ldo(dev, index, &state);
			if (!ret) {
				printf("LDO%d: %s\n", index,
				       state == EC_LDO_STATE_ON ?
				       "on" : "off");
			}
		}

		if (ret) {
			debug("%s: Could not access LDO%d\n", __func__, index);
			return ret;
		}
	} else {
		return CMD_RET_USAGE;
	}

	if (ret < 0) {
		printf("Error: CROS-EC command failed (error %d)\n", ret);
		ret = 1;
	}

	return ret;
}

U_BOOT_CMD(
	crosec,	6,	1,	do_cros_ec,
	"CROS-EC utility command",
	"init                Re-init CROS-EC (done on startup automatically)\n"
	"crosec id                  Read CROS-EC ID\n"
	"crosec info                Read CROS-EC info\n"
	"crosec curimage            Read CROS-EC current image\n"
	"crosec hash                Read CROS-EC hash\n"
	"crosec reboot [rw | ro | cold]  Reboot CROS-EC\n"
	"crosec events              Read CROS-EC host events\n"
	"crosec clrevents [mask]    Clear CROS-EC host events\n"
	"crosec regioninfo <ro|rw>  Read image info\n"
	"crosec flashinfo           Read flash info\n"
	"crosec erase <ro|rw>       Erase EC image\n"
	"crosec read <ro|rw> <addr> [<size>]   Read EC image\n"
	"crosec write <ro|rw> <addr> [<size>]  Write EC image\n"
	"crosec vbnvcontext [hexstring]        Read [write] VbNvContext from EC\n"
	"crosec ldo <idx> [<state>] Switch/Read LDO state\n"
	"crosec test                run tests on cros_ec\n"
	"crosec version             Read CROS-EC version"
);
