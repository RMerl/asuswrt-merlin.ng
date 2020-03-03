/*
 * Common code for low-level network console, dump, and debugger code
 *
 * Derived from netconsole, kgdb-over-ethernet, and netdump patches
 */

#ifndef _LINUX_NETPOLL_H
#define _LINUX_NETPOLL_H

#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/rcupdate.h>
#include <linux/list.h>

union inet_addr {
	__u32		all[4];
	__be32		ip;
	__be32		ip6[4];
	struct in_addr	in;
	struct in6_addr	in6;
};

struct netpoll {
	struct net_device *dev;
	char dev_name[IFNAMSIZ];
	const char *name;

	union inet_addr local_ip, remote_ip;
	bool ipv6;
	u16 local_port, remote_port;
	u8 remote_mac[ETH_ALEN];

	struct work_struct cleanup_work;
};

struct netpoll_info {
	atomic_t refcnt;

	struct semaphore dev_lock;

	struct sk_buff_head txq;

	struct delayed_work tx_work;

	struct netpoll *netpoll;
	struct rcu_head rcu;
};

#ifdef CONFIG_NETPOLL
extern void netpoll_poll_disable(struct net_device *dev);
extern void netpoll_poll_enable(struct net_device *dev);
#else
static inline void netpoll_poll_disable(struct net_device *dev) { return; }
static inline void netpoll_poll_enable(struct net_device *dev) { return; }
#endif

void netpoll_send_udp(struct netpoll *np, const char *msg, int len);
void netpoll_print_options(struct netpoll *np);
int netpoll_parse_options(struct netpoll *np, char *opt);
int __netpoll_setup(struct netpoll *np, struct net_device *ndev);
int netpoll_setup(struct netpoll *np);
void __netpoll_cleanup(struct netpoll *np);
void __netpoll_free_async(struct netpoll *np);
void netpoll_cleanup(struct netpoll *np);
void netpoll_send_skb_on_dev(struct netpoll *np, struct sk_buff *skb,
			     struct net_device *dev);
static inline void netpoll_send_skb(struct netpoll *np, struct sk_buff *skb)
{
	unsigned long flags;
	local_irq_save(flags);
	netpoll_send_skb_on_dev(np, skb, np->dev);
	local_irq_restore(flags);
}

#ifdef CONFIG_NETPOLL
static inline void *netpoll_poll_lock(struct napi_struct *napi)
{
	struct net_device *dev = napi->dev;

	if (dev && dev->npinfo) {
		spin_lock(&napi->poll_lock);
		napi->poll_owner = smp_processor_id();
		return napi;
	}
	return NULL;
}

static inline void netpoll_poll_unlock(void *have)
{
	struct napi_struct *napi = have;

	if (napi) {
		napi->poll_owner = -1;
		spin_unlock(&napi->poll_lock);
	}
}

static inline bool netpoll_tx_running(struct net_device *dev)
{
	return irqs_disabled();
}

#else
static inline void *netpoll_poll_lock(struct napi_struct *napi)
{
	return NULL;
}
static inline void netpoll_poll_unlock(void *have)
{
}
static inline void netpoll_netdev_init(struct net_device *dev)
{
}
static inline bool netpoll_tx_running(struct net_device *dev)
{
	return false;
}
#endif

#endif
