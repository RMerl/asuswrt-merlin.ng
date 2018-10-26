/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
    /* *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  ARMv8 system registers definitions   File: armv8.h
    *  Some definition based on ARM bare-metal example
    * 
    *
    *********************************************************************  */

#ifndef	_ARMV8_H_
#define _ARMV8_H_

#include "lib_types.h"
#include "lib_physio.h"
#include "cpu_config.h"

/* ARM defines */

#ifdef	__ASSEMBLER__

/*
 * LEAF - declare leaf routine
 */
#define LEAF(function)				\
	.section	.text.function, "ax";	\
	.global	function;		\
	.func		function;		\
function:

#define THUMBLEAF(function)			\
	.section	.text.function, "ax";	\
	.global	function;		\
	.func		function;		\
	.thumb;				\
	.thumb_func;				\
function:

/*
 * END - mark end of function
 */
#define END(function)				\
	.ltorg;				\
	.endfunc;				\
	.size		function, . - function

#define DW(var, val)				\
	.global	var;			\
	.type	var, %object;			\
	.size	var, 4;			\
	.align	2;				\
var:	.word	val

#define DDW(var, val)				\
	.global	var;			\
	.type	var, %object;			\
	.size	var, 8;			\
	.align	3;				\
var:	.dword	val


#define _ULCAST_

#else

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define _ULCAST_ (unsigned long)

#endif	/* __ASSEMBLER__ */

/* AArch64 SPSR definition */
#define AARCH64_SPSR_SP_SHIFT	0
#define AARCH64_SPSR_SP_MASK	(0x1<<AARCH64_SPSR_SP_SHIFT)
#define AARCH64_SPSR_EL_SHIFT	2
#define AARCH64_SPSR_EL_MASK	(0x3<<AARCH64_SPSR_EL_SHIFT)
#define AARCH64_SPSR_EL3h	13
#define AARCH64_SPSR_EL3t	12
#define AARCH64_SPSR_EL2h	9
#define AARCH64_SPSR_EL2t	8
#define AARCH64_SPSR_EL1h	5
#define AARCH64_SPSR_EL1t	4
#define AARCH64_SPSR_EL0t	0
#define AARCH64_SPSR_RW_SHIFT	4
#define AARCH64_SPSR_RW		(1 << AARCH64_SPSR_RW_SHIFT)
#define AARCH64_SPSR_F_SHIFT	6
#define AARCH64_SPSR_F		(1 << AARCH64_SPSR_F_SHIFT)
#define AARCH64_SPSR_I_SHIFT	7
#define AARCH64_SPSR_I		(1 << AARCH64_SPSR_I_SHIFT)
#define AARCH64_SPSR_A_SHIFT	8
#define AARCH64_SPSR_A		(1 << AARCH64_SPSR_A_SHIFT)
#define AARCH64_SPSR_D_SHIFT	9
#define AARCH64_SPSR_D		(1 << AARCH64_SPSR_D_SHIFT)
#define AARCH64_SPSR_IL_SHIFT	20
#define AARCH64_SPSR_IL		(1 << AARCH64_SPSR_IL_SHIFT)
#define AARCH64_SPSR_SS_SHIFT	21
#define AARCH64_SPSR_SS		(1 << AARCH64_SPSR_SS_SHIFT)
#define AARCH64_SPSR_V_SHIFT	28
#define AARCH64_SPSR_V		(1 << AARCH64_SPSR_V_SHIFT)
#define AARCH64_SPSR_C_SHIFT	29
#define AARCH64_SPSR_C		(1 << AARCH64_SPSR_C_SHIFT)
#define AARCH64_SPSR_Z_SHIFT	30
#define AARCH64_SPSR_Z		(1 << AARCH64_SPSR_Z_SHIFT)
#define AARCH64_SPSR_N_SHIFT	31
#define AARCH64_SPSR_N		(1 << AARCH64_SPSR_N_SHIFT)

/* AArch32 SPSR definition */

#define AARCH32_SPSR_M_USER	0
#define AARCH32_SPSR_M_FIQ	1
#define AARCH32_SPSR_M_IRQ	2
#define AARCH32_SPSR_M_SVC	3
#define AARCH32_SPSR_M_ABRT	7
#define AARCH32_SPSR_M_HYP	10
#define AARCH32_SPSR_M_UND	11
#define AARCH32_SPSR_M_SYS	15
#define AARCH32_SPSR_M		(1 << 4)
#define AARCH32_SPSR_T_T32	(1 << 5)
#define AARCH32_SPSR_F		(1 << 6)
#define AARCH32_SPSR_I		(1 << 7)
#define AARCH32_SPSR_A		(1 << 8)
#define AARCH32_SPSR_E		(1 << 9)
#define AARCH32_SPSR_IL		(1 << 20)
#define AARCH32_SPSR_SS		(1 << 21)
#define AARCH32_SPSR_V		(1 << 28)
#define AARCH32_SPSR_C		(1 << 29)
#define AARCH32_SPSR_Z		(1 << 30)
#define AARCH32_SPSR_N		(1 << 31)

/* System Control Register*/

#define SCTLR_EL1_UCI		(1 << 26)
#define SCTLR_ELx_EE		(1 << 25)
#define SCTLR_EL1_E0E		(1 << 24)
#define SCTLR_ELx_WXN		(1 << 19)
#define SCTLR_EL1_nTWE		(1 << 18)
#define SCTLR_EL1_nTWI		(1 << 16)
#define SCTLR_EL1_UCT		(1 << 15)
#define SCTLR_EL1_DZE		(1 << 14)
#define SCTLR_ELx_I		(1 << 12)
#define SCTLR_EL1_UMA		(1 << 9)
#define SCTLR_EL1_SED		(1 << 8)
#define SCTLR_EL1_ITD		(1 << 7)
#define SCTLR_EL1_THEE		(1 << 6)
#define SCTLR_EL1_CP15BEN	(1 << 5)
#define SCTLR_EL1_SA0		(1 << 4)
#define SCTLR_ELx_SA		(1 << 3)
#define SCTLR_ELx_C		(1 << 2)
#define SCTLR_ELx_A		(1 << 1)
#define SCTLR_ELx_M		(1 << 0)


/* Architectural Feature Access Control Register */

#define CPACR_EL1_TTA		(1 << 28)
#define CPACR_FPEN		(0xf << 20)

/* Architectural Feature Trap Register */

#define CPTR_ELx_TCPAC		(1 << 31)
#define CPTR_ELx_TTA		(1 << 20)
#define CPTR_ELx_TFP		(1 << 10)

/* Secure Configuration Register */

#define SCR_EL3_TWE		(1 << 13)
#define SCR_EL3_TWI		(1 << 12)
#define SCR_EL3_ST		(1 << 11)
#define SCR_EL3_RW		(1 << 10)
#define SCR_EL3_SIF		(1 << 9)
#define SCR_EL3_HCE		(1 << 8)
#define SCR_EL3_SMD		(1 << 7)
#define SCR_EL3_EA		(1 << 3)
#define SCR_EL3_FIQ		(1 << 2)
#define SCR_EL3_IRQ		(1 << 1)
#define SCR_EL3_NS		(1 << 0)

/* Exception Syndrome Register */
#define ESR_EL3_EC_SHIFT	26
#define ESR_EL3_EC_MASK		(0x3f<<ESR_EL3_EC_SHIFT)
#define ESR_EL3_IL_SHIFT	25
#define ESR_EL3_IL_MASK		(0x1<<ESR_EL3_IL_SHIFT)
#define ESR_EL3_ISS_SHIFT	0
#define ESR_EL3_ISS_MASK	(0x1ffffff<<ESR_ISS_EC_SHIFT)


/* Hypervisor Configuration Register */

#define HCR_EL2_ID		(1 << 33)
#define HCR_EL2_CD		(1 << 32)
#define HCR_EL2_RW		(1 << 31)
#define HCR_EL2_TRVM		(1 << 30)
#define HCR_EL2_HVC		(1 << 29)
#define HCR_EL2_TDZ		(1 << 28)

/* Performance Monitors Control Register */
#define PMCR_EL0_LC		(1 << 6)
#define PMCR_EL0_DP		(1 << 5)
#define PMCR_EL0_X		(1 << 4)
#define PMCR_EL0_D		(1 << 3)
#define PMCR_EL0_C		(1 << 2)
#define PMCR_EL0_P		(1 << 1)
#define PMCR_EL0_E		(1 << 0)

/* Performance Monitors Count Enable Set Register */
#define PMCNTENSET_EL0_C 	(1 << 31)

/* Interrupt Mask Bits Register */
#define DAIF_D			(1 << 3)
#define DAIF_A			(1 << 2)
#define DAIF_I			(1 << 1)
#define DAIF_F			(1 << 0)

/* MMU related definition */

/* Translation Control Register fields */

/* cacheability attribute */
#define TCR_RGN_NC		0
#define TCR_RGN_WBWA		1
#define TCR_RGN_WT		2
#define TCR_RGN_WBRA		3

/* Shareability encodings */
#define TCR_SHARE_NONE		0
#define TCR_SHARE_OUTER		2
#define TCR_SHARE_INNER		3

/* Granule size encodings */
#define TCR_GRANULE_4K		0
#define TCR_GRANULE_64K		1
#define TCR_GRANULE_16K		2

/* Physical Address sizes */
#define TCR_SIZE_4G		0
#define TCR_SIZE_64G		1
#define TCR_SIZE_1T		2
#define TCR_SIZE_4T		3
#define TCR_SIZE_16T		4
#define TCR_SIZE_256T		5

#define TCR_ELx_T0SZ_SHIFT	0
#define TCR_ELx_IRGN0_SHIFT	8
#define TCR_ELx_ORGN0_SHIFT	10
#define TCR_ELx_SH0_SHIFT	12
#define TCR_ELx_TG0_SHIFT	14

#define TCR_EL3_PS_SHIFT	16

#define TCR_EL1_EPD0		(1 << 7)
#define TCR_EL1_T1SZ_SHIFT	16
#define TCR_EL1_A1		(1 << 22)
#define TCR_EL1_EPD1		(1 << 23)
#define TCR_EL1_IRGN1_SHIFT	24
#define TCR_EL1_ORGN1_SHIFT	26
#define TCR_EL1_SH1_SHIFT	28
#define TCR_EL1_TG1_SHIFT	30
#define TCR_EL1_IPS_SHIFT	32
#define TCR_EL1_AS		(1 << 36)
#define TCR_EL1_TBI0		(1 << 37)
#define TCR_EL1_TBI1		(1 << 38)


/* Stage 1 Translation Table descriptor fields */

#define TT_S1_ATTR_FAULT	0
#define TT_S1_ATTR_BLOCK	1 // Level 1/2
#define TT_S1_ATTR_TABLE	3 // Level 0/1/2
#define TT_S1_ATTR_PAGE		3 // Level 3
#define TT_S1_ATTR_MASK		3 // Level 3

#define TT_S1_ATTR_MATTR_LSB	2

#define TT_S1_ATTR_NS		(1 << 5)

#define TT_S1_ATTR_AP_RW_EL1	(0 << 6)
#define TT_S1_ATTR_AP_RW_ANY	(1 << 6)
#define TT_S1_ATTR_AP_RO_EL1	(2 << 6)
#define TT_S1_ATTR_AP_RO_ANY	(3 << 6)

#define TT_S1_ATTR_AP_RW_EL3	(0 << 7)
#define TT_S1_ATTR_AP_RO_EL3	(1 << 7)

#define TT_S1_ATTR_SH_NONE	(0 << 8)
#define TT_S1_ATTR_SH_OUTER	(2 << 8)
#define TT_S1_ATTR_SH_INNER	(3 << 8)

#define TT_S1_ATTR_AF		(1 << 10)
#define TT_S1_ATTR_nG		(1 << 11)

#define TT_S1_ATTR_CONTIG	(1 << 52)
#define TT_S1_ATTR_PXN_EL1	(1 << 53)
#define TT_S1_ATTR_UXN_EL1	(1 << 54)
#define TT_S1_ATTR_XN_EL3	(1 << 54)

#define TT_S1_MAIR_DEV_nGnRnE	0
#define TT_S1_MAIR_DEV_nGnRE	4
#define TT_S1_MAIR_DEV_nGRE	8
#define TT_S1_MAIR_DEV_GRE	12

/* Inner and Outer Normal memory attributes use the same bit patterns
   Outer attributes just need to be shifted up */

#define TT_S1_MAIR_OUTER_SHIFT	4

#define TT_S1_MAIR_WT_TRANS_RA	2

#define TT_S1_MAIR_WB_TRANS_RA	6
#define TT_S1_MAIR_WB_TRANS_RWA 7

#define TT_S1_MAIR_WT_RA	10

#define TT_S1_MAIR_WB_RA	14
#define TT_S1_MAIR_WB_RWA	15


/* virutal address property flags */
#ifdef	__ASSEMBLER__
#define VA_FLAGS_CACHE_MEM	(0x0<<CPUCFG_VA_NONCACHE_BIT)
#define VA_FLAGS_NONCACHE_MEM	(0x1<<CPUCFG_VA_NONCACHE_BIT)
#define VA_FLAGS_CACHE_MEM_MASK	(0x1<<CPUCFG_VA_NONCACHE_BIT)
#else
#define VA_FLAGS_CACHE_MEM	(0x0UL<<CPUCFG_VA_NONCACHE_BIT)
#define VA_FLAGS_NONCACHE_MEM	(0x1UL<<CPUCFG_VA_NONCACHE_BIT)
#define VA_FLAGS_CACHE_MEM_MASK	(0x1UL<<CPUCFG_VA_NONCACHE_BIT)

extern uintptr_t armv8_mmu_pa2va(physaddr_t pa, uint64_t flag, uint64_t mask);
extern physaddr_t armv8_mmu_va2pa(uintptr_t va, uint64_t* pa_size);

/* address mappping macros */
#define cache_to_uncache(va) ((uintptr_t)(va) | (uintptr_t)VA_FLAGS_NONCACHE_MEM)
#define uncache_to_cache(va) ((uintptr_t)(va) & ~((uintptr_t)VA_FLAGS_NONCACHE_MEM))

/* There is no k0 and K1 conception in arm. Definition them here for code compatiblity only*/
#define PHYS_TO_K0(pa)	(armv8_mmu_pa2va(pa,VA_FLAGS_CACHE_MEM,VA_FLAGS_CACHE_MEM_MASK))
#define PHYS_TO_K1(pa)	(armv8_mmu_pa2va(pa,VA_FLAGS_NONCACHE_MEM,VA_FLAGS_CACHE_MEM_MASK))
#define K0_TO_PHYS(va)  (armv8_mmu_va2pa(va, NULL))
#define K1_TO_PHYS(va)	(armv8_mmu_va2pa(va, NULL))
#define K0_TO_K1(va)	cache_to_uncache((va))
#define K1_TO_K0(va)	uncache_to_cache((va))

/* Convert VA to PA and return the maximum continuous PA memory size from the VA */
#define VA_TO_PHYS_SIZE(va, ptr_size)	(armv8_mmu_va2pa(va, ptr_size))

#endif

#endif	/* _ARMV8_H_ */
