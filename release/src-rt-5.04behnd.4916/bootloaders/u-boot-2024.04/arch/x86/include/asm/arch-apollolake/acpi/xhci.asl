/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Intel Corporation.
 */

/* XHCI Controller 0:15.0 */
Device (XHCI) {
	Name (_ADR, 0x00150000)  /* Device 21, Function 0 */

	Name (_S3D, 3)  /* D3 supported in S3 */
	Name (_S0W, 3)  /* D3 can wake device in S0 */
	Name (_S3W, 3)  /* D3 can wake system from S3 */

	/* Declare XHCI GPE status and enable bits are bit 13 */
	Name (_PRW, Package() { GPE0A_XHCI_PME_STS, 3 })

	Method (_STA, 0)
	{
		Return (0xF)
	}

	Device (RHUB)
	{
		/* Root Hub */
		Name (_ADR, Zero)

#if IS_ENABLED(CONFIG_SOC_INTEL_GLK)
#include "xhci_glk_ports.asl"
#else
#include "xhci_apl_ports.asl"
#endif
	}
}
