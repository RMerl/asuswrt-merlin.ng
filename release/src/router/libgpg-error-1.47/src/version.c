/* version.c - Version checking
 * Copyright (C) 2001, 2002, 2012, 2013, 2014 g10 Code GmbH
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gpg-error.h>


#define digitp(a) ((a) >= '0' && (a) <= '9')


/* This is actually a dummy function to make sure that is module is
   not empty.  Some compilers barf on empty modules.  */
static const char *
cright_blurb (void)
{
  static const char blurb[] =
    "\n\n"
    "This is Libgpg-error " PACKAGE_VERSION " - A runtime library\n"
    "Copyright 2001-2022 g10 Code GmbH\n"
    "\n"
    "(" BUILD_REVISION " " BUILD_TIMESTAMP ")\n"
    "\n\n";
  return blurb;
}


/* This function parses the first portion of the version number S and
 * stores it at NUMBER.  On success, this function returns a pointer
 * into S starting with the first character, which is not part of the
 * initial number portion; on failure, NULL is returned.  */
static const char*
parse_version_number (const char *s, int *number)
{
  int val = 0;

  if (*s == '0' && digitp (s[1]))
    return NULL;  /* Leading zeros are not allowed.  */
  for (; digitp (*s); s++)
    {
      val *= 10;
      val += *s - '0';
    }
  *number = val;
  return val < 0 ? NULL : s;
}


/* This function breaks up the complete string-representation of the
 * version number S, which is of the following struture: <major
 * number>.<minor number>.<micro number><patch level>.  The major,
 * minor and micro number components will be stored in *MAJOR, *MINOR
 * and *MICRO.  If MINOR or MICRO is NULL the version number is
 * assumed to have just 1 respective 2 parts.
 *
 * On success, the last component, the patch level, will be returned;
 * in failure, NULL will be returned.  */
static const char *
parse_version_string (const char *s, int *major, int *minor, int *micro)
{
  s = parse_version_number (s, major);
  if (!s)
    return NULL;
  if (!minor)
    {
      if (*s == '.')
        s++;
    }
  else
    {
      if (*s != '.')
        return NULL;
      s++;
      s = parse_version_number (s, minor);
      if (!s)
        return NULL;
      if (!micro)
        {
          if (*s == '.')
            s++;
        }
      else
        {
          if (*s != '.')
            return NULL;
          s++;
          s = parse_version_number (s, micro);
          if (!s)
            return NULL;
        }
    }
  return s; /* patchlevel */
}


/* Helper for _gpgrt_cmp_version.  */
static int
do_cmp_version (const char *a, const char *b, int level)
{
  int a_major, a_minor, a_micro;
  int b_major, b_minor, b_micro;
  const char *a_plvl, *b_plvl;
  int r;
  int ignore_plvl;
  int positive, negative;

  if (level < 0)
    {
      positive = -1;
      negative = 1;
      level = 0 - level;
    }
  else
    {
      positive = 1;
      negative = -1;
    }
  if ((ignore_plvl = (level > 9)))
    level %= 10;

  a_major = a_minor = a_micro = 0;
  a_plvl = parse_version_string (a, &a_major,
                                 level > 1? &a_minor : NULL,
                                 level > 2? &a_micro : NULL);
  if (!a_plvl)
    a_major = a_minor = a_micro = 0; /* Error.  */

  b_major = b_minor = b_micro = 0;
  b_plvl = parse_version_string (b, &b_major,
                                 level > 1? &b_minor : NULL,
                                 level > 2? &b_micro : NULL);
  if (!b_plvl)
    b_major = b_minor = b_micro = 0;

  if (!ignore_plvl)
    {
      if (!a_plvl && !b_plvl)
        return negative;  /* Put invalid strings at the end.  */
      if (a_plvl && !b_plvl)
        return positive;
      if (!a_plvl && b_plvl)
        return negative;
    }

  if (a_major > b_major)
    return positive;
  if (a_major < b_major)
    return negative;

  if (a_minor > b_minor)
    return positive;
  if (a_minor < b_minor)
    return negative;

  if (a_micro > b_micro)
    return positive;
  if (a_micro < b_micro)
    return negative;

  if (ignore_plvl)
    return 0;

  for (; *a_plvl && *b_plvl; a_plvl++, b_plvl++)
    {
      if (*a_plvl == '.' && *b_plvl == '.')
        {
          r = strcmp (a_plvl, b_plvl);
          if (!r)
            return 0;
          else if ( r > 0 )
            return positive;
          else
            return negative;
        }
      else if (*a_plvl == '.')
        return negative; /* B is larger. */
      else if (*b_plvl == '.')
        return positive; /* A is larger. */
      else if (*a_plvl != *b_plvl)
        break;
        }
  if (*a_plvl == *b_plvl)
    return 0;
  else if ((*(signed char *)a_plvl - *(signed char *)b_plvl) > 0)
    return positive;
  else
    return negative;
}


/* Compare function for version strings.  The return value is
 * like strcmp().  LEVEL may be
 *   0 - reserved
 *   1 - format is "<major><patchlevel>".
 *   2 - format is "<major>.<minor><patchlevel>".
 *   3 - format is "<major>.<minor>.<micro><patchlevel>".
 * To ignore the patchlevel in the comparison add 10 to LEVEL.  To get
 * a reverse sorting order use a negative number.
 */
int
_gpgrt_cmp_version (const char *a, const char *b, int level)
{
  return do_cmp_version (a, b, level);
}


/*
 * Check that the the version of the library is at minimum REQ_VERSION
 * and return the actual version string; return NULL if the condition
 * is not met.  If NULL is passed to this function, no check is done
 * and the version string is simply returned.
 */
const char *
_gpg_error_check_version (const char *req_version)
{
  const char *my_version = PACKAGE_VERSION;

  if (req_version && req_version[0] == 1 && req_version[1] == 1)
    return cright_blurb ();
  if (!req_version)
    return my_version;
  return _gpgrt_cmp_version
    (my_version, req_version, 12) >= 0 ? my_version : NULL;
}
