/*/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 */

// Scope (EC0)

/* Mutex for EC battery index interface */
Mutex (BATM, 0)

// Wait for desired battery index to be presented in shared memory
//   Arg0 = battery index
//   Returns Zero on success, One on error.
Method (BTSW, 1)
{
#ifdef EC_ENABLE_SECOND_BATTERY_DEVICE
	If (LEqual (BTIX, Arg0)) {
		Return (Zero)
	}
	If (LGreaterEqual (Arg0, BTCN)) {
		Return (One)
	}
	Store (Arg0, \_SB.PCI0.LPCB.EC0.BTID)
	Store (5, Local0)      // Timeout 5 msec
	While (LNotEqual (BTIX, Arg0))
	{
		Sleep (1)
		Decrement (Local0)
		If (LEqual (Local0, Zero))
		{
			Return (One)
		}
	}
#else
	If (LNotEqual (0, Arg0)) {
		Return (One)
	}
#endif
	Return (Zero)
}

// _STA implementation.
//   Arg0 = battery index
Method (BSTA, 1, Serialized)
{
	If (Acquire (^BATM, 1000)) {
		Return (Zero)
	}

	If (And(Not(BTSW (Arg0)), BTEX)) {
		Store (0x1F, Local0)
	} Else {
		Store (0x0F, Local0)
	}

	Release (^BATM)
	Return (Local0)
}

// _BIF implementation.
//   Arg0 = battery index
//   Arg1 = PBIF
Method (BBIF, 2, Serialized)
{
	If (Acquire (^BATM, 1000)) {
		Return (Arg1)
	}

	If (BTSW (Arg0)) {
		Release (^BATM)
		Return (Arg1)
	}
	// Last Full Charge Capacity
	Store (BTDF, Index (Arg1, 2))

	// Design Voltage
	Store (BTDV, Index (Arg1, 4))

	// Design Capacity
	Store (BTDA, Local0)
	Store (Local0, Index (Arg1, 1))

	// Design Capacity of Warning
	Divide (Multiply (Local0, DWRN), 100, , Local2)
	Store (Local2, Index (Arg1, 5))

	// Design Capacity of Low
	Divide (Multiply (Local0, DLOW), 100, , Local2)
	Store (Local2, Index (Arg1, 6))

	// Get battery info from mainboard
	Store (ToString(Concatenate(BMOD, 0x00)), Index (Arg1, 9))
	Store (ToString(Concatenate(BSER, 0x00)), Index (Arg1, 10))
	Store (ToString(Concatenate(BMFG, 0x00)), Index (Arg1, 12))

	Release (^BATM)
	Return (Arg1)
}

// _BIX implementation.
//   Arg0 = battery index
//   Arg1 = PBIX
Method (BBIX, 2, Serialized)
{
	If (Acquire (^BATM, 1000)) {
		Return (Arg1)
	}

	If (BTSW (Arg0)) {
		Release (^BATM)
		Return (Arg1)
	}
	// Last Full Charge Capacity
	Store (BTDF, Index (Arg1, 3))

	// Design Voltage
	Store (BTDV, Index (Arg1, 5))

	// Design Capacity
	Store (BTDA, Local0)
	Store (Local0, Index (Arg1, 2))

	// Design Capacity of Warning
	Divide (Multiply (Local0, DWRN), 100, , Local2)
	Store (Local2, Index (Arg1, 6))

	// Design Capacity of Low
	Divide (Multiply (Local0, DLOW), 100, , Local2)
	Store (Local2, Index (Arg1, 7))

	// Cycle Count
	Store (BTCC, Index (Arg1, 8))

	// Get battery info from mainboard
	Store (ToString(Concatenate(BMOD, 0x00)), Index (Arg1, 16))
	Store (ToString(Concatenate(BSER, 0x00)), Index (Arg1, 17))
	Store (ToString(Concatenate(BMFG, 0x00)), Index (Arg1, 19))

	Release (^BATM)
	Return (Arg1)
}

// _BST implementation.
//   Arg0 = battery index
//   Arg1 = PBST
//   Arg2 = BSTP
//   Arg3 = BFWK
Method (BBST, 4, Serialized)
{
	If (Acquire (^BATM, 1000)) {
		Return (Arg1)
	}

	If (BTSW (Arg0)) {
		Release (^BATM)
		Return (Arg1)
	}
	//
	// 0: BATTERY STATE
	//
	// bit 0 = discharging
	// bit 1 = charging
	// bit 2 = critical level
	//
	Store (Zero, Local1)

	// Check if AC is present
	If (ACEX) {
		If (BFCG) {
			Store (0x02, Local1)
		} ElseIf (BFDC) {
			Store (0x01, Local1)
		}
	} Else {
		// Always discharging when on battery power
		Store (0x01, Local1)
	}

	// Check for critical battery level
	If (BFCR) {
		Or (Local1, 0x04, Local1)
	}
	Store (Local1, Index (Arg1, 0))

	// Notify if battery state has changed since last time
	If (LNotEqual (Local1, DeRefOf (Arg2))) {
		Store (Local1, Arg2)
		If (LEqual(Arg0, 0)) {
			Notify (BAT0, 0x80)
		}
#ifdef EC_ENABLE_SECOND_BATTERY_DEVICE
		Else {
			Notify (BAT1, 0x80)
		}
#endif
	}

	//
	// 1: BATTERY PRESENT RATE
	//
	Store (BTPR, Index (Arg1, 1))

	//
	// 2: BATTERY REMAINING CAPACITY
	//
	Store (BTRA, Local1)
	If (LAnd (Arg3, LAnd (ACEX, LNot (LAnd (BFDC, BFCG))))) {
		// On AC power and battery is neither charging
		// nor discharging.  Linux expects a full battery
		// to report same capacity as last full charge.
		// https://bugzilla.kernel.org/show_bug.cgi?id=12632
		Store (BTDF, Local2)

		// See if within ~6% of full
		ShiftRight (Local2, 4, Local3)
		If (LAnd (LGreater (Local1, Subtract (Local2, Local3)),
		          LLess (Local1, Add (Local2, Local3))))
		{
			Store (Local2, Local1)
		}
	}
	Store (Local1, Index (Arg1, 2))

	//
	// 3: BATTERY PRESENT VOLTAGE
	//
	Store (BTVO, Index (Arg1, 3))

	Release (^BATM)
	Return (Arg1)
}

Device (BAT0)
{
	Name (_HID, EISAID ("PNP0C0A"))
	Name (_UID, 1)
	Name (_PCL, Package () { \_SB })

	Name (PBIF, Package () {
		0x00000001,  // 0x00: Power Unit: mAh
		0xFFFFFFFF,  // 0x01: Design Capacity
		0xFFFFFFFF,  // 0x02: Last Full Charge Capacity
		0x00000001,  // 0x03: Battery Technology: Rechargeable
		0xFFFFFFFF,  // 0x04: Design Voltage
		0x00000003,  // 0x05: Design Capacity of Warning
		0xFFFFFFFF,  // 0x06: Design Capacity of Low
		0x00000001,  // 0x07: Capacity Granularity 1
		0x00000001,  // 0x08: Capacity Granularity 2
		"",          // 0x09: Model Number
		"",          // 0x0a: Serial Number
		"LION",      // 0x0b: Battery Type
		""           // 0x0c: OEM Information
	})

	Name (PBIX, Package () {
		0x00000000,  // 0x00: Revision
		0x00000001,  // 0x01: Power Unit: mAh
		0xFFFFFFFF,  // 0x02: Design Capacity
		0xFFFFFFFF,  // 0x03: Last Full Charge Capacity
		0x00000001,  // 0x04: Battery Technology: Rechargeable
		0xFFFFFFFF,  // 0x05: Design Voltage
		0x00000003,  // 0x06: Design Capacity of Warning
		0xFFFFFFFF,  // 0x07: Design Capacity of Low
		0x00000000,  // 0x08: Cycle Count
		0x00018000,  // 0x09: Measurement Accuracy (98.3%?)
		0x000001F4,  // 0x0a: Max Sampling Time (500ms)
		0x0000000a,  // 0x0b: Min Sampling Time (10ms)
		0xFFFFFFFF,  // 0x0c: Max Averaging Interval
		0xFFFFFFFF,  // 0x0d: Min Averaging Interval
		0x00000001,  // 0x0e: Capacity Granularity 1
		0x00000001,  // 0x0f: Capacity Granularity 2
		"",          // 0x10 Model Number
		"",          // 0x11: Serial Number
		"LION",      // 0x12: Battery Type
		""           // 0x13: OEM Information
	})

	Name (PBST, Package () {
		0x00000000,  // 0x00: Battery State
		0xFFFFFFFF,  // 0x01: Battery Present Rate
		0xFFFFFFFF,  // 0x02: Battery Remaining Capacity
		0xFFFFFFFF,  // 0x03: Battery Present Voltage
	})
	Name (BSTP, Zero)

	// Workaround for full battery status, disabled by default
	Name (BFWK, Zero)

	// Method to enable full battery workaround
	Method (BFWE)
	{
		Store (One, BFWK)
	}

	// Method to disable full battery workaround
	Method (BFWD)
	{
		Store (Zero, BFWK)
	}

	Method (_STA, 0, Serialized)
	{
		Return (BSTA (0))
	}

	Method (_BIF, 0, Serialized)
	{
		Return (BBIF (0, PBIF))
	}

	Method (_BIX, 0, Serialized)
	{
		Return (BBIX (0, PBIX))
	}

	Method (_BST, 0, Serialized)
	{
		Return (BBST (0, PBST, RefOf (BSTP), BFWK))
	}
}

#ifdef EC_ENABLE_SECOND_BATTERY_DEVICE
Device (BAT1)
{
	Name (_HID, EISAID ("PNP0C0A"))
	Name (_UID, 1)
	Name (_PCL, Package () { \_SB })

	Name (PBIF, Package () {
		0x00000001,  // 0x00: Power Unit: mAh
		0xFFFFFFFF,  // 0x01: Design Capacity
		0xFFFFFFFF,  // 0x02: Last Full Charge Capacity
		0x00000001,  // 0x03: Battery Technology: Rechargeable
		0xFFFFFFFF,  // 0x04: Design Voltage
		0x00000003,  // 0x05: Design Capacity of Warning
		0xFFFFFFFF,  // 0x06: Design Capacity of Low
		0x00000001,  // 0x07: Capacity Granularity 1
		0x00000001,  // 0x08: Capacity Granularity 2
		"",          // 0x09: Model Number
		"",          // 0x0a: Serial Number
		"LION",      // 0x0b: Battery Type
		""           // 0x0c: OEM Information
	})

	Name (PBIX, Package () {
		0x00000000,  // 0x00: Revision
		0x00000001,  // 0x01: Power Unit: mAh
		0xFFFFFFFF,  // 0x02: Design Capacity
		0xFFFFFFFF,  // 0x03: Last Full Charge Capacity
		0x00000001,  // 0x04: Battery Technology: Rechargeable
		0xFFFFFFFF,  // 0x05: Design Voltage
		0x00000003,  // 0x06: Design Capacity of Warning
		0xFFFFFFFF,  // 0x07: Design Capacity of Low
		0x00000000,  // 0x08: Cycle Count
		0x00018000,  // 0x09: Measurement Accuracy (98.3%?)
		0x000001F4,  // 0x0a: Max Sampling Time (500ms)
		0x0000000a,  // 0x0b: Min Sampling Time (10ms)
		0xFFFFFFFF,  // 0x0c: Max Averaging Interval
		0xFFFFFFFF,  // 0x0d: Min Averaging Interval
		0x00000001,  // 0x0e: Capacity Granularity 1
		0x00000001,  // 0x0f: Capacity Granularity 2
		"",          // 0x10 Model Number
		"",          // 0x11: Serial Number
		"LION",      // 0x12: Battery Type
		""           // 0x13: OEM Information
	})

	Name (PBST, Package () {
		0x00000000,  // 0x00: Battery State
		0xFFFFFFFF,  // 0x01: Battery Present Rate
		0xFFFFFFFF,  // 0x02: Battery Remaining Capacity
		0xFFFFFFFF,  // 0x03: Battery Present Voltage
	})
	Name (BSTP, Zero)

	// Workaround for full battery status, disabled by default
	Name (BFWK, Zero)

	// Method to enable full battery workaround
	Method (BFWE)
	{
		Store (One, BFWK)
	}

	// Method to disable full battery workaround
	Method (BFWD)
	{
		Store (Zero, BFWK)
	}

	Method (_STA, 0, Serialized)
	{
		Return (BSTA (1))
	}

	Method (_BIF, 0, Serialized)
	{
		Return (BBIF (1, PBIF))
	}

	Method (_BIX, 0, Serialized)
	{
		Return (BBIX (1, PBIX))
	}

	Method (_BST, 0, Serialized)
	{
		Return (BBST (1, PBST, RefOf (BSTP), BFWK))
	}
}
#endif
