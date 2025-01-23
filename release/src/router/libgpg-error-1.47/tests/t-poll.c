/* t-poll.c - Check the poll function
 * Copyright (C) 2015 g10 Code GmbH
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

/* FIXME: We need much better tests that this very basic one.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#ifdef _WIN32
# include <windows.h>
# include <time.h>
#else
# ifdef USE_POSIX_THREADS
#  include <pthread.h>
# endif
#endif

#define PGM "t-poll"

#include "t-common.h"

#ifdef _WIN32
# define THREAD_RET_TYPE  DWORD WINAPI
# define THREAD_RET_VALUE 0
#else
# define THREAD_RET_TYPE  void *
# define THREAD_RET_VALUE NULL
#endif


/* Object to convey data to a thread.  */
struct thread_arg
{
  const char *name;
  estream_t stream;
  volatile int stop_me;
#ifdef USE_POSIX_THREADS
  pthread_t thread;
#elif _WIN32
  HANDLE thread;
#endif
};


static struct thread_arg peer_stdin;  /* Thread to feed the stdin.  */
static struct thread_arg peer_stdout; /* Thread to feed the stdout. */
static struct thread_arg peer_stderr; /* Thread to feed the stderr. */

static estream_t test_stdin;
static estream_t test_stdout;
static estream_t test_stderr;

#if defined(_WIN32) || defined(USE_POSIX_THREADS)

/* This thread feeds data to the given stream.  */
static THREAD_RET_TYPE
producer_thread (void *argaddr)
{
  struct thread_arg *arg = argaddr;
  int i = 0;

  (void)arg;

  while (!arg->stop_me && i++ < 3)
    {
      show ("thread '%s' about to write\n", arg->name);
      es_fprintf (arg->stream, "This is '%s' count=%d\n", arg->name, i);
      es_fflush (arg->stream);
    }
  es_fclose (arg->stream);
  return THREAD_RET_VALUE;
}

/* This thread eats data from the given stream.  */
static THREAD_RET_TYPE
consumer_thread (void *argaddr)
{
  struct thread_arg *arg = argaddr;
  char buf[15];

  (void)arg;

  while (!arg->stop_me)
    {
      show ("thread '%s' ready to read\n", arg->name);
      if (!es_fgets (buf, sizeof buf, arg->stream))
        {
          show ("Thread '%s' received EOF or error\n", arg->name);
          break;
        }
      show ("Thread '%s' got: '%s'\n", arg->name, buf);
    }
  es_fclose (arg->stream);
  return THREAD_RET_VALUE;
}

#endif /*_WIN32 || USE_POSIX_THREADS */


static void
launch_thread (THREAD_RET_TYPE (*fnc)(void *), struct thread_arg *th)
{
  int fd;

  th->stop_me = 0;
  fd = es_fileno (th->stream);
#ifdef _WIN32

  th->thread = CreateThread (NULL, 0, fnc, th, 0, NULL);
  if (!th->thread)
    die ("creating thread '%s' failed: rc=%d", th->name, (int)GetLastError ());
  show ("thread '%s' launched (fd=%d)\n", th->name, fd);

#elif USE_POSIX_THREADS

  if (pthread_create (&th->thread, NULL, fnc, th))
    die ("creating thread '%s' failed: %s\n", th->name, strerror (errno));
  show ("thread '%s' launched (fd=%d)\n", th->name, fd);

# else /* no thread support */

  verbose++;
  show ("no thread support - skipping test\n", PGM);
  verbose--;

#endif /* no thread support */
}


static void
join_thread (struct thread_arg *th)
{
#ifdef _WIN32
  int rc;

  rc = WaitForSingleObject (th->thread, INFINITE);
  if (rc == WAIT_OBJECT_0)
    show ("thread '%s' has terminated\n", th->name);
  else
    fail ("waiting for thread '%s' failed: %d", th->name, (int)GetLastError ());
  CloseHandle (th->thread);

#elif USE_POSIX_THREADS

  pthread_join (th->thread, NULL);
  show ("thread '%s' has terminated\n", th->name);

#endif
}


static void
create_pipe (estream_t *r_in, estream_t *r_out)
{
  gpg_error_t err;
#ifdef _WIN32
  HANDLE pipe_r;
  HANDLE pipe_w;
#else
  int filedes[2];
#endif
  es_syshd_t syshd[2];

#ifdef _WIN32
  if (CreatePipe (&pipe_r, &pipe_w, NULL, 512) == 0)
    die ("error creating a pipe: rc=%d\n", (int)GetLastError ());

  syshd[0].type = syshd[1].type = ES_SYSHD_HANDLE;
  syshd[0].u.handle = pipe_r;
  syshd[1].u.handle = pipe_w;

  show ("created pipe [%p, %p]\n", pipe_r, pipe_w);
#else
  if (pipe (filedes) == -1)
    {
      err = gpg_error_from_syserror ();
      die ("error creating a pipe: %s\n", gpg_strerror (err));
    }

  syshd[0].type = syshd[1].type = ES_SYSHD_FD;
  syshd[0].u.fd = filedes[0];
  syshd[1].u.fd = filedes[1];

  show ("created pipe [%d, %d]\n", filedes[0], filedes[1]);
#endif

  *r_in = es_sysopen (&syshd[0], "r,pollable");
  if (!*r_in)
    {
      err = gpg_error_from_syserror ();
      die ("error creating a stream for a pipe: %s\n", gpg_strerror (err));
    }

  *r_out = es_sysopen (&syshd[1], "w,pollable");
  if (!*r_out)
    {
      err = gpg_error_from_syserror ();
      die ("error creating a stream for a pipe: %s\n", gpg_strerror (err));
    }
}


static void
test_poll (void)
{
  int ret;
  gpgrt_poll_t fds[3];
  char buffer[16];
  size_t used, nwritten;
  int c;

  memset (fds, 0, sizeof fds);
  fds[0].stream = test_stdin;
  fds[0].want_read = 1;
  fds[1].stream = test_stdout;
  fds[1].want_write = 1;
  /* FIXME: We don't use the next stream at all.  */
  fds[2].stream = test_stderr;
  fds[2].want_write = 1;
  fds[2].ignore = 1;


  used = 0;
  while (used || !fds[0].ignore)
    {
      ret = gpgrt_poll (fds, DIM(fds), -1);
      if (ret == -1)
        {
          fail ("gpgrt_poll failed: %s\n", strerror (errno));
          continue;
        }
      if (!ret)
        {
          fail ("gpgrt_poll unexpectedly timed out\n");
          continue;
        }

      show ("gpgrt_poll detected %d events\n", ret);
      if (debug)
        show ("gpgrt_poll: r=%d"
              " 0:%c%c%c%c%c%c%c%c%c%c%c%c"
              " 1:%c%c%c%c%c%c%c%c%c%c%c%c"
              " 2:%c%c%c%c%c%c%c%c%c%c%c%c"
              "\n",
              ret,
              fds[0].want_read?  'r':'-',
              fds[0].want_write? 'w':'-',
              fds[0].want_oob?   'o':'-',
              fds[0].want_rdhup? 'h':'-',
              fds[0].ignore?     '!':'=',
              fds[0].got_read?   'r':'-',
              fds[0].got_write?  'w':'-',
              fds[0].got_oob?    'o':'-',
              fds[0].got_rdhup?  'h':'-',
              fds[0].got_hup?    'H':' ',
              fds[0].got_err?    'e':' ',
              fds[0].got_nval?   'n':' ',

              fds[1].want_read?  'r':'-',
              fds[1].want_write? 'w':'-',
              fds[1].want_oob?   'o':'-',
              fds[1].want_rdhup? 'h':'-',
              fds[1].ignore?     '!':'=',
              fds[1].got_read?   'r':'-',
              fds[1].got_write?  'w':'-',
              fds[1].got_oob?    'o':'-',
              fds[1].got_rdhup?  'h':'-',
              fds[1].got_hup?    'H':' ',
              fds[1].got_err?    'e':' ',
              fds[1].got_nval?   'n':' ',

              fds[2].want_read?  'r':'-',
              fds[2].want_write? 'w':'-',
              fds[2].want_oob?   'o':'-',
              fds[2].want_rdhup? 'h':'-',
              fds[2].ignore?     '!':'=',
              fds[2].got_read?   'r':'-',
              fds[2].got_write?  'w':'-',
              fds[2].got_oob?    'o':'-',
              fds[2].got_rdhup?  'h':'-',
              fds[2].got_hup?    'H':' ',
              fds[2].got_err?    'e':' ',
              fds[2].got_nval?   'n':' '
              );
      else
        show ("gpgrt_poll detected %d events\n", ret);

      if (fds[0].got_read)
        {
          /* Read from the producer.  */
          for (;;)
            {
              c = es_fgetc (fds[0].stream);
              if (c == EOF)
                {
                  if (es_feof (fds[0].stream))
                    {
                      show ("reading '%s': EOF\n", peer_stdin.name);
                      fds[0].ignore = 1; /* Not anymore needed.  */
                      peer_stdin.stop_me = 1; /* Tell the thread to stop.  */
                    }
                  else if (es_ferror (fds[0].stream))
                    {
                      fail ("error reading '%s': %s\n",
                            peer_stdin.name, strerror (errno));
                      fds[0].ignore = 1;    /* Disable.  */
                      peer_stdin.stop_me = 1; /* Tell the thread to stop.  */
                    }
                  else
                    show ("reading '%s': EAGAIN\n", peer_stdin.name);
                  break;
                }
              else
                {
                  if (used <= sizeof buffer -1)
                    buffer[used++] = c;
                  if (used == sizeof buffer)
                    {
                      show ("throttling reading from '%s'\n", peer_stdin.name);
                      fds[0].ignore = 1;
                      break;
                    }
                }
            }
          show ("read from '%s': %zu bytes\n", peer_stdin.name, used);
          if (used)
            fds[1].ignore = 0; /* Data to send.  */
        }
      if (fds[1].got_write)
        {
          if (used)
            {
              ret = es_write (fds[1].stream, buffer, used, &nwritten);
              show ("result for writing to '%s': ret=%d, n=%zu, nwritten=%zu\n",
                    peer_stdout.name, ret, used, nwritten);
              if (!ret)
                {
                  assert (nwritten <= used);
                  /* Move the remaining data to the front of buffer.  */
                  memmove (buffer, buffer + nwritten,
                           sizeof buffer - nwritten);
                  used -= nwritten;
                }
              ret = es_fflush (fds[1].stream);
              if (ret)
                fail ("Flushing for '%s' failed: %s\n",
                      peer_stdout.name, strerror (errno));
            }
          if (!used)
            fds[1].ignore = 1; /* No need to send data.  */
        }

      if (used < sizeof buffer / 2 && !peer_stdin.stop_me && fds[0].ignore)
        {
          show ("accelerate reading from '%s'\n", peer_stdin.name);
          fds[0].ignore = 0;
        }
    }
}


int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--help"))
        {
          puts (
"usage: ./t-poll [options]\n"
"\n"
"Options:\n"
"  --verbose      Show what is going on\n"
"  --debug        Flyswatter\n"
);
          exit (0);
        }
      if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
    }

  if (!gpg_error_check_version (GPG_ERROR_VERSION))
    {
      die ("gpg_error_check_version returned an error");
      errorcount++;
    }

  peer_stdin.name  = "stdin producer";
  create_pipe (&test_stdin, &peer_stdin.stream);
  peer_stdout.name = "stdout consumer";
  create_pipe (&peer_stdout.stream, &test_stdout);
  peer_stderr.name = "stderr consumer";
  create_pipe (&peer_stderr.stream, &test_stderr);

  if (es_set_nonblock (test_stdin, 1))
    fail ("error setting test_stdin to nonblock: %s\n", strerror (errno));
  if (es_set_nonblock (test_stdout, 1))
    fail ("error setting test_stdout to nonblock: %s\n", strerror (errno));
  if (es_set_nonblock (test_stderr, 1))
    fail ("error setting test_stderr to nonblock: %s\n", strerror (errno));

  launch_thread (producer_thread, &peer_stdin );
  launch_thread (consumer_thread, &peer_stdout);
  launch_thread (consumer_thread, &peer_stderr);
  test_poll ();
  show ("Waiting for threads to terminate...\n");
  es_fclose (test_stdin);
  es_fclose (test_stdout);
  es_fclose (test_stderr);
  peer_stdin.stop_me = 1;
  peer_stdout.stop_me = 1;
  peer_stderr.stop_me = 1;
  join_thread (&peer_stdin);
  join_thread (&peer_stdout);
  join_thread (&peer_stderr);

  return errorcount ? 1 : 0;
}
