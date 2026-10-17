/* SPDX-License-Identifier: GPL-2.0 */
/*
 * RZ/G2L Pin Function Controller
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 */

#ifndef RENESAS_RZG2L_PFC_H
#define RENESAS_RZG2L_PFC_H

/* PIN capabilities */
#define PIN_CFG_IOLH_A			BIT(0)
#define PIN_CFG_IOLH_B			BIT(1)
#define PIN_CFG_SR			BIT(2)
#define PIN_CFG_IEN			BIT(3)
#define PIN_CFG_PUPD			BIT(4)
#define PIN_CFG_IO_VMC_SD0		BIT(5)
#define PIN_CFG_IO_VMC_SD1		BIT(6)
#define PIN_CFG_IO_VMC_QSPI		BIT(7)
#define PIN_CFG_IO_VMC_ETH0		BIT(8)
#define PIN_CFG_IO_VMC_ETH1		BIT(9)
#define PIN_CFG_FILONOFF		BIT(10)
#define PIN_CFG_FILNUM			BIT(11)
#define PIN_CFG_FILCLKSEL		BIT(12)

#define RZG2L_MPXED_PIN_FUNCS		(PIN_CFG_IOLH_A | \
					 PIN_CFG_SR | \
					 PIN_CFG_PUPD | \
					 PIN_CFG_FILONOFF | \
					 PIN_CFG_FILNUM | \
					 PIN_CFG_FILCLKSEL)

#define RZG2L_MPXED_ETH_PIN_FUNCS(x)	((x) | \
					 PIN_CFG_FILONOFF | \
					 PIN_CFG_FILNUM | \
					 PIN_CFG_FILCLKSEL)

/* GPIO port data macros:
 * n indicates number of pins in the port, a is the register index
 * and f is pin configuration capabilities supported.
 */
#define RZG2L_GPIO_PORT_PACK(n, a, f)	(((n) << 28) | ((a) << 20) | (f))
#define RZG2L_GPIO_PORT_GET_PINCNT(x)	(((x) & GENMASK(30, 28)) >> 28)
#define RZG2L_GPIO_PORT_GET_INDEX(x)	(((x) & GENMASK(26, 20)) >> 20)
#define RZG2L_GPIO_PORT_GET_CFGS(x)	((x) & GENMASK(19, 0))

/* Dedicated pin data macros:
 * BIT(31) indicates dedicated pin, p is the register index while
 * referencing to SR/IEN/IOLH/FILxx registers, b is the register bits
 * (b * 8) and f is the pin configuration capabilities supported.
 */
#define RZG2L_SINGLE_PIN		BIT(31)
#define RZG2L_SINGLE_PIN_PACK(p, b, f)	(RZG2L_SINGLE_PIN | \
					 ((p) << 24) | ((b) << 20) | (f))
#define RZG2L_SINGLE_PIN_GET_PORT_OFFSET(x)	(((x) & GENMASK(30, 24)) >> 24)
#define RZG2L_SINGLE_PIN_GET_BIT(x)	(((x) & GENMASK(22, 20)) >> 20)
#define RZG2L_SINGLE_PIN_GET_CFGS(x)	((x) & GENMASK(19, 0))

/* Pinmux data encoded in the device tree uses:
 * 16 lower bits [15:0] for pin identifier
 * 16 higher bits [31:16] for pin mux function
 */
#define MUX_PIN_ID_MASK			GENMASK(15, 0)
#define MUX_FUNC_MASK			GENMASK(31, 16)
#define RZG2L_PINS_PER_PORT		8
#define RZG2L_PINMUX_TO_PORT(conf)	(((conf) & MUX_PIN_ID_MASK) / RZG2L_PINS_PER_PORT)
#define RZG2L_PINMUX_TO_PIN(conf)	(((conf) & MUX_PIN_ID_MASK) % RZG2L_PINS_PER_PORT)
#define RZG2L_PINMUX_TO_FUNC(conf)	(((conf) & MUX_FUNC_MASK) >> 16)

/* Register offsets and values. */
#define P(n)			(0x0000 + 0x10 + (n))
#define PM(n)			(0x0100 + 0x20 + (n) * 2)
#define PMC(n)			(0x0200 + 0x10 + (n))
#define PFC(n)			(0x0400 + 0x40 + (n) * 4)
#define PIN(n)			(0x0800 + 0x10 + (n))
#define IOLH(n)			(0x1000 + (n) * 8)
#define IEN(n)			(0x1800 + (n) * 8)
#define PWPR			0x3014
#define SD_CH(n)		(0x3000 + (n) * 4)
#define QSPI			0x3008

#define PVDD_1800		1	/* I/O domain voltage <= 1.8V */
#define PVDD_3300		0	/* I/O domain voltage >= 3.3V */

#define PWPR_B0WI		BIT(7)	/* Bit Write Disable */
#define PWPR_PFCWE		BIT(6)	/* PFC Register Write Enable */

#define PM_MASK			0x03
#define PVDD_MASK		0x01
#define PFC_MASK		0x07
#define IEN_MASK		0x01
#define IOLH_MASK		0x03

#define PM_HIGH_Z		0x0
#define PM_INPUT		0x1
#define PM_OUTPUT		0x2
#define PM_OUTPUT_IEN		0x3

struct rzg2l_pfc_data {
	void __iomem *base;
	uint num_dedicated_pins;
	uint num_ports;
	uint num_pins;
	const u32 *gpio_configs;
};

int rzg2l_pfc_enable(struct udevice *dev);
bool rzg2l_port_validate(const struct rzg2l_pfc_data *data, u32 port, u8 pin);

#endif /* RENESAS_RZG2L_PFC_H */
