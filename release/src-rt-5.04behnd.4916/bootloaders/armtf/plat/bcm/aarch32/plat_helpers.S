#include <asm_macros.S>
#include <platform_def.h>
#include <cpu_macros.S>

	.globl	plat_is_my_cpu_primary
	.globl	plat_secondary_cold_boot_setup

#define NSACR_SMP		(1 << 18)
#define CORTEX_A15_ACTLR_INV_BTB_BIT	(U(1) << 0)

func cortex_a9_errata_report
	bx	lr
endfunc cortex_a9_errata_report

func cortex_a7_errata_report
	bx	lr
endfunc cortex_a7_errata_report

func cortex_a15_errata_report
	bx	lr
endfunc cortex_a15_errata_report

func plat_is_my_cpu_primary
	mov	r0, #1
	isb
	nop
	bx	lr
endfunc plat_is_my_cpu_primary

func plat_secondary_cold_boot_setup
	b .
	bx lr
endfunc plat_secondary_cold_boot_setup

func cortex_reset_func
#define SCTLR_M    (1 << 0)
#define SCTLR_A    (1 << 1)
#define SCTLR_D    (1 << 2)
#define SCTLR_I    (1 << 12)
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

	// set non-secure ACR. Allow SMP, L2ERR, CP10 and CP11 and Enable Neon/VFP bit for non-secure mode
	movw	r0, #0x0c00
	movt	r0, #0x0006
	mcr	p15, 0, r0, c1, c1, 2
	// Allow non-secure access to ACR
	ldr	r1,=NSACR_SMP
#if defined(PLATFORM_FLAVOR_63138)
	// Cortex A9 specific quirks
#define PLEAUR_EN   (1 << 0)
#define NSACR_PLE   (1 << 16)
	mov	r2, #PLEAUR_EN
	mcr	p15, 0, r2, c11, c1, 0
	orr	r1, r1, #NSACR_PLE
#endif
	ldcopr	r0, NSACR
	orr	r0, r0, r1
	stcopr	r0, NSACR

	mrc	p15, 0, r1, c1, c0, 1
#if defined(PLATFORM_FLAVOR_63138)
	/* Enable SMP and FW */
	orr	r1, r1, #0x41
#else
	/* Enable SMP */
	orr	r1, r1, #0x40
#endif
	mcr	p15, 0, r1, c1, c0, 1
	isb
#if defined(PLATFORM_FLAVOR_63148)
	ldcopr	r1, ACTLR
	orr	r1, #CORTEX_A15_ACTLR_INV_BTB_BIT
	stcopr	r1, ACTLR
#endif
	bx  lr
endfunc cortex_reset_func


func cortex_core_pwr_dwn
	bx	lr
endfunc cortex_core_pwr_dwn

func cortex_cluster_pwr_dwn
	bx	lr
endfunc cortex_cluster_pwr_dwn

#if defined(PLATFORM_FLAVOR_63138)
declare_cpu_ops cortex_a9, 0x4100C090, \
	cortex_reset_func, \
	cortex_core_pwr_dwn, \
	cortex_cluster_pwr_dwn
#elif defined(PLATFORM_FLAVOR_63148)
declare_cpu_ops cortex_a15, 0x420000F0, \
	cortex_reset_func, \
	cortex_core_pwr_dwn, \
	cortex_cluster_pwr_dwn
#else
declare_cpu_ops cortex_a7, 0x4100C070, \
	cortex_reset_func, \
	cortex_core_pwr_dwn, \
	cortex_cluster_pwr_dwn
#endif

