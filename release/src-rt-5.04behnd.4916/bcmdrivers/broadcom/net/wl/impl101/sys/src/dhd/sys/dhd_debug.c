/*
 * DHD debugability support
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: dhd_debug.c 828137 2023-08-02 12:58:06Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmstdlib_s.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>
#include <eldbg_ring.h>
#include <dhd_debug.h>
#ifdef MOBILITY_LOG_TRACE
#include <dhd_mschdbg.h>
#endif
#include <dhd_bus.h>
#include <event_log.h>
#include <event_trace.h>
#include <msgtrace.h>
#if defined(DBG_PKT_MON)
#include <dhd_linux_priv.h>
#include <dhd_linux_wq.h>
#endif /* DBG_PKT_MON */
#if defined(DHD_PKT_LOGGING) && defined(DHD_PKT_LOGGING_DBGRING)
#include <dhd_pktlog.h>
#endif /* DHD_PKT_LOGGING && DHD_PKT_LOGGING_DBGRING */

#ifdef WL_CFGVENDOR_CUST_ADVLOG
void wl_cfgvendor_custom_advlog_roam_log(void *plog, uint32 armcycle);
#endif /* WL_CFGVENDOR_CUST_ADVLOG */

#if defined(DHD_EFI) || defined(NDIS)
#if !defined(offsetof)
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif	/* !defined(offsetof) */

#define container_of(ptr, type, member) \
		(type *)((char *)(ptr) - offsetof(type, member))
#endif /* defined(DHD_EFI ) || defined(NDIS) */

uint8 control_logtrace = CUSTOM_CONTROL_LOGTRACE;

struct map_table {
	uint16 fw_id;
	uint16 host_id;
	char *desc;
};

struct map_table event_map[] = {
	{WLC_E_AUTH, WIFI_EVENT_AUTH_COMPLETE, "AUTH_COMPLETE"},
	{WLC_E_ASSOC, WIFI_EVENT_ASSOC_COMPLETE, "ASSOC_COMPLETE"},
	{TRACE_FW_AUTH_STARTED, WIFI_EVENT_FW_AUTH_STARTED, "AUTH STARTED"},
	{TRACE_FW_ASSOC_STARTED, WIFI_EVENT_FW_ASSOC_STARTED, "ASSOC STARTED"},
	{TRACE_FW_RE_ASSOC_STARTED, WIFI_EVENT_FW_RE_ASSOC_STARTED, "REASSOC STARTED"},
	{TRACE_G_SCAN_STARTED, WIFI_EVENT_G_SCAN_STARTED, "GSCAN STARTED"},
	{WLC_E_PFN_SCAN_COMPLETE, WIFI_EVENT_G_SCAN_COMPLETE, "GSCAN COMPLETE"},
	{WLC_E_DISASSOC, WIFI_EVENT_DISASSOCIATION_REQUESTED, "DIASSOC REQUESTED"},
	{WLC_E_REASSOC, WIFI_EVENT_RE_ASSOCIATION_REQUESTED, "REASSOC REQUESTED"},
	{TRACE_ROAM_SCAN_STARTED, WIFI_EVENT_ROAM_REQUESTED, "ROAM REQUESTED"},
	{WLC_E_BEACON_FRAME_RX, WIFI_EVENT_BEACON_RECEIVED, "BEACON Received"},
	{TRACE_ROAM_SCAN_STARTED, WIFI_EVENT_ROAM_SCAN_STARTED, "ROAM SCAN STARTED"},
	{TRACE_ROAM_SCAN_COMPLETE, WIFI_EVENT_ROAM_SCAN_COMPLETE, "ROAM SCAN COMPLETED"},
	{TRACE_ROAM_AUTH_STARTED, WIFI_EVENT_ROAM_AUTH_STARTED, "ROAM AUTH STARTED"},
	{WLC_E_AUTH, WIFI_EVENT_ROAM_AUTH_COMPLETE, "ROAM AUTH COMPLETED"},
	{TRACE_FW_RE_ASSOC_STARTED, WIFI_EVENT_ROAM_ASSOC_STARTED, "ROAM ASSOC STARTED"},
	{WLC_E_ASSOC, WIFI_EVENT_ROAM_ASSOC_COMPLETE, "ROAM ASSOC COMPLETED"},
	{TRACE_ROAM_SCAN_COMPLETE, WIFI_EVENT_ROAM_SCAN_COMPLETE, "ROAM SCAN COMPLETED"},
	{TRACE_BT_COEX_BT_SCO_START, WIFI_EVENT_BT_COEX_BT_SCO_START, "BT SCO START"},
	{TRACE_BT_COEX_BT_SCO_STOP, WIFI_EVENT_BT_COEX_BT_SCO_STOP, "BT SCO STOP"},
	{TRACE_BT_COEX_BT_SCAN_START, WIFI_EVENT_BT_COEX_BT_SCAN_START, "BT COEX SCAN START"},
	{TRACE_BT_COEX_BT_SCAN_STOP, WIFI_EVENT_BT_COEX_BT_SCAN_STOP, "BT COEX SCAN STOP"},
	{TRACE_BT_COEX_BT_HID_START, WIFI_EVENT_BT_COEX_BT_HID_START, "BT HID START"},
	{TRACE_BT_COEX_BT_HID_STOP, WIFI_EVENT_BT_COEX_BT_HID_STOP, "BT HID STOP"},
	{WLC_E_EAPOL_MSG, WIFI_EVENT_FW_EAPOL_FRAME_RECEIVED, "FW EAPOL PKT RECEIVED"},
	{TRACE_FW_EAPOL_FRAME_TRANSMIT_START, WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_START,
	"FW EAPOL PKT TRANSMITED"},
	{TRACE_FW_EAPOL_FRAME_TRANSMIT_STOP, WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_STOP,
	"FW EAPOL PKT TX STOPPED"},
	{TRACE_BLOCK_ACK_NEGOTIATION_COMPLETE, WIFI_EVENT_BLOCK_ACK_NEGOTIATION_COMPLETE,
	"BLOCK ACK NEGO COMPLETED"},
};

struct map_table event_tag_map[] = {
	{TRACE_TAG_VENDOR_SPECIFIC, WIFI_TAG_VENDOR_SPECIFIC, "VENDOR SPECIFIC DATA"},
	{TRACE_TAG_BSSID, WIFI_TAG_BSSID, "BSSID"},
	{TRACE_TAG_ADDR, WIFI_TAG_ADDR, "ADDR_0"},
	{TRACE_TAG_SSID, WIFI_TAG_SSID, "SSID"},
	{TRACE_TAG_STATUS, WIFI_TAG_STATUS, "STATUS"},
	{TRACE_TAG_CHANNEL_SPEC, WIFI_TAG_CHANNEL_SPEC, "CHANSPEC"},
	{TRACE_TAG_WAKE_LOCK_EVENT, WIFI_TAG_WAKE_LOCK_EVENT, "WAKELOCK EVENT"},
	{TRACE_TAG_ADDR1, WIFI_TAG_ADDR1, "ADDR_1"},
	{TRACE_TAG_ADDR2, WIFI_TAG_ADDR2, "ADDR_2"},
	{TRACE_TAG_ADDR3, WIFI_TAG_ADDR3, "ADDR_3"},
	{TRACE_TAG_ADDR4, WIFI_TAG_ADDR4, "ADDR_4"},
	{TRACE_TAG_TSF, WIFI_TAG_TSF, "TSF"},
	{TRACE_TAG_IE, WIFI_TAG_IE, "802.11 IE"},
	{TRACE_TAG_INTERFACE, WIFI_TAG_INTERFACE, "INTERFACE"},
	{TRACE_TAG_REASON_CODE, WIFI_TAG_REASON_CODE, "REASON CODE"},
	{TRACE_TAG_RATE_MBPS, WIFI_TAG_RATE_MBPS, "RATE"},
};

struct event_log_table {
	int log_level;
	uint16 tag;
	uint8 set;
	char *desc;
};

/* define log level per ring type */
struct event_log_table fw_verbose_level_map[] = {
	{1, EVENT_LOG_TAG_PCI_ERROR, EVENT_LOG_SET_BUS, "PCI_ERROR"},
	{1, EVENT_LOG_TAG_PCI_WARN, EVENT_LOG_SET_PRSRV_BUS, "PCI_WARN"},
	{1, EVENT_LOG_TAG_PCI_INFO, EVENT_LOG_SET_BUS, "PCI_INFO"},
	{3, EVENT_LOG_TAG_PCI_DBG, EVENT_LOG_SET_BUS, "PCI_DEBUG"},
	{2, EVENT_LOG_TAG_BEACON_LOG, EVENT_LOG_SET_PRSRV_CHATTY, "BEACON_LOG"},
	{1, EVENT_LOG_TAG_WL_ASSOC_LOG, EVENT_LOG_SET_PRSRV, "ASSOC_LOG"},
	{1, EVENT_LOG_TAG_WL_ROAM_LOG, EVENT_LOG_SET_PRSRV, "ROAM_LOG"},
	{1, EVENT_LOG_TAG_TRACE_WL_INFO, EVENT_LOG_SET_WL, "WL INFO"},
	{1, EVENT_LOG_TAG_TRACE_BTCOEX_INFO, EVENT_LOG_SET_WL, "BTCOEX INFO"},
#ifdef DHD_RANDMAC_LOGGING
	{1, EVENT_LOG_TAG_RANDMAC_ERR, EVENT_LOG_SET_PRSRV, "RANDMAC_ERR"},
#endif /* DHD_RANDMAC_LOGGING */
#ifdef CUSTOMER_HW4_DEBUG
	{3, EVENT_LOG_TAG_SCAN_WARN, EVENT_LOG_SET_PRSRV_CHATTY, "SCAN_WARN"},
#else
	{1, EVENT_LOG_TAG_SCAN_WARN, EVENT_LOG_SET_PRSRV_CHATTY, "SCAN_WARN"},
#endif /* CUSTOMER_HW4_DEBUG */
	{2, EVENT_LOG_TAG_SCAN_ERROR, EVENT_LOG_SET_ERROR, "SCAN_ERROR"},
	{1, EVENT_LOG_TAG_SCAN_TRACE_LOW, EVENT_LOG_SET_PRSRV, "SCAN_TRACE_LOW"},
	{1, EVENT_LOG_TAG_SCAN_TRACE_HIGH, EVENT_LOG_SET_WL, "SCAN_TRACE_HIGH"},
};

/* reference tab table */
uint ref_tag_tbl[EVENT_LOG_TAG_MAX + 1] = {0};

typedef struct dhddbg_loglist_item {
	dll_t list;
	prcd_event_log_hdr_t prcd_log_hdr;
} loglist_item_t;

typedef struct dhbdbg_pending_item {
	dll_t list;
	eldbg_ring_status_t ring_status;
	eldbg_ring_entry_t *ring_entry;
} pending_item_t;

#ifdef DHD_PKT_LOGGING_DBGRING
int
dhd_dbg_update_to_ring(dhd_pub_t *dhdp, void *ring, uint32 w_len)
{
	return dhd_dbg_push_to_ring(dhdp, ((eldbg_ring_cmn_t *)ring)->id, NULL, (void*)&w_len);
}

static uint32
dhd_dbg_urgent_pull(dhd_pub_t *dhdp, eldbg_ring_cmn_t *ring)
{
	uint32 pending_len = 0;
	struct dhd_pktlog_ring *pktlog_ring;

	if (ring->id != PACKET_LOG_RING_ID) {
		return pending_len;
	}

	if (ring->stat.written_bytes > ring->stat.read_bytes) {
		pending_len = ring->stat.written_bytes - ring->stat.read_bytes;
	} else {
		pending_len = 0;
	}

	pktlog_ring = dhdp->pktlog->pktlog_ring;
	if (pktlog_ring->pktcount >= DHD_PACKET_LOG_RING_SUSPEND_THRESHOLD) {
		dhd_pktlog_suspend(dhdp);
	}

	if (pending_len > ring->threshold) {
		DHD_INFO(("%s: pending_len(%d) is exceeded threshold(%d), pktcount(%d)\n",
			__FUNCTION__, pending_len, ring->threshold, pktlog_ring->pktcount));
	}

	return pending_len;

}
#endif /* DHD_PKT_LOGGING_DBGRING */

#ifdef DHD_DEBUGABILITY_DEBUG_DUMP
int
dhd_debug_dump_ring_push(dhd_pub_t *dhdp, int ring_id, uint32 len, void *data)
{
	eldbg_ring_cmn_t *ring;
	int ret = 0;
	uint32 remain_len = 0;
	char *cur_buf;
	eldbg_ring_entry_t msg_hdr;

	if (!dhdp || !dhdp->dbg) {
		return BCME_BADADDR;
	}

	if (!VALID_RING(ring_id)) {
		DHD_ERROR(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}

	ring = &dhdp->dbg->dbg_rings[ring_id];

	remain_len = len;
	cur_buf = data;

	bzero(&msg_hdr, sizeof(msg_hdr));
	while (remain_len > 0) {
		msg_hdr.type = DBG_RING_ENTRY_DATA_TYPE;
		msg_hdr.len = MIN(remain_len, DHD_DEBUG_DUMP_NETLINK_MAX);
		ret = eldbg_ring_push(ring, &msg_hdr, cur_buf);
		if (ret == BCME_OK) {
			cur_buf += msg_hdr.len;
			remain_len -= msg_hdr.len;
		}
	}
	return ret;
}
#endif /* DHD_DEBUGABILITY_DEBUG_DUMP */

int
dhd_dbg_push_to_ring(dhd_pub_t *dhdp, int ring_id, eldbg_ring_entry_t *hdr, void *data)
{
	eldbg_ring_cmn_t *ring;
	int ret = 0;
	uint32 pending_len = 0;
	BCM_REFERENCE(pending_len);
	if (!dhdp || !dhdp->dbg) {
		return BCME_BADADDR;
	}

	if (!VALID_RING(ring_id)) {
		DHD_ERROR(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}

	ring = &dhdp->dbg->dbg_rings[ring_id];

#ifdef DHD_PKT_LOGGING_DBGRING
	if (ring_id == PACKET_LOG_RING_ID) {
		ret = eldbg_ring_update(ring, *(uint32*)data);
	} else
#endif /* DHD_PKT_LOGGING_DBGRING */
	{
		ret = eldbg_ring_push(ring, hdr, data);
	}
	if (ret != BCME_OK)
		return ret;
#ifdef SHOW_LOGTRACE
	if (dhdp->dbg->pullreq != NULL) {
#ifdef DHD_PKT_LOGGING_DBGRING
		pending_len = dhd_dbg_urgent_pull(dhdp, ring);
#endif /* DHD_PKT_LOGGING_DBGRING */
		if (pending_len == 0) {
			pending_len = eldbg_ring_get_pending_len(ring);
		}
		eldbg_ring_sched_pull(ring, pending_len, dhdp->dbg->pullreq,
			dhdp->dbg->private, ring->id);
	}
#endif /* SHOW_LOGTRACE */

	return ret;
}

eldbg_ring_cmn_t *
eldbg_get_ring_from_ring_id(eldbg_cmn_t *dbg, int ring_id)
{

	if (dbg == NULL) {
		return NULL;
	}

	if (!VALID_RING(ring_id)) {
		DHD_ERROR(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return NULL;
	}

	return &dbg->dbg_rings[ring_id];
}

int
dhd_dbg_pull_single_from_ring(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len,
	bool strip_header)
{
	eldbg_ring_cmn_t *ring;

	if (!dhdp || !dhdp->dbg) {
		return 0;
	}

	if (!VALID_RING(ring_id)) {
		DHD_ERROR(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}

	ring = &dhdp->dbg->dbg_rings[ring_id];

	return eldbg_ring_pull_single(ring, data, buf_len, strip_header);
}

#ifdef DHD_PKT_LOGGING_DBGRING
extern int dhd_pktlog_dump_write(dhd_pub_t *dhdp, void *file,
	const void *user_buf, uint32 *written_bytes);

int
dhd_dbg_pull_from_pktlog(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len)
{
	eldbg_ring_entry_t *r_entry = NULL;
	char *buf = NULL;
	int ret = BCME_OK;
	uint32 written_bytes = 0;

	if (!dhdp || !dhdp->dbg) {
		return 0;
	}
	if (!VALID_RING(ring_id)) {
		DHD_DBGIF(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}
	if (ring_id != PACKET_LOG_RING_ID) {
		return BCME_UNSUPPORTED;
	}

	r_entry = (eldbg_ring_entry_t *)data;
	r_entry->len = buf_len - ELDBG_RING_ENTRY_SIZE;
	written_bytes = (uint32)r_entry->len;
	if (!written_bytes) {
		return 0;
	}

	r_entry->type = DBG_RING_ENTRY_DATA_TYPE;
	r_entry->flags = (DBG_RING_ENTRY_FLAGS_HAS_TIMESTAMP |
		DBG_RING_ENTRY_FLAGS_HAS_BINARY);
	r_entry->timestamp = local_clock();
	r_entry->timestamp = DIV_U64_BY_U32(r_entry->timestamp, NSEC_PER_MSEC);
	buf = (char*)data;

	ret = dhd_pktlog_dump_write(dhdp, NULL, buf + ELDBG_RING_ENTRY_SIZE, &written_bytes);

	if (ret != BCME_OK) {
		DHD_DBGIF(("%s : pktlog_dump error\n", __FUNCTION__));
		buf_len = 0;
	} else {
		r_entry->len = (uint16)written_bytes;
		buf_len = r_entry->len + ELDBG_RING_ENTRY_SIZE;
	}

	return buf_len;
}
#endif /* DHD_PKT_LOGGING_DBGRING */

int
dhd_dbg_pull_from_ring(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len)
{
	eldbg_ring_cmn_t *ring;

	if (!dhdp || !dhdp->dbg)
		return 0;
	if (!VALID_RING(ring_id)) {
		DHD_DBGIF(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}
	ring = &dhdp->dbg->dbg_rings[ring_id];
	return eldbg_ring_pull(ring, data, buf_len, FALSE);
}

static int
dhd_dbg_msgtrace_seqchk(uint32 *prev, uint32 cur)
{
	/* normal case including wrap around */
	if ((cur == 0 && *prev == 0xFFFFFFFF) || ((cur - *prev) == 1)) {
		goto done;
	} else if (cur == *prev) {
		DHD_EVENT(("%s duplicate trace\n", __FUNCTION__));
		return -1;
	} else if (cur > *prev) {
		DHD_EVENT(("%s lost %d packets\n", __FUNCTION__, cur - *prev));
	} else {
		DHD_EVENT(("%s seq out of order, dhd %d, dongle %d\n",
			__FUNCTION__, *prev, cur));
	}
done:
	*prev = cur;
	return 0;
}

static void
dhd_dbg_msgtrace_msg_parser(void *event_data)
{
	msgtrace_hdr_t *hdr;
	char *data, *s;
	static uint32 seqnum_prev = 0;

	if (!event_data) {
		DHD_ERROR(("%s: event_data is NULL\n", __FUNCTION__));
		return;
	}

	hdr = (msgtrace_hdr_t *)event_data;
	data = (char *)event_data + MSGTRACE_HDRLEN;

	/* There are 2 bytes available at the end of data */
	data[ntoh16(hdr->len)] = '\0';

	if (ntoh32(hdr->discarded_bytes) || ntoh32(hdr->discarded_printf)) {
		DHD_DBGIF(("WLC_E_TRACE: [Discarded traces in dongle -->"
			"discarded_bytes %d discarded_printf %d]\n",
			ntoh32(hdr->discarded_bytes),
			ntoh32(hdr->discarded_printf)));
	}

	if (dhd_dbg_msgtrace_seqchk(&seqnum_prev, ntoh32(hdr->seqnum)))
		return;

	/* Display the trace buffer. Advance from
	 * \n to \n to avoid display big
	 * printf (issue with Linux printk )
	 */
	while (*data != '\0' && (s = strstr(data, "\n")) != NULL) {
		*s = '\0';
		DHD_FWLOG(("[FWLOG] %s\n", data));
		data = s+1;
	}
	if (*data)
		DHD_FWLOG(("[FWLOG] %s", data));
}

#ifdef SHOW_LOGTRACE
#define DATA_UNIT_FOR_LOG_CNT 4

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

int
replace_percent_p_to_x(char *fmt)
{
	int p_to_x_done = FALSE;

	while (*fmt != '\0')
	{
		/* Skip characters will we see a % */
		if (*fmt++ != '%')
		{
			continue;
		}

		/*
		 * Skip any flags, field width and precision:
		 *Flags: Followed by %
		 * #, 0, -, ' ', +
		 */
		if (*fmt == '#')
			fmt++;

		if (*fmt == '0' || *fmt == '-' || *fmt == '+')
			fmt++;

		/*
		 * Field width:
		 * An optional decimal digit string (with non-zero first digit)
		 * specifying a minimum field width
		 */
		while (*fmt && bcm_isdigit(*fmt))
			fmt++;

		/*
		 * Precision:
		 * An optional precision, in the form of a period ('.')  followed by an
		 * optional decimal digit string.
		 */
		if (*fmt == '.')
		{
			fmt++;
			while (*fmt && bcm_isdigit(*fmt)) fmt++;
		}

		/* If %p is seen, change it to %x */
		if (*fmt == 'p')
		{
			*fmt = 'x';
			p_to_x_done = TRUE;
		}
		if (*fmt)
			fmt++;
	}

	return p_to_x_done;
}

/* To identify format of types %Ns where N >= 0 is a number */
bool
check_valid_string_format(char *curr_ptr)
{
	char *next_ptr;
	if ((next_ptr = bcmstrstr(curr_ptr, "s")) != NULL) {
		/* Default %s format */
		if (curr_ptr == next_ptr) {
			return TRUE;
		}

		/* Verify each charater between '%' and 's' is a valid number */
		while (curr_ptr < next_ptr) {
			if (bcm_isdigit(*curr_ptr) == FALSE) {
				return FALSE;
			}
			curr_ptr++;
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

/* To identify format of non string format types */
bool
check_valid_non_string_format(char *curr_ptr)
{
	char *next_ptr;
	char *next_fmt_stptr;
	char valid_fmt_types[17] = {'d', 'i', 'x', 'X', 'c', 'p', 'u',
			'f', 'F', 'e', 'E', 'g', 'G', 'o',
			'a', 'A', 'n'};
	int i;
	bool valid = FALSE;

	/* Check for next % in the fmt str */
	next_fmt_stptr = bcmstrstr(curr_ptr, "%");

	for (next_ptr = curr_ptr; *next_ptr != '\0'; next_ptr++) {
		for (i = 0; i < (int)((sizeof(valid_fmt_types))/sizeof(valid_fmt_types[0])); i++) {
			if (*next_ptr == valid_fmt_types[i]) {
				/* Check whether format type found corresponds to current %
				 * and not the next one, if exists.
				 */
				if ((next_fmt_stptr == NULL) ||
						(next_fmt_stptr && (next_ptr < next_fmt_stptr))) {
					/* Not validating for length/width fields in
					 * format specifier.
					 */
					valid = TRUE;
				}
				goto done;
			}
		}
	}

done:
	return valid;
}

#define MAX_NO_OF_ARG	16
#define FMTSTR_SIZE	200
#define ROMSTR_SIZE	268
#define SIZE_LOC_STR	50
#define LOG_PRINT_CNT_MAX	16u
#define EL_MSEC_PER_SEC	1000u
#define NANOSEC_IN_1_SECOND	1000000000u
#define EL_USEC_PER_MSEC	1000u
#ifdef DHD_LOG_PRINT_RATE_LIMIT
#define MAX_LOG_PRINT_COUNT 100u
#define LOG_PRINT_THRESH (1u * USEC_PER_SEC)
#endif
#define EL_PARSE_VER	"V02"

#ifdef SHOW_LOGTRACE

static uint64 verboselog_ts_saved = 0;

bool
dhd_dbg_process_event_log_hdr(event_log_hdr_t *log_hdr, prcd_event_log_hdr_t *prcd_log_hdr)
{
	event_log_extended_hdr_t *ext_log_hdr = NULL;
	uint16 event_log_fmt_num;
	uint8 event_log_hdr_type;
	uint8 extended_fmtnum = 0;

	/* Identify the type of event tag, payload type etc..  */
	event_log_hdr_type = log_hdr->fmt_num & DHD_EVENT_LOG_HDR_MASK;
	event_log_fmt_num = (log_hdr->fmt_num >> DHD_EVENT_LOG_FMT_NUM_OFFSET) &
		DHD_EVENT_LOG_FMT_NUM_MASK;

	switch (event_log_hdr_type) {
		case DHD_OW_NB_EVENT_LOG_HDR:
			prcd_log_hdr->ext_event_log_hdr = FALSE;
			prcd_log_hdr->binary_payload = FALSE;
			break;
		case DHD_TW_NB_EVENT_LOG_HDR:
			prcd_log_hdr->ext_event_log_hdr = TRUE;
			prcd_log_hdr->binary_payload = FALSE;
			break;
		case DHD_BI_EVENT_LOG_HDR:
			if (event_log_fmt_num == DHD_OW_BI_EVENT_FMT_NUM) {
				prcd_log_hdr->ext_event_log_hdr = FALSE;
				prcd_log_hdr->binary_payload = TRUE;
			} else if (event_log_fmt_num == DHD_TW_BI_EVENT_FMT_NUM) {
				prcd_log_hdr->ext_event_log_hdr = TRUE;
				prcd_log_hdr->binary_payload = TRUE;
			} else {
				DHD_ERROR(("%s: invalid format number 0x%X\n",
					__FUNCTION__, event_log_fmt_num));
				return FALSE;
			}
			break;
		case DHD_INVALID_EVENT_LOG_HDR:
		default:
			DHD_ERROR(("%s: invalid event log header type 0x%X\n",
				__FUNCTION__, event_log_hdr_type));
			return FALSE;
	}

	/* Parse extended and legacy event log headers and populate prcd_event_log_hdr_t */
	if (prcd_log_hdr->ext_event_log_hdr) {
		ext_log_hdr = (event_log_extended_hdr_t *)
			((uint8 *)log_hdr - sizeof(event_log_hdr_t));
		extended_fmtnum = (ext_log_hdr->extended_fmtnum & DHD_EXT_FMTNUM_MASK);
	}
	/* Parse extended event log tag */
	if ((prcd_log_hdr->ext_event_log_hdr) && (ext_log_hdr != NULL)) {
		prcd_log_hdr->tag = ((ext_log_hdr->extended_tag &
			DHD_TW_VALID_TAG_BITS_MASK) << DHD_TW_EVENT_LOG_TAG_OFFSET) | log_hdr->tag;
	} else {
		prcd_log_hdr->tag = log_hdr->tag;
	}
	/* Parse extended format number */
	if ((event_log_hdr_type == DHD_TW_NB_EVENT_LOG_HDR) && (extended_fmtnum != 0)) {
		prcd_log_hdr->fmt_num_raw = (extended_fmtnum << 16) | log_hdr->fmt_num;
		prcd_log_hdr->fmt_num = (extended_fmtnum << 14) | event_log_fmt_num;

	} else {
		prcd_log_hdr->fmt_num_raw = log_hdr->fmt_num;
		prcd_log_hdr->fmt_num = event_log_fmt_num;
	}
	prcd_log_hdr->count = log_hdr->count;

	/* update arm cycle */
	/*
	 * For loegacy event tag :-
	 * |payload........|Timestamp| Tag
	 *
	 * For extended event tag:-
	 * |payload........|Timestamp|extended Tag| Tag.
	 *
	 */
	prcd_log_hdr->armcycle = prcd_log_hdr->ext_event_log_hdr ?
		*(uint32 *)(log_hdr - EVENT_TAG_TIMESTAMP_EXT_OFFSET) :
		*(uint32 *)(log_hdr - EVENT_TAG_TIMESTAMP_OFFSET);

	/* update event log data pointer address */
	prcd_log_hdr->log_ptr =
		(uint32 *)log_hdr - log_hdr->count - prcd_log_hdr->ext_event_log_hdr;

	/* handle error cases above this */
	return TRUE;
}

static void
dhd_log_enhanced_timestamp(dhd_pub_t *dhdp, event_log_hdr_t *ts_hdr, uint32 *data)
{
	ets_msg_t *ets_msg_ptr = (ets_msg_t *)ts_hdr - ts_hdr->count;

	if (!dhdp->dbg->event_log_ts_ver) {
		dhdp->dbg->event_log_ts_ver = ets_msg_ptr->version;
	}

	switch (ets_msg_ptr->version) {
		case ENHANCED_TS_MSG_VERSION_1: {
			ets_msg_v1_t *ets = (ets_msg_v1_t *)ets_msg_ptr;
			if ((uint32*)ets >= data &&
				ts_hdr->count >= (sizeof(ets_msg_v1_t) / sizeof(uint32))) {
				DHD_MSGTRACE_LOG(("EVENT_LOG_ENHANCED_TS_V1: "
					"SYS:%08x CPU:%08x CPUFREQ:%u\n",
					ets->timestamp, ets->cyclecount, ets->cpu_freq));
			}
			break;
		}
		case ENHANCED_TS_MSG_VERSION_2: {
			ets_msg_v2_t *ets = (ets_msg_v2_t *)ets_msg_ptr->data;
			uint16 ets_msg_size = (ts_hdr->count - 1) * sizeof(uint32);
			if ((uint32*)ets >= data &&
				ts_hdr->count >= (sizeof(ets_msg_v2_t) / sizeof(uint32))) {
				memcpy_s(dhdp->dbg->ets_msg, sizeof(dhdp->dbg->ets_msg),
					ets_msg_ptr, ets_msg_size);
				DHD_MSGTRACE_LOG(("EVENT_LOG_ENHANCED_TS_V2: "
					"SYS:%llu PTM:%llu HostTime:%llu\n",
					ets->sysuptime_ns, ets->ets_write_ptm_time,
					ets->host_time_offset));
			}
		}
		break;
		default:
			DHD_MSGTRACE_LOG(("%s unsupported timestamp version: %u\n",
				__FUNCTION__, ets_msg_ptr->version));
		break;
	}

}

static void
dhd_dbg_verboselog_handler(dhd_pub_t *dhdp, prcd_event_log_hdr_t *plog_hdr,
		void *raw_event_ptr, uint32 logset, uint16 block, uint32* data)
{
	event_log_hdr_t *ts_hdr;
	uint32 *log_ptr = plog_hdr->log_ptr;
	char fmtstr_loc_buf[ROMSTR_SIZE] = { 0 };
	uint32 rom_str_len = 0;
	uint32 *ts_data;

	if (!raw_event_ptr) {
		return;
	}

	if (log_ptr < data) {
		DHD_ERROR(("Invalid log pointer, logptr : %p data : %p \n", log_ptr, data));
		return;
	}

	if (log_ptr > data) {
		/* Get time stamp if it's updated */
		ts_hdr = (event_log_hdr_t *)((char *)log_ptr - sizeof(event_log_hdr_t));
		if (ts_hdr->tag == EVENT_LOG_TAG_TS) {
			ts_data = (uint32 *)ts_hdr - ts_hdr->count;
			if (ts_data >= data) {
				verboselog_ts_saved = (uint64)ts_data[0];
				DHD_MSGTRACE_LOG(("EVENT_LOG_TS[0x%08x]: SYS:%08x CPU:%08x\n",
					ts_data[ts_hdr->count - 1], ts_data[0], ts_data[1]));
			}
		} else if (ts_hdr->tag == EVENT_LOG_TAG_ENHANCED_TS) {
			dhd_log_enhanced_timestamp(dhdp, ts_hdr, data);
		}
	}

	if (plog_hdr->tag == EVENT_LOG_TAG_ROM_PRINTF) {
		rom_str_len = (plog_hdr->count - 1) * sizeof(uint32);
		if (rom_str_len >= (ROMSTR_SIZE -1))
			rom_str_len = ROMSTR_SIZE - 1;

		/* copy all ascii data for ROM printf to local string */
		memcpy(fmtstr_loc_buf, log_ptr, rom_str_len);
		/* add end of line at last */
		fmtstr_loc_buf[rom_str_len] = '\0';

		DHD_MSGTRACE_LOG(("EVENT_LOG_ROM[0x%08x]: %s",
				log_ptr[plog_hdr->count - 1], fmtstr_loc_buf));

		if (!strlen(fmtstr_loc_buf)) {
			DHD_ERROR(("%s fmt_str_loc_buf len can not be zero\n", __FUNCTION__));
			return;
		}

		/* Add newline if missing */
		if (fmtstr_loc_buf[strlen(fmtstr_loc_buf) - 1] != '\n')
			DHD_MSGTRACE_LOG(("\n"));

		return;
	}

	if (plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE ||
		plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE_TLV) {
#ifdef MOBILITY_LOG_TRACE
		wl_mschdbg_verboselog_handler(dhdp, raw_event_ptr, plog_hdr, log_ptr);
#endif /* MOBILITY_LOG_TRACE */
		return;
	}

	/* print the message out in a logprint  */
	dhd_dbg_verboselog_printf(dhdp, plog_hdr, raw_event_ptr, log_ptr, logset, block, FALSE);
}

#endif /* SHOW_LOGTRACE */

#ifdef SHOW_LOGTRACE
#define DIV_AND_MOD_U64_BY_U32(dividend, divisor)	do_div(dividend, divisor)

	/* (u32)remainder = (u64)dividend % (u32)divisor */
#define MOD_U64_BY_U32(dividend, divisor) ({					\
		uint64 temp_dividend = (dividend);				\
		uint32 rem = DIV_AND_MOD_U64_BY_U32(temp_dividend, (divisor));	\
		rem;								\
	})
#define BCMUTILS_EVENT_LOG_TS_CPU_FREQUENCY_MULTIPLE 1000u

/* Adapted from version 1 timestamp computation in logprint.c:process_event_log_data() */
static uint64
bcmutils_event_log_compute_current_time_v1(ets_msg_t *ets_msg, uint32 log_record_time)
{
	uint32 base_cycles;
	uint32 cpu_freq;
	uint64 base_time;
	int64 cycle_count_offset;

	ets_msg_v1_t *ets_msg_v1 = (ets_msg_v1_t *)ets_msg;

	/* ets_msg_v1->cyclecount is the ARM cycle count */
	base_cycles = ets_msg_v1->cyclecount;
	cpu_freq = ets_msg_v1->cpu_freq * BCMUTILS_EVENT_LOG_TS_CPU_FREQUENCY_MULTIPLE;

	/* ets_msg_v1->timestamp is in ms and holds sys up time */
	base_time = ets_msg_v1->timestamp;

	cycle_count_offset = log_record_time - base_cycles;
	if (cycle_count_offset >= 0) {
		base_time += (log_record_time - base_cycles)/cpu_freq;
	} else {
		/* Negative cycle_count_offset == counter wrapped */
		base_time += (log_record_time +
				(0xffffffffu - base_cycles))/cpu_freq;
	}

	/* basetime is in ms. convert to uint64 and return */
	return base_time;
}

static uint64
bcmutils_event_log_compute_current_time_v2(ets_msg_t *ets_msg, uint64 q4_log_record_time)
{
	ets_msg_v2_t *ets_msg_v2 = (ets_msg_v2_t *)ets_msg->data;
	uint64 ets_msg_40bit_ptm_time = ets_msg_v2->ets_write_ptm_time & 0xFFFFFFFFFFULL;
	uint64 el_40bit_log_record_time = q4_log_record_time << 8;

	uint64 delta_time;

	/* Corner case. The top 40 bits match indicating logging happened in the same time unit
	 * as recorded RAW PTM time in the ETS message.
	 */
	if (q4_log_record_time == (ets_msg_40bit_ptm_time >> 8)) {
		delta_time = 0;
	} else {
		delta_time = (el_40bit_log_record_time - ets_msg_40bit_ptm_time) &
			0xFFFFFFFFFFULL;
	}
	return ets_msg_v2->sysuptime_ns + delta_time;
}

int
bcmutils_event_log_compute_current_time_1(void *ets_msg, uint64 log_record_time,
	uint64 *current_time)
{
	ets_msg_t *ets_msg_ptr = (ets_msg_t *)ets_msg;

	if (ets_msg_ptr == NULL) {
		return BCME_ERROR;
	}

	if (ets_msg_ptr->version == ENHANCED_TS_MSG_VERSION_1) {
		/* log record time is a 32-bit ARM cycle count */
		*current_time = bcmutils_event_log_compute_current_time_v1(ets_msg_ptr,
			(uint32) log_record_time);
	} else if (ets_msg_ptr->version == ENHANCED_TS_MSG_VERSION_2) {
		*current_time = bcmutils_event_log_compute_current_time_v2(ets_msg_ptr,
			log_record_time);
	} else {
		return BCME_VERSION;
	}

	return BCME_OK;
}

#ifdef EVENT_LOG_TS
static void
dhd_dbg_fw_time(dhd_pub_t *dhdp, uint32 arm_time_cycle, char *buf, uint32 len)
{
	uint64 ptm_fw_time_ns = 0;
	uint ret;
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;

	if (!buf) {
		DHD_ERROR(("%s NULL buf\n", __FUNCTION__));
		return;
	}

	bcm_binit(strbuf, buf, len);
	switch (dhdp->dbg->event_log_ts_ver) {
		case ENHANCED_TS_MSG_VERSION_2:
			ret = bcmutils_event_log_compute_current_time_1(
				dhdp->dbg->ets_msg, arm_time_cycle, &ptm_fw_time_ns);
			if (ret == BCME_OK) {
				bcm_bprintf(strbuf, "%lld.%lld",
					(do_div(ptm_fw_time_ns, NANOSEC_IN_1_SECOND)),
					(MOD_U64_BY_U32(ptm_fw_time_ns, NANOSEC_IN_1_SECOND)));
			} else {
				DHD_ERROR(("%s ret: %d ENHANCED_TS_MSG_VERSION_2 compute error\n",
					__FUNCTION__, ret));
			}
			break;
		case ENHANCED_TS_MSG_VERSION_1:
			bcm_bprintf(strbuf, "%06d.%03d",
				(arm_time_cycle / EL_MSEC_PER_SEC),
				(arm_time_cycle % EL_MSEC_PER_SEC));
			break;
		default:
			DHD_MSGTRACE_LOG(("%s unsupported timestamp version: %u\n",
				__FUNCTION__, dhdp->dbg->event_log_ts_ver));
		break;
	}
}
#else
static void
dhd_dbg_fw_time(dhd_pub_t *dhdp, time_us_t sysuptime_us, char *buf, uint32 len)
{
	ASSERT(buf != NULL);

	snprintf(buf, len, "%06d.%03d",
		(sysuptime_us / EL_USEC_PER_MSEC),
		(sysuptime_us % EL_USEC_PER_MSEC));
}
#endif /* EVENT_LOG_TS */

void el_decoded_format_print(char *console, unsigned tag, const char *fmt, ...)
{
	int len = 0;
	char tmp_buf[DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE] = {0, };
	va_list args;

	va_start(args, fmt);
	len = vsnprintf(tmp_buf, DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE, fmt, args);
	/* Non ANSI C99 compliant returns -1,
	 * ANSI compliant return len >= DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE
	 */
	va_end(args);
	if (len < 0) {
		return;
	}
	/* print out the parsed format el log */
	DHD_ELLOG(("%s: EVENT_LOG[%d]: %s", console, tag, tmp_buf));
}

void
dhd_dbg_verboselog_printf(dhd_pub_t *dhdp, prcd_event_log_hdr_t *plog_hdr,
	void *raw_event_ptr, uint32 *log_ptr, uint32 logset, uint16 block, bool coex_log)
{
	dhd_event_log_t *raw_event = (dhd_event_log_t *)raw_event_ptr;
	uint16 count;
	int log_level, id;
	char fmtstr_loc_buf[ROMSTR_SIZE] = { 0 };
	char (*str_buf)[SIZE_LOC_STR] = NULL;
	char *str_tmpptr = NULL;
	uint32 addr = 0;
	typedef union {
		uint32 val;
		char * addr;
	} u_arg;
	u_arg arg[MAX_NO_OF_ARG] = {{0}};
	char *c_ptr = NULL;
	struct bcmstrbuf b;
#ifdef DHD_LOG_PRINT_RATE_LIMIT
	static int log_print_count = 0;
	static uint64 ts0 = 0;
	uint64 ts1 = 0;
#endif /* DHD_LOG_PRINT_RATE_LIMIT */
	char ptm_fw_time[PTM_FW_TIME_LEN] = { 0 };
	dhd_event_log_fmt_t *log_fmts;
	dhd_event_log_map_t *rammap;
	dhd_event_log_map_t *rommap;
	char *raw_print_prefix = "EL";
	char *console_print_prefix = "CONSOLE_E";

	dhd_event_log_fmt_t log_fmts_local;
	dhd_event_log_map_t rammap_local;
	dhd_event_log_map_t rommap_local;

	log_fmts_local.num_fmts = raw_event->num_fmts;
	log_fmts_local.fmts = raw_event->fmts;
	log_fmts_local.raw_fmts = raw_event->raw_fmts;
	log_fmts_local.fmts_size = raw_event->fmts_size;
	log_fmts_local.raw_fmts_size = raw_event->raw_fmts_size;

	rammap_local.raw_sstr = raw_event->raw_sstr;
	rammap_local.raw_sstr_size = raw_event->raw_sstr_size;
	rammap_local.ramstart = raw_event->ramstart;
	rammap_local.rodata_start = raw_event->rodata_start;
	rammap_local.rodata_end = raw_event->rodata_end;

	rommap_local.raw_sstr = raw_event->rom_raw_sstr;
	rommap_local.raw_sstr_size = raw_event->rom_raw_sstr_size;
	rommap_local.ramstart = raw_event->rom_ramstart;
	rommap_local.rodata_start = raw_event->rom_rodata_start;
	rommap_local.rodata_end = raw_event->rom_rodata_end;

	log_fmts = &log_fmts_local;
	rammap = &rammap_local;
	rommap = &rommap_local;

	BCM_REFERENCE(arg);

#ifdef DHD_LOG_PRINT_RATE_LIMIT
	if (!ts0)
		ts0 = OSL_SYSUPTIME_US();

	ts1 = OSL_SYSUPTIME_US();

	if (((ts1 - ts0) <= LOG_PRINT_THRESH) && (log_print_count >= MAX_LOG_PRINT_COUNT)) {
		log_print_threshold = 1;
		ts0 = 0;
		log_print_count = 0;
		DHD_ERROR(("%s: Log print water mark is reached,"
			" console logs are dumped only to debug_dump file\n", __FUNCTION__));
	} else if ((ts1 - ts0) > LOG_PRINT_THRESH) {
		log_print_threshold = 0;
		ts0 = 0;
		log_print_count = 0;
	}
#endif /* DHD_LOG_PRINT_RATE_LIMIT */

	/* print the message out in a logprint  */
	if ((control_logtrace == LOGTRACE_RAW_FMT) || !(log_fmts->fmts)) {
		if (dhdp->dbg && dhdp->dbg->dbg_rings[FW_VERBOSE_RING_ID].state != RING_STOP &&
			!coex_log) {
			log_level = dhdp->dbg->dbg_rings[FW_VERBOSE_RING_ID].log_level;
			for (id = 0; id < ARRAYSIZE(fw_verbose_level_map); id++) {
				if ((fw_verbose_level_map[id].tag == plog_hdr->tag) &&
					(fw_verbose_level_map[id].log_level > log_level))
					return;
			}
		}
		if (plog_hdr->binary_payload) {
			dhd_dbg_fw_time(dhdp, log_ptr[plog_hdr->count - 1],
				ptm_fw_time, sizeof(ptm_fw_time));
			DHD_ECNTR_LOG(("%s %s:tag=%d len=%d fmt=0x%x",
				&ptm_fw_time[0],
				raw_print_prefix,
				plog_hdr->tag,
				plog_hdr->count,
				plog_hdr->fmt_num_raw));

			for (count = 0; count < (plog_hdr->count - 1); count++) {
				/* skip first line feed in case count 0 */
				if (count && (count % LOG_PRINT_CNT_MAX == 0)) {
					DHD_ECNTR_LOG(("\n\t%08x", log_ptr[count]));
				} else {
					DHD_ECNTR_LOG((" %08x", log_ptr[count]));
				}
			}
			DHD_ECNTR_LOG(("\n"));
		} else {
			bcm_binit(&b, fmtstr_loc_buf, FMTSTR_SIZE);
			/* The 'hdr->count - 1' is dongle time */
			dhd_dbg_fw_time(dhdp, log_ptr[plog_hdr->count - 1],
				ptm_fw_time, sizeof(ptm_fw_time));
#ifndef OEM_ANDROID
			bcm_bprintf(&b, "%s %s: %d 0x%x",
				&ptm_fw_time[0],
				raw_print_prefix,
				plog_hdr->tag,
				plog_hdr->fmt_num_raw);
#endif /* !OEM_ANDROID */

// MOG-ON: OEM_ANDROID
#ifdef OEM_ANDROID
			bcm_bprintf(&b, "%s %s:%s:%u:%u %d %d 0x%x",
				&ptm_fw_time[0],
				raw_print_prefix,
				EL_PARSE_VER, logset, block,
				plog_hdr->tag,
				plog_hdr->count,
				plog_hdr->fmt_num_raw);
#endif /* !OEM_ANDROID */
// MOG-OFF: OEM_ANDROID

			for (count = 0; count < (plog_hdr->count - 1); count++) {
				bcm_bprintf(&b, " %x", log_ptr[count]);
			}
			/* raw format el printout */
			DHD_INFO(("%s, b=%s \n", __FUNCTION__, b.origbuf));
			/* ensure preserve fw logs go to debug_dump only in case of customer4 */
			if (!coex_log && logset < dhdp->event_log_max_sets &&
				((0x01ULL << logset) & dhdp->logset_prsrv_mask)) {
				DHD_PRSRV_MEM(("%s\n", b.origbuf));
			} else {
				DHD_FWLOG(("%s\n", b.origbuf));
#ifdef DHD_LOG_PRINT_RATE_LIMIT
				log_print_count++;
#endif /* DHD_LOG_PRINT_RATE_LIMIT */
			}
		}
		return;
	}

	str_buf = MALLOCZ(dhdp->osh, (MAX_NO_OF_ARG * SIZE_LOC_STR));
	if (!str_buf) {
		DHD_ERROR(("%s: malloc failed str_buf\n", __FUNCTION__));
		return;
	}

	if ((plog_hdr->fmt_num) < log_fmts->num_fmts) {
		if (plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE) {
			snprintf(fmtstr_loc_buf, FMTSTR_SIZE, "%s",
				log_fmts->fmts[plog_hdr->fmt_num]);
			plog_hdr->count++;
		} else {
			dhd_dbg_fw_time(dhdp, log_ptr[plog_hdr->count - 1],
				ptm_fw_time, sizeof(ptm_fw_time));
			snprintf(fmtstr_loc_buf, FMTSTR_SIZE, "%u:%u %s %s",
				logset, block,
				&ptm_fw_time[0],
				log_fmts->fmts[plog_hdr->fmt_num]);
		}
		c_ptr = fmtstr_loc_buf;
	} else {
		/* for ecounters, don't print the error as it will flood */
		if ((plog_hdr->fmt_num != DHD_OW_BI_EVENT_FMT_NUM) &&
			(plog_hdr->fmt_num != DHD_TW_BI_EVENT_FMT_NUM)) {
			DHD_ERROR(("%s: fmt number: 0x%x out of range\n",
				__FUNCTION__, plog_hdr->fmt_num));
		} else {
			DHD_INFO(("%s: fmt number: 0x%x out of range\n",
				__FUNCTION__, plog_hdr->fmt_num));
		}

		goto exit;
	}

	if (plog_hdr->count > MAX_NO_OF_ARG) {
		DHD_INFO(("%s: plog_hdr->count(%d) out of range\n",
			__FUNCTION__, plog_hdr->count));
		goto exit;
	}

	/* print the format string which will be needed for debugging incorrect formats */
	DHD_INFO(("%s: fmtstr_loc_buf = %s\n", __FUNCTION__, fmtstr_loc_buf));

	/* Replace all %p to %x to handle 32 bit %p */
	replace_percent_p_to_x(fmtstr_loc_buf);

	for (count = 0; count < (plog_hdr->count - 1); count++) {
		if (c_ptr != NULL)
			if ((c_ptr = bcmstrstr(c_ptr, "%")) != NULL)
				c_ptr++;

		if (c_ptr != NULL) {
			if (check_valid_string_format(c_ptr)) {
				if ((rammap->raw_sstr) &&
					((log_ptr[count] > rammap->rodata_start) &&
					(log_ptr[count] < rammap->rodata_end))) {
					/* ram static string */
					addr = log_ptr[count] - rammap->rodata_start;
					str_tmpptr = rammap->raw_sstr + addr;
					memcpy(str_buf[count], str_tmpptr,
						SIZE_LOC_STR);
					str_buf[count][SIZE_LOC_STR-1] = '\0';
					arg[count].addr = str_buf[count];
				} else if (rommap && (rommap->raw_sstr) &&
						((log_ptr[count] >
						rommap->rodata_start) &&
						(log_ptr[count] <
						rommap->rodata_end))) {
					/* rom static string */
					addr = log_ptr[count] - rommap->rodata_start;
					str_tmpptr = rommap->raw_sstr + addr;
					memcpy(str_buf[count], str_tmpptr,
						SIZE_LOC_STR);
					str_buf[count][SIZE_LOC_STR-1] = '\0';
					arg[count].addr = str_buf[count];
				} else {
					/*
					*  Dynamic string OR
					* No data for static string.
					* So store all string's address as string.
					*/
					snprintf(str_buf[count], SIZE_LOC_STR,
						"(s)0x%x", log_ptr[count]);
					arg[count].addr = str_buf[count];
				}
			} else if (check_valid_non_string_format(c_ptr)) {
				/* Other than string format */
				arg[count].val = log_ptr[count];

			} else {
				*(c_ptr - 1) = '\0';
				break;
			}
		}
	}

#ifdef SHOW_LOGTRACE
		el_decoded_format_print(console_print_prefix, plog_hdr->tag, fmtstr_loc_buf,
			arg[0], arg[1], arg[2], arg[3],	arg[4], arg[5], arg[6],
			arg[7], arg[8], arg[9], arg[10], arg[11], arg[12], arg[13],
			arg[14], arg[15]);
#endif /* SHOW_LOGTRACE */
	/* ensure preserve fw logs go to debug_dump only in case of customer4 */
#ifdef MOBILITY_LOG_TRACE
	if (!coex_log && logset < dhdp->event_log_max_sets &&
			((0x01ULL << logset) & dhdp->logset_prsrv_mask)) {
		DHD_PRSRV_MEM((fmtstr_loc_buf, arg[0], arg[1], arg[2], arg[3],
			arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10],
			arg[11], arg[12], arg[13], arg[14], arg[15]));
	} else {
		DHD_FWLOG((fmtstr_loc_buf, arg[0], arg[1], arg[2], arg[3],
			arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10],
			arg[11], arg[12], arg[13], arg[14], arg[15]));
#ifdef DHD_LOG_PRINT_RATE_LIMIT
		log_print_count++;
#endif /* DHD_LOG_PRINT_RATE_LIMIT */
	}
#endif /* MOBILITY_LOG_TRACE */

exit:
	MFREE(dhdp->osh, str_buf, (MAX_NO_OF_ARG * SIZE_LOC_STR));
}
#endif /* SHOW_LOGTRACE */

#if defined(EWP_BCM_TRACE) || defined(EWP_RTT_LOGGING) || defined(EWP_ECNTRS_LOGGING) \
	|| defined(EWP_EVENTTS_LOG)
static int
dhd_dbg_send_evtlog_to_ring(prcd_event_log_hdr_t *plog_hdr,
	eldbg_ring_entry_t *msg_hdr, eldbg_ring_cmn_t *ring,
	uint16 max_payload_len, uint8 *logbuf)
{
	event_log_hdr_t *log_hdr;
	struct tracelog_header *logentry_header;
	uint16 len_chk = 0;

	BCM_REFERENCE(log_hdr);
	BCM_REFERENCE(logentry_header);
	/*
	 * check msg hdr len before pushing.
	 * FW msg_hdr.len includes length of event log hdr,
	 * logentry header and payload.
	 */
	len_chk = (sizeof(*logentry_header) + sizeof(*log_hdr) +
			max_payload_len);
	/* account extended event log header(extended_event_log_hdr) */
	if (plog_hdr->ext_event_log_hdr) {
		len_chk += sizeof(*log_hdr);
	}
	if (msg_hdr->len > len_chk) {
		DHD_ERROR(("%s: EVENT_LOG_VALIDATION_FAILS: "
			"msg_hdr->len=%u, max allowed for %s=%u\n",
			__FUNCTION__, msg_hdr->len, ring->name, len_chk));
		return BCME_ERROR;
	}
	eldbg_ring_push(ring, msg_hdr, logbuf);
	return BCME_OK;
}
#endif /* EWP_BCM_TRACE || EWP_RTT_LOGGING || EWP_ECNTRS_LOGGING */

#ifdef SHOW_LOGTRACE
static void
dhd_dbg_logtrace_process_payload(dhd_pub_t *dhdp, char *data, uint datalen, dll_t *list_head)
{
	const uint32 log_hdr_len = sizeof(event_log_hdr_t);
	uint32 log_pyld_len;
	event_log_hdr_t *log_hdr;
	prcd_event_log_hdr_t prcd_log_hdr;
	loglist_item_t *log_item;

	/* start parsing from the tail of packet
	 * Sameple format of a meessage
	 * 001d3c54 00000064 00000064 001d3c54 001dba08 035d6ce1 0c540639
	 * 001d3c54 00000064 00000064 035d6d89 0c580439
	 * 0x0c580439 -- 39 is tag, 04 is count, 580c is format number
	 * all these uint32 values comes in reverse order as group as EL data
	 * while decoding we can only parse from last to first
	 * |<-                     datalen                     ->|
	 * |----(payload and maybe more logs)----|event_log_hdr_t|
	 * data                                  log_hdr
	 */
	dll_init(list_head);

	while (datalen > log_hdr_len) {
		log_hdr = (event_log_hdr_t *)(data + datalen - log_hdr_len);
		bzero(&prcd_log_hdr, sizeof(prcd_log_hdr));
		if (!dhd_dbg_process_event_log_hdr(log_hdr, &prcd_log_hdr)) {
			DHD_ERROR(("%s: Error while parsing event log header\n",
				__FUNCTION__));
		}

		/* skip zero padding at end of frame */
		if (prcd_log_hdr.tag == EVENT_LOG_TAG_NULL) {
			datalen -= log_hdr_len;
			continue;
		}
		/* Check argument count (for non-ecounter events only),
		 * any event log should contain at least
		 * one argument (4 bytes) for arm cycle count and up to 16
		 * arguments except EVENT_LOG_TAG_STATS which could use the
		 * whole payload of 256 words
		 */
		if (prcd_log_hdr.count == 0) {
			break;
		}
		/* Both tag_stats and proxd are binary payloads so skip
		 * argument count check for these.
		 */
		if ((prcd_log_hdr.tag != EVENT_LOG_TAG_STATS) &&
			(prcd_log_hdr.tag != EVENT_LOG_TAG_PROXD_SAMPLE_COLLECT) &&
			(prcd_log_hdr.tag != EVENT_LOG_TAG_ROAM_ENHANCED_LOG) &&
			(prcd_log_hdr.tag != EVENT_LOG_TAG_BCM_TRACE) &&
			(prcd_log_hdr.count > MAX_NO_OF_ARG)) {
			break;
		}

		log_pyld_len = (prcd_log_hdr.count + prcd_log_hdr.ext_event_log_hdr) *
			DATA_UNIT_FOR_LOG_CNT;
		/* log data should not cross the event data boundary */
		if ((uint32)((char *)log_hdr - data) < log_pyld_len) {
			break;
		}
		/* skip 4 bytes time stamp packet */
		if (prcd_log_hdr.tag == EVENT_LOG_TAG_TS ||
			prcd_log_hdr.tag == EVENT_LOG_TAG_ENHANCED_TS) {
			datalen -= (log_pyld_len + log_hdr_len);
			continue;
		}
		if (!(log_item = MALLOC(dhdp->osh, sizeof(*log_item)))) {
			DHD_ERROR(("%s allocating log list item failed\n",
				__FUNCTION__));
			break;
		}

		log_item->prcd_log_hdr = prcd_log_hdr;

		dll_insert(&log_item->list, list_head);
		datalen -= (log_pyld_len + log_hdr_len);
	}
}
#endif /* SHOW_LOGTRACE */

void
dhd_dbg_msgtrace_log_parser(dhd_pub_t *dhdp, void *event_data,
	void *raw_event_ptr, uint datalen, bool msgtrace_hdr_present,
	uint32 msgtrace_seqnum)
{
	msgtrace_hdr_t *hdr;
	char *data, *tmpdata;
	static uint32 seqnum_prev = 0;
	event_log_hdr_t *log_hdr;
	eldbg_ring_entry_t msg_hdr;
	char *logbuf;
	struct tracelog_header *logentry_header;
	uint ring_data_len = 0;
	bool ecntr_pushed = FALSE;
	bool rtt_pushed = FALSE;
	bool bcm_trace_pushed = FALSE;
	bool eventts_pushed = FALSE;
#ifdef SHOW_LOGTRACE
	dll_t list_head, *cur;
	prcd_event_log_hdr_t *plog_hdr;
	bool msg_processed = FALSE;
	loglist_item_t *log_item;
	bool dll_inited = FALSE;
	uint32 logset = 0;
	uint16 block = 0;
#endif /* SHOW_LOGTRACE */
	bool event_log_max_sets_queried;
	uint32 event_log_max_sets;
	uint min_expected_len = 0;
	uint16 len_chk = 0;

	BCM_REFERENCE(ecntr_pushed);
	BCM_REFERENCE(rtt_pushed);
	BCM_REFERENCE(bcm_trace_pushed);
	BCM_REFERENCE(eventts_pushed);
	BCM_REFERENCE(len_chk);

	/* store event_logset_queried and event_log_max_sets in local variables
	 * to avoid race conditions as they were set from different contexts(preinit)
	 */
	event_log_max_sets_queried = dhdp->event_log_max_sets_queried;
	/* Make sure queried is read first with wmb and then max_sets,
	 * as it is done in reverse order during preinit ioctls.
	 */

	OSL_SMP_WMB();
	event_log_max_sets = dhdp->event_log_max_sets;

	if (msgtrace_hdr_present)
		min_expected_len = (MSGTRACE_HDRLEN + EVENT_LOG_BLOCK_LEN);
	else
		min_expected_len = EVENT_LOG_BLOCK_LEN;

	/* log trace event consists of:
	 * msgtrace header
	 * event log block header
	 * event log payload
	 */
	if (!event_data || (datalen <= min_expected_len)) {
		DHD_ERROR(("%s: Not processing due to invalid event_data : %p or length : %d\n",
			__FUNCTION__, event_data, datalen));
		if (event_data && msgtrace_hdr_present) {
			dhd_prhex("event_data dump", (volatile uchar *)event_data, datalen,
				DHD_ERROR_VAL);
			tmpdata = (char *)event_data + MSGTRACE_HDRLEN;
			if (tmpdata) {
				DHD_PRINT(("EVENT_LOG_HDR[0x%x]: Set: 0x%08x length = %d\n",
					ltoh16(*((uint16 *)(tmpdata+2))),
					ltoh32(*((uint32 *)(tmpdata + 4))),
					ltoh16(*((uint16 *)(tmpdata)))));
			}
		} else if (!event_data) {
			DHD_ERROR(("%s: event_data is NULL, cannot dump prhex\n", __FUNCTION__));
		}

		return;
	}

	if (msgtrace_hdr_present) {
		hdr = (msgtrace_hdr_t *)event_data;
		data = (char *)event_data + MSGTRACE_HDRLEN;
		datalen -= MSGTRACE_HDRLEN;
		msgtrace_seqnum = ntoh32(hdr->seqnum);
	} else {
		data = (char *)event_data;
	}

	if (dhd_dbg_msgtrace_seqchk(&seqnum_prev, msgtrace_seqnum))
		return;

	/* Save the whole message to event log ring */
	bzero(&msg_hdr, sizeof(eldbg_ring_entry_t));
	/* VMALLOC is failing, data length might be small */
	logbuf = MALLOCZ(dhdp->osh, sizeof(*logentry_header) + datalen);
	if (logbuf == NULL)
		return;
	logentry_header = (struct tracelog_header *)logbuf;
	logentry_header->magic_num = TRACE_LOG_MAGIC_NUMBER;
	logentry_header->buf_size = datalen;
	logentry_header->seq_num = msgtrace_seqnum;
	msg_hdr.type = DBG_RING_ENTRY_DATA_TYPE;

	ring_data_len = datalen + sizeof(*logentry_header);
	if ((sizeof(*logentry_header) + datalen) > ELDBG_PAYLOAD_MAX_LEN) {
		DHD_ERROR(("%s:Payload len=%u exceeds max len\n", __FUNCTION__,
			((uint)sizeof(*logentry_header) + datalen)));
		goto exit;
	}

	msg_hdr.len = sizeof(*logentry_header) + datalen;
	memcpy(logbuf + sizeof(*logentry_header), data, datalen);
	DHD_DBGIF(("%s: datalen %d %d\n", __FUNCTION__, msg_hdr.len, datalen));

#if defined(EVENT_LOG_ACCESS) || defined(DEBUGABILITY)
	if (BCME_OK != dhd_dbg_push_to_ring(dhdp, FW_VERBOSE_RING_ID, &msg_hdr, logbuf))
	{
		DHD_ERROR(("push failed HL %d DL %d RID:%d\n",
			msg_hdr.len, datalen, FW_VERBOSE_RING_ID));
		goto exit;
	}
#endif /* EVENT_LOG_ACCESS && DEBUGABILITY  */

#ifdef SHOW_LOGTRACE
	/* Print sequence number, originating set and length of received
	 * event log buffer. Refer to event log buffer structure in
	 * event_log.h
	 */
	logset = (ltoh32(*((uint32 *)(data + 4))) & EVENT_LOG_SETID_MASK);

	DHD_MSGTRACE_LOG(("EVENT_LOG_HDR[0x%x]: Set: 0x%08x length = %d\n",
		ltoh16(*((uint16 *)(data+2))), logset, ltoh16(*((uint16 *)(data)))));

	if (logset >= event_log_max_sets) {
		DHD_ERROR(("%s logset: %d max: %d out of range queried: %d\n",
			__FUNCTION__, logset, event_log_max_sets, event_log_max_sets_queried));
#ifdef DHD_FW_COREDUMP
		if (event_log_max_sets_queried && !dhd_memdump_is_scheduled(dhdp)) {
			DHD_ERROR(("%s: collect socram for DUMP_TYPE_LOGSET_BEYOND_RANGE\n",
				__FUNCTION__));
			dhdp->memdump_type = DUMP_TYPE_LOGSET_BEYOND_RANGE;
			dhd_bus_mem_dump(dhdp);
		}
#endif /* DHD_FW_COREDUMP */
	}

	block = ltoh16(*((uint16 *)(data + 2)));
	data += EVENT_LOG_BLOCK_HDRLEN;
	datalen -= EVENT_LOG_BLOCK_HDRLEN;

	dhd_dbg_logtrace_process_payload(dhdp, data, datalen, &list_head);
	dll_inited = TRUE;

	while (!dll_empty(&list_head)) {
		msg_processed = FALSE;
		cur = dll_head_p(&list_head);

		log_item = (loglist_item_t *)container_of(cur, loglist_item_t, list);

		plog_hdr = &log_item->prcd_log_hdr;

#if defined(DHD_LOG_DUMP) && (defined(EWP_ECNTRS_LOGGING) || defined(EWP_RTT_LOGGING) \
	|| defined(EWP_BCM_TRACE) || defined(EWP_EVENTTS_LOG))
		/* most of the event log which needs to be pushed to the
		 * debug rings are binary payload, except TSLOG
		 */
		if (plog_hdr->binary_payload ||
			plog_hdr->tag == EVENT_LOG_TAG_TSLOG) {
			void *dbgring = NULL;
			bool push_evtlog = FALSE;
#ifdef EWP_ECNTRS_LOGGING
			if ((plog_hdr->tag == EVENT_LOG_TAG_ECOUNTERS_TIME_DATA ||
				plog_hdr->tag == EVENT_LOG_TAG_STATS) && !ecntr_pushed &&
				dhd_log_dump_ecntr_enabled()) {
				push_evtlog = TRUE;
				dbgring = dhdp->ecntr_dbg_ring;
				ecntr_pushed = TRUE;
			}
#endif /* EWP_ECNTRS_LOGGING */
#ifdef EWP_RTT_LOGGING
			if (plog_hdr->tag == EVENT_LOG_TAG_PROXD_SAMPLE_COLLECT &&
				!rtt_pushed && dhd_log_dump_rtt_enabled()) {
				push_evtlog = TRUE;
				dbgring = dhdp->rtt_dbg_ring;
				rtt_pushed = TRUE;
			}
#endif /* EWP_RTT_LOGGING */
#ifdef EWP_EVENTTS_LOG
			if (plog_hdr->tag == EVENT_LOG_TAG_TSLOG && !eventts_pushed) {
				push_evtlog = TRUE;
				dbgring = dhdp->eventts_dbg_ring;
				eventts_pushed = TRUE;
			}
#endif /* EWP_EVENTTS_LOG */
#ifdef EWP_BCM_TRACE
			if (logset == EVENT_LOG_SET_BCM_TRACE && !bcm_trace_pushed) {
				push_evtlog = TRUE;
				dbgring = dhdp->bcm_trace_dbg_ring;
				bcm_trace_pushed = TRUE;
			}
#endif /* EWP_BCM_TRACE */

			if (dbgring && push_evtlog) {
				if (dhd_dbg_send_evtlog_to_ring(plog_hdr, &msg_hdr, dbgring,
					EVENT_LOG_MAX_BLOCK_SIZE, logbuf) != BCME_OK) {
					goto exit;
				}
			}
		}
#endif /* DHD_LOG_DUMP && (EWP_ECNTRS_LOGGING || EWP_RTT_LOGGING || EWP_BCM_TRACE) */
		if (!msg_processed) {
			dhd_dbg_verboselog_handler(dhdp, plog_hdr, raw_event_ptr,
				logset, block, (uint32 *)data);
		}
		dll_delete(cur);
		MFREE(dhdp->osh, log_item, sizeof(*log_item));

	}
#endif /* SHOW_LOGTRACE */
	BCM_REFERENCE(log_hdr);
exit:
#ifdef SHOW_LOGTRACE
	while (dll_inited && (!dll_empty(&list_head))) {
		cur = dll_head_p(&list_head);

		GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
		log_item = (loglist_item_t *)container_of(cur, loglist_item_t, list);
		GCC_DIAGNOSTIC_POP();

		dll_delete(cur);
		MFREE(dhdp->osh, log_item, sizeof(*log_item));
	}
#endif /* SHOW_LOGTRACE */
	MFREE(dhdp->osh, logbuf, ring_data_len);
}
#else /* !SHOW_LOGTRACE */
static INLINE void dhd_dbg_verboselog_handler(dhd_pub_t *dhdp,
	prcd_event_log_hdr_t *plog_hdr, void *raw_event_ptr, uint32 logset, uint16 block,
	uint32 *data) {};
INLINE void dhd_dbg_msgtrace_log_parser(dhd_pub_t *dhdp,
	void *event_data, void *raw_event_ptr, uint datalen,
	bool msgtrace_hdr_present, uint32 msgtrace_seqnum) {};
#endif /* SHOW_LOGTRACE */
void
dhd_dbg_trace_evnt_handler(dhd_pub_t *dhdp, void *event_data,
		void *raw_event_ptr, uint datalen)
{
	msgtrace_hdr_t *hdr;

	hdr = (msgtrace_hdr_t *)event_data;

	if (hdr->version != MSGTRACE_VERSION) {
		DHD_DBGIF(("%s unsupported MSGTRACE version, dhd %d, dongle %d\n",
			__FUNCTION__, MSGTRACE_VERSION, hdr->version));
		return;
	}

	if (hdr->trace_type == MSGTRACE_HDR_TYPE_MSG)
		dhd_dbg_msgtrace_msg_parser(event_data);
	else if (hdr->trace_type == MSGTRACE_HDR_TYPE_LOG)
		dhd_dbg_msgtrace_log_parser(dhdp, event_data, raw_event_ptr, datalen, TRUE, 0);
}

#ifdef BTLOG
void
dhd_dbg_bt_log_handler(dhd_pub_t *dhdp, void *data, uint datalen)
{
	eldbg_ring_entry_t msg_hdr;
	int ret;

	/* push to ring */
	bzero(&msg_hdr, sizeof(msg_hdr));
	msg_hdr.type = DBG_RING_ENTRY_DATA_TYPE;
	msg_hdr.len = datalen;
	ret = dhd_dbg_push_to_ring(dhdp, BT_LOG_RING_ID, &msg_hdr, data);
	if (ret != BCME_OK) {
		DHD_ERROR(("%s ring push failed %d\n", __FUNCTION__, ret));
	}
}
#endif	/* BTLOG */

/*
 * dhd_dbg_set_event_log_tag : modify the state of an event log tag
 */
void
dhd_dbg_set_event_log_tag(dhd_pub_t *dhdp, uint16 tag, uint8 set_num, uint8 is_set)
{
	wl_el_tag_params_t pars;
	char *cmd = "event_log_tag_control";
	char iovbuf[WLC_IOCTL_SMLEN] = { 0 };
	int ret;

	bzero(&pars, sizeof(pars));
	pars.tag = tag;
	pars.set = set_num;
	pars.flags = (is_set) ? EVENT_LOG_TAG_FLAG_LOG : EVENT_LOG_TAG_FLAG_NONE;

	if (!bcm_mkiovar(cmd, (char *)&pars, sizeof(pars), iovbuf, sizeof(iovbuf))) {
		DHD_ERROR(("%s mkiovar failed\n", __FUNCTION__));
		return;
	}

	ret = dhd_wl_ioctl_cmd(dhdp, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
	if (ret) {
		DHD_ERROR(("%s set log tag iovar failed %d\n", __FUNCTION__, ret));
	}
}

int
dhd_dbg_set_configuration(dhd_pub_t *dhdp, int ring_id, int log_level, int flags, uint32 threshold)
{
	eldbg_ring_cmn_t *ring;
	uint8 set = 1;
	int i, array_len = 0;
	struct event_log_table *log_level_tbl = NULL;
	if (!dhdp || !dhdp->dbg)
		return BCME_BADADDR;

	if (!VALID_RING(ring_id)) {
		DHD_ERROR(("%s : invalid ring_id : %d\n", __FUNCTION__, ring_id));
		return BCME_RANGE;
	}

	ring = &dhdp->dbg->dbg_rings[ring_id];
	eldbg_ring_config(ring, log_level, threshold);

	if (log_level > 0)
		set = TRUE;

	if (ring->id == FW_VERBOSE_RING_ID) {
		log_level_tbl = fw_verbose_level_map;
		array_len = ARRAYSIZE(fw_verbose_level_map);
	}

	for (i = 0; i < array_len; i++) {
		if (log_level == 0 || (log_level_tbl[i].log_level > log_level)) {
			/* clear the reference per ring */
			ref_tag_tbl[log_level_tbl[i].tag] &= ~(1 << ring_id);
		} else {
			/* set the reference per ring */
			ref_tag_tbl[log_level_tbl[i].tag] |= (1 << ring_id);
		}
		set = (ref_tag_tbl[log_level_tbl[i].tag])? 1 : 0;
		DHD_DBGIF(("%s TAG(%s) is %s for the ring(%s)\n", __FUNCTION__,
			log_level_tbl[i].desc, (set)? "SET" : "CLEAR", ring->name));
		dhd_dbg_set_event_log_tag(dhdp, log_level_tbl[i].tag,
			log_level_tbl[i].set, set);
	}
	return BCME_OK;
}

int
__eldbg_get_ring_status(eldbg_ring_cmn_t *ring, eldbg_ring_status_t *get_ring_status)
{
	eldbg_ring_status_t ring_status;
	int ret = BCME_OK;

	if (ring == NULL) {
		return BCME_BADADDR;
	}

	bzero(&ring_status, sizeof(eldbg_ring_status_t));
	(void)memcpy_s(ring_status.name, sizeof(ring_status.name), ring->name, sizeof(ring->name));
	ring_status.ring_id = ring->id;
	ring_status.ring_buffer_byte_size = ring->ring_size;
	ring_status.written_bytes = ring->stat.written_bytes;
	ring_status.written_records = ring->stat.written_records;
	ring_status.read_bytes = ring->stat.read_bytes;
	ring_status.verbose_level = ring->log_level;
	*get_ring_status = ring_status;

	return ret;
}

/*
* dhd_dbg_get_ring_status : get the ring status from the coresponding ring buffer
* Return: An error code or 0 on success.
*/

int
dhd_dbg_get_ring_status(dhd_pub_t *dhdp, int ring_id, eldbg_ring_status_t *eldbg_ring_status)
{
	int ret = BCME_OK;
	int id = 0;
	eldbg_cmn_t *dbg;
	eldbg_ring_cmn_t *dbg_ring;
	if (!dhdp || !dhdp->dbg)
		return BCME_BADADDR;
	dbg = dhdp->dbg;

	for (id = DEBUG_RING_ID_INVALID + 1; id < DEBUG_RING_ID_MAX; id++) {
		dbg_ring = &dbg->dbg_rings[id];
		if (VALID_RING(dbg_ring->id) && (dbg_ring->id == ring_id)) {
			__eldbg_get_ring_status(dbg_ring, eldbg_ring_status);
			break;
		}
	}
	if (!VALID_RING(id)) {
		DHD_DBGIF(("%s : cannot find the ring_id : %d\n", __FUNCTION__, ring_id));
		ret = BCME_NOTFOUND;
	}
	return ret;
}

#ifdef SHOW_LOGTRACE
void
eldbg_read_ring_into_trace_buf(eldbg_ring_cmn_t *ring, trace_buf_info_t *trace_buf_info)
{
	eldbg_ring_status_t ring_status;
	uint32 rlen = 0;

	rlen = eldbg_ring_pull_single(ring, trace_buf_info->buf, TRACE_LOG_BUF_MAX_SIZE, TRUE);

	trace_buf_info->size = rlen;
	trace_buf_info->availability = NEXT_BUF_NOT_AVAIL;
	if (rlen == 0) {
		trace_buf_info->availability = BUF_NOT_AVAILABLE;
		return;
	}
	__eldbg_get_ring_status(ring, &ring_status);

	if (ring_status.written_bytes != ring_status.read_bytes) {
		trace_buf_info->availability = NEXT_BUF_AVAIL;
	}
}
#endif /* SHOW_LOGTRACE */

/*
* dhd_dbg_find_ring_id : return ring_id based on ring_name
* Return: An invalid ring id for failure or valid ring id on success.
*/

int
dhd_dbg_find_ring_id(dhd_pub_t *dhdp, char *ring_name)
{
	int id;
	eldbg_cmn_t *dbg;
	eldbg_ring_cmn_t *ring;

	if (!dhdp || !dhdp->dbg)
		return BCME_BADADDR;

	dbg = dhdp->dbg;
	for (id = DEBUG_RING_ID_INVALID + 1; id < DEBUG_RING_ID_MAX; id++) {
		ring = &dbg->dbg_rings[id];
		if (!strncmp((char *)ring->name, ring_name, sizeof(ring->name) - 1))
			break;
	}
	return id;
}

/*
* dhd_dbg_get_priv : get the private data of dhd dbugability module
* Return : An NULL on failure or valid data address
*/
void *
dhd_dbg_get_priv(dhd_pub_t *dhdp)
{
	if (!dhdp || !dhdp->dbg)
		return NULL;
	return dhdp->dbg->private;
}

/*
* dhd_dbg_start : start and stop All of Ring buffers
* Return: An error code or 0 on success.
*/
int
dhd_dbg_start(dhd_pub_t *dhdp, bool start)
{
	int ret = BCME_OK;
	int ring_id;
	eldbg_cmn_t *dbg;
	eldbg_ring_cmn_t *dbg_ring;
	if (!dhdp)
		return BCME_BADARG;
	dbg = dhdp->dbg;

	for (ring_id = DEBUG_RING_ID_INVALID + 1; ring_id < DEBUG_RING_ID_MAX; ring_id++) {
		dbg_ring = &dbg->dbg_rings[ring_id];
		if (!start) {
			if (VALID_RING(dbg_ring->id)) {
				eldbg_ring_start(dbg_ring);
			}
		}
	}
	return ret;
}

/*
 * dhd_dbg_send_urgent_evt: send the health check evt to Upper layer
 *
 * Return: An error code or 0 on success.
 */

int
dhd_dbg_send_urgent_evt(dhd_pub_t *dhdp, const void *data, const uint32 len)
{
	eldbg_cmn_t *dbg;
	int ret = BCME_OK;
	if (!dhdp || !dhdp->dbg)
		return BCME_BADADDR;

	dbg = dhdp->dbg;
	if (dbg->urgent_notifier) {
		dbg->urgent_notifier(dhdp, data, len);
	}
	return ret;
}

#if defined(DBG_PKT_MON) || defined(DHD_PKT_LOGGING)
uint32
__dhd_dbg_pkt_hash(uintptr_t pkt, uint32 pktid)
{
	uint32 __pkt;
	uint32 __pktid;

	__pkt = ((int)pkt) >= 0 ? (2 * pkt) : (-2 * pkt - 1);
	__pktid = ((int)pktid) >= 0 ? (2 * pktid) : (-2 * pktid - 1);

	return (__pkt >= __pktid ? (__pkt * __pkt + __pkt + __pktid) :
			(__pkt + __pktid * __pktid));
}

#define __TIMESPEC64_TO_US(ts) \
	(((ts).tv_sec * USEC_PER_SEC) + ((ts).tv_nsec / NSEC_PER_USEC))

uint32
__dhd_dbg_driver_ts_usec(void)
{
	struct timespec64 ts;

	ts = ktime_to_timespec64(ktime_get_boottime());
	return ((uint32)(__TIMESPEC64_TO_US(ts)));
}

wifi_tx_packet_fate
__dhd_dbg_map_tx_status_to_pkt_fate(uint16 status)
{
	wifi_tx_packet_fate pkt_fate;

	switch (status) {
		case WLFC_CTL_PKTFLAG_DISCARD:
			pkt_fate = TX_PKT_FATE_ACKED;
			break;
		case WLFC_CTL_PKTFLAG_D11SUPPRESS:
			/* intensional fall through */
		case WLFC_CTL_PKTFLAG_WLSUPPRESS:
			pkt_fate = TX_PKT_FATE_FW_QUEUED;
			break;
		case WLFC_CTL_PKTFLAG_TOSSED_BYWLC:
			pkt_fate = TX_PKT_FATE_FW_DROP_INVALID;
			break;
		case WLFC_CTL_PKTFLAG_DISCARD_NOACK:
			pkt_fate = TX_PKT_FATE_SENT;
			break;
		case WLFC_CTL_PKTFLAG_EXPIRED:
			pkt_fate = TX_PKT_FATE_FW_DROP_EXPTIME;
			break;
		case WLFC_CTL_PKTFLAG_DROPPED:
			pkt_fate = TX_PKT_FATE_DRV_DROP_OTHER;
			break;
		case WLFC_CTL_PKTFLAG_MKTFREE:
			pkt_fate = TX_PKT_FATE_FW_PKT_FREE;
			break;
		case WLFC_CTL_PKTFLAG_MAX_SUP_RETR:
			pkt_fate = TX_PKT_FATE_FW_MAX_SUP_RETR;
			break;
		case WLFC_CTL_PKTFLAG_FORCED_EXPIRED:
			pkt_fate = TX_PKT_FATE_FW_FORCED_EXPIRED;
			break;
		default:
			pkt_fate = TX_PKT_FATE_FW_DROP_OTHER;
			break;
	}

	return pkt_fate;
}
#endif /* DBG_PKT_MON || DHD_PKT_LOGGING */

#ifdef DBG_PKT_MON
static int do_iovar_aml_enable(dhd_pub_t *dhdp, uint val);
static void dhd_do_aml_disable(void *handle, void *event_info, u8 event);
void dhd_schedule_aml_disable(dhd_pub_t *dhdp);

static int
do_iovar_aml_enable(dhd_pub_t *dhdp, uint val)
{
	wl_aml_iovar_t *iov_in;
	wl_aml_iov_uint_data_t *subcmd;
	int buf_size, alloc_len, ret = BCME_OK;

	buf_size = OFFSETOF(wl_aml_iovar_t, data);
	alloc_len = buf_size + sizeof(wl_aml_iov_uint_data_t);

	iov_in = MALLOCZ(dhdp->osh, alloc_len);
	if (!iov_in) {
		DHD_ERROR(("%s: Error allocating %u bytes for aml enable\n",
			__FUNCTION__, alloc_len));
		return BCME_NOMEM;
	}

	iov_in->hdr.ver = htod16(WL_AML_IOV_VERSION_1_0);
	iov_in->hdr.len = htod16(alloc_len);
	iov_in->hdr.subcmd = htod16(WL_AML_SUBCMD_ENABLE);

	if (val & ~(1u << WL_AML_ASSOC_ENABLE | 1u << WL_AML_ROAM_ENABLE)) {
		ret = BCME_BADARG;
		goto fail;
	}

	subcmd = (wl_aml_iov_uint_data_t *)iov_in->data;
	subcmd->val = htod32(val);

	ret = dhd_iovar(dhdp, 0, "aml", (char *)iov_in, alloc_len, NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s aml failed %d\n", __FUNCTION__, ret));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	if (iov_in) {
		MFREE(dhdp->osh, iov_in, alloc_len);
	}

	return ret;
}

static void dhd_do_aml_disable(void *handle, void *event_info, u8 event)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;
	dhd_pub_t *dhdp = NULL;
	uint val = 0; /* Disabled */

	dhdp = &dhd->pub;
	if (!dhdp) {
		DHD_ERROR(("%s: dhdp is NULL\n", __FUNCTION__));
		return;
	}

	if (do_iovar_aml_enable(dhdp, val) == BCME_OK) {
		dhdp->aml_enable = FALSE;
	}

	return;
}

void dhd_schedule_aml_disable(dhd_pub_t *dhdp)
{
	if (dhdp->dbg->pkt_mon.tx_pkt_state == PKT_MON_STOPPED &&
			dhdp->dbg->pkt_mon.rx_pkt_state == PKT_MON_STOPPED) {
		DHD_PRINT(("%s: scheduling aml iovar..\n", __FUNCTION__));
		dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq, NULL,
			DHD_WQ_WORK_AML_IOVAR, dhd_do_aml_disable, DHD_WQ_WORK_PRIORITY_HIGH);
	}
}

static int
__dhd_dbg_free_tx_pkts(dhd_pub_t *dhdp, dhd_dbg_tx_info_t *tx_pkts,
	uint16 pkt_count)
{
	uint16 count;

	count = 0;
	while ((count < pkt_count) && tx_pkts) {
		if (tx_pkts->info.pkt) {
			PKTFREE(dhdp->osh, tx_pkts->info.pkt, TRUE);
		}
		tx_pkts++;
		count++;
	}

	return BCME_OK;
}

static int
__dhd_dbg_free_rx_pkts(dhd_pub_t *dhdp, dhd_dbg_rx_info_t *rx_pkts,
	uint16 pkt_count)
{
	uint16 count;

	count = 0;
	while ((count < pkt_count) && rx_pkts) {
		if (rx_pkts->info.pkt) {
			PKTFREE(dhdp->osh, rx_pkts->info.pkt, TRUE);
		}
		rx_pkts++;
		count++;
	}

	return BCME_OK;
}

void
__dhd_dbg_dump_pkt_info(dhd_pub_t *dhdp, dhd_dbg_pkt_info_t *info)
{
	if (DHD_PKT_MON_DUMP_ON()) {
		DHD_PKT_MON(("payload type   = %d\n", info->payload_type));
		DHD_PKT_MON(("driver ts      = %u\n", info->driver_ts));
		DHD_PKT_MON(("firmware ts    = %u\n", info->firmware_ts));
		DHD_PKT_MON(("packet hash    = %u\n", info->pkt_hash));
		DHD_PKT_MON(("packet length  = %zu\n", info->pkt_len));
		DHD_PKT_MON(("packet address = %p\n", info->pkt));
		DHD_PKT_MON(("packet data    = \n"));
		if (DHD_PKT_MON_ON()) {
			dhd_prhex(NULL, (volatile uchar *)PKTDATA(dhdp->osh, info->pkt),
				info->pkt_len, DHD_ERROR_VAL);
		}
	}
}

void
__dhd_dbg_dump_tx_pkt_info(dhd_pub_t *dhdp, dhd_dbg_tx_info_t *tx_pkt,
	uint16 count)
{
	if (DHD_PKT_MON_DUMP_ON()) {
		DHD_PKT_MON(("\nTX (count: %d)\n", ++count));
		DHD_PKT_MON(("packet fate    = %d\n", tx_pkt->fate));
		__dhd_dbg_dump_pkt_info(dhdp, &tx_pkt->info);
	}
}

void
__dhd_dbg_dump_rx_pkt_info(dhd_pub_t *dhdp, dhd_dbg_rx_info_t *rx_pkt,
	uint16 count)
{
	if (DHD_PKT_MON_DUMP_ON()) {
		DHD_PKT_MON(("\nRX (count: %d)\n", ++count));
		DHD_PKT_MON(("packet fate    = %d\n", rx_pkt->fate));
		__dhd_dbg_dump_pkt_info(dhdp, &rx_pkt->info);
	}
}

int
dhd_dbg_attach_pkt_monitor(dhd_pub_t *dhdp,
	dbg_mon_tx_pkts_t tx_pkt_mon,
	dbg_mon_tx_status_t tx_status_mon,
	dbg_mon_rx_pkts_t rx_pkt_mon)
{

	dhd_dbg_tx_report_t *tx_report = NULL;
	dhd_dbg_rx_report_t *rx_report = NULL;
	dhd_dbg_tx_info_t *tx_pkts = NULL;
	dhd_dbg_rx_info_t *rx_pkts = NULL;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	uint32 alloc_len;
	int ret = BCME_OK;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;

	if (PKT_MON_ATTACHED(tx_pkt_state) || PKT_MON_ATTACHED(tx_status_state) ||
			PKT_MON_ATTACHED(rx_pkt_state)) {
		DHD_PKT_MON(("%s(): packet monitor is already attached, "
			"tx_pkt_state=%d, tx_status_state=%d, rx_pkt_state=%d\n",
			__FUNCTION__, tx_pkt_state, tx_status_state, rx_pkt_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		/* return success as the intention was to initialize packet monitor */
		return BCME_OK;
	}

	/* allocate and initialize tx packet monitoring */
	alloc_len = sizeof(*tx_report);
	tx_report = (dhd_dbg_tx_report_t *)MALLOCZ(dhdp->osh, alloc_len);
	if (unlikely(!tx_report)) {
		DHD_ERROR(("%s(): could not allocate memory for - "
			"dhd_dbg_tx_report_t\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}

	alloc_len = (sizeof(*tx_pkts) * MAX_FATE_LOG_LEN);
	tx_pkts = (dhd_dbg_tx_info_t *)MALLOCZ(dhdp->osh, alloc_len);
	if (unlikely(!tx_pkts)) {
		DHD_ERROR(("%s(): could not allocate memory for - "
			"dhd_dbg_tx_info_t\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}
	dhdp->dbg->pkt_mon.tx_report = tx_report;
	dhdp->dbg->pkt_mon.tx_report->tx_pkts = tx_pkts;
	dhdp->dbg->pkt_mon.tx_pkt_mon = tx_pkt_mon;
	dhdp->dbg->pkt_mon.tx_status_mon = tx_status_mon;
	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_ATTACHED;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_ATTACHED;

	/* allocate and initialze rx packet monitoring */
	alloc_len = sizeof(*rx_report);
	rx_report = (dhd_dbg_rx_report_t *)MALLOCZ(dhdp->osh, alloc_len);
	if (unlikely(!rx_report)) {
		DHD_ERROR(("%s(): could not allocate memory for - "
			"dhd_dbg_rx_report_t\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}

	alloc_len = (sizeof(*rx_pkts) * MAX_FATE_LOG_LEN);
	rx_pkts = (dhd_dbg_rx_info_t *)MALLOCZ(dhdp->osh, alloc_len);
	if (unlikely(!rx_pkts)) {
		DHD_ERROR(("%s(): could not allocate memory for - "
			"dhd_dbg_rx_info_t\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}
	dhdp->dbg->pkt_mon.rx_report = rx_report;
	dhdp->dbg->pkt_mon.rx_report->rx_pkts = rx_pkts;
	dhdp->dbg->pkt_mon.rx_pkt_mon = rx_pkt_mon;
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_ATTACHED;

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	DHD_PKT_MON(("%s(): packet monitor attach succeeded\n", __FUNCTION__));
	return ret;

fail:
	/* tx packet monitoring */
	if (tx_pkts) {
		alloc_len = (sizeof(*tx_pkts) * MAX_FATE_LOG_LEN);
		MFREE(dhdp->osh, tx_pkts, alloc_len);
	}
	if (tx_report) {
		alloc_len = sizeof(*tx_report);
		MFREE(dhdp->osh, tx_report, alloc_len);
	}
	dhdp->dbg->pkt_mon.tx_report = NULL;
	dhdp->dbg->pkt_mon.tx_report->tx_pkts = NULL;
	dhdp->dbg->pkt_mon.tx_pkt_mon = NULL;
	dhdp->dbg->pkt_mon.tx_status_mon = NULL;
	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_DETACHED;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_DETACHED;

	/* rx packet monitoring */
	if (rx_pkts) {
		alloc_len = (sizeof(*rx_pkts) * MAX_FATE_LOG_LEN);
		MFREE(dhdp->osh, rx_pkts, alloc_len);
	}
	if (rx_report) {
		alloc_len = sizeof(*rx_report);
		MFREE(dhdp->osh, rx_report, alloc_len);
	}
	dhdp->dbg->pkt_mon.rx_report = NULL;
	dhdp->dbg->pkt_mon.rx_report->rx_pkts = NULL;
	dhdp->dbg->pkt_mon.rx_pkt_mon = NULL;
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_DETACHED;

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	DHD_ERROR(("%s(): packet monitor attach failed\n", __FUNCTION__));
	return ret;
}

int
dhd_dbg_start_pkt_monitor(dhd_pub_t *dhdp)
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_rx_report_t *rx_report;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	if (do_iovar_aml_enable(dhdp, 1) == BCME_OK) {
		dhdp->aml_enable = TRUE;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;

	if (PKT_MON_DETACHED(tx_pkt_state) || PKT_MON_DETACHED(tx_status_state) ||
			PKT_MON_DETACHED(rx_pkt_state)) {
		DHD_PKT_MON(("%s(): packet monitor is not yet enabled, "
			"tx_pkt_state=%d, tx_status_state=%d, rx_pkt_state=%d\n",
			__FUNCTION__, tx_pkt_state, tx_status_state, rx_pkt_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}

	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_STARTING;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_STARTING;
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_STARTING;

	tx_report = dhdp->dbg->pkt_mon.tx_report;
	rx_report = dhdp->dbg->pkt_mon.rx_report;
	if (!tx_report || !rx_report) {
		DHD_PKT_MON(("%s(): tx_report=%p, rx_report=%p\n",
			__FUNCTION__, tx_report, rx_report));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}

	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;

	/* Safe to free packets as state pkt_state is STARTING */
	__dhd_dbg_free_tx_pkts(dhdp, tx_report->tx_pkts, tx_report->pkt_pos);

	__dhd_dbg_free_rx_pkts(dhdp, rx_report->rx_pkts, rx_report->pkt_pos);

	/* reset array postion */
	tx_report->pkt_pos = 0;
	tx_report->status_pos = 0;
	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_STARTED;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_STARTED;

	rx_report->pkt_pos = 0;
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_STARTED;
	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);

	DHD_PKT_MON(("%s(): packet monitor started\n", __FUNCTION__));
	return BCME_OK;
}

int
dhd_dbg_monitor_tx_pkts(dhd_pub_t *dhdp, void *pkt, uint32 pktid, frame_type type, uint8 mgmt_acked)
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_tx_info_t *tx_pkts;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	uint32 pkt_hash, driver_ts;
	uint16 pkt_pos;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	if (PKT_MON_STARTED(tx_pkt_state)) {
		tx_report = dhdp->dbg->pkt_mon.tx_report;
		pkt_pos = tx_report->pkt_pos;

		if (!PKT_MON_PKT_FULL(pkt_pos)) {
			tx_pkts = tx_report->tx_pkts;
			pkt_hash = __dhd_dbg_pkt_hash((uintptr_t)pkt, pktid);
			driver_ts = __dhd_dbg_driver_ts_usec();

			if (type == FRAME_TYPE_80211_MGMT) {
				tx_pkts[pkt_pos].info.pkt = pkt;
				if (mgmt_acked) {
					tx_pkts[pkt_pos].fate = TX_PKT_FATE_ACKED;
				} else {
					tx_pkts[pkt_pos].fate = TX_PKT_FATE_SENT;
				}
			} else {
				tx_pkts[pkt_pos].info.pkt = PKTDUP(dhdp->osh, pkt);
				tx_pkts[pkt_pos].fate = TX_PKT_FATE_DRV_QUEUED;
			}

			tx_pkts[pkt_pos].info.pkt_len = PKTLEN(dhdp->osh, pkt);
			tx_pkts[pkt_pos].info.pkt_hash = pkt_hash;
			tx_pkts[pkt_pos].info.driver_ts = driver_ts;
			tx_pkts[pkt_pos].info.firmware_ts = 0U;
			tx_pkts[pkt_pos].info.payload_type = type;

			tx_report->pkt_pos++;
			/* TX mgmt packet is updated with the final fate reason,
			 * so status_pos should be also increased.
			 */
			if (type == FRAME_TYPE_80211_MGMT) {
				tx_report->status_pos++;
			}
		} else {
			if (type == FRAME_TYPE_80211_MGMT) {
				PKTFREE(dhdp->osh, pkt, TRUE);
			}
			dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_STOPPED;
			DHD_PKT_MON(("%s(): tx pkt logging stopped, reached "
				"max limit\n", __FUNCTION__));
			if (dhdp->aml_enable) {
				dhd_schedule_aml_disable(dhdp);
			}
		}
	}

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	return BCME_OK;
}

int
dhd_dbg_monitor_tx_status(dhd_pub_t *dhdp, void *pkt, uint32 pktid,
		uint16 status)
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_tx_info_t *tx_pkt;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	wifi_tx_packet_fate pkt_fate;
	uint32 pkt_hash, temp_hash;
	uint16 pkt_pos, status_pos;
	int16 count;
	bool found = FALSE;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	if (PKT_MON_STARTED(tx_status_state)) {
		tx_report = dhdp->dbg->pkt_mon.tx_report;
		pkt_pos = tx_report->pkt_pos;
		status_pos = tx_report->status_pos;

		if (!PKT_MON_STATUS_FULL(pkt_pos, status_pos)) {
			pkt_hash = __dhd_dbg_pkt_hash((uintptr_t)pkt, pktid);
			pkt_fate = __dhd_dbg_map_tx_status_to_pkt_fate(status);

			/* best bet (in-order tx completion) */
			count = status_pos;
			tx_pkt = (((dhd_dbg_tx_info_t *)tx_report->tx_pkts) + status_pos);
			while ((count < pkt_pos) && tx_pkt) {
				temp_hash = tx_pkt->info.pkt_hash;
				if (temp_hash == pkt_hash) {
					tx_pkt->fate = pkt_fate;
					tx_report->status_pos++;
					found = TRUE;
					break;
				}
				tx_pkt++;
				count++;
			}

			/* search until beginning (handles out-of-order completion) */
			if (!found) {
				count = status_pos - 1;
				tx_pkt = (((dhd_dbg_tx_info_t *)tx_report->tx_pkts) + count);
				while ((count >= 0) && tx_pkt) {
					temp_hash = tx_pkt->info.pkt_hash;
					if (temp_hash == pkt_hash) {
						tx_pkt->fate = pkt_fate;
						tx_report->status_pos++;
						found = TRUE;
						break;
					}
					tx_pkt--;
					count--;
				}

				if (!found) {
					/* still couldn't match tx_status */
					DHD_INFO(("%s(): couldn't match tx_status, pkt_pos=%u, "
						"status_pos=%u, pkt_fate=%u\n", __FUNCTION__,
						pkt_pos, status_pos, pkt_fate));
				}
			}
		} else {
			dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_STOPPED;
			DHD_PKT_MON(("%s(): tx_status logging stopped, reached "
				"max limit\n", __FUNCTION__));
		}
	}

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	return BCME_OK;
}

int
dhd_dbg_monitor_rx_pkts(dhd_pub_t *dhdp, void *pkt, frame_type type)
{
	dhd_dbg_rx_report_t *rx_report;
	dhd_dbg_rx_info_t *rx_pkts;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	uint32 driver_ts;
	uint16 pkt_pos;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;
	if (PKT_MON_STARTED(rx_pkt_state)) {
		rx_report = dhdp->dbg->pkt_mon.rx_report;
		pkt_pos = rx_report->pkt_pos;

		if (!PKT_MON_PKT_FULL(pkt_pos)) {
			rx_pkts = rx_report->rx_pkts;
			driver_ts = __dhd_dbg_driver_ts_usec();

			if (type == FRAME_TYPE_80211_MGMT) {
				rx_pkts[pkt_pos].info.pkt = pkt;
			} else {
				rx_pkts[pkt_pos].info.pkt = PKTDUP(dhdp->osh, pkt);
			}
			rx_pkts[pkt_pos].info.pkt_len = PKTLEN(dhdp->osh, pkt);
			rx_pkts[pkt_pos].info.pkt_hash = 0U;
			rx_pkts[pkt_pos].info.driver_ts = driver_ts;
			rx_pkts[pkt_pos].info.firmware_ts = 0U;
			rx_pkts[pkt_pos].info.payload_type = type;
			rx_pkts[pkt_pos].fate = RX_PKT_FATE_SUCCESS;

			rx_report->pkt_pos++;
		} else {
			if (type == FRAME_TYPE_80211_MGMT) {
				PKTFREE(dhdp->osh, pkt, TRUE);
			}
			dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_STOPPED;
			DHD_PKT_MON(("%s(): rx pkt logging stopped, reached "
					"max limit\n", __FUNCTION__));
			if (dhdp->aml_enable) {
				dhd_schedule_aml_disable(dhdp);
			}
		}
	}

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	return BCME_OK;
}

int
dhd_dbg_stop_pkt_monitor(dhd_pub_t *dhdp)
{
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;

	if (PKT_MON_DETACHED(tx_pkt_state) || PKT_MON_DETACHED(tx_status_state) ||
			PKT_MON_DETACHED(rx_pkt_state)) {
		DHD_PKT_MON(("%s(): packet monitor is not yet enabled, "
			"tx_pkt_state=%d, tx_status_state=%d, rx_pkt_state=%d\n",
			__FUNCTION__, tx_pkt_state, tx_status_state, rx_pkt_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}
	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_STOPPED;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_STOPPED;
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_STOPPED;
	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);

	DHD_PKT_MON(("%s(): packet monitor stopped\n", __FUNCTION__));
	return BCME_OK;
}

#define __COPY_TO_USER(to, from, n) \
	do { \
		int __ret; \
		__ret = copy_to_user((void __user *)(to), (void *)(from), \
				(unsigned long)(n)); \
		if (unlikely(__ret)) { \
			DHD_ERROR(("%s():%d: copy_to_user failed, ret=%d\n", \
				__FUNCTION__, __LINE__, __ret)); \
			return __ret; \
		} \
	} while (0);

static wifi_tx_packet_fate
__dhd_dbg_convert_fate(wifi_tx_packet_fate fate)
{
	wifi_tx_packet_fate new_fate = fate;

	 /* To prevent SIG-ABORT, packet_fate > TX_PKT_FATE_DRV_DROP_OTHER */
	if (fate > TX_PKT_FATE_DRV_DROP_OTHER) {
		new_fate = TX_PKT_FATE_FW_DROP_OTHER;
	}

	return new_fate;
}

int
dhd_dbg_monitor_get_tx_pkts(dhd_pub_t *dhdp, void __user *user_buf,
		uint16 req_count, uint16 *resp_count)
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_tx_info_t *tx_pkt;
	wifi_tx_report_t *ptr;
	compat_wifi_tx_report_t *cptr;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	uint16 pkt_count, count;
	unsigned long flags;

	BCM_REFERENCE(ptr);
	BCM_REFERENCE(cptr);

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	if (!PKT_MON_ATTACHED(tx_pkt_state) ||
			!PKT_MON_ATTACHED(tx_status_state)) {
		DHD_PKT_MON(("%s(): packet monitor is not yet enabled, "
			"tx_pkt_state=%d, tx_status_state=%d\n", __FUNCTION__,
			tx_pkt_state, tx_status_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}

	count = 0;
	tx_report = dhdp->dbg->pkt_mon.tx_report;
	tx_pkt = tx_report->tx_pkts;
	pkt_count = MIN(req_count, tx_report->status_pos);
	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);

#ifdef CONFIG_COMPAT
	if (is_compat_task()) {
		cptr = (compat_wifi_tx_report_t *)user_buf;
		while ((count < pkt_count) && tx_pkt && cptr) {
			compat_wifi_tx_report_t *comp_ptr = compat_ptr((uintptr_t) cptr);
			compat_dhd_dbg_pkt_info_t compat_tx_pkt;
			__dhd_dbg_dump_tx_pkt_info(dhdp, tx_pkt, count);
			/* fate convert asscording to wifi_logger.h */
			{
				wifi_tx_packet_fate new_fate = tx_pkt->fate;
				new_fate = __dhd_dbg_convert_fate(new_fate);
				__COPY_TO_USER(&comp_ptr->fate, &new_fate, sizeof(new_fate));
			}

			compat_tx_pkt.payload_type = tx_pkt->info.payload_type;
			compat_tx_pkt.pkt_len = tx_pkt->info.pkt_len;
			compat_tx_pkt.driver_ts = tx_pkt->info.driver_ts;
			compat_tx_pkt.firmware_ts = tx_pkt->info.firmware_ts;
			compat_tx_pkt.pkt_hash = tx_pkt->info.pkt_hash;
			__COPY_TO_USER(&comp_ptr->frame_inf.payload_type,
				&compat_tx_pkt.payload_type,
				OFFSETOF(compat_dhd_dbg_pkt_info_t, pkt_hash));
			__COPY_TO_USER(comp_ptr->frame_inf.frame_content.ethernet_ii,
				PKTDATA(dhdp->osh, tx_pkt->info.pkt), tx_pkt->info.pkt_len);

			cptr++;
			tx_pkt++;
			count++;
		}
	} else
#endif /* CONFIG_COMPAT */
	{
		ptr = (wifi_tx_report_t *)user_buf;
		while ((count < pkt_count) && tx_pkt && ptr) {
			__dhd_dbg_dump_tx_pkt_info(dhdp, tx_pkt, count);
			/* fate convert asscording to wifi_logger.h */
			{
				wifi_tx_packet_fate new_fate = tx_pkt->fate;
				new_fate = __dhd_dbg_convert_fate(new_fate);
				__COPY_TO_USER(&ptr->fate, &new_fate, sizeof(new_fate));
			}
			__COPY_TO_USER(&ptr->frame_inf.payload_type,
				&tx_pkt->info.payload_type,
				OFFSETOF(dhd_dbg_pkt_info_t, pkt_hash));
			__COPY_TO_USER(ptr->frame_inf.frame_content.ethernet_ii,
				PKTDATA(dhdp->osh, tx_pkt->info.pkt), tx_pkt->info.pkt_len);

			ptr++;
			tx_pkt++;
			count++;
		}
	}
	*resp_count = pkt_count;

	if (!pkt_count) {
		DHD_ERROR(("%s(): no tx_status in tx completion messages, "
			"make sure that 'd11status' is enabled in firmware, "
			"status_pos=%u\n", __FUNCTION__, pkt_count));
	}

	return BCME_OK;
}

int
dhd_dbg_monitor_get_rx_pkts(dhd_pub_t *dhdp, void __user *user_buf,
		uint16 req_count, uint16 *resp_count)
{
	dhd_dbg_rx_report_t *rx_report;
	dhd_dbg_rx_info_t *rx_pkt;
	wifi_rx_report_t *ptr;
	compat_wifi_rx_report_t *cptr;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	uint16 pkt_count, count;
	unsigned long flags;

	BCM_REFERENCE(ptr);
	BCM_REFERENCE(cptr);

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;
	if (!PKT_MON_ATTACHED(rx_pkt_state)) {
		DHD_PKT_MON(("%s(): packet fetch is not allowed , "
			"rx_pkt_state=%d\n", __FUNCTION__, rx_pkt_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}

	count = 0;
	rx_report = dhdp->dbg->pkt_mon.rx_report;
	rx_pkt = rx_report->rx_pkts;
	pkt_count = MIN(req_count, rx_report->pkt_pos);
	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);

#ifdef CONFIG_COMPAT
	if (is_compat_task()) {
		cptr = (compat_wifi_rx_report_t *)user_buf;
		while ((count < pkt_count) && rx_pkt && cptr) {
			compat_wifi_rx_report_t *comp_ptr = compat_ptr((uintptr_t) cptr);
			compat_dhd_dbg_pkt_info_t compat_rx_pkt;
			__dhd_dbg_dump_rx_pkt_info(dhdp, rx_pkt, count);
			__COPY_TO_USER(&comp_ptr->fate, &rx_pkt->fate, sizeof(rx_pkt->fate));

			compat_rx_pkt.payload_type = rx_pkt->info.payload_type;
			compat_rx_pkt.pkt_len = rx_pkt->info.pkt_len;
			compat_rx_pkt.driver_ts = rx_pkt->info.driver_ts;
			compat_rx_pkt.firmware_ts = rx_pkt->info.firmware_ts;
			compat_rx_pkt.pkt_hash = rx_pkt->info.pkt_hash;
			__COPY_TO_USER(&comp_ptr->frame_inf.payload_type,
				&compat_rx_pkt.payload_type,
				OFFSETOF(compat_dhd_dbg_pkt_info_t, pkt_hash));
			__COPY_TO_USER(comp_ptr->frame_inf.frame_content.ethernet_ii,
				PKTDATA(dhdp->osh, rx_pkt->info.pkt), rx_pkt->info.pkt_len);

			cptr++;
			rx_pkt++;
			count++;
		}
	} else
#endif /* CONFIG_COMPAT */
	{
		ptr = (wifi_rx_report_t *)user_buf;
		while ((count < pkt_count) && rx_pkt && ptr) {
			__dhd_dbg_dump_rx_pkt_info(dhdp, rx_pkt, count);
			__COPY_TO_USER(&ptr->fate, &rx_pkt->fate, sizeof(rx_pkt->fate));
			__COPY_TO_USER(&ptr->frame_inf.payload_type,
				&rx_pkt->info.payload_type,
				OFFSETOF(dhd_dbg_pkt_info_t, pkt_hash));
			__COPY_TO_USER(ptr->frame_inf.frame_content.ethernet_ii,
				PKTDATA(dhdp->osh, rx_pkt->info.pkt), rx_pkt->info.pkt_len);

			ptr++;
			rx_pkt++;
			count++;
		}
	}

	*resp_count = pkt_count;

	return BCME_OK;
}

int
dhd_dbg_detach_pkt_monitor(dhd_pub_t *dhdp)
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_rx_report_t *rx_report;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;
	unsigned long flags;

	if (!dhdp || !dhdp->dbg) {
		DHD_PKT_MON(("%s(): dhdp=%p, dhdp->dbg=%p\n", __FUNCTION__,
			dhdp, (dhdp ? dhdp->dbg : NULL)));
		return -EINVAL;
	}

	DHD_PKT_MON_LOCK(dhdp->dbg->pkt_mon_lock, flags);
	tx_pkt_state = dhdp->dbg->pkt_mon.tx_pkt_state;
	tx_status_state = dhdp->dbg->pkt_mon.tx_status_state;
	rx_pkt_state = dhdp->dbg->pkt_mon.rx_pkt_state;

	if (PKT_MON_DETACHED(tx_pkt_state) || PKT_MON_DETACHED(tx_status_state) ||
			PKT_MON_DETACHED(rx_pkt_state)) {
		DHD_PKT_MON(("%s(): packet monitor is already detached, "
			"tx_pkt_state=%d, tx_status_state=%d, rx_pkt_state=%d\n",
			__FUNCTION__, tx_pkt_state, tx_status_state, rx_pkt_state));
		DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
		return -EINVAL;
	}

	tx_report = dhdp->dbg->pkt_mon.tx_report;
	rx_report = dhdp->dbg->pkt_mon.rx_report;

	/* free and de-initalize tx packet monitoring */
	dhdp->dbg->pkt_mon.tx_pkt_state = PKT_MON_DETACHED;
	dhdp->dbg->pkt_mon.tx_status_state = PKT_MON_DETACHED;
	if (tx_report) {
		if (tx_report->tx_pkts) {
			__dhd_dbg_free_tx_pkts(dhdp, tx_report->tx_pkts,
				tx_report->pkt_pos);
			MFREE(dhdp->osh, tx_report->tx_pkts,
				(sizeof(*tx_report->tx_pkts) * MAX_FATE_LOG_LEN));
			dhdp->dbg->pkt_mon.tx_report->tx_pkts = NULL;
		}
		MFREE(dhdp->osh, tx_report, sizeof(*tx_report));
		dhdp->dbg->pkt_mon.tx_report = NULL;
	}
	dhdp->dbg->pkt_mon.tx_pkt_mon = NULL;
	dhdp->dbg->pkt_mon.tx_status_mon = NULL;

	/* free and de-initalize rx packet monitoring */
	dhdp->dbg->pkt_mon.rx_pkt_state = PKT_MON_DETACHED;
	if (rx_report) {
		if (rx_report->rx_pkts) {
			__dhd_dbg_free_rx_pkts(dhdp, rx_report->rx_pkts,
				rx_report->pkt_pos);
			MFREE(dhdp->osh, rx_report->rx_pkts,
				(sizeof(*rx_report->rx_pkts) * MAX_FATE_LOG_LEN));
			dhdp->dbg->pkt_mon.rx_report->rx_pkts = NULL;
		}
		MFREE(dhdp->osh, rx_report, sizeof(*rx_report));
		dhdp->dbg->pkt_mon.rx_report = NULL;
	}
	dhdp->dbg->pkt_mon.rx_pkt_mon = NULL;

	DHD_PKT_MON_UNLOCK(dhdp->dbg->pkt_mon_lock, flags);
	DHD_PKT_MON(("%s(): packet monitor detach succeeded\n", __FUNCTION__));
	return BCME_OK;
}
#endif /* DBG_PKT_MON */

#if defined(DBG_PKT_MON) || defined(DHD_PKT_LOGGING)
/*
 * WAR: Because of the overloading by DMA marker field,
 * tx_status in TX completion message cannot be used. As a WAR,
 * send d11 tx_status through unused status field of PCIe
 * completion header.
 */
bool
dhd_dbg_process_tx_status(dhd_pub_t *dhdp, void *pkt, uint32 pktid,
		uint16 status)
{
	bool pkt_fate = TRUE;
	if (dhdp->d11_tx_status) {
		pkt_fate = (status == WLFC_CTL_PKTFLAG_DISCARD) ? TRUE : FALSE;
		DHD_DBG_PKT_MON_TX_STATUS(dhdp, pkt, pktid, status);
	}
	return pkt_fate;
}
#else /* DBG_PKT_MON || DHD_PKT_LOGGING */
bool
dhd_dbg_process_tx_status(dhd_pub_t *dhdp, void *pkt,
		uint32 pktid, uint16 status)
{
	return TRUE;
}
#endif /* DBG_PKT_MON || DHD_PKT_LOGGING */

#define	EL_LOG_STR_LEN	512

#define PRINT_CHN_PER_LINE 8
#define PRINT_CHAN_LINE(cnt) \
{\
	cnt ++; \
	if (cnt >= PRINT_CHN_PER_LINE) { \
		DHD_PRINT(("%s\n", b.origbuf)); \
		bcm_binit(&b, pr_buf, EL_LOG_STR_LEN); \
		bcm_bprintf(&b, "%s: ", prefix); \
		cnt = 0; \
	} \
}

/*
 * dhd_dbg_attach: initialziation of dhd dbugability module
 *
 * Return: An error code or 0 on success.
 */
#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
struct dhd_dbg_ring_buf g_ring_buf;
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */
int
dhd_dbg_attach(dhd_pub_t *dhdp, dbg_pullreq_t os_pullreq,
	dbg_urgent_noti_t os_urgent_notifier, void *os_priv)
{
	eldbg_cmn_t *dbg = NULL;
	int ret = BCME_ERROR;
#if  defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	eldbg_ring_cmn_t *ring = NULL;
	int ring_id = 0;
	void *buf = NULL;
	BCM_REFERENCE(ring);
	BCM_REFERENCE(ring_id);
	BCM_REFERENCE(buf);
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/
#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
	struct dhd_dbg_ring_buf *ring_buf;
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */
#ifdef DHD_PKT_LOGGING_DBGRING
	dhd_pktlog_ring_t *pktlog_ring = NULL;
#endif /* DHD_PKT_LOGGING_DBGRING */

	dbg = VMALLOCZ(dhdp->osh, sizeof(eldbg_cmn_t));
	if (!dbg) {
		DHD_ERROR(("%s:%d: VMALLOC failed for dbg, size %d\n",
			__FUNCTION__, __LINE__, (uint32)sizeof(eldbg_cmn_t)));
		return BCME_NOMEM;
	}

#ifdef DHD_DEBUGABILITY_EVENT_RING
	buf = VMALLOCZ(dhdp->osh, DHD_EVENT_RING_SIZE);
	if (!buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for event_ring, size %d\n",
			__FUNCTION__, __LINE__, DHD_EVENT_RING_SIZE));
		ret = BCME_NOMEM;
		goto error;
	}
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[DHD_EVENT_RING_ID],
		DHD_EVENT_RING_ID, (uint8 *)DHD_EVENT_RING_NAME,
		DHD_EVENT_RING_SIZE, buf, FALSE);
	if (ret) {
		goto error;
	}
#endif /* DHD_DEBUGABILITY_EVENT_RING */

#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(DEBUGABILITY)
	buf = VMALLOCZ(dhdp->osh, FW_VERBOSE_RING_SIZE);
	if (!buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for fw_verbose_ring, size %d\n",
			__FUNCTION__, __LINE__, FW_VERBOSE_RING_SIZE));
		ret = BCME_NOMEM;
		goto error;
	}
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[FW_VERBOSE_RING_ID],
		FW_VERBOSE_RING_ID, (uint8 *)FW_VERBOSE_RING_NAME,
		FW_VERBOSE_RING_SIZE, buf, TRUE);

	if (ret) {
		goto error;
	}
#endif /* EVENT_LOG_ACCESS && DHD_DEBUGABILITY_LOG_DUMP_RING || DEBUGABILITY  */

#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
	buf = VMALLOCZ(dhdp->osh, DRIVER_LOG_RING_SIZE);
	if (!buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for driver_log_ring, size %d\n",
			__FUNCTION__, __LINE__, DRIVER_LOG_RING_SIZE));
		ret = BCME_NOMEM;
		goto error;
	}
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[DRIVER_LOG_RING_ID],
		DRIVER_LOG_RING_ID, (uint8 *)DRIVER_LOG_RING_NAME,
		DRIVER_LOG_RING_SIZE, buf, FALSE);
	if (ret) {
		goto error;
	}

	buf = VMALLOCZ(dhdp->osh, ROAM_STATS_RING_SIZE);
	if (!buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for roam_stats_ring, size %d\n",
			__FUNCTION__, __LINE__, ROAM_STATS_RING_SIZE));
		ret = BCME_NOMEM;
		goto error;
	}
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[ROAM_STATS_RING_ID],
		ROAM_STATS_RING_ID, (uint8 *)ROAM_STATS_RING_NAME,
		ROAM_STATS_RING_SIZE, buf, FALSE);
	if (ret) {
		goto error;
	}
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */

#if defined(DHD_DEBUGABILITY_DEBUG_DUMP) || defined(DHD_HAL_RING_DUMP)
	/*
	 * delayed memory allocation. memory will be allocated when debug_dump is invoked
	 * To prepare the ringbuffer in legacy HAL, we should initialize ring at this time
	 */
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[DEBUG_DUMP_RING1_ID],
		DEBUG_DUMP_RING1_ID, (uint8 *)DEBUG_DUMP_RING1_NAME,
		DEBUG_DUMP_RING1_SIZE, NULL, FALSE);
	if (ret) {
		DHD_ERROR(("%s: Failed to init debug ring1\n", __func__));
		goto error;
	}

	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[DEBUG_DUMP_RING2_ID],
		DEBUG_DUMP_RING2_ID, (uint8 *)DEBUG_DUMP_RING2_NAME,
		DEBUG_DUMP_RING2_SIZE, NULL, FALSE);
	if (ret) {
		DHD_ERROR(("%s: Failed to init debug ring2\n", __func__));
		goto error;
	}
#endif /* defined(DHD_DEBUGABILITY_DEBUG_DUMP) || defined(DHD_HAL_RING_DUMP) */

#if defined(DHD_HAL_RING_DUMP_MEMDUMP)
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[MEM_DUMP_RING_ID], MEM_DUMP_RING_ID,
			(uint8 *)MEM_DUMP_RING_NAME, MEM_DUMP_RING_SIZE, NULL, FALSE);
	if (ret) {
		DHD_ERROR(("%s: Failed to init mem dump ring\n", __func__));
		goto error;
	}
#endif /* DHD_HAL_RING_DUMP_MEMDUMP */

#ifdef BTLOG
	buf = VMALLOCZ(dhdp->osh, BT_LOG_RING_SIZE);
	if (!buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for bt_log_ring, size %d\n",
			__FUNCTION__, __LINE__, BT_LOG_RING_SIZE));
		ret = BCME_NOMEM;
		goto error;
	}
	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[BT_LOG_RING_ID],
		BT_LOG_RING_ID, (uint8 *)BT_LOG_RING_NAME,
		BT_LOG_RING_SIZE, buf, FALSE);
	if (ret) {
		goto error;
	}
#endif	/* BTLOG */
#ifdef DHD_PKT_LOGGING_DBGRING
	if (dhdp && dhdp->pktlog && dhdp->pktlog->pktlog_ring) {
		pktlog_ring = dhdp->pktlog->pktlog_ring;
	}

	if (!pktlog_ring) {
		DHD_ERROR(("%s: pktlog_ring is NULL. return.\n", __FUNCTION__));
		ret = BCME_NOMEM;
		goto error;
	}

	buf = pktlog_ring->ring_info_mem;
	if (!buf) {
		DHD_ERROR(("%s: ring_info_mem is NULL. return.\n", __FUNCTION__));
		ret = BCME_NOMEM;
		goto error;
	}

	ret = eldbg_ring_init(dhdp->osh, &dbg->dbg_rings[PACKET_LOG_RING_ID],
			PACKET_LOG_RING_ID, (uint8 *)DHD_PACKET_LOG_RING_NAME,
			DHD_PACKET_LOG_RING_SIZE, buf, FALSE);
	if (ret) {
		goto error;
	}

	pktlog_ring->dbg_ring = &dbg->dbg_rings[PACKET_LOG_RING_ID];
#endif /* DHD_PKT_LOGGING_DBGRING */

#ifdef MOBILITY_LOG_TRACE
	dbg->wrapper_buf.buf = VMALLOCZ(dhdp->osh, DHD_PCIE_WRAPPER_LEN);
	if (!dbg->wrapper_buf.buf) {
		DHD_ERROR(("%s:%d: VMALLOC failed for wrapper_buf, size %d\n",
			__FUNCTION__, __LINE__, DHD_PCIE_WRAPPER_LEN));
		ret = BCME_NOMEM;
		goto error;
	}
	dbg->wrapper_buf.len = DHD_PCIE_WRAPPER_LEN;

	dbg->private = os_priv;
	dbg->pullreq = os_pullreq;
	dbg->urgent_notifier = os_urgent_notifier;
#endif /* MOBILITY_LOG_TRACE */

	dhdp->dbg = dbg;
#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
	ring_buf = &g_ring_buf;
	ring_buf->dhd_pub = dhdp;
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */

	return BCME_OK;

error:
#ifdef MOBILITY_LOG_TRACE
	if (dbg->wrapper_buf.buf) {
		dbg->wrapper_buf.len = 0;
		VMFREE(dhdp->osh, dbg->wrapper_buf.buf, DHD_PCIE_WRAPPER_LEN);
	}
#endif /* MOBILITY_LOG_TRACE */

#if defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || defined(EVENT_LOG_ACCESS) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	for (ring_id = DEBUG_RING_ID_INVALID + 1; ring_id < DEBUG_RING_ID_MAX; ring_id++) {
		if (VALID_RING(dbg->dbg_rings[ring_id].id)) {
			ring = &dbg->dbg_rings[ring_id];
			eldbg_ring_deinit(dhdp->osh, ring);
			if (ring->ring_buf) {
#ifdef DHD_PKT_LOGGING_DBGRING
				if (ring_id != PACKET_LOG_RING_ID)
#endif /* DHD_PKT_LOGGING_DBGRING */
				{
					VMFREE(dhdp->osh, ring->ring_buf, ring->ring_size);
				}
				ring->ring_buf = NULL;
			}
			ring->ring_size = 0;
		}
	}
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING || EVENT_LOG_ACCESS || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/

	VMFREE(dhdp->osh, dbg, sizeof(eldbg_cmn_t));

	return ret;
}

/*
 * dhd_dbg_detach: clean up dhd dbugability module
 */
void
dhd_dbg_detach(dhd_pub_t *dhdp)
{
	eldbg_cmn_t *dbg;
#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	int ring_id;
	eldbg_ring_cmn_t *ring = NULL;
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/

	dbg = dhdp->dbg;
	if (!dbg) {
		return;
	}

#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	for (ring_id = DEBUG_RING_ID_INVALID + 1; ring_id < DEBUG_RING_ID_MAX; ring_id++) {
		if (VALID_RING(dbg->dbg_rings[ring_id].id)) {
			ring = &dbg->dbg_rings[ring_id];
			eldbg_ring_deinit(dhdp->osh, ring);
			if (ring->ring_buf) {
#ifdef DHD_PKT_LOGGING_DBGRING
				if (ring_id != PACKET_LOG_RING_ID)
#endif /* DHD_PKT_LOGGING_DBGRING */
				{
					VMFREE(dhdp->osh, ring->ring_buf, ring->ring_size);
				}
				ring->ring_buf = NULL;
			}
			ring->ring_size = 0;
		}
	}
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/
#ifdef MOBILITY_LOG_TRACE
	if (dbg->wrapper_buf.buf) {
		dbg->wrapper_buf.len = 0;
		VMFREE(dhdp->osh, dbg->wrapper_buf.buf, DHD_PCIE_WRAPPER_LEN);
	}
#endif /* MOBILITY_LOG_TRACE */
	VMFREE(dhdp->osh, dbg, sizeof(eldbg_cmn_t));

#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
	g_ring_buf.dhd_pub = NULL;
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */
}

uint32
dhd_dbg_get_fwverbose(dhd_pub_t *dhdp)
{
	if (dhdp && dhdp->dbg) {
		return dhdp->dbg->dbg_rings[FW_VERBOSE_RING_ID].log_level;
	}
	return 0;
}

void
dhd_dbg_set_fwverbose(dhd_pub_t *dhdp, uint32 new_val)
{

	if (dhdp && dhdp->dbg) {
		dhdp->dbg->dbg_rings[FW_VERBOSE_RING_ID].log_level = new_val;
	}
}

#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
void
dhd_dbg_ring_write(int type, char *binary_data,
		int binary_len, const char *fmt, ...)
{
	int len = 0;
	va_list args;
	struct dhd_dbg_ring_buf *ring_buf = NULL;
	char tmp_buf[DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE] = {0, };

	ring_buf = &g_ring_buf;

	va_start(args, fmt);
	len = vsnprintf(tmp_buf, DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE, fmt, args);
	/* Non ANSI C99 compliant returns -1,
	 * ANSI compliant return len >= DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE
	 */
	va_end(args);
	if (len < 0) {
		return;
	}

	if (len >= DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE) {
		len = DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE - 1;
		tmp_buf[len] = '\0';
	}

	if (ring_buf->dhd_pub) {
		dhd_pub_t *dhdp = (dhd_pub_t *)ring_buf->dhd_pub;
		if (type == DRIVER_LOG_RING_ID || type == FW_VERBOSE_RING_ID ||
				type == ROAM_STATS_RING_ID) {
			if (DBG_RING_ACTIVE(dhdp, type)) {
				dhd_os_push_push_ring_data(dhdp, type,
						tmp_buf, strlen(tmp_buf));
				return;
			}
		}
	}
	return;
}

#ifdef DHD_DEBUGABILITY_DEBUG_DUMP
void dhd_debug_dump_get_section_len(dhd_pub_t *dhdp, uint32 sec_len[])
{
	sec_len[LOG_DUMP_SECTION_TIMESTAMP] = dhd_get_time_str_len();
#ifdef EWP_ECNTRS_LOGGING
	sec_len[LOG_DUMP_SECTION_ECNTRS] = dhd_get_ecntrs_len(NULL, dhdp);
#endif /* EWP_ECNTRS_LOGGING */
	sec_len[LOG_DUMP_SECTION_DHD_DUMP] = dhd_get_dhd_dump_len(NULL, dhdp);
#if defined(BCMPCIE)
	sec_len[LOG_DUMP_SECTION_EXT_TRAP] = dhd_get_ext_trap_len(NULL, dhdp);
#endif /* BCMPCIE */

#if defined(DHD_FW_COREDUMP) && defined(DNGL_EVENT_SUPPORT)
	sec_len[LOG_DUMP_SECTION_HEALTH_CHK] = dhd_get_health_chk_len(NULL, dhdp);
#endif
	sec_len[LOG_DUMP_SECTION_COOKIE] = dhd_get_cookie_log_len(NULL, dhdp);
#ifdef DHD_DUMP_PCIE_RINGS
	sec_len[LOG_DUMP_SECTION_RING] = dhd_get_flowring_len(NULL, dhdp);
#endif
#ifdef DHD_STATUS_LOGGING
	sec_len[LOG_DUMP_SECTION_STATUS] = dhd_get_status_log_len(NULL, dhdp);
#endif /* DHD_STATUS_LOGGING */
#ifdef EWP_RTT_LOGGING
	sec_len[LOG_DUMP_SECTION_RTT] = dhd_get_rtt_len(NULL, dhdp);
#endif /* EWP_RTT_LOGGING */
#ifdef DHD_MAP_PKTID_LOGGING
	sec_len[LOG_DUMP_SECTION_PKTID_MAP_LOG] =
		dhd_get_pktid_map_logging_len(NULL, dhdp, TRUE);
	sec_len[LOG_DUMP_SECTION_PKTID_UNMAP_LOG] =
		dhd_get_pktid_map_logging_len(NULL, dhdp, FALSE);
#endif /* DHD_MAP_PKTID_LOGGING */

	DHD_PRINT(("%s: TS:%d ECNTRS:%d DHD_DUMP:%d ETRAP:%d"
		" HCK:%d CKI:%d FLOW:%d STATUS:%d RTT:%d PKTID_MAP:%d PKTID_UNMAP:%d\n",
		__func__, sec_len[LOG_DUMP_SECTION_TIMESTAMP], sec_len[LOG_DUMP_SECTION_ECNTRS],
		sec_len[LOG_DUMP_SECTION_DHD_DUMP], sec_len[LOG_DUMP_SECTION_EXT_TRAP],
		sec_len[LOG_DUMP_SECTION_HEALTH_CHK], sec_len[LOG_DUMP_SECTION_COOKIE],
		sec_len[LOG_DUMP_SECTION_RING], sec_len[LOG_DUMP_SECTION_STATUS],
		sec_len[LOG_DUMP_SECTION_RTT], sec_len[LOG_DUMP_SECTION_PKTID_MAP_LOG],
		sec_len[LOG_DUMP_SECTION_PKTID_UNMAP_LOG]));
	return;
}

int dhd_debug_dump_buf_alloc(dhd_pub_t *dhdp, int id, char* name, int size)
{
	eldbg_cmn_t *dbg = dhdp->dbg;
	eldbg_ring_cmn_t *ring = &dbg->dbg_rings[id];
	char *buf = NULL;

	if (!ring->ring_buf) {
		buf = MALLOCZ(dhdp->osh, size);
		if (!buf) {
			DHD_ERROR(("%s: Failed to alloc id:%d, size:%d\n", __func__, id, size));
			return BCME_NOMEM;
		}
		eldbg_ring_set_buf(dhdp->osh, &dbg->dbg_rings[id], buf);
		DHD_PRINT(("%s: success to allocate ring id:%d name:%s\n", __func__, id, name));
	} else {
		DHD_ERROR(("%s: already allocated id:%d name:%s\n", __func__, id, name));
	}

	return BCME_OK;
}

int dhd_debug_dump_to_ring(dhd_pub_t *dhdp)
{
	eldbg_cmn_t *dbg = dhdp->dbg;
	eldbg_ring_cmn_t *dbg_ring;
	int ret = BCME_OK;
	int ring_num, id;
	uint32 sec_len[LOG_DUMP_SECTION_MAX] = {0};
	eldbg_ring_status_t ring_status;
	uint32 sync_retry;
	unsigned long flags = 0;

	ret = dhd_debug_dump_buf_alloc(dhdp, DEBUG_DUMP_RING1_ID,
		DEBUG_DUMP_RING1_NAME, DEBUG_DUMP_RING1_SIZE);
	if (ret) {
		return ret;
	}

	ret = dhd_debug_dump_buf_alloc(dhdp, DEBUG_DUMP_RING2_ID,
		DEBUG_DUMP_RING2_NAME, DEBUG_DUMP_RING2_SIZE);
	if (ret) {
		return ret;
	}

#ifdef DHD_MAP_PKTID_LOGGING
	dhd_pktid_logging_dump(dhdp);
#endif /* DHD_MAP_PKTID_LOGGING */
	dhd_debug_dump_get_section_len(dhdp, sec_len);

	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_TIMESTAMP);
	if (dhd_print_time_str(NULL, NULL, sec_len[LOG_DUMP_SECTION_TIMESTAMP], &ring_num)) {
		DHD_ERROR(("Error section: dhd_print_time_str\n"));
	}
#ifdef EWP_ECNTRS_LOGGING
	if (dhd_print_ecntrs_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_ECNTRS], NULL)) {
		DHD_ERROR(("Error section: ECNTRS\n"));
	}
#endif /* EWP_ECNTRS_LOGGING */

#ifdef DHD_STATUS_LOGGING
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_STATUS);
	if (dhd_print_status_log_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_STATUS], &ring_num)) {
		DHD_ERROR(("Error section: STATUS_LOG\n"));
	}
#endif /* DHD_STATUS_LOGGING */
#ifdef EWP_RTT_LOGGING
	if (dhd_print_rtt_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_RTT], NULL)) {
		DHD_ERROR(("Error section: RTT_LOG\n"));
	}
#endif /* EWP_RTT_LOGGING */
#ifdef DHD_MAP_PKTID_LOGGING
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_PKTID_MAP_LOG);
	if (dhd_print_pktid_map_log_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_PKTID_MAP_LOG], &ring_num, TRUE)) {
		DHD_ERROR(("Error section: PKTID_MAP_LOG\n"));
	}
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_PKTID_UNMAP_LOG);
	if (dhd_print_pktid_map_log_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_PKTID_UNMAP_LOG], &ring_num, FALSE)) {
		DHD_ERROR(("Error section: PKTID_UNMAP_LOG\n"));
	}
#endif /* DHD_MAP_PKTID_LOGGING */
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_DHD_DUMP);
	if (dhd_print_dump_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_DHD_DUMP], &ring_num)) {
		DHD_ERROR(("Error section: DHD_DUMP\n"));
	}
#if defined(BCMPCIE)
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_EXT_TRAP);
	if (dhd_print_ext_trap_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_EXT_TRAP], &ring_num)) {
		DHD_ERROR(("Error section: EXT TRAP\n"));
	}
#endif /* BCMPCIE */
#if defined(DHD_FW_COREDUMP) && defined(DNGL_EVENT_SUPPORT)
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_HEALTH_CHK);
	if (dhd_print_health_chk_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_HEALTH_CHK], &ring_num)) {
		DHD_ERROR(("Error section: HEALTH CHECK\n"));
	}
#endif
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_COOKIE);
	if (dhd_print_cookie_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_COOKIE], &ring_num)) {
		DHD_ERROR(("Error section: COOKIE DATA\n"));
	}
#ifdef DHD_DUMP_PCIE_RINGS
	ring_num = dhd_debug_dump_get_ring_num(LOG_DUMP_SECTION_RING);
	if (dhd_print_flowring_data(NULL, dhdp, NULL, NULL,
		sec_len[LOG_DUMP_SECTION_RING], &ring_num)) {
		DHD_ERROR(("Error section: FLOWRING_DUMP\n"));
	}
#endif
	for (id = DEBUG_RING_ID_INVALID + 1; id < DEBUG_RING_ID_MAX; id++) {
		dbg_ring = &dbg->dbg_rings[id];
		if (VALID_RING(dbg_ring->id)) {
			/* To sync up with the ringbuffer in legacy HAL */
			dhd_os_trigger_get_ring_data(dhdp, dbg_ring->name);

			sync_retry = 0;
			while (sync_retry < DHD_DEBUG_DUMP_MAX_SYNC_CNT) {
				DHD_DBG_RING_LOCK(dbg_ring->lock, flags);
				__eldbg_get_ring_status(dbg_ring, &ring_status);
				DHD_DBG_RING_UNLOCK(dbg_ring->lock, flags);
				if (ring_status.written_bytes == ring_status.read_bytes) {
					break;
				} else {
					OSL_SLEEP(50);
				}
				sync_retry++;
			}
			DHD_PRINT(("%s: ring sync up name:%s w/r bytes(%u/%u) retry:%d\n",
				__func__, dbg_ring->name, ring_status.written_bytes,
				ring_status.read_bytes, sync_retry));
		}
	}
	return ret;
}
#endif /* DHD_DEBUGABILITY_DEBUG_DUMP */
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */
