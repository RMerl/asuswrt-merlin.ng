/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 */

Device (ECPD)
{
	Name (_HID, "GOOG0003")
	Name (_UID, 1)
	Name (_DDN, "EC PD Device")
	Method(_STA, 0)
	{
		Return (0xB)
	}
}
