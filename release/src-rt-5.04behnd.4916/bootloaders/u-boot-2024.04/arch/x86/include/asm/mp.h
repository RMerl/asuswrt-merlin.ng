/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Google, Inc
 *
 * Taken from coreboot file of the same name
 */

#ifndef _X86_MP_H_
#define _X86_MP_H_

#include <asm/atomic.h>
#include <asm/cache.h>
#include <linux/bitops.h>

struct udevice;

enum {
	/*
	 * Indicates that the function should run on all CPUs. We use a large
	 * number, above the number of real CPUs we expect to find.
	 */
	MP_SELECT_ALL	= BIT(16),

	/* Run on boot CPUs */
	MP_SELECT_BSP,

	/* Run on non-boot CPUs */
	MP_SELECT_APS,
};

typedef int (*mp_callback_t)(struct udevice *cpu, void *arg);

/*
 * A mp_flight_record details a sequence of calls for the APs to perform
 * along with the BSP to coordinate sequencing. Each flight record either
 * provides a barrier for each AP before calling the callback or the APs
 * are allowed to perform the callback without waiting. Regardless, each
 * record has the cpus_entered field incremented for each record. When
 * the BSP observes that the cpus_entered matches the number of APs
 * the bsp_call is called with bsp_arg and upon returning releases the
 * barrier allowing the APs to make further progress.
 *
 * Note that ap_call() and bsp_call() can be NULL. In the NULL case the
 * callback will just not be called.
 *
 * @barrier: Ensures that the BSP and AP don't run the flight record at the same
 *	time
 * @cpus_entered: Counts the number of APs that have run this record
 * @ap_call: Function for the APs to call
 * @ap_arg: Argument to pass to @ap_call
 * @bsp_call: Function for the BSP to call
 * @bsp_arg: Argument to pass to @bsp_call
 */
struct mp_flight_record {
	atomic_t barrier;
	atomic_t cpus_entered;
	mp_callback_t ap_call;
	void *ap_arg;
	mp_callback_t bsp_call;
	void *bsp_arg;
} __attribute__((aligned(ARCH_DMA_MINALIGN)));

#define MP_FLIGHT_RECORD(barrier_, ap_func_, ap_arg_, bsp_func_, bsp_arg_) \
	{							\
		.barrier = ATOMIC_INIT(barrier_),		\
		.cpus_entered = ATOMIC_INIT(0),			\
		.ap_call = ap_func_,				\
		.ap_arg = ap_arg_,				\
		.bsp_call = bsp_func_,				\
		.bsp_arg = bsp_arg_,				\
	}

#define MP_FR_BLOCK_APS(ap_func, ap_arg, bsp_func, bsp_arg) \
	MP_FLIGHT_RECORD(0, ap_func, ap_arg, bsp_func, bsp_arg)

#define MP_FR_NOBLOCK_APS(ap_func, ap_arg, bsp_func, bsp_arg) \
	MP_FLIGHT_RECORD(1, ap_func, ap_arg, bsp_func, bsp_arg)

/*
 * mp_init() will set up the SIPI vector and bring up the APs according to
 * mp_params. Each flight record will be executed according to the plan. Note
 * that the MP infrastructure uses SMM default area without saving it. It's
 * up to the chipset or mainboard to either e820 reserve this area or save this
 * region prior to calling mp_init() and restoring it after mp_init returns.
 *
 * At the time mp_init() is called the MTRR MSRs are mirrored into APs then
 * caching is enabled before running the flight plan.
 *
 * The MP init has the following properties:
 * 1. APs are brought up in parallel.
 * 2. The ordering of cpu number and APIC ids is not deterministic.
 *    Therefore, one cannot rely on this property or the order of devices in
 *    the device tree unless the chipset or mainboard know the APIC ids
 *    a priori.
 *
 * mp_init() returns < 0 on error, 0 on success.
 */
int mp_init(void);

/**
 * x86_mp_init() - Set up additional CPUs
 *
 * @returns < 0 on error, 0 on success.
 */
int x86_mp_init(void);

/**
 * mp_run_func() - Function to call on the AP
 *
 * @arg: Argument to pass
 */
typedef void (*mp_run_func)(void *arg);

#if CONFIG_IS_ENABLED(SMP) && !CONFIG_IS_ENABLED(X86_64)
/**
 * mp_run_on_cpus() - Run a function on one or all CPUs
 *
 * This does not return until all CPUs have completed the work
 *
 * Running on anything other than the boot CPU is only supported if
 * CONFIG_SMP_AP_WORK is enabled
 *
 * @cpu_select: CPU to run on (its dev_seq() value), or MP_SELECT_ALL for
 *	all, or MP_SELECT_BSP for BSP
 * @func: Function to run
 * @arg: Argument to pass to the function
 * Return: 0 on success, -ve on error
 */
int mp_run_on_cpus(int cpu_select, mp_run_func func, void *arg);

/**
 * mp_park_aps() - Park the APs ready for the OS
 *
 * This halts all CPUs except the main one, ready for the OS to use them
 *
 * Return: 0 if OK, -ve on error
 */
int mp_park_aps(void);

/**
 * mp_first_cpu() - Get the first CPU to process, from a selection
 *
 * This is used to iterate through selected CPUs. Call this function first, then
 * call mp_next_cpu() repeatedly (with the same @cpu_select) until it returns
 * -EFBIG.
 *
 * @cpu_select: Selected CPUs (either a CPU number or MP_SELECT_...)
 * Return: next CPU number to run on (e.g. 0)
 */
int mp_first_cpu(int cpu_select);

/**
 * mp_next_cpu() - Get the next CPU to process, from a selection
 *
 * This is used to iterate through selected CPUs. After first calling
 * mp_first_cpu() once, call this function repeatedly until it returns -EFBIG.
 *
 * The value of @cpu_select must be the same for all calls and must match the
 * value passed to mp_first_cpu(), otherwise the behaviour is undefined.
 *
 * @cpu_select: Selected CPUs (either a CPU number or MP_SELECT_...)
 * @prev_cpu: Previous value returned by mp_first_cpu()/mp_next_cpu()
 * Return: next CPU number to run on (e.g. 0)
 */
int mp_next_cpu(int cpu_select, int prev_cpu);
#else
static inline int mp_run_on_cpus(int cpu_select, mp_run_func func, void *arg)
{
	/* There is only one CPU, so just call the function here */
	func(arg);

	return 0;
}

static inline int mp_park_aps(void)
{
	/* No APs to park */

	return 0;
}

static inline int mp_first_cpu(int cpu_select)
{
	/* We cannot run on any APs, nor a selected CPU */
	return cpu_select == MP_SELECT_APS ? -EFBIG : MP_SELECT_BSP;
}

static inline int mp_next_cpu(int cpu_select, int prev_cpu)
{
	/*
	 * When MP is not enabled, there is only one CPU and we did it in
	 * mp_first_cpu()
	 */
	return -EFBIG;
}

#endif

#endif /* _X86_MP_H_ */
