// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 * Copyright (C) 2021 Pali Rohár <pali@kernel.org>
 */

#include <common.h>
#include <asm/arch/soc.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/mbox.h>

#define RWTM_BASE		(MVEBU_REGISTER(0xb0000))
#define RWTM_CMD_PARAM(i)	(size_t)(RWTM_BASE + (i) * 4)
#define RWTM_CMD		(RWTM_BASE + 0x40)
#define RWTM_CMD_RETSTATUS	(RWTM_BASE + 0x80)
#define RWTM_CMD_STATUS(i)	(size_t)(RWTM_BASE + 0x84 + (i) * 4)
#define MAX_ARGS		16

#define RWTM_HOST_INT_RESET	(RWTM_BASE + 0xc8)
#define RWTM_HOST_INT_MASK	(RWTM_BASE + 0xcc)
#define SP_CMD_COMPLETE		BIT(0)

#define MBOX_STS_SUCCESS		(0x0 << 30)
#define MBOX_STS_FAIL			(0x1 << 30)
#define MBOX_STS_BADCMD			(0x2 << 30)
#define MBOX_STS_LATER			(0x3 << 30)
#define MBOX_STS_ERROR(s)		((s) & (3 << 30))
#define MBOX_STS_VALUE(s)		(((s) >> 10) & 0xfffff)
#define MBOX_STS_CMD(s)			((s) & 0x3ff)
#define MBOX_STS_MARVELL_ERROR(s)	((s) == 0 ? 0         : \
					 (s) == 2 ? ETIMEDOUT : \
					 (s) == 3 ? EINVAL    : \
					 (s) == 4 ? ENOSYS    : \
					            EIO)

int mbox_do_cmd(enum mbox_cmd cmd, u32 *in, int nin, u32 *out, int nout)
{
	const int tries = 50;
	int i;
	u32 status;

	if (nin > MAX_ARGS || nout > MAX_ARGS)
		return -EINVAL;

	clrbits_le32(RWTM_HOST_INT_MASK, SP_CMD_COMPLETE);

	for (i = 0; i < nin; i++)
		writel(in[i], RWTM_CMD_PARAM(i));
	for (; i < MAX_ARGS; i++)
		writel(0x0, RWTM_CMD_PARAM(i));
	writel(cmd, RWTM_CMD);

	for (i = 0; i < tries; ++i) {
		mdelay(10);
		if (readl(RWTM_HOST_INT_RESET) & SP_CMD_COMPLETE)
			break;
	}

	if (i == tries) {
		/* if timed out, don't read status */
		setbits_le32(RWTM_HOST_INT_RESET, SP_CMD_COMPLETE);
		return -ETIMEDOUT;
	}

	for (i = 0; i < nout; ++i)
		out[i] = readl(RWTM_CMD_STATUS(i));
	status = readl(RWTM_CMD_RETSTATUS);

	setbits_le32(RWTM_HOST_INT_RESET, SP_CMD_COMPLETE);

	if (MBOX_STS_CMD(status) != cmd)
		return -MBOX_STS_MARVELL_ERROR(status);
	else if (MBOX_STS_ERROR(status) == MBOX_STS_FAIL)
		return -(int)MBOX_STS_VALUE(status);
	else if (MBOX_STS_ERROR(status) == MBOX_STS_BADCMD)
		return -ENOSYS;
	else if (MBOX_STS_ERROR(status) != MBOX_STS_SUCCESS)
		return -EIO;
	else
		return MBOX_STS_VALUE(status);
}
