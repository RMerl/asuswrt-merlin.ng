// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
 * 
 */

#ifndef _BDMF_SYSTEM_H_
#define _BDMF_SYSTEM_H_

#include <linux/compat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BDMFSYS_STDIO
#include <stdint.h>
#include <errno.h>

#include <bdmf_errno.h>
#include <bdmf_data_types.h>

#define bdmf_sort(base, num, size, cmp_func, swap_func) \
		qsort(base, num, size, cmp_func)

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#define __PACKING_ATTRIBUTE_STRUCT_END__ __attribute__ ((packed))
#endif
#ifndef __PACKING_ATTRIBUTE_FIELD_LEVEL__
#define __PACKING_ATTRIBUTE_FIELD_LEVEL__
#endif

#define BDMF_RX_CSUM_VERIFIED_MASK   0x01
#define BDMF_RX_CSUM_VERIFIED_SHIFT  0
#define SECONDS_TO_USEC 1000000

extern uint32_t jiffies;

/* runner reserved ddr tables */
#define __exit_refok
#define __init

/* Allocate/release memory */
bdmf_phys_addr_t rsv_virt_to_phys(volatile void *vir_addr);
void *rsv_phys_to_virt(bdmf_phys_addr_t phys_addr);
void *bdmf_alloc_rsv(int size, bdmf_phys_addr_t *phys_addr);
void *bdmf_rsv_mem_init(void);
void bdmf_rsv_mem_destroy(void);
void bdmf_rsv_mem_stats(void);

static inline void *bdmf_alloc(size_t size)
{
	return malloc(size);
}

static inline void *bdmf_calloc(size_t size)
{
	void *p = bdmf_alloc(size);
	if (p)
		memset(p, 0, size);
	return p;
}

static inline void bdmf_free(void *p)
{
	free(p);
}

#define bdmf_alloc_uncached(size, phys_addr_p) bdmf_alloc_rsv(size, phys_addr_p)
#define bdmf_free_uncached(_virt_p, _phys_addr, _size) do { } while(0)

/* Input/Output */
#define bdmf_print(format,args...) printf(format, ## args)
#define bdmf_vprint(format,ap) vprintf(format, ap)
#define bdmf_print_error(format,args...) \
	bdmf_print("***Error in %s:%d>"format, __FUNCTION__, __LINE__, ## args)


/* Task-aware (recursive) mutex
 * The same task can take the same mutex multiple times.
 * However, if task B attempts to take mutex that is already taken by task A,
 * it will block
 */
typedef struct { int initialized; char b[128]; } bdmf_ta_mutex;
void bdmf_ta_mutex_init(bdmf_ta_mutex *pmutex);
int  bdmf_ta_mutex_lock(bdmf_ta_mutex *pmutex);
void bdmf_ta_mutex_unlock(bdmf_ta_mutex *pmutex);
void bdmf_ta_mutex_delete(bdmf_ta_mutex *pmutex);

#define __BDMF_TA_MUTEX_INITIALIZER(lock) {.initialized = 0}

typedef bdmf_ta_mutex bdmf_reent_fastlock;
#define bdmf_reent_fastlock_init(plock)  bdmf_ta_mutex_init(plock)
#define bdmf_reent_fastlock_lock(plock)  bdmf_ta_mutex_lock(plock)
#define bdmf_reent_fastlock_unlock(plock) bdmf_ta_mutex_unlock(plock)

typedef struct { int initialized; int locked; } bdmf_simple_mutex;
static inline void bdmf_simple_mutex_init(bdmf_simple_mutex *pmutex)
{
	pmutex->locked = 0;
	pmutex->initialized = 1;
}

static inline int bdmf_simple_mutex_lock(const char *func, int line,
					 bdmf_simple_mutex *pmutex)
{
	if (!pmutex->initialized)
		bdmf_simple_mutex_init(pmutex);
	if (pmutex->locked)
		bdmf_print("%s:%d> LOCK APPLIED WHEN INTERRUPTS LOCKED (expect "
			   "re-entrant?)!!!\n", func, line);

	pmutex->locked = 1;

	return 0;
}

static inline void bdmf_simple_mutex_unlock(const char *func, int line, bdmf_simple_mutex *pmutex)
{
	if (!pmutex->initialized)
		bdmf_simple_mutex_init(pmutex);

	if (!pmutex->locked)
		bdmf_print("%s:%d> UNLOCK APPLIED WHEN INTERRUPTS "
			   "UNLOCKED!!!\n", func, line);

	pmutex->locked = 0;
}

static inline void bdmf_simple_mutex_delete(bdmf_simple_mutex *pmutex)
{
	pmutex->locked = 0;
	pmutex->initialized = 0; 
}

#define __BDMF_SIMPLE_MUTEX_INITIALIZER(lock) {.initialized = 0}

/* Fast lock/unlock */
typedef bdmf_simple_mutex bdmf_fastlock;
#define DEFINE_BDMF_FASTLOCK(lock) \
	bdmf_fastlock lock = __BDMF_SIMPLE_MUTEX_INITIALIZER(lock)
#define bdmf_fastlock_init(plock) bdmf_simple_mutex_init(plock)
#define bdmf_fastlock_lock(plock) \
	bdmf_simple_mutex_lock(__FUNCTION__, __LINE__, plock)
#define bdmf_fastlock_unlock(plock) \
	bdmf_simple_mutex_unlock(__FUNCTION__, __LINE__, plock)
#define bdmf_fastlock_lock_irq(plock, flags) \
	do { \
		bdmf_simple_mutex_lock(__FUNCTION__, __LINE__, plock); \
		flags = 0; \
	} while (flags)
#define bdmf_fastlock_unlock_irq(plock, flags) \
	bdmf_simple_mutex_unlock(__FUNCTION__, __LINE__, plock)

/* mmap shared area */
void *bdmf_mmap(const char *fname, uint32_t size);

/* IRQ handling */
typedef int (*f_bdmf_irq_cb)(int irq, void *data);
#define BDMFSYS_IRQ__NUM_OF   64
int bdmf_irq_connect(int irq, f_bdmf_irq_cb cb, void *data);
int bdmf_irq_free(int irq, f_bdmf_irq_cb cb, void *data);
void bdmf_irq_raise(int irq);

/*
 * dcache
 */
#define BCM_MAX_PKT_LEN 2048 /* For simulation only */

static inline void bdmf_dcache_flush(unsigned long addr __attribute__((unused)),
				     unsigned long size __attribute__((unused)))
{
}

static inline void bdmf_dcache_inv(unsigned long addr __attribute__((unused)),
				   unsigned long size __attribute__((unused)))
{
}

/*-----------------------------------------------------------------------
 * Timers
 * todo: add "real" timer mapping
 *----------------------------------------------------------------------*/

/** timer handle */
typedef struct bdmf_timer bdmf_timer_t;

/** timer callback function
 * \param[in]	timer - timer that has expired
 * \param[in]	priv - private cookie passed in bdmf_timer_init()
 */
typedef void (*bdmf_timer_cb_t)(bdmf_timer_t *timer, unsigned long priv);

struct bdmf_timer {
	unsigned long priv;
	bdmf_timer_cb_t cb;
};

/** Initialize timer
 * \param[in]	timer - timer handle
 * \param[in]	cb - callback to be called upon expiration
 * \param[in]	priv - private cooke to be passed to cb()
 */
void bdmf_timer_init(bdmf_timer_t *timer, bdmf_timer_cb_t cb,
		     unsigned long priv);

/** Start timer
 * \param[in]	timer - timer handle that has been initialized using
 * 			bdmf_timer_init()
 * \param[in]	ticks - number of ticks from now to expiration
 * \returns	0=OK or error code <0
 */
int bdmf_timer_start(bdmf_timer_t *timer, uint32_t ticks);

/** stop timer
 * \param[in]	timer - timer to be stopped
 * The function is safe to call even if timer is not running
 */
void bdmf_timer_stop(bdmf_timer_t *timer);

/** Delete timer
 * \param[in]	timer - timer to be deleted
 * The timer is stopped if running
 */
void bdmf_timer_delete(bdmf_timer_t *timer);

/** Convert ms to ticks
 * \param[in]	ms - ms
 * \returns timer ticks
 */
uint32_t bdmf_ms_to_ticks(uint32_t ms);



#define BDMF_IRQ_NONE 0		/* IRQ is not from this device */
#define BDMF_IRQ_HANDLED 1	/* IRQ has been handled */

#define BDMF_IRQF_DISABLED 1	/* Interrupt is disabled after connect */

#ifdef XRDP
typedef int (*int_cb_t)(int irq, void *priv);
typedef struct {
	int irq;
	int_cb_t int_cb;
	void *priv;
} bdmf_int_parm_t;
#endif

#define MAX_INT_NUM 32

/** Connect system interrupt
 * \param[in]	irq - IRQ number
 * \param[in]	cpu - CPU number (for SMP)
 * \param[in]	flags - IRQ flags
 * \param[in]	isr - ISR
 * \param[in]	name - device name
 * \param[in]	priv - Private cookie
 * \returns	0=OK, <0- error
 */
int bdmf_int_connect(int irq, int cpu, int flags,
		     int (*isr)(int irq, void *priv), const char *name,
		     void *priv);

/** Disconnect system interrupt
 * \param[in]	irq - IRQ number
 * \param[in]	priv - Private cookie passed in bdmf_int_connect()
 * \returns	0=OK, <0- error
 */
void bdmf_int_disconnect(int irq, void *priv);

/** Unmask IRQ
 * \param[in]	irq - IRQ
 */
static inline void bdmf_int_enable(int irq __attribute__((unused)))
{
}

/** Mask IRQ
 * \param[in]	irq - IRQ
 */
static inline void bdmf_int_disable(int irq __attribute__((unused)))
{
}

/*-----------------------------------------------------------------------
 * Endian-related macros and constants.
 *
 *  __BYTE_ORDER define is set by GCC compilation environment.
 *  No need to do anything here.
 *  The following must be defined:
 *  __BYTE_ORDER
 *  __LITTLE_ENDIAN
 *  __BIG_ENDIAN
 *  __bswap_16(x)
 *  __bswap_32(x)
 *
 *----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 * A few defines to make porting of linux drivers into BDMF smoother
 *----------------------------------------------------------------------*/
struct module;
#define __user
#define __init
#define __exit

#endif /* _BDMF_SYSTEM_H_ */
