/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file files.h
 *
 * \brief Wrappers for reading and writing data to files on disk.
 **/

#ifdef _WIN32
#include <windows.h>
#endif

#include "lib/fs/files.h"
#include "lib/fs/path.h"
#include "lib/container/smartlist.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/escape.h"
#include "lib/err/torerr.h"
#include "lib/malloc/malloc.h"
#include "lib/sandbox/sandbox.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"
#include "lib/fdio/fdio.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>

/** As open(path, flags, mode), but return an fd with the close-on-exec mode
 * set. */
int
tor_open_cloexec(const char *path, int flags, unsigned mode)
{
  int fd;
  const char *p = sandbox_intern_string(path);
#ifdef O_CLOEXEC
  fd = open(p, flags|O_CLOEXEC, mode);
  if (fd >= 0)
    return fd;
  /* If we got an error, see if it is EINVAL. EINVAL might indicate that,
   * even though we were built on a system with O_CLOEXEC support, we
   * are running on one without. */
  if (errno != EINVAL)
    return -1;
#endif /* defined(O_CLOEXEC) */

  log_debug(LD_FS, "Opening %s with flags %x", p, flags);
  fd = open(p, flags, mode);
#ifdef FD_CLOEXEC
  if (fd >= 0) {
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
      log_warn(LD_FS,"Couldn't set FD_CLOEXEC: %s", strerror(errno));
      close(fd);
      return -1;
    }
  }
#endif /* defined(FD_CLOEXEC) */
  return fd;
}

/** As fopen(path,mode), but ensures that the O_CLOEXEC bit is set on the
 * underlying file handle. */
FILE *
tor_fopen_cloexec(const char *path, const char *mode)
{
  const char *p = sandbox_intern_string(path);
  FILE *result = fopen(p, mode);
#ifdef FD_CLOEXEC
  if (result != NULL) {
    if (fcntl(fileno(result), F_SETFD, FD_CLOEXEC) == -1) {
      log_warn(LD_FS,"Couldn't set FD_CLOEXEC: %s", strerror(errno));
      fclose(result);
      return NULL;
    }
  }
#endif /* defined(FD_CLOEXEC) */
  return result;
}

/** As rename(), but work correctly with the sandbox. */
int
tor_rename(const char *path_old, const char *path_new)
{
  log_debug(LD_FS, "Renaming %s to %s", path_old, path_new);
  return rename(sandbox_intern_string(path_old),
                sandbox_intern_string(path_new));
}

/**
 * Rename the file <b>from</b> to the file <b>to</b>.  On Unix, this is
 * the same as rename(2).  On windows, this removes <b>to</b> first if
 * it already exists.
 * Returns 0 on success.  Returns -1 and sets errno on failure.
 */
int
replace_file(const char *from, const char *to)
{
#ifndef _WIN32
  return tor_rename(from, to);
#else
  switch (file_status(to))
    {
    case FN_NOENT:
      break;
    case FN_FILE:
    case FN_EMPTY:
      if (unlink(to)) return -1;
      break;
    case FN_ERROR:
      return -1;
    case FN_DIR:
      errno = EISDIR;
      return -1;
    }
  return tor_rename(from,to);
#endif /* !defined(_WIN32) */
}

/** Change <b>fname</b>'s modification time to now. */
int
touch_file(const char *fname)
{
  if (utime(fname, NULL)!=0)
    return -1;
  return 0;
}

/** Wrapper for unlink() to make it mockable for the test suite; returns 0
 * if unlinking the file succeeded, -1 and sets errno if unlinking fails.
 */

MOCK_IMPL(int,
tor_unlink,(const char *pathname))
{
  return unlink(pathname);
}

/** Write <b>count</b> bytes from <b>buf</b> to <b>fd</b>. Return the number
 * of bytes written, or -1 on error.  Only use if fd is a blocking fd.  */
ssize_t
write_all_to_fd(int fd, const char *buf, size_t count)
{
  size_t written = 0;
  ssize_t result;
  raw_assert(count < SSIZE_MAX);

  while (written != count) {
    result = write(fd, buf+written, count-written);
    if (result<0)
      return -1;
    written += result;
  }
  return (ssize_t)count;
}

/** Read from <b>fd</b> to <b>buf</b>, until we get <b>count</b> bytes or
 * reach the end of the file.  Return the number of bytes read, or -1 on
 * error. Only use if fd is a blocking fd. */
ssize_t
read_all_from_fd(int fd, char *buf, size_t count)
{
  size_t numread = 0;
  ssize_t result;

  if (count > SIZE_T_CEILING || count > SSIZE_MAX) {
    errno = EINVAL;
    return -1;
  }

  while (numread < count) {
    result = read(fd, buf+numread, count-numread);
    if (result<0)
      return -1;
    else if (result == 0)
      break;
    numread += result;
  }
  return (ssize_t)numread;
}

/** Return:
 * FN_ERROR if filename can't be read, is NULL, or is zero-length,
 * FN_NOENT if it doesn't exist,
 * FN_FILE if it is a non-empty regular file, or a FIFO on unix-like systems,
 * FN_EMPTY for zero-byte regular files,
 * FN_DIR if it's a directory, and
 * FN_ERROR for any other file type.
 * On FN_ERROR and FN_NOENT, sets errno.  (errno is not set when FN_ERROR
 * is returned due to an unhandled file type.) */
file_status_t
file_status(const char *fname)
{
  struct stat st;
  char *f;
  int r;
  if (!fname || strlen(fname) == 0) {
    return FN_ERROR;
  }
  f = tor_strdup(fname);
  clean_fname_for_stat(f);
  log_debug(LD_FS, "stat()ing %s", f);
  r = stat(sandbox_intern_string(f), &st);
  tor_free(f);
  if (r) {
    if (errno == ENOENT) {
      return FN_NOENT;
    }
    return FN_ERROR;
  }
  if (st.st_mode & S_IFDIR) {
    return FN_DIR;
  } else if (st.st_mode & S_IFREG) {
    if (st.st_size > 0) {
      return FN_FILE;
    } else if (st.st_size == 0) {
      return FN_EMPTY;
    } else {
      return FN_ERROR;
    }
#ifndef _WIN32
  } else if (st.st_mode & S_IFIFO) {
    return FN_FILE;
#endif
  } else {
    return FN_ERROR;
  }
}

/** Returns true if <b>file_type</b> represents an existing file (even if
 * empty). Returns false otherwise. */
bool
is_file(file_status_t file_type)
{
  return file_type != FN_ERROR && file_type != FN_NOENT && file_type != FN_DIR;
}

/** Returns true if <b>file_type</b> represents an existing directory. Returns
 * false otherwise. */
bool
is_dir(file_status_t file_type)
{
  return file_type == FN_DIR;
}

/** Create a file named <b>fname</b> with the contents <b>str</b>.  Overwrite
 * the previous <b>fname</b> if possible.  Return 0 on success, -1 on failure.
 *
 * This function replaces the old file atomically, if possible.  This
 * function, and all other functions in util.c that create files, create them
 * with mode 0600.
 */
MOCK_IMPL(int,
write_str_to_file,(const char *fname, const char *str, int bin))
{
#ifdef _WIN32
  if (!bin && strchr(str, '\r')) {
    log_warn(LD_BUG,
             "We're writing a text string that already contains a CR to %s",
             escaped(fname));
  }
#endif /* defined(_WIN32) */
  return write_bytes_to_file(fname, str, strlen(str), bin);
}

/** Represents a file that we're writing to, with support for atomic commit:
 * we can write into a temporary file, and either remove the file on
 * failure, or replace the original file on success. */
struct open_file_t {
  char *tempname; /**< Name of the temporary file. */
  char *filename; /**< Name of the original file. */
  unsigned rename_on_close:1; /**< Are we using the temporary file or not? */
  unsigned binary:1; /**< Did we open in binary mode? */
  int fd; /**< fd for the open file. */
  FILE *stdio_file; /**< stdio wrapper for <b>fd</b>. */
};

/** Try to start writing to the file in <b>fname</b>, passing the flags
 * <b>open_flags</b> to the open() syscall, creating the file (if needed) with
 * access value <b>mode</b>.  If the O_APPEND flag is set, we append to the
 * original file.  Otherwise, we open a new temporary file in the same
 * directory, and either replace the original or remove the temporary file
 * when we're done.
 *
 * Return the fd for the newly opened file, and store working data in
 * *<b>data_out</b>.  The caller should not close the fd manually:
 * instead, call finish_writing_to_file() or abort_writing_to_file().
 * Returns -1 on failure.
 *
 * NOTE: When not appending, the flags O_CREAT and O_TRUNC are treated
 * as true and the flag O_EXCL is treated as false.
 *
 * NOTE: Ordinarily, O_APPEND means "seek to the end of the file before each
 * write()".  We don't do that.
 */
int
start_writing_to_file(const char *fname, int open_flags, int mode,
                      open_file_t **data_out)
{
  open_file_t *new_file = tor_malloc_zero(sizeof(open_file_t));
  const char *open_name;
  int append = 0;

  tor_assert(fname);
  tor_assert(data_out);
#if (O_BINARY != 0 && O_TEXT != 0)
  tor_assert((open_flags & (O_BINARY|O_TEXT)) != 0);
#endif
  new_file->fd = -1;
  new_file->filename = tor_strdup(fname);
  if (open_flags & O_APPEND) {
    open_name = fname;
    new_file->rename_on_close = 0;
    append = 1;
    open_flags &= ~O_APPEND;
  } else {
    tor_asprintf(&new_file->tempname, "%s.tmp", fname);
    open_name = new_file->tempname;
    /* We always replace an existing temporary file if there is one. */
    open_flags |= O_CREAT|O_TRUNC;
    open_flags &= ~O_EXCL;
    new_file->rename_on_close = 1;
  }
#if O_BINARY != 0
  if (open_flags & O_BINARY)
    new_file->binary = 1;
#endif

  new_file->fd = tor_open_cloexec(open_name, open_flags, mode);
  if (new_file->fd < 0) {
    log_warn(LD_FS, "Couldn't open \"%s\" (%s) for writing: %s",
        open_name, fname, strerror(errno));
    goto err;
  }
  if (append) {
    if (tor_fd_seekend(new_file->fd) < 0) {
      log_warn(LD_FS, "Couldn't seek to end of file \"%s\": %s", open_name,
               strerror(errno));
      goto err;
    }
  }

  *data_out = new_file;

  return new_file->fd;

 err:
  if (new_file->fd >= 0)
    close(new_file->fd);
  *data_out = NULL;
  tor_free(new_file->filename);
  tor_free(new_file->tempname);
  tor_free(new_file);
  return -1;
}

/** Given <b>file_data</b> from start_writing_to_file(), return a stdio FILE*
 * that can be used to write to the same file.  The caller should not mix
 * stdio calls with non-stdio calls. */
FILE *
fdopen_file(open_file_t *file_data)
{
  tor_assert(file_data);
  if (file_data->stdio_file)
    return file_data->stdio_file;
  tor_assert(file_data->fd >= 0);
  if (!(file_data->stdio_file = fdopen(file_data->fd,
                                       file_data->binary?"ab":"a"))) {
    log_warn(LD_FS, "Couldn't fdopen \"%s\" [%d]: %s", file_data->filename,
             file_data->fd, strerror(errno));
  }
  return file_data->stdio_file;
}

/** Combines start_writing_to_file with fdopen_file(): arguments are as
 * for start_writing_to_file, but  */
FILE *
start_writing_to_stdio_file(const char *fname, int open_flags, int mode,
                            open_file_t **data_out)
{
  FILE *res;
  if (start_writing_to_file(fname, open_flags, mode, data_out)<0)
    return NULL;
  if (!(res = fdopen_file(*data_out))) {
    abort_writing_to_file(*data_out);
    *data_out = NULL;
  }
  return res;
}

/** Helper function: close and free the underlying file and memory in
 * <b>file_data</b>.  If we were writing into a temporary file, then delete
 * that file (if abort_write is true) or replaces the target file with
 * the temporary file (if abort_write is false). */
static int
finish_writing_to_file_impl(open_file_t *file_data, int abort_write)
{
  int r = 0;

  tor_assert(file_data && file_data->filename);
  if (file_data->stdio_file) {
    if (fclose(file_data->stdio_file)) {
      log_warn(LD_FS, "Error closing \"%s\": %s", file_data->filename,
               strerror(errno));
      abort_write = r = -1;
    }
  } else if (file_data->fd >= 0 && close(file_data->fd) < 0) {
    log_warn(LD_FS, "Error flushing \"%s\": %s", file_data->filename,
             strerror(errno));
    abort_write = r = -1;
  }

  if (file_data->rename_on_close) {
    tor_assert(file_data->tempname && file_data->filename);
    if (!abort_write) {
      tor_assert(strcmp(file_data->filename, file_data->tempname));
      if (replace_file(file_data->tempname, file_data->filename)) {
        log_warn(LD_FS, "Error replacing \"%s\": %s", file_data->filename,
                 strerror(errno));
        abort_write = r = -1;
      }
    }
    if (abort_write) {
      int res = unlink(file_data->tempname);
      if (res != 0) {
        /* We couldn't unlink and we'll leave a mess behind */
        log_warn(LD_FS, "Failed to unlink %s: %s",
                 file_data->tempname, strerror(errno));
        r = -1;
      }
    }
  }

  tor_free(file_data->filename);
  tor_free(file_data->tempname);
  tor_free(file_data);

  return r;
}

/** Finish writing to <b>file_data</b>: close the file handle, free memory as
 * needed, and if using a temporary file, replace the original file with
 * the temporary file. */
int
finish_writing_to_file(open_file_t *file_data)
{
  return finish_writing_to_file_impl(file_data, 0);
}

/** Finish writing to <b>file_data</b>: close the file handle, free memory as
 * needed, and if using a temporary file, delete it. */
int
abort_writing_to_file(open_file_t *file_data)
{
  return finish_writing_to_file_impl(file_data, 1);
}

/** Helper: given a set of flags as passed to open(2), open the file
 * <b>fname</b> and write all the sized_chunk_t structs in <b>chunks</b> to
 * the file.  Do so as atomically as possible e.g. by opening temp files and
 * renaming. */
static int
write_chunks_to_file_impl(const char *fname, const smartlist_t *chunks,
                          int open_flags)
{
  open_file_t *file = NULL;
  int fd;
  ssize_t result;
  fd = start_writing_to_file(fname, open_flags, 0600, &file);
  if (fd<0)
    return -1;
  SMARTLIST_FOREACH(chunks, sized_chunk_t *, chunk,
  {
    result = write_all_to_fd(fd, chunk->bytes, chunk->len);
    if (result < 0) {
      log_warn(LD_FS, "Error writing to \"%s\": %s", fname,
          strerror(errno));
      goto err;
    }
    tor_assert((size_t)result == chunk->len);
  });

  return finish_writing_to_file(file);
 err:
  abort_writing_to_file(file);
  return -1;
}

/** Given a smartlist of sized_chunk_t, write them to a file
 * <b>fname</b>, overwriting or creating the file as necessary.
 * If <b>no_tempfile</b> is 0 then the file will be written
 * atomically. */
int
write_chunks_to_file(const char *fname, const smartlist_t *chunks, int bin,
                     int no_tempfile)
{
  int flags = OPEN_FLAGS_REPLACE|(bin?O_BINARY:O_TEXT);

  if (no_tempfile) {
    /* O_APPEND stops write_chunks_to_file from using tempfiles */
    flags |= O_APPEND;
  }
  return write_chunks_to_file_impl(fname, chunks, flags);
}

/** Write <b>len</b> bytes, starting at <b>str</b>, to <b>fname</b>
    using the open() flags passed in <b>flags</b>. */
static int
write_bytes_to_file_impl(const char *fname, const char *str, size_t len,
                         int flags)
{
  int r;
  sized_chunk_t c = { str, len };
  smartlist_t *chunks = smartlist_new();
  smartlist_add(chunks, &c);
  r = write_chunks_to_file_impl(fname, chunks, flags);
  smartlist_free(chunks);
  return r;
}

/** As write_str_to_file, but does not assume a NUL-terminated
 * string. Instead, we write <b>len</b> bytes, starting at <b>str</b>. */
MOCK_IMPL(int,
write_bytes_to_file,(const char *fname, const char *str, size_t len,
                     int bin))
{
  return write_bytes_to_file_impl(fname, str, len,
                                  OPEN_FLAGS_REPLACE|(bin?O_BINARY:O_TEXT));
}

/** As write_bytes_to_file, but if the file already exists, append the bytes
 * to the end of the file instead of overwriting it. */
int
append_bytes_to_file(const char *fname, const char *str, size_t len,
                     int bin)
{
  return write_bytes_to_file_impl(fname, str, len,
                                  OPEN_FLAGS_APPEND|(bin?O_BINARY:O_TEXT));
}

/** Like write_str_to_file(), but also return -1 if there was a file
    already residing in <b>fname</b>. */
int
write_bytes_to_new_file(const char *fname, const char *str, size_t len,
                        int bin)
{
  return write_bytes_to_file_impl(fname, str, len,
                                  OPEN_FLAGS_DONT_REPLACE|
                                  (bin?O_BINARY:O_TEXT));
}

/**
 * Read the contents of the open file <b>fd</b> presuming it is a FIFO
 * (or similar) file descriptor for which the size of the file isn't
 * known ahead of time. Return NULL on failure, and a NUL-terminated
 * string on success.  On success, set <b>sz_out</b> to the number of
 * bytes read.
 */
char *
read_file_to_str_until_eof(int fd, size_t max_bytes_to_read, size_t *sz_out)
{
  ssize_t r;
  size_t pos = 0;
  char *string = NULL;
  size_t string_max = 0;

  if (max_bytes_to_read+1 >= SIZE_T_CEILING) {
    errno = EINVAL;
    return NULL;
  }

  do {
    /* XXXX This "add 1K" approach is a little goofy; if we care about
     * performance here, we should be doubling.  But in practice we shouldn't
     * be using this function on big files anyway. */
    string_max = pos + 1024;
    if (string_max > max_bytes_to_read)
      string_max = max_bytes_to_read + 1;
    string = tor_realloc(string, string_max);
    r = read(fd, string + pos, string_max - pos - 1);
    if (r < 0) {
      int save_errno = errno;
      tor_free(string);
      errno = save_errno;
      return NULL;
    }

    pos += r;
  } while (r > 0 && pos < max_bytes_to_read);

  tor_assert(pos < string_max);
  *sz_out = pos;
  string[pos] = '\0';
  return string;
}

/** Read the contents of <b>filename</b> into a newly allocated
 * string; return the string on success or NULL on failure.
 *
 * If <b>stat_out</b> is provided, store the result of stat()ing the
 * file into <b>stat_out</b>.
 *
 * If <b>flags</b> &amp; RFTS_BIN, open the file in binary mode.
 * If <b>flags</b> &amp; RFTS_IGNORE_MISSING, don't warn if the file
 * doesn't exist.
 *
 * Unless the RFTS_BIN flag is set in <b>flags</b>, this function will strip
 * any CR characters in the return value on all platforms.
 */
/*
 * This function <em>may</em> return an erroneous result if the file
 * is modified while it is running, but must not crash or overflow.
 * Right now, the error case occurs when the file length grows between
 * the call to stat and the call to read_all: the resulting string will
 * be truncated.
 */
MOCK_IMPL(char *,
read_file_to_str, (const char *filename, int flags, struct stat *stat_out))
{
  int fd; /* router file */
  struct stat statbuf;
  char *string;
  ssize_t r;
  int bin = flags & RFTS_BIN;

  tor_assert(filename);

  fd = tor_open_cloexec(filename,O_RDONLY|(bin?O_BINARY:O_TEXT),0);
  if (fd<0) {
    int severity = LOG_WARN;
    int save_errno = errno;
    if (errno == ENOENT && (flags & RFTS_IGNORE_MISSING))
      severity = LOG_INFO;
    log_fn(severity, LD_FS,"Could not open \"%s\": %s",filename,
           strerror(errno));
    errno = save_errno;
    return NULL;
  }

  if (fstat(fd, &statbuf)<0) {
    int save_errno = errno;
    close(fd);
    log_warn(LD_FS,"Could not fstat \"%s\".",filename);
    errno = save_errno;
    return NULL;
  }

#ifndef _WIN32
/** When we detect that we're reading from a FIFO, don't read more than
 * this many bytes.  It's insane overkill for most uses. */
#define FIFO_READ_MAX (1024*1024)
  if (S_ISFIFO(statbuf.st_mode)) {
    size_t sz = 0;
    string = read_file_to_str_until_eof(fd, FIFO_READ_MAX, &sz);
    int save_errno = errno;
    if (string && stat_out) {
      statbuf.st_size = sz;
      memcpy(stat_out, &statbuf, sizeof(struct stat));
    }
    close(fd);
    if (!string)
      errno = save_errno;
    return string;
  }
#endif /* !defined(_WIN32) */

  if ((uint64_t)(statbuf.st_size)+1 >= SIZE_T_CEILING) {
    close(fd);
    errno = EINVAL;
    return NULL;
  }

  string = tor_malloc((size_t)(statbuf.st_size+1));

  r = read_all_from_fd(fd,string,(size_t)statbuf.st_size);
  if (r<0) {
    int save_errno = errno;
    log_warn(LD_FS,"Error reading from file \"%s\": %s", filename,
             strerror(errno));
    tor_free(string);
    close(fd);
    errno = save_errno;
    return NULL;
  }
  string[r] = '\0'; /* NUL-terminate the result. */

  if (!bin && strchr(string, '\r')) {
    log_debug(LD_FS, "We didn't convert CRLF to LF as well as we hoped "
              "when reading %s. Coping.",
              filename);
    tor_strstrip(string, "\r");
    r = strlen(string);
  }
  if (!bin) {
    statbuf.st_size = (size_t) r;
  } else {
    if (r != statbuf.st_size) {
      /* Unless we're using text mode on win32, we'd better have an exact
       * match for size. */
      int save_errno = errno;
      log_warn(LD_FS,"Could read only %d of %ld bytes of file \"%s\".",
               (int)r, (long)statbuf.st_size,filename);
      tor_free(string);
      close(fd);
      errno = save_errno;
      return NULL;
    }
  }
  close(fd);
  if (stat_out) {
    memcpy(stat_out, &statbuf, sizeof(struct stat));
  }

  return string;
}

/** Attempt to read a file <b>fname</b>. If the file's contents is
 * equal to the string <b>str</b>, return 0. Otherwise, attempt to
 * overwrite the file with the contents of <b>str</b> and return
 * the value of write_str_to_file().
 */
int
write_str_to_file_if_not_equal(const char *fname, const char *str)
{
  char *fstr = read_file_to_str(fname, RFTS_IGNORE_MISSING, NULL);
  int rv;

  if (!fstr || strcmp(str, fstr)) {
    rv = write_str_to_file(fname, str, 0);
  } else {
    rv = 0;
  }
  tor_free(fstr);
  return rv;
}

#if !defined(HAVE_GETDELIM) || defined(TOR_UNIT_TESTS)
#include "ext/getdelim.c"
#endif
