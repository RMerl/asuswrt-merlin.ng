/**
 * Network Offload Engine
 * @file
 * @brief
 * Enables in-dongle support for LWIP API while host processor sleeps. Used to
 * provide offload functionality for TCP and UDP.
 *
 * Currently supports IPv4 only.
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
 * $Id: wl_nwoe.c 708017 2017-06-29 14:11:45Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmarp.h>
#include <proto/bcmudp.h>
#include <proto/vlan.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <dngl_stats.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>

#include <wl_nwoe.h>

#include <lwip/opt.h>
#include <lwip/init.h>
#include <lwip/err.h>
#include <lwip/memp.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>
#include <lwip/ip.h>
#include <lwip/stats.h>
#include <lwip/netif.h>
#include <netif/etharp.h>
#include <lwip/ip_frag.h>
#include <lwip/tcp.h>
#include <lwip/tcp_impl.h>
#include <lwip/udp.h>
#include <lwip/igmp.h>

/* wlc_pub_t struct access macros */
#define WLCUNIT(nwoei)	((nwoei)->wlc->pub->unit)
#define WLCOSH(nwoei)	((nwoei)->wlc->osh)

#define IFNAME0			'e'
#define IFNAME1			'n'
#define IF_MTU_DEFAULT		1518
#define LWIP_TMR_INTERVAL       100  /* The LWIP timer interval in milliseconds. */

osl_t *wl_lwip_osh = NULL;

static int wl_lwip_init(wl_nwoe_info_t *nwoei);
static void wl_lwip_timer(void *arg);
static err_t wl_lwip_if_init(struct netif *netif);
static void lwip_if_config_set(wl_nwoe_info_t *nwoei);

#if LWIP_ARP
static uint32 ticks_arp = 0;
#endif // endif
#if LWIP_TCP
static uint32 ticks_tcp = 0;
#endif // endif
#if IP_REASSEMBLY
static uint32 ticks_reass = 0;
#endif // endif
#if LWIP_DHCP
static uint32 ticks_dhcp_fine = 0;
static uint32 ticks_dhcp_coarse = 0;
#endif // endif
#if LWIP_IGMP
static uint32 ticks_igmp = 0;
#endif // endif

struct wl_nwoe_info {
	wlc_info_t		*wlc;
	struct wlc_if           *wlcif;
	struct netif		*netif;
	struct wl_timer         *timer;
	ip_addr_t		host_ip;
	ip_addr_t		host_netmask;
	ip_addr_t		host_gateway;
	bool			enabled;
	bool                    init;
	uint32                  ol_config; /* bitmask set by IOVAR for configuring components */
};

/* IOVar table */
enum {
	IOV_NWOE_OL,
	IOV_NWOE_IFCONFIG
};

static const bcm_iovar_t nwoe_iovars[] = {
	{"nwoe_ol", IOV_NWOE_OL,
	(0), IOVT_UINT32, 0
	},
	{"nwoe_ifconfig", IOV_NWOE_IFCONFIG,
	(0), IOVT_BUFFER, sizeof(nwoe_ifconfig_t),
	},
	{NULL, 0, 0, 0, 0 }
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

uint32
wl_lwip_rand(struct netif *n)
{
	wl_nwoe_info_t *nwoei = n->state;
	uint32 r = 0;
	if (nwoei != NULL) {
		wlc_getrand(nwoei->wlc, (uint8*)&r, sizeof(r));
	}
	return r;
}

static err_t
wl_lwip_if_output(struct netif *netif, struct pbuf *p, ip_addr_t *ipaddr)
{
	LWIP_DEBUGF(NETIF_DEBUG, ("%s: send to ", __FUNCTION__));
	ip_addr_debug_print(NETIF_DEBUG, ipaddr);
	LWIP_DEBUGF(NETIF_DEBUG, ("\n"));

	if (!netif_is_up(netif))
		return ERR_IF;

	return etharp_output(netif, p, ipaddr);
}

static err_t
wl_lwip_linkoutput(struct netif *netif, struct pbuf *p)
{
	void *pkt;
	struct pbuf *q;
	wl_nwoe_info_t *nwoei = netif->state;
	unsigned char *msg, *msgptr;
	int msglen;

	msglen = p->tot_len;

	if (msglen == 0 || (pkt = PKTGET(WLCOSH(nwoei), msglen, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: alloc failed; dropped\n",
		          WLCUNIT(nwoei), __FUNCTION__));
		LINK_STATS_INC(link.drop);
		return ERR_MEM;
	}

	/* Flatten pbuf into native OSL packet */
	msg = (unsigned char *)PKTDATA(WLCOSH(nwoei), pkt);
	msgptr = msg;

	for (q = p; q != NULL; q = q->next) {
		memcpy(msgptr, q->payload, q->len);
		msgptr += q->len;
	}

	ASSERT(msgptr == msg + msglen);

	PKTSETLEN(WLCOSH(nwoei), pkt, msglen);

	if (wlc_sendpkt(nwoei->wlc, pkt, nwoei->wlcif)) {
		LINK_STATS_INC(link.drop);
		LWIP_DEBUGF(NETIF_DEBUG, ("%s: dropped wl packet (%d bytes)\n",
		                          __FUNCTION__, msglen));
	}

	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

static void
wl_lwip_input_free(void *cb_arg, void *cb_mem)
{
	osl_t *osh = cb_mem;
	/* LWIP_DEBUGF(NETIF_DEBUG, ("%s: pkt=%p\n", __FUNCTION__, pkt)); */
	PKTFREE(osh, cb_arg, FALSE);
}

int
wl_nwoe_recv_proc(wl_nwoe_info_t *nwoei, osl_t *osh, void *pkt)
{
	struct netif *netif = nwoei->netif;
	struct pbuf *p;
	unsigned char *msg;
	int msglen;
	struct eth_hdr *ethhdr;
	int err = -1;

	if (!nwoei->enabled)
		return NWOE_PKT_RETURNED;

	LWIP_DEBUGF(NETIF_DEBUG, ("wl%d: %s: pkt=%p len=%d\n",
	                          WLCUNIT(nwoei), __FUNCTION__, (void *)pkt, PKTLEN(osh, pkt)));

	if (!netif_is_up(netif)) {
		LWIP_DEBUGF(NETIF_DEBUG, ("wl%d: %s: interface down dropping pkt\n",
		                          WLCUNIT(nwoei), __FUNCTION__));
		PKTFREE(osh, pkt, FALSE);
		pkt = NULL;
		return NWOE_PKT_CONSUMED;
	}

	msg = PKTDATA(osh, pkt);
	msglen = PKTLEN(osh, pkt);

	/* Not yet handling receive packets that come in multiple pieces */
	ASSERT(PKTNEXT(osh, pkt) == NULL);

	/* Allocate a reference pbuf pointing to the packet payload */
	if ((p = pbuf_alloc(PBUF_RAW, (u16_t)msglen, PBUF_REF)) == NULL) {
		WL_ERROR(("wl%d: %s: alloc failed; dropped\n",
		          WLCUNIT(nwoei), __FUNCTION__));
		PKTFREE(osh, pkt, FALSE);
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
		return NWOE_PKT_CONSUMED;
	}

	p->payload = p->payload_start = msg;
	p->free_cb = wl_lwip_input_free;
	p->free_cb_mem = osh;
	p->free_cb_arg = pkt;

	LINK_STATS_INC(link.recv);

	/* Determine protocol and handle packet */
	ethhdr = p->payload;
	switch (htons(ethhdr->type)) {
	case ETHTYPE_IP:
	case ETHTYPE_ARP:
		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("wl%d: %s: ethernet input error\n",
			                          WLCUNIT(nwoei), __FUNCTION__));
			pbuf_free(p);
			p = NULL;
		}
		else
			err = NWOE_PKT_CONSUMED;
		break;
	default:
		pbuf_free(p);
		p = NULL;
		break;
	}

	return err;
}

static void
lwip_if_config_set(wl_nwoe_info_t *nwoei)
{
	nwoei->enabled = (bool)(nwoei->ol_config & NWOE_OL_ENABLE);
	if (nwoei->enabled)
		netif_set_up(nwoei->netif);
	else
		netif_set_down(nwoei->netif);

	/* flag to route frames into lwip */
	nwoei->wlc->pub->_nwoe = nwoei->enabled;
}

static void
wl_lwip_timer(void *arg)
{
#if LWIP_ARP
	/* call ARP timer every 5 seconds */
	if (++ticks_arp == (ARP_TMR_INTERVAL / LWIP_TMR_INTERVAL)) {
		etharp_tmr();
		ticks_arp = 0;
	}
#endif /* LWIP_ARP */

#if LWIP_TCP
	if (++ticks_tcp == (TCP_TMR_INTERVAL / LWIP_TMR_INTERVAL)) {
		tcp_tmr();
		ticks_tcp = 0;
	}
#endif /* LWIP_TCP */

#if IP_REASSEMBLY
	/* call IP fragmentation/reassembly every 1s */
	if (++ticks_reass == (IP_TMR_INTERVAL / LWIP_TMR_INTERVAL)) {
		ip_reass_tmr();
		ticks_reass = 0;
	}
#endif /* IP_REASSEMBLY */

#if LWIP_DHCP
	/* call DHCP fine timer every 500ms */
	if (++ticks_dhcp_fine == (DHCP_FINE_TIMER_MSECS / LWIP_TMR_INTERVAL)) {
		dhcp_fine_tmr();
		ticks_dhcp_fine = 0;
	}

	/* call DHCP coarse timer every 60s */
	if (++ticks_dhcp_coarse == (DHCP_COARSE_TIMER_MSECS / LWIP_TMR_INTERVAL)) {
		dhcp_coarse_tmr();
		ticks_dhcp_coarse = 0;
	}
#endif /* LWIP_DHCP */

#if LWIP_IGMP
	if (++ticks_igmp == (IGMP_TMR_INTERVAL / LWIP_TMR_INTERVAL)) {
		igmp_tmr();
	}
#endif /* LWIP_IGMP */
}

static int
wl_lwip_init(wl_nwoe_info_t *nwoei)
{
	wlc_info_t *wlc = nwoei->wlc;
	err_t err;

	/* OSH must be set before initialising lwip */
	wl_lwip_osh = WLCOSH(nwoei);
	err = lwip_init();
	if (err != ERR_OK) {
		WL_ERROR(("wl%d: %s: lwip initialization failed\n",
		          WLCUNIT(nwoei), __FUNCTION__));
		return -1;
	}

	/* Start the LWIP timer */
	if ((nwoei->timer =
	     wl_init_timer(wlc->wl, wl_lwip_timer, nwoei, "lwip")) == NULL) {
		WL_ERROR(("wl%d: %s: lwip timer init failed\n",
		          WLCUNIT(nwoei), __FUNCTION__));
		return -1;
	}

	wl_add_timer(wlc->wl, nwoei->timer, LWIP_TMR_INTERVAL, TRUE);

	/* Initialise lwip netif */
	nwoei->netif = MALLOC(WLCOSH(nwoei), sizeof(struct netif));
	if (!nwoei->netif) {
		WL_ERROR(("wl%d: %s: netif MALLOC failed; total mallocs %d bytes\n",
		          WLCUNIT(nwoei), __FUNCTION__, MALLOCED(WLCOSH(nwoei))));
		MFREE(WLCOSH(nwoei), nwoei, sizeof(wl_nwoe_info_t));
		return -1;
	}

	/* Add a default network interface */
	netif_add(nwoei->netif, NULL, NULL, NULL, nwoei, wl_lwip_if_init, ethernet_input);
	netif_set_default(nwoei->netif);

	nwoei->init = TRUE;
	return 0;
}

static err_t
wl_lwip_if_init(struct netif *netif)
{
	wl_nwoe_info_t *nwoei = netif->state;

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = wl_lwip_if_output;
	netif->linkoutput = wl_lwip_linkoutput;

	/* set MAC hardware address */
	netif->hwaddr_len = ETHER_ADDR_LEN;
	memcpy(netif->hwaddr, &nwoei->wlc->pub->cur_etheraddr, ETHER_ADDR_LEN);

	/* set maximum transfer unit */
	netif->mtu = IF_MTU_DEFAULT;

	netif->flags |= NETIF_FLAG_ETHERNET;
	netif->flags |= NETIF_FLAG_BROADCAST;
	netif->flags |= NETIF_FLAG_ETHARP;
	netif->flags |= NETIF_FLAG_IGMP;

	return ERR_OK;
}

static int
nwoe_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_nwoe_info_t *nwoei = hdl;
	int err = 0;
	int32 int_val = 0;

	WL_INFORM(("wl%d: %s\n", WLCUNIT(nwoei), __FUNCTION__));

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {

	case IOV_GVAL(IOV_NWOE_OL):
		bcopy(&nwoei->ol_config, a, sizeof(nwoei->ol_config));
		break;

	case IOV_SVAL(IOV_NWOE_OL):
		if (!nwoei->init)
			return BCME_NOTREADY;

		bcopy(a, &nwoei->ol_config, sizeof(nwoei->ol_config));

		lwip_if_config_set(nwoei);
		break;

	case IOV_SVAL(IOV_NWOE_IFCONFIG):
	{
		nwoe_ifconfig_t config;

		if (alen < sizeof(nwoe_ifconfig_t))
			return BCME_BUFTOOSHORT;

		if (!nwoei->init) {
			if (wl_lwip_init(nwoei) != 0)
				return BCME_NOMEM;
		}

		/* Copy into config, as there is no guarantee that a is aligned */
		bcopy(a, &config, sizeof(nwoe_ifconfig_t));

		nwoei->host_ip.addr = config.ipaddr;
		nwoei->host_netmask.addr = config.ipaddr_netmask;
		nwoei->host_gateway.addr = config.ipaddr_gateway;

		netif_set_addr(nwoei->netif, &nwoei->host_ip,
		               &nwoei->host_netmask, &nwoei->host_gateway);
		break;
	}

	case IOV_GVAL(IOV_NWOE_IFCONFIG):
	{
		nwoe_ifconfig_t config;

		if (alen < sizeof(nwoe_ifconfig_t))
			return BCME_BUFTOOSHORT;

		if (!nwoei->init)
			return BCME_NOTREADY;

		config.ipaddr = nwoei->host_ip.addr;
		config.ipaddr_netmask = nwoei->host_netmask.addr;
		config.ipaddr_gateway = nwoei->host_gateway.addr;
		bcopy(&config, a, sizeof(nwoe_ifconfig_t));
		break;
	}

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

wl_nwoe_info_t *
BCMATTACHFN(wl_nwoe_attach)(wlc_info_t *wlc)
{
	wl_nwoe_info_t *nwoei;

	nwoei = MALLOCZ(wlc->osh, sizeof(wl_nwoe_info_t));
	if (!nwoei) {
		WL_ERROR(("wl%d: %s: nwoei MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	nwoei->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, nwoe_iovars, "nwoe", nwoei, nwoe_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	return nwoei;
}

void
BCMATTACHFN(wl_nwoe_detach)(wl_nwoe_info_t *nwoei)
{
	if (!nwoei)
		return;

	wlc_info_t *wlc = nwoei->wlc;

	WL_INFORM(("wl%d: %s\n", WLCUNIT(nwoei), __FUNCTION__));

	wlc_module_unregister(nwoei->wlc->pub, "nwoe", nwoei);

	/* Stop the lwip timer */
	if (nwoei->timer != NULL)
		wl_free_timer(wlc->wl, nwoei->timer);

	/* Remove and free the default lwip network interface */
	if (nwoei->netif != NULL) {
		netif_remove(nwoei->netif);
		MFREE(WLCOSH(nwoei), nwoei->netif, sizeof(struct netif));
	}

	/* Cleanup module info structure */
	MFREE(WLCOSH(nwoei), nwoei, sizeof(wl_nwoe_info_t));
}
