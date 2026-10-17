/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 */

	Name(_HID, EISAID("PNP0A08"))	/* PCIe */
	Name(_CID, EISAID("PNP0A03"))	/* PCI */
	Name(_BBN, 0)

Device (MCHC)
{
	Name (_ADR, 0x00000000)		/*Dev0 Func0 */

		OperationRegion (MCHP, PCI_Config, 0x00, 0x100)
		Field (MCHP, DWordAcc, NoLock, Preserve)
		{
			Offset(0x60),
			MCNF,	32,	/* PCI MMCONF base */
			Offset (0xA8),
			TUUD, 64,	/* Top of Upper Used Memory */
			Offset(0xB4),
			BGSM,   32,	/* Base of Graphics Stolen Memory */
			Offset(0xBC),
			TLUD,   32,	/* Top of Low Useable DRAM */
		}
}
Name (MCRS, ResourceTemplate()
{
	/* Bus Numbers */
	WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
			0x0000, 0x0000, 0x00ff, 0x0000, 0x0100,,,)

	/* IO Region 0 */
	DWordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
			0x0000, 0x0000, 0x0cf7, 0x0000, 0x0cf8,,,)

	/* PCI Config Space */
	Io (Decode16, 0x0cf8, 0x0cf8, 0x0001, 0x0008)

	/* IO Region 1 */
	DWordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
			0x0000, 0x01000, 0xffff, 0x0000, 0xf000,,,)

	/* VGA memory (0xa0000-0xbffff) */
	DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
			Cacheable, ReadWrite,
			0x00000000, 0x000a0000, 0x000bffff, 0x00000000,
			0x00020000,,,)

	/* Data and GFX stolen memory */
	DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
			Cacheable, ReadWrite,
			0x00000000, 0x3be00000, 0x3fffffff, 0x00000000,
			0x04200000,,, STOM)

	/*
	 * PCI MMIO Region (TOLUD - PCI extended base MMCONF)
	 * This assumes that MMCONF is placed after PCI config space,
	 * and that no resources are allocated after the MMCONF region.
	 * This works, sicne MMCONF is hardcoded to 0xe00000000.
	 */
	DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
			NonCacheable, ReadWrite,
			0x00000000, 0x00000000, 0x00000000, 0x00000000,
			0x00000000,,, PM01)

	/* PCI Memory Region (TOUUD - (TOUUD + ABOVE_4G_MMIO_SIZE)) */
	QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
			NonCacheable, ReadWrite,
			0x00000000, 0x10000, 0x1ffff, 0x00000000,
			0x10000,,, PM02)
})

/* Current Resource Settings */
Method (_CRS, 0, Serialized)
{

	/* Find PCI resource area in MCRS */
	CreateDwordField (MCRS, ^PM01._MIN, PMIN)
	CreateDwordField (MCRS, ^PM01._MAX, PMAX)
	CreateDwordField (MCRS, ^PM01._LEN, PLEN)

	/* Read C-Unit PCI CFG Reg. 0xBC for TOLUD (shadow from B-Unit) */
	And(^MCHC.TLUD, 0xFFF00000, PMIN)
	/* Read MMCONF base */
	And(^MCHC.MCNF, 0xF0000000, PMAX)

	/* Calculate PCI MMIO Length */
	Add(Subtract(PMAX, PMIN), 1, PLEN)

	/* Find GFX resource area in GCRS */
	CreateDwordField(MCRS, ^STOM._MIN, GMIN)
	CreateDwordField(MCRS, ^STOM._MAX, GMAX)
	CreateDwordField(MCRS, ^STOM._LEN, GLEN)

	/* Read BGSM */
	And(^MCHC.BGSM, 0xFFF00000, GMIN)

	/* Read TOLUD */
	And(^MCHC.TLUD, 0xFFF00000, GMAX)
	Decrement(GMAX)
	Add(Subtract(GMAX, GMIN), 1, GLEN)

	/* Patch PM02 range based on Memory Size */
	CreateQwordField (MCRS, ^PM02._MIN, MMIN)
	CreateQwordField (MCRS, ^PM02._MAX, MMAX)
	CreateQwordField (MCRS, ^PM02._LEN, MLEN)

	Store (^MCHC.TUUD, Local0)

	If (LLessEqual (Local0, 0x1000000000))
	{
		Store (0, MMIN)
		Store (0, MLEN)
	}
	Subtract (Add (MMIN, MLEN), 1, MMAX)

	Return (MCRS)
}
