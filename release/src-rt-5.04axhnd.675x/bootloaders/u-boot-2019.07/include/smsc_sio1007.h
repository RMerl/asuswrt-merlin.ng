/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _SMSC_SIO1007_H_
#define _SMSC_SIO1007_H_

/*
 * The I/O base address of SIO1007 at power-up is determined by the SYSOPT0
 * and SYSOPT1 pins at the deasserting edge of PCIRST#. The combination of
 * SYSOPT0 and SYSOPT1 determines one of the following addresses.
 */
#define SIO1007_IOPORT0		0x002e
#define SIO1007_IOPORT1		0x004e
#define SIO1007_IOPORT2		0x162e
#define SIO1007_IOPORT3		0x164e

/* SIO1007 registers */

#define DEV_POWER_CTRL		0x02
#define UART1_POWER_ON		(1 << 3)
#define UART2_POWER_ON		(1 << 7)

#define UART1_IOBASE		0x24
#define UART2_IOBASE		0x25
#define UART_IRQ		0x28

#define RTR_IOBASE_HIGH		0x21
#define RTR_IOBASE_LOW		0x30

#define GPIO0_DIR		0x31
#define GPIO1_DIR		0x35
#define GPIO_DIR_INPUT		0
#define GPIO_DIR_OUTPUT		1

#define GPIO0_POL		0x32
#define GPIO1_POL		0x36
#define GPIO_POL_NO_INVERT	0
#define GPIO_POL_INVERT		1

#define GPIO0_TYPE		0x33
#define GPIO1_TYPE		0x37
#define GPIO_TYPE_PUSH_PULL	0
#define GPIO_TYPE_OPEN_DRAIN	1

#define DEV_ACTIVATE		0x3a
#define RTR_EN			(1 << 1)

/* Runtime register offset */

#define GPIO0_DATA		0xc
#define GPIO1_DATA		0xe

/* Number of serial ports supported */
#define SIO1007_UART_NUM	2

/* Number of gpio pins supported */
#define GPIO_NUM_PER_GROUP	8
#define GPIO_GROUP_NUM		2
#define SIO1007_GPIO_NUM	(GPIO_NUM_PER_GROUP * GPIO_GROUP_NUM)

/**
 * Configure the I/O port address of the specified serial device and
 * enable the serial device.
 *
 * @port:	SIO1007 I/O port address
 * @num:	serial device number (0 or 1)
 * @iobase:	processor I/O port address to assign to this serial device
 * @irq:	processor IRQ number to assign to this serial device
 */
void sio1007_enable_serial(int port, int num, int iobase, int irq);

/**
 * Configure the I/O port address of the runtime register block and
 * enable the address decoding.
 *
 * @port:	SIO1007 I/O port address
 * @iobase:	processor I/O port address to assign to the runtime registers
 */
void sio1007_enable_runtime(int port, int iobase);

/**
 * Configure the direction/polority/type of a specified GPIO pin
 *
 * @port:	SIO1007 I/O port address
 * @gpio:	GPIO number (0-7 for GP10-GP17, 8-15 for GP30-GP37)
 * @dir:	GPIO_DIR_INPUT or GPIO_DIR_OUTPUT
 * @pol:	GPIO_POL_NO_INVERT or GPIO_POL_INVERT
 * @type:	GPIO_TYPE_PUSH_PULL or GPIO_TYPE_OPEN_DRAIN
 */
void sio1007_gpio_config(int port, int gpio, int dir, int pol, int type);

/**
 * Get a GPIO pin value.
 * This will work whether the GPIO is an input or an output.
 *
 * @port:	runtime register block I/O port address
 * @gpio:	GPIO number (0-7 for GP10-GP17, 8-15 for GP30-GP37)
 * @return:	0 if low, 1 if high, -EINVAL if gpio number is invalid
 */
int sio1007_gpio_get_value(int port, int gpio);

/**
 * Set a GPIO pin value.
 * This will only work when the GPIO is configured as an output.
 *
 * @port:	runtime register block I/O port address
 * @gpio:	GPIO number (0-7 for GP10-GP17, 8-15 for GP30-GP37)
 * @val:	0 if low, 1 if high
 */
void sio1007_gpio_set_value(int port, int gpio, int val);

#endif /* _SMSC_SIO1007_H_ */
