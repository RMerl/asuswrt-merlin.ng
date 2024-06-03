/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 */

#include <linux/linkage.h>

/*
 * Note on the LD/ST addressing modes with address register write-back
 *
 * LD.a same as LD.aw
 *
 * LD.a    reg1, [reg2, x]  => Pre Incr
 *      Eff Addr for load = [reg2 + x]
 *
 * LD.ab   reg1, [reg2, x]  => Post Incr
 *      Eff Addr for load = [reg2]
 */

.macro PUSH reg
	st.a	\reg, [%sp, -4]
.endm

.macro PUSHAX aux
	lr	%r9, [\aux]
	PUSH	%r9
.endm

.macro  SAVE_R1_TO_R24
	PUSH	%r1
	PUSH	%r2
	PUSH	%r3
	PUSH	%r4
	PUSH	%r5
	PUSH	%r6
	PUSH	%r7
	PUSH	%r8
	PUSH	%r9
	PUSH	%r10
	PUSH	%r11
	PUSH	%r12
	PUSH	%r13
	PUSH	%r14
	PUSH	%r15
	PUSH	%r16
	PUSH	%r17
	PUSH	%r18
	PUSH	%r19
	PUSH	%r20
	PUSH	%r21
	PUSH	%r22
	PUSH	%r23
	PUSH	%r24
.endm

.macro SAVE_ALL_SYS
	/* saving %r0 to reg->r0 in advance since we read %ecr into it */
	st	%r0, [%sp, -8]
	lr	%r0, [%ecr]	/* all stack addressing is manual so far */
	st	%r0, [%sp]
	st	%sp, [%sp, -4]
	/* now move %sp to reg->r0 position so we can do "push" automatically */
	sub	%sp, %sp, 8

	SAVE_R1_TO_R24
	PUSH	%r25
	PUSH	%gp
	PUSH	%fp
	PUSH	%blink
	PUSHAX	%eret
	PUSHAX	%erstatus
	PUSH	%lp_count
	PUSHAX	%lp_end
	PUSHAX	%lp_start
	PUSHAX	%erbta
.endm

.macro SAVE_EXCEPTION_SOURCE
#ifdef CONFIG_MMU
	/* If MMU exists exception faulting address is loaded in EFA reg */
	lr	%r0, [%efa]
#else
	/* Otherwise in ERET (exception return) reg */
	lr	%r0, [%eret]
#endif
.endm

ENTRY(memory_error)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_memory_error
ENDPROC(memory_error)

ENTRY(instruction_error)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_instruction_error
ENDPROC(instruction_error)

ENTRY(interrupt_handler)
	/* Todo - save and restore CPU context when interrupts will be in use */
	bl	do_interrupt_handler
	rtie
ENDPROC(interrupt_handler)

ENTRY(EV_MachineCheck)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_machine_check_fault
ENDPROC(EV_MachineCheck)

ENTRY(EV_TLBMissI)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_itlb_miss
ENDPROC(EV_TLBMissI)

ENTRY(EV_TLBMissD)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_dtlb_miss
ENDPROC(EV_TLBMissD)

ENTRY(EV_TLBProtV)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_tlb_prot_violation
ENDPROC(EV_TLBProtV)

ENTRY(EV_PrivilegeV)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_privilege_violation
ENDPROC(EV_PrivilegeV)

ENTRY(EV_Trap)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_trap
ENDPROC(EV_Trap)

ENTRY(EV_Extension)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_extension
ENDPROC(EV_Extension)

#ifdef CONFIG_ISA_ARCV2
ENTRY(EV_SWI)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_swi
ENDPROC(EV_SWI)

ENTRY(EV_DivZero)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_divzero
ENDPROC(EV_DivZero)

ENTRY(EV_DCError)
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_dcerror
ENDPROC(EV_DCError)

ENTRY(EV_Maligned)
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_maligned
ENDPROC(EV_Maligned)
#endif
