// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <smsc_sio1007.h>

static inline u8 sio1007_read(int port, int reg)
{
	outb(reg, port);

	return inb(port + 1);
}

static inline void sio1007_write(int port, int reg, int val)
{
	outb(reg, port);
	outb(val, port + 1);
}

static inline void sio1007_clrsetbits(int port, int reg, u8 clr, u8 set)
{
	sio1007_write(port, reg, (sio1007_read(port, reg) & ~clr) | set);
}

void sio1007_enable_serial(int port, int num, int iobase, int irq)
{
	if (num < 0 || num > SIO1007_UART_NUM)
		return;

	/* enter configuration state */
	outb(0x55, port);

	/* power on serial port and set up its i/o base & irq */
	if (!num) {
		sio1007_clrsetbits(port, DEV_POWER_CTRL, 0, UART1_POWER_ON);
		sio1007_clrsetbits(port, UART1_IOBASE, 0xfe, iobase >> 2);
		sio1007_clrsetbits(port, UART_IRQ, 0xf0, irq << 4);
	} else {
		sio1007_clrsetbits(port, DEV_POWER_CTRL, 0, UART2_POWER_ON);
		sio1007_clrsetbits(port, UART2_IOBASE, 0xfe, iobase >> 2);
		sio1007_clrsetbits(port, UART_IRQ, 0x0f, irq);
	}

	/* exit configuration state */
	outb(0xaa, port);
}

void sio1007_enable_runtime(int port, int iobase)
{
	/* enter configuration state */
	outb(0x55, port);

	/* set i/o base for the runtime register block */
	sio1007_clrsetbits(port, RTR_IOBASE_LOW, 0, iobase >> 4);
	sio1007_clrsetbits(port, RTR_IOBASE_HIGH, 0, iobase >> 12);
	/* turn on address decoding for this block */
	sio1007_clrsetbits(port, DEV_ACTIVATE, 0, RTR_EN);

	/* exit configuration state */
	outb(0xaa, port);
}

void sio1007_gpio_config(int port, int gpio, int dir, int pol, int type)
{
	int reg = GPIO0_DIR;

	if (gpio < 0 || gpio > SIO1007_GPIO_NUM)
		return;
	if (gpio >= GPIO_NUM_PER_GROUP) {
		reg = GPIO1_DIR;
		gpio -= GPIO_NUM_PER_GROUP;
	}

	/* enter configuration state */
	outb(0x55, port);

	/* set gpio pin direction, polority and type */
	sio1007_clrsetbits(port, reg, 1 << gpio, dir << gpio);
	sio1007_clrsetbits(port, reg + 1, 1 << gpio, pol << gpio);
	sio1007_clrsetbits(port, reg + 2, 1 << gpio, type << gpio);

	/* exit configuration state */
	outb(0xaa, port);
}

int sio1007_gpio_get_value(int port, int gpio)
{
	int reg = GPIO0_DATA;
	int val;

	if (gpio < 0 || gpio > SIO1007_GPIO_NUM)
		return -EINVAL;
	if (gpio >= GPIO_NUM_PER_GROUP) {
		reg = GPIO1_DATA;
		gpio -= GPIO_NUM_PER_GROUP;
	}

	val = inb(port + reg);
	if (val & (1 << gpio))
		return 1;
	else
		return 0;
}

void sio1007_gpio_set_value(int port, int gpio, int val)
{
	int reg = GPIO0_DATA;
	u8 data;

	if (gpio < 0 || gpio > SIO1007_GPIO_NUM)
		return;
	if (gpio >= GPIO_NUM_PER_GROUP) {
		reg = GPIO1_DATA;
		gpio -= GPIO_NUM_PER_GROUP;
	}

	data = inb(port + reg);
	data &= ~(1 << gpio);
	data |= (val << gpio);
	outb(data, port + reg);
}
