// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2022 Microchip Technology Inc. and its subsidiaries. */

#include <stdio.h>
#include <string.h>
#include "internal.h"

#define LAN743X_ETH_REG_VERSION		1

enum {
	ETH_PRIV_FLAGS,
	ETH_ID_REV,
	ETH_FPGA_REV,
	ETH_STRAP_READ,
	ETH_INT_STS,
	ETH_HW_CFG,
	ETH_PMT_CTL,
	ETH_E2P_CMD,
	ETH_E2P_DATA,
	ETH_MAC_CR,
	ETH_MAC_RX,
	ETH_MAC_TX,
	ETH_FLOW,
	ETH_MII_ACC,
	ETH_MII_DATA,
	ETH_EEE_TX_LPI_REQ_DLY,
	ETH_WUCSR,
	ETH_WK_SRC,

	/* Add new registers above */
	MAX_LAN743X_ETH_REGS
};

void lan743x_comm_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
			    struct ethtool_regs *regs)
{
	u32 *lan743x_reg = (u32 *)regs->data;

	fprintf(stdout, "LAN743x Registers:\n");
	fprintf(stdout, "------------------\n");
	fprintf(stdout, "CHIP_ID_REV  = 0x%08X\n", lan743x_reg[ETH_ID_REV]);
	fprintf(stdout, "FPGA_REV     = 0x%08X\n", lan743x_reg[ETH_FPGA_REV]);
	fprintf(stdout, "STRAP_READ   = 0x%08X\n", lan743x_reg[ETH_STRAP_READ]);
	fprintf(stdout, "INT_STS      = 0x%08X\n", lan743x_reg[ETH_INT_STS]);
	fprintf(stdout, "HW_CFG       = 0x%08X\n", lan743x_reg[ETH_HW_CFG]);
	fprintf(stdout, "PMT_CTRL     = 0x%08X\n", lan743x_reg[ETH_PMT_CTL]);
	fprintf(stdout, "E2P_CMD      = 0x%08X\n", lan743x_reg[ETH_E2P_CMD]);
	fprintf(stdout, "E2P_DATA     = 0x%08X\n", lan743x_reg[ETH_E2P_DATA]);
	fprintf(stdout, "\n");

	fprintf(stdout, "MAC Registers:\n");
	fprintf(stdout, "--------------\n");
	fprintf(stdout, "MAC_CR       = 0x%08X\n", lan743x_reg[ETH_MAC_CR]);
	fprintf(stdout, "MAC_RX       = 0x%08X\n", lan743x_reg[ETH_MAC_RX]);
	fprintf(stdout, "MAC_TX       = 0x%08X\n", lan743x_reg[ETH_MAC_TX]);
	fprintf(stdout, "FLOW         = 0x%08X\n", lan743x_reg[ETH_FLOW]);
	fprintf(stdout, "MII_ACC      = 0x%08X\n", lan743x_reg[ETH_MII_ACC]);
	fprintf(stdout, "MII_DATA     = 0x%08X\n", lan743x_reg[ETH_MII_DATA]);
	fprintf(stdout, "WUCSR        = 0x%08X\n", lan743x_reg[ETH_WUCSR]);
	fprintf(stdout, "WK_SRC       = 0x%08X\n", lan743x_reg[ETH_WK_SRC]);
	fprintf(stdout, "EEE_TX_LPI_REQ_DLY = 0x%08X\n",
					lan743x_reg[ETH_EEE_TX_LPI_REQ_DLY]);
	fprintf(stdout, "\n");
}

int lan743x_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		      struct ethtool_regs *regs)
{

	lan743x_comm_dump_regs(info, regs);

	return 0;
}
