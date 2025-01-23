/* estream.c - Extended Stream I/O Library
 * Copyright (C) 2004, 2005, 2006, 2007, 2009, 2010, 2011,
 *               2014, 2015, 2016, 2017 g10 Code GmbH
 *
 * This file is part of Libestream.
 *
 * Libestream is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libestream is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libestream; if not, see <https://www.gnu.org/licenses/>.
 *
 * ALTERNATIVELY, Libestream may be distributed under the terms of the
 * following license, in which case the provisions of this license are
 * required INSTEAD OF the GNU General Public License. If you wish to
 * allow use of your version of this file only under the terms of the
 * GNU General Public License, and not to allow others to use your
 * version of this file under the terms of the following license,
 * indicate your decision by deleting this paragraph and the license
 * below.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef USE_ESTREAM_SUPPORT_H
# include <estream-support.h>
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(_WIN32) && !defined(HAVE_W32_SYSTEM)
# define HAVE_W32_SYSTEM 1
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#ifdef HAVE_W32_SYSTEM
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# include <windows.h>
#else
# ifdef HAVE_POLL_H
#  include <poll.h>
# else
#  ifdef HAVE_SYS_SELECT_H
#   include <sys/select.h>
#  endif
# endif
#endif

/* Enable tracing.  The value is the module name to be printed.  */
/*#define ENABLE_TRACING "estream"*/

#include "gpgrt-int.h"
#include "estream-printf.h"
#include "thread.h"
#include "lock.h"


#ifndef O_BINARY
# define O_BINARY 0
#endif
#ifndef HAVE_DOSISH_SYSTEM
# ifdef HAVE_W32_SYSTEM
#  define HAVE_DOSISH_SYSTEM 1
# endif
#endif


#ifdef HAVE_W32_SYSTEM
# ifndef  S_IRGRP
#  define S_IRGRP S_IRUSR
# endif
# ifndef  S_IROTH
#  define S_IROTH S_IRUSR
# endif
# ifndef  S_IWGRP
#  define S_IWGRP S_IWUSR
# endif
# ifndef  S_IWOTH
#  define S_IWOTH S_IWUSR
# endif
# ifndef  S_IXGRP
#  define S_IXGRP S_IXUSR
# endif
# ifndef  S_IXOTH
#  define S_IXOTH S_IXUSR
# endif
#endif

#if !defined (EWOULDBLOCK) && defined (HAVE_W32_SYSTEM)
/* Compatibility with errno.h from mingw-2.0 */
# define EWOULDBLOCK 140
#endif

#ifndef EAGAIN
# define EAGAIN  EWOULDBLOCK
#endif


#define _set_errno(a)  do { errno = (a); } while (0)

#define IS_INVALID_FD(a)    ((a) == -1)

/* Calculate array dimension.  */
#ifndef DIM
#define DIM(array) (sizeof (array) / sizeof (*array))
#endif

/* A helper macro used to convert to a hex string.  */
#define tohex(n) ((n) < 10 ? ((n) + '0') : (((n) - 10) + 'A'))


/* Generally used types.  */

typedef void *(*func_realloc_t) (void *mem, size_t size);
typedef void (*func_free_t) (void *mem);




/*
 * A linked list to hold active stream objects.
 * Protected by ESTREAM_LIST_LOCK.
 */
struct estream_list_s
{
  struct estream_list_s *next;
  estream_t stream;  /* Entry is not used if NULL.  */
};
typedef struct estream_list_s *estream_list_t;
static estream_list_t estream_list;

/*
 * File descriptors registered for use as the standard file handles.
 * Protected by ESTREAM_LIST_LOCK.
 */
static int custom_std_fds[3];
static unsigned char custom_std_fds_valid[3];

/*
 * A lock object to protect ESTREAM LIST, CUSTOM_STD_FDS and
 * CUSTOM_STD_FDS_VALID.  Used by lock_list() and unlock_list().
 */
GPGRT_LOCK_DEFINE (estream_list_lock);


/*
 * Error code replacements.
 */
#ifndef EOPNOTSUPP
# define EOPNOTSUPP ENOSYS
#endif


/* Local prototypes.  */
static void fname_set_internal (estream_t stream, const char *fname, int quote);




/*
 * Memory allocation wrappers used in this file.
 */
static void *
mem_alloc (size_t n)
{
  return _gpgrt_malloc (n);
}

static void *
mem_realloc (void *p, size_t n)
{
  return _gpgrt_realloc (p, n);
}

static void
mem_free (void *p)
{
  if (p)
    _gpgrt_free (p);
}


/*
 * A Windows helper function to map a W32 API error code to a standard
 * system error code.  That actually belong into sysutils but to allow
 * standalone use of estream we keep it here.
 */
#ifdef HAVE_W32_SYSTEM
static int
map_w32_to_errno (DWORD w32_err)
{
  switch (w32_err)
    {
    case 0:
      return 0;

    case ERROR_FILE_NOT_FOUND:
      return ENOENT;

    case ERROR_PATH_NOT_FOUND:
      return ENOENT;

    case ERROR_ACCESS_DENIED:
      return EPERM;  /* ReactOS uses EACCES ("Permission denied") and
                      * is likely right because they used an
                      * undocumented function to associate the error
                      * codes.  However we have always used EPERM
                      * ("Operation not permitted", e.g. function is
                      * required to be called by root) and we better
                      * stick to that to avoid surprising bugs. */

    case ERROR_INVALID_HANDLE:
      return EBADF;

    case ERROR_INVALID_BLOCK:
      return ENOMEM;

    case ERROR_NOT_ENOUGH_MEMORY:
      return ENOMEM;

    case ERROR_NO_DATA:
      return EPIPE;

    case ERROR_ALREADY_EXISTS:
      return EEXIST;

      /* This mapping has been taken from reactOS.  */
    case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
    case ERROR_ARENA_TRASHED: return ENOMEM;
    case ERROR_BAD_ENVIRONMENT: return E2BIG;
    case ERROR_BAD_FORMAT: return ENOEXEC;
    case ERROR_INVALID_DRIVE: return ENOENT;
    case ERROR_CURRENT_DIRECTORY: return EACCES;
    case ERROR_NOT_SAME_DEVICE: return EXDEV;
    case ERROR_NO_MORE_FILES: return ENOENT;
    case ERROR_WRITE_PROTECT: return EACCES;
    case ERROR_BAD_UNIT: return EACCES;
    case ERROR_NOT_READY: return EACCES;
    case ERROR_BAD_COMMAND: return EACCES;
    case ERROR_CRC: return EACCES;
    case ERROR_BAD_LENGTH: return EACCES;
    case ERROR_SEEK: return EACCES;
    case ERROR_NOT_DOS_DISK: return EACCES;
    case ERROR_SECTOR_NOT_FOUND: return EACCES;
    case ERROR_OUT_OF_PAPER: return EACCES;
    case ERROR_WRITE_FAULT: return EACCES;
    case ERROR_READ_FAULT: return EACCES;
    case ERROR_GEN_FAILURE: return EACCES;
    case ERROR_SHARING_VIOLATION: return EACCES;
    case ERROR_LOCK_VIOLATION: return EACCES;
    case ERROR_WRONG_DISK: return EACCES;
    case ERROR_SHARING_BUFFER_EXCEEDED: return EACCES;
    case ERROR_BAD_NETPATH: return ENOENT;
    case ERROR_NETWORK_ACCESS_DENIED: return EACCES;
    case ERROR_BAD_NET_NAME: return ENOENT;
    case ERROR_FILE_EXISTS: return EEXIST;
    case ERROR_CANNOT_MAKE: return EACCES;
    case ERROR_FAIL_I24: return EACCES;
    case ERROR_NO_PROC_SLOTS: return EAGAIN;
    case ERROR_DRIVE_LOCKED: return EACCES;
    case ERROR_BROKEN_PIPE: return EPIPE;
    case ERROR_DISK_FULL: return ENOSPC;
    case ERROR_INVALID_TARGET_HANDLE: return EBADF;
    case ERROR_WAIT_NO_CHILDREN: return ECHILD;
    case ERROR_CHILD_NOT_COMPLETE: return ECHILD;
    case ERROR_DIRECT_ACCESS_HANDLE: return EBADF;
    case ERROR_SEEK_ON_DEVICE: return EACCES;
    case ERROR_DIR_NOT_EMPTY: return ENOTEMPTY;
    case ERROR_NOT_LOCKED: return EACCES;
    case ERROR_BAD_PATHNAME: return ENOENT;
    case ERROR_MAX_THRDS_REACHED: return EAGAIN;
    case ERROR_LOCK_FAILED: return EACCES;
    case ERROR_INVALID_STARTING_CODESEG: return ENOEXEC;
    case ERROR_INVALID_STACKSEG: return ENOEXEC;
    case ERROR_INVALID_MODULETYPE: return ENOEXEC;
    case ERROR_INVALID_EXE_SIGNATURE: return ENOEXEC;
    case ERROR_EXE_MARKED_INVALID: return ENOEXEC;
    case ERROR_BAD_EXE_FORMAT: return ENOEXEC;
    case ERROR_ITERATED_DATA_EXCEEDS_64k: return ENOEXEC;
    case ERROR_INVALID_MINALLOCSIZE: return ENOEXEC;
    case ERROR_DYNLINK_FROM_INVALID_RING: return ENOEXEC;
    case ERROR_IOPL_NOT_ENABLED: return ENOEXEC;
    case ERROR_INVALID_SEGDPL: return ENOEXEC;
    case ERROR_AUTODATASEG_EXCEEDS_64k: return ENOEXEC;
    case ERROR_RING2SEG_MUST_BE_MOVABLE: return ENOEXEC;
    case ERROR_RELOC_CHAIN_XEEDS_SEGLIM: return ENOEXEC;
    case ERROR_INFLOOP_IN_RELOC_CHAIN: return ENOEXEC;
    case ERROR_FILENAME_EXCED_RANGE: return ENOENT;
    case ERROR_NESTING_NOT_ALLOWED: return EAGAIN;
    case ERROR_NOT_ENOUGH_QUOTA: return ENOMEM;

    default:
      return EIO;
    }
}

/* Wrapper to be used by other modules to set ERRNO from the Windows
 * error.  EC may be -1 to get the last error.  */
void
_gpgrt_w32_set_errno (int ec)
{
  if (ec == -1)
    ec = GetLastError ();
  _set_errno (map_w32_to_errno (ec));
}


gpg_err_code_t
_gpgrt_w32_get_last_err_code (void)
{
  int ec = GetLastError ();
  errno = map_w32_to_errno (ec);
  return _gpg_err_code_from_errno (errno);
}


#endif /*HAVE_W32_SYSTEM*/

/*
 * Replacement for a missing memrchr.
 */
#ifndef HAVE_MEMRCHR
static void *
memrchr (const void *buffer, int c, size_t n)
{
  const unsigned char *p = buffer;

  for (p += n; n ; n--)
    if (*--p == c)
      return (void *)p;
  return NULL;
}
#endif /*HAVE_MEMRCHR*/



/*
 * Wrappers to lock a stream or the list of streams.
 */
#if 0
# define dbg_lock_0(f)        fprintf (stderr, "estream: " f);
# define dbg_lock_1(f, a)     fprintf (stderr, "estream: " f, (a));
# define dbg_lock_2(f, a, b)  fprintf (stderr, "estream: " f, (a), (b));
#else
# define dbg_lock_0(f)
# define dbg_lock_1(f, a)
# define dbg_lock_2(f, a, b)
#endif

static int
init_stream_lock (estream_t _GPGRT__RESTRICT stream)
{
  int rc;

  if (!stream->intern->samethread)
    {
      dbg_lock_1 ("enter init_stream_lock for %p\n", stream);
      memset (&stream->intern->lock, 0 , sizeof stream->intern->lock);
      rc = _gpgrt_lock_init (&stream->intern->lock);
      dbg_lock_2 ("leave init_stream_lock for %p: rc=%d\n", stream, rc);
    }
  else
    rc = 0;
  return rc;
}


static void
destroy_stream_lock (estream_t _GPGRT__RESTRICT stream)
{
  if (!stream->intern->samethread)
    {
      dbg_lock_1 ("enter destroy_stream_lock for %p\n", stream);
      _gpgrt_lock_destroy (&stream->intern->lock);
      dbg_lock_1 ("leave destroy_stream_lock for %p\n", stream);
    }
}


static void
lock_stream (estream_t _GPGRT__RESTRICT stream)
{
  if (!stream->intern->samethread)
    {
      dbg_lock_1 ("enter lock_stream for %p\n", stream);
      _gpgrt_lock_lock (&stream->intern->lock);
      dbg_lock_1 ("leave lock_stream for %p\n", stream);
    }
}


static int
trylock_stream (estream_t _GPGRT__RESTRICT stream)
{
  int rc;

  if (!stream->intern->samethread)
    {
      dbg_lock_1 ("enter trylock_stream for %p\n", stream);
      rc = _gpgrt_lock_trylock (&stream->intern->lock)? 0 : -1;
      dbg_lock_2 ("leave trylock_stream for %p: rc=%d\n", stream, rc);
    }
  else
    rc = 0;
  return rc;
}


static void
unlock_stream (estream_t _GPGRT__RESTRICT stream)
{
  if (!stream->intern->samethread)
    {
      dbg_lock_1 ("enter unlock_stream for %p\n", stream);
      _gpgrt_lock_unlock (&stream->intern->lock);
      dbg_lock_1 ("leave unlock_stream for %p\n", stream);
    }
}


static void
lock_list (void)
{
  dbg_lock_0 ("enter lock_list\n");
  _gpgrt_lock_lock (&estream_list_lock);
  dbg_lock_0 ("leave lock_list\n");
}


static void
unlock_list (void)
{
  dbg_lock_0 ("enter unlock_list\n");
  _gpgrt_lock_unlock (&estream_list_lock);
  dbg_lock_0 ("leave unlock_list\n");
}


#undef dbg_lock_0
#undef dbg_lock_1
#undef dbg_lock_2



/*
 * Manipulation of the list of stream.
 */

/*
 * Add STREAM to the list of registered stream objects.  If
 * WITH_LOCKED_LIST is true it is assumed that the list of streams is
 * already locked.  The implementation is straightforward: We first
 * look for an unused entry in the list and use that; if none is
 * available we put a new item at the head.  We drawback of the
 * strategy never to shorten the list is that a one time allocation of
 * many streams will lead to scanning unused entries later.  If that
 * turns out to be a problem, we may either free some items from the
 * list or append new entries at the end; or use a table.  Returns 0
 * on success; on error or non-zero is returned and ERRNO set.
 */
static int
do_list_add (estream_t stream, int with_locked_list)
{
  estream_list_t item;

  if (!with_locked_list)
    lock_list ();

  for (item = estream_list; item && item->stream; item = item->next)
    ;
  if (!item)
    {
      item = mem_alloc (sizeof *item);
      if (item)
        {
          item->next = estream_list;
          estream_list = item;
        }
    }
  if (item)
    item->stream = stream;

  if (!with_locked_list)
    unlock_list ();

  return item? 0 : -1;
}

/*
 * Remove STREAM from the list of registered stream objects.
 */
static void
do_list_remove (estream_t stream, int with_locked_list)
{
  estream_list_t item, item_prev = NULL;

  if (!with_locked_list)
    lock_list ();

  for (item = estream_list; item; item = item->next)
    if (item->stream == stream)
      break;
    else
      item_prev = item;

  if (item)
    {
      if (item_prev)
        item_prev->next = item->next;
      else
        estream_list = item->next;
      mem_free (item);
    }

  if (!with_locked_list)
    unlock_list ();
}



/*
 * The atexit handler for the entire gpgrt.
 */
static void
do_deinit (void)
{
  /* Flush all streams. */
  _gpgrt_fflush (NULL);

  /* We should release the estream_list.  However there is one
     problem: That list is also used to search for the standard
     estream file descriptors.  If we would remove the entire list,
     any use of es_foo in another atexit function may re-create the
     list and the streams with possible undesirable effects.  Given
     that we don't close the stream either, it should not matter that
     we keep the list and let the OS clean it up at process end.  */

  /* Reset the syscall clamp.  */
  _gpgrt_set_syscall_clamp (NULL, NULL);
}


/*
 * Initialization of the estream module.
 */
int
_gpgrt_estream_init (void)
{
  static int initialized;

  if (!initialized)
    {
      initialized = 1;
      atexit (do_deinit);
    }
  return 0;
}


/*
 * Implementation of memory based I/O.
 */

/* Cookie for memory objects.  */
typedef struct estream_cookie_mem
{
  unsigned int modeflags;	/* Open flags.  */
  unsigned char *memory;	/* Allocated data buffer.  */
  size_t memory_size;		/* Allocated size of MEMORY.  */
  size_t memory_limit;          /* Caller supplied maximum allowed
                                   allocation size or 0 for no limit.  */
  size_t offset;		/* Current offset in MEMORY.  */
  size_t data_len;		/* Used length of data in MEMORY.  */
  size_t block_size;		/* Block size.  */
  struct {
    unsigned int grow: 1;	/* MEMORY is allowed to grow.  */
  } flags;
  func_realloc_t func_realloc;
  func_free_t func_free;
} *estream_cookie_mem_t;


/*
 * Create function for memory objects.  DATA is either NULL or a user
 * supplied buffer with the initial conetnt of the memory buffer.  If
 * DATA is NULL, DATA_N and DATA_LEN need to be 0 as well.  If DATA is
 * not NULL, DATA_N gives the allocated size of DATA and DATA_LEN the
 * used length in DATA.  If this function succeeds DATA is now owned
 * by this function.  If GROW is false FUNC_REALLOC is not
 * required.
 */
static int
func_mem_create (void *_GPGRT__RESTRICT *_GPGRT__RESTRICT cookie,
                 unsigned char *_GPGRT__RESTRICT data, size_t data_n,
                 size_t data_len,
                 size_t block_size, unsigned int grow,
                 func_realloc_t func_realloc, func_free_t func_free,
                 unsigned int modeflags,
                 size_t memory_limit)
{
  estream_cookie_mem_t mem_cookie;
  int err;

  if (!data && (data_n || data_len))
    {
      _set_errno (EINVAL);
      return -1;
    }
  if (grow && func_free && !func_realloc)
    {
      _set_errno (EINVAL);
      return -1;
    }

  /* Round a memory limit up to the next block length.  */
  if (memory_limit && block_size)
    {
      memory_limit += block_size - 1;
      memory_limit /= block_size;
      memory_limit *= block_size;
    }

  mem_cookie = mem_alloc (sizeof (*mem_cookie));
  if (!mem_cookie)
    err = -1;
  else
    {
      mem_cookie->modeflags = modeflags;
      mem_cookie->memory = data;
      mem_cookie->memory_size = data_n;
      mem_cookie->memory_limit = memory_limit;
      mem_cookie->offset = 0;
      mem_cookie->data_len = data_len;
      mem_cookie->block_size = block_size;
      mem_cookie->flags.grow = !!grow;
      mem_cookie->func_realloc
        = grow? (func_realloc ? func_realloc : mem_realloc) : NULL;
      mem_cookie->func_free = func_free ? func_free : mem_free;
      *cookie = mem_cookie;
      err = 0;
    }

  return err;
}


/*
 * Read function for memory objects.
 */
static gpgrt_ssize_t
func_mem_read (void *cookie, void *buffer, size_t size)
{
  estream_cookie_mem_t mem_cookie = cookie;
  gpgrt_ssize_t ret;

  if (!size)  /* Just the pending data check.  */
    return (mem_cookie->data_len - mem_cookie->offset)? 0 : -1;

  if (size > mem_cookie->data_len - mem_cookie->offset)
    size = mem_cookie->data_len - mem_cookie->offset;

  if (size)
    {
      memcpy (buffer, mem_cookie->memory + mem_cookie->offset, size);
      mem_cookie->offset += size;
    }

  ret = size;
  return ret;
}


/*
 * Write function for memory objects.
 */
static gpgrt_ssize_t
func_mem_write (void *cookie, const void *buffer, size_t size)
{
  estream_cookie_mem_t mem_cookie = cookie;
  gpgrt_ssize_t ret;
  size_t nleft;

  if (!size)
    return 0;  /* A flush is a NOP for memory objects.  */

  if (mem_cookie->modeflags & O_APPEND)
    {
      /* Append to data.  */
      mem_cookie->offset = mem_cookie->data_len;
    }

  gpgrt_assert (mem_cookie->memory_size >= mem_cookie->offset);
  nleft = mem_cookie->memory_size - mem_cookie->offset;

  /* If we are not allowed to grow the buffer, limit the size to the
     left space.  */
  if (!mem_cookie->flags.grow && size > nleft)
    size = nleft;

  /* Enlarge the memory buffer if needed.  */
  if (size > nleft)
    {
      unsigned char *newbuf;
      size_t newsize;

      if (!mem_cookie->memory_size)
        newsize = size;  /* Not yet allocated.  */
      else
        newsize = mem_cookie->memory_size + (size - nleft);
      if (newsize < mem_cookie->offset)
        {
          _set_errno (EINVAL);
          return -1;
        }

      /* Round up to the next block length.  BLOCK_SIZE should always
         be set; we check anyway.  */
      if (mem_cookie->block_size)
        {
          newsize += mem_cookie->block_size - 1;
          if (newsize < mem_cookie->offset)
            {
              _set_errno (EINVAL);
              return -1;
            }
          newsize /= mem_cookie->block_size;
          newsize *= mem_cookie->block_size;
        }

      /* Check for a total limit.  */
      if (mem_cookie->memory_limit && newsize > mem_cookie->memory_limit)
        {
          _set_errno (ENOSPC);
          return -1;
        }

      gpgrt_assert (mem_cookie->func_realloc);
      newbuf = mem_cookie->func_realloc (mem_cookie->memory, newsize);
      if (!newbuf)
        return -1;

      mem_cookie->memory = newbuf;
      mem_cookie->memory_size = newsize;

      gpgrt_assert (mem_cookie->memory_size >= mem_cookie->offset);
      nleft = mem_cookie->memory_size - mem_cookie->offset;

      gpgrt_assert (size <= nleft);
    }

  memcpy (mem_cookie->memory + mem_cookie->offset, buffer, size);
  if (mem_cookie->offset + size > mem_cookie->data_len)
    mem_cookie->data_len = mem_cookie->offset + size;
  mem_cookie->offset += size;

  ret = size;
  return ret;
}


/*
 * Seek function for memory objects.
 */
static int
func_mem_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  estream_cookie_mem_t mem_cookie = cookie;
  gpgrt_off_t pos_new;

  switch (whence)
    {
    case SEEK_SET:
      pos_new = *offset;
      break;

    case SEEK_CUR:
      pos_new = mem_cookie->offset += *offset;
      break;

    case SEEK_END:
      pos_new = mem_cookie->data_len += *offset;
      break;

    default:
      _set_errno (EINVAL);
      return -1;
    }

  if (pos_new > mem_cookie->memory_size)
    {
      size_t newsize;
      void *newbuf;

      if (!mem_cookie->flags.grow)
	{
	  _set_errno (ENOSPC);
	  return -1;
        }

      newsize = pos_new + mem_cookie->block_size - 1;
      if (newsize < pos_new)
        {
          _set_errno (EINVAL);
          return -1;
        }
      newsize /= mem_cookie->block_size;
      newsize *= mem_cookie->block_size;

      if (mem_cookie->memory_limit && newsize > mem_cookie->memory_limit)
        {
          _set_errno (ENOSPC);
          return -1;
        }

      gpgrt_assert (mem_cookie->func_realloc);
      newbuf = mem_cookie->func_realloc (mem_cookie->memory, newsize);
      if (!newbuf)
        return -1;

      mem_cookie->memory = newbuf;
      mem_cookie->memory_size = newsize;
    }

  if (pos_new > mem_cookie->data_len)
    {
      /* Fill spare space with zeroes.  */
      memset (mem_cookie->memory + mem_cookie->data_len,
              0, pos_new - mem_cookie->data_len);
      mem_cookie->data_len = pos_new;
    }

  mem_cookie->offset = pos_new;
  *offset = pos_new;

  return 0;
}


/*
 * The IOCTL function for memory objects.
 */
static int
func_mem_ioctl (void *cookie, int cmd, void *ptr, size_t *len)
{
  estream_cookie_mem_t mem_cookie = cookie;
  int ret;

  if (cmd == COOKIE_IOCTL_SNATCH_BUFFER)
    {
      /* Return the internal buffer of the stream to the caller and
         invalidate it for the stream.  */
      *(void**)ptr = mem_cookie->memory;
      *len = mem_cookie->data_len;
      mem_cookie->memory = NULL;
      mem_cookie->memory_size = 0;
      mem_cookie->offset = 0;
      ret = 0;
    }
  else if (cmd == COOKIE_IOCTL_TRUNCATE)
    {
      gpgrt_off_t length = *(gpgrt_off_t *)ptr;

      ret = func_mem_seek (cookie, &length, SEEK_SET);
      if (ret != -1)
        mem_cookie->data_len = mem_cookie->offset;
    }
  else
    {
      _set_errno (EINVAL);
      ret = -1;
    }

  return ret;
}


/*
 * The destroy function for memory objects.
 */
static int
func_mem_destroy (void *cookie)
{
  estream_cookie_mem_t mem_cookie = cookie;

  if (cookie)
    {
      mem_cookie->func_free (mem_cookie->memory);
      mem_free (mem_cookie);
    }
  return 0;
}

/*
 * Access object for the memory functions.
 */
static struct cookie_io_functions_s estream_functions_mem =
  {
    {
      func_mem_read,
      func_mem_write,
      func_mem_seek,
      func_mem_destroy,
    },
    func_mem_ioctl,
  };



/*
 * Implementation of file descriptor based I/O.
 */

/* Cookie for fd objects.  */
typedef struct estream_cookie_fd
{
  int fd;        /* The file descriptor we are using for actual output.  */
  int no_close;  /* If set we won't close the file descriptor.  */
  int nonblock;  /* Non-blocking mode is enabled.  */
} *estream_cookie_fd_t;


/*
 * Create function for objects indentified by a libc file descriptor.
 */
static int
func_fd_create (void **cookie, int fd, unsigned int modeflags, int no_close)
{
  estream_cookie_fd_t fd_cookie;
  int err;

  trace (("enter: fd=%d mf=%x nc=%d", fd, modeflags, no_close));

  fd_cookie = mem_alloc (sizeof (*fd_cookie));
  if (! fd_cookie)
    err = -1;
  else
    {
#ifdef HAVE_DOSISH_SYSTEM
      /* Make sure it is in binary mode if requested.  */
      if ( (modeflags & O_BINARY) )
        setmode (fd, O_BINARY);
#endif
      fd_cookie->fd = fd;
      fd_cookie->no_close = no_close;
      fd_cookie->nonblock = !!(modeflags & O_NONBLOCK);
      *cookie = fd_cookie;
      err = 0;
    }

  trace_errno (err, ("leave: cookie=%p err=%d", *cookie, err));
  return err;
}


/*
 * Read function for fd objects.
 */
static gpgrt_ssize_t
func_fd_read (void *cookie, void *buffer, size_t size)

{
  estream_cookie_fd_t file_cookie = cookie;
  gpgrt_ssize_t bytes_read;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (!size)
    bytes_read = -1; /* We don't know whether anything is pending.  */
  else if (IS_INVALID_FD (file_cookie->fd))
    {
      _gpgrt_yield ();
      bytes_read = 0;
    }
  else
    {
      _gpgrt_pre_syscall ();
      do
        {
          bytes_read = read (file_cookie->fd, buffer, size);
        }
      while (bytes_read == -1 && errno == EINTR);
      _gpgrt_post_syscall ();
    }

  trace_errno (bytes_read == -1, ("leave: bytes_read=%d", (int)bytes_read));
  return bytes_read;
}


/*
 * Write function for fd objects.
 */
static gpgrt_ssize_t
func_fd_write (void *cookie, const void *buffer, size_t size)
{
  estream_cookie_fd_t file_cookie = cookie;
  gpgrt_ssize_t bytes_written;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (IS_INVALID_FD (file_cookie->fd))
    {
      _gpgrt_yield ();
      bytes_written = size; /* Yeah:  Success writing to the bit bucket.  */
    }
  else if (buffer)
    {
      _gpgrt_pre_syscall ();
      do
        {
          bytes_written = write (file_cookie->fd, buffer, size);
        }
      while (bytes_written == -1 && errno == EINTR);
      _gpgrt_post_syscall ();
    }
  else
    bytes_written = size; /* Note that for a flush SIZE should be 0.  */

  trace_errno (bytes_written == -1,
               ("leave: bytes_written=%d", (int)bytes_written));
  return bytes_written;
}


/*
 * Seek function for fd objects.
 */
static int
func_fd_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  estream_cookie_fd_t file_cookie = cookie;
  gpgrt_off_t offset_new;
  int err;

  if (IS_INVALID_FD (file_cookie->fd))
    {
      _set_errno (ESPIPE);
      err = -1;
    }
  else
    {
      _gpgrt_pre_syscall ();
      offset_new = lseek (file_cookie->fd, *offset, whence);
      _gpgrt_post_syscall ();
      if (offset_new == -1)
        err = -1;
      else
        {
          *offset = offset_new;
          err = 0;
        }
    }

  return err;
}


/*
 * The IOCTL function for fd objects.
 */
static int
func_fd_ioctl (void *cookie, int cmd, void *ptr, size_t *len)
{
  estream_cookie_fd_t fd_cookie = cookie;
  int ret;

  if (cmd == COOKIE_IOCTL_NONBLOCK && !len)
    {
      fd_cookie->nonblock = !!ptr;
      if (IS_INVALID_FD (fd_cookie->fd))
        {
          _set_errno (EINVAL);
          ret = -1;
        }
      else
        {
#ifdef _WIN32
          _set_errno (EOPNOTSUPP); /* FIXME: Implement for Windows.  */
          ret = -1;
#else
          _set_errno (0);
          ret = fcntl (fd_cookie->fd, F_GETFL, 0);
          if (ret == -1 && errno)
            ;
          else if (fd_cookie->nonblock)
            ret = fcntl (fd_cookie->fd, F_SETFL, (ret | O_NONBLOCK));
          else
            ret = fcntl (fd_cookie->fd, F_SETFL, (ret & ~O_NONBLOCK));
#endif
        }
    }
  else
    {
      _set_errno (EINVAL);
      ret = -1;
    }

  return ret;
}

/*
 * The destroy function for fd objects.
 */
static int
func_fd_destroy (void *cookie)
{
  estream_cookie_fd_t fd_cookie = cookie;
  int err;

  trace (("enter: cookie=%p", cookie));

  if (fd_cookie)
    {
      if (IS_INVALID_FD (fd_cookie->fd))
        err = 0;
      else
        err = fd_cookie->no_close? 0 : close (fd_cookie->fd);
      mem_free (fd_cookie);
    }
  else
    err = 0;

  trace_errno (err,("leave: err=%d", err));
  return err;
}


/*
 * Access object for the fd functions.
 */
static struct cookie_io_functions_s estream_functions_fd =
  {
    {
      func_fd_read,
      func_fd_write,
      func_fd_seek,
      func_fd_destroy,
    },
    func_fd_ioctl,
  };



#ifdef HAVE_W32_SYSTEM
/*
 * Implementation of SOCKET based I/O.
 */

/* Cookie for SOCKET objects.  */
typedef struct estream_cookie_sock
{
  SOCKET sock;   /* The SOCKET we are using for actual output.  */
  int no_close;  /* If set we won't close the file descriptor.  */
  int nonblock;  /* Non-blocking mode is enabled.  */
} *estream_cookie_sock_t;


/*
 * Create function for objects indentified by a libc file descriptor.
 */
static int
func_sock_create (void **cookie, SOCKET sock,
                  unsigned int modeflags, int no_close)
{
  estream_cookie_sock_t sock_cookie;
  int err;

  trace (("enter: sock=%d mf=%x nc=%d", (int)sock, modeflags, no_close));

  sock_cookie = mem_alloc (sizeof (*sock_cookie));
  if (! sock_cookie)
    err = -1;
  else
    {
      sock_cookie->sock = sock;
      sock_cookie->no_close = no_close;
      sock_cookie->nonblock = !!(modeflags & O_NONBLOCK);
      *cookie = sock_cookie;
      err = 0;
    }

  trace_errno (err, ("leave: cookie=%p err=%d", *cookie, err));
  return err;
}


/*
 * Read function for SOCKET objects.
 */
static gpgrt_ssize_t
func_sock_read (void *cookie, void *buffer, size_t size)

{
  estream_cookie_sock_t file_cookie = cookie;
  gpgrt_ssize_t bytes_read;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (!size)
    bytes_read = -1; /* We don't know whether anything is pending.  */
  else if (IS_INVALID_FD (file_cookie->sock))
    {
      _gpgrt_yield ();
      bytes_read = 0;
    }
  else
    {
      _gpgrt_pre_syscall ();
      do
        {
          bytes_read = recv (file_cookie->sock, buffer, size, 0);
        }
      while (bytes_read == -1 && errno == EINTR);
      _gpgrt_post_syscall ();
    }

  trace_errno (bytes_read == -1, ("leave: bytes_read=%d", (int)bytes_read));
  return bytes_read;
}


/*
 * Write function for SOCKET objects.
 */
static gpgrt_ssize_t
func_sock_write (void *cookie, const void *buffer, size_t size)
{
  estream_cookie_sock_t file_cookie = cookie;
  gpgrt_ssize_t bytes_written;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (IS_INVALID_FD (file_cookie->sock))
    {
      _gpgrt_yield ();
      bytes_written = size; /* Yeah:  Success writing to the bit bucket.  */
    }
  else if (buffer)
    {
      _gpgrt_pre_syscall ();
      do
        {
          bytes_written = send (file_cookie->sock, buffer, size, 0);
        }
      while (bytes_written == -1 && errno == EINTR);
      _gpgrt_post_syscall ();
    }
  else
    bytes_written = size; /* Note that for a flush SIZE should be 0.  */

  trace_errno (bytes_written == -1,
               ("leave: bytes_written=%d", (int)bytes_written));
  return bytes_written;
}


/*
 * Seek function for SOCKET objects.
 */
static int
func_sock_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  (void)cookie;
  (void)offset;
  (void)whence;
  _set_errno (ESPIPE);
  return -1;
}


/*
 * The IOCTL function for SOCKET objects.
 */
static int
func_sock_ioctl (void *cookie, int cmd, void *ptr, size_t *len)
{
  estream_cookie_sock_t sock_cookie = cookie;
  int ret;

  if (cmd == COOKIE_IOCTL_NONBLOCK && !len)
    {
      sock_cookie->nonblock = !!ptr;
      if (IS_INVALID_FD (sock_cookie->sock))
        {
          _set_errno (EINVAL);
          ret = -1;
        }
      else
        {
          u_long mode = 0;

          if (sock_cookie->nonblock)
            mode = 1;

          ret = ioctlsocket (sock_cookie->sock, FIONBIO, &mode);
        }
    }
  else
    {
      _set_errno (EINVAL);
      ret = -1;
    }

  return ret;
}

/*
 * The destroy function for SOCKET objects.
 */
static int
func_sock_destroy (void *cookie)
{
  estream_cookie_sock_t sock_cookie = cookie;
  int err;

  trace (("enter: cookie=%p", cookie));

  if (sock_cookie)
    {
      if (IS_INVALID_FD (sock_cookie->sock))
        err = 0;
      else
        err = sock_cookie->no_close? 0 : closesocket (sock_cookie->sock);
      mem_free (sock_cookie);
    }
  else
    err = 0;

  trace_errno (err,("leave: err=%d", err));
  return err;
}


/*
 * Access object for the fd functions.
 */
static struct cookie_io_functions_s estream_functions_sock =
  {
    {
      func_sock_read,
      func_sock_write,
      func_sock_seek,
      func_sock_destroy,
    },
    func_sock_ioctl,
  };

/*
 * Implementation of W32 handle based I/O.
 */

/* Cookie for fd objects.  */
typedef struct estream_cookie_w32
{
  HANDLE hd;     /* The handle we are using for actual output.  */
  int no_close;  /* If set we won't close the handle.  */
  int no_syscall_clamp; /* Do not use the syscall clamp. */
} *estream_cookie_w32_t;


/*
 * Create function for w32 handle objects.
 */
static int
func_w32_create (void **cookie, HANDLE hd,
                 unsigned int modeflags, int no_close, int no_syscall_clamp)
{
  estream_cookie_w32_t w32_cookie;
  int err;

  trace (("enter: hd=%p mf=%x nc=%d nsc=%d",
          hd, modeflags, no_close, no_syscall_clamp));
  w32_cookie = mem_alloc (sizeof (*w32_cookie));
  if (!w32_cookie)
    err = -1;
  else
    {
      /* CR/LF translations are not supported when using the bare W32
         API.  If that is really required we need to implemented that
         in the upper layer.  */
      (void)modeflags;

      w32_cookie->hd = hd;
      w32_cookie->no_close = no_close;
      w32_cookie->no_syscall_clamp = no_syscall_clamp;
      *cookie = w32_cookie;
      err = 0;
    }

  trace_errno (err, ("leave: cookie=%p err=%d", *cookie, err));
  return err;
}

/*
 * Read function for W32 handle objects.
 *
 * Note that this function may also be used by the reader thread of
 * w32-stream.  In that case the NO_SYSCALL_CLAMP is set.
 */
static gpgrt_ssize_t
func_w32_read (void *cookie, void *buffer, size_t size)
{
  estream_cookie_w32_t w32_cookie = cookie;
  gpgrt_ssize_t bytes_read;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (!size)
    bytes_read = -1; /* We don't know whether anything is pending.  */
  else if (w32_cookie->hd == INVALID_HANDLE_VALUE)
    {
      _gpgrt_yield ();
      bytes_read = 0;
    }
  else
    {
      if (!w32_cookie->no_syscall_clamp)
        _gpgrt_pre_syscall ();
      do
        {
          DWORD nread, ec;

          trace (("cookie=%p calling ReadFile", cookie));
          if (!ReadFile (w32_cookie->hd, buffer, size, &nread, NULL))
            {
              ec = GetLastError ();
              trace (("cookie=%p ReadFile failed: ec=%ld", cookie,ec));
              if (ec == ERROR_BROKEN_PIPE)
                bytes_read = 0; /* Like our pth_read we handle this as EOF.  */
              else
                {
                  _set_errno (map_w32_to_errno (ec));
                  bytes_read = -1;
                }
            }
          else
            bytes_read = (int)nread;
        }
      while (bytes_read == -1 && errno == EINTR);
      if (!w32_cookie->no_syscall_clamp)
        _gpgrt_post_syscall ();
    }

  trace_errno (bytes_read==-1,("leave: bytes_read=%d", (int)bytes_read));
  return bytes_read;
}


/*
 * Write function for W32 handle objects.
 *
 * Note that this function may also be used by the writer thread of
 * w32-stream.  In that case the NO_SYSCALL_CLAMP is set.
 */
static gpgrt_ssize_t
func_w32_write (void *cookie, const void *buffer, size_t size)
{
  estream_cookie_w32_t w32_cookie = cookie;
  gpgrt_ssize_t bytes_written;

  trace (("enter: cookie=%p buffer=%p size=%d", cookie, buffer, (int)size));

  if (w32_cookie->hd == INVALID_HANDLE_VALUE)
    {
      _gpgrt_yield ();
      bytes_written = size; /* Yeah:  Success writing to the bit bucket.  */
    }
  else if (buffer)
    {
      if (!w32_cookie->no_syscall_clamp)
        _gpgrt_pre_syscall ();
      do
        {
          DWORD nwritten;

          trace (("cookie=%p calling WriteFile", cookie));
	  if (!WriteFile (w32_cookie->hd, buffer, size, &nwritten, NULL))
	    {
              DWORD ec = GetLastError ();
              trace (("cookie=%p WriteFile failed: ec=%ld", cookie, ec));
	      _set_errno (map_w32_to_errno (ec));
	      bytes_written = -1;
	    }
	  else
	    bytes_written = (int)nwritten;
        }
      while (bytes_written == -1 && errno == EINTR);
      if (!w32_cookie->no_syscall_clamp)
        _gpgrt_post_syscall ();
    }
  else
    bytes_written = size; /* Note that for a flush SIZE should be 0.  */

  trace_errno (bytes_written==-1,
               ("leave: bytes_written=%d", (int)bytes_written));
  return bytes_written;
}


/*
 * Seek function for W32 handle objects.
 */
static int
func_w32_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  estream_cookie_w32_t w32_cookie = cookie;
  DWORD method;
  LARGE_INTEGER distance, newoff;

  if (w32_cookie->hd == INVALID_HANDLE_VALUE)
    {
      _set_errno (ESPIPE);
      return -1;
    }

  if (GetFileType (w32_cookie->hd) == FILE_TYPE_PIPE)
    {
      _set_errno (ESPIPE);
      return -1;
    }

  if (whence == SEEK_SET)
    {
      method = FILE_BEGIN;
      distance.QuadPart = (unsigned long long)(*offset);
    }
  else if (whence == SEEK_CUR)
    {
      method = FILE_CURRENT;
      distance.QuadPart = (long long)(*offset);
    }
  else if (whence == SEEK_END)
    {
      method = FILE_END;
      distance.QuadPart = (long long)(*offset);
    }
  else
    {
      _set_errno (EINVAL);
      return -1;
    }
  if (!w32_cookie->no_syscall_clamp)
    _gpgrt_pre_syscall ();
  if (!SetFilePointerEx (w32_cookie->hd, distance, &newoff, method))
    {
      _set_errno (map_w32_to_errno (GetLastError ()));
      _gpgrt_post_syscall ();
      return -1;
    }
  if (!w32_cookie->no_syscall_clamp)
    _gpgrt_post_syscall ();
  /* Note that gpgrt_off_t is always 64 bit.  */
  *offset = (gpgrt_off_t)newoff.QuadPart;
  return 0;
}


/*
 * Destroy function for W32 handle objects.
 */
static int
func_w32_destroy (void *cookie)
{
  estream_cookie_w32_t w32_cookie = cookie;
  int err;

  trace (("enter: cookie=%p", cookie));

  if (w32_cookie)
    {
      if (w32_cookie->hd == INVALID_HANDLE_VALUE)
        err = 0;
      else if (w32_cookie->no_close)
        err = 0;
      else
        {
          trace (("cookie=%p closing handle %p", cookie, w32_cookie->hd));
          if (!CloseHandle (w32_cookie->hd))
            {
              DWORD ec = GetLastError ();
              trace (("cookie=%p CloseHandle failed: ec=%ld", cookie,ec));
	      _set_errno (map_w32_to_errno (ec));
              err = -1;
            }
          else
            err = 0;
        }
      mem_free (w32_cookie);
    }
  else
    err = 0;

  trace_errno (err, ("leave: err=%d", err));
  return err;
}


/*
 * Access object for the W32 handle based objects.
 */
static struct cookie_io_functions_s estream_functions_w32 =
  {
    {
      func_w32_read,
      func_w32_write,
      func_w32_seek,
      func_w32_destroy,
    },
    NULL,
  };
#endif /*HAVE_W32_SYSTEM*/




/*
 * Implementation of stdio based I/O.
 */

/* Cookie for fp objects.  */
typedef struct estream_cookie_fp
{
  FILE *fp;      /* The file pointer we are using for actual output.  */
  int no_close;  /* If set we won't close the file pointer.  */
} *estream_cookie_fp_t;


/*
 * Create function for stdio based objects.
 */
static int
func_fp_create (void **cookie, FILE *fp,
                unsigned int modeflags, int no_close)
{
  estream_cookie_fp_t fp_cookie;
  int err;

  fp_cookie = mem_alloc (sizeof *fp_cookie);
  if (!fp_cookie)
    err = -1;
  else
    {
#ifdef HAVE_DOSISH_SYSTEM
      /* Make sure it is in binary mode if requested.  */
      if ( (modeflags & O_BINARY) )
        setmode (fileno (fp), O_BINARY);
#else
      (void)modeflags;
#endif
      fp_cookie->fp = fp;
      fp_cookie->no_close = no_close;
      *cookie = fp_cookie;
      err = 0;
    }

  return err;
}


/*
 * Read function for stdio based objects.
 */
static gpgrt_ssize_t
func_fp_read (void *cookie, void *buffer, size_t size)

{
  estream_cookie_fp_t file_cookie = cookie;
  gpgrt_ssize_t bytes_read;

  if (!size)
    return -1; /* We don't know whether anything is pending.  */

  if (file_cookie->fp)
    {
      _gpgrt_pre_syscall ();
      bytes_read = fread (buffer, 1, size, file_cookie->fp);
      _gpgrt_post_syscall ();
    }
  else
    bytes_read = 0;
  if (!bytes_read && ferror (file_cookie->fp))
    return -1;
  return bytes_read;
}


/*
 * Write function for stdio bases objects.
 */
static gpgrt_ssize_t
func_fp_write (void *cookie, const void *buffer, size_t size)
{
  estream_cookie_fp_t file_cookie = cookie;
  size_t bytes_written;

  if (file_cookie->fp)
    {
      _gpgrt_pre_syscall ();
      if (buffer)
        {
#ifdef HAVE_W32_SYSTEM
          /* Using an fwrite to stdout connected to the console fails
             with the error "Not enough space" for an fwrite size of
             >= 52KB (tested on Windows XP SP2).  To solve this we
             always chunk the writes up into smaller blocks.  */
          bytes_written = 0;
          while (bytes_written < size)
            {
              size_t cnt = size - bytes_written;

              if (cnt > 32*1024)
                cnt = 32*1024;
              if (fwrite ((const char*)buffer + bytes_written,
                          cnt, 1, file_cookie->fp) != 1)
                break; /* Write error.  */
              bytes_written += cnt;
            }
#else
          bytes_written = fwrite (buffer, 1, size, file_cookie->fp);
#endif
        }
      else /* Only flush requested.  */
        bytes_written = size;

      fflush (file_cookie->fp);
      _gpgrt_post_syscall ();
    }
  else
    bytes_written = size; /* Successfully written to the bit bucket.  */

  if (bytes_written != size)
    return -1;
  return bytes_written;
}


/*
 * Seek function for stdio based objects.
 */
static int
func_fp_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  estream_cookie_fp_t file_cookie = cookie;
  long int offset_new;

  if (!file_cookie->fp)
    {
      _set_errno (ESPIPE);
      return -1;
    }

  _gpgrt_pre_syscall ();
  if ( fseek (file_cookie->fp, (long int)*offset, whence) )
    {
      /* fprintf (stderr, "\nfseek failed: errno=%d (%s)\n", */
      /*          errno,strerror (errno)); */
      _gpgrt_post_syscall ();
      return -1;
    }

  offset_new = ftell (file_cookie->fp);
  _gpgrt_post_syscall ();
  if (offset_new == -1)
    {
      /* fprintf (stderr, "\nftell failed: errno=%d (%s)\n",  */
      /*          errno,strerror (errno)); */
      return -1;
    }
  *offset = offset_new;
  return 0;
}


/*
 * Destroy function for stdio based objects.
 */
static int
func_fp_destroy (void *cookie)
{
  estream_cookie_fp_t fp_cookie = cookie;
  int err;

  if (fp_cookie)
    {
      if (fp_cookie->fp)
        {
          _gpgrt_pre_syscall ();
          fflush (fp_cookie->fp);
          _gpgrt_post_syscall ();
          err = fp_cookie->no_close? 0 : fclose (fp_cookie->fp);
        }
      else
        err = 0;
      mem_free (fp_cookie);
    }
  else
    err = 0;

  return err;
}


/*
 * Access object for stdio based objects.
 */
static struct cookie_io_functions_s estream_functions_fp =
  {
    {
      func_fp_read,
      func_fp_write,
      func_fp_seek,
      func_fp_destroy,
    },
    NULL,
  };




/*
 * Implementation of file name based I/O.
 *
 * Note that only a create function is required because the other
 * operations ares handled by file descriptor based I/O.
 */

#ifdef HAVE_W32_SYSTEM
static int
any8bitchar (const char *string)
{
  if (string)
    for ( ; *string; string++)
      if ((*string & 0x80))
        return 1;
  return 0;
}
#endif /*HAVE_W32_SYSTEM*/

/* Create function for objects identified by a file name.  */
static int
func_file_create (void **cookie, int *filedes,
                  const char *path, unsigned int modeflags, unsigned int cmode)
{
  estream_cookie_fd_t file_cookie;
  int err;
  int fd;

  err = 0;

  file_cookie = mem_alloc (sizeof (*file_cookie));
  if (! file_cookie)
    {
      err = -1;
      goto out;
    }

#ifdef HAVE_W32_SYSTEM
  if (any8bitchar (path))
    {
      wchar_t *wpath;

      wpath = _gpgrt_utf8_to_wchar (path);
      if (!wpath)
        fd = -1;
      else
        {
          fd = _wopen (wpath, modeflags, cmode);
          _gpgrt_free_wchar (wpath);
        }
    }
  else  /* Avoid unnecessary conversion.  */
    fd = open (path, modeflags, cmode);
#else
  fd = open (path, modeflags, cmode);
#endif
  if (fd == -1)
    {
      err = -1;
      goto out;
    }
#ifdef HAVE_DOSISH_SYSTEM
  /* Make sure it is in binary mode if requested.  */
  if ( (modeflags & O_BINARY) )
    setmode (fd, O_BINARY);
#endif

  file_cookie->fd = fd;
  file_cookie->no_close = 0;
  *cookie = file_cookie;
  *filedes = fd;

 out:

  if (err)
    mem_free (file_cookie);

  return err;
}


/* Create function for objects identified by a file name.  Windows
 * version to use CreateFile.  */
#ifdef HAVE_W32_SYSTEM
static int
func_file_create_w32 (void **cookie, HANDLE *rethd, const char *path,
                      unsigned int modeflags, unsigned int cmode)
{
  estream_cookie_w32_t hd_cookie;
  wchar_t *wpath = NULL;
  int err = 0;
  HANDLE hd;
  DWORD desired_access;
  DWORD share_mode;
  DWORD creation_distribution;

  (void)cmode;

  hd_cookie = mem_alloc (sizeof *hd_cookie);
  if (!hd_cookie)
    {
      err = -1;
      goto leave;
    }

  wpath = _gpgrt_fname_to_wchar (path);
  if (!wpath)
    {
      err = -1;
      goto leave;
    }

  if ((modeflags & O_WRONLY))
    {
      desired_access = GENERIC_WRITE;
      share_mode = FILE_SHARE_WRITE;
    }
  else if ((modeflags & O_RDWR))
    {
      desired_access = GENERIC_READ | GENERIC_WRITE;
      share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
  else
    {
      desired_access = GENERIC_READ;
      share_mode = FILE_SHARE_READ;
    }


  creation_distribution = 0;
  if ((modeflags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
     creation_distribution |= CREATE_NEW;
  else if ((modeflags & O_TRUNC) == O_TRUNC)
    {
      if ((modeflags & O_CREAT) == O_CREAT)
        creation_distribution |= CREATE_ALWAYS;
      else if ((modeflags & O_RDONLY) != O_RDONLY)
        creation_distribution |= TRUNCATE_EXISTING;
    }
  else if ((modeflags & O_APPEND) == O_APPEND)
    creation_distribution |= OPEN_EXISTING;
  else if ((modeflags & O_CREAT) == O_CREAT)
    creation_distribution |= OPEN_ALWAYS;
  else
    creation_distribution |= OPEN_EXISTING;

  hd = CreateFileW (wpath,
                    desired_access,
                    share_mode,
                    NULL,  /* security attributes */
                    creation_distribution,
                    0,     /* flags and attributes  */
                    NULL); /* template file  */
  if (hd == INVALID_HANDLE_VALUE)
    {
      _set_errno (map_w32_to_errno (GetLastError ()));
      err = -1;
      goto leave;
    }

  hd_cookie->hd = hd;
  hd_cookie->no_close = 0;
  hd_cookie->no_syscall_clamp = 0;
  *cookie = hd_cookie;
  *rethd = hd;

 leave:
  _gpgrt_free_wchar (wpath);
  if (err)
    mem_free (hd_cookie);
  return err;
}
#endif /*HAVE_W32_SYSTEM*/



/* Flags used by parse_mode and friends.  */
#define X_SAMETHREAD	(1 << 0)
#define X_SYSOPEN	(1 << 1)
#define X_POLLABLE	(1 << 2)

/* Parse the mode flags of fopen et al.  In addition to the POSIX
 * defined mode flags keyword parameters are supported.  These are
 * key/value pairs delimited by comma and optional white spaces.
 * Keywords and values may not contain a comma or white space; unknown
 * keywords are skipped. Supported keywords are:
 *
 * mode=<string>
 *
 *    Creates a file and gives the new file read and write permissions
 *    for the user and read permission for the group.  The format of
 *    the string is the same as shown by the -l option of the ls(1)
 *    command.  However the first letter must be a dash and it is
 *    allowed to leave out trailing dashes.  If this keyword parameter
 *    is not given the default mode for creating files is "-rw-rw-r--"
 *    (664).  Note that the system still applies the current umask to
 *    the mode when creating a file.  Example:
 *
 *       "wb,mode=-rw-r--"
 *
 * samethread
 *
 *    Assumes that the object is only used by the creating thread and
 *    disables any internal locking.  This keyword is also found on
 *    IBM systems.
 *
 * nonblock
 *
 *    The object is opened in non-blocking mode.  This is the same as
 *    calling gpgrt_set_nonblock on the file.
 *
 * sysopen
 *
 *    The object is opened in GPGRT_SYSHD_HANDLE mode.  On POSIX this
 *    is a NOP but under Windows the direct W32 API functions (HANDLE)
 *    are used instead of their libc counterparts (fd).  This flag
 *    also allows to use file names longer than MAXPATH.  Note that
 *    gpgrt_fileno does not not work for such a stream under Windows.
 *
 * pollable
 *
 *    The object is opened in a way suitable for use with es_poll.  On
 *    POSIX this is a NOP but under Windows we create up to two
 *    threads, one for reading and one for writing, do any I/O there,
 *    and synchronize with them in order to support es_poll.
 *
 * Note: R_CMODE is optional because is only required by functions
 * which are able to creat a file.
 */
static int
parse_mode (const char *modestr,
            unsigned int *modeflags,
            unsigned int *r_xmode,
            unsigned int *r_cmode)
{
  unsigned int omode, oflags, cmode;
  int got_cmode = 0;

  *r_xmode = 0;

  switch (*modestr)
    {
    case 'r':
      omode = O_RDONLY;
      oflags = 0;
      break;
    case 'w':
      omode = O_WRONLY;
      oflags = O_TRUNC | O_CREAT;
      break;
    case 'a':
      omode = O_WRONLY;
      oflags = O_APPEND | O_CREAT;
      break;
    default:
      _set_errno (EINVAL);
      return -1;
    }
  for (modestr++; *modestr; modestr++)
    {
      switch (*modestr)
        {
        case '+':
          omode = O_RDWR;
          break;
        case 'b':
          oflags |= O_BINARY;
          break;
        case 'x':
          oflags |= O_EXCL;
          break;
        case ',':
          goto keyvalue;
        default: /* Ignore unknown flags.  */
          break;
        }
    }

 keyvalue:
  /* Parse key/value pairs (similar to fopen on mainframes).  */
  for (cmode=0; *modestr == ','; modestr += strcspn (modestr, ","))
    {
      modestr++;
      modestr += strspn (modestr, " \t");
      if (!strncmp (modestr, "mode=", 5))
        {
          static struct {
            char letter;
            unsigned int value;
          } table[] = { { '-', 0 },
                        { 'r', S_IRUSR }, { 'w', S_IWUSR }, { 'x', S_IXUSR },
                        { 'r', S_IRGRP }, { 'w', S_IWGRP }, { 'x', S_IXGRP },
                        { 'r', S_IROTH }, { 'w', S_IWOTH }, { 'x', S_IXOTH }};
          int idx;

          got_cmode = 1;
          modestr += 5;
          /* For now we only support a string as used by ls(1) and no
             octal numbers.  The first character must be a dash.  */
          for (idx=0; idx < 10 && *modestr; idx++, modestr++)
            {
              if (*modestr == table[idx].letter)
                cmode |= table[idx].value;
              else if (*modestr != '-')
                break;
            }
          if (*modestr && !strchr (" \t,", *modestr))
            {
              _set_errno (EINVAL);
              return -1;
            }
        }
      else if (!strncmp (modestr, "samethread", 10))
        {
          modestr += 10;
          if (*modestr && !strchr (" \t,", *modestr))
            {
              _set_errno (EINVAL);
              return -1;
            }
          *r_xmode |= X_SAMETHREAD;
        }
      else if (!strncmp (modestr, "nonblock", 8))
        {
          modestr += 8;
          if (*modestr && !strchr (" \t,", *modestr))
            {
              _set_errno (EINVAL);
              return -1;
            }
          oflags |= O_NONBLOCK;
#if HAVE_W32_SYSTEM
          /* Currently, nonblock implies pollable on Windows.  */
          *r_xmode |= X_POLLABLE;
#endif
        }
      else if (!strncmp (modestr, "sysopen", 7))
        {
          modestr += 7;
          if (*modestr && !strchr (" \t,", *modestr))
            {
              _set_errno (EINVAL);
              return -1;
            }
          *r_xmode |= X_SYSOPEN;
        }
      else if (!strncmp (modestr, "pollable", 8))
        {
          modestr += 8;
          if (*modestr && !strchr (" \t,", *modestr))
            {
              _set_errno (EINVAL);
              return -1;
            }
          *r_xmode |= X_POLLABLE;
        }
    }
  if (!got_cmode)
    cmode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

  *modeflags = (omode | oflags);
  if (r_cmode)
    *r_cmode = cmode;
  return 0;
}



/*
 * Low level stream functionality.
 */

static int
fill_stream (estream_t stream)
{
  size_t bytes_read = 0;
  int err;

  if (!stream->intern->func_read)
    {
      _set_errno (EOPNOTSUPP);
      err = -1;
    }
  else if (!stream->buffer_size)
    err = 0;
  else
    {
      gpgrt_cookie_read_function_t func_read = stream->intern->func_read;
      gpgrt_ssize_t ret;

      ret = (*func_read) (stream->intern->cookie,
			  stream->buffer, stream->buffer_size);
      if (ret == -1)
	{
	  bytes_read = 0;
	  err = -1;
#if EWOULDBLOCK != EAGAIN
          if (errno == EWOULDBLOCK)
            _set_errno (EAGAIN);
#endif
	}
      else
	{
	  bytes_read = ret;
	  err = 0;
	}
    }

  if (err)
    {
      if (errno != EAGAIN)
        {
          if (errno == EPIPE)
            stream->intern->indicators.hup = 1;
          stream->intern->indicators.err = 1;
        }
    }
  else if (!bytes_read)
    stream->intern->indicators.eof = 1;

  stream->intern->offset += stream->data_len;
  stream->data_len = bytes_read;
  stream->data_offset = 0;

  return err;
}

static int
flush_stream (estream_t stream)
{
  gpgrt_cookie_write_function_t func_write = stream->intern->func_write;
  int err;

  gpgrt_assert (stream->flags.writing);

  if (stream->data_offset)
    {
      size_t bytes_written;
      size_t data_flushed;
      gpgrt_ssize_t ret;

      if (! func_write)
	{
          _set_errno (EOPNOTSUPP);
          err = -1;
	  goto out;
	}

      /* Note: to prevent an endless loop caused by user-provided
	 write-functions that pretend to have written more bytes than
	 they were asked to write, we have to check for
	 "(stream->data_offset - data_flushed) > 0" instead of
	 "stream->data_offset - data_flushed".  */

      data_flushed = 0;
      err = 0;

      while ((((gpgrt_ssize_t) (stream->data_offset - data_flushed)) > 0)
             && !err)
	{
	  ret = (*func_write) (stream->intern->cookie,
			       stream->buffer + data_flushed,
			       stream->data_offset - data_flushed);
	  if (ret == -1)
	    {
	      bytes_written = 0;
	      err = -1;
#if EWOULDBLOCK != EAGAIN
              if (errno == EWOULDBLOCK)
                _set_errno (EAGAIN);
#endif
	    }
	  else
	    bytes_written = ret;

	  data_flushed += bytes_written;
	  if (err)
	    break;
	}

      stream->data_flushed += data_flushed;
      if (stream->data_offset == data_flushed)
	{
	  stream->intern->offset += stream->data_offset;
	  stream->data_offset = 0;
	  stream->data_flushed = 0;
	}
    }
  else
    err = 0;

  /* Always propagate flush event in case gpgrt_fflush was called
   * explictly to do flush buffers in caller's cookie functions.  */
  (*func_write) (stream->intern->cookie, NULL, 0);

 out:

  if (err && errno != EAGAIN)
    {
      if (errno == EPIPE)
        stream->intern->indicators.hup = 1;
      stream->intern->indicators.err = 1;
    }

  return err;
}


/*
 * Discard buffered data for STREAM.
 */
static void
es_empty (estream_t stream)
{
  gpgrt_assert (!stream->flags.writing);
  stream->data_len = 0;
  stream->data_offset = 0;
  stream->unread_data_len = 0;
}


/*
 * Initialize STREAM.
 */
static void
init_stream_obj (estream_t stream,
                 void *cookie, es_syshd_t *syshd,
                 gpgrt_stream_backend_kind_t kind,
                 struct cookie_io_functions_s functions,
                 unsigned int modeflags, unsigned int xmode)
{
  stream->intern->kind = kind;
  stream->intern->cookie = cookie;
  stream->intern->opaque = NULL;
  stream->intern->offset = 0;
  stream->intern->func_read = functions.public.func_read;
  stream->intern->func_write = functions.public.func_write;
  stream->intern->func_seek = functions.public.func_seek;
  stream->intern->func_ioctl = functions.func_ioctl;
  stream->intern->func_close = functions.public.func_close;
  stream->intern->strategy = _IOFBF;
  stream->intern->syshd = *syshd;
  stream->intern->print_ntotal = 0;
  stream->intern->indicators.err = 0;
  stream->intern->indicators.eof = 0;
  stream->intern->indicators.hup = 0;
  stream->intern->is_stdstream = 0;
  stream->intern->stdstream_fd = 0;
  stream->intern->deallocate_buffer = 0;
  stream->intern->printable_fname = NULL;
  stream->intern->printable_fname_inuse = 0;
  stream->intern->samethread = !! (xmode & X_SAMETHREAD);
  stream->intern->onclose = NULL;

  stream->data_len = 0;
  stream->data_offset = 0;
  stream->data_flushed = 0;
  stream->unread_data_len = 0;
  /* Depending on the modeflags we set whether we start in writing or
     reading mode.  This is required in case we are working on a
     stream which is not seeekable (like stdout).  Without this
     pre-initialization we would do a seek at the first write call and
     as this will fail no output will be delivered. */
  if ((modeflags & O_WRONLY) || (modeflags & O_RDWR) )
    stream->flags.writing = 1;
  else
    stream->flags.writing = 0;
}


/*
 * Deinitialize the STREAM object.  This does _not_ free the memory,
 * destroys the lock, or closes the underlying descriptor.
 */
static int
deinit_stream_obj (estream_t stream)
{
  gpgrt_cookie_close_function_t func_close;
  int err, tmp_err;

  trace (("enter: stream %p", stream));
  func_close = stream->intern->func_close;

  err = 0;
  if (stream->flags.writing)
    {
      tmp_err = flush_stream (stream);
      if (!err)
        err = tmp_err;
    }
  if (func_close)
    {
      trace (("stream %p calling func_close", stream));
      tmp_err = func_close (stream->intern->cookie);
      if (!err)
        err = tmp_err;
    }

  mem_free (stream->intern->printable_fname);
  stream->intern->printable_fname = NULL;
  stream->intern->printable_fname_inuse = 0;
  while (stream->intern->onclose)
    {
      notify_list_t tmp = stream->intern->onclose->next;
      mem_free (stream->intern->onclose);
      stream->intern->onclose = tmp;
    }

  trace_errno (err, ("leave: stream %p err=%d", stream, err));
  return err;
}


/*
 * Create a new stream and initialize it.  On success the new stream
 * handle is stored at R_STREAM.  On failure NULL is stored at
 * R_STREAM.
 */
static int
create_stream (estream_t *r_stream, void *cookie, es_syshd_t *syshd,
               gpgrt_stream_backend_kind_t kind,
               struct cookie_io_functions_s functions, unsigned int modeflags,
               unsigned int xmode, int with_locked_list)
{
  estream_internal_t stream_internal_new;
  estream_t stream_new;
  int err;
#if HAVE_W32_SYSTEM
  void *old_cookie = NULL;
#endif

  stream_new = NULL;
  stream_internal_new = NULL;

#if HAVE_W32_SYSTEM
  if ((xmode & X_POLLABLE) && kind != BACKEND_W32)
    {
      /* We require the W32 backend, because only that allows us to
       * write directly using the native W32 API and to disable the
       * system clamp.  Note that func_w32_create has already been
       * called with the flag to disable the system call clamp.  */
      _set_errno (EINVAL);
      err = -1;
      goto out;
    }
#endif /*HAVE_W32_SYSTEM*/

  stream_new = mem_alloc (sizeof (*stream_new));
  if (! stream_new)
    {
      err = -1;
      goto out;
    }

  stream_internal_new = mem_alloc (sizeof (*stream_internal_new));
  if (! stream_internal_new)
    {
      err = -1;
      goto out;
    }

  stream_new->buffer = stream_internal_new->buffer;
  stream_new->buffer_size = sizeof (stream_internal_new->buffer);
  stream_new->unread_buffer = stream_internal_new->unread_buffer;
  stream_new->unread_buffer_size = sizeof (stream_internal_new->unread_buffer);
  stream_new->intern = stream_internal_new;

#if HAVE_W32_SYSTEM
  if ((xmode & X_POLLABLE))
    {
      void *new_cookie;

      err = _gpgrt_w32_pollable_create (&new_cookie, modeflags,
                                        functions, cookie);
      if (err)
        goto out;

      modeflags &= ~O_NONBLOCK;
      old_cookie = cookie;
      cookie = new_cookie;
      kind = BACKEND_W32_POLLABLE;
      functions = _gpgrt_functions_w32_pollable;
    }
#endif /*HAVE_W32_SYSTEM*/

  init_stream_obj (stream_new, cookie, syshd, kind, functions, modeflags,
                   xmode);
  init_stream_lock (stream_new);

  err = do_list_add (stream_new, with_locked_list);
  if (err)
    goto out;

  *r_stream = stream_new;

 out:

  if (err)
    {
      trace_errno (err, ("leave: err=%d", err));
      if (stream_new)
	{
	  deinit_stream_obj (stream_new);
          destroy_stream_lock (stream_new);
	  mem_free (stream_new->intern);
	  mem_free (stream_new);
	}
    }
#if HAVE_W32_SYSTEM
  else if (old_cookie)
    trace (("leave: success stream=%p cookie=%p,%p",
            *r_stream, old_cookie, cookie));
#endif
  else
    trace (("leave: success stream=%p cookie=%p", *r_stream, cookie));

  return err;
}


/*
 * Deinitialize a stream object and destroy it.  With CANCEL_MODE set
 * try to cancel as much as possible (see _gpgrt_fcancel).
 */
static int
do_close (estream_t stream, int cancel_mode, int with_locked_list)
{
  int err;

  trace (("stream %p %s", stream, with_locked_list? "(with locked list)":""));

  if (stream)
    {
      do_list_remove (stream, with_locked_list);
      if (cancel_mode)
        {
          stream->flags.writing = 0;
          es_empty (stream);
        }
      while (stream->intern->onclose)
        {
          notify_list_t tmp = stream->intern->onclose->next;

          if (stream->intern->onclose->fnc)
            stream->intern->onclose->fnc (stream,
                                          stream->intern->onclose->fnc_value);
          mem_free (stream->intern->onclose);
          stream->intern->onclose = tmp;
        }
      err = deinit_stream_obj (stream);
      destroy_stream_lock (stream);
      if (stream->intern->deallocate_buffer)
        mem_free (stream->buffer);
      mem_free (stream->intern);
      mem_free (stream);
    }
  else
    err = 0;

  trace_errno (err, ("stream %p err=%d", stream, err));
  return err;
}


/*
 * The onclose worker function which is called with a locked
 * stream.
 */
static int
do_onclose (estream_t stream, int mode,
            void (*fnc) (estream_t, void*), void *fnc_value)
{
  notify_list_t item;

  if (!mode)
    {
      for (item = stream->intern->onclose; item; item = item->next)
        if (item->fnc && item->fnc == fnc && item->fnc_value == fnc_value)
          item->fnc = NULL; /* Disable this notification.  */
    }
  else
    {
      item = mem_alloc (sizeof *item);
      if (!item)
        return -1;
      item->fnc = fnc;
      item->fnc_value = fnc_value;
      item->next = stream->intern->onclose;
      stream->intern->onclose = item;
    }
  return 0;
}


/*
 * Try to read BYTES_TO_READ bytes from STREAM into BUFFER in
 * unbuffered-mode, storing the amount of bytes read at BYTES_READ.
 */
static int
do_read_nbf (estream_t _GPGRT__RESTRICT stream,
	     unsigned char *_GPGRT__RESTRICT buffer,
	     size_t bytes_to_read, size_t *_GPGRT__RESTRICT bytes_read)
{
  gpgrt_cookie_read_function_t func_read = stream->intern->func_read;
  size_t data_read;
  gpgrt_ssize_t ret;
  int err;

  data_read = 0;
  err = 0;

  while (bytes_to_read - data_read)
    {
      ret = (*func_read) (stream->intern->cookie,
			  buffer + data_read, bytes_to_read - data_read);
      if (ret == -1)
	{
	  err = -1;
#if EWOULDBLOCK != EAGAIN
          if (errno == EWOULDBLOCK)
            _set_errno (EAGAIN);
#endif
	  break;
	}
      else if (ret)
	data_read += ret;
      else
	break;
    }

  stream->intern->offset += data_read;
  *bytes_read = data_read;

  return err;
}


/*
 * Helper for check_pending.
 */
static int
check_pending_nbf (estream_t _GPGRT__RESTRICT stream)
{
  gpgrt_cookie_read_function_t func_read = stream->intern->func_read;
  char buffer[1];

  if (!(*func_read) (stream->intern->cookie, buffer, 0))
    return 1; /* Pending bytes.  */
  return 0; /* No pending bytes or error.  */
}


/*
 * Try to read BYTES_TO_READ bytes from STREAM into BUFFER in
 * fully-buffered-mode, storing the amount of bytes read at
 * BYTES_READ.
 */
static int
do_read_fbf (estream_t _GPGRT__RESTRICT stream,
	     unsigned char *_GPGRT__RESTRICT buffer,
	     size_t bytes_to_read, size_t *_GPGRT__RESTRICT bytes_read)
{
  size_t data_available;
  size_t data_to_read;
  size_t data_read;
  int err;

  data_read = 0;
  err = 0;

  while ((bytes_to_read - data_read) && (! err))
    {
      if (stream->data_offset == stream->data_len)
	{
	  /* Nothing more to read in current container, try to
	     fill container with new data.  */
	  err = fill_stream (stream);
	  if (! err)
	    if (! stream->data_len)
	      /* Filling did not result in any data read.  */
	      break;
	}

      if (! err)
	{
	  /* Filling resulted in some new data.  */

	  data_to_read = bytes_to_read - data_read;
	  data_available = stream->data_len - stream->data_offset;
	  if (data_to_read > data_available)
	    data_to_read = data_available;

	  memcpy (buffer + data_read,
		  stream->buffer + stream->data_offset, data_to_read);
	  stream->data_offset += data_to_read;
	  data_read += data_to_read;
	}
    }

  *bytes_read = data_read;

  return err;
}


/*
 * Helper for check_pending.
 */
static int
check_pending_fbf (estream_t _GPGRT__RESTRICT stream)
{
  gpgrt_cookie_read_function_t func_read = stream->intern->func_read;
  char buffer[1];

  if (stream->data_offset == stream->data_len)
    {
      /* Nothing more to read in current container, check whether it
         would be possible to fill the container with new data.  */
      if (!(*func_read) (stream->intern->cookie, buffer, 0))
        return 1; /* Pending bytes.  */
    }
  else
    return 1;
  return 0;
}


/*
 * Try to read BYTES_TO_READ bytes from STREAM into BUFFER in
 * line-buffered-mode, storing the amount of bytes read at BYTES_READ.
 */
static int
do_read_lbf (estream_t _GPGRT__RESTRICT stream,
	     unsigned char *_GPGRT__RESTRICT buffer,
	     size_t bytes_to_read, size_t *_GPGRT__RESTRICT bytes_read)
{
  int err;

  err = do_read_fbf (stream, buffer, bytes_to_read, bytes_read);

  return err;
}


/*
 * Try to read BYTES_TO_READ bytes from STREAM into BUFFER, storing
 * the amount of bytes read at BYTES_READ.
 */
static int
es_readn (estream_t _GPGRT__RESTRICT stream,
	  void *_GPGRT__RESTRICT buffer_arg,
	  size_t bytes_to_read, size_t *_GPGRT__RESTRICT bytes_read)
{
  unsigned char *buffer = (unsigned char *)buffer_arg;
  size_t data_read_unread, data_read;
  int err;

  data_read_unread = 0;
  data_read = 0;
  err = 0;

  if (stream->flags.writing)
    {
      /* Switching to reading mode -> flush output.  */
      err = flush_stream (stream);
      if (err)
	goto out;
      stream->flags.writing = 0;
    }

  /* Read unread data first.  */
  while ((bytes_to_read - data_read_unread) && stream->unread_data_len)
    {
      buffer[data_read_unread]
	= stream->unread_buffer[stream->unread_data_len - 1];
      stream->unread_data_len--;
      data_read_unread++;
    }

  switch (stream->intern->strategy)
    {
    case _IONBF:
      err = do_read_nbf (stream,
			 buffer + data_read_unread,
			 bytes_to_read - data_read_unread, &data_read);
      break;
    case _IOLBF:
      err = do_read_lbf (stream,
			 buffer + data_read_unread,
			 bytes_to_read - data_read_unread, &data_read);
      break;
    case _IOFBF:
      err = do_read_fbf (stream,
			 buffer + data_read_unread,
			 bytes_to_read - data_read_unread, &data_read);
      break;
    }

 out:

  if (bytes_read)
    *bytes_read = data_read_unread + data_read;

  return err;
}


/*
 * Return true if at least one byte is pending for read.  This is a
 * best effort check and it it possible that bytes are still pending
 * even if false is returned.  If the stream is in writing mode it is
 * switched to read mode.
 */
static int
check_pending (estream_t _GPGRT__RESTRICT stream)
{
  if (stream->flags.writing)
    {
      /* Switching to reading mode -> flush output.  */
      if (flush_stream (stream))
	return 0; /* Better return 0 on error.  */
      stream->flags.writing = 0;
    }

  /* Check unread data first.  */
  if (stream->unread_data_len)
    return 1;

  switch (stream->intern->strategy)
    {
    case _IONBF:
      return check_pending_nbf (stream);
    case _IOLBF:
    case _IOFBF:
      return check_pending_fbf (stream);
    }

  return 0;
}


/*
 * Try to unread DATA_N bytes from DATA into STREAM, storing the
 * amount of bytes successfully unread at BYTES_UNREAD.
 */
static void
es_unreadn (estream_t _GPGRT__RESTRICT stream,
	    const unsigned char *_GPGRT__RESTRICT data, size_t data_n,
	    size_t *_GPGRT__RESTRICT bytes_unread)
{
  size_t space_left;

  space_left = stream->unread_buffer_size - stream->unread_data_len;

  if (data_n > space_left)
    data_n = space_left;

  if (! data_n)
    goto out;

  memcpy (stream->unread_buffer + stream->unread_data_len, data, data_n);
  stream->unread_data_len += data_n;
  stream->intern->indicators.eof = 0;

 out:

  if (bytes_unread)
    *bytes_unread = data_n;
}


/*
 * Seek in STREAM.
 */
static int
es_seek (estream_t _GPGRT__RESTRICT stream, gpgrt_off_t offset, int whence,
	 gpgrt_off_t *_GPGRT__RESTRICT offset_new)
{
  gpgrt_cookie_seek_function_t func_seek = stream->intern->func_seek;
  int err, ret;
  gpgrt_off_t off;

  if (! func_seek)
    {
      _set_errno (EOPNOTSUPP);
      err = -1;
      goto out;
    }

  if (stream->flags.writing)
    {
      /* Flush data first in order to prevent flushing it to the wrong
	 offset.  */
      err = flush_stream (stream);
      if (err)
	goto out;
      stream->flags.writing = 0;
    }

  off = offset;
  if (whence == SEEK_CUR)
    {
      off = off - stream->data_len + stream->data_offset;
      off -= stream->unread_data_len;
    }

  ret = (*func_seek) (stream->intern->cookie, &off, whence);
  if (ret == -1)
    {
      err = -1;
#if EWOULDBLOCK != EAGAIN
      if (errno == EWOULDBLOCK)
        _set_errno (EAGAIN);
#endif
      goto out;
    }

  err = 0;
  es_empty (stream);

  if (offset_new)
    *offset_new = off;

  stream->intern->indicators.eof = 0;
  stream->intern->offset = off;

 out:

  if (err)
    {
      if (errno == EPIPE)
        stream->intern->indicators.hup = 1;
      stream->intern->indicators.err = 1;
    }

  return err;
}


/*
 * Write BYTES_TO_WRITE bytes from BUFFER into STREAM in
 * unbuffered-mode, storing the amount of bytes written at
 * BYTES_WRITTEN.
 */
static int
es_write_nbf (estream_t _GPGRT__RESTRICT stream,
	      const unsigned char *_GPGRT__RESTRICT buffer,
	      size_t bytes_to_write, size_t *_GPGRT__RESTRICT bytes_written)
{
  gpgrt_cookie_write_function_t func_write = stream->intern->func_write;
  size_t data_written;
  gpgrt_ssize_t ret;
  int err;

  if (bytes_to_write && (! func_write))
    {
      _set_errno (EOPNOTSUPP);
      err = -1;
      goto out;
    }

  data_written = 0;
  err = 0;

  while (bytes_to_write - data_written)
    {
      ret = (*func_write) (stream->intern->cookie,
			   buffer + data_written,
			   bytes_to_write - data_written);
      if (ret == -1)
	{
	  err = -1;
#if EWOULDBLOCK != EAGAIN
          if (errno == EWOULDBLOCK)
            _set_errno (EAGAIN);
#endif
	  break;
	}
      else
	data_written += ret;
    }

  stream->intern->offset += data_written;
  *bytes_written = data_written;

 out:

  return err;
}


/*
 * Write BYTES_TO_WRITE bytes from BUFFER into STREAM in
 * fully-buffered-mode, storing the amount of bytes written at
 * BYTES_WRITTEN.
 */
static int
es_write_fbf (estream_t _GPGRT__RESTRICT stream,
	      const unsigned char *_GPGRT__RESTRICT buffer,
	      size_t bytes_to_write, size_t *_GPGRT__RESTRICT bytes_written)
{
  size_t space_available;
  size_t data_to_write;
  size_t data_written;
  int err;

  data_written = 0;
  err = 0;

  while ((bytes_to_write - data_written) && (! err))
    {
      if (stream->data_offset == stream->buffer_size)
	/* Container full, flush buffer.  */
	err = flush_stream (stream);

      if (! err)
	{
	  /* Flushing resulted in empty container.  */

	  data_to_write = bytes_to_write - data_written;
	  space_available = stream->buffer_size - stream->data_offset;
	  if (data_to_write > space_available)
	    data_to_write = space_available;

	  memcpy (stream->buffer + stream->data_offset,
		  buffer + data_written, data_to_write);
	  stream->data_offset += data_to_write;
	  data_written += data_to_write;
	}
    }

  *bytes_written = data_written;

  return err;
}


/* Write BYTES_TO_WRITE bytes from BUFFER into STREAM in
   line-buffered-mode, storing the amount of bytes written in
   *BYTES_WRITTEN.  */
static int
es_write_lbf (estream_t _GPGRT__RESTRICT stream,
	      const unsigned char *_GPGRT__RESTRICT buffer,
	      size_t bytes_to_write, size_t *_GPGRT__RESTRICT bytes_written)
{
  size_t data_flushed = 0;
  size_t data_buffered = 0;
  unsigned char *nlp;
  int err = 0;

  nlp = memrchr (buffer, '\n', bytes_to_write);
  if (nlp)
    {
      /* Found a newline, directly write up to (including) this
	 character.  */
      err = flush_stream (stream);
      if (!err)
	err = es_write_nbf (stream, buffer, nlp - buffer + 1, &data_flushed);
    }

  if (!err)
    {
      /* Write remaining data fully buffered.  */
      err = es_write_fbf (stream, buffer + data_flushed,
			  bytes_to_write - data_flushed, &data_buffered);
    }

  *bytes_written = data_flushed + data_buffered;
  return err;
}


/* Write BYTES_TO_WRITE bytes from BUFFER into STREAM in, storing the
   amount of bytes written in BYTES_WRITTEN.  */
static int
es_writen (estream_t _GPGRT__RESTRICT stream,
	   const void *_GPGRT__RESTRICT buffer,
	   size_t bytes_to_write, size_t *_GPGRT__RESTRICT bytes_written)
{
  size_t data_written;
  int err;

  data_written = 0;
  err = 0;

  if (!stream->flags.writing)
    {
      /* Switching to writing mode -> discard input data and seek to
	 position at which reading has stopped.  We can do this only
	 if a seek function has been registered. */
      if (stream->intern->func_seek)
        {
          err = es_seek (stream, 0, SEEK_CUR, NULL);
          if (err)
            {
              if (errno == ESPIPE)
                err = 0;
              else
                goto out;
            }
          stream->flags.writing = 1;
        }
    }

  switch (stream->intern->strategy)
    {
    case _IONBF:
      err = es_write_nbf (stream, buffer, bytes_to_write, &data_written);
      break;

    case _IOLBF:
      err = es_write_lbf (stream, buffer, bytes_to_write, &data_written);
      break;

    case _IOFBF:
      err = es_write_fbf (stream, buffer, bytes_to_write, &data_written);
      break;
    }

 out:

  if (bytes_written)
    *bytes_written = data_written;

  return err;
}


static int
peek_stream (estream_t _GPGRT__RESTRICT stream,
             unsigned char **_GPGRT__RESTRICT data,
             size_t *_GPGRT__RESTRICT data_len)
{
  int err;

  if (stream->flags.writing)
    {
      /* Switching to reading mode -> flush output.  */
      err = flush_stream (stream);
      if (err)
	goto out;
      stream->flags.writing = 0;
    }

  if (stream->data_offset == stream->data_len)
    {
      /* Refill container.  */
      err = fill_stream (stream);
      if (err)
	goto out;
    }

  if (data)
    *data = stream->buffer + stream->data_offset;
  if (data_len)
    *data_len = stream->data_len - stream->data_offset;
  err = 0;

 out:

  return err;
}


/* Skip SIZE bytes of input data contained in buffer.  */
static int
skip_stream (estream_t stream, size_t size)
{
  int err;

  if (stream->data_offset + size > stream->data_len)
    {
      _set_errno (EINVAL);
      err = -1;
    }
  else
    {
      stream->data_offset += size;
      err = 0;
    }

  return err;
}


static int
doreadline (estream_t _GPGRT__RESTRICT stream, size_t max_length,
            char *_GPGRT__RESTRICT *_GPGRT__RESTRICT line,
            size_t *_GPGRT__RESTRICT line_length)
{
  size_t line_size;
  estream_t line_stream;
  char *line_new;
  void *line_stream_cookie;
  char *newline;
  unsigned char *data;
  size_t data_len;
  int err;
  es_syshd_t syshd;

  line_new = NULL;
  line_stream = NULL;
  line_stream_cookie = NULL;

  err = func_mem_create (&line_stream_cookie, NULL, 0, 0,
                         BUFFER_BLOCK_SIZE, 1,
                         mem_realloc, mem_free,
                         O_RDWR,
                         0);
  if (err)
    goto out;

  memset (&syshd, 0, sizeof syshd);
  err = create_stream (&line_stream, line_stream_cookie,
                       &syshd, BACKEND_MEM,
                       estream_functions_mem, O_RDWR, 1, 0);
  if (err)
    goto out;

  {
    size_t space_left = max_length;

    line_size = 0;
    for (;;)
      {
        if (max_length && (space_left == 1))
          break;

        err = peek_stream (stream, &data, &data_len);
        if (err || (! data_len))
          break;

        if (data_len > (space_left - 1))
          data_len = space_left - 1;

        newline = memchr (data, '\n', data_len);
        if (newline)
          {
            data_len = (newline - (char *) data) + 1;
            err = _gpgrt_write (line_stream, data, data_len, NULL);
            if (! err)
              {
                /* Not needed: space_left -= data_len */
                line_size += data_len;
                skip_stream (stream, data_len);
                break; /* endless loop */
              }
          }
        else
          {
            err = _gpgrt_write (line_stream, data, data_len, NULL);
            if (! err)
              {
                space_left -= data_len;
                line_size += data_len;
                skip_stream (stream, data_len);
              }
          }
        if (err)
          break;
      }
  }
  if (err)
    goto out;

  /* Complete line has been written to line_stream.  */

  if ((max_length > 1) && (! line_size))
    {
      stream->intern->indicators.eof = 1;
      goto out;
    }

  err = es_seek (line_stream, 0, SEEK_SET, NULL);
  if (err)
    goto out;

  if (! *line)
    {
      line_new = mem_alloc (line_size + 1);
      if (! line_new)
	{
	  err = -1;
	  goto out;
	}
    }
  else
    line_new = *line;

  err = _gpgrt_read (line_stream, line_new, line_size, NULL);
  if (err)
    goto out;

  line_new[line_size] = '\0';

  if (! *line)
    *line = line_new;
  if (line_length)
    *line_length = line_size;

 out:

  if (line_stream)
    do_close (line_stream, 0, 0);
  else if (line_stream_cookie)
    func_mem_destroy (line_stream_cookie);

  if (err)
    {
      if (! *line)
	mem_free (line_new);
      stream->intern->indicators.err = 1;
    }

  return err;
}


/* Output function used by estream_format.  */
static int
print_writer (void *outfncarg, const char *buf, size_t buflen)
{
  estream_t stream = outfncarg;
  size_t nwritten;
  int rc;

  nwritten = 0;
  rc = es_writen (stream, buf, buflen, &nwritten);
  stream->intern->print_ntotal += nwritten;
  return rc;
}


/* The core of our printf function.  This is called in locked state. */
static int
do_print_stream (estream_t _GPGRT__RESTRICT stream,
                 gpgrt_string_filter_t sf, void *sfvalue,
                 const char *_GPGRT__RESTRICT format, va_list ap)
{
  int rc;

  stream->intern->print_ntotal = 0;
  rc = _gpgrt_estream_format (print_writer, stream, sf, sfvalue, format, ap);
  if (rc)
    return -1;
  return (int)stream->intern->print_ntotal;
}


static int
es_set_buffering (estream_t _GPGRT__RESTRICT stream,
		  char *_GPGRT__RESTRICT buffer, int mode, size_t size)
{
  int err;

  /* Flush or empty buffer depending on mode.  */
  if (stream->flags.writing)
    {
      err = flush_stream (stream);
      if (err)
	goto out;
    }
  else
    es_empty (stream);

  stream->intern->indicators.eof = 0;

  /* Free old buffer in case that was allocated by this function.  */
  if (stream->intern->deallocate_buffer)
    {
      stream->intern->deallocate_buffer = 0;
      mem_free (stream->buffer);
      stream->buffer = NULL;
    }

  if (mode == _IONBF)
    stream->buffer_size = 0;
  else
    {
      void *buffer_new;

      if (buffer)
	buffer_new = buffer;
      else
	{
          if (!size)
            size = BUFSIZ;
	  buffer_new = mem_alloc (size);
	  if (! buffer_new)
	    {
	      err = -1;
	      goto out;
	    }
	}

      stream->buffer = buffer_new;
      stream->buffer_size = size;
      if (! buffer)
	stream->intern->deallocate_buffer = 1;
    }
  stream->intern->strategy = mode;
  err = 0;

 out:

  return err;
}


static gpgrt_off_t
es_offset_calculate (estream_t stream)
{
  gpgrt_off_t offset;

  offset = stream->intern->offset + stream->data_offset;
  if (offset < stream->unread_data_len)
    /* Offset undefined.  */
    offset = 0;
  else
    offset -= stream->unread_data_len;

  return offset;
}


static void
es_opaque_ctrl (estream_t _GPGRT__RESTRICT stream,
                void *_GPGRT__RESTRICT opaque_new,
		void **_GPGRT__RESTRICT opaque_old)
{
  if (opaque_old)
    *opaque_old = stream->intern->opaque;
  if (opaque_new)
    stream->intern->opaque = opaque_new;
}


/* API.  */

estream_t
_gpgrt_fopen (const char *_GPGRT__RESTRICT path,
              const char *_GPGRT__RESTRICT mode)
{
  unsigned int modeflags, cmode, xmode;
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  int err;
  struct cookie_io_functions_s *functions;
  es_syshd_t syshd;
  int kind;

  err = parse_mode (mode, &modeflags, &xmode, &cmode);
  if (err)
    goto leave;

  /* Convenience hack so that we can use /dev/null on Windows.  */
#ifdef HAVE_W32_SYSTEM
  if (path && !strcmp (path, "/dev/null"))
    path = "nul";
#endif

#ifdef HAVE_W32_SYSTEM
  if ((xmode & X_SYSOPEN))
    {
      kind = BACKEND_W32;
      functions = &estream_functions_w32;
      syshd.type = ES_SYSHD_HANDLE;
      err = func_file_create_w32 (&cookie, &syshd.u.handle,
                                  path, modeflags, cmode);
    }
  else
#endif /* W32 */
    {
      kind = BACKEND_FD;
      functions = &estream_functions_fd;
      syshd.type = ES_SYSHD_FD;
      err = func_file_create (&cookie, &syshd.u.fd,
                              path, modeflags, cmode);
    }
  if (err)
    goto leave;

  create_called = 1;
  err = create_stream (&stream, cookie, &syshd, kind,
                       *functions, modeflags, xmode, 0);
  if (err)
    goto leave;

  if (stream && path)
    fname_set_internal (stream, path, 1);

 leave:
  if (err && create_called)
    functions->public.func_close (cookie);

  return stream;
}



/* Create a new estream object in memory.  If DATA is not NULL this
   buffer will be used as the memory buffer; thus after this functions
   returns with the success the the memory at DATA belongs to the new
   estream.  The allocated length of DATA is given by DATA_LEN and its
   used length by DATA_N.  Usually this is malloced buffer; if a
   static buffer is provided, the caller must pass false for GROW and
   provide a dummy function for FUNC_FREE.  FUNC_FREE and FUNC_REALLOC
   allow the caller to provide custom functions for realloc and free
   to be used by the new estream object.  Note that the realloc
   function is also used for initial allocation.  If DATA is NULL a
   buffer is internally allocated; either using internal function or
   those provide by the caller.  It is an error to provide a realloc
   function but no free function.  Providing only a free function is
   allowed as long as GROW is false.  */
estream_t
_gpgrt_mopen (void *_GPGRT__RESTRICT data, size_t data_n, size_t data_len,
              unsigned int grow,
              func_realloc_t func_realloc, func_free_t func_free,
              const char *_GPGRT__RESTRICT mode)
{
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  unsigned int modeflags, xmode;
  int err;
  es_syshd_t syshd;

  err = parse_mode (mode, &modeflags, &xmode, NULL);
  if (err)
    goto out;

  err = func_mem_create (&cookie, data, data_n, data_len,
                         BUFFER_BLOCK_SIZE, grow,
                         func_realloc, func_free, modeflags, 0);
  if (err)
    goto out;

  memset (&syshd, 0, sizeof syshd);
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd, BACKEND_MEM,
                       estream_functions_mem, modeflags, xmode, 0);

 out:

  if (err && create_called)
    (*estream_functions_mem.public.func_close) (cookie);

  return stream;
}



estream_t
_gpgrt_fopenmem (size_t memlimit, const char *_GPGRT__RESTRICT mode)
{
  unsigned int modeflags, xmode;
  estream_t stream = NULL;
  void *cookie = NULL;
  es_syshd_t syshd;

  /* Memory streams are always read/write.  We use MODE only to get
     the append flag.  */
  if (parse_mode (mode, &modeflags, &xmode, NULL))
    return NULL;
  modeflags |= O_RDWR;

  if (func_mem_create (&cookie, NULL, 0, 0,
                       BUFFER_BLOCK_SIZE, 1,
                       mem_realloc, mem_free, modeflags,
                       memlimit))
    return NULL;

  memset (&syshd, 0, sizeof syshd);
  if (create_stream (&stream, cookie, &syshd, BACKEND_MEM,
                     estream_functions_mem, modeflags, xmode, 0))
    (*estream_functions_mem.public.func_close) (cookie);

  return stream;
}


/* This is the same as es_fopenmem but intializes the memory with a
   copy of (DATA,DATALEN).  The stream is initially set to the
   beginning.  If MEMLIMIT is not 0 but shorter than DATALEN it
   DATALEN will be used as the value for MEMLIMIT.  */
estream_t
_gpgrt_fopenmem_init (size_t memlimit, const char *_GPGRT__RESTRICT mode,
                      const void *data, size_t datalen)
{
  estream_t stream;

  if (memlimit && memlimit < datalen)
    memlimit = datalen;

  stream = _gpgrt_fopenmem (memlimit, mode);
  if (stream && data && datalen)
    {
      if (es_writen (stream, data, datalen, NULL))
        {
          int saveerrno = errno;
          _gpgrt_fclose (stream);
          stream = NULL;
          _set_errno (saveerrno);
        }
      else
        {
          es_seek (stream, 0L, SEEK_SET, NULL);
          stream->intern->indicators.eof = 0;
          stream->intern->indicators.err = 0;
        }
    }
  return stream;
}



estream_t
_gpgrt_fopencookie (void *_GPGRT__RESTRICT cookie,
                    const char *_GPGRT__RESTRICT mode,
                    gpgrt_cookie_io_functions_t functions)
{
  unsigned int modeflags, xmode;
  estream_t stream;
  int err;
  es_syshd_t syshd;
  struct cookie_io_functions_s io_functions = { functions, NULL };

  stream = NULL;
  modeflags = 0;

  err = parse_mode (mode, &modeflags, &xmode, NULL);
  if (err)
    goto out;

  memset (&syshd, 0, sizeof syshd);
  err = create_stream (&stream, cookie, &syshd, BACKEND_USER, io_functions,
                       modeflags, xmode, 0);
  if (err)
    goto out;

 out:
  return stream;
}



static estream_t
do_fdopen (int filedes, const char *mode, int no_close, int with_locked_list)
{
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  unsigned int modeflags, xmode;
  int err;
  es_syshd_t syshd;

  err = parse_mode (mode, &modeflags, &xmode, NULL);
  if (err)
    goto out;
  if ((xmode & X_SYSOPEN))
    {
      /* Not allowed for fdopen.  */
      _set_errno (EINVAL);
      err = -1;
      goto out;
    }

  err = func_fd_create (&cookie, filedes, modeflags, no_close);
  if (err)
    goto out;

  syshd.type = ES_SYSHD_FD;
  syshd.u.fd = filedes;
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd,
                       BACKEND_FD, estream_functions_fd,
                       modeflags, xmode, with_locked_list);

  if (!err && stream)
    {
      if ((modeflags & O_NONBLOCK))
        err = stream->intern->func_ioctl (cookie, COOKIE_IOCTL_NONBLOCK,
                                          "", NULL);
    }

 out:
  if (err && create_called)
    (*estream_functions_fd.public.func_close) (cookie);

  return stream;
}

estream_t
_gpgrt_fdopen (int filedes, const char *mode)
{
  return do_fdopen (filedes, mode, 0, 0);
}

/* A variant of es_fdopen which does not close FILEDES at the end.  */
estream_t
_gpgrt_fdopen_nc (int filedes, const char *mode)
{
  return do_fdopen (filedes, mode, 1, 0);
}



static estream_t
do_fpopen (FILE *fp, const char *mode, int no_close, int with_locked_list)
{
  unsigned int modeflags, cmode, xmode;
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  int err;
  es_syshd_t syshd;

  err = parse_mode (mode, &modeflags, &xmode, &cmode);
  if (err)
    goto out;
  if ((xmode & X_SYSOPEN))
    {
      /* Not allowed for fpopen.  */
      _set_errno (EINVAL);
      err = -1;
      goto out;
    }

  if (fp)
    fflush (fp);
  err = func_fp_create (&cookie, fp, modeflags, no_close);
  if (err)
    goto out;

  syshd.type = ES_SYSHD_FD;
  syshd.u.fd = fp? fileno (fp): -1;
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd,
                       BACKEND_FP, estream_functions_fp,
                       modeflags, xmode, with_locked_list);

 out:
  if (err && create_called)
    (*estream_functions_fp.public.func_close) (cookie);

  return stream;
}


/* Create an estream from the stdio stream FP.  This mechanism is
   useful in case the stdio streams have special properties and may
   not be mixed with fd based functions.  This is for example the case
   under Windows where the 3 standard streams are associated with the
   console whereas a duped and fd-opened stream of one of this stream
   won't be associated with the console.  As this messes things up it
   is easier to keep on using the standard I/O stream as a backend for
   estream. */
estream_t
_gpgrt_fpopen (FILE *fp, const char *mode)
{
  return do_fpopen (fp, mode, 0, 0);
}


/* Same as es_fpopen but does not close  FP at the end.  */
estream_t
_gpgrt_fpopen_nc (FILE *fp, const char *mode)
{
  return do_fpopen (fp, mode, 1, 0);
}



#ifdef HAVE_W32_SYSTEM
static estream_t
do_sockopen (SOCKET sock, const char *mode, int no_close, int with_locked_list)
{
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  unsigned int modeflags, xmode;
  int err;
  es_syshd_t syshd;

  err = parse_mode (mode, &modeflags, &xmode, NULL);
  if (err)
    goto out;
  if ((xmode & X_SYSOPEN))
    {
      /* Not allowed for sockopen.  */
      _set_errno (EINVAL);
      err = -1;
      goto out;
    }

  err = func_sock_create (&cookie, sock, modeflags, no_close);
  if (err)
    goto out;

  syshd.type = ES_SYSHD_SOCK;
  syshd.u.sock = sock;
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd,
                       BACKEND_SOCK, estream_functions_sock,
                       modeflags, xmode, with_locked_list);

  if (!err && stream)
    {
      if ((modeflags & O_NONBLOCK))
        err = stream->intern->func_ioctl (cookie, COOKIE_IOCTL_NONBLOCK,
                                          "", NULL);
    }

 out:
  if (err && create_called)
    (*estream_functions_sock.public.func_close) (cookie);

  return stream;
}


estream_t
do_w32open (HANDLE hd, const char *mode,
            int no_close, int with_locked_list)
{
  unsigned int modeflags, cmode, xmode;
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  int err;
  es_syshd_t syshd;

  /* For obvious reasons we ignore sysmode here.  */
  err = parse_mode (mode, &modeflags, &xmode, &cmode);
  if (err)
    goto leave;

  /* If we are pollable we create the function cookie with syscall
   * clamp disabled.  This is because functions are called from
   * separate reader and writer threads in w32-stream.  */
  err = func_w32_create (&cookie, hd, modeflags,
                         no_close, !!(xmode & X_POLLABLE));
  if (err)
    goto leave;

  syshd.type = ES_SYSHD_HANDLE;
  syshd.u.handle = hd;
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd,
                       BACKEND_W32, estream_functions_w32,
                       modeflags, xmode, with_locked_list);

 leave:
  if (err && create_called)
    (*estream_functions_w32.public.func_close) (cookie);

  return stream;
}
#endif /*HAVE_W32_SYSTEM*/

static estream_t
do_sysopen (es_syshd_t *syshd, const char *mode, int no_close)
{
  estream_t stream;

  switch (syshd->type)
    {
    case ES_SYSHD_FD:
#ifndef HAVE_W32_SYSTEM
    case ES_SYSHD_SOCK:
#endif
      stream = do_fdopen (syshd->u.fd, mode, no_close, 0);
      break;

#ifdef HAVE_W32_SYSTEM
    case ES_SYSHD_SOCK:
      stream = do_sockopen (syshd->u.sock, mode, no_close, 0);
      break;
    case ES_SYSHD_HANDLE:
      stream = do_w32open (syshd->u.handle, mode, no_close, 0);
      break;
#endif

    /* FIXME: Support RVIDs under Wince?  */

    default:
      _set_errno (EINVAL);
      stream = NULL;
    }
  return stream;
}

/* On POSIX systems this function is an alias for es_fdopen.  Under
   Windows it uses the bare W32 API and thus a HANDLE instead of a
   file descriptor.  */
estream_t
_gpgrt_sysopen (es_syshd_t *syshd, const char *mode)
{
  return do_sysopen (syshd, mode, 0);
}

/* Same as es_sysopen but the handle/fd will not be closed by
   es_fclose.  */
estream_t
_gpgrt_sysopen_nc (es_syshd_t *syshd, const char *mode)
{
  return do_sysopen (syshd, mode, 1);
}



/* Set custom standard descriptors to be used for stdin, stdout and
   stderr.  This function needs to be called before any of the
   standard streams are accessed.  This internal version uses a double
   dash inside its name. */
void
_gpgrt__set_std_fd (int no, int fd)
{
  /* fprintf (stderr, "es_set_std_fd(%d, %d)\n", no, fd); */
  lock_list ();
  if (no >= 0 && no < 3 && !custom_std_fds_valid[no])
    {
      custom_std_fds[no] = fd;
      custom_std_fds_valid[no] = 1;
    }
  unlock_list ();
}


/* Return the stream used for stdin, stdout or stderr.
   This internal version uses a double dash inside its name. */
estream_t
_gpgrt__get_std_stream (int fd)
{
  estream_list_t list_obj;
  estream_t stream = NULL;

  fd %= 3; /* We only allow 0, 1 or 2 but we don't want to return an error. */

  lock_list ();

  for (list_obj = estream_list; list_obj; list_obj = list_obj->next)
    if (list_obj->stream && list_obj->stream->intern->is_stdstream
        && list_obj->stream->intern->stdstream_fd == fd)
      {
	stream = list_obj->stream;
	break;
      }
  if (!stream)
    {
      /* Standard stream not yet created.  We first try to create them
         from registered file descriptors.  */
      if (!fd && custom_std_fds_valid[0])
        stream = do_fdopen (custom_std_fds[0], "r", 1, 1);
      else if (fd == 1 && custom_std_fds_valid[1])
        stream = do_fdopen (custom_std_fds[1], "a", 1, 1);
      else if (custom_std_fds_valid[2])
        stream = do_fdopen (custom_std_fds[2], "a", 1, 1);

      if (!stream)
        {
          /* Second try is to use the standard C streams.  */
          if (!fd)
            stream = do_fpopen (stdin, "r", 1, 1);
          else if (fd == 1)
            stream = do_fpopen (stdout, "a", 1, 1);
          else
            stream = do_fpopen (stderr, "a", 1, 1);
        }

      if (!stream)
        {
          /* Last try: Create a bit bucket.  */
          stream = do_fpopen (NULL, fd? "a":"r", 0, 1);
          if (!stream)
            {
              fprintf (stderr, "fatal: error creating a dummy estream"
                       " for %d: %s\n", fd, strerror (errno));
              _gpgrt_abort();
            }
        }

      stream->intern->is_stdstream = 1;
      stream->intern->stdstream_fd = fd;
      if (fd == 2)
        es_set_buffering (stream, NULL, _IOLBF, 0);
      fname_set_internal (stream,
                          fd == 0? "[stdin]" :
                          fd == 1? "[stdout]" : "[stderr]", 0);
    }

  unlock_list ();
  return stream;
}

/* Note: A "samethread" keyword given in "mode" is ignored and the
 * value used by STREAM is used instead.  Note that this function is
 * the reasons why some of the init and deinit code is split up into
 * several functions.  */
estream_t
_gpgrt_freopen (const char *_GPGRT__RESTRICT path,
                const char *_GPGRT__RESTRICT mode,
                estream_t _GPGRT__RESTRICT stream)
{
  int err;

  if (path)
    {
      unsigned int modeflags, cmode, xmode, dummy;
      int create_called;
      void *cookie;
      int fd;
      es_syshd_t syshd;

      cookie = NULL;
      create_called = 0;

  /* Convenience hack so that we can use /dev/null on Windows.  */
#ifdef HAVE_W32_SYSTEM
      if (!strcmp (path, "/dev/null"))
        path = "nul";
#endif

      xmode = stream->intern->samethread ? X_SAMETHREAD : 0;

      lock_stream (stream);

      deinit_stream_obj (stream);

      err = parse_mode (mode, &modeflags, &dummy, &cmode);
      if (err)
	goto leave;
      (void)dummy;

      err = func_file_create (&cookie, &fd, path, modeflags, cmode);
      if (err)
	goto leave;

      syshd.type = ES_SYSHD_FD;
      syshd.u.fd = fd;
      create_called = 1;
      init_stream_obj (stream, cookie, &syshd, BACKEND_FD,
                       estream_functions_fd, modeflags, xmode);

    leave:

      if (err)
	{
	  if (create_called)
	    func_fd_destroy (cookie);

	  do_close (stream, 0, 0);
	  stream = NULL;
	}
      else
        {
          if (path)
            fname_set_internal (stream, path, 1);
          unlock_stream (stream);
        }
    }
  else
    {
      /* FIXME?  We don't support re-opening at the moment.  */
      _set_errno (EINVAL);
      deinit_stream_obj (stream);
      do_close (stream, 0, 0);
      stream = NULL;
    }

  return stream;
}


int
_gpgrt_fclose (estream_t stream)
{
  int err;

  err = do_close (stream, 0, 0);

  return err;
}


/* gpgrt_fcancel does the same as gpgrt_fclose but tries to avoid
 * flushing out any data still held in internal buffers.  It may or
 * may not remove a new file created for that stream by the open
 * function.  */
int
_gpgrt_fcancel (estream_t stream)
{
  int err;

  err = do_close (stream, 1, 0);

  return err;
}


/* This is a special version of es_fclose which can be used with
   es_fopenmem to return the memory buffer.  This is feature is useful
   to write to a memory buffer using estream.  Note that the function
   does not close the stream if the stream does not support snatching
   the buffer.  On error NULL is stored at R_BUFFER.  Note that if no
   write operation has happened, NULL may also be stored at BUFFER on
   success.  The caller needs to release the returned memory using
   gpgrt_free.  */
int
_gpgrt_fclose_snatch (estream_t stream, void **r_buffer, size_t *r_buflen)
{
  int err;

  /* Note: There is no need to lock the stream in a close call.  The
     object will be destroyed after the close and thus any other
     contender for the lock would work on a closed stream.  */

  if (r_buffer)
    {
      cookie_ioctl_function_t func_ioctl = stream->intern->func_ioctl;
      size_t buflen;

      *r_buffer = NULL;

      if (!func_ioctl)
        {
          _set_errno (EOPNOTSUPP);
          err = -1;
          goto leave;
        }

      if (stream->flags.writing)
        {
          err = flush_stream (stream);
          if (err)
            goto leave;
          stream->flags.writing = 0;
        }

      err = func_ioctl (stream->intern->cookie, COOKIE_IOCTL_SNATCH_BUFFER,
                        r_buffer, &buflen);
      if (err)
        goto leave;
      if (r_buflen)
        *r_buflen = buflen;
    }

  err = do_close (stream, 0, 0);

 leave:
  if (err && r_buffer)
    {
      mem_free (*r_buffer);
      *r_buffer = NULL;
    }
  return err;
}


/* Register or unregister a close notification function for STREAM.
   FNC is the function to call and FNC_VALUE the value passed as
   second argument.  To register the notification the value for MODE
   must be 1.  If mode is 0 the function tries to remove or disable an
   already registered notification; for this to work the value of FNC
   and FNC_VALUE must be the same as with the registration and
   FNC_VALUE must be a unique value.  No error will be returned if
   MODE is 0.

   FIXME: I think the next comment is not anymore correct:
   Unregister should only be used in the error case because it may not
   be able to remove memory internally allocated for the onclose
   handler.

   FIXME: Unregister is not thread safe.

   The notification will be called right before the stream is
   closed. If gpgrt_fcancel is used, the cancellation of internal
   buffers is done before the notifications.  The notification handler
   may not call any estream function for STREAM, neither direct nor
   indirectly. */
int
_gpgrt_onclose (estream_t stream, int mode,
                void (*fnc) (estream_t, void*), void *fnc_value)
{
  int err;

  lock_stream (stream);
  err = do_onclose (stream, mode, fnc, fnc_value);
  unlock_stream (stream);

  return err;
}


int
_gpgrt_fileno_unlocked (estream_t stream)
{
  es_syshd_t syshd;

  if (_gpgrt_syshd_unlocked (stream, &syshd))
    return -1;
  switch (syshd.type)
    {
    case ES_SYSHD_FD:   return syshd.u.fd;
    case ES_SYSHD_SOCK: return syshd.u.sock;
    default:
      _set_errno (EINVAL);
      return -1;
    }
}


/* Return the handle of a stream which has been opened by es_sysopen.
   The caller needs to pass a structure which will be filled with the
   sys handle.  Return 0 on success or true on error and sets errno.
   This is the unlocked version.  */
int
_gpgrt_syshd_unlocked (estream_t stream, es_syshd_t *syshd)
{
  if (!stream || !syshd || stream->intern->syshd.type == ES_SYSHD_NONE)
    {
      if (syshd)
        syshd->type = ES_SYSHD_NONE;
      _set_errno (EINVAL);
      return -1;
    }

  *syshd = stream->intern->syshd;
  return 0;
}


void
_gpgrt_flockfile (estream_t stream)
{
  lock_stream (stream);
}


int
_gpgrt_ftrylockfile (estream_t stream)
{
  return trylock_stream (stream);
}


void
_gpgrt_funlockfile (estream_t stream)
{
  unlock_stream (stream);
}


int
_gpgrt_fileno (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_fileno_unlocked (stream);
  unlock_stream (stream);

  return ret;
}


/* Return the handle of a stream which has been opened by es_sysopen.
   The caller needs to pass a structure which will be filled with the
   sys handle.  Return 0 on success or true on error and sets errno.
   This is the unlocked version.  */
int
_gpgrt_syshd (estream_t stream, es_syshd_t *syshd)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_syshd_unlocked (stream, syshd);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt__pending_unlocked (estream_t stream)
{
  return check_pending (stream);
}


/* Return true if there is at least one byte pending for read on
   STREAM.  This does only work if the backend supports checking for
   pending bytes and is thus mostly useful with cookie based backends.

   Note that if this function is used with cookie based functions, the
   read cookie may be called with 0 for the SIZE argument.  If bytes
   are pending the function is expected to return -1 in this case and
   thus deviates from the standard behavior of read(2).   */
int
_gpgrt__pending (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt__pending_unlocked (stream);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt_feof_unlocked (estream_t stream)
{
  return stream->intern->indicators.eof;
}


int
_gpgrt_feof (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_feof_unlocked (stream);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt_ferror_unlocked (estream_t stream)
{
  return stream->intern->indicators.err;
}


int
_gpgrt_ferror (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_ferror_unlocked (stream);
  unlock_stream (stream);

  return ret;
}


void
_gpgrt_clearerr_unlocked (estream_t stream)
{
  stream->intern->indicators.eof = 0;
  stream->intern->indicators.err = 0;
  /* We do not reset the HUP indicator because there is no way to
     get out of this state.  */
}


void
_gpgrt_clearerr (estream_t stream)
{
  lock_stream (stream);
  _gpgrt_clearerr_unlocked (stream);
  unlock_stream (stream);
}


static int
do_fflush (estream_t stream)
{
  int err;

  if (stream->flags.writing)
    err = flush_stream (stream);
  else
    {
      es_empty (stream);
      err = 0;
    }

  return err;
}


int
_gpgrt_fflush (estream_t stream)
{
  int err;

  if (stream)
    {
      lock_stream (stream);
      err = do_fflush (stream);
      unlock_stream (stream);
    }
  else
    {
      estream_list_t item;

      err = 0;
      lock_list ();
      for (item = estream_list; item; item = item->next)
        if (item->stream)
          {
            lock_stream (item->stream);
            err |= do_fflush (item->stream);
            unlock_stream (item->stream);
          }
      unlock_list ();
    }
  return err ? EOF : 0;
}


int
_gpgrt_fseek (estream_t stream, long int offset, int whence)
{
  int err;

  lock_stream (stream);
  err = es_seek (stream, offset, whence, NULL);
  unlock_stream (stream);

  return err;
}


int
_gpgrt_fseeko (estream_t stream, gpgrt_off_t offset, int whence)
{
  int err;

  lock_stream (stream);
  err = es_seek (stream, offset, whence, NULL);
  unlock_stream (stream);

  return err;
}


long int
_gpgrt_ftell (estream_t stream)
{
  long int ret;

  lock_stream (stream);
  ret = es_offset_calculate (stream);
  unlock_stream (stream);

  return ret;
}


gpgrt_off_t
_gpgrt_ftello (estream_t stream)
{
  gpgrt_off_t ret = -1;

  lock_stream (stream);
  ret = es_offset_calculate (stream);
  unlock_stream (stream);

  return ret;
}


void
_gpgrt_rewind (estream_t stream)
{
  lock_stream (stream);
  es_seek (stream, 0L, SEEK_SET, NULL);
  /* Note that es_seek already cleared the EOF flag.  */
  stream->intern->indicators.err = 0;
  unlock_stream (stream);
}


int
_gpgrt_ftruncate (estream_t stream, gpgrt_off_t length)
{
  cookie_ioctl_function_t func_ioctl;
  int ret;

  lock_stream (stream);
  func_ioctl = stream->intern->func_ioctl;
  if (!func_ioctl)
    {
      _set_errno (EOPNOTSUPP);
      ret = -1;
    }
  else
    {
      ret = func_ioctl (stream->intern->cookie, COOKIE_IOCTL_TRUNCATE,
                        &length, NULL);
    }
  unlock_stream (stream);
  return ret;
}


int
_gpgrt__getc_underflow (estream_t stream)
{
  int err;
  unsigned char c;
  size_t bytes_read;

  err = es_readn (stream, &c, 1, &bytes_read);

  return (err || (! bytes_read)) ? EOF : c;
}


int
_gpgrt__putc_overflow (int c, estream_t stream)
{
  unsigned char d = c;
  int err;

  err = es_writen (stream, &d, 1, NULL);

  return err ? EOF : c;
}


int
_gpgrt_fgetc (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_getc_unlocked (stream);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt_fputc (int c, estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = _gpgrt_putc_unlocked (c, stream);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt_ungetc (int c, estream_t stream)
{
  unsigned char data = (unsigned char) c;
  size_t data_unread;

  lock_stream (stream);
  es_unreadn (stream, &data, 1, &data_unread);
  unlock_stream (stream);

  return data_unread ? c : EOF;
}


int
_gpgrt_read (estream_t _GPGRT__RESTRICT stream,
             void *_GPGRT__RESTRICT buffer, size_t bytes_to_read,
             size_t *_GPGRT__RESTRICT bytes_read)
{
  int err;

  if (bytes_to_read)
    {
      lock_stream (stream);
      err = es_readn (stream, buffer, bytes_to_read, bytes_read);
      unlock_stream (stream);
    }
  else
    err = 0;

  return err;
}


int
_gpgrt_write (estream_t _GPGRT__RESTRICT stream,
              const void *_GPGRT__RESTRICT buffer, size_t bytes_to_write,
              size_t *_GPGRT__RESTRICT bytes_written)
{
  int err;

  if (bytes_to_write)
    {
      lock_stream (stream);
      err = es_writen (stream, buffer, bytes_to_write, bytes_written);
      unlock_stream (stream);
    }
  else
    err = 0;

  return err;
}


size_t
_gpgrt_fread (void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
              estream_t _GPGRT__RESTRICT stream)
{
  size_t ret, bytes;

  if (size && nitems)
    {
      lock_stream (stream);
      es_readn (stream, ptr, size * nitems, &bytes);
      unlock_stream (stream);

      ret = bytes / size;
    }
  else
    ret = 0;

  return ret;
}


size_t
_gpgrt_fwrite (const void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
               estream_t _GPGRT__RESTRICT stream)
{
  size_t ret, bytes;

  if (size && nitems)
    {
      lock_stream (stream);
      es_writen (stream, ptr, size * nitems, &bytes);
      unlock_stream (stream);

      ret = bytes / size;
    }
  else
    ret = 0;

  return ret;
}


char *
_gpgrt_fgets (char *_GPGRT__RESTRICT buffer, int length,
              estream_t _GPGRT__RESTRICT stream)
{
  unsigned char *s = (unsigned char*)buffer;
  int c;

  if (!length)
    return NULL;

  c = EOF;
  lock_stream (stream);
  while (length > 1 && (c = _gpgrt_getc_unlocked (stream)) != EOF && c != '\n')
    {
      *s++ = c;
      length--;
    }
  unlock_stream (stream);

  if (c == EOF && s == (unsigned char*)buffer)
    return NULL; /* Nothing read.  */

  if (c != EOF && length > 1)
    *s++ = c;

  *s = 0;
  return buffer;
}


int
_gpgrt_fputs_unlocked (const char *_GPGRT__RESTRICT s,
                       estream_t _GPGRT__RESTRICT stream)
{
  size_t length;
  int err;

  length = strlen (s);
  err = es_writen (stream, s, length, NULL);
  return err ? EOF : 0;
}

int
_gpgrt_fputs (const char *_GPGRT__RESTRICT s, estream_t _GPGRT__RESTRICT stream)
{
  size_t length;
  int err;

  length = strlen (s);
  lock_stream (stream);
  err = es_writen (stream, s, length, NULL);
  unlock_stream (stream);

  return err ? EOF : 0;
}


gpgrt_ssize_t
_gpgrt_getline (char *_GPGRT__RESTRICT *_GPGRT__RESTRICT lineptr,
                size_t *_GPGRT__RESTRICT n, estream_t _GPGRT__RESTRICT stream)
{
  char *line = NULL;
  size_t line_n = 0;
  int err;

  lock_stream (stream);
  err = doreadline (stream, 0, &line, &line_n);
  unlock_stream (stream);
  if (err)
    goto out;

  if (*n)
    {
      /* Caller wants us to use his buffer.  */

      if (*n < (line_n + 1))
	{
	  /* Provided buffer is too small -> resize.  */

	  void *p;

	  p = mem_realloc (*lineptr, line_n + 1);
	  if (! p)
	    err = -1;
	  else
	    {
	      if (*lineptr != p)
		*lineptr = p;
	    }
	}

      if (! err)
	{
	  memcpy (*lineptr, line, line_n + 1);
	  if (*n != line_n)
	    *n = line_n;
	}
      mem_free (line);
    }
  else
    {
      /* Caller wants new buffers.  */
      *lineptr = line;
      *n = line_n;
    }

 out:

  return err ? err : (gpgrt_ssize_t)line_n;
}



/* Same as fgets() but if the provided buffer is too short a larger
   one will be allocated.  This is similar to getline. A line is
   considered a byte stream ending in a LF.

   If MAX_LENGTH is not NULL, it shall point to a value with the
   maximum allowed allocation.

   Returns the length of the line. EOF is indicated by a line of
   length zero. A truncated line is indicated my setting the value at
   MAX_LENGTH to 0.  If the returned value is less then 0 not enough
   memory was available or another error occurred; ERRNO is then set
   accordingly.

   If a line has been truncated, the file pointer is moved forward to
   the end of the line so that the next read starts with the next
   line.  Note that MAX_LENGTH must be re-initialized in this case.

   The caller initially needs to provide the address of a variable,
   initialized to NULL, at ADDR_OF_BUFFER and don't change this value
   anymore with the following invocations.  LENGTH_OF_BUFFER should be
   the address of a variable, initialized to 0, which is also
   maintained by this function.  Thus, both paramaters should be
   considered the state of this function.

   Note: The returned buffer is allocated with enough extra space to
   allow the caller to append a CR,LF,Nul.  The buffer should be
   released using gpgrt_free.
 */
gpgrt_ssize_t
_gpgrt_read_line (estream_t stream,
                  char **addr_of_buffer, size_t *length_of_buffer,
                  size_t *max_length)
{
  int c;
  char  *buffer = *addr_of_buffer;
  size_t length = *length_of_buffer;
  size_t nbytes = 0;
  size_t maxlen = max_length? *max_length : 0;
  char *p;

  if (!buffer)
    {
      /* No buffer given - allocate a new one. */
      length = 256;
      buffer = mem_alloc (length);
      *addr_of_buffer = buffer;
      if (!buffer)
        {
          *length_of_buffer = 0;
          if (max_length)
            *max_length = 0;
          return -1;
        }
      *length_of_buffer = length;
    }

  if (length < 4)
    {
      /* This should never happen. If it does, the function has been
         called with wrong arguments. */
      _set_errno (EINVAL);
      return -1;
    }
  length -= 3; /* Reserve 3 bytes for CR,LF,EOL. */

  lock_stream (stream);
  p = buffer;
  while  ((c = _gpgrt_getc_unlocked (stream)) != EOF)
    {
      if (nbytes == length)
        {
          /* Enlarge the buffer. */
          if (maxlen && length > maxlen)
            {
              /* We are beyond our limit: Skip the rest of the line. */
              while (c != '\n' && (c=_gpgrt_getc_unlocked (stream)) != EOF)
                ;
              *p++ = '\n'; /* Always append a LF (we reserved some space). */
              nbytes++;
              if (max_length)
                *max_length = 0; /* Indicate truncation. */
              break; /* the while loop. */
            }
          length += 3; /* Adjust for the reserved bytes. */
          length += length < 1024? 256 : 1024;
          *addr_of_buffer = mem_realloc (buffer, length);
          if (!*addr_of_buffer)
            {
              int save_errno = errno;
              mem_free (buffer);
              *length_of_buffer = 0;
              if (max_length)
                *max_length = 0;
              unlock_stream (stream);
              _set_errno (save_errno);
              return -1;
            }
          buffer = *addr_of_buffer;
          *length_of_buffer = length;
          length -= 3;
          p = buffer + nbytes;
	}
      *p++ = c;
      nbytes++;
      if (c == '\n')
        break;
    }
  *p = 0; /* Make sure the line is a string. */
  unlock_stream (stream);

  return nbytes;
}

/* Wrapper around free() to match the memory allocation system used by
   estream.  Should be used for all buffers returned to the caller by
   libestream.  If a custom allocation handler has been set with
   gpgrt_set_alloc_func that register function may be used
   instead.  This function has been moved to init.c.  */
/* void */
/* _gpgrt_free (void *a) */
/* { */
/*   mem_free (a); */
/* } */


int
_gpgrt_vfprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                          gpgrt_string_filter_t sf, void *sfvalue,
                          const char *_GPGRT__RESTRICT format,
                          va_list ap)
{
  return do_print_stream (stream, sf, sfvalue, format, ap);
}


int
_gpgrt_vfprintf (estream_t _GPGRT__RESTRICT stream,
                 gpgrt_string_filter_t sf, void *sfvalue,
                 const char *_GPGRT__RESTRICT format,
                 va_list ap)
{
  int ret;

  lock_stream (stream);
  ret = do_print_stream (stream, sf, sfvalue, format, ap);
  unlock_stream (stream);

  return ret;
}


int
_gpgrt_fprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                         const char *_GPGRT__RESTRICT format, ...)
{
  int ret;

  va_list ap;
  va_start (ap, format);
  ret = do_print_stream (stream, NULL, NULL, format, ap);
  va_end (ap);

  return ret;
}


int
_gpgrt_fprintf (estream_t _GPGRT__RESTRICT stream,
                const char *_GPGRT__RESTRICT format, ...)
{
  int ret;

  va_list ap;
  va_start (ap, format);
  lock_stream (stream);
  ret = do_print_stream (stream, NULL, NULL, format, ap);
  unlock_stream (stream);
  va_end (ap);

  return ret;
}


static int
tmpfd (void)
{
#ifdef HAVE_W32_SYSTEM
  int attempts, n;
  char buffer[MAX_PATH+9+12+1];
# define mystrlen(a) strlen (a)
  char *name, *p;
  HANDLE file;
  int pid = GetCurrentProcessId ();
  unsigned int value;
  int i;

  n = GetTempPath (MAX_PATH+1, buffer);
  if (!n || n > MAX_PATH || mystrlen (buffer) > MAX_PATH)
    {
      _set_errno (ENOENT);
      return -1;
    }
  p = buffer + mystrlen (buffer);
  strcpy (p, "_estream");
  p += 8;
  /* We try to create the directory but don't care about an error as
     it may already exist and the CreateFile would throw an error
     anyway.  */
  CreateDirectory (buffer, NULL);
  *p++ = '\\';
  name = p;
  for (attempts=0; attempts < 10; attempts++)
    {
      p = name;
      value = (GetTickCount () ^ ((pid<<16) & 0xffff0000));
      for (i=0; i < 8; i++)
        {
          *p++ = tohex (((value >> 28) & 0x0f));
          value <<= 4;
        }
      strcpy (p, ".tmp");
      file = CreateFile (buffer,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_NEW,
                         FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                         NULL);
      if (file != INVALID_HANDLE_VALUE)
        {
          int fd = _open_osfhandle ((intptr_t)file, 0);
          if (fd == -1)
            {
              CloseHandle (file);
              return -1;
            }
          return fd;
        }
      Sleep (1); /* One ms as this is the granularity of GetTickCount.  */
    }
  _set_errno (ENOENT);
  return -1;
#else /*!HAVE_W32_SYSTEM*/
  FILE *fp;
  int fp_fd;
  int fd;

  fp = NULL;
  fd = -1;

  fp = tmpfile ();
  if (! fp)
    goto out;

  fp_fd = fileno (fp);
  fd = dup (fp_fd);

 out:

  if (fp)
    fclose (fp);

  return fd;
#endif /*!HAVE_W32_SYSTEM*/
}

estream_t
_gpgrt_tmpfile (void)
{
  unsigned int modeflags;
  int create_called = 0;
  estream_t stream = NULL;
  void *cookie = NULL;
  int err;
  int fd;
  es_syshd_t syshd;

  modeflags = O_RDWR | O_TRUNC | O_CREAT;

  fd = tmpfd ();
  if (fd == -1)
    {
      err = -1;
      goto out;
    }

  err = func_fd_create (&cookie, fd, modeflags, 0);
  if (err)
    goto out;

  syshd.type = ES_SYSHD_FD;
  syshd.u.fd = fd;
  create_called = 1;
  err = create_stream (&stream, cookie, &syshd,
                       BACKEND_FD, estream_functions_fd,
                       modeflags, 0, 0);

 out:
  if (err)
    {
      if (create_called)
	func_fd_destroy (cookie);
      else if (fd != -1)
	close (fd);
      stream = NULL;
    }

  return stream;
}


int
_gpgrt_setvbuf (estream_t _GPGRT__RESTRICT stream,
                char *_GPGRT__RESTRICT buf, int type, size_t size)
{
  int err;

  if ((type == _IOFBF || type == _IOLBF || type == _IONBF)
      && (!buf || size || type == _IONBF))
    {
      lock_stream (stream);
      err = es_set_buffering (stream, buf, type, size);
      unlock_stream (stream);
    }
  else
    {
      _set_errno (EINVAL);
      err = -1;
    }

  return err;
}


/* Put a stream into binary mode.  This is only needed for the
   standard streams if they are to be used in a binary way.  On Unix
   systems it is never needed but MSDOS based systems require such a
   call.  It needs to be called before any I/O is done on STREAM.  */
void
_gpgrt_set_binary (estream_t stream)
{
  lock_stream (stream);
  if (!(stream->intern->modeflags & O_BINARY))
    {
      stream->intern->modeflags |= O_BINARY;
#ifdef HAVE_DOSISH_SYSTEM
      if (stream->intern->func_read == func_fd_read)
        {
          estream_cookie_fd_t fd_cookie = stream->intern->cookie;

          if (!IS_INVALID_FD (fd_cookie->fd))
            setmode (fd_cookie->fd, O_BINARY);
        }
      else if (stream->intern->func_read == func_fp_read)
        {
          estream_cookie_fp_t fp_cookie = stream->intern->cookie;

          if (fp_cookie->fp)
            setmode (fileno (fp_cookie->fp), O_BINARY);
        }
#endif
    }
  unlock_stream (stream);
}


/* Set non-blocking mode for STREAM.  Use true for ONOFF to enable and
   false to disable non-blocking mode.  Returns 0 on success or -1 on
   error and sets ERRNO.  Note that not all backends support
   non-blocking mode.

   In non-blocking mode a system call will not block but return an
   error and set errno to EAGAIN.  The estream API always uses EAGAIN
   and not EWOULDBLOCK.  If a buffered function like es_fgetc() or
   es_fgets() returns an error and both, feof() and ferror() return
   false the caller may assume that the error condition was EAGAIN.

   Switching back from non-blocking to blocking may raise problems
   with buffering, thus care should be taken.  Although read+write
   sockets are supported in theory, switching from write to read may
   result into problems because estream may first flush the write
   buffers and there is no way to handle that non-blocking (EAGAIN)
   case.  Explicit flushing should thus be done before before
   switching to read.  */
int
_gpgrt_set_nonblock (estream_t stream, int onoff)
{
  cookie_ioctl_function_t func_ioctl;
  int ret;

  lock_stream (stream);
  func_ioctl = stream->intern->func_ioctl;
  if (!func_ioctl)
    {
      _set_errno (EOPNOTSUPP);
      ret = -1;
    }
  else
    {
      unsigned int save_flags = stream->intern->modeflags;

      if (onoff)
        stream->intern->modeflags |= O_NONBLOCK;
      else
        stream->intern->modeflags &= ~O_NONBLOCK;

      ret = func_ioctl (stream->intern->cookie, COOKIE_IOCTL_NONBLOCK,
                        onoff?"":NULL, NULL);
      if (ret)
        stream->intern->modeflags = save_flags;
    }
  unlock_stream (stream);
  return ret;
}


/* Return true if STREAM is in non-blocking mode.  */
int
_gpgrt_get_nonblock (estream_t stream)
{
  int ret;

  lock_stream (stream);
  ret = !!(stream->intern->modeflags & O_NONBLOCK);
  unlock_stream (stream);
  return ret;
}


/* A version of poll(2) working on estream handles.  Note that not all
   estream types work with this function.  In contrast to the standard
   poll function the gpgrt_poll_t object uses a set of bit flags
   instead of the EVENTS and REVENTS members.  An item with the IGNORE
   flag set is entirely ignored.  The TIMEOUT values is given in
   milliseconds, a value of -1 waits indefinitely, and a value of 0
   returns immediately.

   A positive return value gives the number of fds with new
   information.  A return value of 0 indicates a timeout and -1
   indicates an error in which case ERRNO is set.  */
int
_gpgrt_poll (gpgrt_poll_t *fds, unsigned int nfds, int timeout)
{
  gpgrt_poll_t *item;
  int count = 0;
  int idx;
#ifndef HAVE_W32_SYSTEM
# ifdef HAVE_POLL_H
  struct pollfd *poll_fds = NULL;
  nfds_t poll_nfds;
# else
  fd_set readfds, writefds, exceptfds;
  int any_readfd, any_writefd, any_exceptfd;
  int max_fd;
#endif
  int fd, ret, any;
#endif /*HAVE_W32_SYSTEM*/

  trace (("enter: nfds=%u timeout=%d", nfds, timeout));

  if (!fds)
    {
      _set_errno (EINVAL);
      count = -1;
      goto leave;
    }

  /* Clear all response fields (even for ignored items).  */
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      item->got_read = 0;
      item->got_write = 0;
      item->got_oob = 0;
      item->got_rdhup = 0;
      item->got_err = 0;
      item->got_hup = 0;
      item->got_nval = 0;
    }

  /* Check for pending reads.  */
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      if (!item->want_read)
        continue;
      if (_gpgrt__pending (item->stream))
        {
          item->got_read = 1;
          count++;
        }
    }

  /* Check for space in the write buffers.  */
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      if (!item->want_write)
        continue;
      /* FIXME */
    }

  if (count)
    goto leave;

  /* Now do the real select.  */
#ifdef HAVE_W32_SYSTEM

  _gpgrt_pre_syscall ();
  count = _gpgrt_w32_poll (fds, nfds, timeout);
  _gpgrt_post_syscall ();

#else /*!HAVE_W32_SYSTEM*/
# ifdef HAVE_POLL_H
  poll_fds = xtrymalloc (sizeof (*poll_fds)*nfds);
  if (!poll_fds)
    {
      count = -1;
      goto leave;
    }
  poll_nfds = 0;
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      fd = _gpgrt_fileno (item->stream);
      if (fd == -1)
        continue;  /* Stream does not support polling.  */

      if (item->want_read || item->want_write || item->want_oob)
        {
          poll_fds[poll_nfds].fd = fd;
          poll_fds[poll_nfds].events = ((item->want_read ? POLLIN : 0)
                                        |(item->want_write ? POLLOUT : 0)
                                        |(item->want_oob ? POLLPRI : 0));
          poll_fds[poll_nfds].revents = 0;
          poll_nfds++;
        }
    }

  _gpgrt_pre_syscall ();
  do
    ret = poll (poll_fds, poll_nfds, timeout);
  while (ret == -1 && (errno == EINTR || errno == EAGAIN));
  _gpgrt_post_syscall ();
# else /* !HAVE_POLL_H */
  any_readfd = any_writefd = any_exceptfd = 0;
  max_fd = 0;
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      fd = _gpgrt_fileno (item->stream);
      if (fd == -1)
        continue;  /* Stream does not support polling.  */

      if (item->want_read)
        {
          if (!any_readfd)
            {
              FD_ZERO (&readfds);
              any_readfd = 1;
            }
          FD_SET (fd, &readfds);
          if (fd > max_fd)
            max_fd = fd;
        }
      if (item->want_write)
        {
          if (!any_writefd)
            {
              FD_ZERO (&writefds);
              any_writefd = 1;
            }
          FD_SET (fd, &writefds);
          if (fd > max_fd)
            max_fd = fd;
        }
      if (item->want_oob)
        {
          if (!any_exceptfd)
            {
              FD_ZERO (&exceptfds);
              any_exceptfd = 1;
            }
          FD_SET (fd, &exceptfds);
          if (fd > max_fd)
            max_fd = fd;
        }
    }

  _gpgrt_pre_syscall ();
  do
    {
      struct timeval timeout_val;

      timeout_val.tv_sec = timeout / 1000;
      timeout_val.tv_usec = (timeout % 1000) * 1000;
      ret = select (max_fd+1,
                    any_readfd?   &readfds   : NULL,
                    any_writefd?  &writefds  : NULL,
                    any_exceptfd? &exceptfds : NULL,
                    timeout == -1 ? NULL : &timeout_val);
    }
  while (ret == -1 && errno == EINTR);
  _gpgrt_post_syscall ();
# endif

  if (ret == -1)
    {
# ifdef HAVE_POLL_H
      trace_errno (1, ("poll failed: "));
# else
      trace_errno (1, ("select failed: "));
# endif
      count = -1;
      goto leave;
    }
  if (!ret)
    {
      /* Timeout.  Note that in this case we can't return got_err for
       * an invalid stream.  */
      count = 0;
      goto leave;
    }

# ifdef HAVE_POLL_H
  poll_nfds = 0;
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      fd = _gpgrt_fileno (item->stream);
      if (fd == -1)
        {
          item->got_err = 1;  /* Stream does not support polling.  */
          count++;
          continue;
        }

      any = 0;
      if (item->stream->intern->indicators.hup)
        {
          item->got_hup = 1;
          any = 1;
        }
      if (item->want_read && (poll_fds[poll_nfds].revents & (POLLIN|POLLHUP)))
        {
          item->got_read = 1;
          any = 1;
        }
      if (item->want_write && (poll_fds[poll_nfds].revents & POLLOUT))
        {
          item->got_write = 1;
          any = 1;
        }
      if (item->want_oob && (poll_fds[poll_nfds].revents & ~(POLLIN|POLLOUT)))
        {
          item->got_oob = 1;
          any = 1;
        }

      if (item->want_read || item->want_write || item->want_oob)
        poll_nfds++;
      if (any)
        count++;
    }
# else
  for (item = fds, idx = 0; idx < nfds; item++, idx++)
    {
      if (item->ignore)
        continue;
      fd = _gpgrt_fileno (item->stream);
      if (fd == -1)
        {
          item->got_err = 1;  /* Stream does not support polling.  */
          count++;
          continue;
        }

      any = 0;
      if (item->stream->intern->indicators.hup)
        {
          item->got_hup = 1;
          any = 1;
        }
      if (item->want_read && FD_ISSET (fd, &readfds))
        {
          item->got_read = 1;
          any = 1;
        }
      if (item->want_write && FD_ISSET (fd, &writefds))
        {
          item->got_write = 1;
          any = 1;
        }
      if (item->want_oob && FD_ISSET (fd, &exceptfds))
        {
          item->got_oob = 1;
          any = 1;
        }

      if (any)
        count++;
    }
# endif
#endif /*!HAVE_W32_SYSTEM*/

 leave:
#ifndef HAVE_W32_SYSTEM
# ifdef HAVE_POLL_H
  xfree (poll_fds);
# endif
#endif
#ifdef ENABLE_TRACING
  trace (("leave: count=%d", count));
  if (count > 0)
    {
      for (item = fds, idx = 0; idx < nfds; item++, idx++)
        {
          trace (("     %3d  %c%c%c%c%c  %c%c%c%c%c%c%c",
                  idx,
                  fds[idx].want_read?  'r':'-',
                  fds[idx].want_write? 'w':'-',
                  fds[idx].want_oob?   'o':'-',
                  fds[idx].want_rdhup? 'h':'-',
                  fds[idx].ignore?     'i':'-',
                  fds[idx].got_read?   'r':'-',
                  fds[idx].got_write?  'w':'-',
                  fds[idx].got_oob?    'o':'-',
                  fds[idx].got_rdhup?  'h':'-',
                  fds[idx].got_hup?    'H':'-',
                  fds[idx].got_err?    'e':'-',
                  fds[idx].got_nval?   'n':'-'
                  ));
        }
    }
#endif /*ENABLE_TRACING*/
  return count;
}


void
_gpgrt_opaque_set (estream_t stream, void *opaque)
{
  lock_stream (stream);
  es_opaque_ctrl (stream, opaque, NULL);
  unlock_stream (stream);
}


void *
_gpgrt_opaque_get (estream_t stream)
{
  void *opaque;

  lock_stream (stream);
  es_opaque_ctrl (stream, NULL, &opaque);
  unlock_stream (stream);

  return opaque;
}


static void
fname_set_internal (estream_t stream, const char *fname, int quote)
{
  if (stream->intern->printable_fname
      && !stream->intern->printable_fname_inuse)
    {
      mem_free (stream->intern->printable_fname);
      stream->intern->printable_fname = NULL;
    }
  if (stream->intern->printable_fname)
    return; /* Can't change because it is in use.  */

  if (*fname != '[')
    quote = 0;
  else
    quote = !!quote;

  stream->intern->printable_fname = mem_alloc (strlen (fname) + quote + 1);
  if (quote)
    stream->intern->printable_fname[0] = '\\';
  strcpy (stream->intern->printable_fname+quote, fname);
}


/* Set the filename attribute of STREAM.  There is no error return.
   as long as STREAM is valid.  This function is called internally by
   functions which open a filename.  */
void
_gpgrt_fname_set (estream_t stream, const char *fname)
{
  if (fname)
    {
      lock_stream (stream);
      fname_set_internal (stream, fname, 1);
      unlock_stream (stream);
    }
}


/* Return the filename attribute of STREAM.  In case no filename has
   been set, "[?]" will be returned.  The returned file name is valid
   as long as STREAM is valid.  */
const char *
_gpgrt_fname_get (estream_t stream)
{
  const char *fname;

  lock_stream (stream);
  fname = stream->intern->printable_fname;
  if (fname)
    stream->intern->printable_fname_inuse = 1;
  unlock_stream (stream);
  if (!fname)
    fname = "[?]";
  return fname;
}



/* Print a BUFFER to STREAM while replacing all control characters and
   the characters in DELIMITERS by standard C escape sequences.
   Returns 0 on success or -1 on error.  If BYTES_WRITTEN is not NULL
   the number of bytes actually written are stored at this
   address.  */
int
_gpgrt_write_sanitized (estream_t _GPGRT__RESTRICT stream,
                        const void * _GPGRT__RESTRICT buffer, size_t length,
                        const char * delimiters,
                        size_t * _GPGRT__RESTRICT bytes_written)
{
  const unsigned char *p = buffer;
  size_t count = 0;
  int ret;

  lock_stream (stream);
  for (; length; length--, p++, count++)
    {
      if (*p < 0x20
          || *p == 0x7f
          || (delimiters
              && (strchr (delimiters, *p) || *p == '\\')))
        {
          _gpgrt_putc_unlocked ('\\', stream);
          count++;
          if (*p == '\n')
            {
              _gpgrt_putc_unlocked ('n', stream);
              count++;
            }
          else if (*p == '\r')
            {
              _gpgrt_putc_unlocked ('r', stream);
              count++;
            }
          else if (*p == '\f')
            {
              _gpgrt_putc_unlocked ('f', stream);
              count++;
            }
          else if (*p == '\v')
            {
              _gpgrt_putc_unlocked ('v', stream);
              count++;
            }
          else if (*p == '\b')
            {
              _gpgrt_putc_unlocked ('b', stream);
              count++;
            }
          else if (!*p)
            {
              _gpgrt_putc_unlocked('0', stream);
              count++;
            }
          else
            {
              _gpgrt_fprintf_unlocked (stream, "x%02x", *p);
              count += 3;
            }
	}
      else
        {
          _gpgrt_putc_unlocked (*p, stream);
          count++;
        }
    }

  if (bytes_written)
    *bytes_written = count;
  ret =  _gpgrt_ferror_unlocked (stream)? -1 : 0;
  unlock_stream (stream);

  return ret;
}


/* Write LENGTH bytes of BUFFER to STREAM as a hex encoded string.
   RESERVED must be 0.  Returns 0 on success or -1 on error.  If
   BYTES_WRITTEN is not NULL the number of bytes actually written are
   stored at this address.  */
int
_gpgrt_write_hexstring (estream_t _GPGRT__RESTRICT stream,
                        const void *_GPGRT__RESTRICT buffer, size_t length,
                        int reserved, size_t *_GPGRT__RESTRICT bytes_written )
{
  int ret;
  const unsigned char *s;
  size_t count = 0;

  (void)reserved;

#define tohex(n) ((n) < 10 ? ((n) + '0') : (((n) - 10) + 'A'))

  if (!length)
    return 0;

  lock_stream (stream);

  for (s = buffer; length; s++, length--)
    {
      _gpgrt_putc_unlocked ( tohex ((*s>>4)&15), stream);
      _gpgrt_putc_unlocked ( tohex (*s&15), stream);
      count += 2;
    }

  if (bytes_written)
    *bytes_written = count;
  ret = _gpgrt_ferror_unlocked (stream)? -1 : 0;

  unlock_stream (stream);

  return ret;

#undef tohex
}
