/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#ifndef __AT91_PMC_H__
#define __AT91_PMC_H__

#include <linux/bitops.h>
#include <linux/io.h>

/* Keep a range of 256 available clocks for every clock type. */
#define AT91_TO_CLK_ID(_t, _i)		(((_t) << 8) | ((_i) & 0xff))
#define AT91_CLK_ID_TO_DID(_i)		((_i) & 0xff)

struct clk_range {
	unsigned long min;
	unsigned long max;
};

struct clk_master_layout {
	u32 offset;
	u32 mask;
	u8 pres_shift;
};

extern const struct clk_master_layout at91rm9200_master_layout;
extern const struct clk_master_layout at91sam9x5_master_layout;

struct clk_master_characteristics {
	struct clk_range output;
	u32 divisors[5];
	u8 have_div3_pres;
};

struct clk_pll_characteristics {
	struct clk_range input;
	int num_output;
	const struct clk_range *output;
	u16 *icpll;
	u8 *out;
	u8 upll : 1;
};

struct clk_pll_layout {
	u32 pllr_mask;
	u32 mul_mask;
	u32 frac_mask;
	u32 div_mask;
	u32 endiv_mask;
	u8 mul_shift;
	u8 frac_shift;
	u8 div_shift;
	u8 endiv_shift;
};

struct clk_programmable_layout {
	u8 pres_mask;
	u8 pres_shift;
	u8 css_mask;
	u8 have_slck_mck;
	u8 is_pres_direct;
};

struct clk_pcr_layout {
	u32 offset;
	u32 cmd;
	u32 div_mask;
	u32 gckcss_mask;
	u32 pid_mask;
};

struct clk_usbck_layout {
	u32 offset;
	u32 usbs_mask;
	u32 usbdiv_mask;
};

/**
 * Clock setup description
 * @cid:	clock id corresponding to clock subsystem
 * @pid:	parent clock id corresponding to clock subsystem
 * @rate:	clock rate
 * @prate:	parent rate
 */
struct pmc_clk_setup {
	unsigned int cid;
	unsigned int pid;
	unsigned long rate;
	unsigned long prate;
};

extern const struct clk_programmable_layout at91rm9200_programmable_layout;
extern const struct clk_programmable_layout at91sam9g45_programmable_layout;
extern const struct clk_programmable_layout at91sam9x5_programmable_layout;

extern const struct clk_ops at91_clk_ops;

struct clk *at91_clk_main_rc(void __iomem *reg, const char *name,
			const char *parent_name);
struct clk *at91_clk_main_osc(void __iomem *reg, const char *name,
			const char *parent_name, bool bypass);
struct clk *at91_clk_rm9200_main(void __iomem *reg, const char *name,
			const char *parent_name);
struct clk *at91_clk_sam9x5_main(void __iomem *reg, const char *name,
			const char * const *parent_names, int num_parents,
			const u32 *mux_table, int type);
struct clk *
sam9x60_clk_register_usb(void __iomem *base,  const char *name,
			 const char * const *parent_names, u8 num_parents,
			 const struct clk_usbck_layout *usbck_layout,
			 const u32 *clk_mux_table, const u32 *mux_table, u8 id);
struct clk *
sam9x60_clk_register_div_pll(void __iomem *base, const char *name,
			const char *parent_name, u8 id,
			const struct clk_pll_characteristics *characteristics,
			const struct clk_pll_layout *layout, bool critical);
struct clk *
sam9x60_clk_register_frac_pll(void __iomem *base, const char *name,
			const char *parent_name, u8 id,
			const struct clk_pll_characteristics *characteristics,
			const struct clk_pll_layout *layout, bool critical);
struct clk *
at91_clk_register_master_pres(void __iomem *base, const char *name,
			const char * const *parent_names, int num_parents,
			const struct clk_master_layout *layout,
			const struct clk_master_characteristics *characteristics,
			const u32 *mux_table);
struct clk *
at91_clk_register_master_div(void __iomem *base,
			const char *name, const char *parent_name,
			const struct clk_master_layout *layout,
			const struct clk_master_characteristics *characteristics);
struct clk *
at91_clk_sama7g5_register_master(void __iomem *base, const char *name,
			const char * const *parent_names, int num_parents,
			const u32 *mux_table, const u32 *clk_mux_table,
			bool critical, u8 id);
struct clk *
at91_clk_register_utmi(void __iomem *base, struct udevice *dev,
			const char *name, const char *parent_name);
struct clk *
at91_clk_sama7g5_register_utmi(void __iomem *base, const char *name,
			const char *parent_name);
struct clk *
at91_clk_register_programmable(void __iomem *base, const char *name,
			const char * const *parent_names, u8 num_parents, u8 id,
			const struct clk_programmable_layout *layout,
			const u32 *clk_mux_table, const u32 *mux_table);
struct clk *
at91_clk_register_system(void __iomem *base, const char *name,
			const char *parent_name, u8 id);
struct clk *
at91_clk_register_peripheral(void __iomem *base, const char *name,
			const char *parent_name, u32 id);
struct clk *
at91_clk_register_sam9x5_peripheral(void __iomem *base,
			const struct clk_pcr_layout *layout,
			const char *name, const char *parent_name,
			u32 id, const struct clk_range *range);
struct clk *
at91_clk_register_generic(void __iomem *base,
			const struct clk_pcr_layout *layout, const char *name,
			const char * const *parent_names,
			const u32 *clk_mux_table, const u32 *mux_table,
			u8 num_parents, u8 id, const struct clk_range *range);

int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val);
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index);

void pmc_read(void __iomem *base, unsigned int off, unsigned int *val);
void pmc_write(void __iomem *base, unsigned int off, unsigned int val);
void pmc_update_bits(void __iomem *base, unsigned int off, unsigned int mask,
			unsigned int bits);

int at91_clk_setup(const struct pmc_clk_setup *setup, int size);

#endif
