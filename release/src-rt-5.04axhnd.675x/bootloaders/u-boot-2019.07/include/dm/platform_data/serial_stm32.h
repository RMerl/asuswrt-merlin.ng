/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#ifndef __SERIAL_STM32_H
#define __SERIAL_STM32_H

/* Information about a serial port */
struct stm32_serial_platdata {
	struct stm32_usart *base;  /* address of registers in physical memory */
};

#endif /* __SERIAL_STM32_H */
