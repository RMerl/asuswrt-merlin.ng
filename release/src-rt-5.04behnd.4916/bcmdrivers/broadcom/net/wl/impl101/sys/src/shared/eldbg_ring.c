/*
 * DHD and NIC debug ring API and structures - implementation
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: $
 */
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmstdlib_s.h>
#include <bcmendian.h>
#ifdef EVENT_LOG_NIC
#include <wl_linux.h>
#include <wl_dbg.h>
#else
#include <dhd.h>
#include <dhd_dbg.h>
#include <dhd_debug.h>
#endif /* EVENT_LOG_NIC */
#include <eldbg_ring.h>

#ifdef EVENT_LOG_NIC
#define DBG_RING_LOCK_INIT(osh)		osl_spin_lock_init(osh)
#define DBG_RING_LOCK_DEINIT(osh, lock)	osl_spin_lock_deinit(osh, (lock))
#define DBG_RING_LOCK(lock, flags)	((flags) = osl_spin_lock(lock))
#define DBG_RING_UNLOCK(lock, flags)	osl_spin_unlock((lock), flags)
#else
#define DBG_RING_LOCK_INIT(osh)		dhd_os_spin_lock_init(osh)
#define DBG_RING_LOCK_DEINIT(osh, lock)	dhd_os_spin_lock_deinit(osh, (lock))
#define DBG_RING_LOCK(lock, flags)	((flags) = dhd_os_spin_lock(lock))
#define DBG_RING_UNLOCK(lock, flags)	dhd_os_spin_unlock((lock), flags)
#endif /* EVENT_LOG_NIC */

#ifdef EVENT_LOG_NIC
#define RING_ERROR(x)			WL_ERROR(x)
#define RING_DBGIF(x)			WL_INFORM(x)
#else
#define RING_ERROR(x)			DHD_ERROR(x)
#define RING_DBGIF(x)			DHD_DBGIF(x)
#endif /* EVENT_LOG_NIC */

#ifdef EVENT_LOG_NIC
#define HDL_PTR(x)			((wl_info_t *)(x))
#else
#define HDL_PTR(x)			((dhd_pub_t *)(x))
#endif /* EVENT_LOG_NIC */

#define OSH_PTR(x)			(osl_t *)(HDL_PTR(x)->osh)
#define DBG_PTR(x)			(eldbg_cmn_t *)(HDL_PTR(x)->dbg)

uint32 eldbg_ring_element_size[DEBUG_RING_ID_MAX] = {3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3};

#ifdef EVENT_LOG_NIC
/* eldbg_ring_attach will be used for dhd also after porting dhd_debug.c */
int
eldbg_ring_attach(void *hdl)
{
	int ret = BCME_OK;
	void *buf = NULL;
	uint ring_size;
	int ring_id = 0;
	eldbg_ring_cmn_t *ring = NULL;
	osl_t *osh = NULL;
	eldbg_cmn_t *dbg = (eldbg_cmn_t *)hdl;

	if (dbg == NULL || dbg->osh == NULL) {
		ASSERT(dbg);
		return BCME_BADARG;
	}

	osh = dbg->osh;

#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(DEBUGABILITY)
#ifdef EVENT_LOG_NIC
	ring_size = 2u * FW_VERBOSE_RING_SIZE;
#else
	ring_size = FW_VERBOSE_RING_SIZE;
#endif /* EVENT_LOG_NIC */

	buf = VMALLOCZ(osh, ring_size);

	if (!buf) {
		RING_ERROR(("%s:%d: VMALLOC failed for fw_verbose_ring, size %d\n",
			__FUNCTION__, __LINE__, ring_size));
		ret = BCME_NOMEM;
		goto exit;
	}

	ret = eldbg_ring_init(osh, &dbg->dbg_rings[FW_VERBOSE_RING_ID],
		FW_VERBOSE_RING_ID, (uint8 *)FW_VERBOSE_RING_NAME,
		ring_size, buf, TRUE);

	if (ret) {
		goto exit;
	}
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || DEBUGABILITY  */

	return ret;

exit:

#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	for (ring_id = DEBUG_RING_ID_INVALID + 1; ring_id < DEBUG_RING_ID_MAX; ring_id++) {
		if (VALID_RING(dbg->dbg_rings[ring_id].id)) {
			ring = &dbg->dbg_rings[ring_id];
			eldbg_ring_deinit(osh, ring);
			if (ring->ring_buf) {
#ifdef DHD_PKT_LOGGING_DBGRING
				if (ring_id != PACKET_LOG_RING_ID)
#endif /* DHD_PKT_LOGGING_DBGRING */
				{
					VMFREE(osh, ring->ring_buf, ring->ring_size);
				}
				ring->ring_buf = NULL;
			}
			ring->ring_size = 0;
		}
	}
#endif /* EVENT_LOG_ACCESS ||| DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/

	return ret;
}

/* eldbg_ring_detach will be used for dhd also after porting dhd_debug.c */
void eldbg_ring_detach(void *hdl)
{
#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	int ring_id;
	eldbg_ring_cmn_t *ring = NULL;
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/
	eldbg_cmn_t *dbg = (eldbg_cmn_t *)hdl;

	if (dbg == NULL || dbg->osh == NULL) {
		ASSERT(dbg);
		return;
	}

#if defined(EVENT_LOG_ACCESS) || defined(DHD_DEBUGABILITY_LOG_DUMP_RING) || \
	defined(BTLOG) || defined(DHD_DEBUGABILITY_EVENT_RING) || \
	defined(DHD_PKT_LOGGING_DBGRING) || defined(DEBUGABILITY)
	for (ring_id = DEBUG_RING_ID_INVALID + 1; ring_id < DEBUG_RING_ID_MAX; ring_id++) {
		if (VALID_RING(dbg->dbg_rings[ring_id].id)) {
			ring = &dbg->dbg_rings[ring_id];
			eldbg_ring_deinit(dbg->osh, ring);
			if (ring->ring_buf) {
#ifdef DHD_PKT_LOGGING_DBGRING
				if (ring_id != PACKET_LOG_RING_ID)
#endif /* DHD_PKT_LOGGING_DBGRING */
				{
					VMFREE(dbg->osh, ring->ring_buf, ring->ring_size);
				}
				ring->ring_buf = NULL;
			}
			ring->ring_size = 0;
		}
	}
#endif /* EVENT_LOG_ACCESS || DHD_DEBUGABILITY_LOG_DUMP_RING || BTLOG ||
	* DHD_DEBUGABILITY_EVENT_RING || DHD_PKT_LOGGING_DBGRING || DEBUGABILITY
	*/

	return;
}

#endif /* EVENT_LOG_NIC */

eldbg_ring_cmn_t *
eldbg_ring_alloc_init(osl_t *osh, uint16 ring_id,
	char *ring_name, uint32 ring_sz, void *allocd_buf,
	bool pull_inactive)
{
	eldbg_ring_cmn_t *ring = NULL;
	int ret = 0;
	unsigned long flags = 0;

	ring = MALLOCZ(osh, sizeof(eldbg_ring_cmn_t));
	if (!ring)
		goto fail;

	ret = eldbg_ring_init(osh, ring, ring_id,
			(uint8 *)ring_name, ring_sz,
			allocd_buf, pull_inactive);
	if (ret != BCME_OK) {
		RING_ERROR(("%s: unable to init ring %s!\n",
				__FUNCTION__, ring_name));
		goto fail;
	}
	DBG_RING_LOCK(ring->lock, flags);
	ring->state = RING_ACTIVE;
	ring->threshold = 0;
	DBG_RING_UNLOCK(ring->lock, flags);

	return ring;

fail:
	if (ring) {
		eldbg_ring_deinit(osh, ring);
		ring->ring_buf = NULL;
		ring->ring_size = 0;
		MFREE(osh, ring, sizeof(eldbg_ring_cmn_t));
	}
	return NULL;
}

void
eldbg_ring_dealloc_deinit(void **ring_ptr, osl_t *osh)
{
	eldbg_ring_cmn_t *ring = NULL;
	eldbg_ring_cmn_t **dbgring = (eldbg_ring_cmn_t **)ring_ptr;

	if (!dbgring)
		return;

	ring = *dbgring;

	if (ring) {
		eldbg_ring_deinit(osh, ring);
		ring->ring_buf = NULL;
		ring->ring_size = 0;
		MFREE(osh, ring, sizeof(eldbg_ring_cmn_t));
		*dbgring = NULL;
	}
}

int
eldbg_ring_init(osl_t *osh, eldbg_ring_cmn_t *ring, uint16 id, uint8 *name,
		uint32 ring_sz, void *allocd_buf, bool pull_inactive)
{
	void *buf;
	unsigned long flags = 0;

	if (allocd_buf == NULL) {
		/* for DEBUG_DUMP and MEM_DUMP, buffer can be NULL
		 * since act as delayed allocation or fake rings
		 */
		if (id != DEBUG_DUMP_RING1_ID && id != DEBUG_DUMP_RING2_ID &&
				id != MEM_DUMP_RING_ID) {
			return BCME_NOMEM;
		}
		buf = NULL;
	} else {
		buf = allocd_buf;
	}

	ring->lock = DBG_RING_LOCK_INIT(osh);

	DBG_RING_LOCK(ring->lock, flags);
	ring->id = id;
	strlcpy((char *)ring->name, (char *)name, sizeof(ring->name));
	ring->ring_size = ring_sz;
	ring->wp = ring->rp = 0;
	ring->ring_buf = buf;
	ring->threshold = ELDBG_RING_FLUSH_THRESHOLD(ring);
	ring->state = RING_ACTIVE;
	ring->rem_len = 0;
	ring->sched_pull = TRUE;
	ring->pull_inactive = pull_inactive;
	DBG_RING_UNLOCK(ring->lock, flags);

	return BCME_OK;
}

void
eldbg_ring_deinit(osl_t *osh, eldbg_ring_cmn_t *ring)
{
	unsigned long flags = 0;

	DBG_RING_LOCK(ring->lock, flags);
	ring->id = 0;
	ring->name[0] = 0;
	ring->wp = ring->rp = 0;
	bzero(&ring->stat, sizeof(ring->stat));
	ring->threshold = 0;
	ring->state = RING_STOP;
	DBG_RING_UNLOCK(ring->lock, flags);

	DBG_RING_LOCK_DEINIT(osh, ring->lock);
}

int
eldbg_ring_set_buf(osl_t *osh, eldbg_ring_cmn_t *ring, void *buf)
{
	unsigned long flags = 0;

	DBG_RING_LOCK(ring->lock, flags);
	ring->ring_buf = buf;
	DBG_RING_UNLOCK(ring->lock, flags);

	return BCME_OK;
}

void
eldbg_ring_sched_pull(eldbg_ring_cmn_t *ring, uint32 pending_len,
		os_pullreq_t pull_fn, void *os_pvt, const int id)
{
	unsigned long flags = 0;

	if (pull_fn == NULL) {
		return;
	}

	DBG_RING_LOCK(ring->lock, flags);
	/* if the current pending size is bigger than threshold and
	 * threshold is set
	 */
	if (ring->threshold > 0 &&
	   (pending_len >= ring->threshold) && ring->sched_pull) {
		/*
		 * Update the state and release the lock before calling
		 * the pull_fn. Do not transfer control to other layers
		 * with locks held. If the call back again calls into
		 * the same layer fro this context, can lead to deadlock.
		 */
		ring->sched_pull = FALSE;
		DBG_RING_UNLOCK(ring->lock, flags);
		pull_fn(os_pvt, id);
	} else {
		DBG_RING_UNLOCK(ring->lock, flags);
	}
}

uint32
eldbg_ring_get_pending_len(eldbg_ring_cmn_t *ring)
{
	uint32 pending_len = 0;
	unsigned long flags = 0;

	DBG_RING_LOCK(ring->lock, flags);
	if (ring->stat.written_bytes > ring->stat.read_bytes) {
		pending_len = ring->stat.written_bytes - ring->stat.read_bytes;
	} else if (ring->stat.written_bytes < ring->stat.read_bytes) {
		pending_len = ELDBG_PENDING_LEN_MAX - ring->stat.read_bytes +
			ring->stat.written_bytes;
	} else {
		pending_len = 0;
	}
	DBG_RING_UNLOCK(ring->lock, flags);

	return pending_len;
}

#ifdef DHD_PKT_LOGGING_DBGRING
int
eldbg_ring_update(void *dbg_ring, uint32 w_len)
{
	unsigned long flags;
	eldbg_ring_cmn_t *ring = (eldbg_ring_cmn_t *)dbg_ring;

	if (ring->id != PACKET_LOG_RING_ID) {
		return BCME_UNSUPPORTED;
	}

	DBG_RING_LOCK(ring->lock, flags);

	if (ring->state != RING_ACTIVE) {
		DBG_RING_UNLOCK(ring->lock, flags);
		return BCME_OK;
	}

	/* update statistics */
	ring->stat.written_records++;
	ring->stat.written_bytes += w_len;
	RING_DBGIF(("%s : EL_%d[%s] written_records %d, written_bytes %d, read_bytes=%d,"
		" ring->threshold=%d, wp=%d, rp=%d\n", __FUNCTION__, ring->id, ring->name,
		ring->stat.written_records, ring->stat.written_bytes, ring->stat.read_bytes,
		ring->threshold, ring->wp, ring->rp));

	DBG_RING_UNLOCK(ring->lock, flags);

	return BCME_OK;
}
#endif /* DHD_PKT_LOGGING_DBGRING */

int
eldbg_ring_push(eldbg_ring_cmn_t *ring, eldbg_ring_entry_t *hdr, void *data)
{
	unsigned long flags;
	uint32 w_len;
	uint32 avail_size;
	eldbg_ring_entry_t *w_entry, *r_entry;
	int ret;

	if (!ring || !hdr || !data) {
		return BCME_BADARG;
	}
	BCM_REFERENCE(ret);

#if defined(__linux__)
	/* Prevents the case of accessing the ring buffer in the HardIRQ context.
	 * If an interrupt arise after holding ring lock, It could try the same lock.
	 * This is to use the ring lock as spin_lock_bh instead of spin_lock_irqsave.
	 */
	if (in_irq()) {
		return BCME_BUSY;
	}
#endif /* defined(__linux__) */

	DBG_RING_LOCK(ring->lock, flags);

	if (ring->state != RING_ACTIVE) {
		DBG_RING_UNLOCK(ring->lock, flags);

		return BCME_OK;
	}

	w_len = ELDBG_ENTRY_LENGTH(hdr);

	RING_DBGIF(("%s: EL_%d[%s] hdr->len=%u, w_len=%u, wp=%d, rp=%d, ring_start=0x%p;"
		" ring_size=%u\n",
		__FUNCTION__, ring->id, ring->name, hdr->len, w_len, ring->wp, ring->rp,
		ring->ring_buf, ring->ring_size));

	if (w_len > ring->ring_size) {
		DBG_RING_UNLOCK(ring->lock, flags);
		RING_DBGIF(("%s: EL_%d[%s] w_len=%u, ring_size=%u,"
			" write size exceeds ring size !\n",
			__FUNCTION__, ring->id, ring->name, w_len, ring->ring_size));
		return BCME_ERROR;
	}
	/* Claim the space */
	do {
		avail_size = ELDBG_RING_CHECK_WRITE_SPACE(ring->rp, ring->wp, ring->ring_size);
		if (avail_size <= w_len) {
			/* Prepare the space */
			if (ring->rp <= ring->wp) {
				ring->tail_padded = TRUE;
				ring->rem_len = ring->ring_size - ring->wp;
				RING_DBGIF(("%s: EL_%d[%s] Insuffient tail space,"
					" rp=%d, wp=%d, rem_len=%d, ring_size=%d,"
					" avail_size=%d, w_len=%d\n", __FUNCTION__,
					ring->id, ring->name, ring->rp, ring->wp,
					ring->rem_len, ring->ring_size, avail_size,
					w_len));

				/* 0 pad insufficient tail space */
				bzero((uint8 *)ring->ring_buf + ring->wp, ring->rem_len);
				/* If read pointer is still at the beginning, make some room */
				if (ring->rp == 0) {
					r_entry = (eldbg_ring_entry_t *)((uint8 *)ring->ring_buf +
						ring->rp);
					ring->rp += ELDBG_ENTRY_LENGTH(r_entry);
					ring->stat.read_bytes += ELDBG_ENTRY_LENGTH(r_entry);
					RING_DBGIF(("%s: rp at 0, move by one entry length"
						" (%u bytes)\n",
						__FUNCTION__, (uint32)ELDBG_ENTRY_LENGTH(r_entry)));
				}
				if (ring->rp == ring->wp) {
					ring->rp = 0;
				}
				ring->wp = 0;
				RING_DBGIF(("%s: new rp=%u, wp=%u\n",
					__FUNCTION__, ring->rp, ring->wp));
			} else {
				/* Not enough space for new entry, free some up */
				r_entry = (eldbg_ring_entry_t *)((uint8 *)ring->ring_buf +
					ring->rp);
				/* check bounds before incrementing read ptr */
				if (ring->rp + ELDBG_ENTRY_LENGTH(r_entry) >= ring->ring_size) {
					RING_DBGIF(("%s: EL_%d[%s] rp points out of boundary,"
						"ring->wp=%u, ring->rp=%u, ring->ring_size=%d\n",
						__FUNCTION__, ring->id, ring->name, ring->wp,
						ring->rp, ring->ring_size));
					ASSERT(0);
					DBG_RING_UNLOCK(ring->lock, flags);
					return BCME_BUFTOOSHORT;
				}
				ring->rp += ELDBG_ENTRY_LENGTH(r_entry);
				/* skip padding if there is one */
				if (ring->tail_padded &&
					((ring->rp + ring->rem_len) == ring->ring_size)) {
					RING_DBGIF(("%s: EL_%d[%s] Found padding,"
						" avail_size=%d, w_len=%d, set rp = 0\n",
						__FUNCTION__,
						ring->id, ring->name, avail_size, w_len));
					ring->rp = 0;
					ring->tail_padded = FALSE;
					ring->rem_len = 0;
				}
				ring->stat.read_bytes += ELDBG_ENTRY_LENGTH(r_entry);
				RING_DBGIF(("%s: EL_%d[%s] read_bytes=%d, wp=%d, rp=%d\n",
					__FUNCTION__, ring->id, ring->name, ring->stat.read_bytes,
					ring->wp, ring->rp));
			}
		} else {
			break;
		}
	} while (TRUE);

	/* check before writing to the ring */
	if (ring->wp + w_len >= ring->ring_size) {
		RING_ERROR(("%s: EL_%d[%s] wp pointed out of ring boundary, "
			"wp=%d, ring_size=%d, w_len=%u\n", __FUNCTION__, ring->id,
			ring->name, ring->wp, ring->ring_size, w_len));
		ASSERT(0);
		DBG_RING_UNLOCK(ring->lock, flags);
		return BCME_BUFTOOLONG;
	}

	w_entry = (eldbg_ring_entry_t *)((uint8 *)ring->ring_buf + ring->wp);
	/* header */
	ret = memcpy_s(w_entry, avail_size, hdr, ELDBG_RING_ENTRY_SIZE);
	if (ret) {
		RING_ERROR((" memcpy_s() error : %d, destsz: %d, n: %d\n",
			ret, avail_size, (int)ELDBG_RING_ENTRY_SIZE));
		return BCME_ERROR;
	}
	w_entry->len = hdr->len;
	/* payload */
	avail_size -= ELDBG_RING_ENTRY_SIZE;
	ret = memcpy_s((char *)w_entry + ELDBG_RING_ENTRY_SIZE,
		avail_size, data, w_entry->len);
	if (ret) {
		RING_ERROR((" memcpy_s() error : %d, destsz: %d, n: %d\n",
			ret, avail_size, w_entry->len));
		return BCME_ERROR;
	}
	/* update write pointer */
	ring->wp += w_len;

	/* update statistics */
	ring->stat.written_records++;
	ring->stat.written_bytes += w_len;
	RING_DBGIF(("%s : EL_%d[%s] written_records %d, written_bytes %d, read_bytes=%d,"
		" ring->threshold=%d, wp=%d, rp=%d\n", __FUNCTION__, ring->id, ring->name,
		ring->stat.written_records, ring->stat.written_bytes, ring->stat.read_bytes,
		ring->threshold, ring->wp, ring->rp));

	DBG_RING_UNLOCK(ring->lock, flags);
	return BCME_OK;
}

/*
 * This function folds ring->lock, so callers of this function
 * should not hold ring->lock.
 */
int
eldbg_ring_pull_single(eldbg_ring_cmn_t *ring, void *data, uint32 buf_len, bool strip_header)
{
	eldbg_ring_entry_t *r_entry = NULL;
	uint32 rlen = 0;
	char *buf = NULL;
	unsigned long flags;

	if (!ring || !data || buf_len <= 0) {
		return 0;
	}

	DBG_RING_LOCK(ring->lock, flags);
	/* pull from ring is allowed for inactive (suspended) ring
	 * in case of ecounters only, this is because, for ecounters
	 * when a trap occurs the ring is suspended and data is then
	 * pulled to dump it to a file. For other rings if ring is
	 * not in active state return without processing (as before)
	 */
	if (!ring->pull_inactive && (ring->state != RING_ACTIVE)) {
		goto exit;
	}

	if (ring->rp == ring->wp) {
		goto exit;
	}

	RING_DBGIF(("%s: EL_%d[%s] buf_len=%u, wp=%d, rp=%d, ring_start=0x%p; ring_size=%u\n",
		__FUNCTION__, ring->id, ring->name, buf_len, ring->wp, ring->rp,
		ring->ring_buf, ring->ring_size));

	r_entry = (eldbg_ring_entry_t *)((uint8 *)ring->ring_buf + ring->rp);

	/* Boundary Check */
	rlen = ELDBG_ENTRY_LENGTH(r_entry);
	if ((ring->rp + rlen) > ring->ring_size) {
		RING_DBGIF(("%s: entry len %d is out of boundary of ring size %d,"
			" current ring %d[%s] - rp=%d\n", __FUNCTION__, rlen,
			ring->ring_size, ring->id, ring->name, ring->rp));
		rlen = 0;
		goto exit;
	}

	if (strip_header) {
		rlen = r_entry->len;
		buf = (char *)r_entry + ELDBG_RING_ENTRY_SIZE;
	} else {
		rlen = ELDBG_ENTRY_LENGTH(r_entry);
		buf = (char *)r_entry;
	}
	if (rlen > buf_len) {
		RING_DBGIF(("%s: buf len %d is too small for entry len %d\n",
			__FUNCTION__, buf_len, rlen));
		RING_DBGIF(("%s: ring %d[%s] - ring size=%d, wp=%d, rp=%d\n",
			__FUNCTION__, ring->id, ring->name, ring->ring_size,
			ring->wp, ring->rp));
		/* The state of ringbuffer is different between calculating buf_len
		 * and current. ring->rp have chance to be update by pushing data
		 * to ring buffer when unlocking after calculating buf_len.
		 * But, It doesn't need to ASSERT because we only send up the
		 * entries stored so far.
		 */

		rlen = 0;
		goto exit;
	}

	memcpy(data, buf, rlen);
	/* update ring context */
	ring->rp += ELDBG_ENTRY_LENGTH(r_entry);
	/* don't pass wp but skip padding if there is one */
	if (ring->rp != ring->wp &&
	    ring->tail_padded && ((ring->rp + ring->rem_len) >= ring->ring_size)) {
		RING_DBGIF(("%s: EL_%d[%s] Found padding, rp=%d, wp=%d\n",
			__FUNCTION__, ring->id, ring->name, ring->rp, ring->wp));
		ring->rp = 0;
		ring->tail_padded = FALSE;
		ring->rem_len = 0;
	}
	if (ring->rp >= ring->ring_size) {
		RING_DBGIF(("%s: EL_%d[%s] rp pointed out of ring boundary,"
			" rp=%d, ring_size=%d\n", __FUNCTION__, ring->id,
			ring->name, ring->rp, ring->ring_size));
		ASSERT(0);
		rlen = 0;
		goto exit;
	}
	ring->stat.read_bytes += ELDBG_ENTRY_LENGTH(r_entry);
	RING_DBGIF(("%s EL_%d[%s]read_bytes %d, wp=%d, rp=%d\n", __FUNCTION__,
		ring->id, ring->name, ring->stat.read_bytes, ring->wp, ring->rp));

exit:
	DBG_RING_UNLOCK(ring->lock, flags);

	return rlen;
}

int
eldbg_ring_pull(eldbg_ring_cmn_t *ring, void *data, uint32 buf_len, bool strip_hdr)
{
	int32 r_len, total_r_len = 0;
	unsigned long flags;

	if (!ring || !data)
		return 0;

	DBG_RING_LOCK(ring->lock, flags);
	if (!ring->pull_inactive && (ring->state != RING_ACTIVE)) {
		DBG_RING_UNLOCK(ring->lock, flags);
		return 0;
	}
	DBG_RING_UNLOCK(ring->lock, flags);

	while (buf_len > 0) {
		r_len = eldbg_ring_pull_single(ring, data, buf_len, strip_hdr);
		if (r_len == 0)
			break;
		data = (uint8 *)data + r_len;
		buf_len -= r_len;
		total_r_len += r_len;
	}

	return total_r_len;
}

int
eldbg_ring_config(eldbg_ring_cmn_t *ring, int log_level, uint32 threshold)
{
	unsigned long flags = 0;

	if (!ring)
		return BCME_BADADDR;

	if (ring->state == RING_STOP)
		return BCME_UNSUPPORTED;

	DBG_RING_LOCK(ring->lock, flags);

	if (log_level == 0)
		ring->state = RING_SUSPEND;
	else
		ring->state = RING_ACTIVE;

	ring->log_level = log_level;
	ring->threshold = MIN(threshold, ELDBG_RING_FLUSH_THRESHOLD(ring));

	DBG_RING_UNLOCK(ring->lock, flags);

	return BCME_OK;
}

void
eldbg_ring_start(eldbg_ring_cmn_t *ring)
{
	if (!ring)
		return;

	/* Initialize the information for the ring */
	ring->state = RING_SUSPEND;
	ring->log_level = 0;
	ring->rp = ring->wp = 0;
	ring->threshold = 0;
	bzero(&ring->stat, sizeof(struct eldbg_ring_stat));
	bzero(ring->ring_buf, ring->ring_size);
}
