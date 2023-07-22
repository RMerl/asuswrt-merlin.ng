/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#if defined(SPD_opteed)
#include <asm_macros.S>

.global save_atf_sysreg
.global restore_atf_sysreg

    .macro read_sctlr reg
    mrc p15, 0, \reg, c1, c0, 0
    .endm

    .macro write_sctlr reg
    mcr p15, 0, \reg, c1, c0, 0
    .endm

    .macro read_scr reg
    mrc p15, 0, \reg, c1, c1, 0
    .endm

    .macro write_scr reg
    mcr p15, 0, \reg, c1, c1, 0
    .endm

    .macro write_ttbr0 reg
    mcr p15, 0, \reg, c2, c0, 0
    .endm

    .macro read_ttbr0 reg
    mrc p15, 0, \reg, c2, c0, 0
    .endm

    .macro write_ttbr1 reg
    mcr p15, 0, \reg, c2, c0, 1
    .endm

    .macro read_ttbr1 reg
    mrc p15, 0, \reg, c2, c0, 1
    .endm

    .macro write_ttbcr reg
    mcr p15, 0, \reg, c2, c0, 2
    .endm

    .macro read_ttbcr reg
    mrc p15, 0, \reg, c2, c0, 2
    .endm

    .macro write_dacr reg
    mcr p15, 0, \reg, c3, c0, 0
    .endm

    .macro read_dacr reg
    mrc p15, 0, \reg, c3, c0, 0
    .endm

    .macro read_vbar reg
    mrc p15, 0, \reg, c12, c0, 0
    .endm

    .macro write_vbar reg
    mcr p15, 0, \reg, c12, c0, 0
    .endm

    .macro write_mvbar reg
    mcr p15, 0, \reg, c12, c0, 1
    .endm

    .macro read_mvbar reg
    mrc p15, 0, \reg, c12, c0, 1
    .endm

    .macro write_mair0 reg
    mcr p15, 0, \reg, c10, c2, 0
    .endm

    .macro read_mair0 reg
    mrc p15, 0, \reg, c10, c2, 0
    .endm

    .macro write_mair1 reg
    mcr p15, 0, \reg, c10, c2, 1
    .endm

    .macro read_mair1 reg
    mrc p15, 0, \reg, c10, c2, 1
    .endm

    .macro read_cpsr reg
    mrs \reg, cpsr
    .endm

    .macro write_cpsr reg
    msr cpsr_fsxc, \reg
    .endm

   .macro invalidate_tlb reg
   mov \reg, #0
   mcr p15, 0, \reg, c8, c5, 0
   mcr p15, 0, \reg, c8, c6, 0
   mcr p15, 0, \reg, c8, c7, 0
   isb
   dsb
   .endm

/*
 *
 */
func save_atf_sysreg
    push {r1, r4,r5}

    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #0xff
    lsl r1, r1, #2

    read_sctlr r4
    bic r4, r4, #0x5
    ldr r5, =atf_sctlr
    add r5, r5, r1
    str r4, [r5]
    read_cpsr r4
    ldr r5, =atf_cpsr
    add r5, r5, r1
    str r4, [r5]

    read_scr r4
    ldr r5, =atf_scr
    add r5, r5, r1
    str r4, [r5]
    read_vbar r4
    ldr r5, =atf_vbar
    add r5, r5, r1
    str r4, [r5]
    read_mair0 r4
    ldr r5, =atf_mair0
    add r5, r5, r1
    str r4, [r5]
    read_mair1 r4
    ldr r5, =atf_mair1
    add r5, r5, r1
    str r4, [r5]
    read_dacr r4
    ldr r5, =atf_dacr
    add r5, r5, r1
    str r4, [r5]
    read_ttbcr r4
    ldr r5, =atf_ttbcr
    add r5, r5, r1
    str r4, [r5]
    read_mvbar r4
    ldr r5, =atf_mvbar
    add r5, r5, r1
    str r4, [r5]
    read_ttbr0 r4
    ldr r5, =atf_ttbr0
    add r5, r5, r1
    str r4, [r5]
    read_ttbr1 r4
    ldr r5, =atf_ttbr1
    add r5, r5, r1
    str r4, [r5]
    isb
    dsb
    pop {r1, r4,r5}
    bx lr
endfunc save_atf_sysreg


/*
 *
 */
func restore_atf_sysreg
    mrc p15, 0, r8, c0, c0, 5
    and r8, r8, #0xff
    lsl r8, r8, #2

    ldr r9, =atf_sctlr
    add r9, r9, r8
    ldr r11, [r9]
    write_sctlr r11
    ldr r9, =atf_cpsr
    add r9, r9, r8
    ldr r11, [r9]
    write_cpsr r11
    ldr r9, =atf_scr
    add r9, r9, r8
    ldr r11, [r9]
    write_scr r11
    ldr r9, =atf_vbar
    add r9, r9, r8
    ldr r11, [r9]
    write_vbar r11
    ldr r9, =atf_mair0
    add r9, r9, r8
    ldr r11, [r9]
    write_mair0 r11
    ldr r9, =atf_mair1
    add r9, r9, r8
    ldr r11, [r9]
    write_mair1 r11
    ldr r9, =atf_dacr
    add r9, r9, r8
    ldr r11, [r9]
    write_dacr r11
    ldr r9, =atf_ttbcr
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbcr r11
    ldr r9, =atf_mvbar
    add r9, r9, r8
    ldr r11, [r9]
    write_mvbar r11
    ldr r9, =atf_ttbr0
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbr0 r11
    ldr r9, =atf_ttbr1
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbr1 r11

    invalidate_tlb r11
    bx lr
endfunc restore_atf_sysreg


.section .data
.align 8
atf_cpsr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_sctlr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_scr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_vbar:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_mair0:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_mair1:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_dacr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_ttbcr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_mvbar:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_ttbr0:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
atf_ttbr1:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234

#endif
