// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>

int mscc_phy_rd_wr(u8 read,
		   u32 miimdev,
		   u8 miim_addr,
		   u8 addr,
		   u16 *value)
{
	u32 data;
	int i;

	/* Command part */
	data = (read ? MSCC_F_MII_CMD_MIIM_CMD_OPR_FIELD(2) : /* Read */
		MSCC_F_MII_CMD_MIIM_CMD_OPR_FIELD(1) | /* Write */
		MSCC_F_MII_CMD_MIIM_CMD_WRDATA(*value)); /* value */

	/* Addressing part */
	data |=
		MSCC_F_MII_CMD_MIIM_CMD_VLD(1) | /* Valid command */
		MSCC_F_MII_CMD_MIIM_CMD_REGAD(addr) | /* Reg addr */
		MSCC_F_MII_CMD_MIIM_CMD_PHYAD(miim_addr); /* Miim addr */

	/* Enqueue MIIM operation to be executed */
	writel(data, BASE_DEVCPU_GCB + MIIM_MII_CMD(miimdev));

	/* Wait for MIIM operation to finish */
	i = 0;
	do {
		if (i++ > 100) {
			debug("Miim timeout");
			return -1;
		}
		data = readl(BASE_DEVCPU_GCB + MIIM_MII_STATUS(miimdev));
		debug("Read status miim(%d): 0x%08x\n", miimdev, data);
	} while (data & MSCC_F_MII_STATUS_MIIM_STAT_BUSY(1));

	if (read) {
		data = readl(BASE_DEVCPU_GCB + MIIM_MII_DATA(miimdev));
		if (data & MSCC_M_MII_DATA_MIIM_DATA_SUCCESS) {
			debug("Read(%d, %d) returned 0x%08x\n",
			      miim_addr, addr, data);
			return -1;
		}
		*value = MSCC_X_MII_DATA_MIIM_DATA_RDDATA(data);
	}

	return 0;
}

int mscc_phy_rd(u32 miimdev,
		u8 miim_addr,
		u8 addr,
		u16 *value)
{
	if (mscc_phy_rd_wr(1, miimdev, miim_addr, addr, value) == 0)
		return 0;
	debug("Read(%d, %d) returned error\n", miim_addr, addr);
	return -1;
}

int mscc_phy_wr(u32 miimdev,
		u8 miim_addr,
		u8 addr,
		u16 value)
{
	return mscc_phy_rd_wr(0, miimdev, miim_addr, addr, &value);
}
