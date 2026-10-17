/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Intel Corporation.
 * Copyright (C) 2016 Google Inc.
 *
 */

/* Audio Controller - Device 14, Function 0 */

Device (HDAS)
{
	Name (_ADR, 0x000E0000)
	Name (_DDN, "Audio Controller")
	Name (UUID, ToUUID("A69F886E-6CEB-4594-A41F-7B5DCE24C553"))

	/* Device is D3 wake capable */
	Name (_S0W, 3)

	/* NHLT Table Address populated from GNVS values */
	Name (NBUF, ResourceTemplate() {
		QWordMemory (ResourceConsumer, PosDecode, MinFixed,
			MaxFixed, Cacheable, ReadOnly,
			0, 0, 0, 0, 1,,, NHLT, AddressRangeACPI)
		}
	)

	/* can wake up from S3 state */
	Name (_PRW, Package() { GPE0A_AVS_PME_STS, 3 })

	/*
	 * Device Specific Method
	 * Arg0 - UUID
	 * Arg1 - Revision
	 * Arg2 - Function Index
	*/
	Method (_DSM, 4) {
		If (LEqual (Arg0, ^UUID)) {
			/*
			 * Function 0: Function Support Query
			 * Returns a bitmask of functions supported.
			 */
			If (LEqual (Arg2, Zero)) {
				/*
				 * NHLT Query only supported for revision 1 and
				 * if NHLT address and length are set in NVS.
				 */
				If (LAnd (LEqual (Arg1, One),
					LAnd (LNotEqual (NHLA, Zero),
					     LNotEqual (NHLL, Zero)))) {
					Return (Buffer (One) { 0x03 })
				}
				Else {
					Return (Buffer (One) { 0x01 })
				}
			}

			/*
			 * Function 1: Query NHLT memory address used by
			 * Intel Offload Engine Driver to discover any non-HDA
			 * devices that are supported by the DSP.
			 *
			 * Returns a pointer to NHLT table in memory.
			 */
			If (LEqual (Arg2, One)) {
				CreateQWordField (NBUF, ^NHLT._MIN, NBAS)
				CreateQWordField (NBUF, ^NHLT._MAX, NMAS)
				CreateQWordField (NBUF, ^NHLT._LEN, NLEN)
				Store (NHLA, NBAS)
				Store (NHLA, NMAS)
				Store (NHLL, NLEN)
				Return (NBUF)
			}
		}

		Return (Buffer (One) { 0x00 })
	}
}
