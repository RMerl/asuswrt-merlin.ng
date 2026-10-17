// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 *
 * Based on code from the coreboot file of the same name
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <qfw.h>
#include <asm/atomic.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/interrupt.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/microcode.h>
#include <asm/mp.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/processor.h>
#include <asm/sipi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/delay.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Setting up multiprocessing
 *
 * See https://www.intel.com/content/www/us/en/intelligent-systems/intel-boot-loader-development-kit/minimal-intel-architecture-boot-loader-paper.html
 *
 * Note that this file refers to the boot CPU (the one U-Boot is running on) as
 * the BSP (BootStrap Processor) and the others as APs (Application Processors).
 *
 * This module works by loading some setup code into RAM at AP_DEFAULT_BASE and
 * telling each AP to execute it. The code that each AP runs is in
 * sipi_vector.S (see ap_start16) which includes a struct sipi_params at the
 * end of it. Those parameters are set up by the C code.

 * Setting up is handled by load_sipi_vector(). It inits the common block of
 * parameters (sipi_params) which tell the APs what to do. This block includes
 * microcode and the MTTRs (Memory-Type-Range Registers) from the main CPU.
 * There is also an ap_count which each AP increments as it starts up, so the
 * BSP can tell how many checked in.
 *
 * The APs are started with a SIPI (Startup Inter-Processor Interrupt) which
 * tells an AP to start executing at a particular address, in this case
 * AP_DEFAULT_BASE which contains the code copied from ap_start16. This protocol
 * is handled by start_aps().
 *
 * After being started, each AP runs the code in ap_start16, switches to 32-bit
 * mode, runs the code at ap_start, then jumps to c_handler which is ap_init().
 * This runs a very simple 'flight plan' described in mp_steps(). This sets up
 * the CPU and waits for further instructions by looking at its entry in
 * ap_callbacks[]. Note that the flight plan is only actually run for each CPU
 * in bsp_do_flight_plan(): once the BSP completes each flight record, it sets
 * mp_flight_record->barrier to 1 to allow the APs to executed the record one
 * by one.
 *
 * CPUS are numbered sequentially from 0 using the device tree:
 *
 *	cpus {
 *		bootph-all;
 *		#address-cells = <1>;
 *		#size-cells = <0>;
 *
 *		cpu@0 {
 *			bootph-all;
 *			device_type = "cpu";
 *			compatible = "intel,apl-cpu";
 *			reg = <0>;
 *			intel,apic-id = <0>;
 *		};
 *
 *		cpu@1 {
 *			device_type = "cpu";
 *			compatible = "intel,apl-cpu";
 *			reg = <1>;
 *			intel,apic-id = <2>;
 *		};
 *
 * Here the 'reg' property is the CPU number and then is placed in dev_seq(cpu)
 * so that we can index into ap_callbacks[] using that. The APIC ID is different
 * and may not be sequential (it typically is if hyperthreading is supported).
 *
 * Once APs are inited they wait in ap_wait_for_instruction() for instructions.
 * Instructions come in the form of a function to run. This logic is in
 * mp_run_on_cpus() which supports running on any one AP, all APs, just the BSP
 * or all CPUs. The BSP logic is handled directly in mp_run_on_cpus(), by
 * calling the function. For the APs, callback information is stored in a
 * single, common struct mp_callback and a pointer to this is written to each
 * AP's slot in ap_callbacks[] by run_ap_work(). All APs get the message even
 * if it is only for one of them. When an AP notices a message it checks whether
 * it should call the function (see check in ap_wait_for_instruction()) and then
 * does so if needed. After that it sets its slot to NULL to indicate it is
 * done.
 *
 * While U-Boot is running it can use mp_run_on_cpus() to run code on the APs.
 * An example of this is the 'mtrr' command which allows reading and changing
 * the MTRRs on all CPUs.
 *
 * Before U-Boot exits it calls mp_park_aps() which tells all CPUs to halt by
 * executing a 'hlt' instruction. That allows them to be used by Linux when it
 * starts up.
 */

/* This also needs to match the sipi.S assembly code for saved MSR encoding */
struct __packed saved_msr {
	uint32_t index;
	uint32_t lo;
	uint32_t hi;
};

/**
 * struct mp_flight_plan - Holds the flight plan
 *
 * @num_records: Number of flight records
 * @records: Pointer to each record
 */
struct mp_flight_plan {
	int num_records;
	struct mp_flight_record *records;
};

/**
 * struct mp_callback - Callback information for APs
 *
 * @func: Function to run
 * @arg: Argument to pass to the function
 * @logical_cpu_number: Either a CPU number (i.e. dev_seq(cpu) or a special
 *	value like MP_SELECT_BSP. It tells the AP whether it should process this
 *	callback
 */
struct mp_callback {
	mp_run_func func;
	void *arg;
	int logical_cpu_number;
};

/* Stores the flight plan so that APs can find it */
static struct mp_flight_plan mp_info;

/*
 * ap_callbacks - Callback mailbox array
 *
 * Array of callback, one entry for each available CPU, indexed by the CPU
 * number, which is dev_seq(cpu). The entry for the main CPU is never used.
 * When this is NULL, there is no pending work for the CPU to run. When
 * non-NULL it points to the mp_callback structure. This is shared between all
 * CPUs, so should only be written by the main CPU.
 */
static struct mp_callback **ap_callbacks;

static inline void barrier_wait(atomic_t *b)
{
	while (atomic_read(b) == 0)
		asm("pause");
	mfence();
}

static inline void release_barrier(atomic_t *b)
{
	mfence();
	atomic_set(b, 1);
}

static inline void stop_this_cpu(void)
{
	/* Called by an AP when it is ready to halt and wait for a new task */
	for (;;)
		cpu_hlt();
}

/* Returns 1 if timeout waiting for APs. 0 if target APs found */
static int wait_for_aps(atomic_t *val, int target, int total_delay,
			int delay_step)
{
	int timeout = 0;
	int delayed = 0;

	while (atomic_read(val) != target) {
		udelay(delay_step);
		delayed += delay_step;
		if (delayed >= total_delay) {
			timeout = 1;
			break;
		}
	}

	return timeout;
}

static void ap_do_flight_plan(struct udevice *cpu)
{
	int i;

	for (i = 0; i < mp_info.num_records; i++) {
		struct mp_flight_record *rec = &mp_info.records[i];

		atomic_inc(&rec->cpus_entered);
		barrier_wait(&rec->barrier);

		if (rec->ap_call != NULL)
			rec->ap_call(cpu, rec->ap_arg);
	}
}

static int find_cpu_by_apic_id(int apic_id, struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;
	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_plat *plat = dev_get_parent_plat(dev);

		if (plat->cpu_id == apic_id) {
			*devp = dev;
			return 0;
		}
	}

	return -ENOENT;
}

/*
 * By the time APs call ap_init() caching has been setup, and microcode has
 * been loaded
 */
static void ap_init(unsigned int cpu_index)
{
	struct udevice *dev;
	int apic_id;
	int ret;

	/* Ensure the local apic is enabled */
	enable_lapic();

	apic_id = lapicid();
	ret = find_cpu_by_apic_id(apic_id, &dev);
	if (ret) {
		debug("Unknown CPU apic_id %x\n", apic_id);
		goto done;
	}

	debug("AP: slot %d apic_id %x, dev %s\n", cpu_index, apic_id,
	      dev ? dev->name : "(apic_id not found)");

	/*
	 * Walk the flight plan, which only returns if CONFIG_SMP_AP_WORK is not
	 * enabled
	 */
	ap_do_flight_plan(dev);

done:
	stop_this_cpu();
}

static const unsigned int fixed_mtrrs[NUM_FIXED_MTRRS] = {
	MTRR_FIX_64K_00000_MSR, MTRR_FIX_16K_80000_MSR, MTRR_FIX_16K_A0000_MSR,
	MTRR_FIX_4K_C0000_MSR, MTRR_FIX_4K_C8000_MSR, MTRR_FIX_4K_D0000_MSR,
	MTRR_FIX_4K_D8000_MSR, MTRR_FIX_4K_E0000_MSR, MTRR_FIX_4K_E8000_MSR,
	MTRR_FIX_4K_F0000_MSR, MTRR_FIX_4K_F8000_MSR,
};

static inline struct saved_msr *save_msr(int index, struct saved_msr *entry)
{
	msr_t msr;

	msr = msr_read(index);
	entry->index = index;
	entry->lo = msr.lo;
	entry->hi = msr.hi;

	/* Return the next entry */
	entry++;
	return entry;
}

static int save_bsp_msrs(char *start, int size)
{
	int msr_count;
	int num_var_mtrrs;
	struct saved_msr *msr_entry;
	int i;
	msr_t msr;

	/* Determine number of MTRRs need to be saved */
	msr = msr_read(MTRR_CAP_MSR);
	num_var_mtrrs = msr.lo & 0xff;

	/* 2 * num_var_mtrrs for base and mask. +1 for IA32_MTRR_DEF_TYPE */
	msr_count = 2 * num_var_mtrrs + NUM_FIXED_MTRRS + 1;

	if ((msr_count * sizeof(struct saved_msr)) > size) {
		printf("Cannot mirror all %d msrs\n", msr_count);
		return -ENOSPC;
	}

	msr_entry = (void *)start;
	for (i = 0; i < NUM_FIXED_MTRRS; i++)
		msr_entry = save_msr(fixed_mtrrs[i], msr_entry);

	for (i = 0; i < num_var_mtrrs; i++) {
		msr_entry = save_msr(MTRR_PHYS_BASE_MSR(i), msr_entry);
		msr_entry = save_msr(MTRR_PHYS_MASK_MSR(i), msr_entry);
	}

	msr_entry = save_msr(MTRR_DEF_TYPE_MSR, msr_entry);

	return msr_count;
}

static int load_sipi_vector(atomic_t **ap_countp, int num_cpus)
{
	struct sipi_params_16bit *params16;
	struct sipi_params *params;
	static char msr_save[512];
	char *stack;
	ulong addr;
	int code_len;
	int size;
	int ret;

	/* Copy in the code */
	code_len = ap_start16_code_end - ap_start16;
	debug("Copying SIPI code to %x: %d bytes\n", AP_DEFAULT_BASE,
	      code_len);
	memcpy((void *)AP_DEFAULT_BASE, ap_start16, code_len);

	addr = AP_DEFAULT_BASE + (ulong)sipi_params_16bit - (ulong)ap_start16;
	params16 = (struct sipi_params_16bit *)addr;
	params16->ap_start = (uint32_t)ap_start;
	params16->gdt = (uint32_t)gd->arch.gdt;
	params16->gdt_limit = X86_GDT_SIZE - 1;
	debug("gdt = %x, gdt_limit = %x\n", params16->gdt, params16->gdt_limit);

	params = (struct sipi_params *)sipi_params;
	debug("SIPI 32-bit params at %p\n", params);
	params->idt_ptr = (uint32_t)x86_get_idt();

	params->stack_size = CONFIG_AP_STACK_SIZE;
	size = params->stack_size * num_cpus;
	stack = memalign(4096, size);
	if (!stack)
		return -ENOMEM;
	params->stack_top = (u32)(stack + size);
#if !defined(CONFIG_QEMU) && !defined(CONFIG_HAVE_FSP) && \
	!defined(CONFIG_INTEL_MID)
	params->microcode_ptr = ucode_base;
	debug("Microcode at %x\n", params->microcode_ptr);
#endif
	params->msr_table_ptr = (u32)msr_save;
	ret = save_bsp_msrs(msr_save, sizeof(msr_save));
	if (ret < 0)
		return ret;
	params->msr_count = ret;

	params->c_handler = (uint32_t)&ap_init;

	*ap_countp = &params->ap_count;
	atomic_set(*ap_countp, 0);
	debug("SIPI vector is ready\n");

	return 0;
}

static int check_cpu_devices(int expected_cpus)
{
	int i;

	for (i = 0; i < expected_cpus; i++) {
		struct udevice *dev;
		int ret;

		ret = uclass_find_device(UCLASS_CPU, i, &dev);
		if (ret) {
			debug("Cannot find CPU %d in device tree\n", i);
			return ret;
		}
	}

	return 0;
}

/* Returns 1 for timeout. 0 on success */
static int apic_wait_timeout(int total_delay, const char *msg)
{
	int total = 0;

	if (!(lapic_read(LAPIC_ICR) & LAPIC_ICR_BUSY))
		return 0;

	debug("Waiting for %s...", msg);
	while (lapic_read(LAPIC_ICR) & LAPIC_ICR_BUSY) {
		udelay(50);
		total += 50;
		if (total >= total_delay) {
			debug("timed out: aborting\n");
			return -ETIMEDOUT;
		}
	}
	debug("done\n");

	return 0;
}

/**
 * start_aps() - Start up the APs and count how many we find
 *
 * This is called on the boot processor to start up all the other processors
 * (here called APs).
 *
 * @num_aps: Number of APs we expect to find
 * @ap_count: Initially zero. Incremented by this function for each AP found
 * Return: 0 if all APs were set up correctly or there are none to set up,
 *	-ENOSPC if the SIPI vector is too high in memory,
 *	-ETIMEDOUT if the ICR is busy or the second SIPI fails to complete
 *	-EIO if not all APs check in correctly
 */
static int start_aps(int num_aps, atomic_t *ap_count)
{
	int sipi_vector;
	/* Max location is 4KiB below 1MiB */
	const int max_vector_loc = ((1 << 20) - (1 << 12)) >> 12;

	if (num_aps == 0)
		return 0;

	/* The vector is sent as a 4k aligned address in one byte */
	sipi_vector = AP_DEFAULT_BASE >> 12;

	if (sipi_vector > max_vector_loc) {
		printf("SIPI vector too large! 0x%08x\n",
		       sipi_vector);
		return -ENOSPC;
	}

	debug("Attempting to start %d APs\n", num_aps);

	if (apic_wait_timeout(1000, "ICR not to be busy"))
		return -ETIMEDOUT;

	/* Send INIT IPI to all but self */
	lapic_write(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(0));
	lapic_write(LAPIC_ICR, LAPIC_DEST_ALLBUT | LAPIC_INT_ASSERT |
		    LAPIC_DM_INIT);
	debug("Waiting for 10ms after sending INIT\n");
	mdelay(10);

	/* Send 1st SIPI */
	if (apic_wait_timeout(1000, "ICR not to be busy"))
		return -ETIMEDOUT;

	lapic_write(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(0));
	lapic_write(LAPIC_ICR, LAPIC_DEST_ALLBUT | LAPIC_INT_ASSERT |
		    LAPIC_DM_STARTUP | sipi_vector);
	if (apic_wait_timeout(10000, "first SIPI to complete"))
		return -ETIMEDOUT;

	/* Wait for CPUs to check in up to 200 us */
	wait_for_aps(ap_count, num_aps, 200, 15);

	/* Send 2nd SIPI */
	if (apic_wait_timeout(1000, "ICR not to be busy"))
		return -ETIMEDOUT;

	lapic_write(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(0));
	lapic_write(LAPIC_ICR, LAPIC_DEST_ALLBUT | LAPIC_INT_ASSERT |
		    LAPIC_DM_STARTUP | sipi_vector);
	if (apic_wait_timeout(10000, "second SIPI to complete"))
		return -ETIMEDOUT;

	/* Wait for CPUs to check in */
	if (wait_for_aps(ap_count, num_aps, 10000, 50)) {
		debug("Not all APs checked in: %d/%d\n",
		      atomic_read(ap_count), num_aps);
		return -EIO;
	}

	return 0;
}

/**
 * bsp_do_flight_plan() - Do the flight plan on the BSP
 *
 * This runs the flight plan on the main CPU used to boot U-Boot
 *
 * @cpu: Device for the main CPU
 * @plan: Flight plan to run
 * @num_aps: Number of APs (CPUs other than the BSP)
 * @returns 0 on success, -ETIMEDOUT if an AP failed to come up
 */
static int bsp_do_flight_plan(struct udevice *cpu, struct mp_flight_plan *plan,
			      int num_aps)
{
	int i;
	int ret = 0;
	const int timeout_us = 100000;
	const int step_us = 100;

	for (i = 0; i < plan->num_records; i++) {
		struct mp_flight_record *rec = &plan->records[i];

		/* Wait for APs if the record is not released */
		if (atomic_read(&rec->barrier) == 0) {
			/* Wait for the APs to check in */
			if (wait_for_aps(&rec->cpus_entered, num_aps,
					 timeout_us, step_us)) {
				debug("MP record %d timeout\n", i);
				ret = -ETIMEDOUT;
			}
		}

		if (rec->bsp_call != NULL)
			rec->bsp_call(cpu, rec->bsp_arg);

		release_barrier(&rec->barrier);
	}

	return ret;
}

/**
 * get_bsp() - Get information about the bootstrap processor
 *
 * @devp: If non-NULL, returns CPU device corresponding to the BSP
 * @cpu_countp: If non-NULL, returns the total number of CPUs
 * Return: CPU number of the BSP, or -ve on error. If multiprocessing is not
 *	enabled, returns 0
 */
static int get_bsp(struct udevice **devp, int *cpu_countp)
{
	char processor_name[CPU_MAX_NAME_LEN];
	struct udevice *dev;
	int apic_id;
	int ret;

	cpu_get_name(processor_name);
	debug("CPU: %s\n", processor_name);

	apic_id = lapicid();
	ret = find_cpu_by_apic_id(apic_id, &dev);
	if (ret < 0) {
		printf("Cannot find boot CPU, APIC ID %d\n", apic_id);
		return ret;
	}
	ret = cpu_get_count(dev);
	if (ret < 0)
		return log_msg_ret("count", ret);
	if (devp)
		*devp = dev;
	if (cpu_countp)
		*cpu_countp = ret;

	return dev_seq(dev) >= 0 ? dev_seq(dev) : 0;
}

/**
 * read_callback() - Read the pointer in a callback slot
 *
 * This is called by APs to read their callback slot to see if there is a
 * pointer to new instructions
 *
 * @slot: Pointer to the AP's callback slot
 * Return: value of that pointer
 */
static struct mp_callback *read_callback(struct mp_callback **slot)
{
	dmb();

	return *slot;
}

/**
 * store_callback() - Store a pointer to the callback slot
 *
 * This is called by APs to write NULL into the callback slot when they have
 * finished the work requested by the BSP.
 *
 * @slot: Pointer to the AP's callback slot
 * @val: Value to write (e.g. NULL)
 */
static void store_callback(struct mp_callback **slot, struct mp_callback *val)
{
	*slot = val;
	dmb();
}

/**
 * run_ap_work() - Run a callback on selected APs
 *
 * This writes @callback to all APs and waits for them all to acknowledge it,
 * Note that whether each AP actually calls the callback depends on the value
 * of logical_cpu_number (see struct mp_callback). The logical CPU number is
 * the CPU device's req->seq value.
 *
 * @callback: Callback information to pass to all APs
 * @bsp: CPU device for the BSP
 * @num_cpus: The number of CPUs in the system (= number of APs + 1)
 * @expire_ms: Timeout to wait for all APs to finish, in milliseconds, or 0 for
 *	no timeout
 * Return: 0 if OK, -ETIMEDOUT if one or more APs failed to respond in time
 */
static int run_ap_work(struct mp_callback *callback, struct udevice *bsp,
		       int num_cpus, uint expire_ms)
{
	int cur_cpu = dev_seq(bsp);
	int num_aps = num_cpus - 1; /* number of non-BSPs to get this message */
	int cpus_accepted;
	ulong start;
	int i;

	if (!IS_ENABLED(CONFIG_SMP_AP_WORK)) {
		printf("APs already parked. CONFIG_SMP_AP_WORK not enabled\n");
		return -ENOTSUPP;
	}

	/* Signal to all the APs to run the func. */
	for (i = 0; i < num_cpus; i++) {
		if (cur_cpu != i)
			store_callback(&ap_callbacks[i], callback);
	}
	mfence();

	/* Wait for all the APs to signal back that call has been accepted. */
	start = get_timer(0);

	do {
		mdelay(1);
		cpus_accepted = 0;

		for (i = 0; i < num_cpus; i++) {
			if (cur_cpu == i)
				continue;
			if (!read_callback(&ap_callbacks[i]))
				cpus_accepted++;
		}

		if (expire_ms && get_timer(start) >= expire_ms) {
			log(UCLASS_CPU, LOGL_CRIT,
			    "AP call expired; %d/%d CPUs accepted\n",
			    cpus_accepted, num_aps);
			return -ETIMEDOUT;
		}
	} while (cpus_accepted != num_aps);

	/* Make sure we can see any data written by the APs */
	mfence();

	return 0;
}

/**
 * ap_wait_for_instruction() - Wait for and process requests from the main CPU
 *
 * This is called by APs (here, everything other than the main boot CPU) to
 * await instructions. They arrive in the form of a function call and argument,
 * which is then called. This uses a simple mailbox with atomic read/set
 *
 * @cpu: CPU that is waiting
 * @unused: Optional argument provided by struct mp_flight_record, not used here
 * Return: Does not return
 */
static int ap_wait_for_instruction(struct udevice *cpu, void *unused)
{
	struct mp_callback lcb;
	struct mp_callback **per_cpu_slot;

	if (!IS_ENABLED(CONFIG_SMP_AP_WORK))
		return 0;

	per_cpu_slot = &ap_callbacks[dev_seq(cpu)];

	while (1) {
		struct mp_callback *cb = read_callback(per_cpu_slot);

		if (!cb) {
			asm ("pause");
			continue;
		}

		/* Copy to local variable before using the value */
		memcpy(&lcb, cb, sizeof(lcb));
		mfence();
		if (lcb.logical_cpu_number == MP_SELECT_ALL ||
		    lcb.logical_cpu_number == MP_SELECT_APS ||
		    dev_seq(cpu) == lcb.logical_cpu_number)
			lcb.func(lcb.arg);

		/* Indicate we are finished */
		store_callback(per_cpu_slot, NULL);
	}

	return 0;
}

static int mp_init_cpu(struct udevice *cpu, void *unused)
{
	struct cpu_plat *plat = dev_get_parent_plat(cpu);

	plat->ucode_version = microcode_read_rev();
	plat->device_id = gd->arch.x86_device;

	return device_probe(cpu);
}

static struct mp_flight_record mp_steps[] = {
	MP_FR_BLOCK_APS(mp_init_cpu, NULL, mp_init_cpu, NULL),
	MP_FR_BLOCK_APS(ap_wait_for_instruction, NULL, NULL, NULL),
};

int mp_run_on_cpus(int cpu_select, mp_run_func func, void *arg)
{
	struct mp_callback lcb = {
		.func = func,
		.arg = arg,
		.logical_cpu_number = cpu_select,
	};
	struct udevice *dev;
	int num_cpus;
	int ret;

	ret = get_bsp(&dev, &num_cpus);
	if (ret < 0)
		return log_msg_ret("bsp", ret);
	if (cpu_select == MP_SELECT_ALL || cpu_select == MP_SELECT_BSP ||
	    cpu_select == ret) {
		/* Run on BSP first */
		func(arg);
	}

	if (!IS_ENABLED(CONFIG_SMP_AP_WORK) ||
	    !(gd->flags & GD_FLG_SMP_READY)) {
		/* Allow use of this function on the BSP only */
		if (cpu_select == MP_SELECT_BSP || !cpu_select)
			return 0;
		return -ENOTSUPP;
	}

	/* Allow up to 1 second for all APs to finish */
	ret = run_ap_work(&lcb, dev, num_cpus, 1000 /* ms */);
	if (ret)
		return log_msg_ret("aps", ret);

	return 0;
}

static void park_this_cpu(void *unused)
{
	stop_this_cpu();
}

int mp_park_aps(void)
{
	int ret;

	ret = mp_run_on_cpus(MP_SELECT_APS, park_this_cpu, NULL);
	if (ret)
		return log_ret(ret);

	return 0;
}

int mp_first_cpu(int cpu_select)
{
	struct udevice *dev;
	int num_cpus;
	int ret;

	/*
	 * This assumes that CPUs are numbered from 0. This function tries to
	 * avoid assuming the CPU 0 is the boot CPU
	 */
	if (cpu_select == MP_SELECT_ALL)
		return 0;   /* start with the first one */

	ret = get_bsp(&dev, &num_cpus);
	if (ret < 0)
		return log_msg_ret("bsp", ret);

	/* Return boot CPU if requested */
	if (cpu_select == MP_SELECT_BSP)
		return ret;

	/* Return something other than the boot CPU, if APs requested */
	if (cpu_select == MP_SELECT_APS && num_cpus > 1)
		return ret == 0 ? 1 : 0;

	/* Try to check for an invalid value */
	if (cpu_select < 0 || cpu_select >= num_cpus)
		return -EINVAL;

	return cpu_select;  /* return the only selected one */
}

int mp_next_cpu(int cpu_select, int prev_cpu)
{
	struct udevice *dev;
	int num_cpus;
	int ret;
	int bsp;

	/* If we selected the BSP or a particular single CPU, we are done */
	if (!IS_ENABLED(CONFIG_SMP_AP_WORK) || cpu_select == MP_SELECT_BSP ||
	    cpu_select >= 0)
		return -EFBIG;

	/* Must be doing MP_SELECT_ALL or MP_SELECT_APS; return the next CPU */
	ret = get_bsp(&dev, &num_cpus);
	if (ret < 0)
		return log_msg_ret("bsp", ret);
	bsp = ret;

	/* Move to the next CPU */
	assert(prev_cpu >= 0);
	ret = prev_cpu + 1;

	/* Skip the BSP if needed */
	if (cpu_select == MP_SELECT_APS && ret == bsp)
		ret++;
	if (ret >= num_cpus)
		return -EFBIG;

	return ret;
}

int mp_init(void)
{
	int num_aps, num_cpus;
	atomic_t *ap_count;
	struct udevice *cpu;
	int ret;

	if (IS_ENABLED(CONFIG_QFW)) {
		ret = qemu_cpu_fixup();
		if (ret)
			return ret;
	}

	ret = get_bsp(&cpu, &num_cpus);
	if (ret < 0) {
		debug("Cannot init boot CPU: err=%d\n", ret);
		return ret;
	}

	if (num_cpus < 2)
		debug("Warning: Only 1 CPU is detected\n");

	ret = check_cpu_devices(num_cpus);
	if (ret)
		log_warning("Warning: Device tree does not describe all CPUs. Extra ones will not be started correctly\n");

	ap_callbacks = calloc(num_cpus, sizeof(struct mp_callback *));
	if (!ap_callbacks)
		return -ENOMEM;

	/* Copy needed parameters so that APs have a reference to the plan */
	mp_info.num_records = ARRAY_SIZE(mp_steps);
	mp_info.records = mp_steps;

	/* Load the SIPI vector */
	ret = load_sipi_vector(&ap_count, num_cpus);
	if (ap_count == NULL)
		return -ENOENT;

	/*
	 * Make sure SIPI data hits RAM so the APs that come up will see
	 * the startup code even if the caches are disabled
	 */
	wbinvd();

	/* Start the APs providing number of APs and the cpus_entered field */
	num_aps = num_cpus - 1;
	ret = start_aps(num_aps, ap_count);
	if (ret) {
		mdelay(1000);
		debug("%d/%d eventually checked in?\n", atomic_read(ap_count),
		      num_aps);
		return ret;
	}

	/* Walk the flight plan for the BSP */
	ret = bsp_do_flight_plan(cpu, &mp_info, num_aps);
	if (ret) {
		debug("CPU init failed: err=%d\n", ret);
		return ret;
	}
	gd->flags |= GD_FLG_SMP_READY;

	return 0;
}
