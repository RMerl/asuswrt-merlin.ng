/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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
 * :>
 */

#ifndef _BDMF_SYSTEM_COMMON_H_
#define _BDMF_SYSTEM_COMMON_H_

/* Linux header files referenced */
#include <linux/time.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/irq.h>
#include <linux/in.h>
#include <linux/random.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/sort.h>

#include <bdmf_data_types.h>
#include <bdmf_queue.h>
#include <bdmf_errno.h>


#define __PACKING_ATTRIBUTE_STRUCT_END__ __attribute__ ((packed))
#define __PACKING_ATTRIBUTE_FIELD_LEVEL__

#define BDMF_FALSE 0
#define BDMF_TRUE 1

/*TODO check with Igor ,if we should create a bdmf_sysb_inline.h file
   I think we are uncessarily inlcuding these functions at several palces
   when its not needed
*/

/** \defgroup bdmf_system System Abstraction Layer
 *
 * This module implements a simple system abstraction.
 * BDMF currently supports the following systems
 * - linux kernel
 * - linux user space (simulation)
 * - VxWorks
 * @{
 */

/** \defgroup bdmf_system_mem Dynamic memory allocation
 * \ingroup bdmf_system
 * @{
 */

/** Allocate memory
 * \param[in]   size    block size
 * \returns memory block pointer or NULL
 */
void *bdmf_alloc(size_t size);

/** Allocate memory and clear
 * \param[in]   size    block size
 * \returns memory block pointer or NULL
 */
static inline void *bdmf_calloc(size_t size)
{
    void *p = bdmf_alloc(size);
    if (p)
        memset(p, 0, size);
    return p;
}

/** get time since epoch in usec (micro second)
 * \returns usec time since epoc
 */

static inline uint64_t bdmf_time_since_epoch_usec(void)
{
    struct timespec64 ts_64;
	ktime_get_real_ts64(&ts_64);
    return ts_64.tv_sec * USEC_PER_SEC + ts_64.tv_nsec / NSEC_PER_USEC;
}

void bdmf_free(void *p);

/** Allocate uncached memory
 * \param[in]   size           block size
 * \param[in]   phys_addr_p    physical memory block pointer
 * \returns virtual memory block pointer or NULL
 */
void *bdmf_alloc_uncached(int size, bdmf_phys_addr_t *phys_addr_p);

/** Release memory block allocated by
 * bdmf_alloc_uncached()
 * \param[in]   virt_p         virtual memory block pointer
 * \param[in]   phys_addr      physical memory block address
 * \param[in]   size           block size
 */
void bdmf_free_uncached(void *virt_p, bdmf_phys_addr_t phys_addr, int size);

/** @} end of bdmf_system_mem group */

/** \defgroup bdmf_system_bin_mutex Binary mutex
 * \ingroup bdmf_system
 * Task that attempts to take mutex that is already taken bdmfocks,
 * regardless on who holds the mutex.
 * @{
 */

typedef struct { char b[128]; } bdmf_mutex;

/** Initialize binary mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_mutex_init(bdmf_mutex *pmutex);

/** Lock binary mutex
 * \param[in]   pmutex  mutex handle
 * The caller blocks until the mutex becomes available
 * \return  0=mutex taken, <0-error (interrupted)
 */
int bdmf_mutex_lock(bdmf_mutex *pmutex);

/** Unlock binary mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_mutex_unlock(bdmf_mutex *pmutex);

/** Delete binary mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_mutex_delete(bdmf_mutex *pmutex);

/** @} */


/** \defgroup bdmf_system_ta_mutex Task-aware mutex
 * \ingroup bdmf_system
 * The same task can take the same mutex multiple times.
 * However, if task B attempts to take mutex that is already taken by task A,
 * it locks until the mutex becomes available.
 * If task takes task aware mutex multiple times, number of
 * bdmf_ta_mutex_unlock() calls must match bdmf_ta_mutex_lock()
 * @{
 */

typedef struct { char b[128]; } bdmf_ta_mutex;

/** Initialize task-aware mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_ta_mutex_init(bdmf_ta_mutex *pmutex);

/** Lock task-aware mutex
 * \param[in]   pmutex  mutex handle
 * If the mutex is taken by different task, the caller blocks
 * \return  0=mutex taken, <0-error (interrupted)
 */
int bdmf_ta_mutex_lock(bdmf_ta_mutex *pmutex);

/** Unlock task-aware mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_ta_mutex_unlock(bdmf_ta_mutex *pmutex);

/** Delete task-aware mutex
 * \param[in]   pmutex  mutex handle
 */
void bdmf_ta_mutex_delete(bdmf_ta_mutex *pmutex);

/** @} */

typedef struct { char b[128]; } bdmf_reent_fastlock;

void bdmf_reent_fastlock_init(bdmf_reent_fastlock *lock);
int bdmf_reent_fastlock_lock(bdmf_reent_fastlock *lock);
void bdmf_reent_fastlock_unlock(bdmf_reent_fastlock *lock);

/** \defgroup bdmf_system_fastlock Fast lock
 * \ingroup bdmf_system
 * Fastlock primitives can be used in interrupt context
 * @{
 */
/* Fast lock/unlock */
typedef spinlock_t bdmf_fastlock;

/** Init fast lock
 * \param[in]   plock   Fastlock handle
 */

#define DEFINE_BDMF_FASTLOCK(lock) bdmf_fastlock lock = __SPIN_LOCK_INITIALIZER(lock)

#define bdmf_fastlock_init(plock)               \
    do {                                        \
        spin_lock_init(plock);       \
    } while(0)

/** Take fast lock
 * \param[in]   plock   Fastlock handle
 */
#define bdmf_fastlock_lock(plock)               \
    do { \
        BUG_ON(in_irq()); \
        spin_lock_bh(plock); \
    } while (0)

/** Release fast lock
 * \param[in]   plock   Fastlock handle
 */
#define bdmf_fastlock_unlock(plock)             \
    spin_unlock_bh(plock)

/** Take fast lock in interrupt level
 * \param[in]   plock   Fastlock handle
 * \param[out]   flags   IRQ flags to save
 */
#define bdmf_fastlock_lock_irq(plock, flags)               \
    spin_lock_irqsave(plock, flags)

/** Release fast lock in interrupt level
 * \param[in]   plock   Fastlock handle
 * \param[in]   flags   IRQ flags to restore
 */
#define bdmf_fastlock_unlock_irq(plock, flags)             \
    spin_unlock_irqrestore(plock, flags)

/** @} */

/* Tasks */
typedef struct task_struct *bdmf_task;
int bdmf_task_create(const char *name, int priority, int stack,
    int (*handler)(void *arg), void *arg, bdmf_task *ptask);
#define BDMFSYS_DEFAULT_TASK_PRIORITY     (-1)
#define BDMFSYS_DEFAULT_TASK_STACK        (-1)
int bdmf_task_destroy(bdmf_task task);
#define bdmf_usleep(_us)       mdelay(_us)

const char *bdmf_task_get_name(bdmf_task task, char *name);
const bdmf_task bdmf_task_get_current(void);

/** \defgroup bdmf_system_file Printing and file IO
 * \ingroup bdmf_system
 * @{
 */

/** Print
 * \param[in]   format  printf-like format
 */
int bdmf_print(const char *format, ...) __attribute__((format(printf, 1, 2)));


/** Print
 * \param[in]   format  printf-like format
 * \param[in]   args    argument list
 */
int bdmf_vprint(const char *format, va_list args);

/** Print error message
 * \param[in]   format  printf-like format
 * \param[in]   args    arguments
 */
#define bdmf_print_error(format,args...)       bdmf_print("***Error in %s:%d>"format, __FUNCTION__, __LINE__, ## args)\

/* File IO */
typedef struct file *bdmf_file;

#define BDMF_FMODE_RDONLY   0x1     /**< Read-only file mode */
#define BDMF_FMODE_WRONLY   0x2     /**< Write-only file mode */
#define BDMF_FMODE_RDWR     0x3     /**< Read-Write file mode */
#define BDMF_FMODE_CREATE   0x4     /**< Create file if doesn't exist */
#define BDMF_FMODE_APPEND   0x8     /**< Append to existing file */
#define BDMF_FMODE_TRUNCATE 0x10    /**< Truncate file */
#define BDMF_FMODE_SYNC     0x20    /**< Sync file */

/**< Open file
 * \param[in]   fname   File name
 * \param[in]   flags   Combination of BDMF_FMODE_ flags
 * \returns file handle or NULL
 */
bdmf_file bdmf_file_open(const char *fname, int flags);

/** Close file
 * \param[in]   fd      File handle returned by bdmf_file_open
 */
void bdmf_file_close(bdmf_file fd);

/** Read from file
 * \param[in]   fd      File handle returned by bdmf_file_open
 * \param[in]   buf     Buffer to read into
 * \param[in]   size    Number of bytes to read
 * \returns number of bytes read >= 0 or error < 0
 */
int bdmf_file_read(bdmf_file fd, void *buf, uint32_t size);

/** Write to file
 * \param[in]   fd      File handle returned by bdmf_file_open
 * \param[in]   buf     Buffer
 * \param[in]   size    Number of bytes to write
 * \returns number of bytes written >= 0 or error < 0
 */
int bdmf_file_write(bdmf_file fd, const void *buf, uint32_t size);

/** @} */


/** \defgroup bdmf_system_cache Data cache flush / invalidation
 * \ingroup bdmf_system
 * @{
 */



/** @} */

/** \defgroup bdmf_system_sysb System network buffer support
 * \ingroup bdmf_system
 * @{
 */

/** System buffer type */
typedef enum
{
    bdmf_sysb_skb,          /**< sk_buff */
    bdmf_sysb_fkb,          /**< fkbuff */

    bdmf_sysb_type__num_of
} bdmf_sysb_type;

/** System buffer */
typedef void *bdmf_sysb;

void bdmf_sysb_recycle(bdmf_sysb sysb, unsigned long context, uint32_t flags);
void bdmf_sysb_recycle_skb(bdmf_sysb sysb, unsigned long context, uint32_t flags);

/** Get sysb type
 * \param[in]   sysb        System buffer
 * \return system buffer type
 */



# if 0

/** Reserve headroom
 *
 * The function will assert if called for sysb containing data
 *
 * \param[in]   sysb        System buffer
 * \param[in]   bytes       Bytes to reserve
 */
static inline void bdmf_sysb_reserve(const bdmf_sysb sysb, uint32_t bytes)
{
    BUG_ON(bdmf_sysb_typeof(sysb) != bdmf_sysb_skb);
    skb_reserve((struct sk_buff *)sysb, bytes);
}
#endif


/** @} */

/** \defgroup bdmf_system_timer Timer support
 * \ingroup bdmf_system
 * @{
 */

/** timer handle */
typedef struct bdmf_timer bdmf_timer_t;

/** timer callback function
 * \param[in]   timer   timer that has expired
 * \param[in]   priv    private cookie passed in bdmf_timer_init()
 */
typedef void (*bdmf_timer_cb_t)(bdmf_timer_t *timer, unsigned long priv);

struct bdmf_timer
{
    struct timer_list timer;
    bdmf_timer_cb_t cb;
    unsigned long priv;
};

/** Initialize timer
 * \param[in]   timer   timer handle
 * \param[in]   cb      callback to be called upon expiration
 * \param[in]   priv    private cooke to be passed to cb()
 */
void bdmf_timer_init(bdmf_timer_t *timer, bdmf_timer_cb_t cb, unsigned long priv);

/** Start timer
 * \param[in]   timer   timer handle that has been initialized using bdmf_timer_init()
 * \param[in]   ticks   number of ticks from now to expiration
 * \returns 0=OK or error code <0
 */
static inline int bdmf_timer_start(bdmf_timer_t *timer, uint32_t ticks)
{
    mod_timer(&timer->timer, jiffies + ticks);
    return 0;
}

/** stop timer
 * \param[in]   timer   timer to be stopped
 * The function is safe to call even if timer is not running
 * The function waits for the handler to finish thus can't be called from interrupt context
 */
static inline void bdmf_timer_stop(bdmf_timer_t *timer)
{
    del_timer_sync(&timer->timer);
}

/** stop timer no sync
 * \param[in]   timer   timer to be stopped
 * The function is safe to call even if timer is not running
 */
static inline void bdmf_timer_stop_no_sync(bdmf_timer_t *timer)
{
    del_timer(&timer->timer);
}

/** Delete timer
 * \param[in]   timer   timer to be deleted
 * The timer is stopped if running
 */
static inline void bdmf_timer_delete(bdmf_timer_t *timer)
{
    bdmf_timer_stop(timer);
}

/* Get CPU frequency. */
static inline uint32_t bdmf_get_cpu_frequency(void)
{
    return HZ;
}

/** Convert ms to ticks
 * \param[in]   ms  ms
 * \returns timer ticks
 */
static inline uint32_t bdmf_ms_to_ticks(uint32_t ms)
{
    return msecs_to_jiffies(ms);
}

/** Convert us to ticks
 * \param[in]   us  us
 * \returns timer ticks
 */
static inline uint32_t bdmf_us_to_ticks(uint32_t us)
{
    return usecs_to_jiffies(us);
}

/** @} */


/** \defgroup bdmf_system_endian Big/little Endian support
 * \ingroup bdmf_system
 *
 * Endian-related macros and constants. The following constants must be defined
 *
 *  - __BYTE_ORDER define is set by GCC compilation environment. No need to do anything here.
 *  The following must be defined:
 *  - __BYTE_ORDER
 *  - __LITTLE_ENDIAN
 *  - __BIG_ENDIAN
 *  - __bswap_16(x)
 *  - __bswap_32(x)
 */

/*-----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
#ifdef __LITTLE_ENDIAN
# define __BYTE_ORDER   __LITTLE_ENDIAN
#endif
#ifdef __BIG_ENDIAN
# define __BYTE_ORDER   __BIG_ENDIAN
#endif
#define __bswap_16(x) ___arch__swab16(x)
#define __bswap_32(x) ___arch__swab32(x)



/*
 * Interrupt control
 */

#define BDMF_IRQ_NONE       IRQ_NONE        /**< IRQ is not from this device */
#define BDMF_IRQ_HANDLED    IRQ_HANDLED     /**< IRQ has been handled */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
#define BDMF_IRQF_DISABLED  IRQF_DISABLED   /**< All interrupt are disabled when ISR is called */
#else
#define BDMF_IRQF_DISABLED  0
#endif

/** Connect system interrupt
 * \param[in]   irq     IRQ number
 * \param[in]   cpu     CPU number (for SMP)
 * \param[in]   flags   IRQ flags
 * \param[in]   isr     ISR
 * \param[in]   name    device name
 * \param[in]   priv    Private cookie
 * \returns 0=OK, <0- error
 */
int bdmf_int_connect(int irq, int cpu, int flags,
    int (*isr)(int irq, void *priv), const char *name, void *priv);

/** Disconnect system interrupt
 * \param[in]   irq     IRQ number
 * \param[in]   priv    Private cookie passed in bdmf_int_connect()
 * \returns 0=OK, <0- error
 */
void bdmf_int_disconnect(int irq, void *priv);

/** Unmask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_enable(int irq);

/** Mask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_disable(int irq);

/*-----------------------------------------------------------------------
 * A few standard C library functions that are absent in linux kernel
 *----------------------------------------------------------------------*/
#define strtoul simple_strtoul
#define strtol simple_strtol
#define strtoull simple_strtoull
#define strtoll simple_strtoll
static inline const char *strerror(int rc)
{
        static char serr[20];
        sprintf(serr, "err -%d", rc);
        return serr;
}

#define assert(cond)   BUG_ON(!(cond))

#define bdmf_sort(base, num, size, cmp_func, swap_func) \
    sort(base, num, size, cmp_func, swap_func)

/** \defgroup bdmf_rand Random numbers generation support
 * \ingroup bdmf_system
 * @{
 */

/** Return 16 bit random number
 * \return random number
 */
static inline uint16_t bdmf_rand16(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
    return prandom_u32() & 0xFFFF;
#else
    return random32() & 0xFFFF;
#endif
}

/** Seed random number generation
 * \param[in]   seed      seed
 */
static inline void bdmf_srand(unsigned int seed)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
    prandom_seed(seed);
#else
    srandom32(seed);
#endif
}
/** @} end of bdmf_rand group */

/** \defgroup bdmf_queue message queue support
 * \ingroup bdmf_system
 * @{
 */
typedef struct bdmf_queue_message_t bdmf_queue_message_t;

typedef struct bdmf_queue
{
    struct list_head free_message_list;
    struct list_head full_message_list;
    uint32_t number_of_messages;
    uint32_t number_of_free_messages;
    void *p_messages_data_buffer;
    bdmf_queue_message_t *p_structs_buffer;
    struct semaphore receive_waiting_semaphore;
    bdmf_fastlock lock;
} bdmf_queue_t;

bdmf_error_t bdmf_queue_create(bdmf_queue_t *queue, uint32_t xi_number_of_messages, uint32_t xi_max_message_length);
bdmf_error_t bdmf_queue_delete(bdmf_queue_t *queue);
bdmf_error_t bdmf_queue_send(bdmf_queue_t *queue, char *xi_buffer, uint32_t xi_length);
bdmf_error_t bdmf_queue_receive(bdmf_queue_t *queue, char *xo_buffer, uint32_t *xio_length);

/** @} end of bdmf_queue group */

/** \defgroup bdmf_queue message queue support
 * \ingroup bdmf_system
 * @{
 */

typedef struct semaphore bdmf_semaphore_t;

void bdmf_semaphore_create(bdmf_semaphore_t *semaphore, int val);
void bdmf_semaphore_give(bdmf_semaphore_t *sem);
void bdmf_semaphore_take(bdmf_semaphore_t *sem);

/** @} end of bdmf_queue group */

/* Character IO. Implementation is in bdmf_chrdev.c */

/* Read character from STDIN */
int bdmf_getchar(void);

/* Write character to STDOUT */
void bdmf_putchar(int c);

/** @} end of bdmf_system group */

#endif /* _BDMF_SYSTEM_H_ */
