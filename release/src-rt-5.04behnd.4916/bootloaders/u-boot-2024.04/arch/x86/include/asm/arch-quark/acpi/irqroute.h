/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <asm/arch/device.h>

#define PCI_DEV_PIRQ_ROUTES \
	PCI_DEV_PIRQ_ROUTE(QUARK_DEV_20, E, F, G, H), \
	PCI_DEV_PIRQ_ROUTE(QUARK_DEV_21, E, F, G, H), \
	PCI_DEV_PIRQ_ROUTE(QUARK_DEV_23, A, B, C, D)

#define PCIE_BRIDGE_IRQ_ROUTES \
	PCIE_BRIDGE_DEV(RP, QUARK_DEV_23, A, B, C, D)
