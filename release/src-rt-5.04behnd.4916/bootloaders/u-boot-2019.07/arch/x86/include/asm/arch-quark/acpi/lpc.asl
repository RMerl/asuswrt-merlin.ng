/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

/* Intel LPC Bus Device - 0:1f.0 */

Device (LPCB)
{
	Name(_ADR, 0x001f0000)

	OperationRegion(PRTX, PCI_Config, 0x60, 8)
	Field(PRTX, AnyAcc, NoLock, Preserve) {
		PRTA, 8,
		PRTB, 8,
		PRTC, 8,
		PRTD, 8,
		PRTE, 8,
		PRTF, 8,
		PRTG, 8,
		PRTH, 8,
	}

	#include <asm/acpi/irqlinks.asl>

	/* Firmware Hub */
	Device (FWH)
	{
		Name(_HID, EISAID("INT0800"))
		Name(_CRS, ResourceTemplate()
		{
			Memory32Fixed(ReadOnly, 0xff000000, 0x01000000)
		})
	}

	/* 8259 Interrupt Controller */
	Device (PIC)
	{
		Name(_HID, EISAID("PNP0000"))
		Name(_CRS, ResourceTemplate()
		{
			IO(Decode16, 0x20, 0x20, 0x01, 0x02)
			IO(Decode16, 0x24, 0x24, 0x01, 0x02)
			IO(Decode16, 0x28, 0x28, 0x01, 0x02)
			IO(Decode16, 0x2c, 0x2c, 0x01, 0x02)
			IO(Decode16, 0x30, 0x30, 0x01, 0x02)
			IO(Decode16, 0x34, 0x34, 0x01, 0x02)
			IO(Decode16, 0x38, 0x38, 0x01, 0x02)
			IO(Decode16, 0x3c, 0x3c, 0x01, 0x02)
			IO(Decode16, 0xa0, 0xa0, 0x01, 0x02)
			IO(Decode16, 0xa4, 0xa4, 0x01, 0x02)
			IO(Decode16, 0xa8, 0xa8, 0x01, 0x02)
			IO(Decode16, 0xac, 0xac, 0x01, 0x02)
			IO(Decode16, 0xb0, 0xb0, 0x01, 0x02)
			IO(Decode16, 0xb4, 0xb4, 0x01, 0x02)
			IO(Decode16, 0xb8, 0xb8, 0x01, 0x02)
			IO(Decode16, 0xbc, 0xbc, 0x01, 0x02)
			IO(Decode16, 0x4d0, 0x4d0, 0x01, 0x02)
			IRQNoFlags () { 2 }
		})
	}

	/* 8254 timer */
	Device (TIMR)
	{
		Name(_HID, EISAID("PNP0100"))
		Name(_CRS, ResourceTemplate()
		{
			IO(Decode16, 0x40, 0x40, 0x01, 0x04)
			IO(Decode16, 0x50, 0x50, 0x10, 0x04)
			IRQNoFlags() { 0 }
		})
	}

	/* HPET */
	Device (HPET)
	{
		Name(_HID, EISAID("PNP0103"))
		Name(_CID, 0x010CD041)
		Name(_CRS, ResourceTemplate()
		{
			Memory32Fixed(ReadOnly, HPET_BASE_ADDRESS, HPET_BASE_SIZE)
		})

		Method(_STA)
		{
			Return (STA_VISIBLE)
		}
	}

	/* Real Time Clock */
	Device (RTC)
	{
		Name(_HID, EISAID("PNP0B00"))
		Name(_CRS, ResourceTemplate()
		{
			IO(Decode16, 0x70, 0x70, 1, 8)
			IRQNoFlags() { 8 }
		})
	}

	/* LPC device: Resource consumption */
	Device (LDRC)
	{
		Name(_HID, EISAID("PNP0C02"))
		Name(_UID, 2)

		Name(RBUF, ResourceTemplate()
		{
			IO(Decode16, 0x61, 0x61, 0x1, 0x01) /* NMI Status */
			IO(Decode16, 0x63, 0x63, 0x1, 0x01) /* CPU Reserved */
			IO(Decode16, 0x65, 0x65, 0x1, 0x01) /* CPU Reserved */
			IO(Decode16, 0x67, 0x67, 0x1, 0x01) /* CPU Reserved */
			IO(Decode16, 0x80, 0x80, 0x1, 0x01) /* Port 80 Post */
			IO(Decode16, 0x92, 0x92, 0x1, 0x01) /* CPU Reserved */
			IO(Decode16, 0xb2, 0xb2, 0x1, 0x02) /* SWSMI */
		})

		Method(_CRS, 0, NotSerialized)
		{
			Return (RBUF)
		}
	}
}
