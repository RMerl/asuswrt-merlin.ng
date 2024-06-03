/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MIPS Coherence Manager (CM) Initialisation
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 */

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/cm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>

LEAF(mips_cm_map)
	/* Config3 must exist for a CM to be present */
	mfc0		t0, CP0_CONFIG, 1
	bgez		t0, 2f
	mfc0		t0, CP0_CONFIG, 2
	bgez		t0, 2f

	/* Check Config3.CMGCR to determine CM presence */
	mfc0		t0, CP0_CONFIG, 3
	and		t0, t0, MIPS_CONF3_CMGCR
	beqz		t0, 2f

	/* Find the current physical GCR base address */
1:	MFC0		t0, CP0_CMGCRBASE
	PTR_SLL		t0, t0, 4

	/* If the GCRs are where we want, we're done */
	PTR_LI		t1, CONFIG_MIPS_CM_BASE
	beq		t0, t1, 2f

	/* Move the GCRs to our configured base address */
	PTR_LI		t2, CKSEG1
	PTR_ADDU	t0, t0, t2
	sw		zero, GCR_BASE_UPPER(t0)
	sw		t1, GCR_BASE(t0)

	/* Re-check the GCR base */
	b		1b

2:	jr		ra
	END(mips_cm_map)
