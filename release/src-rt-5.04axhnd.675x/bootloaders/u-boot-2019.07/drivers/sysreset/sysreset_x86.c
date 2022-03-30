// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * Generic reset driver for x86 processor
 */

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <pch.h>
#include <sysreset.h>
#include <asm/acpi_s3.h>
#include <asm/io.h>
#include <asm/processor.h>

struct x86_sysreset_platdata {
	struct udevice *pch;
};

/*
 * Power down the machine by using the power management sleep control
 * of the chipset. This will currently only work on Intel chipsets.
 * However, adapting it to new chipsets is fairly simple. You will
 * have to find the IO address of the power management register block
 * in your southbridge, and look up the appropriate SLP_TYP_S5 value
 * from your southbridge's data sheet.
 *
 * This function never returns.
 */
int pch_sysreset_power_off(struct udevice *dev)
{
	struct x86_sysreset_platdata *plat = dev_get_platdata(dev);
	struct pch_pmbase_info pm;
	u32 reg32;
	int ret;

	if (!plat->pch)
		return -ENOENT;
	ret = pch_ioctl(plat->pch, PCH_REQ_PMBASE_INFO, &pm, sizeof(pm));
	if (ret)
		return ret;

	/*
	 * Mask interrupts or system might stay in a coma, not executing code
	 * anymore, but not powered off either.
	 */
	asm("cli");

	/*
	 * Avoid any GPI waking the system from S5* or the system might stay in
	 * a coma
	 */
	outl(0x00000000, pm.base + pm.gpio0_en_ofs);

	/* Clear Power Button Status */
	outw(PWRBTN_STS, pm.base + pm.pm1_sts_ofs);

	/* PMBASE + 4, Bit 10-12, Sleeping Type, * set to 111 -> S5, soft_off */
	reg32 = inl(pm.base + pm.pm1_cnt_ofs);

	/* Set Sleeping Type to S5 (poweroff) */
	reg32 &= ~(SLP_EN | SLP_TYP);
	reg32 |= SLP_TYP_S5;
	outl(reg32, pm.base + pm.pm1_cnt_ofs);

	/* Now set the Sleep Enable bit */
	reg32 |= SLP_EN;
	outl(reg32, pm.base + pm.pm1_cnt_ofs);

	for (;;)
		asm("hlt");
}

static int x86_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int value;
	int ret;

	switch (type) {
	case SYSRESET_WARM:
		value = SYS_RST | RST_CPU;
		break;
	case SYSRESET_COLD:
		value = SYS_RST | RST_CPU | FULL_RST;
		break;
	case SYSRESET_POWER_OFF:
		ret = pch_sysreset_power_off(dev);
		if (ret)
			return ret;
		return -EINPROGRESS;
	default:
		return -ENOSYS;
	}

	outb(value, IO_PORT_RESET);

	return -EINPROGRESS;
}

static int x86_sysreset_get_last(struct udevice *dev)
{
	return SYSRESET_POWER;
}

#ifdef CONFIG_EFI_LOADER
void __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	int value;

	/*
	 * inline this code since we are not caused in the context of a
	 * udevice and passing NULL to x86_sysreset_request() is too horrible.
	 */
	if (reset_type == EFI_RESET_COLD ||
		 reset_type == EFI_RESET_PLATFORM_SPECIFIC)
		value = SYS_RST | RST_CPU | FULL_RST;
	else /* assume EFI_RESET_WARM since we cannot return an error */
		value = SYS_RST | RST_CPU;
	outb(value, IO_PORT_RESET);

	/* TODO EFI_RESET_SHUTDOWN */

	while (1) { }
}
#endif

static int x86_sysreset_probe(struct udevice *dev)
{
	struct x86_sysreset_platdata *plat = dev_get_platdata(dev);

	/* Locate the PCH if there is one. It isn't essential */
	uclass_first_device(UCLASS_PCH, &plat->pch);

	return 0;
}

static const struct udevice_id x86_sysreset_ids[] = {
	{ .compatible = "x86,reset" },
	{ }
};

static struct sysreset_ops x86_sysreset_ops = {
	.request = x86_sysreset_request,
	.get_last = x86_sysreset_get_last,
};

U_BOOT_DRIVER(x86_sysreset) = {
	.name = "x86-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = x86_sysreset_ids,
	.ops = &x86_sysreset_ops,
	.probe = x86_sysreset_probe,
	.platdata_auto_alloc_size	= sizeof(struct x86_sysreset_platdata),
};
