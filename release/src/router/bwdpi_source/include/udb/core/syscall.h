/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#if !defined(SYSTEM_CALLS_H) && !defined(SYSTEM_CALLS_H_)
#define SYSTEM_CALLS_H
#define SYSTEM_CALLS_H_

/*

#define spin_lock_init(_lock)				\
do {							\
	spinlock_check(_lock);				\
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

*/

typedef int   (*spin_lock_init_t)(void *lock_ptr);
typedef void* (*spin_lock_alloc_t)(void);
typedef void   (*spin_lock_free_t)(void *lock_ptr);


/*
static inline void spin_lock(spinlock_t *lock)
*/
typedef void   (*spin_lock_t)(void *lock_ptr);


/*
static inline void spin_unlock(spinlock_t *lock)
*/
typedef void   (*spin_unlock_t)(void *lock_ptr);


/*
static inline void spin_lock_bh(spinlock_t *lock)
*/
typedef void   (*spin_lock_bh_t)(void *lock_ptr);

/*
static inline void spin_unlock_bh(spinlock_t *lock)
*/
typedef void   (*spin_unlock_bh_t)(void *lock_ptr);

typedef int   (*spin_is_locked_t)(void *lock_ptr);


typedef int (*rwlock_init_t)(void *lck);
typedef void * (*rwlock_alloc_t)(void);
typedef void * (*rwlock_alloc_atomic_t)(void);
typedef void (*rwlock_free_t)(void *lck);
typedef void (*read_lock_t)(void *);
typedef void (*read_unlock_t)(void *);
typedef void (*read_lock_bh_t)(void *);
typedef void (*read_unlock_bh_t)(void *);
typedef void (*write_lock_t)(void *);
typedef void (*write_unlock_t)(void *);
typedef void (*write_lock_bh_t)(void *);
typedef void (*write_unlock_bh_t)(void *);

//----------------------------------------------------------------------------
/*
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
*/



#if 0
typedef int (*printf_t)(const char *format, ...);
typedef int (*fprintf_t)(void *stream, const char *format, ...);
typedef int (*sprintf_t)(char *str, const char *format, ...);
//typedef int (*snprintf_t)(char *str, size_t size, const char *format, ...);
typedef int (*snprintf_t)(char *str, unsigned long size, const char *format, ...);
#endif

#ifdef __GNUC__
	#ifndef _VA_LIST_DEFINED
		#define __need___va_list
		#include <stdarg.h> // take gcc's definition
		#ifdef __GNUC_VA_LIST
			#undef _IO_va_list
			#define _IO_va_list __gnuc_va_list
			typedef __gnuc_va_list va_list;
			#define _VA_LIST_DEFINED		
		#else
			#error "No __GNUC_VA_LIST"
			// typedef char* va_list;
		#endif /* __GNUC_VA_LIST */	
	#else
		//"Here is an va_list."
	#endif
#else
	//#error "We need GCC for va_list\n"
	typedef char* va_list;
#endif


//int vprintk(const char *fmt, va_list args)
//int vprintf(const char *format, va_list ap);
//int vfprintf(FILE *stream, const char *format, va_list ap);
//int vsprintf(char *str, const char *format, va_list ap);
//int vsnprintf(char *str, size_t size, const char *format, va_list ap);

typedef int (*vprintf_t)(const char *format, va_list ap);
//int vfprintf(FILE *stream, const char *format, va_list ap);
typedef int (*vsprintf_t)(char *str, const char *format, va_list ap);
typedef int (*vsnprintf_t)(char *str, unsigned long size, const char *format, va_list ap);
typedef int (*seq_printf_t)(void *sf, const char *format, va_list ap);

//----------------------------------------------------------------------------
/*
void *malloc(size_t size);
void free(void *ptr);

void *vmalloc(unsigned long size);
void vfree(const void *addr);

void *kmalloc(size_t size, gfp_t flags);
void kfree(const void *x);


*/

#if 0
typedef void* (*malloc_t)	(unsigned long size);
typedef void  (*free_t)		(void *ptr);
void *kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags)
void kmem_cache_free(struct kmem_cache *c, void *b);	
struct kmem_cache *kmem_cache_create(const char *name, size_t size,
	size_t align, unsigned long flags, void (*ctor)(void *));
#endif

typedef void* (*vmalloc_t)	(unsigned long size);
typedef void  (*vfree_t)	(const void *addr);
typedef void* (*kmalloc_t)	(unsigned long size);
typedef void  (*kfree_t)	(const void *x);

typedef void* (*alloc_skb_t) 	(unsigned long size);
typedef void* (*skb_clone_t)	(const void *x);
typedef void (*kfree_skb_t)	(const void *x);

typedef void* (*kmem_cache_alloc_t)(void *cachep, unsigned flags);
typedef void (*kmem_cache_free_t)(void *c, void *b);	
typedef void* (*kmem_cache_create_t)(const char *name, unsigned long size,
	unsigned long align, unsigned long flags, void (*ctor)(void *));
typedef void (*kmem_cache_destroy_t)(void *c);


//----------------------------------------------------------------------------
#if 0
static inline int atomic_read(const atomic_t *v)
static inline void atomic_set(atomic_t *v, int i)
static inline void atomic_add(int i, atomic_t *v)
static inline void atomic_sub(int i, atomic_t *v)
static inline void atomic_inc(atomic_t *v)
static inline void atomic_dec(atomic_t *v)
#endif

typedef void* (*atomic_alloc_t)(void);
typedef void  (*atomic_free_t)(void *);
typedef void (*atomic_set_t)(void *v, int i);
typedef void (*atomic_add_t)(int i, void *v);
typedef void (*atomic_sub_t)(int i, void *v);
typedef void (*atomic_inc_t)(void *v);
typedef void (*atomic_dec_t)(void *v);

//----------------------------------------------------------------------------
typedef unsigned long (*get_seconds_t)(void);
typedef void* (*timer_alloc_t)(void);
typedef void* (*timer_alloc_atomic_t)(void);
typedef void  (*timer_free_t)(void *);
typedef void (*timer_setup_t)(void *timer_list_ptr, void (*function)(unsigned long),
		unsigned long data);
typedef int (*timer_mod_t)(void *timer_list_ptr, unsigned long expires);
typedef int (*timer_del_t)(void *timer_list_ptr);
typedef int (*timer_del_sync_t)(void *timer_list_ptr);

//----------------------------------------------------------------------------
typedef int (*bitop_find_first_zero_bit_t)(const void * p, unsigned long size);
typedef unsigned long (*bitop_find_next_zero_bit_t)(const unsigned long *addr, unsigned long size, unsigned long offset);
typedef void (*bitop_set_bit_t)(int bit, unsigned long *p);
typedef void (*bitop_clear_bit_t)(int bit, unsigned long * p);
typedef void (*bitop_bitmap_zero_t)(unsigned long *dst, int nbits);
typedef int (*bitop_test_bit_t)(int nr, const unsigned long *addr);
typedef int (*bitop_find_next_bit_t)(const unsigned long *p, int size, int offset);

//----------------------------------------------------------------------------
//void local_bh_disable(void)
//void local_bh_enable(void)

typedef void (*local_bh_disable_t)(void);
typedef void (*local_bh_enable_t)(void);
//----------------------------------------------------------------------------
typedef void (*smp_mb_t)(void);
//----------------------------------------------------------------------------
//unsigned int smp_processor_id(void)
typedef unsigned int (*smp_processor_id_t)(void);

typedef void (*schedule_t)(void);

//----------------------------------------------------------------------------
int tdts_core_syscall_set_spin_lock(
	spin_lock_init_t	init,
	spin_lock_alloc_t	alloc,
	spin_lock_free_t	free,
	spin_lock_t 		lock,
	spin_unlock_t		unlock,
	spin_lock_bh_t		lock_bh,
	spin_unlock_bh_t	unlock_bh,
        spin_is_locked_t	is_locked);

int tdts_core_syscall_set_rwlock(
	rwlock_init_t init,
	rwlock_alloc_t alloc,
	rwlock_alloc_atomic_t alloc_atomic,
	rwlock_free_t free,
	read_lock_t read_lock,
	read_unlock_t read_unlock,
	read_lock_bh_t read_lock_bh,
	read_unlock_bh_t read_unlock_bh,
	write_lock_t write_lock,
	write_unlock_t write_unlock,
	write_lock_bh_t write_lock_bh,
	write_unlock_bh_t write_unlock_bh);

extern int tdts_core_syscall_set_printf(
	vprintf_t	vprintf,
	vsprintf_t	vsprintf,
	vsnprintf_t vsnprintf);

extern int tdts_core_syscall_set_seq_printf(
	seq_printf_t seq_printf);

extern int tdts_core_syscall_set_malloc(
	vmalloc_t	vmalloc,
	vfree_t 	vfree,
	kmalloc_t	kmalloc_atomic,
	kmalloc_t	kmalloc_sleep,
	kfree_t 	kfree);

extern int tdts_core_syscall_set_alloc_skb(
	alloc_skb_t	alloc_skb_atomic,
	skb_clone_t	skb_clone_atomic,
	kfree_skb_t	kfree_skb);

	
extern int tdts_core_syscall_set_kmem_cache(
	kmem_cache_alloc_t alloc,
	kmem_cache_free_t free,
	kmem_cache_create_t create,
	kmem_cache_destroy_t destroy);

extern int tdts_core_syscall_set_atomic(
	atomic_alloc_t alloc,
	atomic_free_t free,
	atomic_set_t set,
	atomic_add_t add,
	atomic_sub_t sub,
	atomic_inc_t inc,
	atomic_dec_t dec);

//----------------------------------------------------------------------------
extern int tdts_core_syscall_set_time(
	get_seconds_t get_seconds,
	timer_alloc_t timer_alloc,
	timer_alloc_atomic_t timer_alloc_atomic,
	timer_free_t timer_free,
	timer_setup_t timer_setup,
	timer_mod_t timer_mod,
	timer_del_t timer_del,
	timer_del_sync_t timer_del_sync);

//----------------------------------------------------------------------------
extern int tdts_core_syscall_set_local_bh(
		local_bh_disable_t local_bh_disable,local_bh_enable_t local_bh_enable);
//----------------------------------------------------------------------------
extern int tdts_core_syscall_set_smp_mb(smp_mb_t smp_mb);
//----------------------------------------------------------------------------
extern int tdts_core_syscall_set_smp_processor_id(smp_processor_id_t smp_processor_id);

extern int tdts_core_syscall_set_bitop(
        bitop_find_first_zero_bit_t bitop_find_first_zero_bit,
        bitop_set_bit_t bitop_set_bit,
        bitop_clear_bit_t bitop_clear_bit,
        bitop_bitmap_zero_t bitop_bitmap_zero,
        bitop_test_bit_t bitop_test_bit,
        bitop_find_next_bit_t bitop_find_next_bit,
        bitop_find_next_zero_bit_t bitop_find_next_zero_bit);

extern int tdts_core_syscall_set_schedule(schedule_t schedule);

//----------------------------------------------------------------------------
int tdts_core_syscall_init(void);
void tdts_core_syscall_cleanup(void);
int tdts_core_syscall_set_test_value(unsigned int val);
unsigned int tdts_core_syscall_get_test_value(void);


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




#endif //SYSTEM_CALLS_H | SYSTEM_CALLS_H_