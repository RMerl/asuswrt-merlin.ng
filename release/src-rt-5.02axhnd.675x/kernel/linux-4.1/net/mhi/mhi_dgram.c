#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/*
 * File: mhi_dgram.c
 *
 * DGRAM socket implementation for MHI protocol family.
 *
 * It uses the MHI socket framework in mhi_socket.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/socket.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>

#include <asm/ioctls.h>

#include <net/af_mhi.h>
#include <net/mhi/sock.h>
#include <net/mhi/dgram.h>

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("MHI/DGRAM: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

/*** Prototypes ***/

static struct proto mhi_dgram_proto;

static void mhi_dgram_destruct(struct sock *sk);

/*** Functions ***/

int mhi_dgram_sock_create(struct net *net,
			  struct socket *sock, int proto, int kern)
{
	struct sock *sk;
	struct mhi_sock *msk;

	DPRINTK("mhi_dgram_sock_create: proto:%d type:%d\n", proto, sock->type);

	if (sock->type != SOCK_DGRAM)
		return -EPROTONOSUPPORT;

	if (proto == MHI_L3_ANY)
		return -EPROTONOSUPPORT;

	sk = sk_alloc(net, PF_MHI, GFP_KERNEL, &mhi_dgram_proto);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);

	sock->ops = &mhi_socket_ops;
	sock->state = SS_UNCONNECTED;

	sk->sk_protocol = proto;
	sk->sk_destruct = mhi_dgram_destruct;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;

	sk->sk_prot->init(sk);

	msk = mhi_sk(sk);

	msk->sk_l3proto = proto;
	msk->sk_ifindex = -1;

	return 0;
}

static int mhi_dgram_init(struct sock *sk)
{
	return 0;
}

static void mhi_dgram_destruct(struct sock *sk)
{
	skb_queue_purge(&sk->sk_receive_queue);
}

static void mhi_dgram_close(struct sock *sk, long timeout)
{
	sk_common_release(sk);
}

static int mhi_dgram_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
	int err;

	DPRINTK("mhi_dgram_ioctl: cmd:%d arg:%lu\n", cmd, arg);

	switch (cmd) {
	case SIOCOUTQ:
		{
			int len;
			len = sk_wmem_alloc_get(sk);
			err = put_user(len, (int __user *)arg);
		}
		break;

	case SIOCINQ:
		{
			struct sk_buff *skb;
			int len;

			lock_sock(sk);
			{
				skb = skb_peek(&sk->sk_receive_queue);
				len = skb ? skb->len : 0;
			}
			release_sock(sk);

			err = put_user(len, (int __user *)arg);
		}
		break;

	default:
		err = -ENOIOCTLCMD;
	}

	return err;
}

static int mhi_dgram_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
	struct mhi_sock *msk = mhi_sk(sk);
	struct net_device *dev = NULL;
	struct l2muxhdr *l2hdr;
	struct sk_buff *skb;

	int err = -EFAULT;

	if (msg->msg_flags &
	    ~(MSG_DONTWAIT | MSG_EOR | MSG_NOSIGNAL | MSG_CMSG_COMPAT)) {
		pr_warn("mhi_dgram_sendmsg: incompatible socket msg_flags: 0x%08X\n",
		       msg->msg_flags);
		err = -EOPNOTSUPP;
		goto out;
	}

	skb = sock_alloc_send_skb(sk, len + L2MUX_HDR_SIZE,
				  (msg->msg_flags & MSG_DONTWAIT), &err);
	if (!skb) {
		pr_err("mhi_dgram_sendmsg: sock_alloc_send_skb failed: %d\n",
			err);
		goto out;
	}

	skb_reserve(skb, L2MUX_HDR_SIZE);
	skb_reset_transport_header(skb);

	err = memcpy_from_msg((void *)skb_put(skb, len), msg, len);
	if (err < 0) {
		pr_err("mhi_dgram_sendmsg: memcpy_from_msg failed: %d\n",
			err);
		goto drop;
	}

	if (msk->sk_ifindex)
		dev = dev_get_by_index(sock_net(sk), msk->sk_ifindex);

	if (!dev) {
		pr_err("mhi_dgram_sendmsg: no device for ifindex:%d\n",
			msk->sk_ifindex);
		goto drop;
	}

	if (!(dev->flags & IFF_UP)) {
		pr_err("mhi_dgram_sendmsg: device %d not IFF_UP\n",
			msk->sk_ifindex);
		err = -ENETDOWN;
		goto drop;
	}

	if (len + L2MUX_HDR_SIZE > dev->mtu) {
		err = -EMSGSIZE;
		goto drop;
	}

	skb_reset_network_header(skb);

	skb_push(skb, L2MUX_HDR_SIZE);
	skb_reset_mac_header(skb);

	l2hdr = l2mux_hdr(skb);
	l2mux_set_proto(l2hdr, sk->sk_protocol);
	l2mux_set_length(l2hdr, len);

	err = mhi_skb_send(skb, dev, sk->sk_protocol);

	goto put;

drop:
	kfree(skb);
put:
	if (dev)
		dev_put(dev);
out:
	return err;
}

static int mhi_dgram_recvmsg(struct sock *sk,
			     struct msghdr *msg,
			     size_t len, int noblock, int flags, int *addr_len)
{
	struct sk_buff *skb = NULL;
	int cnt, err;

	err = -EOPNOTSUPP;

	if (flags &
	    ~(MSG_PEEK | MSG_TRUNC | MSG_DONTWAIT |
	      MSG_NOSIGNAL | MSG_CMSG_COMPAT)) {
		pr_warn("mhi_dgram_recvmsg: incompatible socket flags: 0x%08X",
			flags);
		goto out2;
	}

	if (addr_len)
		addr_len[0] = 0;

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb)
		goto out2;

	cnt = skb->len - L2MUX_HDR_SIZE;
	if (len < cnt) {
		msg->msg_flags |= MSG_TRUNC;
		cnt = len;
	}

	err = skb_copy_datagram_msg(skb, L2MUX_HDR_SIZE, msg, cnt);
	if (err)
		goto out;

	if (flags & MSG_TRUNC)
		err = skb->len - L2MUX_HDR_SIZE;
	else
		err = cnt;

out:
	skb_free_datagram(sk, skb);
out2:
	return err;
}

static int mhi_dgram_backlog_rcv(struct sock *sk, struct sk_buff *skb)
{
	if (sock_queue_rcv_skb(sk, skb) < 0) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	return NET_RX_SUCCESS;
}

static struct proto mhi_dgram_proto = {
	.name = "MHI-DGRAM",
	.owner = THIS_MODULE,
	.close = mhi_dgram_close,
	.ioctl = mhi_dgram_ioctl,
	.init = mhi_dgram_init,
	.sendmsg = mhi_dgram_sendmsg,
	.recvmsg = mhi_dgram_recvmsg,
	.backlog_rcv = mhi_dgram_backlog_rcv,
	.hash = mhi_sock_hash,
	.unhash = mhi_sock_unhash,
	.obj_size = sizeof(struct mhi_sock),
};

int mhi_dgram_proto_init(void)
{
	DPRINTK("mhi_dgram_proto_init\n");

	return proto_register(&mhi_dgram_proto, 1);
}

void mhi_dgram_proto_exit(void)
{
	DPRINTK("mhi_dgram_proto_exit\n");

	proto_unregister(&mhi_dgram_proto);
}
#endif /* CONFIG_BCM_KF_MHI */
