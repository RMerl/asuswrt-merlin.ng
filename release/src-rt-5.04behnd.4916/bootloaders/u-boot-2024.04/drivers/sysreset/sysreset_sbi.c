// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <asm/sbi.h>

static enum sbi_srst_reset_type reset_type_map[SYSRESET_COUNT] = {
	[SYSRESET_WARM] = SBI_SRST_RESET_TYPE_WARM_REBOOT,
	[SYSRESET_COLD] = SBI_SRST_RESET_TYPE_COLD_REBOOT,
	[SYSRESET_POWER] = SBI_SRST_RESET_TYPE_COLD_REBOOT,
	[SYSRESET_POWER_OFF] = SBI_SRST_RESET_TYPE_SHUTDOWN,
};

static int sbi_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	enum sbi_srst_reset_type reset_type;

	reset_type = reset_type_map[type];
	sbi_srst_reset(reset_type, SBI_SRST_RESET_REASON_NONE);

	return -EINPROGRESS;
}

static int sbi_sysreset_probe(struct udevice *dev)
{
	long have_reset;

	have_reset = sbi_probe_extension(SBI_EXT_SRST);
	if (have_reset)
		return 0;

	log_warning("SBI has no system reset extension\n");
	return -ENOENT;
}

static struct sysreset_ops sbi_sysreset_ops = {
	.request = sbi_sysreset_request,
};

U_BOOT_DRIVER(sbi_sysreset) = {
	.name = "sbi-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &sbi_sysreset_ops,
	.probe = sbi_sysreset_probe,
};
