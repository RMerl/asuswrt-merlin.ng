// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <linux/io.h>
#include "mscc_mac_table.h"

#define ANA_TABLES_MACACCESS_VALID		BIT(11)
#define ANA_TABLES_MACACCESS_ENTRYTYPE(x)	((x) << 9)
#define ANA_TABLES_MACACCESS_DEST_IDX(x)	((x) << 3)
#define ANA_TABLES_MACACCESS_MAC_TABLE_CMD(x)	(x)
#define ANA_TABLES_MACACCESS_MAC_TABLE_CMD_M	GENMASK(2, 0)
#define MACACCESS_CMD_IDLE			0
#define MACACCESS_CMD_LEARN			1

/* MAC table entry types.
 * ENTRYTYPE_NORMAL is subject to aging.
 * ENTRYTYPE_LOCKED is not subject to aging.
 */
enum macaccess_entry_type {
	ENTRYTYPE_NORMAL = 0,
	ENTRYTYPE_LOCKED,
};

static int vlan_wait_for_completion(void __iomem *regs,
				    const unsigned long *mscc_mac_table_offset)
{
	unsigned int val, timeout = 10;

	/* Wait for the issued mac table command to be completed, or timeout.
	 * When the command read from ANA_TABLES_MACACCESS is
	 * MACACCESS_CMD_IDLE, the issued command completed successfully.
	 */
	do {
		val = readl(regs +
			    mscc_mac_table_offset[MSCC_ANA_TABLES_MACACCESS]);
		val &= ANA_TABLES_MACACCESS_MAC_TABLE_CMD_M;
	} while (val != MACACCESS_CMD_IDLE && timeout--);

	if (!timeout)
		return -ETIMEDOUT;

	return 0;
}

int mscc_mac_table_add(void __iomem *regs,
		       const unsigned long *mscc_mac_table_offset,
		       const unsigned char mac[ETH_LEN], int pgid)
{
	u32 macl = 0, mach = 0;

	/* Set the MAC address to handle and the vlan associated in a format
	 * understood by the hardware.
	 */
	mach |= MAC_VID << 16;
	mach |= ((u32)mac[0]) << 8;
	mach |= ((u32)mac[1]) << 0;
	macl |= ((u32)mac[2]) << 24;
	macl |= ((u32)mac[3]) << 16;
	macl |= ((u32)mac[4]) << 8;
	macl |= ((u32)mac[5]) << 0;

	writel(macl, regs + mscc_mac_table_offset[MSCC_ANA_TABLES_MACLDATA]);
	writel(mach, regs + mscc_mac_table_offset[MSCC_ANA_TABLES_MACHDATA]);

	writel(ANA_TABLES_MACACCESS_VALID |
	       ANA_TABLES_MACACCESS_DEST_IDX(pgid) |
	       ANA_TABLES_MACACCESS_ENTRYTYPE(ENTRYTYPE_LOCKED) |
	       ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MACACCESS_CMD_LEARN),
	       regs + mscc_mac_table_offset[MSCC_ANA_TABLES_MACACCESS]);

	return vlan_wait_for_completion(regs, mscc_mac_table_offset);
}
