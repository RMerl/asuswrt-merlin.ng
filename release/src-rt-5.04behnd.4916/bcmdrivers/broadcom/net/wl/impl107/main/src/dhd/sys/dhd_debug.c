/*
 * DHD debugability support
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: dhd_debug.c 841233 2024-06-13 08:13:24Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>
#include <linux_dbg.h>
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

struct map_table {
	uint16 fw_id;
	uint16 host_id;
	char *desc;
};

typedef union {
	uint32 val;
	char * addr;
} u_dbg_arg;

struct map_table event_map[] = {
	{WLC_E_AUTH, WIFI_EVENT_AUTH_COMPLETE, "AUTH_COMPLETE"},
	{WLC_E_ASSOC, WIFI_EVENT_ASSOC_COMPLETE, "ASSOC_COMPLETE"},
	{TRACE_FW_AUTH_STARTED, WIFI_EVENT_FW_AUTH_STARTED, "AUTH STARTED"},
	{TRACE_FW_ASSOC_STARTED, WIFI_EVENT_FW_ASSOC_STARTED, "ASSOC STARTED"},
	{TRACE_FW_RE_ASSOC_STARTED, WIFI_EVENT_FW_RE_ASSOC_STARTED, "REASSOC STARTED"},
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
#ifdef WDS_FREXP
	{WLC_E_WDS_RS_ASSOC, WIFI_EVENT_ASSOC_COMPLETE, "WDS_ASSOC_COMPLETE"},
#endif /* WDS_FREXP */
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

typedef struct dhddbg_loglist_item {
	dll_t list;
	prcd_event_log_hdr_t prcd_log_hdr;
} loglist_item_t;

typedef struct dhbdbg_pending_item {
	dll_t list;
	eldbg_ring_status_t ring_status;
	eldbg_ring_entry_t *ring_entry;
} pending_item_t;

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

#define FMTSTR_SIZE	200
#define ROMSTR_SIZE	268
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
dhd_log_enhanced_timestamp(eldbg_cmn_t *dbg, event_log_hdr_t *ts_hdr, uint32 *data)
{
	ets_msg_t *ets_msg_ptr = (ets_msg_t *)ts_hdr - ts_hdr->count;

	if (!dbg->event_log_ts_ver) {
		dbg->event_log_ts_ver = ets_msg_ptr->version;
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
				memcpy_s(dbg->ets_msg, sizeof(dbg->ets_msg),
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

static uint32
dhd_dbg_verboselog_handler(eldbg_cmn_t *dbg, prcd_event_log_hdr_t *plog_hdr,
	uint32 logset, uint32* data, char *buf_to_write, uint32 buflen)
{
	event_log_hdr_t *ts_hdr;
	uint32 *log_ptr = plog_hdr->log_ptr;
	uint32 *ts_data;
	uint32 parsed_len = 0;

	if (log_ptr < data) {
		DHD_ERROR(("Invalid log pointer, logptr : %p data : %p \n", log_ptr, data));
		return 0;
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
			dhd_log_enhanced_timestamp(dbg, ts_hdr, data);
		}
	}

	if (plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE ||
		plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE_TLV) {
#ifdef MOBILITY_LOG_TRACE
		wl_mschdbg_verboselog_handler(dhdp, raw_event_ptr, plog_hdr, log_ptr);
#endif /* MOBILITY_LOG_TRACE */
		return 0;
	}

	/* print the message out in a logprint  */
	parsed_len = dhd_dbg_verboselog_printf(dbg, plog_hdr, log_ptr, logset,
		buf_to_write, buflen);
	return parsed_len;
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
dhd_dbg_fw_time(eldbg_cmn_t *dbg, uint32 arm_time_cycle, char *buf, uint32 len)
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
	switch (dbg->event_log_ts_ver) {
		case ENHANCED_TS_MSG_VERSION_2:
			ret = bcmutils_event_log_compute_current_time_1(
				dbg->ets_msg, arm_time_cycle, &ptm_fw_time_ns);
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
				__FUNCTION__, dbg->event_log_ts_ver));
		break;
	}
}
#else
static void
dhd_dbg_fw_time(eldbg_cmn_t *dbg, time_us_t sysuptime_us, char *buf, uint32 len)
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

static void
dhd_dbg_parsing_percent_s(el_raw_info_t *el_raw_info, uint32 total_count,
	uint32 *log_ptr, char *fmtstr_loc_buf, u_dbg_arg *arg)
{
	char *str_buf[MAX_NO_OF_ARG];
	char *c_ptr = fmtstr_loc_buf;
	uint32 count, addr_val, raw_pos;
	bool is_found;

	for (count = 0; count < (total_count - 1); count++) {

		is_found = FALSE;

		/* assign pointer from str_buf which is allocated in dhd_init_static_strs_array() */
		str_buf[count] = el_raw_info->str_buf + (count * SIZE_LOC_STR);
		/* search %s specifier in fmt and copy str data of rodata to local str buf */
		while (*c_ptr != '\0') {
			/* find % */
			if (*c_ptr++ != '%') {
				continue;
			}

			/* Skip modifiers (#, 0, -, +, ' ') followed by % */
			if ((*c_ptr == '#') || (*c_ptr == '0') || (*c_ptr == '-') ||
				(*c_ptr == '+') || (*c_ptr == ' ')) {
				c_ptr++;
			}

			/* Skip width with decimal digit string */
			while (*c_ptr && bcm_isdigit(*c_ptr)) {
				c_ptr++;
			}

			/* Skip a period ('.') followed by an decimal digit string */
			if (*c_ptr == '.') {
				c_ptr++;
				while (*c_ptr && bcm_isdigit(*c_ptr)) {
					c_ptr++;
				}
			}

			if (*c_ptr == 's') {
				/* found %s specifier */
				is_found = TRUE;
			}

			c_ptr++;
			break;
		}

		if (is_found) {
			/* check whether address is in read-only data section
			 * if the addr is in rodata, copy the string of rodata to the Nth str buf
			 * 1. check read-only data section first
			 * 2. if not, check pageable read-only string data section
			 * 3. if addr is not in all rodata range,  print addr value with (str)
			 */
			addr_val = log_ptr[count];
			if ((el_raw_info->raw_rodata) &&
				(addr_val > el_raw_info->rodata_start) &&
				(addr_val < el_raw_info->rodata_end)) {
				/* copy string in read-only data by addr value of %s */
				raw_pos = addr_val - el_raw_info->rodata_start;
				(void)strlcpy(str_buf[count], el_raw_info->raw_rodata + raw_pos,
					SIZE_LOC_STR);
			} else if (el_raw_info->raw_rodatastr &&
				(addr_val > el_raw_info->rodatastr_start) &&
				(addr_val < el_raw_info->rodatastr_end)) {
				/* copy string in pageable_rodatastr by addr value of %s */
				raw_pos = addr_val -
					el_raw_info->rodatastr_start;
				(void)strlcpy(str_buf[count], el_raw_info->raw_rodatastr + raw_pos,
					SIZE_LOC_STR);
			} else {
				/* if addr is not in read-only data,
				 * add "(str)" to indicate usecase of %s specifier
				 * and print address value
				 */
				snprintf(str_buf[count], SIZE_LOC_STR,
					"(str)0x%x", log_ptr[count]);
			}
			/* update addr value of union arg to indicate string */
			arg[count].addr = str_buf[count];
		} else {
			/* just copy the same value to data if it is not %s specifier */
			arg[count].val = log_ptr[count];
		}
	}
}

static uint32
dhd_dbg_parsing_string_printf(eldbg_cmn_t *dbg, prcd_event_log_hdr_t *plog_hdr,
	uint32 *log_ptr, char *buf_to_write, uint32 buflen)
{
	el_raw_info_t *el_raw_info = &dbg->el_raw_info;
	char fmtstr_loc_buf[ROMSTR_SIZE] = { 0 };
	char *fmted_buf;
	u_dbg_arg arg[MAX_NO_OF_ARG] = {{ 0 }};
	char ptm_fw_time[PTM_FW_TIME_LEN] = { 0 };
	int parsed_len = 0;
	if ((el_raw_info == NULL) || (plog_hdr == NULL)) {
		DHD_ERROR(("%s: el_raw_info or plog_hdr is NULL\n", __FUNCTION__));
		return parsed_len;
	}

	if ((plog_hdr->fmt_num) >= el_raw_info->num_fmts) {
		DHD_ERROR(("%s: fmt number: 0x%x out of range\n",
			__FUNCTION__, plog_hdr->fmt_num));
		return parsed_len;
	}

	if (plog_hdr->count > MAX_NO_OF_ARG) {
		DHD_ERROR(("%s: plog_hdr->count(%d) out of range\n",
			__FUNCTION__, plog_hdr->count));
		return parsed_len;
	}

	if (el_raw_info->str_buf == NULL) {
		DHD_ERROR(("%s: el_raw_info->str_buf is not allocated\n", __FUNCTION__));
		return parsed_len;
	}

	if (el_raw_info->fmted_buf == NULL) {
		DHD_ERROR(("%s: el_raw_info->fmted_buf is not allocated\n", __FUNCTION__));
		return parsed_len;
	}
	fmted_buf = el_raw_info->fmted_buf;

	/* clear str buf to get string data of %s specifier in rodata */
	bzero(el_raw_info->str_buf, (MAX_NO_OF_ARG * SIZE_LOC_STR));

	if (plog_hdr->tag == EVENT_LOG_TAG_MSCHPROFILE) {
		snprintf(fmtstr_loc_buf, FMTSTR_SIZE, "EL[%d][wl%d] %s",
			plog_hdr->tag, dbg->unit,
			el_raw_info->fmts[plog_hdr->fmt_num]);
		plog_hdr->count++;
	} else {
		dhd_dbg_fw_time(dbg, log_ptr[plog_hdr->count - 1],
			ptm_fw_time, sizeof(ptm_fw_time));
		snprintf(fmtstr_loc_buf, FMTSTR_SIZE, "EL[%d][wl%d] %s %s",
			plog_hdr->tag, dbg->unit,
			&ptm_fw_time[0],
			el_raw_info->fmts[plog_hdr->fmt_num]);
	}

	/* print the format string which will be needed for debugging incorrect formats */
	DHD_INFO(("%s: fmtstr_loc_buf = %s\n", __FUNCTION__, fmtstr_loc_buf));

	/* Replace all %p to %x to handle 32 bit %p */
	replace_percent_p_to_x(fmtstr_loc_buf);

	/* fill arg value of string value of %s specifier from read-only data */
	dhd_dbg_parsing_percent_s(el_raw_info, plog_hdr->count, log_ptr, fmtstr_loc_buf, arg);

	parsed_len = snprintf(fmted_buf, el_raw_info->fmted_buf_size, fmtstr_loc_buf,
		arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7],
		arg[8], arg[9], arg[10], arg[11], arg[12], arg[13], arg[14], arg[15],
		arg[16], arg[17], arg[18], arg[19], arg[20], arg[21], arg[22], arg[23],
		arg[24], arg[25], arg[26], arg[27], arg[28], arg[29], arg[30], arg[31],
		arg[32], arg[33], arg[34], arg[35], arg[36], arg[37], arg[38], arg[39],
		arg[40], arg[41], arg[42], arg[43], arg[44], arg[45], arg[46], arg[47],
		arg[48], arg[49], arg[50], arg[51], arg[52], arg[53], arg[54], arg[55],
		arg[56], arg[57], arg[58], arg[59]);

	if (buf_to_write != NULL) {
		/* copy a parsed log to the buf. len validataion in memcyp_s()
		 * it will be sent up to user space after pasring all logs of event log block
		 */
		if (memcpy_s(buf_to_write, buflen, fmted_buf, parsed_len) != BCME_OK) {
			parsed_len = 0;
			DHD_ERROR(("%s: failed to copy parsed logs to buf len %d, pared len %d\n",
				__FUNCTION__, buflen, parsed_len));
		}
	} else {
		/* print out a log after parsing */
		DHD_ELLOG(("%s", fmted_buf));
	}

	return parsed_len;
}

uint32
dhd_dbg_verboselog_printf(eldbg_cmn_t *dbg, prcd_event_log_hdr_t *plog_hdr,
	uint32 *log_ptr, uint32 logset, char *buf_to_write, uint32 buflen)
{
	uint16 count;
	char fmtstr_loc_buf[ROMSTR_SIZE] = { 0 };
	struct bcmstrbuf b;
	char ptm_fw_time[PTM_FW_TIME_LEN] = { 0 };
	char *raw_print_prefix = "EL";
	uint32 parsed_len = 0;

	if (plog_hdr->binary_payload == FALSE) {
		int ring_id = dbg_get_str_ring_id_from_set_num(logset);
		if ((ring_id == DEBUG_RING_ID_INVALID) ||
			!(CTL_LT_ALL_STR_IS_ON(ring_id, control_logtrace) ||
				(buf_to_write != NULL))) {
			return parsed_len;
		}

		/* procesing string message */
		if ((CTL_LT_ALL_STR_IS_PARSING_ON(ring_id, control_logtrace) ||
			(buf_to_write != NULL)) && dbg->el_raw_info.fmts) {
			/* if parsing option is on and format string exist,
			 * print string message after parsing
			 */
			parsed_len = dhd_dbg_parsing_string_printf(dbg, plog_hdr,
				log_ptr, buf_to_write, buflen);
		} else {
			/* if parsing option is off or
			 * parsing option is on but no format string (el_raw_info.fmts)
			 * print only raw encoded string without parsing
			 */
			bcm_binit(&b, fmtstr_loc_buf, FMTSTR_SIZE);
			/* The 'hdr->count - 1' is dongle time */
			dhd_dbg_fw_time(dbg, log_ptr[plog_hdr->count - 1],
				ptm_fw_time, sizeof(ptm_fw_time));

			bcm_bprintf(&b, "%s %s: %d 0x%x",
				&ptm_fw_time[0],
				raw_print_prefix,
				plog_hdr->tag,
				plog_hdr->fmt_num_raw);

			for (count = 0; count < (plog_hdr->count - 1); count++) {
				bcm_bprintf(&b, " %x", log_ptr[count]);
			}
			/* raw format el printout */
			DHD_INFO(("%s, b=%s \n", __FUNCTION__, b.origbuf));
			DHD_ELLOG(("%s\n", b.origbuf));
		}
	} else if (CTL_LT_BIN_IS_ON(control_logtrace) &&
		/* buf_to_write is to parse string log only */
		(buf_to_write == NULL)) {
		/* procesing binary data */
		dhd_dbg_fw_time(dbg, log_ptr[plog_hdr->count - 1],
			ptm_fw_time, sizeof(ptm_fw_time));
		DHD_ELLOG(("%s %s:tag=%d len=%d fmt=0x%x",
			&ptm_fw_time[0],
			raw_print_prefix,
			plog_hdr->tag,
			plog_hdr->count,
			plog_hdr->fmt_num_raw));

		for (count = 0; count < (plog_hdr->count - 1); count++) {
			/* skip first line feed in case count 0 */
			if (count && (count % LOG_PRINT_CNT_MAX == 0)) {
				DHD_ELLOG(("\n\t%08x", log_ptr[count]));
			} else {
				DHD_ELLOG((" %08x", log_ptr[count]));
			}
		}
		DHD_ELLOG(("\n"));
	}

	return parsed_len;
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
dhd_dbg_logtrace_process_payload(eldbg_cmn_t *dbg, char *data, uint datalen,
	dll_t *list_head)
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
			DHD_ERROR(("%s: wl%d: Error while parsing event log header\n",
				__FUNCTION__, dbg->unit));
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

		/* if it is not binary data but argments is bigger than max #,
		 * do not hanldle it more
		 * to print binary data is controled by bit 2, CTL_LT_BIN_IS_ON(control_logtrace)
		 * in dhd_dbg_verboselog_printf()
		 */
		if ((prcd_log_hdr.binary_payload == FALSE) &&
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
		if (!(log_item = MALLOC(dbg->osh, sizeof(*log_item)))) {
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
	uint datalen, bool msgtrace_hdr_present, uint32 msgtrace_seqnum)
{
	msgtrace_hdr_t *hdr;
	char *data;
	static uint32 seqnum_prev[OSL_DEV_TO_IFID_INVALID] = {0};
	uint unit;
	eldbg_ring_entry_t msg_hdr;
	struct tracelog_header *logentry_header;
	int ring_id = DEBUG_RING_ID_INVALID;
	bool event_log_max_sets_queried;
	uint32 event_log_max_sets;
	uint min_expected_len = 0;
	event_log_hdr_t *log_hdr = NULL;
	const uint32 log_hdr_len = sizeof(*log_hdr);
	uint8 el_hdr_type = 0;
	event_log_block_header_t *evtb_hdr = NULL;
	bool is_vsnprintf_str = FALSE;
	uint32 logset;

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
			evtb_hdr = (event_log_block_header_t *)((char *)event_data +
				MSGTRACE_HDRLEN);
			if (evtb_hdr) {
				DHD_PRINT(("EVENT_LOG_HDR[0x%x]: Set: 0x%08x length = %d\n",
					evtb_hdr->count, evtb_hdr->extra_hdr_info,
					evtb_hdr->pktlen));
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

	unit = dhdp->unit;
	if (unit >= OSL_DEV_TO_IFID_INVALID) {
		if (dhd_get_instance_base() != 0) {
			unit = dhdp->unit - dhd_get_instance_base();
		}
		if (unit >= OSL_DEV_TO_IFID_INVALID) {
			/* error handling for exception */
			return;
		}
	}
	if (dhd_dbg_msgtrace_seqchk(&seqnum_prev[unit], msgtrace_seqnum)) {
		return;
	}

	/* Store message to event log ring */

	/* init ring entry header */
	bzero(&msg_hdr, sizeof(eldbg_ring_entry_t));
	msg_hdr.type = DBG_RING_ENTRY_DATA_TYPE;

	if ((sizeof(*logentry_header) + datalen) > ELDBG_PAYLOAD_MAX_LEN) {
		DHD_ERROR(("%s:Payload len=%u exceeds max len\n", __FUNCTION__,
			((uint)sizeof(*logentry_header) + datalen)));
		goto exit;
	}

	/* Print sequence number, originating set and length of received event log buffer */
	evtb_hdr = (event_log_block_header_t *)data;
	is_vsnprintf_str = (evtb_hdr->extra_hdr_info &
		EVENT_LOG_BLOCK_SET_VSNPRINTF_MASK) ? TRUE : FALSE;
	logset = (evtb_hdr->extra_hdr_info & EVENT_LOG_SETID_MASK);
	DHD_MSGTRACE_LOG(("EVENT_LOG_HDR[%d]: Set: %d length = %d, is_vsnprintf_str %d\n",
		evtb_hdr->count, logset, evtb_hdr->pktlen, is_vsnprintf_str));

	if (logset >= event_log_max_sets) {
		DHD_ERROR(("%s logset: %d max: %d out of range queried: %d\n",
			__FUNCTION__, logset, event_log_max_sets, event_log_max_sets_queried));
	}

	if (is_vsnprintf_str) {
		/* process for formatted string type data via vsnprintf method */

		/* vsnprintf does not need to add logentry_header
		 * move pointer to actual data of string
		 */
		data += EVENT_LOG_BLOCK_HDRLEN;
		datalen -= EVENT_LOG_BLOCK_HDRLEN;

		/* fill actual data len of string to msg_hdr */
		msg_hdr.len = (uint16)strlen(data);

		if (msg_hdr.len > (uint16)datalen) {
			DHD_ERROR(("dhd_dbg_msgtrace_log_parser: invalid len %u vs %u\n",
				msg_hdr.len, (uint)datalen));
			goto exit;
		}

		ring_id = dbg_get_str_ring_id_from_set_num(logset);
		if (ring_id == DEBUG_RING_ID_INVALID) {
			DHD_ERROR(("%s: invalid ring_id logset %d, ring_id %d\n",
				__FUNCTION__, logset, ring_id));
			goto exit;
		}

		/* push to ring */
		if (dbg_push_to_ring(dhdp->dbg, ring_id, &msg_hdr, data) != BCME_OK) {
			DHD_ERROR(("%s: push failed HL %d DL %d RID:%d\n",
				__FUNCTION__, msg_hdr.len, datalen, ring_id));
			goto exit;
		}

#ifdef SHOW_LOGTRACE
		/* print logs if control_logtrace is enabled for string */
		if (CTL_LT_STR_IS_ON(control_logtrace) && ((ring_id == EVENT_LOG_STR_RING_ID) ||
			(CTL_LT_STR_CHATTY_IS_ON(control_logtrace) &&
			(ring_id == EVENT_LOG_STR_CHATTY_RING_ID)))) {
			/* if string log bit is on, print the log to the console */
			DHD_ELLOG(("%s\n", data));
		}
#endif /* SHOW_LOGTRACE */

		/* do not need any process more if it is svnprintf method */
		return;
	}

	/* process for encoding type data */

	/* FW provides space to fill the value of logentry header
	 * copy event log block header to the next of logentry header
	 */
	*(event_log_block_header_t *)(data + sizeof(*logentry_header)) = *evtb_hdr;
	if (sizeof(*logentry_header) < sizeof(*evtb_hdr)) {
		DHD_ERROR(("%s: if logentry_header(%d) < evtb_hdr(%d), it needs memmove_s()\n",
			__FUNCTION__, (uint32)sizeof(*logentry_header),
			(uint32)sizeof(*evtb_hdr)));
		ASSERT(0);
	}

	/* fill logentry to empty slot */
	logentry_header = (struct tracelog_header *)data;
	logentry_header->magic_num = TRACE_LOG_MAGIC_NUMBER;
	logentry_header->buf_size = datalen - sizeof(*logentry_header);
	logentry_header->seq_num = msgtrace_seqnum;

	/* fill actual data len to msg_hdr */
	msg_hdr.len = datalen;

	/* update evt_hdr pointer */
	evtb_hdr = (event_log_block_header_t *)(data + sizeof(*logentry_header));

	/* move pointer to event log block */
	data += sizeof(*logentry_header);
	datalen -= sizeof(*logentry_header);

	/* find event log entry header from the end of buffer excluding logentry headers */
	if ((datalen - EVENT_LOG_BLOCK_HDRLEN) > log_hdr_len) {
		uint len = datalen;
		/* skip zero padding with 4 byte offset  at the end of block */
		while (len > log_hdr_len) {
			log_hdr = (event_log_hdr_t *)(data + len - log_hdr_len);
			if (log_hdr->tag == EVENT_LOG_TAG_NULL) {
				len -= log_hdr_len;
				continue;
			} else {
				break;
			}
		}
		el_hdr_type = log_hdr->fmt_num & DHD_EVENT_LOG_HDR_MASK;
	} else {
		DHD_ERROR(("%s invalid case - datalen %d < log_hdr_len %d \n",
			__FUNCTION__, datalen, (uint32)log_hdr_len));
		goto exit;
	}

	if (el_hdr_type == DHD_BI_EVENT_LOG_HDR) {
		ring_id = EVENT_LOG_BIN_RING_ID;
	} else {
		ring_id = dbg_get_str_ring_id_from_set_num(logset);
		if (ring_id == DEBUG_RING_ID_INVALID) {
			DHD_ERROR(("%s: encoding type invalid ring_id logset %d, ring_id %d\n",
				__FUNCTION__, logset, ring_id));
			goto exit;
		}
	}

	/* push to ring */
	if (dbg_push_to_ring(dhdp->dbg, ring_id, &msg_hdr, logentry_header) != BCME_OK) {
		DHD_ERROR(("push failed HL %d DL %d RID:%d\n",
			msg_hdr.len, datalen, ring_id));
		goto exit;
	}

#ifdef SHOW_LOGTRACE
	/* if control_logtrace is ON for str and binary, start parsing event log block */
	if (CTL_LT_IS_ON(control_logtrace)) {
		/* data ponter indicates event log block header */
		(void)dhd_dbg_event_log_block_parser(dhdp->dbg, data, datalen, NULL);
	}
#endif /* SHOW_LOGTRACE */

exit:
	/* do nothing more */
	return;
}

uint32
dhd_dbg_event_log_block_parser(void *dbg_hdl, char *data, uint32 datalen,
	ring_parser_info_t *el_ring_parser)
{
	dll_t list_head, *cur;
	prcd_event_log_hdr_t *plog_hdr;
	loglist_item_t *log_item;
	bool dll_inited = FALSE;
	uint32 all_parsed_len = 0;
	uint32 parsed_len = 0;
	uint32 logset;
	eldbg_cmn_t *dbg = dbg_hdl;
	/* data ponter indicates event log block header */
	event_log_block_header_t *evtb_hdr = (event_log_block_header_t *)data;

	if ((dbg_hdl == NULL) || (data == NULL)) {
		return 0;
	}

	/* get SET value of the block */
	logset = (evtb_hdr->extra_hdr_info & EVENT_LOG_SETID_MASK);

	DHD_INFO(("%s: logset %d, pktlen %d, count %d, extra 0x%x \n", __FUNCTION__,
		logset, evtb_hdr->pktlen, evtb_hdr->count, evtb_hdr->extra_hdr_info));

	/* make list for each logs from one event log block */
	dhd_dbg_logtrace_process_payload(dbg, data, datalen, &list_head);

	dll_inited = TRUE;
	/* process to handle each log in the list */
	while (!dll_empty(&list_head)) {
		cur = dll_head_p(&list_head);

		log_item = (loglist_item_t *)container_of(cur, loglist_item_t, list);

		plog_hdr = &log_item->prcd_log_hdr;

		if (el_ring_parser == NULL) {
			/* it is to print parsed logs after parsing the data from dongle */
			(void)dhd_dbg_verboselog_handler(dbg, plog_hdr,	logset,
				(uint32 *)data, NULL, 0);

		} else {
			/* it is to write parsed logs to buf after parsing the data in the ring */
			parsed_len = dhd_dbg_verboselog_handler(dbg, plog_hdr, logset,
				(uint32 *)data,
				(el_ring_parser->parser_buf + el_ring_parser->parser_pos),
				(el_ring_parser->parser_buflen - el_ring_parser->parser_pos));

			/* update position to parse & save the next encoded log */
			el_ring_parser->parser_pos += parsed_len;
			all_parsed_len += parsed_len;

			DHD_INFO(("%s: parsed len %d, pos %d, remained len %d\n",
				__FUNCTION__, parsed_len, el_ring_parser->parser_pos,
				(el_ring_parser->parser_buflen - el_ring_parser->parser_pos)));
		}

		dll_delete(cur);
		MFREE(dbg->osh, log_item, sizeof(*log_item));
	}

	/* mostly log_item is freed in the while loop above.
	 * while loop below is to prevent any free missing
	 */
	while (dll_inited && (!dll_empty(&list_head))) {
		cur = dll_head_p(&list_head);
		GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
		log_item = (loglist_item_t *)container_of(cur, loglist_item_t, list);
		GCC_DIAGNOSTIC_POP();

		dll_delete(cur);
		MFREE(dbg->osh, log_item, sizeof(*log_item));
	}

	return all_parsed_len;
}
#else /* !SHOW_LOGTRACE */
static INLINE void dhd_dbg_verboselog_handler(eldbg_cmn_t *dbg, prcd_event_log_hdr_t *plog_hdr,
	uint32 logset, uint32* data, char *buf_to_write, uint32 buflen) {};
INLINE void dhd_dbg_msgtrace_log_parser(dhd_pub_t *dhdp, void *event_data,
	uint datalen, bool msgtrace_hdr_present, uint32 msgtrace_seqnum) {};
#endif /* SHOW_LOGTRACE */
void
dhd_dbg_trace_evnt_handler(dhd_pub_t *dhdp, void *event_data, uint datalen)
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
		dhd_dbg_msgtrace_log_parser(dhdp, event_data, datalen, TRUE, 0);
}

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

static void dhd_do_aml_disable(dhd_pub_t *dhdp, void *event_info, u8 event)
{
	uint val = 0; /* Disabled */

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
		dhd_deferred_schedule_work(dhdp, NULL, DHD_WQ_WORK_AML_IOVAR,
			dhd_do_aml_disable, DHD_WQ_WORK_PRIORITY_HIGH);
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
		if (type == DRIVER_LOG_RING_ID || type == EVENT_LOG_STR_RING_ID) {
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
