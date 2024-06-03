/*
    Copyright (c) 2021 Broadcom
    All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard

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

/**
 * =============================================================================
 *
 * DHD GSO Packet Datapath
 *
 * =============================================================================
 */

#if defined(CONFIG_BCM_SW_GSO) && defined(BCM_CPE_DHD_GSO)

#include <bcmutils.h>
#include <bcm_pktfwd.h>
#include "bcm_async_queue.h"

#if defined(BCM_PKTFWD)
#include <dhd_pktfwd.h>
#endif /* BCM_PKTFWD */

#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_flowring.h>

#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */

#ifdef BCM_NBUFF
#include <dhd_nbuff.h>
#endif /* BCM_NBUFF */

#ifdef BCM_BLOG
#include <dhd_blog.h>
#endif /* BCM_BLOG */

#include <dhd_sw_gso.h>

#ifdef AWL_LOCAL_IN_BYPASS_FILTER
#include <linux/proc_fs.h>
#include <linux/etherdevice.h>
#endif

int BCMFASTPATH dhd_start_xmit(struct sk_buff *skb, struct net_device *net);

#if defined(SW_GSO_PKTLIST)
typedef struct {
	uint16 radio_idx;

	int list_work_avail;
	struct task_struct *list_tsk;
	wait_queue_head_t list_thread_wqh;
	bcm_async_queue_t pktlist_q;
	spinlock_t lock;	/* spinlock for queue exclusive access */
} SW_GSO_PKTLIST_CONTEXT;

#define GSOLIST_LOCK(_c)            spin_lock_bh(&((_c)->lock))
#define GSOLIST_UNLK(_c)            spin_unlock_bh(&((_c)->lock))

typedef struct {
	uint16 flowid;
	pktlist_t pktlist;
	unsigned long mark;
} dhd_gso_priv_t;

typedef struct {
	dhd_gso_priv_t *dhd_gso_priv;
} dhd_gso_pktlist_item_t;

#define GSO_SKB_BUDGET (8)	/* 8 x 64KB GSO => 8 x (45 pkts [1500 Byte] ) = 360 pkts */
#define GSO_FLUSH_PERIOD_US (500)
#define GSO_PKTLIST_FLUSH_LEN (512)

//Per Radio share one GSO
static SW_GSO_PKTLIST_CONTEXT *gso_pktlist_context[FWDER_MAX_RADIO] = { NULL };

extern int dhd_gsopktlist_flush(int radio_idx, pktlist_t * pktlist, uint16 flowid);
//int dhd_gsopktlist_sched(int radio_idx);

uint16 dhd_gsopriv_get_flowid(void *ptr)
{
	if (ptr) {
		dhd_gso_priv_t *dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		return dhd_gso_priv->flowid;
	}

	return ID16_INVALID;
}

void dhd_gsopriv_set_flowid(void *ptr, uint16 flowid)
{
	if (ptr) {
		dhd_gso_priv_t *dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		dhd_gso_priv->flowid = flowid;
	}
	return;
}

unsigned long dhd_gsopriv_get_mark(void *ptr)
{
	if (ptr) {
		dhd_gso_priv_t *dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		return dhd_gso_priv->mark;
	}

	return 0x0;
}

void dhd_gsopriv_set_mark(void *ptr, unsigned long mark)
{
	if (ptr) {
		dhd_gso_priv_t *dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		dhd_gso_priv->mark = mark;
	}
	return;
}

void *dhd_gso_priv_buf_get(void)
{
	dhd_gso_priv_t *dhd_gso_priv;

	//TODO :Use Pre-alloc or stack local var;
	dhd_gso_priv = kmalloc(sizeof(dhd_gso_priv_t), GFP_ATOMIC);

	if (dhd_gso_priv) {
		dhd_gso_priv->flowid = ID16_INVALID;
		PKTLIST_RESET(&(dhd_gso_priv->pktlist));	/* len = 0U, key.v16 = don't care */
	}

	return (void *)dhd_gso_priv;
}

static inline uint32 dhd_gsopriv_get_pktlist_len(void *ptr)
{
	if (ptr) {
		dhd_gso_priv_t *dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		return (dhd_gso_priv->pktlist.len);
	}
	return 0;
}

void dhd_gso_priv_buf_free(void *ptr)
{
	dhd_gso_priv_t *dhd_gso_priv;
	pktlist_t *pktlist = NULL;

	if (ptr) {
		dhd_gso_priv = (dhd_gso_priv_t *) ptr;
		pktlist = &(dhd_gso_priv->pktlist);

		if (unlikely(pktlist->len != 0)) {	/* some pkt in list, release one by one */
			pktlist_pkt_t *pkt, *npkt;
#if 1
			if (net_ratelimit())
				printk("##XX %s:%d XX## error pktlist len:%d \n",
					__FUNCTION__, __LINE__, pktlist->len);
#endif
			for (pkt = (pktlist_pkt_t *) pktlist->head;
			     pkt != PKTLIST_PKT_NULL; pkt = npkt) {
				npkt = PKTLIST_PKT_SLL(pkt, FKBUFF_PTR);
				PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, FKBUFF_PTR);

				/* No osh accounting, as not yet in DHD */
				PKTLIST_PKT_FREE(pkt);
			}
		}
		kfree(ptr);
	}
}
#endif /* SW_GSO_PKTLIST */

int dhd_bcmgso_out(struct sk_buff *nbuff, struct net_device *net)
{
	struct sk_buff *nskb = NULL;
	nskb = nbuff;

	if (IS_FKBUFF_PTR(nbuff)) {
		FkBuff_t *fkb_p;

		fkb_p = (FkBuff_t *) PNBUFF_2_FKBUFF(nbuff);

		/* Reset flags */
		DHD_PKTTAG_FD(nbuff)->flags = 0;

#if 0				//Included in PKTTAG->flags
		/* Make sure wfd flag and DHDHDR is clean */
		DHD_PKT_CLR_WFD_BUF(nbuff);
		DHD_PKT_CLR_DATA_DHDHDR(nbuff);
#endif
	}

	if (nskb)
		return dhd_start_xmit(nskb, net);

	return NETDEV_TX_OK;
}

#if defined(SW_GSO_PKTLIST)
static inline int dhd_gso_pktlist_append_pkt(void *ptr, void *pkt)
{
	dhd_gso_priv_t *dhd_gso_priv;
	pktlist_t *pktlist = NULL;
	int ret = 0;

	if (ptr && pkt) {
		dhd_gso_priv = (dhd_gso_priv_t *) ptr;

		pktlist = &(dhd_gso_priv->pktlist);

		if (likely(pktlist->len != 0)) {
			/* pend to tail */
			PKTLIST_PKT_SET_SLL(pktlist->tail, pkt, FKBUFF_PTR);
			pktlist->tail = pkt;
		} else {
			pktlist->head = pktlist->tail = pkt;
		}

		pktlist->len++;

		ret = 1;
	}

	return ret;
}

int dhd_bcmgsolist_out(struct sk_buff *nbuff, struct net_device *net)
{
	struct sk_buff *nskb = NULL;

	nskb = nbuff;

	if (IS_FKBUFF_PTR(nbuff)) {	//Handle GSO segment FKBs initial for Wifi
		FkBuff_t *fkb_p;
		void *gso_priv = NULL;
		fkb_p = (FkBuff_t *) PNBUFF_2_FKBUFF(nbuff);

		//Retrieve GSO priv data pointer from fkb_p->queue pointer;
		gso_priv = (void *)fkb_p->queue;

		//Clear the queue field to sure no one get GSO priv data pointer as queue pointer.
		fkb_p->queue = NULL;

		/* Rebuild fkb_p->wl for GSO fkb */
		/* Already initial fkb_p->wl ==> [fkb->priority = skb->priority] in BCM_SW_GSO */
		/* Do the jobs like dhd_pktfwd_pktlist_prepare() */

		/* Reset flags */
		DHD_PKTTAG_FD(nbuff)->flags = 0;

		/* Save the flowid */
		DHD_PKT_SET_FLOWID(nbuff, dhd_gsopriv_get_flowid(gso_priv));

		/* Enqueue FKB to GSO priv data pktlist */
		if (dhd_gso_pktlist_append_pkt(gso_priv, (void *)(nbuff)) == 0) {
			/* error: Enqueue Fail */
			if (net_ratelimit())
				printk("#### %s:%d #### %s pkt:0x%px dev:%s"
					" flowid:0x%X prio:%d Error ,Free\n",
					__FUNCTION__, __LINE__,
					(IS_FKBUFF_PTR(nbuff)) ? "FKB" : "SKB",
					nbuff, net->name, PKTFLOWID(nbuff), PKTPRIO(nbuff));
			nbuff_free(nbuff);
		}

		return NETDEV_TX_OK;

	} else if (nskb) {	//SKB with CSUM only
		return dhd_start_xmit(nskb, net);
	}

	return NETDEV_TX_OK;
}

void dhd_wake_gsolist_task(int radio_idx)
{
	if (gso_pktlist_context[radio_idx]->list_work_avail != 1) {
		gso_pktlist_context[radio_idx]->list_work_avail = 1;
		wake_up_interruptible(&(gso_pktlist_context[radio_idx]->list_thread_wqh));
	}
}

/* XFER enq_pktlist to GSO pktlist */
int dhd_gso_enq_pktlist(struct net_device *net_device, void *dhd_gso_priv, int radio_idx)
{
	int ret = 0;
	SW_GSO_PKTLIST_CONTEXT *gso_context =
	    (SW_GSO_PKTLIST_CONTEXT *) gso_pktlist_context[radio_idx];

	if (gso_context && dhd_gso_priv) {
		if (dhd_gsopriv_get_pktlist_len(dhd_gso_priv) > 0) {
			bcm_async_queue_t *queue_ptr = NULL;
			queue_ptr = &(gso_context->pktlist_q);

			GSOLIST_LOCK(gso_context);
			if (likely(bcm_async_queue_not_full(queue_ptr))) {
				dhd_gso_pktlist_item_t *entry_p = NULL;

				entry_p =
				    (dhd_gso_pktlist_item_t *)
				    bcm_async_queue_entry_write(queue_ptr);

				if (entry_p) {
					WRITE_ONCE(entry_p->dhd_gso_priv, dhd_gso_priv);
					bcm_async_queue_entry_enqueue(queue_ptr);
					dhd_wake_gsolist_task(gso_context->radio_idx);
					ret = 1;
				}
			}
			GSOLIST_UNLK(gso_context);
		}
	}

	return ret;
}

static inline int
dhd_gso_merge_pktlist(pktlist_t * src_list, pktlist_t * dst_list, NBuffPtrType_t NBuffPtrType)
{
	int ret = 0;

	//Merge list
	if (dst_list->len == 0u) {
		dst_list->head = src_list->head;
		dst_list->tail = src_list->tail;
	} else {
		PKTLIST_PKT_SET_SLL(dst_list->tail, src_list->head, NBuffPtrType);
		dst_list->tail = src_list->tail;
	}

	dst_list->len += src_list->len;
	PKTLIST_RESET(src_list);	/* head,tail, not reset */

	ret = 1;

	return ret;
}

int dhd_gsopktlist_task(void *thread_data)
{
	SW_GSO_PKTLIST_CONTEXT *gso_context =
	    (SW_GSO_PKTLIST_CONTEXT *) thread_data;
	int budget;
	bcm_async_queue_t *queue_ptr = NULL;
	dhd_gso_pktlist_item_t *entry_p = NULL;
	unsigned long time_limit;

	uint16 dst_flowid = 0x0;
	pktlist_t *dst_list = NULL;

	FLUSH_CAUSE_CODE flush_pendlist = FLUSH_NONE;

	uint16 pend_flowid = 0x0;
	pktlist_t *pend_list = NULL;

	pktlist_t listbuf;
	dhd_gso_priv_t *dhd_gso_priv;

	pend_list = &(listbuf);
	PKTLIST_RESET(pend_list);
	pend_flowid = ID16_INVALID;

	queue_ptr = &(gso_context->pktlist_q);

	//time_limit = jiffies + msecs_to_jiffies(GSO_FLUSH_PERIOD_MS);
	time_limit = 0;

	while (1) {
		wait_event_interruptible((gso_context->list_thread_wqh),
				kthread_should_stop() || (gso_context->list_work_avail));

		if (kthread_should_stop()) {
			printk(KERN_INFO "kthread_should_stop detected in wfd\n");
			break;
		}

		if (time_limit == 0)
			time_limit =
			    jiffies + usecs_to_jiffies(GSO_FLUSH_PERIOD_US);

		budget = GSO_SKB_BUDGET;

		GSOLIST_LOCK(gso_context);
		while (likely(budget-- && bcm_async_queue_not_empty(queue_ptr))) {
			entry_p = (dhd_gso_pktlist_item_t *)
			    bcm_async_queue_entry_read(queue_ptr);

			dhd_gso_priv = READ_ONCE(entry_p->dhd_gso_priv);

			bcm_async_queue_entry_dequeue(queue_ptr);

			if (dhd_gsopriv_get_pktlist_len(dhd_gso_priv) != 0) {
				dst_list = &(dhd_gso_priv->pktlist);
				dst_flowid = (dhd_gso_priv->flowid);
			} else {
				dhd_gso_priv_buf_free((void *)dhd_gso_priv);
				dst_list = NULL;
			}

			if (dst_list) {
				flush_pendlist = FLUSH_NONE;
				if (pend_flowid != ID16_INVALID) {
					//pend_list not empty, decide need flush or not
					if (pend_flowid != dst_flowid) {
						//different flowID, can't merge
						flush_pendlist = FLUSH_DIFF_FLOW;
					} else if (time_after_eq(jiffies, time_limit)) {
						//Reach Idle time limit
						flush_pendlist = FLUSH_IDLETIME_LIMIT;
					} else if (pend_list->len >= GSO_PKTLIST_FLUSH_LEN) {
						//Have enough pkts, just flush
						flush_pendlist = FLUSH_ENOUGH_PKT;
					}
#if 0
					else {

						//always flush, no merge
						flush_pendlist = FLUSH_ALWAYS;

					}
#endif
				}

				if (flush_pendlist != FLUSH_NONE) {
					GSOLIST_UNLK(gso_context);

					//set next flush time
					time_limit =
					    jiffies +
					    usecs_to_jiffies
					    (GSO_FLUSH_PERIOD_US);

					dhd_gsopktlist_flush(gso_context->
							     radio_idx,
							     pend_list,
							     pend_flowid);
					pend_flowid = ID16_INVALID;
					GSOLIST_LOCK(gso_context);
				}

				if (pend_flowid == ID16_INVALID || pend_flowid == dst_flowid) {
					dhd_gso_merge_pktlist(dst_list, pend_list, FKBUFF_PTR);
					pend_flowid = dst_flowid;

					//always flush, no merge pktlist
					if (flush_pendlist == FLUSH_ALWAYS) {
						GSOLIST_UNLK(gso_context);
						time_limit =
						    jiffies +
						    usecs_to_jiffies
						    (GSO_FLUSH_PERIOD_US);
#if 0
						if (net_ratelimit())
							printk("#### %s:%d #### flowid:0x%X list"
								" len:%d reason:%d\n",
								__FUNCTION__, __LINE__,
								pend_flowid, pend_list->len,
								flush_pendlist);
#endif
						dhd_gsopktlist_flush
						    (gso_context->radio_idx,
						     pend_list, pend_flowid);
						pend_flowid = ID16_INVALID;
						GSOLIST_LOCK(gso_context);
					}
				}

				dhd_gso_priv_buf_free((void *)dhd_gso_priv);
			}
		}

		gso_context->list_work_avail =
		    (bcm_async_queue_not_empty(queue_ptr)) ? 1 : 0;

		GSOLIST_UNLK(gso_context);
		//Force flush
		if (pend_flowid != ID16_INVALID) {
			flush_pendlist = FLUSH_FORCE;
#if 0
			if (net_ratelimit())
				printk
				    ("#### %s:%d #### flowid:0x%X list len:%d reason:%d\n",
				     __FUNCTION__, __LINE__, pend_flowid,
				     pend_list->len, flush_pendlist);
#endif

			//set next flush time
			time_limit =
			    jiffies + usecs_to_jiffies(GSO_FLUSH_PERIOD_US);
			dhd_gsopktlist_flush(gso_context->radio_idx, pend_list, pend_flowid);
			pend_flowid = ID16_INVALID;
		}

		/* we still have packets in Q, reschedule the task */
		if (gso_context->list_work_avail) {
			schedule();
		} else {
			time_limit = 0;
		}
	}

	return 0;
}

int dhd_gsopktlist_init(uint radio_idx)
{
	SW_GSO_PKTLIST_CONTEXT *gso_context = NULL;
	struct sched_param param;
	int bind_cpucore = 0;
	unsigned char strbuf[32] = "";

	int xmit_entry_size = ((sizeof(dhd_gso_pktlist_item_t) + 3) & ~3);

	gso_context = (SW_GSO_PKTLIST_CONTEXT *) kmalloc(sizeof(*gso_context), GFP_ATOMIC);

	spin_lock_init(&(gso_context->lock));
	bcm_async_queue_init(&(gso_context->pktlist_q), 1024, xmit_entry_size);

	gso_context->radio_idx = radio_idx;
	gso_pktlist_context[radio_idx] = gso_context;

	init_waitqueue_head(&(gso_context->list_thread_wqh));
	sprintf(strbuf, "dhdgsolist_%d", radio_idx);
	gso_context->list_tsk =
		kthread_create(dhd_gsopktlist_task, (void *)gso_context, strbuf);

	bind_cpucore = radio_idx % num_online_cpus();

#if !(IS_ENABLED(CONFIG_BCM_DHD_ARCHER) || defined(BCM_DHD_RUNNER))
	//bind task on same CORE with wifi driver only when flowring manager on dhd.ko
	kthread_bind(gso_context->list_tsk, bind_cpucore);
#endif

	param.sched_priority = 5;
	sched_setscheduler(gso_context->list_tsk, SCHED_RR, &param);

	wake_up_process(gso_context->list_tsk);

	return (gso_pktlist_context[radio_idx] != NULL);
}

#endif /* SW_GSO_PKTLIST */

#ifdef AWL_LOCAL_IN_BYPASS_FILTER
unsigned char local_in_devname[16] = "";
static unsigned char local_in_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static int
localin_filter_operate_file_write(struct file *file, const char __user * buf,
	size_t cnt, loff_t * ppos);
static const struct file_operations localin_filter_fops = {
	.owner = THIS_MODULE,
	.write = localin_filter_operate_file_write,
};

static inline void clear_mac(void *dst)
{
	memset(dst, 0xff, 6);
}

static inline void save_mac(void *src, void *dst)
{
	if (memcmp(dst, src, 6)) {
		memcpy(dst, src, 6);

		if (net_ratelimit())
			printk("save_mac %02X:%02X:%02X:%02X:%02X:%02X\n",
				((unsigned char *)(dst))[0],
				((unsigned char *)(dst))[1],
				((unsigned char *)(dst))[2],
				((unsigned char *)(dst))[3],
				((unsigned char *)(dst))[4],
				((unsigned char *)(dst))[5]);
	}
}

static int parse_dev_name(unsigned char *str, unsigned char **devname)
{
	int ret = 0;
	if (str && devname) {
		*devname = strstr(str, "/");
		if ((*devname)) {
			(*devname)++;
			ret = 1;
		}
	}
	return ret;
}

static int
localin_filter_operate_file_write(struct file *file, const char __user * buf,
	size_t cnt, loff_t * ppos)
{
	char input[64] = "";
	unsigned char *macstr = NULL;
	char ACT = ' ';

	macstr = NULL;

	if (cnt > 64)
		cnt = 64;

	if (copy_from_user(input, buf, cnt) != 0)
		return -EFAULT;

	input[cnt - 1] = '\0';

	/* Command format :
	   Add   :  A/<Incoming_DEV_NAME>
	   Del   :  D
	 */

	ACT = input[0];

#ifdef DEBUG
	printk("#### %s:%d #### recv:\"%s\" \n", __FUNCTION__, __LINE__, input);
#endif

	switch (ACT) {
	case 'D':		// Disable

		sprintf(local_in_devname, "%s", "");
		clear_mac(&local_in_addr);

		break;

	case 'A':
		{
			unsigned char *devname = NULL;
			struct net_device *dev_p = NULL;

#ifdef DEBUG
			printk("#### %s:%d #### Add! input='%s'\n",
			       __FUNCTION__, __LINE__, input);
#endif
			if (parse_dev_name(input, &devname)) {
				sprintf(local_in_devname, "%s", devname);
				dev_p = dev_get_by_name(&init_net, devname);
#ifdef DEBUG
				printk("#### %s:%d #### Parse dev: %s [%p]\n",
				       __FUNCTION__, __LINE__, devname, dev_p);
#endif
			}

			if (dev_p) {	// Get mac ...
				save_mac(dev_p->dev_addr, &local_in_addr);
				printk("MAC Saved %s [%px]\n", dev_p->name,
				       dev_p);

				dev_put(dev_p);
			} else {	// Non-exist dev clear mac
				clear_mac(&local_in_addr);
			}
		}
		break;

	default:
		break;

	}

	return cnt;
}

unsigned int local_in_bypass_filter(void *pkt)
{
	unsigned int ret = 0;

	if (pkt && ((local_in_addr[0] & 0x01) == 0x00)) {
		struct ethhdr *eh = NULL;
		u16 eth_type;

		eh = (struct ethhdr *)(pkt);
		eth_type = ntohs(eh->h_proto);

		if (ether_addr_equal((u8 *) (eh->h_dest), (u8 *) local_in_addr)) {

#ifdef DEBUG
			if (net_ratelimit())
				printk("#### %s:%d #### hit %02X:%02X:%02X:%02X:%02X:%02X\n",
					__FUNCTION__, __LINE__, local_in_addr[0],
					local_in_addr[1],
					local_in_addr[2],
					local_in_addr[3],
					local_in_addr[4],
					local_in_addr[5]);
#endif
			return 1;
		} else {
#ifdef DEBUG
			if (net_ratelimit())
				printk("#### %s:%d #### miss %02X:%02X:%02X:%02X:%02X:%02X vs"
					" %02X:%02X:%02X:%02X:%02X:%02X\n",
					__FUNCTION__, __LINE__, (eh->h_dest)[0],
					(eh->h_dest)[1],
					(eh->h_dest)[2],
					(eh->h_dest)[3],
					(eh->h_dest)[4],
					(eh->h_dest)[5],
					local_in_addr[0],
					local_in_addr[1],
					local_in_addr[2],
					local_in_addr[3],
					local_in_addr[4],
					local_in_addr[5]);
#endif /* DEBUG */
		}
	} else {
#ifdef DEBUG
		if (net_ratelimit())
			printk("#### %s:%d #### miss pkt:%p mcast:%d\n",
				__FUNCTION__, __LINE__, (pkt), ((local_in_addr[0] & 0x01) == 0x00));
#endif
	}

	return ret;
}
EXPORT_SYMBOL(local_in_bypass_filter);

static int local_in_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev_p = netdev_notifier_info_to_dev(ptr);

	if (!dev_p)
		return NOTIFY_DONE;

	if (strcmp(dev_p->name, local_in_devname))
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_UP:
	case NETDEV_CHANGEADDR:
		save_mac(dev_p->dev_addr, &local_in_addr);
		break;

	case NETDEV_DOWN:
	case NETDEV_GOING_DOWN:
	case NETDEV_UNREGISTER:
		clear_mac(&local_in_addr);
		break;

	case NETDEV_CHANGE:
		if (netif_carrier_ok(dev_p)) {
			save_mac(dev_p->dev_addr, &local_in_addr);
		} else {
			clear_mac(&local_in_addr);
		}
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block local_in_netdev_notifier = {
	.notifier_call = local_in_netdev_event
};

static struct proc_dir_entry *proc_lcbypass_dir = NULL;	/* /proc/lcbypass */
static struct proc_dir_entry *proc_lcbypass_ops_file = NULL;	/* /proc/lcbypass/operate */

int awl_localin_filter_proc_fini(void)
{
	unregister_netdevice_notifier(&local_in_netdev_notifier);
	if (proc_lcbypass_dir) {
		if (proc_lcbypass_ops_file) {
			remove_proc_entry("operate", proc_lcbypass_dir);
			proc_lcbypass_ops_file = NULL;
		}
		remove_proc_entry("lcbypass", NULL);
		proc_lcbypass_dir = NULL;
	}
	return 0;
}

int awl_localin_filter_proc_init(void)
{
	register_netdevice_notifier(&local_in_netdev_notifier);

	if (!(proc_lcbypass_dir = proc_mkdir("lcbypass", NULL)))
		goto fail;

	if (!
	    (proc_lcbypass_ops_file =
	     proc_create("lcbypass/operate", 0644, NULL, &localin_filter_fops)))
		goto fail;

	return 0;

fail:
	printk("%s %s: Failed to create proc /lcbypass\n", __FILE__, __FUNCTION__);
	awl_localin_filter_proc_fini();
	return (-1);
}
#endif /* AWL_LOCAL_IN_BYPASS_FILTER */

#endif /* CONFIG_BCM_SW_GSO && BCM_CPE_DHD_GSO */
