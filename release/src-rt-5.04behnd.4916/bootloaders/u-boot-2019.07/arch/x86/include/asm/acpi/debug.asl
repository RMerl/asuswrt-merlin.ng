/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/arch/x86/acpi/debug.asl
 */

/* POST register region */
OperationRegion(X80, SystemIO, 0x80, 1)
Field(X80, ByteAcc, NoLock, Preserve)
{
	P80, 8
}

/* Legacy serial port register region */
OperationRegion(CREG, SystemIO, 0x3F8, 8)
Field(CREG, ByteAcc, NoLock, Preserve)
{
	CDAT, 8,
	CDLM, 8,
	    , 8,
	CLCR, 8,
	CMCR, 8,
	CLSR, 8
}

/* DINI - Initialize the serial port to 115200 8-N-1 */
Method(DINI)
{
	Store(0x83, CLCR)
	Store(0x01, CDAT)	/* 115200 baud (low) */
	Store(0x00, CDLM)	/* 115200 baud (high) */
	Store(0x03, CLCR)	/* word=8 stop=1 parity=none */
	Store(0x03, CMCR)	/* DTR=1 RTS=1 out1/2=Off loop=Off */
	Store(0x00, CDLM)	/* turn off interrupts */
}

/* THRE - Wait for serial port transmitter holding register to go empty */
Method(THRE)
{
	And(CLSR, 0x20, Local0)
	While (LEqual(Local0, Zero)) {
		And(CLSR, 0x20, Local0)
	}
}

/* OUTX - Send a single raw character */
Method(OUTX, 1)
{
	THRE()
	Store(Arg0, CDAT)
}

/* OUTC - Send a single character, expanding LF into CR/LF */
Method(OUTC, 1)
{
	If (LEqual(Arg0, 0x0a)) {
		OUTX(0x0d)
	}
	OUTX(Arg0)
}

/* DBGN - Send a single hex nibble */
Method(DBGN, 1)
{
	And(Arg0, 0x0f, Local0)
	If (LLess(Local0, 10)) {
		Add(Local0, 0x30, Local0)
	} Else {
		Add(Local0, 0x37, Local0)
	}
	OUTC(Local0)
}

/* DBGB - Send a hex byte */
Method(DBGB, 1)
{
	ShiftRight(Arg0, 4, Local0)
	DBGN(Local0)
	DBGN(Arg0)
}

/* DBGW - Send a hex word */
Method(DBGW, 1)
{
	ShiftRight(Arg0, 8, Local0)
	DBGB(Local0)
	DBGB(Arg0)
}

/* DBGD - Send a hex dword */
Method(DBGD, 1)
{
	ShiftRight(Arg0, 16, Local0)
	DBGW(Local0)
	DBGW(Arg0)
}

/* Get a char from a string */
Method(GETC, 2)
{
	CreateByteField(Arg0, Arg1, DBGC)
	Return (DBGC)
}

/* DBGO - Send either a string or an integer */
Method(DBGO, 1, Serialized)
{
	If (LEqual(ObjectType(Arg0), 1)) {
		If (LGreater(Arg0, 0xffff)) {
			DBGD(Arg0)
		} Else {
			If (LGreater(Arg0, 0xff)) {
				DBGW(Arg0)
			} Else {
				DBGB(Arg0)
			}
		}
	} Else {
		Name(BDBG, Buffer(80) {})
		Store(Arg0, BDBG)
		Store(0, Local1)
		While (One) {
			Store(GETC(BDBG, Local1), Local0)
			If (LEqual(Local0, 0)) {
				Return (Zero)
			}
			OUTC(Local0)
			Increment(Local1)
		}
	}

	Return (Zero)
}
