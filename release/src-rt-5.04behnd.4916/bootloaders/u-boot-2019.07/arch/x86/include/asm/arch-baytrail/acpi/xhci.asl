/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/xhci.asl
 */

/* XHCI Controller 0:14.0 */

Device (XHCI)
{
	Name(_ADR, 0x00140000)

	/* Power Resources for Wake */
	Name(_PRW, Package() { 13, 3 })

	/* Highest D state in S3 state */
	Name(_S3D, 3)

	Device (RHUB)
	{
		Name(_ADR, 0x00000000)

		Device (PRT1) { Name(_ADR, 1) }	/* USB Port 0 */
		Device (PRT2) { Name(_ADR, 2) }	/* USB Port 1 */
		Device (PRT3) { Name(_ADR, 3) }	/* USB Port 2 */
		Device (PRT4) { Name(_ADR, 4) }	/* USB Port 3 */
	}
}
