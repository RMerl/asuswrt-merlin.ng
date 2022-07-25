/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
*
*    Copyright (c) 2012 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
:>
*/

#include <linux/kthread.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include "uapi/linux/bcm_realtime.h"
#if defined(CONFIG_BCM_BPM_BULK_FREE)
#include <linux/gbpm.h>
#endif /* CONFIG_BCM_BPM_BULK_FREE */
//#include "skb_defines.h"

static struct task_struct *skb_free_task = NULL;
static struct sk_buff *skb_completion_queue = NULL;
static unsigned int skb_completion_queue_cnt = 0;
static DEFINE_SPINLOCK(skbfree_lock);
#if defined(CONFIG_BCM_BPM_BULK_FREE)
static struct completion skb_free_complete;
#endif /* CONFIG_BCM_BPM_BULK_FREE */

/* Setting value to WDF budget + some room for SKBs
 * freed by other threads */
#define MAX_SKB_FREE_BUDGET 256

/* the min number of skb to wake up free task */
static unsigned int skb_free_start_budget __read_mostly;

#if defined(CONFIG_BCM_BPM_BULK_FREE)

#define SKB_RECYCLE_FLAGS_FAST  (SKB_RECYCLE | SKB_DATA_RECYCLE | SKB_BPM_PRISTINE)

#define SKB_SHINFO(skb)         ((struct skb_shared_info *)skb_shinfo(skb))
#define SHINFO_NRFLAGS(skb)     (SKB_SHINFO(skb)->nr_frags)
#define SHINFO_FRAGLIST(skb)    (SKB_SHINFO(skb)->frag_list)
#define SHINFO_TXFLAGS(skb)     (SKB_SHINFO(skb)->tx_flags)

/* Evaluate pointers inside scb - true when
 * pointers are null */
#define SKB_PTR_FAST_RELEASE_TO_BPM(skb) \
    (!(((unsigned long) (skb->sp)) | \
    ((unsigned long) (skb_nfct(skb))) | \
    ((unsigned long) (skb->nf_bridge)) | \
    ((unsigned long) (skb->blog_p)) | \
    ((unsigned long) (skb->destructor)) | \
    ((unsigned long) (SHINFO_FRAGLIST(skb)))))

#define SKB_FAST_RELEASE_TO_BPM(skb) \
    SKB_PTR_FAST_RELEASE_TO_BPM(skb) & \
    (skb->recycle_flags == SKB_RECYCLE_FLAGS_FAST) & \
    (!(SHINFO_TXFLAGS(skb) & SKBTX_DEV_ZEROCOPY)) & \
    (!SHINFO_NRFLAGS(skb)) & \
    (!skb->cloned) & \
    (!skb->_skb_refdst)

typedef struct skb_list {
        struct sk_buff *head;
        struct sk_buff *tail;
        uint32_t len;
} skb_list_t;

void dev_kfree_skb_thread_wait(void);
void dev_kfree_skb_thread_bulk(struct sk_buff *head, struct sk_buff *tail, uint32_t len);
#else
void dev_kfree_skb_thread_bulk(struct sk_buff *skb);
#endif /* CONFIG_BCM_BPM_BULK_FREE */

static int skb_free_thread_func(void *thread_data)
{
	unsigned int budget;
	struct sk_buff *skb;
	struct sk_buff *free_list = NULL;
	unsigned long flags;
	/* wake up periodically for every 20ms */
	unsigned timeout_jiffies = msecs_to_jiffies(20);
#if defined(CONFIG_BCM_BPM_BULK_FREE)
        skb_list_t bpm_freelist = { NULL, NULL, 0 };
#if !defined(CONFIG_BCM_XRDP)
        int idx = 0;
#endif /* CONFIG_BCM_XRDP */
        void *bufp_arr[MAX_SKB_FREE_BUDGET];
#endif /* CONFIG_BCM_BPM_BULK_FREE */

	while (!kthread_should_stop()) {
		budget = MAX_SKB_FREE_BUDGET;

update_list:
                spin_lock_irqsave(&skbfree_lock, flags);
                if (free_list == NULL) {
                        if (skb_completion_queue) {
                                free_list = skb_completion_queue;
                                skb_completion_queue = NULL;
                                skb_completion_queue_cnt = 0;
                        }
                }
                spin_unlock_irqrestore(&skbfree_lock, flags);

		while (free_list && budget) {
			skb = free_list;
			free_list = free_list->next;
			skb->next = NULL;
#if defined(CONFIG_BCM_BPM_BULK_FREE)
                        if (SKB_FAST_RELEASE_TO_BPM(skb)) {
#if defined(CONFIG_BCM_XRDP)
	                        (*skb->recycle_hook)(skb, skb->recycle_context, SKB_DATA_RECYCLE);
                                skb->recycle_hook = (RecycleFuncP)gbpm_recycle_skb;
#else
                                bufp_arr[idx++] = gbpm_invalidate_dirtyp(skb);
#endif /* CONFIG_BCM_XRDP */
                                if (bpm_freelist.len != 0) {
                                        bpm_freelist.tail->next = skb;
                                        bpm_freelist.tail = skb;
                                } else {
                                        bpm_freelist.tail =  bpm_freelist.head = skb;
                                }
                                bpm_freelist.len++;
                        } else
#endif /* CONFIG_BCM_BPM_BULK_FREE */
                        {
			        __kfree_skb(skb);
                        }
			budget--;
		}

#if defined(CONFIG_BCM_BPM_BULK_FREE)
                if (bpm_freelist.len) {
                        gbpm_free_skblist(bpm_freelist.head,
                                bpm_freelist.tail, bpm_freelist.len, bufp_arr);
                        bpm_freelist.head = bpm_freelist.tail = NULL;
                        bpm_freelist.len = 0;
#if !defined(CONFIG_BCM_XRDP)
                        idx = 0;
#endif /* CONFIG_BCM_XRDP */
                }
#endif /* CONFIG_BCM_BPM_BULK_FREE */

		if (free_list || skb_completion_queue) {
			if (budget)
				goto update_list;

			/* we still have packets in Q, reschedule the task */
			yield();
		} else {
#if defined(CONFIG_BCM_BPM_BULK_FREE)
                        complete(&skb_free_complete);
#endif /* CONFIG_BCM_BPM_BULK_FREE */
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(timeout_jiffies);
		}
        }
	return 0;
}
static int skb_free_thread_stats_show(struct seq_file *m, void *v)
{
	seq_printf(m, "skb_completion_queue_cnt %d \n", skb_completion_queue_cnt);

    return 0;
} /* skb_free_thread_proc_rd_func */

static struct proc_dir_entry *skb_free_thread_proc_directory;

static int skb_free_thread_proc_init(void)
{
    skb_free_thread_proc_directory = proc_mkdir("skb_free_thread", NULL) ;

    if (!skb_free_thread_proc_directory) goto fail_dir ;

    if (!proc_create_single("skb_free_thread/stats", 0, NULL, skb_free_thread_stats_show)) {
        goto fail_entry;
    }

    return (0) ;

fail_entry:
    printk("%s %s: Failed to create proc entry in skb_free_thread\n", __FILE__, __FUNCTION__);
    remove_proc_entry("skb_free_thread" ,NULL); /* remove already registered directory */

fail_dir:
    printk("%s %s: Failed to create directory skb_free_thread\n", __FILE__, __FUNCTION__) ;
    return (-EIO) ;
} /* skb_free_thread_proc_init */

#ifndef SZ_32M
#define SZ_32M		0x02000000
#endif
#ifndef SZ_64M
#define SZ_64M		0x04000000
#endif
#ifndef SZ_128M
#define SZ_128M		0x08000000
#endif
#ifndef SZ_256M
#define SZ_256M		0x10000000
#endif

struct task_struct *create_skb_free_task(void)
{
	struct task_struct *tsk;
	struct sched_param param;
	struct sysinfo sinfo;

	si_meminfo(&sinfo);

	if (sinfo.totalram <= (SZ_32M / sinfo.mem_unit))
		skb_free_start_budget = 16;
	else if (sinfo.totalram <= (SZ_64M / sinfo.mem_unit))
		skb_free_start_budget = 32;
	else if (sinfo.totalram <= (SZ_128M / sinfo.mem_unit))
		skb_free_start_budget = 64;
	else if (sinfo.totalram <= (SZ_256M / sinfo.mem_unit))
		skb_free_start_budget = 128;
	else
		skb_free_start_budget = 256;

	tsk = kthread_create(skb_free_thread_func, NULL, "skb_free_task");

	if (IS_ERR(tsk)) {
		printk(KERN_EMERG "skb_free_task creation failed\n");
		return NULL;
	}

	/* Initialize the proc interface for debugging information */
    if (skb_free_thread_proc_init()!=0)
    {
        printk(KERN_EMERG "\n%s %s: skb_free_thread_proc_init() failed\n", __FILE__, __FUNCTION__) ;
        return NULL;
    }

#if defined(CONFIG_BCM_BPM_BULK_FREE)
        init_completion(&skb_free_complete);
#endif /* CONFIG_BCM_BPM_BULK_FREE */
	param.sched_priority = BCM_RTPRIO_DATA_CONTROL;
	sched_setscheduler(tsk, SCHED_RR, &param);
	wake_up_process(tsk);

	printk(KERN_EMERG "skb_free_task created successfully with start budget %d\n", skb_free_start_budget);
	return tsk;
}

/* queue the skb so it can be freed in thread context
 * note: this thread is not binded to any cpu,and we rely on scheduler to
 * run it on cpu with less load
 */
void dev_kfree_skb_thread(struct sk_buff *skb)
{
	unsigned long flags;

	if (refcount_dec_and_test(&skb->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
		skb->next = skb_completion_queue;
		skb_completion_queue = skb;
		skb_completion_queue_cnt++ ;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skb_free_start_budget))
			wake_up_process(skb_free_task);
	}
}
EXPORT_SYMBOL(dev_kfree_skb_thread);

#if defined(CONFIG_BCM_BPM_BULK_FREE)
/* There may be packets in skb_completion_queue but the thread
 * not scheduled (becasue of skb_free_start_budget). This API
 * schedules the thread in that case and wait for the thread to
 * complete
 *
 * TODO: Check if we can manage this case within this module
 */
void dev_kfree_skb_thread_wait()
{
        if ((skb_free_task->state != TASK_RUNNING) && skb_completion_queue_cnt) {
                reinit_completion(&skb_free_complete);
                wake_up_process(skb_free_task);

                wait_for_completion(&skb_free_complete);

                if (skb_completion_queue_cnt == 0) {
                    printk("[%s] Waited and freed all pkts\n", __FUNCTION__);
                }
        }
}
EXPORT_SYMBOL(dev_kfree_skb_thread_wait);

/* bulk queue the skb so it can be freed in thread context
 * note: this thread is not binded to any cpu,and we rely on scheduler to
 * run it on cpu with less load
 */
void dev_kfree_skb_thread_bulk(struct sk_buff *head, struct sk_buff *tail, uint32_t len)
{
	unsigned long flags;

	/* Assumption here is all the skb's in list have same refcount
	 *  TODO: check if this is ok in all cases
         */
	if (refcount_dec_and_test(&head->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
                tail->next = skb_completion_queue;
                skb_completion_queue = head;
                skb_completion_queue_cnt += len;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skb_free_start_budget))
			wake_up_process(skb_free_task);
	}
}
#else
void dev_kfree_skb_thread_bulk(struct sk_buff *skb)
{
	unsigned long flags;
	struct sk_buff *skbfreelistend;
	unsigned int skbcnt = 1;

	/* locate last skb of the supplied skb list */
	skbfreelistend = skb;
	while (skbfreelistend->next != NULL) {
		skbfreelistend = skbfreelistend->next;
		/* +1 for first skb already done during init */
		skbcnt++;
	}
	/* Assumption here is all the skb's in list have same refcount
	 *  TODO: check if this is ok in all cases
     */

	if (refcount_dec_and_test(&skb->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
		skbfreelistend->next = skb_completion_queue;
		skb_completion_queue = skb;
		skb_completion_queue_cnt += skbcnt;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skb_free_start_budget))
			wake_up_process(skb_free_task);
	}
}
#endif /* CONFIG_BCM_BPM_BULK_FREE */
EXPORT_SYMBOL(dev_kfree_skb_thread_bulk);

static int __init bcm_skb_free_init(void)
{

	skb_free_task = create_skb_free_task();

	if(skb_free_task == NULL)
		BUG();

	return 0;
}

subsys_initcall(bcm_skb_free_init);
