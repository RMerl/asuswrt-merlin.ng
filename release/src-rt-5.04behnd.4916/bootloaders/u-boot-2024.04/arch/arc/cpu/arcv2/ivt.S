/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 */

.section .ivt, "a",@progbits
.align 4
	/* Critical system events */
.word	_start			/* 0x00 - Reset */
.word	memory_error		/* 0x01 - Memory Error */
.word	instruction_error	/* 0x02 - Instruction Error */

	/* Exceptions */
.word	EV_MachineCheck		/* 0x03 - Fatal Machine check */
.word	EV_TLBMissI		/* 0x04 - Intruction TLB miss */
.word	EV_TLBMissD		/* 0x05 - Data TLB miss */
.word	EV_TLBProtV		/* 0x06 - Protection Violation or Misaligned Access */
.word	EV_PrivilegeV		/* 0x07 - Privilege Violation */
.word	EV_SWI			/* 0x08 - Software Interrupt */
.word	EV_Trap			/* 0x09 - Trap */
.word	EV_Extension		/* 0x0A - Extension Intruction Exception */
.word	EV_DivZero		/* 0x0B - Division by Zero */
.word	EV_DCError		/* 0x0C - Data cache consistency error */
.word	EV_Maligned		/* 0x0D - Misaligned data access */
.word	0			/* 0x0E - Unused */
.word	0			/* 0x0F - Unused */

	/* Device interrupts */
.rept	240
.word	interrupt_handler	/* 0x10 - 0xFF */
.endr
