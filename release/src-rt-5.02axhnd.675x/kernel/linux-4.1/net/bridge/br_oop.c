#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <uapi/linux/netlink.h>


int broop = 0;
static struct sock *netlink_sock;

typedef struct broop_state {
        char ctrl;
        int val;
} broop_st;

enum {
	GET,
	SET
};

static void broop_handle(int pid, int seq, broop_st *st)
{
	struct sk_buff	*skb;
	struct nlmsghdr	*nlh;
	int  size= 4+1;
	int  len = NLMSG_SPACE(size);
	void *data;
	int ret;
	
	if(st->ctrl)
		broop = st->val;

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return;
	nlh= NLMSG_PUT(skb, pid, seq, 0, size);
	nlh->nlmsg_flags = 0;
	data=NLMSG_DATA(nlh);
	memcpy(data, &broop, size);
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;   /* unicast */

	ret=netlink_unicast(netlink_sock, skb, pid, MSG_DONTWAIT);
	if (ret <0)
	{
		printk("send failed\n");
		return;
	}
	return;
	
nlmsg_failure:			/* Used by NLMSG_PUT */
	if (skb)
		kfree_skb(skb);
}

static void broop_receive(struct sk_buff  *skb)
{
	kuid_t  uid;
  	u_int	pid, seq;
	void			*data;
	struct nlmsghdr *nlh;

	nlh = (struct nlmsghdr *)skb->data;
	pid  = NETLINK_CREDS(skb)->pid;
	uid  = NETLINK_CREDS(skb)->uid;
	seq  = nlh->nlmsg_seq;
	data = NLMSG_DATA(nlh);

	broop_handle(pid,seq,data);

	return ;
}

struct netlink_kernel_cfg nl_kernel_cfg = {
	.groups = 0,
	.flags = 0,
	.input = broop_receive,
	.cb_mutex = NULL,
	.bind = NULL,
	.compare = NULL,
};

static int __init broop_init(void)
{
	printk("\n\n>> broop create\n\n");
	netlink_sock = netlink_kernel_create(&init_net, NETLINK_BROOP, &nl_kernel_cfg);
	if(netlink_sock < 0)
		printk("invalid netlink sock\n");
	else
		printk("netlink driver create successfully\n");

	return 0;
}

static void __exit broop_exit(void)
{
	sock_release(netlink_sock->sk_socket);
	printk("netlink driver remove successfully\n");
}
module_init(broop_init);
module_exit(broop_exit);

MODULE_DESCRIPTION("broop state module");
MODULE_AUTHOR("Hamiltonian_Hsue@asus.com");
MODULE_LICENSE("GPL");
