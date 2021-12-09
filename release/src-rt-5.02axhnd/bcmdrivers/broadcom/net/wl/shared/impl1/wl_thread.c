/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#include <wl_thread.h>

#if defined(WL_ALL_PASSIVE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
#include <linux/kthread.h>

#include <wl_dbg.h>
#include <wlc_cfg.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_pktc.h>

extern void wl_dpc_rxwork(struct wl_task *task);
extern void wl_start_txqwork(wl_task_t *task);
extern void BCMFASTPATH wl_start_txchain_txqwork(pktc_info_t *pktci);

#define USER_TASKSET_CMD  "/usr/bin/taskset"

static int
call_usermodehelper_taskset(int pid, int processor_id)
{
	int rc;
	char pidbuf[16] = {0};
	char cpumaskbuf[16] = {0};
	char *argv[] = { USER_TASKSET_CMD, "-p", cpumaskbuf, pidbuf, NULL };
	char *envp[] = { NULL };
	sprintf(pidbuf, "%d", pid);
	if (processor_id == 0)
		sprintf(cpumaskbuf, "1");
	else if (processor_id == 1)
		sprintf(cpumaskbuf, "2");
	else if (processor_id == -1)
		sprintf(cpumaskbuf, "3");
	else {
		printk(KERN_ERR "wl: Unsupported processor_id %d\n", processor_id);
		return -1;
	}
	rc = call_usermodehelper(USER_TASKSET_CMD, argv, envp, UMH_WAIT_PROC);
	if (rc)
		printk(KERN_WARNING "wl: cannot use %s command\n", USER_TASKSET_CMD);

	return rc;
}

static int
wl_worker_thread_func(void *data)
{
	struct wl_info *wl = (struct wl_info *) data;
	struct wl_if *p;

	while (1)
	{
		wait_event_interruptible(wl->kthread_wqh,
		                            (wl->txq_dispatched ||
		                             wl->rxq_dispatched ||
#if defined(PKTC_TBL)
		                             wl->txq_txchain_dispatched ||
#endif
		                             kthread_should_stop()));
		if (kthread_should_stop())
		{
			printk(KERN_INFO "kthread_should_stop detected on wl%d\n", wl->unit);
			break;
		}
		if (wl->rxq_dispatched)
			wl_dpc_rxwork(&wl->wl_dpc_task);
		if (wl->txq_dispatched)
			wl_start_txqwork(&wl->txq_task);

#if defined(PKTC_TBL)
		WL_LOCK(wl);
		p = wl->if_list;
		while (p != NULL) {
			if (p->pktci && p->pktci->_txq_txchain_dispatched) {
				WL_UNLOCK(wl);
				wl_start_txchain_txqwork(p->pktci);
				WL_LOCK(wl);
			}
			p = p->next;
		}
		WL_UNLOCK(wl);
#endif

		/*
		 * If this thread is running with Real Time priority, be nice
		 * to other threads by yielding the CPU after each batch of packets.
		 */
		if (current->policy == SCHED_FIFO || current->policy == SCHED_RR)
			yield();
	}

	return 0;
}

void wl_thread_schedule_work(struct wl_info *wl)
{
	wake_up_interruptible(&wl->kthread_wqh);
}

int wl_thread_attach(struct wl_info *wl)
{
	int ret = 0;
	char kthrd_namebuf[32] = {0};

	init_waitqueue_head(&wl->kthread_wqh);
	sprintf(kthrd_namebuf, "wl%d-kthrd", wl->unit);
	printk(KERN_INFO "wl%d: creating kthread %s\n", wl->unit, kthrd_namebuf);
	wl->kthread = kthread_create(wl_worker_thread_func, wl, kthrd_namebuf);
	if (IS_ERR(wl->kthread)) {
		WL_ERROR(("wl%d: failed to create kernel thread\n", wl->unit));
		ret = -1;
	}
	if (WL_CONFIG_SMP() && num_online_cpus() > 1) 
	{
		if (wl->unit == 1) {
			wl->processor_id = 1; /* default, wl1 -> cpu1 */
		} else {
			wl->processor_id = 0; /* default, wl0 -> cpu0 */
		}
	}
	else
		wl->processor_id = 0; /* wl0 -> cpu0 */

	wake_up_process(wl->kthread);
	if (call_usermodehelper_taskset((int)wl->kthread->pid, wl->processor_id)) {
		WL_ERROR(("wl%d: could not bind kernel thread to CPU\n", wl->unit));
		ret = -1;
	}
	return ret;
}

void wl_thread_detach(struct wl_info *wl)
{
	kthread_stop(wl->kthread);
	wl->kthread = NULL;
}

int wl_get_processor_id(void *wl)
{
	return ((wl_info_t *)wl)->processor_id;
}

static void wl_set_processor_id_usage(void)
{
	char *anystr;
	char *cpu1str = (WL_CONFIG_SMP()) ? "or 1" : "";

	anystr = "(or -1 for any)";

	if (!WL_CONFIG_SMP() && anystr != '\0')
		printk(KERN_ERR "wl_set_processor_id: only tp_id 0 is supported in this config.\n");
	else
		printk(KERN_ERR "wl_set_processor_id: tp_id must be 0 %s %s.\n",
		                cpu1str, anystr);
}

void wl_set_processor_id(void *wl, int processor_id)
{
	if (processor_id == 0 ||
	     (WL_CONFIG_SMP() && processor_id == 1) ||
	     processor_id == -1) {
		pid_t pid = ((wl_info_t *)wl)->kthread->pid;
		if (call_usermodehelper_taskset((int) pid, processor_id) != 0) {
			printk(KERN_ERR "usermodehelper_taskset failed!  "
			                "Unable to set wlan kthread pid %d to CPU%d\n",
			                (int) pid, processor_id);
			return;
		}
		((wl_info_t *)wl)->processor_id = processor_id;
	}
	else
		wl_set_processor_id_usage();

}

#endif /* WL_ALL_PASSIVE && LINUX >= 3.4 */
