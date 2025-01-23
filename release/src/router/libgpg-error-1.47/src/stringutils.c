/* stringutils.c - String helper functions.
 * Copyright (C) 1997, 2014 Werner Koch
 * Copyright (C) 2020 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <config.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#ifdef HAVE_STAT
# include <sys/stat.h>
#endif
#include <sys/types.h>

#include "gpgrt-int.h"


/* Helper for _gpgrt_fnameconcat.  The additional flag WANT_ABS tells
 * whether an absolute file name is requested.  */
char *
_gpgrt_vfnameconcat (int want_abs, const char *first_part, va_list arg_ptr)
{
  const char *argv[32];
  int argc;
  size_t n;
  int skip = 1;  /* Characters to skip from FIRST_PART.  */
  char *home_buffer = NULL;
  char *name, *home, *p;

  /* Put all args into an array because we need to scan them twice.  */
  n = strlen (first_part) + 1;
  argc = 0;
  while ((argv[argc] = va_arg (arg_ptr, const char *)))
    {
      n += strlen (argv[argc]) + 1;
      if (argc >= DIM (argv)-1)
        {
          _gpg_err_set_errno (EINVAL);
          return NULL;
        }
      argc++;
    }
  n++;

  home = NULL;
  if (*first_part == '~')
    {
      if (first_part[1] == '/' || !first_part[1])
        {
          /* This is the "~/" or "~" case.  */
          home_buffer = _gpgrt_getenv ("HOME");
          if (!home_buffer)
            home_buffer = _gpgrt_getpwdir (NULL);
          home = home_buffer;
          if (home && *home)
            n += strlen (home);
        }
      else
        {
          /* This is the "~username/" or "~username" case.  */
          char *user;

          user = _gpgrt_strdup (first_part+1);
          if (!user)
            return NULL;

          p = strchr (user, '/');
          if (p)
            *p = 0;
          skip = 1 + strlen (user);

          home = home_buffer = _gpgrt_getpwdir (user);
          xfree (user);
          if (home)
            n += strlen (home);
          else
            skip = 1;
        }
    }

  name = xtrymalloc (n);
  if (!name)
    {
      _gpgrt_free (home_buffer);
      return NULL;
    }

  if (home)
    p = stpcpy (stpcpy (name, home), first_part + skip);
  else
    p = stpcpy (name, first_part);

  xfree (home_buffer);
  home_buffer = NULL;

  for (argc=0; argv[argc]; argc++)
    {
      /* Avoid a leading double slash if the first part was "/".  */
      if (!argc && name[0] == '/' && !name[1])
        p = stpcpy (p, argv[argc]);
      else
        p = stpcpy (stpcpy (p, "/"), argv[argc]);
    }

  if (want_abs)
    {
#ifdef HAVE_W32_SYSTEM
      p = strchr (name, ':');
      if (p)
        p++;
      else
        p = name;
#else
      p = name;
#endif
      if (*p != '/'
#ifdef HAVE_W32_SYSTEM
          && *p != '\\'
#endif
          )
        {
          home = _gpgrt_getcwd ();
          if (!home)
            {
              xfree (name);
              return NULL;
            }

          n = strlen (home) + 1 + strlen (name) + 1;
          home_buffer = xtrymalloc (n);
          if (!home_buffer)
            {
              xfree (home);
              xfree (name);
              return NULL;
            }

          if (p == name)
            p = home_buffer;
          else /* Windows case.  */
            {
              memcpy (home_buffer, p, p - name + 1);
              p = home_buffer + (p - name + 1);
            }

          /* Avoid a leading double slash if the cwd is "/".  */
          if (home[0] == '/' && !home[1])
            strcpy (stpcpy (p, "/"), name);
          else
            strcpy (stpcpy (stpcpy (p, home), "/"), name);

          xfree (home);
          xfree (name);
          name = home_buffer;
          /* Let's do a simple compression to catch the common case of
           * a trailing "/.".  */
          n = strlen (name);
          if (n > 2 && name[n-2] == '/' && name[n-1] == '.')
            name[n-2] = 0;
        }
    }

#ifdef HAVE_W32_SYSTEM
  for (p=name; *p; p++)
    if (*p == '\\')
      *p = '/';
#endif
  return name;
}


/* Construct a filename from the NULL terminated list of parts.  Tilde
 * expansion is done for the first argument.  The caller must release
 * the result using gpgrt_free; on error ERRNO is set and NULL
 * returned.  */
char *
_gpgrt_fnameconcat (const char *first_part, ... )
{
  va_list arg_ptr;
  char *result;

  va_start (arg_ptr, first_part);
  result = _gpgrt_vfnameconcat (0, first_part, arg_ptr);
  va_end (arg_ptr);
  return result;
}


/* Construct a filename from the NULL terminated list of parts.  Tilde
 * expansion is done for the first argument.  The caller must release
 * the result using gpgrt_free; on error ERRNO is set and NULL
 * returned.  This version returns an absolute filename. */
char *
_gpgrt_absfnameconcat (const char *first_part, ... )
{
  va_list arg_ptr;
  char *result;

  va_start (arg_ptr, first_part);
  result = _gpgrt_vfnameconcat (1, first_part, arg_ptr);
  va_end (arg_ptr);
  return result;
}
