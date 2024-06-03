// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

/*
 *
 * bcm281xx-specific clock tables
 *
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>
#include "clk-core.h"

#define CLOCK_1K		1000
#define CLOCK_1M		(CLOCK_1K * 1000)

/* declare a reference clock */
#define DECLARE_REF_CLK(clk_name, clk_parent, clk_rate, clk_div) \
static struct refclk clk_name = { \
	.clk    =       { \
		.name   =       #clk_name, \
		.parent =       clk_parent, \
		.rate   =       clk_rate, \
		.div    =       clk_div, \
		.ops    =       &ref_clk_ops, \
	}, \
}

/*
 * Reference clocks
 */

/* Declare a list of reference clocks */
DECLARE_REF_CLK(ref_crystal,	0,		26  * CLOCK_1M,	1);
DECLARE_REF_CLK(var_96m,	0,		96  * CLOCK_1M,	1);
DECLARE_REF_CLK(ref_96m,	0,		96  * CLOCK_1M,	1);
DECLARE_REF_CLK(ref_312m,	0,		312 * CLOCK_1M,	0);
DECLARE_REF_CLK(ref_104m,	&ref_312m.clk,	104 * CLOCK_1M,	3);
DECLARE_REF_CLK(ref_52m,	&ref_104m.clk,	52  * CLOCK_1M,	2);
DECLARE_REF_CLK(ref_13m,	&ref_52m.clk,	13  * CLOCK_1M,	4);
DECLARE_REF_CLK(var_312m,	0,		312 * CLOCK_1M,	0);
DECLARE_REF_CLK(var_104m,	&var_312m.clk,	104 * CLOCK_1M,	3);
DECLARE_REF_CLK(var_52m,	&var_104m.clk,	52  * CLOCK_1M,	2);
DECLARE_REF_CLK(var_13m,	&var_52m.clk,	13  * CLOCK_1M,	4);

struct refclk_lkup {
	struct refclk *procclk;
	const char *name;
};

/* Lookup table for string to clk tranlation */
#define MKSTR(x) {&x, #x}
static struct refclk_lkup refclk_str_tbl[] = {
	MKSTR(ref_crystal), MKSTR(var_96m), MKSTR(ref_96m),
	MKSTR(ref_312m), MKSTR(ref_104m), MKSTR(ref_52m),
	MKSTR(ref_13m), MKSTR(var_312m), MKSTR(var_104m),
	MKSTR(var_52m), MKSTR(var_13m),
};

int refclk_entries = sizeof(refclk_str_tbl)/sizeof(refclk_str_tbl[0]);

/* convert ref clock string to clock structure pointer */
struct refclk *refclk_str_to_clk(const char *name)
{
	int i;
	struct refclk_lkup *tblp = refclk_str_tbl;
	for (i = 0; i < refclk_entries; i++, tblp++) {
		if (!(strcmp(name, tblp->name)))
			return tblp->procclk;
	}
	return NULL;
}

/* frequency tables indexed by freq_id */
unsigned long master_axi_freq_tbl[8] = {
	26 * CLOCK_1M,
	52 * CLOCK_1M,
	104 * CLOCK_1M,
	156 * CLOCK_1M,
	156 * CLOCK_1M,
	208 * CLOCK_1M,
	312 * CLOCK_1M,
	312 * CLOCK_1M
};

unsigned long master_ahb_freq_tbl[8] = {
	26 * CLOCK_1M,
	52 * CLOCK_1M,
	52 * CLOCK_1M,
	52 * CLOCK_1M,
	78 * CLOCK_1M,
	104 * CLOCK_1M,
	104 * CLOCK_1M,
	156 * CLOCK_1M
};

unsigned long slave_axi_freq_tbl[8] = {
	26 * CLOCK_1M,
	52 * CLOCK_1M,
	78 * CLOCK_1M,
	104 * CLOCK_1M,
	156 * CLOCK_1M,
	156 * CLOCK_1M
};

unsigned long slave_apb_freq_tbl[8] = {
	26 * CLOCK_1M,
	26 * CLOCK_1M,
	39 * CLOCK_1M,
	52 * CLOCK_1M,
	52 * CLOCK_1M,
	78 * CLOCK_1M
};

unsigned long esub_freq_tbl[8] = {
	78 * CLOCK_1M,
	156 * CLOCK_1M,
	156 * CLOCK_1M,
	156 * CLOCK_1M,
	208 * CLOCK_1M,
	208 * CLOCK_1M,
	208 * CLOCK_1M
};

static struct bus_clk_data bsc1_apb_data = {
	.gate = HW_SW_GATE_AUTO(0x0458, 16, 0, 1),
};

static struct bus_clk_data bsc2_apb_data = {
	.gate = HW_SW_GATE_AUTO(0x045c, 16, 0, 1),
};

static struct bus_clk_data bsc3_apb_data = {
	.gate = HW_SW_GATE_AUTO(0x0484, 16, 0, 1),
};

/* * Master CCU clocks */
static struct peri_clk_data sdio1_data = {
	.gate		= HW_SW_GATE(0x0358, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_52m",
				 "ref_52m",
				 "var_96m",
				 "ref_96m"),
	.sel		= SELECTOR(0x0a28, 0, 3),
	.div		= DIVIDER(0x0a28, 4, 14),
	.trig		= TRIGGER(0x0afc, 9),
};

static struct peri_clk_data sdio2_data = {
	.gate		= HW_SW_GATE(0x035c, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_52m",
				 "ref_52m",
				 "var_96m",
				 "ref_96m"),
	.sel		= SELECTOR(0x0a2c, 0, 3),
	.div		= DIVIDER(0x0a2c, 4, 14),
	.trig		= TRIGGER(0x0afc, 10),
};

static struct peri_clk_data sdio3_data = {
	.gate		= HW_SW_GATE(0x0364, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_52m",
				 "ref_52m",
				 "var_96m",
				 "ref_96m"),
	.sel		= SELECTOR(0x0a34, 0, 3),
	.div		= DIVIDER(0x0a34, 4, 14),
	.trig		= TRIGGER(0x0afc, 12),
};

static struct peri_clk_data sdio4_data = {
	.gate		= HW_SW_GATE(0x0360, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_52m",
				 "ref_52m",
				 "var_96m",
				 "ref_96m"),
	.sel		= SELECTOR(0x0a30, 0, 3),
	.div		= DIVIDER(0x0a30, 4, 14),
	.trig		= TRIGGER(0x0afc, 11),
};

static struct peri_clk_data sdio1_sleep_data = {
	.clocks		= CLOCKS("ref_32k"),
	.gate		= SW_ONLY_GATE(0x0358, 20, 4),
};

static struct peri_clk_data sdio2_sleep_data = {
	.clocks		= CLOCKS("ref_32k"),
	.gate		= SW_ONLY_GATE(0x035c, 20, 4),
};

static struct peri_clk_data sdio3_sleep_data = {
	.clocks		= CLOCKS("ref_32k"),
	.gate		= SW_ONLY_GATE(0x0364, 20, 4),
};

static struct peri_clk_data sdio4_sleep_data = {
	.clocks		= CLOCKS("ref_32k"),
	.gate		= SW_ONLY_GATE(0x0360, 20, 4),
};

static struct bus_clk_data usb_otg_ahb_data = {
	.gate		= HW_SW_GATE_AUTO(0x0348, 16, 0, 1),
};

static struct bus_clk_data sdio1_ahb_data = {
	.gate		= HW_SW_GATE_AUTO(0x0358, 16, 0, 1),
};

static struct bus_clk_data sdio2_ahb_data = {
	.gate		= HW_SW_GATE_AUTO(0x035c, 16, 0, 1),
};

static struct bus_clk_data sdio3_ahb_data = {
	.gate		= HW_SW_GATE_AUTO(0x0364, 16, 0, 1),
};

static struct bus_clk_data sdio4_ahb_data = {
	.gate		= HW_SW_GATE_AUTO(0x0360, 16, 0, 1),
};

/* * Slave CCU clocks */
static struct peri_clk_data bsc1_data = {
	.gate		= HW_SW_GATE(0x0458, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_104m",
				 "ref_104m",
				 "var_13m",
				 "ref_13m"),
	.sel		= SELECTOR(0x0a64, 0, 3),
	.trig		= TRIGGER(0x0afc, 23),
};

static struct peri_clk_data bsc2_data = {
	.gate		= HW_SW_GATE(0x045c, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_104m",
				 "ref_104m",
				 "var_13m",
				 "ref_13m"),
	.sel		= SELECTOR(0x0a68, 0, 3),
	.trig		= TRIGGER(0x0afc, 24),
};

static struct peri_clk_data bsc3_data = {
	.gate		= HW_SW_GATE(0x0484, 18, 2, 3),
	.clocks		= CLOCKS("ref_crystal",
				 "var_104m",
				 "ref_104m",
				 "var_13m",
				 "ref_13m"),
	.sel		= SELECTOR(0x0a84, 0, 3),
	.trig		= TRIGGER(0x0b00, 2),
};

/*
 * CCU clocks
 */

static struct ccu_clock kpm_ccu_clk = {
	.clk = {
		.name = "kpm_ccu_clk",
		.ops = &ccu_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.num_policy_masks = 1,
	.policy_freq_offset = 0x00000008,
	.freq_bit_shift = 8,
	.policy_ctl_offset = 0x0000000c,
	.policy0_mask_offset = 0x00000010,
	.policy1_mask_offset = 0x00000014,
	.policy2_mask_offset = 0x00000018,
	.policy3_mask_offset = 0x0000001c,
	.lvm_en_offset = 0x00000034,
	.freq_id = 2,
	.freq_tbl = master_axi_freq_tbl,
};

static struct ccu_clock kps_ccu_clk = {
	.clk = {
		.name = "kps_ccu_clk",
		.ops = &ccu_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
	},
	.num_policy_masks = 2,
	.policy_freq_offset = 0x00000008,
	.freq_bit_shift = 8,
	.policy_ctl_offset = 0x0000000c,
	.policy0_mask_offset = 0x00000010,
	.policy1_mask_offset = 0x00000014,
	.policy2_mask_offset = 0x00000018,
	.policy3_mask_offset = 0x0000001c,
	.policy0_mask2_offset = 0x00000048,
	.policy1_mask2_offset = 0x0000004c,
	.policy2_mask2_offset = 0x00000050,
	.policy3_mask2_offset = 0x00000054,
	.lvm_en_offset = 0x00000034,
	.freq_id = 2,
	.freq_tbl = slave_axi_freq_tbl,
};

#ifdef CONFIG_BCM_SF2_ETH
static struct ccu_clock esub_ccu_clk = {
	.clk = {
		.name = "esub_ccu_clk",
		.ops = &ccu_clk_ops,
		.ccu_clk_mgr_base = ESUB_CLK_BASE_ADDR,
	},
	.num_policy_masks = 1,
	.policy_freq_offset = 0x00000008,
	.freq_bit_shift = 8,
	.policy_ctl_offset = 0x0000000c,
	.policy0_mask_offset = 0x00000010,
	.policy1_mask_offset = 0x00000014,
	.policy2_mask_offset = 0x00000018,
	.policy3_mask_offset = 0x0000001c,
	.lvm_en_offset = 0x00000034,
	.freq_id = 2,
	.freq_tbl = esub_freq_tbl,
};
#endif

/*
 * Bus clocks
 */

/* KPM bus clocks */
static struct bus_clock usb_otg_ahb_clk = {
	.clk = {
		.name = "usb_otg_ahb_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.freq_tbl = master_ahb_freq_tbl,
	.data = &usb_otg_ahb_data,
};

static struct bus_clock sdio1_ahb_clk = {
	.clk = {
		.name = "sdio1_ahb_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.freq_tbl = master_ahb_freq_tbl,
	.data = &sdio1_ahb_data,
};

static struct bus_clock sdio2_ahb_clk = {
	.clk = {
		.name = "sdio2_ahb_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.freq_tbl = master_ahb_freq_tbl,
	.data = &sdio2_ahb_data,
};

static struct bus_clock sdio3_ahb_clk = {
	.clk = {
		.name = "sdio3_ahb_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.freq_tbl = master_ahb_freq_tbl,
	.data = &sdio3_ahb_data,
};

static struct bus_clock sdio4_ahb_clk = {
	.clk = {
		.name = "sdio4_ahb_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.freq_tbl = master_ahb_freq_tbl,
	.data = &sdio4_ahb_data,
};

static struct bus_clock bsc1_apb_clk = {
	.clk = {
		.name = "bsc1_apb_clk",
		.parent = &kps_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
	},
	.freq_tbl = slave_apb_freq_tbl,
	.data = &bsc1_apb_data,
};

static struct bus_clock bsc2_apb_clk = {
	.clk = {
		.name = "bsc2_apb_clk",
		.parent = &kps_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
		},
	.freq_tbl = slave_apb_freq_tbl,
	.data = &bsc2_apb_data,
};

static struct bus_clock bsc3_apb_clk = {
	.clk = {
		.name = "bsc3_apb_clk",
		.parent = &kps_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
		},
	.freq_tbl = slave_apb_freq_tbl,
	.data = &bsc3_apb_data,
};

/* KPM peripheral */
static struct peri_clock sdio1_clk = {
	.clk = {
		.name = "sdio1_clk",
		.parent = &ref_52m.clk,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio1_data,
};

static struct peri_clock sdio2_clk = {
	.clk = {
		.name = "sdio2_clk",
		.parent = &ref_52m.clk,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio2_data,
};

static struct peri_clock sdio3_clk = {
	.clk = {
		.name = "sdio3_clk",
		.parent = &ref_52m.clk,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio3_data,
};

static struct peri_clock sdio4_clk = {
	.clk = {
		.name = "sdio4_clk",
		.parent = &ref_52m.clk,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio4_data,
};

static struct peri_clock sdio1_sleep_clk = {
	.clk = {
		.name = "sdio1_sleep_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio1_sleep_data,
};

static struct peri_clock sdio2_sleep_clk = {
	.clk = {
		.name = "sdio2_sleep_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio2_sleep_data,
};

static struct peri_clock sdio3_sleep_clk = {
	.clk = {
		.name = "sdio3_sleep_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio3_sleep_data,
};

static struct peri_clock sdio4_sleep_clk = {
	.clk = {
		.name = "sdio4_sleep_clk",
		.parent = &kpm_ccu_clk.clk,
		.ops = &bus_clk_ops,
		.ccu_clk_mgr_base = KONA_MST_CLK_BASE_ADDR,
	},
	.data = &sdio4_sleep_data,
};

/* KPS peripheral clock */
static struct peri_clock bsc1_clk = {
	.clk = {
		.name = "bsc1_clk",
		.parent = &ref_13m.clk,
		.rate = 13 * CLOCK_1M,
		.div = 1,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
	},
	.data = &bsc1_data,
};

static struct peri_clock bsc2_clk = {
	.clk = {
		.name = "bsc2_clk",
		.parent = &ref_13m.clk,
		.rate = 13 * CLOCK_1M,
		.div = 1,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
	},
	.data = &bsc2_data,
};

static struct peri_clock bsc3_clk = {
	.clk = {
		.name = "bsc3_clk",
		.parent = &ref_13m.clk,
		.rate = 13 * CLOCK_1M,
		.div = 1,
		.ops = &peri_clk_ops,
		.ccu_clk_mgr_base = KONA_SLV_CLK_BASE_ADDR,
	},
	.data = &bsc3_data,
};

/* public table for registering clocks */
struct clk_lookup arch_clk_tbl[] = {
	/* Peripheral clocks */
	CLK_LK(sdio1),
	CLK_LK(sdio2),
	CLK_LK(sdio3),
	CLK_LK(sdio4),
	CLK_LK(sdio1_sleep),
	CLK_LK(sdio2_sleep),
	CLK_LK(sdio3_sleep),
	CLK_LK(sdio4_sleep),
	CLK_LK(bsc1),
	CLK_LK(bsc2),
	CLK_LK(bsc3),
	/* Bus clocks */
	CLK_LK(usb_otg_ahb),
	CLK_LK(sdio1_ahb),
	CLK_LK(sdio2_ahb),
	CLK_LK(sdio3_ahb),
	CLK_LK(sdio4_ahb),
	CLK_LK(bsc1_apb),
	CLK_LK(bsc2_apb),
	CLK_LK(bsc3_apb),
#ifdef CONFIG_BCM_SF2_ETH
	CLK_LK(esub_ccu),
#endif
};

/* public array size */
unsigned int arch_clk_tbl_array_size = ARRAY_SIZE(arch_clk_tbl);
