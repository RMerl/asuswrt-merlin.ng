
#include <linux/if_arp.h>
#include <linux/version.h>
#include <linux/nbuff.h>
#include <net/sock.h>
#include <net/ip6_fib.h>
#define BCM_TCP_V4_TASK
#ifdef BCM_TCP_V4_TASK
#include <linux/kthread.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/mm.h>
#include "uapi/linux/bcm_realtime.h"
#include <linux/proc_fs.h>
#endif
#include "bcmnet.h"
#include "bp3_license.h"


extern int tcp_v4_rcv(struct sk_buff *skb);
#if IS_ENABLED(CONFIG_IPV6)
/*even though we are checking  IPV6 for both Built-in and module,
 * but when built as module this will casue build error as 
 * BLOG_LOCALIN_TCP is always Built-in
*/
extern int tcp_v6_rcv(struct sk_buff *skb);
#else
int tcp_v6_rcv(struct sk_buff *skb)
{
	/* when IPv6 is not enabled we dont expect any packets here */
		BUG();
}
#endif


static inline struct sk_buff *bcm_find_skb_by_flowid(uint32_t flowid)
{
	/* TODO add this function later,needed for coalescing */

	return NULL;
}

static inline int set_skb_fields(struct sk_buff *skb, BlogFcArgs_t *fc_args)
{
	struct net_device *dev_p;
	struct dst_entry *dst_p;

	dev_p = bcm_get_netdev_by_id_nohold(fc_args->local_rx_devid);
	dst_p = blog_get_dstentry_by_id(fc_args->dst_entry_id);
	if(!dev_p || !dst_p) 
		return -1;

	skb->dev = dev_p;
	skb_dst_set(skb, dst_p);
	skb->skb_iif = dev_p->ifindex;
	return 0;
}

static inline void position_skb_ptrs_to_transport(struct sk_buff *skb, BlogFcArgs_t *fc_args)
{

	/*initialize ip & tcp header related fields in skb */
	skb_set_mac_header(skb, 0); 
	skb_set_network_header(skb, fc_args->tx_l3_offset);
	skb_set_transport_header(skb, fc_args->tx_l4_offset);

    /*position data pointer to start of TCP hdr */
	skb_pull(skb,fc_args->tx_l4_offset);
	skb->pkt_type = PACKET_HOST;
	return;
}

static inline struct sk_buff * __bcm_tcp_prep_skb(pNBuff_t pNBuff, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;

	if(IS_FKBUFF_PTR(pNBuff))
	{
		FkBuff_t *fkb = PNBUFF_2_FKBUFF(pNBuff);
		/* Translate the fkb to skb */
		/* find the skb for flowid or allocate a new skb */
		skb = bcm_find_skb_by_flowid(fkb->flowid);

		if(!skb)
		{
			skb = skb_xlate_dp(fkb, NULL);

			if(!skb)
			{
				goto fail;
			}
		}
		skb->mark=0;
		skb->priority=0;
	}
	else
	{
		skb = PNBUFF_2_SKBUFF(pNBuff);
		/* Remove any debris in the socket control block
		 * used by IPCB,IP6CB and TCP_SKB_CB
		 * note: not needed for fkb's as entire skb is cleared in skb_xlate_dp above
		 */
		memset(skb->cb, 0, sizeof(skb->cb));
	}

	if (unlikely(set_skb_fields(skb, fc_args) != 0))
		goto fail;
	position_skb_ptrs_to_transport(skb, fc_args);

	return skb;

fail:
	if (skb)
		dev_kfree_skb_any(skb);
	else
		nbuff_free(pNBuff);
	return NULL;
}

struct sk_buff * bcm_tcp_prep_skb(pNBuff_t pNBuff, BlogFcArgs_t *fc_args)
{
	return __bcm_tcp_prep_skb(pNBuff, fc_args);
}
EXPORT_SYMBOL(bcm_tcp_prep_skb);

#ifdef BCM_TCP_V4_TASK

static int g_bcm_tcp_task_en = 1;

typedef struct {
    spinlock_t lock;

    struct sk_buff_head input_q;
    struct sk_buff_head process_q;

    struct task_struct *task;
    wait_queue_head_t thread_wqh;
    int work_avail;
} bcm_tcp_queue_t;

typedef struct {
    struct sk_buff *skb_p;
} bcm_tcp_item_t;

#define TCP_RCVTSK_LOCK(_c)      spin_lock_bh(&((_c)->lock))
#define TCP_RCVTSK_UNLK(_c)      spin_unlock_bh(&((_c)->lock))


#define MAX_BCM_TCP_INPUT_LEN (1024)
#define MAX_BCM_TCP_BUDGET (256)

static bcm_tcp_queue_t bcm_tcp_async_q;

#define WAKEUP_BCM_TCP_TASK() do { \
            wake_up_interruptible(&(bcm_tcp_async_q.thread_wqh)); \
          } while (0)


static inline void __bcm_tcp_enqueue(struct sk_buff *skb)
{
	if (skb) {
		bcm_tcp_queue_t *async_q = (bcm_tcp_queue_t *) (&bcm_tcp_async_q);

		TCP_RCVTSK_LOCK(async_q);
		if(likely(skb_queue_len(&(async_q->input_q))< MAX_BCM_TCP_INPUT_LEN ))
		{
			skb_queue_tail(&(async_q->input_q),skb);
			skb = NULL;
			if(!(async_q->work_avail))
			{
				async_q->work_avail = 1;
				WAKEUP_BCM_TCP_TASK();
			}
		}
		TCP_RCVTSK_UNLK(async_q);

	}
	if(skb)
		__kfree_skb(skb);
}

void bcm_tcp_enqueue(struct sk_buff *skb)
{

	__bcm_tcp_enqueue(skb);

}
EXPORT_SYMBOL(bcm_tcp_enqueue);

/* inject the packet into ipv4_tcp_stack  directly from the network driver */
static inline int bcm_tcp_v4_recv_queue(pNBuff_t pNBuff, struct net_device *txdev, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;

	skb = __bcm_tcp_prep_skb(pNBuff, fc_args);

	if(skb) {
		skb->protocol = htons(ETH_P_IP);

		if(g_bcm_tcp_task_en)
			/*hand over pkt to bcm_tcp_task()*/
			__bcm_tcp_enqueue(skb);
		else {
			/*
			 * bh_disable is needed to prevent deadlock on sock_lock when TCP timers
			 * are executed
			 */
			local_bh_disable();
			tcp_v4_rcv(skb);
			local_bh_enable();
		}
	}
	return 0;
}

/* inject the packet into ipv6_tcp_stack  directly from the network driver */
static inline int bcm_tcp_v6_recv_queue(pNBuff_t pNBuff, struct net_device *txdev, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;

	skb = __bcm_tcp_prep_skb(pNBuff, fc_args);

	if(skb) {
		skb->protocol = htons(ETH_P_IPV6);
		IP6CB(skb)->iif = skb->dev->ifindex;
		IP6CB(skb)->nhoff = offsetof(struct ipv6hdr, nexthdr);

		if(g_bcm_tcp_task_en)
			/*hand over pkt to bcm_tcp_task()*/
			__bcm_tcp_enqueue(skb);
		else {
			/*
			 * bh_disable is needed to prevent deadlock on sock_lock when TCP timers
			 * are executed
			 */
			local_bh_disable();
			tcp_v6_rcv(skb);
			local_bh_enable();
		}
	}
	return 0;
}

static int bcm_tcp_recv_thread_func(void *thread_data)
{
	unsigned int budget;
	struct sk_buff *skb;
	bcm_tcp_queue_t *async_q  = NULL;
	async_q = (bcm_tcp_queue_t *) thread_data;

	while (1) {
		wait_event_interruptible(   (async_q->thread_wqh),
					kthread_should_stop()
					|| (async_q->work_avail)
					);
 
		if (kthread_should_stop())
		{
			printk(KERN_INFO "kthread_should_stop detected in wfd\n");
			break;
		}

		budget = MAX_BCM_TCP_BUDGET;

		if(skb_queue_len(&(async_q->process_q))<= MAX_BCM_TCP_BUDGET)
		{
			TCP_RCVTSK_LOCK(async_q);
			if(!skb_queue_empty(&(async_q->input_q)))
			{
				skb_queue_splice_tail_init(&(async_q->input_q),&(async_q->process_q));
			}
			TCP_RCVTSK_UNLK(async_q);
		}

		/*
		* bh_disable is needed to prevent deadlock on sock_lock when TCP timers
		* are executed
		*/
		local_bh_disable();
		while(likely(budget-- && (skb = __skb_dequeue(&(async_q->process_q))) ))
		{
			if(skb->protocol == htons(ETH_P_IPV6))
				tcp_v6_rcv(skb);
			else
				tcp_v4_rcv(skb);
		}
		local_bh_enable();

		async_q->work_avail = (!skb_queue_empty(&(async_q->process_q))) ? 1 : 0;

		// No more work in process queue , double check input queue.
		if(!async_q->work_avail)
		{
			TCP_RCVTSK_LOCK(async_q);
			if(!skb_queue_empty(&(async_q->input_q)))
			{
				async_q->work_avail = 1;
			}
			TCP_RCVTSK_UNLK(async_q);
		}

		/* we still have packets in Q, reschedule the task */
		if (async_q->work_avail){
			schedule();
		}
	}
	return 0;
}

struct task_struct *create_bcm_tcp_task(bcm_tcp_queue_t *async_q)
{

	struct task_struct *tsk;
	int cpu_num = num_online_cpus();
	unsigned int bind_mask = 0x00;

	spin_lock_init(&async_q->lock);
	async_q->work_avail = 0;
	init_waitqueue_head(&(async_q->thread_wqh));

	skb_queue_head_init(&(async_q->input_q));
	skb_queue_head_init(&(async_q->process_q));

	tsk = kthread_create(bcm_tcp_recv_thread_func, async_q, "bcm_tcp_task");

	if (IS_ERR(tsk)) {
		printk(KERN_EMERG "bcm_tcp_task creation failed\n");
		return NULL;
	}

	async_q->task = tsk;

	//AFFINITY with non-1st (wl0) and Non-last (Archer) CORE 
	if(cpu_num>2)
	{
		struct cpumask aff_mask;
		int cpuid;

		cpumask_clear(&aff_mask);
		for(cpuid = 1; cpuid<=cpu_num ;cpuid++)
		{
			if(cpuid != 1 && cpuid != cpu_num)
			{
				cpumask_or(&aff_mask,&aff_mask,(cpumask_of(cpuid-1)));
				bind_mask |= (1<<(cpuid-1));
			}
		}
		printk(" %s:%d bind_mask:0x%X\n",__FUNCTION__,__LINE__,bind_mask);
		set_cpus_allowed_ptr(async_q->task,&aff_mask);
	}
	wake_up_process(tsk);

	printk(KERN_EMERG "bcm_tcp_task created successfully with budget %d ,cpumask:0x%X\n", MAX_BCM_TCP_BUDGET,bind_mask);
	return tsk;
}

static struct proc_dir_entry *proc_bcm_tcp_recv_dir = NULL;       /* /proc/bcm_tcp_recv */
static struct proc_dir_entry *proc_bcm_tcp_recv_ops_file = NULL;  /* /proc/bcm_tcp_recv/operate */
static ssize_t bcm_tcp_recv_file_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
static const struct proc_ops bcm_tcp_recv_fops = {
	.proc_write  = bcm_tcp_recv_file_write,
};
#else
static const struct file_operations bcm_tcp_recv_fops = {
	.owner  = THIS_MODULE,
	.write  = bcm_tcp_recv_file_write,
};
#endif

static ssize_t bcm_tcp_recv_file_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
	char input[64]="";
	char ACT=' ';

	if(cnt < 1)
		return -EFAULT;

	if (cnt > 64)
		cnt = 64;

	if (copy_from_user(input, buf, cnt) != 0)
		return -EFAULT;

	input[cnt-1] = '\0';

	/* Command format :
	Enable    :  1
	Disable   :  0
	*/

	ACT = input[0];
	switch(ACT)
	{
		case '1':
			g_bcm_tcp_task_en= 1 ;
			printk("g_bcm_tcp_task_en:%d \n",g_bcm_tcp_task_en);
		break;

		case '0':
			g_bcm_tcp_task_en= 0;
			printk("g_bcm_tcp_task_en:%d \n",g_bcm_tcp_task_en);
		break;

		default:
			printk("g_bcm_tcp_task_en:%d \n",g_bcm_tcp_task_en);
		break;
	}
	return cnt;
}

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
int bcm_tcp_recv_proc_init(void)
{
	if (!(proc_bcm_tcp_recv_dir = proc_mkdir("bcm_tcp_recv_task", NULL)))
		goto fail;

	if (!(proc_bcm_tcp_recv_ops_file = proc_create("bcm_tcp_recv_task/operate", 0644, NULL, &bcm_tcp_recv_fops)))
		goto fail;

	return 0;

fail:
	printk("%s %s: Failed to create proc /bcm_tcp_recv_task\n", __FILE__, __FUNCTION__);
	remove_proc_entry("bcm_tcp_recv_task" ,NULL);
	return (-1);
}
EXPORT_SYMBOL(bcm_tcp_recv_proc_init);

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
void bcm_tcp_recv_proc_fini(void)
{
	remove_proc_entry("operate", proc_bcm_tcp_recv_dir);
	remove_proc_entry("bcm_tcp_recv", NULL);
}
EXPORT_SYMBOL(bcm_tcp_recv_proc_fini);

static int __init bcm_tcp_init(void)
{
	bcm_tcp_async_q.task = create_bcm_tcp_task(&bcm_tcp_async_q);
	if(bcm_tcp_async_q.task == NULL)
		BUG();
	else
	{
		bcm_tcp_recv_proc_init();
	}

	return 0;
}

subsys_initcall(bcm_tcp_init);
#endif

/* inject the packet into ipv4_tcp_stack  directly from the network driver */
static inline int bcm_tcp_v4_recv(pNBuff_t pNBuff, struct net_device *txdev, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;

	skb = __bcm_tcp_prep_skb(pNBuff, fc_args);
	if (skb) {
		skb->protocol = htons(ETH_P_IP);

	 /*
	 * bh_disable is needed to prevent deadlock on sock_lock when TCP timers
	 * are executed
	 */
		local_bh_disable();
		tcp_v4_rcv(skb);
		local_bh_enable();
	}
	return 0;
}

/* inject the packet into ipv6_tcp_stack  directly from the network driver */
static inline int bcm_tcp_v6_recv(pNBuff_t pNBuff, struct net_device *txdev, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;

	skb = __bcm_tcp_prep_skb(pNBuff, fc_args);
	if (skb) {
		skb->protocol = htons(ETH_P_IPV6);

		/* always use ifindex of skb->dev as skb_dst can be set in tcp_v6_early_demux
		 * and it's possible skb_dst is different from skb->dev, when Src IP used
		 * for creating socket/route is not part of the outgoing interface
		 */
		IP6CB(skb)->iif = skb->dev->ifindex;
		IP6CB(skb)->nhoff = offsetof(struct ipv6hdr, nexthdr);
		/*TODO check if we need to consider any IPV6 options */

		/*
		 * bh_disable is needed to prevent deadlock on sock_lock when TCP timers
		 * are executed
		 */
		local_bh_disable();
		tcp_v6_rcv(skb);
		local_bh_enable();
	}

	return 0;
}

static int bcm_tcp_recv(pNBuff_t pNBuff, struct net_device *txdev)
{
	/* The expectation is that this dev_hard_xmit() function will
		never be called. Instead the function with args parameter
	(i.e. bcm_tcp_recv_args) would be invoked */
	BUG();
	return 0;
}

int bcm_tcp_recv_args(pNBuff_t pNBuff, struct net_device *txdev, BlogFcArgs_t *fc_args)
{
	if (fc_args->tx_is_ipv4)
	{
		if (fc_args->use_tcplocal_xmit_enq_fn)
		{
			return bcm_tcp_v4_recv_queue(pNBuff, txdev, fc_args);
		}
		else
		{
			return bcm_tcp_v4_recv(pNBuff, txdev, fc_args);
		}
	}
	else
	{
		if (fc_args->use_tcplocal_xmit_enq_fn)
		{
			return bcm_tcp_v6_recv_queue(pNBuff, txdev, fc_args);
		}
		else
		{
			return bcm_tcp_v6_recv(pNBuff, txdev, fc_args);
		}
	}
	return 0;
}

static const struct net_device_ops bcm_tcp_netdev_ops = {
	.ndo_open	= NULL,
	.ndo_stop	= NULL,
	.ndo_start_xmit	= (HardStartXmitFuncP)bcm_tcp_recv,
	.ndo_set_mac_address = NULL,
	.ndo_do_ioctl	= NULL,
	.ndo_tx_timeout	= NULL,
	.ndo_get_stats	= NULL,
	.ndo_change_mtu	= NULL 
};

struct net_device  *blogtcp_local_dev=NULL;

static void bcm_blogtcp_dev_setup(struct net_device  *dev)
{
	dev->type = ARPHRD_RAWIP;
	dev->mtu  = BCM_MAX_MTU_PAYLOAD_SIZE;
	dev->netdev_ops = &bcm_tcp_netdev_ops;

	bcm_netdev_ext_field_set(dev, blog_stats_flags,
			BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
	bcm_netdev_ext_field_set(dev, dev_xmit_args, bcm_tcp_recv_args);
	netdev_accel_tx_fkb_set(dev);
}

void bcm_tcp_register_netdev(void)
{
	int ret;
	blogtcp_local_dev = alloc_netdev(0, "blogtcp_local", NET_NAME_UNKNOWN, bcm_blogtcp_dev_setup);
	if ( blogtcp_local_dev )
	{
		ret = register_netdev(blogtcp_local_dev);
		if (ret)
		{
			printk(KERN_ERR "blogtcp_local register_netdev failed\n");
			free_netdev(blogtcp_local_dev);
			blogtcp_local_dev = NULL;
		}
		else
			printk("blogtcp_local netdev registered successfully \n");
	}
}

inline static int encap_offset(struct sk_buff *skb, uint32_t * encap)
{
	/*start from innermost IP always */
	int	offset = skb->transport_header - skb->network_header;
	*encap = TYPE_IP;
	return offset;
}

int bcm_tcp_blog_emit(struct sk_buff *skb, struct sock *sk)
{
	if(skb->blog_p && skb->blog_p->l2_mode)
	{
		blog_skip(skb,blog_skip_reason_l2_local_termination);
	}
	else if( (sk && sk->sk_state == TCP_ESTABLISHED) && skb->blog_p &&
			(skb->blog_p->rx.info.bmap.ETH_802x == 1))
	{
		struct net_device *tmpdev;
		uint32_t encap ;
		int offset = encap_offset(skb, &encap);

		if(skb->dev == NULL) 
		{
			/* Error */
			return -1;
		}
		skb_push(skb,offset);
		tmpdev = skb->dev;
		skb->dev = blogtcp_local_dev;
		skb->blog_p->local_rx_devid = bcm_netdev_ext_field_get(tmpdev, devid);
		skb->blog_p->use_xmit_args = 1;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
		{
			struct tcp_sock *tp = tcp_sk(sk);
			if(tp->tcp_discard) {
				skb->blog_p->tcp_discard = 1;
				skb->blog_p->fro = 1;
			}
		}
#endif

		skb->blog_p->local_tcp = 1;
		skb->blog_p->hw_cso = 1;
		if (is_netdev_accel_gdx_tx(blogtcp_local_dev))
		{
			blog_emit_generic(skb, blogtcp_local_dev, BLOG_GENPHY);
		}
		else
		{
			blog_emit(skb, blogtcp_local_dev, encap, 0, BLOG_TCP_LOCALPHY);
		}
		skb->dev = tmpdev;
		skb_pull(skb,offset);
	}
	else{
		/*unsupported local tcp */
		blog_skip(skb, blog_skip_reason_local_tcp_termination);
	}

	return 0;
}

extern int bcmnet_configure_gdx_accel(struct net_device *dev, bcmnet_accel_t *accel_p);
static int bcm_tcp_module_load_notify(struct notifier_block *self, unsigned long val, void *data)
{
	bcmnet_accel_t accel={};
	int bp3_htoa_license;

	if (!strcmp("gdx", ((struct module *)data)->name))
	{
		bp3_htoa_license = bcm_license_check(BP3_FEATURE_HTOA);
		if (bp3_htoa_license <= 0)
		{
			/* No valid htoa license. Do not enable GDX */
			printk("%s: ***No valid HTOA license. Do not enable GDX for local tcp acceleration***\n", __func__);
			return 0;
		}
		printk("%s: ***HTOA license present. Enable GDX for local tcp acceleration***\n", __func__);

		switch (val) {
		case MODULE_STATE_LIVE: 
#if defined(CONFIG_BCM_GDX_HW)
				accel.gdx_hw = 1;
#endif
				accel.gdx_tx = 1;
				bcmnet_configure_gdx_accel(blogtcp_local_dev, &accel);
				break;
		case MODULE_STATE_GOING:
#if defined(CONFIG_BCM_GDX_HW)
				accel.gdx_hw = 0;
#endif
				accel.gdx_tx = 0;
				bcmnet_configure_gdx_accel(blogtcp_local_dev, &accel);
				break;
			default: 
				return 0;
		}
	}
	return 0;
}

static struct notifier_block bcm_tcp_module_load_nb = {
	.notifier_call = bcm_tcp_module_load_notify,
};


static int __init bcm_tcp_accel_init(void)
{
	bcm_tcp_register_netdev();
	register_module_notifier(&bcm_tcp_module_load_nb);
	return 0;
}
fs_initcall(bcm_tcp_accel_init);
