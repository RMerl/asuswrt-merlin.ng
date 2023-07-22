#include <arm32_macros.S>
#include <arm.h>
//#include <asm-defines.h>
#include <asm.S>
#include <kernel/unwind.h>

 #if 0
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
#endif
        
    .macro read_cpsr reg
    mrs \reg, cpsr
    .endm

   .macro write_cpsr reg
   msr cpsr_fsxc, \reg
   .endm

   .macro invalidate_tlb reg
   /* Invalidate TLB */
   mov \reg, #0
   mcr p15, 0, \reg, c8, c5, 0
   mcr p15, 0, \reg, c8, c6, 0
   mcr p15, 0, \reg, c8, c7, 0
   isb
   dsb
   .endm


/*
 *
 *
 *
 *
 *
 */

LOCAL_FUNC save_optee_sysreg , :
    push {r1, r4,r5}

    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #0xff
    lsl r1, r1, #2

    ldr r5, =optee_stack
    add r5, r5, r1
    add r4, sp, #12
    str r4, [r5]

    read_sctlr r4
    bic r4, r4, #(SCTLR_M|SCTLR_C)
    ldr r5, =optee_sctlr
    add r5, r5, r1
    str r4, [r5]
    read_cpsr r4
    ldr r5, =optee_cpsr
    add r5, r5, r1
    str r4, [r5]

    read_scr r4
    ldr r5, =optee_scr
    add r5, r5, r1
    str r4, [r5]
    read_vbar r4
    ldr r5, =optee_vbar
    add r5, r5, r1
    str r4, [r5]
    read_mair0 r4
    ldr r5, =optee_mair0
    add r5, r5, r1
    str r4, [r5]
    read_mair1 r4
    ldr r5, =optee_mair1
    add r5, r5, r1
    str r4, [r5]
    read_dacr r4
    ldr r5, =optee_dacr
    add r5, r5, r1
    str r4, [r5]
    read_ttbcr r4
    ldr r5, =optee_ttbcr
    add r5, r5, r1
    str r4, [r5]
    read_mvbar r4
    ldr r5, =optee_mvbar
    add r5, r5, r1
    str r4, [r5]
    read_ttbr0 r4
    ldr r5, =optee_ttbr0
    add r5, r5, r1
    str r4, [r5]
    read_ttbr1 r4
    ldr r5, =optee_ttbr1
    add r5, r5, r1
    str r4, [r5]
    pop {r1, r4,r5}
    bx lr
END_FUNC save_optee_sysreg

/*
 *
 *
 *
 *
 *
 */
LOCAL_FUNC restore_optee_sysreg , :
    mrc p15, 0, r8, c0, c0, 5
    and r8, r8, #0xff
    lsl r8, r8, #2

    ldr r9, =optee_sctlr
    add r9, r9, r8
    ldr r11, [r9]
    write_sctlr r11
    ldr r9, =optee_cpsr
    add r9, r9, r8
    ldr r11, [r9]
    write_cpsr r11
    ldr r9, =optee_scr
    add r9, r9, r8
    ldr r11, [r9]
    write_scr r11
    ldr r9, =optee_vbar
    add r9, r9, r8
    ldr r11, [r9]
    write_vbar r11
    ldr r9, =optee_mair0
    add r9, r9, r8
    ldr r11, [r9]
    write_mair0 r11
    ldr r9, =optee_mair1
    add r9, r9, r8
    ldr r11, [r9]
    write_mair1 r11
    ldr r9, =optee_dacr
    add r9, r9, r8
    ldr r11, [r9]
    write_dacr r11
    ldr r9, =optee_ttbcr
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbcr r11
    ldr r9, =optee_mvbar
    add r9, r9, r8
    ldr r11, [r9]
    write_mvbar r11
    ldr r9, =optee_ttbr0
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbr0 r11
    ldr r9, =optee_ttbr1
    add r9, r9, r8
    ldr r11, [r9]
    write_ttbr1 r11

    invalidate_tlb r11
    bx lr
END_FUNC restore_optee_sysreg

/*
 * Entry into OPTEE
 *
 */
LOCAL_FUNC optee_smc_entry , :
UNWIND(.fnstart)
UNWIND(.cantunwind)
    mov r12, r14                /* Save link register */
    cps #CPSR_MODE_MON          /* Switch to monitor mode */
    mrc p15, 0, r8, c0, c0, 5   /* Get stack pointer */
    and r8, r8, #0xff
    lsl r8, r8, #2

    ldr r9, =optee_stack
    add r9, r9, r8
    ldr sp, [r9]

    bl restore_optee_sysreg           /* Get system regsiters */

    read_sctlr r9
    orr r9, r9, #(SCTLR_M | SCTLR_C)  /* Turn on MMU / Cache before working on OPTEE  */
    write_sctlr r9

    read_scr r9
    orr r9, r9, #(SCR_NS)             /* Switch to non-secure before working on OPTEE */
    write_scr r9
    nop
    mov r14, r12                      /* Restore link register */
    b sm_smc_entry
UNWIND(.fnend)
END_FUNC optee_smc_entry

/*
 * Exit from OPTEE
 *
 */
FUNC optee_exit , :
UNWIND(.fnstart)
UNWIND(.cantunwind)
    pop {r0-r9}                    /* Pop up 7 + 2 (rfefd sp!) registers */
    read_scr r9
    bic r9, r9, #(SCR_NS)          /* Switch to secure before jumping to ATF */
    write_scr r9
    bl save_optee_sysreg
    read_sctlr r9
    bic r9, r9, #(SCTLR_M|SCTLR_C) /* Turn off MMU/Cache before jumping to ATF */
    write_sctlr r9
    bl save_optee_sysreg
    ldr r12,=optee_smc_entry       /* Pass OPTEE's smc entry point */
    blx r8                         /* Jump to ATF */
    b .
    nop
UNWIND(.fnend)
END_FUNC optee_exit


.section .data
.balign 8
optee_cpsr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_sctlr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_stack:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234

optee_scr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_vbar:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_mair0:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_mair1:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_dacr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_ttbcr:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_mvbar:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_ttbr0:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
optee_ttbr1:
.word 0x1234
.word 0x1234
.word 0x1234
.word 0x1234
