/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/irqroute.asl
 */

Name(\PICM, 0)

/*
 * The _PIC method is called by the OS to choose between interrupt
 * routing via the i8259 interrupt controller or the APIC.
 *
 * _PIC is called with a parameter of 0 for i8259 configuration and
 * with a parameter of 1 for Local APIC/IOAPIC configuration.
 */
Method(\_PIC, 1)
{
	/* Remember the OS' IRQ routing choice */
	Store(Arg0, PICM)
}

/* PCI interrupt routing */
Method(_PRT) {
	If (PICM) {
		Return (Package() {
			#undef PIC_MODE
			#include "irq_helper.h"
			PCI_DEV_PIRQ_ROUTES
		})
	} Else {
		Return (Package() {
			#define PIC_MODE
			#include "irq_helper.h"
			PCI_DEV_PIRQ_ROUTES
		})
	}

}

/* PCIe downstream ports interrupt routing */
PCIE_BRIDGE_IRQ_ROUTES
#undef PIC_MODE
#include "irq_helper.h"
PCIE_BRIDGE_IRQ_ROUTES
