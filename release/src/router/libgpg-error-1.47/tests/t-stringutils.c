/* t-stringutils.c - Check some string utilities
 * Copyright (C) 2020 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#ifdef HAVE_STAT
# include <sys/stat.h>
#endif
#include <sys/types.h>
#ifdef HAVE_PWD_H
# include <pwd.h>
#endif
#include <errno.h>
#ifdef HAVE_W32_SYSTEM
# include <windows.h>
#endif

#define PGM "t-stringutils"
#include "t-common.h"


static const char *
my_strusage (int level)
{
  const char *p;

  switch (level)
    {
    case 9: p = "LGPL-2.1-or-later"; break;
    case 11: p = PGM; break;
    default: p = NULL;
    }
  return p;
}


const char *
mygethome (void)
{
  static char *home_buffer;

  if (!home_buffer)
    {
      char *home = getenv("HOME");

      if(home)
        home_buffer = xstrdup (home);
#if defined(HAVE_GETPWUID) && defined(HAVE_PWD_H)
      else
        {
          struct passwd *pwd;

          pwd = getpwuid (getuid());
          if (pwd)
            home_buffer = xstrdup (pwd->pw_dir);
        }
#endif
    }
  return home_buffer;
}


#ifdef HAVE_W32_SYSTEM
static wchar_t *
utf8_to_wchar (const char *string)
{
  int n;
  wchar_t *result;
  size_t nbytes;
  int cbmultibyte = -1;

  n = MultiByteToWideChar (CP_UTF8, 0, string, cbmultibyte, NULL, 0);
  if (n < 0 || (n+1) <= 0)
    die ("utf8_to_wchar failed\n");
  nbytes = (size_t)(n+1) * sizeof(*result);
  if (nbytes / sizeof(*result) != (n+1))
    die ("utf8_to_wchar failed\n");
  result = xmalloc (nbytes);
  n = MultiByteToWideChar (CP_UTF8, 0, string, cbmultibyte, result, n);
  if (n < 0)
    die ("utf8_to_wchar failed\n");
  return result;

}


static char *
wchar_to_utf8 (const wchar_t *string, size_t length)
{
  int n;
  char *result;

  n = WideCharToMultiByte (CP_UTF8, 0, string, length, NULL, 0, NULL, NULL);
  if (n < 0 || (n+1) <= 0)
    die ("wchar_to_utf8 failed\n");

  result = xmalloc (n+1);
  if (!result)
    die ("wchar_to_utf8 failed\n");
  n = WideCharToMultiByte (CP_UTF8, 0, string, length, result, n, NULL, NULL);
  if (n < 0)
    die ("wchar_to_utf8 failed\n");
  result[n] = 0;
  return result;
}
#endif

static char *
mygetcwd (void)
{
#ifdef HAVE_W32_SYSTEM
  wchar_t wbuffer[MAX_PATH + sizeof(wchar_t)];
  wchar_t *wp;
  DWORD wlen;
  char *buf, *p;

  wlen = GetCurrentDirectoryW (MAX_PATH, wbuffer);
  if (!wlen)
    die ("GCDW failed - error code: %d\n", (int)GetLastError ());
  else if (wlen > MAX_PATH)
    die ("GCDW failed - wlen too large\n");

  buf = wchar_to_utf8 (wbuffer, wlen);

  /* Quick test that the reverse works.  */
  wp = utf8_to_wchar (buf);
  if (wcscmp (wp, wbuffer))
    die ("GCDW: reverse converting failed\n");
  xfree (wp);

  for (p=buf; *p; p++)
    if (*p == '\\')
      *p = '/';
  return buf;

#else
  char *buffer;
  size_t size = 100;

  for (;;)
    {
      buffer = xmalloc (size+1);
      if (getcwd (buffer, size) == buffer)
        {
          return buffer;
        }
      xfree (buffer);
      if (errno != ERANGE)
        die ("error getting current cwd: %s\n", strerror (errno));
      size *= 2;
    }
#endif
}


static void
check_fnameconcat (void)
{
  char *out;
  const char *home = mygethome ();
  size_t homelen = home? strlen (home):0;

  out = gpgrt_fnameconcat ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", NULL);
  if (out)
    fail ("fnameconcat succeeded but should not at line %d\n", __LINE__);
  else if (errno != EINVAL)
    fail ("fnameconcat return wrong error at line %d\n", __LINE__);
  xfree (out);

  out = gpgrt_fnameconcat ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", NULL);
  if (out)
    fail ("fnameconcat succeeded but should not at line %d\n", __LINE__);
  else if (errno != EINVAL)
    fail ("fnameconcat return wrong error at line %d\n", __LINE__);
  xfree (out);

  out = gpgrt_fnameconcat ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                           "1", "2", NULL);
  if (!out || strcmp (out,
                      "1/2/3/4/5/6/7/8/9/10/"
                      "1/2/3/4/5/6/7/8/9/10/"
                      "1/2/3/4/5/6/7/8/9/10/"
                      "1/2"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("foo", "~/bar", "baz/cde", NULL);
  if (!out || strcmp (out, "foo/~/bar/baz/cde"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("foo", "~/bar", "baz/cde/", NULL);
  if (!out || strcmp (out, "foo/~/bar/baz/cde/"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("/foo", "~/bar", "baz/cde/", NULL);
  if (!out || strcmp (out, "/foo/~/bar/baz/cde/"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("//foo", "~/bar", "baz/cde/", NULL);
  if (!out || strcmp (out, "//foo/~/bar/baz/cde/"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("", "~/bar", "baz/cde", NULL);
  if (!out || strcmp (out, "/~/bar/baz/cde"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_fnameconcat ("~/foo", "bar", NULL);
  if (!out)
    fail ("fnameconcat failed at line %d\n", __LINE__);
  else if (home)
    {
      if (strlen (out) < homelen + 7)
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
      else if (strncmp (out, home, homelen))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
      else if (strcmp (out+homelen, "/foo/bar"))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
    }
  else
    {
      if (strcmp (out, "~/foo/bar"))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
    }
  xfree (out);

  out = gpgrt_fnameconcat ("~", "bar", NULL);
  if (!out)
    fail ("fnameconcat failed at line %d\n", __LINE__);
  else if (home)
    {
      if (strlen (out) < homelen + 3)
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
      else if (strncmp (out, home, homelen))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
      else if (strcmp (out+homelen, "/bar"))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
    }
  else
    {
      if (strcmp (out, "~/bar"))
        fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
    }
  xfree (out);
}


static void
check_absfnameconcat (void)
{
  char *out;
  char *cwd = mygetcwd ();
  size_t cwdlen = strlen (cwd);

  out = gpgrt_absfnameconcat ("foo", "bar", NULL);
  if (!out)
    fail ("fnameconcat failed at line %d\n", __LINE__);
  else if (strlen (out) < cwdlen + 7)
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strncmp (out, cwd, cwdlen))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strcmp (out+cwdlen, "/foo/bar"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_absfnameconcat ("./foo", NULL);
  if (!out)
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strlen (out) < cwdlen + 5)
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strncmp (out, cwd, cwdlen))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strcmp (out+cwdlen, "/./foo"))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  out = gpgrt_absfnameconcat (".", NULL);
  if (!out)
    fail ("fnameconcat failed at line %d\n", __LINE__);
  else if (strlen (out) < cwdlen)
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strncmp (out, cwd, cwdlen))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  else if (strcmp (out+cwdlen, ""))
    fail ("fnameconcat failed at line %d (out=%s)\n", __LINE__, out);
  xfree (out);

  xfree (cwd);
}


static void
check_access (void)
{
  char *cwd = mygetcwd ();

  if (gpgrt_access (cwd, F_OK))
    fail ("gpgrt_access(%s) failed: %s\n",
          cwd, gpg_strerror (gpg_error_from_syserror ()));
  else
    show ("gpgrt_access(%s) succeeded\n", cwd);

  xfree (cwd);
}


int
main (int argc, char **argv)
{
  gpgrt_opt_t opts[] = {
    ARGPARSE_x  ('v', "verbose", NONE, 0, "Print more diagnostics"),
    ARGPARSE_s_n('d', "debug", "Flyswatter"),
    ARGPARSE_x  (501, "pwd", NONE, 0, "Print working directory"),
    ARGPARSE_end()
  };
  gpgrt_argparse_t pargs = { &argc, &argv, 0 };
  char *cwd;
  int opt_pwd = 0;

  gpgrt_set_strusage (my_strusage);
  gpgrt_log_set_prefix (gpgrt_strusage (11), GPGRT_LOG_WITH_PREFIX);

  while (gpgrt_argparse  (NULL, &pargs, opts))
    {
      switch (pargs.r_opt)
        {
        case 'v': verbose++; break;
        case 'd': debug++; break;
        case 501: opt_pwd = 1; break;
        default : pargs.err = ARGPARSE_PRINT_ERROR; break;
	}
    }
  gpgrt_argparse (NULL, &pargs, NULL);

  cwd = gpgrt_getcwd ();
  if (!cwd)
    fail ("gpgrt_getcwd returned error: %s\n",
          gpg_strerror (gpg_error_from_syserror ()));
  else
    {
      if (opt_pwd)
        {
          int save_verbose = verbose;
          verbose = 1;
          show ("getcwd -> '%s'\n", cwd);
          verbose = save_verbose;
        }
      xfree (cwd);
    }

  show ("testing string utilities\n");

  check_fnameconcat ();
  check_absfnameconcat ();
  check_access ();

  show ("testing string utilities finished\n");
  return !!errorcount;
}
