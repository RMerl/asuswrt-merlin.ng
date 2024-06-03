// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * RISC-V privileged architecture defined generic timer driver
 *
 * This driver relies on RISC-V platform codes to provide the essential API
 * riscv_get_time() which is supposed to return the timer counter as defined
 * by the RISC-V privileged architecture spec.
 *
 * This driver can be used in both M-mode and S-mode U-Boot.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/io.h>

/**
 * riscv_get_time() - get the timer counter
 *
 * Platform codes should provide this API in order to make this driver function.
 *
 * @time:	the 64-bit timer count  as defined by the RISC-V privileged
 *		architecture spec.
 * @return:	0 on success, -ve on error.
 */
extern int riscv_get_time(u64 *time);

static int riscv_timer_get_count(struct udevice *dev, u64 *count)
{
	return riscv_get_time(count);
}

static int riscv_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* clock frequency was passed from the cpu driver as driver data */
	uc_priv->clock_rate = dev->driver_data;

	return 0;
}

static const struct timer_ops riscv_timer_ops = {
	.get_count = riscv_timer_get_count,
};

U_BOOT_DRIVER(riscv_timer) = {
	.name = "riscv_timer",
	.id = UCLASS_TIMER,
	.probe = riscv_timer_probe,
	.ops = &riscv_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
