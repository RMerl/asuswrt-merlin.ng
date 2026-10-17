/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

/* Power Button */
Device (PWRB)
{
	Name(_HID, EISAID("PNP0C0C"))
}

/* PS/2 keyboard and mouse */
Scope (\_SB.PCI0.LPCB)
{
	/* 8042 Keyboard */
	Device (PS2K)
	{
		Name(_HID, EISAID("PNP0303"))
		Name(_CRS, ResourceTemplate()
		{
			IO(Decode16, 0x60, 0x60, 0x00, 0x01)
			IO(Decode16, 0x64, 0x64, 0x00, 0x01)
			IRQNoFlags() { 1 }
		})

		Method(_STA, 0, Serialized)
		{
			Return (STA_VISIBLE)
		}
	}

	/* 8042 Mouse */
	Device (PS2M)
	{
		Name(_HID, EISAID("PNP0F03"))
		Name(_CRS, ResourceTemplate()
		{
			IO(Decode16, 0x60, 0x60, 0x00, 0x01)
			IO(Decode16, 0x64, 0x64, 0x00, 0x01)
			IRQNoFlags() { 12 }
		})

		Method(_STA, 0, Serialized)
		{
			Return (STA_VISIBLE)
		}
	}
}
