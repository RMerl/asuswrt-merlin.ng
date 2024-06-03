// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 * Author: Ruchika Gupta <ruchika.gupta@freescale.com>
 */

#include <common.h>
#include <dm.h>
#include <u-boot/rsa-mod-exp.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/list.h>

int rsa_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
		struct key_prop *node, uint8_t *out)
{
	const struct mod_exp_ops *ops = device_get_ops(dev);

	if (!ops->mod_exp)
		return -ENOSYS;

	return ops->mod_exp(dev, sig, sig_len, node, out);
}

UCLASS_DRIVER(mod_exp) = {
	.id		= UCLASS_MOD_EXP,
	.name		= "rsa_mod_exp",
};
