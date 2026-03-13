/* Auxiliary functions for the creation of subprocesses.  OS/2 kLIBC API.
   Copyright (C) 2001, 2003-2024 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "os2-spawn.h"

/* Get _open_osfhandle().  */
#include <io.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <process.h>
#include <dlfcn.h>
#if HAVE_LIBCX_SPAWN2_H
# include <libcx/spawn2.h>
#endif

#include "cloexec.h"
#include <error.h>
#include "gettext.h"

#define _(str) gettext (str)


/* Duplicates a file handle, making the copy uninheritable.
   Returns -1 for a file handle that is equivalent to closed.  */
static int
dup_noinherit (int fd)
{
  fd = dup_cloexec (fd);
  if (fd < 0 && errno == EMFILE)
    error (EXIT_FAILURE, errno, _("_open_osfhandle failed"));

  return fd;
}

/* Returns a file descriptor equivalent to FD, except that the resulting file
   descriptor is none of STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO.
   FD must be open and non-inheritable.  The result will be non-inheritable as
   well.
   If FD < 0, FD itself is returned.  */
static int
fd_safer_noinherit (int fd)
{
  if (STDIN_FILENO <= fd && fd <= STDERR_FILENO)
    {
      /* The recursion depth is at most 3.  */
      int nfd = fd_safer_noinherit (dup_noinherit (fd));
      int saved_errno = errno;
      close (fd);
      errno = saved_errno;
      return nfd;
    }
  return fd;
}

int
dup_safer_noinherit (int fd)
{
  return fd_safer_noinherit (dup_noinherit (fd));
}

void
undup_safer_noinherit (int tempfd, int origfd)
{
  if (tempfd >= 0)
    {
      if (dup2 (tempfd, origfd) < 0)
        error (EXIT_FAILURE, errno, _("cannot restore fd %d: dup2 failed"),
               origfd);
      close (tempfd);
    }
  else
    {
      /* origfd was closed or open to no handle at all.  Set it to a closed
         state.  This is (nearly) equivalent to the original state.  */
      close (origfd);
    }
}

const char **
prepare_spawn (const char * const *argv, char **mem_to_free)
{
  size_t argc;
  const char **new_argv;
  size_t i;

  /* Count number of arguments.  */
  for (argc = 0; argv[argc] != NULL; argc++)
    ;

  /* Allocate new argument vector.  */
  new_argv = (const char **) malloc ((1 + argc + 1) * sizeof (const char *));
  if (new_argv == NULL)
    return NULL;

  /* Add an element upfront that can be used when argv[0] turns out to be a
     script, not a program.
     On Unix, this would be "/bin/sh".  */
  new_argv[0] = "sh.exe";

  /* Put quoted arguments into the new argument vector.  */
  size_t needed_size = 0;
  for (i = 0; i < argc; i++)
    {
      const char *string = argv[i];
      const char *quoted_string = (string[0] == '\0' ? "\"\"" : string);
      size_t length = strlen (quoted_string);
      needed_size += length + 1;
    }

  char *mem;
  if (needed_size == 0)
    mem = NULL;
  else
    {
      mem = (char *) malloc (needed_size);
      if (mem == NULL)
        {
          /* Memory allocation failure.  */
          free (new_argv);
          errno = ENOMEM;
          return NULL;
        }
    }
  *mem_to_free = mem;

  for (i = 0; i < argc; i++)
    {
      const char *string = argv[i];

      new_argv[1 + i] = mem;
      const char *quoted_string = (string[0] == '\0' ? "\"\"" : string);
      size_t length = strlen (quoted_string);
      memcpy (mem, quoted_string, length + 1);
      mem += length + 1;
    }
  new_argv[1 + argc] = NULL;

  return new_argv;
}

int
spawnpvech (int mode,
            const char *progname, const char * const *argv,
            const char * const *envp,
            const char *currdir,
            int new_stdin, int new_stdout, int new_stderr)
{
#if HAVE_LIBCX_SPAWN2_H
  static int (*libcx_spawn2) (int mode,
                              const char *name, const char * const argv[],
                              const char *cwd, const char * const envp[],
                              const int stdfds[]) = NULL;
  static int libcx_spawn2_loaded = -1;
#else
  static int libcx_spawn2_loaded = 0;
#endif

  int saved_stdin = STDIN_FILENO;
  int saved_stdout = STDOUT_FILENO;
  int saved_stderr = STDERR_FILENO;
  int ret = -1;

#if HAVE_LIBCX_SPAWN2_H
  if (libcx_spawn2_loaded == -1)
    {
      void *libcx_handle;

      libcx_handle = dlopen ("libcx0", RTLD_LAZY);
      if (libcx_handle != NULL)
        libcx_spawn2 = dlsym (libcx_handle, "_spawn2");

      libcx_spawn2_loaded = libcx_handle != NULL && libcx_spawn2 != NULL;
    }
#endif

  if (!(libcx_spawn2_loaded
        || (currdir == NULL || strcmp (currdir, ".") == 0)))
    {
      errno = EINVAL;
      return -1;
    }

  /* Save standard file handles.  */
  /* 0 means no changes. This is a behavior of spawn2().  */
  if (new_stdin != 0)
    saved_stdin = dup_safer_noinherit (STDIN_FILENO);

  if (!(new_stdout == 0 || new_stdout == 1))
    saved_stdout = dup_safer_noinherit (STDOUT_FILENO);

  if (!(new_stderr == 0 || new_stderr == 2))
    saved_stderr = dup_safer_noinherit (STDERR_FILENO);

  if ((saved_stdin == STDIN_FILENO || dup2 (new_stdin, STDIN_FILENO) >= 0)
      && (saved_stdout == STDOUT_FILENO
          || dup2 (new_stdout, STDOUT_FILENO) >= 0)
      && (saved_stderr == STDERR_FILENO
          || dup2 (new_stderr, STDERR_FILENO) >= 0))
    {
      if (!libcx_spawn2_loaded
          || (currdir == NULL || strcmp (currdir, ".") == 0))
        ret = spawnvpe (mode, progname, (char * const *) argv,
                        (char * const *) envp);
#if HAVE_LIBCX_SPAWN2_H
      else
        ret = libcx_spawn2 (mode | P_2_THREADSAFE, progname, argv, currdir,
                            envp, NULL);
#endif
    }

  /* Restores standard file handles.  */
  if (saved_stderr > STDERR_FILENO)
    undup_safer_noinherit (saved_stderr, STDERR_FILENO);

  if (saved_stdout > STDOUT_FILENO)
    undup_safer_noinherit (saved_stdout, STDOUT_FILENO);

  if (saved_stdin > STDIN_FILENO)
    undup_safer_noinherit (saved_stdin, STDIN_FILENO);

  return ret;
}
