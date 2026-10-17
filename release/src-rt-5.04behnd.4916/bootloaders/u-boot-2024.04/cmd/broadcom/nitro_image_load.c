// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom
 */

#include <common.h>
#include <command.h>

#define FW_IMAGE_SIG	0xff123456
#define CFG_IMAGE_SIG	0xcf54321a

/*
 * structure for bin file
 *  signature: fw itb file
 *  size: fw itb file
 *  signature: NS3 config file
 *  size: NS3 config file
 *  Data: fw itb file
 *  ............................
 *  ............................
 *  Data: NS3 config file
 *  ............................
 *  ............................
 */

static struct img_header {
	u32 bin_sig;
	u32 bin_size;
	u32 cfg1_sig;
	u32 cfg1_size;
} *img_header;

static int env_set_val(const char *varname, ulong val)
{
	int ret;

	ret = env_set_hex(varname, val);
	if (ret)
		pr_err("Failed to %s env var\n", varname);

	return ret;
}

static int do_spi_images_addr(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	uintptr_t images_load_addr;
	uintptr_t spi_load_addr;
	u32 len;
	u32 spi_data_offset = sizeof(struct img_header);

	if (argc != 3)
		return CMD_RET_USAGE;

	/* convert command parameter to fastboot address (base 16), i.e. hex */
	images_load_addr = hextoul(argv[1], NULL);
	if (!images_load_addr) {
		pr_err("Invalid load address\n");
		return CMD_RET_USAGE;
	}

	spi_load_addr = hextoul(argv[2], NULL);
	if (!spi_load_addr) {
		pr_err("Invalid spi load address\n");
		return CMD_RET_USAGE;
	}

	img_header = (struct img_header *)images_load_addr;

	if (img_header->bin_sig != FW_IMAGE_SIG) {
		pr_err("Invalid Nitro bin file\n");
		goto error;
	}

	if (env_set_val("spi_nitro_fw_itb_start_addr", 0))
		goto error;

	if (env_set_val("spi_nitro_fw_itb_len", 0))
		goto error;

	if (env_set_val("spi_nitro_fw_ns3_cfg_start_addr", 0))
		goto error;

	if (env_set_val("spi_nitro_fw_ns3_cfg_len", 0))
		goto error;

	len = img_header->bin_size;

	if (env_set_val("spi_nitro_fw_itb_start_addr",
			(spi_load_addr + spi_data_offset)))
		goto error;

	if (env_set_val("spi_nitro_fw_itb_len", img_header->bin_size))
		goto error;

	spi_data_offset += len;

	if (img_header->cfg1_sig == CFG_IMAGE_SIG) {
		len = img_header->cfg1_size;

		if (env_set_val("spi_nitro_fw_ns3_cfg_start_addr",
				(spi_load_addr + spi_data_offset)))
			goto error;

		if (env_set_val("spi_nitro_fw_ns3_cfg_len", len))
			goto error;

		spi_data_offset += len;
	}

	/* disable secure boot */
	if (env_set_val("nitro_fastboot_secure", 0))
		goto error;

	return CMD_RET_SUCCESS;

error:
	return CMD_RET_FAILURE;
}

U_BOOT_CMD
	(spi_nitro_images_addr, 3, 1, do_spi_images_addr,
	 "Load the bnxt bin header and sets envs ",
	 "spi_nitro_images_addr <load_addr> <spi_base_addr>\n"
);
