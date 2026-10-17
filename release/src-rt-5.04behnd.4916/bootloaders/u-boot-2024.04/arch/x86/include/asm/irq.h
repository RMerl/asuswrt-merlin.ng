/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ARCH_IRQ_H_
#define _ARCH_IRQ_H_

#include <dt-bindings/interrupt-router/intel-irq.h>

/**
 * Intel interrupt router configuration mechanism
 *
 * There are two known ways of Intel interrupt router configuration mechanism
 * so far. On most cases, the IRQ routing configuration is controlled by PCI
 * configuration registers on the legacy bridge, normally PCI BDF(0, 31, 0).
 * On some newer platforms like BayTrail and Braswell, the IRQ routing is now
 * in the IBASE register block where IBASE is memory-mapped.
 */
enum pirq_config {
	PIRQ_VIA_PCI,
	PIRQ_VIA_IBASE
};

struct pirq_regmap {
	int link;
	int offset;
};

/**
 * Intel interrupt router control block
 *
 * Its members' value will be filled in based on device tree's input.
 *
 * @config:	PIRQ_VIA_PCI or PIRQ_VIA_IBASE
 * @link_base:	link value base number
 * @link_num:	number of PIRQ links supported
 * @has_regmap:	has mapping table between PIRQ link and routing register offset
 * @irq_mask:	IRQ mask representing the 16 IRQs in 8259, bit N is 1 means
 *		IRQ N is available to be routed
 * @lb_bdf:	irq router's PCI bus/device/function number encoding
 * @ibase:	IBASE register block base address
 * @actl_8bit:	ACTL register width is 8-bit (for ICH series chipset)
 * @actl_addr:	ACTL register offset
 */
struct irq_router {
	int config;
	u32 link_base;
	int link_num;
	bool has_regmap;
	struct pirq_regmap *regmap;
	u16 irq_mask;
	u32 bdf;
	u32 ibase;
	bool actl_8bit;
	int actl_addr;
};

struct pirq_routing {
	int bdf;
	int pin;
	int pirq;
};

#define PIRQ_BITMAP		0xdef8

#endif /* _ARCH_IRQ_H_ */
