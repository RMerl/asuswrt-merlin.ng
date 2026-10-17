/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

.section .ivt, "ax",@progbits
.align 4
_ivt:
	/* Critical system events */
	j	_start			/* 0 - 0x000 */
	j	memory_error		/* 1 - 0x008 */
	j	instruction_error	/* 2 - 0x010 */

	/* Device interrupts */
.rept	29
	j	interrupt_handler	/* 3:31 - 0x018:0xF8 */
.endr
	/* Exceptions */
	j	EV_MachineCheck		/* 0x100, Fatal Machine check  (0x20) */
	j	EV_TLBMissI		/* 0x108, Intruction TLB miss  (0x21) */
	j	EV_TLBMissD		/* 0x110, Data TLB miss        (0x22) */
	j	EV_TLBProtV		/* 0x118, Protection Violation (0x23)
							or Misaligned Access  */
	j	EV_PrivilegeV		/* 0x120, Privilege Violation  (0x24) */
	j	EV_Trap			/* 0x128, Trap exception       (0x25) */
	j	EV_Extension		/* 0x130, Extn Intruction Excp (0x26) */
