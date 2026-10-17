// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <cpu_func.h>
#include <command.h>
#include <common.h>
#include <log.h>
#include <memalign.h>
#include <versalpl.h>
#include <zynqmp_firmware.h>

/**
 * do_versalnet_load_pdi - Handle the "versalnet load pdi" command-line command
 * @cmdtp:      Command data struct pointer
 * @flag:       Command flag
 * @argc:       Command-line argument count
 * @argv:       Array of command-line arguments
 *
 * Processes the Versal NET load pdi command
 *
 * Return: return 0 on success, Error value if command fails.
 * CMD_RET_USAGE incase of incorrect/missing parameters.
 */
static int do_versalnet_load_pdi(struct cmd_tbl *cmdtp, int flag, int argc,
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

	addr = simple_strtol(argv[1], NULL, 16);
	if (!addr) {
		debug("pdi_load: zero pdi_data address\n");
		return CMD_RET_USAGE;
	}

	len = hextoul(argv[2], NULL);
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

	return cmd_process_error(cmdtp, ret);
}

static char versalnet_help_text[] =
	"loadpdi addr len - Load pdi image\n"
	"load pdi image at ddr address 'addr' with pdi image size 'len'\n"
;

U_BOOT_CMD_WITH_SUBCMDS(versalnet, "Versal NET sub-system", versalnet_help_text,
			U_BOOT_SUBCMD_MKENT(loadpdi, 3, 1,
					    do_versalnet_load_pdi));
