#include <stdio.h>
#include <string.h>
#include "internal.h"

int lan78xx_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		      struct ethtool_regs *regs)
{
	unsigned int *lan78xx_reg = (unsigned int *)regs->data;

	fprintf(stdout, "LAN78xx Registers:\n");
	fprintf(stdout, "------------------\n");
	fprintf(stdout, "ID_REV       = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "INT_STS      = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "HW_CFG       = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "PMT_CTRL     = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "E2P_CMD      = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "E2P_DATA     = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "USB_STATUS   = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "VLAN_TYPE    = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "\n");

	fprintf(stdout, "MAC Registers:\n");
	fprintf(stdout, "--------------\n");
	fprintf(stdout, "MAC_CR             = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "MAC_RX             = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "MAC_TX             = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "FLOW               = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "ERR_STS            = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "MII_ACC            = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "MII_DATA           = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "EEE_TX_LPI_REQ_DLY = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "EEE_TW_TX_SYS      = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "EEE_TX_LPI_REM_DLY = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "WUCSR              = 0x%08X\n", *lan78xx_reg++);
	fprintf(stdout, "\n");

	fprintf(stdout, "PHY Registers:\n");
	fprintf(stdout, "--------------\n");
	fprintf(stdout, "Mode Control = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Mode Status  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Device identifier1   = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Device identifier2   = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Auto-Neg Advertisement         = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "Auto-Neg Link Partner Ability  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "Auto-Neg Expansion      = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Auto-Neg Next Page TX   = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Auto-Neg Link Partner Next Page RX  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "1000BASE-T Control  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "1000BASE-T Status   = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Reserved  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Reserved  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "MMD Access Control       = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "MMD Access Address/Data  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "1000BASE-T Status Extension1  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "1000BASE-TX Status Extension  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "1000BASE-T Status Extension2  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "Bypass Control  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout,
		"100BASE-TX/1000BASE-T Rx Error Counter    = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout,
		"100BASE-TX/1000BASE-T FC Err Counter      = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout,
		"10BASE-T/100BASE-TX/1000BASE-T LD Counter = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "Extended 10BASE-T Control and Status      = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "Extended PHY Control1  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Extended PHY Control2  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Interrupt Mask    = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Interrupt Status  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Reserved  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Auxiliary Control and Status  = 0x%04X\n",
		*lan78xx_reg++);
	fprintf(stdout, "LED Mode Select  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "LED Behavior     = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "Extended Page Access  = 0x%04X\n", *lan78xx_reg++);
	fprintf(stdout, "\n");

	return 0;
}
