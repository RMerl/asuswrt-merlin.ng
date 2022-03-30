// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 National Instruments
 * Copyright (c) 2018 Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <asm/eth-raw-os.h>
#include <dm.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

static int eth_raw_bus_post_bind(struct udevice *dev)
{
	struct sandbox_eth_raw_if_nameindex *ni, *i;
	struct udevice *child;
	struct eth_sandbox_raw_priv *priv;
	char *ub_ifname;
	static const char ub_ifname_pfx[] = "host_";
	u32 skip_localhost = 0;

	ni = sandbox_eth_raw_if_nameindex();
	if (!ni)
		return -EINVAL;

	dev_read_u32(dev, "skip-localhost", &skip_localhost);
	for (i = ni; !(i->if_index == 0 && !i->if_name); i++) {
		int local = sandbox_eth_raw_os_is_local(i->if_name);

		if (local < 0)
			continue;
		if (skip_localhost && local)
			continue;

		ub_ifname = calloc(IFNAMSIZ + sizeof(ub_ifname_pfx), 1);
		strcpy(ub_ifname, ub_ifname_pfx);
		strncat(ub_ifname, i->if_name, IFNAMSIZ);
		device_bind_driver(dev, "eth_sandbox_raw", ub_ifname, &child);

		device_set_name_alloced(child);
		device_probe(child);
		priv = dev_get_priv(child);
		if (priv) {
			strcpy(priv->host_ifname, i->if_name);
			priv->host_ifindex = i->if_index;
			priv->local = local;
		}
	}

	sandbox_eth_raw_if_freenameindex(ni);

	return 0;
}

static const struct udevice_id sandbox_eth_raw_bus_ids[] = {
	{ .compatible = "sandbox,eth-raw-bus" },
	{ }
};

U_BOOT_DRIVER(sandbox_eth_raw_bus) = {
	.name       = "sb_eth_raw_bus",
	.id         = UCLASS_SIMPLE_BUS,
	.of_match   = sandbox_eth_raw_bus_ids,
	.bind       = eth_raw_bus_post_bind,
};
