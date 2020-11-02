/*
 * TCP Keep-Alive and Ping (v4 and v6) offloads
 *
 * Supported protocol families: keep-alive: IPV4.
 *				ping: IPV4 and IPV6
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
 * $Id: wl_tcpkeepol.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * XXX Apple specific
 *
 * @file
 * @brief
 * Used in 'NIC+offload' (Apple) builds. Goal is to increase power saving of the host by waking up
 * the host less. Firmware should:
 *     - respond to ICMP PING requests (up to full MTU packet size)
 *     - send out periodic keep alive packets to keep TCP connections alive
 * @brief
 * Twiki: [OffloadsPhase2]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
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
#include <proto/bcmipv6.h>
#include <proto/bcmtcp.h>
#include <proto/bcmicmp.h>
#include <proto/bcmipv6.h>
#include <proto/bcmudp.h>
#include <proto/vlan.h>
#include <bcmendian.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_export.h>
#ifdef BCM_OL_DEV
#include <wlc_types.h>
#include <bcm_ol_msg.h>
#include <wlc_hw_priv.h>
#include <wlc_dngl_ol.h>
#include <wlc_wowlol.h>
#else
#include <wlc_bsscfg.h>
#endif // endif
#include <wl_tcpkoe.h>

#if defined(BCMDBG) || defined(WLMSG_INFORM)
#define WL_MSGBUF
#endif // endif

uint16 ip_cksum(uint16 *val16, uint32 count, int j, uint32 sum);
uint32 ip_cksum_partial(uint8 *val8, uint32 count, int j);
static int icmp_send_echo_reply(wl_icmp_info_t *icmpi, void *sdu, bool snap, uint16 iplen);
static int icmpv6_send_echo_reply(wl_icmp_info_t *icmpi, void *sdu, bool snap, uint16 iplen,
    uint32 psum);
void wl_tcp_setup_keep_pkt(wl_tcp_keep_info_t *tcpkeepi,
    wl_mtcpkeep_alive_conn_pkt_t *tcp_keepalive_connection, void *keep_pkt);
static int tcp_keep_send_pkt(wl_tcp_keep_info_t *tcpkeepi, void *keep_pkt);
static void tcp_keepalive_timer_cb(void *arg);
void wl_tcp_keepalive_event_handler(wl_tcp_keep_info_t *tcpkeepi, uint32 event, void *event_data);
void wl_icmp_event_handler(wl_icmp_info_t *icmpi, uint32 event, void *event_data);

static void tcp_wowl_assert_pme(wl_tcp_keep_info_t *tcp_keep_info, uint32 wake_reason, void *sdu,
    bool malloced);
static void tcp_keepalive_make_reset(wl_tcp_keep_info_t *info, void *keep_pkt, void *reset_pkt);

/** used to help with checksum calculation of TCP frames */
struct tcp_pseudo_hdr {
	uint8   src_ip[IPV4_ADDR_LEN];  /* Source IP Address */
	uint8   dst_ip[IPV4_ADDR_LEN];  /* Destination IP Address */
	uint8	zero;
	uint8	prot;
	uint16	tcp_size;
};

/** ICMP private info structure */
struct wl_icmp_info {
	wlc_info_t		*wlc;		/* Pointer back to wlc structure */
#ifdef BCM_OL_DEV
	bool	icmp_enabled;
#endif // endif
};

struct ipv6_pseudo_hdr
{
	uint8  saddr[16];
	uint8  daddr[16];
	uint16 payload_len;
	uint16 next_hdr;
};

/**
 * TCP keep-alive private info structure.
 * Connection info and parameters for doing TCP keep-alive.
 */
struct tcp_keep_conn_info_t {
	uint8   ether_dhost[ETHER_ADDR_LEN];
	uint8   ether_shost[ETHER_ADDR_LEN];
	uint8   src_ip[IPV4_ADDR_LEN];
	uint8   dst_ip[IPV4_ADDR_LEN];
	uint16  src_port;       /* Source Port Address */
	uint16  dst_port;       /* Destination Port Address */
	uint32  seq_num;        /* TCP Sequence Number */
	uint32  ack_num;        /* TCP Sequence Number */
	uint16  tcpwin;         /* TCP window */
};

#define	TCP_TIMER_RES	1000					/* ms resolution */
#define	TCP_KEEP_ALIVE_INT_DEF		7200 * TCP_TIMER_RES	/* number of seconds * resolution */
#define	TCP_KEEP_ALIVE_RETRY_INT_DEF	75 * TCP_TIMER_RES	/* number of seconds * resolution */
#define	TCP_KEEP_ALIVE_RETRY	10

struct tcp_keep_info {

	wlc_info_t		*wlc;		/* Pointer back to wlc structure */

	int		num_conn_recs;	/* num connections have we been programmed with so far */
	/* for now we only keep one connection record. In the future this
	 * may turn into an array of 4 (this upper bound was provided
	 * by upper management.
	 */
	struct	tcp_keep_conn_info_t	tcp_keep_conn_info;

	uint32		keep_alive_interval;		/* default is 2 hours */
	uint32		keep_alive_retry_interval;	/* default is 75 seconds */
	uint16		keep_alive_retry_cnt;		/* default is 9 */
	uint16		keep_alive_retry_cnt_left;	/* retries left before we fail it */
	/* for now some test data */
	wl_mtcpkeep_alive_conn_pkt_t	tcp_keepalive_connection;
	void		*keep_pkt;	/* this is the TCP packet that we will send out */
	void		*wake_pkt;	/* this is the TCP packet that send to host on wakeup */
	bool		tcp_wowl_pme_asserted;
	int 		tcp_val1;
	int 		tcp_val2;
	int 		tcp_keep_enabled;
	struct wl_timer *tcp_keepalive_timer;
};

#ifndef BCM_OL_DEV
/* forward declarations */
static int tcp_keepalive_doiovar(void *hdl, const bcm_iovar_t	*vi, uint32 actionid,
                       const char *name, void *p, uint plen, void *a, int alen,
                       int vsize, struct wlc_if *wlcif);
#endif // endif
/*
 * IPv4 handler. 'ph' points to protocol specific header,
 * for example, it points to UDP header if it is UDP packet.
 */

/** wlc_pub_t struct access macros */
#define WLCUNITT(icmpi)	((icmpi)->wlc->pub->unit)
#define WLCUNITK(tcpkeepi)	((tcpkeepi)->wlc->pub->unit)
#define WLCOSHH(icmpi)	((icmpi)->wlc->osh)
#define WLCOSHK(tcpkeepi)	((tcpkeepi)->wlc->osh)

/* special values */
/* 802.3 llc/snap header */
static const uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

/* IOVar table */
enum {
	IOV_ICMP_OL		/* Flags for enabling/disabling ICMP offload sub-features */
};

static const bcm_iovar_t icmp_iovars[] = {
	{"icmp_ol", IOV_ICMP_OL,
	(0), IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/**
 * Initialize ICMP private context.
 * Returns a pointer to the ICMP private context, NULL on failure.
 */
wl_icmp_info_t *
BCMATTACHFN(wl_icmp_attach)(wlc_info_t *wlc)
{
	wl_icmp_info_t *icmpi;

	/* allocate arp private info struct */
	icmpi = MALLOCZ(wlc->osh, sizeof(wl_icmp_info_t));
	if (!icmpi) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init icmp private info struct */
	icmpi->icmp_enabled = 0;
	icmpi->wlc = wlc;

#ifndef BCM_OL_DEV
	/* register module */
	if (wlc_module_register(wlc->pub, icmp_iovars, "icmp", icmpi, icmp_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return NULL;
	}
#endif // endif

	return icmpi;
}

/**
 * Malloced is true when we are creating a packet to be sent up to the host. This should only be
 * a reset packet and has a fixed length. If in the future we are creating different sized
 * packets, then we may want to change this function to accept a length as a param as well.
 */
static void
tcp_wowl_assert_pme(wl_tcp_keep_info_t *tcp_keep_info, uint32 wake_reason, void *sdu, bool malloced)
{
	if (tcp_keep_info->wlc->wlc_dngl_ol) {
		switch (wake_reason) {
		case WL_WOWL_TCPKEEP_DATA:
			WL_ERROR(("%s: TCP KEEPALIVE Waking host due to Rx'ed Data\n",
				__FUNCTION__));
			break;
		case WL_WOWL_TCPKEEP_TIME:
			WL_ERROR(("%s: TCP KEEPALIVE Waking host due to Timeout\n",
				__FUNCTION__));
			break;
		default:
			WL_ERROR(("%s: TCP KEEPALIVE Waking host due to unknown reason 0x%x\n",
				__FUNCTION__, wake_reason));
			break;
		}
		if (sdu == NULL) {
			wlc_wowl_ol_wake_host(
				tcp_keep_info->wlc->wlc_dngl_ol->wowl_ol,
				NULL, 0, PKTDATA(WLCOSHK(tcp_keep_info), sdu),
				0, wake_reason);
		} else {
			uint32 pktlen;

			if (malloced == FALSE) {
				pktlen = PKTLEN(WLCOSHK(tcp_keep_info), sdu);
			} else {
				/* no snap headers here */
				pktlen = ETHER_HDR_LEN + sizeof(struct ipv4_hdr) +
				    sizeof(struct bcmtcp_hdr);
			}
			if (pktlen > ETHER_MAX_DATA)
				pktlen = ETHER_MAX_DATA;
			if (malloced == FALSE) {
				wlc_wowl_ol_wake_host(
					tcp_keep_info->wlc->wlc_dngl_ol->wowl_ol,
					NULL, 0, PKTDATA(WLCOSHK(tcp_keep_info), sdu),
					pktlen, wake_reason);
			} else {
				wlc_wowl_ol_wake_host(
					tcp_keep_info->wlc->wlc_dngl_ol->wowl_ol,
					NULL, 0, sdu, pktlen, wake_reason);
			}
		}
	} else {
		WL_ERROR(("%s: Unable to wake host, no wlc_dngl_ol handle\n", __FUNCTION__));
	}
}

static void
tcp_keepalive_make_reset(wl_tcp_keep_info_t *info, void *keep_pkt, void *reset_pkt)
{
	struct ipv4_hdr *ipv4_k, *ipv4_r;
	struct bcmtcp_hdr *tcp_k, *tcp_r;
	struct tcp_pseudo_hdr tcp_ps;
	uint32  psum;

	/* switch MAC addrs */
	bcopy(keep_pkt + ETHER_SRC_OFFSET, reset_pkt + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	bcopy(keep_pkt + ETHER_DEST_OFFSET, reset_pkt + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);

	/* now fix up IP portion */
	ipv4_k = (struct ipv4_hdr *)(keep_pkt + ETHER_HDR_LEN);
	ipv4_r = (struct ipv4_hdr *)(reset_pkt + ETHER_HDR_LEN);
	bcopy(ipv4_k->src_ip, ipv4_r->dst_ip, IPV4_ADDR_LEN);
	bcopy(ipv4_k->dst_ip, ipv4_r->src_ip, IPV4_ADDR_LEN);
	ipv4_r->hdr_chksum = 0;
	/* rest of IP header can be used as is */
	ipv4_r->hdr_chksum = (uint16) ip_cksum((uint16 *)ipv4_r,  IPV4_HLEN(ipv4_r), 0, 0);

	/* now fix up TCP */
	tcp_k = (struct bcmtcp_hdr *)(keep_pkt + sizeof(struct ether_header) +
	    sizeof(struct ipv4_hdr));
	tcp_r = (struct bcmtcp_hdr *)(reset_pkt + sizeof(struct ether_header) +
	    sizeof(struct ipv4_hdr));
	tcp_r->src_port = tcp_k->dst_port;
	tcp_r->dst_port = tcp_k->src_port;
	tcp_r->seq_num = tcp_k->ack_num;
	/* tcp_r->ack_num = hton32(ntoh32(tcp_k->seq_num) + 1); should be 0 if ACK is not set */
	tcp_r->ack_num = 0;
	tcp_r->chksum = 0;
	tcp_r->hdrlen_rsvd_flags = (TCP_FLAG_RST << 8) |
	    ((sizeof(struct bcmtcp_hdr) >> 2) << TCP_HDRLEN_SHIFT);

	/* now need to set up psuedo header for checksum */
	bcopy(ipv4_r->dst_ip, &tcp_ps.dst_ip, IPV4_ADDR_LEN);
	bcopy(ipv4_r->src_ip, &tcp_ps.src_ip, IPV4_ADDR_LEN);
	tcp_ps.zero = 0;
	tcp_ps.prot = ipv4_r->prot;
	tcp_ps.tcp_size = hton16(sizeof(struct bcmtcp_hdr));
	psum = ip_cksum_partial((uint8 *)&tcp_ps, sizeof(struct tcp_pseudo_hdr), 1);
	tcp_r->chksum = (uint16) ip_cksum((uint16 *)tcp_r, sizeof(struct bcmtcp_hdr), 0, psum);
}

/**
 * if we determine keep-alive has failed, we will send a RST packet to the host. We need to
 * generate this packet to send. We take the keep_pkt and copy it into a malloc memory and then
 * send this to the host.
 */
static void
tcp_keepalive_timer_cb(void *arg)
{
	wl_tcp_keep_info_t	*info;
	int i;
	void *new_pkt;
	int pkt_len;

	info = (wl_tcp_keep_info_t *)arg;

	if (info->tcp_keep_enabled == 0) {
		WL_ERROR(("%s: tcpkeep is disabled, don't renew timer\n", __FUNCTION__));
		return;
	}
	WL_ERROR(("%s: entered, retries left %d, retry val %d\n",
		__FUNCTION__, info->keep_alive_retry_cnt_left, info->keep_alive_retry_interval));
	if (info->keep_alive_retry_cnt_left == 0) {
		WL_ERROR(("%s: KEEP-ALIVE retry fail, wake host\n", __FUNCTION__));
		info->tcp_keep_enabled = 0;	/* we are waking up host */
		info->tcp_wowl_pme_asserted = TRUE;
		pkt_len = ETHER_HDR_LEN + sizeof(struct ipv4_hdr) +  sizeof(struct bcmtcp_hdr);
		new_pkt = MALLOC(WLCOSHK(info), pkt_len);

		if (new_pkt != NULL) {
			bcopy(info->keep_pkt, new_pkt,  ETHER_HDR_LEN +
			    sizeof(struct ipv4_hdr) + sizeof(struct bcmtcp_hdr));
			tcp_keepalive_make_reset(info, info->keep_pkt, new_pkt);
		} else {
			WL_ERROR(("%s: malloc failed\n", __FUNCTION__));
		}
		tcp_wowl_assert_pme(info, WL_WOWL_TCPKEEP_TIME, new_pkt, TRUE);
		return;
	}

	info->keep_alive_retry_cnt_left--;
	i = tcp_keep_send_pkt(info, info->keep_pkt);
	wl_add_timer(info->wlc->wl, info->tcp_keepalive_timer,
	    info->keep_alive_retry_interval, FALSE);
}

/**
 * Initialize ICMP private context.
 * Returns a pointer to the ICMP private context, NULL on failure.
 */
wl_tcp_keep_info_t *
BCMATTACHFN(wl_tcp_keep_attach)(wlc_info_t *wlc)
{
	wl_tcp_keep_info_t *info;

	/* Allocate packet filter private info struct. */
	info = MALLOCZ(wlc->osh, sizeof(wl_tcp_keep_info_t));
	if (info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Init packet filter private info struct. */
	info->tcp_val1 = 0;
	info->tcp_val2 = 0;
	info->wake_pkt = NULL;
	info->tcp_wowl_pme_asserted = FALSE;
	info->tcp_keep_enabled = 0;
	/* Allocate the timer */
	info->tcp_keepalive_timer = wl_init_timer(wlc->wl, tcp_keepalive_timer_cb,
	    info, "tcp keepalive timer");
	if (info->tcp_keepalive_timer == NULL) {
		WL_ERROR(("wl_tcp_keep_attach: could not allocate timer\n"));
		goto fail;
	}
	info->keep_pkt = MALLOC(wlc->osh, ETHER_HDR_LEN + sizeof(struct ipv4_hdr) +
	    sizeof(struct bcmtcp_hdr));
	if (info->keep_pkt == NULL) {
		goto fail;
	}
	info->num_conn_recs = 0;
	info->keep_alive_interval = TCP_KEEP_ALIVE_INT_DEF;
	info->keep_alive_retry_interval = TCP_KEEP_ALIVE_RETRY_INT_DEF;
	info->keep_alive_retry_cnt = TCP_KEEP_ALIVE_RETRY;

	info->wlc  = wlc;

#ifndef BCM_OL_DEV
	/* Register this module. */
	if (wlc_module_register(wlc->pub,
	                        tcp_keepalive_iovars,
	                        "tcp_keepalive",
	                        info,
	                        tcp_keepalive_doiovar,
	                        NULL,
	                        NULL,
	                        NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	wlc->pub->_tcp_keepalive = TRUE;

	return (info);

fail:
	if (info != NULL && info->keep_pkt != NULL)
		MFREE(wlc->osh, info->keep_pkt, ETHER_HDR_LEN + sizeof(struct ipv4_hdr) +
		   sizeof(struct bcmtcp_hdr));
	if (NULL != info)
		MFREE(wlc->osh, info, sizeof(wl_tcp_keep_info_t));

	return (NULL);
}

void
BCMATTACHFN(wl_icmp_detach)(wl_icmp_info_t *icmpi)
{

	WL_INFORM(("wl%d: icmp_detach()\n", WLCUNITT(icmpi)));

	if (!icmpi)
		return;
#ifndef BCM_OL_DEV
	wlc_module_unregister(icmpi->wlc->pub, "icmp", icmpi);
#endif // endif
	MFREE(WLCOSHH(icmpi), icmpi, sizeof(wl_icmp_info_t));
}

void
BCMATTACHFN(wl_tcp_keep_detach)(wl_tcp_keep_info_t *tcp_keep_info)
{

	WL_INFORM(("wl%d: tcp keep-alive detach()\n", WLCUNITK(tcp_keep_info)));

	if (!tcp_keep_info)
		return;
#ifndef BCM_OL_DEV
	wlc_module_unregister(tcp_keep_info->wlc->pub, "tcp_keepalive", tcp_keep_info);
#endif // endif
	if (tcp_keep_info->keep_pkt != NULL)
		MFREE(WLCOSHK(tcp_keep_info), tcp_keep_info->keep_pkt, ETHER_HDR_LEN +
		    sizeof(struct ipv4_hdr) + sizeof(struct bcmtcp_hdr));
	if (tcp_keep_info->wake_pkt != NULL)
		PKTFREE(WLCOSHK(tcp_keep_info), tcp_keep_info->wake_pkt, FALSE);
	if (tcp_keep_info->tcp_keepalive_timer != NULL)
		wl_free_timer(tcp_keep_info->wlc->wl, tcp_keep_info->tcp_keepalive_timer);
	MFREE(WLCOSHK(tcp_keep_info), tcp_keep_info, sizeof(wl_tcp_keep_info_t));
}

#ifndef BCM_OL_DEV
/* Handling ICMP-related iovars */
static int
icmp_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_icmp_info_t *icmpi = hdl;
	int err = 0;
	int i;
	uint32 ipaddr;
	uint32 *ret_int_ptr = (uint32 *)a;

#ifndef BCMROMOFFLOAD
	WL_INFORM(("wl%d: icmp_doiovar()\n", WLCUNITT(icmpi)));
#endif /* !BCMROMOFFLOAD */

	/* donot use this ioctl if the module is not yet enabled */
	if (!ICMPOE_ENAB(icmpi->wlc->pub)) {
		return BCME_UNSUPPORTED;
	}

	/* change arpi if wlcif is corr to a virtualIF */
	if (wlcif != NULL) {
		if (wlcif->wlif != NULL) {
			icmpi = (wl_icmp_info_t *)wl_get_ifctx(icmpi->wlc->wl, IFCTX_ARPI,
			                                     wlcif->wlif);
		}
	}

	switch (actionid) {

	case IOV_SVAL(IOV_ICMP_OL):
		bcopy(a, &icmpi->icmp_enabled, sizeof(icmpi->icmp_enabled));
		break;

	case IOV_GVAL(IOV_ICMP_OL):
		bcopy(&icmpi->icmp_enabled, a, sizeof(icmpi->icmp_enabled));
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
tcp_keep_alive_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_tcp_keep_info_t	*info;
	int err;

	info = hdl;

	switch (actionid) {
		default:
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	return (err);
}
#endif /* BCM_OL_DEV */

static int
icmp_send_echo_reply(wl_icmp_info_t *icmpi, void *sdu, bool snap, uint16 iplen)
{
	void *pkt;
	struct ipv4_hdr *ipv4, *ipv4_from;
	struct bcmicmp_hdr *icmp_pkt;
	uint8 *frame;
	uint8 *sdu_frame = PKTDATA(WLCOSHH(icmpi), sdu);
	uint16 eth_pktlen = (ETHER_HDR_LEN +
	                 ((snap == TRUE) ? (SNAP_HDR_LEN + ETHER_TYPE_LEN) : 0));
	uint16 pktlen;

	pktlen = eth_pktlen + iplen;

	if (!(pkt = PKTGET(WLCOSHH(icmpi), pktlen, TRUE))) {
		WL_ERROR(("icmp_send_echo_reply: PKTGET failed\n"));
		return -1;
	}

	frame = PKTDATA(WLCOSHH(icmpi), pkt);
	ipv4 = (struct ipv4_hdr *)(frame + eth_pktlen);
	ipv4_from = (struct ipv4_hdr *)(sdu_frame + eth_pktlen);
	/* note we use both ipv4 and ipv4_from to get length, could do it differently but.. */
	icmp_pkt = (struct bcmicmp_hdr *) ((uint8 *)ipv4 + IPV4_HLEN(ipv4_from));

	/* always copy whole packet to pick up fields we may not change: ie: snap */
	bcopy(sdu_frame, frame, pktlen); 	/* copy original pkt and then muck fields */
	/* switch MAC addrs */
	bcopy(sdu_frame + ETHER_SRC_OFFSET, frame + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	bcopy(sdu_frame + ETHER_DEST_OFFSET, frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);

	/* now fix up IP portion */
	bcopy(ipv4_from->src_ip, ipv4->dst_ip, IPV4_ADDR_LEN);
	bcopy(ipv4_from->dst_ip, ipv4->src_ip, IPV4_ADDR_LEN);
	ipv4->ttl = IP_DEFAULT_TTL;
	ipv4->hdr_chksum = 0;
	ipv4->hdr_chksum = (uint16) ip_cksum((uint16 *)ipv4,  IPV4_HLEN(ipv4), 0, 0);

	/* now fix up ICMP portion */
	icmp_pkt->type = ICMP_TYPE_ECHO_REPLY;
	icmp_pkt->chksum = 0;
	icmp_pkt->chksum = (uint16) ip_cksum((uint16 *)icmp_pkt, iplen -  IPV4_HLEN(ipv4), 0, 0);

	/* packet has been updated so go send */
	wlc_sendpkt(icmpi->wlc, pkt, NULL);

	return 0;
}

static int
icmpv6_send_echo_reply(wl_icmp_info_t *icmpi, void *sdu, bool snap, uint16 iplen, uint32 psum)
{
	void *pkt;
	uint8 *frame;
	uint8 *sdu_frame = PKTDATA(WLCOSHH(icmpi), sdu);
	uint16 eth_pktlen = (ETHER_HDR_LEN +
	    ((snap == TRUE) ? (SNAP_HDR_LEN + ETHER_TYPE_LEN) : 0));
	uint16 pktlen;
	struct ipv6_hdr *ipv6, *ipv6_from;
	struct icmp6_hdr *icmpv6;

	pktlen = eth_pktlen + iplen;

	if (!(pkt = PKTGET(WLCOSHH(icmpi), pktlen, TRUE))) {
		WL_ERROR(("icmpv6_send_echo_reply: PKTGET failed\n"));
		return -1;
	}

	frame = PKTDATA(WLCOSHH(icmpi), pkt);
	ipv6 = (struct ipv6_hdr *)(frame + eth_pktlen);
	ipv6_from = (struct ipv6_hdr *)(sdu_frame + eth_pktlen);
	icmpv6 = (struct icmp6_hdr *) ((uint8 *)ipv6 + sizeof(struct ipv6_hdr));

	/* always copy whole packet to pick up fields we may not change: ie: snap */
	bcopy(sdu_frame, frame, pktlen); 	/* copy original pkt and then muck fields */
	/* switch MAC addrs */
	bcopy(sdu_frame + ETHER_SRC_OFFSET, frame + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
	bcopy(sdu_frame + ETHER_DEST_OFFSET, frame + ETHER_SRC_OFFSET, ETHER_ADDR_LEN);

	/* now fix up IPv6 portion */
	bcopy(ipv6_from->saddr.addr, ipv6->daddr.addr, 16);
	bcopy(ipv6_from->daddr.addr, ipv6->saddr.addr, 16);
	ipv6->hop_limit = IP_DEFAULT_TTL;

	/* now fix up ICMP portion */
	icmpv6->icmp6_type = ICMP6_ECHO_REPLY;
	icmpv6->icmp6_cksum = 0;
	icmpv6->icmp6_cksum = (uint16) ip_cksum((uint16 *)icmpv6, ntoh16(ipv6->payload_len),
	    0, psum);

	/* packet has been updated so go send */
	wlc_sendpkt(icmpi->wlc, pkt, NULL);

	return 0;
}

#ifdef ICMP_DEBUG
struct foo {
uint8   icmp6_type;
	uint8   icmp6_code;
	uint16  icmp6_cksum;
	uint16	id;
	uint16  seq;
};
#endif /* ICMP_DEBUG */

/** Returns -1 if frame is not TCP; otherwise, returns pointer/length of IP portion */
static int
wl_tcpkeep_parse(wl_tcp_keep_info_t *tcp_keep_info, void *sdu)
{
	uint8 *frame = PKTDATA(WLCOSHK(tcp_keep_info), sdu);
	int length = PKTLEN(WLCOSHK(tcp_keep_info), sdu);
	uint8 *pt;
	uint16 ethertype;
	struct ipv4_hdr *ipv4_in;
	struct bcmtcp_hdr *tcp_in;
	struct tcp_pseudo_hdr tcp_ps;
	int iplen;
	bool snap = FALSE;
	uint16 iphdr_len;
	uint16 tcp_len;
	uint16 tcp_len_adj;
	uint16 iphdr_cksum, tcp_cksum;
	uint16 cksum;
	uint32 psum;
	char tcp_flags[0];
	wl_mtcpkeep_alive_conn_pkt_t *tcp_keepalive_connection;
	int rc;

	/* Process Ethernet II or SNAP-encapsulated 802.3 frames */
	if (length < ETHER_HDR_LEN) {
		WL_ERROR(("wl%d: %s: short eth frame (%d)\n",
		          WLCUNITK(tcp_keep_info), __FUNCTION__, length));
		return -1;
	} else if (ntoh16_ua((const void *)(frame + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
		/* Frame is Ethernet II */
		pt = frame + ETHER_TYPE_OFFSET;
	} else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
	           !bcmp(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
		WL_ERROR(("wl%d: wl_tcpkeep_parse: 802.3 LLC/SNAP\n", WLCUNITK(tcp_keep_info)));
		pt = frame + ETHER_HDR_LEN + SNAP_HDR_LEN;
		snap = TRUE;
	} else {
		snap = FALSE;
		return -1;
	}

	ethertype = ntoh16_ua((const void *)pt);

	/* if not any packet type we are interested in just bail out */
	if ((ethertype != ETHER_TYPE_IP) /* && (ethertype != ETHER_TYPE_IPV6)*/) {
		if (ethertype == ETHER_TYPE_8021Q) {
			WL_ERROR(("%s: Dropped 802.1Q packet\n", __FUNCTION__));
		}
		return -1;
	}

	if (ethertype == ETHER_TYPE_IP) {
		ipv4_in = (struct ipv4_hdr *)(pt + ETHER_TYPE_LEN);

		if (ipv4_in->prot != IP_PROT_TCP) {
			return -1;
		}

		if (ntoh16(ipv4_in->frag) & IPV4_FRAG_OFFSET_MASK) {
			WL_ERROR(("wl_tcpkeep_parse: fragmented packet, not first fragment\n"));
			return -1;
			/* return ETHER_TYPE_IP; */
		}

		if (ntoh16(ipv4_in->frag) & IPV4_FRAG_MORE) {
			WL_ERROR(("wl_tcpkeep_parse: fragmented packet, more bit set\n"));
			return -1;
		}

		iplen = IPV4_HLEN(ipv4_in);	/* make it a byte count and not a word count */
		tcp_in = (struct bcmtcp_hdr *)(pt + ETHER_TYPE_LEN + iplen);

		/* now we need to check sum both IP and TCP before we do anything more.
		 * If the IP checksum fails, then just get out of here without accessing the
		 * icmp ptr, as that could lead to trouble.
		 */
		iphdr_len = IPV4_HLEN(ipv4_in);
		iphdr_cksum = ipv4_in->hdr_chksum;
		/* zero it out before we calculate */
		ipv4_in->hdr_chksum = 0;
		cksum = ip_cksum((uint16 *)ipv4_in, iphdr_len, 0, 0);
		if (cksum != iphdr_cksum) {
			WL_ERROR(("wl_tcpkeep_parse: bad IP header checksum: %x %x\n", cksum,
			    iphdr_cksum));
			return -1;
			/* return ETHER_TYPE_IP; */
		}

		bcopy(ipv4_in->dst_ip, &tcp_ps.dst_ip, IPV4_ADDR_LEN);
		bcopy(ipv4_in->src_ip, &tcp_ps.src_ip, IPV4_ADDR_LEN);
		tcp_ps.zero = 0;
		tcp_ps.prot = ipv4_in->prot;
		tcp_len = ntoh16(ipv4_in->tot_len) - iphdr_len;
		tcp_ps.tcp_size = hton16(tcp_len);
		psum = ip_cksum_partial((uint8 *)&tcp_ps, sizeof(struct tcp_pseudo_hdr), 1);
		tcp_cksum = tcp_in->chksum;
		tcp_in->chksum = 0;
		cksum = 0;
		cksum = (uint16) ip_cksum((uint16 *)tcp_in, tcp_len, 0, psum);
		if (cksum != tcp_cksum) {
			WL_ERROR(("wl_tcpkeep_parse: bad TCP header checksum: %x %x\n", cksum,
			    tcp_cksum));
			return -1;
			/* return ETHER_TYPE_IP; */
		}

		tcp_keepalive_connection = &tcp_keep_info->tcp_keepalive_connection;
		if (bcmp(&tcp_keepalive_connection->dipaddr, ipv4_in->src_ip, IPV4_ADDR_LEN) != 0) {
			WL_TRACE(("%s: Dropping, Came from wrong machine\n", __FUNCTION__));
			return -1;
		}
		if (bcmp(&tcp_keepalive_connection->sipaddr, ipv4_in->dst_ip, IPV4_ADDR_LEN) != 0) {
			WL_TRACE(("%s: Dropping, Heading to wrong machine\n", __FUNCTION__));
			return -1;
		}
		if (ntoh16(tcp_in->dst_port) != tcp_keepalive_connection->sport) {
			WL_TRACE(("%s: Dropping, Heading to wrong port\n", __FUNCTION__));
			return -1;
		}
		if (ntoh16(tcp_in->src_port) != tcp_keepalive_connection->dport) {
			WL_TRACE(("%s: Dropping, Came from wrong port\n", __FUNCTION__));
			return -1;
		}
		bcopy(&tcp_in->hdrlen_rsvd_flags, tcp_flags, 2);
		if ((tcp_flags[1] & 0x3f) != TCP_FLAG_ACK) {
			WL_ERROR(("wl_tcpkeep_parse: flags not just ACK: %x\n", tcp_flags[1]));
			tcp_keep_info->tcp_keep_enabled = 0;	/* we are waking up host */
			tcp_keep_info->tcp_wowl_pme_asserted = TRUE;
			tcp_keep_info->wake_pkt = PKTDUP(WLCOSHK(tcp_keep_info), sdu);
			wl_del_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer);
			tcp_wowl_assert_pme(tcp_keep_info, WL_WOWL_TCPKEEP_DATA, sdu, FALSE);
			return -1;
		}

		tcp_len_adj = TCP_HDRLEN(tcp_flags[0]) << 2;
		if (tcp_len != /* sizeof(struct bcmtcp_hdr) */ tcp_len_adj) {
			WL_ERROR(("wl_tcpkeep_parse: have data %x\n", tcp_len));
			tcp_keep_info->tcp_keep_enabled = 0;	/* we are waking up host */
			tcp_keep_info->tcp_wowl_pme_asserted = TRUE;
			tcp_keep_info->wake_pkt = PKTDUP(WLCOSHK(tcp_keep_info), sdu);
			wl_del_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer);
			tcp_wowl_assert_pme(tcp_keep_info, WL_WOWL_TCPKEEP_DATA, sdu, FALSE);
			return -1;
		}

		/* check to see if this is an ACK to our keepalive or a new keepalive */
		if ((tcp_keepalive_connection->ack == ntoh32(tcp_in->seq_num)) &&
		    ((tcp_keepalive_connection->seq + 1) == ntoh32(tcp_in->ack_num))) {
			/* this is an ACK to our keep-alive */
			WL_ERROR(("wl_tcpkeep_parse: we got an ACK to our KEEP-ALIVE\n"));
			tcp_keep_info->keep_alive_retry_cnt_left =
			    tcp_keep_info->keep_alive_retry_cnt;
			wl_del_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer);
			wl_add_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer,
			    tcp_keep_info->keep_alive_interval,
			    FALSE);
			return -1;
		}

		if (((tcp_keepalive_connection->ack - 1) == ntoh32(tcp_in->seq_num)) &&
		    ((tcp_keepalive_connection->seq + 1) == ntoh32(tcp_in->ack_num))) {
			/* this is a keep-alive that wee need to send to */
			WL_ERROR(("wl_tcpkeep_parse: we got a keep-alive\n"));
			tcp_keep_info->keep_alive_retry_cnt_left =
			    tcp_keep_info->keep_alive_retry_cnt;
			rc = tcp_keep_send_pkt(tcp_keep_info, tcp_keep_info->keep_pkt);
			wl_del_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer);
			wl_add_timer(tcp_keep_info->wlc->wl,
			    tcp_keep_info->tcp_keepalive_timer,
			    tcp_keep_info->keep_alive_interval,
			    FALSE);
			return -1;
		}

		return -1;
	}

	return 0;
}

/** Returns -1 if frame is not icmp; otherwise, returns pointer/length of IP portion */
static int
wl_icmp_parse(wl_icmp_info_t *icmpi, void *sdu)
{
#ifdef ICMP_DEBUG
	struct foo *fooo;
#endif	/* ICMP_DEBUG */
	uint8 *frame = PKTDATA(WLCOSHH(icmpi), sdu);
	int length = PKTLEN(WLCOSHH(icmpi), sdu);
	uint8 *pt;
	uint16 ethertype;
	struct ipv4_hdr *ipv4_in;
	struct bcmicmp_hdr *icmpv4_in;
	struct ipv6_hdr *ipv6_in;
	struct icmp6_hdr *icmpv6_in;
	struct ipv6_pseudo_hdr ipv6_pseudo;
	int iplen;
	bool snap = FALSE;
	uint16 iphdr_len, icmp_len;
	uint16 iphdr_cksum, icmp_cksum;
	uint16 cksum;
	uint32 psum;
	int rc;

	/* Process Ethernet II or SNAP-encapsulated 802.3 frames */
	if (length < ETHER_HDR_LEN) {
		return -1;
	} else if (ntoh16_ua((const void *)(frame + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
		/* Frame is Ethernet II */
		pt = frame + ETHER_TYPE_OFFSET;
	} else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
	           !bcmp(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
		pt = frame + ETHER_HDR_LEN + SNAP_HDR_LEN;
		snap = TRUE;
	} else {
		snap = FALSE;
		return -1;
	}

	ethertype = ntoh16_ua((const void *)pt);

	/* if not any packet type we are interested in just bail out */
	if ((ethertype != ETHER_TYPE_IP) && (ethertype != ETHER_TYPE_IPV6)) {
		if (ethertype == ETHER_TYPE_8021Q) {
			WL_ERROR(("%s: Dropped 802.1Q packet\n", __FUNCTION__));
		}
		return -1;
	}

	if (ethertype == ETHER_TYPE_IP) {
		ipv4_in = (struct ipv4_hdr *)(pt + ETHER_TYPE_LEN);

		if (ipv4_in->prot != IP_PROT_ICMP) {
			return -1;
		}

		if (ntoh16(ipv4_in->frag) & IPV4_FRAG_OFFSET_MASK) {
			return -1;
		}

		if (ntoh16(ipv4_in->frag) & IPV4_FRAG_MORE) {
			return -1;
		}

		iplen = IPV4_HLEN(ipv4_in);	/* make it a byte count and not a word count */
		icmpv4_in = (struct bcmicmp_hdr *)(pt + ETHER_TYPE_LEN + iplen);

		/* now we need to check sum both IP and ICMP before we do anything more.
		 * If the IP checksum fails, then just get out of here without accessing the
		 * icmp ptr, as that could lead to trouble.
		 */
		iphdr_len = IPV4_HLEN(ipv4_in);
		iphdr_cksum = ipv4_in->hdr_chksum;
		/* zero it out before we calculate */
		ipv4_in->hdr_chksum = 0;
		cksum = ip_cksum((uint16 *)ipv4_in, iphdr_len, 0, 0);
		if (cksum != iphdr_cksum) {
			WL_ERROR(("%s: bad IP header checksum: %x %x\n", __FUNCTION__, cksum,
			    iphdr_cksum));
			return -1;
		}

		icmp_cksum = icmpv4_in->chksum;
		icmpv4_in->chksum = 0;	/* zero before calculate */
		icmp_len = ntoh16(ipv4_in->tot_len) - iphdr_len;
		cksum = ip_cksum((uint16 *)icmpv4_in, icmp_len, 0, 0);
		if (cksum != icmp_cksum) {
			WL_ERROR(("%s: bad ICMP packet checksum tot_len %d, icmp_len %d\n",
				__FUNCTION__, ntoh16(ipv4_in->tot_len), icmp_len));
			return -1;
		}

		/* OK packet looks good, so lets process the ICMP message */
		switch (icmpv4_in->type) {
			case ICMP_TYPE_ECHO_REQUEST:
				WL_TRACE(("ICMP ECHO_REQUEST: processing\n"));
				/* seems return value is ignored, is this good */
				rc = icmp_send_echo_reply(icmpi, sdu, snap,
				    ntoh16(ipv4_in->tot_len));
				break;
			default:
				break;
		}
		return -1;
	}
	else if (ethertype == ETHER_TYPE_IPV6) {
		ipv6_in = (struct ipv6_hdr *)(pt + ETHER_TYPE_LEN);
		if (ipv6_in->nexthdr != ICMPV6_HEADER_TYPE) {
			return -1;
		}
		icmp_len = ntoh16(ipv6_in->payload_len);
		icmpv6_in = (struct icmp6_hdr *)(pt + ETHER_TYPE_LEN + sizeof(struct ipv6_hdr));

		/* we need to calculate the icmpv6 checksum. Note it uses a pseudo hdr, which is
		 * different than its ipv4 counterpart, but there is no ip layer checksum
		 */
		bzero((char *)&ipv6_pseudo, sizeof(struct ipv6_pseudo_hdr));
		bcopy((char *)ipv6_in->saddr.addr, (char *)ipv6_pseudo.saddr, 16);
		bcopy((char *)ipv6_in->daddr.addr, (char *)ipv6_pseudo.daddr, 16);
		ipv6_pseudo.payload_len = ipv6_in->payload_len;
		ipv6_pseudo.next_hdr =  ntoh16(ipv6_in->nexthdr);

		icmp_cksum = ntoh16(icmpv6_in->icmp6_cksum);
		icmpv6_in->icmp6_cksum = 0;
		psum = ip_cksum_partial((uint8 *)&ipv6_pseudo, sizeof(struct ipv6_pseudo_hdr), 1);
		cksum = ip_cksum((uint16 *)icmpv6_in, icmp_len, 0, psum);
#ifdef ICMP_DEBUG
		fooo = (struct foo *)icmpv6_in;
		WL_ERROR(("wl_icmp_parse: ipv6: len=%d type=%x code=%x ck_sum=%x id=%x seq=%x\n",
		    icmp_len, icmpv6_in->icmp6_type, icmpv6_in->icmp6_code, icmpv6_in->icmp6_cksum,
		    fooo->id, fooo->seq));
#endif /* ICMP_DEBUG */
		if (cksum != ntoh16(icmp_cksum)) {
			WL_ERROR(("wl_icmp_parse: ipv6 BAD cksums(%d): cksum=%x icmp_cksum=%x\n",
			    sizeof(struct ipv6_addr), cksum, icmp_cksum));
			return -1;
		}

		/* OK packet looks good, so lets process the ICMP message */
		switch (icmpv6_in->icmp6_type) {
			case ICMP6_ECHO_REQUEST:
				WL_ERROR(("ICMP6 ECHO_REQUEST: processing\n"));
				rc = icmpv6_send_echo_reply(icmpi, sdu, snap, icmp_len +
				    sizeof(struct ipv6_hdr), psum);
				break;
			default:
				break;
		}
		return -1;
	}

	return 0;
}

uint32
ip_cksum_partial(uint8 *val8, uint32 count, int j)
{
	uint32	sum, i;
	uint16	*val16;

	sum = 0;
	val16 = (uint16 *) val8;
	count /= 2;
	for (i = 0; i < count; i++) {
		if (j)
			sum += *val16++;
		else
			sum += ntoh16(*val16++);
	}
	return (sum);
}

uint16
ip_cksum(uint16 *val16, uint32 count, int j, uint32 sum)
{
	while (count > 1) {
		if (j)
			sum += ntoh16(*val16++);
		else
			sum += *val16++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
		sum += (*(uint8 *)val16);

	/*  Fold 32-bit sum to 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ((uint16)~sum);
}

int
wl_tcpkeep_recv_proc(wl_tcp_keep_info_t *tcp_keep_info, void *sdu)
{
	int proto;

#ifdef BCM_OL_DEV
	if (!tcp_keep_info->tcp_keep_enabled)
		return -1;
#endif // endif
	/* See if this is an TCP packet */
	proto = wl_tcpkeep_parse(tcp_keep_info, sdu);
	if (proto < 0) {
		/* not an TCP packet so return failure */
		return -1;
	}

	return 0;
}

int
wl_icmp_recv_proc(wl_icmp_info_t *icmpi, void *sdu)
{
	int proto;

#ifdef BCM_OL_DEV
	if (!icmpi->icmp_enabled)
		return -1;
#endif // endif

	/* See if this is an ICMP packet */
	proto = wl_icmp_parse(icmpi, sdu);
	if (proto < 0) {
		/* not an ICMP packet so return failure */
		return -1;
	}

	return 0;
}

#ifdef BCM_OL_DEV	/* this came from the ARP code */

static int
tcp_keep_send_pkt(wl_tcp_keep_info_t *tcpkeepi, void *keep_pkt)
{
	void *pkt;
	uint8 *frame;
	uint16 pktlen;

	pktlen = ETHER_HDR_LEN + sizeof(struct ipv4_hdr) + sizeof(struct bcmtcp_hdr);

	if (!(pkt = PKTGET(WLCOSHK(tcpkeepi), pktlen, TRUE))) {
		WL_ERROR(("tcp_keep_send_pkt: PKTGET failed\n"));
		return -1;
	}

	frame = PKTDATA(WLCOSHK(tcpkeepi), pkt);
	bcopy(keep_pkt, frame, pktlen); 	/* copy packet as it should be ready to send */

	/* packet has been updated so go send */
	wlc_sendpkt(tcpkeepi->wlc, pkt, NULL);

	return 0;
}

void
wl_tcp_setup_keep_pkt(wl_tcp_keep_info_t *tcpkeepi,
    wl_mtcpkeep_alive_conn_pkt_t *tcp_keepalive_connection, void *keep_pkt)
{
	struct ether_header *eth;
	struct ipv4_hdr *ip;
	struct bcmtcp_hdr *tcp;
	struct tcp_pseudo_hdr tcp_ps;
	uint32	psum;

	/* here we set up the packet that we are going to send */
	if (keep_pkt == NULL) {
		WL_ERROR(("wl_tcp_setup_keep_pkt: NULL pkt. This is bad\n"));
		return;
	}
	eth = (struct ether_header *)keep_pkt;
	bcopy(&tcp_keepalive_connection->daddr, eth->ether_dhost, ETHER_ADDR_LEN);
	bcopy(&tcp_keepalive_connection->saddr, eth->ether_shost, ETHER_ADDR_LEN);
	eth->ether_type = hton16(ETHER_TYPE_IP);
	ip = (struct ipv4_hdr *)(keep_pkt + sizeof(struct ether_header));
	bcopy(&tcp_keepalive_connection->dipaddr, ip->dst_ip, IPV4_ADDR_LEN);
	bcopy(&tcp_keepalive_connection->sipaddr, ip->src_ip, IPV4_ADDR_LEN);
	ip->version_ihl = 0x45;		/* standard value */
	ip->tos = 0;
	ip->tot_len = hton16(sizeof(struct ipv4_hdr) + sizeof(struct bcmtcp_hdr));
	ip->id = hton16(1638);
	ip->frag = 0;
	ip->ttl = IP_DEFAULT_TTL;
	ip->prot = IP_PROT_TCP;
	ip->hdr_chksum = 0;
	ip->hdr_chksum = (uint16) ip_cksum((uint16 *)ip,  IPV4_HLEN(ip), 0, 0);
	tcp = (struct bcmtcp_hdr *)(keep_pkt + sizeof(struct ether_header) +
	    sizeof(struct ipv4_hdr));
	tcp->src_port = hton16(tcp_keepalive_connection->sport);
	tcp->dst_port = hton16(tcp_keepalive_connection->dport);
	tcp->seq_num = hton32(tcp_keepalive_connection->seq);
	tcp->ack_num = hton32(tcp_keepalive_connection->ack);
	tcp->tcpwin = hton16(tcp_keepalive_connection->tcpwin);
	tcp->chksum = 0;
	tcp->urg_ptr = 0;
	tcp->hdrlen_rsvd_flags = (TCP_FLAG_ACK << 8) |
	    ((sizeof(struct bcmtcp_hdr) >> 2) << TCP_HDRLEN_SHIFT);

	/* now need to set up the pseudo header so we can checksum TCP */
	bcopy(ip->dst_ip, &tcp_ps.dst_ip, IPV4_ADDR_LEN);
	bcopy(ip->src_ip, &tcp_ps.src_ip, IPV4_ADDR_LEN);
	tcp_ps.zero = 0;
	tcp_ps.prot = ip->prot;
	tcp_ps.tcp_size = hton16(sizeof(struct bcmtcp_hdr));
	psum = ip_cksum_partial((uint8 *)&tcp_ps, sizeof(struct tcp_pseudo_hdr), 1);
	tcp->chksum = (uint16) ip_cksum((uint16 *)tcp, sizeof(struct bcmtcp_hdr), 0, psum);
}

void
wl_tcp_keepalive_proc_msg(wlc_dngl_ol_info_t * wlc_dngl_ol, wl_tcp_keep_info_t *tcpkeepi,
    void *buf)
{
	uint8 *pktdata;
	olmsg_header *msg_hdr;
	olmsg_tcp_keep_conn *tcp_keep_conn_msg;
	olmsg_tcp_keep_timers *tcp_keep_timers_msg;

	pktdata = (uint8 *)buf;
	msg_hdr = (olmsg_header *) pktdata;

	switch (msg_hdr->type) {
		case BCM_OL_TCP_KEEP_TIMERS:
			tcp_keep_timers_msg = (olmsg_tcp_keep_timers *)pktdata;
			if (tcp_keep_timers_msg->tcp_keepalive_timers.interval)
			   tcpkeepi->keep_alive_interval =
			      tcp_keep_timers_msg->tcp_keepalive_timers.interval * TCP_TIMER_RES;
			if (tcp_keep_timers_msg->tcp_keepalive_timers.retry_interval)
			   tcpkeepi->keep_alive_retry_interval =
			   tcp_keep_timers_msg->tcp_keepalive_timers.retry_interval * TCP_TIMER_RES;
			if (tcp_keep_timers_msg->tcp_keepalive_timers.retry_count)
			   tcpkeepi->keep_alive_retry_cnt =
			      tcp_keep_timers_msg->tcp_keepalive_timers.retry_count;

			WL_ERROR(("%s: Request (seconds): interval %d retry %d cnt %d\n",
				__FUNCTION__,
			    tcp_keep_timers_msg->tcp_keepalive_timers.interval,
			    tcp_keep_timers_msg->tcp_keepalive_timers.retry_interval,
			    tcp_keep_timers_msg->tcp_keepalive_timers.retry_count));
			WL_ERROR(("%s: New Settings (msecs): interval %d retry %d cnt %d\n",
				__FUNCTION__,
			    tcpkeepi->keep_alive_interval,
			    tcpkeepi->keep_alive_retry_interval,
			    tcpkeepi->keep_alive_retry_cnt));
			break;
		case BCM_OL_TCP_KEEP_CONN:
			tcp_keep_conn_msg = (olmsg_tcp_keep_conn *)pktdata;
			if (tcpkeepi->num_conn_recs >= 1) {
				return;
			}
			bcopy(&tcp_keep_conn_msg->tcp_keepalive_conn,
			    &tcpkeepi->tcp_keepalive_connection,
			    sizeof(wl_mtcpkeep_alive_conn_pkt_t));
			tcpkeepi->num_conn_recs++;
			wl_tcp_setup_keep_pkt(tcpkeepi, &tcpkeepi->tcp_keepalive_connection,
			    tcpkeepi->keep_pkt);
			tcpkeepi->keep_alive_retry_cnt_left = tcpkeepi->keep_alive_retry_cnt;
			tcpkeepi->tcp_keep_enabled = 1;
			break;
		default:
			WL_ERROR(("wl_tcp_keepalive_proc_msg: INVALID message type:%d\n",
			    msg_hdr->type));
	}
}

void
wl_tcp_keepalive_event_handler(wl_tcp_keep_info_t *tcpkeepi, uint32 event, void *event_data)
{

	switch (event) {
		case BCM_OL_E_WOWL_COMPLETE:
			if (tcpkeepi->tcp_keep_enabled == 1) {
				WL_ERROR(("%s: %s: Starting Tcpkeepalive\n",
					__FUNCTION__, bcm_ol_event_str[event]));
				wl_add_timer(
				    tcpkeepi->wlc->wl,
				    tcpkeepi->tcp_keepalive_timer,
				    tcpkeepi->keep_alive_interval,
				    FALSE);
			}
			break;
		case BCM_OL_E_DEAUTH:
		case BCM_OL_E_DISASSOC:
		case BCM_OL_E_BCN_LOSS:
		case BCM_OL_E_PME_ASSERTED:
			if (tcpkeepi->tcp_keep_enabled == 1) {
				WL_ERROR(("%s: %s: Disabling Tcpkeepalive\n",
					__FUNCTION__, bcm_ol_event_str[event]));
				tcpkeepi->tcp_keep_enabled = 0;
				wl_del_timer(tcpkeepi->wlc->wl, tcpkeepi->tcp_keepalive_timer);
			}
			break;
		default:
			WL_TRACE(("%s: unsupported event: %s (%d)\n", __FUNCTION__,
				bcm_ol_event_str[event], event));
			break;
	}
}

void
wl_icmp_event_handler(wl_icmp_info_t *icmpi, uint32 event, void *event_data)
{

	switch (event) {
		case BCM_OL_E_WOWL_COMPLETE:
			WL_ERROR(("%s: %s: Starting ICMP\n",
				__FUNCTION__, bcm_ol_event_str[event]));
			icmpi->icmp_enabled = 1;
			break;
		case BCM_OL_E_DEAUTH:
		case BCM_OL_E_DISASSOC:
		case BCM_OL_E_BCN_LOSS:
		case BCM_OL_E_PME_ASSERTED:
			if (icmpi->icmp_enabled == 1) {
				WL_ERROR(("%s: %s: Disabling ICMP\n",
					__FUNCTION__, bcm_ol_event_str[event]));
				icmpi->icmp_enabled = 0;
			}
			break;
		default:
			WL_TRACE(("%s: unsupported event: %s\n", __FUNCTION__,
				bcm_ol_event_str[event]));
			break;
	}
}

#endif /* BCM_OL_DEV */
