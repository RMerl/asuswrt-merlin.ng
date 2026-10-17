/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Intel Corp.
 */

#include <asm/arch/iomap.h>

#define MAILBOX_DATA 0x7080
#define MAILBOX_INTF 0x7084
#define PMIO_LENGTH 0x80
#define PMIO_LIMIT 0x480

scope (\_SB) {
	Device (IPC1)
	{
		Name (_HID, "INT34D2")
		Name (_CID, "INT34D2")
		Name (_DDN, "Intel(R) IPC1 Controller")
		Name (RBUF, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, 0x0, 0x2000, IBAR)
			Memory32Fixed (ReadWrite, 0x0, 0x4, MDAT)
			Memory32Fixed (ReadWrite, 0x0, 0x4, MINF)
			IO (Decode16, IOMAP_ACPI_BASE, PMIO_LIMIT,
			      0x04, PMIO_LENGTH)
			Memory32Fixed (ReadWrite, 0x0, 0x2000, SBAR)
			Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , )
			{
			      PMC_INT
			}
		})

		Method (_CRS, 0x0, NotSerialized)
		{
			CreateDwordField (^RBUF, ^IBAR._BAS, IBAS)
			Store (PMC_BAR0, IBAS)

			CreateDwordField (^RBUF, ^MDAT._BAS, MDBA)
			Store (MCH_BASE_ADDRESS + MAILBOX_DATA, MDBA)
			CreateDwordField (^RBUF, ^MINF._BAS, MIBA)
			Store (MCH_BASE_ADDRESS + MAILBOX_INTF, MIBA)

			CreateDwordField (^RBUF, ^SBAR._BAS, SBAS)
			Store (SRAM_BASE_0, SBAS)

			Return (^RBUF)
		}
	}
}
