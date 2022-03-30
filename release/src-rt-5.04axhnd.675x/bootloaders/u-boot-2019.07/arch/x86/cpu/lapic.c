// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot file of same name
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2014 Google, Inc
 */

#include <common.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/post.h>

unsigned long lapic_read(unsigned long reg)
{
	return readl(LAPIC_DEFAULT_BASE + reg);
}

#define xchg(ptr, v)	((__typeof__(*(ptr)))__xchg((unsigned long)(v), (ptr), \
						    sizeof(*(ptr))))

struct __xchg_dummy	{ unsigned long a[100]; };
#define __xg(x)		((struct __xchg_dummy *)(x))

/*
 * Note: no "lock" prefix even on SMP. xchg always implies lock anyway.
 *
 * Note 2: xchg has side effect, so that attribute volatile is necessary,
 *         but generally the primitive is invalid, *ptr is output argument.
 */
static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
				   int size)
{
	switch (size) {
	case 1:
		__asm__ __volatile__("xchgb %b0,%1"
			: "=q" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	case 2:
		__asm__ __volatile__("xchgw %w0,%1"
			: "=r" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	case 4:
		__asm__ __volatile__("xchgl %0,%1"
			: "=r" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	}

	return x;
}

void lapic_write(unsigned long reg, unsigned long v)
{
	(void)xchg((volatile unsigned long *)(LAPIC_DEFAULT_BASE + reg), v);
}

void enable_lapic(void)
{
	if (!IS_ENABLED(CONFIG_INTEL_QUARK)) {
		msr_t msr;

		msr = msr_read(MSR_IA32_APICBASE);
		msr.hi &= 0xffffff00;
		msr.lo |= MSR_IA32_APICBASE_ENABLE;
		msr.lo &= ~MSR_IA32_APICBASE_BASE;
		msr.lo |= LAPIC_DEFAULT_BASE;
		msr_write(MSR_IA32_APICBASE, msr);
	}
}

void disable_lapic(void)
{
	if (!IS_ENABLED(CONFIG_INTEL_QUARK)) {
		msr_t msr;

		msr = msr_read(MSR_IA32_APICBASE);
		msr.lo &= ~MSR_IA32_APICBASE_ENABLE;
		msr_write(MSR_IA32_APICBASE, msr);
	}
}

unsigned long lapicid(void)
{
	return lapic_read(LAPIC_ID) >> 24;
}

static void lapic_wait_icr_idle(void)
{
	do { } while (lapic_read(LAPIC_ICR) & LAPIC_ICR_BUSY);
}

int lapic_remote_read(int apicid, int reg, unsigned long *pvalue)
{
	int timeout;
	unsigned long status;
	int result;

	lapic_wait_icr_idle();
	lapic_write(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(apicid));
	lapic_write(LAPIC_ICR, LAPIC_DM_REMRD | (reg >> 4));

	timeout = 0;
	do {
		status = lapic_read(LAPIC_ICR) & LAPIC_ICR_RR_MASK;
	} while (status == LAPIC_ICR_RR_INPROG && timeout++ < 1000);

	result = -1;
	if (status == LAPIC_ICR_RR_VALID) {
		*pvalue = lapic_read(LAPIC_RRR);
		result = 0;
	}

	return result;
}

void lapic_setup(void)
{
	/* Only Pentium Pro and later have those MSR stuff */
	debug("Setting up local apic: ");

	/* Enable the local apic */
	enable_lapic();

	/* Set Task Priority to 'accept all' */
	lapic_write(LAPIC_TASKPRI,
		    lapic_read(LAPIC_TASKPRI) & ~LAPIC_TPRI_MASK);

	/* Put the local apic in virtual wire mode */
	lapic_write(LAPIC_SPIV, (lapic_read(LAPIC_SPIV) &
		    ~(LAPIC_VECTOR_MASK)) | LAPIC_SPIV_ENABLE);
	lapic_write(LAPIC_LVT0, (lapic_read(LAPIC_LVT0) &
		    ~(LAPIC_LVT_MASKED | LAPIC_LVT_LEVEL_TRIGGER |
		    LAPIC_LVT_REMOTE_IRR | LAPIC_INPUT_POLARITY |
		    LAPIC_SEND_PENDING | LAPIC_LVT_RESERVED_1 |
		    LAPIC_DELIVERY_MODE_MASK)) |
		    (LAPIC_LVT_REMOTE_IRR | LAPIC_SEND_PENDING |
		    LAPIC_DELIVERY_MODE_EXTINT));
	lapic_write(LAPIC_LVT1, (lapic_read(LAPIC_LVT1) &
		    ~(LAPIC_LVT_MASKED | LAPIC_LVT_LEVEL_TRIGGER |
		    LAPIC_LVT_REMOTE_IRR | LAPIC_INPUT_POLARITY |
		    LAPIC_SEND_PENDING | LAPIC_LVT_RESERVED_1 |
		    LAPIC_DELIVERY_MODE_MASK)) |
		    (LAPIC_LVT_REMOTE_IRR | LAPIC_SEND_PENDING |
		    LAPIC_DELIVERY_MODE_NMI));

	debug("apic_id: 0x%02lx, ", lapicid());

	debug("done.\n");
	post_code(POST_LAPIC);
}
