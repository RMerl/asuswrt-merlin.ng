/* Program name management.
   Copyright (C) 2016-2024 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  Also get __argv declaration.  */
#include <stdlib.h>

#include <errno.h> /* get program_invocation_name declaration */

#ifdef _AIX
# include <unistd.h>
# include <procinfo.h>
# include <string.h>
#endif

#ifdef __MVS__
# ifndef _OPEN_SYS
#  define _OPEN_SYS
# endif
# include <string.h>
# include <sys/ps.h>
#endif

#ifdef __hpux
# include <unistd.h>
# include <sys/param.h>
# include <sys/pstat.h>
# include <string.h>
#endif

#if defined __sgi || defined __osf__
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <fcntl.h>
# include <sys/procfs.h>
#endif

#if defined __SCO_VERSION__ || defined __sysv5__
# include <fcntl.h>
# include <string.h>
#endif

#include "basename-lgpl.h"

#ifndef HAVE_GETPROGNAME  /* not Mac OS X, FreeBSD, NetBSD, OpenBSD >= 5.4, Solaris >= 11, Cygwin, Android API level >= 21 */
char const *
getprogname (void)
{
# if HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME                /* glibc, BeOS */
  /* https://www.gnu.org/software/libc/manual/html_node/Error-Messages.html */
  return program_invocation_short_name;
# elif HAVE_DECL_PROGRAM_INVOCATION_NAME                    /* glibc, BeOS */
  /* https://www.gnu.org/software/libc/manual/html_node/Error-Messages.html */
  return last_component (program_invocation_name);
# elif HAVE_GETEXECNAME                                     /* Solaris */
  /* https://docs.oracle.com/cd/E19253-01/816-5168/6mbb3hrb1/index.html */
  const char *p = getexecname ();
  if (!p)
    p = "?";
  return last_component (p);
# elif HAVE_DECL___ARGV                                     /* mingw, MSVC */
  /* https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv */
  const char *p = __argv && __argv[0] ? __argv[0] : "?";
  return last_component (p);
# elif HAVE_VAR___PROGNAME                                  /* OpenBSD, Android, QNX */
  /* https://man.openbsd.org/style.9 */
  /* http://www.qnx.de/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Fp%2F__progname.html */
  /* Be careful to declare this only when we absolutely need it
     (OpenBSD 5.1), rather than when it's available.  Otherwise,
     its mere declaration makes program_invocation_short_name
     malfunction (have zero length) with Fedora 25's glibc.  */
  extern char *__progname;
  const char *p = __progname;
#  if defined __ANDROID__
  return last_component (p);
#  else
  return p && p[0] ? p : "?";
#  endif
# elif _AIX                                                 /* AIX */
  /* Idea by Bastien ROUCARIÈS,
     https://lists.gnu.org/r/bug-gnulib/2010-12/msg00095.html
     Reference: https://www.ibm.com/support/knowledgecenter/en/ssw_aix_61/com.ibm.aix.basetrf1/getprocs.htm
  */
  static char *p;
  static int first = 1;
  if (first)
    {
      first = 0;
      pid_t pid = getpid ();
      struct procentry64 procs;
      p = (0 < getprocs64 (&procs, sizeof procs, NULL, 0, &pid, 1)
           ? strdup (procs.pi_comm)
           : NULL);
      if (!p)
        p = "?";
    }
  return p;
# elif defined __hpux
  static char *p;
  static int first = 1;
  if (first)
    {
      first = 0;
      pid_t pid = getpid ();
      struct pst_status status;
      if (pstat_getproc (&status, sizeof status, 0, pid) > 0)
        {
          char *ucomm = status.pst_ucomm;
          char *cmd = status.pst_cmd;
          if (strlen (ucomm) < PST_UCOMMLEN - 1)
            p = ucomm;
          else
            {
              /* ucomm is truncated to length PST_UCOMMLEN - 1.
                 Look at cmd instead.  */
              char *space = strchr (cmd, ' ');
              if (space != NULL)
                *space = '\0';
              p = strrchr (cmd, '/');
              if (p != NULL)
                p++;
              else
                p = cmd;
              if (strlen (p) > PST_UCOMMLEN - 1
                  && memcmp (p, ucomm, PST_UCOMMLEN - 1) == 0)
                /* p is less truncated than ucomm.  */
                ;
              else
                p = ucomm;
            }
          p = strdup (p);
        }
      else
        {
#  if !defined __LP64__
          /* Support for 32-bit programs running in 64-bit HP-UX.
             The documented way to do this is to use the same source code
             as above, but in a compilation unit where '#define _PSTAT64 1'
             is in effect.  I prefer a single compilation unit; the struct
             size and the offsets are not going to change.  */
          char status64[1216];
          if (__pstat_getproc64 (status64, sizeof status64, 0, pid) > 0)
            {
              char *ucomm = status64 + 288;
              char *cmd = status64 + 168;
              if (strlen (ucomm) < PST_UCOMMLEN - 1)
                p = ucomm;
              else
                {
                  /* ucomm is truncated to length PST_UCOMMLEN - 1.
                     Look at cmd instead.  */
                  char *space = strchr (cmd, ' ');
                  if (space != NULL)
                    *space = '\0';
                  p = strrchr (cmd, '/');
                  if (p != NULL)
                    p++;
                  else
                    p = cmd;
                  if (strlen (p) > PST_UCOMMLEN - 1
                      && memcmp (p, ucomm, PST_UCOMMLEN - 1) == 0)
                    /* p is less truncated than ucomm.  */
                    ;
                  else
                    p = ucomm;
                }
              p = strdup (p);
            }
          else
#  endif
            p = NULL;
        }
      if (!p)
        p = "?";
    }
  return p;
# elif __MVS__                                              /* z/OS */
  /* https://www.ibm.com/support/knowledgecenter/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rtwgetp.htm */
  static char *p = "?";
  static int first = 1;
  if (first)
    {
      pid_t pid = getpid ();
      int token;
      W_PSPROC buf;
      first = 0;
      memset (&buf, 0, sizeof(buf));
      buf.ps_cmdptr    = (char *) malloc (buf.ps_cmdlen    = PS_CMDBLEN_LONG);
      buf.ps_conttyptr = (char *) malloc (buf.ps_conttylen = PS_CONTTYBLEN);
      buf.ps_pathptr   = (char *) malloc (buf.ps_pathlen   = PS_PATHBLEN);
      if (buf.ps_cmdptr && buf.ps_conttyptr && buf.ps_pathptr)
        {
          for (token = 0; token >= 0;
               token = w_getpsent (token, &buf, sizeof(buf)))
            {
              if (token > 0 && buf.ps_pid == pid)
                {
                  char *s = strdup (last_component (buf.ps_pathptr));
                  if (s)
                    {
#  if defined __XPLINK__ && __CHARSET_LIB == 1
                      /* The compiler option -qascii is in use.
                         https://makingdeveloperslivesbetter.wordpress.com/2022/01/07/is-z-os-ascii-or-ebcdic-yes/
                         https://www.ibm.com/docs/en/zos/2.5.0?topic=features-macros-related-compiler-option-settings
                         So, convert the result from EBCDIC to ASCII.
                         https://www.ibm.com/docs/en/zos/2.5.0?topic=functions-e2a-s-convert-string-from-ebcdic-ascii */
                      if (__e2a_s (s) == (size_t)-1)
                        free (s);
                      else
#  endif
                        p = s;
                    }
                  break;
                }
            }
        }
      free (buf.ps_cmdptr);
      free (buf.ps_conttyptr);
      free (buf.ps_pathptr);
    }
  return p;
# elif defined __sgi || defined __osf__                     /* IRIX or Tru64 */
  char filename[50];
  int fd;

  # if defined __sgi
    sprintf (filename, "/proc/pinfo/%d", (int) getpid ());
  # else
    sprintf (filename, "/proc/%d", (int) getpid ());
  # endif
  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (0 <= fd)
    {
      prpsinfo_t buf;
      int ioctl_ok = 0 <= ioctl (fd, PIOCPSINFO, &buf);
      close (fd);
      if (ioctl_ok)
        {
          char *name = buf.pr_fname;
          size_t namesize = sizeof buf.pr_fname;
          /* It may not be NUL-terminated.  */
          char *namenul = memchr (name, '\0', namesize);
          size_t namelen = namenul ? namenul - name : namesize;
          char *namecopy = malloc (namelen + 1);
          if (namecopy)
            {
              namecopy[namelen] = '\0';
              return memcpy (namecopy, name, namelen);
            }
        }
    }
  return NULL;
# elif defined __SCO_VERSION__ || defined __sysv5__                /* SCO OpenServer6/UnixWare */
  char buf[80];
  int fd;
  sprintf (buf, "/proc/%d/cmdline", getpid());
  fd = open (buf, O_RDONLY);
  if (0 <= fd)
    {
      size_t n = read (fd, buf, 79);
      if (n > 0)
        {
          buf[n] = '\0'; /* Guarantee null-termination */
          char *progname;
          progname = strrchr (buf, '/');
          if (progname)
            {
              progname = progname + 1; /* Skip the '/' */
            }
          else
            {
              progname = buf;
            }
          char *ret;
          ret = malloc (strlen (progname) + 1);
          if (ret)
            {
              strcpy (ret, progname);
              return ret;
            }
        }
      close (fd);
    }
  return "?";
# else
#  error "getprogname module not ported to this OS"
# endif
}

#endif

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
