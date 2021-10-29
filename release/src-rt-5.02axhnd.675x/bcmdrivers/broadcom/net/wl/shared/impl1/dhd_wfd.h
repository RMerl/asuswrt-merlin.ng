/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
*/

/*
 * wfd functions for DHD driver.
 */
#ifndef __DHD_WFD_H__
#define __DHD_WFD_H__

#if defined(BCM_WFD)

#include <wfd_dev.h>
#include <wlan_shared_defs.h>
#include <dhd.h>

/* DHD_PKT_XXX_FKB_FLOW_UNHANDLED will be defined only in Kudu (dhd.h) */
#if !defined(DHD_PKT_GET_FKB_FLOW_UNHANDLED)
#define DHD_PKT_GET_FKB_FLOW_UNHANDLED(pkt)       FALSE
#endif

#if !defined(DHD_PKT_SET_FKB_FLOW_UNHANDLED)
#define DHD_PKT_SET_FKB_FLOW_UNHANDLED(pkt)       ({ BCM_REFERENCE(pkt); })
#endif

#if !defined(DHD_PKT_CLR_FKB_FLOW_UNHANDLED)
#define DHD_PKT_CLR_FKB_FLOW_UNHANDLED(pkt)       ({ BCM_REFERENCE(pkt); })
#endif

extern dhd_pub_t *g_dhd_info[];

int dhd_handle_wfd_blog(dhd_pub_t *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);
int dhd_wfd_bind(struct net_device *net, unsigned int unit);
#ifdef BCM_PKTFWD
void dhd_wfd_unbind(int wfd_idx, int unit);
#else
void dhd_wfd_unbind(int wfd_idx);
#endif /* BCM_PKTFWD */

void dhd_wfd_invoke_func(int processor_id, void (*func)(struct dhd_bus *bus));

extern dhd_pub_t *dhd_dev_get_dhdpub(struct net_device *dev);
extern int dhd_dev_get_ifidx(struct net_device *dev);
extern void dhd_if_inc_txpkt_cnt(dhd_pub_t *dhdp, int ifidx, void *pkt);
extern void dhd_if_inc_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx);

#ifdef DHD_WMF
extern dhd_wmf_t* dhd_wmf_conf(dhd_pub_t *dhdp, uint32 idx);
#endif

extern void dhd_wfd_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);
extern void dhd_wfd_clear_dump(dhd_pub_t *dhdp);

extern int dhd_wfd_registerdevice(int wfd_idx, struct net_device *dev);
extern int dhd_wfd_unregisterdevice(int wfd_idx, struct net_device *dev);

#endif /* BCM_WFD */

#if (defined(DSLCPE) && defined(BCM_DHD_RUNNER)) || defined(BCM_NBUFF_WLMCAST)
extern int g_multicast_priority;
#endif /* DSLCPE && BCM_DHD_RUNNER || BCM_NBUFF_WLMCAST*/

#endif /* __DHD_WFD_H__ */
