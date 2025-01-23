/* w32-estream.c - es_poll support on W32.
 * Copyright (C) 2000 Werner Koch (dd9jn)
 * Copyright (C) 2001, 2002, 2003, 2004, 2007, 2010, 2016 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

/*
 * This file is based on GPGME's w32-io.c started in 2001.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <io.h>
#include <windows.h>

#ifndef EOPNOTSUPP
# define EOPNOTSUPP ENOSYS
#endif

/* Enable tracing.  The value is the module name to be printed.  */
/*#define ENABLE_TRACING "estream" */

#include "gpgrt-int.h"

/*
 * In order to support es_poll on Windows, we create a proxy shim that
 * we use as the estream I/O functions.  This shim creates reader and
 * writer threads that use the original I/O functions.
 */


/* Calculate array dimension.  */
#ifndef DIM
#define DIM(array) (sizeof (array) / sizeof (*array))
#endif

#define READBUF_SIZE 8192
#define WRITEBUF_SIZE 8192


typedef struct estream_cookie_w32_pollable *estream_cookie_w32_pollable_t;

struct reader_context_s
{
  estream_cookie_w32_pollable_t pcookie;
  HANDLE thread_hd;

  CRITICAL_SECTION mutex;

  int stop_me;
  int eof;
  int eof_shortcut;
  int error;
  int error_code;

  /* This is manually reset.  */
  HANDLE have_data_ev;
  /* This is automatically reset.  */
  HANDLE have_space_ev;
  /* This is manually reset but actually only triggered once.  */
  HANDLE close_ev;

  size_t readpos, writepos;
  char buffer[READBUF_SIZE];
};

struct writer_context_s
{
  estream_cookie_w32_pollable_t pcookie;
  HANDLE thread_hd;

  CRITICAL_SECTION mutex;

  int stop_me;
  int error;
  int error_code;

  /* This is manually reset.  */
  HANDLE have_data;
  HANDLE is_empty;
  HANDLE close_ev;
  size_t nbytes;
  char buffer[WRITEBUF_SIZE];
};

/* Cookie for pollable objects.  */
struct estream_cookie_w32_pollable
{
  unsigned int modeflags;

  struct cookie_io_functions_s next_functions;
  void *next_cookie;

  struct reader_context_s *reader;
  struct writer_context_s *writer;
};


static DWORD CALLBACK
reader (void *arg)
{
  struct reader_context_s *ctx = arg;
  int nbytes;
  ssize_t nread;

  trace (("%p: reader starting", ctx));

  for (;;)
    {
      EnterCriticalSection (&ctx->mutex);
      /* Leave a 1 byte gap so that we can see whether it is empty or
	 full.  */
      while ((ctx->writepos + 1) % READBUF_SIZE == ctx->readpos)
	{
	  /* Wait for space.  */
	  if (!ResetEvent (ctx->have_space_ev))
	    trace (("%p: ResetEvent failed: ec=%d", ctx, (int)GetLastError()));
          LeaveCriticalSection (&ctx->mutex);
          trace (("%p: waiting for space", ctx));
	  WaitForSingleObject (ctx->have_space_ev, INFINITE);
	  trace (("%p: got space", ctx));
          EnterCriticalSection (&ctx->mutex);
        }
      gpgrt_assert (((ctx->writepos + 1) % READBUF_SIZE != ctx->readpos));
      if (ctx->stop_me)
	{
          LeaveCriticalSection (&ctx->mutex);
	  break;
        }
      nbytes = (ctx->readpos + READBUF_SIZE
		- ctx->writepos - 1) % READBUF_SIZE;
      gpgrt_assert (nbytes);
      if (nbytes > READBUF_SIZE - ctx->writepos)
	nbytes = READBUF_SIZE - ctx->writepos;
      LeaveCriticalSection (&ctx->mutex);

      trace (("%p: reading up to %d bytes", ctx, nbytes));

      nread = ctx->pcookie->next_functions.public.func_read
        (ctx->pcookie->next_cookie, ctx->buffer + ctx->writepos, nbytes);
      trace (("%p: got %d bytes", ctx, nread));
      if (nread < 0)
        {
          ctx->error_code = (int) errno;
          if (ctx->error_code == ERROR_BROKEN_PIPE)
            {
              ctx->eof = 1;
              trace (("%p: got EOF (broken pipe)", ctx));
            }
          else
            {
              ctx->error = 1;
              trace (("%p: read error: ec=%d", ctx, ctx->error_code));
            }
          break;
        }

      EnterCriticalSection (&ctx->mutex);
      if (ctx->stop_me)
	{
          LeaveCriticalSection (&ctx->mutex);
	  break;
        }
      if (!nread)
	{
	  ctx->eof = 1;
	  trace (("%p: got eof", ctx));
          LeaveCriticalSection (&ctx->mutex);
	  break;
        }

      ctx->writepos = (ctx->writepos + nread) % READBUF_SIZE;
      if (!SetEvent (ctx->have_data_ev))
	trace (("%p: SetEvent (%p) failed: ec=%d",
                ctx, ctx->have_data_ev, (int)GetLastError ()));
      LeaveCriticalSection (&ctx->mutex);
    }
  /* Indicate that we have an error or EOF.  */
  if (!SetEvent (ctx->have_data_ev))
    trace (("%p: SetEvent (%p) failed: ec=%d",
            ctx, ctx->have_data_ev, (int)GetLastError ()));

  trace (("%p: waiting for close", ctx));
  WaitForSingleObject (ctx->close_ev, INFINITE);

  CloseHandle (ctx->close_ev);
  CloseHandle (ctx->have_data_ev);
  CloseHandle (ctx->have_space_ev);
  CloseHandle (ctx->thread_hd);
  DeleteCriticalSection (&ctx->mutex);
  free (ctx);  /* Standard free!  See comment in create_reader. */

  return 0;
}


static struct reader_context_s *
create_reader (estream_cookie_w32_pollable_t pcookie)
{
  struct reader_context_s *ctx;
  SECURITY_ATTRIBUTES sec_attr;
  DWORD tid;

  memset (&sec_attr, 0, sizeof sec_attr);
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  /* The CTX must be allocated in standard system memory so that we
   * won't use any custom allocation handler which may use our lock
   * primitives for its implementation.  The problem here is that the
   * syscall clamp mechanism (e.g. nPth) would be called recursively:
   * 1. For example by the caller of _gpgrt_w32_poll and 2. by
   * gpgrt_lock_lock on behalf of the the custom allocation and free
   * functions.  */
  ctx = calloc (1, sizeof *ctx);
  if (!ctx)
    {
      return NULL;
    }

  ctx->pcookie = pcookie;

  ctx->have_data_ev = CreateEvent (&sec_attr, TRUE, FALSE, NULL);
  if (ctx->have_data_ev)
    ctx->have_space_ev = CreateEvent (&sec_attr, FALSE, TRUE, NULL);
  if (ctx->have_space_ev)
    ctx->close_ev = CreateEvent (&sec_attr, TRUE, FALSE, NULL);
  if (!ctx->have_data_ev || !ctx->have_space_ev || !ctx->close_ev)
    {
      trace (("%p: CreateEvent failed: ec=%d", ctx, (int)GetLastError ()));
      if (ctx->have_data_ev)
	CloseHandle (ctx->have_data_ev);
      if (ctx->have_space_ev)
	CloseHandle (ctx->have_space_ev);
      if (ctx->close_ev)
	CloseHandle (ctx->close_ev);
      _gpgrt_free (ctx);
      return NULL;
    }

  InitializeCriticalSection (&ctx->mutex);

  ctx->thread_hd = CreateThread (&sec_attr, 0, reader, ctx, 0, &tid);

  if (!ctx->thread_hd)
    {
      trace (("%p: CreateThread failed: ec=%d", ctx, (int)GetLastError ()));
      DeleteCriticalSection (&ctx->mutex);
      if (ctx->have_data_ev)
	CloseHandle (ctx->have_data_ev);
      if (ctx->have_space_ev)
	CloseHandle (ctx->have_space_ev);
      if (ctx->close_ev)
	CloseHandle (ctx->close_ev);
      _gpgrt_free (ctx);
      return NULL;
    }
  else
    {
#if 0
      /* We set the priority of the thread higher because we know that
         it only runs for a short time.  This greatly helps to
         increase the performance of the I/O.  */
      SetThreadPriority (ctx->thread_hd, get_desired_thread_priority ());
#endif
    }

  return ctx;
}


/* Prepare destruction of the reader thread for CTX.  Returns 0 if a
   call to this function is sufficient and destroy_reader_finish shall
   not be called.  */
static void
destroy_reader (struct reader_context_s *ctx)
{
  EnterCriticalSection (&ctx->mutex);
  ctx->stop_me = 1;
  if (ctx->have_space_ev)
    SetEvent (ctx->have_space_ev);
  LeaveCriticalSection (&ctx->mutex);

  /* XXX is it feasible to unblock the thread?  */

  /* After setting this event CTX is void. */
  SetEvent (ctx->close_ev);
}


/*
 * Read function for pollable objects.
 */
static gpgrt_ssize_t
func_w32_pollable_read (void *cookie, void *buffer, size_t count)
{
  estream_cookie_w32_pollable_t pcookie = cookie;
  gpgrt_ssize_t nread;
  struct reader_context_s *ctx;

  trace (("%p: enter buffer=%p count=%u", cookie, buffer, count));

  /* FIXME: implement pending check if COUNT==0 */

  ctx = pcookie->reader;
  if (ctx == NULL)
    {
      pcookie->reader = ctx = create_reader (pcookie);
      if (!ctx)
        {
          _gpg_err_set_errno (EBADF);
          nread = -1;
          goto leave;
        }
      trace (("%p: new reader %p", cookie, pcookie->reader));
    }

  if (ctx->eof_shortcut)
    {
      nread = 0;
      goto leave;
    }

  EnterCriticalSection (&ctx->mutex);
  trace (("%p: readpos: %d, writepos %d", cookie, ctx->readpos, ctx->writepos));
  if (ctx->readpos == ctx->writepos && !ctx->error)
    {
      /* No data available.  */
      int eof = ctx->eof;

      LeaveCriticalSection (&ctx->mutex);

      if (pcookie->modeflags & O_NONBLOCK && ! eof)
        {
          _gpg_err_set_errno (EAGAIN);
          nread = -1;
          goto leave;
        }

      trace (("%p: waiting for data", cookie));
      WaitForSingleObject (ctx->have_data_ev, INFINITE);
      trace (("%p: data available", cookie));
      EnterCriticalSection (&ctx->mutex);
    }

  if (ctx->readpos == ctx->writepos || ctx->error)
    {
      LeaveCriticalSection (&ctx->mutex);
      ctx->eof_shortcut = 1;
      if (ctx->eof)
	return 0;
      if (!ctx->error)
	{
	  trace (("%p: EOF but ctx->eof flag not set", cookie));
          nread = 0;
          goto leave;
	}
      _gpg_err_set_errno (ctx->error_code);
      return -1;
    }

  nread = ctx->readpos < ctx->writepos
    ? ctx->writepos - ctx->readpos
    : READBUF_SIZE - ctx->readpos;
  if (nread > count)
    nread = count;
  memcpy (buffer, ctx->buffer + ctx->readpos, nread);
  ctx->readpos = (ctx->readpos + nread) % READBUF_SIZE;
  if (ctx->readpos == ctx->writepos && !ctx->eof)
    {
      if (!ResetEvent (ctx->have_data_ev))
	{
	  trace (("%p: ResetEvent failed: ec=%d",
                  cookie, (int)GetLastError ()));
          LeaveCriticalSection (&ctx->mutex);
	  /* FIXME: Should translate the error code.  */
	  _gpg_err_set_errno (EIO);
	  nread = -1;
          goto leave;
	}
    }
  if (!SetEvent (ctx->have_space_ev))
    {
      trace (("%p: SetEvent (%p) failed: ec=%d",
              cookie, ctx->have_space_ev, (int)GetLastError ()));
      LeaveCriticalSection (&ctx->mutex);
      /* FIXME: Should translate the error code.  */
      _gpg_err_set_errno (EIO);
      nread = -1;
      goto leave;
    }
  LeaveCriticalSection (&ctx->mutex);

 leave:
  trace_errno (nread==-1,("%p: leave nread=%d", cookie, (int)nread));
  return nread;
}


/* The writer does use a simple buffering strategy so that we are
   informed about write errors as soon as possible (i. e. with the the
   next call to the write function.  */
static DWORD CALLBACK
writer (void *arg)
{
  struct writer_context_s *ctx = arg;
  ssize_t nwritten;

  trace (("%p: writer starting", ctx));

  for (;;)
    {
      EnterCriticalSection (&ctx->mutex);
      if (ctx->stop_me && !ctx->nbytes)
	{
          LeaveCriticalSection (&ctx->mutex);
	  break;
        }
      if (!ctx->nbytes)
	{
	  if (!SetEvent (ctx->is_empty))
	    trace (("%p: SetEvent failed: ec=%d", ctx, (int)GetLastError ()));
	  if (!ResetEvent (ctx->have_data))
	    trace (("%p: ResetEvent failed: ec=%d", ctx, (int)GetLastError ()));
          LeaveCriticalSection (&ctx->mutex);
	  trace (("%p: idle", ctx));
	  WaitForSingleObject (ctx->have_data, INFINITE);
	  trace (("%p: got data to write", ctx));
          EnterCriticalSection (&ctx->mutex);
        }
      if (ctx->stop_me && !ctx->nbytes)
	{
          LeaveCriticalSection (&ctx->mutex);
	  break;
        }
      LeaveCriticalSection (&ctx->mutex);

      trace (("%p: writing up to %d bytes", ctx, ctx->nbytes));

      nwritten = ctx->pcookie->next_functions.public.func_write
        (ctx->pcookie->next_cookie, ctx->buffer, ctx->nbytes);
      trace (("%p: wrote %d bytes", ctx, nwritten));
      if (nwritten < 1)
        {
          /* XXX */
          if (errno == ERROR_BUSY)
            {
              /* Probably stop_me is set now.  */
              trace (("%p: pipe busy (unblocked?)", ctx));
              continue;
            }

          ctx->error_code = errno;
          ctx->error = 1;
          trace (("%p: write error: ec=%d", ctx, ctx->error_code));
          break;
        }

      EnterCriticalSection (&ctx->mutex);
      ctx->nbytes -= nwritten;
      LeaveCriticalSection (&ctx->mutex);
    }
  /* Indicate that we have an error.  */
  if (!SetEvent (ctx->is_empty))
    trace (("%p: SetEvent failed: ec=%d", ctx, (int)GetLastError ()));

  trace (("%p: waiting for close", ctx));
  WaitForSingleObject (ctx->close_ev, INFINITE);

  if (ctx->nbytes)
    trace (("%p: still %d bytes in buffer at close time", ctx, ctx->nbytes));

  CloseHandle (ctx->close_ev);
  CloseHandle (ctx->have_data);
  CloseHandle (ctx->is_empty);
  CloseHandle (ctx->thread_hd);
  DeleteCriticalSection (&ctx->mutex);
  trace (("%p: writer is destroyed", ctx));
  free (ctx); /* Standard free!  See comment in create_writer. */

  return 0;
}


static struct writer_context_s *
create_writer (estream_cookie_w32_pollable_t pcookie)
{
  struct writer_context_s *ctx;
  SECURITY_ATTRIBUTES sec_attr;
  DWORD tid;

  memset (&sec_attr, 0, sizeof sec_attr);
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  /* See comment at create_reader.  */
  ctx = calloc (1, sizeof *ctx);
  if (!ctx)
    {
      return NULL;
    }

  ctx->pcookie = pcookie;

  ctx->have_data = CreateEvent (&sec_attr, TRUE, FALSE, NULL);
  if (ctx->have_data)
    ctx->is_empty  = CreateEvent (&sec_attr, TRUE, TRUE, NULL);
  if (ctx->is_empty)
    ctx->close_ev = CreateEvent (&sec_attr, TRUE, FALSE, NULL);
  if (!ctx->have_data || !ctx->is_empty || !ctx->close_ev)
    {
      trace (("%p: CreateEvent failed: ec=%d", ctx, (int)GetLastError ()));
      if (ctx->have_data)
	CloseHandle (ctx->have_data);
      if (ctx->is_empty)
	CloseHandle (ctx->is_empty);
      if (ctx->close_ev)
	CloseHandle (ctx->close_ev);
      _gpgrt_free (ctx);
      return NULL;
    }

  InitializeCriticalSection (&ctx->mutex);

  ctx->thread_hd = CreateThread (&sec_attr, 0, writer, ctx, 0, &tid );

  if (!ctx->thread_hd)
    {
      trace (("%p: CreateThread failed: ec=%d", ctx, (int)GetLastError ()));
      DeleteCriticalSection (&ctx->mutex);
      if (ctx->have_data)
	CloseHandle (ctx->have_data);
      if (ctx->is_empty)
	CloseHandle (ctx->is_empty);
      if (ctx->close_ev)
	CloseHandle (ctx->close_ev);
      _gpgrt_free (ctx);
      return NULL;
    }
  else
    {
#if 0
      /* We set the priority of the thread higher because we know
	 that it only runs for a short time.  This greatly helps to
	 increase the performance of the I/O.  */
      SetThreadPriority (ctx->thread_hd, get_desired_thread_priority ());
#endif
    }

  return ctx;
}


static void
destroy_writer (struct writer_context_s *ctx)
{
  trace (("%p: enter pollable_destroy_writer", ctx));
  EnterCriticalSection (&ctx->mutex);
  trace (("%p: setting stopme", ctx));
  ctx->stop_me = 1;
  if (ctx->have_data)
    SetEvent (ctx->have_data);
  LeaveCriticalSection (&ctx->mutex);

  trace (("%p: waiting for empty", ctx));

  /* Give the writer a chance to flush the buffer.  */
  WaitForSingleObject (ctx->is_empty, INFINITE);

  /* After setting this event CTX is void.  */
  trace (("%p: set close_ev", ctx));
  SetEvent (ctx->close_ev);
  trace (("%p: leave pollable_destroy_writer", ctx));
}


/*
 * Write function for pollable objects.
 */
static gpgrt_ssize_t
func_w32_pollable_write (void *cookie, const void *buffer, size_t count)
{
  estream_cookie_w32_pollable_t pcookie = cookie;
  struct writer_context_s *ctx = pcookie->writer;
  int nwritten;

  trace (("%p: enter buffer: %p count: %d", cookie, buffer, count));
  if (count == 0)
    {
      nwritten = 0;
      goto leave;
    }

  if (ctx == NULL)
    {
      pcookie->writer = ctx = create_writer (pcookie);
      if (!ctx)
        {
          nwritten = -1;
          goto leave;
        }
      trace (("%p: new writer %p", cookie, pcookie->writer));
    }

  EnterCriticalSection (&ctx->mutex);
  trace (("%p: buffer: %p, count: %d, nbytes: %d",
          cookie, buffer, count, ctx->nbytes));
  if (!ctx->error && ctx->nbytes)
    {
      /* Bytes are pending for send.  */

      /* Reset the is_empty event.  Better safe than sorry.  */
      if (!ResetEvent (ctx->is_empty))
	{
          trace (("%p: ResetEvent failed: ec=%d",
                  cookie, (int)GetLastError ()));
          LeaveCriticalSection (&ctx->mutex);
	  /* FIXME: Should translate the error code.  */
	  _gpg_err_set_errno (EIO);
	  nwritten = -1;
          goto leave;
	}
      LeaveCriticalSection (&ctx->mutex);

      if (pcookie->modeflags & O_NONBLOCK)
        {
          trace (("%p: would block", cookie));
          _gpg_err_set_errno (EAGAIN);
          nwritten = -1;
          goto leave;
        }

      trace (("%p: waiting for empty buffer", cookie));
      WaitForSingleObject (ctx->is_empty, INFINITE);
      trace (("%p: buffer is empty", cookie));
      EnterCriticalSection (&ctx->mutex);
    }

  if (ctx->error)
    {
      LeaveCriticalSection (&ctx->mutex);
      if (ctx->error_code == ERROR_NO_DATA)
        _gpg_err_set_errno (EPIPE);
      else
        _gpg_err_set_errno (EIO);
      nwritten = -1;
      goto leave;
    }

  /* If no error occurred, the number of bytes in the buffer must be
     zero.  */
  gpgrt_assert (!ctx->nbytes);

  if (count > WRITEBUF_SIZE)
    count = WRITEBUF_SIZE;
  memcpy (ctx->buffer, buffer, count);
  ctx->nbytes = count;

  /* We have to reset the is_empty event early, because it is also
     used by the select() implementation to probe the channel.  */
  if (!ResetEvent (ctx->is_empty))
    {
      trace (("%p: ResetEvent failed: ec=%d", cookie, (int)GetLastError ()));
      LeaveCriticalSection (&ctx->mutex);
      /* FIXME: Should translate the error code.  */
      _gpg_err_set_errno (EIO);
      nwritten = -1;
      goto leave;
    }
  if (!SetEvent (ctx->have_data))
    {
      trace (("%p: SetEvent failed: ec=%d", cookie, (int)GetLastError ()));
      LeaveCriticalSection (&ctx->mutex);
      /* FIXME: Should translate the error code.  */
      _gpg_err_set_errno (EIO);
      nwritten = -1;
      goto leave;
    }
  LeaveCriticalSection (&ctx->mutex);

  nwritten = count;

 leave:
  trace_errno (nwritten==-1,("%p: leave nwritten=%d", cookie, nwritten));
  return nwritten;
}


/* This is the core of _gpgrt_poll.  The caller needs to make sure that
 * the syscall clamp has been engaged.  */
int
_gpgrt_w32_poll (gpgrt_poll_t *fds, size_t nfds, int timeout)
{
  HANDLE waitbuf[MAXIMUM_WAIT_OBJECTS];
  int waitidx[MAXIMUM_WAIT_OBJECTS];
#ifdef ENABLE_TRACING
  char waitinfo[MAXIMUM_WAIT_OBJECTS];
#endif
  unsigned int code;
  int nwait;
  int i;
  int any;
  int count;

#if 0
 restart:
#endif

  any = 0;
  nwait = 0;
  count = 0;
  for (i = 0; i < nfds; i++)
    {
      struct estream_cookie_w32_pollable *pcookie;

      if (fds[i].ignore)
	continue;

      if (fds[i].stream->intern->kind != BACKEND_W32_POLLABLE)
        {
          /* This stream does not support polling.  */
          fds[i].got_err = 1;
          continue;
        }

      pcookie = fds[i].stream->intern->cookie;

      if (fds[i].want_read || fds[i].want_write)
	{
          /* XXX: What if one wants read and write, is that supported?  */
	  if (fds[i].want_read)
	    {
	      struct reader_context_s *ctx = pcookie->reader;
              if (ctx == NULL)
                {
                  pcookie->reader = ctx = create_reader (pcookie);
                  if (!ctx)
                    {
                      /* FIXME:  Is the error code appropriate?  */
                      _gpg_err_set_errno (EBADF);
                      return -1;
                    }
                  trace (("%p: new reader %p", pcookie, pcookie->reader));
                }
              trace (("%p: using reader %p", pcookie, pcookie->reader));

              if (nwait >= DIM (waitbuf))
                {
                  trace (("oops: too many objects for WFMO"));
                  /* FIXME: Should translate the error code.  */
                  _gpg_err_set_errno (EIO);
                  return -1;
                }
              waitidx[nwait] = i;
#ifdef ENABLE_TRACING
              waitinfo[nwait] = 'r';
#endif /*ENABLE_TRACING*/
              waitbuf[nwait++] = ctx->have_data_ev;
	      any = 1;
            }
	  else if (fds[i].want_write)
	    {
	      struct writer_context_s *ctx = pcookie->writer;
              if (ctx == NULL)
                {
                  pcookie->writer = ctx = create_writer (pcookie);
                  if (!ctx)
                    {
                      trace (("oops: create writer failed"));
                      /* FIXME:  Is the error code appropriate?  */
                      _gpg_err_set_errno (EBADF);
                      return -1;
                    }
                  trace (("%p: new writer %p", pcookie, pcookie->writer));
                }
              trace (("%p: using writer %p", pcookie, pcookie->writer));

              if (nwait >= DIM (waitbuf))
                {
                  trace (("oops: Too many objects for WFMO"));
                  /* FIXME: Should translate the error code.  */
                  _gpg_err_set_errno (EIO);
                  return -1;
                }
              waitidx[nwait] = i;
#ifdef ENABLE_TRACING
              waitinfo[nwait] = 'w';
#endif /*ENABLE_TRACING*/
              waitbuf[nwait++] = ctx->is_empty;
	      any = 1;
            }
        }
    }
#ifdef ENABLE_TRACING
  trace_start (("poll on [ "));
  for (i = 0; i < nwait; i++)
    trace_append (("%d/%c ", waitidx[i], waitinfo[i]));
  trace_finish (("]"));
#endif /*ENABLE_TRACING*/

  if (!any)
    {
      /* WFMO needs at least one object, thus we use use sleep here.
       * INFINITE wait does not make any sense in this case, so we
       * error out. */
      if (timeout == -1)
        {
          _gpg_err_set_errno (EINVAL);
          return -1;
        }
      if (timeout)
        Sleep (timeout);
      code = WAIT_TIMEOUT;
    }
  else
    code = WaitForMultipleObjects (nwait, waitbuf, 0,
                                   timeout == -1 ? INFINITE : timeout);

  if (code < WAIT_OBJECT_0 + nwait)
    {
      /* This WFMO is a really silly function: It does return either
	 the index of the signaled object or if 2 objects have been
	 signalled at the same time, the index of the object with the
	 lowest object is returned - so and how do we find out how
	 many objects have been signaled???.  The only solution I can
	 imagine is to test each object starting with the returned
	 index individually - how dull.  */
      any = 0;
      for (i = code - WAIT_OBJECT_0; i < nwait; i++)
	{
	  if (WaitForSingleObject (waitbuf[i], 0) == WAIT_OBJECT_0)
	    {
	      gpgrt_assert (waitidx[i] >=0 && waitidx[i] < nfds);
              /* XXX: What if one wants read and write, is that
                 supported?  */
              if (fds[waitidx[i]].want_read)
                fds[waitidx[i]].got_read = 1;
              else if (fds[waitidx[i]].want_write)
                fds[waitidx[i]].got_write = 1;
	      any = 1;
	      count++;
	    }
	}
      if (!any)
	{
	  trace (("no signaled objects found after WFMO"));
	  count = -1;
	}
    }
  else if (code == WAIT_TIMEOUT)
    trace (("WFMO timed out"));
  else if (code == WAIT_FAILED)
    {
      trace (("WFMO failed: ec=%d", (int)GetLastError ()));
#if 0
      if (GetLastError () == ERROR_INVALID_HANDLE)
	{
	  int k;
	  int j = handle_to_fd (waitbuf[i]);

	  trace (("WFMO invalid handle %d removed", j));
	  for (k = 0 ; k < nfds; k++)
	    {
	      if (fds[k].fd == j)
		{
		  fds[k].want_read = fds[k].want_write = 0;
		  goto restart;
                }
            }
	  trace ((" oops, or not???"));
        }
#endif
      count = -1;
    }
  else
    {
      trace (("WFMO returned %u", code));
      count = -1;
    }

  if (count > 0)
    {
      trace_start (("poll OK [ "));
      for (i = 0; i < nfds; i++)
	{
	  if (fds[i].ignore)
	    continue;
	  if (fds[i].got_read || fds[i].got_write)
	    trace_append (("%c%d ", fds[i].want_read ? 'r' : 'w', i));
        }
      trace_finish (("]"));
    }

  if (count < 0)
    {
      /* FIXME: Should determine a proper error code.  */
      _gpg_err_set_errno (EIO);
    }

  return count;
}



/*
 * Implementation of pollable I/O on Windows.
 */

/*
 * Constructor for pollable objects.
 */
int
_gpgrt_w32_pollable_create (void *_GPGRT__RESTRICT *_GPGRT__RESTRICT cookie,
                            unsigned int modeflags,
                            struct cookie_io_functions_s next_functions,
                            void *next_cookie)
{
  estream_cookie_w32_pollable_t pcookie;
  int err;

  pcookie = _gpgrt_malloc (sizeof *pcookie);
  if (!pcookie)
    err = -1;
  else
    {
      pcookie->modeflags = modeflags;
      pcookie->next_functions = next_functions;
      pcookie->next_cookie = next_cookie;
      pcookie->reader = NULL;
      pcookie->writer = NULL;
      *cookie = pcookie;
      err = 0;
    }

  trace_errno (err,("cookie=%p", *cookie));
  return err;
}


/*
 * Seek function for pollable objects.
 */
static int
func_w32_pollable_seek (void *cookie, gpgrt_off_t *offset, int whence)
{
  estream_cookie_w32_pollable_t pcookie = cookie;
  (void) pcookie;
  (void) offset;
  (void) whence;
  /* XXX */
  _gpg_err_set_errno (EOPNOTSUPP);
  return -1;
}


/*
 * The IOCTL function for pollable objects.
 */
static int
func_w32_pollable_ioctl (void *cookie, int cmd, void *ptr, size_t *len)
{
  estream_cookie_w32_pollable_t pcookie = cookie;
  cookie_ioctl_function_t func_ioctl = pcookie->next_functions.func_ioctl;

  if (cmd == COOKIE_IOCTL_NONBLOCK)
    {
      if (ptr)
        pcookie->modeflags |= O_NONBLOCK;
      else
        pcookie->modeflags &= ~O_NONBLOCK;
      return 0;
    }

  if (func_ioctl)
    return func_ioctl (pcookie->next_cookie, cmd, ptr, len);

  _gpg_err_set_errno (EOPNOTSUPP);
  return -1;
}


/*
 * The destroy function for pollable objects.
 */
static int
func_w32_pollable_destroy (void *cookie)
{
  estream_cookie_w32_pollable_t pcookie = cookie;

  if (cookie)
    {
      if (pcookie->reader)
        destroy_reader (pcookie->reader);
      if (pcookie->writer)
        destroy_writer (pcookie->writer);
      pcookie->next_functions.public.func_close (pcookie->next_cookie);
      _gpgrt_free (pcookie);
    }
  return 0;
}

/*
 * Access object for the pollable functions.
 */
struct cookie_io_functions_s _gpgrt_functions_w32_pollable =
  {
    {
      func_w32_pollable_read,
      func_w32_pollable_write,
      func_w32_pollable_seek,
      func_w32_pollable_destroy,
    },
    func_w32_pollable_ioctl,
  };
