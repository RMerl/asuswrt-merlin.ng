/**
 * @file
 * @brief
 * Multi-hop IP forwarding offload (IPFO) functionality for Broadcom 802.11 Networking Driver
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
 * $Id: wlc_ipfo.c 433321 2013-10-31 09:30:44Z $
 */

/**
 * @file
 * @brief
 * Forward packets over multiple hops to support long distance use cases.
 */

/**
 * @file
 * @brief
 * Twiki: [AdHocEnhancements]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlioctl.h>
#include <wlc_bmac.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_ipfo.h>
#include <wlc_alloc.h>
#include <bcmendian.h>
#include <proto/vlan.h>
#include <wlc_ampdu_rx.h>

/* iovar table */
enum {
	IOV_IPFO,	/* enable/disable forwarding offload feature */
	IOV_IP_ROUTE_TABLE	/* Update route table */
};

static const bcm_iovar_t ipfo_iovars[] = {
	{"ipfo", IOV_IPFO, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{"ip_route_table", IOV_IP_ROUTE_TABLE,
	(0), IOVT_BUFFER, 0
	},
	/* This need to be removed after DHD modification */
	{"ibss_route_tbl", IOV_IP_ROUTE_TABLE,
	(0), IOVT_BUFFER, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* IPFO module specific information */
struct wlc_ipfo_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	wlc_pub_t *pub;
	osl_t *osh;
	int cfgh;		/* BSSCFG cubby offset */
};

typedef struct wlc_ipfo_route_tbl_entry {
	struct wlc_ipfo_route_tbl_entry *next;
	wlc_ipfo_route_entry_t entry;
} wlc_ipfo_route_tbl_entry_t;

/* BSSCFG specific information */
typedef struct bss_ipfo_info {
	uint32 route_tbl_size;
	wlc_ipfo_route_tbl_entry_t *route_tbl;
	wlc_ipfo_route_tbl_entry_t **route_hash_tbl;
} bss_ipfo_info_t;

/* bsscfg specific info access accessor */
#define IPFO_BSSCFG_CUBBY_LOC(ipfo, cfg) ((bss_ipfo_info_t **)BSSCFG_CUBBY((cfg), (ipfo)->cfgh))
#define IPFO_BSSCFG_CUBBY(ipfo, cfg) (*(IPFO_BSSCFG_CUBBY_LOC(ipfo, cfg)))

#define WLC_IPFO_INFO_SIZE (sizeof(wlc_ipfo_info_t))
#define WLC_IPFO_ROUTE_HASHTBL_SIZE	16
#define WLC_IPFO_ROUTE_HASH_FN(ip_addr) ((((ip_addr).addr[2] ^ (ip_addr).addr[3]) ^ \
	(((ip_addr).addr[2] ^ (ip_addr).addr[3]) >> 4)) % WLC_IPFO_ROUTE_HASHTBL_SIZE)

static int wlc_ipfo_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

static void wlc_ipfo_route_hash_add(bss_ipfo_info_t *bipfo,
	wlc_ipfo_route_tbl_entry_t *route_tbl_entry);
static int wlc_ipfo_cfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_ipfo_cfg_cubby_deinit(void *ctx, wlc_bsscfg_t *cfg);

wlc_ipfo_info_t *
BCMATTACHFN(wlc_ipfo_attach)(wlc_info_t *wlc)
{
	wlc_ipfo_info_t *ipfo_info;

	/* Allocate info structure */
	if (!(ipfo_info = (wlc_ipfo_info_t *)MALLOC(wlc->osh, WLC_IPFO_INFO_SIZE))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)ipfo_info, WLC_IPFO_INFO_SIZE);
	ipfo_info->wlc = wlc;
	ipfo_info->pub = wlc->pub;
	ipfo_info->osh = wlc->osh;

	/* reserve cubby in the bsscfg container for private data */
	if ((ipfo_info->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_ipfo_info_t *),
		wlc_ipfo_cfg_cubby_init, wlc_ipfo_cfg_cubby_deinit,
		NULL, (void *)ipfo_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, ipfo_iovars, "ipfo",
		ipfo_info, wlc_ipfo_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: IP forwarding offload module register failed\n", wlc->pub->unit));
		goto fail;
	}

	return ipfo_info;

fail:
	wlc_ipfo_detach(ipfo_info);
	return NULL;
}

void
BCMATTACHFN(wlc_ipfo_detach)(wlc_ipfo_info_t *ipfo_info)
{
	if (!ipfo_info)
		return;

	wlc_module_unregister(ipfo_info->pub, "ipfo", ipfo_info);

	MFREE(ipfo_info->osh, ipfo_info, WLC_IPFO_INFO_SIZE);
}

static int
wlc_ipfo_cfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ipfo_info_t *ipfo_info = (wlc_ipfo_info_t *)ctx;
	bss_ipfo_info_t **pbipfo = IPFO_BSSCFG_CUBBY_LOC(ipfo_info, cfg);
	bss_ipfo_info_t *bipfo;
	wlc_info_t *wlc = ipfo_info->wlc;
	int err;

	/* allocate memory and point bsscfg cubby to it */
	if ((bipfo = MALLOC(wlc->osh, sizeof(bss_ipfo_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	bzero(bipfo, sizeof(bss_ipfo_info_t));

	*pbipfo = bipfo;
	return BCME_OK;
fail:
	return err;

}

static void
wlc_ipfo_cfg_cubby_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ipfo_info_t *ipfo_info = (wlc_ipfo_info_t *)ctx;
	bss_ipfo_info_t **pbipfo = IPFO_BSSCFG_CUBBY_LOC(ipfo_info, cfg);
	bss_ipfo_info_t *bipfo = *pbipfo;
	wlc_info_t *wlc = ipfo_info->wlc;

	if (bipfo == NULL)
		return;

	/* Free the hash route table and hash table */
	if (bipfo->route_tbl != NULL) {
		MFREE(ipfo_info->osh, bipfo->route_tbl,
			sizeof(wlc_ipfo_route_tbl_entry_t) *
			bipfo->route_tbl_size +
			sizeof(wlc_ipfo_route_tbl_entry_t *) *
			WLC_IPFO_ROUTE_HASHTBL_SIZE);
	}

	MFREE(wlc->osh, bipfo, sizeof(bss_ipfo_info_t));
	*pbipfo = NULL;
}

static int
wlc_ipfo_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_ipfo_info_t *ipfo_info = (wlc_ipfo_info_t *)hdl;

	int32 int_val = 0;
	bool bool_val;
	uint32 *ret_uint_ptr;
	int err = 0;
	wlc_bsscfg_t *bsscfg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	ret_uint_ptr = (uint32 *)a;

	bsscfg = wlc_bsscfg_find_by_wlcif(ipfo_info->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {

		case IOV_GVAL(IOV_IPFO):
			*ret_uint_ptr = IPFO_ENAB(ipfo_info->pub);
			break;

		case IOV_SVAL(IOV_IPFO):
			ipfo_info->pub->_ipfo = bool_val;
			break;

		case IOV_GVAL(IOV_IP_ROUTE_TABLE):
		{
			wlc_ipfo_route_tbl_t *rtbl = (wlc_ipfo_route_tbl_t *)a;
			int i = 0;
			bss_ipfo_info_t *bipfo = IPFO_BSSCFG_CUBBY(ipfo_info, bsscfg);

			if (plen < WL_IPFO_ROUTE_TBL_FIXED_LEN)
				return BCME_BUFTOOSHORT;

			if (bipfo == NULL) {
				rtbl->num_entry = 0;
				break;
			}

			if (plen < (WL_IPFO_ROUTE_TBL_FIXED_LEN +
				(bipfo->route_tbl_size * sizeof(wlc_ipfo_route_entry_t))))
				return BCME_BUFTOOSHORT;

			rtbl->num_entry = bipfo->route_tbl_size;

			while (i < bipfo->route_tbl_size) {
				bcopy(&bipfo->route_tbl[i].entry, &rtbl->route_entry[i],
					sizeof(wlc_ipfo_route_entry_t));
				i++;
			}

			break;
		}

		case IOV_SVAL(IOV_IP_ROUTE_TABLE):
		{
			wlc_ipfo_route_tbl_t *rtbl = (wlc_ipfo_route_tbl_t *)p;
			int i;
			bss_ipfo_info_t *bipfo = IPFO_BSSCFG_CUBBY(ipfo_info, bsscfg);

			if (bipfo == NULL)
				return BCME_ERROR;

			/* Check whether the table size mentioned is greater than max size */
			if ((rtbl->num_entry > WL_MAX_IPFO_ROUTE_TBL_ENTRY) ||
				plen <= (rtbl->num_entry * sizeof(wlc_ipfo_route_entry_t))) {
				WL_ERROR(("plen : %d, num_entry: %d\n", plen, rtbl->num_entry));
				err = BCME_BADARG;
				break;
			}

			if ((bipfo->route_tbl != NULL) &&
				(rtbl->num_entry != bipfo->route_tbl_size)) {
				MFREE(ipfo_info->osh, bipfo->route_tbl,
					sizeof(wlc_ipfo_route_tbl_entry_t) *
					bipfo->route_tbl_size +
					sizeof(wlc_ipfo_route_tbl_entry_t *) *
					WLC_IPFO_ROUTE_HASHTBL_SIZE);
				bipfo->route_tbl = NULL;
				bipfo->route_hash_tbl = NULL;
			}

			if ((rtbl->num_entry > 0) && (bipfo->route_tbl == NULL)) {
				/* Allocate memory for routing table */
				bipfo->route_tbl = MALLOC(ipfo_info->osh,
					sizeof(wlc_ipfo_route_tbl_entry_t) *
					rtbl->num_entry +
					sizeof(wlc_ipfo_route_tbl_entry_t *) *
					WLC_IPFO_ROUTE_HASHTBL_SIZE);

				if (bipfo->route_tbl == NULL)
					return BCME_ERROR;

				/* Allocate memory for hash table */
				bipfo->route_hash_tbl = (wlc_ipfo_route_tbl_entry_t **)
					((uint8 *)bipfo->route_tbl +
					sizeof(wlc_ipfo_route_tbl_entry_t) * rtbl->num_entry);

			}

			/* Clean up all the existing hash table entries */
			if (bipfo->route_hash_tbl != NULL) {
				bzero(bipfo->route_hash_tbl,
					sizeof(wlc_ipfo_route_tbl_entry_t *) *
					WLC_IPFO_ROUTE_HASHTBL_SIZE);
			}

			for (i = 0; i < rtbl->num_entry; i++) {
				bcopy(&rtbl->route_entry[i],  &bipfo->route_tbl[i].entry,
					sizeof(wlc_ipfo_route_entry_t));
				wlc_ipfo_route_hash_add(bipfo, &bipfo->route_tbl[i]);
			}

			bipfo->route_tbl_size = rtbl->num_entry;
			break;
		}

		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

static void
wlc_ipfo_route_hash_add(bss_ipfo_info_t *bipfo, wlc_ipfo_route_tbl_entry_t *route_tbl_entry)
{
	uint32 hash_idx = WLC_IPFO_ROUTE_HASH_FN(route_tbl_entry->entry.ip_addr);
	route_tbl_entry->next = bipfo->route_hash_tbl[hash_idx];
	bipfo->route_hash_tbl[hash_idx] = route_tbl_entry;
}

static wlc_ipfo_route_entry_t *
wlc_ipfo_route_hash_find(bss_ipfo_info_t *bipfo, struct ipv4_addr *ip_addr)
{
	uint32 hash_idx = WLC_IPFO_ROUTE_HASH_FN(*ip_addr);
	wlc_ipfo_route_tbl_entry_t *route_entry;

	route_entry = bipfo->route_hash_tbl[hash_idx];
	while (route_entry != NULL) {
		if (bcmp(&route_entry->entry.ip_addr, ip_addr, IPV4_ADDR_LEN) == 0)
			return &route_entry->entry;
		route_entry = route_entry->next;
	}
	return NULL;
}

/* Returns TRUE if the packet needs to be forwarded */
static struct ether_addr *
wlc_ipfo_find_nexthop(bss_ipfo_info_t *bipfo,
	struct ipv4_hdr *iph)
{
	wlc_ipfo_route_entry_t *route_entry;

	if ((route_entry = wlc_ipfo_route_hash_find(bipfo,
		(struct ipv4_addr *)&iph->dst_ip)) != NULL)
		return &route_entry->nexthop;

	return NULL;
}

static uint16
wlc_ipfo_iphdr_chksm(void *dataptr, uint16 len)
{
	uint16 tmp = 0;
	uint8 *ptr = (uint8 *)dataptr;
	uint16 *u16_ptr;
	uint32 sum = 0;
	bool odd = ((uint32)dataptr & 0x1);

	if (odd) {
		((uint8 *)&tmp)[1] = *ptr++;
		len--;
	}

	u16_ptr = (uint16 *)ptr;

	while (len > 1) {
		sum += *u16_ptr++;
		len -= 2;
	}

	ptr = (uint8 *)u16_ptr;

	if (len) {
		((uint8 *)&tmp)[0] = *ptr;
	}

	sum += tmp;

	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + (sum & 0xffff);

	return (uint16)(~sum & 0xffff);
}

static int
wlc_ipfo_get_iph(wlc_ipfo_info_t *ipfo_info, void *sdu, struct ipv4_hdr **iph_ptr)
{
	uint8 *frame = PKTDATA(ipfo_info->osh, sdu);
	int length = PKTLEN(ipfo_info->osh, sdu);
	uint8 *pt;			/* Pointer to type field */
	uint16 ethertype, ihl;
	struct ipv4_hdr *iph;		/* IP frame pointer */
	int ipl;			/* IP frame length */

	/* Check whether its a Ethernet II frame or not */
	if ((length < ETHER_HDR_LEN) ||
		(ntoh16(*(uint16 *)(frame + ETHER_TYPE_OFFSET)) < ETHER_TYPE_MIN)) {
		WL_ERROR(("%s : Short or non-Ethernet II frame (%d)\n",
		          __FUNCTION__, length));
		return -1;
	}

	pt = frame + ETHER_TYPE_OFFSET;

	ethertype = ntoh16(*(uint16 *)pt);

	/* Skip VLAN tag, if any */
	if (ethertype == ETHER_TYPE_8021Q) {
		pt += VLAN_TAG_LEN;

		if (pt + ETHER_TYPE_LEN > frame + length) {
			WL_ERROR(("%s: short VLAN frame (%d)\n",
			          __FUNCTION__, length));
			return -1;
		}

		ethertype = ntoh16(*(uint16 *)pt);
	}

	/* Return if not IP packet */
	if (ethertype != ETHER_TYPE_IP) {
		return -1;
	}

	iph = (struct ipv4_hdr *)(pt + ETHER_TYPE_LEN);
	ipl = length - (pt + ETHER_TYPE_LEN - frame);

	/* We support IPv4 only */
	if (ipl < IPV4_OPTIONS_OFFSET || (IP_VER(iph) != IP_VER_4)) {
		WL_ERROR(("%s: short frame (%d) or non-IPv4\n", __FUNCTION__, ipl));
		return -1;
	}

	/* Header length sanity */
	ihl = IPV4_HLEN(iph);
	if (ihl < IPV4_OPTIONS_OFFSET || ihl > ipl) {
		WL_ERROR(("%s: IP-header-len (%d) out of range (%d-%d)\n",
		          __FUNCTION__, ihl, IPV4_OPTIONS_OFFSET, ipl));
		return -1;
	}

	*iph_ptr = iph;

	return 0;
}

bool
wlc_ipfo_forward(wlc_ipfo_info_t *ipfo_info, wlc_bsscfg_t *bsscfg, void *p)
{
	struct ipv4_hdr *iph;
	uint16 ihl;
	struct ether_addr *nexthop;
	struct ether_header *eh;
	wlc_info_t *wlc = ipfo_info->wlc;
	bss_ipfo_info_t *bipfo = IPFO_BSSCFG_CUBBY(ipfo_info, bsscfg);

	if (bipfo->route_tbl_size == 0)
		return FALSE;

	/* Check ip header sanity and get the header */
	if (wlc_ipfo_get_iph(ipfo_info, p, &iph) < 0)
		return FALSE;

	/* Send it to host if the TTL value is less than or equal to 1
	 *  The IP layer should sent the error message in this case.
	 */
	if (iph->ttl <= 1)
		return FALSE;

	if ((nexthop = wlc_ipfo_find_nexthop(bipfo, iph)) == NULL)
		return FALSE;

	/* Modify the ether header for forwarding the packet */
	eh = (struct ether_header *)PKTDATA(wlc->osh, p);
	bcopy(nexthop, &eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy(&bsscfg->cur_etheraddr, &eh->ether_shost, ETHER_ADDR_LEN);

	/* Decrement the IP TTL value and recalculate checksum */
	ihl = IPV4_HLEN(iph);
	iph->ttl = iph->ttl - 1;
	iph->hdr_chksum = 0;
	iph->hdr_chksum = wlc_ipfo_iphdr_chksm(iph, ihl);

	WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(bsscfg));

#ifdef WL_FRWD_REORDER
	if (FRWD_REORDER_ENAB(wlc->pub) && AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
		p = wlc_ampdu_frwd_handle_host_reorder(wlc->ampdu_rx, p, TRUE);
	}
#endif // endif

	/* Sent all the re-ordered packets */
	while (p != NULL) {
		void *next_p = PKTLINK(p);
		PKTSETLINK(p, NULL);

		WLPKTTAGCLEAR(p);
		/* Before forwarding, fix the priority */
		if (QOS_ENAB(wlc->pub) && (PKTPRIO(p) == 0))
			pktsetprio(p, FALSE);
		wlc_sendpkt(wlc, p, bsscfg->wlcif);

		p = next_p;
	}
	return TRUE;
}
