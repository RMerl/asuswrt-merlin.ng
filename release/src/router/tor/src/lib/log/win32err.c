/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file win32err.c
 * \brief Convert windows error codes to useful C strings.
 **/

#ifdef _WIN32
#include "orconfig.h"
#include "lib/log/win32err.h"
#include "lib/malloc/malloc.h"

#include <tchar.h>
#include <windows.h>

/** Return a newly allocated string describing the windows system error code
 * <b>err</b>.  Note that error codes are different from errno.  Error codes
 * come from GetLastError() when a winapi call fails.  errno is set only when
 * ANSI functions fail.  Whee. */
char *
format_win32_error(DWORD err)
{
  TCHAR *str = NULL;
  char *result;
  DWORD n;

  /* Somebody once decided that this interface was better than strerror(). */
  n = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM |
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, err,
                 MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                 (LPVOID)&str,
                 0, NULL);

  if (str && n) {
#ifdef UNICODE
    size_t len;
    if (n > 128*1024)
      len = (128 * 1024) * 2 + 1; /* This shouldn't be possible, but let's
                                   * make sure. */
    else
      len = n * 2 + 1;
    result = tor_malloc(len);
    wcstombs(result,str,len);
    result[len-1] = '\0';
#else /* !defined(UNICODE) */
    result = tor_strdup(str);
#endif /* defined(UNICODE) */
  } else {
    result = tor_strdup("<unformattable error>");
  }
  if (str) {
    LocalFree(str); /* LocalFree != free() */
  }
  return result;
}
#endif /* defined(_WIN32) */
