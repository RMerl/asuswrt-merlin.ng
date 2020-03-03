/*
 *  linux/drivers/cpufreq/cpufreq.c
 *
 *  Copyright (C) 2001 Russell King
 *            (C) 2002 - 2003 Dominik Brodowski <linux@brodo.de>
 *            (C) 2013 Viresh Kumar <viresh.kumar@linaro.org>
 *
 *  Oct 2005 - Ashok Raj <ashok.raj@intel.com>
 *	Added handling for CPU hotplug
 *  Feb 2006 - Jacob Shin <jacob.shin@amd.com>
 *	Fix handling for CPU hotplug -- affected CPUs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/tick.h>
#include <trace/events/power.h>

/* Macros to iterate over lists */
/* Iterate over online CPUs policies */
static LIST_HEAD(cpufreq_policy_list);
#define for_each_policy(__policy)				\
	list_for_each_entry(__policy, &cpufreq_policy_list, policy_list)

/* Iterate over governors */
static LIST_HEAD(cpufreq_governor_list);
#define for_each_governor(__governor)				\
	list_for_each_entry(__governor, &cpufreq_governor_list, governor_list)

/**
 * The "cpufreq driver" - the arch- or hardware-dependent low
 * level driver of CPUFreq support, and its spinlock. This lock
 * also protects the cpufreq_cpu_data array.
 */
static struct cpufreq_driver *cpufreq_driver;
static DEFINE_PER_CPU(struct cpufreq_policy *, cpufreq_cpu_data);
static DEFINE_PER_CPU(struct cpufreq_policy *, cpufreq_cpu_data_fallback);
static DEFINE_RWLOCK(cpufreq_driver_lock);
DEFINE_MUTEX(cpufreq_governor_lock);

/* This one keeps track of the previously set governor of a removed CPU */
static DEFINE_PER_CPU(char[CPUFREQ_NAME_LEN], cpufreq_cpu_governor);

/* Flag to suspend/resume CPUFreq governors */
static bool cpufreq_suspended;

static inline bool has_target(void)
{
	return cpufreq_driver->target_index || cpufreq_driver->target;
}

/*
 * rwsem to guarantee that cpufreq driver module doesn't unload during critical
 * sections
 */
static DECLARE_RWSEM(cpufreq_rwsem);

/* internal prototypes */
static int __cpufreq_governor(struct cpufreq_policy *policy,
		unsigned int event);
static unsigned int __cpufreq_get(struct cpufreq_policy *policy);
static void handle_update(struct work_struct *work);

/**
 * Two notifier lists: the "policy" list is involved in the
 * validation process for a new CPU frequency policy; the
 * "transition" list for kernel code that needs to handle
 * changes to devices when the CPU clock speed changes.
 * The mutex locks both lists.
 */
static BLOCKING_NOTIFIER_HEAD(cpufreq_policy_notifier_list);
static struct srcu_notifier_head cpufreq_transition_notifier_list;

static bool init_cpufreq_transition_notifier_list_called;
static int __init init_cpufreq_transition_notifier_list(void)
{
	srcu_init_notifier_head(&cpufreq_transition_notifier_list);
	init_cpufreq_transition_notifier_list_called = true;
	return 0;
}
pure_initcall(init_cpufreq_transition_notifier_list);

static int off __read_mostly;
static int cpufreq_disabled(void)
{
	return off;
}
void disable_cpufreq(void)
{
	off = 1;
}
static DEFINE_MUTEX(cpufreq_governor_mutex);

bool have_governor_per_policy(void)
{
	return !!(cpufreq_driver->flags & CPUFREQ_HAVE_GOVERNOR_PER_POLICY);
}
EXPORT_SYMBOL_GPL(have_governor_per_policy);

struct kobject *get_governor_parent_kobj(struct cpufreq_policy *policy)
{
	if (have_governor_per_policy())
		return &policy->kobj;
	else
		return cpufreq_global_kobject;
}
EXPORT_SYMBOL_GPL(get_governor_parent_kobj);

static inline u64 get_cpu_idle_time_jiffy(unsigned int cpu, u64 *wall)
{
	u64 idle_time;
	u64 cur_wall_time;
	u64 busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	busy_time = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];

	idle_time = cur_wall_time - busy_time;
	if (wall)
		*wall = cputime_to_usecs(cur_wall_time);

	return cputime_to_usecs(idle_time);
}

u64 get_cpu_idle_time(unsigned int cpu, u64 *wall, int io_busy)
{
	u64 idle_time = get_cpu_idle_time_us(cpu, io_busy ? wall : NULL);

	if (idle_time == -1ULL)
		return get_cpu_idle_time_jiffy(cpu, wall);
	else if (!io_busy)
		idle_time += get_cpu_iowait_time_us(cpu, wall);

	return idle_time;
}
EXPORT_SYMBOL_GPL(get_cpu_idle_time);

/*
 * This is a generic cpufreq init() routine which can be used by cpufreq
 * drivers of SMP systems. It will do following:
 * - validate & show freq table passed
 * - set policies transition latency
 * - policy->cpus with all possible CPUs
 */
int cpufreq_generic_init(struct cpufreq_policy *policy,
		struct cpufreq_frequency_table *table,
		unsigned int transition_latency)
{
	int ret;

	ret = cpufreq_table_validate_and_show(policy, table);
	if (ret) {
		pr_err("%s: invalid frequency table: %d\n", __func__, ret);
		return ret;
	}

	policy->cpuinfo.transition_latency = transition_latency;

	/*
	 * The driver only supports the SMP configuartion where all processors
	 * share the clock and voltage and clock.
	 */
	cpumask_setall(policy->cpus);

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_generic_init);

unsigned int cpufreq_generic_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = per_cpu(cpufreq_cpu_data, cpu);

	if (!policy || IS_ERR(policy->clk)) {
		pr_err("%s: No %s associated to cpu: %d\n",
		       __func__, policy ? "clk" : "policy", cpu);
		return 0;
	}

	return clk_get_rate(policy->clk) / 1000;
}
EXPORT_SYMBOL_GPL(cpufreq_generic_get);

/* Only for cpufreq core internal use */
struct cpufreq_policy *cpufreq_cpu_get_raw(unsigned int cpu)
{
	return per_cpu(cpufreq_cpu_data, cpu);
}

struct cpufreq_policy *cpufreq_cpu_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = NULL;
	unsigned long flags;

	if (cpu >= nr_cpu_ids)
		return NULL;

	if (!down_read_trylock(&cpufreq_rwsem))
		return NULL;

	/* get the cpufreq driver */
	read_lock_irqsave(&cpufreq_driver_lock, flags);

	if (cpufreq_driver) {
		/* get the CPU */
		policy = per_cpu(cpufreq_cpu_data, cpu);
		if (policy)
			kobject_get(&policy->kobj);
	}

	read_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (!policy)
		up_read(&cpufreq_rwsem);

	return policy;
}
EXPORT_SYMBOL_GPL(cpufreq_cpu_get);

void cpufreq_cpu_put(struct cpufreq_policy *policy)
{
	kobject_put(&policy->kobj);
	up_read(&cpufreq_rwsem);
}
EXPORT_SYMBOL_GPL(cpufreq_cpu_put);

/*********************************************************************
 *            EXTERNALLY AFFECTING FREQUENCY CHANGES                 *
 *********************************************************************/

/**
 * adjust_jiffies - adjust the system "loops_per_jiffy"
 *
 * This function alters the system "loops_per_jiffy" for the clock
 * speed change. Note that loops_per_jiffy cannot be updated on SMP
 * systems as each CPU might be scaled differently. So, use the arch
 * per-CPU loops_per_jiffy value wherever possible.
 */
static void adjust_jiffies(unsigned long val, struct cpufreq_freqs *ci)
{
#ifndef CONFIG_SMP
	static unsigned long l_p_j_ref;
	static unsigned int l_p_j_ref_freq;

	if (ci->flags & CPUFREQ_CONST_LOOPS)
		return;

	if (!l_p_j_ref_freq) {
		l_p_j_ref = loops_per_jiffy;
		l_p_j_ref_freq = ci->old;
		pr_debug("saving %lu as reference value for loops_per_jiffy; freq is %u kHz\n",
			 l_p_j_ref, l_p_j_ref_freq);
	}
	if (val == CPUFREQ_POSTCHANGE && ci->old != ci->new) {
		loops_per_jiffy = cpufreq_scale(l_p_j_ref, l_p_j_ref_freq,
								ci->new);
		pr_debug("scaling loops_per_jiffy to %lu for frequency %u kHz\n",
			 loops_per_jiffy, ci->new);
	}
#endif
}

static void __cpufreq_notify_transition(struct cpufreq_policy *policy,
		struct cpufreq_freqs *freqs, unsigned int state)
{
	BUG_ON(irqs_disabled());

	if (cpufreq_disabled())
		return;

	freqs->flags = cpufreq_driver->flags;
	pr_debug("notification %u of frequency transition to %u kHz\n",
		 state, freqs->new);

	switch (state) {

	case CPUFREQ_PRECHANGE:
		/* detect if the driver reported a value as "old frequency"
		 * which is not equal to what the cpufreq core thinks is
		 * "old frequency".
		 */
		if (!(cpufreq_driver->flags & CPUFREQ_CONST_LOOPS)) {
			if ((policy) && (policy->cpu == freqs->cpu) &&
			    (policy->cur) && (policy->cur != freqs->old)) {
				pr_debug("Warning: CPU frequency is %u, cpufreq assumed %u kHz\n",
					 freqs->old, policy->cur);
				freqs->old = policy->cur;
			}
		}
		srcu_notifier_call_chain(&cpufreq_transition_notifier_list,
				CPUFREQ_PRECHANGE, freqs);
		adjust_jiffies(CPUFREQ_PRECHANGE, freqs);
		break;

	case CPUFREQ_POSTCHANGE:
		adjust_jiffies(CPUFREQ_POSTCHANGE, freqs);
		pr_debug("FREQ: %lu - CPU: %lu\n",
			 (unsigned long)freqs->new, (unsigned long)freqs->cpu);
		trace_cpu_frequency(freqs->new, freqs->cpu);
		srcu_notifier_call_chain(&cpufreq_transition_notifier_list,
				CPUFREQ_POSTCHANGE, freqs);
		if (likely(policy) && likely(policy->cpu == freqs->cpu))
			policy->cur = freqs->new;
		break;
	}
}

/**
 * cpufreq_notify_transition - call notifier chain and adjust_jiffies
 * on frequency transition.
 *
 * This function calls the transition notifiers and the "adjust_jiffies"
 * function. It is called twice on all CPU frequency changes that have
 * external effects.
 */
static void cpufreq_notify_transition(struct cpufreq_policy *policy,
		struct cpufreq_freqs *freqs, unsigned int state)
{
	for_each_cpu(freqs->cpu, policy->cpus)
		__cpufreq_notify_transition(policy, freqs, state);
}

/* Do post notifications when there are chances that transition has failed */
static void cpufreq_notify_post_transition(struct cpufreq_policy *policy,
		struct cpufreq_freqs *freqs, int transition_failed)
{
	cpufreq_notify_transition(policy, freqs, CPUFREQ_POSTCHANGE);
	if (!transition_failed)
		return;

	swap(freqs->old, freqs->new);
	cpufreq_notify_transition(policy, freqs, CPUFREQ_PRECHANGE);
	cpufreq_notify_transition(policy, freqs, CPUFREQ_POSTCHANGE);
}

void cpufreq_freq_transition_begin(struct cpufreq_policy *policy,
		struct cpufreq_freqs *freqs)
{

	/*
	 * Catch double invocations of _begin() which lead to self-deadlock.
	 * ASYNC_NOTIFICATION drivers are left out because the cpufreq core
	 * doesn't invoke _begin() on their behalf, and hence the chances of
	 * double invocations are very low. Moreover, there are scenarios
	 * where these checks can emit false-positive warnings in these
	 * drivers; so we avoid that by skipping them altogether.
	 */
	WARN_ON(!(cpufreq_driver->flags & CPUFREQ_ASYNC_NOTIFICATION)
				&& current == policy->transition_task);

wait:
	wait_event(policy->transition_wait, !policy->transition_ongoing);

	spin_lock(&policy->transition_lock);

	if (unlikely(policy->transition_ongoing)) {
		spin_unlock(&policy->transition_lock);
		goto wait;
	}

	policy->transition_ongoing = true;
	policy->transition_task = current;

	spin_unlock(&policy->transition_lock);

	cpufreq_notify_transition(policy, freqs, CPUFREQ_PRECHANGE);
}
EXPORT_SYMBOL_GPL(cpufreq_freq_transition_begin);

void cpufreq_freq_transition_end(struct cpufreq_policy *policy,
		struct cpufreq_freqs *freqs, int transition_failed)
{
	if (unlikely(WARN_ON(!policy->transition_ongoing)))
		return;

	cpufreq_notify_post_transition(policy, freqs, transition_failed);

	policy->transition_ongoing = false;
	policy->transition_task = NULL;

	wake_up(&policy->transition_wait);
}
EXPORT_SYMBOL_GPL(cpufreq_freq_transition_end);


/*********************************************************************
 *                          SYSFS INTERFACE                          *
 *********************************************************************/
static ssize_t show_boost(struct kobject *kobj,
				 struct attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", cpufreq_driver->boost_enabled);
}

static ssize_t store_boost(struct kobject *kobj, struct attribute *attr,
				  const char *buf, size_t count)
{
	int ret, enable;

	ret = sscanf(buf, "%d", &enable);
	if (ret != 1 || enable < 0 || enable > 1)
		return -EINVAL;

	if (cpufreq_boost_trigger_state(enable)) {
		pr_err("%s: Cannot %s BOOST!\n",
		       __func__, enable ? "enable" : "disable");
		return -EINVAL;
	}

	pr_debug("%s: cpufreq BOOST %s\n",
		 __func__, enable ? "enabled" : "disabled");

	return count;
}
define_one_global_rw(boost);

static struct cpufreq_governor *find_governor(const char *str_governor)
{
	struct cpufreq_governor *t;

	for_each_governor(t)
		if (!strncasecmp(str_governor, t->name, CPUFREQ_NAME_LEN))
			return t;

	return NULL;
}

/**
 * cpufreq_parse_governor - parse a governor string
 */
static int cpufreq_parse_governor(char *str_governor, unsigned int *policy,
				struct cpufreq_governor **governor)
{
	int err = -EINVAL;

	if (!cpufreq_driver)
		goto out;

	if (cpufreq_driver->setpolicy) {
		if (!strncasecmp(str_governor, "performance", CPUFREQ_NAME_LEN)) {
			*policy = CPUFREQ_POLICY_PERFORMANCE;
			err = 0;
		} else if (!strncasecmp(str_governor, "powersave",
						CPUFREQ_NAME_LEN)) {
			*policy = CPUFREQ_POLICY_POWERSAVE;
			err = 0;
		}
	} else {
		struct cpufreq_governor *t;

		mutex_lock(&cpufreq_governor_mutex);

		t = find_governor(str_governor);

		if (t == NULL) {
			int ret;

			mutex_unlock(&cpufreq_governor_mutex);
			ret = request_module("cpufreq_%s", str_governor);
			mutex_lock(&cpufreq_governor_mutex);

			if (ret == 0)
				t = find_governor(str_governor);
		}

		if (t != NULL) {
			*governor = t;
			err = 0;
		}

		mutex_unlock(&cpufreq_governor_mutex);
	}
out:
	return err;
}

/**
 * cpufreq_per_cpu_attr_read() / show_##file_name() -
 * print out cpufreq information
 *
 * Write out information from cpufreq_driver->policy[cpu]; object must be
 * "unsigned int".
 */

#define show_one(file_name, object)			\
static ssize_t show_##file_name				\
(struct cpufreq_policy *policy, char *buf)		\
{							\
	return sprintf(buf, "%u\n", policy->object);	\
}

show_one(cpuinfo_min_freq, cpuinfo.min_freq);
show_one(cpuinfo_max_freq, cpuinfo.max_freq);
show_one(cpuinfo_transition_latency, cpuinfo.transition_latency);
show_one(scaling_min_freq, min);
show_one(scaling_max_freq, max);

static ssize_t show_scaling_cur_freq(struct cpufreq_policy *policy, char *buf)
{
	ssize_t ret;

	if (cpufreq_driver && cpufreq_driver->setpolicy && cpufreq_driver->get)
		ret = sprintf(buf, "%u\n", cpufreq_driver->get(policy->cpu));
	else
		ret = sprintf(buf, "%u\n", policy->cur);
	return ret;
}

#if defined(CONFIG_BCM_KF_BCM63XX_CPUFREQ)
int cpufreq_set_policy(struct cpufreq_policy *policy,
				struct cpufreq_policy *new_policy);
#else
static int cpufreq_set_policy(struct cpufreq_policy *policy,
				struct cpufreq_policy *new_policy);
#endif

/**
 * cpufreq_per_cpu_attr_write() / store_##file_name() - sysfs write access
 */
#define store_one(file_name, object)			\
static ssize_t store_##file_name					\
(struct cpufreq_policy *policy, const char *buf, size_t count)		\
{									\
	int ret, temp;							\
	struct cpufreq_policy new_policy;				\
									\
	ret = cpufreq_get_policy(&new_policy, policy->cpu);		\
	if (ret)							\
		return -EINVAL;						\
									\
	ret = sscanf(buf, "%u", &new_policy.object);			\
	if (ret != 1)							\
		return -EINVAL;						\
									\
	temp = new_policy.object;					\
	ret = cpufreq_set_policy(policy, &new_policy);		\
	if (!ret)							\
		policy->user_policy.object = temp;			\
									\
	return ret ? ret : count;					\
}

store_one(scaling_min_freq, min);
store_one(scaling_max_freq, max);

/**
 * show_cpuinfo_cur_freq - current CPU frequency as detected by hardware
 */
static ssize_t show_cpuinfo_cur_freq(struct cpufreq_policy *policy,
					char *buf)
{
	unsigned int cur_freq = __cpufreq_get(policy);

	if (cur_freq)
		return sprintf(buf, "%u\n", cur_freq);

	return sprintf(buf, "<unknown>\n");
}

/**
 * show_scaling_governor - show the current policy for the specified CPU
 */
static ssize_t show_scaling_governor(struct cpufreq_policy *policy, char *buf)
{
	if (policy->policy == CPUFREQ_POLICY_POWERSAVE)
		return sprintf(buf, "powersave\n");
	else if (policy->policy == CPUFREQ_POLICY_PERFORMANCE)
		return sprintf(buf, "performance\n");
	else if (policy->governor)
		return scnprintf(buf, CPUFREQ_NAME_PLEN, "%s\n",
				policy->governor->name);
	return -EINVAL;
}

/**
 * store_scaling_governor - store policy for the specified CPU
 */
static ssize_t store_scaling_governor(struct cpufreq_policy *policy,
					const char *buf, size_t count)
{
	int ret;
	char	str_governor[16];
	struct cpufreq_policy new_policy;

	ret = cpufreq_get_policy(&new_policy, policy->cpu);
	if (ret)
		return ret;

	ret = sscanf(buf, "%15s", str_governor);
	if (ret != 1)
		return -EINVAL;

	if (cpufreq_parse_governor(str_governor, &new_policy.policy,
						&new_policy.governor))
		return -EINVAL;

	ret = cpufreq_set_policy(policy, &new_policy);

	policy->user_policy.policy = policy->policy;
	policy->user_policy.governor = policy->governor;

	if (ret)
		return ret;
	else
		return count;
}

/**
 * show_scaling_driver - show the cpufreq driver currently loaded
 */
static ssize_t show_scaling_driver(struct cpufreq_policy *policy, char *buf)
{
	return scnprintf(buf, CPUFREQ_NAME_PLEN, "%s\n", cpufreq_driver->name);
}

/**
 * show_scaling_available_governors - show the available CPUfreq governors
 */
static ssize_t show_scaling_available_governors(struct cpufreq_policy *policy,
						char *buf)
{
	ssize_t i = 0;
	struct cpufreq_governor *t;

	if (!has_target()) {
		i += sprintf(buf, "performance powersave");
		goto out;
	}

	for_each_governor(t) {
		if (i >= (ssize_t) ((PAGE_SIZE / sizeof(char))
		    - (CPUFREQ_NAME_LEN + 2)))
			goto out;
		i += scnprintf(&buf[i], CPUFREQ_NAME_PLEN, "%s ", t->name);
	}
out:
	i += sprintf(&buf[i], "\n");
	return i;
}

ssize_t cpufreq_show_cpus(const struct cpumask *mask, char *buf)
{
	ssize_t i = 0;
	unsigned int cpu;

	for_each_cpu(cpu, mask) {
		if (i)
			i += scnprintf(&buf[i], (PAGE_SIZE - i - 2), " ");
		i += scnprintf(&buf[i], (PAGE_SIZE - i - 2), "%u", cpu);
		if (i >= (PAGE_SIZE - 5))
			break;
	}
	i += sprintf(&buf[i], "\n");
	return i;
}
EXPORT_SYMBOL_GPL(cpufreq_show_cpus);

/**
 * show_related_cpus - show the CPUs affected by each transition even if
 * hw coordination is in use
 */
static ssize_t show_related_cpus(struct cpufreq_policy *policy, char *buf)
{
	return cpufreq_show_cpus(policy->related_cpus, buf);
}

/**
 * show_affected_cpus - show the CPUs affected by each transition
 */
static ssize_t show_affected_cpus(struct cpufreq_policy *policy, char *buf)
{
	return cpufreq_show_cpus(policy->cpus, buf);
}

static ssize_t store_scaling_setspeed(struct cpufreq_policy *policy,
					const char *buf, size_t count)
{
	unsigned int freq = 0;
	unsigned int ret;

	if (!policy->governor || !policy->governor->store_setspeed)
		return -EINVAL;

	ret = sscanf(buf, "%u", &freq);
	if (ret != 1)
		return -EINVAL;

	policy->governor->store_setspeed(policy, freq);

	return count;
}

static ssize_t show_scaling_setspeed(struct cpufreq_policy *policy, char *buf)
{
	if (!policy->governor || !policy->governor->show_setspeed)
		return sprintf(buf, "<unsupported>\n");

	return policy->governor->show_setspeed(policy, buf);
}

/**
 * show_bios_limit - show the current cpufreq HW/BIOS limitation
 */
static ssize_t show_bios_limit(struct cpufreq_policy *policy, char *buf)
{
	unsigned int limit;
	int ret;
	if (cpufreq_driver->bios_limit) {
		ret = cpufreq_driver->bios_limit(policy->cpu, &limit);
		if (!ret)
			return sprintf(buf, "%u\n", limit);
	}
	return sprintf(buf, "%u\n", policy->cpuinfo.max_freq);
}

cpufreq_freq_attr_ro_perm(cpuinfo_cur_freq, 0400);
cpufreq_freq_attr_ro(cpuinfo_min_freq);
cpufreq_freq_attr_ro(cpuinfo_max_freq);
cpufreq_freq_attr_ro(cpuinfo_transition_latency);
cpufreq_freq_attr_ro(scaling_available_governors);
cpufreq_freq_attr_ro(scaling_driver);
cpufreq_freq_attr_ro(scaling_cur_freq);
cpufreq_freq_attr_ro(bios_limit);
cpufreq_freq_attr_ro(related_cpus);
cpufreq_freq_attr_ro(affected_cpus);
cpufreq_freq_attr_rw(scaling_min_freq);
cpufreq_freq_attr_rw(scaling_max_freq);
cpufreq_freq_attr_rw(scaling_governor);
cpufreq_freq_attr_rw(scaling_setspeed);

static struct attribute *default_attrs[] = {
	&cpuinfo_min_freq.attr,
	&cpuinfo_max_freq.attr,
	&cpuinfo_transition_latency.attr,
	&scaling_min_freq.attr,
	&scaling_max_freq.attr,
	&affected_cpus.attr,
	&related_cpus.attr,
	&scaling_governor.attr,
	&scaling_driver.attr,
	&scaling_available_governors.attr,
	&scaling_setspeed.attr,
	NULL
};

#define to_policy(k) container_of(k, struct cpufreq_policy, kobj)
#define to_attr(a) container_of(a, struct freq_attr, attr)

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct cpufreq_policy *policy = to_policy(kobj);
	struct freq_attr *fattr = to_attr(attr);
	ssize_t ret;

	if (!down_read_trylock(&cpufreq_rwsem))
		return -EINVAL;

	down_read(&policy->rwsem);

	if (fattr->show)
		ret = fattr->show(policy, buf);
	else
		ret = -EIO;

	up_read(&policy->rwsem);
	up_read(&cpufreq_rwsem);

	return ret;
}

static ssize_t store(struct kobject *kobj, struct attribute *attr,
		     const char *buf, size_t count)
{
	struct cpufreq_policy *policy = to_policy(kobj);
	struct freq_attr *fattr = to_attr(attr);
	ssize_t ret = -EINVAL;

	get_online_cpus();

	if (!cpu_online(policy->cpu))
		goto unlock;

	if (!down_read_trylock(&cpufreq_rwsem))
		goto unlock;

	down_write(&policy->rwsem);

	if (fattr->store)
		ret = fattr->store(policy, buf, count);
	else
		ret = -EIO;

	up_write(&policy->rwsem);

	up_read(&cpufreq_rwsem);
unlock:
	put_online_cpus();

	return ret;
}

static void cpufreq_sysfs_release(struct kobject *kobj)
{
	struct cpufreq_policy *policy = to_policy(kobj);
	pr_debug("last reference is dropped\n");
	complete(&policy->kobj_unregister);
}

static const struct sysfs_ops sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct kobj_type ktype_cpufreq = {
	.sysfs_ops	= &sysfs_ops,
	.default_attrs	= default_attrs,
	.release	= cpufreq_sysfs_release,
};

struct kobject *cpufreq_global_kobject;
EXPORT_SYMBOL(cpufreq_global_kobject);

static int cpufreq_global_kobject_usage;

int cpufreq_get_global_kobject(void)
{
	if (!cpufreq_global_kobject_usage++)
		return kobject_add(cpufreq_global_kobject,
				&cpu_subsys.dev_root->kobj, "%s", "cpufreq");

	return 0;
}
EXPORT_SYMBOL(cpufreq_get_global_kobject);

void cpufreq_put_global_kobject(void)
{
	if (!--cpufreq_global_kobject_usage)
		kobject_del(cpufreq_global_kobject);
}
EXPORT_SYMBOL(cpufreq_put_global_kobject);

int cpufreq_sysfs_create_file(const struct attribute *attr)
{
	int ret = cpufreq_get_global_kobject();

	if (!ret) {
		ret = sysfs_create_file(cpufreq_global_kobject, attr);
		if (ret)
			cpufreq_put_global_kobject();
	}

	return ret;
}
EXPORT_SYMBOL(cpufreq_sysfs_create_file);

void cpufreq_sysfs_remove_file(const struct attribute *attr)
{
	sysfs_remove_file(cpufreq_global_kobject, attr);
	cpufreq_put_global_kobject();
}
EXPORT_SYMBOL(cpufreq_sysfs_remove_file);

/* symlink affected CPUs */
static int cpufreq_add_dev_symlink(struct cpufreq_policy *policy)
{
	unsigned int j;
	int ret = 0;

	for_each_cpu(j, policy->cpus) {
		struct device *cpu_dev;

		if (j == policy->cpu)
			continue;

		pr_debug("Adding link for CPU: %u\n", j);
		cpu_dev = get_cpu_device(j);
		ret = sysfs_create_link(&cpu_dev->kobj, &policy->kobj,
					"cpufreq");
		if (ret)
			break;
	}
	return ret;
}

static int cpufreq_add_dev_interface(struct cpufreq_policy *policy,
				     struct device *dev)
{
	struct freq_attr **drv_attr;
	int ret = 0;

	/* set up files for this cpu device */
	drv_attr = cpufreq_driver->attr;
	while (drv_attr && *drv_attr) {
		ret = sysfs_create_file(&policy->kobj, &((*drv_attr)->attr));
		if (ret)
			return ret;
		drv_attr++;
	}
	if (cpufreq_driver->get) {
		ret = sysfs_create_file(&policy->kobj, &cpuinfo_cur_freq.attr);
		if (ret)
			return ret;
	}

	ret = sysfs_create_file(&policy->kobj, &scaling_cur_freq.attr);
	if (ret)
		return ret;

	if (cpufreq_driver->bios_limit) {
		ret = sysfs_create_file(&policy->kobj, &bios_limit.attr);
		if (ret)
			return ret;
	}

	return cpufreq_add_dev_symlink(policy);
}

static void cpufreq_init_policy(struct cpufreq_policy *policy)
{
	struct cpufreq_governor *gov = NULL;
	struct cpufreq_policy new_policy;
	int ret = 0;

	memcpy(&new_policy, policy, sizeof(*policy));

	/* Update governor of new_policy to the governor used before hotplug */
	gov = find_governor(per_cpu(cpufreq_cpu_governor, policy->cpu));
	if (gov)
		pr_debug("Restoring governor %s for cpu %d\n",
				policy->governor->name, policy->cpu);
	else
		gov = CPUFREQ_DEFAULT_GOVERNOR;

	new_policy.governor = gov;

	/* Use the default policy if its valid. */
	if (cpufreq_driver->setpolicy)
		cpufreq_parse_governor(gov->name, &new_policy.policy, NULL);

	/* set default policy */
	ret = cpufreq_set_policy(policy, &new_policy);
	if (ret) {
		pr_debug("setting policy failed\n");
		if (cpufreq_driver->exit)
			cpufreq_driver->exit(policy);
	}
}

static int cpufreq_add_policy_cpu(struct cpufreq_policy *policy,
				  unsigned int cpu, struct device *dev)
{
	int ret = 0;
	unsigned long flags;

	if (has_target()) {
		ret = __cpufreq_governor(policy, CPUFREQ_GOV_STOP);
		if (ret) {
			pr_err("%s: Failed to stop governor\n", __func__);
			return ret;
		}
	}

	down_write(&policy->rwsem);

	write_lock_irqsave(&cpufreq_driver_lock, flags);

	cpumask_set_cpu(cpu, policy->cpus);
	per_cpu(cpufreq_cpu_data, cpu) = policy;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	up_write(&policy->rwsem);

	if (has_target()) {
		ret = __cpufreq_governor(policy, CPUFREQ_GOV_START);
		if (!ret)
			ret = __cpufreq_governor(policy, CPUFREQ_GOV_LIMITS);

		if (ret) {
			pr_err("%s: Failed to start governor\n", __func__);
			return ret;
		}
	}

	return sysfs_create_link(&dev->kobj, &policy->kobj, "cpufreq");
}

static struct cpufreq_policy *cpufreq_policy_restore(unsigned int cpu)
{
	struct cpufreq_policy *policy;
	unsigned long flags;

	read_lock_irqsave(&cpufreq_driver_lock, flags);

	policy = per_cpu(cpufreq_cpu_data_fallback, cpu);

	read_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (policy)
		policy->governor = NULL;

	return policy;
}

static struct cpufreq_policy *cpufreq_policy_alloc(void)
{
	struct cpufreq_policy *policy;

	policy = kzalloc(sizeof(*policy), GFP_KERNEL);
	if (!policy)
		return NULL;

	if (!alloc_cpumask_var(&policy->cpus, GFP_KERNEL))
		goto err_free_policy;

	if (!zalloc_cpumask_var(&policy->related_cpus, GFP_KERNEL))
		goto err_free_cpumask;

	INIT_LIST_HEAD(&policy->policy_list);
	init_rwsem(&policy->rwsem);
	spin_lock_init(&policy->transition_lock);
	init_waitqueue_head(&policy->transition_wait);
	init_completion(&policy->kobj_unregister);
	INIT_WORK(&policy->update, handle_update);

	return policy;

err_free_cpumask:
	free_cpumask_var(policy->cpus);
err_free_policy:
	kfree(policy);

	return NULL;
}

static void cpufreq_policy_put_kobj(struct cpufreq_policy *policy)
{
	struct kobject *kobj;
	struct completion *cmp;

	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_REMOVE_POLICY, policy);

	down_read(&policy->rwsem);
	kobj = &policy->kobj;
	cmp = &policy->kobj_unregister;
	up_read(&policy->rwsem);
	kobject_put(kobj);

	/*
	 * We need to make sure that the underlying kobj is
	 * actually not referenced anymore by anybody before we
	 * proceed with unloading.
	 */
	pr_debug("waiting for dropping of refcount\n");
	wait_for_completion(cmp);
	pr_debug("wait complete\n");
}

static void cpufreq_policy_free(struct cpufreq_policy *policy)
{
	free_cpumask_var(policy->related_cpus);
	free_cpumask_var(policy->cpus);
	kfree(policy);
}

static int update_policy_cpu(struct cpufreq_policy *policy, unsigned int cpu,
			     struct device *cpu_dev)
{
	int ret;

	if (WARN_ON(cpu == policy->cpu))
		return 0;

	/* Move kobject to the new policy->cpu */
	ret = kobject_move(&policy->kobj, &cpu_dev->kobj);
	if (ret) {
		pr_err("%s: Failed to move kobj: %d\n", __func__, ret);
		return ret;
	}

	down_write(&policy->rwsem);
	policy->cpu = cpu;
	up_write(&policy->rwsem);

	return 0;
}

static int __cpufreq_add_dev(struct device *dev, struct subsys_interface *sif)
{
	unsigned int j, cpu = dev->id;
	int ret = -ENOMEM;
	struct cpufreq_policy *policy;
	unsigned long flags;
	bool recover_policy = cpufreq_suspended;

	if (cpu_is_offline(cpu))
		return 0;

	pr_debug("adding CPU %u\n", cpu);

	/* check whether a different CPU already registered this
	 * CPU because it is in the same boat. */
	policy = cpufreq_cpu_get_raw(cpu);
	if (unlikely(policy))
		return 0;

	if (!down_read_trylock(&cpufreq_rwsem))
		return 0;

	/* Check if this cpu was hot-unplugged earlier and has siblings */
	read_lock_irqsave(&cpufreq_driver_lock, flags);
	for_each_policy(policy) {
		if (cpumask_test_cpu(cpu, policy->related_cpus)) {
			read_unlock_irqrestore(&cpufreq_driver_lock, flags);
			ret = cpufreq_add_policy_cpu(policy, cpu, dev);
			up_read(&cpufreq_rwsem);
			return ret;
		}
	}
	read_unlock_irqrestore(&cpufreq_driver_lock, flags);

	/*
	 * Restore the saved policy when doing light-weight init and fall back
	 * to the full init if that fails.
	 */
	policy = recover_policy ? cpufreq_policy_restore(cpu) : NULL;
	if (!policy) {
		recover_policy = false;
		policy = cpufreq_policy_alloc();
		if (!policy)
			goto nomem_out;
	}

	/*
	 * In the resume path, since we restore a saved policy, the assignment
	 * to policy->cpu is like an update of the existing policy, rather than
	 * the creation of a brand new one. So we need to perform this update
	 * by invoking update_policy_cpu().
	 */
	if (recover_policy && cpu != policy->cpu)
		WARN_ON(update_policy_cpu(policy, cpu, dev));
	else
		policy->cpu = cpu;

	cpumask_copy(policy->cpus, cpumask_of(cpu));

	/* call driver. From then on the cpufreq must be able
	 * to accept all calls to ->verify and ->setpolicy for this CPU
	 */
	ret = cpufreq_driver->init(policy);
	if (ret) {
		pr_debug("initialization failed\n");
		goto err_set_policy_cpu;
	}

	down_write(&policy->rwsem);

	/* related cpus should atleast have policy->cpus */
	cpumask_or(policy->related_cpus, policy->related_cpus, policy->cpus);

	/*
	 * affected cpus must always be the one, which are online. We aren't
	 * managing offline cpus here.
	 */
	cpumask_and(policy->cpus, policy->cpus, cpu_online_mask);

	if (!recover_policy) {
		policy->user_policy.min = policy->min;
		policy->user_policy.max = policy->max;

		/* prepare interface data */
		ret = kobject_init_and_add(&policy->kobj, &ktype_cpufreq,
					   &dev->kobj, "cpufreq");
		if (ret) {
			pr_err("%s: failed to init policy->kobj: %d\n",
			       __func__, ret);
			goto err_init_policy_kobj;
		}
	}

	write_lock_irqsave(&cpufreq_driver_lock, flags);
	for_each_cpu(j, policy->cpus)
		per_cpu(cpufreq_cpu_data, j) = policy;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (cpufreq_driver->get && !cpufreq_driver->setpolicy) {
		policy->cur = cpufreq_driver->get(policy->cpu);
		if (!policy->cur) {
			pr_err("%s: ->get() failed\n", __func__);
			goto err_get_freq;
		}
	}

	/*
	 * Sometimes boot loaders set CPU frequency to a value outside of
	 * frequency table present with cpufreq core. In such cases CPU might be
	 * unstable if it has to run on that frequency for long duration of time
	 * and so its better to set it to a frequency which is specified in
	 * freq-table. This also makes cpufreq stats inconsistent as
	 * cpufreq-stats would fail to register because current frequency of CPU
	 * isn't found in freq-table.
	 *
	 * Because we don't want this change to effect boot process badly, we go
	 * for the next freq which is >= policy->cur ('cur' must be set by now,
	 * otherwise we will end up setting freq to lowest of the table as 'cur'
	 * is initialized to zero).
	 *
	 * We are passing target-freq as "policy->cur - 1" otherwise
	 * __cpufreq_driver_target() would simply fail, as policy->cur will be
	 * equal to target-freq.
	 */
	if ((cpufreq_driver->flags & CPUFREQ_NEED_INITIAL_FREQ_CHECK)
	    && has_target()) {
		/* Are we running at unknown frequency ? */
		ret = cpufreq_frequency_table_get_index(policy, policy->cur);
		if (ret == -EINVAL) {
			/* Warn user and fix it */
			pr_warn("%s: CPU%d: Running at unlisted freq: %u KHz\n",
				__func__, policy->cpu, policy->cur);
			ret = __cpufreq_driver_target(policy, policy->cur - 1,
				CPUFREQ_RELATION_L);

			/*
			 * Reaching here after boot in a few seconds may not
			 * mean that system will remain stable at "unknown"
			 * frequency for longer duration. Hence, a BUG_ON().
			 */
			BUG_ON(ret);
			pr_warn("%s: CPU%d: Unlisted initial frequency changed to: %u KHz\n",
				__func__, policy->cpu, policy->cur);
		}
	}

	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
				     CPUFREQ_START, policy);

	if (!recover_policy) {
		ret = cpufreq_add_dev_interface(policy, dev);
		if (ret)
			goto err_out_unregister;
		blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
				CPUFREQ_CREATE_POLICY, policy);
	}

	write_lock_irqsave(&cpufreq_driver_lock, flags);
	list_add(&policy->policy_list, &cpufreq_policy_list);
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	cpufreq_init_policy(policy);

	if (!recover_policy) {
		policy->user_policy.policy = policy->policy;
		policy->user_policy.governor = policy->governor;
	}

	up_write(&policy->rwsem);

	kobject_uevent(&policy->kobj, KOBJ_ADD);

	up_read(&cpufreq_rwsem);

	/* Callback for handling stuff after policy is ready */
	if (cpufreq_driver->ready)
		cpufreq_driver->ready(policy);

	pr_debug("initialization complete\n");

	return 0;

err_out_unregister:
err_get_freq:
	write_lock_irqsave(&cpufreq_driver_lock, flags);
	for_each_cpu(j, policy->cpus)
		per_cpu(cpufreq_cpu_data, j) = NULL;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (!recover_policy) {
		kobject_put(&policy->kobj);
		wait_for_completion(&policy->kobj_unregister);
	}
err_init_policy_kobj:
	up_write(&policy->rwsem);

	if (cpufreq_driver->exit)
		cpufreq_driver->exit(policy);
err_set_policy_cpu:
	if (recover_policy) {
		/* Do not leave stale fallback data behind. */
		per_cpu(cpufreq_cpu_data_fallback, cpu) = NULL;
		cpufreq_policy_put_kobj(policy);
	}
	cpufreq_policy_free(policy);

nomem_out:
	up_read(&cpufreq_rwsem);

	return ret;
}

/**
 * cpufreq_add_dev - add a CPU device
 *
 * Adds the cpufreq interface for a CPU device.
 *
 * The Oracle says: try running cpufreq registration/unregistration concurrently
 * with with cpu hotplugging and all hell will break loose. Tried to clean this
 * mess up, but more thorough testing is needed. - Mathieu
 */
static int cpufreq_add_dev(struct device *dev, struct subsys_interface *sif)
{
	return __cpufreq_add_dev(dev, sif);
}

static int __cpufreq_remove_dev_prepare(struct device *dev,
					struct subsys_interface *sif)
{
	unsigned int cpu = dev->id, cpus;
	int ret;
	unsigned long flags;
	struct cpufreq_policy *policy;

	pr_debug("%s: unregistering CPU %u\n", __func__, cpu);

	write_lock_irqsave(&cpufreq_driver_lock, flags);

	policy = per_cpu(cpufreq_cpu_data, cpu);

	/* Save the policy somewhere when doing a light-weight tear-down */
	if (cpufreq_suspended)
		per_cpu(cpufreq_cpu_data_fallback, cpu) = policy;

	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (!policy) {
		pr_debug("%s: No cpu_data found\n", __func__);
		return -EINVAL;
	}

	if (has_target()) {
		ret = __cpufreq_governor(policy, CPUFREQ_GOV_STOP);
		if (ret) {
			pr_err("%s: Failed to stop governor\n", __func__);
			return ret;
		}

		strncpy(per_cpu(cpufreq_cpu_governor, cpu),
			policy->governor->name, CPUFREQ_NAME_LEN);
	}

	down_read(&policy->rwsem);
	cpus = cpumask_weight(policy->cpus);
	up_read(&policy->rwsem);

	if (cpu != policy->cpu) {
		sysfs_remove_link(&dev->kobj, "cpufreq");
	} else if (cpus > 1) {
		/* Nominate new CPU */
		int new_cpu = cpumask_any_but(policy->cpus, cpu);
		struct device *cpu_dev = get_cpu_device(new_cpu);

		sysfs_remove_link(&cpu_dev->kobj, "cpufreq");
		ret = update_policy_cpu(policy, new_cpu, cpu_dev);
		if (ret) {
			if (sysfs_create_link(&cpu_dev->kobj, &policy->kobj,
					      "cpufreq"))
				pr_err("%s: Failed to restore kobj link to cpu:%d\n",
				       __func__, cpu_dev->id);
			return ret;
		}

		if (!cpufreq_suspended)
			pr_debug("%s: policy Kobject moved to cpu: %d from: %d\n",
				 __func__, new_cpu, cpu);
	} else if (cpufreq_driver->stop_cpu) {
		cpufreq_driver->stop_cpu(policy);
	}

	return 0;
}

static int __cpufreq_remove_dev_finish(struct device *dev,
				       struct subsys_interface *sif)
{
	unsigned int cpu = dev->id, cpus;
	int ret;
	unsigned long flags;
	struct cpufreq_policy *policy;

	write_lock_irqsave(&cpufreq_driver_lock, flags);
	policy = per_cpu(cpufreq_cpu_data, cpu);
	per_cpu(cpufreq_cpu_data, cpu) = NULL;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (!policy) {
		pr_debug("%s: No cpu_data found\n", __func__);
		return -EINVAL;
	}

	down_write(&policy->rwsem);
	cpus = cpumask_weight(policy->cpus);

	if (cpus > 1)
		cpumask_clear_cpu(cpu, policy->cpus);
	up_write(&policy->rwsem);

	/* If cpu is last user of policy, free policy */
	if (cpus == 1) {
		if (has_target()) {
			ret = __cpufreq_governor(policy,
					CPUFREQ_GOV_POLICY_EXIT);
			if (ret) {
				pr_err("%s: Failed to exit governor\n",
				       __func__);
				return ret;
			}
		}

		if (!cpufreq_suspended)
			cpufreq_policy_put_kobj(policy);

		/*
		 * Perform the ->exit() even during light-weight tear-down,
		 * since this is a core component, and is essential for the
		 * subsequent light-weight ->init() to succeed.
		 */
		if (cpufreq_driver->exit)
			cpufreq_driver->exit(policy);

		/* Remove policy from list of active policies */
		write_lock_irqsave(&cpufreq_driver_lock, flags);
		list_del(&policy->policy_list);
		write_unlock_irqrestore(&cpufreq_driver_lock, flags);

		if (!cpufreq_suspended)
			cpufreq_policy_free(policy);
	} else if (has_target()) {
		ret = __cpufreq_governor(policy, CPUFREQ_GOV_START);
		if (!ret)
			ret = __cpufreq_governor(policy, CPUFREQ_GOV_LIMITS);

		if (ret) {
			pr_err("%s: Failed to start governor\n", __func__);
			return ret;
		}
	}

	return 0;
}

/**
 * cpufreq_remove_dev - remove a CPU device
 *
 * Removes the cpufreq interface for a CPU device.
 */
static int cpufreq_remove_dev(struct device *dev, struct subsys_interface *sif)
{
	unsigned int cpu = dev->id;
	int ret;

	if (cpu_is_offline(cpu))
		return 0;

	ret = __cpufreq_remove_dev_prepare(dev, sif);

	if (!ret)
		ret = __cpufreq_remove_dev_finish(dev, sif);

	return ret;
}

static void handle_update(struct work_struct *work)
{
	struct cpufreq_policy *policy =
		container_of(work, struct cpufreq_policy, update);
	unsigned int cpu = policy->cpu;
	pr_debug("handle_update for cpu %u called\n", cpu);
	cpufreq_update_policy(cpu);
}

/**
 *	cpufreq_out_of_sync - If actual and saved CPU frequency differs, we're
 *	in deep trouble.
 *	@policy: policy managing CPUs
 *	@new_freq: CPU frequency the CPU actually runs at
 *
 *	We adjust to current frequency first, and need to clean up later.
 *	So either call to cpufreq_update_policy() or schedule handle_update()).
 */
static void cpufreq_out_of_sync(struct cpufreq_policy *policy,
				unsigned int new_freq)
{
	struct cpufreq_freqs freqs;

	pr_debug("Warning: CPU frequency out of sync: cpufreq and timing core thinks of %u, is %u kHz\n",
		 policy->cur, new_freq);

	freqs.old = policy->cur;
	freqs.new = new_freq;

	cpufreq_freq_transition_begin(policy, &freqs);
	cpufreq_freq_transition_end(policy, &freqs, 0);
}

/**
 * cpufreq_quick_get - get the CPU frequency (in kHz) from policy->cur
 * @cpu: CPU number
 *
 * This is the last known freq, without actually getting it from the driver.
 * Return value will be same as what is shown in scaling_cur_freq in sysfs.
 */
unsigned int cpufreq_quick_get(unsigned int cpu)
{
	struct cpufreq_policy *policy;
	unsigned int ret_freq = 0;

	if (cpufreq_driver && cpufreq_driver->setpolicy && cpufreq_driver->get)
		return cpufreq_driver->get(cpu);

	policy = cpufreq_cpu_get(cpu);
	if (policy) {
		ret_freq = policy->cur;
		cpufreq_cpu_put(policy);
	}

	return ret_freq;
}
EXPORT_SYMBOL(cpufreq_quick_get);

/**
 * cpufreq_quick_get_max - get the max reported CPU frequency for this CPU
 * @cpu: CPU number
 *
 * Just return the max possible frequency for a given CPU.
 */
unsigned int cpufreq_quick_get_max(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
	unsigned int ret_freq = 0;

	if (policy) {
		ret_freq = policy->max;
		cpufreq_cpu_put(policy);
	}

	return ret_freq;
}
EXPORT_SYMBOL(cpufreq_quick_get_max);

static unsigned int __cpufreq_get(struct cpufreq_policy *policy)
{
	unsigned int ret_freq = 0;

	if (!cpufreq_driver->get)
		return ret_freq;

	ret_freq = cpufreq_driver->get(policy->cpu);

	if (ret_freq && policy->cur &&
		!(cpufreq_driver->flags & CPUFREQ_CONST_LOOPS)) {
		/* verify no discrepancy between actual and
					saved value exists */
		if (unlikely(ret_freq != policy->cur)) {
			cpufreq_out_of_sync(policy, ret_freq);
			schedule_work(&policy->update);
		}
	}

	return ret_freq;
}

/**
 * cpufreq_get - get the current CPU frequency (in kHz)
 * @cpu: CPU number
 *
 * Get the CPU current (static) CPU frequency
 */
unsigned int cpufreq_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
	unsigned int ret_freq = 0;

	if (policy) {
		down_read(&policy->rwsem);
		ret_freq = __cpufreq_get(policy);
		up_read(&policy->rwsem);

		cpufreq_cpu_put(policy);
	}

	return ret_freq;
}
EXPORT_SYMBOL(cpufreq_get);

static struct subsys_interface cpufreq_interface = {
	.name		= "cpufreq",
	.subsys		= &cpu_subsys,
	.add_dev	= cpufreq_add_dev,
	.remove_dev	= cpufreq_remove_dev,
};

/*
 * In case platform wants some specific frequency to be configured
 * during suspend..
 */
int cpufreq_generic_suspend(struct cpufreq_policy *policy)
{
	int ret;

	if (!policy->suspend_freq) {
		pr_err("%s: suspend_freq can't be zero\n", __func__);
		return -EINVAL;
	}

	pr_debug("%s: Setting suspend-freq: %u\n", __func__,
			policy->suspend_freq);

	ret = __cpufreq_driver_target(policy, policy->suspend_freq,
			CPUFREQ_RELATION_H);
	if (ret)
		pr_err("%s: unable to set suspend-freq: %u. err: %d\n",
				__func__, policy->suspend_freq, ret);

	return ret;
}
EXPORT_SYMBOL(cpufreq_generic_suspend);

/**
 * cpufreq_suspend() - Suspend CPUFreq governors
 *
 * Called during system wide Suspend/Hibernate cycles for suspending governors
 * as some platforms can't change frequency after this point in suspend cycle.
 * Because some of the devices (like: i2c, regulators, etc) they use for
 * changing frequency are suspended quickly after this point.
 */
void cpufreq_suspend(void)
{
	struct cpufreq_policy *policy;

	if (!cpufreq_driver)
		return;

	if (!has_target())
		goto suspend;

	pr_debug("%s: Suspending Governors\n", __func__);

	for_each_policy(policy) {
		if (__cpufreq_governor(policy, CPUFREQ_GOV_STOP))
			pr_err("%s: Failed to stop governor for policy: %p\n",
				__func__, policy);
		else if (cpufreq_driver->suspend
		    && cpufreq_driver->suspend(policy))
			pr_err("%s: Failed to suspend driver: %p\n", __func__,
				policy);
	}

suspend:
	cpufreq_suspended = true;
}

/**
 * cpufreq_resume() - Resume CPUFreq governors
 *
 * Called during system wide Suspend/Hibernate cycle for resuming governors that
 * are suspended with cpufreq_suspend().
 */
void cpufreq_resume(void)
{
	struct cpufreq_policy *policy;

	if (!cpufreq_driver)
		return;

	cpufreq_suspended = false;

	if (!has_target())
		return;

	pr_debug("%s: Resuming Governors\n", __func__);

	for_each_policy(policy) {
		if (cpufreq_driver->resume && cpufreq_driver->resume(policy))
			pr_err("%s: Failed to resume driver: %p\n", __func__,
				policy);
		else if (__cpufreq_governor(policy, CPUFREQ_GOV_START)
		    || __cpufreq_governor(policy, CPUFREQ_GOV_LIMITS))
			pr_err("%s: Failed to start governor for policy: %p\n",
				__func__, policy);
	}

	/*
	 * schedule call cpufreq_update_policy() for first-online CPU, as that
	 * wouldn't be hotplugged-out on suspend. It will verify that the
	 * current freq is in sync with what we believe it to be.
	 */
	policy = cpufreq_cpu_get_raw(cpumask_first(cpu_online_mask));
	if (WARN_ON(!policy))
		return;

	schedule_work(&policy->update);
}

/**
 *	cpufreq_get_current_driver - return current driver's name
 *
 *	Return the name string of the currently loaded cpufreq driver
 *	or NULL, if none.
 */
const char *cpufreq_get_current_driver(void)
{
	if (cpufreq_driver)
		return cpufreq_driver->name;

	return NULL;
}
EXPORT_SYMBOL_GPL(cpufreq_get_current_driver);

/**
 *	cpufreq_get_driver_data - return current driver data
 *
 *	Return the private data of the currently loaded cpufreq
 *	driver, or NULL if no cpufreq driver is loaded.
 */
void *cpufreq_get_driver_data(void)
{
	if (cpufreq_driver)
		return cpufreq_driver->driver_data;

	return NULL;
}
EXPORT_SYMBOL_GPL(cpufreq_get_driver_data);

/*********************************************************************
 *                     NOTIFIER LISTS INTERFACE                      *
 *********************************************************************/

/**
 *	cpufreq_register_notifier - register a driver with cpufreq
 *	@nb: notifier function to register
 *      @list: CPUFREQ_TRANSITION_NOTIFIER or CPUFREQ_POLICY_NOTIFIER
 *
 *	Add a driver to one of two lists: either a list of drivers that
 *      are notified about clock rate changes (once before and once after
 *      the transition), or a list of drivers that are notified about
 *      changes in cpufreq policy.
 *
 *	This function may sleep, and has the same return conditions as
 *	blocking_notifier_chain_register.
 */
int cpufreq_register_notifier(struct notifier_block *nb, unsigned int list)
{
	int ret;

	if (cpufreq_disabled())
		return -EINVAL;

	WARN_ON(!init_cpufreq_transition_notifier_list_called);

	switch (list) {
	case CPUFREQ_TRANSITION_NOTIFIER:
		ret = srcu_notifier_chain_register(
				&cpufreq_transition_notifier_list, nb);
		break;
	case CPUFREQ_POLICY_NOTIFIER:
		ret = blocking_notifier_chain_register(
				&cpufreq_policy_notifier_list, nb);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(cpufreq_register_notifier);

/**
 *	cpufreq_unregister_notifier - unregister a driver with cpufreq
 *	@nb: notifier block to be unregistered
 *	@list: CPUFREQ_TRANSITION_NOTIFIER or CPUFREQ_POLICY_NOTIFIER
 *
 *	Remove a driver from the CPU frequency notifier list.
 *
 *	This function may sleep, and has the same return conditions as
 *	blocking_notifier_chain_unregister.
 */
int cpufreq_unregister_notifier(struct notifier_block *nb, unsigned int list)
{
	int ret;

	if (cpufreq_disabled())
		return -EINVAL;

	switch (list) {
	case CPUFREQ_TRANSITION_NOTIFIER:
		ret = srcu_notifier_chain_unregister(
				&cpufreq_transition_notifier_list, nb);
		break;
	case CPUFREQ_POLICY_NOTIFIER:
		ret = blocking_notifier_chain_unregister(
				&cpufreq_policy_notifier_list, nb);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(cpufreq_unregister_notifier);


/*********************************************************************
 *                              GOVERNORS                            *
 *********************************************************************/

/* Must set freqs->new to intermediate frequency */
static int __target_intermediate(struct cpufreq_policy *policy,
				 struct cpufreq_freqs *freqs, int index)
{
	int ret;

	freqs->new = cpufreq_driver->get_intermediate(policy, index);

	/* We don't need to switch to intermediate freq */
	if (!freqs->new)
		return 0;

	pr_debug("%s: cpu: %d, switching to intermediate freq: oldfreq: %u, intermediate freq: %u\n",
		 __func__, policy->cpu, freqs->old, freqs->new);

	cpufreq_freq_transition_begin(policy, freqs);
	ret = cpufreq_driver->target_intermediate(policy, index);
	cpufreq_freq_transition_end(policy, freqs, ret);

	if (ret)
		pr_err("%s: Failed to change to intermediate frequency: %d\n",
		       __func__, ret);

	return ret;
}

static int __target_index(struct cpufreq_policy *policy,
			  struct cpufreq_frequency_table *freq_table, int index)
{
	struct cpufreq_freqs freqs = {.old = policy->cur, .flags = 0};
	unsigned int intermediate_freq = 0;
	int retval = -EINVAL;
	bool notify;

	notify = !(cpufreq_driver->flags & CPUFREQ_ASYNC_NOTIFICATION);
	if (notify) {
		/* Handle switching to intermediate frequency */
		if (cpufreq_driver->get_intermediate) {
			retval = __target_intermediate(policy, &freqs, index);
			if (retval)
				return retval;

			intermediate_freq = freqs.new;
			/* Set old freq to intermediate */
			if (intermediate_freq)
				freqs.old = freqs.new;
		}

		freqs.new = freq_table[index].frequency;
		pr_debug("%s: cpu: %d, oldfreq: %u, new freq: %u\n",
			 __func__, policy->cpu, freqs.old, freqs.new);

		cpufreq_freq_transition_begin(policy, &freqs);
	}

	retval = cpufreq_driver->target_index(policy, index);
	if (retval)
		pr_err("%s: Failed to change cpu frequency: %d\n", __func__,
		       retval);

	if (notify) {
		cpufreq_freq_transition_end(policy, &freqs, retval);

		/*
		 * Failed after setting to intermediate freq? Driver should have
		 * reverted back to initial frequency and so should we. Check
		 * here for intermediate_freq instead of get_intermediate, in
		 * case we have't switched to intermediate freq at all.
		 */
		if (unlikely(retval && intermediate_freq)) {
			freqs.old = intermediate_freq;
			freqs.new = policy->restore_freq;
			cpufreq_freq_transition_begin(policy, &freqs);
			cpufreq_freq_transition_end(policy, &freqs, 0);
		}
	}

	return retval;
}

int __cpufreq_driver_target(struct cpufreq_policy *policy,
			    unsigned int target_freq,
			    unsigned int relation)
{
	unsigned int old_target_freq = target_freq;
	int retval = -EINVAL;

	if (cpufreq_disabled())
		return -ENODEV;

	/* Make sure that target_freq is within supported range */
	if (target_freq > policy->max)
		target_freq = policy->max;
	if (target_freq < policy->min)
		target_freq = policy->min;

	pr_debug("target for CPU %u: %u kHz, relation %u, requested %u kHz\n",
		 policy->cpu, target_freq, relation, old_target_freq);

	/*
	 * This might look like a redundant call as we are checking it again
	 * after finding index. But it is left intentionally for cases where
	 * exactly same freq is called again and so we can save on few function
	 * calls.
	 */
	if (target_freq == policy->cur)
		return 0;

	/* Save last value to restore later on errors */
	policy->restore_freq = policy->cur;

	if (cpufreq_driver->target)
		retval = cpufreq_driver->target(policy, target_freq, relation);
	else if (cpufreq_driver->target_index) {
		struct cpufreq_frequency_table *freq_table;
		int index;

		freq_table = cpufreq_frequency_get_table(policy->cpu);
		if (unlikely(!freq_table)) {
			pr_err("%s: Unable to find freq_table\n", __func__);
			goto out;
		}

		retval = cpufreq_frequency_table_target(policy, freq_table,
				target_freq, relation, &index);
		if (unlikely(retval)) {
			pr_err("%s: Unable to find matching freq\n", __func__);
			goto out;
		}

		if (freq_table[index].frequency == policy->cur) {
			retval = 0;
			goto out;
		}

		retval = __target_index(policy, freq_table, index);
	}

out:
	return retval;
}
EXPORT_SYMBOL_GPL(__cpufreq_driver_target);

int cpufreq_driver_target(struct cpufreq_policy *policy,
			  unsigned int target_freq,
			  unsigned int relation)
{
	int ret = -EINVAL;

	down_write(&policy->rwsem);

	ret = __cpufreq_driver_target(policy, target_freq, relation);

	up_write(&policy->rwsem);

	return ret;
}
EXPORT_SYMBOL_GPL(cpufreq_driver_target);

static int __cpufreq_governor(struct cpufreq_policy *policy,
					unsigned int event)
{
	int ret;

	/* Only must be defined when default governor is known to have latency
	   restrictions, like e.g. conservative or ondemand.
	   That this is the case is already ensured in Kconfig
	*/
#ifdef CONFIG_CPU_FREQ_GOV_PERFORMANCE
	struct cpufreq_governor *gov = &cpufreq_gov_performance;
#else
	struct cpufreq_governor *gov = NULL;
#endif

	/* Don't start any governor operations if we are entering suspend */
	if (cpufreq_suspended)
		return 0;
	/*
	 * Governor might not be initiated here if ACPI _PPC changed
	 * notification happened, so check it.
	 */
	if (!policy->governor)
		return -EINVAL;

	if (policy->governor->max_transition_latency &&
	    policy->cpuinfo.transition_latency >
	    policy->governor->max_transition_latency) {
		if (!gov)
			return -EINVAL;
		else {
			pr_warn("%s governor failed, too long transition latency of HW, fallback to %s governor\n",
				policy->governor->name, gov->name);
			policy->governor = gov;
		}
	}

	if (event == CPUFREQ_GOV_POLICY_INIT)
		if (!try_module_get(policy->governor->owner))
			return -EINVAL;

	pr_debug("__cpufreq_governor for CPU %u, event %u\n",
		 policy->cpu, event);

	mutex_lock(&cpufreq_governor_lock);
	if ((policy->governor_enabled && event == CPUFREQ_GOV_START)
	    || (!policy->governor_enabled
	    && (event == CPUFREQ_GOV_LIMITS || event == CPUFREQ_GOV_STOP))) {
		mutex_unlock(&cpufreq_governor_lock);
		return -EBUSY;
	}

	if (event == CPUFREQ_GOV_STOP)
		policy->governor_enabled = false;
	else if (event == CPUFREQ_GOV_START)
		policy->governor_enabled = true;

	mutex_unlock(&cpufreq_governor_lock);

	ret = policy->governor->governor(policy, event);

	if (!ret) {
		if (event == CPUFREQ_GOV_POLICY_INIT)
			policy->governor->initialized++;
		else if (event == CPUFREQ_GOV_POLICY_EXIT)
			policy->governor->initialized--;
	} else {
		/* Restore original values */
		mutex_lock(&cpufreq_governor_lock);
		if (event == CPUFREQ_GOV_STOP)
			policy->governor_enabled = true;
		else if (event == CPUFREQ_GOV_START)
			policy->governor_enabled = false;
		mutex_unlock(&cpufreq_governor_lock);
	}

	if (((event == CPUFREQ_GOV_POLICY_INIT) && ret) ||
			((event == CPUFREQ_GOV_POLICY_EXIT) && !ret))
		module_put(policy->governor->owner);

	return ret;
}

int cpufreq_register_governor(struct cpufreq_governor *governor)
{
	int err;

	if (!governor)
		return -EINVAL;

	if (cpufreq_disabled())
		return -ENODEV;

	mutex_lock(&cpufreq_governor_mutex);

	governor->initialized = 0;
	err = -EBUSY;
	if (!find_governor(governor->name)) {
		err = 0;
		list_add(&governor->governor_list, &cpufreq_governor_list);
	}

	mutex_unlock(&cpufreq_governor_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(cpufreq_register_governor);

void cpufreq_unregister_governor(struct cpufreq_governor *governor)
{
	int cpu;

	if (!governor)
		return;

	if (cpufreq_disabled())
		return;

	for_each_present_cpu(cpu) {
		if (cpu_online(cpu))
			continue;
		if (!strcmp(per_cpu(cpufreq_cpu_governor, cpu), governor->name))
			strcpy(per_cpu(cpufreq_cpu_governor, cpu), "\0");
	}

	mutex_lock(&cpufreq_governor_mutex);
	list_del(&governor->governor_list);
	mutex_unlock(&cpufreq_governor_mutex);
	return;
}
EXPORT_SYMBOL_GPL(cpufreq_unregister_governor);


/*********************************************************************
 *                          POLICY INTERFACE                         *
 *********************************************************************/

/**
 * cpufreq_get_policy - get the current cpufreq_policy
 * @policy: struct cpufreq_policy into which the current cpufreq_policy
 *	is written
 *
 * Reads the current cpufreq policy.
 */
int cpufreq_get_policy(struct cpufreq_policy *policy, unsigned int cpu)
{
	struct cpufreq_policy *cpu_policy;
	if (!policy)
		return -EINVAL;

	cpu_policy = cpufreq_cpu_get(cpu);
	if (!cpu_policy)
		return -EINVAL;

	memcpy(policy, cpu_policy, sizeof(*policy));

	cpufreq_cpu_put(cpu_policy);
	return 0;
}
EXPORT_SYMBOL(cpufreq_get_policy);

/*
 * policy : current policy.
 * new_policy: policy to be set.
 */
#if defined(CONFIG_BCM_KF_BCM63XX_CPUFREQ)
int cpufreq_set_policy(struct cpufreq_policy *policy,
				struct cpufreq_policy *new_policy)
#else
static int cpufreq_set_policy(struct cpufreq_policy *policy,
				struct cpufreq_policy *new_policy)
#endif
{
	struct cpufreq_governor *old_gov;
	int ret;

	pr_debug("setting new policy for CPU %u: %u - %u kHz\n",
		 new_policy->cpu, new_policy->min, new_policy->max);

	memcpy(&new_policy->cpuinfo, &policy->cpuinfo, sizeof(policy->cpuinfo));

	if (new_policy->min > policy->max || new_policy->max < policy->min)
		return -EINVAL;

	/* verify the cpu speed can be set within this limit */
	ret = cpufreq_driver->verify(new_policy);
	if (ret)
		return ret;

	/* adjust if necessary - all reasons */
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_ADJUST, new_policy);

	/* adjust if necessary - hardware incompatibility*/
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_INCOMPATIBLE, new_policy);

	/*
	 * verify the cpu speed can be set within this limit, which might be
	 * different to the first one
	 */
	ret = cpufreq_driver->verify(new_policy);
	if (ret)
		return ret;

	/* notification of the new policy */
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_NOTIFY, new_policy);

	policy->min = new_policy->min;
	policy->max = new_policy->max;

	pr_debug("new min and max freqs are %u - %u kHz\n",
		 policy->min, policy->max);

	if (cpufreq_driver->setpolicy) {
		policy->policy = new_policy->policy;
		pr_debug("setting range\n");
		return cpufreq_driver->setpolicy(new_policy);
	}

	if (new_policy->governor == policy->governor)
		goto out;

	pr_debug("governor switch\n");

	/* save old, working values */
	old_gov = policy->governor;
	/* end old governor */
	if (old_gov) {
		__cpufreq_governor(policy, CPUFREQ_GOV_STOP);
		up_write(&policy->rwsem);
		__cpufreq_governor(policy, CPUFREQ_GOV_POLICY_EXIT);
		down_write(&policy->rwsem);
	}

	/* start new governor */
	policy->governor = new_policy->governor;
	if (!__cpufreq_governor(policy, CPUFREQ_GOV_POLICY_INIT)) {
		if (!__cpufreq_governor(policy, CPUFREQ_GOV_START))
			goto out;

		up_write(&policy->rwsem);
		__cpufreq_governor(policy, CPUFREQ_GOV_POLICY_EXIT);
		down_write(&policy->rwsem);
	}

	/* new governor failed, so re-start old one */
	pr_debug("starting governor %s failed\n", policy->governor->name);
	if (old_gov) {
		policy->governor = old_gov;
		__cpufreq_governor(policy, CPUFREQ_GOV_POLICY_INIT);
		__cpufreq_governor(policy, CPUFREQ_GOV_START);
	}

	return -EINVAL;

 out:
	pr_debug("governor: change or update limits\n");
	return __cpufreq_governor(policy, CPUFREQ_GOV_LIMITS);
}

#if defined(CONFIG_BCM_KF_BCM63XX_CPUFREQ)
EXPORT_SYMBOL(cpufreq_set_policy);

// set governor, and if supported, set speed to specified fraction of max
int cpufreq_set_speed(const char *govstr, int fraction)
{
	struct cpufreq_policy *data, policy;
	struct cpufreq_governor *governor;
	unsigned cpu;
	int rc;

	governor = find_governor(govstr);
	if (governor == 0) {
		pr_warn("governor %s unavailable\n", govstr);
		return -ENOENT;
	}

	cpu = get_cpu();
	data = cpufreq_cpu_get(cpu);
	put_cpu();
	if (data == 0) {
		pr_warn("policy unavailable\n");
		return -ENOENT;
	}
	policy = *data;
	policy.governor = governor;

	down_write(&data->rwsem);
	rc = cpufreq_set_policy(data, &policy);
	up_write(&data->rwsem);
	if (rc == 0) {
		data->user_policy.policy = data->policy;
		data->user_policy.governor = data->governor;
		if (fraction && data->governor->store_setspeed) {
			data->governor->store_setspeed(data, data->max / fraction);
		}
	}
	cpufreq_cpu_put(data);
	return rc;
}
EXPORT_SYMBOL(cpufreq_set_speed);

// return current frequency (0 if dynamic) and max frequency
unsigned cpufreq_get_freq_max(unsigned *max)
{
	struct cpufreq_policy *data;
	unsigned cpu, freq = 0;

	cpu = get_cpu();
	data = cpufreq_cpu_get(cpu);
	put_cpu();
	if (max) *max = data->max;
	if (data->governor && data->governor->store_setspeed) {
		freq = data->cur;
	}
	cpufreq_cpu_put(data);

	return freq;
}
EXPORT_SYMBOL(cpufreq_get_freq_max);

// set maximum frequency for governor
// [ XXX also set current frequency for userspace governor? ]
void cpufreq_set_freq_max(unsigned maxdiv)
{
	struct cpufreq_policy *data;
	unsigned cpu;

	cpu = get_cpu();
	data = cpufreq_cpu_get(cpu);
	put_cpu();

	if (data) {
		data->user_policy.max = data->cpuinfo.max_freq / (maxdiv ?: 1);
		cpufreq_update_policy(data->cpu);
		cpufreq_cpu_put(data);
	}
}
EXPORT_SYMBOL(cpufreq_set_freq_max);
#endif

/**
 *	cpufreq_update_policy - re-evaluate an existing cpufreq policy
 *	@cpu: CPU which shall be re-evaluated
 *
 *	Useful for policy notifiers which have different necessities
 *	at different times.
 */
int cpufreq_update_policy(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
	struct cpufreq_policy new_policy;
	int ret;

	if (!policy)
		return -ENODEV;

	down_write(&policy->rwsem);

	pr_debug("updating policy for CPU %u\n", cpu);
	memcpy(&new_policy, policy, sizeof(*policy));
	new_policy.min = policy->user_policy.min;
	new_policy.max = policy->user_policy.max;
	new_policy.policy = policy->user_policy.policy;
	new_policy.governor = policy->user_policy.governor;

	/*
	 * BIOS might change freq behind our back
	 * -> ask driver for current freq and notify governors about a change
	 */
	if (cpufreq_driver->get && !cpufreq_driver->setpolicy) {
		new_policy.cur = cpufreq_driver->get(cpu);
		if (WARN_ON(!new_policy.cur)) {
			ret = -EIO;
			goto unlock;
		}

		if (!policy->cur) {
			pr_debug("Driver did not initialize current freq\n");
			policy->cur = new_policy.cur;
		} else {
			if (policy->cur != new_policy.cur && has_target())
				cpufreq_out_of_sync(policy, new_policy.cur);
		}
	}

	ret = cpufreq_set_policy(policy, &new_policy);

unlock:
	up_write(&policy->rwsem);

	cpufreq_cpu_put(policy);
	return ret;
}
EXPORT_SYMBOL(cpufreq_update_policy);

static int cpufreq_cpu_callback(struct notifier_block *nfb,
					unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	struct device *dev;

	dev = get_cpu_device(cpu);
	if (dev) {
		switch (action & ~CPU_TASKS_FROZEN) {
		case CPU_ONLINE:
			__cpufreq_add_dev(dev, NULL);
			break;

		case CPU_DOWN_PREPARE:
			__cpufreq_remove_dev_prepare(dev, NULL);
			break;

		case CPU_POST_DEAD:
			__cpufreq_remove_dev_finish(dev, NULL);
			break;

		case CPU_DOWN_FAILED:
			__cpufreq_add_dev(dev, NULL);
			break;
		}
	}
	return NOTIFY_OK;
}

static struct notifier_block __refdata cpufreq_cpu_notifier = {
	.notifier_call = cpufreq_cpu_callback,
};

/*********************************************************************
 *               BOOST						     *
 *********************************************************************/
static int cpufreq_boost_set_sw(int state)
{
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_policy *policy;
	int ret = -EINVAL;

	for_each_policy(policy) {
		freq_table = cpufreq_frequency_get_table(policy->cpu);
		if (freq_table) {
			ret = cpufreq_frequency_table_cpuinfo(policy,
							freq_table);
			if (ret) {
				pr_err("%s: Policy frequency update failed\n",
				       __func__);
				break;
			}
			policy->user_policy.max = policy->max;
			__cpufreq_governor(policy, CPUFREQ_GOV_LIMITS);
		}
	}

	return ret;
}

int cpufreq_boost_trigger_state(int state)
{
	unsigned long flags;
	int ret = 0;

	if (cpufreq_driver->boost_enabled == state)
		return 0;

	write_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_driver->boost_enabled = state;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	ret = cpufreq_driver->set_boost(state);
	if (ret) {
		write_lock_irqsave(&cpufreq_driver_lock, flags);
		cpufreq_driver->boost_enabled = !state;
		write_unlock_irqrestore(&cpufreq_driver_lock, flags);

		pr_err("%s: Cannot %s BOOST\n",
		       __func__, state ? "enable" : "disable");
	}

	return ret;
}

int cpufreq_boost_supported(void)
{
	if (likely(cpufreq_driver))
		return cpufreq_driver->boost_supported;

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_boost_supported);

int cpufreq_boost_enabled(void)
{
	return cpufreq_driver->boost_enabled;
}
EXPORT_SYMBOL_GPL(cpufreq_boost_enabled);

/*********************************************************************
 *               REGISTER / UNREGISTER CPUFREQ DRIVER                *
 *********************************************************************/

/**
 * cpufreq_register_driver - register a CPU Frequency driver
 * @driver_data: A struct cpufreq_driver containing the values#
 * submitted by the CPU Frequency driver.
 *
 * Registers a CPU Frequency driver to this core code. This code
 * returns zero on success, -EBUSY when another driver got here first
 * (and isn't unregistered in the meantime).
 *
 */
int cpufreq_register_driver(struct cpufreq_driver *driver_data)
{
	unsigned long flags;
	int ret;

	if (cpufreq_disabled())
		return -ENODEV;

	if (!driver_data || !driver_data->verify || !driver_data->init ||
	    !(driver_data->setpolicy || driver_data->target_index ||
		    driver_data->target) ||
	     (driver_data->setpolicy && (driver_data->target_index ||
		    driver_data->target)) ||
	     (!!driver_data->get_intermediate != !!driver_data->target_intermediate))
		return -EINVAL;

	pr_debug("trying to register driver %s\n", driver_data->name);

	write_lock_irqsave(&cpufreq_driver_lock, flags);
	if (cpufreq_driver) {
		write_unlock_irqrestore(&cpufreq_driver_lock, flags);
		return -EEXIST;
	}
	cpufreq_driver = driver_data;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);

	if (driver_data->setpolicy)
		driver_data->flags |= CPUFREQ_CONST_LOOPS;

	if (cpufreq_boost_supported()) {
		/*
		 * Check if driver provides function to enable boost -
		 * if not, use cpufreq_boost_set_sw as default
		 */
		if (!cpufreq_driver->set_boost)
			cpufreq_driver->set_boost = cpufreq_boost_set_sw;

		ret = cpufreq_sysfs_create_file(&boost.attr);
		if (ret) {
			pr_err("%s: cannot register global BOOST sysfs file\n",
			       __func__);
			goto err_null_driver;
		}
	}

	ret = subsys_interface_register(&cpufreq_interface);
	if (ret)
		goto err_boost_unreg;

	if (!(cpufreq_driver->flags & CPUFREQ_STICKY) &&
	    list_empty(&cpufreq_policy_list)) {
		/* if all ->init() calls failed, unregister */
		ret = -ENODEV;
		pr_debug("%s: No CPU initialized for driver %s\n", __func__,
			 driver_data->name);
		goto err_if_unreg;
	}

	register_hotcpu_notifier(&cpufreq_cpu_notifier);
	pr_debug("driver %s up and running\n", driver_data->name);

	return 0;
err_if_unreg:
	subsys_interface_unregister(&cpufreq_interface);
err_boost_unreg:
	if (cpufreq_boost_supported())
		cpufreq_sysfs_remove_file(&boost.attr);
err_null_driver:
	write_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_driver = NULL;
	write_unlock_irqrestore(&cpufreq_driver_lock, flags);
	return ret;
}
EXPORT_SYMBOL_GPL(cpufreq_register_driver);

/**
 * cpufreq_unregister_driver - unregister the current CPUFreq driver
 *
 * Unregister the current CPUFreq driver. Only call this if you have
 * the right to do so, i.e. if you have succeeded in initialising before!
 * Returns zero if successful, and -EINVAL if the cpufreq_driver is
 * currently not initialised.
 */
int cpufreq_unregister_driver(struct cpufreq_driver *driver)
{
	unsigned long flags;

	if (!cpufreq_driver || (driver != cpufreq_driver))
		return -EINVAL;

	pr_debug("unregistering driver %s\n", driver->name);

	subsys_interface_unregister(&cpufreq_interface);
	if (cpufreq_boost_supported())
		cpufreq_sysfs_remove_file(&boost.attr);

	unregister_hotcpu_notifier(&cpufreq_cpu_notifier);

	down_write(&cpufreq_rwsem);
	write_lock_irqsave(&cpufreq_driver_lock, flags);

	cpufreq_driver = NULL;

	write_unlock_irqrestore(&cpufreq_driver_lock, flags);
	up_write(&cpufreq_rwsem);

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_unregister_driver);

/*
 * Stop cpufreq at shutdown to make sure it isn't holding any locks
 * or mutexes when secondary CPUs are halted.
 */
static struct syscore_ops cpufreq_syscore_ops = {
	.shutdown = cpufreq_suspend,
};

static int __init cpufreq_core_init(void)
{
	if (cpufreq_disabled())
		return -ENODEV;

	cpufreq_global_kobject = kobject_create();
	BUG_ON(!cpufreq_global_kobject);

	register_syscore_ops(&cpufreq_syscore_ops);

	return 0;
}
core_initcall(cpufreq_core_init);
