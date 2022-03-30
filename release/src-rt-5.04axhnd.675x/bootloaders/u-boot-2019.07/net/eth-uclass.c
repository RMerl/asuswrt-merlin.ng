// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <net.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include "eth_internal.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct eth_device_priv - private structure for each Ethernet device
 *
 * @state: The state of the Ethernet MAC driver (defined by enum eth_state_t)
 */
struct eth_device_priv {
	enum eth_state_t state;
};

/**
 * struct eth_uclass_priv - The structure attached to the uclass itself
 *
 * @current: The Ethernet device that the network functions are using
 */
struct eth_uclass_priv {
	struct udevice *current;
};

/* eth_errno - This stores the most recent failure code from DM functions */
static int eth_errno;

static struct eth_uclass_priv *eth_get_uclass_priv(void)
{
	struct uclass *uc;

	uclass_get(UCLASS_ETH, &uc);
	assert(uc);
	return uc->priv;
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
 * to return the first device. If that device doesn't exist or fails to probe,
 * this function will return NULL.
 */
struct udevice *eth_get_dev(void)
{
	struct eth_uclass_priv *uc_priv;

	uc_priv = eth_get_uclass_priv();
	if (!uc_priv->current)
		eth_errno = uclass_first_device(UCLASS_ETH,
				    &uc_priv->current);
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

	/* Must be longer than 3 to be an alias */
	if (!strncmp(devname, "eth", len) && strlen(devname) > len) {
		startp = devname + len;
		seq = simple_strtoul(startp, &endp, 10);
	}

	uclass_get(UCLASS_ETH, &uc);
	uclass_foreach_dev(it, uc) {
		/*
		 * We need the seq to be valid, so try to probe it.
		 * If the probe fails, the seq will not match since it will be
		 * -1 instead of what we are looking for.
		 * We don't care about errors from probe here. Either they won't
		 * match an alias or it will match a literal name and we'll pick
		 * up the error when we try to probe again in eth_set_dev().
		 */
		if (device_probe(it))
			continue;
		/* Check for the name or the sequence number to match */
		if (strcmp(it->name, devname) == 0 ||
		    (endp > startp && it->seq == seq))
			return it;
	}

	return NULL;
}

unsigned char *eth_get_ethaddr(void)
{
	struct eth_pdata *pdata;

	if (eth_get_dev()) {
		pdata = eth_get_dev()->platdata;
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

	priv = current->uclass_priv;
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

	priv = current->uclass_priv;
	priv->state = ETH_STATE_PASSIVE;
}

int eth_get_dev_index(void)
{
	if (eth_get_dev())
		return eth_get_dev()->seq;
	return -1;
}

static int eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata;
	int ret = 0;

	if (!dev || !device_active(dev))
		return -EINVAL;

	/* seq is valid since the device is active */
	if (eth_get_ops(dev)->write_hwaddr && !eth_mac_skip(dev->seq)) {
		pdata = dev->platdata;
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
	index = simple_strtoul(name + 3, NULL, 10);

	retval = uclass_find_device_by_seq(UCLASS_ETH, index, false, &dev);
	if (!retval) {
		struct eth_pdata *pdata = dev->platdata;
		switch (op) {
		case env_op_create:
		case env_op_overwrite:
			eth_parse_enetaddr(value, pdata->enetaddr);
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
			printf("No ethernet found.\n");
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
						current->uclass_priv;

					priv->state = ETH_STATE_ACTIVE;
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
	if (!current || !eth_is_active(current))
		return;

	eth_get_ops(current)->stop(current);
	priv = current->uclass_priv;
	if (priv)
		priv->state = ETH_STATE_PASSIVE;
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
	for (i = 0; i < 32; i++) {
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
		printf("No ethernet found.\n");
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
			if (num_devices)
				printf(", ");

			printf("eth%d: %s", dev->seq, dev->name);

			if (ethprime && dev == prime_dev)
				printf(" [PRIME]");

			eth_write_hwaddr(dev);

			uclass_next_device_check(&dev);
			num_devices++;
		} while (dev);

		putc('\n');
	}

	return num_devices;
}

static int eth_post_bind(struct udevice *dev)
{
	if (strchr(dev->name, ' ')) {
		printf("\nError: eth device name \"%s\" has a space!\n",
		       dev->name);
		return -EINVAL;
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
#if IS_ENABLED(CONFIG_OF_CONTROL)
	const uint8_t *p;

	p = dev_read_u8_array_ptr(dev, "mac-address", ARP_HLEN);
	if (!p)
		p = dev_read_u8_array_ptr(dev, "local-mac-address", ARP_HLEN);

	if (!p)
		return false;

	memcpy(mac, p, ARP_HLEN);

	return true;
#else
	return false;
#endif
}

static int eth_post_probe(struct udevice *dev)
{
	struct eth_device_priv *priv = dev->uclass_priv;
	struct eth_pdata *pdata = dev->platdata;
	unsigned char env_enetaddr[ARP_HLEN];

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct eth_ops *ops = eth_get_ops(dev);
	static int reloc_done;

	if (!reloc_done) {
		if (ops->start)
			ops->start += gd->reloc_off;
		if (ops->send)
			ops->send += gd->reloc_off;
		if (ops->recv)
			ops->recv += gd->reloc_off;
		if (ops->free_pkt)
			ops->free_pkt += gd->reloc_off;
		if (ops->stop)
			ops->stop += gd->reloc_off;
		if (ops->mcast)
			ops->mcast += gd->reloc_off;
		if (ops->write_hwaddr)
			ops->write_hwaddr += gd->reloc_off;
		if (ops->read_rom_hwaddr)
			ops->read_rom_hwaddr += gd->reloc_off;

		reloc_done++;
	}
#endif

	priv->state = ETH_STATE_INIT;

	/* Check if the device has a valid MAC address in device tree */
	if (!eth_dev_get_mac_address(dev, pdata->enetaddr) ||
	    !is_valid_ethaddr(pdata->enetaddr)) {
		/* Check if the device has a MAC address in ROM */
		if (eth_get_ops(dev)->read_rom_hwaddr)
			eth_get_ops(dev)->read_rom_hwaddr(dev);
	}

	eth_env_get_enetaddr_by_index("eth", dev->seq, env_enetaddr);
	if (!is_zero_ethaddr(env_enetaddr)) {
		if (!is_zero_ethaddr(pdata->enetaddr) &&
		    memcmp(pdata->enetaddr, env_enetaddr, ARP_HLEN)) {
			printf("\nWarning: %s MAC addresses don't match:\n",
			       dev->name);
			printf("Address in ROM is          %pM\n",
			       pdata->enetaddr);
			printf("Address in environment is  %pM\n",
			       env_enetaddr);
		}

		/* Override the ROM MAC address */
		memcpy(pdata->enetaddr, env_enetaddr, ARP_HLEN);
	} else if (is_valid_ethaddr(pdata->enetaddr)) {
		eth_env_set_enetaddr_by_index("eth", dev->seq, pdata->enetaddr);
		printf("\nWarning: %s using MAC address from ROM\n",
		       dev->name);
	} else if (is_zero_ethaddr(pdata->enetaddr) ||
		   !is_valid_ethaddr(pdata->enetaddr)) {
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(pdata->enetaddr);
		printf("\nWarning: %s (eth%d) using random MAC address - %pM\n",
		       dev->name, dev->seq, pdata->enetaddr);
#else
		printf("\nError: %s address not set.\n",
		       dev->name);
		return -EINVAL;
#endif
	}

	eth_write_hwaddr(dev);

	return 0;
}

static int eth_pre_remove(struct udevice *dev)
{
	struct eth_pdata *pdata = dev->platdata;

	eth_get_ops(dev)->stop(dev);

	/* clear the MAC address */
	memset(pdata->enetaddr, 0, ARP_HLEN);

	return 0;
}

UCLASS_DRIVER(eth) = {
	.name		= "eth",
	.id		= UCLASS_ETH,
	.post_bind	= eth_post_bind,
	.pre_unbind	= eth_pre_unbind,
	.post_probe	= eth_post_probe,
	.pre_remove	= eth_pre_remove,
	.priv_auto_alloc_size = sizeof(struct eth_uclass_priv),
	.per_device_auto_alloc_size = sizeof(struct eth_device_priv),
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};
