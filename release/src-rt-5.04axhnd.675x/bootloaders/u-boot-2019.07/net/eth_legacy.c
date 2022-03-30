// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <net.h>
#include <phy.h>
#include <linux/errno.h>
#include "eth_internal.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * CPU and board-specific Ethernet initializations.  Aliased function
 * signals caller to move on
 */
static int __def_eth_init(bd_t *bis)
{
	return -1;
}
int cpu_eth_init(bd_t *bis) __attribute__((weak, alias("__def_eth_init")));
int board_eth_init(bd_t *bis) __attribute__((weak, alias("__def_eth_init")));

#ifdef CONFIG_API
static struct {
	uchar data[PKTSIZE];
	int length;
} eth_rcv_bufs[PKTBUFSRX];

static unsigned int eth_rcv_current, eth_rcv_last;
#endif

static struct eth_device *eth_devices;
struct eth_device *eth_current;

void eth_set_current_to_next(void)
{
	eth_current = eth_current->next;
}

void eth_set_dev(struct eth_device *dev)
{
	eth_current = dev;
}

struct eth_device *eth_get_dev_by_name(const char *devname)
{
	struct eth_device *dev, *target_dev;

	BUG_ON(devname == NULL);

	if (!eth_devices)
		return NULL;

	dev = eth_devices;
	target_dev = NULL;
	do {
		if (strcmp(devname, dev->name) == 0) {
			target_dev = dev;
			break;
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return target_dev;
}

struct eth_device *eth_get_dev_by_index(int index)
{
	struct eth_device *dev, *target_dev;

	if (!eth_devices)
		return NULL;

	dev = eth_devices;
	target_dev = NULL;
	do {
		if (dev->index == index) {
			target_dev = dev;
			break;
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return target_dev;
}

int eth_get_dev_index(void)
{
	if (!eth_current)
		return -1;

	return eth_current->index;
}

static int on_ethaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	int index;
	struct eth_device *dev;

	if (!eth_devices)
		return 0;

	/* look for an index after "eth" */
	index = simple_strtoul(name + 3, NULL, 10);

	dev = eth_devices;
	do {
		if (dev->index == index) {
			switch (op) {
			case env_op_create:
			case env_op_overwrite:
				eth_parse_enetaddr(value, dev->enetaddr);
				eth_write_hwaddr(dev, "eth", dev->index);
				break;
			case env_op_delete:
				memset(dev->enetaddr, 0, ARP_HLEN);
			}
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return 0;
}
U_BOOT_ENV_CALLBACK(ethaddr, on_ethaddr);

int eth_write_hwaddr(struct eth_device *dev, const char *base_name,
		   int eth_number)
{
	unsigned char env_enetaddr[ARP_HLEN];
	int ret = 0;

	eth_env_get_enetaddr_by_index(base_name, eth_number, env_enetaddr);

	if (!is_zero_ethaddr(env_enetaddr)) {
		if (!is_zero_ethaddr(dev->enetaddr) &&
		    memcmp(dev->enetaddr, env_enetaddr, ARP_HLEN)) {
			printf("\nWarning: %s MAC addresses don't match:\n",
			       dev->name);
			printf("Address in SROM is         %pM\n",
			       dev->enetaddr);
			printf("Address in environment is  %pM\n",
			       env_enetaddr);
		}

		memcpy(dev->enetaddr, env_enetaddr, ARP_HLEN);
	} else if (is_valid_ethaddr(dev->enetaddr)) {
		eth_env_set_enetaddr_by_index(base_name, eth_number,
					      dev->enetaddr);
	} else if (is_zero_ethaddr(dev->enetaddr)) {
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(dev->enetaddr);
		printf("\nWarning: %s (eth%d) using random MAC address - %pM\n",
		       dev->name, eth_number, dev->enetaddr);
#else
		printf("\nError: %s address not set.\n",
		       dev->name);
		return -EINVAL;
#endif
	}

	if (dev->write_hwaddr && !eth_mac_skip(eth_number)) {
		if (!is_valid_ethaddr(dev->enetaddr)) {
			printf("\nError: %s address %pM illegal value\n",
			       dev->name, dev->enetaddr);
			return -EINVAL;
		}

		ret = dev->write_hwaddr(dev);
		if (ret)
			printf("\nWarning: %s failed to set MAC address\n",
			       dev->name);
	}

	return ret;
}

int eth_register(struct eth_device *dev)
{
	struct eth_device *d;
	static int index;

	assert(strlen(dev->name) < sizeof(dev->name));

	if (!eth_devices) {
		eth_devices = dev;
		eth_current = dev;
		eth_current_changed();
	} else {
		for (d = eth_devices; d->next != eth_devices; d = d->next)
			;
		d->next = dev;
	}

	dev->state = ETH_STATE_INIT;
	dev->next  = eth_devices;
	dev->index = index++;

	return 0;
}

int eth_unregister(struct eth_device *dev)
{
	struct eth_device *cur;

	/* No device */
	if (!eth_devices)
		return -ENODEV;

	for (cur = eth_devices; cur->next != eth_devices && cur->next != dev;
	     cur = cur->next)
		;

	/* Device not found */
	if (cur->next != dev)
		return -ENODEV;

	cur->next = dev->next;

	if (eth_devices == dev)
		eth_devices = dev->next == eth_devices ? NULL : dev->next;

	if (eth_current == dev) {
		eth_current = eth_devices;
		eth_current_changed();
	}

	return 0;
}

int eth_initialize(void)
{
	int num_devices = 0;

	eth_devices = NULL;
	eth_current = NULL;
	eth_common_init();
	/*
	 * If board-specific initialization exists, call it.
	 * If not, call a CPU-specific one
	 */
	if (board_eth_init != __def_eth_init) {
		if (board_eth_init(gd->bd) < 0)
			printf("Board Net Initialization Failed\n");
	} else if (cpu_eth_init != __def_eth_init) {
		if (cpu_eth_init(gd->bd) < 0)
			printf("CPU Net Initialization Failed\n");
	} else {
		printf("Net Initialization Skipped\n");
	}

	if (!eth_devices) {
		puts("No ethernet found.\n");
		bootstage_error(BOOTSTAGE_ID_NET_ETH_START);
	} else {
		struct eth_device *dev = eth_devices;
		char *ethprime = env_get("ethprime");

		bootstage_mark(BOOTSTAGE_ID_NET_ETH_INIT);
		do {
			if (dev->index)
				puts(", ");

			printf("%s", dev->name);

			if (ethprime && strcmp(dev->name, ethprime) == 0) {
				eth_current = dev;
				puts(" [PRIME]");
			}

			if (strchr(dev->name, ' '))
				puts("\nWarning: eth device name has a space!"
					"\n");

			eth_write_hwaddr(dev, "eth", dev->index);

			dev = dev->next;
			num_devices++;
		} while (dev != eth_devices);

		eth_current_changed();
		putc('\n');
	}

	return num_devices;
}

/* Multicast.
 * mcast_addr: multicast ipaddr from which multicast Mac is made
 * join: 1=join, 0=leave.
 */
int eth_mcast_join(struct in_addr mcast_ip, int join)
{
	u8 mcast_mac[ARP_HLEN];
	if (!eth_current || !eth_current->mcast)
		return -1;
	mcast_mac[5] = htonl(mcast_ip.s_addr) & 0xff;
	mcast_mac[4] = (htonl(mcast_ip.s_addr)>>8) & 0xff;
	mcast_mac[3] = (htonl(mcast_ip.s_addr)>>16) & 0x7f;
	mcast_mac[2] = 0x5e;
	mcast_mac[1] = 0x0;
	mcast_mac[0] = 0x1;
	return eth_current->mcast(eth_current, mcast_mac, join);
}

int eth_init(void)
{
	struct eth_device *old_current;

	if (!eth_current) {
		puts("No ethernet found.\n");
		return -ENODEV;
	}

	old_current = eth_current;
	do {
		debug("Trying %s\n", eth_current->name);

		if (eth_current->init(eth_current, gd->bd) >= 0) {
			eth_current->state = ETH_STATE_ACTIVE;

			return 0;
		}
		debug("FAIL\n");

		eth_try_another(0);
	} while (old_current != eth_current);

	return -ETIMEDOUT;
}

void eth_halt(void)
{
	if (!eth_current)
		return;

	eth_current->halt(eth_current);

	eth_current->state = ETH_STATE_PASSIVE;
}

int eth_is_active(struct eth_device *dev)
{
	return dev && dev->state == ETH_STATE_ACTIVE;
}

int eth_send(void *packet, int length)
{
	if (!eth_current)
		return -ENODEV;

	return eth_current->send(eth_current, packet, length);
}

int eth_rx(void)
{
	if (!eth_current)
		return -ENODEV;

	return eth_current->recv(eth_current);
}

#ifdef CONFIG_API
static void eth_save_packet(void *packet, int length)
{
	char *p = packet;
	int i;

	if ((eth_rcv_last+1) % PKTBUFSRX == eth_rcv_current)
		return;

	if (PKTSIZE < length)
		return;

	for (i = 0; i < length; i++)
		eth_rcv_bufs[eth_rcv_last].data[i] = p[i];

	eth_rcv_bufs[eth_rcv_last].length = length;
	eth_rcv_last = (eth_rcv_last + 1) % PKTBUFSRX;
}

int eth_receive(void *packet, int length)
{
	char *p = packet;
	void *pp = push_packet;
	int i;

	if (eth_rcv_current == eth_rcv_last) {
		push_packet = eth_save_packet;
		eth_rx();
		push_packet = pp;

		if (eth_rcv_current == eth_rcv_last)
			return -1;
	}

	length = min(eth_rcv_bufs[eth_rcv_current].length, length);

	for (i = 0; i < length; i++)
		p[i] = eth_rcv_bufs[eth_rcv_current].data[i];

	eth_rcv_current = (eth_rcv_current + 1) % PKTBUFSRX;
	return length;
}
#endif /* CONFIG_API */
