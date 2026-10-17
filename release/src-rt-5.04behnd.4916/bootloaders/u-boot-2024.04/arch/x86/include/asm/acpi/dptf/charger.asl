/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2016 Intel Corporation.
 */

Device (TCHG)
{
	Name (_HID, "INT3403")
	Name (_UID, 0)
	Name (PTYP, 0x0B)
	Name (_STR, Unicode("Battery Charger"))

	Method (_STA)
	{
		If (LEqual (\DPTE, One)) {
			Return (0xF)
		} Else {
			Return (0x0)
		}
	}

	/* Return charger performance states defined by mainboard */
	Method (PPSS)
	{
		Return (\_SB.CHPS)
	}

	/* Return maximum charger current limit */
	Method (PPPC)
	{
		/* Convert size of PPSS table to index */
		Store (SizeOf (\_SB.CHPS), Local0)
		Decrement (Local0)

		/* Check if charging is disabled (AC removed) */
		If (LEqual (\_SB.PCI0.LPCB.EC0.ACEX, Zero)) {
			/* Return last power state */
			Return (Local0)
		} Else {
			/* Return highest power state */
			Return (0)
		}

		Return (0)
	}

	/* Set charger current limit */
	Method (SPPC, 1)
	{
		/* Retrieve Control (index 4) for specified PPSS level */
		Store (DeRefOf (Index (DeRefOf (Index
			(\_SB.CHPS, ToInteger (Arg0))), 4)), Local0)

		/* Pass Control value to EC to limit charging */
		\_SB.PCI0.LPCB.EC0.CHGS (Local0)
	}

	/* Initialize charger participant */
	Method (INIT)
	{
		/* Disable charge limit */
		\_SB.PCI0.LPCB.EC0.CHGD ()
	}
}
