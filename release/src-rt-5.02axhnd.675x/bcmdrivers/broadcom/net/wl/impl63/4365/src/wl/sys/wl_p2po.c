/*
 * P2P Offload
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
 * $Id: wl_p2po.c 733849 2017-11-30 07:25:00Z $
 *
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_p2p.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wl_export.h>
#include <proto/p2p.h>
#include <proto/bcmevent.h>

#include <bcm_p2p_disc.h>
#include <bcm_gas.h>
#include <bcm_decode.h>
#include <bcm_encode.h>
#include <bcm_decode_anqp.h>
#include <bcm_encode_anqp.h>
#include <bcm_decode_ie.h>
#include <bcm_decode_p2p.h>
#include <wl_dbg.h>

#include <wl_eventq.h>
#include <wl_gas.h>
#include <wl_p2po_disc.h>
#include <wl_p2po.h>

#ifndef MAX_REGISTERED_SERVICES
#define MAX_REGISTERED_SERVICES		10
#endif // endif
#ifndef P2PO_INSTANCE_TIMEOUT
#define P2PO_INSTANCE_TIMEOUT	30000
#endif // endif
#ifndef P2PO_SD_RECHECK_TIMEOUT
#define P2PO_SD_RECHECK_TIMEOUT	60000
#endif // endif
#ifndef MAX_SD_INSTANCE
#define MAX_SD_INSTANCE		8
#endif // endif
#ifndef P2PO_DEFAULT_LISTEN_CHANNEL
#define	P2PO_DEFAULT_LISTEN_CHANNEL	11
#endif // endif

/* listen period/interval time in msec, default is minimum 500ms period on every 5s interval */
#define P2PO_LISTEN_PERIOD			(500)
#define P2PO_LISTEN_INTERVAL			(5000)

/* Default GAS configuration values */
#define P2PO_GAS_PARAM_DEFAULT		(0xffff)	/* parameter not config - use default */
#define P2PO_DEFAULT_MAX_RETRANSMIT	0		/* max retransmit on no ACK from peer */
#define P2PO_DEFAULT_RESPONSE_TIMEOUT	500		/* msec to wait for resp after tx packet */
#define P2PO_DEFAULT_MAX_COMEBACK_DELAY	(0xffff)	/* max comeback delay in resp else fail */
#define P2PO_DEFAULT_MAX_RETRIES	0		/* max retries on failure */

#define WFDS_SEEK_HDL_INVALID		0xffffffff

/* service discovery state */
typedef enum {
	SD_STATE_IDLE,
	SD_STATE_WAIT_TO_START,
	SD_STATE_RUNNING
} sd_state_t;

typedef struct wl_p2po_service {
	struct wl_p2po_service	*next;			/* point to next service */
	uint8		protocol;		/* query protocol type */
	uint8		*request_data;		/* query request data */
	uint16		request_len;		/* query request data length */
	uint8		*response_data;		/* query response data */
	uint16		response_len;		/* query response data length */
	uint8		num_service;		/* num service */
} service_t;

typedef struct probe_resp {
	bool		add_device;		/* track if ADD device already generated */
	uint32		len;			/* Current length of probe response */
	uint8*		buf;			/* buffer to probe response */
} probe_resp_t;

typedef struct sd_instance {
	sd_state_t		state;			/* service discovery state */
	bcm_gas_t		*gas;			/* gas instance */
	struct ether_addr	addr;			/* peer destination address */
	uint32			seek_hdl;		/* matched seek entry */
	struct wl_timer		*tmr_instance_removal;	/* timer to remove the instance if the
							 * device is not found for some time
							 */
	uint16			channel;		/* peer channel */
	bool			is_fragment;		/* response fragmented */
	int16			service_update_indicator; /* Peer device service update indicator */
	uint16			retries;		/* # of SD retries attempted */
	probe_resp_t		probe_resp;		/* probe response -holding */
	wl_p2po_info_t		*p2po;			/* point to p2po handler */
} sd_instance_t;

typedef struct {
	uint32 seek_hdl;		/* unique id chosen by host */
	uint8 macaddr[6];		/* Seek service from a specific device with this
					 * MAC address, all 1's for any device.
					 */
	bool macaddr_is_wildcard;	/* Shortcut to check if macaddr is all 1's */
	uint8 svc_hash[6];		/* Service hash */
	uint8 svc_name_len;
	uint8 svc_info_req_len;
	uint16 query_data_len;
	uint8 *query_data;	/* Encoded ANQP Query Request data. Contains:
				 * - offset 0: Service Name Length
				 * - offset 1: Service Name (variable length)
				 * - offset svc_name_len: Service Info Request Length
				 * - offset svc_name_len+1: Service Info Request (variable length)
				 */
} wl_p2po_seek_t;

/* p2po private info structure */
struct wl_p2po_info {
	wlc_info_t		*wlc;			/* Pointer back to wlc structure */
	wl_eventq_info_t	*wlevtqi;			/* local event queue handler */
	wl_gas_info_t		*gasi;			/* gas handler */
	wl_disc_info_t		*disci;			/* discovery handler */

	bcm_p2p_disc_t		*disc;

	wl_p2po_listen_t	listen;			/* extended listen timing */

	disc_mode_t		disc_mode;		/* discovery mode */

	service_t		*services;		/* registered service list */
	uint8			service_count;		/* count of registered services */
	uint16			service_update_indicator;	/* local service update indicator */

	struct {
		uint8		protocol;		/* protocol to iterate */
		service_t	*at;			/* current position of iterator */
	} iterator;

	sd_instance_t		instance[MAX_SD_INSTANCE];	/* service discovery instances */

	uint8			listen_channel;		/* listen channel */
	uint8			protocol;		/* query protocol */
	uint16			query_len;		/* query length */
	uint8			*query;			/* query body */
	uint8			transaction_id;		/* for outbound query */
	uint16			sd_recheck_timeout;	/* Timeout for sending the service
							 * discovery again to all the peers
							 */

	wl_gas_config_t		gas_params;		/* GAS tunable parameters */

	/* Configured seek services */
	uint8			num_seek_svcs;		/* # of WFDS services to seek */
	int			seek_svcs_len[MAX_WFDS_SEEK_SVC];
	wl_p2po_seek_t		*seek_svcs[MAX_WFDS_SEEK_SVC]; /* WFDS services to seek */

	/* Configured advertised services */
	uint8			num_advert_svcs;	/* # of WFDS services to advertise */
	wl_p2po_wfds_advertise_add_t* advert_svcs[MAX_WFDS_ADVERT_SVC];
							/* WFDS services to advertise */
	int advert_svcs_len[MAX_WFDS_SEEK_SVC];

	/* Service data returned by WFDS service search */
	service_t		wfds_service;
	uint16			wfds_resp_buf_len;
	uint8			*wfds_resp_buf;

	/* Probe request Service Hash P2P IE vndr_ie data */
	uint8			*prbreq_svc_hash_vndr_ie;
	uint16			prbreq_svc_hash_vndr_ie_len;
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)

enum {
	IOV_P2PO_LISTEN,
	IOV_P2PO_FIND,
	IOV_P2PO_STOP,
	IOV_P2PO_STATE,
	IOV_P2PO_SD_RECHECK_TIMEOUT,
	IOV_P2PO_LISTEN_CHANNEL,
	IOV_P2PO_FIND_CONFIG,
	IOV_P2PO_GAS_CONFIG,
	IOV_P2PO_WFDS_SEEK_ADD,
	IOV_P2PO_WFDS_SEEK_DEL,
	IOV_P2PO_WFDS_ADVERT_ADD,
	IOV_P2PO_WFDS_ADVERT_DEL,
	IOV_P2PO_WFDS_SEEK_DUMP, /* DEBUG */
	IOV_P2PO_WFDS_ADVERT_DUMP, /* DEBUG */
	IOV_P2PO_TRACELEVEL /* DEBUG */
};

static const bcm_iovar_t p2po_iovars[] = {
	{"p2po_listen", IOV_P2PO_LISTEN,
	(0), IOVT_BUFFER, sizeof(wl_p2po_listen_t)
	},
	{"p2po_find", IOV_P2PO_FIND,
	(0), IOVT_VOID, 0
	},
	{"p2po_stop", IOV_P2PO_STOP,
	(0), IOVT_VOID, 0
	},
	{"p2po_state", IOV_P2PO_STATE,
	(0), IOVT_UINT32, 0
	},
	{"p2po_sd_recheck_timeout", IOV_P2PO_SD_RECHECK_TIMEOUT,
	(0), IOVT_UINT32, 0
	},
	{"p2po_listen_channel", IOV_P2PO_LISTEN_CHANNEL,
	(0), IOVT_UINT32, sizeof(int32)
	},
	{"p2po_find_config", IOV_P2PO_FIND_CONFIG,
	(0), IOVT_BUFFER, sizeof(wl_p2po_find_config_t)
	},
	{"p2po_gas_config", IOV_P2PO_GAS_CONFIG,
	(0), IOVT_BUFFER, sizeof(wl_gas_config_t)
	},
	{"p2po_wfds_seek_add", IOV_P2PO_WFDS_SEEK_ADD,
	(0), IOVT_BUFFER, sizeof(int32)
	},
	{"p2po_wfds_seek_del", IOV_P2PO_WFDS_SEEK_DEL,
	(0), IOVT_BUFFER, sizeof(int32)
	},
	{"p2po_wfds_advertise_add", IOV_P2PO_WFDS_ADVERT_ADD,
	(0), IOVT_BUFFER, sizeof(int32)
	},
	{"p2po_wfds_advertise_del", IOV_P2PO_WFDS_ADVERT_DEL,
	(0), IOVT_BUFFER, sizeof(int32)
	},

#if defined(BCMDBG)
	{"p2po_wfds_seek_dump", IOV_P2PO_WFDS_SEEK_DUMP,
	(0), IOVT_VOID, 0
	},
	{"p2po_wfds_advertise_dump", IOV_P2PO_WFDS_ADVERT_DUMP,
	(0), IOVT_VOID, 0
	},
#endif /* BCMDBG */
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	{"p2po_tracelevel", IOV_P2PO_TRACELEVEL,
	(0), IOVT_UINT32, 0
	},
#endif /* BCMDBG || BCMDBG_ERR */
	{NULL, 0, 0, 0, 0 }
};

/* forward declarations */
static int p2po_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen,
	int vsize, struct wlc_if *wlcif);
static int wl_p2po_listen(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_find(wl_p2po_info_t *p2po);
static int wl_p2po_stop(wl_p2po_info_t *p2po);
static uint32 wl_p2po_state(wl_p2po_info_t *p2po);
static int wl_p2po_listen_channel(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_gas_config(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_wfds_seek_get(wl_p2po_info_t *p2po, void *p, int plen, void *a, int alen);
static int wl_p2po_wfds_seek_add(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_wfds_seek_del(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_wfds_advert_get(wl_p2po_info_t *p2po, void *p, int plen, void *a, int alen);
static int wl_p2po_wfds_advert_add(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_wfds_advert_del(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_wfds_del_all_svcs(wl_p2po_info_t *p2po);
static wl_p2po_seek_t* wl_p2po_wfds_seek_match(wl_p2po_info_t *p2po,
	struct ether_addr *addr, int ad_descs_len, uint8 *ad_descs);
static int wl_p2po_upd_seek_svc_hash(wl_p2po_info_t *p2po);
static int wl_p2po_set_find_config(wl_p2po_info_t *p2po, void *arg, int len);
static int wl_p2po_get_find_config(wl_p2po_info_t *p2po, void *arg, int len);
#if defined(BCMDBG) || defined(BCMDBG_ERR)
static int wl_p2po_tracelevel(wl_p2po_info_t *p2po, void *arg, int len);
#endif /* BCMDBG || BCMDBG_ERR */

#if defined(BCMDBG)
static int wl_p2po_wfds_seek_dump(wl_p2po_info_t *p2po);
static int wl_p2po_wfds_advert_dump(wl_p2po_info_t *p2po);
#endif /* BCMDBG */

#if defined(WFDS_DEBUG)
	#define WL_P2PO_TRACE WL_PRINT
	#define WL_P2PO_TRACE_NAME(p, l, n) wl_p2po_print_name(p, l, n)
	#undef WL_ERROR
	#define WL_ERROR(x) printf x
#else /* BCMDBG || BCMDBG_ERR */
	#define WL_P2PO_TRACE(x)
	#define WL_P2PO_TRACE_NAME(p, l, n)
#endif /* WFDS_DEBUG */

#if defined(BCMDBG) && !defined(DONGLEBUILD)
	static void _dump_buf(uint8 *buf, int len);
	#define dump_buf(buf, len) _dump_buf(buf, len)
#else
	#define dump_buf(buf, len)
#endif // endif

#if defined(BCMDBG) || defined(WFDS_DEBUG)
void wl_p2po_print_name(char *prefix, uint namelen, uint8 *name);
#endif /* defined(BCMDBG) || defined(WFDS_DEBUG) */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* Structure to keep track of which p2po->advert_svcs[i] entries match the query */
typedef struct {
	int	adv_index;	/* Index of the entry in p2po->advert_svcs[i]
				 * whose Svc Name matches the query's Svc Name.
				 */
	bool	match_svc_info;	/* Whether the advert_svcs[i]'s Svc Info matches
				 * the query's Svc Info Req.
				 */
} wl_p2po_adv_match_t;

/* Find all p2po->advert_svcs[i] entries that match the given query data.
 * Record all matches into the given adv_matches[] array.
 * Returns the number of matches.
 */
static int
wl_p2po_find_svc_matches(wl_p2po_info_t *p2po, uint8 *query, uint16 query_len,
	uint8 *status, wl_p2po_adv_match_t *adv_matches, uint16 *si_desc_size)
{
	uint8* svc_name;
	uint8 svc_name_len;
	uint8* svc_info_req;
	uint8 svc_info_req_len;
	bool is_wildcard = FALSE;
	uint8 num_adv_matches = 0;
	wl_p2po_adv_match_t *match;
	int i;

	if (query == NULL || query_len < 2) {
		WL_P2PO_TRACE(("find_wfds: no query %u,%p\n", query_len, query));
		*status = P2PSD_RESP_STATUS_BAD_REQUEST; /* bad request */
		return 0;
	}

	/* Find the Service Name and Service Info Request in the query data */
	svc_name_len = query[0];
	svc_name = query + 1;
	svc_info_req_len = query[1 + svc_name_len];
	svc_info_req = query + 1 + svc_name_len + 1;
	WL_P2PO_TRACE_NAME("find_wfds: qry name=", svc_name_len, svc_name);
	if (svc_name_len == 0) {
		WL_P2PO_TRACE(("find_wfds: no qry svc_name\n"));
		*status = P2PSD_RESP_STATUS_BAD_REQUEST; /* bad request */
		return 0;
	}
	if (query_len < 1 + svc_name_len + 1 + svc_info_req_len) {
		WL_P2PO_TRACE(("find_wfds: BAD query len %u < (1 + %u + 1 + %u)\n",
			query_len, svc_name_len, svc_info_req_len));
		*status = P2PSD_RESP_STATUS_BAD_REQUEST; /* bad request */
		return 0;
	}
	if (svc_name[svc_name_len - 1] == '*') {
		is_wildcard = TRUE;
		svc_name_len--;
	}

	/* Match the service name to the service names in the configured WFDS
	 * advertised services.
	 */
	for (i = 0; i < p2po->num_advert_svcs; i++) {
		wl_p2po_wfds_advertise_add_t* ads;
		int ads_len;

		ads = p2po->advert_svcs[i];
		ads_len = p2po->advert_svcs_len[i];
		if (!ads || ads_len == 0)
			continue;
		if (svc_name_len <= ads->service_name_len &&
			memcmp(svc_name, ads->service_name, svc_name_len) == 0 &&
			(is_wildcard || svc_name_len == ads->service_name_len)) {

			/* Matched service name */
			match = &adv_matches[num_adv_matches];
			++num_adv_matches;
			match->adv_index = i;

			/* Check for a Service Info Request match */
			if (svc_info_req_len > 0 && ads->service_info_len > 0 &&
				NULL != bcmstrnstr((char*)ads->service_info,
				ads->service_info_len, (char*)svc_info_req,
				svc_info_req_len)) {
				match->match_svc_info = TRUE;
			} else {
				match->match_svc_info = FALSE;
			}

			/* Add the Service Info Descriptor size for this service
			 * to the total service info descriptor size.
			 *
			 * The contents of a Service Info Descriptor is described
			 * in table G3 of the document "Wi-Fi P2P Technical
			 * Specification Wi-Fi Direct Service Addendum 0.44".
			 */
			*si_desc_size +=
				4	/* Adv Svc Descriptor: Advertisement ID */
				+ 2	/* Adv Svc Descriptor: Config Methods */
				+ 1	/* Adv Svc Descriptor: Service Name Length */
				+ ads->service_name_len
					/* Adv Svc Descriptor: Service Name */
				+ 1	/* Service Status */
				+ 2	/* Service Information Length */
				+ (match->match_svc_info ? ads->service_info_len : 0);
					/* Service Information */
		} else {
			WL_P2PO_TRACE(("%s: i=%u ", __FUNCTION__, i));
			WL_P2PO_TRACE_NAME("len mismatch: ", svc_name_len, svc_name);
			WL_P2PO_TRACE_NAME(", ", ads->service_name_len, ads->service_name);
		}
	}

	if (num_adv_matches == 0) {
		*status = P2PSD_RESP_STATUS_DATA_NA; /* requested information not available */
	}

	return num_adv_matches;
}

/* Find a configured WFDS advertised service whose service name matches the one
 * in the given service discovery request vendor specific query data.  The query
 * data's service name may contain a '*' wildcard.
 *
 * If found, returns a service structure pointing to encoded data for
 * "ANQP Query Response Vendor-specific Content for WFDS".
 * If not found, returns NULL.
 *
 * Notes: The caller must free p2po->wfds_resp_buf if it is not NULL.
 */
static service_t *
wl_p2po_match_wfds_query(wl_p2po_info_t *p2po, uint8 *query, uint16 query_len, uint8 *status)
{
	uint8 num_adv_matches;
	wl_p2po_adv_match_t adv_matches[MAX_WFDS_ADVERT_SVC];
	service_t *service = NULL;
	bcm_encode_t resp_data;
	int i;

	p2po->wfds_resp_buf_len = 0;
	num_adv_matches = wl_p2po_find_svc_matches(p2po, query, query_len, status,
		&adv_matches[0], &p2po->wfds_resp_buf_len);
	if (num_adv_matches == 0) {
		WL_P2PO_TRACE(("match_wfds: no match\n"));
		return NULL;
	}

	/* Allocate memory for service data's response_data.  This response_data
	 * consists of the Service Info Descriptors data in "Table G2 - ANQP
	 * Query Response Vendor-specific Content for Wi-Fi Direct Services" of
	 * the document "Wi-Fi P2P Technical Specification Wi-Fi Direct Service
	 * Addendum 0.44":
	 *
	 * The contents of each Service Info Descriptor is described in table
	 * G3 of the same document.
	 */
	p2po->wfds_resp_buf = MALLOC(WLCOSH(p2po), p2po->wfds_resp_buf_len);
	if (!p2po->wfds_resp_buf) {
		WL_ERROR(("wl%d: %s: wfds_resp_buf MALLOC failed len=%u\n",
			WLCUNIT(p2po), __FUNCTION__, p2po->wfds_resp_buf_len));
		/* requested information not available */
		*status = P2PSD_RESP_STATUS_DATA_NA;
		return NULL;
	}

	/* Encode a Service Info Descriptor for each matched service */
	bcm_encode_init(&resp_data, p2po->wfds_resp_buf_len, p2po->wfds_resp_buf);

	for (i = 0; i < num_adv_matches; i++) {
		wl_p2po_wfds_advertise_add_t* ads;
		int ads_len;
		wl_p2po_adv_match_t *match = &adv_matches[i];

		ads = p2po->advert_svcs[match->adv_index];
		ads_len = p2po->advert_svcs_len[match->adv_index];
		if (!ads || ads_len == 0) {
			WL_ERROR(("match_wfds: no svc info at match %u adv %u\n",
				i, match->adv_index));
			continue;
		}

		/* Encode service info descriptor */
		bcm_encode_anqp_wfds_response(&resp_data,
			ads->advertisement_id, ads->service_config_method,
			ads->service_name_len, ads->service_name,
			ads->service_status,
			match->match_svc_info ? ads->service_info_len : 0,
			match->match_svc_info ? ads->service_info : NULL);
	}

	/* Prepare the service data to return */
	service = &p2po->wfds_service;
	bzero(service, sizeof(*service));
	service->num_service = num_adv_matches;
	service->protocol = SVC_RPOTYPE_WFDS;
	service->response_data = bcm_encode_buf(&resp_data);
	service->response_len = bcm_encode_length(&resp_data);

	WL_P2PO_TRACE(("match_wfds: matches=%u rsplen=%u bufsz=%u\n",
		num_adv_matches, service->response_len, p2po->wfds_resp_buf_len));

	*status = P2PSD_RESP_STATUS_SUCCESS;
	return service;
}

/* get the service with provided protocol and query */
static service_t *
wl_p2po_match_query(wl_p2po_info_t *p2po, uint8 protocol, uint8 *query, uint16 len, uint8 *status)
{
	service_t *service = p2po->services;

	while (service) {
		if (service->protocol == protocol &&
			service->request_len == len &&
			memcmp(service->request_data, query, len) == 0)
				/* found */
				break;
		service = service->next;
	}

	/* no service match query */
	if (!service)
		*status = P2PSD_RESP_STATUS_DATA_NA; /* requested information not available */

	return service;
}

/* get service with provided protocol and maybe query */
static service_t *
wl_p2po_get_service(wl_p2po_info_t *p2po, uint8 protocol, uint8 *query, uint16 len, uint8 *status)
{
	*status = P2PSD_RESP_STATUS_SUCCESS;

	p2po->iterator.at = NULL;

	switch (protocol) {
	/* query all available protocols */
	case SVC_RPOTYPE_ALL:
		if (len == 0) {
			/* query all available services */
			p2po->iterator.protocol = protocol;
			p2po->iterator.at = p2po->services;
		}
		else {
			/* The request has not been successful as
			 * one or more parameters have invalid values
			 */
			*status = P2PSD_RESP_STATUS_BAD_REQUEST;
		}
		break;

	case SVC_RPOTYPE_BONJOUR:
	case SVC_RPOTYPE_UPNP:
		if (len == 0) {
			/* query all Bonjour or UPnP services */
			p2po->iterator.protocol = protocol;
			p2po->iterator.at = p2po->services;
			/* get the first service with matched protocol */
			while (p2po->iterator.at && p2po->iterator.at->protocol != protocol)
				p2po->iterator.at = p2po->iterator.at->next;
		}
		else {
			return wl_p2po_match_query(p2po, protocol, query, len, status);
		}
		break;

	case SVC_RPOTYPE_WFDS:
		if (len == 0) {
			WL_P2PO_TRACE(("%s: SVC_RPOTYPE_WFDS: WFDS zero len!\n", __FUNCTION__));
			*status = P2PSD_RESP_STATUS_BAD_REQUEST; /* bad request */
			return NULL;
		}
		else {
			return wl_p2po_match_wfds_query(p2po, query, len, status);
		}
		break;

	default:
		/* protocol not supported */
		WL_P2PO_TRACE(("%s: unknown proto %d!\n", __FUNCTION__, protocol));
		*status = P2PSD_RESP_STATUS_PROTYPE_NA;
		break;
	}

	return p2po->iterator.at;
}

/* called repeatedly after wl_p2po_get_service to retrieve a service per invocation */
static service_t *
wl_p2po_get_next_service(wl_p2po_info_t *p2po)
{
	/* no service available */
	if (p2po->services == NULL)
		return NULL;

	/* no more matched services */
	if (p2po->iterator.at == NULL)
		return NULL;

	if (p2po->iterator.at->next &&
		(p2po->iterator.protocol == SVC_RPOTYPE_ALL ||
		p2po->iterator.protocol == p2po->iterator.at->next->protocol))
		/* next available service  */
		p2po->iterator.at = p2po->iterator.at->next;
	else
		/* no more matched services */
		p2po->iterator.at = NULL;

	return p2po->iterator.at;
}

/* find sd instance by ethernet address and seek handle */
static sd_instance_t *
wl_p2po_find_instance(wl_p2po_info_t *p2po, struct ether_addr *addr, uint32 seek_hdl)
{
	sd_instance_t *instance = 0;
	sd_instance_t *inst;
	int i;

	/* search for addr and seek handle */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		inst = &p2po->instance[i];
		if (memcmp(addr, inst->addr.octet, sizeof(*addr)) == 0 &&
			(inst->seek_hdl == seek_hdl || inst->seek_hdl == WFDS_SEEK_HDL_INVALID)) {
			instance = inst;
			break;
		}
	}

	return instance;
}

/* find a running or completed sd instance by ethernet address and seek handle */
static sd_instance_t *
wl_p2po_find_completed_instance(wl_p2po_info_t *p2po, struct ether_addr *addr, uint32 seek_hdl)
{
	sd_instance_t *instance = 0;
	sd_instance_t *inst;
	int i;

	/* search for addr and seek handle */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		inst = &p2po->instance[i];
		if (memcmp(addr, inst->addr.octet, sizeof(*addr)) == 0 &&
			(inst->seek_hdl == seek_hdl || inst->seek_hdl == WFDS_SEEK_HDL_INVALID) &&
			inst->state != SD_STATE_WAIT_TO_START) {
			instance = inst;
			break;
}
	}

	return instance;
}

/* find sd instance by seek hdl */
static sd_instance_t *
wl_p2po_find_instance_by_seek_hdl(wl_p2po_info_t *p2po, uint32 seek_hdl)
{
	sd_instance_t *instance = 0;
	int i;

	/* search for addr */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		if (p2po->instance[i].seek_hdl == seek_hdl) {
			instance = &p2po->instance[i];
			break;
		}
	}

	return instance;
}

/* initialize a sd instance */
static void
wl_p2po_sd_instance_init(sd_instance_t *instance)
{
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
	UNUSED_PARAMETER(eabuf);
#endif // endif

	instance->is_fragment = FALSE;
	instance->state = SD_STATE_WAIT_TO_START;
	instance->retries = 0;
	instance->seek_hdl = WFDS_SEEK_HDL_INVALID;
}

/* create a new sd instance */
static sd_instance_t *
wl_p2po_new_instance(wl_p2po_info_t *p2po, struct ether_addr *addr, uint32 seek_hdl)
{
	sd_instance_t *instance = 0;
	int i;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
	UNUSED_PARAMETER(eabuf);
#endif // endif

	/* Search for existing sd instance that matches the given P2P dev addr
	 * and seek handle.
	 */
	instance = wl_p2po_find_instance(p2po, addr, seek_hdl);
	if (instance != 0) {
		/* existing instance */
		if (instance->seek_hdl == WFDS_SEEK_HDL_INVALID &&
			seek_hdl != WFDS_SEEK_HDL_INVALID) {
			instance->seek_hdl = seek_hdl;
		}

		/* cancel the timer */
		wl_del_timer(p2po->wlc->wl, instance->tmr_instance_removal);
		/* restart the timer */
		wl_add_timer(p2po->wlc->wl, instance->tmr_instance_removal,
			P2PO_INSTANCE_TIMEOUT, 0);
		return instance;
	}

	/* Allocate a new sd instance */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		if (memcmp(&p2po->instance[i].addr, &ether_null, sizeof(ether_null)) == 0) {
			instance = &p2po->instance[i];
			wl_p2po_sd_instance_init(instance);
			memcpy(&instance->addr, addr, sizeof(*addr));
			instance->seek_hdl = seek_hdl;

			WL_P2PO(("%s:Allocated new instance for %s\n",
				__FUNCTION__,
				bcm_ether_ntoa(&instance->addr, eabuf)));

			/* start timer */
			wl_add_timer(p2po->wlc->wl, instance->tmr_instance_removal,
				P2PO_INSTANCE_TIMEOUT, 0);
			break;
		}
	}

	if (instance == 0) {
		WL_ERROR(("wl%d: %s: no space for new service discovery instance\n",
		          WLCUNIT(p2po), __FUNCTION__));
	}

	return instance;
}

/* find sd instance by gas */
static sd_instance_t *
wl_p2po_find_instance_by_gas(wl_p2po_info_t *p2po, bcm_gas_t *gas)
{
	sd_instance_t *instance = 0;
	int i;

	if (gas) {
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
			if (p2po->instance[i].gas == gas) {
				instance = &p2po->instance[i];
				break;
			}
		}
	}
	return instance;
}

/* sd instance reset */
static void
wl_p2po_sd_instance_reset(sd_instance_t *instance)
{
	/* destroy gas */
	if (instance->gas != 0) {
		bcm_gas_destroy(instance->gas);
		instance->gas = 0;
	}

	wl_p2po_sd_instance_init(instance);
	instance->state = SD_STATE_IDLE;
	memset(&instance->addr, 0, sizeof(ether_null));
}

/* sd instance delete */
static void
wl_p2po_sd_instance_delete(wl_p2po_info_t *p2po, sd_instance_t *instance)
{
	wl_del_timer(p2po->wlc->wl, instance->tmr_instance_removal);
	wl_p2po_sd_instance_reset(instance);
}

/* sd instance timeout handler */
static void
wl_p2po_sd_instance_timeout(void *arg)
{
	sd_instance_t *instance = (sd_instance_t *)arg;
	wl_p2po_info_t *p2po = instance->p2po;
	wlc_info_t *wlc = p2po->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
	UNUSED_PARAMETER(eabuf);
#endif // endif

	WL_P2PO(("%s: for peer %s\n",
		__FUNCTION__, bcm_ether_ntoa(&instance->addr, eabuf)));

	/* Generate an event to host */

	wlc_bss_mac_event(wlc, wl_disc_bsscfg(p2po->disci),
		WLC_E_P2PO_DEL_DEVICE, &instance->addr,
		0, 0, 0, NULL, 0);
	WL_INFORM(("%s:Device Removed\n", __FUNCTION__));

	wl_p2po_sd_instance_reset(instance);
}

/* handle incoming GAS query request frame */
static void
wl_p2po_process_query_request(wl_p2po_info_t *p2po, bcm_gas_t *gas, int len, uint8 *data)
{
	wlc_info_t *wlc = p2po->wlc;
	bcm_decode_t decodePkt, decodePkt2;
	bcm_decode_anqp_t anqp;
	bcm_decode_anqp_query_request_vendor_specific_tlv_t anqpVs;
	uint16 serviceUpdateIndicator;
	bcm_encode_t encodePkt, tlvData;
	int resp_len = 0;
	uint8 *resp_data = NULL;
	int bufferSize;
	uint8 *pktBuf = NULL, *tlvBuf = NULL;
	service_t *service;
	uint8 statusCode;
	uint32 size;

	WL_P2PO_TRACE(("rx ANQPreq\n"));

	/* decode received ANQP query request */
	bcm_decode_init(&decodePkt, len, data);
	if (!bcm_decode_anqp(&decodePkt, &anqp)) {
		WL_P2PO(("%s:decode error\n", __FUNCTION__));
		return;
	}

	/*
	 * Do a 1st decode pass to find the required ANQP response length.
	 */
	bcm_decode_init(&decodePkt2, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer);
	if (!bcm_decode_anqp_wfa_service_discovery(&decodePkt2, &serviceUpdateIndicator)) {
		WL_P2PO(("%s: decode error 2\n", __FUNCTION__));
		return;
	}
	bufferSize = 0;

	/* # of bytes added by bcm_encode_anqp_query_response_vendor_specific_tlv() */
#define ANQP_RSP_SVC_TLV_FIXED_SIZE 6
	/* # of bytes added by bcm_encode_anqp_wfa_service_discovery() */
#define ANQP_RSP_WFA_SD_FIXED_SIZE 10

	while (bcm_decode_remaining(&decodePkt2) > 0 &&
		bcm_decode_anqp_query_request_vendor_specific_tlv(&decodePkt2, &anqpVs) == TRUE) {

		service = wl_p2po_get_service(p2po, anqpVs.serviceProtocolType,
			anqpVs.queryData, anqpVs.queryLen, &statusCode);
		if (service) {
			bufferSize += service->response_len + ANQP_RSP_SVC_TLV_FIXED_SIZE;
		}

		/* Free memory allocated by wl_p2po_get_service() */
		if (p2po->wfds_resp_buf) {
			MFREE(WLCOSH(p2po), p2po->wfds_resp_buf, p2po->wfds_resp_buf_len);
			p2po->wfds_resp_buf = NULL;
			p2po->wfds_resp_buf_len = 0;
		}
	}
	bufferSize += ANQP_RSP_WFA_SD_FIXED_SIZE;

	/* Do a sanity check on the buffer length before proceeding */
	if (bufferSize > MAX_WFDS_ADV_SVC_INFO_LEN + 1000) {
		WL_P2PO(("%s: encode buffer too big: %u\n", __FUNCTION__, bufferSize));
		goto exit;
	}

	/*
	 * Do a 2nd decode pass which encodes the ANQP response.
	 */
	bcm_decode_init(&decodePkt2, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer);
	if (!bcm_decode_anqp_wfa_service_discovery(&decodePkt2, &serviceUpdateIndicator)) {
		WL_P2PO(("%s: decode error 2\n", __FUNCTION__));
		return;
	}

	/* prepare ANQP packet for response */
	pktBuf = MALLOC(wlc->osh, bufferSize);
	if (pktBuf == 0) {
		WL_ERROR(("wl%d: %s:MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto exit;
	}
	memset(pktBuf, 0, bufferSize);
	bcm_encode_init(&encodePkt, bufferSize, pktBuf);

	/* prepare buffer for vendor specific TLVs  */
	tlvBuf = MALLOC(wlc->osh, bufferSize);
	if (tlvBuf == 0) {
		WL_ERROR(("wl%d: %s:MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto exit;
	}
	memset(tlvBuf, 0, bufferSize);
	bcm_encode_init(&tlvData, bufferSize, tlvBuf);

	WL_P2PO(("%s:Ready to send Query response to peer\n", __FUNCTION__));

	/* decode service discovery query request TLVs */
	while (bcm_decode_remaining(&decodePkt2) > 0 &&
		bcm_decode_anqp_query_request_vendor_specific_tlv(&decodePkt2, &anqpVs) == TRUE) {

		/* get service */
		service = wl_p2po_get_service(p2po, anqpVs.serviceProtocolType,
			anqpVs.queryData, anqpVs.queryLen, &statusCode);
		do {
			/* TODO: need to check if packet buffer is big enough for encoding */
			/* encode ANQP vendor specific TLV */
			uint8 protocol;

			protocol = service ? service->protocol : anqpVs.serviceProtocolType;
			bcm_encode_anqp_query_response_vendor_specific_tlv(
				&tlvData,
				protocol,
				anqpVs.serviceTransactionId,
				statusCode,
				protocol == SVC_RPOTYPE_WFDS ? TRUE : FALSE,
				service ? service->num_service : 0,
				service ? service->response_len : 0,
				service ? service->response_data : NULL);

			/* get next service */
		} while (service && (service = wl_p2po_get_next_service(p2po)) != NULL);

		if (p2po->wfds_resp_buf) {
			MFREE(WLCOSH(p2po), p2po->wfds_resp_buf, p2po->wfds_resp_buf_len);
			p2po->wfds_resp_buf = NULL;
			p2po->wfds_resp_buf_len = 0;
		}
	}

	/* append vendor specific TLVs to ANQP query response */
	size = bcm_encode_anqp_wfa_service_discovery(&encodePkt,
		p2po->service_update_indicator,
		bcm_encode_length(&tlvData), bcm_encode_buf(&tlvData));

	(void) size;
	WL_P2PO_TRACE(("tx qry resp\n"));
	WL_P2PO_TRACE(("created qry resp: len=%u ", bcm_encode_length(&encodePkt)));
	if (bcm_encode_length(&encodePkt) > 10)
		WL_P2PO_TRACE_NAME("tail=...", 16,
			bcm_encode_buf(&tlvData) + bcm_encode_length(&tlvData) - 16);
	else
		WL_P2PO_TRACE(("\n"));

	/* prepare query response data */
	resp_len = bcm_encode_length(&encodePkt);
	resp_data = MALLOC(wlc->osh, resp_len);
	if (resp_data) {
		memcpy(resp_data, bcm_encode_buf(&encodePkt), resp_len);
		/* pass query response data to response handler */
		bcm_gas_set_bsscfg_index(gas, bcm_p2p_disc_get_bsscfg_index(p2po->disc));
		bcm_gas_set_query_response(gas, resp_len, resp_data);
	}
	else
		WL_ERROR(("wl%d: %s:MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));

exit:
	if (tlvBuf)
		MFREE(wlc->osh, tlvBuf, bufferSize);
	if (pktBuf)
		MFREE(wlc->osh, pktBuf, bufferSize);
	if (resp_data)
		MFREE(wlc->osh, resp_data, resp_len);
}

#if defined(BCMDBG) && !defined(DONGLEBUILD)
static void
_dump_buf(uint8 *buf, int len)
						{
	int i;
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("\n%02d:  ", i);
		printf("%02x ", buf[i]);
	}
	printf("\n");
}
#endif // endif

/* generate wl event */
static void
wl_p2po_gen_event(wl_p2po_info_t *p2po, sd_instance_t *instance, uint status,
	uint16 channel, uint8 dialog_token, uint8 fragment_id, uint16 status_code)
{
	wlc_info_t *wlc = p2po->wlc;
	int length = 0;
	uint8 *buffer = NULL;
	int rsp_length = 0;

	if (status != WLC_E_STATUS_FAIL)
		/* get query response/fragment length */
		rsp_length = bcm_gas_get_query_response_length(instance->gas);

	length = OFFSETOF(wl_event_gas_t, data) + rsp_length;
	buffer = MALLOC(wlc->osh, length);
	if (buffer != 0) {
		wl_event_gas_t *gas_data = (wl_event_gas_t *)buffer;
		int bufLen;

		/* initialize gas event data */
		memset(buffer, 0, length);
		gas_data->channel = channel;
		gas_data->dialog_token = dialog_token;
		gas_data->fragment_id = fragment_id;
		gas_data->status_code = status_code;
		gas_data->data_len = rsp_length;

		/* get query response/fragment */
		if (rsp_length && bcm_gas_get_query_response(instance->gas, rsp_length,
			&bufLen, gas_data->data)) {
			WL_INFORM(("%s:GAS fragment 0x%02x\n", __FUNCTION__, fragment_id));
				}
	} else {
		WL_ERROR(("wl%d: %s:MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto exit;
	}

	wlc_bss_mac_event(wlc, wl_disc_bsscfg(p2po->disci), WLC_E_GAS_FRAGMENT_RX, &instance->addr,
		status, 0, 0, buffer, length);
	WL_P2PO(("WLC_E_GAS_FRAGMENT_RX len=%u status=%d\n", length, status));
#ifdef BCMDBG
	dump_buf(buffer, length);
#endif // endif
exit:
	if (buffer)
	MFREE(wlc->osh, buffer, length);
}

/* callback to handle gas events */
static void
wl_p2po_gas_event_cb(void *context, bcm_gas_t *gas, bcm_gas_event_t *event)
{
	wl_p2po_info_t *p2po = (wl_p2po_info_t *)context;
	wlc_info_t *wlc = p2po->wlc;
	sd_instance_t *instance;
	char eabuf[ETHER_ADDR_STR_LEN];

	UNUSED_PARAMETER(eabuf);
#if defined(BCMDBG) && defined(DONGLEBUILD)
	WL_P2PO_TRACE(("%s: peer=%s type=%d stat=%d\n", __FUNCTION__,
		bcm_ether_ntoa(&event->peer, eabuf), event->type,
		event->status.statusCode));
#endif /* DONGLEBUILD */
	WL_P2PO(("%s: Event-->Peer %s gas %p type %d statusCode %d\n",
		__FUNCTION__,
		bcm_ether_ntoa(&event->peer, eabuf),
		event->gas,
		event->type,
		event->status.statusCode));

	if (event->type == BCM_GAS_EVENT_QUERY_REQUEST) {
		if (p2po->disc_mode != WL_P2PO_DISC_STOP)
			/* incoming GAS request - stop discovery */
			bcm_p2p_disc_reset(p2po->disc);

		wl_p2po_process_query_request(p2po, event->gas,
			event->queryReq.len, event->queryReq.data);
		return;
	}

	instance = wl_p2po_find_instance_by_gas(p2po, gas);
	if (instance != 0) {
		WL_P2PO(("%s:Found instance %s\n",
			__FUNCTION__,
			bcm_ether_ntoa(&instance->addr, eabuf)));
		if (event->type == BCM_GAS_EVENT_RESPONSE_FRAGMENT) {
			wl_p2po_gen_event(p2po, instance,
				WLC_E_STATUS_PARTIAL, instance->channel,
				event->dialogToken, event->rspFragment.fragmentId,
				DOT11_SC_SUCCESS);
		}
		else if (event->type == BCM_GAS_EVENT_STATUS) {

			/* abort dwell time */
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

			if (event->status.statusCode == DOT11_SC_SUCCESS) {
				/* event for last fragment */
				WL_P2PO(("%s: rx success GAS response\n", __FUNCTION__));
				wl_p2po_gen_event(p2po, instance,
					WLC_E_STATUS_SUCCESS,
					instance->channel, event->dialogToken, 0,
					event->status.statusCode);
				instance->state = SD_STATE_IDLE;
				instance->retries = 0;
			} else if (++instance->retries < p2po->gas_params.max_retries) {
				/* Retry ANQP request */
				WL_P2PO_TRACE(("%s: GAS retry %u\n", __FUNCTION__,
					instance->retries));
				instance->state = SD_STATE_WAIT_TO_START;
			} else {
				wl_p2po_gen_event(p2po, instance,
					WLC_E_STATUS_FAIL,
					instance->channel, event->dialogToken, 0,
					event->status.statusCode);
				instance->state = SD_STATE_IDLE;
				instance->retries = 0;
			}

			bcm_gas_destroy(instance->gas);
			instance->gas = 0;
		}
	}

	/* restart listen/discovery at the completion of GAS
	 * for both initiated and incoming GAS queries
	 */
	if (event->type == BCM_GAS_EVENT_STATUS) {
		if (p2po->disc_mode == WL_P2PO_DISC_LISTEN) {
			/* restart listen */
			bcm_p2p_disc_start_ext_listen(p2po->disc, p2po->listen.period,
				p2po->listen.interval - p2po->listen.period);
		}
		else if (p2po->disc_mode == WL_P2PO_DISC_DISCOVERY) {
			/* restart discovery */
			bcm_p2p_disc_start_discovery(p2po->disc);
		}
	}
}

/* callback to handle scan results */
static void
wl_p2po_event_cb(void *ctx,
	uint32 event_type, wl_event_msg_t *wl_event, uint8 *data, uint32 length)
{
	wl_p2po_info_t *p2po = (wl_p2po_info_t *)ctx;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
	UNUSED_PARAMETER(eabuf);
#endif // endif

	if (p2po == 0)
		return;

	if (event_type == WLC_E_ESCAN_RESULT && wl_event->status == WLC_E_STATUS_PARTIAL) {
		wl_escan_result_t *escan_data = (wl_escan_result_t *)data;

		if (length >= sizeof(*escan_data)) {
			wlc_info_t *wlc = p2po->wlc;
			wl_bss_info_t *bi = &escan_data->bss_info[0];
			uint8 *biData = (uint8 *)bi;
			struct ether_addr *addr;
			bcm_decode_p2p_t p2p;
			bcm_decode_t pkt;
			bcm_decode_ie_t ies;
			uint16 channel;
			bcm_decode_t dec1;
			uint8 *p2pBuffer;
			int p2pLength;

			sd_instance_t *instance;
			int bsscfgIndex;
			wl_p2po_seek_t* seek;
			bcm_encode_t encTlv;
			uint8 *tlvBuf = 0;
			int tlvBufLen;
			bcm_encode_t encVsc;
			uint8 *vscBuf = 0;
			int vscBufLen;
			bcm_decode_p2p_device_info_t deviceInfo;

			/* decode probe responses only */
			if ((bi->flags & WL_BSS_FLAGS_FROM_BEACON) != 0)
				return;

			/* default address */
			addr = &bi->BSSID;

			bcm_decode_init(&pkt, bi->ie_length, &biData[bi->ie_offset]);
			bcm_decode_ie(&pkt, &ies);

			/* channel from DS IE if available */
			channel = ies.dsLength == 1 ? *ies.ds :
				wf_chspec_ctlchan(bi->chanspec);

			/* check for P2P IEs */
			p2pLength = bcm_decode_ie_get_p2p_ie_length(&pkt, &ies);
			if (p2pLength == 0)
				return;

			/* concatenate and decode P2P IEs */
			p2pBuffer = MALLOC(wlc->osh, p2pLength);
			if (p2pBuffer == 0)
				return;
			bcm_decode_ie_get_p2p_ie(&pkt, &ies, p2pBuffer);
			bcm_decode_init(&dec1, p2pLength, p2pBuffer);
			if (!bcm_decode_p2p(&dec1, &p2p)) {
				WL_P2PO_TRACE(("%s: decode_p2p failed\n", __FUNCTION__));
				goto exit1;
			}

			/* decode P2P device info */
			if (p2p.deviceInfoBuffer) {
				bcm_decode_t dec2;

				bcm_decode_init(&dec2, p2p.deviceInfoLength,
					p2p.deviceInfoBuffer);
				if (bcm_decode_p2p_device_info(&dec2, &deviceInfo)) {
					/* use device address */
					addr = &deviceInfo.deviceAddress;
				}
			}

			if (p2p.capabilityBuffer) {
				bcm_decode_t dec3;
				bcm_decode_p2p_capability_t cap;

				bcm_decode_init(&dec3, p2p.capabilityLength,
					p2p.capabilityBuffer);
				if (!bcm_decode_p2p_capability(&dec3, &cap) ||
				    (cap.device & P2P_CAPSE_DEV_SERVICE_DIS) == 0) {
					/* service discovery not supported */
					WL_P2PO_TRACE(("%s: SD not supported\n", __FUNCTION__));
					goto exit1;
				}
			}

			/* Check if the probe response P2P IE has a Advertised
			 * Services Info attribute with a Advertised Services
			 * Descriptor containning a Service Name that matches
			 * one of our configured Seek service names.
			 */
			seek = wl_p2po_wfds_seek_match(p2po, addr,
				p2p.advertiseServiceLength, p2p.advertiseServiceBuffer);

			/* If our matching Seek service has a non-wildcard
			 * MAC address, check if the probe response's MAC
			 * address matches it.
			 */
			if (seek && !seek->macaddr_is_wildcard &&
				memcmp(seek->macaddr, addr, sizeof(*addr)) != 0) {
				WL_P2PO_TRACE(("%s: MAC mismatch "
					"%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
					addr->octet[0], addr->octet[1], addr->octet[2],
					addr->octet[3], addr->octet[4], addr->octet[5]));
				WL_P2PO_TRACE(("    looking for %02x:%02x:%02x:%02x:%02x:%02x\n",
					seek->macaddr[0], seek->macaddr[1], seek->macaddr[2],
					seek->macaddr[3], seek->macaddr[4], seek->macaddr[5]));
				goto exit1;
			}

			/* create new instance or find the existing instance */
			instance = wl_p2po_new_instance(p2po, addr, seek ?
				seek->seek_hdl : WFDS_SEEK_HDL_INVALID);
			if (instance == 0)
				goto exit1;

			if (instance->state != SD_STATE_WAIT_TO_START)
				goto exit1;

			WL_P2PO_TRACE(("p2po: prbrsp %02x:%02x:%02x:%02x:%02x:%02x ch=0x%x\n",
				addr->octet[0], addr->octet[1], addr->octet[2],
				addr->octet[3], addr->octet[4], addr->octet[5],
				bi->chanspec));

			if (seek == NULL) {
				/* WL_P2PO_TRACE(("p2po: no svc name match\n", __FUNCTION__)); */
				goto exit1;
			}
			/* save channel */
			instance->channel = channel;

			bsscfgIndex = bcm_p2p_disc_get_bsscfg_index(p2po->disc);

			/* stop discovery */
			bcm_p2p_disc_reset(p2po->disc);

			/* 4 = 2bytes(length) + 1byte(service protocol type) +
			 * 1byte(service transaction id)
			 */
			tlvBufLen = seek->query_data_len + 4;
			tlvBuf = MALLOC(wlc->osh, tlvBufLen);
			if (tlvBuf == 0) {
				WL_ERROR(("wl%d: %s:MALLOC failed\n",
					WLCWLUNIT(wlc), __FUNCTION__));
				goto exit;
			}

			WL_P2PO_TRACE(("%s: matched prbrsp %02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__,
				addr->octet[0], addr->octet[1], addr->octet[2],
				addr->octet[3], addr->octet[4], addr->octet[5]));

			/* encode service discovery query */
			bcm_encode_init(&encTlv, tlvBufLen, tlvBuf);
			/* Encode the Query Data with the WFDS protocol ID,
			 * a unique transaction ID, and the prebuilt Seek anqp
			 * query data
			 */
			++p2po->transaction_id;
			bcm_encode_anqp_query_request_vendor_specific_tlv(&encTlv,
				SVC_RPOTYPE_WFDS, p2po->transaction_id,
				seek->query_data_len, seek->query_data);

			/* 10 = 2bytes(info id) + 2bytes(length) + 3bytes(OI) +
			 * 1byte(subtype) + 2bytes(srevice update indicator)
			 */
			vscBufLen = tlvBufLen + 10;
			vscBuf = MALLOC(wlc->osh, vscBufLen);
			if (vscBuf == 0) {
				WL_ERROR(("wl%d: %s:MALLOC failed\n",
					WLCWLUNIT(wlc), __FUNCTION__));
				goto exit;
			}
			bcm_encode_init(&encVsc, vscBufLen, vscBuf);
			bcm_encode_anqp_wfa_service_discovery(
				&encVsc, p2po->service_update_indicator,
				bcm_encode_length(&encTlv), bcm_encode_buf(&encTlv));

			/* create query handler */
			instance->gas = bcm_gas_create((struct bcm_gas_wl_drv_hdl *)wlc,
				bsscfgIndex, channel, addr);
			if (instance->gas == 0) {
				WL_ERROR(("wl%d: %s: bcm_gas_create failed\n",
					WLCWLUNIT(wlc), __FUNCTION__));
				goto exit;
			}
			WL_P2PO(("%s:Instance %s gas %p\n",
				__FUNCTION__,
				bcm_ether_ntoa(&instance->addr, eabuf),
				instance->gas));
			bcm_gas_set_bsscfg_index(instance->gas, bsscfgIndex);

			/* configure GAS parameters */
			bcm_gas_set_max_retransmit(instance->gas,
				p2po->gas_params.max_retransmit);
			bcm_gas_set_max_comeback_delay(instance->gas,
				p2po->gas_params.max_comeback_delay);
			bcm_gas_set_response_timeout(instance->gas,
				p2po->gas_params.response_timeout);

			/* set up query */
			bcm_gas_set_query_request(instance->gas,
				bcm_encode_length(&encVsc), bcm_encode_buf(&encVsc));

			/* start query handler */
			WL_P2PO_TRACE(("%s: tx query req: bssidx=%u len=%u seekhdl=%u\n",
				__FUNCTION__, bsscfgIndex,
				bcm_encode_length(&encVsc), seek->seek_hdl));
			bcm_gas_start(instance->gas);

			instance->state = SD_STATE_RUNNING;

exit:
			if (tlvBuf)
				MFREE(wlc->osh, tlvBuf, tlvBufLen);
			if (vscBuf)
				MFREE(wlc->osh, vscBuf, vscBufLen);

exit1:
			if (p2pBuffer)
				MFREE(wlc->osh, p2pBuffer, p2pLength);
		}
	}

	/* invoke discovery state machine */
	bcm_p2p_disc_process_wlan_event(0, event_type, wl_event, data, length);
}

/*
 * initialize p2po private context.
 * returns a pointer to the p2po private context, NULL on failure.
 */
wl_p2po_info_t *
BCMATTACHFN(wl_p2po_attach)(wlc_info_t *wlc, wl_eventq_info_t *wlevtq,
	wl_gas_info_t *gas, wl_disc_info_t *p2po_disc)
{
	wl_p2po_info_t *p2po;
	int i;

	/* allocate p2po private info struct */
	p2po = MALLOC(wlc->osh, sizeof(wl_p2po_info_t));
	if (!p2po) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init p2po private info struct */
	bzero(p2po, sizeof(wl_p2po_info_t));
	p2po->wlc = wlc;
	p2po->wlevtqi = wlevtq;
	p2po->gasi = gas;
	p2po->disci = p2po_disc;
	p2po->disc_mode = WL_P2PO_DISC_STOP;

	/* default listen timing */
	p2po->listen.period = P2PO_LISTEN_PERIOD;
	p2po->listen.interval = P2PO_LISTEN_INTERVAL;
	p2po->sd_recheck_timeout = P2PO_SD_RECHECK_TIMEOUT;

	/* default GAS parameters */
	p2po->gas_params.max_retransmit = P2PO_DEFAULT_MAX_RETRANSMIT;
	p2po->gas_params.response_timeout = P2PO_DEFAULT_RESPONSE_TIMEOUT;
	p2po->gas_params.max_comeback_delay = P2PO_DEFAULT_MAX_COMEBACK_DELAY;
	p2po->gas_params.max_retries = P2PO_DEFAULT_MAX_RETRIES;

	/* initialize discovery state machine persistent data */
	bcm_p2p_disc_config_init();

	/* disable gas for incoming request by default */
	bcm_gas_incoming_request(FALSE);

	/* create p2po instance timers */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		sd_instance_t *instance = &p2po->instance[i];
		instance->p2po = p2po;
		instance->tmr_instance_removal = wl_init_timer(wlc->wl,
			wl_p2po_sd_instance_timeout, instance,
			"wl_p2po_sd_instance_removal_timeout");
	}

	/* register module */
	if (wlc_module_register(wlc->pub, p2po_iovars, "p2po", p2po, p2po_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return NULL;
	}

	wlc->pub->_p2po = TRUE;

	return p2po;
}

/* cleanup p2po private context */
void
BCMATTACHFN(wl_p2po_detach)(wl_p2po_info_t *p2po)
{
	int i;

	WL_INFORM(("wl%d: p2po_detach()\n", WLCUNIT(p2po)));

	if (!p2po)
		return;

	/* No need to forward the events up, so its safe to set the event mask bit to 0 */
	wlc_eventq_set_ind(p2po->wlc->eventq, WLC_E_SERVICE_FOUND, 0);
	wlc_eventq_set_ind(p2po->wlc->eventq, WLC_E_P2PO_ADD_DEVICE, 0);
	wlc_eventq_set_ind(p2po->wlc->eventq, WLC_E_P2PO_DEL_DEVICE, 0);

	p2po->wlc->pub->_p2po = FALSE;

	/* destroy p2po instance timers */
	for (i = 0; i < MAX_SD_INSTANCE; i++) {
		sd_instance_t *instance = &p2po->instance[i];
		wl_del_timer(p2po->wlc->wl, instance->tmr_instance_removal);
		wl_free_timer(p2po->wlc->wl, instance->tmr_instance_removal);
	}

	if (p2po->disc) {
		bcm_p2p_disc_destroy(p2po->disc);
		p2po->disc = NULL;
	}
	bcm_p2p_disc_config_cleanup((struct bcm_p2p_wl_drv_hdl *)p2po->wlc);

	/* Delete all services to seek and advertise */
	wl_p2po_wfds_del_all_svcs(p2po);

	/* Delete probreq Service Hash P2P IE, free vndr IE data */
	wl_p2po_upd_seek_svc_hash(p2po);

	wlc_module_unregister(p2po->wlc->pub, "p2po", p2po);
	MFREE(WLCOSH(p2po), p2po, sizeof(wl_p2po_info_t));

	p2po = NULL;
}

/* handling p2po-related iovars */
static int
p2po_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_p2po_info_t *p2po = hdl;
	int err = BCME_OK;

	ASSERT(p2po);

#ifndef BCMROMOFFLOAD
	WL_INFORM(("wl%d: p2po_doiovar()\n", WLCUNIT(p2po)));
#endif /* !BCMROMOFFLOAD */

	/* Do nothing if p2p is not yet enabled */
	if (!P2P_ENAB(p2po->wlc->pub)) {
		WL_P2PO_TRACE(("p2po_doiovar: P2P not enabled\n"));
		return BCME_UNSUPPORTED;
	}

	switch (actionid) {
	case IOV_SVAL(IOV_P2PO_LISTEN):
		err = wl_p2po_listen(p2po, a, alen);
		break;
	case IOV_GVAL(IOV_P2PO_LISTEN):
		bcopy(&p2po->listen, a, sizeof(p2po->listen));
		break;
	case IOV_SVAL(IOV_P2PO_FIND):
		err = wl_p2po_find(p2po);
		break;
	case IOV_SVAL(IOV_P2PO_STOP):
		err = wl_p2po_stop(p2po);
		break;
	case IOV_GVAL(IOV_P2PO_STATE):
		*((uint32*)a) = wl_p2po_state(p2po);
		break;
	case IOV_SVAL(IOV_P2PO_SD_RECHECK_TIMEOUT):
		p2po->sd_recheck_timeout = (*(int32 *)a);
		break;
	case IOV_SVAL(IOV_P2PO_LISTEN_CHANNEL):
		err = wl_p2po_listen_channel(p2po, a, alen);
		break;
	case IOV_GVAL(IOV_P2PO_LISTEN_CHANNEL):
		*((uint32*)a) = p2po->listen_channel;
		break;
	case IOV_SVAL(IOV_P2PO_FIND_CONFIG):
		err = wl_p2po_set_find_config(p2po, a, alen);
		break;
	case IOV_GVAL(IOV_P2PO_FIND_CONFIG):
		err = wl_p2po_get_find_config(p2po, a, alen);
		break;

	case IOV_SVAL(IOV_P2PO_GAS_CONFIG):
		err = wl_p2po_gas_config(p2po, a, alen);
		break;
	case IOV_GVAL(IOV_P2PO_GAS_CONFIG):
		bcopy(&p2po->gas_params, a, sizeof(p2po->gas_params));
		break;
	case IOV_GVAL(IOV_P2PO_WFDS_SEEK_ADD):
		err = wl_p2po_wfds_seek_get(p2po, p, plen, a, alen);
		break;
	case IOV_SVAL(IOV_P2PO_WFDS_SEEK_ADD):
		err = wl_p2po_wfds_seek_add(p2po, a, alen);
		break;
	case IOV_SVAL(IOV_P2PO_WFDS_SEEK_DEL):
		err = wl_p2po_wfds_seek_del(p2po, a, alen);
		break;
	case IOV_GVAL(IOV_P2PO_WFDS_ADVERT_ADD):
		err = wl_p2po_wfds_advert_get(p2po, p, plen, a, alen);
		break;
	case IOV_SVAL(IOV_P2PO_WFDS_ADVERT_ADD):
		err = wl_p2po_wfds_advert_add(p2po, a, alen);
		break;
	case IOV_SVAL(IOV_P2PO_WFDS_ADVERT_DEL):
		err = wl_p2po_wfds_advert_del(p2po, a, alen);
		break;

#if defined(BCMDBG)
	case IOV_SVAL(IOV_P2PO_WFDS_SEEK_DUMP):
		err = wl_p2po_wfds_seek_dump(p2po);
		break;
	case IOV_SVAL(IOV_P2PO_WFDS_ADVERT_DUMP):
		err = wl_p2po_wfds_advert_dump(p2po);
		break;
#endif /* BCMDBG */
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	case IOV_SVAL(IOV_P2PO_TRACELEVEL):
		err = wl_p2po_tracelevel(p2po, a, alen);
		break;
#endif /* BCMDBG || BCMDBG_ERR */
	default:
		WL_P2PO_TRACE(("%s: unsupported iovar %u\n", __FUNCTION__, actionid));
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* prepare listen channel */
static uint8
wl_p2po_prepare_listen_channel(wl_p2po_info_t *p2po)
{
	wlc_info_t *wlc = p2po->wlc;

	/* use specified listen channel if set */
	if (p2po->listen_channel)
		return p2po->listen_channel;

	/* if primary interface is associated, use associated channel if it is a social channel */
	if (wlc->pub->associated) {
		uint8	channel;
		channel = wf_chspec_ctlchan(wlc_get_home_chanspec(wlc_bsscfg_primary(wlc)));
		if (channel == 1 || channel == 6 || channel == 11)
			return channel;
	}

	/* use default listen channel */
	return P2PO_DEFAULT_LISTEN_CHANNEL;
}

/* start listen with extended listen timing parameters which are
 * expected to be the same as extented listen timing SE of P2P IE
 */
static int
wl_p2po_listen(wl_p2po_info_t *p2po, void *arg, int len)
{
	wlc_info_t *wlc = p2po->wlc;
	wl_p2po_listen_t *listen = (wl_p2po_listen_t *)arg;
	uint32 events[] = { WLC_E_ESCAN_RESULT };
	uint8 listen_channel;

	WL_P2PO(("%s:Entry\n", __FUNCTION__));

	WL_P2PO_TRACE(("%s: period=%u int=%u\n", __FUNCTION__, listen->period, listen->interval));
	if (listen->period && listen->interval) {
		p2po->listen.period = listen->period;
		p2po->listen.interval = listen->interval;
	}
	else {
		p2po->listen.period = P2PO_LISTEN_PERIOD;
		p2po->listen.interval = P2PO_LISTEN_INTERVAL;
	}

	/* make sure interval is greater or equal to period */
	if (p2po->listen.interval < p2po->listen.period)
		p2po->listen.interval = p2po->listen.period;

	/* create discovery instance on home listen channel */
	if (!p2po->disc) {
		if (wl_disc_bsscfg(p2po->disci)) {
			listen_channel = wl_p2po_prepare_listen_channel(p2po);
			WL_P2PO_TRACE(("%s: li_ch=%u\n", __FUNCTION__, listen_channel));
			p2po->disc = bcm_p2p_disc_create((struct bcm_p2p_wl_drv_hdl *)wlc,
				listen_channel);
		} else {
			WL_P2PO_TRACE(("%s: !p2po->disc\n", __FUNCTION__));
			return BCME_ERROR;
		}
	}

	/* Don't send the LISTEN_COMPLETE event to the host for the p2po_find/listen */
	wlc_eventq_set_ind(wlc->eventq, WLC_E_P2P_DISC_LISTEN_COMPLETE, 0);

	bcm_gas_subscribe_event(p2po, wl_p2po_gas_event_cb);
	/* enable gas for incoming request */
	bcm_gas_incoming_request(TRUE);

	/* register callback and events for local event q */
	if (wl_eventq_register_event_cb(p2po->wlevtqi,
		events, sizeof(events)/sizeof(int32), wl_p2po_event_cb, (void *)p2po) != BCME_OK)
		return BCME_ERROR;

	if (wl_gas_start_eventq(p2po->gasi) != BCME_OK)
		return BCME_ERROR;

	bcm_p2p_disc_reset(p2po->disc);
	p2po->disc_mode = WL_P2PO_DISC_LISTEN;

	/* start listen */
	bcm_p2p_disc_start_ext_listen(p2po->disc,
		p2po->listen.period, p2po->listen.interval - p2po->listen.period);

	return BCME_OK;
}

/* start discovery */
static int
wl_p2po_find(wl_p2po_info_t *p2po)
{
	wlc_info_t *wlc = p2po->wlc;
	uint32 events[] = { WLC_E_ESCAN_RESULT };
	uint8 listen_channel;

	/* create discovery instance on home listen channel */
	WL_P2PO(("%s:Entry\n", __FUNCTION__));

	if (!p2po->disc) {
		if (wl_disc_bsscfg(p2po->disci)) {
			listen_channel = wl_p2po_prepare_listen_channel(p2po);
			WL_P2PO_TRACE(("%s: soc_ch=%u %u %u, li_ch=%u\n", __FUNCTION__,
				listen_channel));
			p2po->disc = bcm_p2p_disc_create((struct bcm_p2p_wl_drv_hdl *)wlc,
				listen_channel);
		} else {
			WL_P2PO_TRACE(("%s: !wl_disc_bsscfg\n", __FUNCTION__));
			return BCME_ERROR;
		}
	}
	wlc_eventq_set_ind(wlc->eventq, WLC_E_SERVICE_FOUND, 1);
	wlc_eventq_set_ind(wlc->eventq, WLC_E_P2PO_ADD_DEVICE, 1);
	wlc_eventq_set_ind(wlc->eventq, WLC_E_P2PO_DEL_DEVICE, 1);

	/* Don't send the LISTEN_COMPLETE event to the host for the p2po_find/listen */
	wlc_eventq_set_ind(wlc->eventq, WLC_E_P2P_DISC_LISTEN_COMPLETE, 0);

	bcm_gas_subscribe_event(p2po, wl_p2po_gas_event_cb);
	/* enable gas for incoming request */
	bcm_gas_incoming_request(TRUE);

	/* register callback and events for local event q */
	if (wl_eventq_register_event_cb(p2po->wlevtqi,
		events, sizeof(events)/sizeof(*events), wl_p2po_event_cb, (void *)p2po) != BCME_OK)
		return BCME_ERROR;

	if (wl_gas_start_eventq(p2po->gasi) != BCME_OK)
		return BCME_ERROR;

	bcm_p2p_disc_reset(p2po->disc);
	p2po->disc_mode = WL_P2PO_DISC_DISCOVERY;

	/* start discovery and query */
	bcm_p2p_disc_start_discovery(p2po->disc);

	return BCME_OK;
}

/* stop discovery/listen */
static int
wl_p2po_stop(wl_p2po_info_t *p2po)
{
	wlc_info_t *wlc = p2po->wlc;

	WL_P2PO(("%s: Entry p2po->disc %p\n", __FUNCTION__, p2po->disc));

	if (p2po->disc) {
		int i;

		/* turn off event forwarding */
		wlc_eventq_set_ind(wlc->eventq, WLC_E_SERVICE_FOUND, 0);
		wlc_eventq_set_ind(wlc->eventq, WLC_E_P2PO_ADD_DEVICE, 0);
		/* Need the delete event up, even after issue of p2po_stop find */
		/* wlc_eventq_set_ind(p2po->wlc->eventq, WLC_E_P2PO_DEL_DEVICE, 0); */

		/* Enable back the disabled events */
		wlc_eventq_set_ind(wlc->eventq, WLC_E_P2P_DISC_LISTEN_COMPLETE, 1);

		wl_gas_stop_eventq(p2po->gasi);

		wl_eventq_unregister_event_cb(p2po->wlevtqi, wl_p2po_event_cb);

		bcm_gas_unsubscribe_event(wl_p2po_gas_event_cb);
		/* disable gas for incoming request */
		bcm_gas_incoming_request(FALSE);

		p2po->disc_mode = WL_P2PO_DISC_STOP;

		bcm_p2p_disc_reset(p2po->disc);

		/* clean up instances */
		for (i = 0; i < MAX_SD_INSTANCE; i++) {
			sd_instance_t *instance = &p2po->instance[i];
			wl_p2po_sd_instance_delete(p2po, instance);
		}
		/* We need to free the discovery because in next find,
		* we may have different listen channel
		*/
		bcm_p2p_disc_destroy(p2po->disc);
		p2po->disc = NULL;
	}

	return BCME_OK;
}

/* Return whether p2po find/listen is running */
static uint32
wl_p2po_state(wl_p2po_info_t *p2po)
{
	uint32 disc_mode = WL_P2PO_DISC_STOP;

	WL_P2PO(("%s: Entry p2po->disc %p\n", __FUNCTION__, p2po->disc));

	if (p2po->disc)
		disc_mode = p2po->disc_mode;

	return disc_mode;
}

/* set listen channel */
static int
wl_p2po_listen_channel(wl_p2po_info_t *p2po, void *arg, int len)
{
	uint32 channel = *(uint32 *)arg;

	/* 0 is to use associated channel of primary interface if it is a social channel,
	 * otherwise use default listen channel(P2PO_DEFAULT_LISTEN_CHANNEL)
	 */
	p2po->listen_channel = channel;
	WL_P2PO_TRACE(("%s: ch=%u\n", __FUNCTION__, channel));

	return BCME_OK;
}

static int
wl_p2po_set_find_config(wl_p2po_info_t *p2po, void *arg, int len)
{
	wl_p2po_find_config_t *fc = (wl_p2po_find_config_t*)arg;
	int i;
	int ret = BCME_OK;

	UNUSED_PARAMETER(len);

	/* Check if the iovar structure is compatible */
	if (fc->length != sizeof(*fc)) {
		WL_ERROR(("wl%d: %s: bad struct length %u, expect %u\n",
			WLCUNIT(p2po), __FUNCTION__, fc->length, sizeof(*fc)));
		ret = BCME_VERSION;
		goto exit;
	}
	if (fc->version != WL_P2PO_FIND_CONFIG_VERSION) {
		WL_ERROR(("wl%d: %s: bad struct version %u, expect %u\n",
			WLCUNIT(p2po), __FUNCTION__, fc->version,
			WL_P2PO_FIND_CONFIG_VERSION));
		ret = BCME_VERSION;
		goto exit;
	}

	/* Validate the social channels list */
	if (fc->num_social_channels > WL_P2P_SOCIAL_CHANNELS_MAX) {
		WL_ERROR(("wl%d: %s: too many channels\n", WLCUNIT(p2po), __FUNCTION__));
		ret = BCME_RANGE;
		goto exit;
	}
	for (i = 0; i < fc->num_social_channels; i++) {
		if (!wlc_valid_chanspec_db(p2po->wlc->cmi,
			CH20MHZ_CHSPEC(fc->social_channels[i]))) {
			WL_ERROR(("wl%d: %s: bad channel %u\n",
				WLCUNIT(p2po), __FUNCTION__, fc->social_channels[i]));
			ret = BCME_BADCHAN;
			goto exit;
		}
	}

	/* Store the find config parameters */
	ret = bcm_p2p_disc_config_set((struct bcm_p2p_wl_drv_hdl *)p2po->wlc,
		fc->search_home_time, fc->flags, fc->num_social_channels,
		fc->social_channels);

exit:
	return ret;
}

static int
wl_p2po_get_find_config(wl_p2po_info_t *p2po, void *arg, int len)
{
	wl_p2po_find_config_t *fc = (wl_p2po_find_config_t*)arg;
	uint16 *social_channels;
	int ret;

	UNUSED_PARAMETER(len);

	ret = bcm_p2p_disc_config_get(&fc->search_home_time, &fc->flags,
		&fc->num_social_channels, &social_channels);
	if (ret != BCME_OK)
		return ret;

	memcpy(fc->social_channels, social_channels,
		fc->num_social_channels * sizeof(*social_channels));
	fc->version = WL_P2PO_FIND_CONFIG_VERSION;
	fc->length = sizeof(*fc);

	return BCME_OK;
}

/* Set GAS tunable parameters */
static int
wl_p2po_gas_config(wl_p2po_info_t *p2po, void *arg, int len)
{
	wl_gas_config_t *set = (wl_gas_config_t*) arg;

	if (len != sizeof(p2po->gas_params)) {
		WL_ERROR(("wl%d: %s: bad iovar len %u, expect %u\n",
			WLCUNIT(p2po), __FUNCTION__, len, sizeof(p2po->gas_params)));
		return BCME_BADLEN;
	}

	if (set->max_retransmit == P2PO_GAS_PARAM_DEFAULT)
		p2po->gas_params.max_retransmit = P2PO_DEFAULT_MAX_RETRANSMIT;
	else
		p2po->gas_params.max_retransmit = set->max_retransmit;

	if (set->response_timeout == P2PO_GAS_PARAM_DEFAULT)
		p2po->gas_params.response_timeout = P2PO_DEFAULT_RESPONSE_TIMEOUT;
	else
		p2po->gas_params.response_timeout = set->response_timeout;

	if (set->max_comeback_delay == P2PO_GAS_PARAM_DEFAULT)
		p2po->gas_params.max_comeback_delay = P2PO_DEFAULT_MAX_COMEBACK_DELAY;
	else
		p2po->gas_params.max_comeback_delay = set->max_comeback_delay;

	if (set->max_retries == P2PO_GAS_PARAM_DEFAULT)
		p2po->gas_params.max_retries = P2PO_DEFAULT_MAX_RETRIES;
	else
		p2po->gas_params.max_retries = set->max_retries;

	WL_P2PO_TRACE(("%s: rtran=%u rtmo=%u cbdel=%u rtries=%u\n", __FUNCTION__,
		p2po->gas_params.max_retransmit, p2po->gas_params.response_timeout,
		p2po->gas_params.max_comeback_delay, p2po->gas_params.max_retries));
	return BCME_OK;
}

/* "vndr_ie" iovar template for adding a P2P IE with a Service Hash attribute */
static uint8 svc_hash_p2p_ie_vndr_ie_template[] = {
	'a', 'd', 'd', 0,
	0x1, 0x0, 0x0, 0x0,			/* ie count */
	/* vendor ie 0 */
	VNDR_IE_PRBREQ_FLAG, 0x0, 0x0, 0x0,	/* flags */
	DOT11_MNG_VS_ID,			/* P2P IE */
	13,					/* IE length */
	0x50, 0x6f, 0x9a, WFA_OUI_TYPE_P2P,	/* WFA OUI + type */
	P2P_SEID_SERVICE_HASH,			/* Service Hash SE */
	6, 0,					/* Service Hash SE: length */
	1, 2, 3, 4, 5, 6			/* Service Hash SE: first service hash */
};
/* Offset of the first service hash in the above template */
#define SVC_HASH_TEMPLATE_IE_LEN_OFFSET 13
#define SVC_HASH_TEMPLATE_HASH_LEN_OFFSET 19
#define SVC_HASH_TEMPLATE_FIRST_HASH_OFFSET 21

/** To prevent ROMming shdat issue because of ROMmed functions accessing RAM */
static uint8* BCMRAMFN(get_svc_hash_p2p_ie_vndr_ie_template)(void)
{
	return (uint8 *)svc_hash_p2p_ie_vndr_ie_template;
}

/* Update the Probe Request P2P IE Service Hash attribute at a Seeker */
static int
wl_p2po_upd_seek_svc_hash(wl_p2po_info_t *p2po)
{
	wlc_info_t *wlc = p2po->wlc;
	wlc_bsscfg_t *disc_cfg;
	uint8 num_hashes;
	uint8 *dest;
	int i;
	int ret = BCME_OK;

	disc_cfg = wl_disc_bsscfg(p2po->disci);

	/* Delete the previous probe request Service Hash P2P IE if there is one */
	if (p2po->prbreq_svc_hash_vndr_ie) {
		memcpy(p2po->prbreq_svc_hash_vndr_ie, "del", 3);
		if (disc_cfg) {
			wlc_iovar_op(wlc, "vndr_ie", NULL, 0, p2po->prbreq_svc_hash_vndr_ie,
				p2po->prbreq_svc_hash_vndr_ie_len, IOV_SET, disc_cfg->wlcif);
		} else {
			WL_P2PO_TRACE(("%s: no discovery, do not del vndr_ie\n", __FUNCTION__));
		}

		/* Free the previous Service Hash P2P IE vndr_ie buffer */
		MFREE(WLCOSH(p2po), p2po->prbreq_svc_hash_vndr_ie,
			p2po->prbreq_svc_hash_vndr_ie_len);
		p2po->prbreq_svc_hash_vndr_ie = NULL;
		p2po->prbreq_svc_hash_vndr_ie_len = 0;
	}

	/* If we have no seek services then do not add a new Service Hash P2P IE */
	if (p2po->num_seek_svcs == 0) {
		/* WL_P2PO_TRACE(("%s: no new vndr_ie to add\n", __FUNCTION__)); */
		return ret;
	}

	/* Allocate a new Service Hash P2P IE vndr_ie buffer with a recalculated size */
	p2po->prbreq_svc_hash_vndr_ie_len = sizeof(svc_hash_p2p_ie_vndr_ie_template)
		+ (P2P_WFDS_HASH_LEN * (p2po->num_seek_svcs - 1));
	p2po->prbreq_svc_hash_vndr_ie = MALLOC(WLCOSH(p2po),
		p2po->prbreq_svc_hash_vndr_ie_len);
	if (!p2po->prbreq_svc_hash_vndr_ie) {
		WL_ERROR(("wl%d: %s: no mem\n", WLCUNIT(p2po), __FUNCTION__));
		return BCME_NOMEM;
	}

	/* Create the new Service Hash P2P IE vndr_ie iovar data from a template.
	 * Copy all seek service hashes to it.
	 */
	memcpy(p2po->prbreq_svc_hash_vndr_ie, get_svc_hash_p2p_ie_vndr_ie_template(),
		p2po->prbreq_svc_hash_vndr_ie_len);
	num_hashes = 0;
	for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
		if (p2po->seek_svcs[i] == NULL)
			continue;
		if (num_hashes >= p2po->num_seek_svcs) {
			WL_P2PO_TRACE(("%s: too many hashes! %u > %u\n", __FUNCTION__,
				num_hashes, p2po->num_seek_svcs));
			break;
		}
		dest = p2po->prbreq_svc_hash_vndr_ie + SVC_HASH_TEMPLATE_FIRST_HASH_OFFSET
			+ (num_hashes * P2P_WFDS_HASH_LEN);
		memcpy(dest, p2po->seek_svcs[i]->svc_hash, P2P_WFDS_HASH_LEN);
		++num_hashes;

		/* Debug only: sanity check */
		if (dest + P2P_WFDS_HASH_LEN >
			p2po->prbreq_svc_hash_vndr_ie + p2po->prbreq_svc_hash_vndr_ie_len) {
			WL_P2PO_TRACE(("%s: dst > src! dst=%p #=%u vndr=%p vndrlen=%u\n",
				__FUNCTION__, dest, num_hashes,
				p2po->prbreq_svc_hash_vndr_ie,
				p2po->prbreq_svc_hash_vndr_ie_len));
		}
	}

	/* If no hashes were added, return without adding the P2P IE */
	if (num_hashes == 0) {
		MFREE(WLCOSH(p2po), p2po->prbreq_svc_hash_vndr_ie,
			p2po->prbreq_svc_hash_vndr_ie_len);
		p2po->prbreq_svc_hash_vndr_ie = NULL;
		p2po->prbreq_svc_hash_vndr_ie_len = 0;
		return ret;
	}

	/* Update the length fields in the P2P IE */
	p2po->prbreq_svc_hash_vndr_ie[SVC_HASH_TEMPLATE_IE_LEN_OFFSET] +=
		(num_hashes - 1) * P2P_WFDS_HASH_LEN;
	p2po->prbreq_svc_hash_vndr_ie[SVC_HASH_TEMPLATE_HASH_LEN_OFFSET] +=
		(num_hashes - 1) * P2P_WFDS_HASH_LEN;

	/* Add the new probe request Service Hash P2P IE */
	if (disc_cfg) {
		ret = wlc_iovar_op(wlc, "vndr_ie", NULL, 0, p2po->prbreq_svc_hash_vndr_ie,
			p2po->prbreq_svc_hash_vndr_ie_len, IOV_SET, disc_cfg->wlcif);
	} else {
		WL_P2PO_TRACE(("%s: no discovery, do not add vndr_ie\n", __FUNCTION__));
	}

	return ret;
}

/* Get a configured WFDS service */
static int
wl_p2po_wfds_seek_get(wl_p2po_info_t *p2po, void *p, int plen, void *a, int alen)
{
	int i;
	bool found;
	wl_p2po_wfds_seek_add_t* in = (wl_p2po_wfds_seek_add_t*) p;
	wl_p2po_wfds_seek_add_t* out = (wl_p2po_wfds_seek_add_t*) a;
	wl_p2po_seek_t* seek;
	int seek_len;

	ASSERT(p2po);
	ASSERT(a);
	ASSERT(alen > 0);

	if (in->seek_hdl == WFDS_SEEK_HDL_INVALID) {
		WL_ERROR(("wl%d: %s: invalid seek hdl %u\n",
			WLCUNIT(p2po), __FUNCTION__, in->seek_hdl));
		return BCME_BADARG;
	}

	/* Search for a configured seek with a matching seek hdl */
	found = FALSE;
	for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
		seek = p2po->seek_svcs[i];
		if (seek != NULL && seek->seek_hdl == in->seek_hdl) {
			found = TRUE;
			break;
		}
	}
	if (!found) {
		WL_ERROR(("wl%d: %s: seek hdl %u not found\n",
			WLCUNIT(p2po), __FUNCTION__, in->seek_hdl));
		return BCME_NOTFOUND;
	}

	seek_len = sizeof(*seek) + seek->svc_info_req_len - 1;
	if (alen < seek_len) {
		return BCME_BUFTOOSHORT;
	}

	/* Copy the seek data */
	out->seek_hdl = seek->seek_hdl;
	memcpy(out->addr, seek->macaddr, sizeof(seek->macaddr));
	memcpy(out->service_hash, seek->svc_hash, sizeof(seek->macaddr));
	out->service_name_len = seek->svc_name_len;
	memcpy(out->service_name, &seek->query_data[1], seek->svc_name_len);
	out->service_info_req_len = seek->svc_info_req_len;
	memcpy(out->service_info_req, &seek->query_data[1 + seek->svc_name_len + 1],
		seek->svc_info_req_len);

	WL_P2PO_TRACE(("%s: #=%u i=%d hdl=%u m,n,i,q_len=%u,%u,%u,%u\n", __FUNCTION__,
		p2po->num_seek_svcs, i, seek->seek_hdl,
		p2po->seek_svcs_len[i], seek->svc_name_len,
		seek->svc_info_req_len, seek->query_data_len));

	return BCME_OK;
}

/* Add a WFDS service to seek */
static int
wl_p2po_wfds_seek_add(wl_p2po_info_t *p2po, void *arg, int len)
{
	int i;
	bool found;
	wl_p2po_wfds_seek_add_t* src = (wl_p2po_wfds_seek_add_t*) arg;
	wl_p2po_seek_t* seek;
	int query_len;

	ASSERT(p2po);
	ASSERT(arg);
	ASSERT(len > 0);

	if (src->seek_hdl == WFDS_SEEK_HDL_INVALID) {
		WL_ERROR(("wl%d: %s: invalid seek hdl %u\n",
			WLCUNIT(p2po), __FUNCTION__, src->seek_hdl));
		return BCME_BADARG;
	}

	/* Search for a free seek data slot */
	found = FALSE;
	for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
		if (p2po->seek_svcs[i] == NULL) {
			found = TRUE;
			break;
		}
	}
	if (!found) {
		WL_ERROR(("wl%d: %s: no room to add seek service, #=%d\n",
			WLCUNIT(p2po), __FUNCTION__, p2po->num_seek_svcs));
		return BCME_NORESOURCE;
	}

	/* Allocate memory to copy the seek data */
	seek = MALLOC(WLCOSH(p2po), sizeof(wl_p2po_seek_t));
	if (seek == NULL) {
		WL_ERROR(("wl%d: %s: seek MALLOC failed len=%u\n",
			WLCUNIT(p2po), __FUNCTION__, len));
		return BCME_NOMEM;
	}
	/* +1 for the service_name_len field, +1 for service_info_req_len field */
	query_len = 1 + src->service_name_len + 1 + src->service_info_req_len;
	seek->query_data_len = query_len;
	seek->query_data = MALLOC(WLCOSH(p2po), query_len);
	if (seek->query_data == NULL) {
		WL_ERROR(("wl%d: %s: query MALLOC failed len=%u\n",
			WLCUNIT(p2po), __FUNCTION__, seek->query_data_len));
		MFREE(WLCOSH(p2po), seek, sizeof(wl_p2po_seek_t));
		return BCME_NOMEM;
	}

	/* Copy the seek data */
	seek->seek_hdl = src->seek_hdl;
	memcpy(seek->macaddr, src->addr, sizeof(seek->macaddr));
	if (memcmp(seek->macaddr, BSSID_BROADCAST, sizeof(seek->macaddr)) == 0)
		seek->macaddr_is_wildcard = TRUE;
	else
		seek->macaddr_is_wildcard = FALSE;
	memcpy(seek->svc_hash, src->service_hash, sizeof(seek->svc_hash));
	seek->svc_name_len = src->service_name_len;
	seek->svc_info_req_len = src->service_info_req_len;
	seek->query_data[0] = seek->svc_name_len;
	memcpy(&seek->query_data[1], src->service_name, seek->svc_name_len);
	seek->query_data[1 + seek->svc_name_len] = seek->svc_info_req_len;
	memcpy(&seek->query_data[1 + seek->svc_name_len + 1],
		src->service_info_req, seek->svc_info_req_len);

	/* Add the seek data to the free seek data slot */
	p2po->seek_svcs[i] = seek;
	p2po->seek_svcs_len[i] = sizeof(wl_p2po_seek_t);
	p2po->num_seek_svcs++;

	WL_P2PO_TRACE(("%s: #=%u i=%d hdl=%u m,n,i,q_len=%u,%u,%u,%u\n", __FUNCTION__,
		p2po->num_seek_svcs, i, seek->seek_hdl,
		p2po->seek_svcs_len[i], seek->svc_name_len,
		seek->svc_info_req_len, seek->query_data_len));

	/* Update the probreq Service Hash P2P IE */
	wl_p2po_upd_seek_svc_hash(p2po);

	return BCME_OK;
}

/* Delete a WFDS service to seek */
static int
wl_p2po_wfds_seek_del(wl_p2po_info_t *p2po, void *arg, int len)
{
	int i;
	wl_p2po_seek_t* seek;
	int seek_len;
	wl_p2po_wfds_seek_del_t* del = (wl_p2po_wfds_seek_del_t*) arg;
	bool found = FALSE;
	sd_instance_t *instance;

	ASSERT(del);
	for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
		if (!(seek = p2po->seek_svcs[i]))
			continue;
		seek_len = p2po->seek_svcs_len[i];
		if (seek->seek_hdl == del->seek_hdl) {
			WL_P2PO_TRACE(("%s: #%u len=%u hdl=%u\n",
				__FUNCTION__, i, seek_len, seek->seek_hdl));
			if (seek->query_data)
				MFREE(WLCOSH(p2po), seek->query_data,
					seek->query_data_len);
			MFREE(WLCOSH(p2po), seek, seek_len);
			p2po->seek_svcs[i] = NULL;
			p2po->seek_svcs_len[i] = 0;
			p2po->num_seek_svcs--;
			found = TRUE;

			/* Delete any sd instances matched to this seek hdl */
			instance = wl_p2po_find_instance_by_seek_hdl(p2po, del->seek_hdl);
			if (instance) {
				WL_P2PO_TRACE(("%s: del sd instance "
					"%02x:\%02x:\%02x:\%02x:\%02x:\%02x state=%u\n",
					__FUNCTION__,
					instance->addr.octet[0], instance->addr.octet[1],
					instance->addr.octet[2], instance->addr.octet[3],
					instance->addr.octet[4], instance->addr.octet[5],
					instance->state));
				wl_p2po_sd_instance_delete(p2po, instance);
			}
			break;
		}
	}

	/* If all seek services have been deleted, change the state of every
	 * every instance to WAIT_TO_START if not IDLE.
	 */
	if (found && p2po->num_seek_svcs == 0) {
		WL_P2PO_TRACE(("%s: set SD_STATE_IDLE\n", __FUNCTION__));
		for (i = 0; i < MAX_SD_INSTANCE; i++) {
			sd_instance_t *instance = &p2po->instance[i];

			if (instance->state != SD_STATE_IDLE) {
				wl_p2po_sd_instance_init(instance);
			}
		}
	}

	/* Update the probreq Service Hash P2P IE */
	wl_p2po_upd_seek_svc_hash(p2po);

	if (!found) {
		WL_P2PO_TRACE(("%s: hdl %u not found\n", __FUNCTION__, del->seek_hdl));
	}
	return (found ? BCME_OK : BCME_NOTFOUND);
}

/* Find a matching WFDS seek service in a probe response P2P IE:
 * Check if the probe response P2P IE has a Advertised Services Info attribute
 * with a Advertised Services Descriptor containning a Service Name that matches
 * one of our configured Seek service names.
 */
static wl_p2po_seek_t*
wl_p2po_wfds_seek_match(wl_p2po_info_t *p2po, struct ether_addr *addr,
	int ad_descs_len, uint8 *ad_descs)
{
	uint8 svc_name_len;
	uint8 *svc_name;
	uint8 *seek_name;
	wl_p2po_seek_t* seek = NULL;
	bool found = FALSE;
	int i;
	sd_instance_t *instance;

	/* Check if the probe response P2P IE has a Advertised Services Info
	 * attribute
	 */
	if (ad_descs == NULL) {
		WL_P2PO_TRACE(("%s: no sinfo in P2P IE\n", __FUNCTION__));
		return NULL;
	}

	/* Walk through the attribute's Advertised Services Descriptors to
	 * look for a Service Name that matches one of our configured Seek
	 * service names.
	 */
	while (ad_descs_len > 7 && !found) {
		/* 7 is the min length of a Advertised Services Descriptor */
		svc_name = ad_descs + 7;
		svc_name_len = ad_descs[6];
		ad_descs_len -= (7 + svc_name_len);
		ad_descs += (7 + svc_name_len);
		if (svc_name_len == 0)
			continue;
		/* WL_P2PO_TRACE_NAME("desc name : ", svc_name_len, svc_name); */

		/* Walk through our configured seek services to look for a
		 * matching service name.
		 */
		for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
			uint8 cmp_len;

			seek = p2po->seek_svcs[i];
			if (!seek)
				continue;
			seek_name = seek->query_data + 1;

			/* Check for a generic advertised service name match */
			if (svc_name_len == P2P_GEN_WFDS_SVC_NAME_LEN &&
				memcmp(svc_name, P2P_GEN_WFDS_SVC_NAME, svc_name_len) == 0) {
				WL_P2PO_TRACE(("%s: generic match for seek hdl %u\n",
					__FUNCTION__, seek->seek_hdl));
				found = TRUE;
				break;
			}

			/* Check for a wildcard in the seek service name */
			if (seek_name[seek->svc_name_len - 1] == '*') {
				cmp_len = seek->svc_name_len - 1;
				if (svc_name_len < cmp_len)
					continue;
			}
			else {
				cmp_len = seek->svc_name_len;
				if (svc_name_len != seek->svc_name_len)
					continue;
			}

			/* wl_p2po_print_name(" seek_name: ", seek->svc_name_len, seek_name); */
			if (memcmp(seek_name, svc_name, cmp_len) == 0) {

				/* Name match found.
				 * Check if this seek entry + the probe response's P2P
				 * Device Address corresponds to a sd instance that has
				 * already completed the ANQP req/resp.  If so, ignore
				 * this seek match.
				 */
				instance = wl_p2po_find_completed_instance(p2po, addr,
					seek->seek_hdl);
				if (instance) {
					WL_P2PO_TRACE(("%s: ignore seek_hdl %u\n",
						__FUNCTION__, seek->seek_hdl));
					continue;
				}

				found = TRUE;
#ifdef BCMDBG
				WL_P2PO_TRACE_NAME("Seek found service ",
					seek->svc_name_len, seek_name);
#endif // endif
				break;
			}
		}
	}
	return found ? seek : NULL;
}

static int
wl_p2po_svc_hash_iovar_op(wl_p2po_info_t *p2po, char *iov_name,
	wl_p2po_wfds_advertise_add_t* advert)
{
	wlc_info_t *wlc = p2po->wlc;
	wlc_bsscfg_t *disc_cfg;
	wl_p2p_wfds_hash_t* params;
	int ret;

	if (p2po->disci == NULL) {
		WL_ERROR(("wl%d: %s: no offload discovery\n", WLCUNIT(p2po), __FUNCTION__));
		return BCME_NOTREADY;
	}
	disc_cfg = wl_disc_bsscfg(p2po->disci);
	if (disc_cfg == NULL) {
		WL_ERROR(("wl%d: %s: no discovery bsscfg\n", WLCUNIT(p2po), __FUNCTION__));
		return BCME_NOTREADY;
	}
	if (disc_cfg->wlcif == NULL) {
		WL_ERROR(("wl%d: %s: no wlcif\n", WLCUNIT(p2po), __FUNCTION__));
		return BCME_NOTREADY;
	}

	params = (wl_p2p_wfds_hash_t*) MALLOCZ(wlc->osh, sizeof(*params));
	if (params == NULL) {
		WL_ERROR(("wl%d: %s: no mem\n", WLCUNIT(p2po), __FUNCTION__));
		return BCME_NOMEM;
	}

	params->advt_id = advert->advertisement_id;
	params->nw_cfg_method = advert->service_config_method;
	memcpy(params->wfds_hash, advert->service_hash, sizeof(advert->service_hash));
	params->name_len = advert->service_name_len;
	if (params->name_len > sizeof(params->service_name))
		params->name_len = sizeof(params->service_name);
	memcpy(params->service_name, advert->service_name, params->name_len);

	ret = wlc_iovar_op(wlc, iov_name, NULL, 0, params,
		sizeof(*params), IOV_SET, disc_cfg->wlcif);
	WL_P2PO_TRACE(("%s: ret=%d hash=%02x%02x%02x%02x%02x%02x", __FUNCTION__, ret,
		params->wfds_hash[0], params->wfds_hash[1], params->wfds_hash[2],
		params->wfds_hash[3], params->wfds_hash[4], params->wfds_hash[5]));
	WL_P2PO_TRACE_NAME(" name=", params->name_len, params->service_name);

	MFREE(wlc->osh, params, sizeof(*params));
	return ret;
}

/* Get a configured WFDS advertise service */
static int
wl_p2po_wfds_advert_get(wl_p2po_info_t *p2po, void *p, int plen, void *a, int alen)
{
	int i;
	bool found;
	wl_p2po_wfds_advertise_add_t* in = (wl_p2po_wfds_advertise_add_t*) p;
	wl_p2po_wfds_advertise_add_t* out = (wl_p2po_wfds_advertise_add_t*) a;
	wl_p2po_wfds_advertise_add_t* adv;
	int adv_len;

	ASSERT(p2po);
	ASSERT(a);
	ASSERT(alen > 0);

	if (in->advertise_hdl == 0) {
		WL_ERROR(("wl%d: %s: invalid advert hdl %u\n",
			WLCUNIT(p2po), __FUNCTION__, in->advertise_hdl));
		return BCME_BADARG;
	}

	/* Search for a configured advertisement with a matching advertisement hdl */
	found = FALSE;
	for (i = 0; i < MAX_WFDS_ADVERT_SVC; i++) {
		adv = p2po->advert_svcs[i];
		if (adv != NULL && adv->advertise_hdl == in->advertise_hdl) {
			found = TRUE;
			break;
		}
	}
	if (!found) {
		WL_ERROR(("wl%d: %s: advert hdl %u not found\n",
			WLCUNIT(p2po), __FUNCTION__, in->advertise_hdl));
		return BCME_NOTFOUND;
	}

	adv_len = sizeof(*adv) + adv->service_info_len - 1;
	if (alen < adv_len) {
		return BCME_BUFTOOSHORT;
	}

	/* Copy the advertisement data */
	memcpy(out, adv, adv_len);

	WL_P2PO_TRACE(("%s: #=%u i=%d len=%u silen=%u hdl=%u\n", __FUNCTION__,
		p2po->num_advert_svcs, i, adv_len, out->service_info_len,
		out->advertise_hdl));
	return BCME_OK;
}

/* Add a WFDS service to advertise */
static int
wl_p2po_wfds_advert_add(wl_p2po_info_t *p2po, void *arg, int len)
{
	int i;
	bool found;
	wl_p2po_wfds_advertise_add_t* src;
	wl_p2po_wfds_advertise_add_t* dest;

	ASSERT(p2po);

	/* Check if the service name exceeds the limit */
	src = (wl_p2po_wfds_advertise_add_t *) arg;
	if (src->service_name_len > MAX_WFDS_SVC_NAME_LEN) {
		WL_ERROR(("wl%d: %s: service name length %u > limit of %d\n",
			WLCUNIT(p2po), __FUNCTION__,
			src->service_name_len, MAX_WFDS_SVC_NAME_LEN));
		return BCME_BUFTOOLONG;
	}

	/* Check if the service info length exceeds the limit */
	if (src->service_info_len > MAX_WFDS_ADV_SVC_INFO_LEN) {
		WL_ERROR(("wl%d: %s: service info length %u > limit of %d\n",
			WLCUNIT(p2po), __FUNCTION__,
			src->service_info_len, MAX_WFDS_ADV_SVC_INFO_LEN));
		return BCME_BUFTOOLONG;
	}

	/* Search for a free slot */
	found = FALSE;
	for (i = 0; i < MAX_WFDS_ADVERT_SVC; i++) {
		dest = p2po->advert_svcs[i];
		if (p2po->advert_svcs[i] == NULL) {
			found = TRUE;
			break;
		}
	}
	if (!found) {
		WL_ERROR(("wl%d: %s: no room to add advertise service\n",
			WLCUNIT(p2po), __FUNCTION__));
		return BCME_NORESOURCE;
	}

	dest = MALLOC(WLCOSH(p2po), len);
	if (dest == NULL) {
		WL_ERROR(("wl%d: %s: add advertise service MALLOC failed len=%u\n",
			WLCUNIT(p2po), __FUNCTION__, len));
		return BCME_NOMEM;
	}

	memcpy(dest, arg, len);
	p2po->advert_svcs[i] = dest;
	p2po->advert_svcs_len[i] = len;
	p2po->num_advert_svcs++;
	WL_P2PO_TRACE(("%s: #=%u i=%d len=%u hdl=%u\n", __FUNCTION__,
		p2po->num_advert_svcs, i, len, dest->advertise_hdl));

	return wl_p2po_svc_hash_iovar_op(p2po, "p2p_add_wfds_hash", dest);
}

/* Delete a WFDS service to advertise */
static int
wl_p2po_wfds_advert_del(wl_p2po_info_t *p2po, void *arg, int len)
{
	int i;
	wl_p2po_wfds_advertise_add_t* advert;
	int advert_len;
	wl_p2po_wfds_advertise_del_t* del = (wl_p2po_wfds_advertise_del_t*) arg;
	int ret = BCME_OK;

	ASSERT(del);
	for (i = 0; i < MAX_WFDS_ADVERT_SVC; i++) {
		if (!(advert = p2po->advert_svcs[i]))
			continue;
		if (advert->advertise_hdl != del->advertise_hdl)
			continue;

		advert_len = p2po->advert_svcs_len[i];
		WL_P2PO_TRACE(("%s: #%u len=%u hdl=%u\n", __FUNCTION__, i,
			advert_len, advert->advertise_hdl));
		if (wl_disc_bsscfg(p2po->disci)) {
			/* Delete the hash.  Ignore the return code because it
			 * is ok if the hash is not found.  When the hash was
			 * originally added it could have been a duplicate of
			 * an existing hash.  That hash could have been deleted
			 * already.
			 */
			(void) wl_p2po_svc_hash_iovar_op(p2po,
				"p2p_del_wfds_hash", advert);
		} else {
			WL_P2PO_TRACE(("%s: discovery disabled, do not del hash\n",
				__FUNCTION__));
		}
		MFREE(WLCOSH(p2po), advert, advert_len);
		p2po->advert_svcs[i] = NULL;
		p2po->advert_svcs_len[i] = 0;
		p2po->num_advert_svcs--;
		return ret;
	}
	WL_ERROR(("%s: hdl %u not found\n", __FUNCTION__, del->advertise_hdl));
	return BCME_NOTFOUND;
}

/* Delete all WFDS services to seek and advertise */
static int
wl_p2po_wfds_del_all_svcs(wl_p2po_info_t *p2po)
{
	int i;
	wl_p2po_seek_t* seek;
	wl_p2po_wfds_advertise_add_t* advert;
	int len;

	WL_P2PO_TRACE(("%s: %u seek, %u adv to delete\n", __FUNCTION__,
		p2po->num_seek_svcs, p2po->num_advert_svcs));
	ASSERT(p2po);
	for (i = 0; i < p2po->num_seek_svcs; i++) {
		seek = p2po->seek_svcs[i];
		len = p2po->seek_svcs_len[i];
		if (seek) {
			WL_P2PO_TRACE(("    del seek #%u len=%u hdl=%u\n",
				i, len, seek->seek_hdl));
			if (seek->query_data)
				MFREE(WLCOSH(p2po), seek->query_data,
				      seek->query_data_len);
			MFREE(WLCOSH(p2po), seek, len);
			p2po->seek_svcs[i] = NULL;
			p2po->seek_svcs_len[i] = 0;
		}
	}
	p2po->num_seek_svcs = 0;

	for (i = 0; i < p2po->num_advert_svcs; i++) {
		advert = p2po->advert_svcs[i];
		len = p2po->advert_svcs_len[i];
		if (advert) {
			WL_P2PO_TRACE(("    del adv #%u len=%u hdl=%u\n",
				i, len, advert->advertise_hdl));
			MFREE(WLCOSH(p2po), advert, len);
			p2po->advert_svcs[i] = NULL;
			p2po->advert_svcs_len[i] = 0;
		}
	}
	p2po->num_advert_svcs = 0;

	return BCME_OK;
}

#if defined(BCMDBG) || defined(WFDS_DEBUG)
/* Debug print a non-NULL terminated string */
void
wl_p2po_print_name(char *prefix, uint namelen, uint8 *name)
{
	char namez[32 + 1];

	if (namelen > sizeof(namez) - 1)
		namelen = sizeof(namez) - 1;
	memcpy(namez, name, namelen);
	namez[namelen] = '\0';

	WL_PRINT(("%s%u,%s\n", prefix, namelen, namez));
	(void) namez;
}
#endif /* defined(BCMDBG) || defined(WFDS_DEBUG) */

#if defined(BCMDBG)
/* Show all WFDS services to seek */
static int
wl_p2po_wfds_seek_dump(wl_p2po_info_t *p2po)
{
	int i;
	wl_p2po_seek_t* seek;
	int seek_len;

	ASSERT(p2po);
	WL_PRINT(("p2po_seek_dump: %u used\n", p2po->num_seek_svcs));
	for (i = 0; i < MAX_WFDS_SEEK_SVC; i++) {
		seek = p2po->seek_svcs[i];
		seek_len = p2po->seek_svcs_len[i];
		WL_PRINT((" %u: seek=%u,%p", i, seek_len, seek));
		if (seek) {
		WL_PRINT((" hdl=%u query=%u,%p\n",
		       seek->seek_hdl, seek->query_data_len, seek->query_data));
		WL_PRINT(("    hash=%02x%02x%02x%02x%02x%02x"
		       " %02x:%02x:%02x:%02x:%02x:%02x w=%u\n",
		       seek->svc_hash[0], seek->svc_hash[1], seek->svc_hash[2],
		       seek->svc_hash[3], seek->svc_hash[4], seek->svc_hash[5],
		       seek->macaddr[0], seek->macaddr[1], seek->macaddr[2],
		       seek->macaddr[3], seek->macaddr[4], seek->macaddr[5],
		       seek->macaddr_is_wildcard));
		wl_p2po_print_name("    name=", seek->svc_name_len, seek->query_data + 1);
		wl_p2po_print_name("    info=", seek->svc_info_req_len,
			seek->query_data + 1 + seek->svc_name_len + 1);
		}
		else {
			printf("\n");
		}
	}
	return BCME_OK;
}

/* Show all WFDS services to advertise */
static int
wl_p2po_wfds_advert_dump(wl_p2po_info_t *p2po)
{
	int i;
	wl_p2po_wfds_advertise_add_t* advert;
	int advert_len;

	WL_PRINT(("p2po_advert_dump: %u used\n", p2po->num_advert_svcs));
	ASSERT(p2po);
	for (i = 0; i < MAX_WFDS_ADVERT_SVC; i++) {
		advert = p2po->advert_svcs[i];
		advert_len = p2po->advert_svcs_len[i];
		WL_PRINT(("%u: len=%u ptr=%p", i, advert_len, advert));
		if (advert) {
			uint8 *hash;
			hash = &advert->service_hash[0];
			WL_PRINT((" hdl=%u %02x%02x%02x%02x%02x%02x\n",
				advert->advertise_hdl,
				hash[0], hash[1], hash[2], hash[3], hash[4], hash[5]));
			WL_PRINT((" id=%08x m=%04x ", advert->advertisement_id,
				advert->service_config_method));
			wl_p2po_print_name(" n=", advert->service_name_len,
				advert->service_name);

			WL_PRINT((" st=%u ", advert->service_status));
			wl_p2po_print_name("sinfo=", advert->service_info_len,
				advert->service_info);
		}
		else {
			WL_PRINT(("\n"));
		}
	}
	return BCME_OK;
}
#endif /* BCMDBG */

#ifndef TRACE_LEVEL_SET
#define TRACE_LEVEL_SET(a)
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_ERR)
/* set tracelevel for enabling debug messages of dot11u and disc modules */
static int
wl_p2po_tracelevel(wl_p2po_info_t *p2po, void *arg, int len)
{
	TRACE_LEVEL_SET(*(int32 *)arg);
	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_ERR */
