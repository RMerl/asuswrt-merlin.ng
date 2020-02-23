#ifdef CONFIG_HOTPLUG_CPU
#include <linux/module.h>
#include <linux/kernel_stat.h>
#include <linux/kthread.h>
#include <linux/tick.h>
#include <linux/cpu.h>

static struct cpuoff_stat {
	u64 idle_prev;		// previous idle-time since boot
	u64 idle_curr;		// current idle-time since boot
	int idle_cnt;		// consecutive idle interval count
	int busy_cnt;		// consecutive busy interval count
} cpuoff_stat[NR_CPUS];

static DECLARE_BITMAP(allow_bits, CONFIG_NR_CPUS) = CPU_BITS_ALL;
// cpumask eligible for power-down
static struct cpumask *allow_mask = to_cpumask(allow_bits);
module_param_array(allow_bits, ulong, 0, 0640);
static unsigned busy = 50;	// how close to busy (ms)
module_param(busy, uint, 0640);
static unsigned busy_run = 1;	// consecutive busy slots before power-up
module_param(busy_run, uint, 0640);
static unsigned debug = 0;	// debug verbosity
module_param(debug, uint, 0640);
static unsigned enable = 0;	// enable/disable operation
module_param(enable, uint, 0640);
static unsigned epsilon = 5;	// how close to full slice (ms)
module_param(epsilon, uint, 0640);
static unsigned period = 100;	// delay between idle checks (ms)
module_param(period, uint, 0640);
static unsigned idle_run = 100;	// consecutive idle slots before power-down
module_param(idle_run, uint, 0640);

// update sysfs cpu offline state
static void cpuoff_offline(int cpu, int state)
{
	struct device *dev = get_cpu_device(cpu);
	dev->offline = state;
}

static void cpuoff_update(void)
{
	static ktime_t then;
	unsigned slice;
	ktime_t now;
	int cpu;

	for_each_online_cpu(cpu) {
		cpuoff_stat[cpu].idle_prev = cpuoff_stat[cpu].idle_curr;
#ifdef CONFIG_NO_HZ_COMMON
		cpuoff_stat[cpu].idle_curr = get_cpu_idle_time_us(cpu, 0);
#else
		cpuoff_stat[cpu].idle_curr = cputime_to_usecs(
			kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE] +
			kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT]);
#endif
	}

	now = ktime_get();
	slice = ktime_to_ms(ktime_sub(now, then));
	then = now;

	// find online cpus that were entirely idle for the last time-slice
	for_each_online_cpu(cpu) {
		struct cpuoff_stat *stat = &cpuoff_stat[cpu];
		unsigned idled = stat->idle_curr - stat->idle_prev;
		int core;

		idled /= 1000; // us->ms
		if (cpu == 0) {
			// if cpu0 idleness < `busy` then power-up 1st offline core
			if (idled <= busy) {
				if (debug)
					printk("%s/%d: cpu%d idle %u slice %u\n", __func__,
						stat->busy_cnt, cpu, idled, slice);
				if (++stat->busy_cnt < busy_run)
					continue;
				stat->busy_cnt = 0;
				for_each_cpu(core, allow_mask) {
					if (cpu_is_offline(core) && cpu_present(core)) {
						if (cpu_up(core) == 0)
							cpuoff_offline(core, 0);
						break;
					}
				}
				return; // done if cpu0 busy
			}
			stat->busy_cnt = 0;
		} else if (idled + epsilon >= slice && idled <= 2 * slice) {
			// cpu was idle for almost entire time-slice
			// (paranoid check for less than 2 time-slices
			//  so we don't power-down a cpu unnecessarily)
			if (debug > 1)
				printk("%s/%d: cpu%d idle %u slice %u\n", __func__,
					stat->idle_cnt, cpu, idled, slice);
			if (cpumask_test_cpu(cpu, allow_mask) &&
				++stat->idle_cnt >= idle_run) {
				stat->idle_cnt = 0;
				if (cpu_down(cpu) == 0)
					cpuoff_offline(cpu, 1);
			}
		} else {
			if (debug > 1 && stat->idle_cnt)
				printk("%s: cpu%d idle %u slice %u\n", __func__,
					cpu, idled, slice);
			stat->idle_cnt = 0;
		}
	}
}

static int cpuoff_thread(void *arg)
{
	DECLARE_WAIT_QUEUE_HEAD(cpuoff_wq);

	if (debug)
		printk("%s: allow %*pbl idle_run %u period %u epsilon %u\n", __func__,
			CONFIG_NR_CPUS, allow_mask, idle_run, period, epsilon);
	for (;;) {
		unsigned long timeout = msecs_to_jiffies(period);

		if (wait_event_interruptible_timeout(cpuoff_wq, kthread_should_stop(), timeout))
			break;
		if (enable)
			cpuoff_update();
	}

	return 0;
}

static struct task_struct *cpuoff_ts;

static int __init cpuoff_init(void)
{
	struct sched_param sch = { .sched_priority = 5 };

	cpuoff_ts = kthread_create(cpuoff_thread, 0, "koffd");
	if (IS_ERR(cpuoff_ts))
		return PTR_ERR(cpuoff_ts);

	sched_setscheduler(cpuoff_ts, SCHED_RR, &sch);
	kthread_bind(cpuoff_ts, 0); // monitor from cpu0
	wake_up_process(cpuoff_ts);
	return 0;
}

static void __exit cpuoff_exit(void)
{
	int core;

	kthread_stop(cpuoff_ts);

	// power-on all cpus?
	for_each_cpu(core, allow_mask) {
		if (cpu_is_offline(core) && cpu_present(core)) {
			if (cpu_up(core) == 0)
				cpuoff_offline(core, 0);
		}
	}
}

module_init(cpuoff_init);
module_exit(cpuoff_exit);

MODULE_DESCRIPTION("CPU off scheduler");
MODULE_LICENSE("GPL");
#endif
