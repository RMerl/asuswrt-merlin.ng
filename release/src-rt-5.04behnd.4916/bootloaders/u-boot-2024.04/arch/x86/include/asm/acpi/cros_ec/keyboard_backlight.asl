/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015 Google Inc.
 */

Scope (\_SB)
{
	/*
	 * Chrome EC Keyboard Backlight interface
	 */
	Device (KBLT)
	{
		Name (_HID, "GOOG0002")
		Name (_UID, 1)

		/* Ask EC if we even have a backlight
		 * Return 0xf (present, enabled, show in UI, functioning) or 0
		 *
		 * With older EC codebases that don't support the Device
		 * Features bitfield, this reports the keyboard backlight as
		 * enabled since reads to undefined addresses in EC address
		 * space return 0xff and so KBLE will be 1.
		 */
		Method (_STA, 0, NotSerialized)
		{
			/* If query is unsupported, but this code is compiled
			 * in, assume the backlight exists physically.
			 */
			If (LEqual (1, \_SB.PCI0.LPCB.EC0.DFUD)) {
				Return (0xf)
			}
			/* If EC reports that backlight exists, trust it */
			If (LEqual (1, \_SB.PCI0.LPCB.EC0.KBLE)) {
				Return (0xf)
			}
			/* Otherwise: no device -> disable */
			Return (0)
		}

		/* Read current backlight value */
		Method (KBQC, 0, NotSerialized)
		{
			Return (\_SB.PCI0.LPCB.EC0.KBLV)
		}

		/* Write new backlight value */
		Method (KBCM, 1, NotSerialized)
		{
			Store (Arg0, \_SB.PCI0.LPCB.EC0.KBLV)
		}
	}
}
