/*
 * DHD WFD header file
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_wfd.h $
 */

/*
 * wfd functions for DHD driver.
 */
#ifndef __DHD_WFD_H__
#define __DHD_WFD_H__

#if defined(BCM_WFD)

#include <wfd_dev.h>
#include <wlan_shared_defs.h>

extern dhd_pub_t *g_dhd_info[];

#if defined(DSLCPE) && defined(BCM_DHD_RUNNER)
extern int g_multicast_priority;
#endif /* DSLCPE && BCM_DHD_RUNNER */

int dhd_handle_wfd_blog(dhd_pub_t *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);
int dhd_wfd_bind(struct net_device *net, unsigned int unit);
void dhd_wfd_unbind(int wfd_idx);

void dhd_wfd_invoke_func(int processor_id, void (*func)(struct dhd_bus *bus));

extern dhd_pub_t *dhd_dev_get_dhdpub(struct net_device *dev);
extern int dhd_dev_get_ifidx(struct net_device *dev);
#ifdef DSLCPE
extern void dhd_if_inc_txpkt_mcnt(dhd_pub_t *dhdp, int ifidx, void *pkt);
#endif
extern void dhd_if_inc_txpkt_cnt(dhd_pub_t *dhdp, int ifidx, void *pkt);
extern void dhd_if_inc_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx);

#ifdef DHD_WMF
extern dhd_wmf_t* dhd_wmf_conf(dhd_pub_t *dhdp, uint32 idx);
#endif

extern void dhd_wfd_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);
extern int dhd_wfd_registerdevice(int wfd_idx, struct net_device *dev);
extern int dhd_wfd_unregisterdevice(int wfd_idx, struct net_device *dev);

#endif /* BCM_WFD */

#endif /* __DHD_WFD_H__ */
