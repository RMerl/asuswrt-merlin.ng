/**
 * @file DHD Station Buffer Fairness functionality code
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_sbf.c 821234 2023-02-06 14:16:52Z $
 */
#if defined(DHD_SBF)
#include <typedefs.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmmsgbuf.h>
#include <bcmendian.h>
#include <pcie_core.h>
#include <bcmpcie.h>
#include <bcmtcp.h>
#include <bcm_ring.h>
#include <siutils.h>
#include <bcm_sbftbl.h>

#if defined(BCM_PKTFWD)
#include <bcm_pktfwd.h>
#endif /* BCM_PKTFWD */

#include <dhd.h>
#include <dhd_dbg.h>
#include <dhd_bus.h>
#include <dhd_pcie.h>
#include <dhd_proto.h>
#include <dhd_flowring.h>
#include <dhd_linux.h>
#include <dhd_sbf.h>

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#if defined(STB) || defined(STBAP) || (defined(CMWIFI) && (LINUX_VERSION_CODE >= \
	KERNEL_VERSION(5, 4, 0)))
/* Handle the argument difference of the osl_cache_flush/inv() function */
#undef OSHPTR
#define OSHPTR dhdp->osh
#endif /* ! (STB || STBAP || (CMWIFI && KERNEL >= (5, 4, 0)) */
/*
 * Station Buffer Fairness Table (SBFTBL) section
 */
#define DHD_SBFTBL(sbf)             ((sbf)->table.hdr)
#define DHD_SBF_BFWTBL(sbf)         ((sbf)->table.bfwtbl)
#define DHD_SBF_MACTBL(sbf)         ((sbf)->table.addrtbl)

/* sbftbl info maintained in dhd */
typedef struct dhd_sbftbl {
	sbftbl_header_t *hdr;           /* header in hme area */
	sbftbl_weight_t *bfwtbl;        /* BFW table in hme area */
	sbftbl_addr_t   *addrtbl;       /* Station EA table in hme area */
	uint16          addrtblsize;    /* Station EA table size in hme area */
	uint16          entries;        /* number of table entries */
	uint16          type;           /* table type ? */
} dhd_sbftbl_t;

/**
 * Host Flow Control modes
 */
#define DHD_SBF_HFC_MAP             0    /* use static map */
#define DHD_SBF_HFC_SYSBUF          1    /* system buffers per radio */

/**
 * HFC using static Queue threshold mapping Section
 */
#define DHD_SBF_MAP_LEVELS_MAX      32
#define DHD_SBF_MAP_LEVELS_DEF      5
#define DHD_SBF_MAP_LEVELS(sbf)     ((sbf)->map.levels)

typedef struct dhd_sbf2thresh {
	uint16 levels;
	uint16 weights[DHD_SBF_MAP_LEVELS_MAX];
	uint16 thresholds[DHD_SBF_MAP_LEVELS_MAX];
} dhd_sbf2thresh_t;

static const uint16 dhd_sbf_level_weights[DHD_SBF_MAP_LEVELS_DEF] = {
	0,     /* level1: 0    - 255   threshold: 0%   */
	256,   /* level2: 255  - 511   threshold: 25%  */
	512,   /* level3: 512  - 1023  threshold: 50%  */
	1024,  /* level4: 1024 - 2047  threshold: 75%  */
	2048   /* level5: 2048 -       threshold: 100% */
};

/**
 * HFC using system buffers per radio
 */
#define DHD_SBF_SYS_BUFFERS_INV     0xFFFFFFFF
#define DHD_SBF_STA_THRESHOLD_MAX   2048
#define DHD_SBF_STA_WEIGHT_MAX      4096
#define DHD_SBF_WEIGHT_LEVEL_MASK   0x1FE1

typedef struct sbf_hfc_sysbuf {
	uint32 threshold;         /* #of system buffers per radio */

	/* Used per polling interval */
	uint32 cumm_weight;       /* cummulative weight of all stations */
	uint32 num_sta;           /* total number of stations */
} sbf_hfc_sysbuf_t;

/**
 * SBF debug log section
 */
#define SBF_NOOP                    do { } while (0)

#define SBF_DEBUG_BUILD

#ifdef SBF_DEBUG_BUILD
#define SBF_DBGLVL_NONE             0
#define SBF_DBGLVL_INIT             1
#define SBF_DBGLVL_CHANGE           2
#define SBF_DBGLVL_POLL             3
#define SBF_LOG(fmt, arg...)        printf("SBF: " fmt, ##arg)
#define SBF_LOG_INIT(fmt, arg...)   \
	do {                            \
	    if (dhd_sbf_msglevel >= SBF_DBGLVL_INIT) SBF_LOG(fmt, ##arg); \
	} while (0)
#define SBF_LOG_CHANGE(fmt, arg...) \
	do {                            \
	    if (dhd_sbf_msglevel >= SBF_DBGLVL_CHANGE) SBF_LOG(fmt, ##arg); \
	} while (0)
#define SBF_LOG_POLL(fmt, arg...)   \
	do {                            \
	    if (dhd_sbf_msglevel >= SBF_DBGLVL_POLL) SBF_LOG(fmt, ##arg); \
	} while (0)
#define SBF_ASSERT(expr)            ASSERT(expr)
#else  /* ! SBF_DEBUG_BUILD */
#define SBF_LOG(fmt, arg...)        SBF_NOOP
#define SBF_LOG_INIT(fmt, arg...)   SBF_NOOP
#define SBF_LOG_CHANGE(fmt, arg...) SBF_NOOP
#define SBF_LOG_POLL(fmt, arg...)   SBF_NOOP
#define SBF_ASSERT(expr)            SBF_NOOP
#endif /* ! SBF_DEBUG_BUILD */

#define SBF_DUMP(p, fmt, arg...)            \
	do {                                    \
	    if (p) bcm_bprintf(p, fmt, ##arg);  \
	    else SBF_LOG(fmt, ##arg);           \
	} while (0)

/**
 * SBF module section
 */
#define DHD_SBF_INFO(dhdp)          ((dhd_sbf_info_t*)(dhdp)->sbf)
#define DHD_SBF_ENABLED(sbf)        ((sbf)->enable)

#define DHD_SBF_UPDATE_INTERVAL     1000 /* Milli Seconds */
#define DBS_SBF_UPDATE_FACTOR       10   /* 20 times Dongle interval */
#define DHD_SBF_UPDATE_INTERVAL_DEF 0    /* Default disabled */

/* TCP ACK grace threshold */
#define DHD_SBF_GRACE_THRESHOLD     8    /* 8 packets per station */

#define	DHD_SBF_TCPACKSZMIN	        \
	(ETHER_HDR_LEN + IPV4_MIN_HEADER_LEN + TCP_MIN_HEADER_LEN)
/* Size of MAX possible TCP ACK packet. Extra bytes for IP/TCP option fields */
#define	DHD_SBF_TCPACKSZMAX	       (DHD_SBF_TCPACKSZMIN + 100)

/* SBF module control block */
typedef struct dhd_sbf_info
{
	bool     enable;                /* enable status */

	/* Protocol Section */
	dhd_sbftbl_t table;             /* SBFTBL information */

	/* Timer Section */
	uint16   wd_cnt;                /* current #of watchdog timer expires */
	uint16   wd_cnt_expiry;         /* #of watchdog expires for sbf timer */
	int      wd_interval;           /* DHD watchdog interval */

	/* Radio HFC */
	int      hfc_mode;              /* DHD_SBF_HFC_XXX */

	/* Threshold map Section DHD_SBF_HFC_MAP */
	dhd_sbf2thresh_t map;           /* HFC using static mapping table */

	/* Threshold map Section DHD_SBF_HFC_SYSBUF */
	sbf_hfc_sysbuf_t sysbuf;        /* HFC using sysbuf thresholds */

	/* grace threshold */
	uint16   grace_threshold;       /* Grace threshold for TCP ACK */
	ulong    grace_pktsqueued;      /* pkts queued due to grace threshold */
	ulong    grace_pktschecked;     /* pkts checked for grace threshold */
} dhd_sbf_info_t;

#ifdef SBF_DEBUG_BUILD
int dhd_sbf_msglevel = SBF_DBGLVL_INIT;
#endif /* SBF_DEBUG_BUILD */

/*
 * Station Buffer Fairness Table (SBFTBL) section
 */
static uint16 dhd_sbftbl_get_index(struct dhd_pub *dhdp, dhd_sbf_info_t *sbf,
	uint8 *ea);
static uint16 dhd_sbftbl_get_weight(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf,
	uint16 idx);
static int dhd_sbftbl_init(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf);
static int dhd_sbftbl_fini(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf);

/**
 * Queue threshold mapping Section
 */
static void dhd_sbf_dump_threshold_map(dhd_sbf_info_t *sbf,
	struct bcmstrbuf *b);
static INLINE uint16 dhd_sbf_get_threshold_map(dhd_sbf_info_t *sbf,
	uint16 weight);
static int dhd_sbf_init_threshold_map(dhd_sbf_info_t *sbf, uint8 unit,
	uint16 sta_threshold);

/**
 * Flowring Queue Section
 */
static bool BCMFASTPATH dhd_sbf_is_favoredpkt(dhd_pub_t *dhdp, void *pkt);

/**
 * Station Section
 */
static int dhd_sbf_set_sta_threshold(dhd_pub_t* dhdp, dhd_sta_t *sta,
	uint16 weight);
static int dhd_sbf_wd_sta_hfc_sysbuf_get(dhd_pub_t *dhdp, dhd_sta_t *sta,
	void* arg);
static int dhd_sbf_wd_sta_hfc_sysbuf_apply(dhd_pub_t *dhdp, dhd_sta_t *sta,
	void* arg);
static int dhd_sbf_wd_sta_hfc_map(dhd_pub_t *dhdp, dhd_sta_t *sta, void* arg);
static int dhd_sbf_reset_sta(dhd_pub_t *dhdp, dhd_sta_t *sta, void* arg);
static int dhd_sbf_dump_sta(dhd_pub_t *dhdp, dhd_sta_t *sta, void* arg);

/*
 * Station Buffer Fairness Table (SBFTBL) section
 */

/**
 * dhd_sbftbl_get_index - Given a station mac address, return index of the
 * sbftable entry
 *
 * returns ID16_INVALID if station is not found in the sbftbl
 */
static uint16
dhd_sbftbl_get_index(struct dhd_pub *dhdp, dhd_sbf_info_t *sbf, uint8 *ea)
{
	dhd_sbftbl_t *sbftbl = &sbf->table;
	uint16       index = ID16_INVALID;

	if (DHD_SBFTBL(sbf) == NULL) {
	    /* Check if dongle initiailized the table */
	    dhd_sbftbl_init(dhdp, sbf);
	}

	if (DHD_SBFTBL(sbf) != NULL) {
	    int    idx;

	    /* search the entire mac table for a matching station mac address */
	    OSL_CACHE_INV(sbftbl->addrtbl, sbftbl->addrtblsize);

	    /* Dongle usually allocates from the top index */
	    for (idx = (sbftbl->entries - 1); idx >= 0; idx--) {
	        sbftbl_addr_t *paddr = sbftbl->addrtbl + idx;

	        if (memcmp(ea, paddr->octet, ETHER_ADDR_LEN) == 0) {
	            /* Found the station entry in the table */
	            index = idx;
	            break;
	        }
	    }
	}

	return index;
} // dhd_sbftbl_get_index

/**
 * dhd_sbftbl_get_weight - Given a bfwtbl entry index, return the sbf weight
 *
 * returns max weight value, if index is out of range
 */
static uint16
dhd_sbftbl_get_weight(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf, uint16 idx)
{
	dhd_sbftbl_t    *sbftbl = &sbf->table;
	uint16          weight = SBFTBL_WEIGHT_MAX;

	if (DHD_SBFTBL(sbf) == NULL) {
	    /* Check if dongle initiailized the table */
	    dhd_sbftbl_init(dhdp, sbf);
	}

	if ((sbftbl->bfwtbl != NULL) && (idx < sbftbl->entries)) {
	    sbftbl_weight_t *pweight = sbftbl->bfwtbl + idx;

	    weight = *(pweight);
	}

	return weight;
} // dhd_sbftbl_get_weight

/**
 * dhd_sbftbl_init - Initialize sbftbl information
 *
 * returns
 *
 *  BCME_UNSUPPORTED: feature not supported
 *  BCME_OK:          On success
 */
static int
dhd_sbftbl_init(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf)
{
	dhd_dma_buf_t   *hme_dma;
	dhd_sbftbl_t    *sbftbl = &sbf->table;
	sbftbl_header_t *hdr;
	uint16          entry;

	if (DHD_SBFTBL(sbf)) {
	    /* Already initialized, nothing to do */
	    return BCME_OK;
	}

	/* Get the SBFTBL area in the HME */
	hme_dma = dhd_prot_hme_get_dma_buf(dhdp, PCIE_IPC_HME_USER_SBFTBL);
	if (hme_dma == NULL) {
	    DHD_ERROR(("%s: SBFTBL DMA buffer not allocated.\n", __FUNCTION__));
	    return BCME_UNSUPPORTED;
	}

	/* Read SBFTBL header information */
	hdr = (sbftbl_header_t*)(hme_dma->va);
	OSL_CACHE_INV(hdr, SBFTBL_HEADER_LEN);

	if (hdr->signature == SBFTBL_START_SIGNATURE) {
	    /* Dongle updated the start signature, table ready to use */
	    sbftbl->hdr = hdr;

	    sbftbl->bfwtbl = (sbftbl_weight_t*)(hme_dma->va + hdr->bfwtbl_offset);
	    sbftbl->addrtbl = (sbftbl_addr_t*)(hme_dma->va + hdr->mactbl_offset);
	    sbftbl->addrtblsize = hdr->num_entries * SBFTBL_ADDRESS_LEN;
	    sbftbl->entries = hdr->num_entries;
	    sbftbl->type = hdr->entry_type;

	    SBF_LOG_INIT("DNGL " SBFTBL_VRP_FMT " hme [0x%px] size [%d]\n",
	        SBFTBL_VRP_VAL(hdr->version_code), hdr, hme_dma->len);
	    SBF_LOG_INIT("             entries [%d] type [%d] offsets [%d] [%d]\n",
	        hdr->num_entries, hdr->entry_type, hdr->bfwtbl_offset,
	        hdr->mactbl_offset);

	    SBF_LOG_INIT("HOST " SBFTBL_VRP_FMT " Initialized Type [%d]\n",
	        SBFTBL_VRP_VAL(SBFTBL_VERSIONCODE), sbftbl->type);
	    SBF_LOG_INIT("hdr [0x%px] bfwtbl [0x%px] addrtbl [0x%px]\n",
	        sbftbl->hdr, sbftbl->bfwtbl, sbftbl->addrtbl);

	    SBF_LOG_INIT("Table dump:\n");
	    SBF_LOG_INIT("entry  SBF    MAC\n");
	    for (entry = 0; entry < sbftbl->entries; entry++) {
	        SBF_LOG_INIT("[%03d]  0x%x " MACDBG "\n",
	            entry, *(sbftbl->bfwtbl + entry),
	            MAC2STRDBG(sbftbl->addrtbl + entry));
	    }
	}

	return BCME_OK;
} // dhd_sbftbl_init

/**
 * dhd_sbftbl_fini - de-initialize sbftbl information
 *
 * returns
 *
 *  BCME_UNSUPPORTED: feature not supported
 *  BCME_OK:          On success
 */
static int
dhd_sbftbl_fini(dhd_pub_t *dhdp, dhd_sbf_info_t *sbf)
{
	dhd_sbftbl_t    *sbftbl;

	if (sbf == NULL)
	    return BCME_UNSUPPORTED;

	sbftbl = &sbf->table;
	sbftbl->hdr = NULL;

	return BCME_OK;
} // dhd_sbftbl_fini

/**
 * Threshold map Section
 */
/**
 * dhd_sbf_dump_threshold_map - dump current threshold map
 *
 * returns None
 */
static void
dhd_sbf_dump_threshold_map(dhd_sbf_info_t *sbf, struct bcmstrbuf *b)
{
	int level;

	SBF_DUMP(b, " Threshold map [ ");
	for (level = 0; level < DHD_SBF_MAP_LEVELS(sbf); level++) {
	    SBF_DUMP(b, " %d:%d", sbf->map.weights[level],
	        sbf->map.thresholds[level]);
	}
	SBF_DUMP(b, " ]\n");

	return;
}

/**
 * dhd_sbf_get_threshold_map - Get station threshold given BFW
 *
 * returns
 *  station threshold level
 */
static INLINE uint16
dhd_sbf_get_threshold_map(dhd_sbf_info_t *sbf, uint16 weight)
{
	int    level;
	uint16 threshold;

	/* Covert SBF weight into Queue threshold */
	if (sbf->hfc_mode == DHD_SBF_HFC_MAP) {
	    /* Using map tables */
	    threshold = sbf->map.thresholds[0];
	    for (level = (DHD_SBF_MAP_LEVELS(sbf) - 1); level > 0; level--) {
	        if (weight >= sbf->map.weights[level]) {
	            threshold = sbf->map.thresholds[level];
	            break;
	        }
	    }
	} else {
	    /* HFC using available sys buffers */
	    uint32 sysbuf_thresh = sbf->sysbuf.threshold;

	    /*
	     * assuming hfc_cumm_weight is already calculated before using
	     * this function
	     */
	    if (sbf->sysbuf.num_sta &&
	        (sysbuf_thresh/sbf->sysbuf.num_sta) < DHD_SBF_STA_THRESHOLD_MAX) {
	        threshold = (sysbuf_thresh * weight)/sbf->sysbuf.cumm_weight;
	        if (threshold > DHD_SBF_STA_THRESHOLD_MAX) {
	            threshold = DHD_SBF_STA_THRESHOLD_MAX;
	        }
	    } else {
	        threshold = DHD_SBF_STA_THRESHOLD_MAX;
	    }
	}
	SBF_LOG_POLL("BFW %d => threshold %d\n", weight, threshold);

	return threshold;
} // dhd_sbf_get_threshold_map

/**
 * dhd_sbf_init_threshold_map - Initialize BFW to threshold mapping
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_init_threshold_map(dhd_sbf_info_t *sbf, uint8 unit,
	uint16 sta_threshold)
{
	int level;
	char *var = NULL;
	char varstr[32];

	/* Initialize weight to Queue threshold map table with defaults */
	for (level = 0; level < DHD_SBF_MAP_LEVELS_DEF; level++) {
	    sbf->map.weights[level] = dhd_sbf_level_weights[level];
	    sbf->map.thresholds[level] =
	        (sta_threshold * level)/(DHD_SBF_MAP_LEVELS_DEF-1);
	}

	/* Use user specified threshold map if exists */
	snprintf(varstr, sizeof(varstr), "dhd%d_sbf_threshold_map", unit);
	var = getvar(NULL, varstr);

	if (!var) {
	    var = getvar(NULL, "dhd_sbf_threshold_map");
	}

	if (var) {
	    int threshold, weight;
	    char *token, *mapstr;

	    /*
	     * Parse the threshold map string
	     * format:  w1:th1 w2:th2 w3:th3 w4:th4 ..... w[n]:th[n]
	     */
	    level = 0;
	    mapstr = var;

	    token = bcmstrtok(&mapstr, " ", NULL);
	    while (token && level < DHD_SBF_MAP_LEVELS_MAX) {
	        sscanf(token, "%d:%d", &weight, &threshold);

	        if (threshold > DHD_SBF_STA_THRESHOLD_MAX)
	            threshold = DHD_SBF_STA_THRESHOLD_MAX;

	        if (weight > DHD_SBF_STA_WEIGHT_MAX)
	            weight = DHD_SBF_STA_WEIGHT_MAX;

	        sbf->map.weights[level] = weight;
	        sbf->map.thresholds[level] = threshold;
	        level++;
	        token = bcmstrtok(&mapstr, " ", NULL);
	    }
	}

	DHD_SBF_MAP_LEVELS(sbf) = level;

	dhd_sbf_dump_threshold_map(sbf, NULL);

	return BCME_OK;
} // dhd_sbf_init_threshold_map

/**
 * Station Section
 */
/**
 * dhd_sbf_set_sta_threshold - Walk through and set the station threshold for
 *  all active flow-rings queues
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_set_sta_threshold(dhd_pub_t* dhdp, dhd_sta_t *sta, uint16 weight)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	flow_queue_t *  queue;
	uint16          q_threshold;
	int             prio;

	/* Covert SBF Weight into Queue threshold */
	sta->sbf_threshold = dhd_sbf_get_threshold_map(sbf, weight);

	/* Walk through all Active Flow rings of the statuion */
	for (prio = 0; prio < NUMPRIO; prio++) {
	    uint16 flowid = sta->flowid[prio];

	    /* nothing to process  - inactive flow-ids */
	    if (flowid == FLOWID_INVALID)
	            continue;

	    queue = dhd_flow_queue(dhdp, flowid);
	    q_threshold = DHD_FLOW_QUEUE_THRESHOLD(queue);
	    if (q_threshold == sta->sbf_threshold) {
	        break;
	    }
	    SBF_LOG_CHANGE("dhd%d: FR [%d] UPD BQSIZE [%d] => [%d]\n",
	        dhdp->unit, flowid, q_threshold, sta->sbf_threshold);
	    /* flowring cumthreshold will be set to 1 less than the input */
	    DHD_FLOW_QUEUE_SET_THRESHOLD(queue, sta->sbf_threshold+1);

#if defined(BCM_DHD_RUNNER) && defined(FLRING_BQSIZE_NOTIF_GROUP)
	    {
	        flow_ring_node_t *flow_ring_node = DHD_FLOW_RING(dhdp, flowid);

	        /* Update Runner */
	        if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
	            dhd_runner_notify(dhdp->runner_hlp, H2R_FLRING_BQSIZE_NOTIF,
	                flowid, sta->sbf_threshold);
	        }
	    }
#endif /* BCM_DHD_RUNNER && FLRING_BQSIZE_NOTIF_GROUP */
	}

	return BCME_OK;
} // dhd_sbf_set_sta_threshold

/**
 * dhd_sbf_reset_sta - Reset station's SBF parameters
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_reset_sta(dhd_pub_t *dhdp, dhd_sta_t *sta, void* arg)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	uint16 sbf_index = sta->sbf_index;

	if (sbf_index == ID16_INVALID) {
	    /* Station is not initialized with sbf yet, update index */
	    sbf_index = dhd_sbftbl_get_index(dhdp, sbf, sta->ea.octet);
	    sta->sbf_index = sbf_index;
	}

	if (sbf_index != ID16_INVALID) {
	    uint16 weight = SBFTBL_WEIGHT_MAX;

	    /* Update the weight */
	    sta->sbf_weight = weight;
	    SBF_LOG_CHANGE("dhd%d: STA ["MACDBG"] RST SBF %d => %d\n",
	        dhdp->unit, MAC2STRDBG(sta->ea.octet), sta->sbf_weight, weight);
	    dhd_sbf_set_sta_threshold(dhdp, sta, weight);
	}

	return BCME_OK;
} // dhd_sbf_reset_sta

/**
 * dhd_sbf_wd_sta_hfc_sysbuf_get - Check for update in the Staion's BFW and
 *  update queue threshold if needed. This function gets called at every
 *  sbf_interval by dhd_sbf_watchdog() function
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_wd_sta_hfc_sysbuf_get(dhd_pub_t *dhdp, dhd_sta_t *sta, void *arg)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	uint16 sbf_index = sta->sbf_index;

	if (sbf_index == ID16_INVALID) {
	    /* Station is not initialized with sbf yet, update index */
	    sbf_index = dhd_sbftbl_get_index(dhdp, sbf, sta->ea.octet);
	    sta->sbf_index = sbf_index;
	}

	if (sbf_index != ID16_INVALID) {
	    uint16 weight;
		int level;

	    weight = dhd_sbftbl_get_weight(dhdp, sbf, sbf_index);
	    SBF_LOG_POLL("dhd%d: STA ["MACDBG"] POLL SBF %d:%d\n",
	        dhdp->unit, MAC2STRDBG(sta->ea.octet), sta->sbf_weight, weight);

	    /* Update the weight */
	    if (weight > DHD_SBF_STA_WEIGHT_MAX) {
	        /* Limit weight to dhd max weight level */
	        weight = DHD_SBF_STA_WEIGHT_MAX;
	    }

	    /*
	     * convert to the nearest power of 2 to reduce frequent changes of
	     * threshold due to small fluctuations in rate
	     */
		for (level = 0; level < NUM_BITS_U16; level++) {
			if ((1 << level) & DHD_SBF_WEIGHT_LEVEL_MASK) {
				sta->sbf_weight = 1 << level;
				if (sta->sbf_weight >= weight) {
					break;
				}
			}
		}
	}

	sbf->sysbuf.cumm_weight += sta->sbf_weight;
	sbf->sysbuf.num_sta++;

	return BCME_OK;
} // dhd_sbf_wd_sta_hfc_sysbuf_get

/**
 * dhd_sbf_wd_sta_hfc_sysbuf_apply - Check for update in the Staion's BFW and
 *  update queue threshold if needed. This function gets called at every
 *  sbf_interval by dhd_sbf_watchdog() function
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_wd_sta_hfc_sysbuf_apply(dhd_pub_t *dhdp, dhd_sta_t *sta, void *arg)
{
	uint16 sbf_index = sta->sbf_index;

	if (sbf_index != ID16_INVALID) {
	    /* Update the weight */
	    SBF_LOG_CHANGE("dhd%d: STA ["MACDBG"] CHG SBF => %d\n",
	        dhdp->unit, MAC2STRDBG(sta->ea.octet), sta->sbf_weight);
	    dhd_sbf_set_sta_threshold(dhdp, sta, sta->sbf_weight);
	}

	return BCME_OK;
} // dhd_sbf_wd_sta_hfc_sysbuf_apply

/**
 * dhd_sbf_wd_sta_hfc_map - Check for update in the Staion's BFW and update
 *  queue threshold if needed. This function gets called at every sbf_interval
 *  by dhd_sbf_watchdog() function
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_wd_sta_hfc_map(dhd_pub_t *dhdp, dhd_sta_t *sta, void *arg)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	uint16 sbf_index = sta->sbf_index;

	if (sbf_index == ID16_INVALID) {
	    /* Station is not initialized with sbf yet, update index */
	    sbf_index = dhd_sbftbl_get_index(dhdp, sbf, sta->ea.octet);
	    sta->sbf_index = sbf_index;
	}

	if (sbf_index != ID16_INVALID) {
	    uint16 weight;

	    weight = dhd_sbftbl_get_weight(dhdp, sbf, sbf_index);
	    SBF_LOG_POLL("dhd%d: STA ["MACDBG"] POLL SBF %d:%d\n",
	        dhdp->unit, MAC2STRDBG(sta->ea.octet), sta->sbf_weight, weight);

	    /* Update the weight */
	    if (sta->sbf_weight != weight) {
	        SBF_LOG_CHANGE("dhd%d: STA ["MACDBG"] CHG SBF %d => %d\n",
	            dhdp->unit, MAC2STRDBG(sta->ea.octet), sta->sbf_weight, weight);
	        dhd_sbf_set_sta_threshold(dhdp, sta, weight);
	        sta->sbf_weight = weight;
	    }
	}

	return BCME_OK;
} // dhd_sbf_wd_sta_hfc_map

/**
 * dhd_sbf_dump_sta - dump stations buffer fairness information
 *  This function gets called during "sbf_dump" iovar by dhd_sbf_dump() function
 *
 * returns
 *  BCME_OK on Success
 */
static int
dhd_sbf_dump_sta(dhd_pub_t *dhdp, dhd_sta_t *sta, void *arg)
{
	struct bcmstrbuf *b = (struct bcmstrbuf *)arg;

	SBF_DUMP(b, "   %3d    "MACDBG"  %4d    %4d\n", sta->sbf_index,
	    MAC2STRDBG(sta->ea.octet), sta->sbf_weight, sta->sbf_threshold);

	return BCME_OK;
} // dhd_sbf_dump_sta

/**
 * dhd_sbf_init_sta - Initialize flowring queues station threshold value
 *  This function gets called during station creation by dhd_add_sta() function
 *
 * returns
 *  BCME_OK on Success
 */
int
dhd_sbf_init_sta(dhd_pub_t *dhdp, dhd_sta_t *sta)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);

	if ((sbf == NULL) || (DHD_SBF_ENABLED(sbf) == FALSE)) {
	    return BCME_UNSUPPORTED;
	}

	/* Initialize SBF parameters (index, weight and threshold) */
	sta->sbf_weight = SBFTBL_WEIGHT_MAX;
	sta->sbf_index = dhd_sbftbl_get_index(dhdp, sbf, sta->ea.octet);
	if (sta->sbf_index != ID16_INVALID) {
	    sta->sbf_threshold = dhd_sbf_get_threshold_map(sbf, sta->sbf_weight);
	}

	SBF_LOG_INIT("dhd%d: INIT ["MACDBG"] ID %d BFW %d THRSH %d\n",
	    dhdp->unit, MAC2STRDBG(sta->ea.octet),
	    sta->sbf_index, sta->sbf_weight, sta->sbf_threshold);

	return BCME_OK;
} // dhd_sbf_init_sta

/**
 * dhd_sbf_dump - SBF module dump function that dumps either to console or to
 *  user buffer specified by input parameter
 *
 * returns
 *  BCME_OK on Success
 */
int
dhd_sbf_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	int             ifidx;

	if ((sbf == NULL) || (DHD_SBF_ENABLED(sbf) == FALSE)) {
	    SBF_DUMP(b, "Disabled\n");
	    return BCME_UNSUPPORTED;
	}

	SBF_DUMP(b, SBFTBL_VRP_FMT" EN [%d] entries [%d] type [%d]\n",
	    SBFTBL_VRP_VAL(SBFTBL_VERSIONCODE), sbf->enable,
	    sbf->table.entries, sbf->table.type);

	SBF_DUMP(b, " interval [%d] wd_expiry [%d] grace_threshold [%d]\n",
	    sbf->wd_cnt_expiry * sbf->wd_interval, sbf->wd_cnt_expiry,
	    sbf->grace_threshold);
	SBF_DUMP(b, " wd_cnt [%d] grace_pkts queued/checked [%lu/%lu]\n",
	    sbf->wd_cnt, sbf->grace_pktsqueued, sbf->grace_pktschecked);
	if (sbf->hfc_mode == DHD_SBF_HFC_SYSBUF) {
	    SBF_DUMP(b, " sys_buff inuse/max [%d/%d]\n",
	        DHD_CUMM_SYSBUF_CTR(dhdp), sbf->sysbuf.threshold);
	} else {
	    dhd_sbf_dump_threshold_map(sbf, b);
	}

	if (DHD_SBF_ENABLED(sbf)) {
	    SBF_DUMP(b, "\n");
	    SBF_DUMP(b, " CLIENT BFW TABLE:\n");
	    SBF_DUMP(b, "  [Idx]       [MAC]          [BFW] [SBFTH]\n");

	    /* Walk all interfaces of this radio */
	    for (ifidx = 0; ifidx < DHD_MAX_IFS; ifidx++) {
	        dhd_sta_list_invoke_func(dhdp, ifidx, dhd_sbf_dump_sta, b);
	    }
	}

	return BCME_OK;
} // dhd_sbf_dump

/**
 * dhd_sbf_is_favoredpkt - Check if the packet is a favored packet list
 *  (ICMP, ARP, DHCP, EAPOL, TCP ACK ...) to be part of grace queue.
 *  This implemenation covers TCP ACK only
 *
 * returns
 *  TRUE:  on favored packet
 *  FALSE: otherwise
 */
static bool BCMFASTPATH
dhd_sbf_is_favoredpkt(dhd_pub_t *dhdp, void *pkt)
{
	bool  tcpack = FALSE;

	uint8 *ether_hdr;	/* Ethernet header of the packet */
	uint16 ether_type;	/* Ethernet type of the packet */
	uint8 *ip_hdr;		/* IP header of the packet */
	uint32 ip_hdr_len;  /* IP header length of the packet */
	uint8 *tcp_hdr;	    /* TCP header of the packet */
	uint32 framelen;

	/* Check if packet is TCP ACK packet */
	ether_hdr = PKTDATA(dhdp->osh, pkt);
	framelen = PKTLEN(dhdp->osh, pkt);

	if (framelen < DHD_SBF_TCPACKSZMIN || framelen > DHD_SBF_TCPACKSZMAX) {
	    DHD_TRACE(("%s %d: Too short or long length %d to be TCP ACK\n",
	        __FUNCTION__, __LINE__, framelen));
	    goto done_parse;
	}

	ether_type = ether_hdr[12] << 8 | ether_hdr[13];

	if (ether_type != ETHER_TYPE_IP) {
	    DHD_TRACE(("%s %d: Not a IP packet 0x%x\n",
	        __FUNCTION__, __LINE__, ether_type));
	    goto done_parse;
	}

	ip_hdr = ether_hdr + ETHER_HDR_LEN;
	framelen -= ETHER_HDR_LEN;

	SBF_ASSERT(framelen >= IPV4_MIN_HEADER_LEN);

	ip_hdr_len = IPV4_HLEN(ip_hdr);
	if (IP_VER(ip_hdr) != IP_VER_4 || IPV4_PROT(ip_hdr) != IP_PROT_TCP) {
	    DHD_TRACE(("%s %d: Not IPv4 nor TCP! ip ver %d, prot %d\n",
	        __FUNCTION__, __LINE__, IP_VER(ip_hdr), IPV4_PROT(ip_hdr)));
	    goto done_parse;
	}

	tcp_hdr = ip_hdr + ip_hdr_len;
	framelen -= ip_hdr_len;

	/* is it an ack ? Allow only ACK flag, not to suppress others. */
	if (tcp_hdr[TCP_FLAGS_OFFSET] != TCP_FLAG_ACK) {
	    DHD_TRACE(("%s %d: Do not touch TCP flag 0x%x\n",
	        __FUNCTION__, __LINE__, tcp_hdr[TCP_FLAGS_OFFSET]));
	    goto done_parse;
	}

	/* TCP ACK packets can be admitted to the queue */
	tcpack = TRUE;

done_parse:
	return tcpack;
}

/**
 * dhd_sbf_queue_throttle_pkt - Check if the incoming packet needs to be
 * throttled based on Queue thresholds including grace threshold
 *
 * returns
 *  BCME_OK:           pkt can be admitted to the queued
 *  BCME_NOT_ADMITTED: pkt can not be admitted to the queue
 */
int BCMFASTPATH
dhd_sbf_queue_throttle_pkt(dhd_pub_t *dhdp, flow_queue_t *queue, void *pkt)
{
	int             ret = BCME_OK;
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);

	flow_ring_node_t *flow_ring_node;
	void *gp_clen_ptr;
	void *parent_clen_ptr;
	int gp_cumm_threshold;
	int cumm_threshold;
	int pkt_count = 1;
	uint16 avail_space;
	int sysbuf_inuse;

	SBF_ASSERT(queue != NULL);

	/* Check packet admittance to the queue */
	/* Two tests
	 * Overall level 2 (grandparent) cummulative threshold crossed.
	 *                      - OR -
	 * physical queue's full and overall cummulative threshold crossed.
	 */
	flow_ring_node = container_of(queue, flow_ring_node_t, queue);
	gp_clen_ptr = DHD_FLOW_QUEUE_L2CLEN_PTR(queue);
	parent_clen_ptr = DHD_FLOW_QUEUE_CLEN_PTR(queue);
	gp_cumm_threshold = DHD_FLOW_QUEUE_L2THRESHOLD(queue);
	cumm_threshold = DHD_FLOW_QUEUE_THRESHOLD(queue);
	avail_space = dhd_prot_flow_ring_avail_space(dhdp, flow_ring_node);
	sysbuf_inuse = DHD_CUMM_SYSBUF_CTR(dhdp);

	ret = (((sysbuf_inuse + pkt_count) > sbf->sysbuf.threshold) |
	    ((DHD_CUMM_CTR_READ(gp_clen_ptr) + pkt_count) > gp_cumm_threshold) |
	    ((pkt_count > avail_space) &
	    ((DHD_CUMM_CTR_READ(parent_clen_ptr) + pkt_count) > cumm_threshold))) ?
	    BCME_NOT_ADMITTED : BCME_OK;

	if ((ret == BCME_OK) || (sbf->grace_threshold == 0)) {
	    /* Packet can be queued or no additional grace queue threshold */
	    goto done;
	}

	/* Check if packet is allowed to go through additional grace threshold */
	cumm_threshold += sbf->grace_threshold;
	ret = ((pkt_count > avail_space) &&
	    ((DHD_CUMM_CTR_READ(parent_clen_ptr) + pkt_count) > cumm_threshold)) ?
	    BCME_NOT_ADMITTED : BCME_OK;

	if (ret != BCME_OK) {
	    /* used up additional grace threshold as well */
	    goto done;
	}

	/* Check if packet is TCP ACK packet */
	if (dhd_sbf_is_favoredpkt(dhdp, pkt) == TRUE) {
	    /* TCP ACK packets can be admitted to the queue */
	    ret = BCME_OK;
	    sbf->grace_pktsqueued++;
	} else {
	    ret = BCME_NOT_ADMITTED;
	}
	sbf->grace_pktschecked++;

done:
	return ret;
}

#if defined(BCM_PKTFWD)
static INLINE void
dhd_sbf_pktlist_add(pktlist_t *pktlist, void *pkt)
{
	if (likely(pktlist->len != 0)) {
	    /* pend to tail */
	    PKTLIST_PKT_SET_SLL(pktlist->tail, pkt, DHD_NBUFF_PTR_TYPE);
	    pktlist->tail = pkt;
	} else {
	    pktlist->head = pktlist->tail = pkt;
	}
	++pktlist->len;
}

static INLINE void
dhd_sbf_pktlist_del(pktlist_t *pktlist, void *pkt, void *pkt_prev)
{
	if (pktlist->len == 1) {
	    /* Head = Tail = Pkt, Only one packet, Reset the pktlist */
	    PKTLIST_RESET(pktlist);
	} else {
	    if (pktlist->tail == pkt) {
	        /* Tail = Pkt, Last packet, move the Tail */
	        pktlist->tail = pkt_prev;
	    } else if (pktlist->head == pkt) {
	        /* Head = Pkt, First packet, move the Head */
	        /* in the middle of the list */
	        pktlist->head = PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);
	    } else {
	        PKTLIST_PKT_SET_SLL(pkt_prev,
	                        PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE),
	                        DHD_NBUFF_PTR_TYPE);
	    }
	    --pktlist->len;
	}

	return;
}

/**
 * dhd_sbf_queue_throttle_pktlist - Check if the incoming packets needs to be
 * throttled based on Queue thresholds including grace threshold. Move packets
 * that can be queued to pktlist_enq. Original "pktlist" holds pkts that needs
 * to be throttled.
 *
 * pktlist [in/out]:   [in]  incoming packet list
 *                     [out] to be dropped packet list
 * pktlist_enq [out]:  [out] to be enqueued packet list
 *
 * returns
 *  BCME_OK:           All packets can be queued
 *  BCME_NOT_ADMITTED: All packets can not be queued
 *  BCME_NORESOURCE:   Only some packets (TCP ACK)  can be queued
 */
int BCMFASTPATH
dhd_sbf_queue_throttle_pktlist(dhd_pub_t *dhdp, flow_queue_t *queue,
	pktlist_t *pktlist, pktlist_t *pktlist_enq)
{
	int             ret = BCME_OK;
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);

	flow_ring_node_t *flow_ring_node;
	int gp_clen;
	int p_clen;
	int gp_cumm_threshold;
	int cumm_threshold;
	int pkt_count = pktlist->len;
	int grace_pkts = 0;
	uint16 avail;
	int sysbuf_inuse;

	SBF_ASSERT(queue != NULL);

	/* Check packets admittance to the queue */
	/* Two tests
	 * Overall level 2 (grandparent) cummulative threshold crossed.
	 *                      - OR -
	 * physical queue's full and overall cummulative threshold crossed.
	 */
	flow_ring_node = container_of(queue, flow_ring_node_t, queue);
	gp_clen = DHD_CUMM_CTR_READ(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));
	p_clen = DHD_CUMM_CTR_READ(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	gp_cumm_threshold = DHD_FLOW_QUEUE_L2THRESHOLD(queue);
	cumm_threshold = DHD_FLOW_QUEUE_THRESHOLD(queue);
	avail = dhd_prot_flow_ring_avail_space(dhdp, flow_ring_node);
	sysbuf_inuse = DHD_CUMM_SYSBUF_CTR(dhdp);

	ret = (((sysbuf_inuse + pkt_count) > sbf->sysbuf.threshold) |
	    ((gp_clen + pkt_count) > gp_cumm_threshold) |
	    ((pkt_count > avail) & ((p_clen + pkt_count) > cumm_threshold))) ?
	    BCME_NOT_ADMITTED : BCME_OK;

	/* Get the available grace packets that can be queued */
	if (ret == BCME_NOT_ADMITTED) {
	    cumm_threshold += sbf->grace_threshold;
	    grace_pkts = cumm_threshold - p_clen;
	}

	PKTLIST_RESET(pktlist_enq);
	if (ret == BCME_OK) {
	    /* Queue All */
	    pktlist_enq->head = pktlist->head;
	    pktlist_enq->tail = pktlist->tail;
	    pktlist_enq->len = pktlist->len;
	    pktlist_enq->key.v16 = pktlist->key.v16;

	    /* No Drops */
	    pktlist->len = 0;
	} else if (grace_pkts > 0) {
	    void *pkt = pktlist->head;
	    void *pkt_prev = pktlist->head;
	    void *pkt_next;

	    ret = pkt_count;
	    pkt = pktlist->head;
	    pkt_prev = pktlist->head;

	    ASSERT(NBUFF_PTR_TYPE(pkt) == DHD_NBUFF_PTR_TYPE);

	    /* Go through the packet list until grace packets are available */
	    while (grace_pkts && pkt_count--) {

	        pkt_next = PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);

	        /* TCP ACK packets can be admitted to the queue */
	        if (dhd_sbf_is_favoredpkt(dhdp, pkt) == TRUE) {
	            /* delete packet from pktlist */
	            dhd_sbf_pktlist_del(pktlist, pkt, pkt_prev);

	            /* Add packet to enqueue list */
	            dhd_sbf_pktlist_add(pktlist_enq, pkt);
	            sbf->grace_pktsqueued++;
	            grace_pkts--;
	            --ret;
	        } else {
	            pkt_prev = pkt;
	        }
	        sbf->grace_pktschecked++;

	        pkt = pkt_next;
	    }
	    ret = BCME_NORESOURCE;
	/* } else { Throttle All */
	}

	return ret;
}
#endif /* BCM_PKTFWD */

/**
 * dhd_sbf_active - Check if SBF feature is supported and enabled
 *
 * returns
 *  TRUE:   SBF is active
 *  FALSE:  SBF ins inactive
 */
bool
dhd_sbf_active(dhd_pub_t *dhdp)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);

	/* Check if feature is enabled and active */
	if ((sbf == NULL) || (DHD_SBF_ENABLED(sbf) == FALSE)) {
	    return FALSE;
	}

	return TRUE;
}

/**
 * dhd_sbf_watchdog - SBF module watchdog function called by main DHD watchdog
 *  function. This function will check BFW updates and process the corresponding
 *  Queue threshold updates
 *
 * returns
 *  BCME_OK:   on success
 */
int
dhd_sbf_watchdog(dhd_pub_t *dhdp)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	dhd_sbftbl_t    *sbftbl = &sbf->table;
	int             ifidx;

	/* Check if feature is enabled and device is active */
	if ((SBF_ACTIVE(dhdp) == FALSE) || dhd_is_device_removed(dhdp)) {
	    /* Nothing to be done */
	    return BCME_OK;
	}

	/* TODO: Need exact time?? */
	/* Check for update timer expiry */
	if (sbf->wd_cnt++ < sbf->wd_cnt_expiry) {
	    /* Update time not expired yet */
	    return BCME_OK;
	}

	DHD_LOCK(dhdp);

	sbf->wd_cnt = 0;
	/* Get the updated weight from the station */
	OSL_CACHE_INV(sbftbl->bfwtbl, sbftbl->entries * SBFTBL_WEIGHT_LEN);

	/* Walk all interfaces of this radio */
	if (sbf->hfc_mode == DHD_SBF_HFC_MAP) {
	    /* HFC using static mapping */
	    for (ifidx = 0; ifidx < DHD_MAX_IFS; ifidx++) {
	        dhd_sta_list_invoke_func(dhdp, ifidx,
	            dhd_sbf_wd_sta_hfc_map, NULL);
	    }
	} else {
	    /* HFC using available sys buffers */
	    sbf->sysbuf.cumm_weight = 0;
	    sbf->sysbuf.num_sta = 0;

	    /* Walk all interfaces of this radio and get cumulative weight */
	    for (ifidx = 0; ifidx < DHD_MAX_IFS; ifidx++) {
	        dhd_sta_list_invoke_func(dhdp, ifidx,
	            dhd_sbf_wd_sta_hfc_sysbuf_get, NULL);
	    }

	    /* Walk all interfaces of this radio and apply the normalized weight */
	    for (ifidx = 0; ifidx < DHD_MAX_IFS; ifidx++) {
	        dhd_sta_list_invoke_func(dhdp, ifidx,
	            dhd_sbf_wd_sta_hfc_sysbuf_apply, NULL);
	    }

	    sbf->sysbuf.cumm_weight = 0;
	    sbf->sysbuf.num_sta = 0;
	}

	DHD_UNLOCK(dhdp);

	return BCME_OK;
} //dhd_sbf_watchdog

/**
 * dhd_sbf_set_iovar - SBF module IOVAR processing function
 *  Called by dhd_common dhd_doiovar() function for any set opration
 *
 * returns
 *  BCME_OK:   on success
 */
int
dhd_sbf_set_iovar(dhd_pub_t *dhdp, int cmd, int val)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	int             ret = BCME_OK;

	if (DHD_SBF_INFO(dhdp) == NULL) {
	    return BCME_UNSUPPORTED;
	}

	/* Set IOVAR */
	switch (cmd) {

	    case DHD_SBF_CMD_INTERVAL:
	        {
	            uint16 wd_cnt_expiry = sbf->wd_cnt_expiry;

	            sbf->wd_cnt_expiry = val/sbf->wd_interval;
	            if (sbf->wd_cnt_expiry && (wd_cnt_expiry == 0)) {
	                /* disable -> enable */
	                sbf->enable = TRUE;
	            } else if ((sbf->wd_cnt_expiry == 0) && wd_cnt_expiry) {
	                int ifidx;

	                /* enable -> disable */
	                sbf->enable = FALSE;

	                /* Walk all interfaces of this radio and reset bfw */
	                for (ifidx = 0; ifidx < DHD_MAX_IFS; ifidx++) {
	                    dhd_sta_list_invoke_func(dhdp, ifidx,
	                        dhd_sbf_reset_sta, NULL);
	                }
	            }
	            /* else change in timeout value */
	            break;
	        }

	    case DHD_SBF_CMD_GRACETH:
	        sbf->grace_threshold = val;
	        break;

#ifdef SBF_DEBUG_BUILD
	    case DHD_SBF_CMD_MSGLEVEL:
	        dhd_sbf_msglevel = val;
	        break;
#endif /* SBF_DEBUG_BUILD */

	    default:
	        ret = BCME_UNSUPPORTED;
	        break;
	}

	return ret;
} // dhd_sbf_set_iovar

/**
 * dhd_sbf_get_iovar - SBF module IOVAR processing function
 *  Called by dhd_common dhd_doiovar() function for any get opration
 *
 * returns
 *  BCME_OK:   on success
 */
int
dhd_sbf_get_iovar(dhd_pub_t *dhdp, int cmd)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);
	int             ret;

	if (DHD_SBF_INFO(dhdp) == NULL) {
	    return (int)0;
	}

	/* Set IOVAR */
	switch (cmd) {

	    case DHD_SBF_CMD_INTERVAL:
	        ret = (int)(sbf->wd_cnt_expiry * sbf->wd_interval);
	        break;

	    case DHD_SBF_CMD_GRACETH:
	        ret = (int)(sbf->grace_threshold);
	        break;

#ifdef SBF_DEBUG_BUILD
	    case DHD_SBF_CMD_MSGLEVEL:
	        ret = dhd_sbf_msglevel;
	        break;
#endif /* SBF_DEBUG_BUILD */

	    default:
	        ret = BCME_UNSUPPORTED;
	        break;
	}

	return ret;
} // dhd_sbf_get_iovar

/**
 * dhd_sbf_init - SBF module init function
 *  Should be called after protocol initialization is complete (for HME)
 *
 * returns
 *  BCME_OK:          On successful initialization
 *  BCME_UNSUPPORTED: if SBF is not supported
 *  BCME_ERROR:       Otherwise
 */
int
dhd_sbf_init(dhd_pub_t *dhdp, int wd_interval, int sta_threshold)
{
	dhd_sbf_info_t *sbf;
	uint32         poll_interval = DHD_SBF_UPDATE_INTERVAL_DEF;

	if (DHD_SBF_INFO(dhdp)) {
	    DHD_INFO(("dhd%d: %s already initialized\n", dhdp->unit, __FUNCTION__));
	    return BCME_OK;
	}

	/* Check if SBFTBL is disabled by nvram */
	{
	    char   *var = NULL;

	    if ((var = si_getdevpathvar(dhdp->bus->sih, "sbftbl")) == NULL) {
	        var = getvar(NULL, "sbftbl");
	    }

	    if (var) {
	        poll_interval = bcm_strtoul(var, NULL, 0);
	    }

	    if (poll_interval == 0) {
	        DHD_ERROR(("dhd%d: SBFTBL disabled\n", dhdp->unit));
	        return BCME_UNSUPPORTED;
	    } else if (poll_interval == 1) {
	        poll_interval = DHD_SBF_UPDATE_INTERVAL;
	    } else {
	        poll_interval *= DBS_SBF_UPDATE_FACTOR;
	    }
	}

	/* Allocate SBFTBL control block */
	sbf = (dhd_sbf_info_t*)MALLOCZ(dhdp->osh, sizeof(dhd_sbf_info_t));
	if (sbf == NULL) {
	    DHD_ERROR(("%s: Failed to allocate SBFTBL CB memory\n", __FUNCTION__));
	    return BCME_ERROR;
	}
	dhdp->sbf = (void*)sbf;
	SBF_LOG_INIT("CB: [0x%px] Allocated\n", sbf);

	/* Initialize protocol part (HME) */
	if (dhd_sbftbl_init(dhdp, sbf) != BCME_OK) {
	    DHD_ERROR(("%s: Failed to Initialize SBFTBL Protocol\n", __FUNCTION__));
	    dhd_sbf_fini(dhdp);
	    return BCME_UNSUPPORTED;
	}

	/* Initialize weight to Queue threshold map table */
	dhd_sbf_init_threshold_map(sbf, dhdp->unit, sta_threshold);

	/* Initialize sbf update timer */
	sbf->wd_cnt = 0;
	sbf->wd_cnt_expiry = poll_interval/wd_interval;
	sbf->wd_interval = wd_interval;
	if (sbf->wd_cnt_expiry > 0) {
	    sbf->enable = TRUE;
	} else {
	    sbf->enable = FALSE;
	}

	/* Check if Max buffers nvram variable is set */
	{
	    uint32 max_buffers = 0;
	    char   *var = NULL;
	    char   varstr[32];

	    snprintf(varstr, sizeof(varstr), "dhd%d_sbf_sysbuf_threshold", dhdp->unit);
	    var = getvar(NULL, varstr);

	    if (var) {
	        max_buffers = bcm_strtoul(var, NULL, 0);
	    }

	    if (max_buffers == 0) {
	        /* TODO: Get max buffers per radio automatically from BPM pool */
	        SBF_LOG_INIT("dhd%d: Using Static Threshold map HFC\n",
	            dhdp->unit);
	        max_buffers = DHD_SBF_SYS_BUFFERS_INV;
	        sbf->hfc_mode = DHD_SBF_HFC_MAP;
	    } else {
	        SBF_LOG_INIT("dhd%d: Using System buffer threshold HFC\n",
	            dhdp->unit);
	        sbf->hfc_mode = DHD_SBF_HFC_SYSBUF;
	    }
	    sbf->sysbuf.threshold = max_buffers;
	}

	sbf->grace_threshold = DHD_SBF_GRACE_THRESHOLD;
	sbf->grace_pktsqueued = 0;
	sbf->grace_pktschecked = 0;

	SBF_LOG_INIT("wd_cnt_expiry [%d]\n", sbf->wd_cnt_expiry);
	SBF_LOG_INIT("grace_threshold [%d]\n", sbf->grace_threshold);

	SBF_LOG("dhd%d " SBFTBL_VRP_FMT " Initialized\n",
	    dhdp->unit, SBFTBL_VRP_VAL(SBFTBL_VERSIONCODE));

	return BCME_OK;
}  // dhd_sbf_init()

/**
 * dhd_sbf_fini - SBF module de-init function
 *
 * returns
 *  BCME_OK:          On successful initialization
 *  BCME_UNSUPPORTED: if SBF is not supported
 */
int
dhd_sbf_fini(dhd_pub_t *dhdp)
{
	dhd_sbf_info_t  *sbf = DHD_SBF_INFO(dhdp);

	if (sbf == NULL)
	    return BCME_UNSUPPORTED;

	dhd_sbftbl_fini(dhdp, sbf);
	MFREE(dhdp->osh, sbf, sizeof(dhd_sbf_info_t));
	dhdp->sbf = NULL;

	SBF_LOG_INIT("dhd%d %s\n", dhdp->unit, __FUNCTION__);

	return BCME_OK;
} // dhd_sbf_fini()
#endif /* DHD_SBF */
