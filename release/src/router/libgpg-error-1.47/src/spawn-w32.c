/* spawn-w32.c - Fork and exec helpers for W32.
 * Copyright (C) 2004, 2007-2009, 2010 Free Software Foundation, Inc.
 * Copyright (C) 2004, 2006-2012, 2014-2017 g10 Code GmbH
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

#if !defined(HAVE_W32_SYSTEM)
#error This code is only used on W32.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_STAT
# include <sys/stat.h>
#endif
#define WIN32_LEAN_AND_MEAN  /* We only need the OS core stuff.  */
#include <windows.h>

#include "gpgrt-int.h"

/* Define to 1 do enable debugging.  */
#define DEBUG_W32_SPAWN 0


/* It seems Vista doesn't grok X_OK and so fails access() tests.
 * Previous versions interpreted X_OK as F_OK anyway, so we'll just
 * use F_OK directly. */
#undef X_OK
#define X_OK F_OK

/* For HANDLE and the internal file descriptor (fd) of this module:
 * HANDLE can be represented by an intptr_t which should be true for
 * all systems (HANDLE is defined as void *).  Further, we assume that
 * -1 denotes an invalid handle.
 *
 * Note that a C run-time file descriptor (the exposed one to API) is
 * always represented by an int.
 */
#define fd_to_handle(a)  ((HANDLE)(a))
#define handle_to_fd(a)  ((intptr_t)(a))


/* Return the maximum number of currently allowed open file
 * descriptors.  Only useful on POSIX systems but returns a value on
 * other systems too.  */
int
get_max_fds (void)
{
  int max_fds = -1;

#ifdef OPEN_MAX
  if (max_fds == -1)
    max_fds = OPEN_MAX;
#endif

  if (max_fds == -1)
    max_fds = 256;  /* Arbitrary limit.  */

  return max_fds;
}


/* Under Windows this is a dummy function.  */
/* static void */
/* close_all_fds (int first, int *except) */
/* { */
/*   (void)first; */
/*   (void)except; */
/* } */


/* Returns an array with all currently open file descriptors.  The end
 * of the array is marked by -1.  The caller needs to release this
 * array using the *standard free* and not with xfree.  This allow the
 * use of this function right at startup even before libgcrypt has
 * been initialized.  Returns NULL on error and sets ERRNO
 * accordingly.  Note that fstat prints a warning to DebugView for all
 * invalid fds which is a bit annoying.  We actually do not need this
 * function in real code (close_all_fds is a dummy anyway) but we keep
 * it for use by t-exechelp.c.  */
#if 0
int *
get_all_open_fds (void)
{
  int *array;
  size_t narray;
  int fd, max_fd, idx;
#ifndef HAVE_STAT
  array = calloc (1, sizeof *array);
  if (array)
    array[0] = -1;
#else /*HAVE_STAT*/
  struct stat statbuf;

  max_fd = get_max_fds ();
  narray = 32;  /* If you change this change also t-exechelp.c.  */
  array = calloc (narray, sizeof *array);
  if (!array)
    return NULL;

  /* Note:  The list we return is ordered.  */
  for (idx=0, fd=0; fd < max_fd; fd++)
    if (!(fstat (fd, &statbuf) == -1 && errno == EBADF))
      {
        if (idx+1 >= narray)
          {
            int *tmp;

            narray += (narray < 256)? 32:256;
            tmp = realloc (array, narray * sizeof *array);
            if (!tmp)
              {
                free (array);
                return NULL;
              }
            array = tmp;
          }
        array[idx++] = fd;
      }
  array[idx] = -1;
#endif /*HAVE_STAT*/
  return array;
}
#endif


/* Helper function to build_w32_commandline. */
static char *
build_w32_commandline_copy (char *buffer, const char *string)
{
  char *p = buffer;
  const char *s;

  if (!*string) /* Empty string. */
    p = stpcpy (p, "\"\"");
  else if (strpbrk (string, " \t\n\v\f\""))
    {
      /* Need to do some kind of quoting.  */
      p = stpcpy (p, "\"");
      for (s=string; *s; s++)
        {
          *p++ = *s;
          if (*s == '\"')
            *p++ = *s;
        }
      *p++ = '\"';
      *p = 0;
    }
  else
    p = stpcpy (p, string);

  return p;
}


/* Build a command line for use with W32's CreateProcess.  On success
 * CMDLINE gets the address of a newly allocated string.  */
static gpg_err_code_t
build_w32_commandline (const char *pgmname, const char * const *argv,
                       char **cmdline)
{
  int i, n;
  const char *s;
  char *buf, *p;

  *cmdline = NULL;
  n = 0;
  s = pgmname;
  n += strlen (s) + 1 + 2;  /* (1 space, 2 quoting */
  for (; *s; s++)
    if (*s == '\"')
      n++;  /* Need to double inner quotes.  */
  for (i=0; (s=argv[i]); i++)
    {
      n += strlen (s) + 1 + 2;  /* (1 space, 2 quoting */
      for (; *s; s++)
        if (*s == '\"')
          n++;  /* Need to double inner quotes.  */
    }
  n++;

  buf = p = xtrymalloc (n);
  if (!buf)
    return _gpg_err_code_from_syserror ();

  p = build_w32_commandline_copy (p, pgmname);
  for (i=0; argv[i]; i++)
    {
      *p++ = ' ';
      p = build_w32_commandline_copy (p, argv[i]);
    }

  *cmdline= buf;
  return 0;
}


#define INHERIT_READ	1
#define INHERIT_WRITE	2
#define INHERIT_BOTH	(INHERIT_READ|INHERIT_WRITE)

/* Create pipe.  FLAGS indicates which ends are inheritable.  */
static int
create_inheritable_pipe (HANDLE filedes[2], int flags)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES sec_attr;

  memset (&sec_attr, 0, sizeof sec_attr );
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = TRUE;

  _gpgrt_pre_syscall ();
  if (!CreatePipe (&r, &w, &sec_attr, 0))
    {
      _gpgrt_post_syscall ();
      return -1;
    }
  _gpgrt_post_syscall ();

  if ((flags & INHERIT_READ) == 0)
    if (! SetHandleInformation (r, HANDLE_FLAG_INHERIT, 0))
      goto fail;

  if ((flags & INHERIT_WRITE) == 0)
    if (! SetHandleInformation (w, HANDLE_FLAG_INHERIT, 0))
      goto fail;

  filedes[0] = r;
  filedes[1] = w;
  return 0;

 fail:
  _gpgrt_log_error ("SetHandleInformation failed: ec=%d\n",
                    (int)GetLastError ());
  CloseHandle (r);
  CloseHandle (w);
  return -1;
}


static HANDLE
w32_open_null (int for_write)
{
  HANDLE hfile;

  hfile = CreateFileW (L"nul",
                       for_write? GENERIC_WRITE : GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL);
  if (hfile == INVALID_HANDLE_VALUE)
    _gpgrt_log_debug ("can't open 'nul': ec=%d\n", (int)GetLastError ());
  return hfile;
}


static gpg_err_code_t
do_create_pipe_and_estream (int filedes[2],
                            estream_t *r_fp, int direction, int nonblock)
{
  gpg_err_code_t err = 0;
  int flags;
  HANDLE fds[2];
  gpgrt_syshd_t syshd;

  if (direction < 0)
    flags = INHERIT_WRITE;
  else if (direction > 0)
    flags = INHERIT_READ;
  else
    flags = INHERIT_BOTH;

  filedes[0] = filedes[1] = -1;
  err = GPG_ERR_GENERAL;
  if (!create_inheritable_pipe (fds, flags))
    {
      filedes[0] = _open_osfhandle (handle_to_fd (fds[0]), O_RDONLY);
      if (filedes[0] == -1)
        {
          _gpgrt_log_error ("failed to translate osfhandle %p\n", fds[0]);
          CloseHandle (fds[1]);
        }
      else
        {
          filedes[1] = _open_osfhandle (handle_to_fd (fds[1]), O_APPEND);
          if (filedes[1] == -1)
            {
              _gpgrt_log_error ("failed to translate osfhandle %p\n", fds[1]);
              close (filedes[0]);
              filedes[0] = -1;
              CloseHandle (fds[1]);
            }
          else
            err = 0;
        }
    }

  if (! err && r_fp)
    {
      syshd.type = ES_SYSHD_HANDLE;
      if (direction < 0)
        {
          syshd.u.handle = fds[0];
          *r_fp = _gpgrt_sysopen (&syshd, nonblock? "r,nonblock" : "r");
        }
      else
        {
          syshd.u.handle = fds[1];
          *r_fp = _gpgrt_sysopen (&syshd, nonblock? "w,nonblock" : "w");
        }
      if (!*r_fp)
        {
          err = _gpg_err_code_from_syserror ();
          _gpgrt_log_error (_("error creating a stream for a pipe: %s\n"),
                            _gpg_strerror (err));
          close (filedes[0]);
          close (filedes[1]);
          filedes[0] = filedes[1] = -1;
          return err;
        }
    }

  return err;
}


/* Create a pipe.  The DIRECTION parameter gives the type of the created pipe:
 *   DIRECTION < 0 := Inbound pipe: On Windows the write end is inheritable.
 *   DIRECTION > 0 := Outbound pipe: On Windows the read end is inheritable.
 * If R_FP is NULL a standard pipe and no stream is created, DIRECTION
 * should then be 0.  */
gpg_err_code_t
_gpgrt_make_pipe (int filedes[2], estream_t *r_fp, int direction, int nonblock)
{
  if (r_fp && direction)
    return do_create_pipe_and_estream (filedes, r_fp, direction, nonblock);
  else
    return do_create_pipe_and_estream (filedes, NULL, 0, 0);
}


/*
 * UNION PROCESS_ID:
 *
 * gpgrt_process_t is an object which represents process handle.
 * It must be same size as HANDLE and must have same bit pattern.
 */
union process {
  gpgrt_process_t process_id;
  HANDLE process_handle;
};

static gpgrt_process_t
convert_from_handle (HANDLE process_handle)
{
  union process u;

  u.process_handle = process_handle;
  return u.process_id;
}

static HANDLE
convert_from_process (gpgrt_process_t process_id)
{
  union process u;

  u.process_id = process_id;
  return u.process_handle;
}


/* Fork and exec the PGMNAME, see gpgrt-int.h for details.  */
gpg_err_code_t
_gpgrt_spawn_process (const char *pgmname, const char *argv[],
                      int *except, unsigned int flags,
                      estream_t *r_infp, estream_t *r_outfp, estream_t *r_errfp,
                      gpgrt_process_t *r_process_id)
{
  gpg_err_code_t err;
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi =
    {
      NULL,      /* Returns process handle.  */
      0,         /* Returns primary thread handle.  */
      0,         /* Returns pid.  */
      0          /* Returns tid.  */
    };
  STARTUPINFO si;
  int cr_flags;
  char *cmdline;
  HANDLE inpipe[2]  = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  HANDLE outpipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  HANDLE errpipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  estream_t infp = NULL;
  estream_t outfp = NULL;
  estream_t errfp = NULL;
  HANDLE nullhd[3] = {INVALID_HANDLE_VALUE,
                      INVALID_HANDLE_VALUE,
                      INVALID_HANDLE_VALUE};
  int i;
  es_syshd_t syshd;
  int nonblock = !!(flags & GPGRT_SPAWN_NONBLOCK);
  int ret;

  (void)except; /* Not yet used.  */

  if (r_infp)
    *r_infp = NULL;
  if (r_outfp)
    *r_outfp = NULL;
  if (r_errfp)
    *r_errfp = NULL;
  *r_process_id = convert_from_handle (INVALID_HANDLE_VALUE);

  if (r_infp)
    {
      if (create_inheritable_pipe (inpipe, INHERIT_READ))
        {
          err = GPG_ERR_GENERAL;
          _gpgrt_log_error (_("error creating a pipe: %s\n"),
                            _gpg_strerror (err));
          return err;
        }

      syshd.type = ES_SYSHD_HANDLE;
      syshd.u.handle = inpipe[1];
      infp = _gpgrt_sysopen (&syshd, nonblock? "w,nonblock" : "w");
      if (!infp)
        {
          err = _gpg_err_code_from_syserror ();
          _gpgrt_log_error (_("error creating a stream for a pipe: %s\n"),
                            _gpg_strerror (err));
          CloseHandle (inpipe[0]);
          CloseHandle (inpipe[1]);
          inpipe[0] = inpipe[1] = INVALID_HANDLE_VALUE;
          return err;
        }
    }

  if (r_outfp)
    {
      if (create_inheritable_pipe (outpipe, INHERIT_WRITE))
        {
          err = GPG_ERR_GENERAL;
          _gpgrt_log_error (_("error creating a pipe: %s\n"),
                            _gpg_strerror (err));
          return err;
        }

      syshd.type = ES_SYSHD_HANDLE;
      syshd.u.handle = outpipe[0];
      outfp = _gpgrt_sysopen (&syshd, nonblock? "r,nonblock" : "r");
      if (!outfp)
        {
          err = _gpg_err_code_from_syserror ();
          _gpgrt_log_error (_("error creating a stream for a pipe: %s\n"),
                            _gpg_strerror (err));
          CloseHandle (outpipe[0]);
          CloseHandle (outpipe[1]);
          outpipe[0] = outpipe[1] = INVALID_HANDLE_VALUE;
          if (infp)
            _gpgrt_fclose (infp);
          else if (inpipe[1] != INVALID_HANDLE_VALUE)
            CloseHandle (inpipe[1]);
          if (inpipe[0] != INVALID_HANDLE_VALUE)
            CloseHandle (inpipe[0]);
          return err;
        }
    }

  if (r_errfp)
    {
      if (create_inheritable_pipe (errpipe, INHERIT_WRITE))
        {
          err = GPG_ERR_GENERAL;
          _gpgrt_log_error (_("error creating a pipe: %s\n"),
                            _gpg_strerror (err));
          return err;
        }

      syshd.type = ES_SYSHD_HANDLE;
      syshd.u.handle = errpipe[0];
      errfp = _gpgrt_sysopen (&syshd, nonblock? "r,nonblock" : "r");
      if (!errfp)
        {
          err = _gpg_err_code_from_syserror ();
          _gpgrt_log_error (_("error creating a stream for a pipe: %s\n"),
                            _gpg_strerror (err));
          CloseHandle (errpipe[0]);
          CloseHandle (errpipe[1]);
          errpipe[0] = errpipe[1] = INVALID_HANDLE_VALUE;
          if (outfp)
            _gpgrt_fclose (outfp);
          else if (outpipe[0] != INVALID_HANDLE_VALUE)
            CloseHandle (outpipe[0]);
          if (outpipe[1] != INVALID_HANDLE_VALUE)
            CloseHandle (outpipe[1]);
          if (infp)
            _gpgrt_fclose (infp);
          else if (inpipe[1] != INVALID_HANDLE_VALUE)
            CloseHandle (inpipe[1]);
          if (inpipe[0] != INVALID_HANDLE_VALUE)
            CloseHandle (inpipe[0]);
          return err;
        }
    }

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr );
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  /* Build the command line.  */
  err = build_w32_commandline (pgmname, argv, &cmdline);
  if (err)
    return err;

  if (inpipe[0] == INVALID_HANDLE_VALUE)
    nullhd[0] = ((flags & GPGRT_SPAWN_KEEP_STDIN)?
                 GetStdHandle (STD_INPUT_HANDLE) : w32_open_null (0));
   if (outpipe[1] == INVALID_HANDLE_VALUE)
    nullhd[1] = ((flags & GPGRT_SPAWN_KEEP_STDOUT)?
                 GetStdHandle (STD_OUTPUT_HANDLE) : w32_open_null (1));
   if (errpipe[1] == INVALID_HANDLE_VALUE)
    nullhd[2] = ((flags & GPGRT_SPAWN_KEEP_STDOUT)?
                 GetStdHandle (STD_ERROR_HANDLE) : w32_open_null (1));

  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.wShowWindow = DEBUG_W32_SPAWN? SW_SHOW : SW_MINIMIZE;
  si.hStdInput  = inpipe[0]  == INVALID_HANDLE_VALUE? nullhd[0] : inpipe[0];
  si.hStdOutput = outpipe[1] == INVALID_HANDLE_VALUE? nullhd[1] : outpipe[1];
  si.hStdError  = errpipe[1] == INVALID_HANDLE_VALUE? nullhd[2] : errpipe[1];

  cr_flags = (CREATE_DEFAULT_ERROR_MODE
              | ((flags & GPGRT_SPAWN_DETACHED)? DETACHED_PROCESS : 0)
              | GetPriorityClass (GetCurrentProcess ())
              | CREATE_SUSPENDED);
  _gpgrt_log_debug ("CreateProcess, path='%s' cmdline='%s'\n",
                    pgmname, cmdline);
  ret = CreateProcess (pgmname,     /* Program to start.  */
                      cmdline,       /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      cr_flags,      /* Creation flags.  */
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                        );
  if (!ret)
    {
      _gpgrt_log_error ("CreateProcess failed: ec=%d\n", (int)GetLastError ());
      xfree (cmdline);
      if (infp)
        _gpgrt_fclose (infp);
      else if (inpipe[1] != INVALID_HANDLE_VALUE)
        CloseHandle (outpipe[1]);
      if (inpipe[0] != INVALID_HANDLE_VALUE)
        CloseHandle (inpipe[0]);
      if (outfp)
        _gpgrt_fclose (outfp);
      else if (outpipe[0] != INVALID_HANDLE_VALUE)
        CloseHandle (outpipe[0]);
      if (outpipe[1] != INVALID_HANDLE_VALUE)
        CloseHandle (outpipe[1]);
      if (errfp)
        _gpgrt_fclose (errfp);
      else if (errpipe[0] != INVALID_HANDLE_VALUE)
        CloseHandle (errpipe[0]);
      if (errpipe[1] != INVALID_HANDLE_VALUE)
        CloseHandle (errpipe[1]);
      return GPG_ERR_GENERAL;
    }
  xfree (cmdline);
  cmdline = NULL;

  /* Close the inherited handles to /dev/null.  */
  for (i=0; i < DIM (nullhd); i++)
    if (nullhd[i] != INVALID_HANDLE_VALUE)
      CloseHandle (nullhd[i]);

  /* Close the inherited ends of the pipes.  */
  if (inpipe[0] != INVALID_HANDLE_VALUE)
    CloseHandle (inpipe[0]);
  if (outpipe[1] != INVALID_HANDLE_VALUE)
    CloseHandle (outpipe[1]);
  if (errpipe[1] != INVALID_HANDLE_VALUE)
    CloseHandle (errpipe[1]);

  _gpgrt_log_debug ("CreateProcess ready: hProcess=%p hThread=%p"
                    " dwProcessID=%d dwThreadId=%d\n",
                    pi.hProcess, pi.hThread,
                    (int) pi.dwProcessId, (int) pi.dwThreadId);
  _gpgrt_log_debug ("                     outfp=%p errfp=%p\n", outfp, errfp);

  if ((flags & GPGRT_SPAWN_RUN_ASFW))
    {
      /* Fixme: For unknown reasons AllowSetForegroundWindow returns
       * an invalid argument error if we pass it the correct
       * processID.  As a workaround we use -1 (ASFW_ANY).  */
      if (!AllowSetForegroundWindow (ASFW_ANY /*pi.dwProcessId*/))
        _gpgrt_log_info ("AllowSetForegroundWindow() failed: ec=%d\n",
                         (int)GetLastError ());
    }

  /* Process has been created suspended; resume it now. */
  _gpgrt_pre_syscall ();
  ResumeThread (pi.hThread);
  CloseHandle (pi.hThread);
  _gpgrt_post_syscall ();

  if (r_infp)
    *r_infp = infp;
  if (r_outfp)
    *r_outfp = outfp;
  if (r_errfp)
    *r_errfp = errfp;

  *r_process_id = convert_from_handle (pi.hProcess);
  return 0;
}


/* Fork and exec the PGMNAME using FDs, see gpgrt-int.h for details.  */
gpg_err_code_t
_gpgrt_spawn_process_fd (const char *pgmname, const char *argv[],
                         int infd, int outfd, int errfd,
                         int (*spawn_cb) (void *),
                         void *spawn_cb_arg,
                         gpgrt_process_t *r_process_id)
{
  gpg_err_code_t err;
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi = { NULL, 0, 0, 0 };
  STARTUPINFO si;
  char *cmdline;
  int ret, i;
  HANDLE stdhd[3];
  int ask_inherit = 0;

  if (spawn_cb)
    ask_inherit = (*spawn_cb) (spawn_cb_arg);

  /* Setup return values.  */
  *r_process_id = convert_from_handle (INVALID_HANDLE_VALUE);

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr );
  sec_attr.nLength = sizeof sec_attr;

  if (ask_inherit)
    sec_attr.bInheritHandle = TRUE;
  else
    sec_attr.bInheritHandle = FALSE;

  /* Build the command line.  */
  err = build_w32_commandline (pgmname, argv, &cmdline);
  if (err)
    return err;

  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.wShowWindow = DEBUG_W32_SPAWN? SW_SHOW : SW_MINIMIZE;
  stdhd[0] = infd  == -1? w32_open_null (0) : INVALID_HANDLE_VALUE;
  stdhd[1] = outfd == -1? w32_open_null (1) : INVALID_HANDLE_VALUE;
  stdhd[2] = errfd == -1? w32_open_null (1) : INVALID_HANDLE_VALUE;
  si.hStdInput  = infd  == -1? stdhd[0] : (void*)_get_osfhandle (infd);
  si.hStdOutput = outfd == -1? stdhd[1] : (void*)_get_osfhandle (outfd);
  si.hStdError  = errfd == -1? stdhd[2] : (void*)_get_osfhandle (errfd);

  _gpgrt_log_debug ("CreateProcess, path='%s' cmdline='%s'\n",
                    pgmname, cmdline);
  ret = CreateProcess (pgmname,      /* Program to start.  */
                      cmdline,       /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      (CREATE_DEFAULT_ERROR_MODE
                       | GetPriorityClass (GetCurrentProcess ())
                       | CREATE_SUSPENDED | DETACHED_PROCESS),
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                      );
  if (!ret)
    {
      _gpgrt_log_error ("CreateProcess failed: ec=%d\n", (int)GetLastError ());
      err = GPG_ERR_GENERAL;
    }
  else
    err = 0;

  xfree (cmdline);

  for (i=0; i < 3; i++)
    if (stdhd[i] != INVALID_HANDLE_VALUE)
      CloseHandle (stdhd[i]);

  if (err)
    return err;

  _gpgrt_log_debug ("CreateProcess ready: hProcess=%p hThread=%p"
                    " dwProcessID=%d dwThreadId=%d\n",
                    pi.hProcess, pi.hThread,
                    (int) pi.dwProcessId, (int) pi.dwThreadId);

  /* Process has been created suspended; resume it now. */
  ResumeThread (pi.hThread);
  CloseHandle (pi.hThread);

  *r_process_id = convert_from_handle (pi.hProcess);
  return 0;
}


/* See gpgrt-int.h for a description.  */
gpg_err_code_t
_gpgrt_wait_process (const char *pgmname, gpgrt_process_t process_id,
                     int hang, int *r_exitcode)
{
  return _gpgrt_wait_processes (&pgmname, &process_id, 1, hang, r_exitcode);
}


/* See gpgrt-int.h for a description.  */
gpg_err_code_t
_gpgrt_wait_processes (const char **pgmnames, gpgrt_process_t *process_ids,
                       size_t count, int hang, int *r_exitcodes)
{
  gpg_err_code_t ec = 0;
  size_t i;
  HANDLE *procs;
  int code;

  procs = xtrycalloc (count, sizeof *procs);
  if (procs == NULL)
    return _gpg_err_code_from_syserror ();

  for (i = 0; i < count; i++)
    {
      HANDLE process_handle = convert_from_process (process_ids[i]);

      if (r_exitcodes)
        r_exitcodes[i] = -1;

      if (process_handle == INVALID_HANDLE_VALUE)
        return GPG_ERR_INV_VALUE;

      procs[i] = process_handle;
    }

  _gpgrt_pre_syscall ();
  code = WaitForMultipleObjects (count, procs, TRUE, hang? INFINITE : 0);
  _gpgrt_post_syscall ();
  switch (code)
    {
    case WAIT_TIMEOUT:
      ec = GPG_ERR_TIMEOUT;
      goto leave;

    case WAIT_FAILED:
      _gpgrt_log_error (_("waiting for processes to terminate failed: ec=%d\n"),
                        (int)GetLastError ());
      ec = GPG_ERR_GENERAL;
      goto leave;

    case WAIT_OBJECT_0:
      for (i = 0; i < count; i++)
        {
          DWORD exc;

          if (! GetExitCodeProcess (procs[i], &exc))
            {
              _gpgrt_log_error (_("error getting exit code of process %p:"
                                  " ec=%d\n"),
                                process_ids[i], (int)GetLastError ());
              ec = GPG_ERR_GENERAL;
            }
          else if (exc)
            {
              if (!r_exitcodes)
                _gpgrt_log_error (_("error running '%s': exit status %d\n"),
                                  pgmnames[i], (int)exc);
              else
                r_exitcodes[i] = (int)exc;
              ec = GPG_ERR_GENERAL;
            }
          else
            {
              if (r_exitcodes)
                r_exitcodes[i] = 0;
            }
        }
      break;

    default:
      _gpgrt_log_debug ("WaitForMultipleObjects returned unexpected code %d\n",
                        code);
      ec = GPG_ERR_GENERAL;
      break;
    }

 leave:
  return ec;
}


/* See gpgrt-int.h for a description.  */
gpg_err_code_t
_gpgrt_spawn_process_detached (const char *pgmname, const char *argv[],
                               const char *envp[])
{
  gpg_err_code_t err;
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi =
    {
      NULL,      /* Returns process handle.  */
      0,         /* Returns primary thread handle.  */
      0,         /* Returns pid.  */
      0          /* Returns tid.  */
    };
  STARTUPINFO si;
  int cr_flags;
  char *cmdline;
  int ret;
  gpg_err_code_t ec;

  /* We don't use ENVP.  */
  (void)envp;

  ec = _gpgrt_access (pgmname, X_OK);
  if (ec)
    return ec;

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr );
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  /* Build the command line.  */
  err = build_w32_commandline (pgmname, argv, &cmdline);
  if (err)
    return err;

  /* Start the process.  */
  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = DEBUG_W32_SPAWN? SW_SHOW : SW_MINIMIZE;

  cr_flags = (CREATE_DEFAULT_ERROR_MODE
              | GetPriorityClass (GetCurrentProcess ())
              | CREATE_NEW_PROCESS_GROUP
              | DETACHED_PROCESS);
  _gpgrt_log_debug ("CreateProcess(detached), path='%s' cmdline='%s'\n",
                    pgmname, cmdline);
  ret = CreateProcess (pgmname,       /* Program to start.  */
                      cmdline,       /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      FALSE,         /* Inherit handles.  */
                      cr_flags,      /* Creation flags.  */
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                       );
  if (!ret)
    {
      _gpgrt_log_error ("CreateProcess(detached) failed: ec=%d\n",
                        (int)GetLastError ());
      xfree (cmdline);
      return GPG_ERR_GENERAL;
    }
  xfree (cmdline);
  cmdline = NULL;

  _gpgrt_log_debug ("CreateProcess(detached) ready: hProcess=%p hThread=%p"
                    " dwProcessID=%d dwThreadId=%d\n",
                    pi.hProcess, pi.hThread,
                    (int) pi.dwProcessId, (int) pi.dwThreadId);

  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);

  return 0;
}


/* Kill a process; that is send an appropriate signal to the process.
   gnupg_wait_process must be called to actually remove the process
   from the system.  An invalid PID is ignored.  */
void
_gpgrt_kill_process (gpgrt_process_t process_id)
{
  HANDLE process_handle = convert_from_process (process_id);

  if (process_handle != INVALID_HANDLE_VALUE)
    {
      /* Arbitrary error code.  */
      _gpgrt_pre_syscall ();
      TerminateProcess (process_handle, 1);
      _gpgrt_post_syscall ();
    }
}


void
_gpgrt_release_process (gpgrt_process_t process_id)
{
  HANDLE process_handle = convert_from_process (process_id);

  if (process_handle != INVALID_HANDLE_VALUE)
    CloseHandle (process_handle);
}

void
_gpgrt_close_all_fds (int from, int *keep_fds)
{
  (void)from;
  (void)keep_fds;
}
