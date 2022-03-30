// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * TSC calibration codes are adapted from Linux kernel
 * arch/x86/kernel/tsc_msr.c and arch/x86/kernel/tsc.c
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <timer.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/i8254.h>
#include <asm/ibmpc.h>
#include <asm/msr.h>
#include <asm/u-boot-x86.h>

#define MAX_NUM_FREQS	9

#define INTEL_FAM6_SKYLAKE_MOBILE	0x4E
#define INTEL_FAM6_ATOM_GOLDMONT	0x5C /* Apollo Lake */
#define INTEL_FAM6_SKYLAKE_DESKTOP	0x5E
#define INTEL_FAM6_ATOM_GOLDMONT_X	0x5F /* Denverton */
#define INTEL_FAM6_KABYLAKE_MOBILE	0x8E
#define INTEL_FAM6_KABYLAKE_DESKTOP	0x9E

DECLARE_GLOBAL_DATA_PTR;

/*
 * native_calibrate_tsc
 * Determine TSC frequency via CPUID, else return 0.
 */
static unsigned long native_calibrate_tsc(void)
{
	struct cpuid_result tsc_info;
	unsigned int crystal_freq;

	if (gd->arch.x86_vendor != X86_VENDOR_INTEL)
		return 0;

	if (cpuid_eax(0) < 0x15)
		return 0;

	tsc_info = cpuid(0x15);

	if (tsc_info.ebx == 0 || tsc_info.eax == 0)
		return 0;

	crystal_freq = tsc_info.ecx / 1000;

	if (!crystal_freq) {
		switch (gd->arch.x86_model) {
		case INTEL_FAM6_SKYLAKE_MOBILE:
		case INTEL_FAM6_SKYLAKE_DESKTOP:
		case INTEL_FAM6_KABYLAKE_MOBILE:
		case INTEL_FAM6_KABYLAKE_DESKTOP:
			crystal_freq = 24000;	/* 24.0 MHz */
			break;
		case INTEL_FAM6_ATOM_GOLDMONT_X:
			crystal_freq = 25000;	/* 25.0 MHz */
			break;
		case INTEL_FAM6_ATOM_GOLDMONT:
			crystal_freq = 19200;	/* 19.2 MHz */
			break;
		default:
			return 0;
		}
	}

	return (crystal_freq * tsc_info.ebx / tsc_info.eax) / 1000;
}

static unsigned long cpu_mhz_from_cpuid(void)
{
	if (gd->arch.x86_vendor != X86_VENDOR_INTEL)
		return 0;

	if (cpuid_eax(0) < 0x16)
		return 0;

	return cpuid_eax(0x16);
}

/*
 * According to Intel 64 and IA-32 System Programming Guide,
 * if MSR_PERF_STAT[31] is set, the maximum resolved bus ratio can be
 * read in MSR_PLATFORM_ID[12:8], otherwise in MSR_PERF_STAT[44:40].
 * Unfortunately some Intel Atom SoCs aren't quite compliant to this,
 * so we need manually differentiate SoC families. This is what the
 * field msr_plat does.
 */
struct freq_desc {
	u8 x86_family;	/* CPU family */
	u8 x86_model;	/* model */
	/* 2: use 100MHz, 1: use MSR_PLATFORM_INFO, 0: MSR_IA32_PERF_STATUS */
	u8 msr_plat;
	u32 freqs[MAX_NUM_FREQS];
};

static struct freq_desc freq_desc_tables[] = {
	/* PNW */
	{ 6, 0x27, 0, { 0, 0, 0, 0, 0, 99840, 0, 83200, 0 } },
	/* CLV+ */
	{ 6, 0x35, 0, { 0, 133200, 0, 0, 0, 99840, 0, 83200, 0 } },
	/* TNG - Intel Atom processor Z3400 series */
	{ 6, 0x4a, 1, { 0, 100000, 133300, 0, 0, 0, 0, 0, 0 } },
	/* VLV2 - Intel Atom processor E3000, Z3600, Z3700 series */
	{ 6, 0x37, 1, { 83300, 100000, 133300, 116700, 80000, 0, 0, 0, 0 } },
	/* ANN - Intel Atom processor Z3500 series */
	{ 6, 0x5a, 1, { 83300, 100000, 133300, 100000, 0, 0, 0, 0, 0 } },
	/* AMT - Intel Atom processor X7-Z8000 and X5-Z8000 series */
	{ 6, 0x4c, 1, { 83300, 100000, 133300, 116700,
			80000, 93300, 90000, 88900, 87500 } },
	/* Ivybridge */
	{ 6, 0x3a, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};

static int match_cpu(u8 family, u8 model)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(freq_desc_tables); i++) {
		if ((family == freq_desc_tables[i].x86_family) &&
		    (model == freq_desc_tables[i].x86_model))
			return i;
	}

	return -1;
}

/* Map CPU reference clock freq ID(0-7) to CPU reference clock freq(KHz) */
#define id_to_freq(cpu_index, freq_id) \
	(freq_desc_tables[cpu_index].freqs[freq_id])

/*
 * TSC on Intel Atom SoCs capable of determining TSC frequency by MSR is
 * reliable and the frequency is known (provided by HW).
 *
 * On these platforms PIT/HPET is generally not available so calibration won't
 * work at all and there is no other clocksource to act as a watchdog for the
 * TSC, so we have no other choice than to trust it.
 *
 * Returns the TSC frequency in MHz or 0 if HW does not provide it.
 */
static unsigned long __maybe_unused cpu_mhz_from_msr(void)
{
	u32 lo, hi, ratio, freq_id, freq;
	unsigned long res;
	int cpu_index;

	if (gd->arch.x86_vendor != X86_VENDOR_INTEL)
		return 0;

	cpu_index = match_cpu(gd->arch.x86, gd->arch.x86_model);
	if (cpu_index < 0)
		return 0;

	if (freq_desc_tables[cpu_index].msr_plat) {
		rdmsr(MSR_PLATFORM_INFO, lo, hi);
		ratio = (lo >> 8) & 0xff;
	} else {
		rdmsr(MSR_IA32_PERF_STATUS, lo, hi);
		ratio = (hi >> 8) & 0x1f;
	}
	debug("Maximum core-clock to bus-clock ratio: 0x%x\n", ratio);

	if (freq_desc_tables[cpu_index].msr_plat == 2) {
		/* TODO: Figure out how best to deal with this */
		freq = 100000;
		debug("Using frequency: %u KHz\n", freq);
	} else {
		/* Get FSB FREQ ID */
		rdmsr(MSR_FSB_FREQ, lo, hi);
		freq_id = lo & 0x7;
		freq = id_to_freq(cpu_index, freq_id);
		debug("Resolved frequency ID: %u, frequency: %u KHz\n",
		      freq_id, freq);
	}

	/* TSC frequency = maximum resolved freq * maximum resolved bus ratio */
	res = freq * ratio / 1000;
	debug("TSC runs at %lu MHz\n", res);

	return res;
}

/*
 * This reads the current MSB of the PIT counter, and
 * checks if we are running on sufficiently fast and
 * non-virtualized hardware.
 *
 * Our expectations are:
 *
 *  - the PIT is running at roughly 1.19MHz
 *
 *  - each IO is going to take about 1us on real hardware,
 *    but we allow it to be much faster (by a factor of 10) or
 *    _slightly_ slower (ie we allow up to a 2us read+counter
 *    update - anything else implies a unacceptably slow CPU
 *    or PIT for the fast calibration to work.
 *
 *  - with 256 PIT ticks to read the value, we have 214us to
 *    see the same MSB (and overhead like doing a single TSC
 *    read per MSB value etc).
 *
 *  - We're doing 2 reads per loop (LSB, MSB), and we expect
 *    them each to take about a microsecond on real hardware.
 *    So we expect a count value of around 100. But we'll be
 *    generous, and accept anything over 50.
 *
 *  - if the PIT is stuck, and we see *many* more reads, we
 *    return early (and the next caller of pit_expect_msb()
 *    then consider it a failure when they don't see the
 *    next expected value).
 *
 * These expectations mean that we know that we have seen the
 * transition from one expected value to another with a fairly
 * high accuracy, and we didn't miss any events. We can thus
 * use the TSC value at the transitions to calculate a pretty
 * good value for the TSC frequencty.
 */
static inline int pit_verify_msb(unsigned char val)
{
	/* Ignore LSB */
	inb(0x42);
	return inb(0x42) == val;
}

static inline int pit_expect_msb(unsigned char val, u64 *tscp,
				 unsigned long *deltap)
{
	int count;
	u64 tsc = 0, prev_tsc = 0;

	for (count = 0; count < 50000; count++) {
		if (!pit_verify_msb(val))
			break;
		prev_tsc = tsc;
		tsc = rdtsc();
	}
	*deltap = rdtsc() - prev_tsc;
	*tscp = tsc;

	/*
	 * We require _some_ success, but the quality control
	 * will be based on the error terms on the TSC values.
	 */
	return count > 5;
}

/*
 * How many MSB values do we want to see? We aim for
 * a maximum error rate of 500ppm (in practice the
 * real error is much smaller), but refuse to spend
 * more than 50ms on it.
 */
#define MAX_QUICK_PIT_MS 50
#define MAX_QUICK_PIT_ITERATIONS (MAX_QUICK_PIT_MS * PIT_TICK_RATE / 1000 / 256)

static unsigned long __maybe_unused quick_pit_calibrate(void)
{
	int i;
	u64 tsc, delta;
	unsigned long d1, d2;

	/* Set the Gate high, disable speaker */
	outb((inb(0x61) & ~0x02) | 0x01, 0x61);

	/*
	 * Counter 2, mode 0 (one-shot), binary count
	 *
	 * NOTE! Mode 2 decrements by two (and then the
	 * output is flipped each time, giving the same
	 * final output frequency as a decrement-by-one),
	 * so mode 0 is much better when looking at the
	 * individual counts.
	 */
	outb(0xb0, 0x43);

	/* Start at 0xffff */
	outb(0xff, 0x42);
	outb(0xff, 0x42);

	/*
	 * The PIT starts counting at the next edge, so we
	 * need to delay for a microsecond. The easiest way
	 * to do that is to just read back the 16-bit counter
	 * once from the PIT.
	 */
	pit_verify_msb(0);

	if (pit_expect_msb(0xff, &tsc, &d1)) {
		for (i = 1; i <= MAX_QUICK_PIT_ITERATIONS; i++) {
			if (!pit_expect_msb(0xff-i, &delta, &d2))
				break;

			/*
			 * Iterate until the error is less than 500 ppm
			 */
			delta -= tsc;
			if (d1+d2 >= delta >> 11)
				continue;

			/*
			 * Check the PIT one more time to verify that
			 * all TSC reads were stable wrt the PIT.
			 *
			 * This also guarantees serialization of the
			 * last cycle read ('d2') in pit_expect_msb.
			 */
			if (!pit_verify_msb(0xfe - i))
				break;
			goto success;
		}
	}
	debug("Fast TSC calibration failed\n");
	return 0;

success:
	/*
	 * Ok, if we get here, then we've seen the
	 * MSB of the PIT decrement 'i' times, and the
	 * error has shrunk to less than 500 ppm.
	 *
	 * As a result, we can depend on there not being
	 * any odd delays anywhere, and the TSC reads are
	 * reliable (within the error).
	 *
	 * kHz = ticks / time-in-seconds / 1000;
	 * kHz = (t2 - t1) / (I * 256 / PIT_TICK_RATE) / 1000
	 * kHz = ((t2 - t1) * PIT_TICK_RATE) / (I * 256 * 1000)
	 */
	delta *= PIT_TICK_RATE;
	delta /= (i*256*1000);
	debug("Fast TSC calibration using PIT\n");
	return delta / 1000;
}

/* Get the speed of the TSC timer in MHz */
unsigned notrace long get_tbclk_mhz(void)
{
	return get_tbclk() / 1000000;
}

static ulong get_ms_timer(void)
{
	return (get_ticks() * 1000) / get_tbclk();
}

ulong get_timer(ulong base)
{
	return get_ms_timer() - base;
}

ulong notrace timer_get_us(void)
{
	return get_ticks() / get_tbclk_mhz();
}

ulong timer_get_boot_us(void)
{
	return timer_get_us();
}

void __udelay(unsigned long usec)
{
	u64 now = get_ticks();
	u64 stop;

	stop = now + usec * get_tbclk_mhz();

	while ((int64_t)(stop - get_ticks()) > 0)
#if defined(CONFIG_QEMU) && defined(CONFIG_SMP)
		/*
		 * Add a 'pause' instruction on qemu target,
		 * to give other VCPUs a chance to run.
		 */
		asm volatile("pause");
#else
		;
#endif
}

static int tsc_timer_get_count(struct udevice *dev, u64 *count)
{
	u64 now_tick = rdtsc();

	*count = now_tick - gd->arch.tsc_base;

	return 0;
}

static void tsc_timer_ensure_setup(bool early)
{
	if (gd->arch.tsc_base)
		return;
	gd->arch.tsc_base = rdtsc();

	if (!gd->arch.clock_rate) {
		unsigned long fast_calibrate;

		fast_calibrate = native_calibrate_tsc();
		if (fast_calibrate)
			goto done;

		fast_calibrate = cpu_mhz_from_cpuid();
		if (fast_calibrate)
			goto done;

		fast_calibrate = cpu_mhz_from_msr();
		if (fast_calibrate)
			goto done;

		fast_calibrate = quick_pit_calibrate();
		if (fast_calibrate)
			goto done;

		if (early)
			fast_calibrate = CONFIG_X86_TSC_TIMER_EARLY_FREQ;
		else
			return;

done:
		gd->arch.clock_rate = fast_calibrate * 1000000;
	}
}

static int tsc_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Try hardware calibration first */
	tsc_timer_ensure_setup(false);
	if (!gd->arch.clock_rate) {
		/*
		 * Use the clock frequency specified in the
		 * device tree as last resort
		 */
		if (!uc_priv->clock_rate)
			panic("TSC frequency is ZERO");
	} else {
		uc_priv->clock_rate = gd->arch.clock_rate;
	}

	return 0;
}

unsigned long notrace timer_early_get_rate(void)
{
	/*
	 * When TSC timer is used as the early timer, be warned that the timer
	 * clock rate can only be calibrated via some hardware ways. Specifying
	 * it in the device tree won't work for the early timer.
	 */
	tsc_timer_ensure_setup(true);

	return gd->arch.clock_rate;
}

u64 notrace timer_early_get_count(void)
{
	return rdtsc() - gd->arch.tsc_base;
}

static const struct timer_ops tsc_timer_ops = {
	.get_count = tsc_timer_get_count,
};

static const struct udevice_id tsc_timer_ids[] = {
	{ .compatible = "x86,tsc-timer", },
	{ }
};

U_BOOT_DRIVER(tsc_timer) = {
	.name	= "tsc_timer",
	.id	= UCLASS_TIMER,
	.of_match = tsc_timer_ids,
	.probe = tsc_timer_probe,
	.ops	= &tsc_timer_ops,
};
