/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _STV0991_CREG_H
#define _STV0991_CREG_H

struct stv0991_creg {
	u32 version;		/* offset 0x0 */
	u32 hdpctl;		/* offset 0x4 */
	u32 hdpval;		/* offset 0x8 */
	u32 hdpgposet;		/* offset 0xc */
	u32 hdpgpoclr;		/* offset 0x10 */
	u32 hdpgpoval;		/* offset 0x14 */
	u32 stm_mux;		/* offset 0x18 */
	u32 sysctrl_1;		/* offset 0x1c */
	u32 sysctrl_2;		/* offset 0x20 */
	u32 sysctrl_3;		/* offset 0x24 */
	u32 sysctrl_4;		/* offset 0x28 */
	u32 reserved_1[0x35];	/* offset 0x2C-0xFC */
	u32 mux1;		/* offset 0x100 */
	u32 mux2;		/* offset 0x104 */
	u32 mux3;		/* offset 0x108 */
	u32 mux4;		/* offset 0x10c */
	u32 mux5;		/* offset 0x110 */
	u32 mux6;		/* offset 0x114 */
	u32 mux7;		/* offset 0x118 */
	u32 mux8;		/* offset 0x11c */
	u32 mux9;		/* offset 0x120 */
	u32 mux10;		/* offset 0x124 */
	u32 mux11;		/* offset 0x128 */
	u32 mux12;		/* offset 0x12c */
	u32 mux13;		/* offset 0x130 */
	u32 reserved_2[0x33];	/* offset 0x134-0x1FC */
	u32 cfg_pad1;		/* offset 0x200 */
	u32 cfg_pad2;		/* offset 0x204 */
	u32 cfg_pad3;		/* offset 0x208 */
	u32 cfg_pad4;		/* offset 0x20c */
	u32 cfg_pad5;		/* offset 0x210 */
	u32 cfg_pad6;		/* offset 0x214 */
	u32 cfg_pad7;		/* offset 0x218 */
	u32 reserved_3[0x39];	/* offset 0x21C-0x2FC */
	u32 vdd_pad1;		/* offset 0x300 */
	u32 vdd_pad2;		/* offset 0x304 */
	u32 reserved_4[0x3e];	/* offset 0x308-0x3FC */
	u32 vdd_comp1;		/* offset 0x400 */
};

/* CREG MUX 13 register */
#define FLASH_CS_NC_SHIFT	4
#define FLASH_CS_NC_MASK	~(7 << FLASH_CS_NC_SHIFT)
#define CFG_FLASH_CS_NC		(0 << FLASH_CS_NC_SHIFT)

#define FLASH_CLK_SHIFT		0
#define FLASH_CLK_MASK		~(7 << FLASH_CLK_SHIFT)
#define CFG_FLASH_CLK		(0 << FLASH_CLK_SHIFT)

/* CREG MUX 12 register */
#define GPIOC_30_MUX_SHIFT	24
#define GPIOC_30_MUX_MASK	~(1 << GPIOC_30_MUX_SHIFT)
#define CFG_GPIOC_30_UART_TX	(1 << GPIOC_30_MUX_SHIFT)

#define GPIOC_31_MUX_SHIFT	28
#define GPIOC_31_MUX_MASK	~(1 << GPIOC_31_MUX_SHIFT)
#define CFG_GPIOC_31_UART_RX	(1 << GPIOC_31_MUX_SHIFT)

/* CREG MUX 7 register */
#define GPIOB_16_MUX_SHIFT	0
#define GPIOB_16_MUX_MASK	~(1 << GPIOB_16_MUX_SHIFT)
#define CFG_GPIOB_16_UART_TX	(1 << GPIOB_16_MUX_SHIFT)

#define GPIOB_17_MUX_SHIFT	4
#define GPIOB_17_MUX_MASK	~(1 << GPIOB_17_MUX_SHIFT)
#define CFG_GPIOB_17_UART_RX	(1 << GPIOB_17_MUX_SHIFT)

/* CREG CFG_PAD6 register */

#define GPIOC_31_MODE_SHIFT	30
#define GPIOC_31_MODE_MASK	~(1 << GPIOC_31_MODE_SHIFT)
#define CFG_GPIOC_31_MODE_OD	(0 << GPIOC_31_MODE_SHIFT)
#define CFG_GPIOC_31_MODE_PP	(1 << GPIOC_31_MODE_SHIFT)

#define GPIOC_30_MODE_SHIFT	28
#define GPIOC_30_MODE_MASK	~(1 << GPIOC_30_MODE_SHIFT)
#define CFG_GPIOC_30_MODE_LOW	(0 << GPIOC_30_MODE_SHIFT)
#define CFG_GPIOC_30_MODE_HIGH	(1 << GPIOC_30_MODE_SHIFT)

/* CREG Ethernet pad config */

#define VDD_ETH_PS_1V8		0
#define VDD_ETH_PS_2V5		2
#define VDD_ETH_PS_3V3		3
#define VDD_ETH_PS_MASK		0x3

#define VDD_ETH_PS_SHIFT	12
#define ETH_VDD_CFG		(VDD_ETH_PS_1V8 << VDD_ETH_PS_SHIFT)

#define VDD_ETH_M_PS_SHIFT	28
#define ETH_M_VDD_CFG		(VDD_ETH_PS_1V8 << VDD_ETH_M_PS_SHIFT)

#endif
