/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/arch/x86/acpi/globutil.asl
 */

Method(MIN, 2)
{
	If (LLess(Arg0, Arg1)) {
		Return (Arg0)
	} Else {
		Return (Arg1)
	}
}

Method(SLEN, 1)
{
	Store(Arg0, Local0)
	Return (Sizeof(Local0))
}

Method(S2BF, 1, Serialized)
{
	Add(SLEN(Arg0), One, Local0)
	Name(BUFF, Buffer(Local0) {})
	Store(Arg0, BUFF)
	Return (BUFF)
}

/*
 * SCMP - Strong string compare
 *
 * Checks both length and content
 */
Method(SCMP, 2)
{
	Store(S2BF(Arg0), Local0)
	Store(S2BF(Arg1), Local1)
	Store(Zero, Local4)
	Store(SLEN(Arg0), Local5)
	Store(SLEN(Arg1), Local6)
	Store(MIN(Local5, Local6), Local7)

	While (LLess(Local4, Local7)) {
		Store(Derefof(Index(Local0, Local4)), Local2)
		Store(Derefof(Index(Local1, Local4)), Local3)
		If (LGreater(Local2, Local3)) {
			Return (One)
		} Else {
			If (LLess(Local2, Local3)) {
				Return (Ones)
			}
		}
		Increment(Local4)
	}

	If (LLess(Local4, Local5)) {
		Return (One)
	} Else {
		If (LLess(Local4, Local6)) {
			Return (Ones)
		} Else {
			Return (Zero)
		}
	}
}

/*
 * WCMP - Weak string compare
 *
 * Checks to find Arg1 at beginning of Arg0.
 * Fails if length(Arg0) < length(Arg1).
 * Returns 0 on fail, 1 on pass.
 */
Method(WCMP, 2)
{
	Store(S2BF(Arg0), Local0)
	Store(S2BF(Arg1), Local1)
	If (LLess(SLEN(Arg0), SLEN(Arg1))) {
		Return (Zero)
	}
	Store(Zero, Local2)
	Store(SLEN(Arg1), Local3)

	While (LLess(Local2, Local3)) {
		If (LNotEqual(Derefof(Index(Local0, Local2)),
			Derefof(Index(Local1, Local2)))) {
			Return (Zero)
		}
		Increment(Local2)
	}

	Return (One)
}

/*
 * I2BM - Returns Bit Map
 *
 * Arg0 = IRQ Number (0-15)
 */
Method(I2BM, 1)
{
	Store(0, Local0)
	If (LNotEqual(Arg0, 0)) {
		Store(1, Local1)
		ShiftLeft(Local1, Arg0, Local0)
	}

	Return (Local0)
}
