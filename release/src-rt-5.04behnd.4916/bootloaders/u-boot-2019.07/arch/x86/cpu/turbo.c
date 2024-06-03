// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot file of the same name
 *
 * Copyright (C) 2011 The Chromium Authors.
 */

#include <common.h>
#include <asm/cpu.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/turbo.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CPU_INTEL_TURBO_NOT_PACKAGE_SCOPED
static inline int get_global_turbo_state(void)
{
	return TURBO_UNKNOWN;
}

static inline void set_global_turbo_state(int state)
{
}
#else
static inline int get_global_turbo_state(void)
{
	return gd->arch.turbo_state;
}

static inline void set_global_turbo_state(int state)
{
	gd->arch.turbo_state = state;
}
#endif

static const char *const turbo_state_desc[] = {
	[TURBO_UNKNOWN]		= "unknown",
	[TURBO_UNAVAILABLE]	= "unavailable",
	[TURBO_DISABLED]	= "available but hidden",
	[TURBO_ENABLED]		= "available and visible"
};

/*
 * Determine the current state of Turbo and cache it for later.
 * Turbo is a package level config so it does not need to be
 * enabled on every core.
 */
int turbo_get_state(void)
{
	struct cpuid_result cpuid_regs;
	int turbo_en, turbo_cap;
	msr_t msr;
	int turbo_state = get_global_turbo_state();

	/* Return cached state if available */
	if (turbo_state != TURBO_UNKNOWN)
		return turbo_state;

	cpuid_regs = cpuid(CPUID_LEAF_PM);
	turbo_cap = !!(cpuid_regs.eax & PM_CAP_TURBO_MODE);

	msr = msr_read(MSR_IA32_MISC_ENABLES);
	turbo_en = !(msr.hi & H_MISC_DISABLE_TURBO);

	if (!turbo_cap && turbo_en) {
		/* Unavailable */
		turbo_state = TURBO_UNAVAILABLE;
	} else if (!turbo_cap && !turbo_en) {
		/* Available but disabled */
		turbo_state = TURBO_DISABLED;
	} else if (turbo_cap && turbo_en) {
		/* Available */
		turbo_state = TURBO_ENABLED;
	}

	set_global_turbo_state(turbo_state);
	debug("Turbo is %s\n", turbo_state_desc[turbo_state]);
	return turbo_state;
}

void turbo_enable(void)
{
	msr_t msr;

	/* Only possible if turbo is available but hidden */
	if (turbo_get_state() == TURBO_DISABLED) {
		/* Clear Turbo Disable bit in Misc Enables */
		msr = msr_read(MSR_IA32_MISC_ENABLES);
		msr.hi &= ~H_MISC_DISABLE_TURBO;
		msr_write(MSR_IA32_MISC_ENABLES, msr);

		/* Update cached turbo state */
		set_global_turbo_state(TURBO_ENABLED);
		debug("Turbo has been enabled\n");
	}
}
