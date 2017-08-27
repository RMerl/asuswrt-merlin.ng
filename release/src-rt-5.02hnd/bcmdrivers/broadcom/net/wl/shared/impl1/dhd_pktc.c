/*
 * Broadcom Dongle Host Driver (DHD), Linux-specific network interface
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * $Id: dhd_pktc.c $
 */

#if defined(PKTC_TBL)

#include <typedefs.h>
#include <osl.h>

#include <linux/skbuff.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <wl_pktc.h>

wl_pktc_tbl_t pktc_tbl[CHAIN_ENTRY_NUM];
pktc_handle_t pktc_wldev[WLAN_DEVICE_MAX];

wl_pktc_tbl_t *dhd_pktc_attach(void *p)
{
	dhd_pub_t *dhdp; 
	wl_pktc_tbl_t *tbl;

	dhdp = (dhd_pub_t *)p;

	dhd_pktc_req_hook = dhd_pktc_req;
#if defined(BCM_BLOG)
	dhd_pktc_del_hook = dhd_pktc_del;
#endif /* BCM_BLOG */

	tbl = (wl_pktc_tbl_t *)dhd_pktc_req(PKTC_TBL_GET_START_ADDRESS, 0, 0, 0);

	dhdp->pktc = 1;	/* by default, rx pktc is enabled */
	dhdp->pktcbnd = 256; /*DHD_PKT_CTF_MAX_CHAIN_LEN*/ // rx chaining bound

	return tbl;
}

/* for packet chaining */
unsigned long dhd_pktc_req( int req_id, unsigned long param0, unsigned long param1, unsigned long param2 )
{
	int i;
	struct bcmstrbuf *strbuf;
	wl_pktc_tbl_t *pt;

	switch (req_id) {
	case PKTC_TBL_GET_BY_DA:
            /* param0 is DA */
            return (PKTC_TBL_FN_LOOKUP(pktc_tbl, (uint8_t*)param0));

	case PKTC_TBL_GET_START_ADDRESS:
            return (unsigned long)(&pktc_tbl[0]);

	case PKTC_TBL_GET_BY_IDX:
		/* param0 is pktc chain table index */
		if (param0 >= CHAIN_ENTRY_NUM) {
			printk("chain idx is out of range! (%ld)\n", param0);
			return 0;
		}
		if (!(pktc_tbl[param0].in_use) || !(pktc_tbl[param0].wl_handle)) {
			printk("Error : chain idx %ld is not in use or invalid handle 0x%lx\n",
				param0, pktc_tbl[param0].wl_handle);
			return 0;
		}
		return (unsigned long)(&pktc_tbl[param0]);

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
		pt = (wl_pktc_tbl_t *)PKTC_TBL_FN_UPDATE(pktc_tbl, (uint8_t *)param0,
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
			PKTC_TBL_FN_CLEAR(pktc_tbl, (uint8 *)param0);
			return PKTC_INVALID_CHAIN_IDX;
		}

		return pt->idx; /* return chain index */
		
	case PKTC_TBL_DELETE:
		PKTC_TBL_FN_CLEAR(pktc_tbl, (uint8 *)param0);
		return 0;

	case PKTC_TBL_DUMP:
		strbuf = (struct bcmstrbuf *)param0;
		bcm_bprintf(strbuf, "\npktc dump: (rx path)\n");
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (pktc_tbl[i].in_use) 
			{
				bcm_bprintf(strbuf, "[%02d] %02x:%02x:%02x:%02x:%02x:%02x, dev=%s, hits=%d\n",
				pktc_tbl[i].idx,
				pktc_tbl[i].ea.octet[0],
				pktc_tbl[i].ea.octet[1],
				pktc_tbl[i].ea.octet[2],
				pktc_tbl[i].ea.octet[3],
				pktc_tbl[i].ea.octet[4],
				pktc_tbl[i].ea.octet[5],
				(pktc_tbl[i].tx_dev == NULL) ? "NULL" : pktc_tbl[i].tx_dev->name,
				pktc_tbl[i].hits);
			}
		}
		return 0;

	case PKTC_TBL_FLUSH:
		printk("pktc_tbl flush!\n");
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (pktc_tbl[i].in_use)
				memset(&pktc_tbl[i], 0, sizeof(wl_pktc_tbl_t));
		}
		return 0;

	default:
		return 0;
	}
	return 0;
}

void dhd_pktc_del(unsigned long addr) 
{
    dhd_pktc_req(PKTC_TBL_DELETE, addr, 0, 0);
}

void pktc_tbl_clear_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		memset(pt, 0, sizeof(wl_pktc_tbl_t));
		return;
	}
	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		memset(pt, 0, sizeof(wl_pktc_tbl_t));
		return;
	}
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
			if (pt->tx_dev->netdev_ops == NULL)
				return (BCME_ERROR);
			dev_xmit = (unsigned long)(pt->tx_dev->netdev_ops->ndo_start_xmit);
			if (dev_xmit) {
				dhdp->rx_enet_cnt++;
				pt->hits ++;
#if defined(BCM_WFD) && (defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858))
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
	bcm_bprintf(strbuf, "forward_cnt %lu mcast_forward_cnt %lu\n",
		dhdp->forward_cnt, dhdp->mcast_forward_cnt);
	bcm_bprintf(strbuf, "cur_pktccnt %lu max_pktccnt %lu\n",
		dhdp->cur_pktccnt, dhdp->max_pktccnt);

        return;
}


#endif /* PKTC_TBL */
