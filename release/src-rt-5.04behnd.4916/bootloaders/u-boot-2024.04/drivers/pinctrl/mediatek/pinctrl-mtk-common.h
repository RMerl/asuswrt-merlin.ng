/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */
#ifndef __PINCTRL_MEDIATEK_H__
#define __PINCTRL_MEDIATEK_H__

#define MTK_PINCTRL_V0 0x0
#define MTK_PINCTRL_V1 0x1
#define BASE_CALC_NONE 0
#define MAX_BASE_CALC 10

#define MTK_RANGE(_a)		{ .range = (_a), .nranges = ARRAY_SIZE(_a), }

#define MTK_PIN(_number, _name, _drv_n)					\
	MTK_TYPED_PIN(_number, _name, _drv_n, IO_TYPE_DEFAULT)

#define MTK_TYPED_PIN(_number, _name, _drv_n, _io_n) {			\
		.number = _number,					\
		.name = _name,						\
		.drv_n = _drv_n,					\
		.io_n = _io_n,						\
	}

#define PINCTRL_PIN_GROUP(name, id)					\
	{								\
		name,							\
		id##_pins,						\
		ARRAY_SIZE(id##_pins),					\
		id##_funcs,						\
	}

#define PIN_FIELD_BASE_CALC(_s_pin, _e_pin, _i_base, _s_addr, _x_addrs,	\
			    _s_bit, _x_bits, _sz_reg, _fixed) {		\
		.s_pin = _s_pin,					\
		.e_pin = _e_pin,					\
		.i_base = _i_base,					\
		.s_addr = _s_addr,					\
		.x_addrs = _x_addrs,					\
		.s_bit = _s_bit,					\
		.x_bits = _x_bits,					\
		.sz_reg = _sz_reg,					\
		.fixed = _fixed,					\
	}

#define PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
			_x_bits, _sz_reg, _fixed)			\
	 PIN_FIELD_BASE_CALC(_s_pin, _e_pin, BASE_CALC_NONE, _s_addr,	\
			    _x_addrs, _s_bit, _x_bits, _sz_reg, _fixed)

/* List these attributes which could be modified for the pin */
enum {
	PINCTRL_PIN_REG_MODE,
	PINCTRL_PIN_REG_DIR,
	PINCTRL_PIN_REG_DI,
	PINCTRL_PIN_REG_DO,
	PINCTRL_PIN_REG_SMT,
	PINCTRL_PIN_REG_PD,
	PINCTRL_PIN_REG_PU,
	PINCTRL_PIN_REG_E4,
	PINCTRL_PIN_REG_E8,
	PINCTRL_PIN_REG_IES,
	PINCTRL_PIN_REG_PULLEN,
	PINCTRL_PIN_REG_PULLSEL,
	PINCTRL_PIN_REG_DRV,
	PINCTRL_PIN_REG_PUPD,
	PINCTRL_PIN_REG_R0,
	PINCTRL_PIN_REG_R1,
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

/* Group the pins by the io type */
enum {
	IO_TYPE_DEFAULT,
	IO_TYPE_GRP0,
	IO_TYPE_GRP1,
	IO_TYPE_GRP2,
	IO_TYPE_GRP3,
	IO_TYPE_GRP4,
	IO_TYPE_GRP5,
	IO_TYPE_GRP6,
};

/**
 * struct mtk_pin_field - the structure that holds the information of the field
 *			  used to describe the attribute for the pin
 * @index:		the index pointing to the entry in base address list
 * @offset:		the register offset relative to the base address
 * @mask:		the mask used to filter out the field from the register
 * @bitpos:		the start bit relative to the register
 * @next:		the indication that the field would be extended to the
			next register
 */
struct mtk_pin_field {
	u8  index;
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
 * @i_base:		the index pointing to the entry in base address list
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
	u8  i_base;
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
 * @io_n:		the index with the io type
 */
struct mtk_pin_desc {
	unsigned int number;
	const char *name;
	u8 drv_n;
	u8 io_n;
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
	const int *pins;
	int num_pins;
	const void *data;
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

/**
 * struct mtk_io_type_desc - io class descriptor for specific pins
 * @name: name of the io class
 */
struct mtk_io_type_desc {
	const char *name;
#if CONFIG_IS_ENABLED(PINCONF)
	/* Specific pinconfig operations */
	int (*bias_set)(struct udevice *dev, u32 pin, bool disable,
			bool pullup, u32 val);
	int (*drive_set)(struct udevice *dev, u32 pin, u32 arg);
	int (*input_enable)(struct udevice *dev, u32 pin, u32 arg);
#endif
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
	const struct mtk_io_type_desc *io_type;
	u8 ntype;
	int gpio_mode;
	const char * const *base_names;
	unsigned int nbase_names;
	int rev;
	int base_calc;
};

/**
 * struct mtk_pinctrl_priv - private data for MTK pinctrl driver
 *
 * @base: base address of the pinctrl device
 * @soc: SoC specific data
 */
struct mtk_pinctrl_priv {
	void __iomem *base[MAX_BASE_CALC];
	const struct mtk_pinctrl_soc *soc;
};

extern const struct pinctrl_ops mtk_pinctrl_ops;

/* A common read-modify-write helper for MediaTek chips */
void mtk_rmw(struct udevice *dev, u32 reg, u32 mask, u32 set);
void mtk_i_rmw(struct udevice *dev, u8 i, u32 reg, u32 mask, u32 set);
int mtk_pinctrl_common_probe(struct udevice *dev,
			     const struct mtk_pinctrl_soc *soc);

#if CONFIG_IS_ENABLED(PINCONF)

int mtk_pinconf_bias_set_pu_pd(struct udevice *dev, u32 pin, bool disable,
			       bool pullup, u32 val);
int mtk_pinconf_bias_set_pullen_pullsel(struct udevice *dev, u32 pin,
					bool disable, bool pullup, u32 val);
int mtk_pinconf_bias_set_pupd_r1_r0(struct udevice *dev, u32 pin, bool disable,
				    bool pullup, u32 val);
int mtk_pinconf_bias_set_v0(struct udevice *dev, u32 pin, bool disable,
			    bool pullup, u32 val);
int mtk_pinconf_bias_set_v1(struct udevice *dev, u32 pin, bool disable,
			    bool pullup, u32 val);
int mtk_pinconf_input_enable_v1(struct udevice *dev, u32 pin, u32 arg);
int mtk_pinconf_drive_set_v0(struct udevice *dev, u32 pin, u32 arg);
int mtk_pinconf_drive_set_v1(struct udevice *dev, u32 pin, u32 arg);

#endif

#endif /* __PINCTRL_MEDIATEK_H__ */
