/*
 * Linux-specific abstractions to gain some independence from linux kernel versions.
 * Pave over some 2.2 versus 2.4 versus 2.6 kernel differences.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: linuxver.h 830213 2023-09-16 13:30:43Z $
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
#include <linux/config.h>
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33))
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
#include <linux/kconfig.h>
#endif
#include <linux/module.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 0))
/* __NO_VERSION__ must be defined for all linkables except one in 2.2 */
#ifdef __UNDEF_NO_VERSION__
#undef __NO_VERSION__
#else
#define __NO_VERSION__
#endif
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#define module_param(_name_, _type_, _perm_)	MODULE_PARM(_name_, "i")
#define module_param_string(_name_, _string_, _size_, _perm_) \
		MODULE_PARM(_string_, "c" __MODULE_STRING(_size_))
#endif

/* linux/malloc.h is deprecated, use linux/slab.h instead. */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 9))
#include <linux/malloc.h>
#else
#include <linux/slab.h>
#endif

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28))
#undef IP_TOS
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)) */
#include <asm/io.h>

#include <linux/workqueue.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
#define DAEMONIZE(a)	do { \
		allow_signal(SIGKILL);	\
		allow_signal(SIGTERM);	\
	} while (0)
#elif ((LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)))
#define DAEMONIZE(a) daemonize(a); \
	allow_signal(SIGKILL); \
	allow_signal(SIGTERM);
#else /* Linux 2.4 (w/o preemption patch) */
#define RAISE_RX_SOFTIRQ() \
	cpu_raise_softirq(smp_processor_id(), NET_RX_SOFTIRQ)
#define DAEMONIZE(a) daemonize(); \
	do { if (a) \
		strncpy(current->comm, a, MIN(sizeof(current->comm), (strlen(a)))); \
	} while (0);
#endif /* LINUX_VERSION_CODE  */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
/* Some distributions have their own 2.6.x compatibility layers */
#ifndef IRQ_NONE
typedef void irqreturn_t;
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
#endif
#else
typedef irqreturn_t(*FN_ISR) (int irq, void *dev_id, struct pt_regs *ptregs);
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18)
#define IRQF_SHARED	SA_SHIRQ
#endif /* < 2.6.18 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 17)
#ifdef	CONFIG_NET_RADIO
#define	CONFIG_WIRELESS_EXT
#endif
#endif	/* < 2.6.17 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 67)
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 67) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
#include <linux/sched.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#include <linux/sched/rt.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <uapi/linux/sched/types.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#include <net/lib80211.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#include <linux/ieee80211.h>
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#include <net/ieee80211.h>
#endif
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */

#ifndef __exit
#define __exit
#endif
#ifndef __devexit
#define __devexit
#endif
#ifndef __devinit
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
#define __devinit	__init
#else
/* All devices are hotpluggable since linux 3.8.0 */
#define __devinit
#endif
#endif /* !__devinit */
#ifndef __devinitdata
#define __devinitdata
#endif
#ifndef __devexit_p
#define __devexit_p(x)	x
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0))

#define pci_get_drvdata(dev)		(dev)->sysdata
#define pci_set_drvdata(dev, value)	(dev)->sysdata = (value)

/*
 * New-style (2.4.x) PCI/hot-pluggable PCI/CardBus registration
 */

struct pci_device_id {
	unsigned int vendor, device;		/* Vendor and device ID or PCI_ANY_ID */
	unsigned int subvendor, subdevice;	/* Subsystem ID's or PCI_ANY_ID */
	unsigned int class, class_mask;		/* (class,subclass,prog-if) triplet */
	unsigned long driver_data;		/* Data private to the driver */
};

struct pci_driver {
	struct list_head node;
	char *name;
	const struct pci_device_id *id_table;	/* NULL if wants all devices */
	int (*probe)(struct pci_dev *dev,
	             const struct pci_device_id *id); /* New device inserted */
	void (*remove)(struct pci_dev *dev);	/* Device removed (NULL if not a hot-plug
						 * capable driver)
						 */
	void (*suspend)(struct pci_dev *dev);	/* Device suspended */
	void (*resume)(struct pci_dev *dev);	/* Device woken up */
};

#define MODULE_DEVICE_TABLE(type, name)
#define PCI_ANY_ID (~0)

/* compatpci.c */
#define pci_module_init pci_register_driver
extern int pci_register_driver(struct pci_driver *drv);
extern void pci_unregister_driver(struct pci_driver *drv);

#endif /* PCI registration */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
#define pci_module_init pci_register_driver
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 2, 18))
#ifdef MODULE
#define module_init(x) int init_module(void) { return x(); }
#define module_exit(x) void cleanup_module(void) { x(); }
#else
#define module_init(x)	__initcall(x);
#define module_exit(x)	__exitcall(x);
#endif
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2, 2, 18) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
#define WL_USE_NETDEV_OPS
#else
#undef WL_USE_NETDEV_OPS
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)) && defined(CONFIG_RFKILL)
#define WL_CONFIG_RFKILL
#else
#undef WL_CONFIG_RFKILL
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 48))
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 13))
#define pci_resource_start(dev, bar)	((dev)->base_address[(bar)])
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 44))
#define pci_resource_start(dev, bar)	((dev)->resource[(bar)].start)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 23))
#define pci_enable_device(dev) do { } while (0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 14))
#define net_device device
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 42))

/*
 * DMA mapping
 *
 * See linux/Documentation/DMA-mapping.txt
 */

#ifndef PCI_DMA_TODEVICE
#define	PCI_DMA_TODEVICE	1
#define	PCI_DMA_FROMDEVICE	2
#endif

typedef u32 dma_addr_t;

/* Pure 2^n version of get_order */
static inline int get_order(unsigned long size)
{
	int order;

	size = (size-1) >> (PAGE_SHIFT-1);
	order = -1;
	do {
		size >>= 1;
		order++;
	} while (size);
	return order;
}

static inline void *pci_alloc_consistent(struct pci_dev *hwdev, size_t size,
                                         dma_addr_t *dma_handle)
{
	void *ret;
	int gfp = GFP_ATOMIC | GFP_DMA;

	ret = (void *)__get_free_pages(gfp, get_order(size));

	if (ret != NULL) {
		memset(ret, 0, size);
		*dma_handle = virt_to_bus(ret);
	}
	return ret;
}

static inline void pci_free_consistent(struct pci_dev *hwdev, size_t size,
                                       void *vaddr, dma_addr_t dma_handle)
{
	free_pages((unsigned long)vaddr, get_order(size));
}

#ifdef ILSIM
extern uint pci_map_single(void *dev, void *va, uint size, int direction);
extern void pci_unmap_single(void *dev, uint pa, uint size, int direction);
#else
#define pci_map_single(cookie, address, size, dir)	virt_to_bus(address)
#define pci_unmap_single(cookie, address, size, dir)
#endif

#endif /* DMA mapping */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 43))

#define dev_kfree_skb_any(a)		dev_kfree_skb(a)
#define netif_down(dev)			do { (dev)->start = 0; } while (0)

#ifndef _COMPAT_NETDEVICE_H

/*
 * SoftNet
 *
 * For pre-softnet kernels we need to tell the upper layer not to
 * re-enter start_xmit() while we are in there. However softnet
 * guarantees not to enter while we are in there so there is no need
 * to do the netif_stop_queue() dance unless the transmit queue really
 * gets stuck. This should also improve performance according to tests
 * done by Aman Singla.
 */

#define dev_kfree_skb_irq(a)	dev_kfree_skb(a)
#define netif_wake_queue(dev) \
		do { clear_bit(0, &(dev)->tbusy); mark_bh(NET_BH); } while (0)
#define netif_stop_queue(dev)	set_bit(0, &(dev)->tbusy)

static inline void netif_start_queue(struct net_device *dev)
{
	dev->tbusy = 0;
	dev->interrupt = 0;
	dev->start = 1;
}

#define netif_queue_stopped(dev)	(dev)->tbusy
#define netif_running(dev)		(dev)->start

#endif /* _COMPAT_NETDEVICE_H */

#define netif_device_attach(dev)	netif_start_queue(dev)
#define netif_device_detach(dev)	netif_stop_queue(dev)

/* 2.4.x renamed bottom halves to tasklets */
#define tasklet_struct				tq_struct
static inline void tasklet_schedule(struct tasklet_struct *tasklet)
{
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

static inline void tasklet_init(struct tasklet_struct *tasklet,
                                void (*func)(unsigned long),
                                unsigned long data)
{
	tasklet->next = NULL;
	tasklet->sync = 0;
	tasklet->routine = (void (*)(void *))func;
	tasklet->data = (void *)data;
}

#define tasklet_kill(tasklet)	{ do {} while (0); }

/* 2.4.x introduced del_timer_sync() */
#define del_timer_sync(timer) del_timer(timer)

#else

#define netif_down(dev)

#endif /* SoftNet */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 3))

/*
 * Emit code to initialise a tq_struct's routine and data pointers
 */
#define PREPARE_TQUEUE(_tq, _routine, _data)			\
	do {							\
		(_tq)->routine = _routine;			\
		(_tq)->data = _data;				\
	} while (0)

/*
 * Emit code to initialise all of a tq_struct
 */
#define INIT_TQUEUE(_tq, _routine, _data)			\
	do {							\
		INIT_LIST_HEAD(&(_tq)->list);			\
		(_tq)->sync = 0;				\
		PREPARE_TQUEUE((_tq), (_routine), (_data));	\
	} while (0)

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 3) */

/* Power management related macro & routines */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 9)
#define	PCI_SAVE_STATE(a, b)	pci_save_state(a)
#define	PCI_RESTORE_STATE(a, b)	pci_restore_state(a)
#else
#define	PCI_SAVE_STATE(a, b)	pci_save_state(a, b)
#define	PCI_RESTORE_STATE(a, b)	pci_restore_state(a, b)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6))
static inline int
pci_save_state(struct pci_dev *dev, u32 *buffer)
{
	int i;
	if (buffer) {
		for (i = 0; i < 16; i++)
			pci_read_config_dword(dev, i * 4, &buffer[i]);
	}
	return 0;
}

static inline int
pci_restore_state(struct pci_dev *dev, u32 *buffer)
{
	int i;

	if (buffer) {
		for (i = 0; i < 16; i++)
			pci_write_config_dword(dev, i * 4, buffer[i]);
	}
	/*
	 * otherwise, write the context information we know from bootup.
	 * This works around a problem where warm-booting from Windows
	 * combined with a D3(hot)->D0 transition causes PCI config
	 * header data to be forgotten.
	 */
	else {
		for (i = 0; i < 6; i ++)
			pci_write_config_dword(dev,
			                       PCI_BASE_ADDRESS_0 + (i * 4),
			                       pci_resource_start(dev, i));
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
	}
	return 0;
}
#endif /* PCI power management */

/* Old cp0 access macros deprecated in 2.4.19 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 19))
#define read_c0_count() read_32bit_cp0_register(CP0_COUNT)
#endif

/* Module refcount handled internally in 2.6.x */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev)		do {} while (0)
#define OLD_MOD_INC_USE_COUNT		MOD_INC_USE_COUNT
#define OLD_MOD_DEC_USE_COUNT		MOD_DEC_USE_COUNT
#else
#define OLD_MOD_INC_USE_COUNT		do {} while (0)
#define OLD_MOD_DEC_USE_COUNT		do {} while (0)
#endif
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24) */
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
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24) */

#ifndef SET_NETDEV_DEV
#define SET_NETDEV_DEV(net, pdev)	do {} while (0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
#ifndef HAVE_FREE_NETDEV
#define free_netdev(dev)		kfree(dev)
#endif
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0) */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
/* struct packet_type redefined in 2.6.x */
#define af_packet_priv			data
#endif

/* suspend args */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11)
#define DRV_SUSPEND_STATE_TYPE pm_message_t
#else
#define DRV_SUSPEND_STATE_TYPE uint32
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
#define CHECKSUM_HW	CHECKSUM_PARTIAL
#endif

typedef struct {
	void	*parent;  /* some external entity that the thread supposed to work for */
	char	*proc_name;
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
	(tsk_ctl)->proc_name = name;  \
	(tsk_ctl)->terminated = FALSE; \
	(tsk_ctl)->p_task  = kthread_run(thread_func, tsk_ctl, (char*)name); \
	if (IS_ERR((tsk_ctl)->p_task)) { \
		(tsk_ctl)->thr_pid = -1; \
		DBG_THR(("%s(): thread:%s create failed\n", __FUNCTION__, \
			(tsk_ctl)->proc_name)); \
	} else { \
		(tsk_ctl)->thr_pid = (tsk_ctl)->p_task->pid; \
		spin_lock_init(&((tsk_ctl)->spinlock)); \
		DBG_THR(("%s(): thread:%s:%lx started\n", __FUNCTION__, \
			(tsk_ctl)->proc_name, (tsk_ctl)->thr_pid)); \
	}; \
}

#define PROC_STOP(tsk_ctl) \
{ \
	(tsk_ctl)->terminated = TRUE; \
	smp_wmb(); \
	up(&((tsk_ctl)->sema));	\
	wait_for_completion(&((tsk_ctl)->completed)); \
	DBG_THR(("%s(): thread:%s:%lx terminated OK\n", __FUNCTION__, \
			 (tsk_ctl)->proc_name, (tsk_ctl)->thr_pid)); \
	(tsk_ctl)->thr_pid = -1; \
}

/*  ----------------------- */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31))
#define KILL_PROC(nr, sig) \
{ \
struct task_struct *tsk; \
struct pid *pid;    \
pid = find_get_pid((pid_t)nr);    \
tsk = pid_task(pid, PIDTYPE_PID);    \
if (tsk) send_sig(sig, tsk, 1); \
}
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && (LINUX_VERSION_CODE <= \
	KERNEL_VERSION(2, 6, 30))
#define KILL_PROC(pid, sig) \
{ \
	struct task_struct *tsk; \
	tsk = find_task_by_vpid(pid); \
	if (tsk) send_sig(sig, tsk, 1); \
}
#else
#define KILL_PROC(pid, sig) \
{ \
	kill_proc(pid, sig, 1); \
}
#endif
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/time.h>
#include <linux/wait.h>
#else
#include <linux/sched.h>

#define __wait_event_interruptible_timeout(wq, condition, ret)		\
do {									\
	wait_queue_t __wait;						\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			ret = schedule_timeout(ret);			\
			if (!ret)					\
				break;					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

#define wait_event_interruptible_timeout(wq, condition, timeout)	\
({									\
	long __ret = timeout;						\
	if (!(condition))						\
		__wait_event_interruptible_timeout(wq, condition, __ret); \
	__ret;								\
})

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)) */

/*
For < 2.6.24, wl creates its own netdev but doesn't
align the priv area like the genuine alloc_netdev().
Since netdev_priv() always gives us the aligned address, it will
not match our unaligned address for < 2.6.24
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define DEV_PRIV(dev)	(dev->priv)
#else
#define DEV_PRIV(dev)	netdev_priv(dev)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
#define WL_ISR(i, d, p)         wl_isr((i), (d))
#else
#define WL_ISR(i, d, p)         wl_isr((i), (d), (p))
#endif  /* < 2.6.20 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
#define netdev_priv(dev) dev->priv
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25))
#define CAN_SLEEP()	((!in_atomic() && !irqs_disabled()))
#else
#define CAN_SLEEP()	(FALSE)
#endif

#define KMALLOC_FLAG (CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
#define RANDOM32	prandom_u32
#else
#define RANDOM32	random32
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
#define SRANDOM32(entropy)	prandom_seed(entropy)
#else
#define SRANDOM32(entropy)	srandom32(entropy)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0) */

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
static inline struct inode *file_inode(const struct file *f)
{
	return f->f_dentry->d_inode;
}
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)) */

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
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
	return vfs_write(f, buf, len, pos);
#else
	int ret = kernel_write(f, buf, len, *pos);
	if (ret > 0)
		*pos += ret;
	return ret;
#endif
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
/* Our STATIC_ASSERT uses variable-length array which is banned */
#ifndef STATIC_ASSERT
#warning "inclusion issue as STATIC_ASSERT is not defined here"
#else
#undef STATIC_ASSERT
#endif /* !STATIC_ASSERT */
#define STATIC_ASSERT(expr)	BUILD_BUG_ON(!(expr))
#endif /* KERNEL_VERSION >= 5.0 */

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

#endif /* _linuxver_h_ */
