// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2020 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <cpu_func.h>
#include <command.h>
#include <common.h>
#include <log.h>
#include <memalign.h>
#include <versalpl.h>
#include <zynqmp_firmware.h>

static int do_versal_load_pdi(struct cmd_tbl *cmdtp, int flag, int argc,
			      char * const argv[])
{
	u32 buf_lo, buf_hi;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	ulong addr, *pdi_buf;
	size_t len;
	int ret;

	if (argc != cmdtp->maxargs) {
		debug("pdi_load: incorrect parameters passed\n");
		return CMD_RET_USAGE;
	}

	addr = simple_strtol(argv[2], NULL, 16);
	if (!addr) {
		debug("pdi_load: zero pdi_data address\n");
		return CMD_RET_USAGE;
	}

	len = hextoul(argv[3], NULL);
	if (!len) {
		debug("pdi_load: zero size\n");
		return CMD_RET_USAGE;
	}

	pdi_buf = (ulong *)ALIGN((ulong)addr, ARCH_DMA_MINALIGN);
	if ((ulong)addr != (ulong)pdi_buf) {
		memcpy((void *)pdi_buf, (void *)addr, len);
		debug("Pdi addr:0x%lx aligned to 0x%lx\n",
		      addr, (ulong)pdi_buf);
	}

	flush_dcache_range((ulong)pdi_buf, (ulong)pdi_buf + len);

	buf_lo = lower_32_bits((ulong)pdi_buf);
	buf_hi = upper_32_bits((ulong)pdi_buf);

	ret = xilinx_pm_request(VERSAL_PM_LOAD_PDI, VERSAL_PM_PDI_TYPE, buf_lo,
				buf_hi, 0, ret_payload);
	if (ret)
		printf("PDI load failed with err: 0x%08x\n", ret);

	return ret;
}

static struct cmd_tbl cmd_versal_sub[] = {
	U_BOOT_CMD_MKENT(loadpdi, 4, 1, do_versal_load_pdi, "", ""),
};

/**
 * do_versal - Handle the "versal" command-line command
 * @cmdtp:      Command data struct pointer
 * @flag:       Command flag
 * @argc:       Command-line argument count
 * @argv:       Array of command-line arguments
 *
 * Processes the versal specific commands
 *
 * Return: return 0 on success, Error value if command fails.
 * CMD_RET_USAGE incase of incorrect/missing parameters.
 */
static int do_versal(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	struct cmd_tbl *c;
	int ret = CMD_RET_USAGE;

	if (argc < 2)
		return CMD_RET_USAGE;

	c = find_cmd_tbl(argv[1], &cmd_versal_sub[0],
			 ARRAY_SIZE(cmd_versal_sub));
	if (c)
		ret = c->cmd(c, flag, argc, argv);

	return cmd_process_error(c, ret);
}

U_BOOT_LONGHELP(versal,
	"loadpdi addr len - Load pdi image\n"
	"load pdi image at ddr address 'addr' with pdi image size 'len'\n");

U_BOOT_CMD(versal, 4, 1, do_versal,
	   "versal sub-system",
	   versal_help_text
);
