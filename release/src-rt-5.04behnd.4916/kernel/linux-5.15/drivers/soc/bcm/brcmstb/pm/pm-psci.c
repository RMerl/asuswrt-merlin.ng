// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB PSCI based system wide PM support
 *
 * Copyright © 2018 Broadcom
 */

#define pr_fmt(fmt) "brcmstb-pm-psci: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/arm-smccc.h>
#include <linux/psci.h>
#include <linux/pm.h>
#include <linux/soc/brcmstb/aon_defs.h>
#include <linux/soc/brcmstb/brcmstb.h>
#include <linux/soc/brcmstb/brcmstb-smccc.h>
#include <linux/reboot.h>
#include <linux/syscore_ops.h>
#include <linux/kobject.h>
#include <linux/panic_notifier.h>

#include <uapi/linux/psci.h>

#include <asm/suspend.h>
#include <asm/system_misc.h>

#include "pm-common.h"

static psci_fn *invoke_psci_fn;
static bool brcmstb_psci_system_reset2_supported;
static bool brcmstb_psci_system_suspend_supported;
static u32 brcmstb_psci_sleep_states_supported;

/* Enumeration needs to match with the firmware layout */
enum brcmstb_system_suspend_mode {
	BRCMSTB_SYSTEM_SUSPEND_S3,
	BRCMSTB_SYSTEM_SUSPEND_S2,
	BRCMSTB_SYSTEM_SUSPEND_S2_CPU_OFF,
	BRCMSTB_SYSTEM_SUSPEND_MAX,
};

static const char *brcmstb_system_suspend_modes_str[] = {
	[BRCMSTB_SYSTEM_SUSPEND_S3] = "s3",
	[BRCMSTB_SYSTEM_SUSPEND_S2] = "s2",
	[BRCMSTB_SYSTEM_SUSPEND_S2_CPU_OFF] = "s2-cpu-off",
};

static enum brcmstb_system_suspend_mode brcmstb_system_suspend_mode;

static u32 brcmstb_system_suspend_mode_pstate(void)
{
	/* We always have a system wide state */
	u32 pstate = 2 << PSCI_0_2_POWER_STATE_AFFL_SHIFT;

	switch (brcmstb_system_suspend_mode) {
	case BRCMSTB_SYSTEM_SUSPEND_S3:
		break;
	case BRCMSTB_SYSTEM_SUSPEND_S2:
		/* Retention */
		pstate |= 1 << PSCI_0_2_POWER_STATE_TYPE_SHIFT | 1;
		break;
	case BRCMSTB_SYSTEM_SUSPEND_S2_CPU_OFF:
		/* No retention */
		pstate |= 0 << PSCI_0_2_POWER_STATE_TYPE_SHIFT | 2;
		break;
	default:
		break;
	}

	return pstate;
}

static int brcmstb_psci_integ_region(unsigned long function_id,
				     unsigned long base,
				     unsigned long size)
{
	unsigned long end;

	if (!size)
		return -EINVAL;

	end = DIV_ROUND_UP(base + size, SIP_MIN_REGION_SIZE);
	base /= SIP_MIN_REGION_SIZE;
	size = end - base;

	return invoke_psci_fn(function_id, base, size, 0);
}

static int brcmstb_psci_integ_region_set(unsigned long base,
					 unsigned long size)
{
	return brcmstb_psci_integ_region(SIP_FUNC_INTEG_REGION_SET, base, size);
}

static int brcmstb_psci_integ_region_del(unsigned long base,
					 unsigned long size)
{
	return brcmstb_psci_integ_region(SIP_FUNC_INTEG_REGION_DEL, base, size);
}

static int brcmstb_psci_integ_region_reset_all(void)
{
	return invoke_psci_fn(SIP_FUNC_INTEG_REGION_RESET_ALL, 0, 0, 0);
}

static int brcmstb_psci_system_mem_finish(void)
{
	struct dma_region combined_regions[MAX_EXCLUDE + MAX_REGION + MAX_EXTRA];
	const int max = ARRAY_SIZE(combined_regions);
	unsigned int i;
	int nregs, ret;
	u32 pstate;

	/* Skip supplying DRAM regions to hash unless we are going to
	 * enter S3 standby.
	 */
	if (brcmstb_system_suspend_mode != BRCMSTB_SYSTEM_SUSPEND_S3)
		goto no_s3;

	nregs = configure_main_hash(combined_regions, max,
				    exclusions, num_exclusions);
	if (nregs < 0)
		return nregs;

	for (i = 0; i < num_regions && nregs + i < max; i++)
		combined_regions[nregs + i] = regions[i];
	nregs += i;

	for (i = 0; i < nregs; i++) {
		ret = brcmstb_psci_integ_region_set(combined_regions[i].addr,
						    combined_regions[i].len);
		if (ret != PSCI_RET_SUCCESS) {
			pr_err("Error setting combined region %d\n", i);
			continue;
		}
	}

	for (i = 0; i < num_exclusions; i++) {
		ret = brcmstb_psci_integ_region_del(exclusions[i].addr,
						    exclusions[i].len);
		if (ret != PSCI_RET_SUCCESS) {
			pr_err("Error removing exclusion region %d\n", i);
			continue;
		}
	}

	/* Not all firmware versions support that SiP call so do not
	 * make it fatal.
	 */
no_s3:
	pstate = brcmstb_system_suspend_mode_pstate();
	(void)invoke_psci_fn(SIP_FUNC_PSCI_SYSTEM_SLEEP_MODIFY, pstate, 0, 0);

	return 0;
}

static int brcmstb_psci_sys_reset(struct notifier_block *nb,
				  unsigned long action,
				  void *data)
{
	const char *cmd = data;

	/*
	 * reset_type[31] = 0 (architectural)
	 * reset_type[30:0] = 0 (SYSTEM_WARM_RESET)
	 * cookie = 0 (ignored by the implementation)
	 */
	uint32_t reboot_type = 0;


	if ((action == REBOOT_COLD || action == REBOOT_WARM ||
	    action == REBOOT_SOFT) &&
	    brcmstb_psci_system_reset2_supported) {
		if (cmd && !strcmp(cmd, "powercycle"))
			reboot_type = BIT(31) | 1;
		invoke_psci_fn(PSCI_FN_NATIVE(1_1, SYSTEM_RESET2), reboot_type, 0, 0);
	} else {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
	}

	return NOTIFY_DONE;
}

static struct notifier_block brcmstb_psci_sys_reset_nb = {
	.notifier_call = brcmstb_psci_sys_reset,
	.priority = 255,
};

void brcmstb_psci_sys_poweroff(void)
{
	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
}

static int psci_features(u32 psci_func_id)
{
	u32 features_func_id;

	switch (ARM_SMCCC_OWNER_NUM(psci_func_id)) {
	case ARM_SMCCC_OWNER_SIP:
		features_func_id = SIP_FUNC_PSCI_FEATURES;
		break;
	case ARM_SMCCC_OWNER_STANDARD:
		features_func_id = PSCI_1_0_FN_PSCI_FEATURES;
		break;
	default:
		return PSCI_RET_NOT_SUPPORTED;
	}

	return invoke_psci_fn(features_func_id, psci_func_id, 0, 0);
}

static int brcmstb_psci_panic_notify(struct notifier_block *nb,
				     unsigned long action, void *data)
{
	int ret;

	ret = invoke_psci_fn(SIP_FUNC_PANIC_NOTIFY, BRCMSTB_PANIC_MAGIC, 0, 0);
	if (ret != PSCI_RET_SUCCESS)
		return NOTIFY_BAD;

	return NOTIFY_DONE;
}

static struct notifier_block brcmstb_psci_nb = {
	.notifier_call = brcmstb_psci_panic_notify,
};

static void brcmstb_psci_discover_sleep_states_supported(void)
{
	struct arm_smccc_res res = { };

	if (invoke_psci_fn == __invoke_psci_fn_hvc)
		arm_smccc_hvc(SIP_FUNC_PSCI_SYSTEM_SLEEP_SUPPORTED,
			      0, 0, 0, 0, 0, 0, 0, &res);
	else
		arm_smccc_smc(SIP_FUNC_PSCI_SYSTEM_SLEEP_SUPPORTED,
			      0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 != PSCI_RET_SUCCESS) {
		brcmstb_psci_sleep_states_supported = ~0;
		return;
	}

	brcmstb_psci_sleep_states_supported = res.a1;
}

static bool brcmstb_psci_sleep_state_supported(unsigned int i)
{
	return !!(brcmstb_psci_sleep_states_supported & (1 << i));
}

static ssize_t brcmstb_psci_version_show(struct kobject *kobj,
					 struct kobj_attribute *attr,
					 char *buf)
{
	struct arm_smccc_res res = { };
	u32 version;

	if (invoke_psci_fn == __invoke_psci_fn_hvc)
		arm_smccc_hvc(SIP_FUNC_PSCI_BRCMSTB_VERSION,
			      0, 0, 0, 0, 0, 0, 0, &res);
	else
		arm_smccc_smc(SIP_FUNC_PSCI_BRCMSTB_VERSION,
			      0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 != PSCI_RET_SUCCESS)
		return -EOPNOTSUPP;

	version = res.a1;

	return sprintf(buf, "%d.%d.%d.%d\n",
		       (version >> 24) & 0xff, (version >> 16) & 0xff,
		       (version >> 8) & 0xff, version & 0xff);
}

static struct kobj_attribute brcmstb_psci_version_attr =
	__ATTR(mon_version, 0400, brcmstb_psci_version_show, NULL);

static ssize_t brcmstb_system_suspend_mode_show(struct kobject *kobj,
						struct kobj_attribute *attr,
						char *buf)
{
	bool current_mode = false;
	unsigned int i;
	char *s = buf;

	for (i = 0; i < ARRAY_SIZE(brcmstb_system_suspend_modes_str); i++) {
		if (!brcmstb_psci_sleep_state_supported(i))
			continue;

		current_mode = brcmstb_system_suspend_mode == i;

		s += sprintf(s, "%s%s%s",
			     current_mode ? "[" : "",
			     brcmstb_system_suspend_modes_str[i],
			     current_mode ? "] " : " ");
	}

	if (s != buf)
		*(s - 1) = '\n';

	return (s - buf);
}

static ssize_t brcmstb_system_suspend_mode_store(struct kobject *kobj,
						 struct kobj_attribute *attr,
						 const char *buf, size_t count)
{
	const char *p, *mode;
	unsigned int i;
	int len;

	/* Take out of \n if present for length computations */
	p = memchr(buf, '\n', count);
	len = p ? p - buf : count;

	for (i = 0; i < ARRAY_SIZE(brcmstb_system_suspend_modes_str); i++) {
		mode = brcmstb_system_suspend_modes_str[i];
		if (len == strlen(mode) && !strncmp(buf, mode, len))
			break;
	}

	if (i == ARRAY_SIZE(brcmstb_system_suspend_modes_str))
		return -EINVAL;

	if (!brcmstb_psci_sleep_state_supported(i))
		return -EOPNOTSUPP;

	brcmstb_system_suspend_mode = i;

	return count;
}

static struct kobj_attribute brcmstb_system_suspend_mode_attr =
	__ATTR(suspend_mode, 0644, brcmstb_system_suspend_mode_show,
	       brcmstb_system_suspend_mode_store);

#define SEC_TYPE_LEN	4
#define SEC_TYPE_FMT	"%" __stringify(SEC_TYPE_LEN) "s"

static ssize_t brcmstb_sec_subsystem_show(struct kobject *kobj,
					  struct kobj_attribute *attr,
					  char *buf)
{
	struct arm_smccc_res res = { };
	char type[SEC_TYPE_LEN + 1];
	u32 version;

	if (invoke_psci_fn == __invoke_psci_fn_hvc)
		arm_smccc_hvc(SIP_FUNC_PSCI_SEC_SUBSYSTEM,
			      0, 0, 0, 0, 0, 0, 0, &res);
	else
		arm_smccc_smc(SIP_FUNC_PSCI_SEC_SUBSYSTEM,
			      0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 != PSCI_RET_SUCCESS)
		return -EOPNOTSUPP;

	strncpy(type, (char *)&res.a1, sizeof(type) - 1);
	type[sizeof(type) - 1] = '\0';
	version = res.a2;

	if (version >> 8)
		return sprintf(buf, SEC_TYPE_FMT "-%d.%d.%d\n", type,
			       (version >> 8) & 0xf,
			       (version >> 4) & 0xf, version & 0xf);
	else
		return sprintf(buf, SEC_TYPE_FMT "-%d.%d\n", type,
			       (version >> 4) & 0xf, version & 0xf);
}

static struct kobj_attribute brcmstb_sec_subsystem_attr =
	__ATTR(sec_subsystem, 0400, brcmstb_sec_subsystem_show, NULL);

static const struct attribute *brcmstb_psci_attributes[] = {
	&brcmstb_psci_version_attr.attr,
	&brcmstb_system_suspend_mode_attr.attr,
	&brcmstb_sec_subsystem_attr.attr,
	NULL,
};

static struct syscore_ops brcmstb_psci_syscore_ops = {
	.suspend	= brcmstb_psci_system_mem_finish,
};

int brcmstb_pm_psci_init(void)
{
	unsigned long funcs_id[] = {
		PSCI_0_2_FN_SYSTEM_OFF,
		SIP_FUNC_INTEG_REGION_SET,
		SIP_FUNC_INTEG_REGION_DEL,
		SIP_FUNC_INTEG_REGION_RESET_ALL,
	};
	struct kobject *brcmstb_kobj;
	struct arm_smccc_res res = { };
	unsigned int i;
	int ret;

	switch (arm_smccc_1_1_get_conduit()) {
	case SMCCC_CONDUIT_HVC:
		invoke_psci_fn = __invoke_psci_fn_hvc;
		break;
	case SMCCC_CONDUIT_SMC:
		invoke_psci_fn = __invoke_psci_fn_smc;
		break;
	default:
		return -EINVAL;
	}

	/* Check the revision of Mon64 */
	if (invoke_psci_fn == __invoke_psci_fn_hvc)
		arm_smccc_hvc(SIP_SVC_REVISION,
			      0, 0, 0, 0, 0, 0, 0, &res);
	else
		arm_smccc_smc(SIP_SVC_REVISION,
			      0, 0, 0, 0, 0, 0, 0, &res);

	/* Test for our supported features */
	for (i = 0; i < ARRAY_SIZE(funcs_id); i++) {
		ret = psci_features(funcs_id[i]);
		if (ret == PSCI_RET_NOT_SUPPORTED) {
			pr_err("Firmware does not support function 0x%lx\n",
			       funcs_id[i]);
			return -EOPNOTSUPP;
		}
	}

	ret = psci_features(PSCI_FN_NATIVE(1_1, SYSTEM_RESET2));
	if (ret != PSCI_RET_NOT_SUPPORTED)
		brcmstb_psci_system_reset2_supported = true;

	ret = psci_features(PSCI_FN_NATIVE(1_0, SYSTEM_SUSPEND));
	if (ret != PSCI_RET_NOT_SUPPORTED)
		brcmstb_psci_system_suspend_supported = true;

	ret = brcmstb_psci_integ_region_reset_all();
	if (ret != PSCI_RET_SUCCESS) {
		pr_err("Error resetting all integrity checking regions\n");
		return -EIO;
	}

	/* Firmware is new enough to participate in S3/S5, but does not
	 * take over all suspend operations and still needs assistance
	 * from pm-arm.c.
	 */
	if (res.a0 == SIP_REVISION_MAJOR && res.a1 < SIP_REVISION_MINOR) {
		pr_info("Firmware is too old! Please update\n");
		return -EOPNOTSUPP;
	}

	/* Firmware is fully taking over the S2/S3/S5 states and requires
	 * only PSCI calls to enter those states
	 */
	ret = brcmstb_regsave_init();
	if (ret)
		return ret;

	brcmstb_kobj = kobject_create_and_add("brcmstb", firmware_kobj);
	if (brcmstb_kobj) {
		ret = sysfs_create_files(brcmstb_kobj, brcmstb_psci_attributes);
		if (ret) {
			kobject_del(brcmstb_kobj);
			kobject_put(brcmstb_kobj);
			return ret;
		}
	}

	brcmstb_psci_discover_sleep_states_supported();
	pm_power_off = brcmstb_psci_sys_poweroff;
	register_restart_handler(&brcmstb_psci_sys_reset_nb);
	register_syscore_ops(&brcmstb_psci_syscore_ops);
	atomic_notifier_chain_register(&panic_notifier_list,
				       &brcmstb_psci_nb);

	pr_info("Using PSCI based system PM (full featured)\n");

	return 0;
}
module_init(brcmstb_pm_psci_init);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB PSCI extension");
MODULE_LICENSE("GPL v2");
