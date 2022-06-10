/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file mmap.c
 *
 * \brief Cross-platform support for mapping files into our address space.
 **/

#include "lib/fs/mmap.h"
#include "lib/fs/files.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/win32err.h"
#include "lib/string/compat_string.h"
#include "lib/malloc/malloc.h"

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <errno.h>
#include <string.h>

#if defined(HAVE_MMAP) || defined(RUNNING_DOXYGEN)
/** Try to create a memory mapping for <b>filename</b> and return it.  On
 * failure, return NULL. Sets errno properly, using ERANGE to mean
 * "empty file". Must only be called on trusted Tor-owned files, as changing
 * the underlying file's size causes unspecified behavior. */
MOCK_IMPL(tor_mmap_t *,
tor_mmap_file,(const char *filename))
{
  int fd; /* router file */
  char *string;
  int result;
  tor_mmap_t *res;
  size_t size, filesize;
  struct stat st;

  tor_assert(filename);

  fd = tor_open_cloexec(filename, O_RDONLY, 0);
  if (fd<0) {
    int save_errno = errno;
    int severity = (errno == ENOENT) ? LOG_INFO : LOG_WARN;
    log_fn(severity, LD_FS,"Could not open \"%s\" for mmap(): %s",filename,
           strerror(errno));
    errno = save_errno;
    return NULL;
  }

  /* Get the size of the file */
  result = fstat(fd, &st);
  if (result != 0) {
    int save_errno = errno;
    log_warn(LD_FS,
             "Couldn't fstat opened descriptor for \"%s\" during mmap: %s",
             filename, strerror(errno));
    close(fd);
    errno = save_errno;
    return NULL;
  }
  size = filesize = (size_t)(st.st_size);

  if (st.st_size > SSIZE_T_CEILING || (off_t)size < st.st_size) {
    log_warn(LD_FS, "File \"%s\" is too large. Ignoring.",filename);
    errno = EFBIG;
    close(fd);
    return NULL;
  }
  if (!size) {
    /* Zero-length file. If we call mmap on it, it will succeed but
     * return NULL, and bad things will happen. So just fail. */
    log_info(LD_FS,"File \"%s\" is empty. Ignoring.",filename);
    errno = ERANGE;
    close(fd);
    return NULL;
  }

  string = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  if (string == MAP_FAILED) {
    int save_errno = errno;
    log_warn(LD_FS,"Could not mmap file \"%s\": %s", filename,
             strerror(errno));
    errno = save_errno;
    return NULL;
  }

  res = tor_malloc_zero(sizeof(tor_mmap_t));
  res->data = string;
  res->size = filesize;
  res->mapping_size = size;

  return res;
}
/** Release storage held for a memory mapping; returns 0 on success,
 * or -1 on failure (and logs a warning). */
MOCK_IMPL(int,
tor_munmap_file,(tor_mmap_t *handle))
{
  int res;

  if (handle == NULL)
    return 0;

  res = munmap((char*)handle->data, handle->mapping_size);
  if (res == 0) {
    /* munmap() succeeded */
    tor_free(handle);
  } else {
    log_warn(LD_FS, "Failed to munmap() in tor_munmap_file(): %s",
             strerror(errno));
    res = -1;
  }

  return res;
}
#elif defined(_WIN32)
MOCK_IMPL(tor_mmap_t *,
tor_mmap_file,(const char *filename))
{
  TCHAR tfilename[MAX_PATH]= {0};
  tor_mmap_t *res = tor_malloc_zero(sizeof(tor_mmap_t));
  int empty = 0;
  HANDLE file_handle = INVALID_HANDLE_VALUE;
  DWORD size_low, size_high;
  uint64_t real_size;
  res->mmap_handle = NULL;
#ifdef UNICODE
  mbstowcs(tfilename,filename,MAX_PATH);
#else
  strlcpy(tfilename,filename,MAX_PATH);
#endif
  file_handle = CreateFile(tfilename,
                           GENERIC_READ, FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           0);

  if (file_handle == INVALID_HANDLE_VALUE)
    goto win_err;

  size_low = GetFileSize(file_handle, &size_high);

  if (size_low == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
    log_warn(LD_FS,"Error getting size of \"%s\".",filename);
    goto win_err;
  }
  if (size_low == 0 && size_high == 0) {
    log_info(LD_FS,"File \"%s\" is empty. Ignoring.",filename);
    empty = 1;
    goto err;
  }
  real_size = (((uint64_t)size_high)<<32) | size_low;
  if (real_size > SIZE_MAX) {
    log_warn(LD_FS,"File \"%s\" is too big to map; not trying.",filename);
    goto err;
  }
  res->size = real_size;

  res->mmap_handle = CreateFileMapping(file_handle,
                                       NULL,
                                       PAGE_READONLY,
                                       size_high,
                                       size_low,
                                       NULL);
  if (res->mmap_handle == NULL)
    goto win_err;
  res->data = (char*) MapViewOfFile(res->mmap_handle,
                                    FILE_MAP_READ,
                                    0, 0, 0);
  if (!res->data)
    goto win_err;

  CloseHandle(file_handle);
  return res;
 win_err: {
    DWORD e = GetLastError();
    int severity = (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) ?
      LOG_INFO : LOG_WARN;
    char *msg = format_win32_error(e);
    log_fn(severity, LD_FS, "Couldn't mmap file \"%s\": %s", filename, msg);
    tor_free(msg);
    if (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND)
      errno = ENOENT;
    else
      errno = EINVAL;
  }
 err:
  if (empty)
    errno = ERANGE;
  if (file_handle != INVALID_HANDLE_VALUE)
    CloseHandle(file_handle);
  tor_munmap_file(res);
  return NULL;
}

/* Unmap the file, and return 0 for success or -1 for failure */
MOCK_IMPL(int,
tor_munmap_file,(tor_mmap_t *handle))
{
  if (handle == NULL)
    return 0;

  if (handle->data) {
    /* This is an ugly cast, but without it, "data" in struct tor_mmap_t would
       have to be redefined as non-const. */
    BOOL ok = UnmapViewOfFile( (LPVOID) handle->data);
    if (!ok) {
      log_warn(LD_FS, "Failed to UnmapViewOfFile() in tor_munmap_file(): %d",
               (int)GetLastError());
    }
  }

  if (handle->mmap_handle != NULL)
    CloseHandle(handle->mmap_handle);
  tor_free(handle);

  return 0;
}
#else
#error "cannot implement tor_mmap_file"
#endif /* defined(HAVE_MMAP) || defined(RUNNING_DOXYGEN) || ... */
