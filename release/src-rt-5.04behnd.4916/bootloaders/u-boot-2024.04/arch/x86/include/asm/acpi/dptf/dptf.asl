/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2016 Intel Corporation.
 */

Device (DPTF)
{
	Name (_HID, EISAID ("INT3400"))
	Name (_UID, 0)

	Name (IDSP, Package()
	{
		/* DPPM Passive Policy 1.0 */
		ToUUID ("42A441D6-AE6A-462B-A84B-4A8CE79027D3"),

		/* DPPM Critical Policy */
		ToUUID ("97C68AE7-15FA-499c-B8C9-5DA81D606E0A"),

		/* DPPM Cooling Policy */
		ToUUID ("16CAF1B7-DD38-40ED-B1C1-1B8A1913D531"),

#ifdef DPTF_ENABLE_FAN_CONTROL
		/* DPPM Active Policy */
		ToUUID ("3A95C389-E4B8-4629-A526-C52C88626BAE"),
#endif
	})

	Method (_STA)
	{
		If (LEqual (\DPTE, One)) {
			Return (0xF)
		} Else {
			Return (0x0)
		}
	}

	/*
	 * Arg0: Buffer containing UUID
	 * Arg1: Integer containing Revision ID of buffer format
	 * Arg2: Integer containing count of entries in Arg3
	 * Arg3: Buffer containing list of DWORD capabilities
	 * Return: Buffer containing list of DWORD capabilities
	 */
	Method (_OSC, 4, Serialized)
	{
		/* Check for Passive Policy UUID */
		If (LEqual (DeRefOf (Index (IDSP, 0)), Arg0)) {
			/* Initialize Thermal Devices */
			^TINI ()

#ifdef DPTF_ENABLE_CHARGER
			/* Initialize Charger Device */
			^TCHG.INIT ()
#endif
		}

		Return (Arg3)
	}

	/* Priority based _TRT */
	Name (TRTR, 1)

	Method (_TRT)
	{
		Return (\_SB.DTRT)
	}

#ifdef DPTF_ENABLE_FAN_CONTROL
	/* _ART : Active Cooling Relationship Table */
	Method (_ART)
	{
		Return (\_SB.DART)
	}
#endif

	/* Convert from Degrees C to 1/10 Kelvin for ACPI */
	Method (CTOK, 1) {
		/* 10th of Degrees C */
		Multiply (Arg0, 10, Local0)

		/* Convert to Kelvin */
		Add (Local0, 2732, Local0)

		Return (Local0)
	}

	/* Convert from 1/10 Kelvin to Degrees C for ACPI */
	Method (KTOC, 1) {
		If (LLessEqual (Arg0, 2732)) {
			Return (0)
		}

		/* Convert to Celsius */
		Subtract (Arg0, 2732, Local0)

		/* Convert from 10th of degrees */
		Divide (Local0, 10,, Local0)

		Return (Local0)
	}

	/* Include Thermal Participants */
	#include "thermal.asl"

#ifdef DPTF_ENABLE_CHARGER
	/* Include Charger Participant */
	#include "charger.asl"
#endif

#ifdef DPTF_ENABLE_FAN_CONTROL
	/* Include Fan Participant */
	#include "fan.asl"
#endif

}

Scope (\_SB.PCI0)
{
	#include "cpu.asl"
}
