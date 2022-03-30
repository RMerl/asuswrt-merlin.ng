/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/include/asm/mach-bcm63xx/bcm63xx_regs.h
 */

#ifndef __DT_BINDINGS_RESET_BCM6328_H
#define __DT_BINDINGS_RESET_BCM6328_H

#define BCM6328_RST_SPI		0
#define BCM6328_RST_EPHY	1
#define BCM6328_RST_SAR		2
#define BCM6328_RST_ENETSW	3
#define BCM6328_RST_USBS	4
#define BCM6328_RST_USBH	5
#define BCM6328_RST_PCM		6
#define BCM6328_RST_PCIE_CORE	7
#define BCM6328_RST_PCIE	8
#define BCM6328_RST_PCIE_EXT	9
#define BCM6328_RST_PCIE_HARD	10

#endif /* __DT_BINDINGS_RESET_BCM6328_H */
