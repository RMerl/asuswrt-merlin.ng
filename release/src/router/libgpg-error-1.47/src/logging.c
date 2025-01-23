/* logging.c - Useful logging functions
 * Copyright (C) 1998-2001, 2003-2006, 2009-2010,
 *               2017  Free Software Foundation, Inc.
 * Copyright (C) 1998-1999, 2001-2006, 2008-2017  Werner Koch
 *
 * This file is part of Libgpg-error.
 *
 * Libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * This file was originally a part of GnuPG.
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_W32_SYSTEM
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# include <windows.h>
#else /*!HAVE_W32_SYSTEM*/
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif /*!HAVE_W32_SYSTEM*/
#include <unistd.h>
#include <fcntl.h>
/* #include <execinfo.h> */

#define _GPGRT_NEED_AFLOCAL 1
#include "gpgrt-int.h"


#ifdef HAVE_W32_SYSTEM
# ifndef S_IRWXG
#  define S_IRGRP S_IRUSR
#  define S_IWGRP S_IWUSR
# endif
# ifndef S_IRWXO
#  define S_IROTH S_IRUSR
#  define S_IWOTH S_IWUSR
# endif
#endif


#undef WITH_IPV6
#if defined (AF_INET6) && defined(PF_INET) \
    && defined (INET6_ADDRSTRLEN) && defined(HAVE_INET_PTON)
# define WITH_IPV6 1
#endif

#ifndef EAFNOSUPPORT
# define EAFNOSUPPORT EINVAL
#endif
#ifndef INADDR_NONE  /* Slowaris is missing that.  */
#define INADDR_NONE  ((unsigned long)(-1))
#endif /*INADDR_NONE*/

#ifdef HAVE_W32_SYSTEM
#define sock_close(a)  closesocket(a)
#else
#define sock_close(a)  close(a)
#endif


static estream_t logstream;
static int log_socket = -1;
static char prefix_buffer[80];
static int with_time;
static int with_prefix;
static int with_pid;
#ifdef HAVE_W32_SYSTEM
static int no_registry;
#endif
static int (*get_pid_suffix_cb)(unsigned long *r_value);
static const char * (*socket_dir_cb)(void);
static int running_detached;
static int force_prefixes;

static int missing_lf;
static int errorcount;


/* An object to convey data to the fmt_string_filter.  */
struct fmt_string_filter_s
{
  char *last_result;
};



/* Get the error count as maintained by the log fucntions.  With CLEAR
 * set reset the counter.  */
int
_gpgrt_get_errorcount (int clear)
{
  int n = errorcount;
  if (clear)
    errorcount = 0;
  return n;
}


/* Increment the error count as maintained by the log functions.  */
void
_gpgrt_inc_errorcount (void)
{
  /* Protect against counter overflow.  */
  if (errorcount < 30000)
    errorcount++;
}


/* The following 3 functions are used by _gpgrt_fopencookie to write logs
   to a socket.  */
struct fun_cookie_s
{
  int fd;
  int quiet;
  int want_socket;
  int is_socket;
  char name[1];
};


/* Write NBYTES of BUFFER to file descriptor FD. */
static int
writen (int fd, const void *buffer, size_t nbytes, int is_socket)
{
  const char *buf = buffer;
  size_t nleft = nbytes;
  int nwritten;
#ifndef HAVE_W32_SYSTEM
  (void)is_socket; /* Not required.  */
#endif

  while (nleft > 0)
    {
#ifdef HAVE_W32_SYSTEM
      if (is_socket)
        nwritten = send (fd, buf, nleft, 0);
      else
#endif
        nwritten = write (fd, buf, nleft);

      if (nwritten < 0 && errno == EINTR)
        continue;
      if (nwritten < 0)
        return -1;
      nleft -= nwritten;
      buf = buf + nwritten;
    }

  return 0;
}


/* Returns true if STR represents a valid port number in decimal
   notation and no garbage is following.  */
static int
parse_portno (const char *str, unsigned short *r_port)
{
  unsigned int value;

  for (value=0; *str && (*str >= '0' && *str <= '9'); str++)
    {
      value = value * 10 + (*str - '0');
      if (value > 65535)
        return 0;
    }
  if (*str || !value)
    return 0;

  *r_port = value;
  return 1;
}


static gpgrt_ssize_t
fun_writer (void *cookie_arg, const void *buffer, size_t size)
{
  struct fun_cookie_s *cookie = cookie_arg;

  /* Note that we always try to reconnect to the socket but print
     error messages only the first time an error occurred.  If
     RUNNING_DETACHED is set we don't fall back to stderr and even do
     not print any error messages.  This is needed because detached
     processes often close stderr and by writing to file descriptor 2
     we might send the log message to a file not intended for logging
     (e.g. a pipe or network connection). */
  if (cookie->want_socket && cookie->fd == -1)
    {
#ifdef WITH_IPV6
      struct sockaddr_in6 srvr_addr_in6;
#endif
      struct sockaddr_in srvr_addr_in;
#ifndef HAVE_W32_SYSTEM
      struct sockaddr_un srvr_addr_un;
#endif
      const char *name_for_err = "";
      size_t addrlen;
      struct sockaddr *srvr_addr = NULL;
      unsigned short port = 0;
      int af = AF_LOCAL;
      int pf = PF_LOCAL;
      const char *name = cookie->name;

      /* Not yet open or meanwhile closed due to an error. */
      cookie->is_socket = 0;

      /* Check whether this is a TCP socket or a local socket.  */
      if (!strncmp (name, "tcp://", 6) && name[6])
        {
          name += 6;
          af = AF_INET;
          pf = PF_INET;
        }
#ifndef HAVE_W32_SYSTEM
      else if (!strncmp (name, "socket://", 9))
        name += 9;
#endif

      if (af == AF_LOCAL)
        {
          addrlen = 0;
#ifndef HAVE_W32_SYSTEM
          memset (&srvr_addr, 0, sizeof srvr_addr);
          srvr_addr_un.sun_family = af;
          if (!*name)
            {
              if (socket_dir_cb && (name = socket_dir_cb ()) && *name
                  && strlen (name) + 7 < sizeof (srvr_addr_un.sun_path)-1)
                {
                  strncpy (srvr_addr_un.sun_path,
                           name, sizeof (srvr_addr_un.sun_path)-1);
                  strcat (srvr_addr_un.sun_path, "/S.log");
                  srvr_addr_un.sun_path[sizeof (srvr_addr_un.sun_path)-1] = 0;
                  srvr_addr = (struct sockaddr *)&srvr_addr_un;
                  addrlen = SUN_LEN (&srvr_addr_un);
                  name_for_err = srvr_addr_un.sun_path;
                }
            }
          else
            {
              if (strlen (name) < sizeof (srvr_addr_un.sun_path)-1)
                {
                  strncpy (srvr_addr_un.sun_path,
                           name, sizeof (srvr_addr_un.sun_path)-1);
                  srvr_addr_un.sun_path[sizeof (srvr_addr_un.sun_path)-1] = 0;
                  srvr_addr = (struct sockaddr *)&srvr_addr_un;
                  addrlen = SUN_LEN (&srvr_addr_un);
                }
            }
#endif /*!HAVE_W32SYSTEM*/
        }
      else
        {
          char *addrstr, *p;
#ifdef HAVE_INET_PTON
          void *addrbuf = NULL;
#endif /*HAVE_INET_PTON*/

          addrstr = _gpgrt_malloc (strlen (name) + 1);
          if (!addrstr)
            addrlen = 0; /* This indicates an error.  */
          else if (*name == '[')
            {
              /* Check for IPv6 literal address.  */
              strcpy (addrstr, name+1);
              p = strchr (addrstr, ']');
              if (!p || p[1] != ':' || !parse_portno (p+2, &port))
                {
                  _gpg_err_set_errno (EINVAL);
                  addrlen = 0;
                }
              else
                {
                  *p = 0;
#ifdef WITH_IPV6
                  af = AF_INET6;
                  pf = PF_INET6;
                  memset (&srvr_addr_in6, 0, sizeof srvr_addr_in6);
                  srvr_addr_in6.sin6_family = af;
                  srvr_addr_in6.sin6_port = htons (port);
#ifdef HAVE_INET_PTON
                  addrbuf = &srvr_addr_in6.sin6_addr;
#endif /*HAVE_INET_PTON*/
                  srvr_addr = (struct sockaddr *)&srvr_addr_in6;
                  addrlen = sizeof srvr_addr_in6;
#else
                  _gpg_err_set_errno (EAFNOSUPPORT);
                  addrlen = 0;
#endif
                }
            }
          else
            {
              /* Check for IPv4 literal address.  */
              strcpy (addrstr, name);
              p = strchr (addrstr, ':');
              if (!p || !parse_portno (p+1, &port))
                {
                  _gpg_err_set_errno (EINVAL);
                  addrlen = 0;
                }
              else
                {
                  *p = 0;
                  memset (&srvr_addr_in, 0, sizeof srvr_addr_in);
                  srvr_addr_in.sin_family = af;
                  srvr_addr_in.sin_port = htons (port);
#ifdef HAVE_INET_PTON
                  addrbuf = &srvr_addr_in.sin_addr;
#endif /*HAVE_INET_PTON*/
                  srvr_addr = (struct sockaddr *)&srvr_addr_in;
                  addrlen = sizeof srvr_addr_in;
                }
            }

          if (addrlen)
            {
#ifdef HAVE_INET_PTON
              if (inet_pton (af, addrstr, addrbuf) != 1)
                addrlen = 0;
#else /*!HAVE_INET_PTON*/
              /* We need to use the old function.  If we are here v6
                 support isn't enabled anyway and thus we can do fine
                 without.  Note that Windows has a compatible inet_pton
                 function named inetPton, but only since Vista.  */
              srvr_addr_in.sin_addr.s_addr = inet_addr (addrstr);
              if (srvr_addr_in.sin_addr.s_addr == INADDR_NONE)
                addrlen = 0;
#endif /*!HAVE_INET_PTON*/
            }

          _gpgrt_free (addrstr);
        }

      cookie->fd = addrlen? socket (pf, SOCK_STREAM, 0) : -1;
      if (cookie->fd == -1)
        {
          if (!cookie->quiet && !running_detached
              && isatty (_gpgrt_fileno (es_stderr)))
            _gpgrt_fprintf (es_stderr,
                            "failed to create socket for logging: %s\n",
                            strerror (errno));
        }
      else
        {
          if (connect (cookie->fd, srvr_addr, addrlen) == -1)
            {
              if (!cookie->quiet && !running_detached
                  && isatty (_gpgrt_fileno (es_stderr)))
                _gpgrt_fprintf (es_stderr, "can't connect to '%s%s': %s\n",
                                cookie->name, name_for_err, strerror(errno));
              sock_close (cookie->fd);
              cookie->fd = -1;
            }
        }

      if (cookie->fd == -1)
        {
          if (!running_detached)
            {
              /* Due to all the problems with apps not running
                 detached but being called with stderr closed or used
                 for a different purposes, it does not make sense to
                 switch to stderr.  We therefore disable it. */
              if (!cookie->quiet)
                {
                  /* fputs ("switching logging to stderr\n", stderr);*/
                  cookie->quiet = 1;
                }
              cookie->fd = -1; /*fileno (stderr);*/
            }
        }
      else /* Connection has been established. */
        {
          cookie->quiet = 0;
          cookie->is_socket = 1;
        }
    }

  log_socket = cookie->fd;
  if (cookie->fd != -1)
    {
      if (!writen (cookie->fd, buffer, size, cookie->is_socket))
        return (gpgrt_ssize_t)size; /* Okay. */
    }

  if (!running_detached && cookie->fd != -1
      && isatty (_gpgrt_fileno (es_stderr)))
    {
      if (*cookie->name)
        _gpgrt_fprintf (es_stderr, "error writing to '%s': %s\n",
                        cookie->name, strerror(errno));
      else
        _gpgrt_fprintf (es_stderr, "error writing to file descriptor %d: %s\n",
                        cookie->fd, strerror(errno));
    }
  if (cookie->is_socket && cookie->fd != -1)
    {
      sock_close (cookie->fd);
      cookie->fd = -1;
      log_socket = -1;
    }

  return (gpgrt_ssize_t)size;
}


static int
fun_closer (void *cookie_arg)
{
  struct fun_cookie_s *cookie = cookie_arg;

  if (cookie->fd != -1 && cookie->fd != 2)
    sock_close (cookie->fd);
  _gpgrt_free (cookie);
  log_socket = -1;
  return 0;
}


/* Common function to either set the logging to a file or a file
   descriptor. */
static void
set_file_fd (const char *name, int fd, estream_t stream)
{
  estream_t fp;
  int want_socket = 0;
  struct fun_cookie_s *cookie;

  /* Close an open log stream.  */
  if (logstream)
    {
      if (logstream != es_stderr)
        _gpgrt_fclose (logstream);
      logstream = NULL;
    }

  if (stream)
    {
      /* We don't use a cookie to log directly to a stream.  */
      fp = stream;
      goto leave;
    }

  /* Figure out what kind of logging we want.  */
  if (name && !strcmp (name, "-"))
    {
      fp = es_stderr;
      goto leave;
    }
  else if (name && !strncmp (name, "tcp://", 6) && name[6])
    want_socket = 1;
#ifndef HAVE_W32_SYSTEM
  else if (name && !strncmp (name, "socket://", 9))
    want_socket = 2;
#endif /*HAVE_W32_SYSTEM*/

  /* Setup a new stream.  */

  if (!name)
    fp = _gpgrt_fdopen (fd, "w");
  else if (!want_socket)
    fp = _gpgrt_fopen (name, "a");
  else
    {
      es_cookie_io_functions_t io = { NULL };

      cookie = _gpgrt_malloc (sizeof *cookie + (name? strlen (name):0));
      if (!cookie)
        return; /* oops */
      strcpy (cookie->name, name? name:"");
      cookie->quiet = 0;
      cookie->is_socket = 0;
      cookie->want_socket = want_socket;
      cookie->fd = -1;
      log_socket = cookie->fd;

      io.func_write = fun_writer;
      io.func_close = fun_closer;

      fp = _gpgrt_fopencookie (cookie, "w", io);
    }

  /* On error default to a stderr based estream.  */
  if (!fp)
    fp = es_stderr;

 leave:
  _gpgrt_setvbuf (fp, NULL, _IOLBF, 0);

  logstream = fp;

  /* We always need to print the prefix and the pid for socket mode,
     so that the server reading the socket can do something
     meaningful. */
  force_prefixes = want_socket;

  missing_lf = 0;
}


/* Set the file to write log to.  The special names NULL and "-" may
 * be used to select stderr and names formatted like
 * "socket:///home/foo/mylogs" may be used to write the logging to the
 * socket "/home/foo/mylogs".  If the connection to the socket fails
 * or a write error is detected, the function writes to stderr and
 * tries the next time again to connect the socket.  Calling this
 * function with (NULL, NULL, -1) sets the default sink.
 * Warning: This function is not thread-safe.
 */
void
_gpgrt_log_set_sink (const char *name, estream_t stream, int fd)
{
  if (name && !stream && fd == -1)
    set_file_fd (name, -1, NULL);
  else if (!name && !stream && fd != -1)
    {
      if (!_gpgrt_fd_valid_p (fd))
        _gpgrt_log_fatal ("gpgrt_log_set_sink: fd is invalid: %s\n",
                     strerror (errno));
      set_file_fd (NULL, fd, NULL);
    }
  else if (!name && stream && fd == -1)
    {
      set_file_fd (NULL, -1, stream);
    }
  else /* default */
    set_file_fd ("-", -1, NULL);
}


/* Set a function to retrieve the directory name of a socket if
 * only "socket://" has been given to log_set_file.
 * Warning: This function is not thread-safe.  */
void
_gpgrt_log_set_socket_dir_cb (const char *(*fnc)(void))
{
  socket_dir_cb = fnc;
}


/* Warning: This function is not thread-safe.  */
void
_gpgrt_log_set_pid_suffix_cb (int (*cb)(unsigned long *r_value))
{
  get_pid_suffix_cb = cb;
}


/* Warning: Changing TEXT is not thread-safe.  Changing only flags
 * might be thread-safe.  */
void
_gpgrt_log_set_prefix (const char *text, unsigned int flags)
{
  if (text)
    {
      strncpy (prefix_buffer, text, sizeof (prefix_buffer)-1);
      prefix_buffer[sizeof (prefix_buffer)-1] = 0;
    }

  with_prefix = (flags & GPGRT_LOG_WITH_PREFIX);
  with_time = (flags & GPGRT_LOG_WITH_TIME);
  with_pid  = (flags & GPGRT_LOG_WITH_PID);
  running_detached = (flags & GPGRT_LOG_RUN_DETACHED);
#ifdef HAVE_W32_SYSTEM
  no_registry = (flags & GPGRT_LOG_NO_REGISTRY);
#endif
}


const char *
_gpgrt_log_get_prefix (unsigned int *flags)
{
  if (flags)
    {
      *flags = 0;
      if (with_prefix)
        *flags |= GPGRT_LOG_WITH_PREFIX;
      if (with_time)
        *flags |= GPGRT_LOG_WITH_TIME;
      if (with_pid)
        *flags |= GPGRT_LOG_WITH_PID;
      if (running_detached)
        *flags |= GPGRT_LOG_RUN_DETACHED;
#ifdef HAVE_W32_SYSTEM
      if (no_registry)
        *flags |= GPGRT_LOG_NO_REGISTRY;
#endif
    }
  return prefix_buffer;
}

/* This function returns true if the file descriptor FD is in use for
 * logging.  This is preferable over a test using log_get_fd in that
 * it allows the logging code to use more then one file descriptor.  */
int
_gpgrt_log_test_fd (int fd)
{
  if (logstream)
    {
      int tmp = _gpgrt_fileno (logstream);
      if ( tmp != -1 && tmp == fd)
        return 1;
    }
  if (log_socket != -1 && log_socket == fd)
    return 1;
  return 0;
}

int
_gpgrt_log_get_fd (void)
{
  return logstream? _gpgrt_fileno (logstream) : -1;
}

estream_t
_gpgrt_log_get_stream (void)
{
  if (!logstream)
    {
      /* Make sure a log stream has been set.  */
      _gpgrt_log_set_sink (NULL, NULL, -1);
      if (!logstream)
        {
          fputs ("gpgrt fatal: failed to init log stream\n", stderr);
          _gpgrt_abort ();
        }
    }
  return logstream;
}


/* A filter used with the fprintf_sf function to sanitize the args for
 * "%s" format specifiers.  */
static char *
fmt_string_filter (const char *string, int no, void *opaque)
{
  struct fmt_string_filter_s *state = opaque;
  const unsigned char *p;
  size_t buflen;
  char *d;
  int any;

  if (no == -1)
    {
      /* The printf engine asked us to release resources.  */
      if (state->last_result)
        {
          _gpgrt_free (state->last_result);
          state->last_result = NULL;
        }
      return NULL;
    }

  if (!string)
    return NULL; /* Nothing to filter - printf handles NULL nicely.  */

  /* Check whether escaping is needed and count needed length. */
  any = 0;
  buflen = 1;
  for (p = (const unsigned char *)string; *p; p++)
    {
      switch (*p)
        {
        case '\n':
        case '\r':
        case '\f':
        case '\v':
        case '\b':
        case '\t':
        case '\a':
        case '\\':
          buflen += 2;
          any = 1;
          break;
        default:
          if (*p < 0x20 || *p == 0x7f)
            {
              buflen += 5;
              any = 1;
            }
          else
            buflen++;
        }
    }
  if (!any)
    return (char*)string;  /* Nothing to escape.  */

  /* Create a buffer and escape the input.  */
  _gpgrt_free (state->last_result);
  state->last_result = _gpgrt_malloc (buflen);
  if (!state->last_result)
    return "[out_of_core_in_format_string_filter]";

  d = state->last_result;
  for (p = (const unsigned char *)string; *p; p++)
    {
      switch (*p)
        {
        case '\n': *d++ = '\\'; *d++ = 'n'; break;
        case '\r': *d++ = '\\'; *d++ = 'r'; break;
        case '\f': *d++ = '\\'; *d++ = 'f'; break;
        case '\v': *d++ = '\\'; *d++ = 'v'; break;
        case '\b': *d++ = '\\'; *d++ = 'b'; break;
        case '\t': *d++ = '\\'; *d++ = 't'; break;
        case '\a': *d++ = '\\'; *d++ = 'a'; break;
        case '\\': *d++ = '\\'; *d++ = '\\'; break;

        default:
          if (*p < 0x20 || *p == 0x7f)
            {
              snprintf (d, 5, "\\x%02x", *p);
              d += 4;
            }
          else
            *d++ = *p;
        }
    }
  *d = 0;
  return state->last_result;
}


/* Note: LOGSTREAM is expected to be locked.  */
static int
print_prefix (int level, int leading_backspace)
{
  int rc;
  int length = 0;

  if (level != GPGRT_LOGLVL_CONT)
    { /* Note this does not work for multiple line logging as we would
       * need to print to a buffer first */
      if (with_time && !force_prefixes)
        {
          struct tm *tp;
          time_t atime = time (NULL);

          tp = localtime (&atime);
          rc = _gpgrt_fprintf_unlocked (logstream,
                                        "%04d-%02d-%02d %02d:%02d:%02d ",
                               1900+tp->tm_year, tp->tm_mon+1, tp->tm_mday,
                               tp->tm_hour, tp->tm_min, tp->tm_sec );
          if (rc > 0)
            length += rc;
        }
      if (with_prefix || force_prefixes)
        {
          _gpgrt_fputs_unlocked (prefix_buffer, logstream);
          length += strlen (prefix_buffer);
        }
      if (with_pid || force_prefixes)
        {
          unsigned long pidsuf;
          int pidfmt;

          if (get_pid_suffix_cb && (pidfmt=get_pid_suffix_cb (&pidsuf)))
            rc = _gpgrt_fprintf_unlocked (logstream,
                                          pidfmt == 1? "[%u.%lu]":"[%u.%lx]",
                                          (unsigned int)getpid (), pidsuf);
          else
            rc = _gpgrt_fprintf_unlocked (logstream, "[%u]",
                                          (unsigned int)getpid ());
          if (rc > 0)
            length += rc;
        }
      if ((!with_time && (with_prefix || with_pid)) || force_prefixes)
        {
          _gpgrt_putc_unlocked (':', logstream);
          length++;
        }
      /* A leading backspace suppresses the extra space so that we can
         correctly output, programname, filename and linenumber. */
      if (!leading_backspace
          && (with_time || with_prefix || with_pid || force_prefixes))
        {
          _gpgrt_putc_unlocked (' ', logstream);
          length++;
        }
    }

  switch (level)
    {
    case GPGRT_LOGLVL_BEGIN: break;
    case GPGRT_LOGLVL_CONT: break;
    case GPGRT_LOGLVL_INFO: break;
    case GPGRT_LOGLVL_WARN: break;
    case GPGRT_LOGLVL_ERROR: break;
    case GPGRT_LOGLVL_FATAL:
      _gpgrt_fputs_unlocked ("Fatal: ", logstream);
      length += 7;
      break;
    case GPGRT_LOGLVL_BUG:
      _gpgrt_fputs_unlocked ("Ohhhh jeeee: ", logstream);
      length += 13;
      break;
    case GPGRT_LOGLVL_DEBUG:
      _gpgrt_fputs_unlocked ("DBG: ", logstream);
      length += 5;
      break;
    default:
      rc = _gpgrt_fprintf_unlocked (logstream,
                                    "[Unknown log level %d]: ", level);
      if (rc > 0)
        length += rc;
      break;
    }

  return length;
}


/* Internal worker function.  Exported so that we can use it in
 * visibility.c.  Returs the number of characters printed or 0 if the
 * line ends in a LF. */
int
_gpgrt_logv_internal (int level, int ignore_arg_ptr, const char *extrastring,
                      const char *prefmt, const char *fmt, va_list arg_ptr)
{
  int leading_backspace = (fmt && *fmt == '\b');
  int length;
  int rc;

  if (!logstream)
    {
#ifdef HAVE_W32_SYSTEM
      char *tmp;

      tmp = (no_registry
             ? NULL
             : _gpgrt_w32_reg_query_string (NULL, "Software\\\\GNU\\\\GnuPG",
                                            "DefaultLogFile"));
      _gpgrt_log_set_sink (tmp && *tmp? tmp : NULL, NULL, -1);
      _gpgrt_free (tmp);
#else
      /* Make sure a log stream has been set.  */
      _gpgrt_log_set_sink (NULL, NULL, -1);
#endif
      if (!logstream)
        {
          fputs ("gpgrt fatal: failed to init log stream\n", stderr);
          _gpgrt_abort ();
        }
    }

  _gpgrt_flockfile (logstream);
  if (missing_lf && level != GPGRT_LOGLVL_CONT)
    _gpgrt_putc_unlocked ('\n', logstream );
  missing_lf = 0;

  length = print_prefix (level, leading_backspace);
  if (leading_backspace)
    fmt++;

  if (fmt)
    {
      if (prefmt)
        {
          _gpgrt_fputs_unlocked (prefmt, logstream);
          length += strlen (prefmt);
        }

      if (ignore_arg_ptr)
        { /* This is used by log_string and comes with the extra
           * feature that after a LF the next line is indented by the
           * length of the prefix.  Note that we do not yet include
           * the length of the timestamp and pid in the indent
           * computation.  */
          const char *p, *pend;

          for (p = fmt; (pend = strchr (p, '\n')); p = pend+1)
            {
              rc = _gpgrt_fprintf_unlocked (logstream, "%*s%.*s",
                                 (int)((p != fmt
                                        && (with_prefix || force_prefixes))
                                       ?strlen (prefix_buffer)+2:0), "",
                                 (int)(pend - p)+1, p);
              if (rc > 0)
                length += rc;
            }
          _gpgrt_fputs_unlocked (p, logstream);
          length += strlen (p);
        }
      else
        {
          struct fmt_string_filter_s sf = {NULL};

          rc = _gpgrt_vfprintf_unlocked (logstream, fmt_string_filter, &sf,
                                         fmt, arg_ptr);
          if (rc > 0)
            length += rc;
        }

      if (*fmt && fmt[strlen(fmt)-1] != '\n')
        missing_lf = 1;
    }

  /* If we have an EXTRASTRING print it now while we still hold the
   * lock on the logstream.  */
  if (extrastring)
    {
      int c;

      if (missing_lf)
        {
          _gpgrt_putc_unlocked ('\n', logstream);
          missing_lf = 0;
          length = 0;
        }
      length += print_prefix (level, leading_backspace);
      _gpgrt_fputs_unlocked (">> ", logstream);
      length += 3;
      missing_lf = 1;
      while ((c = *extrastring++))
        {
          missing_lf = 1;
          if (c == '\\')
            {
              _gpgrt_fputs_unlocked ("\\\\", logstream);
              length += 2;
            }
          else if (c == '\r')
            {
              _gpgrt_fputs_unlocked ("\\r", logstream);
              length += 2;
            }
          else if (c == '\n')
            {
              _gpgrt_fputs_unlocked ("\\n\n", logstream);
              length = 0;
              if (*extrastring)
                {
                  length += print_prefix (level, leading_backspace);
                  _gpgrt_fputs_unlocked (">> ", logstream);
                  length += 3;
                }
              else
                missing_lf = 0;
            }
          else
            {
              _gpgrt_putc_unlocked (c, logstream);
              length++;
            }
        }
      if (missing_lf)
        {
          _gpgrt_putc_unlocked ('\n', logstream);
          length = 0;
          missing_lf = 0;
        }
    }

  if (level == GPGRT_LOGLVL_FATAL)
    {
      if (missing_lf)
        _gpgrt_putc_unlocked ('\n', logstream);
      _gpgrt_funlockfile (logstream);
      exit (2);
    }
  else if (level == GPGRT_LOGLVL_BUG)
    {
      if (missing_lf)
        _gpgrt_putc_unlocked ('\n', logstream );
      _gpgrt_funlockfile (logstream);
      /* Using backtrace requires a configure test and to pass
       * -rdynamic to gcc.  Thus we do not enable it now.  */
      /* { */
      /*   void *btbuf[20]; */
      /*   int btidx, btlen; */
      /*   char **btstr; */

      /*   btlen = backtrace (btbuf, DIM (btbuf)); */
      /*   btstr = backtrace_symbols (btbuf, btlen); */
      /*   if (btstr) */
      /*     for (btidx=0; btidx < btlen; btidx++) */
      /*       log_debug ("[%d] %s\n", btidx, btstr[btidx]); */
      /* } */
      _gpgrt_abort ();
    }
  else
    _gpgrt_funlockfile (logstream);

  /* Bumb the error counter for log_error.  */
  if (level == GPGRT_LOGLVL_ERROR)
    _gpgrt_inc_errorcount ();

  return length;
}


void
_gpgrt_log (int level, const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt) ;
  _gpgrt_logv_internal (level, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt_logv (int level, const char *fmt, va_list arg_ptr)
{
  _gpgrt_logv_internal (level, 0, NULL, NULL, fmt, arg_ptr);
}


/* Same as log_logv but PREFIX is printed immediately before FMT.
 * Note that PREFIX is an additional string and independent of the
 * prefix set by gpgrt_log_set_prefix.  */
void
_gpgrt_logv_prefix (int level, const char *prefix,
                    const char *fmt, va_list arg_ptr)
{
  _gpgrt_logv_internal (level, 0, NULL, prefix, fmt, arg_ptr);
}


static void
do_log_ignore_arg (int level, const char *str, ...)
{
  va_list arg_ptr;
  va_start (arg_ptr, str);
  _gpgrt_logv_internal (level, 1, NULL, NULL, str, arg_ptr);
  va_end (arg_ptr);
}


/* Log STRING at LEVEL but indent from the second line on by the
 * length of the prefix.  */
void
_gpgrt_log_string (int level, const char *string)
{
  /* We need a dummy arg_ptr, but there is no portable way to create
   * one.  So we call the _gpgrt_logv_internal function through a
   * variadic wrapper. */
  do_log_ignore_arg (level, string);
}


void
_gpgrt_log_info (const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_INFO, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt_log_error (const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_ERROR, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt_log_fatal (const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_FATAL, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
  _gpgrt_abort (); /* Never called; just to make the compiler happy.  */
}


void
_gpgrt_log_bug (const char *fmt, ...)
{
  va_list arg_ptr ;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_BUG, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
  _gpgrt_abort (); /* Never called; just to make the compiler happy.  */
}


void
_gpgrt_log_debug (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG, 0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


/* The same as log_debug but at the end of the output STRING is
 * printed with LFs expanded to include the prefix and a final --end--
 * marker.  */
void
_gpgrt_log_debug_string (const char *string, const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG, 0, string, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt_log_printf (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_internal (fmt ? GPGRT_LOGLVL_CONT : GPGRT_LOGLVL_BEGIN,
                        0, NULL, NULL, fmt, arg_ptr);
  va_end (arg_ptr);
}


/* Flush the log - this is useful to make sure that the trailing
   linefeed has been printed.  */
void
_gpgrt_log_flush (void)
{
  do_log_ignore_arg (GPGRT_LOGLVL_CONT, NULL);
}


/* Print a hexdump of (BUFFER,LENGTH).  With FMT passed as NULL print
 * just the raw dump (in this case ARG_PTR is not used), with FMT
 * being an empty string, print a trailing linefeed, otherwise print
 * an entire debug line with the expanded FMT followed by a possible
 * wrapped hexdump and a final LF.  */
void
_gpgrt_logv_printhex (const void *buffer, size_t length,
                      const char *fmt, va_list arg_ptr)
{
  int wrap = 0;
  int cnt = 0;
  const unsigned char *p;

  /* FIXME: This printing is not yet protected by _gpgrt_flockfile.  */
  if (fmt && *fmt)
    {
      _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG, 0, NULL, NULL, fmt, arg_ptr);
      wrap = 1;
    }

  if (length)
    {
      if (wrap)
        _gpgrt_log_printf (" ");

      for (p = buffer; length--; p++)
        {
          _gpgrt_log_printf ("%02x", *p);
          if (wrap && ++cnt == 32 && length)
            {
              cnt = 0;
              /* (we indicate continuations with a backslash) */
              _gpgrt_log_printf (" \\\n");
              _gpgrt_log_debug ("%s", "");
              if (fmt && *fmt)
                _gpgrt_log_printf (" ");
            }
        }
    }

  if (fmt)
    _gpgrt_log_printf ("\n");
}


/* Print a hexdump of (BUFFER,LENGTH).  With FMT passed as NULL print
 * just the raw dump, with FMT being an empty string, print a trailing
 * linefeed, otherwise print an entire debug line with the expanded
 * FMT followed by the hexdump and a final LF.  */
void
_gpgrt_log_printhex (const void *buffer, size_t length,
                     const char *fmt, ...)
{
  va_list arg_ptr;

  if (fmt)
    {
      va_start (arg_ptr, fmt);
      _gpgrt_logv_printhex (buffer, length, fmt, arg_ptr);
      va_end (arg_ptr);
    }
  else
    {
      /* va_list is not necessary a pointer and thus we can't use NULL
       * because that would conflict with platforms using a straight
       * struct for it (e.g. arm64).  We use a dummy variable instead;
       * the static is a simple way zero it out so to not get
       * complains about uninitialized use.  */
      static va_list dummy_argptr;

      _gpgrt_logv_printhex (buffer, length, NULL, dummy_argptr);
    }
}


/* Print a microsecond timestamp followed by FMT.  */
void
_gpgrt_logv_clock (const char *fmt, va_list arg_ptr)
{
#if ENABLE_LOG_CLOCK
  static unsigned long long initial;
  struct timespec tv;
  unsigned long long now;
  char clockbuf[50];

  if (clock_gettime (CLOCK_REALTIME, &tv))
    {
      _gpgrt_log_debug ("error getting the realtime clock value\n");
      return;
    }
  now = tv.tv_sec * 1000000000ull;
  now += tv.tv_nsec;

  if (!initial)
    initial = now;

  snprintf (clockbuf, sizeof clockbuf, "[%6llu] ", (now - initial)/1000);
  _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG, 0, NULL, clockbuf, fmt, arg_ptr);

#else /*!ENABLE_LOG_CLOCK*/

  /* You may need to link with -ltr to use the above code.  */

  _gpgrt_logv_internal (GPGRT_LOGLVL_DEBUG,
                        0, NULL, "[no clock] ", fmt, arg_ptr);

#endif  /*!ENABLE_LOG_CLOCK*/
}


/* Print a microsecond timestamp followed by FMT.  */
void
_gpgrt_log_clock (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  _gpgrt_logv_clock (fmt, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt__log_assert (const char *expr, const char *file,
                   int line, const char *func)
{
#ifdef GPGRT_HAVE_MACRO_FUNCTION
  _gpgrt_log (GPGRT_LOGLVL_BUG, "Assertion \"%s\" in %s failed (%s:%d)\n",
              expr, func, file, line);
#else /*!GPGRT_HAVE_MACRO_FUNCTION*/
  _gpgrt_log (GPGRT_LOGLVL_BUG, "Assertion \"%s\" failed (%s:%d)\n",
           expr, file, line);
#endif /*!GPGRT_HAVE_MACRO_FUNCTION*/
  _gpgrt_abort (); /* Never called; just to make the compiler happy.  */
}
