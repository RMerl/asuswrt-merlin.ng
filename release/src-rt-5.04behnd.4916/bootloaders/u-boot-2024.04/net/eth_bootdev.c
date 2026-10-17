// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for ethernet (uses PXE)
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <command.h>
#include <bootmeth.h>
#include <dm.h>
#include <extlinux.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <test/test.h>

static int eth_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			    struct bootflow *bflow)
{
	char name[60];
	int ret;

	/* Must be an Ethernet device */
	ret = bootflow_iter_check_net(iter);
	if (ret)
		return log_msg_ret("net", ret);

	ret = bootmeth_check(bflow->method, iter);
	if (ret)
		return log_msg_ret("check", ret);

	/*
	 * Like extlinux boot, this assumes there is only one Ethernet device.
	 * In this case, that means that @eth is ignored
	 */

	snprintf(name, sizeof(name), "%s.%d", dev->name, iter->part);
	bflow->name = strdup(name);
	if (!bflow->name)
		return log_msg_ret("name", -ENOMEM);

	/* See distro_pxe_read_bootflow() for the standard impl of this */
	log_debug("dhcp complete - reading bootflow with method '%s'\n",
		  bflow->method->name);
	ret = bootmeth_read_bootflow(bflow->method, bflow);
	log_debug("reading bootflow returned %d\n", ret);
	if (ret)
		return log_msg_ret("method", ret);

	return 0;
}

static int eth_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_6_NET_BASE;

	return 0;
}

static int eth_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	if (!test_eth_enabled())
		return 0;

	/* init PCI first since this is often used to provide Ehternet */
	if (IS_ENABLED(CONFIG_PCI)) {
		ret = pci_init();
		if (ret)
			log_warning("Failed to init PCI (%dE)\n", ret);
	}

	/*
	 * Ethernet devices can also come from USB, but that is a higher
	 * priority (BOOTDEVP_5_SCAN_SLOW) than ethernet, so it should have been
	 * enumerated already. If something like 'bootflow scan dhcp' is used
	 * then the user will need to run 'usb start' first.
	 */
	if (IS_ENABLED(CONFIG_CMD_DHCP)) {
		ret = dhcp_run(0, NULL, false);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

struct bootdev_ops eth_bootdev_ops = {
	.get_bootflow	= eth_get_bootflow,
};

static const struct udevice_id eth_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-eth" },
	{ }
};

U_BOOT_DRIVER(eth_bootdev) = {
	.name		= "eth_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &eth_bootdev_ops,
	.bind		= eth_bootdev_bind,
	.of_match	= eth_bootdev_ids,
};

BOOTDEV_HUNTER(eth_bootdev_hunt) = {
	.prio		= BOOTDEVP_6_NET_BASE,
	.uclass		= UCLASS_ETH,
	.hunt		= eth_bootdev_hunt,
	.drv		= DM_DRIVER_REF(eth_bootdev),
};
