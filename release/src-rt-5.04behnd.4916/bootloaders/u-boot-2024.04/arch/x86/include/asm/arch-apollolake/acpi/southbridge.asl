/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 */

#include <p2sb.h>
#include <asm/arch/gpe.h>

/* PCIE device */
#include "pcie.asl"

/* LPSS device */
#include "lpss.asl"

/* PCI IRQ assignment */
#include "pci_irqs.asl"

/* GPIO controller */
#include "gpio.asl"

#include "xhci.asl"

/* LPC */
#include <asm/acpi/lpc.asl>

/* eMMC */
#include "scs.asl"

/* PMC IPC controller */
#include "pmc_ipc.asl"

/* PCI _OSC */
#include <asm/acpi/pci_osc.asl>
