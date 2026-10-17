/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Intel Corporation.
 */

Device (TFN1)
{
	Name (_HID, "INT3404")
	Name (_UID, 0)
	Name (_STR, Unicode("Fan Control"))

	/* _FIF: Fan Information */
	Name (_FIF, Package ()
	{
		0,	// Revision
		1,	// Fine Grained Control
		2,	// Step Size
		0	// No Low Speed Notification
	})

	/* Return Fan Performance States defined by mainboard */
	Method (_FPS)
	{
		Return (\_SB.DFPS)
	}

	Name (TFST, Package ()
	{
		0,	// Revision
		0x00,	// Control
		0x00	// Speed
	})

	/* _FST: Fan current Status */
	Method (_FST, 0, Serialized,,PkgObj)
	{
		/* Fill in TFST with current control. */
		Store (\_SB.PCI0.LPCB.EC0.FAND, Index (TFST, 1))
		Return (TFST)
	}

	/* _FSL: Fan Speed Level */
	Method (_FSL, 1, Serialized)
	{
		Store (Arg0, \_SB.PCI0.LPCB.EC0.FAND)
	}

	Method (_STA)
	{
		If (LEqual (\DPTE, One))
		{
			Return (0xF)
		} Else {
			Return (0x0)
		}
	}
}
