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
#include <malloc.h>
#include <qfw.h>
#include <asm/atomic.h>
#include <asm/cpu.h>
#include <asm/interrupt.h>
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
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

/* Total CPUs include BSP */
static int num_cpus;

/* This also needs to match the sipi.S assembly code for saved MSR encoding */
struct saved_msr {
	uint32_t index;
	uint32_t lo;
	uint32_t hi;
} __packed;


struct mp_flight_plan {
	int num_records;
	struct mp_flight_record *records;
};

static struct mp_flight_plan mp_info;

struct cpu_map {
	struct udevice *dev;
	int apic_id;
	int err_code;
};

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
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);

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

	/* Walk the flight plan */
	ap_do_flight_plan(dev);

	/* Park the AP */
	debug("parking\n");
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

static int start_aps(int ap_count, atomic_t *num_aps)
{
	int sipi_vector;
	/* Max location is 4KiB below 1MiB */
	const int max_vector_loc = ((1 << 20) - (1 << 12)) >> 12;

	if (ap_count == 0)
		return 0;

	/* The vector is sent as a 4k aligned address in one byte */
	sipi_vector = AP_DEFAULT_BASE >> 12;

	if (sipi_vector > max_vector_loc) {
		printf("SIPI vector too large! 0x%08x\n",
		       sipi_vector);
		return -ENOSPC;
	}

	debug("Attempting to start %d APs\n", ap_count);

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
	wait_for_aps(num_aps, ap_count, 200, 15);

	/* Send 2nd SIPI */
	if (apic_wait_timeout(1000, "ICR not to be busy"))
		return -ETIMEDOUT;

	lapic_write(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(0));
	lapic_write(LAPIC_ICR, LAPIC_DEST_ALLBUT | LAPIC_INT_ASSERT |
		    LAPIC_DM_STARTUP | sipi_vector);
	if (apic_wait_timeout(10000, "second SIPI to complete"))
		return -ETIMEDOUT;

	/* Wait for CPUs to check in */
	if (wait_for_aps(num_aps, ap_count, 10000, 50)) {
		debug("Not all APs checked in: %d/%d\n",
		      atomic_read(num_aps), ap_count);
		return -EIO;
	}

	return 0;
}

static int bsp_do_flight_plan(struct udevice *cpu, struct mp_params *mp_params)
{
	int i;
	int ret = 0;
	const int timeout_us = 100000;
	const int step_us = 100;
	int num_aps = num_cpus - 1;

	for (i = 0; i < mp_params->num_records; i++) {
		struct mp_flight_record *rec = &mp_params->flight_plan[i];

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

static int init_bsp(struct udevice **devp)
{
	char processor_name[CPU_MAX_NAME_LEN];
	int apic_id;
	int ret;

	cpu_get_name(processor_name);
	debug("CPU: %s\n", processor_name);

	apic_id = lapicid();
	ret = find_cpu_by_apic_id(apic_id, devp);
	if (ret) {
		printf("Cannot find boot CPU, APIC ID %d\n", apic_id);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_QFW
static int qemu_cpu_fixup(void)
{
	int ret;
	int cpu_num;
	int cpu_online;
	struct udevice *dev, *pdev;
	struct cpu_platdata *plat;
	char *cpu;

	/* first we need to find '/cpus' */
	for (device_find_first_child(dm_root(), &pdev);
	     pdev;
	     device_find_next_child(&pdev)) {
		if (!strcmp(pdev->name, "cpus"))
			break;
	}
	if (!pdev) {
		printf("unable to find cpus device\n");
		return -ENODEV;
	}

	/* calculate cpus that are already bound */
	cpu_num = 0;
	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		cpu_num++;
	}

	/* get actual cpu number */
	cpu_online = qemu_fwcfg_online_cpus();
	if (cpu_online < 0) {
		printf("unable to get online cpu number: %d\n", cpu_online);
		return cpu_online;
	}

	/* bind addtional cpus */
	dev = NULL;
	for (; cpu_num < cpu_online; cpu_num++) {
		/*
		 * allocate device name here as device_bind_driver() does
		 * not copy device name, 8 bytes are enough for
		 * sizeof("cpu@") + 3 digits cpu number + '\0'
		 */
		cpu = malloc(8);
		if (!cpu) {
			printf("unable to allocate device name\n");
			return -ENOMEM;
		}
		sprintf(cpu, "cpu@%d", cpu_num);
		ret = device_bind_driver(pdev, "cpu_qemu", cpu, &dev);
		if (ret) {
			printf("binding cpu@%d failed: %d\n", cpu_num, ret);
			return ret;
		}
		plat = dev_get_parent_platdata(dev);
		plat->cpu_id = cpu_num;
	}
	return 0;
}
#endif

int mp_init(struct mp_params *p)
{
	int num_aps;
	atomic_t *ap_count;
	struct udevice *cpu;
	int ret;

	/* This will cause the CPUs devices to be bound */
	struct uclass *uc;
	ret = uclass_get(UCLASS_CPU, &uc);
	if (ret)
		return ret;

#ifdef CONFIG_QFW
	ret = qemu_cpu_fixup();
	if (ret)
		return ret;
#endif

	ret = init_bsp(&cpu);
	if (ret) {
		debug("Cannot init boot CPU: err=%d\n", ret);
		return ret;
	}

	if (p == NULL || p->flight_plan == NULL || p->num_records < 1) {
		printf("Invalid MP parameters\n");
		return -EINVAL;
	}

	num_cpus = cpu_get_count(cpu);
	if (num_cpus < 0) {
		debug("Cannot get number of CPUs: err=%d\n", num_cpus);
		return num_cpus;
	}

	if (num_cpus < 2)
		debug("Warning: Only 1 CPU is detected\n");

	ret = check_cpu_devices(num_cpus);
	if (ret)
		debug("Warning: Device tree does not describe all CPUs. Extra ones will not be started correctly\n");

	/* Copy needed parameters so that APs have a reference to the plan */
	mp_info.num_records = p->num_records;
	mp_info.records = p->flight_plan;

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
	ret = bsp_do_flight_plan(cpu, p);
	if (ret) {
		debug("CPU init failed: err=%d\n", ret);
		return ret;
	}

	return 0;
}

int mp_init_cpu(struct udevice *cpu, void *unused)
{
	struct cpu_platdata *plat = dev_get_parent_platdata(cpu);

	/*
	 * Multiple APs are brought up simultaneously and they may get the same
	 * seq num in the uclass_resolve_seq() during device_probe(). To avoid
	 * this, set req_seq to the reg number in the device tree in advance.
	 */
	cpu->req_seq = fdtdec_get_int(gd->fdt_blob, dev_of_offset(cpu), "reg",
				      -1);
	plat->ucode_version = microcode_read_rev();
	plat->device_id = gd->arch.x86_device;

	return device_probe(cpu);
}
