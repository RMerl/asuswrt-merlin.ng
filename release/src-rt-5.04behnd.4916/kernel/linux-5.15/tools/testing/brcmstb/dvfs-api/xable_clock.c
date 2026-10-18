// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2022 Broadcom */
#include <linux/brcmstb/brcmstb.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>

/*
 * USAGE:
 *   insmod xable_clock.ko  name=<clock> [enable=<int>]
 *
 * EXAMPLES:
 *
 *   insmod xable_clock.ko name=sw_xpt
 *   insmod xable_clock.ko name=sw_xpt enable=1
 *   insmod xable_clock.ko name=sw_xpt enable=0
 *   insmod xable_clock.ko name=show_rates
 *
 * NOTE:
 *   The insmod will always fail to do the insmod operation but
 *   should execut the clock dis/enable -- this is by design so the
 *   user does not have to keep running an rmmod for every insmod.
 */


/*
 * Stand-alone build example:
 *
 * $ cat Makefile
 * obj-m += xable_clock.o
 *
 * $ make ARCH=arm64 CROSS_COMPILE=aarch64-linux- \
 *	-C /work3/jquinlan/git/515-arm64/linux SUBDIRS=$PWD modules
 */
#include <linux/brcmstb/clk_api.h>

static int enable = 1;
module_param(enable, int, 0660);

static char *name = "need_help";
module_param(name, charp, 0660);

/* These strings' positions must align with the defs in clk_api.h */
static const char * const clk_names_stb[] = {
	/* Software Clocks and/or Cores */
	/* [00..0f] */
	"sw_cpu0", "sw_v3d", "sw_sysif", "sw_scb",
	"sw_hvd0", "sw_raaga0", "sw_vice0", "sw_vice0_pss",
	"sw_vice1", "sw_vice1_pss", "sw_xpt", "sw_m2mc0",
	"sw_m2mc1", "sw_mipmap0", "sw_tsx0", "sw_smartcard0",

	/* [10..1f] */
	"sw_smartcard1", "reserved", "sw_bne", "sw_asp",
	"sw_hvd_cabac0", "sw_axi0", "sw_bstm", "sw_cpu1",
	"sw_cpu2", "sw_cpu3", "sw_dvpht_core", "reserved",
	"reserved", "reserved", "reserved", "reserved",

	/* [20..2f] */
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",

	/* Software Clocks ONLY */
	/* [30..3f] */
	"sw_aio", "sw_bvn", "sw_dvphr", "sw_dvpht",
	"sw_genet0", "sw_genetwol0", "sw_hvd0_cpu", "sw_itu656",
	"sw_mmm2mc0", "sw_pcie0", "sw_pcie1", "sw_potp",
	"sw_raaga0_cpu", "sw_sata3", "sw_sdio0", "sw_sdio1",

	/* [40..4f] */
	"sw_sid", "sw_v3d_cpu", "sw_vec", "sw_xpt_wakeup",
	"sw_tsio", "sw_mbvn", "sw_raaga0_wkup", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",

	/* [50..5f] */
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",

	/* [60..6f] */
	"sw_aio_sram", "sw_bvn_sram", "sw_dvphr_sram", "sw_hvd0_sram",
	"sw_m2mc0_sram", "sw_m2mc1_sram", "sw_mmm2mc0_sram", "sw_raaga0_sram",
	"sw_v3d_sram", "sw_vec_sram", "sw_vice0_sram", "sw_vice1_sram",
	"sw_xpt_sram",
};

static int get_clk_id(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clk_names_stb); i++)
		if (strcmp(name, clk_names_stb[i]) == 0)
			return i + BCLK_SW_OFFSET;
	return -1;
}


static int xable_clock_init(void)
{
	int i, n, ret = 0, clk_id = -1;
	bool show_rates = !strcmp(name, "show_rates");
	bool need_help = !strcmp(name, "need_help");

	if (need_help) {
		pr_info("USAGE:\n  insmod xable_clock.ko name=<clk> [enable=<int>]\n");
		pr_info("\nEXAMPLES:\n");
		pr_info("\tinsmod xable_clock.ko name=sw_xpt /* Enable is implied */\n");
		pr_info("\tinsmod xable_clock.ko name=sw_xpt enable=1\n");
		pr_info("\tinsmod xable_clock.ko name=sw_xpt enable=0\n");
		pr_info("\tinsmod xable_clock.ko name=sw_xpt enable=0\n");
		pr_info("\nAVAILABLE CLOCKS:\n");
	}

	if (need_help || show_rates) {
		struct device_node *dn, *cdn;
		const char *avail_name;
		int len;

		pr_info("\nBRCM SW CLOCKS:\n");

		dn = of_find_compatible_node(NULL, NULL, "arm,scmi-smc");
		if (!dn)
			goto node_err;


		for (cdn = NULL; (cdn = of_get_next_child(dn, cdn)) != NULL;)
			if (of_find_property(cdn, "brcm,clk-ver", &len))
				break;

		if (!cdn)
			goto node_err;

		n = of_property_count_strings(cdn, "clock-names");
		for (i = 0; i < n; i++) {
			u64 rate = 0;

			of_property_read_string_index(cdn, "clock-names", i, &avail_name);

			if (show_rates) {
				ret = -1;
				clk_id = get_clk_id(avail_name);
				if (clk_id >= 0)
					ret = brcm_clk_get_rate(clk_id, &rate);
				if (ret < 0)
					pr_info("  %-16s  %12s\n", avail_name, "-");
				else
					pr_info("  %-16s  %12lld\n", avail_name,
						(unsigned long long)rate);
			} else {
				pr_info("  %-16s\n", avail_name);
			}
		}
		pr_info("\n");
		return -EIO;
	}

	clk_id = get_clk_id(name);
	if (clk_id < 0) {
		pr_info("Err: clk '%s' doesn't seem to be in the Clock API\n", name);
		return -EIO;
	}

	if (!!enable)
		ret = brcm_clk_prepare_enable(clk_id);
	else
		brcm_clk_disable_unprepare(clk_id);

	pr_info("\n%s to %s clock %s\n\n", ret ? "FAILED" : "Succeeded",
		enable ? "enable" : "disable", name);
	pr_info("[By design, this insmod will always fail]\n\n");

	return -EIO;

node_err:
	pr_info("\tSorry, could not find SCMI BRCM proto DT node\n\n");
	return -EIO;
}


static void xable_clock_exit(void)
{
}

module_init(xable_clock_init);
module_exit(xable_clock_exit);
MODULE_LICENSE("GPL");
