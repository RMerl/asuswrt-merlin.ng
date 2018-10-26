#if defined(CONFIG_BCM_KF_ACCEL_PPTP)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <net/protocol.h>

#include "gre.h"

struct gre_protocol *gre_proto[GREPROTO_MAX] ____cacheline_aligned_in_smp;
static DEFINE_SPINLOCK(gre_proto_lock);

int gre_add_protocol(struct gre_protocol *proto, u8 version)
{
	int ret;

	if (version >= GREPROTO_MAX)
		return -EINVAL;
	
	spin_lock(&gre_proto_lock);
	if (gre_proto[version]) {
		ret = -EAGAIN;
	} else {
		rcu_assign_pointer(gre_proto[version], proto);
		ret = 0;
	}
	spin_unlock(&gre_proto_lock);

	return ret;
}

int gre_del_protocol(struct gre_protocol *proto, u8 version)
{
	if (version >= GREPROTO_MAX)
		goto out_err;

	spin_lock(&gre_proto_lock);
	if (gre_proto[version] == proto)
		rcu_assign_pointer(gre_proto[version], NULL);
	else
		goto out_err_unlock;
	spin_unlock(&gre_proto_lock);
	synchronize_rcu();
	return 0;

out_err_unlock:
	spin_unlock(&gre_proto_lock);
out_err:
	return -EINVAL;
}

static int gre_rcv(struct sk_buff *skb)
{
	u8 ver;
	int ret;
	struct gre_protocol *proto;

	if (!pskb_may_pull(skb, 12))
		goto drop_nolock;

	ver = skb->data[1]&0x7f;
	if (ver >= GREPROTO_MAX)
		goto drop_nolock;
	
	rcu_read_lock();
	proto = rcu_dereference(gre_proto[ver]);
	if (!proto || !proto->handler)
		goto drop;

	ret = proto->handler(skb);

	rcu_read_unlock();

	return ret;

drop:
	rcu_read_unlock();
drop_nolock:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static void gre_err(struct sk_buff *skb, u32 info)
{
	u8 ver;
	struct gre_protocol *proto;

	if (!pskb_may_pull(skb, 12))
		goto drop_nolock;

	ver=skb->data[1]&0x7f;
	if (ver>=GREPROTO_MAX)
		goto drop_nolock;
		
	rcu_read_lock();
	proto = rcu_dereference(gre_proto[ver]);
	if (!proto || !proto->err_handler)
		goto drop;

	proto->err_handler(skb, info);
	rcu_read_unlock();

	return;

drop:
	rcu_read_unlock();
drop_nolock:
	kfree_skb(skb);
}

static struct net_protocol net_gre_protocol = {
	.handler	= gre_rcv,
	.err_handler	= gre_err,
	.netns_ok=1,
};

static int __init gre_init(void)
{
	printk(KERN_INFO "GRE over IPv4 demultiplexor driver");

	if (inet_add_protocol(&net_gre_protocol, IPPROTO_GRE) < 0) {
		printk(KERN_INFO "gre: can't add protocol\n");
		return -EAGAIN;
	}
	return 0;
}

static void __exit gre_exit(void)
{
	inet_del_protocol(&net_gre_protocol, IPPROTO_GRE);
}

module_init(gre_init);
module_exit(gre_exit);

MODULE_DESCRIPTION("GRE over IPv4 demultiplexor driver");
MODULE_AUTHOR("Kozlov D. (xeb@mail.ru)");
MODULE_LICENSE("GPL");
EXPORT_SYMBOL_GPL(gre_add_protocol);
EXPORT_SYMBOL_GPL(gre_del_protocol);
#endif
