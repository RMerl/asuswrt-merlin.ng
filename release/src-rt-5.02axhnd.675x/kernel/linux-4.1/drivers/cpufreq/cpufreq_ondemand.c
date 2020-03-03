/*
 *  drivers/cpufreq/cpufreq_ondemand.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2003 Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>.
 *                      Jun Nakajima <jun.nakajima@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/percpu-defs.h>
#include <linux/slab.h>
#include <linux/tick.h>
#include "cpufreq_governor.h"

/* On-demand governor macros */
#if defined(CONFIG_BCM_KF_ONDEMAND)
#define DEF_FREQUENCY_UP_THRESHOLD		(20)
#define DEF_SAMPLING_DOWN_FACTOR		(10)
#else
#define DEF_FREQUENCY_UP_THRESHOLD		(80)
#define DEF_SAMPLING_DOWN_FACTOR		(1)
#endif
#define MAX_SAMPLING_DOWN_FACTOR		(100000)
#if defined(CONFIG_BCM_KF_ONDEMAND)
#define MICRO_FREQUENCY_UP_THRESHOLD		(40)
#else
#define MICRO_FREQUENCY_UP_THRESHOLD		(95)
#endif
#define MICRO_FREQUENCY_MIN_SAMPLE_RATE		(10000)
#define MIN_FREQUENCY_UP_THRESHOLD		(11)
#define MAX_FREQUENCY_UP_THRESHOLD		(100)

static DEFINE_PER_CPU(struct od_cpu_dbs_info_s, od_cpu_dbs_info);

static struct od_ops od_ops;

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_ONDEMAND
static struct cpufreq_governor cpufreq_gov_ondemand;
#if defined(CONFIG_BCM_KF_ONDEMAND)
// clear current frequency table indices
static void min_request_clr(struct cpufreq_frequency_table *table)
{
	int idx;

	for (idx = 0; table[idx].frequency != CPUFREQ_TABLE_END; idx++)
		table[idx].driver_data = 0;
}

// change governor policy min
static void min_request_chg(struct cpufreq_policy *policy,
	struct cpufreq_frequency_table *table)
{
	unsigned freq, max = 0;
	int idx = 0;

	// find highest entry whose request count is non-zero
	while ((freq = table[idx].frequency) != CPUFREQ_TABLE_END) {
		if (table[idx].driver_data && freq > max)
			max = freq;
		idx++;
	}

	// use lowest min freq if no requests
	if (max == 0)
		max = table[0].frequency;

	// update policy if current min != max
	if (policy->min != max) {
		struct cpufreq_policy new_policy = *policy;
		new_policy.min = max;

		if (policy->min < max)
			__cpufreq_driver_target(policy, max, CPUFREQ_RELATION_L);
		cpufreq_set_policy(policy, &new_policy);
	}
}

// decrement request count for min freq of 'freq'
static int min_request_dec(struct cpufreq_policy *policy,
	struct cpufreq_frequency_table *table, unsigned freqmin)
{
	unsigned freq;
	int idx = 0;

	// assume table ordered by frequency; find first entry >= freqmin
	// (cpufreq_frequency_table_target() honors current policy->min)
	while ((freq = table[idx].frequency) != CPUFREQ_TABLE_END) {
		if (freq != CPUFREQ_ENTRY_INVALID && freq >= freqmin) {
			if (table[idx].driver_data && --table[idx].driver_data == 0) {
				// expired minimum ... update policy
				min_request_chg(policy, table);
			}
			return 0;
		}
		idx++;
	}
	return idx;
}

// increment request count for min freq of 'freq'
static int min_request_inc(struct cpufreq_policy *policy,
		struct cpufreq_frequency_table *table, unsigned freqmin)
{
	unsigned freq;
	int idx = 0;

	// assume table ordered by frequency; find first entry >= freqmin
	// (cpufreq_frequency_table_target() honors current policy->min)
	while ((freq = table[idx].frequency) != CPUFREQ_TABLE_END) {
		if (freq != CPUFREQ_ENTRY_INVALID && freq >= freqmin) {
			if (table[idx].driver_data++ == 0 && freq > policy->min) {
				// higher minimum ... update policy
				min_request_chg(policy, table);
			}
			return 0;
		}
		idx++;
	}
	return idx;
}
#endif

#endif

static unsigned int default_powersave_bias;

static void ondemand_powersave_bias_init_cpu(int cpu)
{
	struct od_cpu_dbs_info_s *dbs_info = &per_cpu(od_cpu_dbs_info, cpu);

	dbs_info->freq_table = cpufreq_frequency_get_table(cpu);
#if defined(CONFIG_BCM_KF_ONDEMAND)
	min_request_clr(dbs_info->freq_table);
#endif
	dbs_info->freq_lo = 0;
}

/*
 * Not all CPUs want IO time to be accounted as busy; this depends on how
 * efficient idling at a higher frequency/voltage is.
 * Pavel Machek says this is not so for various generations of AMD and old
 * Intel systems.
 * Mike Chan (android.com) claims this is also not true for ARM.
 * Because of this, whitelist specific known (series) of CPUs by default, and
 * leave all others up to the user.
 */
static int should_io_be_busy(void)
{
#if defined(CONFIG_X86)
	/*
	 * For Intel, Core 2 (model 15) and later have an efficient idle.
	 */
	if (boot_cpu_data.x86_vendor == X86_VENDOR_INTEL &&
			boot_cpu_data.x86 == 6 &&
			boot_cpu_data.x86_model >= 15)
		return 1;
#endif
	return 0;
}

/*
 * Find right freq to be set now with powersave_bias on.
 * Returns the freq_hi to be used right now and will set freq_hi_jiffies,
 * freq_lo, and freq_lo_jiffies in percpu area for averaging freqs.
 */
static unsigned int generic_powersave_bias_target(struct cpufreq_policy *policy,
		unsigned int freq_next, unsigned int relation)
{
	unsigned int freq_req, freq_reduc, freq_avg;
	unsigned int freq_hi, freq_lo;
	unsigned int index = 0;
	unsigned int jiffies_total, jiffies_hi, jiffies_lo;
	struct od_cpu_dbs_info_s *dbs_info = &per_cpu(od_cpu_dbs_info,
						   policy->cpu);
	struct dbs_data *dbs_data = policy->governor_data;
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;

	if (!dbs_info->freq_table) {
		dbs_info->freq_lo = 0;
		dbs_info->freq_lo_jiffies = 0;
		return freq_next;
	}

	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_next,
			relation, &index);
	freq_req = dbs_info->freq_table[index].frequency;
	freq_reduc = freq_req * od_tuners->powersave_bias / 1000;
	freq_avg = freq_req - freq_reduc;

	/* Find freq bounds for freq_avg in freq_table */
	index = 0;
	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_avg,
			CPUFREQ_RELATION_H, &index);
	freq_lo = dbs_info->freq_table[index].frequency;
	index = 0;
	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_avg,
			CPUFREQ_RELATION_L, &index);
	freq_hi = dbs_info->freq_table[index].frequency;

	/* Find out how long we have to be in hi and lo freqs */
	if (freq_hi == freq_lo) {
		dbs_info->freq_lo = 0;
		dbs_info->freq_lo_jiffies = 0;
		return freq_lo;
	}
	jiffies_total = usecs_to_jiffies(od_tuners->sampling_rate);
	jiffies_hi = (freq_avg - freq_lo) * jiffies_total;
	jiffies_hi += ((freq_hi - freq_lo) / 2);
	jiffies_hi /= (freq_hi - freq_lo);
	jiffies_lo = jiffies_total - jiffies_hi;
	dbs_info->freq_lo = freq_lo;
	dbs_info->freq_lo_jiffies = jiffies_lo;
	dbs_info->freq_hi_jiffies = jiffies_hi;
	return freq_hi;
}

static void ondemand_powersave_bias_init(void)
{
	int i;
	for_each_online_cpu(i) {
		ondemand_powersave_bias_init_cpu(i);
	}
}

static void dbs_freq_increase(struct cpufreq_policy *policy, unsigned int freq)
{
	struct dbs_data *dbs_data = policy->governor_data;
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;

	if (od_tuners->powersave_bias)
		freq = od_ops.powersave_bias_target(policy, freq,
				CPUFREQ_RELATION_H);
	else if (policy->cur == policy->max)
		return;

	__cpufreq_driver_target(policy, freq, od_tuners->powersave_bias ?
			CPUFREQ_RELATION_L : CPUFREQ_RELATION_H);
}

/*
 * Every sampling_rate, we check, if current idle time is less than 20%
 * (default), then we try to increase frequency. Else, we adjust the frequency
 * proportional to load.
 */
static void od_check_cpu(int cpu, unsigned int load)
{
	struct od_cpu_dbs_info_s *dbs_info = &per_cpu(od_cpu_dbs_info, cpu);
	struct cpufreq_policy *policy = dbs_info->cdbs.cur_policy;
	struct dbs_data *dbs_data = policy->governor_data;
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;

	dbs_info->freq_lo = 0;

	/* Check for frequency increase */
	if (load > od_tuners->up_threshold) {
		/* If switching to max speed, apply sampling_down_factor */
		if (policy->cur < policy->max)
			dbs_info->rate_mult =
				od_tuners->sampling_down_factor;
		dbs_freq_increase(policy, policy->max);
	} else {
		/* Calculate the next frequency proportional to load */
		unsigned int freq_next, min_f, max_f;

		min_f = policy->cpuinfo.min_freq;
		max_f = policy->cpuinfo.max_freq;
		freq_next = min_f + load * (max_f - min_f) / 100;

		/* No longer fully busy, reset rate_mult */
		dbs_info->rate_mult = 1;

		if (!od_tuners->powersave_bias) {
			__cpufreq_driver_target(policy, freq_next,
					CPUFREQ_RELATION_C);
			return;
		}

		freq_next = od_ops.powersave_bias_target(policy, freq_next,
					CPUFREQ_RELATION_L);
		__cpufreq_driver_target(policy, freq_next, CPUFREQ_RELATION_C);
	}
}

static void od_dbs_timer(struct work_struct *work)
{
	struct od_cpu_dbs_info_s *dbs_info =
		container_of(work, struct od_cpu_dbs_info_s, cdbs.work.work);
	unsigned int cpu = dbs_info->cdbs.cur_policy->cpu;
	struct od_cpu_dbs_info_s *core_dbs_info = &per_cpu(od_cpu_dbs_info,
			cpu);
	struct dbs_data *dbs_data = dbs_info->cdbs.cur_policy->governor_data;
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	int delay = 0, sample_type = core_dbs_info->sample_type;
	bool modify_all = true;

	mutex_lock(&core_dbs_info->cdbs.timer_mutex);
	if (!need_load_eval(&core_dbs_info->cdbs, od_tuners->sampling_rate)) {
		modify_all = false;
		goto max_delay;
	}

	/* Common NORMAL_SAMPLE setup */
	core_dbs_info->sample_type = OD_NORMAL_SAMPLE;
	if (sample_type == OD_SUB_SAMPLE) {
		delay = core_dbs_info->freq_lo_jiffies;
		__cpufreq_driver_target(core_dbs_info->cdbs.cur_policy,
				core_dbs_info->freq_lo, CPUFREQ_RELATION_H);
	} else {
		dbs_check_cpu(dbs_data, cpu);
		if (core_dbs_info->freq_lo) {
			/* Setup timer for SUB_SAMPLE */
			core_dbs_info->sample_type = OD_SUB_SAMPLE;
			delay = core_dbs_info->freq_hi_jiffies;
		}
	}

max_delay:
#if defined(CONFIG_BCM_KF_ONDEMAND)
		/* 
		 * If we're at the highest frequency we set rate_mult > 1
		 * so we don't switch to a lower frequency for multiple
		 * sampling_rate intervals.
		 *
		 * delay_for_sampling_rate() tries to align the next timer
		 * to the next interval where (jiffy % sampling_rate == 0)
		 *
		 * e.g. if sampling_rate*1 is 100 jiffies, we try to align
		 * the timer expiry on all cpus to when jiffies are evenly
		 * divisible by 100.
		 *
		 * Similarly if sampling_rate*10 is 1000 jiffies, the
		 * timer expiry would be aligned to multiples of 1000.
		 * But if the current jiffy is 990 then the next aligned
		 * interval would be at 1000 which is much sooner than
		 * requested by rate_mult
		 *
		 * align to sampling_rate and add rate_mult-1 intervals
		 */
		delay = delay_for_sampling_rate(od_tuners->sampling_rate)
			+ usecs_to_jiffies(od_tuners->sampling_rate
				* (core_dbs_info->rate_mult - 1));
#else
	if (!delay)
		delay = delay_for_sampling_rate(od_tuners->sampling_rate
				* core_dbs_info->rate_mult);
#endif

	gov_queue_work(dbs_data, dbs_info->cdbs.cur_policy, delay, modify_all);
	mutex_unlock(&core_dbs_info->cdbs.timer_mutex);
}

/************************** sysfs interface ************************/
static struct common_dbs_data od_dbs_cdata;

/**
 * update_sampling_rate - update sampling rate effective immediately if needed.
 * @new_rate: new sampling rate
 *
 * If new rate is smaller than the old, simply updating
 * dbs_tuners_int.sampling_rate might not be appropriate. For example, if the
 * original sampling_rate was 1 second and the requested new sampling rate is 10
 * ms because the user needs immediate reaction from ondemand governor, but not
 * sure if higher frequency will be required or not, then, the governor may
 * change the sampling rate too late; up to 1 second later. Thus, if we are
 * reducing the sampling rate, we need to make the new value effective
 * immediately.
 */
static void update_sampling_rate(struct dbs_data *dbs_data,
		unsigned int new_rate)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	int cpu;

	od_tuners->sampling_rate = new_rate = max(new_rate,
			dbs_data->min_sampling_rate);

	for_each_online_cpu(cpu) {
		struct cpufreq_policy *policy;
		struct od_cpu_dbs_info_s *dbs_info;
		unsigned long next_sampling, appointed_at;

		policy = cpufreq_cpu_get(cpu);
		if (!policy)
			continue;
		if (policy->governor != &cpufreq_gov_ondemand) {
			cpufreq_cpu_put(policy);
			continue;
		}
		dbs_info = &per_cpu(od_cpu_dbs_info, cpu);
		cpufreq_cpu_put(policy);

		mutex_lock(&dbs_info->cdbs.timer_mutex);

		if (!delayed_work_pending(&dbs_info->cdbs.work)) {
			mutex_unlock(&dbs_info->cdbs.timer_mutex);
			continue;
		}

		next_sampling = jiffies + usecs_to_jiffies(new_rate);
		appointed_at = dbs_info->cdbs.work.timer.expires;

		if (time_before(next_sampling, appointed_at)) {

			mutex_unlock(&dbs_info->cdbs.timer_mutex);
			cancel_delayed_work_sync(&dbs_info->cdbs.work);
			mutex_lock(&dbs_info->cdbs.timer_mutex);

			gov_queue_work(dbs_data, dbs_info->cdbs.cur_policy,
					usecs_to_jiffies(new_rate), true);

		}
		mutex_unlock(&dbs_info->cdbs.timer_mutex);
	}
}

static ssize_t store_sampling_rate(struct dbs_data *dbs_data, const char *buf,
		size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	update_sampling_rate(dbs_data, input);
	return count;
}

static ssize_t store_io_is_busy(struct dbs_data *dbs_data, const char *buf,
		size_t count)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	unsigned int input;
	int ret;
	unsigned int j;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	od_tuners->io_is_busy = !!input;

	/* we need to re-evaluate prev_cpu_idle */
	for_each_online_cpu(j) {
		struct od_cpu_dbs_info_s *dbs_info = &per_cpu(od_cpu_dbs_info,
									j);
		dbs_info->cdbs.prev_cpu_idle = get_cpu_idle_time(j,
			&dbs_info->cdbs.prev_cpu_wall, od_tuners->io_is_busy);
	}
	return count;
}

static ssize_t store_up_threshold(struct dbs_data *dbs_data, const char *buf,
		size_t count)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_FREQUENCY_UP_THRESHOLD ||
			input < MIN_FREQUENCY_UP_THRESHOLD) {
		return -EINVAL;
	}

	od_tuners->up_threshold = input;
	return count;
}

static ssize_t store_sampling_down_factor(struct dbs_data *dbs_data,
		const char *buf, size_t count)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_FACTOR || input < 1)
		return -EINVAL;
	od_tuners->sampling_down_factor = input;

	/* Reset down sampling multiplier in case it was active */
	for_each_online_cpu(j) {
		struct od_cpu_dbs_info_s *dbs_info = &per_cpu(od_cpu_dbs_info,
				j);
		dbs_info->rate_mult = 1;
	}
	return count;
}

static ssize_t store_ignore_nice_load(struct dbs_data *dbs_data,
		const char *buf, size_t count)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	unsigned int input;
	int ret;

	unsigned int j;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	if (input > 1)
		input = 1;

	if (input == od_tuners->ignore_nice_load) { /* nothing to do */
		return count;
	}
	od_tuners->ignore_nice_load = input;

	/* we need to re-evaluate prev_cpu_idle */
	for_each_online_cpu(j) {
		struct od_cpu_dbs_info_s *dbs_info;
		dbs_info = &per_cpu(od_cpu_dbs_info, j);
		dbs_info->cdbs.prev_cpu_idle = get_cpu_idle_time(j,
			&dbs_info->cdbs.prev_cpu_wall, od_tuners->io_is_busy);
		if (od_tuners->ignore_nice_load)
			dbs_info->cdbs.prev_cpu_nice =
				kcpustat_cpu(j).cpustat[CPUTIME_NICE];

	}
	return count;
}

static ssize_t store_powersave_bias(struct dbs_data *dbs_data, const char *buf,
		size_t count)
{
	struct od_dbs_tuners *od_tuners = dbs_data->tuners;
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input > 1000)
		input = 1000;

	od_tuners->powersave_bias = input;
	ondemand_powersave_bias_init();
	return count;
}

show_store_one(od, sampling_rate);
show_store_one(od, io_is_busy);
show_store_one(od, up_threshold);
show_store_one(od, sampling_down_factor);
show_store_one(od, ignore_nice_load);
show_store_one(od, powersave_bias);
declare_show_sampling_rate_min(od);

gov_sys_pol_attr_rw(sampling_rate);
gov_sys_pol_attr_rw(io_is_busy);
gov_sys_pol_attr_rw(up_threshold);
gov_sys_pol_attr_rw(sampling_down_factor);
gov_sys_pol_attr_rw(ignore_nice_load);
gov_sys_pol_attr_rw(powersave_bias);
gov_sys_pol_attr_ro(sampling_rate_min);

#if defined(CONFIG_BCM_KF_ONDEMAND)
static int reservation_update(int freq)
{
	struct cpufreq_policy *policy;
	struct od_cpu_dbs_info_s *dbs_info;
	int ret, cpu;

	cpu = get_cpu();
	policy = cpufreq_cpu_get(cpu);
	put_cpu();
	if (!policy) return -EFAULT;
	if (!policy->governor_enabled) return -EINVAL;
	dbs_info = &per_cpu(od_cpu_dbs_info, policy->cpu);
	cpufreq_cpu_put(policy);

	if (freq > 0)
		ret = min_request_inc(policy, dbs_info->freq_table, freq);
	else
		ret = min_request_dec(policy, dbs_info->freq_table, -freq);

	return ret ? -ENOENT : 0;
}

// request to reserve minimum frequency
static ssize_t show_reserve(struct kobject *a, struct attribute *b,
	char *buf)
{
	struct cpufreq_policy *policy;
	struct od_cpu_dbs_info_s *dbs_info;
	unsigned count = 0;
	unsigned freq;
	int idx = 0;
	int cpu;

	cpu = get_cpu();
	policy = cpufreq_cpu_get(cpu);
	put_cpu();
	if (!policy) return -ENOENT;
	dbs_info = &per_cpu(od_cpu_dbs_info, policy->cpu);
	cpufreq_cpu_put(policy);

	while ((freq = dbs_info->freq_table[idx].frequency) != CPUFREQ_TABLE_END) {
		count += sprintf(buf + count, "%u:%u ",
			dbs_info->freq_table[idx].frequency,
			dbs_info->freq_table[idx].driver_data);
		idx++;
	}
	count += sprintf(buf + count - 1, "\n") - 1;
	return count;
}

static ssize_t store_reserve(struct kobject *a, struct attribute *b,
	const char *buf, size_t count)
{
	int freq;

	if (sscanf(buf, "%d", &freq) != 1)
		return -EINVAL;

	return reservation_update(freq) ?: count;
}

define_one_global_rw(reserve);

int cpufreq_minimum_reserve(int freq)
{
	return reservation_update(freq);
}
EXPORT_SYMBOL(cpufreq_minimum_reserve);

int cpufreq_minimum_unreserve(int freq)
{
	return reservation_update(-freq);
}
EXPORT_SYMBOL(cpufreq_minimum_unreserve);
#endif

static struct attribute *dbs_attributes_gov_sys[] = {
#if defined(CONFIG_BCM_KF_ONDEMAND)
	&reserve.attr,
#endif
	&sampling_rate_min_gov_sys.attr,
	&sampling_rate_gov_sys.attr,
	&up_threshold_gov_sys.attr,
	&sampling_down_factor_gov_sys.attr,
	&ignore_nice_load_gov_sys.attr,
	&powersave_bias_gov_sys.attr,
	&io_is_busy_gov_sys.attr,
	NULL
};

static struct attribute_group od_attr_group_gov_sys = {
	.attrs = dbs_attributes_gov_sys,
	.name = "ondemand",
};

static struct attribute *dbs_attributes_gov_pol[] = {
	&sampling_rate_min_gov_pol.attr,
	&sampling_rate_gov_pol.attr,
	&up_threshold_gov_pol.attr,
	&sampling_down_factor_gov_pol.attr,
	&ignore_nice_load_gov_pol.attr,
	&powersave_bias_gov_pol.attr,
	&io_is_busy_gov_pol.attr,
	NULL
};

static struct attribute_group od_attr_group_gov_pol = {
	.attrs = dbs_attributes_gov_pol,
	.name = "ondemand",
};

/************************** sysfs end ************************/

static int od_init(struct dbs_data *dbs_data)
{
	struct od_dbs_tuners *tuners;
	u64 idle_time;
	int cpu;

	tuners = kzalloc(sizeof(*tuners), GFP_KERNEL);
	if (!tuners) {
		pr_err("%s: kzalloc failed\n", __func__);
		return -ENOMEM;
	}

	cpu = get_cpu();
	idle_time = get_cpu_idle_time_us(cpu, NULL);
	put_cpu();
	if (idle_time != -1ULL) {
		/* Idle micro accounting is supported. Use finer thresholds */
		tuners->up_threshold = MICRO_FREQUENCY_UP_THRESHOLD;
		/*
		 * In nohz/micro accounting case we set the minimum frequency
		 * not depending on HZ, but fixed (very low). The deferred
		 * timer might skip some samples if idle/sleeping as needed.
		*/
		dbs_data->min_sampling_rate = MICRO_FREQUENCY_MIN_SAMPLE_RATE;
	} else {
		tuners->up_threshold = DEF_FREQUENCY_UP_THRESHOLD;

		/* For correct statistics, we need 10 ticks for each measure */
		dbs_data->min_sampling_rate = MIN_SAMPLING_RATE_RATIO *
			jiffies_to_usecs(10);
	}

	tuners->sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR;
	tuners->ignore_nice_load = 0;
	tuners->powersave_bias = default_powersave_bias;
	tuners->io_is_busy = should_io_be_busy();

	dbs_data->tuners = tuners;
	mutex_init(&dbs_data->mutex);
	return 0;
}

static void od_exit(struct dbs_data *dbs_data)
{
	kfree(dbs_data->tuners);
}

define_get_cpu_dbs_routines(od_cpu_dbs_info);

static struct od_ops od_ops = {
	.powersave_bias_init_cpu = ondemand_powersave_bias_init_cpu,
	.powersave_bias_target = generic_powersave_bias_target,
	.freq_increase = dbs_freq_increase,
};

static struct common_dbs_data od_dbs_cdata = {
	.governor = GOV_ONDEMAND,
	.attr_group_gov_sys = &od_attr_group_gov_sys,
	.attr_group_gov_pol = &od_attr_group_gov_pol,
	.get_cpu_cdbs = get_cpu_cdbs,
	.get_cpu_dbs_info_s = get_cpu_dbs_info_s,
	.gov_dbs_timer = od_dbs_timer,
	.gov_check_cpu = od_check_cpu,
	.gov_ops = &od_ops,
	.init = od_init,
	.exit = od_exit,
};

static void od_set_powersave_bias(unsigned int powersave_bias)
{
	struct cpufreq_policy *policy;
	struct dbs_data *dbs_data;
	struct od_dbs_tuners *od_tuners;
	unsigned int cpu;
	cpumask_t done;

	default_powersave_bias = powersave_bias;
	cpumask_clear(&done);

	get_online_cpus();
	for_each_online_cpu(cpu) {
		if (cpumask_test_cpu(cpu, &done))
			continue;

		policy = per_cpu(od_cpu_dbs_info, cpu).cdbs.cur_policy;
		if (!policy)
			continue;

		cpumask_or(&done, &done, policy->cpus);

		if (policy->governor != &cpufreq_gov_ondemand)
			continue;

		dbs_data = policy->governor_data;
		od_tuners = dbs_data->tuners;
		od_tuners->powersave_bias = default_powersave_bias;
	}
	put_online_cpus();
}

void od_register_powersave_bias_handler(unsigned int (*f)
		(struct cpufreq_policy *, unsigned int, unsigned int),
		unsigned int powersave_bias)
{
	od_ops.powersave_bias_target = f;
	od_set_powersave_bias(powersave_bias);
}
EXPORT_SYMBOL_GPL(od_register_powersave_bias_handler);

void od_unregister_powersave_bias_handler(void)
{
	od_ops.powersave_bias_target = generic_powersave_bias_target;
	od_set_powersave_bias(0);
}
EXPORT_SYMBOL_GPL(od_unregister_powersave_bias_handler);

static int od_cpufreq_governor_dbs(struct cpufreq_policy *policy,
		unsigned int event)
{
	return cpufreq_governor_dbs(policy, &od_dbs_cdata, event);
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_ONDEMAND
static
#endif
struct cpufreq_governor cpufreq_gov_ondemand = {
	.name			= "ondemand",
	.governor		= od_cpufreq_governor_dbs,
	.max_transition_latency	= TRANSITION_LATENCY_LIMIT,
	.owner			= THIS_MODULE,
};

static int __init cpufreq_gov_dbs_init(void)
{
	return cpufreq_register_governor(&cpufreq_gov_ondemand);
}

static void __exit cpufreq_gov_dbs_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_ondemand);
}

MODULE_AUTHOR("Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>");
MODULE_AUTHOR("Alexey Starikovskiy <alexey.y.starikovskiy@intel.com>");
MODULE_DESCRIPTION("'cpufreq_ondemand' - A dynamic cpufreq governor for "
	"Low Latency Frequency Transition capable processors");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_ONDEMAND
fs_initcall(cpufreq_gov_dbs_init);
#else
module_init(cpufreq_gov_dbs_init);
#endif
module_exit(cpufreq_gov_dbs_exit);
