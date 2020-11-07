/*
 * EMFL Linux Port: These functions handle the interface between EMFL
 * and the native OS.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: emf_linux.c 785735 2020-04-04 13:13:02Z $
 */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <linux/proc_fs.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <ethernet.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <osl.h>
#include <emf/igs/osl_linux.h>
#include <emf/emf/emf_cfg.h>
#include <emf/emf/emfc_export.h>
#include "emf_linux.h"
#ifdef BCM_NBUFF_WLMCAST_IPV6
#include <net/ipv6.h>
#include <bcmipv6.h>
typedef struct priv_link {
	void *(*nic_hook_fn)(int cmd, void *p, void *p2);
} priv_link_t;
#endif // endif

MODULE_LICENSE("Proprietary");

static emf_struct_t *emf;

#if !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG)
#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *root_emf_dir;
#endif // endif
#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG) */

/* for HNDROUTER or CPEROUTER, the netlink of 17 for emf won't work as 17 is already used
 * and emf command is not useful at all as emf instance is not created by command, so we
 * just disable it
 */
#if !defined(BCM_NBUFF_WLMCAST_IPV6)

static int emf_net_device_event_notify(struct notifier_block *nb, unsigned long event, void *ptr);

struct notifier_block emf_net_device_nb = {
	.notifier_call = emf_net_device_event_notify,
};

static emf_info_t *
emf_instance_find_by_ifptr(emf_struct_t *emf, struct net_device *if_ptr)
{
	emf_iflist_t *ptr;
	emf_info_t *emfi;

	ASSERT(if_ptr != NULL);

	OSL_LOCK(emf->lock);

	for (emfi = emf->list_head; emfi != NULL; emfi = emfi->next)
	{
		for (ptr = emfi->iflist_head; ptr != NULL; ptr = ptr->next)
		{
			if (ptr->if_ptr == if_ptr)
			{
				OSL_UNLOCK(emf->lock);
				return (emfi);
			}
		}
	}

	OSL_UNLOCK(emf->lock);

	return (NULL);
}

static emf_info_t *
emf_instance_find_by_brptr(emf_struct_t *emf, struct net_device *br_ptr)
{
	emf_info_t *emfi;

	ASSERT(br_ptr != NULL);
	ASSERT(emf->lock != NULL);
	OSL_LOCK(emf->lock);

	for (emfi = emf->list_head; emfi != NULL; emfi = emfi->next)
	{
		if (br_ptr == emfi->br_dev)
		{
			OSL_UNLOCK(emf->lock);
			return (emfi);
		}
	}

	OSL_UNLOCK(emf->lock);
	return (NULL);
}

/*
 * Description: This function is called by Netfilter when packet hits
 *              the bridge pre routing hook. All IP packets are given
 *              to EMFL for its processing.
 *
 * Input:       pskb - Pointer to the packet buffer. Other parameters
 *                     are not used.
 *
 * Return:      Returns the value indicating packet can be forwarded
 *              or packet is stolen.
 */
#ifdef CONFIG_NETFILTER
static uint32
emf_br_pre_hook(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	void *priv,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	const struct nf_hook_ops *ops,
#else
	uint32 hook,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	struct sk_buff *skb,
#else
	struct sk_buff **pskb,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	const struct nf_hook_state *st)
#else
	const struct net_device *in,
	const struct net_device *out,
	int32 (*okfn)(struct sk_buff *))
#endif // endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	struct sk_buff **pskb = &skb;
#endif // endif
	emf_info_t *emfi;

	EMF_INFO("Frame at BR_PRE_HOOK received from if %p %s\n",
	         (*pskb)->dev, (*pskb)->dev->name);

	/* Find the bridge that the receive interface corresponds to */
	emfi = emf_instance_find_by_ifptr(emf, (*pskb)->dev);
	if (emfi == NULL)
	{
		EMF_INFO("No EMF processing needed for unknown ports\n");
		return (NF_ACCEPT);
	}

	/* Non IP packet received from LAN port is returned back to
	 * bridge.
	 */
	if ((*pskb)->protocol != __constant_htons(ETH_P_IP))
	{
		EMF_INFO("Ignoring non IP packets from LAN ports\n");
		return (NF_ACCEPT);
	}

	EMF_DUMP_PKT((*pskb)->data);

	return (emfc_input(emfi->emfci, *pskb, (*pskb)->dev,
	                   PKTDATA(NULL, *pskb), FALSE));
}

/*
 * Description: This function is called by Netfilter when packet hits
 *              the ip post routing hook. The packet is sent to EMFL
 *              only when it is going on to bridge port.
 *
 * Input:       pskb - Pointer to the packet buffer. Other parameters
 *                     are not used.
 *
 * Return:      Returns the value indicating packet can be forwarded
 *              or packet is stolen.
 */
static uint32
emf_ip_post_hook(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	void *priv,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	const struct nf_hook_ops *ops,
#else
	uint32 hook,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	struct sk_buff *skb,
#else
	struct sk_buff **pskb,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	const struct nf_hook_state *st)
#else
	const struct net_device *in,
	const struct net_device *out,
	int32 (*okfn)(struct sk_buff *))
#endif // endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	struct sk_buff **pskb = &skb;
#endif // endif
	emf_info_t *emfi;

	ASSERT((*pskb)->protocol == __constant_htons(ETH_P_IP));

	EMF_DEBUG("Frame at IP_POST_HOOK going to if %p %s\n",
	          (*pskb)->dev, (*pskb)->dev->name);

	/* Find the LAN that the bridge interface corresponds to */
	emfi = emf_instance_find_by_brptr(emf, (*pskb)->dev);
	if (emfi == NULL)
	{
		EMF_INFO("No EMF processing needed for unknown ports\n");
		return (NF_ACCEPT);
	}

	EMF_DUMP_PKT((*pskb)->data);

	return (emfc_input(emfi->emfci, *pskb, (*pskb)->dev,
	                   PKTDATA(NULL, *pskb), TRUE));
}
#endif /* CONFIG_NETFILTER */

#ifdef CONFIG_PROC_FS
/*
 * EMFL Packet Counters/Statistics
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int32
emf_stats_get(struct seq_file *seq, void *v)
{
	emf_info_t *emfi = seq->private;
	emf_cfg_request_t cfg;
	emf_stats_t *emfs;

	ASSERT(emfi);

	cfg.command_id = EMFCFG_CMD_EMF_STATS;
	cfg.oper_type = EMFCFG_OPER_TYPE_GET;
	cfg.size = sizeof(emf_stats_t);
	emfs = (emf_stats_t *)cfg.arg;

	emfc_cfg_request_process(emfi->emfci, &cfg);
	if (cfg.status != EMFCFG_STATUS_SUCCESS)
	{
		EMF_ERROR("Unable to get the EMF stats\n");
		return (FAILURE);
	}

	seq_printf(seq, "McastDataPkts   McastDataFwd    McastFlooded    "
		    "McastDataSentUp McastDataDropped\n");
	seq_printf(seq, "%-15d %-15d %-15d %-15d %d\n",
	            emfs->mcast_data_frames, emfs->mcast_data_fwd,
	            emfs->mcast_data_flooded, emfs->mcast_data_sentup,
	            emfs->mcast_data_dropped);
	seq_printf(seq, "IgmpPkts        IgmpPktsFwd     "
		    "IgmpPktsSentUp  MFDBCacheHits   MFDBCacheMisses\n");
	seq_printf(seq, "%-15d %-15d %-15d %-15d %d\n",
	            emfs->igmp_frames, emfs->igmp_frames_fwd,
	            emfs->igmp_frames_sentup, emfs->mfdb_cache_hits,
	            emfs->mfdb_cache_misses);

	return 0;
}

static int32
emf_mfdb_list(struct seq_file *seq, void *v)
{
	emf_info_t *emfi = seq->private;
	emf_cfg_request_t cfg;
	emf_cfg_mfdb_list_t *list;
	int32 i;

	cfg.command_id = EMFCFG_CMD_MFDB_LIST;
	cfg.oper_type = EMFCFG_OPER_TYPE_GET;
	cfg.size = sizeof(cfg.arg);
	list = (emf_cfg_mfdb_list_t *)cfg.arg;

	emfc_cfg_request_process(emfi->emfci, &cfg);
	if (cfg.status != EMFCFG_STATUS_SUCCESS)
	{
		EMF_ERROR("Unable to get the MFDB list\n");
		return (FAILURE);
	}

	seq_printf(seq, "Group           Interface      Pkts\n");

	for (i = 0; i < list->num_entries; i++)
	{
		seq_printf(seq, "%08x        ", list->mfdb_entry[i].mgrp_ip);
		seq_printf(seq, "%-15s", list->mfdb_entry[i].if_name);
		seq_printf(seq, "%d\n", list->mfdb_entry[i].pkts_fwd);
	}

	return 0;
}
#else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
static int32
emf_stats_get(char *buf, char **start, off_t offset, int32 size,
              int32 *eof, void *data)
{
	emf_info_t *emfi;
	emf_cfg_request_t cfg;
	emf_stats_t *emfs;
	struct bcmstrbuf b;

	emfi = (emf_info_t *)data;
	ASSERT(emfi);

	cfg.command_id = EMFCFG_CMD_EMF_STATS;
	cfg.oper_type = EMFCFG_OPER_TYPE_GET;
	cfg.size = sizeof(emf_stats_t);
	emfs = (emf_stats_t *)cfg.arg;

	emfc_cfg_request_process(emfi->emfci, &cfg);
	if (cfg.status != EMFCFG_STATUS_SUCCESS)
	{
		EMF_ERROR("Unable to get the EMF stats\n");
		return (FAILURE);
	}

	bcm_binit(&b, buf, size);
	bcm_bprintf(&b, "McastDataPkts   McastDataFwd    McastFlooded    "
		    "McastDataSentUp McastDataDropped\n");
	bcm_bprintf(&b, "%-15d %-15d %-15d %-15d %d\n",
	            emfs->mcast_data_frames, emfs->mcast_data_fwd,
	            emfs->mcast_data_flooded, emfs->mcast_data_sentup,
	            emfs->mcast_data_dropped);
	bcm_bprintf(&b, "IgmpPkts        IgmpPktsFwd     "
		    "IgmpPktsSentUp  MFDBCacheHits   MFDBCacheMisses\n");
	bcm_bprintf(&b, "%-15d %-15d %-15d %-15d %d\n",
	            emfs->igmp_frames, emfs->igmp_frames_fwd,
	            emfs->igmp_frames_sentup, emfs->mfdb_cache_hits,
	            emfs->mfdb_cache_misses);

	if (b.size == 0)
	{
		EMF_ERROR("Input buffer overflow\n");
		return (FAILURE);
	}

	return (b.buf - b.origbuf);
}

static int32
emf_mfdb_list(char *buf, char **start, off_t offset, int32 size,
              int32 *eof, void *data)
{
	emf_info_t *emfi;
	emf_cfg_request_t cfg;
	struct bcmstrbuf b;
	emf_cfg_mfdb_list_t *list;
	int32 i;

	emfi = (emf_info_t *)data;
	cfg.command_id = EMFCFG_CMD_MFDB_LIST;
	cfg.oper_type = EMFCFG_OPER_TYPE_GET;
	cfg.size = sizeof(cfg.arg);
	list = (emf_cfg_mfdb_list_t *)cfg.arg;

	emfc_cfg_request_process(emfi->emfci, &cfg);
	if (cfg.status != EMFCFG_STATUS_SUCCESS)
	{
		EMF_ERROR("Unable to get the MFDB list\n");
		return (FAILURE);
	}

	bcm_binit(&b, buf, size);
	bcm_bprintf(&b, "Group           Interface      Pkts\n");

	for (i = 0; i < list->num_entries; i++)
	{
		bcm_bprintf(&b, "%08x        ", list->mfdb_entry[i].mgrp_ip);
		bcm_bprintf(&b, "%-15s", list->mfdb_entry[i].if_name);
		bcm_bprintf(&b, "%d\n", list->mfdb_entry[i].pkts_fwd);
	}

	if (b.size == 0)
	{
		EMF_ERROR("Input buffer overflow\n");
		return (FAILURE);
	}

	return (b.buf - b.origbuf);
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
#endif /* CONFIG_PROC_FS */
/*
 * Description: This function is called to send up the packet buffer
 *              to the IP stack.
 *
 * Input:       emfi - EMF instance information
 *              skb  - Pointer to the packet buffer.
 */
void
emf_sendup(emf_info_t *emfi, struct sk_buff *skb)
{
	/* Called only when frame is received from one of the LAN ports */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	ASSERT(skb->dev->br_port);
#endif // endif
	ASSERT(skb->protocol == __constant_htons(ETH_P_IP));

	/* Send the buffer as if the packet is being sent by bridge */
	skb->dev = emfi->br_dev;
	skb->pkt_type = PACKET_HOST;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
	skb_reset_mac_header(skb);
#else
	skb->mac.raw = skb->data;
#endif // endif

	netif_rx(skb);

	return;
}

/*
 * Description: This function is called by EMFL common code when it wants
 *              to forward the packet on to a specific port. It adds the
 *              MAC header and queues the frame to the interface.
 *
 * Input:       emfi    - EMF instance information
 *              skb     - Pointer to the packet buffer.
 *              mgrp_ip - Multicast destination address.
 *              txif    - Interface to send the frame on.
 *
 * Return:      SUCCESS or FAILURE.
 */
int32
emf_forward(emf_info_t *emfi, struct sk_buff *skb, uint32 mgrp_ip,
            struct net_device *txif, int rt_port)
{
	struct ether_header *eh;

	EMF_DEBUG("Forwarding the frame %p on to %s\n", skb, txif->name);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	ASSERT(txif->br_port);
	/* Send only when the port is in forwarding state */
	if (EMF_BRPORT_STATE(txif) != BR_STATE_FORWARDING)
	{
		EMF_INFO("Dropping the frame as the port %s is not in"
		         " FORWARDING state\n", txif->name);
		kfree_skb(skb);
		return (FAILURE);
	}
	/* there is no access to the "bridge" struct from netif in 2.6.36 */
#endif // endif

	/* No need to push and fill the ether header fields for packets received
	 * from LAN ports.
	 */
	if (rt_port) {
		eh = (struct ether_header *)skb_push(skb, ETH_HLEN);
		eh->ether_type = __constant_htons(ETH_P_IP);

		ETHER_FILL_MCAST_ADDR_FROM_IP(eh->ether_dhost, mgrp_ip);

		memcpy(eh->ether_shost, skb->dev->dev_addr, skb->dev->addr_len);
	} else {
		eh = (struct ether_header *)skb->data;
	}

	EMF_INFO("Group Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
	         eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
	         eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5]);

	/* Send buffer as if it is delivered by bridge */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
	skb_reset_mac_header(skb);
#else
	skb->mac.raw = skb->data;
#endif // endif

	skb->dev = txif;
	dev_queue_xmit(skb);

	return (SUCCESS);
}

static emf_iflist_t *
emf_iflist_find(emf_struct_t *emf, emf_info_t *emfi, struct net_device *if_ptr,
                emf_iflist_t **prev)
{
	emf_iflist_t *ptr;

	ASSERT(if_ptr != NULL);

	/* Lock is already held by the callers */

	*prev = NULL;
	for (ptr = emfi->iflist_head; ptr != NULL;
	     *prev = ptr, ptr = ptr->next)
	{
		if (ptr->if_ptr == if_ptr)
		{
			return (ptr);
		}
	}

	return (NULL);
}

static int32
emf_iflist_add(emf_struct_t *emf, emf_info_t *emfi, struct net_device *if_ptr)
{
	emf_iflist_t *ptr, *prev;

	OSL_LOCK(emf->lock);

	if (emf_iflist_find(emf, emfi, if_ptr, &prev) == NULL)
	{
		ptr = MALLOC(emfi->osh, sizeof(emf_iflist_t));

		if (ptr == NULL)
		{
			EMF_ERROR("Unable to allocate iflist entry\n");
			OSL_UNLOCK(emf->lock);
			return (FAILURE);
		}

		/* Initialize the iflist entry */
		ptr->if_ptr = if_ptr;

		/* Add the entry to iflist for this EMF instance */
		ptr->next = emfi->iflist_head;
		emfi->iflist_head = ptr;

		OSL_UNLOCK(emf->lock);
		return (SUCCESS);
	}

	OSL_UNLOCK(emf->lock);

	return (FAILURE);
}

static int32
emf_iflist_del(emf_struct_t *emf, emf_info_t *emfi, struct net_device *if_ptr)
{
	emf_iflist_t *ptr, *prev;

	OSL_LOCK(emf->lock);

	if ((ptr = emf_iflist_find(emf, emfi, if_ptr, &prev)) != NULL)
	{
		/* Delete the entry from iflist */
		if (prev != NULL)
			prev->next = ptr->next;
		else
			emfi->iflist_head = ptr->next;

		MFREE(emfi->osh, ptr, sizeof(emf_iflist_t));

		OSL_UNLOCK(emf->lock);
		return (SUCCESS);
	}

	OSL_UNLOCK(emf->lock);

	return (FAILURE);
}

static void
emf_iflist_clear(emf_struct_t *emf, emf_info_t *emfi)
{
	emf_iflist_t *ptr, *next = NULL;

	/* Lock is already held by the callers */

	ptr = emfi->iflist_head;
	while (ptr != NULL)
	{
		next = ptr->next;
		MFREE(emfi->osh, ptr, sizeof(emf_iflist_t));
		ptr = next;
	}

	emfi->iflist_head = NULL;

	return;
}

static void *
emf_if_name_validate(uint8 *if_name)
{
	struct net_device *dev;

	/* Get the interface pointer */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	dev = dev_get_by_name(if_name);
#else
	dev = dev_get_by_name(&init_net, if_name);
#endif // endif

	if (dev == NULL)
	{
		EMF_ERROR("Interface %s doesn't exist\n", if_name);
		return (NULL);
	}

	dev_put(dev);

	return (dev);
}

#ifdef CONFIG_NETFILTER
static struct nf_hook_ops emf_nf_ops[] =
{
	{
		.hook =         emf_br_pre_hook,
		.pf =           PF_BRIDGE,	/* should be NFPROTO_BRIDGE */
		.hooknum =      NF_BR_PRE_ROUTING,
		.priority =     0
	},
	{
		.hook =         emf_ip_post_hook,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
		.pf =           PF_INET,
		.hooknum =      NF_IP_POST_ROUTING,
#else
		.pf =           NFPROTO_IPV4,
		.hooknum =      NF_INET_POST_ROUTING,
#endif // endif
		.priority =     NF_IP_PRI_MANGLE
	}
};

int32
emf_hooks_register(emf_info_t *emfi)
{
	int32 i, ret, j;

	if (emf->hooks_reg)
		return (SUCCESS);

	/* Register at Netfilter bridge pre-routing and ip post-routing
	 * hooks to capture and process the packets.
	 */
	for (i = 0; i < sizeof(emf_nf_ops)/sizeof(struct nf_hook_ops); i++)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
		ret = nf_register_net_hook(&init_net, &emf_nf_ops[i]);
#else
		ret = nf_register_hook(&emf_nf_ops[i]);
#endif // endif

		if (ret < 0)
		{
			EMF_ERROR("Unable to register netfilter hooks\n");
			for (j = 0; j < i; j++)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
				nf_unregister_net_hook(&init_net, &emf_nf_ops[j]);
#else
				nf_unregister_hook(&emf_nf_ops[j]);
#endif // endif
			return (FAILURE);
		}
	}

	emf->hooks_reg++;

	return (SUCCESS);
}

void
emf_hooks_unregister(emf_info_t *emfi)
{
	int32 i;

	if (emf->hooks_reg == 0)
	{
		EMF_ERROR("Hooks already unregistered\n");
		return;
	}

	emf->hooks_reg--;

	/* Unregister all the hooks */
	for (i = 0; i < sizeof(emf_nf_ops)/sizeof(struct nf_hook_ops); i++)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
		nf_unregister_net_hook(&init_net, &emf_nf_ops[i]);
#else
		nf_unregister_hook(&emf_nf_ops[i]);
#endif // endif
	}

	return;
}
#endif /* CONFIG_NETFILTER */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
#include <linux/seq_file.h>

static int emf_proc_stats_get_open(struct inode *inode, struct file *file)
{
	return single_open(file, emf_stats_get, PDE_DATA(inode));
}

static const struct file_operations emf_proc_stats_get_fops = {
	.owner          = THIS_MODULE,
	.open           = emf_proc_stats_get_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release,
};

static int emf_proc_mfdb_list_open(struct inode *inode, struct file *file)
{
	return single_open(file, emf_mfdb_list, PDE_DATA(inode));
}

static const struct file_operations emf_proc_mfdb_list_fops = {
	.owner          = THIS_MODULE,
	.open           = emf_proc_mfdb_list_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
/*
 * Description: This function is called when the user application enables
 *              EMF on a bridge interface. It primarily allocates memory
 *              for instance data and calls the common code init function.
 *
 * Input:       emf     - EMF module global data pointer
 *              inst_id - EMF instance name
 *              br_ptr  - Bridge device pointer
 */
static emf_info_t *
emf_instance_add(emf_struct_t *emf, int8 *inst_id, struct net_device *br_ptr)
{
	emf_info_t *emfi;
	osl_t *osh;
#ifdef CONFIG_PROC_FS
	uint8 proc_name[64];
#endif /* CONFIG_PROC_FS */
	emfc_wrapper_t emfl = {0};

	if (emf->inst_count > EMF_MAX_INST)
	{
		EMF_ERROR("Max instance limit %d exceeded\n", EMF_MAX_INST);
		return (NULL);
	}

	emf->inst_count++;

	EMF_INFO("Creating EMF instance for %s\n", inst_id);

	osh = osl_attach(NULL, PCI_BUS, FALSE);

	ASSERT(osh);

	/* Allocate os specfic EMF info object */
	emfi = MALLOC(osh, sizeof(emf_info_t));
	if (emfi == NULL)
	{
		EMF_ERROR("Out of memory allocating emf_info\n");
		osl_detach(osh);
		return (NULL);
	}

	emfi->osh = osh;

	/* Save the EMF instance identifier */
	strncpy(emfi->inst_id, inst_id, IFNAMSIZ);
	emfi->inst_id[IFNAMSIZ - 1] = 0;

	/* Save the device pointer */
	emfi->br_dev = br_ptr;

	/* Fill the linux wrapper specific functions */
	emfl.forward_fn = (forward_fn_ptr)emf_forward;
	emfl.sendup_fn = (sendup_fn_ptr)emf_sendup;
#ifdef CONFIG_NETFILTER
	emfl.hooks_register_fn = (hooks_register_fn_ptr)emf_hooks_register;
	emfl.hooks_unregister_fn = (hooks_unregister_fn_ptr)emf_hooks_unregister;
#endif // endif

	/* Initialize EMFC instance */
	if ((emfi->emfci = emfc_init(inst_id, (void *)emfi, osh, &emfl)) == NULL)
	{
		EMF_ERROR("EMFC init failed\n");
		MFREE(osh, emfi, sizeof(emf_info_t));
		osl_detach(osh);
		return (NULL);
	}

	EMF_INFO("Created EMFC instance for %s\n", inst_id);

	/* Initialize the iflist head */
	emfi->iflist_head = NULL;

#ifdef CONFIG_PROC_FS
	sprintf(proc_name, "emf/emf_stats_%s", inst_id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	proc_create_data(proc_name, S_IRUGO, 0, &emf_proc_stats_get_fops, (void *)emfi);
#else
	create_proc_read_entry(proc_name, 0, 0, emf_stats_get, emfi);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
	sprintf(proc_name, "emf/emfdb_%s", inst_id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	proc_create_data(proc_name, S_IRUGO, 0, &emf_proc_mfdb_list_fops, (void *)emfi);
#else
	create_proc_read_entry(proc_name, 0, 0, emf_mfdb_list, emfi);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
#endif /* CONFIG_PROC_FS */

	/* Add to the global EMF instance list */
	emfi->next = emf->list_head;
	emf->list_head = emfi;

	return (emfi);
}

/*
 * Description: This function is called when user disables EMF on a bridge
 *              interface. It unregisters the Netfilter hook functions,
 *              calls the common code cleanup routine and releases all the
 *              resources allocated during instance creation.
 *
 * Input:       emf      - EMF module global data pointer
 *              emf_info - EMF instance data pointer
 */
static int32
emf_instance_del(emf_struct_t *emf, emf_info_t *emfi)
{
	bool found = FALSE;
	osl_t *osh;
	emf_info_t *ptr, *prev;
#ifdef CONFIG_PROC_FS
	uint8 proc_name[64];
#endif /* CONFIG_PROC_FS */

	/* Interfaces attached to the EMF instance should be deleted first */
	emf_iflist_clear(emf, emfi);

	/* Delete the EMF instance */
	prev = NULL;
	for (ptr = emf->list_head; ptr != NULL; prev = ptr, ptr = ptr->next)
	{
		if (ptr == emfi)
		{
			found = TRUE;
			if (prev != NULL)
				prev->next = ptr->next;
			else
				emf->list_head = emf->list_head->next;
			break;
		}
	}

	if (!found)
	{
		EMF_ERROR("EMF instance not found\n");
		return (FAILURE);
	}

	emf->inst_count--;

	/* Free the EMF instance */
	OSL_UNLOCK(emf->lock);
	emf_hooks_unregister(ptr);

#ifdef CONFIG_PROC_FS
	sprintf(proc_name, "emf/emf_stats_%s", emfi->inst_id);
	remove_proc_entry(proc_name, 0);
	sprintf(proc_name, "emf/emfdb_%s", emfi->inst_id);
	remove_proc_entry(proc_name, 0);
#endif /* CONFIG_PROC_FS */

	OSL_LOCK(emf->lock);
	emfc_exit(ptr->emfci);
	osh = ptr->osh;
	MFREE(emfi->osh, ptr, sizeof(emf_info_t));
	osl_detach(osh);

	return (SUCCESS);
}

int32
emf_iflist_list(emf_info_t *emfi, emf_cfg_if_list_t *list, uint32 size)
{
	int32 index = 0;
	emf_iflist_t *ptr;

	if (emfi == NULL)
	{
		EMF_ERROR("Invalid EMF instance handle passed\n");
		return (FAILURE);
	}

	if (list == NULL)
	{
		EMF_ERROR("Invalid buffer input\n");
		return (FAILURE);
	}

	for (ptr = emfi->iflist_head; ptr != NULL; ptr = ptr->next)
	{
		if (strlen(ptr->if_ptr->name) >= 16) {
			EMF_ERROR("if_ptr name %s is longer than 15 chars\n", ptr->if_ptr->name);
			return (FAILURE);
		}
		else
			strncpy(list->if_entry[index++].if_name,
				ptr->if_ptr->name, 16);
	}

	/* Update the total number of entries */
	list->num_entries = index;

	return (SUCCESS);
}

/*
 * Description: This function handles the OS specific processing
 *              required for configuration commands.
 *
 * Input:       data - Configuration command parameters
 */
void
emf_cfg_request_process(emf_cfg_request_t *cfg)
{
	emf_info_t *emfi;
	emf_cfg_mfdb_t *mfdb;
	emf_cfg_uffp_t *uffp;
	emf_cfg_rtport_t *rtport;
	emf_cfg_if_t *if_cfg;
	emf_iflist_t *iflist_prev, *ifl_ptr;
	struct net_device *if_ptr, *br_ptr;

	BUG_ON(cfg == NULL);

	/* Validate the instance identifier */
	br_ptr = emf_if_name_validate(cfg->inst_id);
	if (br_ptr == NULL)
	{
		cfg->status = EMFCFG_STATUS_FAILURE;
		cfg->size = sprintf(cfg->arg, "Unknown instance identifier %s\n",
		                    cfg->inst_id);
		return;
	}

	/* Locate the EMF instance */
	emfi = emf_instance_find_by_brptr(emf, br_ptr);
	if ((emfi == NULL) && (cfg->command_id != EMFCFG_CMD_BR_ADD))
	{
		cfg->status = EMFCFG_STATUS_FAILURE;
		cfg->size = sprintf(cfg->arg, "Invalid instance identifier %s\n",
		                    cfg->inst_id);
		return;
	}

	/* Convert the interface name in arguments to interface pointer */
	switch (cfg->command_id)
	{
		case EMFCFG_CMD_MFDB_ADD:
		case EMFCFG_CMD_MFDB_DEL:
			mfdb = (emf_cfg_mfdb_t *)cfg->arg;

			mfdb->if_ptr = emf_if_name_validate(mfdb->if_name);
			if (mfdb->if_ptr == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
				                    "Invalid interface specified "
						    "during MFDB entry add/delete\n");
				return;
			}

			OSL_LOCK(emf->lock);
			/* The interfaces specified should be in the iflist */
			ifl_ptr = emf_iflist_find(emf, emfi, mfdb->if_ptr, &iflist_prev);
			OSL_UNLOCK(emf->lock);

			if ((strncmp(mfdb->if_name, "wds", 3) != 0) &&
			    ifl_ptr == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
				                    "Interface not part of bridge %s\n",
				                    br_ptr->name);
			} else {
				emfc_cfg_request_process(emfi->emfci, cfg);
			}

			break;

		case EMFCFG_CMD_IF_ADD:
		case EMFCFG_CMD_IF_DEL:
			/* Add/Del the interface to/from the global list */
			if_cfg = (emf_cfg_if_t *)cfg->arg;

			if_ptr = emf_if_name_validate(if_cfg->if_name);

			if (if_ptr == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
						"Invalid interface specified\n");
				return;
			}

			if (cfg->command_id == EMFCFG_CMD_IF_ADD)
			{
				if (emf_iflist_add(emf, emfi, if_ptr) != SUCCESS)
				{
					cfg->status = EMFCFG_STATUS_FAILURE;
					cfg->size = sprintf(cfg->arg,
					                    "Interface add failed\n");
					return;
				}

				if (emfc_iflist_add(emfi->emfci, if_ptr) != SUCCESS)
				{
					emf_iflist_del(emf, emfi, if_ptr);
					cfg->status = EMFCFG_STATUS_FAILURE;
					cfg->size = sprintf(cfg->arg,
					                    "Interface add failed\n");
					return;
				}
			}

			if (cfg->command_id == EMFCFG_CMD_IF_DEL)
			{
				if ((emf_iflist_del(emf, emfi, if_ptr) != SUCCESS) ||
				    (emfc_iflist_del(emfi->emfci, if_ptr) != SUCCESS))
				{
					cfg->status = EMFCFG_STATUS_FAILURE;
					cfg->size = sprintf(cfg->arg,
					                    "Interface delete failed\n");
					return;
				}
			}

			cfg->status = EMFCFG_STATUS_SUCCESS;
			break;

		case EMFCFG_CMD_IF_LIST:
			if (emf_iflist_list(emfi, (emf_cfg_if_list_t *)cfg->arg,
			                    cfg->size) != SUCCESS)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size += sprintf(cfg->arg, "EMF if list get failed\n");
				break;
			}

			cfg->status = EMFCFG_STATUS_SUCCESS;
			break;

		case EMFCFG_CMD_BR_ADD:
			if (emfi != NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
				                    "EMF already configured for %s\n",
				                    cfg->inst_id);
				return;
			}

			/* Create a new EMF instance corresponding to the bridge
			 * interface.
			 */
			emfi = emf_instance_add(emf, cfg->inst_id, br_ptr);

			if (emfi == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "EMF add on %s failed\n",
				                    cfg->inst_id);
				return;
			}

			cfg->status = EMFCFG_STATUS_SUCCESS;
			break;

		case EMFCFG_CMD_BR_DEL:
			OSL_LOCK(emf->lock);
			/* Delete and free the EMF instance */
			if (emf_instance_del(emf, emfi) != SUCCESS)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "EMF delete on %s failed\n",
				                    cfg->inst_id);
			} else {
				cfg->status = EMFCFG_STATUS_SUCCESS;
			}
			OSL_UNLOCK(emf->lock);
			break;

		case EMFCFG_CMD_UFFP_ADD:
		case EMFCFG_CMD_UFFP_DEL:
			uffp = (emf_cfg_uffp_t *)cfg->arg;
			if_ptr = uffp->if_ptr = emf_if_name_validate(uffp->if_name);
			/* FALLTHRU */

		case EMFCFG_CMD_RTPORT_ADD:
		case EMFCFG_CMD_RTPORT_DEL:
			rtport = (emf_cfg_rtport_t *)cfg->arg;
			if_ptr = rtport->if_ptr = emf_if_name_validate(rtport->if_name);

			if (if_ptr == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
				                    "Invalid iface specified for deletion\n");
				return;
			}

			OSL_LOCK(emf->lock);
			ifl_ptr = emf_iflist_find(emf, emfi, if_ptr, &iflist_prev);
			OSL_UNLOCK(emf->lock);

			if (ifl_ptr == NULL)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg,
				                    "Interface not part of bridge %s\n",
				                    br_ptr->name);
			} else {
				emfc_cfg_request_process(emfi->emfci, cfg);
			}

			break;

		default:
			emfc_cfg_request_process(emfi->emfci, cfg);
			break;
	}

	return;
}

/*
 * Description: This function is called by Linux kernel when user
 *              applications sends a message on netlink socket. It
 *              dequeues the message, calls the functions to process
 *              the commands and sends the result back to user.
 *
 * Input:       sk  - Kernel socket structure
 *              len - Length of the message received from user app.
 */
static void
emf_netlink_sock_cb(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	struct sock *sk, int32 len)
#else
	struct sk_buff *skb)
#endif // endif
{
	struct nlmsghdr	*nlh = NULL;
	uint8 *data = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	struct sk_buff	*skb;
#endif	/* >= 2.6.36 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	/* buffer already dequeued, will be freed when we return */
	skb = skb_clone(skb, GFP_KERNEL);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	/* Dequeue the message from netlink socket */
	while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
#else	/* 2.4.x */
	while ((skb = skb_dequeue(&sk->receive_queue)) != NULL)
#endif // endif
	{
		EMF_DEBUG("Length of the command buffer %d\n", skb->len);

		/* Check the buffer for min size */
		if (skb == NULL || skb->len < sizeof(emf_cfg_request_t))
		{
			EMF_ERROR("Configuration request size not > %zu or skb_clone failed\n",
			          sizeof(emf_cfg_request_t));
			if (skb)
				dev_kfree_skb(skb);
			return;
		}

		/* Buffer contains netlink header followed by data */
		nlh = (struct nlmsghdr *)skb->data;
		data = NLMSG_DATA(nlh);

		/* Process the message */
		emf_cfg_request_process((emf_cfg_request_t *)data);

		/* Send the result to user process */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		NETLINK_CB(skb).portid = nlh->nlmsg_pid;
#else
		NETLINK_CB(skb).pid = nlh->nlmsg_pid;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0) */
		NETLINK_CB(skb).dst_group = 0;
#else
		NETLINK_CB(skb).groups = 0;
		NETLINK_CB(skb).pid = 0;
		NETLINK_CB(skb).dst_groups = 0;
		NETLINK_CB(skb).dst_pid = nlh->nlmsg_pid;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */

		netlink_unicast(emf->nl_sk, skb, nlh->nlmsg_pid, MSG_DONTWAIT);
	}

	return;
}

static void
emf_instances_clear(emf_struct_t *emf)
{
	emf_info_t *ptr, *tmp;

	OSL_LOCK(emf->lock);

	ptr = emf->list_head;

	while (ptr != NULL)
	{
		tmp = ptr->next;
		emf_instance_del(emf, ptr);
		ptr = tmp;
	}

	emf->list_head = NULL;

	OSL_UNLOCK(emf->lock);

	return;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
struct netlink_kernel_cfg emf_cfg = {
	.input  = emf_netlink_sock_cb,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */

/*
 * Description: This function is called by net_device notifier when network device events occur
 *
 * Input:       nb -	notifier_block passed in register_netdevice_notifier().
 *		event - Net_device event
 *		ptr -	Used in different ways under different circumstances.
 *			E.g., when a new net_device is registered, associated notification uses
 *			this ptr to identify the net_device data structure.
 *
 * Return:      Return any of the NOTIFY_XXX values defined in include/linux/notifier.h
 *		NOTIFY_OK	- Notification was processed correctly.
 *		NOTIFY_DONE	- Not interested in the notification.
 */

static int emf_net_device_event_notify(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct net_device *dev = NULL;
	emf_info_t *emfi = NULL;
	int ret = NOTIFY_DONE;

	switch (event)
	{
		case NETDEV_UNREGISTER:
		{
			ASSERT(ptr != NULL);
			dev = (struct net_device *)ptr;

			if ((emfi = emf_instance_find_by_ifptr(emf, dev)) == NULL) {
				break;
			}

			/* Delete interface from EMF/IGS database */
			emf_iflist_del(emf, emfi, dev);
			emfc_iflist_del(emfi->emfci, dev);
			ret = NOTIFY_OK;
			break;
		}

		default:
			break;
	}
	return ret;
}

#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) */

/*
 * Description: This function is called during module load time. It
 *              opens communication channel to configure and start EMF.
 */
static int32 __init
emf_module_init(void)
{
	EMF_DEBUG("Loading EMF\n");

	/* Allocate EMF global data object */
	emf = MALLOC(NULL, sizeof(emf_struct_t));
	if (emf == NULL)
	{
		EMF_ERROR("Out of memory allocating emf_info\n");
		return (FAILURE);
	}

	memset(emf, 0, sizeof(emf_struct_t));

#if !defined(BCM_NBUFF_WLMCAST_IPV6)

	/* LR: move this before netlink_create to avoid race condition */
	emf->lock = OSL_LOCK_CREATE("EMF Instance List");

	if (emf->lock == NULL)
	{
		EMF_ERROR("EMF instance list lock create failed\n");
		memset(emf, 0, sizeof(emf_struct_t));
		MFREE(NULL, emf, sizeof(emf_struct_t));
		return (FAILURE);
	}

	/* Create a Netlink socket in kernel-space */
#define NETLINK_EMFC 17		/* Still vacant in 2.6.36 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	emf->nl_sk = netlink_kernel_create(
			&init_net,	/* struct net */
			NETLINK_EMFC,	/* unit ? */
			&emf_cfg);	/* callback */
#else
	emf->nl_sk = netlink_kernel_create(
			&init_net,	/* struct net */
			NETLINK_EMFC,	/* unit ? */
			0,		/* group ? */
			emf_netlink_sock_cb,	/* callback */
			NULL,		/* mutex */
			THIS_MODULE);	/* module */
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	emf->nl_sk = netlink_kernel_create(NETLINK_EMFC, 0, emf_netlink_sock_cb,
	                                   NULL, THIS_MODULE);
#else
	emf->nl_sk = netlink_kernel_create(NETLINK_EMFC, emf_netlink_sock_cb);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */
	if (emf->nl_sk == NULL)
	{
		OSL_LOCK_DESTROY(emf->lock);
		EMF_ERROR("Netlink kernel socket create failed\n");
		MFREE(NULL, emf, sizeof(emf_struct_t));
		return (FAILURE);
	}
#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) */

	/* Call the common code global init function */
	if (emfc_module_init() != SUCCESS)
	{
		OSL_LOCK_DESTROY(emf->lock);
		MFREE(NULL, emf, sizeof(emf_struct_t));
		return (FAILURE);
	}

#if !defined(BCM_NBUFF_WLMCAST_IPV6)
	/*  Register a notifier to be called when network device events occur */
	if (register_netdevice_notifier(&emf_net_device_nb) < 0) {
		OSL_LOCK_DESTROY(emf->lock);
		MFREE(NULL, emf, sizeof(emf_struct_t));
		return (FAILURE);
	}

#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) */

#if !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG)
#ifdef CONFIG_PROC_FS
	/* create /proc/emf */
	root_emf_dir = proc_mkdir("emf", NULL);
	if (!root_emf_dir) {
		OSL_LOCK_DESTROY(emf->lock);
		MFREE(NULL, emf, sizeof(emf_struct_t));
		return (FAILURE);
	}
#endif // endif
#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG) */

	EMF_DEBUG("EMF Init done\n");

	return (SUCCESS);
}

static void __exit
emf_module_exit(void)
{
	/* Call the common code global exit function */
	emfc_module_exit();

#if !defined(BCM_NBUFF_WLMCAST_IPV6)
	/* Clean up the instances and exit */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	netlink_kernel_release(emf->nl_sk);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	sock_release(emf->nl_sk->sk_socket);
#else
	sock_release(emf->nl_sk->socket);
#endif // endif
	emf_instances_clear(emf);

	/* Unregister net_device notifier block */
	unregister_netdevice_notifier(&emf_net_device_nb);
	OSL_LOCK_DESTROY(emf->lock);
#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) */

#if !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG)
#ifdef CONFIG_PROC_FS
	if (root_emf_dir)
		remove_proc_entry("emf", NULL);
#endif // endif
#endif /* !defined(BCM_NBUFF_WLMCAST_IPV6) || defined(BCM_WMF_MCAST_DBG) */

	memset(emf, 0, sizeof(emf_struct_t));
	MFREE(NULL, emf, sizeof(emf_struct_t));

	return;
}

module_init(emf_module_init);
module_exit(emf_module_exit);

#ifdef BCM_NBUFF_WLMCAST_IPV6

int ipv6_type(const struct ipv6_addr *addr)
{
	return ipv6_addr_type((const struct in6_addr *)addr);
}

void
*emfc_wmf_get_igsc(int ifindex)
{
	struct net_device *from_device = dev_get_by_index(&init_net, ifindex);
	void *ret = NULL;
	if (from_device) {
		struct priv_link *wlif = *(priv_link_t **)netdev_priv(from_device);
		if (wlif && wlif->nic_hook_fn)
			ret = wlif->nic_hook_fn(WLEMF_CMD_GETIGSC, from_device, NULL);
		dev_put(from_device);
	}
	return ret;
}

void
*emfc_wmf_get_igsc_by_dev(struct net_device *from_device)
{
	void *ret = NULL;
	if (from_device) {
		struct priv_link *wlif = *(priv_link_t **)netdev_priv(from_device);
		if (wlif && wlif->nic_hook_fn)
			ret = wlif->nic_hook_fn(WLEMF_CMD_GETIGSC, from_device, NULL);
	}
	return ret;
}

void
*emfc_wmf_scbfind(int ifindex, unsigned char *mac)
{
	struct net_device *from_device = dev_get_by_index(&init_net, ifindex);
	void *ret = NULL;
	if (from_device) {
		struct priv_link *wlif = *(priv_link_t **)netdev_priv(from_device);
		if (wlif && wlif->nic_hook_fn)
			ret = wlif->nic_hook_fn(WLEMF_CMD_SCBFIND, from_device, mac);
		dev_put(from_device);
	}
	return ret;
}

#endif /* BCM_NBUFF_WLMCAST_IPV6 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
EXPORT_SYMBOL(emfc_init);
EXPORT_SYMBOL(emfc_exit);
EXPORT_SYMBOL(emfc_input);
EXPORT_SYMBOL(emfc_cfg_request_process);
EXPORT_SYMBOL(emfc_mfdb_membership_add);
EXPORT_SYMBOL(emfc_mfdb_membership_del);
EXPORT_SYMBOL(emfc_rtport_add);
EXPORT_SYMBOL(emfc_rtport_del);
EXPORT_SYMBOL(emfc_igmp_snooper_register);
EXPORT_SYMBOL(emfc_igmp_snooper_unregister);
#ifdef BCM_NBUFF_WLMCAST_IPV6
EXPORT_SYMBOL(emfc_ipv6_input);
EXPORT_SYMBOL(emfc_mfdb_ipv6_membership_del);
EXPORT_SYMBOL(emfc_mfdb_ipv6_membership_add);
EXPORT_SYMBOL(emfc_wmf_scbfind);
EXPORT_SYMBOL(emfc_wmf_get_igsc);
EXPORT_SYMBOL(emfc_wmf_get_igsc_by_dev);
#endif /* BCM_NBUFF_WLMCAST */
#endif /* LINUX_VERSION_CODE */
