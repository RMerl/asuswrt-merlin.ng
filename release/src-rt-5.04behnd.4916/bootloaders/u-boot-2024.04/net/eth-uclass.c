// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <bootdev.h>
#include <bootstage.h>
#include <dm.h>
#include <env.h>
#include <log.h>
#include <net.h>
#include <nvmem.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <net/pcap.h>
#include "eth_internal.h"
#include <eth_phy.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct eth_device_priv - private structure for each Ethernet device
 *
 * @state: The state of the Ethernet MAC driver (defined by enum eth_state_t)
 */
struct eth_device_priv {
	enum eth_state_t state;
	bool running;
};

/**
 * struct eth_uclass_priv - The structure attached to the uclass itself
 *
 * @current: The Ethernet device that the network functions are using
 * @no_bootdevs: true to skip binding Ethernet bootdevs (this is a negative flag
 * so that the default value enables it)
 */
struct eth_uclass_priv {
	struct udevice *current;
	bool no_bootdevs;
};

/* eth_errno - This stores the most recent failure code from DM functions */
static int eth_errno;

/* board-specific Ethernet Interface initializations. */
__weak int board_interface_eth_init(struct udevice *dev,
				    phy_interface_t interface_type)
{
	return 0;
}

static struct eth_uclass_priv *eth_get_uclass_priv(void)
{
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_ETH, &uc);
	if (ret)
		return NULL;

	assert(uc);
	return uclass_get_priv(uc);
}

void eth_set_enable_bootdevs(bool enable)
{
	struct eth_uclass_priv *priv = eth_get_uclass_priv();

	if (priv)
		priv->no_bootdevs = !enable;
}

void eth_set_current_to_next(void)
{
	struct eth_uclass_priv *uc_priv;

	uc_priv = eth_get_uclass_priv();
	if (uc_priv->current)
		uclass_next_device(&uc_priv->current);
	if (!uc_priv->current)
		uclass_first_device(UCLASS_ETH, &uc_priv->current);
}

/*
 * Typically this will simply return the active device.
 * In the case where the most recent active device was unset, this will attempt
 * to return the device with sequence id 0 (which can be configured by the
 * device tree). If this fails, fall back to just getting the first device.
 * The latter is non-deterministic and depends on the order of the probing.
 * If that device doesn't exist or fails to probe, this function will return
 * NULL.
 */
struct udevice *eth_get_dev(void)
{
	struct eth_uclass_priv *uc_priv;

	uc_priv = eth_get_uclass_priv();
	if (!uc_priv)
		return NULL;

	if (!uc_priv->current) {
		eth_errno = uclass_get_device_by_seq(UCLASS_ETH, 0,
						     &uc_priv->current);
		if (eth_errno)
			eth_errno = uclass_first_device_err(UCLASS_ETH,
							    &uc_priv->current);
		if (eth_errno)
			uc_priv->current = NULL;
	}
	return uc_priv->current;
}

/*
 * Typically this will just store a device pointer.
 * In case it was not probed, we will attempt to do so.
 * dev may be NULL to unset the active device.
 */
void eth_set_dev(struct udevice *dev)
{
	if (dev && !device_active(dev)) {
		eth_errno = device_probe(dev);
		if (eth_errno)
			dev = NULL;
	}

	eth_get_uclass_priv()->current = dev;
}

/*
 * Find the udevice that either has the name passed in as devname or has an
 * alias named devname.
 */
struct udevice *eth_get_dev_by_name(const char *devname)
{
	int seq = -1;
	char *endp = NULL;
	const char *startp = NULL;
	struct udevice *it;
	struct uclass *uc;
	int len = strlen("eth");
	int ret;

	/* Must be longer than 3 to be an alias */
	if (!strncmp(devname, "eth", len) && strlen(devname) > len) {
		startp = devname + len;
		seq = dectoul(startp, &endp);
	}

	ret = uclass_get(UCLASS_ETH, &uc);
	if (ret)
		return NULL;

	uclass_foreach_dev(it, uc) {
		/*
		 * We don't care about errors from probe here. Either they won't
		 * match an alias or it will match a literal name and we'll pick
		 * up the error when we try to probe again in eth_set_dev().
		 */
		if (device_probe(it))
			continue;
		/* Check for the name or the sequence number to match */
		if (strcmp(it->name, devname) == 0 ||
		    (endp > startp && dev_seq(it) == seq))
			return it;
	}

	return NULL;
}

unsigned char *eth_get_ethaddr(void)
{
	struct eth_pdata *pdata;

	if (eth_get_dev()) {
		pdata = dev_get_plat(eth_get_dev());
		return pdata->enetaddr;
	}

	return NULL;
}

/* Set active state without calling start on the driver */
int eth_init_state_only(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current || !device_active(current))
		return -EINVAL;

	priv = dev_get_uclass_priv(current);
	priv->state = ETH_STATE_ACTIVE;

	return 0;
}

/* Set passive state without calling stop on the driver */
void eth_halt_state_only(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current || !device_active(current))
		return;

	priv = dev_get_uclass_priv(current);
	priv->state = ETH_STATE_PASSIVE;
}

int eth_get_dev_index(void)
{
	if (eth_get_dev())
		return dev_seq(eth_get_dev());
	return -1;
}

static int eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata;
	int ret = 0;

	if (!dev || !device_active(dev))
		return -EINVAL;

	/* seq is valid since the device is active */
	if (eth_get_ops(dev)->write_hwaddr && !eth_mac_skip(dev_seq(dev))) {
		pdata = dev_get_plat(dev);
		if (!is_valid_ethaddr(pdata->enetaddr)) {
			printf("\nError: %s address %pM illegal value\n",
			       dev->name, pdata->enetaddr);
			return -EINVAL;
		}

		/*
		 * Drivers are allowed to decide not to implement this at
		 * run-time. E.g. Some devices may use it and some may not.
		 */
		ret = eth_get_ops(dev)->write_hwaddr(dev);
		if (ret == -ENOSYS)
			ret = 0;
		if (ret)
			printf("\nWarning: %s failed to set MAC address\n",
			       dev->name);
	}

	return ret;
}

static int on_ethaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	int index;
	int retval;
	struct udevice *dev;

	/* look for an index after "eth" */
	index = dectoul(name + 3, NULL);

	retval = uclass_find_device_by_seq(UCLASS_ETH, index, &dev);
	if (!retval) {
		struct eth_pdata *pdata = dev_get_plat(dev);
		switch (op) {
		case env_op_create:
		case env_op_overwrite:
			string_to_enetaddr(value, pdata->enetaddr);
			eth_write_hwaddr(dev);
			break;
		case env_op_delete:
			memset(pdata->enetaddr, 0, ARP_HLEN);
		}
	}

	return 0;
}
U_BOOT_ENV_CALLBACK(ethaddr, on_ethaddr);

int eth_init(void)
{
	char *ethact = env_get("ethact");
	char *ethrotate = env_get("ethrotate");
	struct udevice *current = NULL;
	struct udevice *old_current;
	int ret = -ENODEV;

	/*
	 * When 'ethrotate' variable is set to 'no' and 'ethact' variable
	 * is already set to an ethernet device, we should stick to 'ethact'.
	 */
	if ((ethrotate != NULL) && (strcmp(ethrotate, "no") == 0)) {
		if (ethact) {
			current = eth_get_dev_by_name(ethact);
			if (!current)
				return -EINVAL;
		}
	}

	if (!current) {
		current = eth_get_dev();
		if (!current) {
			log_err("No ethernet found.\n");
			return -ENODEV;
		}
	}

	old_current = current;
	do {
		if (current) {
			debug("Trying %s\n", current->name);

			if (device_active(current)) {
				ret = eth_get_ops(current)->start(current);
				if (ret >= 0) {
					struct eth_device_priv *priv =
						dev_get_uclass_priv(current);

					priv->state = ETH_STATE_ACTIVE;
					priv->running = true;
					return 0;
				}
			} else {
				ret = eth_errno;
			}

			debug("FAIL\n");
		} else {
			debug("PROBE FAIL\n");
		}

		/*
		 * If ethrotate is enabled, this will change "current",
		 * otherwise we will drop out of this while loop immediately
		 */
		eth_try_another(0);
		/* This will ensure the new "current" attempted to probe */
		current = eth_get_dev();
	} while (old_current != current);

	return ret;
}

void eth_halt(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current)
		return;

	priv = dev_get_uclass_priv(current);
	if (!priv || !priv->running)
		return;

	eth_get_ops(current)->stop(current);
	priv->state = ETH_STATE_PASSIVE;
	priv->running = false;
}

int eth_is_active(struct udevice *dev)
{
	struct eth_device_priv *priv;

	if (!dev || !device_active(dev))
		return 0;

	priv = dev_get_uclass_priv(dev);
	return priv->state == ETH_STATE_ACTIVE;
}

int eth_send(void *packet, int length)
{
	struct udevice *current;
	int ret;

	current = eth_get_dev();
	if (!current)
		return -ENODEV;

	if (!eth_is_active(current))
		return -EINVAL;

	ret = eth_get_ops(current)->send(current, packet, length);
	if (ret < 0) {
		/* We cannot completely return the error at present */
		debug("%s: send() returned error %d\n", __func__, ret);
	}
#if defined(CONFIG_CMD_PCAP)
	if (ret >= 0)
		pcap_post(packet, length, true);
#endif
	return ret;
}

int eth_rx(void)
{
	struct udevice *current;
	uchar *packet;
	int flags;
	int ret;
	int i;

	current = eth_get_dev();
	if (!current)
		return -ENODEV;

	if (!eth_is_active(current))
		return -EINVAL;

	/* Process up to 32 packets at one time */
	flags = ETH_RECV_CHECK_DEVICE;
	for (i = 0; i < ETH_PACKETS_BATCH_RECV; i++) {
		ret = eth_get_ops(current)->recv(current, flags, &packet);
		flags = 0;
		if (ret > 0)
			net_process_received_packet(packet, ret);
		if (ret >= 0 && eth_get_ops(current)->free_pkt)
			eth_get_ops(current)->free_pkt(current, packet, ret);
		if (ret <= 0)
			break;
	}
	if (ret == -EAGAIN)
		ret = 0;
	if (ret < 0) {
		/* We cannot completely return the error at present */
		debug("%s: recv() returned error %d\n", __func__, ret);
	}
	return ret;
}

int eth_initialize(void)
{
	int num_devices = 0;
	struct udevice *dev;

	eth_common_init();

	/*
	 * Devices need to write the hwaddr even if not started so that Linux
	 * will have access to the hwaddr that u-boot stored for the device.
	 * This is accomplished by attempting to probe each device and calling
	 * their write_hwaddr() operation.
	 */
	uclass_first_device_check(UCLASS_ETH, &dev);
	if (!dev) {
		log_err("No ethernet found.\n");
		bootstage_error(BOOTSTAGE_ID_NET_ETH_START);
	} else {
		char *ethprime = env_get("ethprime");
		struct udevice *prime_dev = NULL;

		if (ethprime)
			prime_dev = eth_get_dev_by_name(ethprime);
		if (prime_dev) {
			eth_set_dev(prime_dev);
			eth_current_changed();
		} else {
			eth_set_dev(NULL);
		}

		bootstage_mark(BOOTSTAGE_ID_NET_ETH_INIT);
		do {
			if (device_active(dev)) {
				if (num_devices)
					printf(", ");

				printf("eth%d: %s", dev_seq(dev), dev->name);

				if (ethprime && dev == prime_dev)
					printf(" [PRIME]");
			}

			eth_write_hwaddr(dev);

			if (device_active(dev))
				num_devices++;
			uclass_next_device_check(&dev);
		} while (dev);

		if (!num_devices)
			log_err("No ethernet found.\n");
		putc('\n');
	}

	return num_devices;
}

static int eth_post_bind(struct udevice *dev)
{
	struct eth_uclass_priv *priv = uclass_get_priv(dev->uclass);
	int ret;

	if (strchr(dev->name, ' ')) {
		printf("\nError: eth device name \"%s\" has a space!\n",
		       dev->name);
		return -EINVAL;
	}

#ifdef CONFIG_DM_ETH_PHY
	eth_phy_binds_nodes(dev);
#endif
	if (CONFIG_IS_ENABLED(BOOTDEV_ETH) && !priv->no_bootdevs) {
		ret = bootdev_setup_for_dev(dev, "eth_bootdev");
		if (ret)
			return log_msg_ret("bootdev", ret);
	}

	return 0;
}

static int eth_pre_unbind(struct udevice *dev)
{
	/* Don't hang onto a pointer that is going away */
	if (dev == eth_get_uclass_priv()->current)
		eth_set_dev(NULL);

	return 0;
}

static bool eth_dev_get_mac_address(struct udevice *dev, u8 mac[ARP_HLEN])
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	const uint8_t *p;
	struct nvmem_cell mac_cell;

	p = dev_read_u8_array_ptr(dev, "mac-address", ARP_HLEN);
	if (!p)
		p = dev_read_u8_array_ptr(dev, "local-mac-address", ARP_HLEN);

	if (p) {
		memcpy(mac, p, ARP_HLEN);
		return true;
	}

	if (nvmem_cell_get_by_name(dev, "mac-address", &mac_cell))
		return false;

	return !nvmem_cell_read(&mac_cell, mac, ARP_HLEN);
#else
	return false;
#endif
}

static int eth_post_probe(struct udevice *dev)
{
	struct eth_device_priv *priv = dev_get_uclass_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	unsigned char env_enetaddr[ARP_HLEN];
	char *source = "DT";

	priv->state = ETH_STATE_INIT;
	priv->running = false;

	/* Check if the device has a valid MAC address in device tree */
	if (!eth_dev_get_mac_address(dev, pdata->enetaddr) ||
	    !is_valid_ethaddr(pdata->enetaddr)) {
		/* Check if the device has a MAC address in ROM */
		if (eth_get_ops(dev)->read_rom_hwaddr) {
			int ret;

			ret = eth_get_ops(dev)->read_rom_hwaddr(dev);
			if (!ret)
				source = "ROM";
		}
	}

	eth_env_get_enetaddr_by_index("eth", dev_seq(dev), env_enetaddr);
	if (!is_zero_ethaddr(env_enetaddr)) {
		if (!is_zero_ethaddr(pdata->enetaddr) &&
		    memcmp(pdata->enetaddr, env_enetaddr, ARP_HLEN)) {
			printf("\nWarning: %s MAC addresses don't match:\n",
			       dev->name);
			printf("Address in %s is\t\t%pM\n",
			       source, pdata->enetaddr);
			printf("Address in environment is\t%pM\n",
			       env_enetaddr);
		}

		/* Override the ROM MAC address */
		memcpy(pdata->enetaddr, env_enetaddr, ARP_HLEN);
	} else if (is_valid_ethaddr(pdata->enetaddr)) {
		eth_env_set_enetaddr_by_index("eth", dev_seq(dev),
					      pdata->enetaddr);
	} else if (is_zero_ethaddr(pdata->enetaddr) ||
		   !is_valid_ethaddr(pdata->enetaddr)) {
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(pdata->enetaddr);
		printf("\nWarning: %s (eth%d) using random MAC address - %pM\n",
		       dev->name, dev_seq(dev), pdata->enetaddr);
		eth_env_set_enetaddr_by_index("eth", dev_seq(dev),
					      pdata->enetaddr);
#else
		printf("\nError: %s No valid MAC address found.\n",
		       dev->name);
		return -EINVAL;
#endif
	}

	eth_write_hwaddr(dev);

	return 0;
}

static int eth_pre_remove(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	eth_get_ops(dev)->stop(dev);

	/* clear the MAC address */
	memset(pdata->enetaddr, 0, ARP_HLEN);

	return 0;
}

UCLASS_DRIVER(ethernet) = {
	.name		= "ethernet",
	.id		= UCLASS_ETH,
	.post_bind	= eth_post_bind,
	.pre_unbind	= eth_pre_unbind,
	.post_probe	= eth_post_probe,
	.pre_remove	= eth_pre_remove,
	.priv_auto	= sizeof(struct eth_uclass_priv),
	.per_device_auto	= sizeof(struct eth_device_priv),
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};
