/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

/* These come from the dynamically created CPU SSDT */
External (\_PR.CNOT, MethodObj)

/* Notify OS to re-read CPU tables */
Method (PNOT)
{
	\_PR.CNOT (0x81)
}

/* Notify OS to re-read CPU _PPC limit */
Method (PPCN)
{
	\_PR.CNOT (0x80)
}

/* Notify OS to re-read Throttle Limit tables */
Method (TNOT)
{
	\_PR.CNOT (0x82)
}
