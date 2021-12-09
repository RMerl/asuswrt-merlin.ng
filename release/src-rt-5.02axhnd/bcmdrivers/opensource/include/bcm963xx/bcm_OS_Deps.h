/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

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
#ifndef BCM_OS_DEPS_H
#define BCM_OS_DEPS_H

#ifdef NON_LINUX_BUILD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef NR_CPUS
#define NR_CPUS  2
#endif /* NR_CPUS */

/*Basic types not defined in stdint.h*/
#ifndef bool
typedef unsigned char       bool;
#endif /* bool */
#ifndef cycles_t
typedef unsigned char       cycles_t;
#endif /* cycles_t */
#ifndef ssize_t
typedef int                 ssize_t;
#endif /* ssize_t */
#ifndef size_t
typedef unsigned int        size_t;
#endif /* size_t */

#ifndef BUG
#define BUG(...)             do { } while(0)
#endif /* BUG */
void panic(char *s);

/*Not a std C function*/
size_t strnlen(const char *s, size_t maxlen);

struct net_device {
  unsigned int dev;
};

#define IN
#define OUT

#define true 1
#define false 0

//#define PAGE_SHIFT 0

#define virt_to_page(val1) 0
#define SetPageReserved(val1)

#define PAGE_SIZE 1
#define PAGE_MASK 0

cycles_t get_cycles(void);
#define read_c0_count()     0

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define min_t(type, x, y) ({  \
  type __min1 = (x);  \
  type __min2 = (y);  \
  __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({  \
  type __max1 = (x);  \
  type __max2 = (y);  \
  __max1 > __max2 ? __max1: __max2; })

// GCC-specific directives
#define __force

#define ETH_ALEN  6   /* Octets in one ethernet addr */

/*From linux/mii.h*/
#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_ADVERTISE       0x04        /* Advertisement control reg   */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */

/* Basic mode control register. */
#define BMCR_SPEED1000		    0x0040  /* MSB of Speed (1000)         */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */

/*WORK API*/

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
//  atomic_long_t data;
//  struct list_head entry;
  work_func_t func;
};
#define DECLARE_WORK(val1, val2) struct work_struct val1 = { .func = val2 }

int schedule_work(struct work_struct *work);

/* Kernel MODULE API */
#define __devinit
#define __exit
#define __user
#define __init

#define module_init(val1)
#define module_exit(val1)
#define module_param(...)
#define MODULE_PARM_DESC(...)
#define MODULE_DESCRIPTION(val1)
#define MODULE_VERSION(val1)
#define MODULE_LICENSE(val1)
#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(val1)
#endif /* EXPORT_SYMBOL */

#define kmem_cache_alloc(val1, val2) 0
#define kmem_cache_free(val1, val2)
#define kmem_cache_create(...) 0
#define kmem_cache_destroy(val)
#define KERNEL_VERSION(...) 0

#define BcmHalInterruptDisable(val1)  
#define BcmHalInterruptEnable(val1)  

struct module;
#define THIS_MODULE ((struct module *)0)

typedef struct {
  unsigned int lock;
} raw_spinlock_t;

typedef struct {
  raw_spinlock_t raw_lock;
} spinlock_t;

/* IRQ Save/Restore*/

/*Return nonzero if in interrupt state*/
#define in_interrupt() 0

void spin_lock_irqsave_f(spinlock_t *lock, unsigned long *flags);
#define spin_lock_irqsave(val1, val2) spin_lock_irqsave_f((val1), &(val2));

void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);

void local_bh_disable(void);
void local_bh_enable(void);

void local_irq_save_f(unsigned long* flags);
#define local_irq_save(val1) local_irq_save_f(&(val1))
void local_irq_restore(unsigned long flags);

#define irqs_disabled() 0

enum irq_return {
  IRQ_NONE,
  IRQ_HANDLED,
  IRQ_WAKE_THREAD,
};



#define CPHYSADDR(val1) ((unsigned long)(val1) & 0x1fffffff)
#define KSEG0ADDR(a)    (CPHYSADDR(a) | 0x80000000)
#define KSEG1ADDR(a)    (CPHYSADDR(a) | 0xa0000000)

/* Semaphore API*/
struct semaphore {
//  spinlock_t        lock;
  unsigned int      count;
//  struct list_head  wait_list;
};

void up(struct semaphore *sem);
void down(struct semaphore *sem);
int down_interruptible(struct semaphore *sem);
void sema_init(struct semaphore *sem, int val);
#define init_MUTEX(sem) sema_init(sem, 1)

struct task_struct {
  char comm[1];
};
/* current points to task structure of current. Field comm is a string containing the name of the current task.*/
extern struct task_struct *current;

/* kmalloc/kfree API */

#define kmalloc(sz,gfp) calloc(sz,1)
#define kfree(val) free(val)

/* This equals 0, but use constants in case they ever change */
#define GFP_NOWAIT            (1 << 0)
/* GFP_ATOMIC means both !wait (__GFP_WAIT not set) and use emergency pool */
#define GFP_ATOMIC            (1 << 1)
#define GFP_NOIO              (1 << 2)
#define GFP_NOFS              (1 << 3)
#define GFP_KERNEL            (1 << 4)
#define GFP_TEMPORARY         (1 << 5)
#define GFP_USER              (1 << 6)
#define GFP_HIGHUSER          (1 << 7)
#define GFP_HIGHUSER_MOVABLE  (1 << 8)

#ifdef CONFIG_NUMA
#define GFP_THISNODE          (1 << 8)
#else
#define GFP_THISNODE          (1 << 9)
#endif

/* This mask makes up all the page movable related flags */
#define GFP_MOVABLE_MASK (1 << 10)

/* Control page allocator reclaim behavior */
#define GFP_RECLAIM_MASK (1 << 11)

/* Control allocation constraints */
#define GFP_CONSTRAINT_MASK (1 << 12)

/* Do not use these with a slab allocator */
#define GFP_SLAB_BUG_MASK (1 << 13)

/* Flag - indicates that the buffer will be suitable for DMA.  Ignored on some
   platforms, used as appropriate on others */

#define GFP_DMA   (1 << 14)

/* 4GB DMA on some platforms */
#define GFP_DMA32 (1 << 15)

/* errno */
#define ENODATA 61
#define ENOMEM  12  /* Out of memory */
#define EINVAL  22  /* Invalid argument */
#define ENOENT  2   /* No such file or directory */
#define ENONET  64  /* Machine is not on the network */
#define EBUSY   16  /* Device or resource busy */
#define EPERM   1   /* Operation not permitted */
#define ESPIPE  29  /* Illegal seek */
#define ERESTARTSYS 512
#define EFAULT  14  /* Bad address */
#define EIO     5   /* I/O error */
#define EAGAIN  11  /* Try again */
#define ENOLINK 67  /* Link has been severed */

/* jiffies */
#define HZ 1000
uint32_t GetJiffies(void);
#define jiffies GetJiffies()

/* copy to/from user API */
unsigned long __copy_to_user(void __user *to, const void *from, unsigned long n);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);

/* printk API */
#define KERN_INFO
int printk(const char *s, ...);

/* Tasklet API*/
struct tasklet_struct
{
  struct tasklet_struct *next;
  unsigned long state;
  //atomic_t count;
  void (*func)(unsigned long);
  unsigned long data;
};

void tasklet_schedule(struct tasklet_struct *tasklet);
void tasklet_init(struct tasklet_struct *tasklet, void (*func)(unsigned long), unsigned long data);
void tasklet_disable(struct tasklet_struct *t);
void tasklet_enable(struct tasklet_struct *t);

/*Device file API*/
struct inode;

#ifndef loff_t
typedef long long loff_t;
#endif /* loff_t */

/*Open Device File Flags*/
#ifndef O_RDONLY
#define O_RDONLY  00000000
#endif /* O_RDONLY */
#ifndef O_WRONLY
#define O_WRONLY  00000001
#endif /* O_WRONLY */
#ifndef O_RDWR
#define O_RDWR    00000002
#endif /* O_RDWR */
#ifndef O_NONBLOCK
#define O_NONBLOCK  00004000
#endif /* O_NONBLOCK */

#define POLLIN (1 << 0)
#define POLLRDNORM (1 << 1)

#define FMODE_READ  1
#define FMODE_WRITE 2

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)

#define MAJOR(dev)  ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)  ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)  (((ma) << MINORBITS) | (mi))

#define POLLOUT 4
#define POLLWRNORM  POLLOUT

struct poll_table_struct;
typedef struct poll_table_struct poll_table;

struct file {
  unsigned int  f_flags;
  void*         private_data;
  unsigned int  f_mode;
};

struct file_operations {
  struct module *owner;
  loff_t (*llseek) (struct file *, loff_t, int);
  ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
  ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
//  ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
//  ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
//  int (*readdir) (struct file *, void *, filldir_t);
  unsigned int (*poll) (struct file *, struct poll_table_struct *);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
  long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
#else
  int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
#endif
//  int (*mmap) (struct file *, struct vm_area_struct *);
  int (*open) (struct inode *, struct file *);
//  int (*flush) (struct file *, fl_owner_t id);
  int (*release) (struct inode *, struct file *);
//  int (*fsync) (struct file *, struct dentry *, int datasync);
//  int (*aio_fsync) (struct kiocb *, int datasync);
//  int (*fasync) (int, struct file *, int);
//  int (*lock) (struct file *, int, struct file_lock *);
//  ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
//  unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
//  int (*check_flags)(int);
//  int (*flock) (struct file *, int, struct file_lock *);
//  ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
//  ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
//  int (*setlease)(struct file *, long, struct file_lock **);
};

int register_chrdev(unsigned int major, const char *name, const struct file_operations *fops);
void unregister_chrdev(unsigned int major, const char *name);
loff_t no_llseek(struct file *file, loff_t offset, int origin);
int nonseekable_open(struct inode *inode, struct file *filp);

/* /proc API */

typedef int (read_proc_t)(char *page, char **start, off_t off, int count, int *eof, void *data);
typedef int (write_proc_t)(struct file *file, const char __user *buffer, unsigned long count, void *data);

struct proc_dir_entry {
//  unsigned int low_ino;
//  unsigned short namelen;
//  const char *name;
//  mode_t mode;
//  nlink_t nlink;
//  uid_t uid;
//  gid_t gid;
//  loff_t size;
//  const struct inode_operations *proc_iops;
  /*
   * NULL ->proc_fops means "PDE is going away RSN" or
   * "PDE is just created". In either case, e.g. ->read_proc won't be
   * called because it's too late or too early, respectively.
   *
   * If you're allocating ->proc_fops dynamically, save a pointer
   * somewhere.
   */
  const struct file_operations *proc_fops;
//  struct proc_dir_entry *next, *parent, *subdir;
//  void *data;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
//  atomic_t count;   /* use count */
//  int pde_users;  /* number of callers into module in progress */
//  spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
//  struct completion *pde_unload_completion;
//  struct list_head pde_openers; /* who did ->open, but not ->release */
};

struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent);
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode, struct proc_dir_entry *parent);
void remove_proc_entry(const char *name, struct proc_dir_entry *parent);

struct proc_dir_entry *create_proc_read_entry(const char *name,
  mode_t mode, struct proc_dir_entry *base, 
  read_proc_t *read_proc, void * data);

/* Wait queue API*/
struct __wait_queue_head {
//  spinlock_t lock;
//  struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

/* Needed for __wake_up definition. */
#define TASK_INTERRUPTIBLE  1

void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p);
void init_waitqueue_head(wait_queue_head_t *q);
void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr_exclusive, void *key);
#define wake_up_interruptible(x)  __wake_up(x, TASK_INTERRUPTIBLE, 1, NULL)
#define wait_event_interruptible(val1, val2) 0

void get_random_bytes(void *buf, int nbytes);

/* Cache Flush API */
#define pgprot_noncached(val) 0

struct cache_info {
  uint32_t  linesz;
};

struct cpu_data {
  struct cache_info  dcache;
};

extern struct cpu_data current_cpu_data;

void flush_dcache_line(unsigned long vaddr);
void invalidate_dcache_line(unsigned long vaddr);

/* Timer API */
struct timer_list {
  //struct list_head entry;
  unsigned long expires;
  void (*function)(unsigned long);
  unsigned long data;
  //struct tvec_base *base;
};

void add_timer(struct timer_list *timer);
int del_timer(struct timer_list *timer);
void init_timer(struct timer_list *timer);

void udelay(int uSecs);
#define mdelay(n) udelay((n) * 1000)

void msleep(unsigned int msecs);

/*Preemption*/
#define preempt_count() 0

void preempt_enable(void);
void preempt_disable(void);
void smp_wmb(void);

/*blog*/
struct blog_t;
typedef struct blog_t Blog_t;

#else /* else of NON_LINUX_BUILD */

// LINUX BUILD INCLUDES *************************************************************************
// **********************************************************************************************
// **********************************************************************************************

#include <linux/smp.h>
#include <asm/io.h>
#ifdef CONFIG_MIPS
#include <asm/addrspace.h>
#include <asm/cpu-info.h>

/*prevent asm/asm.h from being included by r4kcache.h*/
#define __ASM_ASM_H
#include <asm/r4kcache.h>
#undef __ASM_ASM_H /*remove the evidence*/
#endif

#include <linux/timer.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/bug.h>
#include <linux/timex.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/random.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/fcntl.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/cdev.h>
#include <linux/mii.h>

// pktcmf

#include <linux/blog.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <asm/system.h>
#endif
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <net/ip.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
/* All devices are hotpluggable since linux 3.8.0 */
#define __devinit
#define __devexit
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
#define HLIST_FOR_EACH_ENTRY_RCU(tpos, pos, head, member)	\
	(void)(pos);						\
	hlist_for_each_entry_rcu((tpos), (head), member)
#else
#define HLIST_FOR_EACH_ENTRY_RCU(tpos, pos, head, member)	\
	hlist_for_each_entry_rcu((tpos), (pos), (head), member) 
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
#define is_ether_addr_same(addr1, addr2)    ether_addr_equal(addr1, addr2)
#else
#define is_ether_addr_same(addr1, addr2)    (!compare_ether_addr(addr1, addr2))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
#define GET_FDB_VLAN(fdb) ((fdb)->vid != VLAN_N_VID ? (fdb)->vid : 0)
#else
#define GET_FDB_VLAN(fdb) ((fdb)->vlan_id != VLAN_N_VID ? (fdb)->vlan_id : 0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
#define NETDEV_NOTIFIER_GET_DEV(data)    ((struct net_device *)(data))
#else
#define NETDEV_NOTIFIER_GET_DEV(data)    netdev_notifier_info_to_dev(data)
#endif

#include <bcm_intr.h>

#include <linux/semaphore.h>
#define init_MUTEX(sem) sema_init(sem, 1)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static inline struct inode *file_inode(struct file *file)
{
    return file->f_path.dentry->d_inode;
}

static inline void *PDE_DATA(const struct inode *inode)
{
    return PDE(inode)->data;
}
#endif

#endif /* else of NON_LINUX_BUILD */

#endif /* BCM_OS_DEPS_H */
