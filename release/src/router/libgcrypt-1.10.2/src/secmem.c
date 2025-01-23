/* secmem.c  -	memory allocation from a secure heap
 * Copyright (C) 1998, 1999, 2000, 2001, 2002,
 *               2003, 2007 Free Software Foundation, Inc.
 * Copyright (C) 2013, 2016 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <stddef.h>

#if defined(HAVE_MLOCK) || defined(HAVE_MMAP)
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef USE_CAPABILITIES
#include <sys/capability.h>
#endif
#endif

#include "g10lib.h"
#include "secmem.h"

#if defined (MAP_ANON) && ! defined (MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif

#define MINIMUM_POOL_SIZE 16384
#define STANDARD_POOL_SIZE 32768
#define DEFAULT_PAGE_SIZE 4096

typedef struct memblock
{
  unsigned size;		/* Size of the memory available to the
				   user.  */
  int flags;			/* See below.  */
  PROPERLY_ALIGNED_TYPE aligned;
} memblock_t;

/* This flag specifies that the memory block is in use.  */
#define MB_FLAG_ACTIVE (1 << 0)

/* An object describing a memory pool.  */
typedef struct pooldesc_s
{
  /* A link to the next pool.  This is used to connect the overflow
   * pools.  */
  struct pooldesc_s * volatile next;

  /* A memory buffer used as allocation pool.  */
  void *mem;

  /* The allocated size of MEM. */
  size_t size;

  /* Flag indicating that this memory pool is ready for use.  May be
   * checked in an atexit function.  */
  volatile int okay;

  /* Flag indicating whether MEM is mmapped.  */
  volatile int is_mmapped;

  /* The number of allocated bytes and the number of used blocks in
   * this pool.  */
  unsigned int cur_alloced, cur_blocks;
} pooldesc_t;


/* The pool of secure memory.  This is the head of a linked list with
 * the first element being the standard mlock-ed pool and the
 * following elements being the overflow pools. */
static pooldesc_t mainpool;


/* A couple of flags with some being set early.  */
static int disable_secmem;
static int show_warning;
static int not_locked;
static int no_warning;
static int suspend_warning;
static int no_mlock;
static int no_priv_drop;
static unsigned int auto_expand;


/* Lock protecting accesses to the memory pools.  */
GPGRT_LOCK_DEFINE (secmem_lock);

/* Convenient macros.  */
#define SECMEM_LOCK   gpgrt_lock_lock   (&secmem_lock)
#define SECMEM_UNLOCK gpgrt_lock_unlock (&secmem_lock)

/* The size of the memblock structure; this does not include the
   memory that is available to the user.  */
#define BLOCK_HEAD_SIZE \
  offsetof (memblock_t, aligned)

/* Convert an address into the according memory block structure.  */
#define ADDR_TO_BLOCK(addr) \
  (memblock_t *) (void *) ((char *) addr - BLOCK_HEAD_SIZE)

/* Prototypes. */
static void secmem_dump_stats_internal (int extended);


/*
 * Functions
 */

/* Memory barrier */
static inline void
memory_barrier(void)
{
#ifdef HAVE_SYNC_SYNCHRONIZE
#ifdef HAVE_GCC_ASM_VOLATILE_MEMORY
  asm volatile ("":::"memory");
#endif
  /* Use GCC / clang intrinsic for memory barrier. */
  __sync_synchronize();
#else
  /* Slow portable alternative, implement memory barrier by using mutex. */
  gpgrt_lock_t tmp;
  memset (&tmp, 0, sizeof(tmp));
  gpgrt_lock_init (&tmp);
  gpgrt_lock_lock (&tmp);
  gpgrt_lock_unlock (&tmp);
  gpgrt_lock_destroy (&tmp);
#endif
}


/* Check whether P points into POOL.  */
static inline int
ptr_into_pool_p (pooldesc_t *pool, const void *p)
{
  /* We need to convert pointers to addresses.  This is required by
     C-99 6.5.8 to avoid undefined behaviour.  See also
     http://lists.gnupg.org/pipermail/gcrypt-devel/2007-February/001102.html
  */
  uintptr_t p_addr    = (uintptr_t)p;
  uintptr_t pool_addr = (uintptr_t)pool->mem;

  return p_addr >= pool_addr && p_addr <  pool_addr + pool->size;
}

/* Update the stats.  */
static void
stats_update (pooldesc_t *pool, size_t add, size_t sub)
{
  if (add)
    {
      pool->cur_alloced += add;
      pool->cur_blocks++;
    }
  if (sub)
    {
      pool->cur_alloced -= sub;
      pool->cur_blocks--;
    }
}

/* Return the block following MB or NULL, if MB is the last block.  */
static memblock_t *
mb_get_next (pooldesc_t *pool, memblock_t *mb)
{
  memblock_t *mb_next;

  mb_next = (memblock_t *) (void *) ((char *) mb + BLOCK_HEAD_SIZE + mb->size);

  if (! ptr_into_pool_p (pool, mb_next))
    mb_next = NULL;

  return mb_next;
}

/* Return the block preceding MB or NULL, if MB is the first
   block.  */
static memblock_t *
mb_get_prev (pooldesc_t *pool, memblock_t *mb)
{
  memblock_t *mb_prev, *mb_next;

  if (mb == pool->mem)
    mb_prev = NULL;
  else
    {
      mb_prev = (memblock_t *) pool->mem;
      while (1)
	{
	  mb_next = mb_get_next (pool, mb_prev);
	  if (mb_next == mb)
	    break;
	  else
	    mb_prev = mb_next;
	}
    }

  return mb_prev;
}

/* If the preceding block of MB and/or the following block of MB
   exist and are not active, merge them to form a bigger block.  */
static void
mb_merge (pooldesc_t *pool, memblock_t *mb)
{
  memblock_t *mb_prev, *mb_next;

  mb_prev = mb_get_prev (pool, mb);
  mb_next = mb_get_next (pool, mb);

  if (mb_prev && (! (mb_prev->flags & MB_FLAG_ACTIVE)))
    {
      mb_prev->size += BLOCK_HEAD_SIZE + mb->size;
      mb = mb_prev;
    }
  if (mb_next && (! (mb_next->flags & MB_FLAG_ACTIVE)))
    mb->size += BLOCK_HEAD_SIZE + mb_next->size;
}

/* Return a new block, which can hold SIZE bytes.  */
static memblock_t *
mb_get_new (pooldesc_t *pool, memblock_t *block, size_t size)
{
  memblock_t *mb, *mb_split;

  for (mb = block; ptr_into_pool_p (pool, mb); mb = mb_get_next (pool, mb))
    if (! (mb->flags & MB_FLAG_ACTIVE) && mb->size >= size)
      {
	/* Found a free block.  */
	mb->flags |= MB_FLAG_ACTIVE;

	if (mb->size - size > BLOCK_HEAD_SIZE)
	  {
	    /* Split block.  */

	    mb_split = (memblock_t *) (void *) (((char *) mb) + BLOCK_HEAD_SIZE
						+ size);
	    mb_split->size = mb->size - size - BLOCK_HEAD_SIZE;
	    mb_split->flags = 0;

	    mb->size = size;

	    mb_merge (pool, mb_split);

	  }

	break;
      }

  if (! ptr_into_pool_p (pool, mb))
    {
      gpg_err_set_errno (ENOMEM);
      mb = NULL;
    }

  return mb;
}

/* Print a warning message.  */
static void
print_warn (void)
{
  if (!no_warning)
    log_info (_("Warning: using insecure memory!\n"));
}


/* Lock the memory pages of pool P of size N into core and drop
 * privileges.  */
static void
lock_pool_pages (void *p, size_t n)
{
#if defined(USE_CAPABILITIES) && defined(HAVE_MLOCK)
  int err;

  {
    cap_t cap;

    if (!no_priv_drop)
      {
        cap = cap_from_text ("cap_ipc_lock+ep");
        cap_set_proc (cap);
        cap_free (cap);
      }
    err = no_mlock? 0 : mlock (p, n);
    if (err && errno)
      err = errno;
    if (!no_priv_drop)
      {
        cap = cap_from_text ("cap_ipc_lock+p");
        cap_set_proc (cap);
        cap_free(cap);
      }
  }

  if (err)
    {
      if (err != EPERM
#ifdef EAGAIN	/* BSD and also Linux may return EAGAIN */
	  && err != EAGAIN
#endif
#ifdef ENOSYS	/* Some SCOs return this (function not implemented) */
	  && err != ENOSYS
#endif
#ifdef ENOMEM  /* Linux might return this. */
            && err != ENOMEM
#endif
	  )
	log_error ("can't lock memory: %s\n", strerror (err));
      show_warning = 1;
      not_locked = 1;
    }

#elif defined(HAVE_MLOCK)
  uid_t uid;
  int err;

  uid = getuid ();

#ifdef HAVE_BROKEN_MLOCK
  /* Under HP/UX mlock segfaults if called by non-root.  Note, we have
     noch checked whether mlock does really work under AIX where we
     also detected a broken nlock.  Note further, that using plock ()
     is not a good idea under AIX. */
  if (uid)
    {
      errno = EPERM;
      err = errno;
    }
  else
    {
      err = no_mlock? 0 : mlock (p, n);
      if (err && errno)
	err = errno;
    }
#else /* !HAVE_BROKEN_MLOCK */
  err = no_mlock? 0 : mlock (p, n);
  if (err && errno)
    err = errno;
#endif /* !HAVE_BROKEN_MLOCK */

  /* Test whether we are running setuid(0).  */
  if (uid && ! geteuid ())
    {
      /* Yes, we are.  */
      if (!no_priv_drop)
        {
          /* Check that we really dropped the privs.
           * Note: setuid(0) should always fail */
          if (setuid (uid) || getuid () != geteuid () || !setuid (0))
            log_fatal ("failed to reset uid: %s\n", strerror (errno));
        }
    }

  if (err)
    {
      if (err != EPERM
#ifdef EAGAIN	/* BSD and also Linux may return this. */
	  && err != EAGAIN
#endif
#ifdef ENOSYS	/* Some SCOs return this (function not implemented). */
	  && err != ENOSYS
#endif
#ifdef ENOMEM  /* Linux might return this. */
            && err != ENOMEM
#endif
	  )
	log_error ("can't lock memory: %s\n", strerror (err));
      show_warning = 1;
      not_locked = 1;
    }

#elif defined ( __QNX__ )
  /* QNX does not page at all, so the whole secure memory stuff does
   * not make much sense.  However it is still of use because it
   * wipes out the memory on a free().
   * Therefore it is sufficient to suppress the warning.  */
  (void)p;
  (void)n;
#elif defined (HAVE_DOSISH_SYSTEM) || defined (__CYGWIN__)
    /* It does not make sense to print such a warning, given the fact that
     * this whole Windows !@#$% and their user base are inherently insecure. */
  (void)p;
  (void)n;
#elif defined (__riscos__)
    /* No virtual memory on RISC OS, so no pages are swapped to disc,
     * besides we don't have mmap, so we don't use it! ;-)
     * But don't complain, as explained above.  */
  (void)p;
  (void)n;
#else
  (void)p;
  (void)n;
  if (!no_mlock)
    log_info ("Please note that you don't have secure memory on this system\n");
#endif
}

/* Initialize POOL.  */
static void
init_pool (pooldesc_t *pool, size_t n)
{
  memblock_t *mb;

  pool->size = n;

  if (disable_secmem)
    log_bug ("secure memory is disabled");


#if HAVE_MMAP
  {
    size_t pgsize;
    long int pgsize_val;

# if defined(HAVE_SYSCONF) && defined(_SC_PAGESIZE)
    pgsize_val = sysconf (_SC_PAGESIZE);
# elif defined(HAVE_GETPAGESIZE)
    pgsize_val = getpagesize ();
# else
    pgsize_val = -1;
# endif
    pgsize = (pgsize_val > 0)? pgsize_val:DEFAULT_PAGE_SIZE;

    pool->size = (pool->size + pgsize - 1) & ~(pgsize - 1);
# ifdef MAP_ANONYMOUS
    pool->mem = mmap (0, pool->size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
# else /* map /dev/zero instead */
    {
      int fd;

      fd = open ("/dev/zero", O_RDWR);
      if (fd == -1)
        {
          log_error ("can't open /dev/zero: %s\n", strerror (errno));
          pool->mem = (void *) -1;
        }
      else
        {
          pool->mem = mmap (0, pool->size,
                           (PROT_READ | PROT_WRITE), MAP_PRIVATE, fd, 0);
          close (fd);
        }
    }
# endif
    if (pool->mem == (void *) -1)
      log_info ("can't mmap pool of %u bytes: %s - using malloc\n",
                (unsigned) pool->size, strerror (errno));
    else
      {
        pool->is_mmapped = 1;
        pool->okay = 1;
      }
  }
#endif /*HAVE_MMAP*/

  if (!pool->okay)
    {
      pool->mem = malloc (pool->size);
      if (!pool->mem)
	log_fatal ("can't allocate memory pool of %u bytes\n",
		   (unsigned) pool->size);
      else
	pool->okay = 1;
    }

  /* Initialize first memory block.  */
  mb = (memblock_t *) pool->mem;
  mb->size = pool->size - BLOCK_HEAD_SIZE;
  mb->flags = 0;
}


/* Enable overflow pool allocation in all cases.  CHUNKSIZE is a hint
 * on how large to allocate overflow pools.  */
void
_gcry_secmem_set_auto_expand (unsigned int chunksize)
{
  /* Round up to a multiple of the STANDARD_POOL_SIZE.  */
  chunksize = ((chunksize + (2*STANDARD_POOL_SIZE) - 1)
               / STANDARD_POOL_SIZE ) * STANDARD_POOL_SIZE;
  if (chunksize < STANDARD_POOL_SIZE) /* In case of overflow.  */
    chunksize = STANDARD_POOL_SIZE;

  SECMEM_LOCK;
  auto_expand = chunksize;
  SECMEM_UNLOCK;
}


void
_gcry_secmem_set_flags (unsigned flags)
{
  int was_susp;

  SECMEM_LOCK;

  was_susp = suspend_warning;
  no_warning = flags & GCRY_SECMEM_FLAG_NO_WARNING;
  suspend_warning = flags & GCRY_SECMEM_FLAG_SUSPEND_WARNING;
  no_mlock      = flags & GCRY_SECMEM_FLAG_NO_MLOCK;
  no_priv_drop = flags & GCRY_SECMEM_FLAG_NO_PRIV_DROP;

  /* and now issue the warning if it is not longer suspended */
  if (was_susp && !suspend_warning && show_warning)
    {
      show_warning = 0;
      print_warn ();
    }

  SECMEM_UNLOCK;
}

unsigned int
_gcry_secmem_get_flags (void)
{
  unsigned flags;

  SECMEM_LOCK;

  flags = no_warning ? GCRY_SECMEM_FLAG_NO_WARNING : 0;
  flags |= suspend_warning ? GCRY_SECMEM_FLAG_SUSPEND_WARNING : 0;
  flags |= not_locked ? GCRY_SECMEM_FLAG_NOT_LOCKED : 0;
  flags |= no_mlock ? GCRY_SECMEM_FLAG_NO_MLOCK : 0;
  flags |= no_priv_drop ? GCRY_SECMEM_FLAG_NO_PRIV_DROP : 0;

  SECMEM_UNLOCK;

  return flags;
}


/* This function initializes the main memory pool MAINPOOL.  It is
 * expected to be called with the secmem lock held.  */
static void
_gcry_secmem_init_internal (size_t n)
{
  pooldesc_t *pool;

  pool = &mainpool;
  if (!n)
    {
#ifdef USE_CAPABILITIES
      /* drop all capabilities */
      if (!no_priv_drop)
        {
          cap_t cap;

          cap = cap_from_text ("all-eip");
          cap_set_proc (cap);
          cap_free (cap);
        }

#elif !defined(HAVE_DOSISH_SYSTEM)
      uid_t uid;

      disable_secmem = 1;
      uid = getuid ();
      if (uid != geteuid ())
	{
	  if (setuid (uid) || getuid () != geteuid () || !setuid (0))
	    log_fatal ("failed to drop setuid\n");
	}
#endif
    }
  else
    {
      if (n < MINIMUM_POOL_SIZE)
	n = MINIMUM_POOL_SIZE;
      if (! pool->okay)
	{
	  init_pool (pool, n);
	  lock_pool_pages (pool->mem, n);
	}
      else
	log_error ("Oops, secure memory pool already initialized\n");
    }
}



/* Initialize the secure memory system.  If running with the necessary
   privileges, the secure memory pool will be locked into the core in
   order to prevent page-outs of the data.  Furthermore allocated
   secure memory will be wiped out when released.  */
void
_gcry_secmem_init (size_t n)
{
  SECMEM_LOCK;

  _gcry_secmem_init_internal (n);

  SECMEM_UNLOCK;
}


gcry_err_code_t
_gcry_secmem_module_init ()
{
  /* Not anymore needed.  */
  return 0;
}


static void *
_gcry_secmem_malloc_internal (size_t size, int xhint)
{
  pooldesc_t *pool;
  memblock_t *mb;

  pool = &mainpool;

  if (!pool->okay)
    {
      /* Try to initialize the pool if the user forgot about it.  */
      _gcry_secmem_init_internal (STANDARD_POOL_SIZE);
      if (!pool->okay)
        {
          log_info (_("operation is not possible without "
                      "initialized secure memory\n"));
          gpg_err_set_errno (ENOMEM);
          return NULL;
        }
    }
  if (not_locked && fips_mode ())
    {
      log_info (_("secure memory pool is not locked while in FIPS mode\n"));
      gpg_err_set_errno (ENOMEM);
      return NULL;
    }
  if (show_warning && !suspend_warning)
    {
      show_warning = 0;
      print_warn ();
    }

  /* Blocks are always a multiple of 32. */
  size = ((size + 31) / 32) * 32;

  mb = mb_get_new (pool, (memblock_t *) pool->mem, size);
  if (mb)
    {
      stats_update (pool, mb->size, 0);
      return &mb->aligned.c;
    }

  /* If we are called from xmalloc style functions resort to the
   * overflow pools to return memory.  We don't do this in FIPS mode,
   * though.  If the auto-expand option is active we do the expanding
   * also for the standard malloc functions.
   *
   * The idea of using them by default only for the xmalloc function
   * is so that a user can control whether memory will be allocated in
   * the initial created mlock protected secmem area or may also be
   * allocated from the overflow pools.  */
  if ((xhint || auto_expand) && !fips_mode ())
    {
      /* Check whether we can allocate from the overflow pools.  */
      for (pool = pool->next; pool; pool = pool->next)
        {
          mb = mb_get_new (pool, (memblock_t *) pool->mem, size);
          if (mb)
            {
              stats_update (pool, mb->size, 0);
              return &mb->aligned.c;
            }
        }
      /* Allocate a new overflow pool.  We put a new pool right after
       * the mainpool so that the next allocation will happen in that
       * pool and not in one of the older pools.  When this new pool
       * gets full we will try to find space in the older pools.  */
      pool = calloc (1, sizeof *pool);
      if (!pool)
        return NULL;  /* Not enough memory for a new pool descriptor.  */
      pool->size = auto_expand? auto_expand : STANDARD_POOL_SIZE;
      pool->mem = malloc (pool->size);
      if (!pool->mem)
        {
          free (pool);
          return NULL; /* Not enough memory available for a new pool.  */
        }
      /* Initialize first memory block.  */
      mb = (memblock_t *) pool->mem;
      mb->size = pool->size - BLOCK_HEAD_SIZE;
      mb->flags = 0;

      pool->okay = 1;

      /* Take care: in _gcry_private_is_secure we do not lock and thus
       * we assume that the second assignment below is atomic.  Memory
       * barrier prevents reordering of stores to new pool structure after
       * MAINPOOL.NEXT assigment and prevents _gcry_private_is_secure seeing
       * non-initialized POOL->NEXT pointers.  */
      pool->next = mainpool.next;
      memory_barrier();
      mainpool.next = pool;

      /* After the first time we allocated an overflow pool, print a
       * warning.  */
      if (!pool->next)
        print_warn ();

      /* Allocate.  */
      mb = mb_get_new (pool, (memblock_t *) pool->mem, size);
      if (mb)
        {
          stats_update (pool, mb->size, 0);
          return &mb->aligned.c;
        }
    }

  return NULL;
}


/* Allocate a block from the secmem of SIZE.  With XHINT set assume
 * that the caller is a xmalloc style function.  */
void *
_gcry_secmem_malloc (size_t size, int xhint)
{
  void *p;

  SECMEM_LOCK;
  p = _gcry_secmem_malloc_internal (size, xhint);
  SECMEM_UNLOCK;

  return p;
}

static int
_gcry_secmem_free_internal (void *a)
{
  pooldesc_t *pool;
  memblock_t *mb;
  int size;

  for (pool = &mainpool; pool; pool = pool->next)
    if (pool->okay && ptr_into_pool_p (pool, a))
      break;
  if (!pool)
    return 0; /* A does not belong to use.  */

  mb = ADDR_TO_BLOCK (a);
  size = mb->size;

  /* This does not make much sense: probably this memory is held in the
   * cache. We do it anyway: */
#define MB_WIPE_OUT(byte) \
  wipememory2 (((char *) mb + BLOCK_HEAD_SIZE), (byte), size);

  MB_WIPE_OUT (0xff);
  MB_WIPE_OUT (0xaa);
  MB_WIPE_OUT (0x55);
  MB_WIPE_OUT (0x00);

  /* Update stats.  */
  stats_update (pool, 0, size);

  mb->flags &= ~MB_FLAG_ACTIVE;

  mb_merge (pool, mb);

  return 1; /* Freed.  */
}


/* Wipe out and release memory.  Returns true if this function
 * actually released A.  */
int
_gcry_secmem_free (void *a)
{
  int mine;

  if (!a)
    return 1; /* Tell caller that we handled it.  */

  SECMEM_LOCK;
  mine = _gcry_secmem_free_internal (a);
  SECMEM_UNLOCK;
  return mine;
}


static void *
_gcry_secmem_realloc_internal (void *p, size_t newsize, int xhint)
{
  memblock_t *mb;
  size_t size;
  void *a;

  mb = (memblock_t *) (void *) ((char *) p
				- ((size_t) &((memblock_t *) 0)->aligned.c));
  size = mb->size;
  if (newsize < size)
    {
      /* It is easier to not shrink the memory.  */
      a = p;
    }
  else
    {
      a = _gcry_secmem_malloc_internal (newsize, xhint);
      if (a)
	{
	  memcpy (a, p, size);
	  memset ((char *) a + size, 0, newsize - size);
	  _gcry_secmem_free_internal (p);
	}
    }

  return a;
}


/* Realloc memory.  With XHINT set assume that the caller is a xmalloc
 * style function.  */
void *
_gcry_secmem_realloc (void *p, size_t newsize, int xhint)
{
  void *a;

  SECMEM_LOCK;
  a = _gcry_secmem_realloc_internal (p, newsize, xhint);
  SECMEM_UNLOCK;

  return a;
}


/* Return true if P points into the secure memory areas.  */
int
_gcry_private_is_secure (const void *p)
{
  pooldesc_t *pool;

  /* We do no lock here because once a pool is allocated it will not
   * be removed anymore (except for gcry_secmem_term).  Further, as
   * assigment of POOL->NEXT in new pool structure is visible in
   * this thread before assigment of MAINPOOL.NEXT, pool list can be
   * iterated locklessly.  This visiblity is ensured by memory barrier
   * between POOL->NEXT and MAINPOOL.NEXT assignments in
   * _gcry_secmem_malloc_internal. */
  for (pool = &mainpool; pool; pool = pool->next)
    if (pool->okay && ptr_into_pool_p (pool, p))
      return 1;

  return 0;
}


/****************
 * Warning:  This code might be called by an interrupt handler
 *	     and frankly, there should really be such a handler,
 *	     to make sure that the memory is wiped out.
 *	     We hope that the OS wipes out mlocked memory after
 *	     receiving a SIGKILL - it really should do so, otherwise
 *	     there is no chance to get the secure memory cleaned.
 */
void
_gcry_secmem_term ()
{
  pooldesc_t *pool, *next;

  for (pool = &mainpool; pool; pool = next)
    {
      next = pool->next;
      if (!pool->okay)
        continue;

      wipememory2 (pool->mem, 0xff, pool->size);
      wipememory2 (pool->mem, 0xaa, pool->size);
      wipememory2 (pool->mem, 0x55, pool->size);
      wipememory2 (pool->mem, 0x00, pool->size);
      if (0)
        ;
#if HAVE_MMAP
      else if (pool->is_mmapped)
        munmap (pool->mem, pool->size);
#endif
      else
        free (pool->mem);
      pool->mem = NULL;
      pool->okay = 0;
      pool->size = 0;
      if (pool != &mainpool)
        free (pool);
    }
  mainpool.next = NULL;
  not_locked = 0;
}


/* Print stats of the secmem allocator.  With EXTENDED passwed as true
 * a detiled listing is returned (used for testing).  */
void
_gcry_secmem_dump_stats (int extended)
{
  SECMEM_LOCK;
  secmem_dump_stats_internal (extended);
  SECMEM_UNLOCK;
}


static void
secmem_dump_stats_internal (int extended)
{
  pooldesc_t *pool;
  memblock_t *mb;
  int i, poolno;

  for (pool = &mainpool, poolno = 0; pool; pool = pool->next, poolno++)
    {
      if (!extended)
        {
          if (pool->okay)
            log_info ("%-13s %u/%lu bytes in %u blocks\n",
                      pool == &mainpool? "secmem usage:":"",
                      pool->cur_alloced, (unsigned long)pool->size,
                      pool->cur_blocks);
        }
      else
        {
          for (i = 0, mb = (memblock_t *) pool->mem;
               ptr_into_pool_p (pool, mb);
               mb = mb_get_next (pool, mb), i++)
            log_info ("SECMEM: pool %d %s block %i size %i\n",
                      poolno,
                      (mb->flags & MB_FLAG_ACTIVE) ? "used" : "free",
                      i,
                      mb->size);
        }
    }
}
