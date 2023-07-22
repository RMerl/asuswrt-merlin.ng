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
#include <platform_support_63138.h>
#include <arm.h>
#include <arm32_macros.S>
#include <kernel/cache_helpers.h>
#include <asm.S>
#include <kernel/unwind.h>

/*
void spin_nanosec (uint32_t nanosec)
parameters:
   r0 contains the number of nanoseconds to spin
returns:
notes:
   This function is not exact for lower wait values. Note that if the core's frequrncy oscillates,
   this function is inexact. The default/expected A9 cores' frequency is 1000MHz, which maps to
   a one-to-one cycle-to-nanosec translation. That assumption symplifies this function's implementation.
*/
.arm

.section ".text"

FUNC spin_nanosec, :
UNWIND(	.fnstart)
    push {r2-r3}
    // save previous state
    MRC p15, 0, r3, c9, c12, 0
    // count each cycle, reset PMCCNTR, enable counters
    LDR r2, =(PMC_RESET_PMCCNTR || PMC_COUNTERS_ENABLE)
    // store current state
    MCR p15, 0, r2, c9, c12, 0
    // explicitly enable PMCCNTR. we should not need (PMCR.E should do it), but we do...
    MOV r2, #PMCCNTR_ENABLE
    MCR p15, 0, r2, c9, c12, 1

spin_more_nano:
    YIELD
    // read PMCCNTR
    MRC p15, 0, r1, c9, c13, 0
    CMP r0, r1
    BHI spin_more_nano

    // restore previous state
    MCR p15, 0, r3, c9, c12, 0

    pop {r2-r3}
    MOV pc, lr
UNWIND(	.fnend)
END_FUNC spin_nanosec

FUNC reset_plat_cpu, :
UNWIND(	.fnstart)
    push {r0-r1,lr}
    // disable caches and  MMU, invalidate caches.
#define SCTLR_D    (1 << 2)
    mrc p15, 0, r0, c1, c0, 0
    mov r1, #SCTLR_I
    orr r1, r1, #SCTLR_A
    orr r1, r1, #SCTLR_D
    orr r1, r1, #SCTLR_M
    bic r0, r0, r1
    mcr p15, 0, r0, c1, c0, 0
    isb

    // Invalidate TLB, ICache and Branch predictor
    mcr p15, 0, r0, c8, c7, 0
    mcr p15, 0, r0, c7, c5, 0
    mcr p15, 0, r0, c7, c5, 6
    mov r0, #DCACHE_OP_INV
    bl  dcache_op_level1
    pop {r0-r1,lr}
    bx lr
UNWIND(	.fnend)
END_FUNC reset_plat_cpu

FUNC plat_cpu_reset_early, :
UNWIND(	.fnstart)
    /* It seems OPTEE assumes SMP is enabled before it is loaded */
    // Enable SMP, FW
    mrc p15, 0, r8, c1, c0, 1
    orr r8, r8, #0x41
    mcr p15, 0, r8, c1, c0, 1
    /* Do not use ARM's default reset vector located at 0xFFFF0000 */
    read_sctlr r8
    bic r8, r8, #SCTLR_V
    write_sctlr r8

#define PLEAUR_EN   (1 << 0)
#define NSACR_PLE   (1 << 16)

    mov r8, #PLEAUR_EN
    mcr p15, 0, r8, c11, c1, 0

    /* ARM Support stated that RFR is not implemented on A9 and A15.
    SMP, PLE, CP10 and CP11. */
    mov r8, #NSACR_NS_SMP
    orr r8, r8, #NSACR_PLE
    orr r8, r8, #NSACR_CP11
    orr r8, r8, #NSACR_CP10
    mcr p15, 0, r8, c1, c1, 2
    bx lr
UNWIND(	.fnend)
END_FUNC plat_cpu_reset_early
