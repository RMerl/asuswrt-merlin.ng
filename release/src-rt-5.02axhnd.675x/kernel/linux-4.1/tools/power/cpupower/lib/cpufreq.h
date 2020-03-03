/*
 *  cpufreq.h - definitions for libcpufreq
 *
 *  Copyright (C) 2004-2009  Dominik Brodowski <linux@dominikbrodowski.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _CPUFREQ_H
#define _CPUFREQ_H 1

struct cpufreq_policy {
	unsigned long min;
	unsigned long max;
	char *governor;
};

struct cpufreq_available_governors {
	char *governor;
	struct cpufreq_available_governors *next;
	struct cpufreq_available_governors *first;
};

struct cpufreq_available_frequencies {
	unsigned long frequency;
	struct cpufreq_available_frequencies *next;
	struct cpufreq_available_frequencies *first;
};


struct cpufreq_affected_cpus {
	unsigned int cpu;
	struct cpufreq_affected_cpus *next;
	struct cpufreq_affected_cpus *first;
};

struct cpufreq_stats {
	unsigned long frequency;
	unsigned long long time_in_state;
	struct cpufreq_stats *next;
	struct cpufreq_stats *first;
};



#ifdef __cplusplus
extern "C" {
#endif

/*
 * returns 0 if the specified CPU is present (it doesn't say
 * whether it is online!), and an error value if not.
 */

extern int cpufreq_cpu_exists(unsigned int cpu);

/* determine current CPU frequency
 * - _kernel variant means kernel's opinion of CPU frequency
 * - _hardware variant means actual hardware CPU frequency,
 *    which is only available to root.
 *
 * returns 0 on failure, else frequency in kHz.
 */

extern unsigned long cpufreq_get_freq_kernel(unsigned int cpu);

extern unsigned long cpufreq_get_freq_hardware(unsigned int cpu);

#define cpufreq_get(cpu) cpufreq_get_freq_kernel(cpu);


/* determine CPU transition latency
 *
 * returns 0 on failure, else transition latency in 10^(-9) s = nanoseconds
 */
extern unsigned long cpufreq_get_transition_latency(unsigned int cpu);


/* determine hardware CPU frequency limits
 *
 * These may be limited further by thermal, energy or other
 * considerations by cpufreq policy notifiers in the kernel.
 */

extern int cpufreq_get_hardware_limits(unsigned int cpu,
				unsigned long *min,
				unsigned long *max);


/* determine CPUfreq driver used
 *
 * Remember to call cpufreq_put_driver when no longer needed
 * to avoid memory leakage, please.
 */

extern char *cpufreq_get_driver(unsigned int cpu);

extern void cpufreq_put_driver(char *ptr);


/* determine CPUfreq policy currently used
 *
 * Remember to call cpufreq_put_policy when no longer needed
 * to avoid memory leakage, please.
 */


extern struct cpufreq_policy *cpufreq_get_policy(unsigned int cpu);

extern void cpufreq_put_policy(struct cpufreq_policy *policy);


/* determine CPUfreq governors currently available
 *
 * may be modified by modprobe'ing or rmmod'ing other governors. Please
 * free allocated memory by calling cpufreq_put_available_governors
 * after use.
 */


extern struct cpufreq_available_governors
*cpufreq_get_available_governors(unsigned int cpu);

extern void cpufreq_put_available_governors(
	struct cpufreq_available_governors *first);


/* determine CPU frequency states available
 *
 * Only present on _some_ ->target() cpufreq drivers. For information purposes
 * only. Please free allocated memory by calling
 * cpufreq_put_available_frequencies after use.
 */

extern struct cpufreq_available_frequencies
*cpufreq_get_available_frequencies(unsigned int cpu);

extern void cpufreq_put_available_frequencies(
		struct cpufreq_available_frequencies *first);


/* determine affected CPUs
 *
 * Remember to call cpufreq_put_affected_cpus when no longer needed
 * to avoid memory leakage, please.
 */

extern struct cpufreq_affected_cpus *cpufreq_get_affected_cpus(unsigned
							int cpu);

extern void cpufreq_put_affected_cpus(struct cpufreq_affected_cpus *first);


/* determine related CPUs
 *
 * Remember to call cpufreq_put_related_cpus when no longer needed
 * to avoid memory leakage, please.
 */

extern struct cpufreq_affected_cpus *cpufreq_get_related_cpus(unsigned
							int cpu);

extern void cpufreq_put_related_cpus(struct cpufreq_affected_cpus *first);


/* determine stats for cpufreq subsystem
 *
 * This is not available in all kernel versions or configurations.
 */

extern struct cpufreq_stats *cpufreq_get_stats(unsigned int cpu,
					unsigned long long *total_time);

extern void cpufreq_put_stats(struct cpufreq_stats *stats);

extern unsigned long cpufreq_get_transitions(unsigned int cpu);


/* set new cpufreq policy
 *
 * Tries to set the passed policy as new policy as close as possible,
 * but results may differ depending e.g. on governors being available.
 */

extern int cpufreq_set_policy(unsigned int cpu, struct cpufreq_policy *policy);


/* modify a policy by only changing min/max freq or governor
 *
 * Does not check whether result is what was intended.
 */

extern int cpufreq_modify_policy_min(unsigned int cpu, unsigned long min_freq);
extern int cpufreq_modify_policy_max(unsigned int cpu, unsigned long max_freq);
extern int cpufreq_modify_policy_governor(unsigned int cpu, char *governor);


/* set a specific frequency
 *
 * Does only work if userspace governor can be used and no external
 * interference (other calls to this function or to set/modify_policy)
 * occurs. Also does not work on ->range() cpufreq drivers.
 */

extern int cpufreq_set_frequency(unsigned int cpu,
				unsigned long target_frequency);

#ifdef __cplusplus
}
#endif

#endif /* _CPUFREQ_H */
