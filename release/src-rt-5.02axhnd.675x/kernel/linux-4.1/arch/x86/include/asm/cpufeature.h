/*
 * Defines x86 CPU feature bits
 */
#ifndef _ASM_X86_CPUFEATURE_H
#define _ASM_X86_CPUFEATURE_H

#ifndef _ASM_X86_REQUIRED_FEATURES_H
#include <asm/required-features.h>
#endif

#ifndef _ASM_X86_DISABLED_FEATURES_H
#include <asm/disabled-features.h>
#endif

#define NCAPINTS	13	/* N 32-bit words worth of info */
#define NBUGINTS	1	/* N 32-bit bug flags */

/*
 * Note: If the comment begins with a quoted string, that string is used
 * in /proc/cpuinfo instead of the macro name.  If the string is "",
 * this feature bit is not displayed in /proc/cpuinfo at all.
 */

/* Intel-defined CPU features, CPUID level 0x00000001 (edx), word 0 */
#define X86_FEATURE_FPU		( 0*32+ 0) /* Onboard FPU */
#define X86_FEATURE_VME		( 0*32+ 1) /* Virtual Mode Extensions */
#define X86_FEATURE_DE		( 0*32+ 2) /* Debugging Extensions */
#define X86_FEATURE_PSE		( 0*32+ 3) /* Page Size Extensions */
#define X86_FEATURE_TSC		( 0*32+ 4) /* Time Stamp Counter */
#define X86_FEATURE_MSR		( 0*32+ 5) /* Model-Specific Registers */
#define X86_FEATURE_PAE		( 0*32+ 6) /* Physical Address Extensions */
#define X86_FEATURE_MCE		( 0*32+ 7) /* Machine Check Exception */
#define X86_FEATURE_CX8		( 0*32+ 8) /* CMPXCHG8 instruction */
#define X86_FEATURE_APIC	( 0*32+ 9) /* Onboard APIC */
#define X86_FEATURE_SEP		( 0*32+11) /* SYSENTER/SYSEXIT */
#define X86_FEATURE_MTRR	( 0*32+12) /* Memory Type Range Registers */
#define X86_FEATURE_PGE		( 0*32+13) /* Page Global Enable */
#define X86_FEATURE_MCA		( 0*32+14) /* Machine Check Architecture */
#define X86_FEATURE_CMOV	( 0*32+15) /* CMOV instructions */
					  /* (plus FCMOVcc, FCOMI with FPU) */
#define X86_FEATURE_PAT		( 0*32+16) /* Page Attribute Table */
#define X86_FEATURE_PSE36	( 0*32+17) /* 36-bit PSEs */
#define X86_FEATURE_PN		( 0*32+18) /* Processor serial number */
#define X86_FEATURE_CLFLUSH	( 0*32+19) /* CLFLUSH instruction */
#define X86_FEATURE_DS		( 0*32+21) /* "dts" Debug Store */
#define X86_FEATURE_ACPI	( 0*32+22) /* ACPI via MSR */
#define X86_FEATURE_MMX		( 0*32+23) /* Multimedia Extensions */
#define X86_FEATURE_FXSR	( 0*32+24) /* FXSAVE/FXRSTOR, CR4.OSFXSR */
#define X86_FEATURE_XMM		( 0*32+25) /* "sse" */
#define X86_FEATURE_XMM2	( 0*32+26) /* "sse2" */
#define X86_FEATURE_SELFSNOOP	( 0*32+27) /* "ss" CPU self snoop */
#define X86_FEATURE_HT		( 0*32+28) /* Hyper-Threading */
#define X86_FEATURE_ACC		( 0*32+29) /* "tm" Automatic clock control */
#define X86_FEATURE_IA64	( 0*32+30) /* IA-64 processor */
#define X86_FEATURE_PBE		( 0*32+31) /* Pending Break Enable */

/* AMD-defined CPU features, CPUID level 0x80000001, word 1 */
/* Don't duplicate feature flags which are redundant with Intel! */
#define X86_FEATURE_SYSCALL	( 1*32+11) /* SYSCALL/SYSRET */
#define X86_FEATURE_MP		( 1*32+19) /* MP Capable. */
#define X86_FEATURE_NX		( 1*32+20) /* Execute Disable */
#define X86_FEATURE_MMXEXT	( 1*32+22) /* AMD MMX extensions */
#define X86_FEATURE_FXSR_OPT	( 1*32+25) /* FXSAVE/FXRSTOR optimizations */
#define X86_FEATURE_GBPAGES	( 1*32+26) /* "pdpe1gb" GB pages */
#define X86_FEATURE_RDTSCP	( 1*32+27) /* RDTSCP */
#define X86_FEATURE_LM		( 1*32+29) /* Long Mode (x86-64) */
#define X86_FEATURE_3DNOWEXT	( 1*32+30) /* AMD 3DNow! extensions */
#define X86_FEATURE_3DNOW	( 1*32+31) /* 3DNow! */

/* Transmeta-defined CPU features, CPUID level 0x80860001, word 2 */
#define X86_FEATURE_RECOVERY	( 2*32+ 0) /* CPU in recovery mode */
#define X86_FEATURE_LONGRUN	( 2*32+ 1) /* Longrun power control */
#define X86_FEATURE_LRTI	( 2*32+ 3) /* LongRun table interface */

/* Other features, Linux-defined mapping, word 3 */
/* This range is used for feature bits which conflict or are synthesized */
#define X86_FEATURE_CXMMX	( 3*32+ 0) /* Cyrix MMX extensions */
#define X86_FEATURE_K6_MTRR	( 3*32+ 1) /* AMD K6 nonstandard MTRRs */
#define X86_FEATURE_CYRIX_ARR	( 3*32+ 2) /* Cyrix ARRs (= MTRRs) */
#define X86_FEATURE_CENTAUR_MCR	( 3*32+ 3) /* Centaur MCRs (= MTRRs) */
/* cpu types for specific tunings: */
#define X86_FEATURE_K8		( 3*32+ 4) /* "" Opteron, Athlon64 */
#define X86_FEATURE_K7		( 3*32+ 5) /* "" Athlon */
#define X86_FEATURE_P3		( 3*32+ 6) /* "" P3 */
#define X86_FEATURE_P4		( 3*32+ 7) /* "" P4 */
#define X86_FEATURE_CONSTANT_TSC ( 3*32+ 8) /* TSC ticks at a constant rate */
#define X86_FEATURE_UP		( 3*32+ 9) /* smp kernel running on up */
/* free, was #define X86_FEATURE_FXSAVE_LEAK ( 3*32+10) * "" FXSAVE leaks FOP/FIP/FOP */
#define X86_FEATURE_ARCH_PERFMON ( 3*32+11) /* Intel Architectural PerfMon */
#define X86_FEATURE_PEBS	( 3*32+12) /* Precise-Event Based Sampling */
#define X86_FEATURE_BTS		( 3*32+13) /* Branch Trace Store */
#define X86_FEATURE_SYSCALL32	( 3*32+14) /* "" syscall in ia32 userspace */
#define X86_FEATURE_SYSENTER32	( 3*32+15) /* "" sysenter in ia32 userspace */
#define X86_FEATURE_REP_GOOD	( 3*32+16) /* rep microcode works well */
#define X86_FEATURE_MFENCE_RDTSC ( 3*32+17) /* "" Mfence synchronizes RDTSC */
#define X86_FEATURE_LFENCE_RDTSC ( 3*32+18) /* "" Lfence synchronizes RDTSC */
/* free, was #define X86_FEATURE_11AP	( 3*32+19) * "" Bad local APIC aka 11AP */
#define X86_FEATURE_NOPL	( 3*32+20) /* The NOPL (0F 1F) instructions */
#define X86_FEATURE_ALWAYS	( 3*32+21) /* "" Always-present feature */
#define X86_FEATURE_XTOPOLOGY	( 3*32+22) /* cpu topology enum extensions */
#define X86_FEATURE_TSC_RELIABLE ( 3*32+23) /* TSC is known to be reliable */
#define X86_FEATURE_NONSTOP_TSC	( 3*32+24) /* TSC does not stop in C states */
/* free, was #define X86_FEATURE_CLFLUSH_MONITOR ( 3*32+25) * "" clflush reqd with monitor */
#define X86_FEATURE_EXTD_APICID	( 3*32+26) /* has extended APICID (8 bits) */
#define X86_FEATURE_AMD_DCM     ( 3*32+27) /* multi-node processor */
#define X86_FEATURE_APERFMPERF	( 3*32+28) /* APERFMPERF */
#define X86_FEATURE_EAGER_FPU	( 3*32+29) /* "eagerfpu" Non lazy FPU restore */
#define X86_FEATURE_NONSTOP_TSC_S3 ( 3*32+30) /* TSC doesn't stop in S3 state */

/* Intel-defined CPU features, CPUID level 0x00000001 (ecx), word 4 */
#define X86_FEATURE_XMM3	( 4*32+ 0) /* "pni" SSE-3 */
#define X86_FEATURE_PCLMULQDQ	( 4*32+ 1) /* PCLMULQDQ instruction */
#define X86_FEATURE_DTES64	( 4*32+ 2) /* 64-bit Debug Store */
#define X86_FEATURE_MWAIT	( 4*32+ 3) /* "monitor" Monitor/Mwait support */
#define X86_FEATURE_DSCPL	( 4*32+ 4) /* "ds_cpl" CPL Qual. Debug Store */
#define X86_FEATURE_VMX		( 4*32+ 5) /* Hardware virtualization */
#define X86_FEATURE_SMX		( 4*32+ 6) /* Safer mode */
#define X86_FEATURE_EST		( 4*32+ 7) /* Enhanced SpeedStep */
#define X86_FEATURE_TM2		( 4*32+ 8) /* Thermal Monitor 2 */
#define X86_FEATURE_SSSE3	( 4*32+ 9) /* Supplemental SSE-3 */
#define X86_FEATURE_CID		( 4*32+10) /* Context ID */
#define X86_FEATURE_FMA		( 4*32+12) /* Fused multiply-add */
#define X86_FEATURE_CX16	( 4*32+13) /* CMPXCHG16B */
#define X86_FEATURE_XTPR	( 4*32+14) /* Send Task Priority Messages */
#define X86_FEATURE_PDCM	( 4*32+15) /* Performance Capabilities */
#define X86_FEATURE_PCID	( 4*32+17) /* Process Context Identifiers */
#define X86_FEATURE_DCA		( 4*32+18) /* Direct Cache Access */
#define X86_FEATURE_XMM4_1	( 4*32+19) /* "sse4_1" SSE-4.1 */
#define X86_FEATURE_XMM4_2	( 4*32+20) /* "sse4_2" SSE-4.2 */
#define X86_FEATURE_X2APIC	( 4*32+21) /* x2APIC */
#define X86_FEATURE_MOVBE	( 4*32+22) /* MOVBE instruction */
#define X86_FEATURE_POPCNT      ( 4*32+23) /* POPCNT instruction */
#define X86_FEATURE_TSC_DEADLINE_TIMER	( 4*32+24) /* Tsc deadline timer */
#define X86_FEATURE_AES		( 4*32+25) /* AES instructions */
#define X86_FEATURE_XSAVE	( 4*32+26) /* XSAVE/XRSTOR/XSETBV/XGETBV */
#define X86_FEATURE_OSXSAVE	( 4*32+27) /* "" XSAVE enabled in the OS */
#define X86_FEATURE_AVX		( 4*32+28) /* Advanced Vector Extensions */
#define X86_FEATURE_F16C	( 4*32+29) /* 16-bit fp conversions */
#define X86_FEATURE_RDRAND	( 4*32+30) /* The RDRAND instruction */
#define X86_FEATURE_HYPERVISOR	( 4*32+31) /* Running on a hypervisor */

/* VIA/Cyrix/Centaur-defined CPU features, CPUID level 0xC0000001, word 5 */
#define X86_FEATURE_XSTORE	( 5*32+ 2) /* "rng" RNG present (xstore) */
#define X86_FEATURE_XSTORE_EN	( 5*32+ 3) /* "rng_en" RNG enabled */
#define X86_FEATURE_XCRYPT	( 5*32+ 6) /* "ace" on-CPU crypto (xcrypt) */
#define X86_FEATURE_XCRYPT_EN	( 5*32+ 7) /* "ace_en" on-CPU crypto enabled */
#define X86_FEATURE_ACE2	( 5*32+ 8) /* Advanced Cryptography Engine v2 */
#define X86_FEATURE_ACE2_EN	( 5*32+ 9) /* ACE v2 enabled */
#define X86_FEATURE_PHE		( 5*32+10) /* PadLock Hash Engine */
#define X86_FEATURE_PHE_EN	( 5*32+11) /* PHE enabled */
#define X86_FEATURE_PMM		( 5*32+12) /* PadLock Montgomery Multiplier */
#define X86_FEATURE_PMM_EN	( 5*32+13) /* PMM enabled */

/* More extended AMD flags: CPUID level 0x80000001, ecx, word 6 */
#define X86_FEATURE_LAHF_LM	( 6*32+ 0) /* LAHF/SAHF in long mode */
#define X86_FEATURE_CMP_LEGACY	( 6*32+ 1) /* If yes HyperThreading not valid */
#define X86_FEATURE_SVM		( 6*32+ 2) /* Secure virtual machine */
#define X86_FEATURE_EXTAPIC	( 6*32+ 3) /* Extended APIC space */
#define X86_FEATURE_CR8_LEGACY	( 6*32+ 4) /* CR8 in 32-bit mode */
#define X86_FEATURE_ABM		( 6*32+ 5) /* Advanced bit manipulation */
#define X86_FEATURE_SSE4A	( 6*32+ 6) /* SSE-4A */
#define X86_FEATURE_MISALIGNSSE ( 6*32+ 7) /* Misaligned SSE mode */
#define X86_FEATURE_3DNOWPREFETCH ( 6*32+ 8) /* 3DNow prefetch instructions */
#define X86_FEATURE_OSVW	( 6*32+ 9) /* OS Visible Workaround */
#define X86_FEATURE_IBS		( 6*32+10) /* Instruction Based Sampling */
#define X86_FEATURE_XOP		( 6*32+11) /* extended AVX instructions */
#define X86_FEATURE_SKINIT	( 6*32+12) /* SKINIT/STGI instructions */
#define X86_FEATURE_WDT		( 6*32+13) /* Watchdog timer */
#define X86_FEATURE_LWP		( 6*32+15) /* Light Weight Profiling */
#define X86_FEATURE_FMA4	( 6*32+16) /* 4 operands MAC instructions */
#define X86_FEATURE_TCE		( 6*32+17) /* translation cache extension */
#define X86_FEATURE_NODEID_MSR	( 6*32+19) /* NodeId MSR */
#define X86_FEATURE_TBM		( 6*32+21) /* trailing bit manipulations */
#define X86_FEATURE_TOPOEXT	( 6*32+22) /* topology extensions CPUID leafs */
#define X86_FEATURE_PERFCTR_CORE ( 6*32+23) /* core performance counter extensions */
#define X86_FEATURE_PERFCTR_NB  ( 6*32+24) /* NB performance counter extensions */
#define X86_FEATURE_BPEXT	(6*32+26) /* data breakpoint extension */
#define X86_FEATURE_PERFCTR_L2	( 6*32+28) /* L2 performance counter extensions */

/*
 * Auxiliary flags: Linux defined - For features scattered in various
 * CPUID levels like 0x6, 0xA etc, word 7
 */
#define X86_FEATURE_IDA		( 7*32+ 0) /* Intel Dynamic Acceleration */
#define X86_FEATURE_ARAT	( 7*32+ 1) /* Always Running APIC Timer */
#define X86_FEATURE_CPB		( 7*32+ 2) /* AMD Core Performance Boost */
#define X86_FEATURE_EPB		( 7*32+ 3) /* IA32_ENERGY_PERF_BIAS support */
#define X86_FEATURE_PLN		( 7*32+ 5) /* Intel Power Limit Notification */
#define X86_FEATURE_PTS		( 7*32+ 6) /* Intel Package Thermal Status */
#define X86_FEATURE_DTHERM	( 7*32+ 7) /* Digital Thermal Sensor */
#define X86_FEATURE_HW_PSTATE	( 7*32+ 8) /* AMD HW-PState */
#define X86_FEATURE_PROC_FEEDBACK ( 7*32+ 9) /* AMD ProcFeedbackInterface */
#define X86_FEATURE_HWP		( 7*32+ 10) /* "hwp" Intel HWP */
#define X86_FEATURE_HWP_NOITFY	( 7*32+ 11) /* Intel HWP_NOTIFY */
#define X86_FEATURE_HWP_ACT_WINDOW ( 7*32+ 12) /* Intel HWP_ACT_WINDOW */
#define X86_FEATURE_HWP_EPP	( 7*32+13) /* Intel HWP_EPP */
#define X86_FEATURE_HWP_PKG_REQ ( 7*32+14) /* Intel HWP_PKG_REQ */
#define X86_FEATURE_INTEL_PT	( 7*32+15) /* Intel Processor Trace */

/* Virtualization flags: Linux defined, word 8 */
#define X86_FEATURE_TPR_SHADOW  ( 8*32+ 0) /* Intel TPR Shadow */
#define X86_FEATURE_VNMI        ( 8*32+ 1) /* Intel Virtual NMI */
#define X86_FEATURE_FLEXPRIORITY ( 8*32+ 2) /* Intel FlexPriority */
#define X86_FEATURE_EPT         ( 8*32+ 3) /* Intel Extended Page Table */
#define X86_FEATURE_VPID        ( 8*32+ 4) /* Intel Virtual Processor ID */
#define X86_FEATURE_NPT		( 8*32+ 5) /* AMD Nested Page Table support */
#define X86_FEATURE_LBRV	( 8*32+ 6) /* AMD LBR Virtualization support */
#define X86_FEATURE_SVML	( 8*32+ 7) /* "svm_lock" AMD SVM locking MSR */
#define X86_FEATURE_NRIPS	( 8*32+ 8) /* "nrip_save" AMD SVM next_rip save */
#define X86_FEATURE_TSCRATEMSR  ( 8*32+ 9) /* "tsc_scale" AMD TSC scaling support */
#define X86_FEATURE_VMCBCLEAN   ( 8*32+10) /* "vmcb_clean" AMD VMCB clean bits support */
#define X86_FEATURE_FLUSHBYASID ( 8*32+11) /* AMD flush-by-ASID support */
#define X86_FEATURE_DECODEASSISTS ( 8*32+12) /* AMD Decode Assists support */
#define X86_FEATURE_PAUSEFILTER ( 8*32+13) /* AMD filtered pause intercept */
#define X86_FEATURE_PFTHRESHOLD ( 8*32+14) /* AMD pause filter threshold */
#define X86_FEATURE_VMMCALL     ( 8*32+15) /* Prefer vmmcall to vmcall */


/* Intel-defined CPU features, CPUID level 0x00000007:0 (ebx), word 9 */
#define X86_FEATURE_FSGSBASE	( 9*32+ 0) /* {RD/WR}{FS/GS}BASE instructions*/
#define X86_FEATURE_TSC_ADJUST	( 9*32+ 1) /* TSC adjustment MSR 0x3b */
#define X86_FEATURE_BMI1	( 9*32+ 3) /* 1st group bit manipulation extensions */
#define X86_FEATURE_HLE		( 9*32+ 4) /* Hardware Lock Elision */
#define X86_FEATURE_AVX2	( 9*32+ 5) /* AVX2 instructions */
#define X86_FEATURE_SMEP	( 9*32+ 7) /* Supervisor Mode Execution Protection */
#define X86_FEATURE_BMI2	( 9*32+ 8) /* 2nd group bit manipulation extensions */
#define X86_FEATURE_ERMS	( 9*32+ 9) /* Enhanced REP MOVSB/STOSB */
#define X86_FEATURE_INVPCID	( 9*32+10) /* Invalidate Processor Context ID */
#define X86_FEATURE_RTM		( 9*32+11) /* Restricted Transactional Memory */
#define X86_FEATURE_CQM		( 9*32+12) /* Cache QoS Monitoring */
#define X86_FEATURE_MPX		( 9*32+14) /* Memory Protection Extension */
#define X86_FEATURE_AVX512F	( 9*32+16) /* AVX-512 Foundation */
#define X86_FEATURE_RDSEED	( 9*32+18) /* The RDSEED instruction */
#define X86_FEATURE_ADX		( 9*32+19) /* The ADCX and ADOX instructions */
#define X86_FEATURE_SMAP	( 9*32+20) /* Supervisor Mode Access Prevention */
#define X86_FEATURE_PCOMMIT	( 9*32+22) /* PCOMMIT instruction */
#define X86_FEATURE_CLFLUSHOPT	( 9*32+23) /* CLFLUSHOPT instruction */
#define X86_FEATURE_CLWB	( 9*32+24) /* CLWB instruction */
#define X86_FEATURE_AVX512PF	( 9*32+26) /* AVX-512 Prefetch */
#define X86_FEATURE_AVX512ER	( 9*32+27) /* AVX-512 Exponential and Reciprocal */
#define X86_FEATURE_AVX512CD	( 9*32+28) /* AVX-512 Conflict Detection */

/* Extended state features, CPUID level 0x0000000d:1 (eax), word 10 */
#define X86_FEATURE_XSAVEOPT	(10*32+ 0) /* XSAVEOPT */
#define X86_FEATURE_XSAVEC	(10*32+ 1) /* XSAVEC */
#define X86_FEATURE_XGETBV1	(10*32+ 2) /* XGETBV with ECX = 1 */
#define X86_FEATURE_XSAVES	(10*32+ 3) /* XSAVES/XRSTORS */

/* Intel-defined CPU QoS Sub-leaf, CPUID level 0x0000000F:0 (edx), word 11 */
#define X86_FEATURE_CQM_LLC	(11*32+ 1) /* LLC QoS if 1 */

/* Intel-defined CPU QoS Sub-leaf, CPUID level 0x0000000F:1 (edx), word 12 */
#define X86_FEATURE_CQM_OCCUP_LLC (12*32+ 0) /* LLC occupancy monitoring if 1 */

/*
 * BUG word(s)
 */
#define X86_BUG(x)		(NCAPINTS*32 + (x))

#define X86_BUG_F00F		X86_BUG(0) /* Intel F00F */
#define X86_BUG_FDIV		X86_BUG(1) /* FPU FDIV */
#define X86_BUG_COMA		X86_BUG(2) /* Cyrix 6x86 coma */
#define X86_BUG_AMD_TLB_MMATCH	X86_BUG(3) /* "tlb_mmatch" AMD Erratum 383 */
#define X86_BUG_AMD_APIC_C1E	X86_BUG(4) /* "apic_c1e" AMD Erratum 400 */
#define X86_BUG_11AP		X86_BUG(5) /* Bad local APIC aka 11AP */
#define X86_BUG_FXSAVE_LEAK	X86_BUG(6) /* FXSAVE leaks FOP/FIP/FOP */
#define X86_BUG_CLFLUSH_MONITOR	X86_BUG(7) /* AAI65, CLFLUSH required before MONITOR */
#define X86_BUG_SYSRET_SS_ATTRS	X86_BUG(8) /* SYSRET doesn't fix up SS attrs */

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)

#include <asm/asm.h>
#include <linux/bitops.h>

#ifdef CONFIG_X86_FEATURE_NAMES
extern const char * const x86_cap_flags[NCAPINTS*32];
extern const char * const x86_power_flags[32];
#define X86_CAP_FMT "%s"
#define x86_cap_flag(flag) x86_cap_flags[flag]
#else
#define X86_CAP_FMT "%d:%d"
#define x86_cap_flag(flag) ((flag) >> 5), ((flag) & 31)
#endif

/*
 * In order to save room, we index into this array by doing
 * X86_BUG_<name> - NCAPINTS*32.
 */
extern const char * const x86_bug_flags[NBUGINTS*32];

#define test_cpu_cap(c, bit)						\
	 test_bit(bit, (unsigned long *)((c)->x86_capability))

#define REQUIRED_MASK_BIT_SET(bit)					\
	 ( (((bit)>>5)==0 && (1UL<<((bit)&31) & REQUIRED_MASK0)) ||	\
	   (((bit)>>5)==1 && (1UL<<((bit)&31) & REQUIRED_MASK1)) ||	\
	   (((bit)>>5)==2 && (1UL<<((bit)&31) & REQUIRED_MASK2)) ||	\
	   (((bit)>>5)==3 && (1UL<<((bit)&31) & REQUIRED_MASK3)) ||	\
	   (((bit)>>5)==4 && (1UL<<((bit)&31) & REQUIRED_MASK4)) ||	\
	   (((bit)>>5)==5 && (1UL<<((bit)&31) & REQUIRED_MASK5)) ||	\
	   (((bit)>>5)==6 && (1UL<<((bit)&31) & REQUIRED_MASK6)) ||	\
	   (((bit)>>5)==7 && (1UL<<((bit)&31) & REQUIRED_MASK7)) ||	\
	   (((bit)>>5)==8 && (1UL<<((bit)&31) & REQUIRED_MASK8)) ||	\
	   (((bit)>>5)==9 && (1UL<<((bit)&31) & REQUIRED_MASK9)) )

#define DISABLED_MASK_BIT_SET(bit)					\
	 ( (((bit)>>5)==0 && (1UL<<((bit)&31) & DISABLED_MASK0)) ||	\
	   (((bit)>>5)==1 && (1UL<<((bit)&31) & DISABLED_MASK1)) ||	\
	   (((bit)>>5)==2 && (1UL<<((bit)&31) & DISABLED_MASK2)) ||	\
	   (((bit)>>5)==3 && (1UL<<((bit)&31) & DISABLED_MASK3)) ||	\
	   (((bit)>>5)==4 && (1UL<<((bit)&31) & DISABLED_MASK4)) ||	\
	   (((bit)>>5)==5 && (1UL<<((bit)&31) & DISABLED_MASK5)) ||	\
	   (((bit)>>5)==6 && (1UL<<((bit)&31) & DISABLED_MASK6)) ||	\
	   (((bit)>>5)==7 && (1UL<<((bit)&31) & DISABLED_MASK7)) ||	\
	   (((bit)>>5)==8 && (1UL<<((bit)&31) & DISABLED_MASK8)) ||	\
	   (((bit)>>5)==9 && (1UL<<((bit)&31) & DISABLED_MASK9)) )

#define cpu_has(c, bit)							\
	(__builtin_constant_p(bit) && REQUIRED_MASK_BIT_SET(bit) ? 1 :	\
	 test_cpu_cap(c, bit))

#define this_cpu_has(bit)						\
	(__builtin_constant_p(bit) && REQUIRED_MASK_BIT_SET(bit) ? 1 : 	\
	 x86_this_cpu_test_bit(bit, (unsigned long *)&cpu_info.x86_capability))

/*
 * This macro is for detection of features which need kernel
 * infrastructure to be used.  It may *not* directly test the CPU
 * itself.  Use the cpu_has() family if you want true runtime
 * testing of CPU features, like in hypervisor code where you are
 * supporting a possible guest feature where host support for it
 * is not relevant.
 */
#define cpu_feature_enabled(bit)	\
	(__builtin_constant_p(bit) && DISABLED_MASK_BIT_SET(bit) ? 0 :	\
	 cpu_has(&boot_cpu_data, bit))

#define boot_cpu_has(bit)	cpu_has(&boot_cpu_data, bit)

#define set_cpu_cap(c, bit)	set_bit(bit, (unsigned long *)((c)->x86_capability))
#define clear_cpu_cap(c, bit)	clear_bit(bit, (unsigned long *)((c)->x86_capability))
#define setup_clear_cpu_cap(bit) do { \
	clear_cpu_cap(&boot_cpu_data, bit);	\
	set_bit(bit, (unsigned long *)cpu_caps_cleared); \
} while (0)
#define setup_force_cpu_cap(bit) do { \
	set_cpu_cap(&boot_cpu_data, bit);	\
	set_bit(bit, (unsigned long *)cpu_caps_set);	\
} while (0)

#define setup_force_cpu_bug(bit) setup_force_cpu_cap(bit)

#define cpu_has_fpu		boot_cpu_has(X86_FEATURE_FPU)
#define cpu_has_de		boot_cpu_has(X86_FEATURE_DE)
#define cpu_has_pse		boot_cpu_has(X86_FEATURE_PSE)
#define cpu_has_tsc		boot_cpu_has(X86_FEATURE_TSC)
#define cpu_has_pge		boot_cpu_has(X86_FEATURE_PGE)
#define cpu_has_apic		boot_cpu_has(X86_FEATURE_APIC)
#define cpu_has_sep		boot_cpu_has(X86_FEATURE_SEP)
#define cpu_has_mtrr		boot_cpu_has(X86_FEATURE_MTRR)
#define cpu_has_mmx		boot_cpu_has(X86_FEATURE_MMX)
#define cpu_has_fxsr		boot_cpu_has(X86_FEATURE_FXSR)
#define cpu_has_xmm		boot_cpu_has(X86_FEATURE_XMM)
#define cpu_has_xmm2		boot_cpu_has(X86_FEATURE_XMM2)
#define cpu_has_xmm3		boot_cpu_has(X86_FEATURE_XMM3)
#define cpu_has_ssse3		boot_cpu_has(X86_FEATURE_SSSE3)
#define cpu_has_aes		boot_cpu_has(X86_FEATURE_AES)
#define cpu_has_avx		boot_cpu_has(X86_FEATURE_AVX)
#define cpu_has_avx2		boot_cpu_has(X86_FEATURE_AVX2)
#define cpu_has_ht		boot_cpu_has(X86_FEATURE_HT)
#define cpu_has_nx		boot_cpu_has(X86_FEATURE_NX)
#define cpu_has_xstore		boot_cpu_has(X86_FEATURE_XSTORE)
#define cpu_has_xstore_enabled	boot_cpu_has(X86_FEATURE_XSTORE_EN)
#define cpu_has_xcrypt		boot_cpu_has(X86_FEATURE_XCRYPT)
#define cpu_has_xcrypt_enabled	boot_cpu_has(X86_FEATURE_XCRYPT_EN)
#define cpu_has_ace2		boot_cpu_has(X86_FEATURE_ACE2)
#define cpu_has_ace2_enabled	boot_cpu_has(X86_FEATURE_ACE2_EN)
#define cpu_has_phe		boot_cpu_has(X86_FEATURE_PHE)
#define cpu_has_phe_enabled	boot_cpu_has(X86_FEATURE_PHE_EN)
#define cpu_has_pmm		boot_cpu_has(X86_FEATURE_PMM)
#define cpu_has_pmm_enabled	boot_cpu_has(X86_FEATURE_PMM_EN)
#define cpu_has_ds		boot_cpu_has(X86_FEATURE_DS)
#define cpu_has_pebs		boot_cpu_has(X86_FEATURE_PEBS)
#define cpu_has_clflush		boot_cpu_has(X86_FEATURE_CLFLUSH)
#define cpu_has_bts		boot_cpu_has(X86_FEATURE_BTS)
#define cpu_has_gbpages		boot_cpu_has(X86_FEATURE_GBPAGES)
#define cpu_has_arch_perfmon	boot_cpu_has(X86_FEATURE_ARCH_PERFMON)
#define cpu_has_pat		boot_cpu_has(X86_FEATURE_PAT)
#define cpu_has_xmm4_1		boot_cpu_has(X86_FEATURE_XMM4_1)
#define cpu_has_xmm4_2		boot_cpu_has(X86_FEATURE_XMM4_2)
#define cpu_has_x2apic		boot_cpu_has(X86_FEATURE_X2APIC)
#define cpu_has_xsave		boot_cpu_has(X86_FEATURE_XSAVE)
#define cpu_has_xsaveopt	boot_cpu_has(X86_FEATURE_XSAVEOPT)
#define cpu_has_xsaves		boot_cpu_has(X86_FEATURE_XSAVES)
#define cpu_has_osxsave		boot_cpu_has(X86_FEATURE_OSXSAVE)
#define cpu_has_hypervisor	boot_cpu_has(X86_FEATURE_HYPERVISOR)
#define cpu_has_pclmulqdq	boot_cpu_has(X86_FEATURE_PCLMULQDQ)
#define cpu_has_perfctr_core	boot_cpu_has(X86_FEATURE_PERFCTR_CORE)
#define cpu_has_perfctr_nb	boot_cpu_has(X86_FEATURE_PERFCTR_NB)
#define cpu_has_perfctr_l2	boot_cpu_has(X86_FEATURE_PERFCTR_L2)
#define cpu_has_cx8		boot_cpu_has(X86_FEATURE_CX8)
#define cpu_has_cx16		boot_cpu_has(X86_FEATURE_CX16)
#define cpu_has_eager_fpu	boot_cpu_has(X86_FEATURE_EAGER_FPU)
#define cpu_has_topoext		boot_cpu_has(X86_FEATURE_TOPOEXT)
#define cpu_has_bpext		boot_cpu_has(X86_FEATURE_BPEXT)

#if __GNUC__ >= 4
extern void warn_pre_alternatives(void);
extern bool __static_cpu_has_safe(u16 bit);

/*
 * Static testing of CPU features.  Used the same as boot_cpu_has().
 * These are only valid after alternatives have run, but will statically
 * patch the target code for additional performance.
 */
static __always_inline __pure bool __static_cpu_has(u16 bit)
{
#ifdef CC_HAVE_ASM_GOTO

#ifdef CONFIG_X86_DEBUG_STATIC_CPU_HAS

		/*
		 * Catch too early usage of this before alternatives
		 * have run.
		 */
		asm_volatile_goto("1: jmp %l[t_warn]\n"
			 "2:\n"
			 ".section .altinstructions,\"a\"\n"
			 " .long 1b - .\n"
			 " .long 0\n"		/* no replacement */
			 " .word %P0\n"		/* 1: do replace */
			 " .byte 2b - 1b\n"	/* source len */
			 " .byte 0\n"		/* replacement len */
			 " .byte 0\n"		/* pad len */
			 ".previous\n"
			 /* skipping size check since replacement size = 0 */
			 : : "i" (X86_FEATURE_ALWAYS) : : t_warn);

#endif

		asm_volatile_goto("1: jmp %l[t_no]\n"
			 "2:\n"
			 ".section .altinstructions,\"a\"\n"
			 " .long 1b - .\n"
			 " .long 0\n"		/* no replacement */
			 " .word %P0\n"		/* feature bit */
			 " .byte 2b - 1b\n"	/* source len */
			 " .byte 0\n"		/* replacement len */
			 " .byte 0\n"		/* pad len */
			 ".previous\n"
			 /* skipping size check since replacement size = 0 */
			 : : "i" (bit) : : t_no);
		return true;
	t_no:
		return false;

#ifdef CONFIG_X86_DEBUG_STATIC_CPU_HAS
	t_warn:
		warn_pre_alternatives();
		return false;
#endif

#else /* CC_HAVE_ASM_GOTO */

		u8 flag;
		/* Open-coded due to __stringify() in ALTERNATIVE() */
		asm volatile("1: movb $0,%0\n"
			     "2:\n"
			     ".section .altinstructions,\"a\"\n"
			     " .long 1b - .\n"
			     " .long 3f - .\n"
			     " .word %P1\n"		/* feature bit */
			     " .byte 2b - 1b\n"		/* source len */
			     " .byte 4f - 3f\n"		/* replacement len */
			     " .byte 0\n"		/* pad len */
			     ".previous\n"
			     ".section .discard,\"aw\",@progbits\n"
			     " .byte 0xff + (4f-3f) - (2b-1b)\n" /* size check */
			     ".previous\n"
			     ".section .altinstr_replacement,\"ax\"\n"
			     "3: movb $1,%0\n"
			     "4:\n"
			     ".previous\n"
			     : "=qm" (flag) : "i" (bit));
		return flag;

#endif /* CC_HAVE_ASM_GOTO */
}

#define static_cpu_has(bit)					\
(								\
	__builtin_constant_p(boot_cpu_has(bit)) ?		\
		boot_cpu_has(bit) :				\
	__builtin_constant_p(bit) ?				\
		__static_cpu_has(bit) :				\
		boot_cpu_has(bit)				\
)

static __always_inline __pure bool _static_cpu_has_safe(u16 bit)
{
#ifdef CC_HAVE_ASM_GOTO
		asm_volatile_goto("1: jmp %l[t_dynamic]\n"
			 "2:\n"
			 ".skip -(((5f-4f) - (2b-1b)) > 0) * "
			         "((5f-4f) - (2b-1b)),0x90\n"
			 "3:\n"
			 ".section .altinstructions,\"a\"\n"
			 " .long 1b - .\n"		/* src offset */
			 " .long 4f - .\n"		/* repl offset */
			 " .word %P1\n"			/* always replace */
			 " .byte 3b - 1b\n"		/* src len */
			 " .byte 5f - 4f\n"		/* repl len */
			 " .byte 3b - 2b\n"		/* pad len */
			 ".previous\n"
			 ".section .altinstr_replacement,\"ax\"\n"
			 "4: jmp %l[t_no]\n"
			 "5:\n"
			 ".previous\n"
			 ".section .altinstructions,\"a\"\n"
			 " .long 1b - .\n"		/* src offset */
			 " .long 0\n"			/* no replacement */
			 " .word %P0\n"			/* feature bit */
			 " .byte 3b - 1b\n"		/* src len */
			 " .byte 0\n"			/* repl len */
			 " .byte 0\n"			/* pad len */
			 ".previous\n"
			 : : "i" (bit), "i" (X86_FEATURE_ALWAYS)
			 : : t_dynamic, t_no);
		return true;
	t_no:
		return false;
	t_dynamic:
		return __static_cpu_has_safe(bit);
#else
		u8 flag;
		/* Open-coded due to __stringify() in ALTERNATIVE() */
		asm volatile("1: movb $2,%0\n"
			     "2:\n"
			     ".section .altinstructions,\"a\"\n"
			     " .long 1b - .\n"		/* src offset */
			     " .long 3f - .\n"		/* repl offset */
			     " .word %P2\n"		/* always replace */
			     " .byte 2b - 1b\n"		/* source len */
			     " .byte 4f - 3f\n"		/* replacement len */
			     " .byte 0\n"		/* pad len */
			     ".previous\n"
			     ".section .discard,\"aw\",@progbits\n"
			     " .byte 0xff + (4f-3f) - (2b-1b)\n" /* size check */
			     ".previous\n"
			     ".section .altinstr_replacement,\"ax\"\n"
			     "3: movb $0,%0\n"
			     "4:\n"
			     ".previous\n"
			     ".section .altinstructions,\"a\"\n"
			     " .long 1b - .\n"		/* src offset */
			     " .long 5f - .\n"		/* repl offset */
			     " .word %P1\n"		/* feature bit */
			     " .byte 4b - 3b\n"		/* src len */
			     " .byte 6f - 5f\n"		/* repl len */
			     " .byte 0\n"		/* pad len */
			     ".previous\n"
			     ".section .discard,\"aw\",@progbits\n"
			     " .byte 0xff + (6f-5f) - (4b-3b)\n" /* size check */
			     ".previous\n"
			     ".section .altinstr_replacement,\"ax\"\n"
			     "5: movb $1,%0\n"
			     "6:\n"
			     ".previous\n"
			     : "=qm" (flag)
			     : "i" (bit), "i" (X86_FEATURE_ALWAYS));
		return (flag == 2 ? __static_cpu_has_safe(bit) : flag);
#endif /* CC_HAVE_ASM_GOTO */
}

#define static_cpu_has_safe(bit)				\
(								\
	__builtin_constant_p(boot_cpu_has(bit)) ?		\
		boot_cpu_has(bit) :				\
		_static_cpu_has_safe(bit)			\
)
#else
/*
 * gcc 3.x is too stupid to do the static test; fall back to dynamic.
 */
#define static_cpu_has(bit)		boot_cpu_has(bit)
#define static_cpu_has_safe(bit)	boot_cpu_has(bit)
#endif

#define cpu_has_bug(c, bit)		cpu_has(c, (bit))
#define set_cpu_bug(c, bit)		set_cpu_cap(c, (bit))
#define clear_cpu_bug(c, bit)		clear_cpu_cap(c, (bit))

#define static_cpu_has_bug(bit)		static_cpu_has((bit))
#define static_cpu_has_bug_safe(bit)	static_cpu_has_safe((bit))
#define boot_cpu_has_bug(bit)		cpu_has_bug(&boot_cpu_data, (bit))

#define MAX_CPU_FEATURES		(NCAPINTS * 32)
#define cpu_have_feature		boot_cpu_has

#define CPU_FEATURE_TYPEFMT		"x86,ven%04Xfam%04Xmod%04X"
#define CPU_FEATURE_TYPEVAL		boot_cpu_data.x86_vendor, boot_cpu_data.x86, \
					boot_cpu_data.x86_model

#endif /* defined(__KERNEL__) && !defined(__ASSEMBLY__) */
#endif /* _ASM_X86_CPUFEATURE_H */
