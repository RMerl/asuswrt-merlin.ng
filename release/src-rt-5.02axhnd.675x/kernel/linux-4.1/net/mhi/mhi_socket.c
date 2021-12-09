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
 * File: mhi_socket.c
 *
 * Socket layer implementation for AF_MHI.
 *
 * This module implements generic sockets for MHI.
 * The protocol is implemented separately, like mhi_dgram.c.
 *
 * As MHI does not have addressed, the MHI interface is
 * defined by sa_ifindex field in sockaddr_mhi.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/gfp.h>
#include <linux/net.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>
#include <linux/if_mhi.h>

#include <net/tcp_states.h>
#include <net/af_mhi.h>
#include <net/mhi/sock.h>
#include <net/mhi/dgram.h>
#include <net/mhi/raw.h>

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("MHI/SOCKET: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

/* Master lock for MHI sockets */
static DEFINE_SPINLOCK(mhi_sock_lock);

/* List of MHI sockets */
static struct hlist_head mhi_sock_list;

static int mhi_sock_create(struct net *net,
			   struct socket *sock, int proto, int kern)
{
	int err = 0;

	DPRINTK("mhi_sock_create: type:%d proto:%d\n", sock->type, proto);

	if (!capable(CAP_SYS_ADMIN) || !capable(CAP_NET_ADMIN)) {
		pr_warn("AF_MHI: socket create failed: PERMISSION DENIED\n");
		return -EPERM;
	}

	if (!mhi_protocol_registered(proto)) {
		pr_warn("AF_MHI: socket create failed: No support for L2 channel %d\n",
			proto);
		return -EPROTONOSUPPORT;
	}

	if (sock->type == SOCK_DGRAM)
		err = mhi_dgram_sock_create(net, sock, proto, kern);
	else if (sock->type == SOCK_RAW)
		err = mhi_raw_sock_create(net, sock, proto, kern);
	else {
		pr_warn("AF_MHI: trying to create a socket with unknown type %d\n",
			sock->type);
		err = -EPROTONOSUPPORT;
	}

	if (err)
		pr_warn("AF_MHI: socket create failed: %d\n", err);

	return err;
}

static int mhi_sock_release(struct socket *sock)
{
	if (sock->sk) {
		DPRINTK("mhi_sock_release: proto:%d type:%d\n",
			sock->sk->sk_protocol, sock->type);

		sock->sk->sk_prot->close(sock->sk, 0);
		sock->sk = NULL;
	}

	return 0;
}

static int mhi_sock_bind(struct socket *sock, struct sockaddr *addr, int len)
{
	struct sock *sk = sock->sk;
	struct mhi_sock *msk = mhi_sk(sk);
	struct sockaddr_mhi *sam = sa_mhi(addr);

	int err = 0;

	DPRINTK("mhi_sock_bind: proto:%d state:%d\n",
		sk->sk_protocol, sk->sk_state);

	if (sk->sk_prot->bind)
		return sk->sk_prot->bind(sk, addr, len);

	if (len < sizeof(struct sockaddr_mhi))
		return -EINVAL;

	lock_sock(sk);
	{
		if (sk->sk_state == TCP_CLOSE) {
			msk->sk_ifindex = sam->sa_ifindex;
			WARN_ON(sk_hashed(sk));
			sk->sk_prot->hash(sk);
		} else {
			err = -EINVAL;	/* attempt to rebind */
		}
	}
	release_sock(sk);

	return err;
}

int mhi_sock_rcv_unicast(struct sk_buff *skb, u8 l3proto, u32 l3length)
{
	struct sock *sknode;
	struct mhi_sock *msk;

	DPRINTK("mhi_sock_rcv_unicast: proto:%d, len:%d\n", l3proto, l3length);

	spin_lock(&mhi_sock_lock);
	{
		sk_for_each(sknode, &mhi_sock_list) {
			msk = mhi_sk(sknode);
			if ((msk->sk_l3proto == MHI_L3_ANY ||
			     msk->sk_l3proto == l3proto) &&
			    (msk->sk_ifindex == skb->dev->ifindex)) {
				sock_hold(sknode);
				sk_receive_skb(sknode, skb, 0);
				skb = NULL;
				break;
			}
		}
	}
	spin_unlock(&mhi_sock_lock);

	if (skb)
		kfree_skb(skb);

	return NET_RX_SUCCESS;
}

int mhi_sock_rcv_multicast(struct sk_buff *skb, u8 l3proto, u32 l3length)
{
	struct sock *sknode;
	struct mhi_sock *msk;
	struct sk_buff *clone;

	DPRINTK("mhi_sock_rcv_multicast: proto:%d, len:%d\n",
		l3proto, l3length);

	spin_lock(&mhi_sock_lock);
	{
		sk_for_each(sknode, &mhi_sock_list) {
			msk = mhi_sk(sknode);
			if ((msk->sk_l3proto == MHI_L3_ANY ||
			     msk->sk_l3proto == l3proto) &&
			    (msk->sk_ifindex == skb->dev->ifindex)) {
				clone = skb_clone(skb, GFP_ATOMIC);
				if (likely(clone)) {
					sock_hold(sknode);
					sk_receive_skb(sknode, clone, 0);
				}
			}
		}
	}
	spin_unlock(&mhi_sock_lock);

	kfree_skb(skb);

	return NET_RX_SUCCESS;
}

int mhi_sock_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
	DPRINTK("mhi_sock_sendmsg: len:%u\n", len);

	return sock->sk->sk_prot->sendmsg(sock->sk, msg, len);
}

int mhi_sock_recvmsg(struct socket *sock,
		     struct msghdr *msg, size_t len, int flags)
{
	int addrlen = 0;
	int err;

	err = sock->sk->sk_prot->recvmsg(sock->sk, msg, len,
					 flags & MSG_DONTWAIT,
					 flags & ~MSG_DONTWAIT, &addrlen);

	if (err >= 0)
		msg->msg_namelen = addrlen;

	return err;
}

int mhi_getsockopt(struct socket *sock, int level, int optname,
		   char __user *optval, int __user *optlen)
{
	struct sock *sk = sock->sk;
	int len, val;
	void *data;

	if (get_user(len, optlen))
		return -EFAULT;

	if (len < 0)
		return -EINVAL;

	switch (optname) {
	case MHI_DROP_COUNT:
		if (len > sizeof(int))
			len = sizeof(int);
		spin_lock_bh(&sk->sk_receive_queue.lock);
		val = atomic_read(&sk->sk_drops);
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		data = &val;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (put_user(len, optlen))
		return -EFAULT;
	if (copy_to_user(optval, data, len))
		return -EFAULT;
	return 0;
}

void mhi_sock_hash(struct sock *sk)
{
	DPRINTK("mhi_sock_hash: proto:%d\n", sk->sk_protocol);

	spin_lock_bh(&mhi_sock_lock);
	sk_add_node(sk, &mhi_sock_list);
	spin_unlock_bh(&mhi_sock_lock);
}

void mhi_sock_unhash(struct sock *sk)
{
	DPRINTK("mhi_sock_unhash: proto:%d\n", sk->sk_protocol);

	spin_lock_bh(&mhi_sock_lock);
	sk_del_node_init(sk);
	spin_unlock_bh(&mhi_sock_lock);
}

const struct proto_ops mhi_socket_ops = {
	.family = AF_MHI,
	.owner = THIS_MODULE,
	.release = mhi_sock_release,
	.bind = mhi_sock_bind,
	.connect = sock_no_connect,
	.socketpair = sock_no_socketpair,
	.accept = sock_no_accept,
	.getname = sock_no_getname,
	.poll = datagram_poll,
	.ioctl = sock_no_ioctl,
	.listen = sock_no_listen,
	.shutdown = sock_no_shutdown,
	.setsockopt = sock_no_setsockopt,
	.getsockopt = mhi_getsockopt,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = sock_no_setsockopt,
	.compat_getsockopt = sock_no_getsockopt,
#endif
	.sendmsg = mhi_sock_sendmsg,
	.recvmsg = mhi_sock_recvmsg,
	.mmap = sock_no_mmap,
	.sendpage = sock_no_sendpage,
};

static const struct net_proto_family mhi_proto_family = {
	.family = PF_MHI,
	.create = mhi_sock_create,
	.owner = THIS_MODULE,
};

int mhi_sock_init(void)
{
	DPRINTK("mhi_sock_init\n");

	INIT_HLIST_HEAD(&mhi_sock_list);
	spin_lock_init(&mhi_sock_lock);

	return sock_register(&mhi_proto_family);
}

void mhi_sock_exit(void)
{
	DPRINTK("mhi_sock_exit\n");

	sock_unregister(PF_MHI);
}
#endif /* CONFIG_BCM_KF_MHI */
