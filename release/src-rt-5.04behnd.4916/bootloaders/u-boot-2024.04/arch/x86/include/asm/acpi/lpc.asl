/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Intel Corporation.
 */

/* Intel LPC/eSPI Bus Device  - 0:1f.0 */
#include <asm/arch/iomap.h>

Device (LPCB)
{
	Name (_ADR, 0x001f0000)
	Name (_DDN, "LPC Bus Device")

	/* DMA Controller */
	Device (DMAC)
	{
		Name (_HID, EISAID("PNP0200"))
		Name (_CRS, ResourceTemplate()
		{
			IO (Decode16, 0x00, 0x00, 0x01, 0x20)
			IO (Decode16, 0x81, 0x81, 0x01, 0x11)
			IO (Decode16, 0x93, 0x93, 0x01, 0x0d)
			IO (Decode16, 0xc0, 0xc0, 0x01, 0x20)
			DMA (Compatibility, NotBusMaster, Transfer8_16) { 4 }
		})
	}

	/* Firmware Hub */
	Device (FWH)
	{
		Name (_HID, EISAID ("INT0800"))
		Name (_DDN, "Firmware Hub")
		Name (_CRS, ResourceTemplate ()
		{
			Memory32Fixed (ReadOnly, 0xff000000, 0x01000000)
		})
	}

	/* High Precision Event Timer */
	Device (HPET)
	{
		Name (_HID, EISAID ("PNP0103"))
		Name (_CID, 0x010CD041)
		Name (_DDN, "High Precision Event Timer")
		Name (_CRS, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, HPET_BASE_ADDRESS, 0x400)
		})
		Method (_STA, 0)
		{
			Return (0xF)
		}
	}

	/* FPU */
	Device(MATH)
	{
		Name (_HID, EISAID("PNP0C04"))
		Name (_CRS, ResourceTemplate()
		{
			IO (Decode16, 0xf0, 0xf0, 0x01, 0x01)
			IRQNoFlags() { 13 }
		})
	}

	/* AT Interrupt Controller */
	Device (PIC)
	{
		Name (_HID, EISAID ("PNP0000"))
		Name (_DDN, "8259 Interrupt Controller")
		Name (_CRS, ResourceTemplate()
		{
			IO (Decode16, 0x20, 0x20, 0x01, 0x02)
			IO (Decode16, 0x24, 0x24, 0x01, 0x02)
			IO (Decode16, 0x28, 0x28, 0x01, 0x02)
			IO (Decode16, 0x2c, 0x2c, 0x01, 0x02)
			IO (Decode16, 0x30, 0x30, 0x01, 0x02)
			IO (Decode16, 0x34, 0x34, 0x01, 0x02)
			IO (Decode16, 0x38, 0x38, 0x01, 0x02)
			IO (Decode16, 0x3c, 0x3c, 0x01, 0x02)
			IO (Decode16, 0xa0, 0xa0, 0x01, 0x02)
			IO (Decode16, 0xa4, 0xa4, 0x01, 0x02)
			IO (Decode16, 0xa8, 0xa8, 0x01, 0x02)
			IO (Decode16, 0xac, 0xac, 0x01, 0x02)
			IO (Decode16, 0xb0, 0xb0, 0x01, 0x02)
			IO (Decode16, 0xb4, 0xb4, 0x01, 0x02)
			IO (Decode16, 0xb8, 0xb8, 0x01, 0x02)
			IO (Decode16, 0xbc, 0xbc, 0x01, 0x02)
			IO (Decode16, 0x4d0, 0x4d0, 0x01, 0x02)
			IRQNoFlags () { 2 }
		})
	}

	/* LPC device: Resource consumption */
	Device (LDRC)
	{
		Name (_HID, EISAID ("PNP0C02"))
		Name (_UID, 2)
		Name (_DDN, "Legacy Device Resources")
		Name (_CRS, ResourceTemplate ()
		{
			IO (Decode16, 0x2e, 0x2e, 0x1, 0x02) // First SuperIO
			IO (Decode16, 0x4e, 0x4e, 0x1, 0x02) // Second SuperIO
			IO (Decode16, 0x61, 0x61, 0x1, 0x01) // NMI Status
			IO (Decode16, 0x63, 0x63, 0x1, 0x01) // CPU Reserved
			IO (Decode16, 0x65, 0x65, 0x1, 0x01) // CPU Reserved
			IO (Decode16, 0x67, 0x67, 0x1, 0x01) // CPU Reserved
			IO (Decode16, 0x80, 0x80, 0x1, 0x01) // Port 80 Post
			IO (Decode16, 0x92, 0x92, 0x1, 0x01) // CPU Reserved
			IO (Decode16, 0xb2, 0xb2, 0x1, 0x02) // SWSMI
			IO (Decode16, ACPI_BASE_ADDRESS, ACPI_BASE_ADDRESS,
			    0x1, 0xff)
		})
	}

	/* Real Time Clock Device */
	Device (RTC)
	{
		Name (_HID, EISAID ("PNP0B00"))
		Name (_DDN, "Real Time Clock")
		Name (_CRS, ResourceTemplate ()
		{
			IO (Decode16, 0x70, 0x70, 1, 8)
		})
	}

	/* Timer */
	Device (TIMR)
	{
		Name (_HID, EISAID ("PNP0100"))
		Name (_DDN, "8254 Timer")
		Name (_CRS, ResourceTemplate ()
		{
			IO (Decode16, 0x40, 0x40, 0x01, 0x04)
			IO (Decode16, 0x50, 0x50, 0x10, 0x04)
			IRQNoFlags () {0}
		})
	}
}
