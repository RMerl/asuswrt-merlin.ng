/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/usb.asl
 */

/* EHCI Controller 0:1d.0 */

Device (EHC1)
{
	Name(_ADR, 0x001d0000)

	/* Power Resources for Wake */
	Name(_PRW, Package() { 13, 4 })

	/* Highest D state in S3 state */
	Name(_S3D, 2)

	/* Highest D state in S4 state */
	Name(_S4D, 2)

	Device (HUB7)
	{
		Name(_ADR, 0x00000000)

		Device(PRT1) { Name(_ADR, 1) }	/* USB Port 0 */
		Device(PRT2) { Name(_ADR, 2) }	/* USB Port 1 */
		Device(PRT3) { Name(_ADR, 3) }	/* USB Port 2 */
		Device(PRT4) { Name(_ADR, 4) }	/* USB Port 3 */
	}
}
