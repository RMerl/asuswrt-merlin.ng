/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015 Google Inc.
 * Copyright (C) 2018 Intel Corporation.
 */

#include <intelblocks/pcr.h>

/*
 * Calculate PCR register base at specified PID
 * Arg0 - PCR Port ID
 */
Method (PCRB, 1, NotSerialized)
{
	Return (Add (IOMAP_P2SB_BAR,
				ShiftLeft (Arg0, PCR_PORTID_SHIFT)))
}

/*
 * Read a PCR register at specified PID and offset
 * Arg0 - PCR Port ID
 * Arg1 - Register Offset
 */
Method (PCRR, 2, Serialized)
{
	OperationRegion (PCRD, SystemMemory, Add (PCRB (Arg0), Arg1), 4)
	Field (PCRD, DWordAcc, NoLock, Preserve)
	{
		DATA, 32
	}
	Return (DATA)
}

/*
 * AND a value with PCR register at specified PID and offset
 * Arg0 - PCR Port ID
 * Arg1 - Register Offset
 * Arg2 - Value to AND
 */
Method (PCRA, 3, Serialized)
{
	OperationRegion (PCRD, SystemMemory, Add (PCRB (Arg0), Arg1), 4)
	Field (PCRD, DWordAcc, NoLock, Preserve)
	{
		DATA, 32
	}
	And (DATA, Arg2, DATA)

	/*
	 * After every write one needs to read an innocuous register
	 * to ensure the writes are completed for certain ports. This is done
	 * for all ports so that the callers don't need the per-port knowledge
	 * for each transaction.
	 */
	PCRR (Arg0, Arg1)
}

/*
 * OR a value with PCR register at specified PID and offset
 * Arg0 - PCR Port ID
 * Arg1 - Register Offset
 * Arg2 - Value to OR
 */
Method (PCRO, 3, Serialized)
{
	OperationRegion (PCRD, SystemMemory, Add (PCRB (Arg0), Arg1), 4)
	Field (PCRD, DWordAcc, NoLock, Preserve)
	{
		DATA, 32
	}
	Or (DATA, Arg2, DATA)

	/*
	 * After every write one needs to read an innocuous register
	 * to ensure the writes are completed for certain ports. This is done
	 * for all ports so that the callers don't need the per-port knowledge
	 * for each transaction.
	 */
	PCRR (Arg0, Arg1)
}
