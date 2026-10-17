// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <common.h>
#include <net.h>
#include <net/udp.h>

static struct udp_ops *udp_ops;

int udp_prereq(void)
{
	int ret = 0;

	if (udp_ops->prereq)
		ret = udp_ops->prereq(udp_ops->data);

	return ret;
}

int udp_start(void)
{
	return udp_ops->start(udp_ops->data);
}

int udp_loop(struct udp_ops *ops)
{
	int ret = -1;

	if (!ops) {
		printf("%s: ops should not be null\n", __func__);
		goto out;
	}

	if (!ops->start) {
		printf("%s: no start function defined\n", __func__);
		goto out;
	}

	udp_ops = ops;
	ret = net_loop(UDP);

 out:
	return ret;
}
