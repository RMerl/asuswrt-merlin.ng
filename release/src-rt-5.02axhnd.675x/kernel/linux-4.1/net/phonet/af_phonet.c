/*
 * File: af_phonet.c
 *
 * Phonet protocols family
 *
 * Copyright (C) 2008 Nokia Corporation.
 *
 * Authors: Sakari Ailus <sakari.ailus@nokia.com>
 *          Rémi Denis-Courmont
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <net/sock.h>

#include <linux/if_phonet.h>
#include <linux/phonet.h>
#include <net/phonet/phonet.h>
#include <net/phonet/pn_dev.h>

#ifdef CONFIG_BCM_KF_PHONET
#ifdef ACTIVATE_PHONET_DEBUG

phonet_debug_state phonet_dbg_state = OFF;

static ssize_t phonet_show(struct kobject *kobj, struct kobj_attribute *attr,
			   char *buf)
{

	switch (phonet_dbg_state) {
	case ON:
		return sprintf(buf, "on\n");
	case OFF:
		return sprintf(buf, "off\n");
	case DATA:
		return sprintf(buf, "data\n");
	default:
		return -ENODEV;
	}

	return -ENODEV;
}

static ssize_t phonet_store(struct kobject *kobj, struct kobj_attribute *attr,
			    const char *buf, size_t n)
{

	if (sysfs_streq(buf, "on")) {
		phonet_dbg_state = ON;
		pr_alert(
		       "Phonet traces activated\nBe Careful do not trace Dmesg in MTD\n");
	} else if (sysfs_streq(buf, "off")) {
		phonet_dbg_state = OFF;
	} else if (sysfs_streq(buf, "data")) {
		phonet_dbg_state = DATA;
	} else {
		pr_alert("please use  on/off/data\n");
	}
	return -EINVAL;
}

static struct kobj_attribute phonet_attr =
__ATTR(phonet_dbg, 0644, phonet_show, phonet_store);
#endif
#endif /* CONFIG_BCM_KF_PHONET */

/* Transport protocol registration */
static struct phonet_protocol *proto_tab[PHONET_NPROTO] __read_mostly;

static struct phonet_protocol *phonet_proto_get(unsigned int protocol)
{
	struct phonet_protocol *pp;

	if (protocol >= PHONET_NPROTO)
		return NULL;

	rcu_read_lock();
	pp = rcu_dereference(proto_tab[protocol]);
	if (pp && !try_module_get(pp->prot->owner))
		pp = NULL;
	rcu_read_unlock();

	return pp;
}

static inline void phonet_proto_put(struct phonet_protocol *pp)
{
	module_put(pp->prot->owner);
}

/* protocol family functions */

static int pn_socket_create(struct net *net, struct socket *sock, int protocol,
			    int kern)
{
	struct sock *sk;
	struct pn_sock *pn;
	struct phonet_protocol *pnp;
	int err;

#ifdef CONFIG_BCM_KF_PHONET
	/* skip the capable check in order to allow user
	 * applications creating phonet socket */
#else
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
#endif

	if (protocol == 0) {
		/* Default protocol selection */
		switch (sock->type) {
		case SOCK_DGRAM:
			protocol = PN_PROTO_PHONET;
			break;
		case SOCK_SEQPACKET:
			protocol = PN_PROTO_PIPE;
			break;
		default:
			return -EPROTONOSUPPORT;
		}
	}

	pnp = phonet_proto_get(protocol);
	if (pnp == NULL &&
	    request_module("net-pf-%d-proto-%d", PF_PHONET, protocol) == 0)
		pnp = phonet_proto_get(protocol);

	if (pnp == NULL)
		return -EPROTONOSUPPORT;
	if (sock->type != pnp->sock_type) {
		err = -EPROTONOSUPPORT;
		goto out;
	}

	sk = sk_alloc(net, PF_PHONET, GFP_KERNEL, pnp->prot);
	if (sk == NULL) {
		err = -ENOMEM;
		goto out;
	}

	sock_init_data(sock, sk);
	sock->state = SS_UNCONNECTED;
	sock->ops = pnp->ops;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;
	sk->sk_protocol = protocol;
	pn = pn_sk(sk);
	pn->sobject = 0;
	pn->dobject = 0;
	pn->resource = 0;
#ifdef CONFIG_BCM_KF_PHONET
	pn->resource_type = 0;
	pn->resource_subtype = 0;
#endif
	sk->sk_prot->init(sk);
	err = 0;

out:
	phonet_proto_put(pnp);
	return err;
}

static const struct net_proto_family phonet_proto_family = {
	.family = PF_PHONET,
	.create = pn_socket_create,
	.owner = THIS_MODULE,
};

/* Phonet device header operations */
static int pn_header_create(struct sk_buff *skb, struct net_device *dev,
				unsigned short type, const void *daddr,
				const void *saddr, unsigned int len)
{
	u8 *media = skb_push(skb, 1);

	if (type != ETH_P_PHONET)
		return -1;

	if (!saddr)
		saddr = dev->dev_addr;
	*media = *(const u8 *)saddr;
	return 1;
}

static int pn_header_parse(const struct sk_buff *skb, unsigned char *haddr)
{
	const u8 *media = skb_mac_header(skb);
	*haddr = *media;
	return 1;
}

struct header_ops phonet_header_ops = {
	.create = pn_header_create,
	.parse = pn_header_parse,
};
EXPORT_SYMBOL(phonet_header_ops);

/*
 * Prepends an ISI header and sends a datagram.
 */
static int pn_send(struct sk_buff *skb, struct net_device *dev,
			u16 dst, u16 src, u8 res, u8 irq)
{
	struct phonethdr *ph;
	int err;
#ifdef CONFIG_BCM_KF_PHONET
	int i;
#endif

	if (skb->len + 2 > 0xffff /* Phonet length field limit */ ||
	    skb->len + sizeof(struct phonethdr) > dev->mtu) {
		err = -EMSGSIZE;
		goto drop;
	}

	/* Broadcast sending is not implemented */
	if (pn_addr(dst) == PNADDR_BROADCAST) {
		err = -EOPNOTSUPP;
		goto drop;
	}

	skb_reset_transport_header(skb);
	WARN_ON(skb_headroom(skb) & 1); /* HW assumes word alignment */
	skb_push(skb, sizeof(struct phonethdr));
	skb_reset_network_header(skb);
	ph = pn_hdr(skb);
	ph->pn_rdev = pn_dev(dst);
	ph->pn_sdev = pn_dev(src);
	ph->pn_res = res;
	ph->pn_length = __cpu_to_be16(skb->len + 2 - sizeof(*ph));
	ph->pn_robj = pn_obj(dst);
	ph->pn_sobj = pn_obj(src);

	skb->protocol = htons(ETH_P_PHONET);
	skb->priority = 0;
	skb->dev = dev;

#ifdef CONFIG_BCM_KF_PHONET
	PN_PRINTK("pn_send  rdev %x sdev %x res %x robj %x sobj %x netdev=%s\n",
		  ph->pn_rdev, ph->pn_sdev, ph->pn_res, ph->pn_robj,
		  ph->pn_sobj, dev->name);
	PN_DATA_PRINTK("PHONET : skb  data = %d\nPHONET :", skb->len);
	for (i = 1; i <= skb->len; i++) {
		PN_DATA_PRINTK(" %02x", skb->data[i - 1]);
		if ((i % 8) == 0)
			PN_DATA_PRINTK("\n");
	}
#endif

	if (skb->pkt_type == PACKET_LOOPBACK) {
		skb_reset_mac_header(skb);
		skb_orphan(skb);
		err = (irq ? netif_rx(skb) : netif_rx_ni(skb)) ? -ENOBUFS : 0;
	} else {
		err = dev_hard_header(skb, dev, ntohs(skb->protocol),
					NULL, NULL, skb->len);
		if (err < 0) {
			err = -EHOSTUNREACH;
			goto drop;
		}
		err = dev_queue_xmit(skb);
		if (unlikely(err > 0))
			err = net_xmit_errno(err);
	}

	return err;
drop:
	kfree_skb(skb);
	return err;
}

static int pn_raw_send(const void *data, int len, struct net_device *dev,
			u16 dst, u16 src, u8 res)
{
	struct sk_buff *skb = alloc_skb(MAX_PHONET_HEADER + len, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	if (phonet_address_lookup(dev_net(dev), pn_addr(dst)) == 0)
		skb->pkt_type = PACKET_LOOPBACK;

	skb_reserve(skb, MAX_PHONET_HEADER);
	__skb_put(skb, len);
	skb_copy_to_linear_data(skb, data, len);
	return pn_send(skb, dev, dst, src, res, 1);
}

/*
 * Create a Phonet header for the skb and send it out. Returns
 * non-zero error code if failed. The skb is freed then.
 */
int pn_skb_send(struct sock *sk, struct sk_buff *skb,
		const struct sockaddr_pn *target)
{
	struct net *net = sock_net(sk);
	struct net_device *dev;
	struct pn_sock *pn = pn_sk(sk);
	int err;
	u16 src, dst;
	u8 daddr, saddr, res;

	src = pn->sobject;
	if (target != NULL) {
		dst = pn_sockaddr_get_object(target);
		res = pn_sockaddr_get_resource(target);
	} else {
		dst = pn->dobject;
		res = pn->resource;
	}
	daddr = pn_addr(dst);

	err = -EHOSTUNREACH;
	if (sk->sk_bound_dev_if)
		dev = dev_get_by_index(net, sk->sk_bound_dev_if);
	else if (phonet_address_lookup(net, daddr) == 0) {
		dev = phonet_device_get(net);
		skb->pkt_type = PACKET_LOOPBACK;
	} else if (dst == 0) {
		/* Resource routing (small race until phonet_rcv()) */
		struct sock *sk = pn_find_sock_by_res(net, res);
		if (sk)	{
			sock_put(sk);
			dev = phonet_device_get(net);
			skb->pkt_type = PACKET_LOOPBACK;
		} else
			dev = phonet_route_output(net, daddr);
	} else
		dev = phonet_route_output(net, daddr);

	if (!dev || !(dev->flags & IFF_UP))
		goto drop;

	saddr = phonet_address_get(dev, daddr);
	if (saddr == PN_NO_ADDR)
		goto drop;

	if (!pn_addr(src))
#ifdef CONFIG_BCM_KF_PHONET
		src = pn_object(saddr, pn_port(src));
#else
		src = pn_object(saddr, pn_obj(src));
#endif

	err = pn_send(skb, dev, dst, src, res, 0);
	dev_put(dev);
	return err;

drop:
	kfree_skb(skb);
	if (dev)
		dev_put(dev);
	return err;
}
EXPORT_SYMBOL(pn_skb_send);

/* Do not send an error message in response to an error message */
static inline int can_respond(struct sk_buff *skb)
{
	const struct phonethdr *ph;
	const struct phonetmsg *pm;
	u8 submsg_id;

	if (!pskb_may_pull(skb, 3))
		return 0;

	ph = pn_hdr(skb);
	if (ph->pn_res == PN_PREFIX && !pskb_may_pull(skb, 5))
		return 0;
	if (ph->pn_res == PN_COMMGR) /* indications */
		return 0;

	ph = pn_hdr(skb); /* re-acquires the pointer */
	pm = pn_msg(skb);
	if (pm->pn_msg_id != PN_COMMON_MESSAGE)
		return 1;
	submsg_id = (ph->pn_res == PN_PREFIX)
		? pm->pn_e_submsg_id : pm->pn_submsg_id;
	if (submsg_id != PN_COMM_ISA_ENTITY_NOT_REACHABLE_RESP &&
		pm->pn_e_submsg_id != PN_COMM_SERVICE_NOT_IDENTIFIED_RESP)
		return 1;
	return 0;
}

static int send_obj_unreachable(struct sk_buff *rskb)
{
	const struct phonethdr *oph = pn_hdr(rskb);
	const struct phonetmsg *opm = pn_msg(rskb);
	struct phonetmsg resp;

	memset(&resp, 0, sizeof(resp));
	resp.pn_trans_id = opm->pn_trans_id;
	resp.pn_msg_id = PN_COMMON_MESSAGE;
	if (oph->pn_res == PN_PREFIX) {
		resp.pn_e_res_id = opm->pn_e_res_id;
		resp.pn_e_submsg_id = PN_COMM_ISA_ENTITY_NOT_REACHABLE_RESP;
		resp.pn_e_orig_msg_id = opm->pn_msg_id;
		resp.pn_e_status = 0;
	} else {
		resp.pn_submsg_id = PN_COMM_ISA_ENTITY_NOT_REACHABLE_RESP;
		resp.pn_orig_msg_id = opm->pn_msg_id;
		resp.pn_status = 0;
	}
	return pn_raw_send(&resp, sizeof(resp), rskb->dev,
				pn_object(oph->pn_sdev, oph->pn_sobj),
				pn_object(oph->pn_rdev, oph->pn_robj),
				oph->pn_res);
}

static int send_reset_indications(struct sk_buff *rskb)
{
	struct phonethdr *oph = pn_hdr(rskb);
	static const u8 data[4] = {
		0x00 /* trans ID */, 0x10 /* subscribe msg */,
		0x00 /* subscription count */, 0x00 /* dummy */
	};

	return pn_raw_send(data, sizeof(data), rskb->dev,
				pn_object(oph->pn_sdev, 0x00),
				pn_object(oph->pn_rdev, oph->pn_robj),
				PN_COMMGR);
}


/* packet type functions */

/*
 * Stuff received packets to associated sockets.
 * On error, returns non-zero and releases the skb.
 */
static int phonet_rcv(struct sk_buff *skb, struct net_device *dev,
			struct packet_type *pkttype,
			struct net_device *orig_dev)
{
	struct net *net = dev_net(dev);
	struct phonethdr *ph;
	struct sockaddr_pn sa;
	u16 len;
#ifdef CONFIG_BCM_KF_PHONET
	int i;
#endif

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return NET_RX_DROP;

	/* check we have at least a full Phonet header */
	if (!pskb_pull(skb, sizeof(struct phonethdr)))
		goto out;

	/* check that the advertised length is correct */
	ph = pn_hdr(skb);
	len = get_unaligned_be16(&ph->pn_length);
	if (len < 2)
		goto out;
	len -= 2;
	if ((len > skb->len) || pskb_trim(skb, len))
		goto out;
	skb_reset_transport_header(skb);

	pn_skb_get_dst_sockaddr(skb, &sa);

#ifdef CONFIG_BCM_KF_PHONET
	PN_PRINTK(
	    "phonet rcv : phonet hdr rdev %x sdev %x res %x robj %x sobj %x netdev=%s orig_netdev=%s\n",
	     ph->pn_rdev, ph->pn_sdev, ph->pn_res, ph->pn_robj, ph->pn_sobj,
	     dev->name, orig_dev->name);

	PN_DATA_PRINTK("PHONET : skb  data = %d\nPHONET :", skb->len);
	for (i = 1; i <= skb->len; i++) {
		PN_DATA_PRINTK(" %02x", skb->data[i - 1]);
		if ((i % 8) == 0)
			PN_DATA_PRINTK("\n");
	}

	/* check if this is multicasted */
	if (pn_sockaddr_get_object(&sa) == PNOBJECT_MULTICAST) {
		pn_deliver_sock_broadcast(net, skb);
		goto out;
	}
#endif

	/* check if this is broadcasted */
	if (pn_sockaddr_get_addr(&sa) == PNADDR_BROADCAST) {
		pn_deliver_sock_broadcast(net, skb);
		goto out;
	}

	/* resource routing */
	if (pn_sockaddr_get_object(&sa) == 0) {
		struct sock *sk = pn_find_sock_by_res(net, sa.spn_resource);
		if (sk)
			return sk_receive_skb(sk, skb, 0);
	}

	/* check if we are the destination */
	if (phonet_address_lookup(net, pn_sockaddr_get_addr(&sa)) == 0) {
		/* Phonet packet input */
#ifdef CONFIG_BCM_KF_PHONET
		struct sock *sk = pn_find_sock_by_sa_and_skb(net, &sa, skb);
#else
		struct sock *sk = pn_find_sock_by_sa(net, &sa);
#endif

		if (sk)
			return sk_receive_skb(sk, skb, 0);

		if (can_respond(skb)) {
			send_obj_unreachable(skb);
			send_reset_indications(skb);
		}
	} else if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
		goto out; /* Race between address deletion and loopback */
	else {
		/* Phonet packet routing */
		struct net_device *out_dev;

		out_dev = phonet_route_output(net, pn_sockaddr_get_addr(&sa));
		if (!out_dev) {
			net_dbg_ratelimited("No Phonet route to %02X\n",
					    pn_sockaddr_get_addr(&sa));
			goto out;
		}

		__skb_push(skb, sizeof(struct phonethdr));
		skb->dev = out_dev;
		if (out_dev == dev) {
#ifdef CONFIG_BCM_KF_PHONET
			net_dbg_ratelimited(KERN_ERR
				       "Phonet loop to %02X on %s ori=%s\n",
				       pn_sockaddr_get_addr(&sa), dev->name,
				       orig_dev->name);
#else
			net_dbg_ratelimited("Phonet loop to %02X on %s\n",
					    pn_sockaddr_get_addr(&sa),
					    dev->name);
#endif
			goto out_dev;
		}
		/* Some drivers (e.g. TUN) do not allocate HW header space */
		if (skb_cow_head(skb, out_dev->hard_header_len))
			goto out_dev;

		if (dev_hard_header(skb, out_dev, ETH_P_PHONET, NULL, NULL,
					skb->len) < 0)
			goto out_dev;
		dev_queue_xmit(skb);
		dev_put(out_dev);
		return NET_RX_SUCCESS;
out_dev:
		dev_put(out_dev);
	}

out:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static struct packet_type phonet_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_PHONET),
	.func = phonet_rcv,
};

static DEFINE_MUTEX(proto_tab_lock);

int __init_or_module phonet_proto_register(unsigned int protocol,
						struct phonet_protocol *pp)
{
	int err = 0;

	if (protocol >= PHONET_NPROTO)
		return -EINVAL;

	err = proto_register(pp->prot, 1);
	if (err)
		return err;

	mutex_lock(&proto_tab_lock);
	if (proto_tab[protocol])
		err = -EBUSY;
	else
		rcu_assign_pointer(proto_tab[protocol], pp);
	mutex_unlock(&proto_tab_lock);

	return err;
}
EXPORT_SYMBOL(phonet_proto_register);

void phonet_proto_unregister(unsigned int protocol, struct phonet_protocol *pp)
{
	mutex_lock(&proto_tab_lock);
	BUG_ON(proto_tab[protocol] != pp);
	RCU_INIT_POINTER(proto_tab[protocol], NULL);
	mutex_unlock(&proto_tab_lock);
	synchronize_rcu();
	proto_unregister(pp->prot);
}
EXPORT_SYMBOL(phonet_proto_unregister);

/* Module registration */
static int __init phonet_init(void)
{
	int err;
#ifdef CONFIG_BCM_KF_PHONET
#ifdef ACTIVATE_PHONET_DEBUG
	err = sysfs_create_file(kernel_kobj, &phonet_attr.attr);
	if (err)
		pr_alert("phonet sysfs_create_file failed: %d\n", err);
#endif
#endif

	err = phonet_device_init();
	if (err)
		return err;

	pn_sock_init();
	err = sock_register(&phonet_proto_family);
	if (err) {
		printk(KERN_ALERT
			"phonet protocol family initialization failed\n");
		goto err_sock;
	}

	dev_add_pack(&phonet_packet_type);
	phonet_sysctl_init();

	err = isi_register();
	if (err)
		goto err;
	return 0;

err:
	phonet_sysctl_exit();
	sock_unregister(PF_PHONET);
	dev_remove_pack(&phonet_packet_type);
err_sock:
	phonet_device_exit();
	return err;
}

static void __exit phonet_exit(void)
{
	isi_unregister();
	phonet_sysctl_exit();
	sock_unregister(PF_PHONET);
	dev_remove_pack(&phonet_packet_type);
	phonet_device_exit();
}

module_init(phonet_init);
module_exit(phonet_exit);
MODULE_DESCRIPTION("Phonet protocol stack for Linux");
MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_PHONET);
