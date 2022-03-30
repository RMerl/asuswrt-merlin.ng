// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Ryan Chen, ST Micoelectronics, ryan.chen@st.com.
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <nand.h>
#include <asm/io.h>
#include <linux/mtd/fsmc_nand.h>
#include <asm/mach-types.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_defs.h>
#include <asm/arch/spr_misc.h>

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];

int board_init(void)
{
	return spear_board_init(MACH_TYPE_SPEAR310);
}

/*
 * board_nand_init - Board specific NAND initialization
 * @nand:	mtd private chip structure
 *
 * Called by nand_init_chip to initialize the board specific functions
 */

void board_nand_init()
{
	struct misc_regs *const misc_regs_p =
	    (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	struct nand_chip *nand = &nand_chip[0];

#if defined(CONFIG_NAND_FSMC)
	if (((readl(&misc_regs_p->auto_cfg_reg) & MISC_SOCCFGMSK) ==
	     MISC_SOCCFG30) ||
	    ((readl(&misc_regs_p->auto_cfg_reg) & MISC_SOCCFGMSK) ==
	     MISC_SOCCFG31)) {

		fsmc_nand_init(nand);
	}
#endif
	return;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

#if defined(CONFIG_ETH_DESIGNWARE)
	u32 interface = PHY_INTERFACE_MODE_MII;
	if (designware_initialize(CONFIG_SPEAR_ETHBASE, interface) >= 0)
		ret++;
#endif
#if defined(CONFIG_MACB)
	if (macb_eth_initialize(0, (void *)CONFIG_SYS_MACB0_BASE,
				CONFIG_MACB0_PHY) >= 0)
		ret++;

	if (macb_eth_initialize(1, (void *)CONFIG_SYS_MACB1_BASE,
				CONFIG_MACB1_PHY) >= 0)
		ret++;

	if (macb_eth_initialize(2, (void *)CONFIG_SYS_MACB2_BASE,
				CONFIG_MACB2_PHY) >= 0)
		ret++;

	if (macb_eth_initialize(3, (void *)CONFIG_SYS_MACB3_BASE,
				CONFIG_MACB3_PHY) >= 0)
		ret++;
#endif
	return ret;
}
