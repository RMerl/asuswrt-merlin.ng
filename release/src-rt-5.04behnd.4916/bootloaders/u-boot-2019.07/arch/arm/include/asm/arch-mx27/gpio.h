/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012
 * Philippe Reynes <tremyfr@yahoo.fr>
 */


#ifndef __ASM_ARCH_MX27_GPIO_H
#define __ASM_ARCH_MX27_GPIO_H

/* GPIO registers */
struct gpio_regs {
	u32 gpio_dir; /* DDIR */
	u32 ocr1;
	u32 ocr2;
	u32 iconfa1;
	u32 iconfa2;
	u32 iconfb1;
	u32 iconfb2;
	u32 gpio_dr; /* DR */
	u32 gius;
	u32 gpio_psr; /* SSR */
	u32 icr1;
	u32 icr2;
	u32 imr;
	u32 isr;
	u32 gpr;
	u32 swr;
	u32 puen;
	u32 res[0x2f];
};

/* This structure is used by the function imx_gpio_mode */
struct gpio_port_regs {
	struct gpio_regs port[6];
};

/*
 *  GPIO Module and I/O Multiplexer
 */
#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5

#define GPIO_PIN_MASK		0x1f
#define GPIO_PORT_SHIFT		5
#define GPIO_PORT_MASK		(0x7 << GPIO_PORT_SHIFT)
#define GPIO_PORTA		(PORTA << GPIO_PORT_SHIFT)
#define GPIO_PORTB		(PORTB << GPIO_PORT_SHIFT)
#define GPIO_PORTC		(PORTC << GPIO_PORT_SHIFT)
#define GPIO_PORTD		(PORTD << GPIO_PORT_SHIFT)
#define GPIO_PORTE		(PORTE << GPIO_PORT_SHIFT)
#define GPIO_PORTF		(PORTF << GPIO_PORT_SHIFT)

#endif
