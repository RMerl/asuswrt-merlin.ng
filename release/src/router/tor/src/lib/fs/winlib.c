/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file winlib.c
 *
 * \brief Find and load windows system libraries.
 *
 * We use this function to dynamically load code at runtime that might not be
 * available on all versions of Windows that we support.
 **/

#ifdef _WIN32
#include "lib/fs/winlib.h"

HANDLE
load_windows_system_library(const TCHAR *library_name)
{
  TCHAR path[MAX_PATH];
  unsigned n;
  n = GetSystemDirectory(path, MAX_PATH);
  if (n == 0 || n + _tcslen(library_name) + 2 >= MAX_PATH)
    return 0;
  _tcscat(path, TEXT("\\"));
  _tcscat(path, library_name);
  return LoadLibrary(path);
}
#endif /* defined(_WIN32) */
