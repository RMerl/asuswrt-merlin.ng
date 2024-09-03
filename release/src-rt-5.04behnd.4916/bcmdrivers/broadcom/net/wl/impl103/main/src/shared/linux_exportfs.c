
#include <linux/proc_fs.h>
#include <osl.h>

#ifdef EVENT_LOG_NIC
#include <wl_linux.h>
#include <wl_dbg.h>
#else
#include <dhd.h>
#include <dhd_dbg.h>
#include <dhd_debug.h>
#endif /* EVENT_LOG_NIC */

#ifndef DUMP_FILENAME_SZ
#define DUMP_FILENAME_SZ     40
#endif

#ifdef EVENT_LOG_NIC
#define HDL_PTR(x)		((wl_info_t *)(x))
#else
#define HDL_PTR(x)		((dhd_pub_t *)(x))
#endif /* EVENT_LOG_NIC */

#define OSH_PTR(x)		(osl_t *)(HDL_PTR(x)->osh)
#define DBG_PTR(x)		(eldbg_cmn_t *)(HDL_PTR(x)->dbg)

#ifdef EVENT_LOG_NIC
#define EXP_ERROR(x)		WL_ERROR(x)
#define EXP_INFO(x)		WL_INFORM(x)
#else
#define EXP_ERROR(x)		DHD_ERROR(x)
#define EXP_INFO(x)		DHD_INFO(x)
#endif /* EVENT_LOG_NIC */

#ifdef SHOW_LOGTRACE

void *g_hdl = NULL;

#ifdef EVENT_LOG_NIC
#define PROC_NAME_FW_VERBOSE	"wl_trace_nic"
#else
#define PROC_NAME_FW_VERBOSE	"wl_trace"
#endif
#define PROC_NAME_ECOUNTERS	"wl_ecounters"

#ifdef EVENT_LOG_NIC
uint8 control_logtrace = CUSTOM_CONTROL_LOGTRACE;
#endif /* EVENT_LOG_NIC */

static int ring_proc_open(struct inode *inode, struct file *file);
ssize_t ring_proc_read(struct file *file, char *buffer, size_t tt, loff_t *loff);

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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		ret = single_open(file, 0, PDE_DATA(inode));
#else
		ret = single_open(file, 0, NULL);
#endif
	} else {
		EXP_ERROR(("%s: inode is NULL\n", __FUNCTION__));
	}
	return ret;
}

ssize_t
ring_proc_read(struct file *file, char __user *buffer, size_t tt, loff_t *loff)
{
	trace_buf_info_t *trace_buf_info;
	int ret = BCME_ERROR;
	eldbg_ring_cmn_t *ring =
		(eldbg_ring_cmn_t *)((struct seq_file *)(file->private_data))->private;

	if (ring == NULL) {
		EXP_ERROR(("%s: ring is NULL\n", __FUNCTION__));
		return ret;
	}

	ASSERT(g_hdl);

	trace_buf_info = (trace_buf_info_t *)MALLOCZ(HDL_PTR(g_hdl)->osh, sizeof(trace_buf_info_t));
	if (trace_buf_info) {
		eldbg_read_ring_into_trace_buf(ring, trace_buf_info);
		if (copy_to_user(buffer, (void*)trace_buf_info->buf, MIN(trace_buf_info->size, tt)))
		{
			ret = -EFAULT;
			goto exit;
		}
		if (trace_buf_info->availability == BUF_NOT_AVAILABLE) {
			ret = BUF_NOT_AVAILABLE;
		} else {
			ret = trace_buf_info->size;
		}
	} else
		EXP_ERROR(("Memory allocation Failed\n"));

exit:
	if (trace_buf_info) {
		MFREE(HDL_PTR(g_hdl)->osh, trace_buf_info, sizeof(trace_buf_info_t));
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
		EXP_ERROR(("%s: ring is NULL!\n", __FUNCTION__));
		return -EFAULT;
	}

	rlen = eldbg_ring_pull_single(ring, tmpbuf, sizeof(tmpbuf), TRUE);
	if (!rlen) {
		/* rlen can also be zero when there is no data in the ring */
		EXP_INFO(("%s: eldbg_ring_pull_single, rlen=%d, tmpbuf size=%lu\n",
			__FUNCTION__, rlen, sizeof(tmpbuf)));
		return -EAGAIN;
	}

	EXP_INFO(("%s: eldbg_ring_pull_single rlen=%d , usrsz=%lu\n", __FUNCTION__, rlen, usrsz));
	if (rlen > usrsz) {
		EXP_ERROR(("%s: usr buf insufficient! rlen=%d, usrsz=%ld \n",
			__FUNCTION__, rlen, usrsz));
		return -EFAULT;
	}

	ret = copy_to_user(usrbuf, (void*)tmpbuf, rlen);
	if (ret) {
		EXP_ERROR(("%s: copy_to_usr fails! rlen=%d, usrsz=%ld \n",
			__FUNCTION__, rlen, usrsz));
		return -EFAULT;
	}

	*loff += rlen;
	return rlen;
}
#endif /* EWP_EVENTTS_LOG */

void
eldbg_ring_proc_create(void *hdl)
{
	char buf[DUMP_FILENAME_SZ] = {0};
#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	eldbg_ring_cmn_t *dbg_verbose_ring = NULL;
#endif /* DEBUGABILITY OR EVENT_LOG_ACCESS */

	if (hdl == NULL) {
		return;
	}
	if (g_hdl != NULL && hdl != g_hdl) {
		/* multi hdl? create proc per interface later? */
		EXP_ERROR(("%s: only one ring is possible.\n", __FUNCTION__));
		return;
	}
	g_hdl = hdl;

#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	dbg_verbose_ring = eldbg_get_ring_from_ring_id(DBG_PTR(g_hdl), FW_VERBOSE_RING_ID);

	if (dbg_verbose_ring) {
		sprintf(buf, "%s_radio", PROC_NAME_FW_VERBOSE);
		if (!proc_create_data(buf, S_IRUSR, NULL, &dhd_ring_proc_ops,
			dbg_verbose_ring)) {
			EXP_ERROR(("Failed to create %s procfs interface\n", buf));
		} else {
			EXP_INFO(("Created %s procfs interface\n", buf));
		}
	} else {
		EXP_ERROR(("dbg_verbose_ring is NULL, %s  not created\n", buf));
	}
#endif /* DEBUGABILITY OR EVENT_LOG_ACCESS */

#ifdef EWP_ECNTRS_LOGGING

	sprintf(buf, "%s_radio", PROC_NAME_ECOUNTERS, HDL_PTR(g_hdl)->unit);
	if (!proc_create_data(buf, S_IRUSR, NULL, &dhd_ring_proc_ops,
		HDL_PTR(g_hdl)->ecntr_dbg_ring)) {
		EXP_ERROR(("Failed to create %s procfs interface\n", buf));
	} else {
		EXP_INFO(("Created %s procfs interface\n", buf));
	}
#endif /* EWP_ECNTRS_LOGGING */
}

void
eldbg_ring_proc_destroy(void *hdl)
{
	char buf[DUMP_FILENAME_SZ] = {0};

	if (hdl == NULL || hdl != g_hdl) {
		return;
	}

#if defined(DEBUGABILITY) || defined(EVENT_LOG_ACCESS)
	sprintf(buf, "%s_radio", PROC_NAME_FW_VERBOSE);
	remove_proc_entry(buf, NULL);
#endif /* DEBUGABILITY */

#ifdef EWP_ECNTRS_LOGGING
	sprintf(buf, "%s_radio%d", PROC_NAME_ECOUNTERS, HDL_PTR(g_hdl)->unit);
	remove_proc_entry(buf, NULL);
#endif /* EWP_ECNTRS_LOGGING */

	g_hdl = NULL;
}
#endif /* SHOW_LOGTRACE */
