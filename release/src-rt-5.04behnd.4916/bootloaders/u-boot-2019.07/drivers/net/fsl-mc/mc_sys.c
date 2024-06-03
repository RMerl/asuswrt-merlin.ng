// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright (C) 2013-2015 Freescale Semiconductor, Inc.
 * Author: German Rivera <German.Rivera@freescale.com>
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <common.h>
#include <errno.h>
#include <asm/io.h>

#define MC_CMD_HDR_READ_CMDID(_hdr) \
	((uint16_t)mc_dec((_hdr), MC_CMD_HDR_CMDID_O, MC_CMD_HDR_CMDID_S))

/**
 * mc_send_command - Send MC command and wait for response
 *
 * @mc_io: Pointer to MC I/O object to be used
 * @cmd: MC command buffer. On input, it contains the command to send to the MC.
 * On output, it contains the response from the MC if any.
 *
 * Depending on the sharing option specified when creating the MC portal
 * wrapper, this function will use a spinlock or mutex to ensure exclusive
 * access to the MC portal from the point when the command is sent until a
 * response is received from the MC.
 */
int mc_send_command(struct fsl_mc_io *mc_io,
		    struct mc_command *cmd)
{
	enum mc_cmd_status status;
	int timeout = 12000;

	mc_write_command(mc_io->mmio_regs, cmd);

	for ( ; ; ) {
		status = mc_read_response(mc_io->mmio_regs, cmd);
		if (status != MC_CMD_STATUS_READY)
			break;

		if (--timeout == 0) {
			printf("Error: Timeout waiting for MC response\n");
			return -ETIMEDOUT;
		}

		udelay(500);
	}

	if (status != MC_CMD_STATUS_OK) {
		printf("Error: MC command failed (portal: %p, obj handle: %#x, command: %#x, status: %#x)\n",
		       mc_io->mmio_regs,
			(unsigned int)MC_CMD_HDR_READ_TOKEN(cmd->header),
		       (unsigned int)MC_CMD_HDR_READ_CMDID(cmd->header),
		       (unsigned int)status);

		return -EIO;
	}

	return 0;
}
