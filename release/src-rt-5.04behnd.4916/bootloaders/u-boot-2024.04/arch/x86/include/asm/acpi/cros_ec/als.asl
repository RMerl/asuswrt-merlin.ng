/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 */

Device (ALS)
{
	Name (_HID, "ACPI0008")
	Name (_UID, 1)

	Method (_STA, 0, NotSerialized)
	{
		Return (0xF)
	}

	/*
	 * Returns the current ambient light illuminance reading in lux
	 *
	 *  0: Reading is below the range of sensitivity of the sensor
	 * -1: Reading is above the range or sensitivity of the sensor
	 */
	Method (_ALI, 0, NotSerialized)
	{
		Return (^^ALS0)
	}

	/*
	 * Returns a recommended polling frequency in tenths of seconds
	 *
	 *  0: No need to poll, async notifications will indicate changes
	 */
	Name (_ALP, 10)

	/*
	 * Returns a package of packages where each tuple consists of a pair
	 * of integers mapping ambient light illuminance to display brightness.
	 *
	 * {<display luminance adjustment>, <ambient light illuminance>}
	 *
	 * Ambient light illuminance values are specified in lux.
	 *
	 * Display luminance adjustment values are relative percentages where
	 * 100 is no (0%) display brightness adjustment.  Values <100 indicate
	 * negative adjustment (dimming) and values >100 indicate positive
	 * adjustment (brightening).
	 *
	 * This is currently unused by the Linux kernel ACPI ALS driver but
	 * is required by the ACPI specification so just define a basic two
	 * point response curve.
	 */
	Name (_ALR, Package ()
	{
		Package () { 70, 30 },    // Min { -30% adjust at 30 lux }
		Package () { 150, 1000 }  // Max { +50% adjust at 1000 lux }
	})
}
