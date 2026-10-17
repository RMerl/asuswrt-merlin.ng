/*
 * DHD and NIC debug ring header file - interface
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
 * $Id$
 */

#ifndef __ELDBG_RING_H__
#define __ELDBG_RING_H__

#include <bcmutils.h>

#if defined(__linux__)
#define PACKED_STRUCT __attribute__ ((packed))
#else
#define PACKED_STRUCT
#endif

#define DBG_RING_NAME_MAX 32

/* DBG_RING_ENTRY_DATA_TYPE came from dhd_debug.h */
enum {
	/* set for binary entries */
	DBG_RING_ENTRY_FLAGS_HAS_BINARY = (1 << (0)),
	/* set if 64 bits timestamp is present */
	DBG_RING_ENTRY_FLAGS_HAS_TIMESTAMP = (1 << (1))
};

enum {
	/* Verbose logging level to set for each debug ring buffer */
	RING_LOG_LEVEL_NONE = 0,
	RING_LOG_LEVEL_DEFAULT = 1,
	RING_LOG_LEVEL_VERBOSE = 2,
	RING_LOG_LEVEL_EXCESSIVE = 3
};

/* DBG_RING_ENTRY_DATA_TYPE came from dhd_debug.h */
/* ring for main string log */
#define EVENT_LOG_STR_RING_NAME		"el_string"
#define EVENT_LOG_STR_RING_SIZE		(128 * 1024)
/* ring for string chatty log */
#define EVENT_LOG_STR_CHATTY_RING_NAME	"el_string_chatty"
#define EVENT_LOG_STR_CHATTY_RING_SIZE	(128 * 1024)
/* ring for binary data via event_log_buffer */
#define EVENT_LOG_BIN_RING_NAME		"el_binary"
#define EVENT_LOG_BIN_RING_SIZE		(128 * 1024)

#ifdef DHD_DEBUGABILITY_LOG_DUMP_RING
/* DHD driver log ring */
#define DRIVER_LOG_RING_NAME		"driver_log"
#define DRIVER_LOG_RING_SIZE		(256 * 1024)
/* ROAM stats log ring */
#define ROAM_STATS_RING_NAME		"roam_stats"
#define ROAM_STATS_RING_SIZE		(64 * 1024)

#define DEBUG_DUMP_RING1_NAME		"debug_dump1_"
#define DEBUG_DUMP_RING1_SIZE		(2 * 1024 * 1024)

#define DEBUG_DUMP_RING2_NAME		"debug_dump2_"
#define DEBUG_DUMP_RING2_SIZE		(2 * 1024 * 1024)

#define MEM_DUMP_RING_NAME		"mem_dump"
#define MEM_DUMP_RING_SIZE		(3 * 1024 * 1024)

#define DHD_DEBUG_DUMP_NETLINK_MAX	(1024 * 8)
#define DHD_DEBUG_DUMP_MAX_SYNC_CNT	5u
#endif /* DHD_DEBUGABILITY_LOG_DUMP_RING */

/* Packet log ring, ring id 7 */
#ifdef DHD_PKT_LOGGING_DBGRING
#define DHD_PACKET_LOG_RING_NAME	"packet_log"
#define DHD_PACKET_LOG_RING_PKTS	MIN_PKTLOG_LEN
#define DHD_PACKET_LOG_RING_SUSPEND_THRESHOLD	\
	(DHD_PACKET_LOG_RING_PKTS * 8u / 10u)
#define DHD_PACKET_LOG_RING_RESUME_THRESHOLD	\
	(DHD_PACKET_LOG_RING_PKTS * 2u / 10u)
#define DHD_PACKET_LOG_RING_SIZE	\
	(DHD_PACKET_LOG_RING_PKTS * sizeof(dhd_pktlog_ring_info_t))
#endif /* DHD_PKT_LOGGING_DBGRING */

#define ELDBG_RING_STATUS_SIZE (sizeof(eldbg_ring_status_t))

#define VALID_RING(id)	\
	((id > DEBUG_RING_ID_INVALID) && (id < DEBUG_RING_ID_MAX))

#ifdef DEBUGABILITY
#define DBG_RING_ACTIVE(dhdp, ring_id) \
	((dhdp)->dbg && (dhdp)->dbg->dbg_rings[(ring_id)].state == RING_ACTIVE)
#else
#define DBG_RING_ACTIVE(dhdp, ring_id) 0
#endif /* DEBUGABILITY */

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
	EVENT_LOG_STR_RING_ID =	0x2,
	EVENT_LOG_STR_CHATTY_RING_ID =	0x3,
	EVENT_LOG_BIN_RING_ID =	0x4,
	DRIVER_LOG_RING_ID =	0x5,
	PACKET_LOG_RING_ID =	0x6,
	DEBUG_DUMP_RING1_ID =	0x7,
	DEBUG_DUMP_RING2_ID =	0x8,
	MEM_DUMP_RING_ID =	0x9,
	DEBUG_RING_ID_MAX =	0xa
} dhd_ring_id_t;

enum eldbg_ring_state {
	RING_STOP = 0,  /* ring is not initialized */
	RING_ACTIVE,    /* ring is live and logging */
	RING_SUSPEND    /* ring is initialized but not logging */
};

/* each entry in dbg ring has below header, to handle
 * variable length records in ring
 */
typedef struct eldbg_ring_entry {
	uint16 len; /* payload length excluding the header */
	uint8 flags;
	uint8 type; /* Per ring specific */
	uint64 timestamp; /* present if has_timestamp bit is set. */
} PACKED_STRUCT eldbg_ring_entry_t;

struct eldbg_ring_stat {
	/* number of bytes that was written to the buffer by driver */
	uint32 written_bytes;
	/* number of bytes that was read from the buffer by user land */
	uint32 read_bytes;
	/* number of records that was written to the buffer by driver */
	uint32 written_records;
};

typedef struct eldbg_ring_status {
	uint8 name[DBG_RING_NAME_MAX];
	uint32 flags;
	int ring_id; /* unique integer representing the ring */
	/* total memory size allocated for the buffer */
	uint32 ring_buffer_byte_size;
	uint32 verbose_level;
	/* number of bytes that was written to the buffer by driver */
	uint32 written_bytes;
	/* number of bytes that was read from the buffer by user land */
	uint32 read_bytes;
	/* number of records that was read from the buffer by user land */
	uint32 written_records;
} eldbg_ring_status_t;

typedef struct ring_parser_info {
	/* ring parser buffer to parse encoded logs of one event log blocks stored in ring */
	uint8	*parser_buf;
	/* the next pointer of parser buffer to parse the next encoded log in an event log block */
	uint32	parser_pos;
	uint32	parser_buflen;	/* total buf size to prevent buffer overflow */
} ring_parser_info_t;

typedef struct eldbg_ring_cmn {
	int     id;		/* ring id */
	uint8   name[DBG_RING_NAME_MAX]; /* name string */
	uint32  ring_size;	/* numbers of item in ring */
	uint32  wp;		/* write pointer */
	uint32  rp;		/* read pointer */
	uint32  rp_tmp;		/* tmp read pointer */
	uint32  log_level;	/* log_level */
	uint32  threshold;	/* threshold bytes */
	void *  ring_buf;	/* pointer of actually ring buffer */
	void *  lock;		/* lock for ring access */
	struct eldbg_ring_stat stat;	/* statistics */
	enum eldbg_ring_state state;	/* ring state enum */
	bool	tail_padded;	/* writer does not have enough space */
	uint32	rem_len;	/* number of bytes from wp_pad to end */
	bool	sched_pull;	/* schedule reader immediately */
	bool	pull_inactive;	/* pull contents from ring even if it is inactive */
	uint	unit;		/* device instance number */
	/* trace_buf replaces local malloc in ring_proc_read()
	 * and use it when usr size of ring_proc_read is not sufficient.
	 * if usr size is not sufficient, keep the remained data in trace_buf
	 * and send it up in the next call
	 */
	uint8	*trace_buf;	/* buffer to read one event log block */
	uint16	trace_offset;	/* offset to indicate remained data of trace_buf or parser buf */
	uint16	trace_remained_len;	/* save remained len when usr size is not sufficient */
	/* ring parser buffer to parse encoded logs of one event log blocks stored in ring */
	ring_parser_info_t el_ring_parser;
	void *dbg_hdl;		/* dbg handle to be used in ring functions */
} eldbg_ring_cmn_t;

extern uint32 eldbg_ring_element_size[];
#define ELDBG_RING_FLUSH_THRESHOLD(ring)	\
	(ring->ring_size / eldbg_ring_element_size[ring->id - 1])

#define ELDBG_RING_ENTRY_SIZE	(sizeof(eldbg_ring_entry_t))
#define ELDBG_ENTRY_LENGTH(hdr)	((hdr)->len + ELDBG_RING_ENTRY_SIZE)
#define ELDBG_PAYLOAD_MAX_LEN	65535u
#define ELDBG_PENDING_LEN_MAX	0xFFFFFFFFu
#define ELDBG_RING_STATUS_SIZE	(sizeof(eldbg_ring_status_t))

#define ELDBG_TXACTIVESZ(r, w, d)	(((r) <= (w)) ? ((w) - (r)) : ((d) - (r) + (w)))
#define ELDBG_RING_READ_AVAIL_SPACE(w, r, d) \
		(((w) >= (r)) ? ((w) - (r)) : ((d) - (r)))
#define ELDBG_RING_WRITE_SPACE_AVAIL_CONT(r, w, d) \
		(((w) >= (r)) ? ((d) - (w)) : ((r) - (w)))
#define ELDBG_RING_WRITE_SPACE_AVAIL(r, w, d)	(d - (ELDBG_TXACTIVESZ(r, w, d)))
#define ELDBG_RING_CHECK_WRITE_SPACE(r, w, d) \
	MIN(ELDBG_RING_WRITE_SPACE_AVAIL(r, w, d), ELDBG_RING_WRITE_SPACE_AVAIL_CONT(r, w, d))

typedef void (*os_pullreq_t)(void *os_priv, const int ring_id);
int eldbg_ring_set_buf(osl_t *osh, eldbg_ring_cmn_t *ring, void *buf);
#ifdef DHD_PKT_LOGGING_DBGRING
int eldbg_ring_update(void *dbg_ring, uint32 w_len);
#endif /* DHD_PKT_LOGGING_DBGRING */
int eldbg_ring_push(eldbg_ring_cmn_t *ring, eldbg_ring_entry_t *hdr, void *data);
int eldbg_ring_pull(eldbg_ring_cmn_t *ring, void *data, uint32 buf_len,
		bool strip_hdr);
int eldbg_ring_pull_single(eldbg_ring_cmn_t *ring, void *data, uint32 buf_len,
	bool strip_header);
uint32 eldbg_ring_get_pending_len(eldbg_ring_cmn_t *ring);
void eldbg_ring_sched_pull(eldbg_ring_cmn_t *ring, uint32 pending_len,
		os_pullreq_t pull_fn, void *os_pvt, const int id);
int eldbg_ring_config(eldbg_ring_cmn_t *ring, int log_level, uint32 threshold);
void eldbg_ring_start(eldbg_ring_cmn_t *ring);

int __eldbg_get_ring_status(eldbg_ring_cmn_t *ring, eldbg_ring_status_t *ring_status);
#ifdef SHOW_LOGTRACE
void eldbg_read_ring_into_trace_buf(eldbg_ring_cmn_t *ring, trace_buf_info_t *trace_buf_info);
#endif /* SHOW_LOGTRACE */
#endif /* __ELDBG_RING_H__ */
