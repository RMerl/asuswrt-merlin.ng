/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/gpio.asl
 */

/* SouthCluster GPIO */
Device (GPSC)
{
	Name(_HID, "INT33FC")
	Name(_CID, "INT33FC")
	Name(_UID, 1)

	Name(RBUF, ResourceTemplate()
	{
		Memory32Fixed(ReadWrite, 0, 0x1000, RMEM)
		Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , ,)
		{
			GPIO_SC_IRQ
		}
	})

	Method(_CRS)
	{
		CreateDwordField(^RBUF, ^RMEM._BAS, RBAS)
		Add(IO_BASE_ADDRESS, IO_BASE_OFFSET_GPSCORE, RBAS)
		Return (^RBUF)
	}

	Method(_STA)
	{
		Return (STA_VISIBLE)
	}
}

/* NorthCluster GPIO */
Device (GPNC)
{
	Name(_HID, "INT33FC")
	Name(_CID, "INT33FC")
	Name(_UID, 2)

	Name(RBUF, ResourceTemplate()
	{
		Memory32Fixed(ReadWrite, 0, 0x1000, RMEM)
		Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , ,)
		{
			GPIO_NC_IRQ
		}
	})

	Method(_CRS)
	{
		CreateDwordField(^RBUF, ^RMEM._BAS, RBAS)
		Add(IO_BASE_ADDRESS, IO_BASE_OFFSET_GPNCORE, RBAS)
		Return (^RBUF)
	}

	Method(_STA)
	{
		Return (STA_VISIBLE)
	}
}

/* SUS GPIO */
Device (GPSS)
{
	Name(_HID, "INT33FC")
	Name(_CID, "INT33FC")
	Name(_UID, 3)

	Name(RBUF, ResourceTemplate()
	{
		Memory32Fixed(ReadWrite, 0, 0x1000, RMEM)
		Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , ,)
		{
			GPIO_SUS_IRQ
		}
	})

	Method(_CRS)
	{
		CreateDwordField(^RBUF, ^RMEM._BAS, RBAS)
		Add(IO_BASE_ADDRESS, IO_BASE_OFFSET_GPSSUS, RBAS)
		Return (^RBUF)
	}

	Method(_STA)
	{
		Return (STA_VISIBLE)
	}
}
