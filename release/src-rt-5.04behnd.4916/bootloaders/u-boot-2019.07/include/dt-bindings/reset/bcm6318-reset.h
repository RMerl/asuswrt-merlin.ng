/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/include/asm/mach-bcm63xx/bcm63xx_regs.h
 */

#ifndef __DT_BINDINGS_RESET_BCM6318_H
#define __DT_BINDINGS_RESET_BCM6318_H

#define BCM6318_RST_SPI		0
#define BCM6318_RST_EPHY	1
#define BCM6318_RST_SAR		2
#define BCM6318_RST_ENETSW	3
#define BCM6318_RST_USBD	4
#define BCM6318_RST_USBH	5
#define BCM6318_RST_PCIE_CORE	6
#define BCM6318_RST_PCIE	7
#define BCM6318_RST_PCIE_EXT	8
#define BCM6318_RST_PCIE_HARD	9
#define BCM6318_RST_ADSL	10
#define BCM6318_RST_PHYMIPS	11
#define BCM6318_RST_HOSTMIPS	11

#endif /* __DT_BINDINGS_RESET_BCM6318_H */
