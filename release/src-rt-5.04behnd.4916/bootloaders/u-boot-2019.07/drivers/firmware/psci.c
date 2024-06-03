// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * Based on drivers/firmware/psci.c from Linux:
 * Copyright (C) 2015 ARM Limited
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <efi_loader.h>
#include <linux/libfdt.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <linux/psci.h>

#define DRIVER_NAME "psci"

#define PSCI_METHOD_HVC 1
#define PSCI_METHOD_SMC 2

int __efi_runtime_data psci_method;

unsigned long __efi_runtime invoke_psci_fn
		(unsigned long function_id, unsigned long arg0,
		 unsigned long arg1, unsigned long arg2)
{
	struct arm_smccc_res res;

	/*
	 * In the __efi_runtime we need to avoid the switch statement. In some
	 * cases the compiler creates lookup tables to implement switch. These
	 * tables are not correctly relocated when SetVirtualAddressMap is
	 * called.
	 */
	if (psci_method == PSCI_METHOD_SMC)
		arm_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	else if (psci_method == PSCI_METHOD_HVC)
		arm_smccc_hvc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	else
		res.a0 = PSCI_RET_DISABLED;
	return res.a0;
}

static int psci_bind(struct udevice *dev)
{
	/* No SYSTEM_RESET support for PSCI 0.1 */
	if (device_is_compatible(dev, "arm,psci-0.2") ||
	    device_is_compatible(dev, "arm,psci-1.0")) {
		int ret;

		/* bind psci-sysreset optionally */
		ret = device_bind_driver(dev, "psci-sysreset", "psci-sysreset",
					 NULL);
		if (ret)
			pr_debug("PSCI System Reset was not bound.\n");
	}

	return 0;
}

static int psci_probe(struct udevice *dev)
{
	DECLARE_GLOBAL_DATA_PTR;
	const char *method;

	method = fdt_stringlist_get(gd->fdt_blob, dev_of_offset(dev), "method",
				    0, NULL);
	if (!method) {
		pr_warn("missing \"method\" property\n");
		return -ENXIO;
	}

	if (!strcmp("hvc", method)) {
		psci_method = PSCI_METHOD_HVC;
	} else if (!strcmp("smc", method)) {
		psci_method = PSCI_METHOD_SMC;
	} else {
		pr_warn("invalid \"method\" property: %s\n", method);
		return -EINVAL;
	}

	return 0;
}

/**
 * void do_psci_probe() - probe PSCI firmware driver
 *
 * Ensure that psci_method is initialized.
 */
static void __maybe_unused do_psci_probe(void)
{
	struct udevice *dev;

	uclass_get_device_by_name(UCLASS_FIRMWARE, DRIVER_NAME, &dev);
}

#if IS_ENABLED(CONFIG_EFI_LOADER) && IS_ENABLED(CONFIG_PSCI_RESET)
efi_status_t efi_reset_system_init(void)
{
	do_psci_probe();
	return EFI_SUCCESS;
}

void __efi_runtime EFIAPI efi_reset_system(enum efi_reset_type reset_type,
					   efi_status_t reset_status,
					   unsigned long data_size,
					   void *reset_data)
{
	if (reset_type == EFI_RESET_COLD ||
	    reset_type == EFI_RESET_WARM ||
	    reset_type == EFI_RESET_PLATFORM_SPECIFIC) {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
	} else if (reset_type == EFI_RESET_SHUTDOWN) {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
	}
	while (1)
		;
}
#endif /* IS_ENABLED(CONFIG_EFI_LOADER) && IS_ENABLED(CONFIG_PSCI_RESET) */

#ifdef CONFIG_PSCI_RESET
void reset_misc(void)
{
	do_psci_probe();
	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
}
#endif /* CONFIG_PSCI_RESET */

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	do_psci_probe();

	puts("poweroff ...\n");
	udelay(50000); /* wait 50 ms */

	disable_interrupts();
	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
	enable_interrupts();

	log_err("Power off not supported on this platform\n");
	return CMD_RET_FAILURE;
}
#endif

static const struct udevice_id psci_of_match[] = {
	{ .compatible = "arm,psci" },
	{ .compatible = "arm,psci-0.2" },
	{ .compatible = "arm,psci-1.0" },
	{},
};

U_BOOT_DRIVER(psci) = {
	.name = DRIVER_NAME,
	.id = UCLASS_FIRMWARE,
	.of_match = psci_of_match,
	.bind = psci_bind,
	.probe = psci_probe,
};
