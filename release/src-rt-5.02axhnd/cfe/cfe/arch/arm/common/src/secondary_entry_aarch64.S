/*  *********************************************************************
    *
    <:copyright-BRCM:2018:proprietary:standard
    
       Copyright (c) 2018 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>
    ********************************************************************
*/
#include <armv8.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "bcm_map.h"

#if defined(CFG_RAMAPP)
/*  *********************************************************************
    *  secondary_startup
    *
    *  Eentry point for secondary CPUs. The code must be pic independent
    *  and all subroutine within ths block. This boot code will be relocated to
    *  a reserved, secure access only memory area for Linux to use to dynamically
    *  power on/off cores. So it must NOT do the following:
    *  - call a function into cfe ram code. The function must be embedded in
    *    this file
    *  - use any symbol directly such as ldr r0, =<sybmol>.  Use mov r0, #imm
    *    if possible. Otherwise, use movw and movt pair. If the symbol is in
    *    this file, use adrl instruction
    *
    *  This function copies most main cpu init sequence and inline the
    *  subroutines. CFE RAM start up the secondary cores and it parse the dtb
    *  to find out the linux kernel cpu release address as the jump target
    *  after wake up
    ********************************************************************* */
	.align 3
	.global sec_entry_begin	
sec_entry_begin:
	b	secondary_startup
	nop
	/* rel_addr must be 8 byte off from the entry_begin. No other code allowed.
	dtb init function set this address based on cpu_rel_addr value in the dtb
	must be the first entry */
rel_addr:
	.dword 0x0
cntfrq:
	.dword 0x02FAF080
#if defined(_BCM94908_)
cpubiu:
	.dword BIUCTRL_BASE
/* debugging only*/
#endif
prtfunc:
	.dword board_setleds
prtchar:
	.dword 0x43505530
sctlr:
	.dword 0x30C50830
gicdb:
	.dword GICD_BASE
giccb:
	.dword GICC_BASE
FUNC(secondary_startup)
#ifdef BCM_OPTEE
	/* Do nothing and wait for an valid entry address from OPTEE */
	mrs	x1, mpidr_el1
	and	x1, x1, #0xff
	mov	x0, #CFG_PEN_ADDRESS
	add	x0, x0, x1, LSL #3
	mov	x1, #0
	str	x1, [x0]
	wfe	/* wait for startup signal from OPTEE */
addr_loop:
	/* find out the cpu release address as specified by OPTEE */
	ldr	x4, [x0]
	cbz	x4, addr_loop
	br	x4
	b .
#endif /* BCM_OPTEE */
	/* copy of armv8_cpuinit */

	/* Initialize system control register, disable d-cache, i-cache
	   disable mmu, strict stack and aligment */
	mov	x0, #(SCTLR_ELx_A|SCTLR_ELx_SA)
	msr	SCTLR_EL3, x0
	isb

	/* set lower exception levels in the non-secure world, with no access
	   back to EL2 or EL3, and AArch64 capable */
	mov	x0, #(SCR_EL3_RW|SCR_EL3_SMD|SCR_EL3_NS)
	msr	SCR_EL3, x0

	/*  no traps or VM modifications from the Hypervisor, EL1 is AArch64 */
	mov	x0, #HCR_EL2_RW
	msr	HCR_EL2, x0

	/* VMID is still significant, even when virtualisation is not
	   being used, so ensure VTTBR_EL2 is properly initialised */
	msr	VTTBR_EL2, xzr

	/* neither EL3 nor EL2 trap floating point or accesses to CPACR */
	msr	 CPTR_EL3, xzr
	msr	 CPTR_EL2, xzr

	/* set SCTLRs for lower ELs to safe values */
	msr	SCTLR_EL1, xzr
	msr	SCTLR_EL2, xzr

	/* set the system freq to 50MHz */
	adr	x0, cntfrq
	ldr	x0, [x0]
	msr	CNTFRQ_EL0, x0

	/* set smp enable in the extended control register */
	mrs	x0, S3_1_c15_c2_1
	orr	x0, x0, #(1 << 6)
	msr	S3_1_c15_c2_1, x0

	/* enable PMNC counter */
	mov	x0, #PMCR_EL0_E
	msr	PMCR_EL0, x0
	mov	x0, #PMCNTENSET_EL0_C
	msr	PMCNTENSET_EL0, x0
	
	adr	x0, gicdb
	ldr	x0, [x0]
	mov	w1, #0x3
	str	w1, [x0, GICD_CTLR]

	/* assign all the interrupt to group 1 */
	ldr	w1, [x0, GICD_TYPER]
	and     w1, w1, #0x1f
	add	w1, w1, #1
	mov	w2, #0xffffffff
	add	x3, x0, GICD_IGROUPR0
1:
	str	w2, [x3], #4
	sub	w1, w1, #1
	cbnz	w1, 1b


	adr	x0, giccb
	ldr	x0, [x0]
	ldr	w1, [x0, GICC_CTLR]
	orr	w1, w1, #0x3
	str	w1, [x0, GICC_CTLR]

	/* set cpu priority mask view for nonsecure */
	mov	w1, #0x80
	str	w1, [x0, GICC_PMR]

#if defined(_BCM94908_) && defined(CONFIG_BRCM_IKOS)
	/* set up the faster clock for ikos */
	adr	x0, cpubiu
	ldr	x0, [x0]
	ldr	w1, [x0, #CPU_CLOCK_CONFIG]     // program SysIF clock to divide by 1
	and	w1, w1, #0xffffffe0
	str	w1, [x0, #CPU_CLOCK_CONFIG]
	str     wzr, [x0, #CPU_CLUSTER0_CLOCK_CONTROL]	// program SEL_CLK_PATTERN to divide by 1
#endif

#if 1
	/* debugging print out cpu # */
	mrs	x0, MPIDR_EL1
	ubfx	x0, x0, #0, #8
	adr	x1, prtfunc
	ldr	x1, [x1]
	adr	x2, prtchar
	ldr	x2, [x2]
	add	x0, x0, x2
	blr	x1
#endif

	/* drop to nonsecure EL2, the *rel_addr is in non-secure memory setup  by linux  */
	adr	x1, 1f
	msr	ELR_EL3, x1
	mov	x2, #(AARCH64_SPSR_EL2h|AARCH64_SPSR_F|AARCH64_SPSR_I|AARCH64_SPSR_A|AARCH64_SPSR_D)
	msr	SPSR_EL3, x2
	wfe	/* wait for linux startup, must be put before eret to avoid racing */
	eret
1:
	wfe	/* consume the event from eret */

	adr	x0, rel_addr
	ldr	x0, [x0]
2:
	/* find out the cpu release address - linux kernel secondary cpu entry ptr */
	ldr	x4, [x0]
	cbz	x4, 2b

	br	x4

END(secondary_startup)
	.global sec_entry_end
sec_entry_end:
#endif
