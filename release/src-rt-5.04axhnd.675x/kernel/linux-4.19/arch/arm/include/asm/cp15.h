/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_CP15_H
#define __ASM_ARM_CP15_H

#include <asm/barrier.h>
#if defined(CONFIG_BCM_KF_ARM_KASAN)
#include <linux/stringify.h>
#endif

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)
#ifdef CONFIG_MMU
#define CR_HA	(1 << 17)	/* Hardware management of Access Flag   */
#else
#define CR_BR	(1 << 17)	/* MPU Background region enable (PMSA)  */
#endif
#define CR_IT	(1 << 18)
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

#ifndef __ASSEMBLY__

#if __LINUX_ARM_ARCH__ >= 4
#define vectors_high()	(get_cr() & CR_V)
#else
#define vectors_high()	(0)
#endif

#ifdef CONFIG_CPU_CP15

#define __ACCESS_CP15(CRn, Op1, CRm, Op2)	\
	"mrc", "mcr", __stringify(p15, Op1, %0, CRn, CRm, Op2), u32
#define __ACCESS_CP15_64(Op1, CRm)		\
	"mrrc", "mcrr", __stringify(p15, Op1, %Q0, %R0, CRm), u64

#define __read_sysreg(r, w, c, t) ({				\
	t __val;						\
	asm volatile(r " " c : "=r" (__val));			\
	__val;							\
})
#define read_sysreg(...)		__read_sysreg(__VA_ARGS__)

#define __write_sysreg(v, r, w, c, t)	asm volatile(w " " c : : "r" ((t)(v)))
#define write_sysreg(v, ...)		__write_sysreg(v, __VA_ARGS__)

#define BPIALL				__ACCESS_CP15(c7, 0, c5, 6)
#define ICIALLU				__ACCESS_CP15(c7, 0, c5, 0)

#define CNTVCT				__ACCESS_CP15_64(1, c14)

#if defined(CONFIG_BCM_KF_ARM_KASAN)
#define TTBR0_32	__ACCESS_CP15(c2, 0, c0, 0)
#define TTBR1_32	__ACCESS_CP15(c2, 0, c0, 1)
#define PAR_32		__ACCESS_CP15(c7, 0, c4, 0)
#define TTBR0_64	__ACCESS_CP15_64(0, c2)
#define TTBR1_64	__ACCESS_CP15_64(1, c2)
#define PAR_64		__ACCESS_CP15_64(0, c7)
#define VTTBR		__ACCESS_CP15_64(6, c2)
#define CNTP_CVAL      __ACCESS_CP15_64(2, c14)
#define CNTV_CVAL	__ACCESS_CP15_64(3, c14)
#define CNTVOFF		__ACCESS_CP15_64(4, c14)

#define MIDR		__ACCESS_CP15(c0, 0, c0, 0)
#define CSSELR		__ACCESS_CP15(c0, 2, c0, 0)
#define VPIDR		__ACCESS_CP15(c0, 4, c0, 0)
#define VMPIDR		__ACCESS_CP15(c0, 4, c0, 5)
#define SCTLR		__ACCESS_CP15(c1, 0, c0, 0)
#define CPACR		__ACCESS_CP15(c1, 0, c0, 2)
#define HCR		__ACCESS_CP15(c1, 4, c1, 0)
#define HDCR		__ACCESS_CP15(c1, 4, c1, 1)
#define HCPTR		__ACCESS_CP15(c1, 4, c1, 2)
#define HSTR		__ACCESS_CP15(c1, 4, c1, 3)
#define TTBCR		__ACCESS_CP15(c2, 0, c0, 2)
#define HTCR		__ACCESS_CP15(c2, 4, c0, 2)
#define VTCR		__ACCESS_CP15(c2, 4, c1, 2)
#define DACR		__ACCESS_CP15(c3, 0, c0, 0)
#define DFSR		__ACCESS_CP15(c5, 0, c0, 0)
#define IFSR		__ACCESS_CP15(c5, 0, c0, 1)
#define ADFSR		__ACCESS_CP15(c5, 0, c1, 0)
#define AIFSR		__ACCESS_CP15(c5, 0, c1, 1)
#define HSR		__ACCESS_CP15(c5, 4, c2, 0)
#define DFAR		__ACCESS_CP15(c6, 0, c0, 0)
#define IFAR		__ACCESS_CP15(c6, 0, c0, 2)
#define HDFAR		__ACCESS_CP15(c6, 4, c0, 0)
#define HIFAR		__ACCESS_CP15(c6, 4, c0, 2)
#define HPFAR		__ACCESS_CP15(c6, 4, c0, 4)
#define ICIALLUIS	__ACCESS_CP15(c7, 0, c1, 0)
#define BPIALLIS	__ACCESS_CP15(c7, 0, c1, 6)
#define ICIMVAU		__ACCESS_CP15(c7, 0, c5, 1)
#define ATS1CPR		__ACCESS_CP15(c7, 0, c8, 0)
#define TLBIALLIS	__ACCESS_CP15(c8, 0, c3, 0)
#define TLBIALL		__ACCESS_CP15(c8, 0, c7, 0)
#define TLBIALLNSNHIS	__ACCESS_CP15(c8, 4, c3, 4)
#define PRRR		__ACCESS_CP15(c10, 0, c2, 0)
#define NMRR		__ACCESS_CP15(c10, 0, c2, 1)
#define AMAIR0		__ACCESS_CP15(c10, 0, c3, 0)
#define AMAIR1		__ACCESS_CP15(c10, 0, c3, 1)
#define VBAR		__ACCESS_CP15(c12, 0, c0, 0)
#define CID		__ACCESS_CP15(c13, 0, c0, 1)
#define TID_URW		__ACCESS_CP15(c13, 0, c0, 2)
#define TID_URO		__ACCESS_CP15(c13, 0, c0, 3)
#define TID_PRIV	__ACCESS_CP15(c13, 0, c0, 4)
#define HTPIDR		__ACCESS_CP15(c13, 4, c0, 2)
#define CNTKCTL		__ACCESS_CP15(c14, 0, c1, 0)
#define CNTP_CTL	__ACCESS_CP15(c14, 0, c2, 1)
#define CNTV_CTL	__ACCESS_CP15(c14, 0, c3, 1)
#define CNTHCTL		__ACCESS_CP15(c14, 4, c1, 0)

#endif
extern unsigned long cr_alignment;	/* defined in entry-armv.S */
#if defined(CONFIG_BCM_KF_ARM_KASAN)

static inline void set_par(u64 val)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		write_sysreg(val, PAR_64);
	else
		write_sysreg(val, PAR_32);
}

static inline u64 get_par(void)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		return read_sysreg(PAR_64);
	else
		return read_sysreg(PAR_32);
}

static inline void set_ttbr0(u64 val)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		write_sysreg(val, TTBR0_64);
	else
		write_sysreg(val, TTBR0_32);
}

static inline u64 get_ttbr0(void)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		return read_sysreg(TTBR0_64);
	else
		return read_sysreg(TTBR0_32);
}

static inline void set_ttbr1(u64 val)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		write_sysreg(val, TTBR1_64);
	else
		write_sysreg(val, TTBR1_32);
}

static inline u64 get_ttbr1(void)
{
	if (IS_ENABLED(CONFIG_ARM_LPAE))
		return read_sysreg(TTBR1_64);
	else
		return read_sysreg(TTBR1_32);
}
#endif

static inline unsigned long get_cr(void)
{
	unsigned long val;
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_cr(unsigned long val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

static inline unsigned int get_auxcr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 1	@ get AUXCR" : "=r" (val));
	return val;
}

static inline void set_auxcr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 1	@ set AUXCR"
	  : : "r" (val));
	isb();
}

#define CPACC_FULL(n)		(3 << (n * 2))
#define CPACC_SVC(n)		(1 << (n * 2))
#define CPACC_DISABLE(n)	(0 << (n * 2))

static inline unsigned int get_copro_access(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 2 @ get copro access"
	  : "=r" (val) : : "cc");
	return val;
}

static inline void set_copro_access(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 2 @ set copro access"
	  : : "r" (val) : "cc");
	isb();
}

#else /* ifdef CONFIG_CPU_CP15 */

/*
 * cr_alignment is tightly coupled to cp15 (at least in the minds of the
 * developers). Yielding 0 for machines without a cp15 (and making it
 * read-only) is fine for most cases and saves quite some #ifdeffery.
 */
#define cr_alignment	UL(0)

static inline unsigned long get_cr(void)
{
	return 0;
}

#endif /* ifdef CONFIG_CPU_CP15 / else */

#endif /* ifndef __ASSEMBLY__ */

#endif
