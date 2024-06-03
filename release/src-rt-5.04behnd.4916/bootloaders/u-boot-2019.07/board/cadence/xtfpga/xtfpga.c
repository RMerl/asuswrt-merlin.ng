// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 - 2013 Tensilica Inc.
 * (C) Copyright 2014 - 2016 Cadence Design Systems Inc.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/platform_data/net_ethoc.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Check board idendity.
 * (Print information about the board to stdout.)
 */


#if defined(CONFIG_XTFPGA_LX60)
const char *board = "XT_AV60";
const char *description = "Avnet Xilinx LX60 FPGA Evaluation Board / ";
#elif defined(CONFIG_XTFPGA_LX110)
const char *board = "XT_AV110";
const char *description = "Avnet Xilinx Virtex-5 LX110 Evaluation Kit / ";
#elif defined(CONFIG_XTFPGA_LX200)
const char *board = "XT_AV200";
const char *description = "Avnet Xilinx Virtex-4 LX200 Evaluation Kit / ";
#elif defined(CONFIG_XTFPGA_ML605)
const char *board = "XT_ML605";
const char *description = "Xilinx Virtex-6 FPGA ML605 Evaluation Kit / ";
#elif defined(CONFIG_XTFPGA_KC705)
const char *board = "XT_KC705";
const char *description = "Xilinx Kintex-7 FPGA KC705 Evaluation Kit / ";
#else
const char *board = "<unknown>";
const char *description = "";
#endif

int checkboard(void)
{
	printf("Board: %s: %sTensilica bitstream\n", board, description);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_memstart = PHYSADDR(CONFIG_SYS_SDRAM_BASE);
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int board_postclk_init(void)
{
	/*
	 * Obtain CPU clock frequency from board and cache in global
	 * data structure (Hz). Return 0 on success (OK to continue),
	 * else non-zero (hang).
	 */

#ifdef CONFIG_SYS_FPGAREG_FREQ
	gd->cpu_clk = (*(volatile unsigned long *)CONFIG_SYS_FPGAREG_FREQ);
#else
	/* early Tensilica bitstreams lack this reg, but most run at 50 MHz */
	gd->cpu_clk = 50000000UL;
#endif
	return 0;
}

/*
 *  Miscellaneous late initializations.
 *  The environment has been set up, so we can set the Ethernet address.
 */

int misc_init_r(void)
{
#ifdef CONFIG_CMD_NET
	/*
	 * Initialize ethernet environment variables and board info.
	 * Default MAC address comes from CONFIG_ETHADDR + DIP switches 1-6.
	 */

	char *s = env_get("ethaddr");
	if (s == 0) {
		unsigned int x;
		char s[] = __stringify(CONFIG_ETHBASE);
		x = (*(volatile u32 *)CONFIG_SYS_FPGAREG_DIPSW)
			& FPGAREG_MAC_MASK;
		sprintf(&s[15], "%02x", x);
		env_set("ethaddr", s);
	}
#endif /* CONFIG_CMD_NET */

	return 0;
}

U_BOOT_DEVICE(sysreset) = {
	.name = "xtfpga_sysreset",
};

static struct ethoc_eth_pdata ethoc_pdata = {
	.eth_pdata = {
		.iobase = CONFIG_SYS_ETHOC_BASE,
	},
	.packet_base = CONFIG_SYS_ETHOC_BUFFER_ADDR,
};

U_BOOT_DEVICE(ethoc) = {
	.name = "ethoc",
	.platdata = &ethoc_pdata,
};
