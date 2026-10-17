/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Common Clock Framework support for all Samsung platforms.
 */

#ifndef __EXYNOS_CLK_H
#define __EXYNOS_CLK_H

#include <errno.h>
#include <linux/clk-provider.h>
#include "clk-pll.h"

/**
 * struct samsung_mux_clock - information about mux clock
 * @id: platform specific id of the clock
 * @name: name of this mux clock
 * @parent_names: array of pointer to parent clock names
 * @num_parents: number of parents listed in @parent_names
 * @flags: optional flags for basic clock
 * @offset: offset of the register for configuring the mux
 * @shift: starting bit location of the mux control bit-field in @reg
 * @width: width of the mux control bit-field in @reg
 * @mux_flags: flags for mux-type clock
 */
struct samsung_mux_clock {
	unsigned int		id;
	const char		*name;
	const char		* const *parent_names;
	u8			num_parents;
	unsigned long		flags;
	unsigned long		offset;
	u8			shift;
	u8			width;
	u8			mux_flags;
};

#define PNAME(x) static const char * const x[]

#define __MUX(_id, cname, pnames, o, s, w, f, mf)			\
	{								\
		.id		= _id,					\
		.name		= cname,				\
		.parent_names	= pnames,				\
		.num_parents	= ARRAY_SIZE(pnames),			\
		.flags		= (f) | CLK_SET_RATE_NO_REPARENT,	\
		.offset		= o,					\
		.shift		= s,					\
		.width		= w,					\
		.mux_flags	= mf,					\
	}

#define MUX(_id, cname, pnames, o, s, w)				\
	__MUX(_id, cname, pnames, o, s, w, 0, 0)

#define MUX_F(_id, cname, pnames, o, s, w, f, mf)			\
	__MUX(_id, cname, pnames, o, s, w, f, mf)

/**
 * struct samsung_div_clock - information about div clock
 * @id: platform specific id of the clock
 * @name: name of this div clock
 * @parent_name: name of the parent clock
 * @flags: optional flags for basic clock
 * @offset: offset of the register for configuring the div
 * @shift: starting bit location of the div control bit-field in @reg
 * @width: width of the bitfield
 * @div_flags: flags for div-type clock
 */
struct samsung_div_clock {
	unsigned int		id;
	const char		*name;
	const char		*parent_name;
	unsigned long		flags;
	unsigned long		offset;
	u8			shift;
	u8			width;
	u8			div_flags;
};

#define __DIV(_id, cname, pname, o, s, w, f, df)	\
	{						\
		.id		= _id,			\
		.name		= cname,		\
		.parent_name	= pname,		\
		.flags		= f,			\
		.offset		= o,			\
		.shift		= s,			\
		.width		= w,			\
		.div_flags	= df,			\
	}

#define DIV(_id, cname, pname, o, s, w)			\
	__DIV(_id, cname, pname, o, s, w, 0, 0)

#define DIV_F(_id, cname, pname, o, s, w, f, df)	\
	__DIV(_id, cname, pname, o, s, w, f, df)

/**
 * struct samsung_gate_clock - information about gate clock
 * @id: platform specific id of the clock
 * @name: name of this gate clock
 * @parent_name: name of the parent clock
 * @flags: optional flags for basic clock
 * @offset: offset of the register for configuring the gate
 * @bit_idx: bit index of the gate control bit-field in @reg
 * @gate_flags: flags for gate-type clock
 */
struct samsung_gate_clock {
	unsigned int		id;
	const char		*name;
	const char		*parent_name;
	unsigned long		flags;
	unsigned long		offset;
	u8			bit_idx;
	u8			gate_flags;
};

#define __GATE(_id, cname, pname, o, b, f, gf)			\
	{							\
		.id		= _id,				\
		.name		= cname,			\
		.parent_name	= pname,			\
		.flags		= f,				\
		.offset		= o,				\
		.bit_idx	= b,				\
		.gate_flags	= gf,				\
	}

#define GATE(_id, cname, pname, o, b, f, gf)			\
	__GATE(_id, cname, pname, o, b, f, gf)

/**
 * struct samsung_pll_clock - information about pll clock
 * @id: platform specific id of the clock
 * @name: name of this pll clock
 * @parent_name: name of the parent clock
 * @flags: optional flags for basic clock
 * @con_offset: offset of the register for configuring the PLL
 * @type: type of PLL to be registered
 */
struct samsung_pll_clock {
	unsigned int		id;
	const char		*name;
	const char		*parent_name;
	unsigned long		flags;
	int			con_offset;
	enum samsung_pll_type	type;
};

#define PLL(_typ, _id, _name, _pname, _con)		\
	{						\
		.id		= _id,			\
		.name		= _name,		\
		.parent_name	= _pname,		\
		.flags		= CLK_GET_RATE_NOCACHE,	\
		.con_offset	= _con,			\
		.type		= _typ,			\
	}

enum samsung_clock_type {
	S_CLK_MUX,
	S_CLK_DIV,
	S_CLK_GATE,
	S_CLK_PLL,
};

/**
 * struct samsung_clock_group - contains a list of clocks of one type
 * @type: type of clocks this structure contains
 * @clk_list: list of clocks
 * @nr_clk: count of clocks in @clk_list
 */
struct samsung_clk_group {
	enum samsung_clock_type type;
	const void *clk_list;
	unsigned int nr_clk;
};

void samsung_clk_register_mux(void __iomem *base,
			      const struct samsung_mux_clock *clk_list,
			      unsigned int nr_clk);
void samsung_clk_register_div(void __iomem *base,
			      const struct samsung_div_clock *clk_list,
			      unsigned int nr_clk);
void samsung_clk_register_gate(void __iomem *base,
			       const struct samsung_gate_clock *clk_list,
			       unsigned int nr_clk);
void samsung_clk_register_pll(void __iomem *base,
			      const struct samsung_pll_clock *clk_list,
			      unsigned int nr_clk);

void samsung_cmu_register_clocks(void __iomem *base,
				 const struct samsung_clk_group *clk_groups,
				 unsigned int nr_groups);
int samsung_cmu_register_one(struct udevice *dev,
			     const struct samsung_clk_group *clk_groups,
			     unsigned int nr_groups);

/**
 * samsung_register_cmu - Register CMU clocks ensuring parent CMU is present
 * @dev: CMU device
 * @clk_groups: list of CMU clock groups
 * @parent_drv: name of parent CMU driver
 *
 * Register provided CMU clocks, but make sure CMU_TOP driver is instantiated
 * first.
 *
 * Return: 0 on success or negative value on error.
 */
#define samsung_register_cmu(dev, clk_groups, parent_drv)		\
({									\
	struct udevice *__parent;					\
	int __ret;							\
									\
	__ret = uclass_get_device_by_driver(UCLASS_CLK,			\
		DM_DRIVER_GET(parent_drv), &__parent);			\
	if (__ret || !__parent)						\
		__ret = -ENOENT;					\
	else								\
		__ret = samsung_cmu_register_one(dev, clk_groups,	\
			ARRAY_SIZE(clk_groups));			\
	__ret;								\
})

#endif /* __EXYNOS_CLK_H */
