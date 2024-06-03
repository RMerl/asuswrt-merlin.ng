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
#include <linux/netdevice.h>
#include "uapi/linux/bcm_realtime.h"
#if defined(CONFIG_BCM_BPM_BULK_FREE)
#include <linux/gbpm.h>
#endif /* CONFIG_BCM_BPM_BULK_FREE */

#include <linux/bcm_version_compat.h>

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
static unsigned int skbfree_coalescing_budget __read_mostly;
static unsigned int skbfree_free_budget __read_mostly;
static unsigned int skbfree_timer_period __read_mostly;
static unsigned int skbfree_thread_enable __read_mostly;

#if defined(CONFIG_BCM_BPM_BULK_FREE)

#define SKB_RECYCLE_FLAGS_FAST  (SKB_RECYCLE | SKB_DATA_RECYCLE | SKB_BPM_PRISTINE)

#define SKB_SHINFO(skb)         ((struct skb_shared_info *)skb_shinfo(skb))
#define SHINFO_NRFLAGS(skb)     (SKB_SHINFO(skb)->nr_frags)
#define SHINFO_FRAGLIST(skb)    (SKB_SHINFO(skb)->frag_list)
#define SHINFO_TXFLAGS(skb)     (SKB_SHINFO(skb)->tx_flags)


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))

/* Evaluate pointers inside scb - true when
 * pointers are null */

// /// Need to add the back in this check 
// ((unsigned long) (skb_ext_find(skb, SKB_EXT_BRIDGE_NF))) | 
// in place of --  ((unsigned long) (skb->nf_bridge)) |


#if defined(CONFIG_XFRM)
#define SKB_PTR_FAST_RELEASE_TO_BPM(skb) \
    (!(((unsigned long) (skb_ext_find(skb, SKB_EXT_SEC_PATH))) | \
    ((unsigned long) (skb_nfct(skb))) | \
    ((unsigned long) (skb->blog_p)) | \
    ((unsigned long) (skb->destructor)) | \
    ((unsigned long) (SHINFO_FRAGLIST(skb)))))
#else
#define SKB_PTR_FAST_RELEASE_TO_BPM(skb) \
    (!(((unsigned long) (skb_nfct(skb))) | \
    ((unsigned long) (skb->blog_p)) | \
    ((unsigned long) (skb->destructor)) | \
    ((unsigned long) (SHINFO_FRAGLIST(skb)))))

#endif


#define SKB_FAST_RELEASE_TO_BPM(skb) \
    SKB_PTR_FAST_RELEASE_TO_BPM(skb) & \
    (skb->recycle_flags == SKB_RECYCLE_FLAGS_FAST) & \
    (!(SHINFO_TXFLAGS(skb) & SKBFL_ZEROCOPY_FRAG)) & \
    (!SHINFO_NRFLAGS(skb)) & \
    (!skb->cloned) & \
    (!skb->_skb_refdst)

#else

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

#endif

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
#if defined(CONFIG_BCM_BPM_BULK_FREE)
	skb_list_t bpm_freelist = { NULL, NULL, 0 };
#if !defined(CONFIG_BCM_XRDP)
	int idx = 0;
#endif /* CONFIG_BCM_XRDP */
	static void *bufp_arr[MAX_SKB_FREE_BUDGET];
#endif /* CONFIG_BCM_BPM_BULK_FREE */

	while (!kthread_should_stop()) {
		budget = skbfree_free_budget;

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
				(*skb->recycle_hook)(skb, skb->recycle_context,
						     SKB_DATA_RECYCLE);
				skb->recycle_context &= ~SKB_DATA_RECYCLE;
				skb->head = skb->data = NULL;
				skb->recycle_hook = (RecycleFuncP)gbpm_recycle_skb;
#else
				bufp_arr[idx++] = gbpm_invalidate_dirtyp(skb);
#endif /* CONFIG_BCM_XRDP */
				if (bpm_freelist.len != 0) {
					bpm_freelist.tail->next = skb;
					bpm_freelist.tail = skb;
				} else
					bpm_freelist.tail = bpm_freelist.head = skb;

				bpm_freelist.len++;
			} else
#endif /* CONFIG_BCM_BPM_BULK_FREE */
				__kfree_skb(skb);
			budget--;
		}

#if defined(CONFIG_BCM_BPM_BULK_FREE)
		if (bpm_freelist.len) {
			gbpm_free_skblist(bpm_freelist.head, bpm_freelist.tail,
					  bpm_freelist.len, bufp_arr);
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
			schedule_timeout(msecs_to_jiffies(skbfree_timer_period));
		}
	}
	return 0;
}

/* proc files */
#define SKB_FREE_THREAD_PROC_DIR_NAME		"skb_free_thread"
#define SKB_FREE_THREAD_COALESCING_BUDGET_NAME	"coalescing_budget"
#define SKB_FREE_THREAD_FREE_BUDGET_NAME	"free_budget"
#define SKB_FREE_THREAD_COALESCING_PERIOD_NAME	"coalescing_period"
#define SKB_FREE_THREAD_ENABLE_NAME		"enable"
#define SKB_FREE_THREAD_STATS_NAME		"stats"

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *coalescing_budget_proc_file = NULL;
static struct proc_dir_entry *free_budget_proc_file = NULL;
static struct proc_dir_entry *period_proc_file = NULL;
static struct proc_dir_entry *enable_proc_file = NULL;
static struct proc_dir_entry *stats_proc_file = NULL;

static ssize_t coalescing_budget_file_write(struct file *file,
				       const char __user *ubuf,
				       size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len;
	unsigned long input_val;

	if (cnt >= sizeof(buf))
		goto budget_write_fail;

	len = min((unsigned long)cnt, (unsigned long)(sizeof(buf) - 1));
	if (copy_from_user(buf, ubuf, len))
		goto budget_write_fail;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &input_val))
		goto budget_write_fail;

	skbfree_coalescing_budget = input_val;
	return cnt;

budget_write_fail:

	pr_err("invalid input value\n");
	return cnt;
}

static ssize_t coalescing_budget_file_read(struct file *file, char __user *ubuf,
				      size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len = 0;

	if ((*ppos > 0) || (cnt < sizeof(buf)))
		return 0;

	len += sprintf(buf, "%d\n", skbfree_coalescing_budget);
	if (copy_to_user(ubuf, buf, len))
		return -EFAULT;

	*ppos = len;
	return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
static const struct proc_ops coalescing_budget_fops = {
	.proc_write = coalescing_budget_file_write,
	.proc_read = coalescing_budget_file_read,
};
#else
static const struct file_operations coalescing_budget_fops = {
	.owner = THIS_MODULE,
	.write = coalescing_budget_file_write,
	.read = coalescing_budget_file_read,
};
#endif

static ssize_t free_budget_file_write(struct file *file,
				      const char __user *ubuf,
				      size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len;
	unsigned long input_val;

	if (cnt >= sizeof(buf))
		goto budget_write_fail;

	len = min((unsigned long)cnt, (unsigned long)(sizeof(buf) - 1));
	if (copy_from_user(buf, ubuf, len))
		goto budget_write_fail;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &input_val))
		goto budget_write_fail;

	if ((input_val > MAX_SKB_FREE_BUDGET) || (input_val == 0))
		goto budget_write_fail;

	skbfree_free_budget = input_val;
	return cnt;

budget_write_fail:

	pr_err("invalid input value, the range is 1 to %d\n", MAX_SKB_FREE_BUDGET);

	return cnt;
}

static ssize_t free_budget_file_read(struct file *file, char __user *ubuf,
				     size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len = 0;

	if ((*ppos > 0) || (cnt < sizeof(buf)))
		return 0;

	len += sprintf(buf, "%d\n", skbfree_free_budget);
	if (copy_to_user(ubuf, buf, len))
		return -EFAULT;

	*ppos = len;
	return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
static const struct proc_ops free_budget_fops = {
	.proc_write = free_budget_file_write,
	.proc_read = free_budget_file_read,
};
#else
static const struct file_operations free_budget_fops = {
	.owner = THIS_MODULE,
	.write = free_budget_file_write,
	.read = free_budget_file_read,
};
#endif

static ssize_t period_file_write(struct file *file, const char __user *ubuf,
				 size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len;
	unsigned long input_val;

	if (cnt >= sizeof(buf))
		goto period_write_fail;

	len = min((unsigned long)cnt, (unsigned long)(sizeof(buf) - 1));
	if (copy_from_user(buf, ubuf, len))
		goto period_write_fail;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &input_val))
		goto period_write_fail;

	if (input_val == 0)
		goto period_write_fail;

	skbfree_timer_period = input_val;
	return cnt;

period_write_fail:

	pr_err("invalid input value. it has to be 1ms or longer\n");
	return cnt;
}

static ssize_t period_file_read(struct file *file, char __user *ubuf,
				size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len = 0;

	if ((*ppos > 0) || (cnt < sizeof(buf)))
		return 0;

	len += sprintf(buf, "%dms\n", skbfree_timer_period);
	if (copy_to_user(ubuf, buf, len))
		return -EFAULT;

	*ppos = len;
	return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
static const struct proc_ops period_fops = {
	.proc_write = period_file_write,
	.proc_read = period_file_read,
};
#else
static const struct file_operations period_fops = {
	.owner = THIS_MODULE,
	.write = period_file_write,
	.read = period_file_read,
};
#endif

static ssize_t enable_file_write(struct file *file, const char __user *ubuf,
				 size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len;
	unsigned long input_val;

	if (cnt >= sizeof(buf))
		goto enable_write_fail;

	len = min((unsigned long)cnt, (unsigned long)(sizeof(buf) - 1));
	if (copy_from_user(buf, ubuf, len))
		goto enable_write_fail;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &input_val))
		goto enable_write_fail;

	if ((input_val != 0) && (input_val != 1))
		goto enable_write_fail;

	skbfree_thread_enable = input_val;

	return cnt;

enable_write_fail:

	pr_err("invalid input value\n");
	return cnt;
}

static ssize_t enable_file_read(struct file *file, char __user *ubuf,
				size_t cnt, loff_t *ppos)
{
	char buf[16];
	int len = 0;

	if ((*ppos > 0) || (cnt < sizeof(buf)))
		return 0;

	len += sprintf(buf, "%d\n", skbfree_thread_enable);
	if (copy_to_user(ubuf, buf, len))
		return -EFAULT;

	*ppos = len;
	return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
static const struct proc_ops enable_fops = {
	.proc_write = enable_file_write,
	.proc_read = enable_file_read,
};
#else
static const struct file_operations enable_fops = {
	.owner = THIS_MODULE,
	.write = enable_file_write,
	.read = enable_file_read,
};
#endif

static int skb_free_thread_stats_show(struct seq_file *m, void *v)
{
	seq_printf(m, "skb_completion_queue_cnt = %d\n", skb_completion_queue_cnt);

	return 0;
} /* skb_free_thread_stats_show */

static int __init skb_free_thread_proc_init(void)
{
	proc_dir = proc_mkdir(SKB_FREE_THREAD_PROC_DIR_NAME, NULL);
	if (!proc_dir)
		goto proc_init_fail;

	coalescing_budget_proc_file = proc_create(SKB_FREE_THREAD_COALESCING_BUDGET_NAME,
						  S_IRUGO | S_IWUGO, proc_dir,
						  &coalescing_budget_fops);
	if (!coalescing_budget_proc_file)
		goto proc_init_fail;

	free_budget_proc_file = proc_create(SKB_FREE_THREAD_FREE_BUDGET_NAME,
					    S_IRUGO | S_IWUGO, proc_dir,
					    &free_budget_fops);
	if (!free_budget_proc_file)
		goto proc_init_fail;

	period_proc_file = proc_create(SKB_FREE_THREAD_COALESCING_PERIOD_NAME,
				       S_IRUGO | S_IWUGO, proc_dir,
				       &period_fops);
	if (!period_proc_file)
		goto proc_init_fail;

	enable_proc_file = proc_create(SKB_FREE_THREAD_ENABLE_NAME,
				       S_IRUGO | S_IWUGO, proc_dir,
				       &enable_fops);
	if (!enable_proc_file)
		goto proc_init_fail;

	stats_proc_file = proc_create_single(SKB_FREE_THREAD_STATS_NAME, 0,
					     proc_dir, skb_free_thread_stats_show);
	return 0;

proc_init_fail:

	pr_err("Failed to create PROC directory and/or files for %s.\n",
	       SKB_FREE_THREAD_PROC_DIR_NAME);

	if ((coalescing_budget_proc_file != NULL) && (proc_dir != NULL))
		remove_proc_entry(SKB_FREE_THREAD_COALESCING_BUDGET_NAME,
				  proc_dir);

	if ((free_budget_proc_file != NULL) && (proc_dir != NULL))
		remove_proc_entry(SKB_FREE_THREAD_FREE_BUDGET_NAME,
				  proc_dir);

	if ((period_proc_file != NULL) && (proc_dir != NULL))
		remove_proc_entry(SKB_FREE_THREAD_COALESCING_PERIOD_NAME,
				  proc_dir);

	if ((enable_proc_file != NULL) && (proc_dir != NULL))
		remove_proc_entry(SKB_FREE_THREAD_ENABLE_NAME, proc_dir);

	if ((stats_proc_file != NULL) && (proc_dir != NULL))
		remove_proc_entry(SKB_FREE_THREAD_STATS_NAME, proc_dir);

	remove_proc_entry(SKB_FREE_THREAD_PROC_DIR_NAME, NULL);

	return -EIO;
} /* skb_free_thread_proc_init */

void bcm_skb_free_proc_exit(void)
{
	/* assume all proc entries and dir are not NULL */
	remove_proc_entry(SKB_FREE_THREAD_COALESCING_BUDGET_NAME, proc_dir);
	remove_proc_entry(SKB_FREE_THREAD_FREE_BUDGET_NAME, proc_dir);
	remove_proc_entry(SKB_FREE_THREAD_COALESCING_PERIOD_NAME, proc_dir);
	remove_proc_entry(SKB_FREE_THREAD_ENABLE_NAME, proc_dir);
	remove_proc_entry(SKB_FREE_THREAD_STATS_NAME, proc_dir);
	remove_proc_entry(SKB_FREE_THREAD_PROC_DIR_NAME, NULL);
}

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

	skbfree_thread_enable = 1;
	skbfree_coalescing_budget = 32;
	skbfree_free_budget = MAX_SKB_FREE_BUDGET;
	skbfree_timer_period = 10;

	tsk = kthread_create(skb_free_thread_func, NULL, "skb_free_task");

	if (IS_ERR(tsk)) {
		printk(KERN_EMERG "skb_free_task creation failed\n");
		return NULL;
	}

	/* Initialize the proc interface for debugging information */
	if (skb_free_thread_proc_init()) {
		pr_err("%s:%s:skb_free_thread_proc_init() failed\n",
		       __FILE__, __func__);
		return NULL;
	}

#if defined(CONFIG_BCM_BPM_BULK_FREE)
	init_completion(&skb_free_complete);
#endif /* CONFIG_BCM_BPM_BULK_FREE */
	wake_up_process(tsk);

	printk(KERN_EMERG "skb_free_task created successfully with start "
	       "budget %d, period %dms\n", skbfree_coalescing_budget,
	       skbfree_timer_period);
	return tsk;
}

/* queue the skb so it can be freed in thread context
 * note: this thread is not binded to any cpu,and we rely on scheduler to
 * run it on cpu with less load
 */
void dev_kfree_skb_thread(struct sk_buff *skb)
{
	unsigned long flags;

	if (skbfree_thread_enable == 0) {
		dev_kfree_skb_any(skb);
		return;
	}

	if (refcount_dec_and_test(&skb->users)) {
		spin_lock_irqsave(&skbfree_lock, flags);
		skb->next = skb_completion_queue;
		skb_completion_queue = skb;
		skb_completion_queue_cnt++ ;
		spin_unlock_irqrestore(&skbfree_lock, flags);

		if ((skb_free_task->__state != TASK_RUNNING) &&
		    (skb_completion_queue_cnt >= skbfree_coalescing_budget))
			wake_up_process(skb_free_task);
	}
}
EXPORT_SYMBOL(dev_kfree_skb_thread);

#if defined(CONFIG_BCM_BPM_BULK_FREE)
/* There may be packets in skb_completion_queue but the thread
 * not scheduled (becasue of skbfree_coalescing_budget). This API
 * schedules the thread in that case and wait for the thread to
 * complete
 *
 * TODO: Check if we can manage this case within this module
 */
void dev_kfree_skb_thread_wait()
{
	if ((skb_free_task->__state != TASK_RUNNING) && skb_completion_queue_cnt) {
		reinit_completion(&skb_free_complete);
		wake_up_process(skb_free_task);

		wait_for_completion(&skb_free_complete);

		if (skb_completion_queue_cnt == 0)
			printk("[%s] Waited and freed all pkts\n", __func__);
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

		if ((skb_free_task->__state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skbfree_coalescing_budget))
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

		if ((skb_free_task->__state != TASK_RUNNING) &&
				(skb_completion_queue_cnt >= skbfree_coalescing_budget))
			wake_up_process(skb_free_task);
	}
}
#endif /* CONFIG_BCM_BPM_BULK_FREE */
EXPORT_SYMBOL(dev_kfree_skb_thread_bulk);

static int __init bcm_skb_free_init(void)
{
	skb_free_task = create_skb_free_task();

	if (skb_free_task == NULL)
		BUG();

	return 0;
}

subsys_initcall(bcm_skb_free_init);
