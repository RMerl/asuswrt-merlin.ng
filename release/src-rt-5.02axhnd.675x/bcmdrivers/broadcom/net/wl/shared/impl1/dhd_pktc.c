/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#if defined(PKTC_TBL)

#include <typedefs.h>
#include <osl.h>

#include <linux/skbuff.h>
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd_dbg.h>
#include <dhd.h>
#include <dhd_blog.h>
#include <wl_pktc.h>

/*
 * Global variables
 */
wl_pktc_tbl_t *g_pktc_tbl = NULL;
uint8 g_pktc_tbl_ref_cnt = 0;     /* reference count for g_pktc_tbl */
pktc_handle_t pktc_wldev[WLAN_DEVICE_MAX] = {0};

wl_pktc_tbl_t *dhd_pktc_attach(void *p)
{
	wl_pktc_tbl_t *tbl;
	dhd_pub_t *dhdp = (dhd_pub_t *)p; 
	size_t size = sizeof(wl_pktc_tbl_t) * CHAIN_ENTRY_NUM;

	/* Allocate pktc table */
	if (g_pktc_tbl == NULL) {
		g_pktc_tbl = (wl_pktc_tbl_t *) kmalloc(size, GFP_KERNEL);
		if (!g_pktc_tbl) {
			DHD_ERROR(("%s: malloc of g_pktc_tbl failed\n", __FUNCTION__));
			return NULL;
		}
		bzero((void*)g_pktc_tbl, size);
	}
	/* Increment pktc table reference count */
	g_pktc_tbl_ref_cnt++;

	dhd_pktc_req_hook = dhd_pktc_req;
#if defined(BCM_BLOG)
	dhd_pktc_del_hook = dhd_pktc_del;
#endif /* BCM_BLOG */

	tbl = (wl_pktc_tbl_t *)dhd_pktc_req(PKTC_TBL_GET_START_ADDRESS, 0, 0, 0);

	dhdp->pktc = 1;	/* by default, rx pktc is enabled */
	dhdp->pktcbnd = 256; /*DHD_PKT_CTF_MAX_CHAIN_LEN*/ // rx chaining bound

	return tbl;
}

void dhd_pktc_detach(void *p)
{
	/* Free pktc table if there are no more interface references to the data*/
	if (g_pktc_tbl) {
		/* Decrement the pktc table reference count */
		g_pktc_tbl_ref_cnt--;

		if (g_pktc_tbl_ref_cnt <= 0) {
			kfree(g_pktc_tbl);
			g_pktc_tbl = NULL;
		}
	}
#ifdef BCM_BLOG
	fdb_check_expired_dhd_hook = NULL;
#endif
}

/* for packet chaining */
INLINE BCMFASTPATH 
unsigned long dhd_pktc_req( int req_id, unsigned long param0, unsigned long param1, unsigned long param2 )
{
	int i;
	struct bcmstrbuf *strbuf;
	wl_pktc_tbl_t *pt;

	switch (req_id) {
	case PKTC_TBL_GET_BY_DA:
            /* param0 is DA */
            return (PKTC_TBL_FN_LOOKUP(g_pktc_tbl, (uint8_t*)param0));

	case PKTC_TBL_GET_START_ADDRESS:
            return (unsigned long)(&g_pktc_tbl[0]);

	case PKTC_TBL_GET_BY_IDX:
		/* param0 is pktc chain table index */
		if (param0 >= CHAIN_ENTRY_NUM) {
			printk("%s: chain idx is out of range! (%ld, 0x%lx)\n", __FUNCTION__, param0, param0);
			return 0;
		}
		if (!(g_pktc_tbl[param0].in_use) || !(g_pktc_tbl[param0].wl_handle)) {
			printk("Error : chain idx %ld is not in use or invalid handle 0x%lx\n",
				param0, g_pktc_tbl[param0].wl_handle);
			return 0;
		}
		return (unsigned long)(&g_pktc_tbl[param0]);

	case PKTC_TBL_UPDATE:
		param2 = (unsigned long)NULL;

		/* param1 is tx device */
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].dev == param1) {
				param2 = (unsigned long)&pktc_wldev[i];
				break;
			}
		}
		/* param0 is addr, param1 is dev, param2 is wl handle if any */
		pt = (wl_pktc_tbl_t *)PKTC_TBL_FN_UPDATE(g_pktc_tbl, (uint8_t *)param0,
			(struct net_device *)param1, (pktc_handle_t *)param2);
		if ((pt == NULL) || (pt->idx >= CHAIN_ENTRY_NUM))
			return PKTC_INVALID_CHAIN_IDX;

		/* if wl_handle is NULL and tx dev is 'wl', which means pkt is going to dhd drv,
		 * we should not create the chain entry for it, hence pkt won't be chained and sent
		 * to tx_dev directly but fcache.
		 */
		if ((pt->tx_dev == NULL) || ((param2 == (unsigned long)NULL) &&
			(!strncmp(pt->tx_dev->name, "wl", 2)))) {
			/* remove this chain entry */
			PKTC_TBL_FN_CLEAR(g_pktc_tbl, (uint8 *)param0);
			return PKTC_INVALID_CHAIN_IDX;
		}

		return pt->idx; /* return chain index */
		
	case PKTC_TBL_DELETE:
		PKTC_TBL_FN_CLEAR(g_pktc_tbl, (uint8 *)param0);
		return 0;

	case PKTC_TBL_DUMP:
		strbuf = (struct bcmstrbuf *)param0;
		bcm_bprintf(strbuf, "\npktc dump: (rx path)\n");
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (g_pktc_tbl[i].in_use) 
			{
				bcm_bprintf(strbuf, "[%02d] %02x:%02x:%02x:%02x:%02x:%02x, dev=%s, hits=%d\n",
				g_pktc_tbl[i].idx,
				g_pktc_tbl[i].ea.octet[0],
				g_pktc_tbl[i].ea.octet[1],
				g_pktc_tbl[i].ea.octet[2],
				g_pktc_tbl[i].ea.octet[3],
				g_pktc_tbl[i].ea.octet[4],
				g_pktc_tbl[i].ea.octet[5],
				(g_pktc_tbl[i].tx_dev == NULL) ? "NULL" : g_pktc_tbl[i].tx_dev->name,
				g_pktc_tbl[i].hits);
			}
		}
		return 0;

	case PKTC_TBL_FLUSH:
		/* DHD_INFO(("%s: pktc_tbl flush!\n", __FUNCTION__)); */
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (g_pktc_tbl[i].in_use)
				memset(&g_pktc_tbl[i], 0, sizeof(wl_pktc_tbl_t));
		}
		return 0;

	default:
		return 0;
	}
	return 0;
}

void dhd_pktc_del(unsigned long addr, struct net_device * net_device) 
{
	dhd_pktc_req(PKTC_TBL_DELETE, addr, 0, 0);
}

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
INLINE BCMFASTPATH 
uint16 pktc_tbl_hash(uint8 *da)
{
	return (hndcrc16((uint8 *)da, 6, CRC16_INIT_VALUE) % (CHAIN_ENTRY_NUM/2));
}
#else
INLINE BCMFASTPATH 
uint8 pktc_tbl_hash(uint8 *da)
{
	return (hndcrc8((uint8 *)da, 6, CRC8_INIT_VALUE) % (CHAIN_ENTRY_NUM/2));
}
#endif

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
INLINE BCMFASTPATH 
uint16 pktc_tbl_hash2(uint8 *da)
{
	return ((hndcrc16((uint8 *)da, 6, CRC16_INIT_VALUE) % (CHAIN_ENTRY_NUM/2)) + (CHAIN_ENTRY_NUM/2));
}
#else
INLINE BCMFASTPATH 
uint8 pktc_tbl_hash2(uint8 *da)
{
	return ((hndcrc8((uint8 *)da, 6, CRC8_INIT_VALUE) % (CHAIN_ENTRY_NUM/2)) + (CHAIN_ENTRY_NUM/2));
}
#endif

void pktc_tbl_clear_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	int i = 0;
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0)
		goto free_pkts;

	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0)
		goto free_pkts;

	/* return: do nothing */
	return;

free_pkts:
	for (i=0; i < PKT_PRIO_LVL_CNT; i++) {
		if (pt->chain[i].chead != NULL) {
			PKTCFREE(NULL, pt->chain[i].chead, TRUE);
		}
	}
	memset(pt, 0, sizeof(wl_pktc_tbl_t));
	return;
}

unsigned long pktc_tbl_update_fn(wl_pktc_tbl_t *tbl, uint8_t *da, struct net_device *dev,
	pktc_handle_t *handle_p)
{
	uint16_t pktc_tbl_hash_idx = 0, pktc_tbl_hash_idx2 = 0;
	wl_pktc_tbl_t *pt, *pt2;

	if (!dev) { /* device is a mandatory parameter */
		goto invalid_param_out;
	}
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pktc_tbl_hash_idx2 = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	pt2 = &tbl[pktc_tbl_hash_idx2];
	if (!pt->in_use && !pt2->in_use) {
		/* Both Primary & Secondary hash index not in-use; Grab the first one */
		goto add_primary;
	}
	else if (!pt->in_use) { /* Primary not in-use */
		if (_eacmp(pt2->ea.octet, (da)) == 0) { /* Exist in secondary */
			goto add_secondary_exist;
		} else { /* Secondary is occupied - use unsed Primary */
			goto add_primary;
		}
	}
	else if (!pt2->in_use) { /* Secondary not in-use */
		if (_eacmp(pt->ea.octet, (da)) == 0) { /* Exist in Primary */
			goto add_primary_exist;
		} else { /* Primary is occupied - use unsed Secondary */
			goto add_secondary;
		}
	} else { /* Both are in-use */
		if (_eacmp(pt->ea.octet, (da)) == 0) { /* Exist in Primary */
			goto add_primary_exist;
		}
		else if (_eacmp(pt2->ea.octet, (da)) == 0)  { /* Exist in Secondary */
			goto add_secondary_exist;
		}
	}
invalid_param_out:
	/* Reaching here means - both are occupied with different MAC : Unavailable */
#if 0
	printk("Hash collision : Entry %d occupied [%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pktc_tbl_hash_idx, pt->ea.octet[0], pt->ea.octet[1], pt->ea.octet[2],
		pt->ea.octet[3], pt->ea.octet[4], pt->ea.octet[5]);
#endif
	return 0;

add_primary:
	pt->ea = *(struct _mac_address *)(da);
	pt->idx = pktc_tbl_hash_idx;
	pt->in_use = 1;
add_primary_exist:
	pt->tx_dev = dev;
	if (handle_p) {
		pt->wl_handle = handle_p->handle;
#if defined(BCM_WFD)
		pt->wfd_idx = handle_p->wfd_idx;
#endif
	}
	return (unsigned long)pt;
add_secondary:
	pt2->ea = *(struct _mac_address *)(da);
	pt2->idx = pktc_tbl_hash_idx2;
	pt2->in_use = 1;
add_secondary_exist:
	pt2->tx_dev = dev;
	if (handle_p) {
		pt2->wl_handle = handle_p->handle;
#if defined(BCM_WFD)
		pt2->wfd_idx = handle_p->wfd_idx;
#endif
	}
	return (unsigned long)pt2;
}

INLINE BCMFASTPATH 
unsigned long pktc_tbl_lookup_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		return (unsigned long)pt;
	}
	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		return (unsigned long)pt;
	}
	return 0;
}

/* this is for receive path */
INLINE BCMFASTPATH 
int32 dhd_rxchainhandler(void *p, struct sk_buff *skb)
{
	dhd_pub_t *dhdp;
	wl_pktc_tbl_t *pt;
	unsigned long dev_xmit;
	dhdp = (dhd_pub_t *)p;

	if (PKTISCHAINED(skb)) {
		struct ether_header *eh = (struct ether_header *)PKTDATA(dhdp->osh, skb);
		pt = (wl_pktc_tbl_t *)dhd_pktc_req(PKTC_TBL_GET_BY_DA, (unsigned long)(eh->ether_dhost), 0, 0);
		if (pt && pt->tx_dev != NULL) {

#if defined(CONFIG_BCM_FC_BASED_WFD)
			if(pt->tx_dev->priv_flags & (IFF_BCM_WLANDEV) )
			{ /* wlX not support handle CHAIN XMIT yet ..*/
  			     return (BCME_ERROR);
			}
#endif            
			if (pt->tx_dev->netdev_ops == NULL)
				return (BCME_ERROR);
			dev_xmit = (unsigned long)(pt->tx_dev->netdev_ops->ndo_start_xmit);
			if (dev_xmit) {
				dhdp->rx_enet_cnt++;
				pt->hits ++;
#if defined(BCM_WFD) && defined(CONFIG_BCM_PON)
				if (inject_to_fastpath &&  netdev_path_get_hw_port_type(pt->tx_dev)!= BLOG_WLANPHY )
					/* call to registered fastpath callback */
					send_packet_to_upper_layer(skb);
				else
#endif
					/* call enet xmit directly */
					((HardStartXmitFuncP)dev_xmit)(skb, pt->tx_dev);

				return (BCME_OK);
			}
		}
	}
	return (BCME_ERROR);
}

/** Add pktc dump output to a buffer */
void
dhd_pktc_dump(void *p, void *buf)
{
	struct bcmstrbuf *strbuf = (struct bcmstrbuf *)buf; 
	dhd_pub_t *dhdp = (dhd_pub_t *)p;

        bcm_bprintf(strbuf, "\n------------------------------------------\n");
	dhd_pktc_req(PKTC_TBL_DUMP, (unsigned long)strbuf, 0, 0);

	bcm_bprintf(strbuf, "\npktc: %s    pktcbnd: %d\n", dhdp->pktc ? "enabled" : "disabled", dhdp->pktcbnd);
	bcm_bprintf(strbuf, "rx_enet_cnt %lu rx_fcache_cnt %lu rx_linux_cnt %lu\n",
		dhdp->rx_enet_cnt, dhdp->rx_fcache_cnt, dhdp->rx_linux_cnt);
#ifdef DSLCPE
	bcm_bprintf(strbuf, "forward_cnt %lu mcast_forward_cnt %lu\n",
		dhdp->forward_cnt, dhdp->mcast_forward_cnt);
#endif
	bcm_bprintf(strbuf, "cur_pktccnt %lu max_pktccnt %lu\n",
		dhdp->cur_pktccnt, dhdp->max_pktccnt);

        return;
}


#endif /* PKTC_TBL */
