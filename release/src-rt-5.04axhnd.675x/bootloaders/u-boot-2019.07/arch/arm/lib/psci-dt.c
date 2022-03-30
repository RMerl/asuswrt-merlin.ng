// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 NXP Semiconductor, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <linux/sizes.h>
#include <linux/kernel.h>
#include <asm/psci.h>
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#include <asm/armv8/sec_firmware.h>
#endif

int fdt_psci(void *fdt)
{
#if defined(CONFIG_ARMV7_PSCI) || defined(CONFIG_ARMV8_PSCI) || \
	defined(CONFIG_SEC_FIRMWARE_ARMV8_PSCI)
	int nodeoff;
	unsigned int psci_ver = 0;
	int tmp;

	nodeoff = fdt_path_offset(fdt, "/cpus");
	if (nodeoff < 0) {
		printf("couldn't find /cpus\n");
		return nodeoff;
	}

	/* add 'enable-method = "psci"' to each cpu node */
	for (tmp = fdt_first_subnode(fdt, nodeoff);
	     tmp >= 0;
	     tmp = fdt_next_subnode(fdt, tmp)) {
		const struct fdt_property *prop;
		int len;

		prop = fdt_get_property(fdt, tmp, "device_type", &len);
		if (!prop)
			continue;
		if (len < 4)
			continue;
		if (strcmp(prop->data, "cpu"))
			continue;

		/*
		 * Not checking rv here, our approach is to skip over errors in
		 * individual cpu nodes, hopefully some of the nodes are
		 * processed correctly and those will boot
		 */
		fdt_setprop_string(fdt, tmp, "enable-method", "psci");
	}

	nodeoff = fdt_path_offset(fdt, "/psci");
	if (nodeoff >= 0)
		goto init_psci_node;

	nodeoff = fdt_path_offset(fdt, "/");
	if (nodeoff < 0)
		return nodeoff;

	nodeoff = fdt_add_subnode(fdt, nodeoff, "psci");
	if (nodeoff < 0)
		return nodeoff;

init_psci_node:
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
	psci_ver = sec_firmware_support_psci_version();
#elif defined(CONFIG_ARMV7_PSCI_1_0) || defined(CONFIG_ARMV8_PSCI)
	psci_ver = ARM_PSCI_VER_1_0;
#elif defined(CONFIG_ARMV7_PSCI_0_2)
	psci_ver = ARM_PSCI_VER_0_2;
#endif
	if (psci_ver >= ARM_PSCI_VER_1_0) {
		tmp = fdt_setprop_string(fdt, nodeoff,
				"compatible", "arm,psci-1.0");
		if (tmp)
			return tmp;
	}

	if (psci_ver >= ARM_PSCI_VER_0_2) {
		tmp = fdt_appendprop_string(fdt, nodeoff,
				"compatible", "arm,psci-0.2");
		if (tmp)
			return tmp;
	}

#ifndef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
	/*
	 * The Secure firmware framework isn't able to support PSCI version 0.1.
	 */
	if (psci_ver < ARM_PSCI_VER_0_2) {
		tmp = fdt_appendprop_string(fdt, nodeoff,
				"compatible", "arm,psci");
		if (tmp)
			return tmp;
		tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_suspend",
				ARM_PSCI_FN_CPU_SUSPEND);
		if (tmp)
			return tmp;
		tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_off",
				ARM_PSCI_FN_CPU_OFF);
		if (tmp)
			return tmp;
		tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_on",
				ARM_PSCI_FN_CPU_ON);
		if (tmp)
			return tmp;
		tmp = fdt_setprop_u32(fdt, nodeoff, "migrate",
				ARM_PSCI_FN_MIGRATE);
		if (tmp)
			return tmp;
	}
#endif

	tmp = fdt_setprop_string(fdt, nodeoff, "method", "smc");
	if (tmp)
		return tmp;

	tmp = fdt_setprop_string(fdt, nodeoff, "status", "okay");
	if (tmp)
		return tmp;

#endif
	return 0;
}
