/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Intel Corporation
 */

/* Include in each PCIe Root Port device */

/* lowest D-state supported by
 * PCIe root port during S0 state
 */
Name (_S0W, 4)

Name (PDST, 0) /* present Detect status */

/* Dynamic Opregion needed to access registers
 * when the controller is in D3 cold
 */
OperationRegion (PX01, PCI_Config, 0x00, 0xFF)
Field (PX01, AnyAcc, NoLock, Preserve)
{
	Offset(0x5A),
	, 6,
	PDS, 1,		/* 6, Presence detect Change */
	Offset(0xE2),	/* RPPGEN - Root Port Power Gating Enable */
	, 2,
	L23E, 1,	/* 2, L23_Rdy Entry Request (L23ER) */
	L23R, 1,	/* 3, L23_Rdy to Detect Transition (L23R2DT) */
	Offset(0xF4),	/* BLKPLLEN */
	, 10,
	BPLL, 1,
}

OperationRegion (PX02, PCI_Config, 0x338, 0x4)
Field (PX02, AnyAcc, NoLock, Preserve)
{
	, 26,
	BDQA, 1		/* BLKDQDA */
}

PowerResource (PXP, 0, 0)
{
	/* Define the PowerResource for PCIe slot */
	Method (_STA, 0, Serialized)
	{
		Store (PDS, PDST)
		If (LEqual (PDS, 1)) {
			Return (0xf)
		} Else {
			Return (0)
		}
	}

	Method (_ON, 0, Serialized)
	{
		If (LAnd (LEqual (PDST, 1), LNotEqual (\PRT0, 0))) {
			/* Enter this condition if device
			 * is connected
			 */

			/* De-assert PERST */
			\_SB.PCI0.PRDA (\PRT0)

			Store (0, BDQA) /* Set BLKDQDA to 0 */
			Store (0, BPLL) /* Set BLKPLLEN to 0 */

			/* Set L23_Rdy to Detect Transition
			 * (L23R2DT)
			 */
			Store (1, L23R)
			Sleep (16)
			Store (0, Local0)

			/* Delay for transition Detect
			 * and link to train
			 */
			While (L23R) {
				If (Lgreater (Local0, 4)) {
					Break
				}
				Sleep (16)
				Increment (Local0)
			}
		} /* End PDS condition check */
	}

	Method (_OFF, 0, Serialized)
	{
		/* Set L23_Rdy Entry Request (L23ER) */
		If (LAnd (LEqual (PDST, 1), LNotEqual (\PRT0, 0))) {
			/* enter this condition if device
			 * is connected
			 */
			Store (1, L23E)
			Sleep (16)
			Store (0, Local0)
			While (L23E) {
				If (Lgreater (Local0, 4)) {
					Break
				}
				Sleep (16)
				Increment (Local0)
			}
			Store (1, BDQA) /* Set BLKDQDA to 1 */
			Store (1, BPLL) /* Set BLKPLLEN to 1 */

			/* Assert PERST */
			\_SB.PCI0.PRAS (\PRT0)
		} /* End PDS condition check */
	} /* End of Method_OFF */
} /* End PXP */

Name(_PR0, Package() { PXP })
Name(_PR3, Package() { PXP })
