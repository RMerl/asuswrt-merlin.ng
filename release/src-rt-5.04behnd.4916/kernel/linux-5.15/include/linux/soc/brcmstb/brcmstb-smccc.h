// SPDX-License-Identifier: GPL-2.0
#ifndef __BRCMSTB_SMCCC_H
#define __BRCMSTB_SMCCC_H

#include <linux/arm-smccc.h>
#include <uapi/linux/psci.h>

#ifdef CONFIG_64BIT
#define PSCI_FN_NATIVE(version, name)   PSCI_##version##_FN64_##name
#else
#define PSCI_FN_NATIVE(version, name)   PSCI_##version##_FN_##name
#endif

/* Broadcom STB custom SIP function calls */
#define SIP_FUNC_INTEG_REGION_SET	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   0)
#define SIP_FUNC_INTEG_REGION_DEL	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   1)
#define SIP_FUNC_INTEG_REGION_RESET_ALL	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   2)
#define SIP_FUNC_PANIC_NOTIFY		\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   3)
#define SIP_FUNC_PSCI_FEATURES		\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   4)
#define SIP_FUNC_PSCI_BRCMSTB_VERSION		\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   5)
#define SIP_FUNC_PSCI_SYSTEM_SLEEP_MODIFY	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   6)
#define SIP_FUNC_PSCI_SYSTEM_SLEEP_SUPPORTED	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   7)
#define SIP_FUNC_PSCI_SEC_SUBSYSTEM		\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   10)

#define SIP_SVC_REVISION		\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
			   IS_ENABLED(CONFIG_64BIT), \
			   ARM_SMCCC_OWNER_SIP, \
			   0xFF02)

#define SIP_MIN_REGION_SIZE	4096
#define SIP_REVISION_MAJOR	0
#define SIP_REVISION_MINOR	2

typedef unsigned long (psci_fn)(unsigned long, unsigned long,
				unsigned long, unsigned long);

static inline unsigned long __invoke_psci_fn_hvc(unsigned long function_id,
						 unsigned long arg0,
						 unsigned long arg1,
						 unsigned long arg2)
{
	struct arm_smccc_res res;

	arm_smccc_hvc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);

	return res.a0;
}

static inline unsigned long __invoke_psci_fn_smc(unsigned long function_id,
						 unsigned long arg0,
						 unsigned long arg1,
						 unsigned long arg2)
{
	struct arm_smccc_res res;

	arm_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);

	return res.a0;
}


#endif /* __BRCMSTB_SMCCC_H */
