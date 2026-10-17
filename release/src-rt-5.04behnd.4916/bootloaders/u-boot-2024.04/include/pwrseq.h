/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __pwrseq_h
#define __pwrseq_h

struct pwrseq_ops {
	int (*set_power)(struct udevice *dev, bool enable);
};

#define pwrseq_get_ops(dev)	((struct pwrseq_ops *)(dev)->driver->ops)

int pwrseq_set_power(struct udevice *dev, bool enable);

#endif
