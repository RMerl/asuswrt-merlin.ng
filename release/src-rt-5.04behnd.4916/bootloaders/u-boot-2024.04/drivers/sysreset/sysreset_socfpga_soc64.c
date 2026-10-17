// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Pepperl+Fuchs
 * Simon Goldschmidt <simon.k.r.goldschmidt@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/arch/mailbox_s10.h>

static int socfpga_sysreset_request(struct udevice *dev,
				    enum sysreset_t type)
{
	puts("Mailbox: Issuing mailbox cmd REBOOT_HPS\n");
	mbox_reset_cold();
	return -EINPROGRESS;
}

static struct sysreset_ops socfpga_sysreset = {
	.request = socfpga_sysreset_request,
};

U_BOOT_DRIVER(sysreset_socfpga) = {
	.id	= UCLASS_SYSRESET,
	.name	= "socfpga_sysreset",
	.ops	= &socfpga_sysreset,
};
