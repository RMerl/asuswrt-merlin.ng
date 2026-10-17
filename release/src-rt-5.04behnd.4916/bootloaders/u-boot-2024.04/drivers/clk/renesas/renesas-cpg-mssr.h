/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Renesas RCar Gen3 CPG MSSR driver
 *
 * Copyright (C) 2017-2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 */

#ifndef __DRIVERS_CLK_RENESAS_CPG_MSSR__
#define __DRIVERS_CLK_RENESAS_CPG_MSSR__

#include <linux/bitops.h>

enum clk_reg_layout {
	CLK_REG_LAYOUT_RCAR_GEN2_AND_GEN3 = 0,
	CLK_REG_LAYOUT_RCAR_GEN4,
};

struct cpg_mssr_info {
	const struct cpg_core_clk	*core_clk;
	unsigned int			core_clk_size;
	enum clk_reg_layout		reg_layout;
	const struct mssr_mod_clk	*mod_clk;
	unsigned int			mod_clk_size;
	const struct mstp_stop_table	*mstp_table;
	unsigned int			mstp_table_size;
	const char			*reset_node;
	unsigned int			reset_modemr_offset;
	const char			*extalr_node;
	const char			*extal_usb_node;
	unsigned int			mod_clk_base;
	unsigned int			clk_extal_id;
	unsigned int			clk_extalr_id;
	unsigned int			clk_extal_usb_id;
	unsigned int			pll0_div;
	const void			*(*get_pll_config)(const u32 cpg_mode);
	const u16			*status_regs;
	const u16			*control_regs;
	const u16			*reset_regs;
	const u16			*reset_clear_regs;
};

/*
 * Definitions of CPG Core Clocks
 *
 * These include:
 *   - Clock outputs exported to DT
 *   - External input clocks
 *   - Internal CPG clocks
 */
struct cpg_core_clk {
	/* Common */
	const char *name;
	unsigned int id;
	unsigned int type;
	/* Depending on type */
	unsigned int parent;	/* Core Clocks only */
	unsigned int div;
	unsigned int mult;
	unsigned int offset;
};

enum clk_types {
	/* Generic */
	CLK_TYPE_IN,		/* External Clock Input */
	CLK_TYPE_FF,		/* Fixed Factor Clock */
	CLK_TYPE_DIV6P1,	/* DIV6 Clock with 1 parent clock */
	CLK_TYPE_DIV6_RO,	/* DIV6 Clock read only with extra divisor */
	CLK_TYPE_FR,		/* Fixed Rate Clock */

	/* Custom definitions start here */
	CLK_TYPE_CUSTOM,
};

#define DEF_TYPE(_name, _id, _type...)	\
	{ .name = _name, .id = _id, .type = _type }
#define DEF_BASE(_name, _id, _type, _parent...)	\
	DEF_TYPE(_name, _id, _type, .parent = _parent)

#define DEF_INPUT(_name, _id) \
	DEF_TYPE(_name, _id, CLK_TYPE_IN)
#define DEF_FIXED(_name, _id, _parent, _div, _mult)	\
	DEF_BASE(_name, _id, CLK_TYPE_FF, _parent, .div = _div, .mult = _mult)
#define DEF_DIV6P1(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_DIV6P1, _parent, .offset = _offset)
#define DEF_DIV6_RO(_name, _id, _parent, _offset, _div)	\
	DEF_BASE(_name, _id, CLK_TYPE_DIV6_RO, _parent, .offset = _offset, .div = _div, .mult = 1)
#define DEF_RATE(_name, _id, _rate)	\
	DEF_TYPE(_name, _id, CLK_TYPE_FR, .mult = _rate)

/*
 * Definitions of Module Clocks
 */
struct mssr_mod_clk {
	const char *name;
	unsigned int id;
	unsigned int parent;	/* Add MOD_CLK_BASE for Module Clocks */
};

/* Convert from sparse base-100 to packed index space */
#define MOD_CLK_PACK(x)	((x) - ((x) / 100) * (100 - 32))

#define MOD_CLK_ID(x)	(MOD_CLK_BASE + MOD_CLK_PACK(x))

#define DEF_MOD(_name, _mod, _parent...)	\
	{ .name = _name, .id = MOD_CLK_ID(_mod), .parent = _parent }

struct mstp_stop_table {
	u32	sdis;
	u32	sen;
	u32	rdis;
	u32	ren;
};

#define TSTR0		0x04
#define TSTR0_STR0	BIT(0)

bool renesas_clk_is_mod(struct clk *clk);
int renesas_clk_get_mod(struct clk *clk, struct cpg_mssr_info *info,
			const struct mssr_mod_clk **mssr);
int renesas_clk_get_core(struct clk *clk, struct cpg_mssr_info *info,
			 const struct cpg_core_clk **core);
int renesas_clk_get_parent(struct clk *clk, struct cpg_mssr_info *info,
			   struct clk *parent);
int renesas_clk_endisable(struct clk *clk, void __iomem *base,
			  struct cpg_mssr_info *info, bool enable);
int renesas_clk_remove(void __iomem *base, struct cpg_mssr_info *info);

/*
 * Module Standby and Software Reset register offets.
 *
 * If the registers exist, these are valid for SH-Mobile, R-Mobile,
 * R-Car Gen2, R-Car Gen3, R-Car Gen4 and RZ/G1.
 * These are NOT valid for R-Car Gen1 and RZ/A1!
 */

/*
 * Module Stop Status Register offsets
 */

static const u16 mstpsr[] = {
	0x030, 0x038, 0x040, 0x048, 0x04C, 0x03C, 0x1C0, 0x1C4,
	0x9A0, 0x9A4, 0x9A8, 0x9AC,
};

static const u16 mstpsr_for_gen4[] = {
	0x2E00, 0x2E04, 0x2E08, 0x2E0C, 0x2E10, 0x2E14, 0x2E18, 0x2E1C,
	0x2E20, 0x2E24, 0x2E28, 0x2E2C, 0x2E30, 0x2E34, 0x2E38, 0x2E3C,
	0x2E40, 0x2E44, 0x2E48, 0x2E4C, 0x2E50, 0x2E54, 0x2E58, 0x2E5C,
	0x2E60, 0x2E64, 0x2E68, 0x2E6C,
};

/*
 * System Module Stop Control Register offsets
 */

static const u16 smstpcr[] = {
	0x130, 0x134, 0x138, 0x13C, 0x140, 0x144, 0x148, 0x14C,
	0x990, 0x994, 0x998, 0x99C,
};

static const u16 mstpcr_for_gen4[] = {
	0x2D00, 0x2D04, 0x2D08, 0x2D0C, 0x2D10, 0x2D14, 0x2D18, 0x2D1C,
	0x2D20, 0x2D24, 0x2D28, 0x2D2C, 0x2D30, 0x2D34, 0x2D38, 0x2D3C,
	0x2D40, 0x2D44, 0x2D48, 0x2D4C, 0x2D50, 0x2D54, 0x2D58, 0x2D5C,
	0x2D60, 0x2D64, 0x2D68, 0x2D6C,
};

/*
 * Software Reset Register offsets
 */

static const u16 srcr[] = {
	0x0A0, 0x0A8, 0x0B0, 0x0B8, 0x0BC, 0x0C4, 0x1C8, 0x1CC,
	0x920, 0x924, 0x928, 0x92C,
};

static const u16 srcr_for_gen4[] = {
	0x2C00, 0x2C04, 0x2C08, 0x2C0C, 0x2C10, 0x2C14, 0x2C18, 0x2C1C,
	0x2C20, 0x2C24, 0x2C28, 0x2C2C, 0x2C30, 0x2C34, 0x2C38, 0x2C3C,
	0x2C40, 0x2C44, 0x2C48, 0x2C4C, 0x2C50, 0x2C54, 0x2C58, 0x2C5C,
	0x2C60, 0x2C64, 0x2C68, 0x2C6C,
};

/* Realtime Module Stop Control Register offsets */
#define RMSTPCR(i)	((i) < 8 ? smstpcr[i] - 0x20 : smstpcr[i] - 0x10)

/* Modem Module Stop Control Register offsets (r8a73a4) */
#define MMSTPCR(i)	(smstpcr[i] + 0x20)

/* Software Reset Clearing Register offsets */

static const u16 srstclr[] = {
	0x940, 0x944, 0x948, 0x94C, 0x950, 0x954, 0x958, 0x95C,
	0x960, 0x964, 0x968, 0x96C,
};

static const u16 srstclr_for_gen4[] = {
	0x2C80, 0x2C84, 0x2C88, 0x2C8C, 0x2C90, 0x2C94, 0x2C98, 0x2C9C,
	0x2CA0, 0x2CA4, 0x2CA8, 0x2CAC, 0x2CB0, 0x2CB4, 0x2CB8, 0x2CBC,
	0x2CC0, 0x2CC4, 0x2CC8, 0x2CCC, 0x2CD0, 0x2CD4, 0x2CD8, 0x2CDC,
	0x2CE0, 0x2CE4, 0x2CE8, 0x2CEC,
};

#endif /* __DRIVERS_CLK_RENESAS_CPG_MSSR__ */
