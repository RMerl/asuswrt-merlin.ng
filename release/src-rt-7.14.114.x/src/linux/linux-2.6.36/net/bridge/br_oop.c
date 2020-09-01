#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>


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
	NETLINK_CB(skb).pid = 0;         /* from kernel */
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


/* Receive messages from netlink socket. */
static void udp_receive(struct sk_buff  *skb)
{
  	u_int	uid, pid, seq, sid;
	void			*data;
	struct nlmsghdr *nlh;

	nlh = (struct nlmsghdr *)skb->data;
	pid  = NETLINK_CREDS(skb)->pid;
	uid  = NETLINK_CREDS(skb)->uid;
	sid  = NETLINK_CB(skb).sid;
	seq  = nlh->nlmsg_seq;
	data = NLMSG_DATA(nlh);
	printk("recv skb from user space uid:%d pid:%d seq:%d,sid:%d\n",uid,pid,seq,sid);
	broop_handle(pid,seq,data);

	return ;
}

static int __init broop_init(void)
{
	printk("\n\n>> broop create\n\n");
	netlink_sock = netlink_kernel_create(&init_net, NETLINK_BROOP, 0, udp_receive, NULL, THIS_MODULE);
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
