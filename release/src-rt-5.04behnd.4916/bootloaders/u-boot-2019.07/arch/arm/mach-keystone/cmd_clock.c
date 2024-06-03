// SPDX-License-Identifier: GPL-2.0+
/*
 * keystone2: commands for clocks
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/arch/psc_defs.h>

struct pll_init_data cmd_pll_data = {
	.pll = MAIN_PLL,
	.pll_m = 16,
	.pll_d = 1,
	.pll_od = 2,
};

int do_pll_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 5)
		goto pll_cmd_usage;

	if (strncmp(argv[1], "pa", 2) == 0)
		cmd_pll_data.pll = PASS_PLL;
#ifndef CONFIG_SOC_K2E
	else if (strncmp(argv[1], "arm", 3) == 0)
		cmd_pll_data.pll = TETRIS_PLL;
#endif
#ifdef CONFIG_SOC_K2HK
	else if (strncmp(argv[1], "ddr3a", 5) == 0)
		cmd_pll_data.pll = DDR3A_PLL;
	else if (strncmp(argv[1], "ddr3b", 5) == 0)
		cmd_pll_data.pll = DDR3B_PLL;
#else
	else if (strncmp(argv[1], "ddr3", 4) == 0)
		cmd_pll_data.pll = DDR3_PLL;
#endif
	else
		goto pll_cmd_usage;

	cmd_pll_data.pll_m   = simple_strtoul(argv[2], NULL, 10);
	cmd_pll_data.pll_d   = simple_strtoul(argv[3], NULL, 10);
	cmd_pll_data.pll_od  = simple_strtoul(argv[4], NULL, 10);

	printf("Trying to set pll %d; mult %d; div %d; OD %d\n",
	       cmd_pll_data.pll, cmd_pll_data.pll_m,
	       cmd_pll_data.pll_d, cmd_pll_data.pll_od);
	init_pll(&cmd_pll_data);

	return 0;

pll_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	pllset, 5,      0,      do_pll_cmd,
	"set pll multiplier and pre divider",
	PLLSET_CMD_LIST " <mult> <div> <OD>\n"
);

int do_getclk_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int clk;
	unsigned long freq;

	if (argc != 2)
		goto getclk_cmd_usage;

	clk = simple_strtoul(argv[1], NULL, 10);

	freq = ks_clk_get_rate(clk);
	if (freq)
		printf("clock index [%d] - frequency %lu\n", clk, freq);
	else
		printf("clock index [%d] Not available\n", clk);
	return 0;

getclk_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	getclk,	2,	0,	do_getclk_cmd,
	"get clock rate",
	"<clk index>\n"
	"The indexes for clocks:\n"
	CLOCK_INDEXES_LIST
);

int do_psc_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int	psc_module;
	int	res;

	if (argc != 3)
		goto psc_cmd_usage;

	psc_module = simple_strtoul(argv[1], NULL, 10);
	if (strcmp(argv[2], "en") == 0) {
		res = psc_enable_module(psc_module);
		printf("psc_enable_module(%d) - %s\n", psc_module,
		       (res) ? "ERROR" : "OK");
		return 0;
	}

	if (strcmp(argv[2], "di") == 0) {
		res = psc_disable_module(psc_module);
		printf("psc_disable_module(%d) - %s\n", psc_module,
		       (res) ? "ERROR" : "OK");
		return 0;
	}

	if (strcmp(argv[2], "domain") == 0) {
		res = psc_disable_domain(psc_module);
		printf("psc_disable_domain(%d) - %s\n", psc_module,
		       (res) ? "ERROR" : "OK");
		return 0;
	}

psc_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	psc,	3,	0,	do_psc_cmd,
	"<enable/disable psc module os disable domain>",
	"<mod/domain index> <en|di|domain>\n"
	"Intended to control Power and Sleep Controller (PSC) domains and\n"
	"modules. The module or domain index exectly corresponds to ones\n"
	"listed in official TRM. For instance, to enable MSMC RAM clock\n"
	"domain use command: psc 14 en.\n"
);
