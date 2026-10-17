/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot gpio_defs.h
 */

#ifndef _ASM_INTEL_PINCTRL_DEFS_H_
#define _ASM_INTEL_PINCTRL_DEFS_H_

/* This file is included by device trees, so avoid BIT() macros */

#define GPIO_DW_SIZE(x)			(sizeof(u32) * (x))
#define PAD_CFG_OFFSET(x, dw_num)	((x) + GPIO_DW_SIZE(dw_num))
#define PAD_CFG0_OFFSET(x)		PAD_CFG_OFFSET(x, 0)
#define PAD_CFG1_OFFSET(x)		PAD_CFG_OFFSET(x, 1)

#define PAD_CFG0_TX_STATE_BIT		0
#define PAD_CFG0_TX_STATE		(1 << PAD_CFG0_TX_STATE_BIT)
#define PAD_CFG0_RX_STATE_BIT		1
#define PAD_CFG0_RX_STATE		(1 << PAD_CFG0_RX_STATE_BIT)
#define PAD_CFG0_TX_DISABLE		(1 << 8)
#define PAD_CFG0_RX_DISABLE		(1 << 9)

#define PAD_CFG0_MODE_SHIFT		10
#define PAD_CFG0_MODE_MASK		(7 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_GPIO		(0 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF1		(1 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF2		(2 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF3		(3 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF4		(4 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF5		(5 << PAD_CFG0_MODE_SHIFT)
#define  PAD_CFG0_MODE_NF6		(6 << PAD_CFG0_MODE_SHIFT)

#define PAD_CFG0_ROUTE_MASK		(0xf << 17)
#define  PAD_CFG0_ROUTE_NMI		(1 << 17)
#define  PAD_CFG0_ROUTE_SMI		(1 << 18)
#define  PAD_CFG0_ROUTE_SCI		(1 << 19)
#define  PAD_CFG0_ROUTE_IOAPIC		(1 << 20)
#define PAD_CFG0_RXTENCFG_MASK		(3 << 21)
#define PAD_CFG0_RXINV_MASK		(1 << 23)
#define  PAD_CFG0_RX_POL_INVERT		(1 << 23)
#define  PAD_CFG0_RX_POL_NONE		(0 << 23)
#define  PAD_CFG0_PREGFRXSEL		(1 << 24)
#define PAD_CFG0_TRIG_MASK		(3 << 25)
#define  PAD_CFG0_TRIG_LEVEL		(0 << 25)
#define  PAD_CFG0_TRIG_EDGE_SINGLE	(1 << 25) /* controlled by RX_INVERT*/
#define  PAD_CFG0_TRIG_OFF		(2 << 25)
#define  PAD_CFG0_TRIG_EDGE_BOTH	(3 << 25)
#define PAD_CFG0_RXRAW1_MASK		(1 << 28)
#define PAD_CFG0_RXPADSTSEL_MASK	(1 << 29)
#define PAD_CFG0_RESET_MASK		(3 << 30)
#define  PAD_CFG0_LOGICAL_RESET_PWROK	(0U << 30)
#define  PAD_CFG0_LOGICAL_RESET_DEEP	(1U << 30)
#define  PAD_CFG0_LOGICAL_RESET_PLTRST	(2U << 30)
#define  PAD_CFG0_LOGICAL_RESET_RSMRST	(3U << 30)

/*
 * Use the fourth bit in IntSel field to indicate gpio ownership. This field is
 * RO and hence not used during gpio configuration.
 */
#define PAD_CFG1_GPIO_DRIVER		(0x1 << 4)
#define PAD_CFG1_IRQ_MASK		(0xff << 0)
#define PAD_CFG1_IOSTERM_MASK		(0x3 << 8)
#define PAD_CFG1_IOSTERM_SAME		(0x0 << 8)
#define PAD_CFG1_IOSTERM_DISPUPD	(0x1 << 8)
#define PAD_CFG1_IOSTERM_ENPD		(0x2 << 8)
#define PAD_CFG1_IOSTERM_ENPU		(0x3 << 8)
#define PAD_CFG1_PULL_MASK		(0xf << 10)
#define  PAD_CFG1_PULL_NONE		(0x0 << 10)
#define  PAD_CFG1_PULL_DN_5K		(0x2 << 10)
#define  PAD_CFG1_PULL_DN_20K		(0x4 << 10)
#define  PAD_CFG1_PULL_UP_1K		(0x9 << 10)
#define  PAD_CFG1_PULL_UP_5K		(0xa << 10)
#define  PAD_CFG1_PULL_UP_2K		(0xb << 10)
#define  PAD_CFG1_PULL_UP_20K		(0xc << 10)
#define  PAD_CFG1_PULL_UP_667		(0xd << 10)
#define  PAD_CFG1_PULL_NATIVE		(0xf << 10)

/* Tx enabled driving last value driven, Rx enabled */
#define PAD_CFG1_IOSSTATE_TX_LAST_RXE	(0x0 << 14)
/*
 * Tx enabled driving 0, Rx disabled and Rx driving 0 back to its controller
 * internally
 */
#define PAD_CFG1_IOSSTATE_TX0_RX_DCR_X0	(0x1 << 14)
/*
 * Tx enabled driving 0, Rx disabled and Rx driving 1 back to its controller
 * internally
 */
#define PAD_CFG1_IOSSTATE_TX0_RX_DCR_X1	(0x2 << 14)
/*
 * Tx enabled driving 1, Rx disabled and Rx driving 0 back to its controller
 * internally
 */
#define PAD_CFG1_IOSSTATE_TX1_RX_DCR_X0	(0x3 << 14)
/*
 * Tx enabled driving 1, Rx disabled and Rx driving 1 back to its controller
 * internally
 */
#define PAD_CFG1_IOSSTATE_TX1_RX_DCR_X1	(0x4 << 14)
/* Tx enabled driving 0, Rx enabled */
#define PAD_CFG1_IOSSTATE_TX0_RXE	(0x5 << 14)
/* Tx enabled driving 1, Rx enabled */
#define PAD_CFG1_IOSSTATE_TX1_RXE	(0x6 << 14)
/* Hi-Z, Rx driving 0 back to its controller internally */
#define PAD_CFG1_IOSSTATE_HIZCRX0	(0x7 << 14)
/* Hi-Z, Rx driving 1 back to its controller internally */
#define PAD_CFG1_IOSSTATE_HIZCRX1	(0x8 << 14)
/* Tx disabled, Rx enabled */
#define PAD_CFG1_IOSSTATE_TXD_RXE	(0x9 << 14)
#define PAD_CFG1_IOSSTATE_IGNORE	(0xf << 14) /* Ignore Iostandby */
/* mask to extract Iostandby bits */
#define PAD_CFG1_IOSSTATE_MASK		(0xf << 14)
#define PAD_CFG1_IOSSTATE_SHIFT		14 /* set Iostandby bits [17:14] */

#define PAD_CFG2_DEBEN			1
/* Debounce Duration = (2 ^ PAD_CFG2_DEBOUNCE_x_RTC) * RTC clock duration */
#define PAD_CFG2_DEBOUNCE_8_RTC		(0x3 << 1)
#define PAD_CFG2_DEBOUNCE_16_RTC	(0x4 << 1)
#define PAD_CFG2_DEBOUNCE_32_RTC	(0x5 << 1)
#define PAD_CFG2_DEBOUNCE_64_RTC	(0x6 << 1)
#define PAD_CFG2_DEBOUNCE_128_RTC	(0x7 << 1)
#define PAD_CFG2_DEBOUNCE_256_RTC	(0x8 << 1)
#define PAD_CFG2_DEBOUNCE_512_RTC	(0x9 << 1)
#define PAD_CFG2_DEBOUNCE_1K_RTC	(0xa << 1)
#define PAD_CFG2_DEBOUNCE_2K_RTC	(0xb << 1)
#define PAD_CFG2_DEBOUNCE_4K_RTC	(0xc << 1)
#define PAD_CFG2_DEBOUNCE_8K_RTC	(0xd << 1)
#define PAD_CFG2_DEBOUNCE_16K_RTC	(0xe << 1)
#define PAD_CFG2_DEBOUNCE_32K_RTC	(0xf << 1)
#define PAD_CFG2_DEBOUNCE_MASK		0x1f

/* voltage tolerance  0=3.3V default 1=1.8V tolerant */
#if IS_ENABLED(INTEL_PINCTRL_IOSTANDBY)
#define PAD_CFG1_TOL_MASK		(0x1 << 25)
#define  PAD_CFG1_TOL_1V8		(0x1 << 25)
#endif

#define PAD_FUNC(value)		PAD_CFG0_MODE_##value
#define PAD_RESET(value)	PAD_CFG0_LOGICAL_RESET_##value
#define PAD_PULL(value)		PAD_CFG1_PULL_##value

#define PAD_IOSSTATE(value)	PAD_CFG1_IOSSTATE_##value
#define PAD_IOSTERM(value)	PAD_CFG1_IOSTERM_##value

#define PAD_IRQ_CFG(route, trig, inv) \
				(PAD_CFG0_ROUTE_##route | \
				PAD_CFG0_TRIG_##trig | \
				PAD_CFG0_RX_POL_##inv)

#if IS_ENABLED(INTEL_PINCTRL_DUAL_ROUTE_SUPPORT)
#define PAD_IRQ_CFG_DUAL_ROUTE(route1, route2, trig, inv)  \
				(PAD_CFG0_ROUTE_##route1 | \
				PAD_CFG0_ROUTE_##route2 | \
				PAD_CFG0_TRIG_##trig | \
				PAD_CFG0_RX_POL_##inv)
#endif /* CONFIG_INTEL_PINCTRL_DUAL_ROUTE_SUPPORT */

#define _PAD_CFG_STRUCT(__pad, __config0, __config1)	\
		__pad(__config0) (__config1)

/* Native function configuration */
#define PAD_CFG_NF(pad, pull, rst, func) \
	_PAD_CFG_STRUCT(pad, PAD_RESET(rst) | PAD_FUNC(func), PAD_PULL(pull) | \
		PAD_IOSSTATE(TX_LAST_RXE))

#if IS_ENABLED(CONFIG_INTEL_GPIO_PADCFG_PADTOL)
/*
 * Native 1.8V tolerant pad, only applies to some pads like I2C/I2S. Not
 * applicable to all SOCs. Refer EDS.
 */
#define PAD_CFG_NF_1V8(pad, pull, rst, func) \
	_PAD_CFG_STRUCT(pad, PAD_RESET(rst) | PAD_FUNC(func), PAD_PULL(pull) |\
		PAD_IOSSTATE(TX_LAST_RXE) | PAD_CFG1_TOL_1V8)
#endif

/* Native function configuration for standby state */
#define PAD_CFG_NF_IOSSTATE(pad, pull, rst, func, iosstate) \
	_PAD_CFG_STRUCT(pad, PAD_RESET(rst) | PAD_FUNC(func), PAD_PULL(pull) | \
		PAD_IOSSTATE(iosstate))

/*
 * Native function configuration for standby state, also configuring iostandby
 * as masked
 */
#define PAD_CFG_NF_IOSTANDBY_IGNORE(pad, pull, rst, func) \
	_PAD_CFG_STRUCT(pad, PAD_RESET(rst) | PAD_FUNC(func), PAD_PULL(pull) | \
		PAD_IOSSTATE(IGNORE))

/*
 * Native function configuration for standby state, also configuring iosstate
 * and iosterm
 */
#define PAD_CFG_NF_IOSSTATE_IOSTERM(pad, pull, rst, func, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad, PAD_RESET(rst) | PAD_FUNC(func), PAD_PULL(pull) | \
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

/* General purpose output, no pullup/down */
#define PAD_CFG_GPO(pad, val, rst)	\
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_RX_DISABLE | !!val, \
		PAD_PULL(NONE) | PAD_IOSSTATE(TX_LAST_RXE))

/* General purpose output, with termination specified */
#define PAD_CFG_TERM_GPO(pad, val, pull, rst)	\
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_RX_DISABLE | !!val, \
		PAD_PULL(pull) | PAD_IOSSTATE(TX_LAST_RXE))

/* General purpose output, no pullup/down */
#define PAD_CFG_GPO_GPIO_DRIVER(pad, val, rst, pull)	\
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_RX_DISABLE | !!val, \
		PAD_PULL(pull) | PAD_IOSSTATE(TX_LAST_RXE) | \
			PAD_CFG1_GPIO_DRIVER)

/* General purpose output */
#define PAD_CFG_GPO_IOSSTATE_IOSTERM(pad, val, rst, pull, iosstate, ioterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_RX_DISABLE | !!val, \
		PAD_PULL(pull) | PAD_IOSSTATE(iosstate) | PAD_IOSTERM(ioterm))

/* General purpose input */
#define PAD_CFG_GPI(pad, pull, rst) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE, \
		PAD_PULL(pull) | PAD_IOSSTATE(TXD_RXE))

/* General purpose input. The following macro sets the
 * Host Software Pad Ownership to GPIO Driver mode.
 */
#define PAD_CFG_GPI_GPIO_DRIVER(pad, pull, rst) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE, \
		PAD_PULL(pull) | PAD_CFG1_GPIO_DRIVER | PAD_IOSSTATE(TXD_RXE))

#define PAD_CFG_GPIO_DRIVER_HI_Z(pad, pull, rst, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE |	\
		PAD_CFG0_RX_DISABLE,					\
		PAD_PULL(pull) | PAD_CFG1_GPIO_DRIVER |			\
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

#define PAD_CFG_GPIO_HI_Z(pad, pull, rst, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE |	\
		PAD_CFG0_RX_DISABLE, PAD_PULL(pull) |			\
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

/* GPIO Interrupt */
#define PAD_CFG_GPI_INT(pad, pull, rst, trig) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE |	\
			PAD_CFG0_TRIG_##trig | PAD_CFG0_RX_POL_NONE,	\
		PAD_PULL(pull) | PAD_CFG1_GPIO_DRIVER | PAD_IOSSTATE(TXD_RXE))

/*
 * No Connect configuration for unused pad.
 * Both TX and RX are disabled. RX disabling is done to avoid unnecessary
 * setting of GPI_STS.
 */
#define PAD_NC(pad, pull)			\
	_PAD_CFG_STRUCT(pad,					\
		PAD_FUNC(GPIO) | PAD_RESET(DEEP) |		\
		PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE,	\
		PAD_PULL(pull) | PAD_IOSSTATE(TXD_RXE))

/* General purpose input, routed to APIC */
#define PAD_CFG_GPI_APIC(pad, pull, rst, trig, inv) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(IOAPIC, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(TXD_RXE))

/* General purpose input, routed to APIC - with IOStandby Config*/
#define PAD_CFG_GPI_APIC_IOS(pad, pull, rst, trig, inv, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(IOAPIC, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

/*
 * The following APIC macros assume the APIC will handle the filtering
 * on its own end. One just needs to pass an active high message into the
 * ITSS.
 */
#define PAD_CFG_GPI_APIC_LOW(pad, pull, rst) \
	PAD_CFG_GPI_APIC(pad, pull, rst, LEVEL, INVERT)

#define PAD_CFG_GPI_APIC_HIGH(pad, pull, rst) \
	PAD_CFG_GPI_APIC(pad, pull, rst, LEVEL, NONE)

#define PAD_CFG_GPI_APIC_EDGE_LOW(pad, pull, rst) \
	PAD_CFG_GPI_APIC(pad, pull, rst, EDGE_SINGLE, INVERT)

/* General purpose input, routed to SMI */
#define PAD_CFG_GPI_SMI(pad, pull, rst, trig, inv) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(SMI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(TXD_RXE))

/* General purpose input, routed to SMI */
#define PAD_CFG_GPI_SMI_IOS(pad, pull, rst, trig, inv, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(SMI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

#define PAD_CFG_GPI_SMI_LOW(pad, pull, rst, trig) \
	PAD_CFG_GPI_SMI(pad, pull, rst, trig, INVERT)

#define PAD_CFG_GPI_SMI_HIGH(pad, pull, rst, trig) \
	PAD_CFG_GPI_SMI(pad, pull, rst, trig, NONE)

/* General purpose input, routed to SCI */
#define PAD_CFG_GPI_SCI(pad, pull, rst, trig, inv) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(SCI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(TXD_RXE))

/* General purpose input, routed to SCI */
#define PAD_CFG_GPI_SCI_IOS(pad, pull, rst, trig, inv, iosstate, iosterm) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(SCI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(iosstate) | PAD_IOSTERM(iosterm))

#define PAD_CFG_GPI_SCI_LOW(pad, pull, rst, trig) \
	PAD_CFG_GPI_SCI(pad, pull, rst, trig, INVERT)

#define PAD_CFG_GPI_SCI_HIGH(pad, pull, rst, trig) \
	PAD_CFG_GPI_SCI(pad, pull, rst, trig, NONE)

#define PAD_CFG_GPI_SCI_DEBEN(pad, pull, rst, trig, inv, dur) \
	_PAD_CFG_STRUCT_3(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(SCI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(TXD_RXE), PAD_CFG2_DEBEN | PAD_CFG2_##dur)

#define PAD_CFG_GPI_SCI_LOW_DEBEN(pad, pull, rst, trig, dur) \
	PAD_CFG_GPI_SCI_DEBEN(pad, pull, rst, trig, INVERT, dur)

#define PAD_CFG_GPI_SCI_HIGH_DEBEN(pad, pull, rst, trig, dur) \
	PAD_CFG_GPI_SCI_DEBEN(pad, pull, rst, trig, NONE, dur)

/* General purpose input, routed to NMI */
#define PAD_CFG_GPI_NMI(pad, pull, rst, trig, inv) \
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG(NMI, trig, inv), PAD_PULL(pull) | \
		PAD_IOSSTATE(TXD_RXE))

#if IS_ENABLED(INTEL_PINCTRL_DUAL_ROUTE_SUPPORT)
/* GPI, GPIO Driver, SCI interrupt */
#define PAD_CFG_GPI_GPIO_DRIVER_SCI(pad, pull, rst, trig, inv)	\
	_PAD_CFG_STRUCT(pad,		\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
			PAD_IRQ_CFG(SCI, trig, inv),	\
		PAD_PULL(pull) | PAD_CFG1_GPIO_DRIVER | PAD_IOSSTATE(TXD_RXE))

#define PAD_CFG_GPI_DUAL_ROUTE(pad, pull, rst, trig, inv, route1, route2) \
	_PAD_CFG_STRUCT(pad,						\
		PAD_FUNC(GPIO) | PAD_RESET(rst) | PAD_CFG0_TX_DISABLE | \
		PAD_IRQ_CFG_DUAL_ROUTE(route1, route2,  trig, inv), \
		PAD_PULL(pull) | PAD_IOSSTATE(TXD_RXE))

#define PAD_CFG_GPI_IRQ_WAKE(pad, pull, rst, trig, inv)	\
	PAD_CFG_GPI_DUAL_ROUTE(pad, pull, rst, trig, inv, IOAPIC, SCI)

#endif /* CONFIG_INTEL_PINCTRL_DUAL_ROUTE_SUPPORT */

#endif /* _ASM_INTEL_PINCTRL_DEFS_H_ */
