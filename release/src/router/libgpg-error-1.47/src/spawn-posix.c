/* exechelp.c - Fork and exec helpers for POSIX
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

#if defined(HAVE_W32_SYSTEM)
#error This code is only used on POSIX
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#include <unistd.h>
#include <fcntl.h>

#include <sys/wait.h>

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif /*HAVE_GETRLIMIT*/

#ifdef HAVE_STAT
# include <sys/stat.h>
#endif

#if __linux__
# include <sys/types.h>
# include <dirent.h>
#endif /*__linux__ */

#include "gpgrt-int.h"


static void
out_of_core (int line)
{
  _gpgrt_log_fatal ("malloc failed at line %d: %s\n",
                    line, _gpg_strerror (_gpg_err_code_from_syserror ()));
  /*NOTREACHED*/
}


/* Return the maximum number of currently allowed open file
 * descriptors.  Only useful on POSIX systems but returns a value on
 * other systems too.  */
static int
get_max_fds (void)
{
  int max_fds = -1;
#ifdef HAVE_GETRLIMIT
  struct rlimit rl;

  /* Under Linux we can figure out the highest used file descriptor by
   * reading /proc/PID/fd.  This is in the common cases much fast than
   * for example doing 4096 close calls where almost all of them will
   * fail.  On a system with a limit of 4096 files and only 8 files
   * open with the highest number being 10, we speedup close_all_fds
   * from 125ms to 0.4ms including readdir.
   *
   * Another option would be to close the file descriptors as returned
   * from reading that directory - however then we need to snapshot
   * that list before starting to close them.  */
#ifdef __linux__
  {
    DIR *dir = NULL;
    struct dirent *dir_entry;
    const char *s;
    int x;

    /* FIXME: Check gpgme on how to do this right on Linux.  */
    dir = opendir ("/proc/self/fd");
    if (dir)
      {
        while ((dir_entry = readdir (dir)))
          {
            s = dir_entry->d_name;
            if ( *s < '0' || *s > '9')
              continue;
            x = atoi (s);
            if (x > max_fds)
              max_fds = x;
          }
        closedir (dir);
      }
    if (max_fds != -1)
      return max_fds + 1;
    }
#endif /* __linux__ */


# ifdef RLIMIT_NOFILE
  if (!getrlimit (RLIMIT_NOFILE, &rl))
    max_fds = rl.rlim_max;
# endif

# ifdef RLIMIT_OFILE
  if (max_fds == -1 && !getrlimit (RLIMIT_OFILE, &rl))
    max_fds = rl.rlim_max;

# endif
#endif /*HAVE_GETRLIMIT*/

#ifdef _SC_OPEN_MAX
  if (max_fds == -1)
    {
      long int scres = sysconf (_SC_OPEN_MAX);
      if (scres >= 0)
        max_fds = scres;
    }
#endif

#ifdef _POSIX_OPEN_MAX
  if (max_fds == -1)
    max_fds = _POSIX_OPEN_MAX;
#endif

#ifdef OPEN_MAX
  if (max_fds == -1)
    max_fds = OPEN_MAX;
#endif

  if (max_fds == -1)
    max_fds = 256;  /* Arbitrary limit.  */

  /* AIX returns INT32_MAX instead of a proper value.  We assume that
     this is always an error and use an arbitrary limit.  */
#ifdef INT32_MAX
  if (max_fds == INT32_MAX)
    max_fds = 256;
#endif

  return max_fds;
}


/* Close all file descriptors starting with descriptor FIRST.  If
 * EXCEPT is not NULL, it is expected to be a list of file descriptors
 * which shall not be closed.  This list shall be sorted in ascending
 * order with the end marked by -1.  */
void
_gpgrt_close_all_fds (int first, int *except)
{
  int max_fd = get_max_fds ();
  int fd, i, except_start;

  if (except)
    {
      except_start = 0;
      for (fd=first; fd < max_fd; fd++)
        {
          for (i=except_start; except[i] != -1; i++)
            {
              if (except[i] == fd)
                {
                  /* If we found the descriptor in the exception list
                     we can start the next compare run at the next
                     index because the exception list is ordered.  */
                except_start = i + 1;
                break;
                }
            }
          if (except[i] == -1)
            close (fd);
        }
    }
  else
    {
      for (fd=first; fd < max_fd; fd++)
        close (fd);
    }

  _gpg_err_set_errno (0);
}


/* Returns an array with all currently open file descriptors.  The end
 * of the array is marked by -1.  The caller needs to release this
 * array using the *standard free* and not with xfree.  This allow the
 * use of this function right at startup even before libgcrypt has
 * been initialized.  Returns NULL on error and sets ERRNO
 * accordingly.
 *
 * FIXME: Needs to be adjusted for use here.
 */
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
#endif /*0*/


/* The exec core used right after the fork.  This will never return.  */
static void
do_exec (const char *pgmname, const char *argv[],
         int fd_in, int fd_out, int fd_err,
         int *except, unsigned int flags)
{
  char **arg_list;
  int i, j;
  int fds[3];
  int nodevnull[3];

  fds[0] = fd_in;
  fds[1] = fd_out;
  fds[2] = fd_err;

  nodevnull[0] = !!(flags & GPGRT_SPAWN_KEEP_STDIN);
  nodevnull[1] = !!(flags & GPGRT_SPAWN_KEEP_STDOUT);
  nodevnull[2] = !!(flags & GPGRT_SPAWN_KEEP_STDERR);

  /* Create the command line argument array.  */
  i = 0;
  if (argv)
    while (argv[i])
      i++;
  arg_list = xtrycalloc (i+2, sizeof *arg_list);
  if (!arg_list)
    out_of_core (__LINE__);
  arg_list[0] = strrchr (pgmname, '/');
  if (arg_list[0])
    arg_list[0]++;
  else
    {
      arg_list[0] = xtrystrdup (pgmname);
      if (!arg_list[0])
        out_of_core (__LINE__);
    }
  if (argv)
    for (i=0,j=1; argv[i]; i++, j++)
      arg_list[j] = (char*)argv[i];

  /* Assign /dev/null to unused FDs. */
  for (i=0; i <= 2; i++)
    {
      if (nodevnull[i])
        continue;
      if (fds[i] == -1)
        {
          fds[i] = open ("/dev/null", i? O_WRONLY : O_RDONLY);
          if (fds[i] == -1)
            _gpgrt_log_fatal ("failed to open '%s': %s\n",
                              "/dev/null", strerror (errno));
        }
    }

  /* Connect the standard files.  */
  for (i=0; i <= 2; i++)
    {
      if (nodevnull[i])
        continue;
      if (fds[i] != i && dup2 (fds[i], i) == -1)
        _gpgrt_log_fatal ("dup2 std%s failed: %s\n",
                          i==0?"in":i==1?"out":"err", strerror (errno));
    }

  /* Close all other files. */
  if (!(flags & GPGRT_SPAWN_INHERIT_FILE))
    _gpgrt_close_all_fds (3, except);

  execv (pgmname, arg_list);
  /* No way to print anything, as we have may have closed all streams. */
  _exit (127);
}


/* Helper for _gpgrt_make_pipe.  */
static gpg_err_code_t
do_create_pipe (int filedes[2])
{
  gpg_error_t err = 0;

  _gpgrt_pre_syscall ();
  if (pipe (filedes) == -1)
    {
      err = _gpg_err_code_from_syserror ();
      filedes[0] = filedes[1] = -1;
    }
  _gpgrt_post_syscall ();

  return err;
}


/* Helper for _gpgrt_make_pipe.  */
static gpg_err_code_t
do_create_pipe_and_estream (int filedes[2], estream_t *r_fp,
                            int outbound, int nonblock)
{
  gpg_err_code_t err;

  _gpgrt_pre_syscall ();
  if (pipe (filedes) == -1)
    {
      err = _gpg_err_code_from_syserror ();
      _gpgrt_log_error (_("error creating a pipe: %s\n"), _gpg_strerror (err));
      filedes[0] = filedes[1] = -1;
      *r_fp = NULL;
      return err;
    }
  _gpgrt_post_syscall ();

  if (!outbound)
    *r_fp = _gpgrt_fdopen (filedes[0], nonblock? "r,nonblock" : "r");
  else
    *r_fp = _gpgrt_fdopen (filedes[1], nonblock? "w,nonblock" : "w");
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
  return 0;
}


/* Create a pipe.  The DIRECTION parameter gives the type of the created pipe:
 *   DIRECTION < 0 := Inbound pipe: On Windows the write end is inheritable.
 *   DIRECTION > 0 := Outbound pipe: On Windows the read end is inheritable.
 * If R_FP is NULL a standard pipe and no stream is created, DIRECTION
 * should then be 0.  */
gpg_err_code_t
_gpgrt_make_pipe (int filedes[2], estream_t *r_fp, int direction,
                  int nonblock)
{
  if (r_fp && direction)
    return do_create_pipe_and_estream (filedes, r_fp,
                                       (direction > 0), nonblock);
  else
    return do_create_pipe (filedes);
}

/*
 * UNION PROCESS_ID:
 *
 * gpgrt_process_t object is an object which represents process handle.
 * It must be same size as pid_t and must have same bit pattern.
 */
union process {
  gpgrt_process_t process_id;
  pid_t pid;
};

static gpgrt_process_t
convert_from_pid (pid_t pid)
{
  union process u;

  u.pid = pid;
  return u.process_id;
}

static pid_t
convert_from_process (gpgrt_process_t process_id)
{
  union process u;

  u.process_id = process_id;
  return u.pid;
}


/* Fork and exec the PGMNAME, see gpgrt-int.h for details.  */
gpg_err_code_t
_gpgrt_spawn_process (const char *pgmname, const char *argv[],
                      int *except, unsigned int flags,
                      estream_t *r_infp,
                      estream_t *r_outfp,
                      estream_t *r_errfp,
                      gpgrt_process_t *r_process_id)
{
  gpg_error_t err;
  int inpipe[2] = {-1, -1};
  int outpipe[2] = {-1, -1};
  int errpipe[2] = {-1, -1};
  estream_t infp = NULL;
  estream_t outfp = NULL;
  estream_t errfp = NULL;
  int nonblock = !!(flags & GPGRT_SPAWN_NONBLOCK);
  pid_t pid;

  if (r_infp)
    *r_infp = NULL;
  if (r_outfp)
    *r_outfp = NULL;
  if (r_errfp)
    *r_errfp = NULL;
  *r_process_id = convert_from_pid (-1);

  if (r_infp)
    {
      err = _gpgrt_create_outbound_pipe (inpipe, &infp, nonblock);
      if (err)
        return err;
    }

  if (r_outfp)
    {
      err = _gpgrt_create_inbound_pipe (outpipe, &outfp, nonblock);
      if (err)
        {
          if (infp)
            _gpgrt_fclose (infp);
          else if (inpipe[1] != -1)
            close (inpipe[1]);
          if (inpipe[0] != -1)
            close (inpipe[0]);

          return err;
        }
    }

  if (r_errfp)
    {
      err = _gpgrt_create_inbound_pipe (errpipe, &errfp, nonblock);
      if (err)
        {
          if (infp)
            _gpgrt_fclose (infp);
          else if (inpipe[1] != -1)
            close (inpipe[1]);
          if (inpipe[0] != -1)
            close (inpipe[0]);

          if (outfp)
            _gpgrt_fclose (outfp);
          else if (outpipe[0] != -1)
            close (outpipe[0]);
          if (outpipe[1] != -1)
            close (outpipe[1]);

          return err;
        }
    }

  _gpgrt_pre_syscall ();
  pid = fork ();
  _gpgrt_post_syscall ();
  if (pid == (pid_t)(-1))
    {
      err = _gpg_err_code_from_syserror ();
      _gpgrt_log_error (_("error forking process: %s\n"), _gpg_strerror (err));

      if (infp)
        _gpgrt_fclose (infp);
      else if (inpipe[1] != -1)
        close (inpipe[1]);
      if (inpipe[0] != -1)
        close (inpipe[0]);

      if (outfp)
        _gpgrt_fclose (outfp);
      else if (outpipe[0] != -1)
        close (outpipe[0]);
      if (outpipe[1] != -1)
        close (outpipe[1]);

      if (errfp)
        _gpgrt_fclose (errfp);
      else if (errpipe[0] != -1)
        close (errpipe[0]);
      if (errpipe[1] != -1)
        close (errpipe[1]);
      return err;
    }

  if (!pid)
    {
      /* This is the child. */
      _gpgrt_fclose (infp);
      _gpgrt_fclose (outfp);
      _gpgrt_fclose (errfp);
      do_exec (pgmname, argv, inpipe[0], outpipe[1], errpipe[1],
               except, flags);
      /*NOTREACHED*/
    }

  /* This is the parent. */
  if (inpipe[0] != -1)
    close (inpipe[0]);
  if (outpipe[1] != -1)
    close (outpipe[1]);
  if (errpipe[1] != -1)
    close (errpipe[1]);

  if (r_infp)
    *r_infp = infp;
  if (r_outfp)
    *r_outfp = outfp;
  if (r_errfp)
    *r_errfp = errfp;

  *r_process_id = convert_from_pid (pid);
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
  gpg_error_t err;
  int ask_inherit_fds = 0;
  pid_t pid;

  *r_process_id = convert_from_pid (-1);
  _gpgrt_pre_syscall ();
  pid = fork ();
  _gpgrt_post_syscall ();
  if (pid == (pid_t)(-1))
    {
      err = _gpg_err_code_from_syserror ();
      _gpgrt_log_error (_("error forking process: %s\n"), _gpg_strerror (err));
      return err;
    }

  if (!pid)
    {
      if (spawn_cb)
        ask_inherit_fds = (*spawn_cb) (spawn_cb_arg);

      /* Run child. */
      do_exec (pgmname, argv, infd, outfd, errfd, NULL,
               ask_inherit_fds? GPGRT_SPAWN_INHERIT_FILE : 0);
      /*NOTREACHED*/
    }

  *r_process_id = convert_from_pid (pid);
  return 0;
}


/* Waiting for child processes.
 *
 * waitpid(2) may return information about terminated children that we
 * did not yet request, and there is no portable way to wait for a
 * specific set of children.
 *
 * As a workaround, we store the results of children for later use.
 *
 * XXX: This assumes that PIDs are not reused too quickly.
 * FIXME: This is not thread-safe.
 */

struct terminated_child
{
  pid_t pid;
  int exitcode;
  struct terminated_child *next;
};

static struct terminated_child *terminated_children;


static gpg_err_code_t
store_result (pid_t pid, int exitcode)
{
  struct terminated_child *c;

  c = xtrymalloc (sizeof *c);
  if (c == NULL)
    return _gpg_err_code_from_syserror ();

  c->pid = pid;
  c->exitcode = exitcode;
  c->next = terminated_children;
  terminated_children = c;

  return 0;
}


static int
get_result (pid_t pid, int *r_exitcode)
{
  struct terminated_child *c, **prevp;

  for (prevp = &terminated_children, c = terminated_children;
       c;
       prevp = &c->next, c = c->next)
    if (c->pid == pid)
      {
        *prevp = c->next;
        *r_exitcode = c->exitcode;
        xfree (c);
        return 1;
      }

  return 0;
}


/* See gpgrt-int.h for a description.  */
gpg_err_code_t
_gpgrt_wait_process (const char *pgmname, gpgrt_process_t process_id, int hang,
                     int *r_exitcode)
{
  gpg_err_code_t ec;
  int i, status;
  pid_t pid;

  pid = convert_from_process (process_id);

  if (r_exitcode)
    *r_exitcode = -1;

  if (pid == (pid_t)(-1))
    return GPG_ERR_INV_VALUE;

  _gpgrt_pre_syscall ();
  while ((i=waitpid (pid, &status, hang? 0:WNOHANG)) == (pid_t)(-1)
	 && errno == EINTR);
  _gpgrt_post_syscall ();

  if (i == (pid_t)(-1))
    {
      ec = _gpg_err_code_from_syserror ();
      _gpgrt_log_error (_("waiting for process %d to terminate failed: %s\n"),
                        (int)pid, _gpg_strerror (ec));
    }
  else if (!i)
    {
      ec = GPG_ERR_TIMEOUT; /* Still running.  */
    }
  else if (WIFEXITED (status) && WEXITSTATUS (status) == 127)
    {
      /* FIXME: This is GnuPG specific.  */
      _gpgrt_log_error (_("error running '%s': probably not installed\n"),
                        pgmname);
      ec = GPG_ERR_CONFIGURATION;
    }
  else if (WIFEXITED (status) && WEXITSTATUS (status))
    {
      if (!r_exitcode)
        _gpgrt_log_error (_("error running '%s': exit status %d\n"), pgmname,
                          WEXITSTATUS (status));
      else
        *r_exitcode = WEXITSTATUS (status);
      ec = GPG_ERR_GENERAL;
    }
  else if (!WIFEXITED (status))
    {
      _gpgrt_log_error (_("error running '%s': terminated\n"), pgmname);
      ec = GPG_ERR_GENERAL;
    }
  else
    {
      if (r_exitcode)
        *r_exitcode = 0;
      ec = 0;
    }

  return ec;
}


/* See gpgrt-int.h for a description.
 *
 * FIXME: What about using a poll like data structure for the pids and
 * their exit codes?  The whole thing is anyway problematic in a
 * threaded processs because waitpid has no association between PIDS
 * and threads.
 */
gpg_err_code_t
_gpgrt_wait_processes (const char **pgmnames, gpgrt_process_t *process_ids,
                       size_t count, int hang, int *r_exitcodes)
{
  gpg_err_code_t ec = 0;
  size_t i, left;
  int *dummy = NULL;
  pid_t pid;

  if (!r_exitcodes)
    {
      dummy = r_exitcodes = xtrymalloc (sizeof *r_exitcodes * count);
      if (!dummy)
        return _gpg_err_code_from_syserror ();
    }

  for (i = 0, left = count; i < count; i++)
    {
      int status = -1;

      pid = convert_from_process (process_ids[i]);

      /* Skip invalid PID.  */
      if (pid == (pid_t)(-1))
        {
          r_exitcodes[i] = -1;
          left -= 1;
          continue;
        }

      /* See if there was a previously stored result for this pid.  */
      if (get_result (pid, &status))
        left -= 1;

      r_exitcodes[i] = status;
    }

  while (left > 0)
    {
      pid_t pid0;
      int status;

      _gpgrt_pre_syscall ();
      while ((pid0 = waitpid (-1, &status, hang ? 0 : WNOHANG)) == (pid_t)(-1)
             && errno == EINTR);
      _gpgrt_post_syscall ();

      if (pid0 == (pid_t)(-1))
        {
          ec = _gpg_err_code_from_syserror ();
          _gpgrt_log_error (_("waiting for processes to terminate"
                              " failed: %s\n"), _gpg_strerror (ec));
          break;
        }
      else if (!pid0)
        {
          ec = GPG_ERR_TIMEOUT; /* Still running.  */
          break;
        }
      else
        {
          for (i = 0; i < count; i++)
            {
              pid = convert_from_process (process_ids[i]);
              if (pid0 == pid)
                break;
            }

          if (i == count)
            {
              /* No match, store this result.  */
              ec = store_result (pid0, status);
              if (ec)
                break;
              continue;
            }

          /* Process PROCESS_PIDS[i] died.  */
          if (r_exitcodes[i] != (pid_t) -1)
            {
              _gpgrt_log_error ("PID %d was reused", (int)pid0);
              ec = GPG_ERR_GENERAL;
              break;
            }

          left -= 1;
          r_exitcodes[i] = status;
        }
    }

  for (i = 0; i < count; i++)
    {
      if (r_exitcodes[i] == -1)
        continue;

      if (WIFEXITED (r_exitcodes[i]) && WEXITSTATUS (r_exitcodes[i]) == 127)
        {
          _gpgrt_log_error (_("error running '%s': probably not installed\n"),
                            pgmnames[i]);
          ec = GPG_ERR_CONFIGURATION;
        }
      else if (WIFEXITED (r_exitcodes[i]) && WEXITSTATUS (r_exitcodes[i]))
        {
          if (dummy)
            _gpgrt_log_error (_("error running '%s': exit status %d\n"),
                              pgmnames[i], WEXITSTATUS (r_exitcodes[i]));
          else
            r_exitcodes[i] = WEXITSTATUS (r_exitcodes[i]);
          ec = GPG_ERR_GENERAL;
        }
      else if (!WIFEXITED (r_exitcodes[i]))
        {
          _gpgrt_log_error (_("error running '%s': terminated\n"), pgmnames[i]);
          ec = GPG_ERR_GENERAL;
        }
    }

  xfree (dummy);
  return ec;
}


/* See gpgrt-int.h for a description.  FIXME: We should add a prexec
 * callback. */
gpg_err_code_t
_gpgrt_spawn_process_detached (const char *pgmname, const char *argv[],
                               const char *envp[])
{
  gpg_err_code_t ec;
  pid_t pid;
  int i;

  /* FIXME: Is this GnuPG specific or should we keep it.  */
  if (getuid() != geteuid())
    return GPG_ERR_BUG;

  if (access (pgmname, X_OK))
    return _gpg_err_code_from_syserror ();

  _gpgrt_pre_syscall ();
  pid = fork ();
  _gpgrt_post_syscall ();
  if (pid == (pid_t)(-1))
    {
      ec = _gpg_err_code_from_syserror ();
      _gpgrt_log_error (_("error forking process: %s\n"), _gpg_strerror (ec));
      return ec;
    }

  if (!pid)
    {
      pid_t pid2;

      /* gcry_control (GCRYCTL_TERM_SECMEM); */
      if (setsid() == -1 || chdir ("/"))
        _exit (1);

      pid2 = fork (); /* Double fork to let init take over the new child. */
      if (pid2 == (pid_t)(-1))
        _exit (1);
      if (pid2)
        _exit (0);  /* Let the parent exit immediately. */

      for (i=0; envp && envp[i]; i++)
        {
          char *p = xtrystrdup (envp[i]);
          if (!p)
            out_of_core (__LINE__);
          putenv (p);
        }

      do_exec (pgmname, argv, -1, -1, -1, NULL, 0);
      /*NOTREACHED*/
    }

  _gpgrt_pre_syscall ();
  if (waitpid (pid, NULL, 0) == -1)
    {
      _gpgrt_post_syscall ();
      ec = _gpg_err_code_from_syserror ();
      _gpgrt_log_error ("waitpid failed in gpgrt_spawn_process_detached: %s",
                        _gpg_strerror (ec));
      return ec;
    }
  else
    _gpgrt_post_syscall ();

  return 0;
}


/* Kill a process; that is send an appropriate signal to the process.
 * gnupg_wait_process must be called to actually remove the process
   from the system.  An invalid PID is ignored.  */
void
_gpgrt_kill_process (gpgrt_process_t process_id)
{
  pid_t pid;

  pid = convert_from_process (process_id);
  if (pid != (pid_t)(-1))
    {
      _gpgrt_pre_syscall ();
      kill (pid, SIGTERM);
      _gpgrt_post_syscall ();
    }
}


void
_gpgrt_release_process (gpgrt_process_t process_id)
{
  (void)process_id;
}
