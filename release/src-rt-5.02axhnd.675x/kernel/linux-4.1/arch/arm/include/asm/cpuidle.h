#ifndef __ASM_ARM_CPUIDLE_H
#define __ASM_ARM_CPUIDLE_H

#include <asm/proc-fns.h>

#ifdef CONFIG_CPU_IDLE
extern int arm_cpuidle_simple_enter(struct cpuidle_device *dev,
		struct cpuidle_driver *drv, int index);
#else
static inline int arm_cpuidle_simple_enter(struct cpuidle_device *dev,
		struct cpuidle_driver *drv, int index) { return -ENODEV; }
#endif

/* Common ARM WFI state */
#define ARM_CPUIDLE_WFI_STATE_PWR(p) {\
	.enter                  = arm_cpuidle_simple_enter,\
	.exit_latency           = 1,\
	.target_residency       = 1,\
	.power_usage		= p,\
	.name                   = "WFI",\
	.desc                   = "ARM WFI",\
}

/*
 * in case power_specified == 1, give a default WFI power value needed
 * by some governors
 */
#define ARM_CPUIDLE_WFI_STATE ARM_CPUIDLE_WFI_STATE_PWR(UINT_MAX)

struct device_node;

struct cpuidle_ops {
	int (*suspend)(int cpu, unsigned long arg);
	int (*init)(struct device_node *, int cpu);
};

struct of_cpuidle_method {
	const char *method;
	struct cpuidle_ops *ops;
};

#define CPUIDLE_METHOD_OF_DECLARE(name, _method, _ops)			\
	static const struct of_cpuidle_method __cpuidle_method_of_table_##name \
	__used __section(__cpuidle_method_of_table)			\
	= { .method = _method, .ops = _ops }

extern int arm_cpuidle_suspend(int index);

extern int arm_cpuidle_init(int cpu);

#endif
