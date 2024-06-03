/*
 * DHD debugability header file
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
 * $Id: $
 */

#ifndef _linux_dbg_h_
#define _linux_dbg_h_
#include <osl.h>
#include <bcmutils.h>

#include <eldbg_ring.h>

/* --------------------------------------------------------- */
/* these definition will be unified when porting dhd_debug.x to common */

// DBG_RING_ENTRY_DATA_TYPE came from dhd_debug.h
/* firmware verbose ring, ring id 1 */
#define FW_VERBOSE_RING_NAME		"fw_verbose"
#define FW_VERBOSE_RING_SIZE		(256 * 1024)
/* firmware event ring, ring id 2 */
#define FW_EVENT_RING_NAME		"fw_event"
#define FW_EVENT_RING_SIZE		(64 * 1024)
/* DHD connection event ring, ring id 3 */
#define DHD_EVENT_RING_NAME		"dhd_event"
#define DHD_EVENT_RING_SIZE		(64 * 1024)

// DBG_RING_ENTRY_DATA_TYPE came from dhd_debug.h
enum {
	/* set for binary entries */
	DBG_RING_ENTRY_FLAGS_HAS_BINARY = (1 << (0)),
	/* set if 64 bits timestamp is present */
	DBG_RING_ENTRY_FLAGS_HAS_TIMESTAMP = (1 << (1))
};

// DBG_RING_ENTRY_DATA_TYPE came from dhd_debug.h
enum {
	DBG_RING_ENTRY_EVENT_TYPE = 1,
	DBG_RING_ENTRY_PKT_TYPE,
	DBG_RING_ENTRY_WAKE_LOCK_EVENT_TYPE,
	DBG_RING_ENTRY_POWER_EVENT_TYPE,
	DBG_RING_ENTRY_DATA_TYPE,
	DBG_RING_ENTRY_NAN_EVENT_TYPE
};

// dhd_ring_id is came from dhd.h
typedef enum dhd_ring_id {
	DEBUG_RING_ID_INVALID =	0x1,
	FW_VERBOSE_RING_ID =	0x2,
	DHD_EVENT_RING_ID =	0x3,
	DRIVER_LOG_RING_ID =	0x4,
	ROAM_STATS_RING_ID =	0x5,
	BT_LOG_RING_ID =	0x6,
	PACKET_LOG_RING_ID =	0x7,
	DEBUG_DUMP_RING1_ID =	0x8,
	DEBUG_DUMP_RING2_ID =	0x9,
	MEM_DUMP_RING_ID =	0xa,
	PHY_PERIO_LOG_RING_ID =	0xb,
	DEBUG_RING_ID_MAX =	0xc
} dhd_ring_id_t;

typedef enum logtrace_ctrl {
	LOGTRACE_DISABLE = 0,
	LOGTRACE_RAW_FMT = 1,
	LOGTRACE_PARSED_FMT = 2
} logtrace_ctrl_t;

/* default will be changed to LOGTRACE_DISABLE when it is checked in
 * it means that EVENT_LOG message will not be printed to kerenl as a default
 */
#define DEFAULT_CONTROL_LOGTRACE	LOGTRACE_DISABLE
#ifndef CUSTOM_CONTROL_LOGTRACE
#define CUSTOM_CONTROL_LOGTRACE		DEFAULT_CONTROL_LOGTRACE
#endif

/* --------------------------------------------------------- */

#ifdef EVENT_LOG_NIC
#if defined(__linux__)
	/* (u64)result = (u64)dividend / (u64)divisor */
#define DIV_U64_BY_U64(dividend, divisor)	div64_u64(dividend, divisor)

	/* (u64)result = (u64)dividend / (u32)divisor */
#define DIV_U64_BY_U32(dividend, divisor)	div_u64(dividend, divisor)

	/* Be careful while using this, as it divides dividend also
	 * (u32)remainder = (u64)dividend % (u32)divisor
	 * (u64)dividend = (u64)dividend / (u32)divisor
	 */
#define DIV_AND_MOD_U64_BY_U32(dividend, divisor)	do_div(dividend, divisor)

	/* (u32)remainder = (u64)dividend % (u32)divisor */
#define MOD_U64_BY_U32(dividend, divisor) ({					\
		uint64 temp_dividend = (dividend);				\
		uint32 rem = DIV_AND_MOD_U64_BY_U32(temp_dividend, (divisor));	\
		rem;								\
	})

#define SEC_USEC_FMT \
		"%5llu.%06u"
#else
	/* (u64)result = (u64)dividend / (u64)divisor */
#define DIV_U64_BY_U64(dividend, divisor)	(uint64)(dividend) / (uint64)(divisor)

	/* (u64)result = (u64)dividend / (u32)divisor */
#define DIV_U64_BY_U32(dividend, divisor)	(uint64)(dividend) / (uint32)(divisor)

	/* Be careful while using this, as it divides dividend also
	 * (u32)remainder = (u64)dividend % (u32)divisor
	 * (u64)dividend = (u64)dividend / (u32)divisor
	 */
#define DIV_AND_MOD_U64_BY_U32(dividend, divisor) ({			\
		uint32 rem = (uint64)(dividend) % (uint32)(divisor);	\
		(dividend) = (uint64)(dividend) / (uint32)(divisor);	\
		rem;							\
	})

	/* (u32)remainder = (u64)dividend % (u32)divisor */
#define MOD_U64_BY_U32(dividend, divisor)	(uint32)((uint64)(dividend) % (uint32)(divisor))

#define SEC_USEC_FMT \
		"%015llu.%06u"
#endif /* __linux__ */
#endif /* EVENT_LOG_NIC */

#define TLV_LOG_SIZE(tlv) ((tlv) ? (sizeof(tlv_log) + (tlv)->len) : 0)

#define TLV_LOG_NEXT(tlv) \
	((tlv) ? ((tlv_log *)((uint8 *)tlv + TLV_LOG_SIZE(tlv))) : 0)

#define ELDBG_RING_STATUS_SIZE (sizeof(eldbg_ring_status_t))

#define VALID_RING(id)	\
	((id > DEBUG_RING_ID_INVALID) && (id < DEBUG_RING_ID_MAX))

typedef struct eldbg_cmn {
	osl_t *osh;
	void *hdl;
	eldbg_ring_cmn_t dbg_rings[DEBUG_RING_ID_MAX];
#ifndef EVENT_LOG_NIC
	/* other variables will be used after porting dhd_debug.c */
	void *private;	/* os private_data */
	dhd_dbg_pkt_mon_t pkt_mon;	/* packet monitoring structure */
	void *pkt_mon_lock;	/* spin lock for packet monitoring */
	dbg_pullreq_t pullreq;	/* os pull request */
	dbg_urgent_noti_t urgent_notifier;	/* os urgent nofifier */
	dhd_dbg_buf_t wrapper_buf;	/* logtrace wrapper buffer */
	uint32 wrapper_regdump_size;	/* wrapper buffer size */
	uint32 event_log_ts_ver;	/* event log timestamp version being supported */
	uint8 ets_msg[ENHANCED_TIMESTAMP_V2_MSG_LEN];	/* snapshot of the latest ETS V2 message */
#endif /* EVENT_LOG_NIC */
} eldbg_cmn_t;

eldbg_cmn_t *dbg_module_attach(void *hdl);
void dbg_module_detach(void *hdl);

int dbg_push_to_ring(void *hdl, int ring_id, eldbg_ring_entry_t *hdr, void *data);
extern void eldbg_ring_proc_create(void *info);
extern void eldbg_ring_proc_destroy(void *info);

/* below functions should be removed from dhd_debug.c & dhd_debug.h after porting */
int __eldbg_get_ring_status(eldbg_ring_cmn_t *ring, eldbg_ring_status_t *get_ring_status);
#ifdef SHOW_LOGTRACE
void eldbg_read_ring_into_trace_buf(eldbg_ring_cmn_t *ring, trace_buf_info_t *trace_buf_info);
#endif /* SHOW_LOGTRACE */
eldbg_ring_cmn_t *eldbg_get_ring_from_ring_id(eldbg_cmn_t *dbg, int ring_id);
#endif /* _linux_dbg_h_ */
