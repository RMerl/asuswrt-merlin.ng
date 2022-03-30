/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on Linux i.MX iomux-v3.h file:
 * Copyright (C) 2009 by Jan Weitzel Phytec Messtechnik GmbH,
 *			<armlinux@phytec.de>
 *
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 */

#ifndef __MACH_IOMUX_V3_H__
#define __MACH_IOMUX_V3_H__

#include <common.h>

/*
 *	build IOMUX_PAD structure
 *
 * This iomux scheme is based around pads, which are the physical balls
 * on the processor.
 *
 * - Each pad has a pad control register (IOMUXC_SW_PAD_CTRL_x) which controls
 *   things like driving strength and pullup/pulldown.
 * - Each pad can have but not necessarily does have an output routing register
 *   (IOMUXC_SW_MUX_CTL_PAD_x).
 * - Each pad can have but not necessarily does have an input routing register
 *   (IOMUXC_x_SELECT_INPUT)
 *
 * The three register sets do not have a fixed offset to each other,
 * hence we order this table by pad control registers (which all pads
 * have) and put the optional i/o routing registers into additional
 * fields.
 *
 * The naming convention for the pad modes is SOC_PAD_<padname>__<padmode>
 * If <padname> or <padmode> refers to a GPIO, it is named GPIO_<unit>_<num>
 *
 * IOMUX/PAD Bit field definitions
 *
 * MUX_CTRL_OFS:	    0..11 (12)
 * PAD_CTRL_OFS:	   12..23 (12)
 * SEL_INPUT_OFS:	   24..35 (12)
 * MUX_MODE + SION + LPSR: 36..41  (6)
 * PAD_CTRL + NO_PAD_CTRL: 42..59 (18)
 * SEL_INP:		   60..63  (4)
*/

typedef u64 iomux_v3_cfg_t;

#define MUX_CTRL_OFS_SHIFT	0
#define MUX_CTRL_OFS_MASK	((iomux_v3_cfg_t)0xfff << MUX_CTRL_OFS_SHIFT)
#define MUX_PAD_CTRL_OFS_SHIFT	12
#define MUX_PAD_CTRL_OFS_MASK	((iomux_v3_cfg_t)0xfff << \
	MUX_PAD_CTRL_OFS_SHIFT)
#define MUX_SEL_INPUT_OFS_SHIFT	24
#define MUX_SEL_INPUT_OFS_MASK	((iomux_v3_cfg_t)0xfff << \
	MUX_SEL_INPUT_OFS_SHIFT)

#define MUX_MODE_SHIFT		36
#define MUX_MODE_MASK		((iomux_v3_cfg_t)0x3f << MUX_MODE_SHIFT)
#define MUX_PAD_CTRL_SHIFT	42
#define MUX_PAD_CTRL_MASK	((iomux_v3_cfg_t)0x3ffff << MUX_PAD_CTRL_SHIFT)
#define MUX_SEL_INPUT_SHIFT	60
#define MUX_SEL_INPUT_MASK	((iomux_v3_cfg_t)0xf << MUX_SEL_INPUT_SHIFT)

#define MUX_MODE_SION		((iomux_v3_cfg_t)IOMUX_CONFIG_SION << \
	MUX_MODE_SHIFT)
#define MUX_PAD_CTRL(x)		((iomux_v3_cfg_t)(x) << MUX_PAD_CTRL_SHIFT)

#define IOMUX_PAD(pad_ctrl_ofs, mux_ctrl_ofs, mux_mode, sel_input_ofs,	\
		sel_input, pad_ctrl)					\
	(((iomux_v3_cfg_t)(mux_ctrl_ofs) << MUX_CTRL_OFS_SHIFT)     |	\
	((iomux_v3_cfg_t)(mux_mode)      << MUX_MODE_SHIFT)         |	\
	((iomux_v3_cfg_t)(pad_ctrl_ofs)  << MUX_PAD_CTRL_OFS_SHIFT) |	\
	((iomux_v3_cfg_t)(pad_ctrl)      << MUX_PAD_CTRL_SHIFT)     |	\
	((iomux_v3_cfg_t)(sel_input_ofs) << MUX_SEL_INPUT_OFS_SHIFT)|	\
	((iomux_v3_cfg_t)(sel_input)     << MUX_SEL_INPUT_SHIFT))

#define NEW_PAD_CTRL(cfg, pad)	(((cfg) & ~MUX_PAD_CTRL_MASK) | \
					MUX_PAD_CTRL(pad))

#define __NA_			0x000
#define NO_MUX_I		0
#define NO_PAD_I		0

#define NO_PAD_CTRL		(1 << 17)

#define IOMUX_CONFIG_LPSR       0x20
#define MUX_MODE_LPSR           ((iomux_v3_cfg_t)IOMUX_CONFIG_LPSR << \
				MUX_MODE_SHIFT)
#ifdef CONFIG_IMX8M
#define PAD_CTL_DSE0		(0x0 << 0)
#define PAD_CTL_DSE1		(0x1 << 0)
#define PAD_CTL_DSE2		(0x2 << 0)
#define PAD_CTL_DSE3		(0x3 << 0)
#define PAD_CTL_DSE4		(0x4 << 0)
#define PAD_CTL_DSE5		(0x5 << 0)
#define PAD_CTL_DSE6		(0x6 << 0)
#define PAD_CTL_DSE7		(0x7 << 0)

#define PAD_CTL_FSEL0		(0x0 << 3)
#define PAD_CTL_FSEL1		(0x1 << 3)
#define PAD_CTL_FSEL2		(0x2 << 3)
#define PAD_CTL_FSEL3		(0x3 << 3)

#define PAD_CTL_ODE		(0x1 << 5)
#define PAD_CTL_PUE		(0x1 << 6)
#define PAD_CTL_HYS		(0x1 << 7)
#define PAD_CTL_LVTTL		(0x1 << 8)

#elif defined CONFIG_MX7

#define IOMUX_LPSR_SEL_INPUT_OFS 0x70000

#define PAD_CTL_DSE_1P8V_140OHM   (0x0<<0)
#define PAD_CTL_DSE_1P8V_35OHM    (0x1<<0)
#define PAD_CTL_DSE_1P8V_70OHM    (0x2<<0)
#define PAD_CTL_DSE_1P8V_23OHM    (0x3<<0)

#define PAD_CTL_DSE_3P3V_196OHM   (0x0<<0)
#define PAD_CTL_DSE_3P3V_49OHM    (0x1<<0)
#define PAD_CTL_DSE_3P3V_98OHM    (0x2<<0)
#define PAD_CTL_DSE_3P3V_32OHM    (0x3<<0)

#define PAD_CTL_SRE_FAST     (0 << 2)
#define PAD_CTL_SRE_SLOW     (0x1 << 2)

#define PAD_CTL_HYS       (0x1 << 3)
#define PAD_CTL_PUE       (0x1 << 4)

#define PAD_CTL_PUS_PD100KOHM  ((0x0 << 5) | PAD_CTL_PUE)
#define PAD_CTL_PUS_PU5KOHM    ((0x1 << 5) | PAD_CTL_PUE)
#define PAD_CTL_PUS_PU47KOHM   ((0x2 << 5) | PAD_CTL_PUE)
#define PAD_CTL_PUS_PU100KOHM  ((0x3 << 5) | PAD_CTL_PUE)

#else

#ifdef CONFIG_MX6

#define PAD_CTL_HYS		(1 << 16)

#define PAD_CTL_PUS_100K_DOWN	(0 << 14 | PAD_CTL_PUE)
#define PAD_CTL_PUS_47K_UP	(1 << 14 | PAD_CTL_PUE)
#define PAD_CTL_PUS_100K_UP	(2 << 14 | PAD_CTL_PUE)
#define PAD_CTL_PUS_22K_UP	(3 << 14 | PAD_CTL_PUE)
#define PAD_CTL_PUE		(1 << 13 | PAD_CTL_PKE)
#define PAD_CTL_PKE		(1 << 12)

#define PAD_CTL_ODE		(1 << 11)

#if defined(CONFIG_MX6SL)
#define PAD_CTL_SPEED_LOW	(1 << 6)
#else
#define PAD_CTL_SPEED_LOW	(0 << 6)
#endif
#define PAD_CTL_SPEED_MED	(2 << 6)
#define PAD_CTL_SPEED_HIGH	(3 << 6)

#define PAD_CTL_DSE_DISABLE	(0 << 3)
#define PAD_CTL_DSE_240ohm	(1 << 3)
#define PAD_CTL_DSE_120ohm	(2 << 3)
#define PAD_CTL_DSE_80ohm	(3 << 3)
#define PAD_CTL_DSE_60ohm	(4 << 3)
#define PAD_CTL_DSE_48ohm	(5 << 3)
#define PAD_CTL_DSE_40ohm	(6 << 3)
#define PAD_CTL_DSE_34ohm	(7 << 3)

#define PAD_CTL_DSE_260ohm	(1 << 3)
#define PAD_CTL_DSE_130ohm	(2 << 3)
#define PAD_CTL_DSE_88ohm	(3 << 3)
#define PAD_CTL_DSE_65ohm	(4 << 3)
#define PAD_CTL_DSE_52ohm	(5 << 3)
#define PAD_CTL_DSE_43ohm	(6 << 3)
#define PAD_CTL_DSE_37ohm	(7 << 3)

/* i.MX6SL/SLL */
#define PAD_CTL_LVE		(1 << 1)
#define PAD_CTL_LVE_BIT		(1 << 22)

/* i.MX6SLL */
#define PAD_CTL_IPD_BIT		(1 << 27)

#elif defined(CONFIG_VF610)

#define PAD_MUX_MODE_SHIFT	20

#define PAD_CTL_INPUT_DIFFERENTIAL (1 << 16)

#define PAD_CTL_SPEED_MED	(1 << 12)
#define PAD_CTL_SPEED_HIGH	(3 << 12)

#define PAD_CTL_SRE		(1 << 11)

#define PAD_CTL_ODE		(1 << 10)

#define PAD_CTL_DSE_150ohm	(1 << 6)
#define PAD_CTL_DSE_75ohm	(2 << 6)
#define PAD_CTL_DSE_50ohm	(3 << 6)
#define PAD_CTL_DSE_37ohm	(4 << 6)
#define PAD_CTL_DSE_30ohm	(5 << 6)
#define PAD_CTL_DSE_25ohm	(6 << 6)
#define PAD_CTL_DSE_20ohm	(7 << 6)

#define PAD_CTL_PUS_47K_UP	(1 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PUS_100K_UP	(2 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PUS_22K_UP	(3 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PKE		(1 << 3)
#define PAD_CTL_PUE		(1 << 2 | PAD_CTL_PKE)

#define PAD_CTL_OBE_IBE_ENABLE	(3 << 0)
#define PAD_CTL_OBE_ENABLE	(1 << 1)
#define PAD_CTL_IBE_ENABLE	(1 << 0)

#else

#define PAD_CTL_DVS		(1 << 13)
#define PAD_CTL_INPUT_DDR	(1 << 9)
#define PAD_CTL_HYS		(1 << 8)

#define PAD_CTL_PKE		(1 << 7)
#define PAD_CTL_PUE		(1 << 6 | PAD_CTL_PKE)
#define PAD_CTL_PUS_100K_DOWN	(0 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PUS_47K_UP	(1 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PUS_100K_UP	(2 << 4 | PAD_CTL_PUE)
#define PAD_CTL_PUS_22K_UP	(3 << 4 | PAD_CTL_PUE)

#define PAD_CTL_ODE		(1 << 3)

#define PAD_CTL_DSE_LOW		(0 << 1)
#define PAD_CTL_DSE_MED		(1 << 1)
#define PAD_CTL_DSE_HIGH	(2 << 1)
#define PAD_CTL_DSE_MAX		(3 << 1)

#endif

#define PAD_CTL_SRE_SLOW	(0 << 0)
#define PAD_CTL_SRE_FAST	(1 << 0)

#endif

#define IOMUX_CONFIG_SION	0x10

#define GPIO_PIN_MASK		0x1f
#define GPIO_PORT_SHIFT		5
#define GPIO_PORT_MASK		(0x7 << GPIO_PORT_SHIFT)
#define GPIO_PORTA		(0 << GPIO_PORT_SHIFT)
#define GPIO_PORTB		(1 << GPIO_PORT_SHIFT)
#define GPIO_PORTC		(2 << GPIO_PORT_SHIFT)
#define GPIO_PORTD		(3 << GPIO_PORT_SHIFT)
#define GPIO_PORTE		(4 << GPIO_PORT_SHIFT)
#define GPIO_PORTF		(5 << GPIO_PORT_SHIFT)

void imx_iomux_v3_setup_pad(iomux_v3_cfg_t pad);
void imx_iomux_v3_setup_multiple_pads(iomux_v3_cfg_t const *pad_list,
				     unsigned count);
/*
* Set bits for general purpose registers
*/
void imx_iomux_set_gpr_register(int group, int start_bit,
					 int num_bits, int value);
#ifdef CONFIG_IOMUX_SHARE_CONF_REG
void imx_iomux_gpio_set_direction(unsigned int gpio,
				unsigned int direction);
void imx_iomux_gpio_get_function(unsigned int gpio,
				u32 *gpio_state);
#endif

/* macros for declaring and using pinmux array */
#if defined(CONFIG_MX6QDL)
#define IOMUX_PADS(x) (MX6Q_##x), (MX6DL_##x)
#define SETUP_IOMUX_PAD(def)					\
if (is_mx6dq() || is_mx6dqp()) {				\
	imx_iomux_v3_setup_pad(MX6Q_##def);			\
} else {							\
	imx_iomux_v3_setup_pad(MX6DL_##def);			\
}
#define SETUP_IOMUX_PADS(x)					\
	imx_iomux_v3_setup_multiple_pads(x, ARRAY_SIZE(x)/2)
#elif defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
#define IOMUX_PADS(x) MX6Q_##x
#define SETUP_IOMUX_PAD(def)					\
	imx_iomux_v3_setup_pad(MX6Q_##def);
#define SETUP_IOMUX_PADS(x)					\
	imx_iomux_v3_setup_multiple_pads(x, ARRAY_SIZE(x))
#elif defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
#define IOMUX_PADS(x) MX6_##x
#define SETUP_IOMUX_PAD(def)					\
	imx_iomux_v3_setup_pad(MX6_##def);
#define SETUP_IOMUX_PADS(x)					\
	imx_iomux_v3_setup_multiple_pads(x, ARRAY_SIZE(x))
#else
#define IOMUX_PADS(x) MX6DL_##x
#define SETUP_IOMUX_PAD(def)					\
	imx_iomux_v3_setup_pad(MX6DL_##def);
#define SETUP_IOMUX_PADS(x)					\
	imx_iomux_v3_setup_multiple_pads(x, ARRAY_SIZE(x))
#endif

#endif	/* __MACH_IOMUX_V3_H__*/
