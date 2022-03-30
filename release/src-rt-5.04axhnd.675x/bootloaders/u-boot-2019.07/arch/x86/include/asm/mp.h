/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Google, Inc
 *
 * Taken from coreboot file of the same name
 */

#ifndef _X86_MP_H_
#define _X86_MP_H_

#include <asm/atomic.h>

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
 * The mp_params structure provides the arguments to the mp subsystem
 * for bringing up APs.
 *
 * At present this is overkill for U-Boot, but it may make it easier to add
 * SMM support.
 */
struct mp_params {
	int parallel_microcode_load;
	const void *microcode_pointer;
	/* Flight plan  for APs and BSP */
	struct mp_flight_record *flight_plan;
	int num_records;
};

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
int mp_init(struct mp_params *params);

/* Probes the CPU device */
int mp_init_cpu(struct udevice *cpu, void *unused);

/* Set up additional CPUs */
int x86_mp_init(void);

#endif /* _X86_MP_H_ */
