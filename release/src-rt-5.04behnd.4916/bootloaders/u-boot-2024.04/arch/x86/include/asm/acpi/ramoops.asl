/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 */

Scope (\_SB)
{
	Device(RMOP)
	{
		Name (_HID, "GOOG9999")
		Name (_CID, "GOOG9999")
		Name (_UID, 1)

		Name (RBUF, ResourceTemplate()
		{
			Memory32Fixed (ReadWrite, 0, 0, MRES)
		})

		Method (_CRS)
		{
			CreateDwordField (^RBUF, ^MRES._BAS, RBAS)
			CreateDwordField (^RBUF, ^MRES._LEN, RLEN)
			Store (\RMOB, RBAS)
			Store (\RMOL, RLEN)
			Return (^RBUF)
		}
		Method(_STA, 0)
		{
			Return (0xB)
		}
	}
}
