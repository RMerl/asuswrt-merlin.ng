
/* ---- Include Files ---------------------------------------------------- */

#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>

#include <wl_export.h>
#include <wlc_pkt_filter.h>

#include <ethernet.h>
#include <802.3.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <bcmudp.h>
#include <bcmtcp.h>
#include <bcmendian.h>
#include <bcmdefs.h>

#ifdef WOWLPF
#include <wlc_wowlpf.h>
#endif
#include <wlc_bsscfg.h>
#ifdef APF
#include <apf_interpreter.h>
#endif /* APF */
#include <wlc_event_utils.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* Targeted debugging for extended packet filters */
#ifdef PF2_DEBUG
static uint32 pf2dbg = FALSE;
static uint32 pf2dbgcnt = 64;

#define PF2_DEBUG_MATCH	0x1
#define PF2_DEBUG_BASE	0x2
#define PF2_DEBUG_CFG	0x4

#define PF2DBG(args)		do {if (pf2dbg) printf args; } while (0)
#define PF2DBG_ON()		(pf2dbg)
#define PF2DBG_MATCH(args)	do {if (pf2dbg & PF2_DEBUG_MATCH) printf args; } while (0)
#define PF2DBG_MATCH_ON()	(pf2dbg & PF2_DEBUG_MATCH)
#define PF2DBG_BASE(args)	do {if (pf2dbg & PF2_DEBUG_BASE) printf args; } while (0)
#define PF2DBG_BASE_ON()	(pf2dbf & PF2_DEBUG_BASE)
#define PF2DBG_CFG(args)	do {if (pf2dbg & PF2_DEBUG_CFG) printf args; } while (0)
#define PF2DBG_CFG_ON()		(pf2dbg & PF2_DEBUG_CFG)
#else /* PF2_DEBUG */
#define PF2DBG(args)		do {} while (0)
#define PF2DBG_ON()		(0)
#define PF2DBG_MATCH(args)	do {} while (0)
#define PF2DBG_MATCH_ON()	(0)
#define PF2DBG_BASE(args)	do {} while (0)
#define PF2DBG_BASE_ON()	(0)
#define PF2DBG_CFG(args)	do {} while (0)
#define PF2DBG_CFG_ON()		(0)
#endif /* PF2_DEBUG */

/* Enable capability to retrieve list of installed filters for debugging. */
#define PKT_FILTER_LIST_SUPPORT	1

/* By default, cap the number of supported filters to 22(WHCK Requirement).
 * This can be overriden by an IOVAR.
 */
#define DEFAULT_MAX_NUM_FILTERS	22

/* This is useful during development to avoid compiler warnings/errors about
 * un-used static functions.
 */
/* #define STATIC */
#ifndef STATIC
#define STATIC	static
#endif

/* wlc_pub_t struct access macros */
#define WLCOSH(info)	((info)->wlc->osh)
#define WLCUNIT(info)	((info)->wlc->pub->unit)

/* IOVar table */
enum {
	/* Install new packet filter. */
	IOV_PKT_FILTER_ADD,

	/* Uninstall a previously added packet filter. */
	IOV_PKT_FILTER_DELETE,

	/* Enable/disable packet filter. */
	IOV_PKT_FILTER_ENABLE,

	/* Set global packet filter engine match action, e.g.
	 *  - forward on match.
	 *  - discard on match.
	 */
	IOV_PKT_FILTER_MODE,

	/* Retrieve list of installed filters. */
	IOV_PKT_FILTER_LIST,

	/* Retrieve debug filter stats. */
	IOV_PKT_FILTER_STATS,

	/* Clear debug filter stats. */
	IOV_PKT_FILTER_CLEAR_STATS,

	/* Get/set maximum number of supported filters. */
	IOV_PKT_FILTER_MAX,

	/* Explicit discard of ICMP packets */
	IOV_PKT_FILTER_ICMP,

	/* Count of explicitly discarded packets. */
	IOV_PKT_FILTER_ICMP_CNT,

	/* List of TCP/UDP ports for final filtering */
	IOV_PKT_FILTER_PORTS,

	/* Count of packets checked against the port filter */
	IOV_PKT_FILTER_PORTS_CHKCNT,

	/* Count of packets tossed due to port filtering */
	IOV_PKT_FILTER_PORTS_CNT,

	/* Count of packet fragmenets tossed due to port filtering */
	IOV_PKT_FILTER_FRAGS_CNT,

	/* Get APF interpreter version */
	IOV_APF_VERSION,

	/* Get the size limit of APF program */
	IOV_APF_PROGRAM_SIZE_LIMIT,

#ifdef PF2_DEBUG
	/* Dynamic debug enable */
	IOV_PF2DBG,
	IOV_PF2DBGCNT
#endif /* PF2_DEBUG */
};

typedef struct pkt_filter_pattern_timeout {
	uint32	offset;	/* Offset within received packet to start pattern matching.
					 * Offset '0' is the first byte of the ethernet header.
					 */
	uint32	size_bytes;	/* Size of the pattern.  Bitmask must be the same size. */
	uint32	timeout;		/* Timeout(seconds) */
	uint32	time_since_last_pkt; /* Time since last packet received, internal use only */
	uint8	mask_and_pattern[1]; /* Variable length mask and pattern data.
								  * mask starts at offset 0. Pattern
								  * immediately follows mask.
								  */
} pkt_filter_pattern_timeout_t;

#define PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN	\
			OFFSETOF(pkt_filter_pattern_timeout_t, mask_and_pattern)

/* Generic packet filter state. */
typedef struct pkt_filter {

	/* Pointer to next filter in list, NULL if no more filters. */
	struct pkt_filter	*next;

	/* Unique filter id, specified by app. Used to un-install individual filters. */
	uint32  		id;

	/* Filter type. */
	wl_pkt_filter_type_t	type;

	/* Negate the result of filter matches. e.g. Create a UDP packet filter, and
	 * set this to 1 to create a filter for all non-UDP packets.
	 * TYPE 6 : it don't use megate match, use for forward or drop.
	 */
	bool   			negate_match;

	/* Internal flags, e.g. for caching derived filter attributes */
	uint8			flags;

	/* Debug statistics counter. */
	unsigned int		num_pkts_matched;

	/* Filter types. */
	union {
		/* Pattern matching filter criteria. */
		wl_pkt_filter_pattern_t		*pattern;
		wl_pkt_filter_pattern_list_t	*patlist;
		wl_apf_program_t		*apf_program;
		pkt_filter_pattern_timeout_t	*pattern_timeout;
		/* Add more filter types here... */
	} u;

} pkt_filter_t;

/* Internal flags definitions */
#define WL_PKT_FILTER_FLAGS_PSEUDOMAGIC	0x01

/* Packet filter private info structure. Container for the set of
 * installed filters.
 */
struct pkt_filter_info {

	/* Pointer back to wlc structure */
	/* Pointer back to wlc structure */
	wlc_info_t		*wlc;

	/* If PKT_FILTER_MODE_FORWARD_ON_MATCH set:
	 *  - Forward packet on filter match.
	 *  - Discard packet on non-match.
	 * If PKT_FILTER_MODE_FORWARD_ON_MATCH not set:
	 *  - Discard packet on filter match.
	 *  - Forward packet on non-match.
	 * See other PKT_FILTER_MODE_XXX definitions
	 */
	uint8			operation_mode;

	/* Statistics counters for debug. */
	unsigned int 		num_pkts_forwarded;
	unsigned int 		num_pkts_discarded;

	/* Linked list of filters. NULL if no filters. */
	pkt_filter_t		*disabled_list;
	pkt_filter_t		*enabled_list;

	/* Maximum and current number of filters. */
	unsigned int		max_num_filters;
	unsigned int		num_filters;

	/* For ICMP discard */
	uint32			toss_icmp;
	uint32			icmp_tossed;

	/* Cached packet */
	pm_wake_packet_t	*cached_wake_packet;

#ifdef PACKET_FILTER2
	/* Some info for port filtering */
	uint16			nports;
	uint16			*ports;
	uint32			port_checked;
	uint32			port_tossed;
	uint32			frag_tossed;

	/* Base offsets for type 2 matches, IP protocol/frag info */
	int16			base_offs[WL_PKT_FILTER_BASE_COUNT];
	uint16			ipfrag;
	uint8			ipprot;
	bool			lastffwd;
	uint8			matched_flags;
#endif /* PACKET_FILTER2 */

	/* size of the installed APF programs */
	uint32 apf_program_tot_size;
};

/* Some convenient macros for port filtering */
#ifdef PACKET_FILTER2
#define WL_PF2_NPORTS(info) (info)->nports
#define WL_PF2_PORTTOSS(info, sdu) wlc_pkt_filter_porttoss((info), (sdu))
#define WL_PF2_MATCHED_FLAGS(info) (info)->matched_flags
#define WL_PF2_SET_MATCHED_FLAGS(info, filter) (info)->matched_flags = (filter)->flags
#else
#define WL_PF2_NPORTS(info) 0
#define WL_PF2_PORTTOSS(info, sdu) 0
#define WL_PF2_MATCHED_FLAGS(info) 0
#define WL_PF2_SET_MATCHED_FLAGS(info, filter) do { } while (0)
#endif

#if defined(APF)
#define WL_APF_FILTER_AGE_IN_SEC(apf_program) \
	((OSL_SYSUPTIME() - (apf_program)->inst_ts) / 1000)
#define WL_APF_PROGRAM_SIZE_LIMIT(info)	\
	(WL_APF_PROGRAM_MAX_SIZE - (info)->apf_program_tot_size)
#define WL_APF_PROGRAM_INC_USAGE_SIZE(info, size) \
	((info)->apf_program_tot_size += (size));
#define WL_APF_PROGRAM_DEC_USAGE_SIZE(info, size) \
	((info)->apf_program_tot_size -= (size));
#endif /* APF */

/* ---- Private Variables ------------------------------------------------ */
static const bcm_iovar_t pkt_filter_iovars[] = {
	{
		"pkt_filter_add",
		IOV_PKT_FILTER_ADD,
		(0), 0,
		IOVT_BUFFER,
		WL_PKT_FILTER_FIXED_LEN
	},

	{
		"pkt_filter_delete",
		IOV_PKT_FILTER_DELETE,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_enable",
		IOV_PKT_FILTER_ENABLE,
		(0), 0,
		IOVT_BUFFER,
		sizeof(wl_pkt_filter_enable_t)
	},

	{
		"pkt_filter_mode",
		IOV_PKT_FILTER_MODE,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_list",
		IOV_PKT_FILTER_LIST,
		(0), 0,
		IOVT_BUFFER,
		sizeof(wl_pkt_filter_list_t)
	},

	{
		"pkt_filter_stats",
		IOV_PKT_FILTER_STATS,
		(0), 0,
		IOVT_BUFFER,
		sizeof(wl_pkt_filter_stats_t)
	},

	{
		"pkt_filter_clear_stats",
		IOV_PKT_FILTER_CLEAR_STATS,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_max",
		IOV_PKT_FILTER_MAX,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_icmp",
		IOV_PKT_FILTER_ICMP,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_icmp_cnt",
		IOV_PKT_FILTER_ICMP_CNT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

#ifdef PACKET_FILTER2
	{
		"pkt_filter_ports",
		IOV_PKT_FILTER_PORTS,
		(0), 0,
		IOVT_BUFFER,
		WL_PKT_FILTER_PORTS_FIXED_LEN
	},

	{
		"pkt_filter_ports_chkcnt",
		IOV_PKT_FILTER_PORTS_CHKCNT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_ports_cnt",
		IOV_PKT_FILTER_PORTS_CNT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pkt_filter_frags_cnt",
		IOV_PKT_FILTER_FRAGS_CNT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},
#ifdef PF2_DEBUG
	{
		"pf2dbg",
		IOV_PF2DBG,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"pf2dbgcnt",
		IOV_PF2DBGCNT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},
#endif /* PF2_DEBUG */
#endif /* PACKET_FILTER2 */
#if defined APF
	{
		"apf_ver",
		IOV_APF_VERSION,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},
	{
		"apf_size_limit",
		IOV_APF_PROGRAM_SIZE_LIMIT,
		(0), 0,
		IOVT_UINT32,
		sizeof(uint32)
	},
#endif /* APF */
	{NULL, 0, 0, 0, 0, 0 }
};

/* ---- Private Function Prototypes -------------------------------------- */

STATIC INLINE void
add_to_filter_list(pkt_filter_t **list, pkt_filter_t *pkt_filter);

STATIC pkt_filter_t*
delete_from_filter_list(pkt_filter_t **list, uint32 id);

STATIC pkt_filter_t*
find_in_filter_list(pkt_filter_t *list, uint32 id);

STATIC int
pkt_filter_doiovar
(
	void 			*hdl,
	uint32 			actionid,
	void 			*p,
	uint			plen,
	void 			*a,
	uint 			alen,
	uint 			vsize,
	struct wlc_if 		*wlcif
);

STATIC void
pkt_filter_watchdog(void *hdl);

STATIC int
add_filter
(
	wlc_pkt_filter_info_t	*info,
	const wl_pkt_filter_t	*wl_pkt_filter,
	int 			data_len
);

STATIC int
delete_filter(wlc_pkt_filter_info_t *info, uint32 id);

STATIC int
enable_filter(wlc_pkt_filter_info_t *info, uint32 id, bool enable);

STATIC pkt_filter_t*
find_filter(wlc_pkt_filter_info_t *info, uint32 id);

STATIC int
add_pattern_filter
(
	wlc_pkt_filter_info_t	*info,
	pkt_filter_t		*pkt_filter,
	const wl_pkt_filter_t	*wl_pkt_filter,
	int			data_len
);

STATIC int
add_pattern_timeout_filter
(
	wlc_pkt_filter_info_t *info,
	pkt_filter_t *pkt_filter,
	const wl_pkt_filter_t *filter_not_aligned,
	int data_len
);

STATIC int
delete_pattern_filter(wlc_pkt_filter_info_t *info, pkt_filter_t *filter);

STATIC int
free_pattern_filter(wlc_pkt_filter_info_t *info, wl_pkt_filter_pattern_t *filter);

STATIC int
free_pattern_timeout_filter(wlc_pkt_filter_info_t *info, pkt_filter_pattern_timeout_t *filter);

STATIC bool
run_pattern_filter(wlc_pkt_filter_info_t *info,	const pkt_filter_t *filter, void *sdu);

#ifdef PACKET_FILTER2
STATIC bool
run_pattern_list_filter(wlc_pkt_filter_info_t *info, const pkt_filter_t *filter, void *sdu);

STATIC void
wlc_pkt_filter_parse_bases(wlc_pkt_filter_info_t *info, void *sdu);

STATIC bool
wlc_pkt_filter_porttoss(wlc_pkt_filter_info_t *info, void *sdu);
#endif /* PACKET_FILTER2 */

#ifdef PKT_FILTER_LIST_SUPPORT
STATIC int
get_filters
(
	const wlc_pkt_filter_info_t	*info,
	bool				enable,
	wl_pkt_filter_t			*dst_filter,
	unsigned int 			buf_len,
	unsigned int 			*num_filters
);

STATIC unsigned int
get_pattern_filter
(
	const wlc_pkt_filter_info_t	*info,
	const pkt_filter_t		*src_filter,
	wl_pkt_filter_t			*dst_filter,
	unsigned int 			buf_len
);

#ifdef PACKET_FILTER2
STATIC unsigned int
get_pattern_list_filter
(
	const wlc_pkt_filter_info_t	*info,
	const pkt_filter_t		*src_filter,
	wl_pkt_filter_t			*dst_filter,
	unsigned int 			buf_len
);
#endif /* PACKET_FILTER2 */
#endif   /* PKT_FILTER_LIST_SUPPORT */
#if defined(APF)
static int
add_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *pkt_filter,
	const wl_pkt_filter_t *filter_not_aligned, int data_len);
static int
delete_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *filter);
static int
free_apf_program(wlc_pkt_filter_info_t *info, wl_apf_program_t *apf_program);
static bool
run_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *filter,
	const void *sdu);
static unsigned int
get_apf_program(const wlc_pkt_filter_info_t *info,
	const pkt_filter_t *src, wl_pkt_filter_t *dst,
	unsigned int buf_len);
static void
dump_apf_program(wl_apf_program_t *apf_program);
#endif  /* APF */

/* ---- Public Functions -------------------------------------------------------- */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/*
*****************************************************************************
* Function:   wlc_pkt_filter_attach
*
* Purpose:    Initialize packet filter private context.
*
* Parameters: context	(mod)	Common driver context.
*
* Returns:    Pointer to the packet filter private context. Returns NULL on error.
*****************************************************************************
*/
wlc_pkt_filter_info_t *
BCMATTACHFN(wlc_pkt_filter_attach)(void *context)
{
	wlc_info_t	*wlc = (wlc_info_t *)context;
	osl_t		*osh = wlc->osh;
	wlc_pkt_filter_info_t *info;

#ifdef PACKET_FILTER2
	STATIC_ASSERT((OFFSETOF(wl_pkt_filter_pattern_t, size_bytes) ==
		OFFSETOF(wl_pkt_filter_pattern_listel_t, size_bytes)) ||
		(OFFSETOF(wl_pkt_filter_pattern_t, mask_and_pattern) ==
		OFFSETOF(wl_pkt_filter_pattern_listel_t, mask_and_data)));
#endif /* PACKET_FILTER2 */

	/* Allocate packet filter private info struct. */
	info = MALLOCZ(osh, sizeof(wlc_pkt_filter_info_t));
	if (info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		    wlc->pub->unit, __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	/* Init packet filter private info struct. */
	info->max_num_filters = DEFAULT_MAX_NUM_FILTERS;
	info->wlc  = wlc;

	/* Register this module. */
	if (wlc_module_register(wlc->pub,
	                        pkt_filter_iovars,
	                        "pkt_filter",
	                        info,
	                        pkt_filter_doiovar,
	                        pkt_filter_watchdog,
	                        NULL,
	                        NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->_pkt_filter = TRUE;
#if (defined(PACKET_FILTER2) && !defined(PKT_FLT2_DISABLED))
	wlc->pub->_pkt_filter2 = TRUE;
#if (defined(PACKET_FILTER6) && !defined(PKT_FLT6_DISABLED))
	wlc->pub->_pkt_filter6 = TRUE;
#endif /* PACKET_FILTER6 && !PKT_FLT6_DISABLED */
#endif /* PACKET_FILTER2 && !PKT_FLT2_DISABLED */

#if (defined(APF) && !defined(APF_DISABLED))
	wlc->pub->_apf_filter = TRUE;
#endif  /* APF && !APF_DISABLED */

	info->operation_mode = PKT_FILTER_MODE_FORWARD_ON_MATCH;
	return (info);

fail:
	if (NULL != info)
		MFREE(WLCOSH(info), info, sizeof(wlc_pkt_filter_info_t));

	return (NULL);
}

/*
*****************************************************************************
* Function:   wlc_pkt_filter_detach
*
* Purpose:    Cleanup packet filter private context.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
void
BCMATTACHFN(wlc_pkt_filter_detach)(wlc_pkt_filter_info_t *info)
{
	if (info == NULL)
		return;

	/* De-register this module. */
	wlc_module_unregister(info->wlc->pub, "pkt_filter", info);

	/* Delete all all of the filters */
	wlc_pkt_fitler_delete_filters(info);

	/* Free allocated packet filter engine state. */
	MFREE(WLCOSH(info), info, sizeof(wlc_pkt_filter_info_t));
}

/*
*****************************************************************************
* Function:   wlc_pkt_fitler_delete_filters
*
* Purpose:    Delete all filters but remain attached.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    None.
*****************************************************************************
*/
void wlc_pkt_fitler_delete_filters(wlc_pkt_filter_info_t *info)
{
	pkt_filter_t *filter;
	pkt_filter_t *next_filter;

	if (info == NULL)
		return;

	/* Free per-filter allocated memory. */
	filter = info->enabled_list;
	while (NULL != filter) {
		next_filter = filter->next;
		delete_filter(info, filter->id);

		filter = next_filter;
	}

	filter = info->disabled_list;
	while (NULL != filter) {
		next_filter = filter->next;
		delete_filter(info, filter->id);

		filter = next_filter;
	}

	/* Free cached wake packet structure */
	if (info->cached_wake_packet) {
		MFREE(WLCOSH(info), info->cached_wake_packet,
			sizeof(pm_wake_packet_t));
		info->cached_wake_packet = NULL;
	}

#ifdef PACKET_FILTER2
	if (PKT_FILTER2_ENAB(info->wlc->pub)) {
		/* Free any portlist */
		if (info->ports) {
			MFREE(WLCOSH(info), info->ports,
				(WL_PKT_FILTER_PORTS_FIXED_LEN +
					WL_PKT_FILTER_PORTS_MAX * sizeof(uint16)));
			info->ports = NULL;
		}
	}
#endif /* PACKET_FILTER2 */

}

#ifdef PACKET_FILTER2
/* ----------------------------------------------------------------------- */
STATIC void
wlc_pkt_filter_parse_bases(wlc_pkt_filter_info_t *info, void *sdu)
{
	uint8 *pkt, *hdr;
	uint16 pktlen;
	int16 offset;	/* Running parse offset in packet */
	uint16 field;	/* Aribtrary packet content to examine */
	int i;

	hdr = pkt = PKTDATA(WLCOSH(info), sdu);
	pktlen = PKTLEN(WLCOSH(info), sdu);

	/* Could just memset to 0xff? */
	for (i = 0; i < WL_PKT_FILTER_BASE_COUNT; i++)
		info->base_offs[i] = -1;
	info->ipprot = 0;
	info->ipfrag = 0;

	/* Start by setting the packet as a whole */
	info->base_offs[WL_PKT_FILTER_BASE_PKT] = 0;
	info->base_offs[WL_PKT_FILTER_BASE_END] = pktlen;

	PF2DBG_BASE(("Bases: setd %d 0, %d %d\n",
	             WL_PKT_FILTER_BASE_PKT, WL_PKT_FILTER_BASE_END, pktlen));

	offset = 0;
	info->base_offs[WL_PKT_FILTER_BASE_ETH_H] = 0;
	if (pktlen >= ETHER_HDR_LEN) {
		info->base_offs[WL_PKT_FILTER_BASE_ETH_D] = ETHER_HDR_LEN;
		offset = ETHER_TYPE_OFFSET;
	}

	/* Grab ether type and move past it */
	field = ntoh16(*(uint16*)(hdr + offset));
	offset += ETHER_TYPE_LEN;
	PF2DBG_BASE(("D11 h/d: %d/%d, Eth h/d: %d/%d, eth type 0x%04x (%s)\n",
	             info->base_offs[WL_PKT_FILTER_BASE_D11_H],
	             info->base_offs[WL_PKT_FILTER_BASE_D11_D],
	             info->base_offs[WL_PKT_FILTER_BASE_ETH_H],
	             info->base_offs[WL_PKT_FILTER_BASE_ETH_D], field,
	             ((field == ETHER_TYPE_ARP) ? "ARP" :
	              (field == ETHER_TYPE_IP) ? "IP4" :
	              (field == ETHER_TYPE_IPV6) ? "IP6" : "unknown")));

	/* Set up offsets for recognized ether types */
	if (field == ETHER_TYPE_ARP) {
		info->base_offs[WL_PKT_FILTER_BASE_ARP_H] = offset;
	} else if ((field == ETHER_TYPE_IP) && (pktlen >= offset + sizeof(struct ipv4_hdr))) {
		struct ipv4_hdr *ipv4 = (struct ipv4_hdr*)(hdr + offset);
		if (IP_VER(ipv4) == 4) {
			info->base_offs[WL_PKT_FILTER_BASE_IP4_H] = offset;
			offset += IPV4_HLEN(ipv4);
			info->base_offs[WL_PKT_FILTER_BASE_IP4_D] = offset;
			info->ipfrag = ntoh16(ipv4->frag);
			field = info->ipfrag & IPV4_FRAG_OFFSET_MASK;
			PF2DBG_BASE(("IP4: frag %04x len %d prot %d (%s)\n",
			             field, IPV4_HLEN(ipv4), ipv4->prot,
			             ((ipv4->prot == IP_PROT_UDP) ? "UDP" :
			              (ipv4->prot == IP_PROT_TCP) ? "TCP" :
			              (ipv4->prot == IP_PROT_ICMP) ? "ICMP" : "unknown")));
			if (!field && (ipv4->prot == IP_PROT_UDP)) {
				info->base_offs[WL_PKT_FILTER_BASE_UDP_H] = offset;
				info->base_offs[WL_PKT_FILTER_BASE_UDP_D] = offset + UDP_HDR_LEN;
			} else if (!field && (ipv4->prot == IP_PROT_TCP)) {
				info->base_offs[WL_PKT_FILTER_BASE_TCP_H] = offset;
				/* Variable TCP header, calculate below */
			}
			info->ipprot = ipv4->prot;
		}
	} else if ((field == ETHER_TYPE_IPV6) && (pktlen >= offset + sizeof(struct ipv6_hdr))) {
		struct ipv6_hdr *ipv6 = (struct ipv6_hdr*)(hdr + offset);
		if (ipv6->version == 6) {
			uint size = sizeof(struct ipv6_hdr);
			uint8 *nextp = &ipv6->nexthdr;
			uint8 next = *nextp;

			PF2DBG_BASE(("IPv6 (size %d, next %d):\n", size, next));

			info->base_offs[WL_PKT_FILTER_BASE_IP6_H] = offset;
			offset += size;

			field = 0;
			while (size && (pktlen >= offset + IPV6_EXT_WORD)) {
				switch (next) {
				case IPV6_EXT_AUTH:
					nextp = &hdr[offset + IPV6_EXT_NEXTHDR];
					size = (4 * hdr[offset + IPV6_EXT_HDRLEN]);
					break;
				case IPV6_EXT_FRAG: {
					struct ipv6_frag *fraghdr;
					fraghdr = (struct ipv6_frag*)&hdr[offset];
					info->ipfrag = ntoh16(fraghdr->frag_offset);
					field = info->ipfrag & IPV6_FRAG_OFFS_MASK;
				}
					/* FALLTHROUGH */
				case IPV6_EXT_HOP:
				case IPV6_EXT_ROUTE:
				case IPV6_EXT_DEST:
					nextp = &hdr[offset + IPV6_EXT_NEXTHDR];
					size = hdr[offset + IPV6_EXT_HDRLEN] + 1;
					size *= IPV6_EXT_WORD;
					break;
				default:
					/* Unknown or end -- stop moving */
					size = 0;
				}

				next = *nextp;

				if (size) {
					PF2DBG_BASE(("\tsize %d, next %d\n", size, next));
				} else {
					PF2DBG_BASE(("\tsize unknown, next %d (%s)\n", next,
					             ((next == ICMPV6_HEADER_TYPE) ? "ICMPV6" :
					              (next == IP_PROT_UDP) ? "UDP" :
					              (next == IP_PROT_TCP) ? "TCP" : "uknown")));
				}
				offset += size;
			}

			/* The ipv6 data is the next header (offset) */
			info->base_offs[WL_PKT_FILTER_BASE_IP6_D] = offset;
			info->base_offs[WL_PKT_FILTER_BASE_IP6_P] = nextp - hdr;

			if (!field && (next == IP_PROT_UDP)) {
				info->base_offs[WL_PKT_FILTER_BASE_UDP_H] = offset;
				info->base_offs[WL_PKT_FILTER_BASE_UDP_D] = offset + UDP_HDR_LEN;
			} else if (!field && (next == IP_PROT_TCP)) {
				info->base_offs[WL_PKT_FILTER_BASE_TCP_H] = offset;
				/* Variable TCP header, calculate below */
			}
			info->ipprot = next;
		}
	}

	if ((info->base_offs[WL_PKT_FILTER_BASE_TCP_H] != -1) &&
	    (pktlen >= offset + sizeof(struct bcmtcp_hdr))) {
		struct bcmtcp_hdr *tcp = (struct bcmtcp_hdr*)(hdr + offset);
		field = (ntoh16(tcp->hdrlen_rsvd_flags) & TCP_HLEN_MASK) >> TCP_HLEN_SHIFT;
		PF2DBG_BASE(("TCPH: field 0x%04x (from %04x -> %04x), offset %d -> %d.",
		             field, tcp->hdrlen_rsvd_flags, ntoh16(tcp->hdrlen_rsvd_flags),
		             offset, offset + 4 * field));
		offset += 4 * field;
		info->base_offs[WL_PKT_FILTER_BASE_TCP_D] = offset;
	}

	PF2DBG_BASE(("Base DONE\n"));
}
#endif /* PACKET_FILTER2 */

/*
*****************************************************************************
* Function:   wlc_pkt_fitler_recv_proc
*
* Purpose:    Process received frames.
*
* Parameters: info	(mod)	Packet filter engine private context.
*             sdu	(in)	Received packet.
*
* Returns:    TRUE if received packet should be forwarded. FALSE if
*             it should be discarded.
*****************************************************************************
*/
bool wlc_pkt_filter_recv_proc(wlc_pkt_filter_info_t *info, void *sdu)
{
	pkt_filter_t	*filter;
	bool		match = TRUE;

	if (info->operation_mode & PKT_FILTER_MODE_DISABLE) {
		 /* pkt filter is disabled, all pkts go through */
		return TRUE;
	}
	/* Handle the special ICMP filter outside the normal path */
	if (info->toss_icmp) {
		uint8 *pkt = PKTDATA(WLCOSH(info), sdu);
		int   plen = PKTLEN(WLCOSH(info), sdu);

		/* 802.3 type packet */
		int offs = ETHER_TYPE_OFFSET;

		if (plen >= (offs + sizeof(uint16) + IPV4_PROT_OFFSET)) {
			uint16 etype;
			pkt += offs;
			etype = (*pkt++ << 8);
			etype += *pkt++;
			if ((etype == ETHER_TYPE_IP) &&
			    (IP_VER(pkt) == 4) && (pkt[IPV4_PROT_OFFSET] == IP_PROT_ICMP)) {
				info->icmp_tossed++;
				info->num_pkts_discarded++;
				PF2DBG_MATCH(("Toss (ICMP)\n"));
				return FALSE;
			}
		}
	}

	/* If no filters (including ports) are installed, or none are enabled, act per mode */
	if ((info->enabled_list == NULL) && !WL_PF2_NPORTS(info)) {
		if (info->operation_mode & PKT_FILTER_MODE_PKT_FORWARD_OFF_DEFAULT) {
			 /* Filter out packet for EXT STA to not disturb host in sleep */
			return (FALSE);
		} else {
			/* Forward received packets to the host */
			PF2DBG_MATCH(("Fwd (disabled)\n"));
			return (TRUE);
		}
	}

#ifdef PACKET_FILTER2
	if (PKT_FILTER2_ENAB(info->wlc->pub)) {
		PF2DBG_MATCH(("FLTR:\n"));

		/* Parse the packet for base offsets */
		wlc_pkt_filter_parse_bases(info, sdu);
		info->matched_flags = 0;

#ifdef PF2_DEBUG
		if (PF2DBG_MATCH_ON())
		{
			int i, nchar;
			uint8 *pkt = PKTDATA(WLCOSH(info), sdu);

			printf("Packet (len %d):\n", PKTLEN(WLCOSH(info), sdu));
			for (i = 0; i < pf2dbgcnt; i++) {
				printf("%02x ", pkt[i]);
				if ((i & 0x0f) == 0x0f)
					printf("\n");
			}

			printf("Offsets and Values:\n  ");
			for (i = 0; i < WL_PKT_FILTER_BASE_COUNT; i++) {
				nchar = printf("%d", i);
				while (nchar++ < 4) printf(" ");
			}
			printf("\n  ");
			for (i = 0; i < WL_PKT_FILTER_BASE_COUNT; i++) {
				nchar = printf("%d", info->base_offs[i]);
				while (nchar++ < 4) printf(" ");
			}
			printf("\n");
		}
#endif /* PF2_DEBUG */
	}

#ifdef PACKET_FILTER6
	/* Immediately Packet filter (TYPE 6: WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) */
	filter = info->enabled_list;
	while (NULL != filter) {
		ASSERT((filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) ||
			(PKT_FILTER2_ENAB(info->wlc->pub) &&
				filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
			(PKT_FILTER6_ENAB(info->wlc->pub) &&
				filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) ||
			(filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) ||
			(filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH));

		if (PKT_FILTER6_ENAB(info->wlc->pub) &&
			filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) {
			PF2DBG_MATCH(("Immediately PF(TYPE 6), id %d\n", filter->id));
			match = run_pattern_list_filter(info, filter, sdu);

			/* Check pattern match */
			if (match != filter->negate_match) {
				PF2DBG_MATCH(("Match ID %d %d, done\n",
					filter->id, filter->negate_match));
				/* Pattern match packet counter */
				filter->num_pkts_matched++;
				if (filter->negate_match) {
					/* Discard packet counter */
					info->num_pkts_discarded++;
					return (FALSE);
				} else {
					/* Forward packet counter */
					info->num_pkts_forwarded++;
					return (TRUE);
				}
			}
			PF2DBG_MATCH(("No Match %d\n", filter->id));
		}

		filter = filter->next;
	}
#endif /* PACKET_FILTER6 */
#endif /* PACKET_FILTER2 */

	/* Iterate linked-list of filters, apply each enabled filter to received
	 * packet, looking for a match.
	 */
	filter = info->enabled_list;
	while (NULL != filter) {
		ASSERT((filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) ||
#ifdef PACKET_FILTER2
			(PKT_FILTER2_ENAB(info->wlc->pub) &&
				filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
#ifdef PACKET_FILTER6
			(PKT_FILTER6_ENAB(info->wlc->pub) &&
				filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) ||
#endif /* PACKET_FILTER6 */
#endif /* PACKET_FILTER2 */
#if defined(APF)
			(APF_ENAB(info->wlc->pub) &&
		        (filter->type == WL_PKT_FILTER_TYPE_APF_MATCH)) ||
#endif /* APF */
			(filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) ||
			(filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH) ||
			(filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT));

#ifdef PACKET_FILTER2
#ifdef PACKET_FILTER6
		if (PKT_FILTER6_ENAB(info->wlc->pub) &&
			filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) {
			/* Immedeately packet filter type */
			filter = filter->next;
			continue;
		}
#endif /* PACKET_FILTER6 */
		if (PKT_FILTER2_ENAB(info->wlc->pub) &&
				filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) {
			PF2DBG_MATCH(("PF2, id %d\n", filter->id));
			match = run_pattern_list_filter(info, filter, sdu);
		}
		else
#endif /* PACKET_FILTER2 */
		{
			PF2DBG_MATCH(("PF, id %d\n", filter->id));
			match = run_pattern_filter(info, filter, sdu);
		}

		if (match) {
			/* Update debug stat and match status, then bail... */
			PF2DBG_MATCH(("Match %d, done.\n", filter->id));
			WL_PF2_SET_MATCHED_FLAGS(info, filter);
			filter->num_pkts_matched++;
			break;
		}

		filter = filter->next;
	}

#if defined(WOWLPF)
	if (WOWLPF_ENAB(info->wlc->pub)) {
		if (match) {
			wlc_wowlpf_pktfilter_cb(info->wlc, filter->type,
				filter->id, filter->u.pattern, sdu);
		}
	}
#endif

#if defined(APF)
	if (APF_ENAB(info->wlc->pub)) {
		/*
		 * In case of APF, the filters are executed one after the other
		 * and the packet is accepted only if none of the filters drop it.
		 * In case of Android, APF is enabled in early suspend stage
		 * (i.e, only LCD is off and host is still active). Therefore for
		 * PCIe do an explicit check to make sure that APF is enabled only
		 * in PCIe D3 mode.
		 */
		if (TRUE &&
#ifdef BCMPCIEDEV
		    BCMPCIEDEV_ENAB() && !info->wlc->cmn->hostmem_access_enabled &&
#endif /* BCMPCIEDEV */
		    info->apf_program_tot_size && match) {
			filter = info->enabled_list;
			while (NULL != filter) {
				if (filter->type == WL_PKT_FILTER_TYPE_APF_MATCH) {
					PF2DBG_MATCH(("APF, id %d\n", filter->id));
					match &= run_apf_program(info, filter, sdu);
					if (match) {
						PF2DBG_MATCH(("Match %d, done.\n", filter->id));
						WL_PF2_SET_MATCHED_FLAGS(info, filter);
						filter->num_pkts_matched++;
					}
				}
				filter = filter->next;
			}
		}
	}
#endif /* APF */

	/*	The received packet should be forwarded if:
	 *		- A filter match was found AND the filter engine has been
	 *		  configured to forward on match.
	 *	OR
	 *		- A filter match was not found AND the filter engine has been
	 *		  configured to discard on match.
	 */
	if (match == (bool)(info->operation_mode & PKT_FILTER_MODE_FORWARD_ON_MATCH)) {
		/* Account for any final TCP/UDP port filters before forwarding */
		PF2DBG_MATCH(("Check ports\n"));
		if (!WL_PF2_NPORTS(info) ||
		    (WL_PF2_MATCHED_FLAGS(info) & WL_PKT_FILTER_FLAGS_PSEUDOMAGIC) ||
		    !WL_PF2_PORTTOSS(info, sdu)) {
			info->num_pkts_forwarded++;
			return (TRUE);
		}
	}

	/*	The received packet should be discarded if:
	 *		- A filter match was found AND the filter engine has been
	 *		  configured to discard on match.
	 *	OR
	 *		- A filter match was not found AND the filter engine has been
	 *		  configured to forward on match.
	 */
	info->num_pkts_discarded++;
	return (FALSE);
}

/* ---- Private Functions -------------------------------------------------------- */
/*
*****************************************************************************
* Function:   pkt_filter_doiovar
*
* Purpose:    Handles packet filtering related IOVars.
*
* Parameters:
*
* Returns:    0 on success.
*****************************************************************************
*/
STATIC int
pkt_filter_doiovar
(
	void 			*hdl,
	uint32 			actionid,
	void 			*p,
	uint 			plen,
	void 			*a,
	uint 			alen,
	uint 			vsize,
	struct wlc_if 		*wlcif
)
{
	wlc_pkt_filter_info_t	*info;
	int err = BCME_OK;

	info = hdl;

	switch (actionid) {

		case IOV_SVAL(IOV_PKT_FILTER_ADD):
		{
			/* Install packet filter. */
			err = add_filter(info, a, alen);
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_DELETE):
		{
			uint32 id;
			bcopy(a, &id, sizeof(id));

			/* Uninstall packet filter. */
			err = delete_filter(info, id);
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_ENABLE):
		{
			wl_pkt_filter_enable_t	filter_enable;

			/* Make a local copy of the received buffer arg. Can't simply cast
			 * arg to a 'wl_pkt_filter_enable_t' pointer because the arg may not be
			 * aligned correctly.
			 */
			bcopy(a, &filter_enable, sizeof(filter_enable));

			/* Enable/disable filter. */
			err = enable_filter(info, filter_enable.id, (bool) filter_enable.enable);
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_MODE):
		{
			uint32 mode;
			bcopy(a, &mode, sizeof(mode));
			info->operation_mode = (uint8) mode;
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_MODE):
		{
			uint32 mode = (uint32) info->operation_mode;
			bcopy(&mode, a, sizeof(mode));
		}
		break;

#ifdef PKT_FILTER_LIST_SUPPORT
		case IOV_GVAL(IOV_PKT_FILTER_LIST):
		{
			unsigned int 		num_filters;
			wl_pkt_filter_list_t	*list;
			uint32 			enable;

			/* Determine which filter list to retrieve (enabled or disabled list). */
			bcopy(p, &enable, sizeof(enable));

			/* Check if the memory provided is sufficient */
			if (alen < sizeof(wl_pkt_filter_list_t)) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			list = a;
			alen -= WL_PKT_FILTER_LIST_FIXED_LEN;
			err = get_filters(info,
			                  (bool) enable,
			                  (wl_pkt_filter_t *)list->filter,
			                  alen,
			                  &num_filters);

			bcopy(&num_filters, &list->num, sizeof(list->num));

		}
		break;
#endif   /* PKT_FILTER_LIST_SUPPORT */

		case IOV_GVAL(IOV_PKT_FILTER_STATS):
		{
			uint32 			id;
			pkt_filter_t		*filter;
			wl_pkt_filter_stats_t	stats;

			/* Retrieve specified filter id. */
			bcopy(p, &id, sizeof(id));

			/* Get filter state that corresponds to specified filter id. */
			filter = find_filter(info, id);

			if (filter == NULL) {
				err = BCME_BADARG;
				break;
			}

			/* Return current stats. */
			stats.num_pkts_matched 		= filter->num_pkts_matched;
			stats.num_pkts_discarded 	= info->num_pkts_discarded;
			stats.num_pkts_forwarded	= info->num_pkts_forwarded;
			bcopy(&stats, a, sizeof(stats));
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_CLEAR_STATS):
		{
			uint32 		id;
			pkt_filter_t	*filter;

			/* Retrieve specified filter id. */
			bcopy(a, &id, sizeof(id));

			/* Get filter state that corresponds to specified filter id. */
			filter = find_filter(info, id);

			if (filter == NULL) {
				err = BCME_BADARG;
				break;
			}

			/* Clear debug stats. */
			filter->num_pkts_matched	= 0;
			info->num_pkts_discarded	= 0;
			info->num_pkts_forwarded	= 0;
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_MAX):
		{
			uint32 max_filters;
			bcopy(a, &max_filters, sizeof(max_filters));

			if (max_filters >= info->num_filters)
				info->max_num_filters = max_filters;
			else
				err = BCME_BADARG;
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_MAX):
		{
			uint32 max_filters = info->max_num_filters;
			bcopy(&max_filters, a, sizeof(max_filters));
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_ICMP):
		{
			bcopy(a, &info->toss_icmp, sizeof(uint32));
			if (info->toss_icmp)
				info->icmp_tossed = 0;
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_ICMP):
		{
			bcopy(&info->toss_icmp, a, sizeof(uint32));
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_ICMP_CNT):
		{
			bcopy(a, &info->icmp_tossed, sizeof(uint32));
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_ICMP_CNT):
		{
			bcopy(&info->icmp_tossed, a, sizeof(uint32));
		}
		break;

#ifdef PACKET_FILTER2
		case IOV_GVAL(IOV_PKT_FILTER_PORTS):
		{
			uint portlen;
			wl_pkt_filter_ports_t *portlist;

			ASSERT((info->ports && info->nports) ||
			       ((info->ports == NULL) && !info->nports));

			portlist = (wl_pkt_filter_ports_t*)a;
			portlen = info->nports * sizeof(uint16);
			if (alen < (WL_PKT_FILTER_PORTS_FIXED_LEN + portlen)) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			portlist->version = WL_PKT_FILTER_PORTS_VERSION;
			portlist->reserved = 0;
			portlist->count = info->nports;
			bcopy(info->ports, portlist->ports, portlen);
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_PORTS):
		{
			uint portlen;
			wl_pkt_filter_ports_t *portlist;
			wl_pkt_filter_ports_t l_portlist;

			ASSERT((info->ports && info->nports) ||
			       ((info->ports == NULL) && !info->nports));

			bcopy(a, &l_portlist, WL_PKT_FILTER_PORTS_FIXED_LEN);
			if (l_portlist.version != WL_PKT_FILTER_PORTS_VERSION) {
				err = BCME_VERSION;
				break;
			}

			if ((l_portlist.count > WL_PKT_FILTER_PORTS_MAX) ||
			    (l_portlist.reserved != 0)) {
				err = BCME_BADARG;
				break;
			}

			portlen = l_portlist.count * sizeof(uint16);
			if (alen < WL_PKT_FILTER_PORTS_FIXED_LEN + portlen) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			if (portlen == 0) {
				if (info->ports) {
					info->nports = 0;
					MFREE(WLCOSH(info), info->ports, FALSE);
					info->ports = NULL;
				}
				break;
			}

			if (info->ports == NULL) {
				info->ports = MALLOC(WLCOSH(info),
				                     WL_PKT_FILTER_PORTS_FIXED_LEN +
				                     WL_PKT_FILTER_PORTS_MAX*sizeof(uint16));
				if (info->ports == NULL) {
					err = BCME_NOMEM;
					break;
				}
			}

			info->nports = l_portlist.count;
			portlist = (wl_pkt_filter_ports_t*)a;
			bcopy(portlist->ports, info->ports, portlen);
			info->lastffwd = FALSE;

			if (PF2DBG_CFG_ON()) {
				uint portidx;
				printf("Config %d ports:\n", info->nports);
				for (portidx = 0; portidx < info->nports; portidx++)
					printf(" %d\n", info->ports[portidx]);
			}
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_PORTS_CHKCNT):
		{
			bcopy(a, &info->port_checked, sizeof(uint32));
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_PORTS_CHKCNT):
		{
			bcopy(&info->port_checked, a, sizeof(uint32));
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_PORTS_CNT):
		{
			bcopy(a, &info->port_tossed, sizeof(uint32));
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_PORTS_CNT):
		{
			bcopy(&info->port_tossed, a, sizeof(uint32));
		}
		break;

		case IOV_SVAL(IOV_PKT_FILTER_FRAGS_CNT):
		{
			bcopy(a, &info->frag_tossed, sizeof(uint32));
		}
		break;

		case IOV_GVAL(IOV_PKT_FILTER_FRAGS_CNT):
		{
			bcopy(&info->frag_tossed, a, sizeof(uint32));
		}
		break;

#ifdef PF2_DEBUG
		case IOV_SVAL(IOV_PF2DBG):
			bcopy(a, &pf2dbg, sizeof(uint32));
			break;

		case IOV_GVAL(IOV_PF2DBG):
			bcopy(&pf2dbg, a, sizeof(uint32));
			break;

		case IOV_SVAL(IOV_PF2DBGCNT):
			bcopy(a, &pf2dbgcnt, sizeof(uint32));
			break;

		case IOV_GVAL(IOV_PF2DBGCNT):
			bcopy(&pf2dbgcnt, a, sizeof(uint32));
			break;
#endif /* PF2_DEBUG */
#endif /* PACKET_FILTER2 */

#ifdef APF
		case IOV_GVAL(IOV_APF_VERSION):
		{
			uint32 apf_version = APF_VERSION;
			bcopy(&apf_version, a, sizeof(uint32));
		}

		break;
		case IOV_GVAL(IOV_APF_PROGRAM_SIZE_LIMIT):
		{
			uint32 limit = WL_APF_PROGRAM_SIZE_LIMIT(info);
			bcopy(&limit, a, sizeof(uint32));
		}
		break;
#endif /* APF */

		default:
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	return (err);
}

/*
*****************************************************************************
* Function: pkt_filter_watchdog
*
* Purpose: Keep a watch on packet filter activities. At present it checks
*		for time since last matching packet for each pattern timeout
*		filter and sends timeout event to host if matching packet is
*		not received in last X (timeout) seconds.
*
* Parameters: Packet filter engine private context (info).
*
* Returns: Nothing.
*****************************************************************************
*/
STATIC void
pkt_filter_watchdog(void *hdl)
{
	wlc_pkt_filter_info_t *info = (wlc_pkt_filter_info_t *) hdl;
	pkt_filter_t *filter;

	if (info->enabled_list == NULL) {
		WL_TRACE(("%s: filter list empty\n", __FUNCTION__));
		return;
	}

	filter = info->enabled_list;
	while (NULL != filter) {
		if (filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT &&
			filter->u.pattern_timeout->timeout > 0) {
			uint32 time_since_last_pkt = 0, now = 0;
			pkt_filter_pattern_timeout_t *pattern_timeout = filter->u.pattern_timeout;

			now = OSL_SYSUPTIME();
			if (now > pattern_timeout->time_since_last_pkt)
				time_since_last_pkt = now - pattern_timeout->time_since_last_pkt;
			else
				time_since_last_pkt =
					(uint32)~0 - pattern_timeout->time_since_last_pkt + now + 1;

			if (time_since_last_pkt >= (pattern_timeout->timeout * 1000)) {
				/* send timeout event */
				wlc_bss_mac_event(info->wlc, info->wlc->primary_bsscfg,
					WLC_E_PKT_FILTER, NULL, WLC_E_PKT_FILTER_TIMEOUT,
					filter->id, 0, 0, 0);
				/* reset time since last packet */
				pattern_timeout->time_since_last_pkt = OSL_SYSUPTIME();
				WL_TRACE(("Send pkt filter(id=%d) timeout event.\n", filter->id));
			}
		}
		filter = filter->next;
	}

	return;
}

/*
*****************************************************************************
* Function:   add_to_filter_list
*
* Purpose:    Helper function to add a filter to a linked list.
*
* Parameters: list       (mod) Add filter to this list.
*             pkt_filter (in)  Filter to add.
*
* Returns:    Nothing.
*****************************************************************************
*/
STATIC INLINE void
add_to_filter_list(pkt_filter_t **list, pkt_filter_t *pkt_filter)
{
	if ((pkt_filter->flags & WL_PKT_FILTER_FLAGS_PSEUDOMAGIC) == 0) {
		while (*list && (*list)->flags & WL_PKT_FILTER_FLAGS_PSEUDOMAGIC)
			list = &(*list)->next;
	}

	pkt_filter->next 	= *list;
	*list			= pkt_filter;
}

/*
*****************************************************************************
* Function:   delete_from_filter_list
*
* Purpose:    Helper function to delete a filter from a linked list.
*
* Parameters: list (mod) Remove filter from this list.
*             id   (in)  Id of filter to remove.
*
* Returns:    Removed filter. NULL on error.
*****************************************************************************
*/
STATIC pkt_filter_t*
delete_from_filter_list(pkt_filter_t **list, uint32 id)
{
	pkt_filter_t	*curr;
	pkt_filter_t	*prev;
	pkt_filter_t	*removed_filter;

	removed_filter = NULL;

	/* Iterate linked-list of filters, remove 'id'. */
	prev = curr = *list;
	while (NULL != curr) {
		if (curr->id == id) {

			/* Found filter to remove. */
			if (curr == prev) {
				/* Filter to remove is at front of list. */
				*list = curr->next;
			}
			else {
				/* Filter to remove is either in the middle or at the
				 * end of the list.
				 */
				prev->next = curr->next;
			}

			removed_filter = curr;
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	return (removed_filter);
}

/*
****************************************************************************
* Function:   find_in_filter_list
*
* Purpose:    Helper function to find a filter in a linked list.
*
* Parameters: list (mod) Find filter from this list.
*             id   (in)  Id of filter to find.
*
* Returns:    Filter. NULL on error.
*****************************************************************************
*/
STATIC pkt_filter_t*
find_in_filter_list(pkt_filter_t *list, uint32 id)
{
	pkt_filter_t *filter;

	/* Iterate linked-list of filters, remove 'id'. */
	filter = list;
	while (NULL != filter) {
		if (filter->id == id)
			return (filter);

		filter = filter->next;
	}

	return (NULL);
}

/*
*****************************************************************************
* Function:   add_filter
*
* Purpose:    Install a new packet filter.
*
* Parameters: info               (mod) Packet filter engine context state.
*             filter_not_aligned (in)  Parameters for filter to add. Note
*                                      that the struct may NOT be aligned
*                                      properly. Do not directly access
*                                      struct members.
*             data_len           (in)  Data length in bytes of filter.
*                                      Note that filter may contain
*                                      variable length arrays.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
add_filter
(
	wlc_pkt_filter_info_t	*info,
	const wl_pkt_filter_t	*filter_not_aligned,
	int 			data_len
)
{
	pkt_filter_t		*pkt_filter;
#ifdef BCMDBG_ERR
	int			wlc_unit;
#endif /* BCMDBG_ERR */
	osl_t			*osh;
	wl_pkt_filter_t 	pkt_filter_iovar;
	int 			rc;

#ifdef BCMDBG_ERR
	wlc_unit       = WLCUNIT(info);
#endif /* BCMDBG_ERR */
	osh	       = WLCOSH(info);

	PF2DBG_CFG(("In add: data_len %d\n", data_len));

	/* Make a local copy of the received filter iovar. Can't simply cast
	 * arg to a 'wl_pkt_filter_t' pointer because the arg may not be
	 * aligned correctly.
	 */
	bcopy(filter_not_aligned, &pkt_filter_iovar, WL_PKT_FILTER_FIXED_LEN);

	/* Place cap on maximum number of filters that can be added. */
	if (info->num_filters >= info->max_num_filters) {
		return (BCME_BADARG);
	}

	/* Validate filter type - only support pattern matching. */
	if ((WL_PKT_FILTER_TYPE_PATTERN_MATCH != pkt_filter_iovar.type) &&
#ifdef PACKET_FILTER2
		(PKT_FILTER2_ENAB(info->wlc->pub) &&
			WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH != pkt_filter_iovar.type) &&
#ifdef PACKET_FILTER6
		(PKT_FILTER6_ENAB(info->wlc->pub) &&
			WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH != pkt_filter_iovar.type) &&
#endif /* PACKET_FILTER6 */
#endif /* PACKET_FILTER2 */
#if defined(APF)
	(APF_ENAB(info->wlc->pub)) &&
	    (WL_PKT_FILTER_TYPE_APF_MATCH != pkt_filter_iovar.type) &&
#endif /* APF */
	    (WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH != pkt_filter_iovar.type) &&
	    (WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH != pkt_filter_iovar.type) &&
	    (WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT != pkt_filter_iovar.type)) {
		return (BCME_BADARG);
	}

	/* Validate that another filter with the same id doesn't already exist. */
	pkt_filter = find_filter(info, pkt_filter_iovar.id);
	if (pkt_filter != NULL) {
		return (BCME_BADARG);
	}

	/* Allocate memory for new packet filter. */
	pkt_filter = MALLOC(osh, sizeof(pkt_filter_t));
	if (pkt_filter == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc_unit, __FUNCTION__, MALLOCED(osh)));

		rc = BCME_NOMEM;
		goto fail;
	}

	/* Init packet filter. */
	bzero(pkt_filter, sizeof(pkt_filter_t));
	pkt_filter->id 			= pkt_filter_iovar.id;
	pkt_filter->negate_match	= (bool) pkt_filter_iovar.negate_match;
	pkt_filter->type		= pkt_filter_iovar.type;

	PF2DBG_CFG(("ID %d, negate %d, type %d; adding pattern\n",
	            pkt_filter->id, pkt_filter->negate_match, pkt_filter->type));
#if defined(APF)
	if (APF_ENAB(info->wlc->pub) &&
		pkt_filter->type == WL_PKT_FILTER_TYPE_APF_MATCH) {
			rc = add_apf_program(info, pkt_filter, filter_not_aligned, data_len);
	} else
#endif
	if (pkt_filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT) {
		rc = add_pattern_timeout_filter(info, pkt_filter,
				filter_not_aligned, data_len);
	} else {
		/* Init filter type-specific data. */
		rc = add_pattern_filter(info, pkt_filter, filter_not_aligned, data_len);
	}
	if (BCME_OK != rc)
		goto fail;

	/* Add newly created filter to list of disabled filters. */
	add_to_filter_list(&info->disabled_list, pkt_filter);

	info->num_filters++;
	return (BCME_OK);

fail:

	if (NULL != pkt_filter)
		MFREE(osh, pkt_filter, sizeof(pkt_filter_t));

	return (rc);
}

/*
*****************************************************************************
* Function:   delete_filter
*
* Purpose:    Remove a previously installed packet filter.
*
* Parameters: info (mod) Packet filter engine context state.
*             id   (in)  Id of filter to remove.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
delete_filter(wlc_pkt_filter_info_t *info, uint32 id)
{
	pkt_filter_t	*removed_filter;
	int		rc;
	osl_t		*osh;

	osh	= WLCOSH(info);
	rc	= BCME_BADARG;

	removed_filter = delete_from_filter_list(&info->disabled_list, id);

	if (removed_filter == NULL) {
		removed_filter = delete_from_filter_list(&info->enabled_list, id);
	}

	if (NULL != removed_filter) {

		/* Free allocated memory associated with the filter. */
		ASSERT((removed_filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_APF_MATCH) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT) ||
		       (removed_filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH));

#ifndef PACKET_FILTER6
		ASSERT(removed_filter->type != WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH);
#endif /* !PACKET_FILTER6 */
#ifndef PACKET_FILTER2
		ASSERT(removed_filter->type != WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH);
#endif /* PACKET_FILTER2 */
#ifndef APF
	ASSERT(removed_filter->type != WL_PKT_FILTER_TYPE_APF_MATCH);
#endif /* APF */
#if defined(APF)
	if (APF_ENAB(info->wlc->pub) &&
		(removed_filter->type == WL_PKT_FILTER_TYPE_APF_MATCH)) {
		delete_apf_program(info, removed_filter);
	} else
#endif /* APF */
		delete_pattern_filter(info, removed_filter);
		MFREE(osh, removed_filter, sizeof(pkt_filter_t));

		info->num_filters--;
		rc = BCME_OK;
	}

	return (rc);
}

/*
*****************************************************************************
* Function:    enable_filter
*
* Purpose:     Enable/disable a previously installed packet filter.
*
* Parameters:  info   (mod) Packet filter engine context state.
*              id     (in)  Id of filter to enable/disable.
*              enable (in)  Enable/disable.
*
* Returns:     BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
enable_filter(wlc_pkt_filter_info_t *info, uint32 id, bool enable)
{
	pkt_filter_t	*filter;
	pkt_filter_t	**src_list;
	pkt_filter_t	**dst_list;

	if (enable) {
		src_list = &info->disabled_list;
		dst_list = &info->enabled_list;
	}
	else {
		src_list = &info->enabled_list;
		dst_list = &info->disabled_list;
	}

	/* Check if filter is already in the correct state. */
	if (NULL != find_in_filter_list(*dst_list, id))
		return (BCME_OK);

	/* Move the filter to the appropriate list. */
	filter = delete_from_filter_list(src_list, id);
	if (NULL != filter) {
		if (enable &&
			filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT) {
			filter->u.pattern_timeout->time_since_last_pkt = OSL_SYSUPTIME();
		}
		add_to_filter_list(dst_list, filter);

		return (BCME_OK);
	}

	return (BCME_BADARG);
}

/*
*****************************************************************************
* Function:    find_filter
*
* Purpose:     Get filter state that corresponds to specified filter id.
*					Searches both the enabled and disabled filter list.
*
* Parameters:  info   (in) Packet filter engine context state.
*              id     (in)  Id of filter to enable/disable.
*
* Returns:     Pointer to filter state, NULL if not found.
*****************************************************************************
*/
STATIC pkt_filter_t*
find_filter(wlc_pkt_filter_info_t *info, uint32 id)
{
	pkt_filter_t	*filter;

	/* Get filter state that corresponds to specified filter id. */
	filter = find_in_filter_list(info->disabled_list, id);
	if (filter == NULL)
		filter = find_in_filter_list(info->enabled_list, id);

	return (filter);
}

#ifdef PKT_FILTER_LIST_SUPPORT
/*
*****************************************************************************
* Function:    get_filters
*
* Purpose:     Retrieve a list of filters for display by the host.
*              Used for debugging.
*
* Parameters:  info             (mod) Packet filter engine context state.
*              enable           (in)  List of filters to retrieve
*                                     (enabled or disabled).
*              dst_filter_iovar (out) Output buffer to fill with filter list.
*              buf_len          (in)  Length of output buffer in bytes.
*              num_filters      (out) Number of filters in the list.
*
* Returns:     BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
get_filters
(
	const wlc_pkt_filter_info_t	*info,
	bool				enable,
	wl_pkt_filter_t			*dst_filter_iovar,
	unsigned int 			buf_len,
	unsigned int 			*num_filters
)
{
	pkt_filter_t	*src_filter;
	int		rc;
	uint8		*dst_buf_end;
	unsigned int	num_bytes;

	rc = BCME_OK;
	*num_filters = 0;

	dst_buf_end = (uint8 *)dst_filter_iovar + buf_len;

	if (enable)
		src_filter = info->enabled_list;
	else
		src_filter = info->disabled_list;

	while (NULL != src_filter) {

		/* Check if the memory provided is sufficient */
		if ((dst_buf_end - (uint8 *)dst_filter_iovar) < WL_PKT_FILTER_FIXED_LEN) {
			rc = BCME_BUFTOOSHORT;
			*num_filters = 0;
			break;
		}

		/* Fill in generic filter state. */
		dst_filter_iovar->id 		= src_filter->id;
		dst_filter_iovar->negate_match	= src_filter->negate_match;
		dst_filter_iovar->type 		= src_filter->type;

		/* Fill in filter-type specific state. */
		ASSERT((src_filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) ||
		       (src_filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH) ||
		       (src_filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
		       (src_filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) ||
		       (src_filter->type == WL_PKT_FILTER_TYPE_APF_MATCH));
#ifndef PACKET_FILTER6
		ASSERT(src_filter->type != WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH);
#endif /* !PACKET_FILTER6 */
#ifndef PACKET_FILTER2
		ASSERT(src_filter->type != WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH);
#endif /* PACKET_FILTER2 */
#ifndef APF
		ASSERT(src_filter->type != WL_PKT_FILTER_TYPE_APF_MATCH);
#endif
#if defined(APF)
		if (APF_ENAB(info->wlc->pub) &&
			src_filter->type == WL_PKT_FILTER_TYPE_APF_MATCH) {
				num_bytes = get_apf_program(info, src_filter, dst_filter_iovar,
						dst_buf_end - (uint8 *)dst_filter_iovar);
			}
			else
#endif /* APF */
#ifdef PACKET_FILTER2
		if ((PKT_FILTER2_ENAB(info->wlc->pub) &&
				src_filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
#ifdef PACKET_FILTER6
			(PKT_FILTER6_ENAB(info->wlc->pub) &&
			src_filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) ||
#endif /* PACKET_FILTER6 */
			0) {
			num_bytes = get_pattern_list_filter(info,
			                                    src_filter,
			                                    dst_filter_iovar,
			                                    dst_buf_end -
			                                    (uint8 *)dst_filter_iovar);
		}
		else
#endif /* PACKET_FILTER2 */
		{
			num_bytes = get_pattern_filter(info, src_filter, dst_filter_iovar,
			                               dst_buf_end - (uint8 *)dst_filter_iovar);
		}

		if (num_bytes == 0) {
			rc = BCME_BUFTOOSHORT;
			*num_filters = 0;
			break;
		}

		/* Filter state may contain variable length arrays. Ensure that
		 * filter structures are aligned properly so that they can be
		 * indexed directly by the host.
		 */
		dst_filter_iovar = (wl_pkt_filter_t *) ((uint8 *)dst_filter_iovar + num_bytes);
		dst_filter_iovar = ALIGN_ADDR(dst_filter_iovar, sizeof(uint32));

		*num_filters	+= 1;
		src_filter = src_filter->next;
	}

	return (rc);
}
#endif   /* PKT_FILTER_LIST_SUPPORT */

/*
*****************************************************************************
* Function:   add_pattern_filter
*
* Purpose:    Install a new pattern matching packet filter.
*             Handle basic pattern, magic pattern, and pattern list.
*
* Parameters: info               (mod) Packet filter engine context state.
*             pkt_filter         (mod) Context for newly added filter.
*             filter_not_aligned (in)  Parameters for filter to add. Note
*                                      that the struct may NOT be aligned
*                                      properly. Do not directly access
*                                      struct members.
*             data_len           (in)  Data length in bytes of filter.
*                                      Note that filter may contain variable
*                                      length arrays.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
add_pattern_filter
(
	wlc_pkt_filter_info_t	*info,
	pkt_filter_t		*pkt_filter,
	const wl_pkt_filter_t	*filter_not_aligned,
	int			data_len
)
{
	wl_pkt_filter_pattern_t	*wl_pattern_filter;
#ifdef BCMDBG_ERR
	int			wlc_unit;
#endif /* BCMDBG_ERR */
	osl_t			*osh;
	unsigned int		i;
	int			rc;
	uint8			*mask;
	uint8			*pattern;
	uint32			maskoffset = 0;

#ifdef BCMDBG_ERR
	wlc_unit	= WLCUNIT(info);
#endif /* BCMDBG_ERR */
	osh		= WLCOSH(info);

	PF2DBG_CFG(("Add pattern: datalen %d\n", data_len));

	data_len -= WL_PKT_FILTER_FIXED_LEN;

	/* Allocate memory for pattern filter. */
	wl_pattern_filter = MALLOC(osh, data_len);
	if (wl_pattern_filter == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc_unit, __FUNCTION__, MALLOCED(osh)));

		rc = BCME_NOMEM;
		goto fail;
	}

	/* Init pattern filter. */
	bcopy(&filter_not_aligned->u.pattern, wl_pattern_filter, data_len);
	pkt_filter->u.pattern = wl_pattern_filter;

#ifdef PACKET_FILTER2
	if (PKT_FILTER2_ENAB(info->wlc->pub) || PKT_FILTER6_ENAB(info->wlc->pub)) {
		if ((pkt_filter->type == WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH) ||
#ifdef PACKET_FILTER6
			(pkt_filter->type == WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH) ||
#endif /* PACKET_FILTER6 */
			0) {
			int fnum;
			wl_pkt_filter_pattern_listel_t *listel;
			wl_pkt_filter_pattern_list_t *filter_list = pkt_filter->u.patlist;

			if (data_len < WL_PKT_FILTER_PATTERN_LIST_FIXED_LEN) {
				rc = BCME_BUFTOOSHORT;
				goto fail;
			}
			if (data_len != filter_list->totsize) {
				PF2DBG_CFG(("Data size %d doesn't match list size %d\n",
					data_len, filter_list->totsize));
				rc = BCME_BADARG;
				goto fail;
			}
			data_len -= WL_PKT_FILTER_PATTERN_LIST_FIXED_LEN;

			PF2DBG_CFG(("Adding pattern list of %d elements (total size %d)\n",
				filter_list->list_cnt, filter_list->totsize));

			listel = (wl_pkt_filter_pattern_listel_t *)filter_list->patterns;
			for (fnum = filter_list->list_cnt; fnum; fnum--) {
				uint32 elsize;

				/* Validate sufficient length remaining for this element */
				elsize = WL_PKT_FILTER_PATTERN_LISTEL_FIXED_LEN;
				elsize += (2 * listel->size_bytes);
				if (data_len < elsize) {
					rc = BCME_BUFTOOSHORT;
					goto fail;
				}

				/* Do the pre-mask for more efficient comparison */
				mask    = &listel->mask_and_data[0];
				pattern = mask + listel->size_bytes;
				for (i = 0; i < listel->size_bytes; i++) {
					pattern[i] &= mask[i];
				}

				if (PF2DBG_CFG_ON())
				{
					printf("Added element %d:\n",
						filter_list->list_cnt - fnum + 1);
					printf("Offset %d:%d, flags %d, size_bytes %d\n",
						listel->base_offs, listel->rel_offs,
						listel->match_flags, listel->size_bytes);
					printf("Mask:");
					for (i = 0; i < listel->size_bytes; i++)
						printf(" %02x", mask[i]);
					printf("\n");
					printf("Pattern:");
					for (i = 0; i < listel->size_bytes; i++)
						printf(" %02x", pattern[i]);
					printf("\n");
				}

				/* Move on to the next element */
				data_len -= elsize;
				listel =
					(wl_pkt_filter_pattern_listel_t *)((uint8*)listel + elsize);
			}

			if ((filter_list->list_cnt == 1) &&
			(((wl_pkt_filter_pattern_listel_t *)(filter_list->patterns))->base_offs
					== WL_PKT_FILTER_BASE_END)) {
				pkt_filter->flags |= WL_PKT_FILTER_FLAGS_PSEUDOMAGIC;
			}

			return (BCME_OK);
		}
	}
#endif /* PACKET_FILTER2 */

	if (pkt_filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) {
		maskoffset = sizeof(unsigned long);
	}

	/* Regular pattern and magic packet are identical */
	PF2DBG_CFG(("Comparing data_len %d with constant %d + 2*%d (%d)\n",
	            data_len, WL_PKT_FILTER_PATTERN_FIXED_LEN,
	            wl_pattern_filter->size_bytes,
	            WL_PKT_FILTER_PATTERN_FIXED_LEN + maskoffset
	             + 2 * wl_pattern_filter->size_bytes));

	/* Validate filter length. */
	if (data_len < (WL_PKT_FILTER_PATTERN_FIXED_LEN + maskoffset
		 + 2 * wl_pattern_filter->size_bytes)) {
		rc = BCME_BUFTOOSHORT;
		goto fail;
	}

	/* Apply the bitmask to the pattern. This will clear any bits in the pattern
	 * that are specified as don't-cares (0's) in the bitmask. This is performed
	 * in case the user set don't-care bits in the pattern. They are cleared
	 * now, when adding the filter for efficiency reasons - instead of clearing
	 * them when performing the comparison for every received packet.
	 */
	mask    = &wl_pattern_filter->mask_and_pattern[maskoffset];
	pattern = &wl_pattern_filter->mask_and_pattern[maskoffset + wl_pattern_filter->size_bytes];
	for (i = 0; i < wl_pattern_filter->size_bytes; i++)
		pattern[i] &= mask[i];

	if (PF2DBG_CFG_ON())
	{
		printf("Added pattern filter:\noffset %d\nsize_bytes %d\n",
		       wl_pattern_filter->offset, wl_pattern_filter->size_bytes);
		printf("Mask:");
		for (i = 0; i < wl_pattern_filter->size_bytes; i++)
			printf(" %02x", mask[i]);
		printf("\n");
		printf("Pattern:");
		for (i = 0; i < wl_pattern_filter->size_bytes; i++)
			printf(" %02x", pattern[i]);
		printf("\n");
	}

#ifdef PACKET_FILTER2
	if (PKT_FILTER2_ENAB(info->wlc->pub)) {
		if (((int32)wl_pattern_filter->offset < 0) ||
				(pkt_filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH)) {
			pkt_filter->flags |= WL_PKT_FILTER_FLAGS_PSEUDOMAGIC;
		}
	}
#endif /* PACKET_FILTER2 */

	return (BCME_OK);

fail:
	free_pattern_filter(info, wl_pattern_filter);

	return (rc);
}

STATIC int
add_pattern_timeout_filter
(
	wlc_pkt_filter_info_t *info,
	pkt_filter_t *pkt_filter,
	const wl_pkt_filter_t *filter_not_aligned,
	int data_len
)
{
#ifdef BCMDBG_ERR
	int wlc_unit;
#endif /* BCMDBG_ERR */
	osl_t *osh;
	unsigned int i;
	uint8 *mask, *pattern;
	pkt_filter_pattern_timeout_t *pattern_timeout;

#ifdef BCMDBG_ERR
	wlc_unit = WLCUNIT(info);
#endif /* BCMDBG_ERR */
	osh = WLCOSH(info);

	data_len -= WL_PKT_FILTER_FIXED_LEN;

	/* Validate filter length. */
	if (data_len != (WL_PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN +
		(2 * filter_not_aligned->u.pattern_timeout.size_bytes))) {
		return BCME_BUFTOOSHORT;
	}

	/* Calculate len including internal variables like time_since_last_pkt */
	data_len = PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN +
			(2 * filter_not_aligned->u.pattern_timeout.size_bytes);

	/* Allocate memory for pattern filter. */
	pattern_timeout = MALLOC(osh, data_len);
	if (pattern_timeout == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc_unit, __FUNCTION__, MALLOCED(osh)));
		return BCME_NOMEM;
	}
	pkt_filter->u.pattern_timeout = pattern_timeout;

	/* Initialize pattern timeout filter. */
	pattern_timeout->offset = filter_not_aligned->u.pattern_timeout.offset;
	pattern_timeout->size_bytes = filter_not_aligned->u.pattern_timeout.size_bytes;
	pattern_timeout->timeout = filter_not_aligned->u.pattern_timeout.timeout;
	memcpy(pattern_timeout->mask_and_pattern,
			&filter_not_aligned->u.pattern_timeout.mask_and_pattern,
			(2 * filter_not_aligned->u.pattern_timeout.size_bytes));
	pattern_timeout->time_since_last_pkt = 0;

	/* Apply the bitmask to the pattern. This will clear any bits in the pattern
	 * that are specified as don't-cares (0's) in the bitmask. This is performed
	 * in case the user set don't-care bits in the pattern. They are cleared
	 * now, when adding the filter for efficiency reasons - instead of clearing
	 * them when performing the comparison for every received packet.
	 */
	mask = &pattern_timeout->mask_and_pattern[0];
	pattern = &pattern_timeout->mask_and_pattern[pattern_timeout->size_bytes];
	for (i = 0; i < pattern_timeout->size_bytes; i++)
		pattern[i] &= mask[i];

	return BCME_OK;
}

/*
*****************************************************************************
* Function:   delete_pattern_filter
*
* Purpose:    Remove a previously installed pattern matching packet filter.
*
* Parameters: info   (mod) Packet filter engine context state.
*             filter (mod) Filter to remove.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
delete_pattern_filter(wlc_pkt_filter_info_t *info, pkt_filter_t *filter)
{
	/* Free allocated memory associated with pattern filter. */
	if (filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT)
		return (free_pattern_timeout_filter(info, filter->u.pattern_timeout));
	else
		return (free_pattern_filter(info, filter->u.pattern));
}

/*
*****************************************************************************
* Function:   free_pattern_filter
*
* Purpose:    Helper function to de-allocation memory associated with
*             pattern matching packet filter.
*
* Parameters: info   (mod) Packet filter engine context state.
*             filter (mod) Filter to remove.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
free_pattern_filter(wlc_pkt_filter_info_t *info, wl_pkt_filter_pattern_t *filter)
{
	if (NULL != filter)
		MFREE(WLCOSH(info),
		      filter,
		      WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * filter->size_bytes);

	return (BCME_OK);
}

STATIC int
free_pattern_timeout_filter(wlc_pkt_filter_info_t *info, pkt_filter_pattern_timeout_t *filter)
{
	if (NULL != filter)
		MFREE(WLCOSH(info),
		      filter,
		      PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN + 2 * filter->size_bytes);

	return (BCME_OK);
}

/*
*****************************************************************************
* Function:   run_pattern_filter
*
* Purpose:    Run the specified filter against a received packet. Determine
*             if the received packet matches the filter specification.
*
* Parameters: info      (mod) Packet filter engine context state.
*             filter   (mod)   Filter to run.
*             sdu      (in)   Received packet.
*
* Returns:    TRUE for filter match, else FALSE.
*****************************************************************************
*/
STATIC bool
run_pattern_filter(wlc_pkt_filter_info_t *info,	const pkt_filter_t *filter, void *sdu)
{
	uint8			*pkt;
	uint8			*pkt_offset;
	int 			pkt_len;
	osl_t			*osh = WLCOSH(info);
	wl_pkt_filter_pattern_t	*pattern_filter;
	int			i;
	bool			match;
	uint8			*mask;
	uint8			*pattern;
	int32                   pattern_filter_offset;
	uint32                  maskoffset = 0;

	/* Used for magic packet matching */
	uint8			*pkt_end;
	int32			size_bytes;

	pkt		= PKTDATA(osh, sdu);
	pkt_len 	= PKTLEN(osh, sdu);

	pattern_filter = filter->u.pattern;

	size_bytes = pattern_filter->size_bytes;
	if (filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH)
		maskoffset = sizeof(unsigned long);
	mask    = &pattern_filter->mask_and_pattern[maskoffset];
	pattern = &pattern_filter->mask_and_pattern[maskoffset + pattern_filter->size_bytes];

	if (filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH) {
		wl_pkt_decrypter_t **decrypt_ctx =
		 (wl_pkt_decrypter_t **)(pattern_filter->mask_and_pattern);
		pkt_offset = NULL;
		if (*decrypt_ctx)
			pkt_offset = (*decrypt_ctx)->dec_cb((*decrypt_ctx)->dec_ctx, sdu, 0);
		if (!pkt_offset) {
			match = FALSE;
			goto exit;
		}
	} else {
		if ((int32) pattern_filter->offset < 0)
			pattern_filter_offset = pkt_len + pattern_filter->offset;
		else
			pattern_filter_offset = pattern_filter->offset;

		if (pattern_filter_offset < 0) {
		  match = FALSE;
		  goto exit;
		}
		if ((pattern_filter_offset + pattern_filter->size_bytes) > pkt_len) {
			/* Received packet length is too short for match to be successful. */
			match = FALSE;
			goto exit;
		}
		pkt_offset = &pkt[pattern_filter_offset];
	}

	/* Compare the received packet against the filter pattern. Only compare
	 * bits specified by the filter bitmask. A match is found if all enabled
	 * bits of the pattern match the corresponding bits of the received packet.
	 */

	if ((filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH) ||
		(filter->type == WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH)) {
		int j;
		match = TRUE;
		for (i = 0, j = 0; i < size_bytes; i++, j++) {
			if ((mask[i] & pkt_offset[j]) != pattern[i]) {
				match = FALSE;
				break;
			}
		}
	} else if (filter->type == WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH) {
		match = FALSE;
		pkt_end = pkt + pkt_len - pattern_filter->size_bytes;

		while (pkt_offset <= pkt_end) {
			match = TRUE;
			for (i = 0; i < size_bytes; i++) {
				if ((mask[i] & pkt_offset[i]) != pattern[i]) {
					match = FALSE;
					break;
				}
			}
			if (match == TRUE) {
				break;
			}
			pkt_offset++;
		}
	} else if (filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT) {
		mask = &filter->u.pattern_timeout->mask_and_pattern[0];
		pattern = &filter->u.pattern_timeout->mask_and_pattern[pattern_filter->size_bytes];
		match = TRUE;
		for (i = 0; i < size_bytes; i++) {
			if ((mask[i] & pkt_offset[i]) != pattern[i]) {
				match = FALSE;
				break;
			}
		}

		if (match)
			filter->u.pattern_timeout->time_since_last_pkt = OSL_SYSUPTIME();
	} else {
		/* Not a valid pattern type, do nothing */
		match = FALSE;
	}

exit:
	if (filter->negate_match) {
		/* Flip polarity of filter match result. */
		match = !match;
	}

	PF2DBG_MATCH(("RunFltr(E) %c\n", (match ? 'T' : 'F')));

	if (match)
		WL_PF2_SET_MATCHED_FLAGS(info, filter);

	return (match);
}

#ifdef PACKET_FILTER2
/*
*****************************************************************************
* Function:   run_pattern_list_filter
*
* Purpose:    Run the specified filter against a received packet. Determine
*             if the received packet matches the filter specification.
*
* Parameters: info     (mod) Packet filter engine context state.
*             filter   (mod) Filter to run.
*             sdu      (in)  Received packet.
*
* Returns:    TRUE for filter match, else FALSE.
*****************************************************************************
*/
STATIC bool
run_pattern_list_filter(wlc_pkt_filter_info_t *info, const pkt_filter_t	*filter, void *sdu)
{
	wl_pkt_filter_pattern_list_t	*pattern_list;
	wl_pkt_filter_pattern_listel_t	*pattern_listel;

	uint8	cnt;
	int	pkt_len;
	bool	match;
	uint8	*pktdata;

	pattern_list = filter->u.patlist;
	pattern_listel = (wl_pkt_filter_pattern_listel_t *)pattern_list->patterns;
	pkt_len = PKTLEN(WLCOSH(info), sdu);
	pktdata = PKTDATA(WLCOSH(info), sdu);

	match = TRUE;

	PF2DBG_MATCH(("PFlist(S)\n"));

	/* Compare each list element */
	for (cnt = 0; cnt < pattern_list->list_cnt; cnt++) {
		uint8 *mask, *pattern, *pktoffs;
		uint16 base, rel, size, offset;
		int i;

		base = pattern_listel->base_offs;
		rel = pattern_listel->rel_offs;
		size = pattern_listel->size_bytes;
		offset = 0;

		PF2DBG_MATCH(("PFE %d/%d\n", cnt, pattern_list->list_cnt));

		if (info->base_offs[base] == -1) {
			PF2DBG_MATCH(("Bad Base %d\n", base));
			match = FALSE;
			break;
		}

		if ((base == WL_PKT_FILTER_BASE_END) && (rel <= pkt_len))
			offset = pkt_len - rel;
		else
			offset = info->base_offs[base] + rel;

		if (offset + size > pkt_len) {
			PF2DBG_MATCH(("Short: len %d < %d + %d\n",
			              pkt_len, offset, size));
			match = FALSE;
			break;
		}

		PF2DBG_MATCH(("base %d [%d] rel %d => %d, flgs 0x%04x size %d\n",
		              base, info->base_offs[base], rel, offset,
		              pattern_listel->match_flags, size));
		if (PF2DBG_MATCH_ON()) {
			printf("Mask:\n");
			for (i = 0; i < size; i++) {
				printf("%02x ", pattern_listel->mask_and_data[i]);
				if ((i & 0x0f) == 0x0f)
					printf("\n");
			}
			if ((i & 0x0f))
				printf("\n");
			printf("Patn:\n");
			for (i = size; i < size * 2; i++) {
				printf("%02x ", pattern_listel->mask_and_data[i]);
				if ((i & 0x0f) == 0x0f)
					printf("\n");
			}
			if ((i & 0x0f))
				printf("\n");
		}

		/* Actually going to do the match: set up pointers and compare */
		mask = &pattern_listel->mask_and_data[0];
		pattern = &pattern_listel->mask_and_data[size];
		pktoffs = &pktdata[offset];

		for (i = 0; i < size; i++) {
			if ((mask[i] & pktoffs[i]) != pattern[i]) {
				match = FALSE;
				break;
			}
		}
		if (pattern_listel->match_flags & WL_PKT_FILTER_MFLAG_NEG)
			match = !match;

		if (!match) {
			PF2DBG_MATCH(("PFE fail\n"));
			break;
		}

		pattern_listel = (wl_pkt_filter_pattern_listel_t*)((uint8*)pattern_listel +
		        WL_PKT_FILTER_PATTERN_LISTEL_FIXED_LEN + 2 * size);
	}

	if (filter->negate_match) {
		/* Flip polarity of filter match result. */
		match = !match;
	}

	if (match)
		WL_PF2_SET_MATCHED_FLAGS(info, filter);

	PF2DBG_MATCH(("PFlist(E) %c\n", (match ? 'T' : 'F')));
	return (match);
}

/*
*****************************************************************************
* Function:   wlc_pkt_filter_porttoss
*
* Purpose:    Check to forward only specified TCP/UDP ports, toss others
*
* Parameters:
*		info    (mod)	Packet filter engine context state.
*		sdu	(in)   	Received packet.
*
* Returns:	TRUE if packet should be tossed (fails port check)
*		FALSE otherwise (continue to forward packet to host)
*****************************************************************************
*/
STATIC bool
wlc_pkt_filter_porttoss(wlc_pkt_filter_info_t *info, void *sdu)
{
	int16	port_offs;	/* Offset in packet, index in portlist */
	uint16	portnum;	/* Destination port number from packet */

	ASSERT(sdu && PKTDATA(WLCOSH(info), sdu));

	PF2DBG_MATCH(("Ports(S): (%d, %d)\n",
	              info->base_offs[WL_PKT_FILTER_BASE_TCP_H],
	              info->base_offs[WL_PKT_FILTER_BASE_UDP_H]));

	/* Check for not filtering */
	if (!info->ports || !info->nports)
		return FALSE;

	/* Forward irrelevant packets (not TCP/UDP) */
	if ((info->ipprot != IP_PROT_TCP) && (info->ipprot != IP_PROT_UDP))
		return FALSE;

	/* Otherwise, will truly attempt to check it */
	info->port_checked++;

	/* If we don't have port info (non-initial fragment) treat as last initial frag */
	if (((port_offs = info->base_offs[WL_PKT_FILTER_BASE_TCP_H]) == -1) &&
	    ((port_offs = info->base_offs[WL_PKT_FILTER_BASE_UDP_H]) == -1)) {
		if (info->ipfrag) {
			if (info->lastffwd)
				return FALSE;

			info->frag_tossed++;
			return TRUE;
		}
		return FALSE;
	}

	/* Ports locations in UDP are same as TCP */
	port_offs += TCP_DEST_PORT_OFFSET;

	/* If we can't locate the port in this packet, fail */
	if ((port_offs + 2) > info->base_offs[WL_PKT_FILTER_BASE_END]) {
		if (info->ipfrag) {
			info->frag_tossed++;
			info->lastffwd = FALSE;
		} else {
			PF2DBG_MATCH(("Port offset %d > %d\n", port_offs,
			              info->base_offs[WL_PKT_FILTER_BASE_END]));
			info->port_tossed++;
		}
		return TRUE;
	}

	/* Port is available in this packet: grab it... */
	bcopy((PKTDATA(WLCOSH(info), sdu) + port_offs), &portnum, sizeof(uint16));
	portnum = ntoh16(portnum);
	PF2DBG_MATCH(("Offset %d, port %d, nports %d\n", port_offs, portnum, info->nports));

	/* ...and compare against the list */
	for (port_offs = 0; port_offs < info->nports; port_offs++) {
		PF2DBG_MATCH(("%d vs. %d\n", portnum, info->ports[port_offs]));
		if (portnum == info->ports[port_offs]) {
			/* Matched: return don't TOSS, update frag fwd flag if needed */
			if (info->ipfrag)
				info->lastffwd = TRUE;
			return FALSE;
		}
	}

	/* Didn't match: return TOSS, count it, update frag fwd flag if needed */
	info->port_tossed++;
	if (info->ipfrag)
		info->lastffwd = FALSE;
	return TRUE;
}
#endif /* PACKET_FILTER2 */

#if defined(APF)
static int
add_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *pkt_filter,
const wl_pkt_filter_t *filter_not_aligned, int data_len)
{
	wl_apf_program_t *apf_program;
	osl_t *osh;
	uint16 version;
	PF2DBG_CFG(("Add APF: datalen: %d\n", data_len));
	osh	= WLCOSH(info);
	version = filter_not_aligned->u.apf_program.version;
	data_len -= WL_PKT_FILTER_FIXED_LEN;
	/* validate structure version */
	if (version != WL_APF_INTERNAL_VERSION) {
		WL_ERROR(("wl%d: %s: APF: incorrect version, version=%d, "
			"expected version=%d\n", WLCUNIT(info), __FUNCTION__,
			version, WL_APF_INTERNAL_VERSION));
		return BCME_BADARG;
	}

	/* validate memory limit of APF program */
	if (WL_APF_PROGRAM_SIZE_LIMIT(info) < data_len) {
		WL_ERROR(("wl%d: %s: APF program size limit exceeded, "
			"maximum size=%d, installed size=%d, program size=%d\n",
			WLCUNIT(info), __FUNCTION__, WL_APF_PROGRAM_MAX_SIZE,
			info->apf_program_tot_size, data_len));

	return BCME_NORESOURCE;

	}

	/* allocate memory for APF program. */
	apf_program = MALLOCZ(osh, data_len);

	if (apf_program == NULL) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed, malloced %d bytes\n",
			WLCUNIT(info), __FUNCTION__, MALLOCED(osh)));
		return BCME_NOMEM;
	}

	/* copy APF program */
	apf_program->version = version;
	apf_program->instr_len = filter_not_aligned->u.apf_program.instr_len;
	memcpy(apf_program->instrs, &filter_not_aligned->u.apf_program.instrs,
		WL_APF_PROGRAM_LEN(apf_program));
	apf_program->inst_ts = OSL_SYSUPTIME();
	pkt_filter->u.apf_program = apf_program;

	WL_APF_PROGRAM_INC_USAGE_SIZE(info, WL_APF_PROGRAM_TOTAL_LEN(apf_program));

	dump_apf_program(apf_program);

	return BCME_OK;
}

static int
delete_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *filter)
{
	wl_apf_program_t *apf_program;
	unsigned int apf_program_size;
	int ret = BCME_OK;

	apf_program = filter->u.apf_program;
	apf_program_size = WL_APF_PROGRAM_TOTAL_LEN(apf_program);

	ret = free_apf_program(info, apf_program);
	if (ret == BCME_OK) {
		WL_APF_PROGRAM_DEC_USAGE_SIZE(info, apf_program_size);
		filter->u.apf_program = NULL;
}

	return ret;
}

static int
free_apf_program(wlc_pkt_filter_info_t *info, wl_apf_program_t *apf_program)
{
	if (NULL != apf_program) {
		MFREE(WLCOSH(info), apf_program,
			WL_APF_PROGRAM_TOTAL_LEN(apf_program));
	}

	return BCME_OK;
}

static bool
run_apf_program(wlc_pkt_filter_info_t *info, pkt_filter_t *filter,
	const void *sdu)
{
	wl_apf_program_t *apf_program;
	osl_t *osh;
	uint8 *pkt;
	uint32 pkt_len;
	uint32 filter_age;
	uint32 ret;
	bool match;

	osh =  WLCOSH(info);
	pkt	= PKTDATA(osh, sdu);
	pkt_len = PKTLEN(osh, sdu);
	apf_program = filter->u.apf_program;
	filter_age = WL_APF_FILTER_AGE_IN_SEC(apf_program);

	ret = accept_packet(apf_program->instrs, apf_program->instr_len,
			pkt, pkt_len, filter_age);

	match = ret ? TRUE : FALSE;
	if (filter->negate_match) {
		match = !match;
	}

	PF2DBG_MATCH(("APF (id:%d) match - %s\n", filter->id,
		(match ? "TRUE" : "FALSE")));

	return match;
}

static unsigned int
get_apf_program(const wlc_pkt_filter_info_t *info,
	const pkt_filter_t *src, wl_pkt_filter_t *dst,
	unsigned int buf_len)
{
	unsigned int filter_len;
	unsigned int apf_program_size;

	apf_program_size = WL_APF_PROGRAM_TOTAL_LEN(src->u.apf_program);
	filter_len = WL_PKT_FILTER_FIXED_LEN + apf_program_size;

	if (buf_len < filter_len) {
		return (0);
	}

	memcpy(&dst->u.apf_program, src->u.apf_program, apf_program_size);

	return (filter_len);
}

#ifdef PF2_DEBUG
static void
dump_apf_program(wl_apf_program_t *apf_program)
{
	uint8 *pc = apf_program->instrs;
	uint32 instr_len = apf_program->instr_len;

	if (PF2DBG_CFG_ON())
	{
		uint32 count;

		WL_PRINT("APF program len :%d\n", instr_len);
		WL_PRINT("APF program     :");
		for (count = 0; pc && (count < instr_len); pc++, count++) {
			printf("%02X", *pc);
		}
		WL_PRINT("\n");
	}
}
#else
static void
dump_apf_program(wl_apf_program_t *apf_program)
{
	return;
}
#endif /* PF2_DEBUG */
#endif /* APF */

#ifdef PKT_FILTER_LIST_SUPPORT
/*
****************************************************************************
* Function:   get_pattern_filter
*
* Purpose:    Retrieve state for a pattern filter for display by the host.
*             Used for debugging.
*
* Parameters: info             (mod) Packet filter engine context state.
*             src_filter       (in)  Source filter state.
*             dst_filter_iovar (out) Output buffer to fill with filter state.
*             buf_len          (in)  Length of output buffer in bytes.
*
* Returns:    Number of bytes copied to output buffer.
*****************************************************************************
*/
STATIC unsigned int
get_pattern_filter
(
	const wlc_pkt_filter_info_t	*info,
	const pkt_filter_t		*src_filter,
	wl_pkt_filter_t			*dst_filter_iovar,
	unsigned int 			buf_len
)
{
	unsigned int	filter_len;
	unsigned int	pattern_filter_len;
	unsigned int	pattern_len;

	if (src_filter->type == WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT) {
		pattern_len = src_filter->u.pattern_timeout->size_bytes;
		pattern_filter_len = WL_PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN + 2 * pattern_len;
		filter_len = WL_PKT_FILTER_FIXED_LEN + pattern_filter_len;

		if (buf_len < filter_len)
			return (0);

		dst_filter_iovar->u.pattern_timeout.offset = src_filter->u.pattern_timeout->offset;

		dst_filter_iovar->u.pattern_timeout.size_bytes =
			src_filter->u.pattern_timeout->size_bytes;

		dst_filter_iovar->u.pattern_timeout.timeout =
			src_filter->u.pattern_timeout->timeout;

		memcpy(&dst_filter_iovar->u.pattern_timeout.mask_and_pattern,
			src_filter->u.pattern_timeout->mask_and_pattern, (2 * pattern_len));
	} else {
		pattern_len = src_filter->u.pattern->size_bytes;
		pattern_filter_len = WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * pattern_len;
		filter_len = WL_PKT_FILTER_FIXED_LEN + pattern_filter_len;

		if (buf_len < filter_len)
			return (0);

		bcopy(src_filter->u.pattern, &dst_filter_iovar->u.pattern, pattern_filter_len);
	}

	return (filter_len);
}

#ifdef PACKET_FILTER2
/*
****************************************************************************
* Function:   get_pattern_list_filter
*
* Purpose:    Retrieve state for a pattern list filter for display by the host.
*             Used for debugging.
*
* Parameters: info             (mod) Packet filter engine context state.
*             src_filter       (in)  Source filter state.
*             dst_filter_iovar (out) Output buffer to fill with filter state.
*             buf_len          (in)  Length of output buffer in bytes.
*
* Returns:    Number of bytes copied to output buffer.
*****************************************************************************
*/
STATIC unsigned int
get_pattern_list_filter
(
	const wlc_pkt_filter_info_t	*info,
	const pkt_filter_t		*src_filter,
	wl_pkt_filter_t			*dst_filter_iovar,
	unsigned int 			buf_len
)
{
	unsigned int	pattern_filter_len = src_filter->u.patlist->totsize;
	unsigned int	filter_len = WL_PKT_FILTER_FIXED_LEN + pattern_filter_len;

	if (buf_len < filter_len)
		return (0);

	bcopy(src_filter->u.patlist, &dst_filter_iovar->u.patlist, pattern_filter_len);

	return (filter_len);
}
#endif /* PACKET_FILTER2 */
#endif   /* PKT_FILTER_LIST_SUPPORT */
