/*
 * Linux-specific abstractions to gain some independence from linux kernel versions.
 * Pave over some 2.2 versus 2.4 versus 2.6 kernel differences.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: linuxver.h 852108 2025-02-17 20:32:41Z $
 */

#ifndef _linuxver_h_
#define _linuxver_h_

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif

#include <typedefs.h>
#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/kconfig.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/semaphore.h>

#undef IP_TOS
#include <asm/io.h>

#include <linux/workqueue.h>

#define DAEMONIZE(a)	do { \
		allow_signal(SIGKILL);	\
		allow_signal(SIGTERM);	\
	} while (0)

#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT

#include <linux/sched.h>
#include <linux/sched/rt.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <uapi/linux/sched/types.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0) */

#include <net/lib80211.h>
#include <linux/ieee80211.h>

#ifndef __exit
#define __exit
#endif
#ifndef __devexit
#define __devexit
#endif
#ifndef __devinit
/* All devices are hotpluggable since linux 3.8.0 */
#define __devinit
#endif /* !__devinit */
#ifndef __devinitdata
#define __devinitdata
#endif
#ifndef __devexit_p
#define __devexit_p(x)	x
#endif

#define pci_module_init pci_register_driver

#define WL_USE_NETDEV_OPS

#if defined(CONFIG_RFKILL)
#define WL_CONFIG_RFKILL
#else
#undef WL_CONFIG_RFKILL
#endif

#define netif_down(dev)

/* Power management related macro & routines */
#define	PCI_SAVE_STATE(a, b)	pci_save_state(a)
#define	PCI_RESTORE_STATE(a, b)	pci_restore_state(a)

#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev)		do {} while (0)
#endif
#ifndef MOD_INC_USE_COUNT
#define MOD_INC_USE_COUNT			do {} while (0)
#endif
#ifndef MOD_DEC_USE_COUNT
#define MOD_DEC_USE_COUNT			do {} while (0)
#endif
#define OLD_MOD_INC_USE_COUNT		MOD_INC_USE_COUNT
#define OLD_MOD_DEC_USE_COUNT		MOD_DEC_USE_COUNT

#ifndef SET_NETDEV_DEV
#define SET_NETDEV_DEV(net, pdev)	do {} while (0)
#endif

/* suspend args */
#define DRV_SUSPEND_STATE_TYPE pm_message_t

#define CHECKSUM_HW	CHECKSUM_PARTIAL

typedef struct {
	void	*parent;  /* some external entity that the thread supposed to work for */
	struct	task_struct *p_task;
	long	thr_pid;
	int		prio; /* priority */
	struct	semaphore sema;
	int	terminated;
	struct	completion completed;
	spinlock_t	spinlock;
	int		up_cnt;
} tsk_ctl_t;

/* requires  tsk_ctl_t tsk  argument, the caller's priv data is passed in owner ptr */
/* note this macro assumes there may be only one context waiting on thread's completion */
#ifdef DHD_DEBUG
#define DBG_THR(x) printk x
#else
#define DBG_THR(x)
#endif

static inline bool binary_sema_down(tsk_ctl_t *tsk)
{
	if (down_interruptible(&tsk->sema) == 0) {
		unsigned long flags = 0;
		spin_lock_irqsave(&tsk->spinlock, flags);
		if (tsk->up_cnt == 1)
			tsk->up_cnt--;
		else {
			DBG_THR(("dhd_dpc_thread: Unexpected up_cnt %d\n", tsk->up_cnt));
		}
		spin_unlock_irqrestore(&tsk->spinlock, flags);
		return false;
	} else
		return true;
}

static inline bool binary_sema_up(tsk_ctl_t *tsk)
{
	bool sem_up = false;
	unsigned long flags = 0;

	spin_lock_irqsave(&tsk->spinlock, flags);
	if (tsk->up_cnt == 0) {
		tsk->up_cnt++;
		sem_up = true;
	} else if (tsk->up_cnt == 1) {
		/* dhd_sched_dpc: dpc is alread up! */
	} else
		DBG_THR(("dhd_sched_dpc: unexpected up cnt %d!\n", tsk->up_cnt));

	spin_unlock_irqrestore(&tsk->spinlock, flags);

	if (sem_up)
		up(&tsk->sema);

	return sem_up;
}

#define PROC_START(thread_func, owner, tsk_ctl, flags, name) \
{ \
	sema_init(&((tsk_ctl)->sema), 0); \
	init_completion(&((tsk_ctl)->completed)); \
	(tsk_ctl)->parent = owner; \
	(tsk_ctl)->terminated = FALSE; \
	(tsk_ctl)->p_task  = kthread_run(thread_func, tsk_ctl, (char*)name); \
	if (IS_ERR((tsk_ctl)->p_task)) { \
		(tsk_ctl)->thr_pid = -1; \
		DBG_THR(("%s(): thread:%s create failed\n", __FUNCTION__, \
			(name))); \
	} else { \
		(tsk_ctl)->thr_pid = (tsk_ctl)->p_task->pid; \
		tkcore_add_task((tsk_ctl)->p_task); \
		spin_lock_init(&((tsk_ctl)->spinlock)); \
		DBG_THR(("%s(): thread:%s:%lx started\n", __FUNCTION__, \
			(name), (tsk_ctl)->thr_pid)); \
	}; \
}

#define PROC_STOP(tsk_ctl) \
{ \
	DBG_THR(("%s(): thread:%s:%lx terminating\n", __FUNCTION__, \
			 (tsk_ctl)->p_task->comm, (tsk_ctl)->thr_pid)); \
	(tsk_ctl)->terminated = TRUE; \
	smp_wmb(); \
	up(&((tsk_ctl)->sema));	\
	wait_for_completion(&((tsk_ctl)->completed)); \
	tkcore_remove_task((tsk_ctl)->p_task); \
	(tsk_ctl)->thr_pid = -1; \
}

/*  ----------------------- */

#define KILL_PROC(nr, sig) \
{ \
struct task_struct *tsk; \
struct pid *pid;    \
pid = find_get_pid((pid_t)nr);    \
tsk = pid_task(pid, PIDTYPE_PID);    \
if (tsk) send_sig(sig, tsk, 1); \
}

#include <linux/time.h>
#include <linux/wait.h>

#define DEV_PRIV(dev)	netdev_priv(dev)

#define WL_ISR(i, d, p)	wl_isr((i), (d))

#define CAN_SLEEP()	((!in_atomic() && !irqs_disabled()))

#define KMALLOC_FLAG (CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
#define RANDOM32	get_random_u32
#else
#define RANDOM32	prandom_u32
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0) */

#define SRANDOM32(entropy)	prandom_seed(entropy)

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
#define PCI_IRQ_LEGACY      (1 << 0) /* allow legacy interrupts */
#define PCI_IRQ_MSI	        (1 << 1) /* allow MSI interrupts */
#define pci_alloc_irq_vectors(dev, minv, maxv, flags) pci_enable_msi_range(dev, minv, maxv)
#define pci_free_irq_vectors  pci_disable_msi
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)) */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static inline int __my_kernel_read(struct file *f, void *buf, size_t len, loff_t *pos)
{
	int ret = kernel_read(f, *pos, buf, len);
	if (ret > 0)
		*pos += ret;
	return ret;
}

static inline int __my_kernel_write(struct file *f, void *buf, size_t len, loff_t *pos)
{
	int ret = kernel_write(f, buf, len, *pos);
	if (ret > 0)
		*pos += ret;
	return ret;
}

#define kernel_read	__my_kernel_read
#define kernel_write	__my_kernel_write
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0) */

/* struct netdev_ops::ndo_get_stats64 return changed to void in v4.11 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
#define STATS64_RETURN_TYPE	struct rtnl_link_stats64*
#define RETURN_STATS64(_stats)	return (_stats)
#else
#define STATS64_RETURN_TYPE	void
#define RETURN_STATS64(_stats)	return
#endif /* KERNEL_VERSION < 4.11 */

static inline
void wl_netdev_set_free_netdev(struct net_device *net)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
	net->destructor = free_netdev;
#else
	net->needs_free_netdev = true;
#endif
}

static inline
void wl_netdev_set_destructor(struct net_device *net,
	void (*des)(struct net_device *net))
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
	net->destructor = des;
#else
	net->priv_destructor = des;
	net->needs_free_netdev = true;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
#define uaccess_kernel()	segment_eq(get_fs(), KERNEL_DS)
#endif /* KERNEL < 4.12 */

/*
 * eth_hw_addr_set() was introduced in 5.15, but it is being back-ported
 * maintenance branches. 4.19 got this in 4.19.291 and our router kernel
 * (4.19.294) includes this function.
 * However LINUX_VERSION_CODE macro tops out at 4.19.255 because of legacy
 * reasons. So we can't quite test against it. No SUBLEVEL-related macro seems
 * to be in the kernel in this release. So the contract here is the router build
 * provides CONFIG_HAVE_ETH_HW_ADDR and we check against that.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)) && \
	!defined(CONFIG_HAVE_ETH_HW_ADDR)
static inline void eth_hw_addr_set(struct net_device *dev, const u8 *addr)
{
	memcpy(dev->dev_addr, addr, ETH_ALEN);
}
#endif /* KERNEL < 5.15 && !CONFIG_HAVE_ETH_HW_ADDR */

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
#define kthread_complete_and_exit	complete_and_exit
#endif /* KERNEL < 5.17 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
#define pde_data(_i)	PDE_DATA(_i)
#endif /* KERNEL < 5.17 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 9)
typedef struct kernel_siginfo kernel_siginfo_t;
#else
typedef struct siginfo kernel_siginfo_t;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 9) */

/*
 * from 5.18 kernel the netif_rx() function can be called
 * from interrupt and process context and netif_rx_ni() is
 * no longer available.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
#define netif_rx_ni(_skb)	netif_rx(_skb)
#endif /* KERNEL >= 5.18 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)) || !defined(WL_CFG80211)
#define cfg80211_register_netdevice(_netdev)	register_netdevice(_netdev)
#define cfg80211_unregister_netdevice(_netdev)	unregister_netdevice(_netdev)
#endif /* KERNEL < 5.12 */

#endif /* _linuxver_h_ */
