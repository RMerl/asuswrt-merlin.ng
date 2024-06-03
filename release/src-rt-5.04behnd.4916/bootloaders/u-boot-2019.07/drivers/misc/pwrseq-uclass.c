// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pwrseq.h>

int pwrseq_set_power(struct udevice *dev, bool enable)
{
	struct pwrseq_ops *ops = pwrseq_get_ops(dev);

	if (!ops->set_power)
		return -ENOSYS;

	return ops->set_power(dev, enable);
}

UCLASS_DRIVER(pwrseq) = {
	.id		= UCLASS_PWRSEQ,
	.name		= "pwrseq",
};
