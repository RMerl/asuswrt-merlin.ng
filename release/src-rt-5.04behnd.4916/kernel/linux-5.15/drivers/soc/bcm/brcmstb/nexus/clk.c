// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2018-2022 Broadcom */


#include <linux/brcmstb/brcmstb.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include "brcm_clk.h"

static struct brcm_clk_iface {
	/* clks -- contiguous array of SW clocks */
	struct clk			**clks;
	unsigned int			num_clks;
	struct mutex			lock;
	const char * const		*clk_names;
	struct device			*dev;
} *iface;

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
	"sw_cpu2", "sw_cpu3", "sw_dvpht_core", "sw_hvs0",
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

static inline bool brcm_is_sw_clk(unsigned int clk_id)
{
	return (clk_id >= BCLK_SW_OFFSET) &&
		(clk_id < BCLK_SW_OFFSET + iface->num_clks);
}

static inline int brcm_get_clk_idx(unsigned int clk_id)
{
	int idx = -1;

	if (clk_id == BCLK_SW_DVPHT_CORE)
		clk_id = BCLK_SW_DVPHT;

	if (brcm_is_sw_clk(clk_id))
		idx = clk_id - BCLK_SW_OFFSET;
	else
		pr_debug("brcmstb-clk: bad clk_id: 0x%x\n", clk_id);

	return idx;
}

/* This is called if one is sure the clock has already been gotten */
static struct clk *brcm_find_clk(unsigned int clk_id)
{
	int idx = brcm_get_clk_idx(clk_id);

	return idx < 0 ? NULL : iface->clks[idx];
}


static int brcm_clk_name_to_idx(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clk_names_stb); i++)
		if (strcmp(name, clk_names_stb[i]) == 0)
			return i;
	return -1;
}

int brcm_clk_prepare_enable(unsigned int clk_id)
{
	struct clk *clk = brcm_find_clk(clk_id);

	return clk_prepare_enable(clk);
}
EXPORT_SYMBOL(brcm_clk_prepare_enable);

void brcm_clk_disable_unprepare(unsigned int clk_id)
{
	struct clk *clk = brcm_find_clk(clk_id);

	clk_disable_unprepare(clk);
}
EXPORT_SYMBOL(brcm_clk_disable_unprepare);

int brcm_clk_get_rate(unsigned int clk_id, u64 *rate)
{
	struct clk *clk = brcm_find_clk(clk_id);

	*rate = (u64)clk_get_rate(clk);

	return 0;
}
EXPORT_SYMBOL(brcm_clk_get_rate);

int brcm_clk_init(struct device *dev)
{
	struct device_node *dn = dev->of_node;
	const int n = of_property_count_strings(dn, "clock-names");
	struct clk *clk;
	int i, count = 0;

	iface = devm_kzalloc(dev, sizeof(struct brcm_clk_iface), GFP_KERNEL);
	if (!iface)
		return -ENOMEM;

	iface->dev = dev;
	iface->clk_names = clk_names_stb;
	iface->num_clks = ARRAY_SIZE(clk_names_stb);
	iface->clks = devm_kcalloc(dev, iface->num_clks, sizeof(struct clk *),
				   GFP_KERNEL);
	if (!iface->clks) {
		iface = NULL;
		return -ENOMEM;
	}

	mutex_init(&iface->lock);
	mutex_lock(&iface->lock);

	for (i = 0; i < n; i++) {
		const char *name = NULL;
		int idx;

		of_property_read_string_index(dn, "clock-names", i, &name);
		idx = brcm_clk_name_to_idx(name);
		if (idx < 0) {
			dev_err(dev, "no brcm API match for clk '%s'\n", name);
			continue;
		}

		clk = devm_clk_get(dev, name);
		if (IS_ERR(clk)) {
			dev_err(dev, "failed to get clk '%s'\n", name);
		} else {
			iface->clks[idx] = clk;
			count++;
		}
	}
	mutex_unlock(&iface->lock);
	dev_info(dev, "matched %d/%d clocks for Nexus/Linux API\n", count, n);

	return 0;
}
EXPORT_SYMBOL(brcm_clk_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Broadcom STB Clock Interface Driver");
MODULE_AUTHOR("Broadcom");
