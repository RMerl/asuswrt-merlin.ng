/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Taken from the linux kernel file of the same name
 *
 * (C) Copyright 2012
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#ifndef _ASM_X86_MSR_H
#define _ASM_X86_MSR_H

#include <asm/msr-index.h>

#ifndef __ASSEMBLY__

#include <linux/types.h>
#include <linux/ioctl.h>

#define X86_IOC_RDMSR_REGS	_IOWR('c', 0xA0, __u32[8])
#define X86_IOC_WRMSR_REGS	_IOWR('c', 0xA1, __u32[8])

#ifdef __KERNEL__

#include <linux/errno.h>

struct msr {
	union {
		struct {
			u32 l;
			u32 h;
		};
		u64 q;
	};
};

struct msr_info {
	u32 msr_no;
	struct msr reg;
	struct msr *msrs;
	int err;
};

struct msr_regs_info {
	u32 *regs;
	int err;
};

static inline unsigned long long native_read_tscp(unsigned int *aux)
{
	unsigned long low, high;
	asm volatile(".byte 0x0f,0x01,0xf9"
		     : "=a" (low), "=d" (high), "=c" (*aux));
	return low | ((u64)high << 32);
}

/*
 * both i386 and x86_64 returns 64-bit value in edx:eax, but gcc's "A"
 * constraint has different meanings. For i386, "A" means exactly
 * edx:eax, while for x86_64 it doesn't mean rdx:rax or edx:eax. Instead,
 * it means rax *or* rdx.
 */
#ifdef CONFIG_X86_64
#define DECLARE_ARGS(val, low, high)	unsigned low, high
#define EAX_EDX_VAL(val, low, high)	((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)	"a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)	"=a" (low), "=d" (high)
#else
#define DECLARE_ARGS(val, low, high)	unsigned long long val
#define EAX_EDX_VAL(val, low, high)	(val)
#define EAX_EDX_ARGS(val, low, high)	"A" (val)
#define EAX_EDX_RET(val, low, high)	"=A" (val)
#endif

static inline __attribute__((no_instrument_function))
	unsigned long long native_read_msr(unsigned int msr)
{
	DECLARE_ARGS(val, low, high);

	asm volatile("rdmsr" : EAX_EDX_RET(val, low, high) : "c" (msr));
	return EAX_EDX_VAL(val, low, high);
}

static inline void native_write_msr(unsigned int msr,
				    unsigned low, unsigned high)
{
	asm volatile("wrmsr" : : "c" (msr), "a"(low), "d" (high) : "memory");
}

extern unsigned long long native_read_tsc(void);

extern int native_rdmsr_safe_regs(u32 regs[8]);
extern int native_wrmsr_safe_regs(u32 regs[8]);

static inline unsigned long long native_read_pmc(int counter)
{
	DECLARE_ARGS(val, low, high);

	asm volatile("rdpmc" : EAX_EDX_RET(val, low, high) : "c" (counter));
	return EAX_EDX_VAL(val, low, high);
}

#ifdef CONFIG_PARAVIRT
#include <asm/paravirt.h>
#else
#include <errno.h>
/*
 * Access to machine-specific registers (available on 586 and better only)
 * Note: the rd* operations modify the parameters directly (without using
 * pointer indirection), this allows gcc to optimize better
 */

#define rdmsr(msr, val1, val2)					\
do {								\
	u64 __val = native_read_msr((msr));			\
	(void)((val1) = (u32)__val);				\
	(void)((val2) = (u32)(__val >> 32));			\
} while (0)

static inline void wrmsr(unsigned msr, unsigned low, unsigned high)
{
	native_write_msr(msr, low, high);
}

#define rdmsrl(msr, val)			\
	((val) = native_read_msr((msr)))

#define wrmsrl(msr, val)						\
	native_write_msr((msr), (u32)((u64)(val)), (u32)((u64)(val) >> 32))

static inline void msr_clrsetbits_64(unsigned msr, u64 clear, u64 set)
{
	u64 val;

	val = native_read_msr(msr);
	val &= ~clear;
	val |= set;
	wrmsrl(msr, val);
}

static inline void msr_setbits_64(unsigned msr, u64 set)
{
	u64 val;

	val = native_read_msr(msr);
	val |= set;
	wrmsrl(msr, val);
}

static inline void msr_clrbits_64(unsigned msr, u64 clear)
{
	u64 val;

	val = native_read_msr(msr);
	val &= ~clear;
	wrmsrl(msr, val);
}

/* rdmsr with exception handling */
#define rdmsr_safe(msr, p1, p2)					\
({								\
	int __err;						\
	u64 __val = native_read_msr_safe((msr), &__err);	\
	(*p1) = (u32)__val;					\
	(*p2) = (u32)(__val >> 32);				\
	__err;							\
})

static inline int rdmsrl_amd_safe(unsigned msr, unsigned long long *p)
{
	u32 gprs[8] = { 0 };
	int err;

	gprs[1] = msr;
	gprs[7] = 0x9c5a203a;

	err = native_rdmsr_safe_regs(gprs);

	*p = gprs[0] | ((u64)gprs[2] << 32);

	return err;
}

static inline int wrmsrl_amd_safe(unsigned msr, unsigned long long val)
{
	u32 gprs[8] = { 0 };

	gprs[0] = (u32)val;
	gprs[1] = msr;
	gprs[2] = val >> 32;
	gprs[7] = 0x9c5a203a;

	return native_wrmsr_safe_regs(gprs);
}

static inline int rdmsr_safe_regs(u32 regs[8])
{
	return native_rdmsr_safe_regs(regs);
}

static inline int wrmsr_safe_regs(u32 regs[8])
{
	return native_wrmsr_safe_regs(regs);
}

typedef struct msr_t {
	uint32_t lo;
	uint32_t hi;
} msr_t;

static inline struct msr_t msr_read(unsigned msr_num)
{
	struct msr_t msr;

	rdmsr(msr_num, msr.lo, msr.hi);

	return msr;
}

static inline void msr_write(unsigned msr_num, msr_t msr)
{
	wrmsr(msr_num, msr.lo, msr.hi);
}

#define rdtscl(low)						\
	((low) = (u32)__native_read_tsc())

#define rdtscll(val)						\
	((val) = __native_read_tsc())

#define rdpmc(counter, low, high)			\
do {							\
	u64 _l = native_read_pmc((counter));		\
	(low)  = (u32)_l;				\
	(high) = (u32)(_l >> 32);			\
} while (0)

#define rdtscp(low, high, aux)					\
do {                                                            \
	unsigned long long _val = native_read_tscp(&(aux));     \
	(low) = (u32)_val;                                      \
	(high) = (u32)(_val >> 32);                             \
} while (0)

#define rdtscpll(val, aux) (val) = native_read_tscp(&(aux))

#endif	/* !CONFIG_PARAVIRT */


#define checking_wrmsrl(msr, val) wrmsr_safe((msr), (u32)(val),		\
					     (u32)((val) >> 32))

#define write_tsc(val1, val2) wrmsr(MSR_IA32_TSC, (val1), (val2))

#define write_rdtscp_aux(val) wrmsr(MSR_TSC_AUX, (val), 0)

struct msr *msrs_alloc(void);
void msrs_free(struct msr *msrs);

#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */
#endif /* _ASM_X86_MSR_H */
