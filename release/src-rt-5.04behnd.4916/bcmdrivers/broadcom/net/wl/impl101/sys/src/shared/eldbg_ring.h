/*
 * DHD and NIC debug ring header file - interface
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

#ifndef __ELDBG_RING_H__
#define __ELDBG_RING_H__

#include <bcmutils.h>

#if defined(__linux__)
#define PACKED_STRUCT __attribute__ ((packed))
#else
#define PACKED_STRUCT
#endif

#define DBG_RING_NAME_MAX 32

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
	bool tail_padded;	/* writer does not have enough space */
	uint32 rem_len;		/* number of bytes from wp_pad to end */
	bool sched_pull;	/* schedule reader immediately */
	bool pull_inactive;	/* pull contents from ring even if it is inactive */
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

#ifdef EVENT_LOG_NIC
int eldbg_ring_attach(void *hdl);
void eldbg_ring_detach(void *hdl);
#endif /* EVENT_LOG_NIC */

eldbg_ring_cmn_t *eldbg_ring_alloc_init(osl_t *osh, uint16 ring_id,
	char *ring_name, uint32 ring_sz, void *allocd_buf,
	bool pull_inactive);
void eldbg_ring_dealloc_deinit(void **dbgring, osl_t *osh);
int eldbg_ring_init(osl_t *osh, eldbg_ring_cmn_t *ring, uint16 id, uint8 *name,
		uint32 ring_sz, void *allocd_buf, bool pull_inactive);
void eldbg_ring_deinit(osl_t *osh, eldbg_ring_cmn_t *ring);
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
#endif /* __ELDBG_RING_H__ */
