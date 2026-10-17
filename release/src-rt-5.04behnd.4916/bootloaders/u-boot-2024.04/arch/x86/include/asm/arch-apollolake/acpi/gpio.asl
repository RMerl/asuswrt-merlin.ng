/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 */
#include <asm/arch/gpio.h>
#include <asm/intel_pinctrl_defs.h>
// #include <intelblocks/pcr.h>
// #include <soc/pcr_ids.h>
#include <asm/arch/iomap.h>
#include <p2sb.h>
#include "gpiolib.asl"

scope (\_SB) {

	Device (GPO0)
	{
		Name (_HID, GPIO_COMM_NAME)
		Name (_CID, GPIO_COMM_NAME)
		Name (_DDN, GPIO_COMM_0_DESC)
		Name (_UID, 1)

		Name (RBUF, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, 0, 0x4000, RMEM)
			Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , )
			{
				GPIO_BANK_INT
			}
		})

		Method (_CRS, 0x0, NotSerialized)
		{
			CreateDwordField (^RBUF, ^RMEM._BAS, RBAS)
			ShiftLeft (GPIO_COMM0_PID, PCR_PORTID_SHIFT, Local0)
			Or (IOMAP_P2SB_BAR, Local0, RBAS)
			Return (^RBUF)
		}

		Method (_STA, 0x0, NotSerialized)
		{
			Return(0xf)
		}
	}

	Device (GPO1)
	{
		Name (_HID, GPIO_COMM_NAME)
		Name (_CID, GPIO_COMM_NAME)
		Name (_DDN, GPIO_COMM_1_DESC)
		Name (_UID, 2)

		Name (RBUF, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, 0, 0x4000, RMEM)
			Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , )
			{
				GPIO_BANK_INT
			}
		})

		Method (_CRS, 0x0, NotSerialized)
		{
			CreateDwordField (^RBUF, ^RMEM._BAS, RBAS)
			ShiftLeft (GPIO_COMM1_PID, PCR_PORTID_SHIFT, Local0)
			Or (IOMAP_P2SB_BAR, Local0, RBAS)
			Return (^RBUF)
		}

		Method (_STA, 0x0, NotSerialized)
		{
			Return(0xf)
		}
	}

	Device (GPO2)
	{
		Name (_HID, GPIO_COMM_NAME)
		Name (_CID, GPIO_COMM_NAME)
		Name (_DDN, GPIO_COMM_2_DESC)
		Name (_UID, 3)

		Name (RBUF, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, 0, 0x4000, RMEM)
			Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , )
			{
				GPIO_BANK_INT
			}
		})

		Method (_CRS, 0x0, NotSerialized)
		{
			CreateDwordField (^RBUF, ^RMEM._BAS, RBAS)
			ShiftLeft (GPIO_COMM2_PID, PCR_PORTID_SHIFT, Local0)
			Or (IOMAP_P2SB_BAR, Local0, RBAS)
			Return (^RBUF)
		}

		Method (_STA, 0x0, NotSerialized)
		{
			Return(0xf)
		}
	}

	Device (GPO3)
	{
		Name (_HID, GPIO_COMM_NAME)
		Name (_CID, GPIO_COMM_NAME)
		Name (_DDN, GPIO_COMM_3_DESC)
		Name (_UID, 4)

		Name (RBUF, ResourceTemplate ()
		{
			Memory32Fixed (ReadWrite, 0, 0x4000, RMEM)
			Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , )
			{
				GPIO_BANK_INT
			}
		})

		Method (_CRS, 0x0, NotSerialized)
		{
			CreateDwordField (^RBUF, ^RMEM._BAS, RBAS)
			ShiftLeft (GPIO_COMM3_PID, PCR_PORTID_SHIFT, Local0)
			Or (IOMAP_P2SB_BAR, Local0, RBAS)
			Return (^RBUF)
		}

		Method (_STA, 0x0, NotSerialized)
		{
			Return(0xf)
		}
	}

	Scope(\_SB.PCI0) {
		/* PERST Assertion
		 * Note: PERST is Active High
		 */
		Method (PRAS, 0x1, Serialized)
		{
			/*
			 * Assert PERST
			 * local1 - to toggle Tx pin of Dw0
			 * local2 - Address of PERST
			 */
			Store (Arg0, Local2)
			Store (\_SB.GPC0 (Local2), Local1)
			Or (Local1, PAD_CFG0_TX_STATE, Local1)
			\_SB.SPC0 (Local2, Local1)
		}

		/* PERST DE-Assertion */
		Method (PRDA, 0x1, Serialized)
		{
			/*
			 * De-assert PERST
			 * local1 - to toggle Tx pin of Dw0
			 * local2 - Address of PERST
			 */
			Store (Arg0, Local2)
			Store (\_SB.GPC0 (Local2), Local1)
			And (Local1, Not (PAD_CFG0_TX_STATE), Local1)
			\_SB.SPC0 (Local2, Local1)
		}
	}

	/*
	 * Sleep button device ASL code. We are using this device to
	 * add the _PRW method for a dummy wake event to kernel so that
	 * before going to sleep kernel does not clear bit 15 in ACPI
	 * gpe0a enable register which is actually the GPIO_TIER1_SCI_EN bit.
	 */
	Device (SLP)
	{
		Name (_HID, EisaId ("PNP0C0E"))

		Name (_PRW, Package() { GPE0A_GPIO_TIER1_SCI_STS, 0x3 })
	}
}

Scope(\_GPE)
{
	/*
	 * Dummy method for the Tier 1 GPIO SCI enable bit. When kernel reads
	 * _L0F in scope GPE it sets bit for gpio_tier1_sci_en in ACPI enable
	 * register at 0x430. For APL ACPI enable register DW0 i.e., ACPI
	 * GPE0a_EN at 0x430 is reserved.
	 */
	Method(_L0F, 0) {}
}
