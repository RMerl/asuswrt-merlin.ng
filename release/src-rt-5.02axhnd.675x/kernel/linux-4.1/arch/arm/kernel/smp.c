/*
 *  linux/arch/arm/kernel/smp.c
 *
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/cache.h>
#include <linux/profile.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/percpu.h>
#include <linux/clockchips.h>
#include <linux/completion.h>
#include <linux/cpufreq.h>
#include <linux/irq_work.h>

#include <linux/atomic.h>
#if defined(CONFIG_BCM_KF_SPECTRE_PATCH) && defined(CONFIG_BCM_SPECTRE_PATCH_ENABLE)
#include <asm/bugs.h>
#endif
#include <asm/smp.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/cputype.h>
#include <asm/exception.h>
#include <asm/idmap.h>
#include <asm/topology.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/processor.h>
#include <asm/sections.h>
#include <asm/tlbflush.h>
#include <asm/ptrace.h>
#include <asm/smp_plat.h>
#include <asm/virt.h>
#include <asm/mach/arch.h>
#include <asm/mpu.h>

#define CREATE_TRACE_POINTS
#include <trace/events/ipi.h>

/*
 * as from 2.5, kernels no longer have an init_tasks structure
 * so we need some other way of telling a new secondary core
 * where to place its SVC stack
 */
struct secondary_data secondary_data;

/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
volatile int pen_release = -1;

enum ipi_msg_type {
	IPI_WAKEUP,
	IPI_TIMER,
	IPI_RESCHEDULE,
	IPI_CALL_FUNC,
	IPI_CALL_FUNC_SINGLE,
	IPI_CPU_STOP,
	IPI_IRQ_WORK,
	IPI_COMPLETION,
};

static DECLARE_COMPLETION(cpu_running);

static struct smp_operations smp_ops;

void __init smp_set_ops(struct smp_operations *ops)
{
	if (ops)
		smp_ops = *ops;
};

static unsigned long get_arch_pgd(pgd_t *pgd)
{
	phys_addr_t pgdir = virt_to_idmap(pgd);
	BUG_ON(pgdir & ARCH_PGD_MASK);
	return pgdir >> ARCH_PGD_SHIFT;
}

int __cpu_up(unsigned int cpu, struct task_struct *idle)
{
	int ret;

	if (!smp_ops.smp_boot_secondary)
		return -ENOSYS;

	/*
	 * We need to tell the secondary core where to find
	 * its stack and the page tables.
	 */
	secondary_data.stack = task_stack_page(idle) + THREAD_START_SP;
#ifdef CONFIG_ARM_MPU
	secondary_data.mpu_rgn_szr = mpu_rgn_info.rgns[MPU_RAM_REGION].drsr;
#endif

#ifdef CONFIG_MMU
	secondary_data.pgdir = get_arch_pgd(idmap_pgd);
	secondary_data.swapper_pg_dir = get_arch_pgd(swapper_pg_dir);
#endif
	sync_cache_w(&secondary_data);

	/*
	 * Now bring the CPU into our world.
	 */
	ret = smp_ops.smp_boot_secondary(cpu, idle);
	if (ret == 0) {
		/*
		 * CPU was successfully started, wait for it
		 * to come online or time out.
		 */
		wait_for_completion_timeout(&cpu_running,
						 msecs_to_jiffies(1000));

		if (!cpu_online(cpu)) {
			pr_crit("CPU%u: failed to come online\n", cpu);
			ret = -EIO;
		}
	} else {
		pr_err("CPU%u: failed to boot: %d\n", cpu, ret);
	}


	memset(&secondary_data, 0, sizeof(secondary_data));
	return ret;
}

/* platform specific SMP operations */
void __init smp_init_cpus(void)
{
	if (smp_ops.smp_init_cpus)
		smp_ops.smp_init_cpus();
}

int platform_can_secondary_boot(void)
{
	return !!smp_ops.smp_boot_secondary;
}

int platform_can_cpu_hotplug(void)
{
#ifdef CONFIG_HOTPLUG_CPU
	if (smp_ops.cpu_kill)
		return 1;
#endif

	return 0;
}

#ifdef CONFIG_HOTPLUG_CPU
static int platform_cpu_kill(unsigned int cpu)
{
	if (smp_ops.cpu_kill)
		return smp_ops.cpu_kill(cpu);
	return 1;
}

static int platform_cpu_disable(unsigned int cpu)
{
	if (smp_ops.cpu_disable)
		return smp_ops.cpu_disable(cpu);

	/*
	 * By default, allow disabling all CPUs except the first one,
	 * since this is special on a lot of platforms, e.g. because
	 * of clock tick interrupts.
	 */
	return cpu == 0 ? -EPERM : 0;
}
/*
 * __cpu_disable runs on the processor to be shutdown.
 */
int __cpu_disable(void)
{
	unsigned int cpu = smp_processor_id();
	int ret;

	ret = platform_cpu_disable(cpu);
	if (ret)
		return ret;

	/*
	 * Take this CPU offline.  Once we clear this, we can't return,
	 * and we must not schedule until we're ready to give up the cpu.
	 */
	set_cpu_online(cpu, false);

	/*
	 * OK - migrate IRQs away from this CPU
	 */
	migrate_irqs();

	/*
	 * Flush user cache and TLB mappings, and then remove this CPU
	 * from the vm mask set of all processes.
	 *
	 * Caches are flushed to the Level of Unification Inner Shareable
	 * to write-back dirty lines to unified caches shared by all CPUs.
	 */
	flush_cache_louis();
	local_flush_tlb_all();

	clear_tasks_mm_cpumask(cpu);

	return 0;
}

static DECLARE_COMPLETION(cpu_died);

/*
 * called on the thread which is asking for a CPU to be shutdown -
 * waits until shutdown has completed, or it is timed out.
 */
void __cpu_die(unsigned int cpu)
{
	if (!wait_for_completion_timeout(&cpu_died, msecs_to_jiffies(5000))) {
		pr_err("CPU%u: cpu didn't die\n", cpu);
		return;
	}
	pr_notice("CPU%u: shutdown\n", cpu);

	/*
	 * platform_cpu_kill() is generally expected to do the powering off
	 * and/or cutting of clocks to the dying CPU.  Optionally, this may
	 * be done by the CPU which is dying in preference to supporting
	 * this call, but that means there is _no_ synchronisation between
	 * the requesting CPU and the dying CPU actually losing power.
	 */
	if (!platform_cpu_kill(cpu))
		pr_err("CPU%u: unable to kill\n", cpu);
}

/*
 * Called from the idle thread for the CPU which has been shutdown.
 *
 * Note that we disable IRQs here, but do not re-enable them
 * before returning to the caller. This is also the behaviour
 * of the other hotplug-cpu capable cores, so presumably coming
 * out of idle fixes this.
 */
void __ref cpu_die(void)
{
	unsigned int cpu = smp_processor_id();

	idle_task_exit();

	local_irq_disable();

	/*
	 * Flush the data out of the L1 cache for this CPU.  This must be
	 * before the completion to ensure that data is safely written out
	 * before platform_cpu_kill() gets called - which may disable
	 * *this* CPU and power down its cache.
	 */
	flush_cache_louis();

	/*
	 * Tell __cpu_die() that this CPU is now safe to dispose of.  Once
	 * this returns, power and/or clocks can be removed at any point
	 * from this CPU and its cache by platform_cpu_kill().
	 */
	complete(&cpu_died);

	/*
	 * Ensure that the cache lines associated with that completion are
	 * written out.  This covers the case where _this_ CPU is doing the
	 * powering down, to ensure that the completion is visible to the
	 * CPU waiting for this one.
	 */
	flush_cache_louis();

	/*
	 * The actual CPU shutdown procedure is at least platform (if not
	 * CPU) specific.  This may remove power, or it may simply spin.
	 *
	 * Platforms are generally expected *NOT* to return from this call,
	 * although there are some which do because they have no way to
	 * power down the CPU.  These platforms are the _only_ reason we
	 * have a return path which uses the fragment of assembly below.
	 *
	 * The return path should not be used for platforms which can
	 * power off the CPU.
	 */
	if (smp_ops.cpu_die)
		smp_ops.cpu_die(cpu);

	pr_warn("CPU%u: smp_ops.cpu_die() returned, trying to resuscitate\n",
		cpu);

	/*
	 * Do not return to the idle loop - jump back to the secondary
	 * cpu initialisation.  There's some initialisation which needs
	 * to be repeated to undo the effects of taking the CPU offline.
	 */
	__asm__("mov	sp, %0\n"
	"	mov	fp, #0\n"
	"	b	secondary_start_kernel"
		:
		: "r" (task_stack_page(current) + THREAD_SIZE - 8));
}
#endif /* CONFIG_HOTPLUG_CPU */

/*
 * Called by both boot and secondaries to move global data into
 * per-processor storage.
 */
static void smp_store_cpu_info(unsigned int cpuid)
{
	struct cpuinfo_arm *cpu_info = &per_cpu(cpu_data, cpuid);

	cpu_info->loops_per_jiffy = loops_per_jiffy;
	cpu_info->cpuid = read_cpuid_id();

	store_cpu_topology(cpuid);
}

/*
 * This is the secondary CPU boot entry.  We're using this CPUs
 * idle thread stack, but a set of temporary page tables.
 */
asmlinkage void secondary_start_kernel(void)
{
	struct mm_struct *mm = &init_mm;
	unsigned int cpu;

	/*
	 * The identity mapping is uncached (strongly ordered), so
	 * switch away from it before attempting any exclusive accesses.
	 */
	cpu_switch_mm(mm->pgd, mm);
	local_flush_bp_all();
	enter_lazy_tlb(mm, current);
	local_flush_tlb_all();

	/*
	 * All kernel threads share the same mm context; grab a
	 * reference and switch to it.
	 */
	cpu = smp_processor_id();
	atomic_inc(&mm->mm_count);
	current->active_mm = mm;
	cpumask_set_cpu(cpu, mm_cpumask(mm));

	cpu_init();

	pr_debug("CPU%u: Booted secondary processor\n", cpu);

	preempt_disable();
	trace_hardirqs_off();

	/*
	 * Give the platform a chance to do its own initialisation.
	 */
	if (smp_ops.smp_secondary_init)
		smp_ops.smp_secondary_init(cpu);

	notify_cpu_starting(cpu);

	calibrate_delay();

	smp_store_cpu_info(cpu);

	/*
	 * OK, now it's safe to let the boot CPU continue.  Wait for
	 * the CPU migration code to notice that the CPU is online
	 * before we continue - which happens after __cpu_up returns.
	 */
	set_cpu_online(cpu, true);
#if defined(CONFIG_BCM_KF_SPECTRE_PATCH) && defined(CONFIG_BCM_SPECTRE_PATCH_ENABLE)
	check_other_bugs();
#endif
	complete(&cpu_running);

	local_irq_enable();
	local_fiq_enable();

	/*
	 * OK, it's off to the idle thread for us
	 */
	cpu_startup_entry(CPUHP_ONLINE);
}

void __init smp_cpus_done(unsigned int max_cpus)
{
	int cpu;
	unsigned long bogosum = 0;

	for_each_online_cpu(cpu)
		bogosum += per_cpu(cpu_data, cpu).loops_per_jiffy;

	printk(KERN_INFO "SMP: Total of %d processors activated "
	       "(%lu.%02lu BogoMIPS).\n",
	       num_online_cpus(),
	       bogosum / (500000/HZ),
	       (bogosum / (5000/HZ)) % 100);

	hyp_mode_check();
}

void __init smp_prepare_boot_cpu(void)
{
	set_my_cpu_offset(per_cpu_offset(smp_processor_id()));
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned int ncores = num_possible_cpus();

	init_cpu_topology();

	smp_store_cpu_info(smp_processor_id());

	/*
	 * are we trying to boot more cores than exist?
	 */
	if (max_cpus > ncores)
		max_cpus = ncores;
	if (ncores > 1 && max_cpus) {
		/*
		 * Initialise the present map, which describes the set of CPUs
		 * actually populated at the present time. A platform should
		 * re-initialize the map in the platforms smp_prepare_cpus()
		 * if present != possible (e.g. physical hotplug).
		 */
		init_cpu_present(cpu_possible_mask);

		/*
		 * Initialise the SCU if there are more than one CPU
		 * and let them know where to start.
		 */
		if (smp_ops.smp_prepare_cpus)
			smp_ops.smp_prepare_cpus(max_cpus);
	}
}

static void (*__smp_cross_call)(const struct cpumask *, unsigned int);

void __init set_smp_cross_call(void (*fn)(const struct cpumask *, unsigned int))
{
	if (!__smp_cross_call)
		__smp_cross_call = fn;
}

static const char *ipi_types[NR_IPI] __tracepoint_string = {
#define S(x,s)	[x] = s
	S(IPI_WAKEUP, "CPU wakeup interrupts"),
	S(IPI_TIMER, "Timer broadcast interrupts"),
	S(IPI_RESCHEDULE, "Rescheduling interrupts"),
	S(IPI_CALL_FUNC, "Function call interrupts"),
	S(IPI_CALL_FUNC_SINGLE, "Single function call interrupts"),
	S(IPI_CPU_STOP, "CPU stop interrupts"),
	S(IPI_IRQ_WORK, "IRQ work interrupts"),
	S(IPI_COMPLETION, "completion interrupts"),
};

static void smp_cross_call(const struct cpumask *target, unsigned int ipinr)
{
	trace_ipi_raise(target, ipi_types[ipinr]);
	__smp_cross_call(target, ipinr);
}

void show_ipi_list(struct seq_file *p, int prec)
{
	unsigned int cpu, i;

	for (i = 0; i < NR_IPI; i++) {
		seq_printf(p, "%*s%u: ", prec - 1, "IPI", i);

		for_each_online_cpu(cpu)
			seq_printf(p, "%10u ",
				   __get_irq_stat(cpu, ipi_irqs[i]));

		seq_printf(p, " %s\n", ipi_types[i]);
	}
}

u64 smp_irq_stat_cpu(unsigned int cpu)
{
	u64 sum = 0;
	int i;

	for (i = 0; i < NR_IPI; i++)
		sum += __get_irq_stat(cpu, ipi_irqs[i]);

	return sum;
}

void arch_send_call_function_ipi_mask(const struct cpumask *mask)
{
	smp_cross_call(mask, IPI_CALL_FUNC);
}

void arch_send_wakeup_ipi_mask(const struct cpumask *mask)
{
	smp_cross_call(mask, IPI_WAKEUP);
}

void arch_send_call_function_single_ipi(int cpu)
{
	smp_cross_call(cpumask_of(cpu), IPI_CALL_FUNC_SINGLE);
}

#ifdef CONFIG_IRQ_WORK
void arch_irq_work_raise(void)
{
	if (arch_irq_work_has_interrupt())
		smp_cross_call(cpumask_of(smp_processor_id()), IPI_IRQ_WORK);
}
#endif

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
void tick_broadcast(const struct cpumask *mask)
{
	smp_cross_call(mask, IPI_TIMER);
}
#endif

static DEFINE_RAW_SPINLOCK(stop_lock);

/*
 * ipi_cpu_stop - handle IPI from smp_send_stop()
 */
static void ipi_cpu_stop(unsigned int cpu)
{
	if (system_state == SYSTEM_BOOTING ||
	    system_state == SYSTEM_RUNNING) {
		raw_spin_lock(&stop_lock);
		pr_crit("CPU%u: stopping\n", cpu);
		dump_stack();
		raw_spin_unlock(&stop_lock);
	}

	set_cpu_online(cpu, false);

	local_fiq_disable();
	local_irq_disable();

	while (1)
		cpu_relax();
}

static DEFINE_PER_CPU(struct completion *, cpu_completion);

int register_ipi_completion(struct completion *completion, int cpu)
{
	per_cpu(cpu_completion, cpu) = completion;
	return IPI_COMPLETION;
}

static void ipi_complete(unsigned int cpu)
{
	complete(per_cpu(cpu_completion, cpu));
}

/*
 * Main handler for inter-processor interrupts
 */
asmlinkage void __exception_irq_entry do_IPI(int ipinr, struct pt_regs *regs)
{
	handle_IPI(ipinr, regs);
}

void handle_IPI(int ipinr, struct pt_regs *regs)
{
	unsigned int cpu = smp_processor_id();
	struct pt_regs *old_regs = set_irq_regs(regs);

	if ((unsigned)ipinr < NR_IPI) {
		trace_ipi_entry_rcuidle(ipi_types[ipinr]);
		__inc_irq_stat(cpu, ipi_irqs[ipinr]);
	}

	switch (ipinr) {
	case IPI_WAKEUP:
		break;

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
	case IPI_TIMER:
		irq_enter();
		tick_receive_broadcast();
		irq_exit();
		break;
#endif

	case IPI_RESCHEDULE:
		scheduler_ipi();
		break;

	case IPI_CALL_FUNC:
		irq_enter();
		generic_smp_call_function_interrupt();
		irq_exit();
		break;

	case IPI_CALL_FUNC_SINGLE:
		irq_enter();
		generic_smp_call_function_single_interrupt();
		irq_exit();
		break;

	case IPI_CPU_STOP:
		irq_enter();
		ipi_cpu_stop(cpu);
		irq_exit();
		break;

#ifdef CONFIG_IRQ_WORK
	case IPI_IRQ_WORK:
		irq_enter();
		irq_work_run();
		irq_exit();
		break;
#endif

	case IPI_COMPLETION:
		irq_enter();
		ipi_complete(cpu);
		irq_exit();
		break;

	default:
		pr_crit("CPU%u: Unknown IPI message 0x%x\n",
		        cpu, ipinr);
		break;
	}

	if ((unsigned)ipinr < NR_IPI)
		trace_ipi_exit_rcuidle(ipi_types[ipinr]);
	set_irq_regs(old_regs);
}

void smp_send_reschedule(int cpu)
{
	smp_cross_call(cpumask_of(cpu), IPI_RESCHEDULE);
}

void smp_send_stop(void)
{
	unsigned long timeout;
	struct cpumask mask;

	cpumask_copy(&mask, cpu_online_mask);
	cpumask_clear_cpu(smp_processor_id(), &mask);
	if (!cpumask_empty(&mask))
		smp_cross_call(&mask, IPI_CPU_STOP);

	/* Wait up to one second for other CPUs to stop */
	timeout = USEC_PER_SEC;
	while (num_online_cpus() > 1 && timeout--)
		udelay(1);

	if (num_online_cpus() > 1)
		pr_warn("SMP: failed to stop secondary CPUs\n");
}

/*
 * not supported here
 */
int setup_profiling_timer(unsigned int multiplier)
{
	return -EINVAL;
}

#ifdef CONFIG_CPU_FREQ

static DEFINE_PER_CPU(unsigned long, l_p_j_ref);
static DEFINE_PER_CPU(unsigned long, l_p_j_ref_freq);
static unsigned long global_l_p_j_ref;
static unsigned long global_l_p_j_ref_freq;

static int cpufreq_callback(struct notifier_block *nb,
					unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	int cpu = freq->cpu;

	if (freq->flags & CPUFREQ_CONST_LOOPS)
		return NOTIFY_OK;

	if (!per_cpu(l_p_j_ref, cpu)) {
		per_cpu(l_p_j_ref, cpu) =
			per_cpu(cpu_data, cpu).loops_per_jiffy;
		per_cpu(l_p_j_ref_freq, cpu) = freq->old;
		if (!global_l_p_j_ref) {
			global_l_p_j_ref = loops_per_jiffy;
			global_l_p_j_ref_freq = freq->old;
		}
	}

	if ((val == CPUFREQ_PRECHANGE  && freq->old < freq->new) ||
	    (val == CPUFREQ_POSTCHANGE && freq->old > freq->new)) {
		loops_per_jiffy = cpufreq_scale(global_l_p_j_ref,
						global_l_p_j_ref_freq,
						freq->new);
		per_cpu(cpu_data, cpu).loops_per_jiffy =
			cpufreq_scale(per_cpu(l_p_j_ref, cpu),
					per_cpu(l_p_j_ref_freq, cpu),
					freq->new);
	}
	return NOTIFY_OK;
}

static struct notifier_block cpufreq_notifier = {
	.notifier_call  = cpufreq_callback,
};

static int __init register_cpufreq_notifier(void)
{
	return cpufreq_register_notifier(&cpufreq_notifier,
						CPUFREQ_TRANSITION_NOTIFIER);
}
core_initcall(register_cpufreq_notifier);

#endif
