/*
 * Broadcom DHD and NIC driver, provided /proc/xyz interface to read contents of
 * debug messages stored in internal circural buffers(Rings$)
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
#include <linux/proc_fs.h>
#include <osl.h>
#include <linux_dbg.h>
#include <bcmstdlib_s.h>
#include <event_log.h>

#ifndef DUMP_FILENAME_SZ
#define DUMP_FILENAME_SZ     40
#endif

#ifdef SHOW_LOGTRACE

#define PROC_NAME_EL_STR	"el_string"
#define PROC_NAME_EL_STR_CHATTY	"el_string_chatty"
#define PROC_NAME_EL_BIN	"el_binary"

uint8 control_logtrace = CUSTOM_CONTROL_LOGTRACE;

static int ring_proc_open(struct inode *inode, struct file *file);
ssize_t ring_proc_read(struct file *file, char *buffer, size_t usrsz, loff_t *loff);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0))
static const struct file_operations dhd_ring_proc_ops = {
	.open = ring_proc_open,
	.read = ring_proc_read,
	.release = single_release,
};
#else
static const struct proc_ops dhd_ring_proc_ops = {
	.proc_open = ring_proc_open,
	.proc_read = ring_proc_read,
	.proc_release = single_release,
};
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0) */

#ifdef EWP_EVENTTS_LOG
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0))
static const struct file_operations dhd_ring_eventts_proc_ops = {
	.open = ring_proc_open,
	.read = ring_eventts_proc_read,
	.release = single_release,
};
#else
static const struct proc_ops dhd_ring_eventts_proc_ops = {
	.proc_open = ring_proc_open,
	.proc_read = ring_eventts_proc_read,
	.proc_release = single_release,
};
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0) */
#endif /* EWP_EVENTTS_LOG */

static int
ring_proc_open(struct inode *inode, struct file *file)
{
	int ret = BCME_ERROR;
	if (inode) {
		ret = single_open(file, 0, pde_data(inode));
	} else {
		DBG_ERROR(("%s: inode is NULL\n", __FUNCTION__));
	}
	return ret;
}

ssize_t
ring_proc_read(struct file *file, char __user *buffer, size_t usrsz, loff_t *loff)
{
	/* static variable to use buf pointer to handle remaining data
	 * if usrsz is smaller than the size read, remaining data is delivered on the next call
	 * trace_buf_info.buf is updated to:
	 *	1. ring->trace_buf to read one event log block from proc fs
	 *	if it is required to parse event log block of encoded string data,
	 *	2. ring->el_ring_parser to write parsed log from ring->trace_buf
	 */
	static trace_buf_info_t trace_buf_info;
	int ret = BCME_ERROR;
	eldbg_ring_cmn_t *ring =
		(eldbg_ring_cmn_t *)((struct seq_file *)(file->private_data))->private;
	int32 cp2usr_len = 0;
	eldbg_cmn_t *dbg;

	if (ring == NULL) {
		DBG_ERROR(("%s: ring is NULL\n", __FUNCTION__));
		return -EFAULT;
	}

	dbg = (eldbg_cmn_t *)ring->dbg_hdl;

	if (ring->trace_remained_len) {
		DBG_INFO(("%s: trace_remained_len %d, usrsz %d, offset %d\n",
			__FUNCTION__, ring->trace_remained_len, (int)usrsz, ring->trace_offset));

		if (ring->trace_remained_len <= usrsz) {
			cp2usr_len = ring->trace_remained_len;
			ring->trace_remained_len = 0;
		} else {
			/* if usrsz is too small, send up usrsz only and wait for the next call */
			cp2usr_len = usrsz;
			ring->trace_remained_len = ring->trace_remained_len - usrsz;
		}

		if (copy_to_user(buffer, (void*)(trace_buf_info.buf + ring->trace_offset),
			cp2usr_len)) {
			DBG_ERROR(("%s: copy_to_user failure from trace_buf \n", __FUNCTION__));
			return -EFAULT;
		}

		if (ring->trace_remained_len) {
			/* update offset after sending up and it will be sent up in the next call */
			ring->trace_offset += cp2usr_len;
		} else {
			ring->trace_offset = 0;
		}

		DBG_INFO(("%s: usrsz %d, copied len %d, new remained_len %d, new offset %d \n",
			__FUNCTION__, (int)usrsz, cp2usr_len, ring->trace_remained_len,
			ring->trace_offset));

		return cp2usr_len;
	}

	/* use trace_buf of ring to read event log block from ring of proc fs */
	trace_buf_info.buf = ring->trace_buf;
	if (trace_buf_info.buf == NULL) {
		DBG_ERROR(("%s: buf is not allocated\n", __FUNCTION__));
		return -EFAULT;
	}

	/* read one entry of ring */
	eldbg_read_ring_into_trace_buf(ring, &trace_buf_info);

	/* if parser function exits, try to parse event log block */
	if ((dbg->func_parser) && (trace_buf_info.size >
		(sizeof(struct tracelog_header) + sizeof(event_log_hdr_t)))) {
		char *data = trace_buf_info.buf;
		uint32 datalen = trace_buf_info.size;
		uint32 all_parsed_len = 0;

		/* move pointer to event log block header */
		data += sizeof(struct tracelog_header);
		datalen -= sizeof(struct tracelog_header);

		if ((datalen - EVENT_LOG_BLOCK_HDRLEN) > sizeof(event_log_hdr_t)) {
			all_parsed_len = dbg->func_parser(dbg, data, datalen,
				&ring->el_ring_parser);
			if (all_parsed_len > 0) {
				/* if parsed logs exists, replace buf pointer to parser_buf */
				ring->el_ring_parser.parser_pos = 0;
				trace_buf_info.size = all_parsed_len;
				trace_buf_info.buf = ring->el_ring_parser.parser_buf;
			}
		} else {
			DBG_INFO(("%s: read datalen is too short %d\n", __FUNCTION__, datalen));
		}
	}

	if (trace_buf_info.size > usrsz) {
		/* dest size is not sufficient. prepare to send it up on the next call */
		ring->trace_remained_len = (trace_buf_info.size - usrsz);
		ring->trace_offset = usrsz;
		cp2usr_len = usrsz;
		DBG_INFO(("%s: insufficient usrsz %d, remained_len %d, offset %d\n",
			__FUNCTION__, (uint32)usrsz,
			ring->trace_remained_len, ring->trace_offset));
	} else {
		ring->trace_remained_len = 0;
		ring->trace_offset = 0;
		cp2usr_len = trace_buf_info.size;
	}

	DBG_INFO(("%s: read size %d, trace_remained_len %d, usrsz %d , cp2usr_len %d, avail %d\n",
		__FUNCTION__, trace_buf_info.size, ring->trace_remained_len, (uint32)usrsz,
		cp2usr_len, trace_buf_info.availability));

	if (copy_to_user(buffer, (void*)trace_buf_info.buf,  cp2usr_len))
	{
		DBG_ERROR(("%s: copy_to_user failure \n", __FUNCTION__));
		return -EFAULT;
	}
	if (trace_buf_info.availability == BUF_NOT_AVAILABLE) {
		ret = BUF_NOT_AVAILABLE;
	} else {
		ret = cp2usr_len;
	}

	return ret;
}

#ifdef EWP_EVENTTS_LOG
ssize_t
ring_eventts_proc_read(struct file *file, char __user *usrbuf, size_t usrsz, loff_t *loff)
{
	char tmpbuf[TRACE_LOG_BUF_MAX_SIZE] = {0};
	int ret = 0;
	int rlen = 0;
	eldbg_ring_cmn_t *ring = NULL;

	ring = (eldbg_ring_cmn_t *)((struct seq_file *)(file->private_data))->private;
	if (ring == NULL) {
		DBG_ERROR(("%s: ring is NULL!\n", __FUNCTION__));
		return -EFAULT;
	}

	rlen = eldbg_ring_pull_single(ring, tmpbuf, sizeof(tmpbuf), TRUE);
	if (!rlen) {
		/* rlen can also be zero when there is no data in the ring */
		DBG_INFO(("%s: eldbg_ring_pull_single, rlen=%d, tmpbuf size=%lu\n",
			__FUNCTION__, rlen, sizeof(tmpbuf)));
		return -EAGAIN;
	}

	DBG_INFO(("%s: eldbg_ring_pull_single rlen=%d , usrsz=%lu\n", __FUNCTION__, rlen, usrsz));
	if (rlen > usrsz) {
		DBG_ERROR(("%s: usr buf insufficient! rlen=%d, usrsz=%ld \n",
			__FUNCTION__, rlen, usrsz));
		return -EFAULT;
	}

	ret = copy_to_user(usrbuf, (void*)tmpbuf, rlen);
	if (ret) {
		DBG_ERROR(("%s: copy_to_usr fails! rlen=%d, usrsz=%ld \n",
			__FUNCTION__, rlen, usrsz));
		return -EFAULT;
	}

	*loff += rlen;
	return rlen;
}
#endif /* EWP_EVENTTS_LOG */

void
eldbg_ring_proc_create(eldbg_cmn_t *dbg, uint unit, bool isnic)
{
#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	eldbg_ring_cmn_t *ring_cmn = NULL;
#endif /* DEBUGABILITY OR EVENT_LOG_ACCESS */
	char buf[DUMP_FILENAME_SZ] = {0};
	static bool iscreated = FALSE;

	if (dbg == NULL) {
		ASSERT(dbg);
		return;
	}

#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	ring_cmn = dbg_get_ring_from_ring_id(dbg, EVENT_LOG_STR_RING_ID);
	/* iscreated: NIC only needs proc fs for string regardless of # of chipsets */
	if ((iscreated == FALSE) && ring_cmn) {
		if (isnic) {
			snprintf(buf, sizeof(buf), "%s_nic", PROC_NAME_EL_STR);
		} else {
			snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_STR, unit);
		}
		if (!proc_create_data(buf, S_IRUSR, NULL, &dhd_ring_proc_ops, ring_cmn)) {
			DBG_ERROR(("Failed to create %s procfs interface\n", PROC_NAME_EL_STR));
		} else {
			DBG_INFO(("Created %s procfs interface\n", PROC_NAME_EL_STR));
		}
	}

	ring_cmn = dbg_get_ring_from_ring_id(dbg, EVENT_LOG_STR_CHATTY_RING_ID);
	/* iscreated: NIC only needs proc fs for string regardless of # of chipsets */
	if ((iscreated == FALSE) && ring_cmn) {
		if (isnic) {
			snprintf(buf, sizeof(buf), "%s_nic", PROC_NAME_EL_STR_CHATTY);
		} else {
			snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_STR_CHATTY, unit);
		}
		if (!proc_create_data(buf, S_IRUSR, NULL, &dhd_ring_proc_ops, ring_cmn)) {
			DBG_ERROR(("Failed to create %s procfs interface\n",
				PROC_NAME_EL_STR_CHATTY));
		} else {
			DBG_INFO(("Created %s procfs interface\n", PROC_NAME_EL_STR_CHATTY));
		}
	}

	ring_cmn = dbg_get_ring_from_ring_id(dbg, EVENT_LOG_BIN_RING_ID);
	if (ring_cmn) {
		snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_BIN, unit);
		if (!proc_create_data(buf, S_IRUSR, NULL, &dhd_ring_proc_ops, ring_cmn)) {
			DBG_ERROR(("Failed to create /proc/%s interface\n", PROC_NAME_EL_BIN));
		} else {
			DBG_INFO(("Created /proc/%s procfs interface\n", PROC_NAME_EL_BIN));
		}
	}
#endif /* DEBUGABILITY OR EVENT_LOG_ACCESS */

	if (isnic) {
		iscreated = TRUE;
	}
}

void
eldbg_ring_proc_destroy(eldbg_cmn_t *dbg, uint unit, bool isnic)
{
	char buf[DUMP_FILENAME_SZ] = {0};
	static bool isdestroied = FALSE;

	if (dbg == NULL) {
		ASSERT(dbg);
		return;
	}

#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	/* isdestroied: NIC proc fs for string is only one regardless of # of chipsets */
	if (isdestroied == FALSE) {
		if (isnic) {
			snprintf(buf, sizeof(buf), "%s_nic", PROC_NAME_EL_STR);
		} else {
			snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_STR, unit);
		}
		remove_proc_entry(buf, NULL);
	}

	if (isdestroied == FALSE) {
		if (isnic) {
			snprintf(buf, sizeof(buf), "%s_nic", PROC_NAME_EL_STR_CHATTY);
		} else {
			snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_STR_CHATTY, unit);
		}
		remove_proc_entry(buf, NULL);
	}

	snprintf(buf, sizeof(buf), "%s_%d", PROC_NAME_EL_BIN, unit);
	remove_proc_entry(buf, NULL);
#endif /* DEBUGABILITY || EVENT_LOG_ACCESS */
	if (isnic) {
		isdestroied = TRUE;
	}
}
#endif /* SHOW_LOGTRACE */
