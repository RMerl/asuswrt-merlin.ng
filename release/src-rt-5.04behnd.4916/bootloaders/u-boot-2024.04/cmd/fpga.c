// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#include <env.h>
#include <fpga.h>
#include <fs.h>
#include <gzip.h>
#include <image.h>
#include <log.h>
#include <malloc.h>

static long do_fpga_get_device(char *arg)
{
	long dev = FPGA_INVALID_DEVICE;
	char *devstr = env_get("fpga");

	if (devstr)
		/* Should be strtol to handle -1 cases */
		dev = simple_strtol(devstr, NULL, 16);

	if (dev == FPGA_INVALID_DEVICE && arg)
		dev = simple_strtol(arg, NULL, 16);

	debug("%s: device = %ld\n", __func__, dev);

	return dev;
}

static int do_fpga_check_params(long *dev, long *fpga_data, size_t *data_size,
				struct cmd_tbl *cmdtp, int argc,
				char *const argv[])
{
	size_t local_data_size;
	long local_fpga_data;

	debug("%s %d, %d\n", __func__, argc, cmdtp->maxargs);

	if (argc != cmdtp->maxargs) {
		debug("fpga: incorrect parameters passed\n");
		return CMD_RET_USAGE;
	}

	*dev = do_fpga_get_device(argv[0]);

	local_fpga_data = simple_strtol(argv[1], NULL, 16);
	if (!local_fpga_data) {
		debug("fpga: zero fpga_data address\n");
		return CMD_RET_USAGE;
	}
	*fpga_data = local_fpga_data;

	local_data_size = hextoul(argv[2], NULL);
	if (!local_data_size) {
		debug("fpga: zero size\n");
		return CMD_RET_USAGE;
	}
	*data_size = local_data_size;

	return 0;
}

#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
int do_fpga_loads(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;
	struct fpga_secure_info fpga_sec_info;

	memset(&fpga_sec_info, 0, sizeof(fpga_sec_info));

	if (argc < 5) {
		debug("fpga: incorrect parameters passed\n");
		return CMD_RET_USAGE;
	}

	if (argc == 6)
		fpga_sec_info.userkey_addr = (u8 *)(uintptr_t)
					      simple_strtoull(argv[5],
							      NULL, 16);
	else
		/*
		 * If 6th parameter is not passed then do_fpga_check_params
		 * will get 5 instead of expected 6 which means that function
		 * return CMD_RET_USAGE. Increase number of params +1 to pass
		 * this.
		 */
		argc++;

	fpga_sec_info.encflag = (u8)hextoul(argv[4], NULL);
	fpga_sec_info.authflag = (u8)hextoul(argv[3], NULL);

	if (fpga_sec_info.authflag >= FPGA_NO_ENC_OR_NO_AUTH &&
	    fpga_sec_info.encflag >= FPGA_NO_ENC_OR_NO_AUTH) {
		debug("fpga: Use <fpga load> for NonSecure bitstream\n");
		return CMD_RET_USAGE;
	}

	if (fpga_sec_info.encflag == FPGA_ENC_USR_KEY &&
	    !fpga_sec_info.userkey_addr) {
		debug("fpga: User key not provided\n");
		return CMD_RET_USAGE;
	}

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_loads(dev, (void *)fpga_data, data_size, &fpga_sec_info);
}
#endif

#if defined(CONFIG_CMD_FPGA_LOADFS)
static int do_fpga_loadfs(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;
	fpga_fs_info fpga_fsinfo;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	fpga_fsinfo.fstype = FS_TYPE_ANY;
	fpga_fsinfo.blocksize = (unsigned int)hextoul(argv[3], NULL);
	fpga_fsinfo.interface = argv[4];
	fpga_fsinfo.dev_part = argv[5];
	fpga_fsinfo.filename = argv[6];

	return fpga_fsload(dev, (void *)fpga_data, data_size, &fpga_fsinfo);
}
#endif

static int do_fpga_info(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	long dev = do_fpga_get_device(argv[0]);

	return fpga_info(dev);
}

static int do_fpga_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_dump(dev, (void *)fpga_data, data_size);
}

static int do_fpga_load(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_load(dev, (void *)fpga_data, data_size, BIT_FULL, 0);
}

static int do_fpga_loadb(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_loadbitstream(dev, (void *)fpga_data, data_size, BIT_FULL);
}

#if defined(CONFIG_CMD_FPGA_LOADP)
static int do_fpga_loadp(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_load(dev, (void *)fpga_data, data_size, BIT_PARTIAL, 0);
}
#endif

#if defined(CONFIG_CMD_FPGA_LOADBP)
static int do_fpga_loadbp(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_loadbitstream(dev, (void *)fpga_data, data_size,
				  BIT_PARTIAL);
}
#endif

#if defined(CONFIG_CMD_FPGA_LOADMK)
static int do_fpga_loadmk(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	size_t data_size = 0;
	void *fpga_data = NULL;
#if defined(CONFIG_FIT)
	const char *fit_uname = NULL;
	ulong fit_addr;
#endif
	ulong dev = do_fpga_get_device(argv[0]);
	char *datastr = env_get("fpgadata");

	debug("fpga: argc %x, dev %lx, datastr %s\n", argc, dev, datastr);

	if (dev == FPGA_INVALID_DEVICE) {
		debug("fpga: Invalid fpga device\n");
		return CMD_RET_USAGE;
	}

	if (argc == 0 && !datastr) {
		debug("fpga: No datastr passed\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		datastr = argv[1];
		debug("fpga: Full command with two args\n");
	} else if (argc == 1 && !datastr) {
		debug("fpga: Dev is setup - fpgadata passed\n");
		datastr = argv[0];
	}

#if defined(CONFIG_FIT)
	if (fit_parse_subimage(datastr, (ulong)fpga_data,
			       &fit_addr, &fit_uname)) {
		fpga_data = (void *)fit_addr;
		debug("*  fpga: subimage '%s' from FIT image ",
		      fit_uname);
		debug("at 0x%08lx\n", fit_addr);
	} else
#endif
	{
		fpga_data = (void *)hextoul(datastr, NULL);
		debug("*  fpga: cmdline image address = 0x%08lx\n",
		      (ulong)fpga_data);
	}
	debug("%s: fpga_data = 0x%lx\n", __func__, (ulong)fpga_data);
	if (!fpga_data) {
		puts("Zero fpga_data address\n");
		return CMD_RET_USAGE;
	}

	switch (genimg_get_format(fpga_data)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
	{
		struct legacy_img_hdr *hdr = (struct legacy_img_hdr *)fpga_data;
		ulong data;
		u8 comp;

		comp = image_get_comp(hdr);
		if (comp == IH_COMP_GZIP) {
#if defined(CONFIG_GZIP)
			ulong image_buf = image_get_data(hdr);
			ulong image_size = ~0UL;

			data = image_get_load(hdr);

			if (gunzip((void *)data, ~0UL, (void *)image_buf,
				   &image_size) != 0) {
				puts("GUNZIP: error\n");
				return CMD_RET_FAILURE;
			}
			data_size = image_size;
#else
			puts("Gunzip image is not supported\n");
			return 1;
#endif
		} else {
			data = (ulong)image_get_data(hdr);
			data_size = image_get_data_size(hdr);
		}
		return fpga_load(dev, (void *)data, data_size,
				  BIT_FULL, 0);
	}
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
	{
		const void *fit_hdr = (const void *)fpga_data;
		int err;
		const void *fit_data;

		if (!fit_uname) {
			puts("No FIT subimage unit name\n");
			return CMD_RET_FAILURE;
		}

		if (fit_check_format(fit_hdr, IMAGE_SIZE_INVAL)) {
			puts("Bad FIT image format\n");
			return CMD_RET_FAILURE;
		}

		err = fit_get_data_node(fit_hdr, fit_uname, &fit_data,
					&data_size);
		if (err) {
			printf("Could not load '%s' subimage (err %d)\n",
			       fit_uname, err);
			return CMD_RET_FAILURE;
		}

		return fpga_load(dev, fit_data, data_size, BIT_FULL, 0);
	}
#endif
	default:
		puts("** Unknown image type\n");
		return CMD_RET_FAILURE;
	}
}
#endif

static struct cmd_tbl fpga_commands[] = {
	U_BOOT_CMD_MKENT(info, 1, 1, do_fpga_info, "", ""),
	U_BOOT_CMD_MKENT(dump, 3, 1, do_fpga_dump, "", ""),
	U_BOOT_CMD_MKENT(load, 3, 1, do_fpga_load, "", ""),
	U_BOOT_CMD_MKENT(loadb, 3, 1, do_fpga_loadb, "", ""),
#if defined(CONFIG_CMD_FPGA_LOADP)
	U_BOOT_CMD_MKENT(loadp, 3, 1, do_fpga_loadp, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADBP)
	U_BOOT_CMD_MKENT(loadbp, 3, 1, do_fpga_loadbp, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	U_BOOT_CMD_MKENT(loadfs, 7, 1, do_fpga_loadfs, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADMK)
	U_BOOT_CMD_MKENT(loadmk, 2, 1, do_fpga_loadmk, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	U_BOOT_CMD_MKENT(loads, 6, 1, do_fpga_loads, "", ""),
#endif
};

static int do_fpga_wrapper(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct cmd_tbl *fpga_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	fpga_cmd = find_cmd_tbl(argv[1], fpga_commands,
				ARRAY_SIZE(fpga_commands));
	if (!fpga_cmd) {
		debug("fpga: non existing command\n");
		return CMD_RET_USAGE;
	}

	argc -= 2;
	argv += 2;

	if (argc > fpga_cmd->maxargs) {
		debug("fpga: more parameters passed\n");
		return CMD_RET_USAGE;
	}

	ret = fpga_cmd->cmd(fpga_cmd, flag, argc, argv);

	return cmd_process_error(fpga_cmd, ret);
}

#if defined(CONFIG_CMD_FPGA_LOADFS) || defined(CONFIG_CMD_FPGA_LOAD_SECURE)
U_BOOT_CMD(fpga, 9, 1, do_fpga_wrapper,
#else
U_BOOT_CMD(fpga, 6, 1, do_fpga_wrapper,
#endif
	   "loadable FPGA image support",
	   "[operation type] [device number] [image address] [image size]\n"
	   "fpga operations:\n"
	   "  dump\t[dev] [address] [size]\tLoad device to memory buffer\n"
	   "  info\t[dev]\t\t\tlist known device information\n"
	   "  load\t[dev] [address] [size]\tLoad device from memory buffer\n"
#if defined(CONFIG_CMD_FPGA_LOADP)
	   "  loadp\t[dev] [address] [size]\t"
	   "Load device from memory buffer with partial bitstream\n"
#endif
	   "  loadb\t[dev] [address] [size]\t"
	   "Load device from bitstream buffer (Xilinx only)\n"
#if defined(CONFIG_CMD_FPGA_LOADBP)
	   "  loadbp\t[dev] [address] [size]\t"
	   "Load device from bitstream buffer with partial bitstream"
	   "(Xilinx only)\n"
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	   "Load device from filesystem (FAT by default) (Xilinx only)\n"
	   "  loadfs [dev] [address] [image size] [blocksize] <interface>\n"
	   "        [<dev[:part]>] <filename>\n"
#endif
#if defined(CONFIG_CMD_FPGA_LOADMK)
	   "  loadmk [dev] [address]\tLoad device generated with mkimage"
#if defined(CONFIG_FIT)
	   "\n"
	   "\tFor loadmk operating on FIT format uImage address must include\n"
	   "\tsubimage unit name in the form of addr:<subimg_uname>"
#endif
#endif
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	   "Load encrypted bitstream (Xilinx only)\n"
	   "  loads [dev] [address] [size] [auth-OCM-0/DDR-1/noauth-2]\n"
	   "        [enc-devkey(0)/userkey(1)/nenc(2) [Userkey address]\n"
	   "Loads the secure bistreams(authenticated/encrypted/both\n"
	   "authenticated and encrypted) of [size] from [address].\n"
	   "The auth-OCM/DDR flag specifies to perform authentication\n"
	   "in OCM or in DDR. 0 for OCM, 1 for DDR, 2 for no authentication.\n"
	   "The enc flag specifies which key to be used for decryption\n"
	   "0-device key, 1-user key, 2-no encryption.\n"
	   "The optional Userkey address specifies from which address key\n"
	   "has to be used for decryption if user key is selected.\n"
	   "NOTE: the secure bitstream has to be created using Xilinx\n"
	   "bootgen tool only.\n"
#endif
);
