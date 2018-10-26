/*  *********************************************************************
    *
    <:copyright-BRCM:2015:proprietary:standard
    
       Copyright (c) 2015 Broadcom 
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
    ********************************************************************* */

#include <armv8.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "bcm_map.h"

/*  *********************************************************************
    *  armv8_cpuinit(void)
    *
    *  perform fundemetnal CPU initialization
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    ********************************************************************* */

FUNC(armv8_cpuinit)
	
	/* This should be default but update anyway
	   disable FIQ and IRQ exception, enable data abort exception */
	msr	DAIFSet, #(DAIF_I|DAIF_F)
	msr	DAIFClr, #(DAIF_A)
	
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
	ldr	x0, =50000000
	msr	CNTFRQ_EL0, x0

	/* set smp enable in the extended control register */
	mrs	x0, S3_1_c15_c2_1
	orr	x0, x0, #(1 << 6)
	msr	S3_1_c15_c2_1, x0

 #if defined(CORTEXA)
	//
	// Configure ACTLR_EL[23]
	// ----------------------
	//
	// These bits are IMPLEMENTATION DEFINED, so are different for
	// different processors
	//
	// For Cortex-A57, the controls we set are:
	//
	//  Enable lower level access to CPUACTLR_EL1
	//  Enable lower level access to CPUECTLR_EL1
	//  Enable lower level access to L2CTLR_EL1
	//  Enable lower level access to L2ECTLR_EL1
	//  Enable lower level access to L2ACTLR_EL1
	//
	mov x0, #((1 << 0) | \
			  (1 << 1) | \
			  (1 << 4) | \
			  (1 << 5) | \
			  (1 << 6))

	msr ACTLR_EL3, x0
	msr ACTLR_EL2, x0
#endif

	ic	ialluis			/* Invalidate icache */

	/* Initialize system control register, disable i-cache, disable d-cache
	   disable mmu, strict stack and aligment */
    mov x0, #(SCTLR_ELx_A|SCTLR_ELx_SA)
	msr	SCTLR_EL3, x0

	isb

	ret
END(armv8_cpuinit)

/*  *********************************************************************
    *  armv8_enable_tick(void)
    *
    *  Enble arm cycle count through PMNC
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	  x1
    ********************************************************************* */
FUNC(armv8_enable_tick)
	ldr	x1,=PMCR_EL0_E
	msr	PMCR_EL0, x1
	ldr	x1,=PMCNTENSET_EL0_C
	msr	PMCNTENSET_EL0, x1

	ret
END(armv8_enable_tick)

/*  *********************************************************************
    *  _getticks
    *
    *  function for returning a tick
    *
    *  Input parameters:
    *
    *  Return value:
    *     x0
    ********************************************************************* */
FUNC(_getticks)
	mrs	x0, PMCCNTR_EL0
	ret
END(_getticks)


/*  *********************************************************************
    *  armv8_drop_to_el2(void)
    *
    *  drop to exception level 2 Aarch64 mode. Secure/NonSecure depends
    *  on SCR_EL3 setting. NonSecure mode in our case.
    *
    *  Input parameters:
    *	   x0: el2 entry
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	  x0, x1
    ********************************************************************* */
FUNC(armv8_drop_to_el2)
	/* Save the jump address */
	mov	x1, x0
	GET_EXCEPTION_LEVEL
	cmp	x0, #EX_LEVEL_2
	beq	already_done
	/* Load the jump address */
	msr	ELR_EL3, x1
	mov	x1, #(AARCH64_SPSR_EL2h|AARCH64_SPSR_F|AARCH64_SPSR_I|AARCH64_SPSR_A)
	msr	SPSR_EL3, x1
	eret
already_done:
	ret
END(armv8_drop_to_el2)

/*  *********************************************************************
    *  armv8_drop_to_el1(void)
    *
    *  drop to exception level 1 Aarch64 mode. Secure/NonSecure depends
    *  on SCR_EL3 setting. NonSecure mode in our case.
    *
    *  Input parameters:
    *	   x0: el1 entry
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	  x0, x1
    ********************************************************************* */
FUNC(armv8_drop_to_el1)
	msr	ELR_EL3, x0
	mov	x1, #(AARCH64_SPSR_EL1h|AARCH64_SPSR_F|AARCH64_SPSR_I|AARCH64_SPSR_A)
	msr	SPSR_EL3, x1
	eret
END(armv8_drop_to_el1)
    
/*  *********************************************************************
    *  armv8_drop_to_el1_aarch32_svc(void)
    *
    *  drop to exception level 1, Aarch32, supervisor mode. 
    *
    *  Input parameters:
    *	   x0: el1 entry
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	  x0, x1
    ********************************************************************* */
FUNC(armv8_drop_to_el1_aarch32_svc)
	msr	ELR_EL3, x0
	mov	x1, #(AARCH32_SPSR_M_SVC|AARCH32_SPSR_M|AARCH32_SPSR_F|AARCH32_SPSR_I|AARCH32_SPSR_A)
	msr	SPSR_EL3, x1
	eret
END(armv8_drop_to_el1_aarch32_svc)


/*  *********************************************************************
    *  armv8_gic_secure_init(void)
    *
    *  .init gic for nonsecure os
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	  x0, x1, x2, x3
    ********************************************************************* */
FUNC(armv8_gic_secure_init)
	GET_EXCEPTION_LEVEL
	cmp	x0, #EX_LEVEL_3
	bne	gic_secure_init_done

	/* setup the GIC for non secure world. group 0 secure state, group 1 non secure state */
	/* enable interrupt for both groups */
	ldr	x0, =GICD_BASE
	mov	w1, #0x3
	str	w1, [x0, GICD_CTLR]

	/* assign all the interrupt to group 1 */
	ldr	w1, [x0, GICD_TYPER]
	and     w1, w1, #0x1f
	add	w1, w1, #1
	mov	w2, #0xffffffff
	add	x3, x0, GICD_IGROUPR0
setgroup:
	str	w2, [x3], #4
	sub	w1, w1, #1
	cbnz	w1, setgroup

	/* enable both group interrupt in the cpu interface */
	ldr	x0, =GICC_BASE
	ldr	w1, [x0, GICC_CTLR]
	orr	w1, w1, #0x3
	str	w1, [x0, GICC_CTLR]

	/* set cpu priority mask view for nonsecure */
	mov	w1, #0x80
	str	w1, [x0, GICC_PMR]

gic_secure_init_done:
	ret
END(armv8_gic_secure_init)
