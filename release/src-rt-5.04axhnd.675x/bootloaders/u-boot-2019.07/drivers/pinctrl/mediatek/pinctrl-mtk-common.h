/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#ifndef __PINCTRL_MEDIATEK_H__
#define __PINCTRL_MEDIATEK_H__

#define MTK_RANGE(_a)		{ .range = (_a), .nranges = ARRAY_SIZE(_a), }
#define MTK_PIN(_number, _name, _drv_n) {				\
		.number = _number,					\
		.name = _name,						\
		.drv_n = _drv_n,					\
	}

#define PINCTRL_PIN_GROUP(name, id)					\
	{								\
		name,							\
		id##_pins,						\
		ARRAY_SIZE(id##_pins),					\
		id##_funcs,						\
	}

#define PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
			_x_bits, _sz_reg, _fixed) {			\
		.s_pin = _s_pin,					\
		.e_pin = _e_pin,					\
		.s_addr = _s_addr,					\
		.x_addrs = _x_addrs,					\
		.s_bit = _s_bit,					\
		.x_bits = _x_bits,					\
		.sz_reg = _sz_reg,					\
		.fixed = _fixed,					\
	}

/* List these attributes which could be modified for the pin */
enum {
	PINCTRL_PIN_REG_MODE,
	PINCTRL_PIN_REG_DIR,
	PINCTRL_PIN_REG_DI,
	PINCTRL_PIN_REG_DO,
	PINCTRL_PIN_REG_IES,
	PINCTRL_PIN_REG_SMT,
	PINCTRL_PIN_REG_PULLEN,
	PINCTRL_PIN_REG_PULLSEL,
	PINCTRL_PIN_REG_DRV,
	PINCTRL_PIN_REG_MAX,
};

/* Group the pins by the driving current */
enum {
	DRV_FIXED,
	DRV_GRP0,
	DRV_GRP1,
	DRV_GRP2,
	DRV_GRP3,
	DRV_GRP4,
};

/**
 * struct mtk_pin_field - the structure that holds the information of the field
 *			  used to describe the attribute for the pin
 * @offset:		the register offset relative to the base address
 * @mask:		the mask used to filter out the field from the register
 * @bitpos:		the start bit relative to the register
 * @next:		the indication that the field would be extended to the
			next register
 */
struct mtk_pin_field {
	u32 offset;
	u32 mask;
	u8 bitpos;
	u8 next;
};

/**
 * struct mtk_pin_field_calc - the structure that holds the range providing
 *			       the guide used to look up the relevant field
 * @s_pin:		the start pin within the range
 * @e_pin:		the end pin within the range
 * @s_addr:		the start address for the range
 * @x_addrs:		the address distance between two consecutive registers
 *			within the range
 * @s_bit:		the start bit for the first register within the range
 * @x_bits:		the bit distance between two consecutive pins within
 *			the range
 * @sz_reg:		the size of bits in a register
 * @fixed:		the consecutive pins share the same bits with the 1st
 *			pin
 */
struct mtk_pin_field_calc {
	u16 s_pin;
	u16 e_pin;
	u32 s_addr;
	u8 x_addrs;
	u8 s_bit;
	u8 x_bits;
	u8 sz_reg;
	u8 fixed;
};

/**
 * struct mtk_pin_reg_calc - the structure that holds all ranges used to
 *			     determine which register the pin would make use of
 *			     for certain pin attribute.
 * @range:		     the start address for the range
 * @nranges:		     the number of items in the range
 */
struct mtk_pin_reg_calc {
	const struct mtk_pin_field_calc *range;
	unsigned int nranges;
};

/**
 * struct mtk_pin_desc - the structure that providing information
 *			 for each pin of chips
 * @number:		unique pin number from the global pin number space
 * @name:		name for this pin
 * @drv_n:		the index with the driving group
 */
struct mtk_pin_desc {
	unsigned int number;
	const char *name;
	u8 drv_n;
};

/**
 * struct mtk_group_desc - generic pin group descriptor
 * @name: name of the pin group
 * @pins: array of pins that belong to the group
 * @num_pins: number of pins in the group
 * @data: pin controller driver specific data
 */
struct mtk_group_desc {
	const char *name;
	int *pins;
	int num_pins;
	void *data;
};

/**
 * struct mtk_function_desc - generic function descriptor
 * @name: name of the function
 * @group_names: array of pin group names
 * @num_group_names: number of pin group names
 */
struct mtk_function_desc {
	const char *name;
	const char * const *group_names;
	int num_group_names;
};

/* struct mtk_pin_soc - the structure that holds SoC-specific data */
struct mtk_pinctrl_soc {
	const char *name;
	const struct mtk_pin_reg_calc *reg_cal;
	const struct mtk_pin_desc *pins;
	int npins;
	const struct mtk_group_desc *grps;
	int ngrps;
	const struct mtk_function_desc *funcs;
	int nfuncs;
};

/**
 * struct mtk_pinctrl_priv - private data for MTK pinctrl driver
 *
 * @base: base address of the pinctrl device
 * @soc: SoC specific data
 */
struct mtk_pinctrl_priv {
	void __iomem *base;
	struct mtk_pinctrl_soc *soc;
};

extern const struct pinctrl_ops mtk_pinctrl_ops;

/* A common read-modify-write helper for MediaTek chips */
void mtk_rmw(struct udevice *dev, u32 reg, u32 mask, u32 set);
int mtk_pinctrl_common_probe(struct udevice *dev,
			     struct mtk_pinctrl_soc *soc);

#endif /* __PINCTRL_MEDIATEK_H__ */
