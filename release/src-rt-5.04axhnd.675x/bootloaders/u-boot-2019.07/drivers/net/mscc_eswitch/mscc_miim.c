// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <miiphy.h>
#include <wait_bit.h>
#include "mscc_miim.h"

#define MIIM_STATUS			0x0
#define		MIIM_STAT_BUSY			BIT(3)
#define MIIM_CMD			0x8
#define		MIIM_CMD_SCAN		BIT(0)
#define		MIIM_CMD_OPR_WRITE	BIT(1)
#define		MIIM_CMD_OPR_READ	BIT(2)
#define		MIIM_CMD_SINGLE_SCAN	BIT(3)
#define		MIIM_CMD_WRDATA(x)	((x) << 4)
#define		MIIM_CMD_REGAD(x)	((x) << 20)
#define		MIIM_CMD_PHYAD(x)	((x) << 25)
#define		MIIM_CMD_VLD		BIT(31)
#define MIIM_DATA			0xC
#define		MIIM_DATA_ERROR		(0x2 << 16)

static int mscc_miim_wait_ready(struct mscc_miim_dev *miim)
{
	return wait_for_bit_le32(miim->regs + MIIM_STATUS, MIIM_STAT_BUSY,
				 false, 250, false);
}

int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	u32 val;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	writel(MIIM_CMD_VLD | MIIM_CMD_PHYAD(addr) |
	       MIIM_CMD_REGAD(reg) | MIIM_CMD_OPR_READ,
	       miim->regs + MIIM_CMD);

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	val = readl(miim->regs + MIIM_DATA);
	if (val & MIIM_DATA_ERROR) {
		ret = -EIO;
		goto out;
	}

	ret = val & 0xFFFF;
 out:
	return ret;
}

int mscc_miim_write(struct mii_dev *bus, int addr, int devad, int reg,
		    u16 val)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret < 0)
		goto out;

	writel(MIIM_CMD_VLD | MIIM_CMD_PHYAD(addr) |
	       MIIM_CMD_REGAD(reg) | MIIM_CMD_WRDATA(val) |
	       MIIM_CMD_OPR_WRITE, miim->regs + MIIM_CMD);
 out:
	return ret;
}
