/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file libc.c
 * @brief Functions to get the name and version of the system libc.
 **/

#include "orconfig.h"
#include "lib/osinfo/libc.h"
#include <stdlib.h>

#ifdef HAVE_GNU_LIBC_VERSION_H
#include <gnu/libc-version.h>
#endif

#ifdef HAVE_GNU_LIBC_VERSION_H
#ifdef HAVE_GNU_GET_LIBC_VERSION
#define CHECK_LIBC_VERSION
#endif
#endif

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

/** Return the name of the compile time libc. Returns NULL if we
 * cannot identify the libc. */
const char *
tor_libc_get_name(void)
{
#ifdef __GLIBC__
  return "Glibc";
#else /* !defined(__GLIBC__) */
  return NULL;
#endif /* defined(__GLIBC__) */
}

/** Return a string representation of the version of the currently running
 * version of Glibc. */
const char *
tor_libc_get_version_str(void)
{
#ifdef CHECK_LIBC_VERSION
  const char *version = gnu_get_libc_version();
  if (version == NULL)
    return "N/A";
  return version;
#else /* !defined(CHECK_LIBC_VERSION) */
  return "N/A";
#endif /* defined(CHECK_LIBC_VERSION) */
}

/** Return a string representation of the version of Glibc that was used at
 * compilation time. */
const char *
tor_libc_get_header_version_str(void)
{
#ifdef __GLIBC__
  return STR(__GLIBC__) "." STR(__GLIBC_MINOR__);
#else
  return "N/A";
#endif /* defined(__GLIBC__) */
}
