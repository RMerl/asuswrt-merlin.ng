/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file storagedir.c
 *
 * \brief An abstraction for a directory full of similar files.
 *
 * Storagedirs are used by our consensus cache code, and may someday also get
 * used for unparseable objects. A large part of the need for this type is to
 * work around the limitations in our sandbox code, where all filenames need
 * to be registered in advance.
 **/

#include "lib/fs/storagedir.h"

#include "lib/container/smartlist.h"
#include "lib/encoding/confline.h"
#include "lib/fs/dir.h"
#include "lib/fs/files.h"
#include "lib/fs/mmap.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/memarea/memarea.h"
#include "lib/sandbox/sandbox.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define FNAME_MIN_NUM 1000

/** A storage_dir_t represents a directory full of similar cached
 * files. Filenames are decimal integers. Files can be cleaned as needed
 * to limit total disk usage. */
struct storage_dir_t {
  /** Directory holding the files for this storagedir. */
  char *directory;
  /** Either NULL, or a directory listing of the directory (as a smartlist
   * of strings */
  smartlist_t *contents;
  /** The largest number of non-temporary files we'll place in the
   * directory. */
  int max_files;
  /** If true, then 'usage' has been computed. */
  int usage_known;
  /** The total number of bytes used in this directory */
  uint64_t usage;
};

/** Create or open a new storage directory at <b>dirname</b>, with
 * capacity for up to <b>max_files</b> files.
 */
storage_dir_t *
storage_dir_new(const char *dirname, int max_files)
{
  if (check_private_dir(dirname, CPD_CREATE, NULL) < 0)
    return NULL;

  storage_dir_t *d = tor_malloc_zero(sizeof(storage_dir_t));
  d->directory = tor_strdup(dirname);
  d->max_files = max_files;
  return d;
}

/**
 * Drop all in-RAM storage for <b>d</b>.  Does not delete any files.
 */
void
storage_dir_free_(storage_dir_t *d)
{
  if (d == NULL)
    return;
  tor_free(d->directory);
  if (d->contents) {
    SMARTLIST_FOREACH(d->contents, char *, cp, tor_free(cp));
    smartlist_free(d->contents);
  }
  tor_free(d);
}

/**
 * Tell the sandbox (if any) configured by <b>cfg</b> to allow the
 * operations that <b>d</b> will need.
 *
 * The presence of this function is why we need an upper limit on the
 * number of files in a storage_dir_t: we need to approve file operations
 * one by one.
 */
int
storage_dir_register_with_sandbox(storage_dir_t *d, sandbox_cfg_t **cfg)
{
  int problems = 0;
  int idx;
  for (idx = FNAME_MIN_NUM; idx < FNAME_MIN_NUM + d->max_files; ++idx) {
    char *path = NULL, *tmppath = NULL;
    tor_asprintf(&path, "%s/%d", d->directory, idx);
    tor_asprintf(&tmppath, "%s/%d.tmp", d->directory, idx);

    problems += sandbox_cfg_allow_open_filename(cfg, tor_strdup(path));
    problems += sandbox_cfg_allow_open_filename(cfg, tor_strdup(tmppath));
    problems += sandbox_cfg_allow_stat_filename(cfg, tor_strdup(path));
    problems += sandbox_cfg_allow_stat_filename(cfg, tor_strdup(tmppath));
    problems += sandbox_cfg_allow_rename(cfg,
                                      tor_strdup(tmppath), tor_strdup(path));

    tor_free(path);
    tor_free(tmppath);
  }

  return problems ? -1 : 0;
}

/**
 * Remove all files in <b>d</b> whose names end with ".tmp".
 *
 * Requires that the contents field of <b>d</b> is set.
 */
static void
storage_dir_clean_tmpfiles(storage_dir_t *d)
{
  if (!d->contents)
    return;
  SMARTLIST_FOREACH_BEGIN(d->contents, char *, fname) {
    if (strcmpend(fname, ".tmp"))
      continue;
    char *path = NULL;
    tor_asprintf(&path, "%s/%s", d->directory, fname);
    if (unlink(sandbox_intern_string(path))) {
      log_warn(LD_FS, "Unable to unlink %s while cleaning "
               "temporary files: %s", escaped(path), strerror(errno));
      tor_free(path);
      continue;
    }
    tor_free(path);
    SMARTLIST_DEL_CURRENT(d->contents, fname);
    tor_free(fname);
  } SMARTLIST_FOREACH_END(fname);

  d->usage_known = 0;
}

/**
 * Re-scan the directory <b>d</b> to learn its contents.
 */
static int
storage_dir_rescan(storage_dir_t *d)
{
  if (d->contents) {
    SMARTLIST_FOREACH(d->contents, char *, cp, tor_free(cp));
    smartlist_free(d->contents);
  }
  d->usage = 0;
  d->usage_known = 0;
  if (NULL == (d->contents = tor_listdir(d->directory))) {
    return -1;
  }
  storage_dir_clean_tmpfiles(d);
  return 0;
}

/**
 * Return a smartlist containing the filenames within <b>d</b>.
 */
const smartlist_t *
storage_dir_list(storage_dir_t *d)
{
  if (! d->contents)
    storage_dir_rescan(d);
  return d->contents;
}

/**
 * Return the total number of bytes used for storage in <b>d</b>.
 */
uint64_t
storage_dir_get_usage(storage_dir_t *d)
{
  if (d->usage_known)
    return d->usage;

  uint64_t total = 0;
  SMARTLIST_FOREACH_BEGIN(storage_dir_list(d), const char *, cp) {
    char *path = NULL;
    struct stat st;
    tor_asprintf(&path, "%s/%s", d->directory, cp);
    if (stat(sandbox_intern_string(path), &st) == 0) {
      total += st.st_size;
    }
    tor_free(path);
  } SMARTLIST_FOREACH_END(cp);

  d->usage = total;
  d->usage_known = 1;
  return d->usage;
}

/** Mmap a specified file within <b>d</b>.
 *
 * On failure, return NULL and set errno as for tor_mmap_file(). */
tor_mmap_t *
storage_dir_map(storage_dir_t *d, const char *fname)
{
  char *path = NULL;
  tor_asprintf(&path, "%s/%s", d->directory, fname);
  tor_mmap_t *result = tor_mmap_file(path);
  int errval = errno;
  tor_free(path);
  if (result == NULL)
    errno = errval;
  return result;
}

/** Read a file within <b>d</b> into a newly allocated buffer.  Set
 * *<b>sz_out</b> to its size. */
uint8_t *
storage_dir_read(storage_dir_t *d, const char *fname, int bin, size_t *sz_out)
{
  const int flags = bin ? RFTS_BIN : 0;

  char *path = NULL;
  tor_asprintf(&path, "%s/%s", d->directory, fname);
  struct stat st;
  char *contents = read_file_to_str(path, flags, &st);
  if (contents && sz_out) {
    // it fits in RAM, so we know its size is less than SIZE_MAX
#if UINT64_MAX > SIZE_MAX
    tor_assert((uint64_t)st.st_size <= SIZE_MAX);
#endif
    *sz_out = (size_t) st.st_size;
  }

  tor_free(path);
  return (uint8_t *) contents;
}

/** Helper: Find an unused filename within the directory */
static char *
find_unused_fname(storage_dir_t *d)
{
  if (!d->contents) {
    if (storage_dir_rescan(d) < 0)
      return NULL;
  }

  char buf[16];
  int i;
  /* Yuck; this is quadratic.  Fortunately, that shouldn't matter much,
   * since disk writes are more expensive by a lot. */
  for (i = FNAME_MIN_NUM; i < FNAME_MIN_NUM + d->max_files; ++i) {
    tor_snprintf(buf, sizeof(buf), "%d", i);
    if (!smartlist_contains_string(d->contents, buf)) {
      return tor_strdup(buf);
    }
  }
  return NULL;
}

/** Helper: As storage_dir_save_bytes_to_file, but store a smartlist of
 * sized_chunk_t rather than a single byte array. */
static int
storage_dir_save_chunks_to_file(storage_dir_t *d,
                                const smartlist_t *chunks,
                                int binary,
                                char **fname_out)
{
  uint64_t total_length = 0;
  char *fname = find_unused_fname(d);
  if (!fname)
    return -1;

  SMARTLIST_FOREACH(chunks, const sized_chunk_t *, ch,
                    total_length += ch->len);

  char *path = NULL;
  tor_asprintf(&path, "%s/%s", d->directory, fname);

  int r = write_chunks_to_file(path, chunks, binary, 0);
  if (r == 0) {
    if (d->usage_known)
      d->usage += total_length;
    if (fname_out) {
      *fname_out = tor_strdup(fname);
    }
    if (d->contents)
      smartlist_add(d->contents, tor_strdup(fname));
  }
  tor_free(fname);
  tor_free(path);
  return r;
}

/** Try to write the <b>length</b> bytes at <b>data</b> into a new file
 * in <b>d</b>.  On success, return 0 and set *<b>fname_out</b> to a
 * newly allocated string containing the filename.  On failure, return
 * -1. */
int
storage_dir_save_bytes_to_file(storage_dir_t *d,
                               const uint8_t *data,
                               size_t length,
                               int binary,
                               char **fname_out)
{
  smartlist_t *chunks = smartlist_new();
  sized_chunk_t chunk = { (const char *)data, length };
  smartlist_add(chunks, &chunk);
  int r = storage_dir_save_chunks_to_file(d, chunks, binary, fname_out);
  smartlist_free(chunks);
  return r;
}

/**
 * As storage_dir_save_bytes_to_file, but saves a NUL-terminated string
 * <b>str</b>.
 */
int
storage_dir_save_string_to_file(storage_dir_t *d,
                                const char *str,
                                int binary,
                                char **fname_out)
{
  return storage_dir_save_bytes_to_file(d,
                (const uint8_t*)str, strlen(str), binary, fname_out);
}

/**
 * As storage_dir_save_bytes_to_file, but associates the data with the
 * key-value pairs in <b>labels</b>. Files stored in this format can be
 * recovered with storage_dir_map_labeled() or storage_dir_read_labeled().
 */
int
storage_dir_save_labeled_to_file(storage_dir_t *d,
                                  const config_line_t *labels,
                                  const uint8_t *data,
                                  size_t length,
                                  char **fname_out)
{
  /*
   * The storage format is to prefix the data with the key-value pairs in
   * <b>labels</b>, and a single NUL separator.  But code outside this module
   * MUST NOT rely on that format.
   */

  smartlist_t *chunks = smartlist_new();
  memarea_t *area = memarea_new();
  const config_line_t *line;
  for (line = labels; line; line = line->next) {
    sized_chunk_t *sz = memarea_alloc(area, sizeof(sized_chunk_t));
    sz->len = strlen(line->key) + 1 + strlen(line->value) + 1;
    const size_t allocated = sz->len + 1;
    char *bytes = memarea_alloc(area, allocated);
    tor_snprintf(bytes, allocated, "%s %s\n", line->key, line->value);
    sz->bytes = bytes;
    smartlist_add(chunks, sz);
  }

  sized_chunk_t *nul = memarea_alloc(area, sizeof(sized_chunk_t));
  nul->len = 1;
  nul->bytes = "\0";
  smartlist_add(chunks, nul);

  sized_chunk_t *datachunk = memarea_alloc(area, sizeof(sized_chunk_t));
  datachunk->bytes = (const char *)data;
  datachunk->len = length;
  smartlist_add(chunks, datachunk);

  int r = storage_dir_save_chunks_to_file(d, chunks, 1, fname_out);
  smartlist_free(chunks);
  memarea_drop_all(area);
  return r;
}

/**
 * Map a file that was created with storage_dir_save_labeled_to_file().  On
 * failure, return NULL.  On success, write a set of newly allocated labels
 * into *<b>labels_out</b>, a pointer to the data into *<b>data_out</b>, and
 * the data's size into *<b>sz_out</b>. On success, also return a tor_mmap_t
 * object whose contents should not be used -- it needs to be kept around,
 * though, for as long as <b>data_out</b> is going to be valid.
 *
 * On failure, set errno as for tor_mmap_file() if the file was missing or
 * empty, and set errno to EINVAL if the file was not in the labeled
 * format expected.
 */
tor_mmap_t *
storage_dir_map_labeled(storage_dir_t *dir,
                         const char *fname,
                         config_line_t **labels_out,
                         const uint8_t **data_out,
                         size_t *sz_out)
{
  tor_mmap_t *m = storage_dir_map(dir, fname);
  int errval;
  if (! m) {
    errval = errno;
    goto err;
  }
  const char *nulp = memchr(m->data, '\0', m->size);
  if (! nulp) {
    errval = EINVAL;
    goto err;
  }
  if (labels_out && config_get_lines(m->data, labels_out, 0) < 0) {
    errval = EINVAL;
    goto err;
  }
  size_t offset = nulp - m->data + 1;
  tor_assert(offset <= m->size);
  *data_out = (const uint8_t *)(m->data + offset);
  *sz_out = m->size - offset;

  return m;
 err:
  tor_munmap_file(m);
  errno = errval;
  return NULL;
}

/** As storage_dir_map_labeled, but return a new byte array containing the
 * data. */
uint8_t *
storage_dir_read_labeled(storage_dir_t *dir,
                          const char *fname,
                          config_line_t **labels_out,
                          size_t *sz_out)
{
  const uint8_t *data = NULL;
  tor_mmap_t *m = storage_dir_map_labeled(dir, fname, labels_out,
                                           &data, sz_out);
  if (m == NULL)
    return NULL;
  uint8_t *result = tor_memdup(data, *sz_out);
  tor_munmap_file(m);
  return result;
}

/* Reduce the cached usage amount in <b>d</b> by <b>removed_file_size</b>.
 * This function is a no-op if <b>d->usage_known</b> is 0. */
static void
storage_dir_reduce_usage(storage_dir_t *d, uint64_t removed_file_size)
{
  if (d->usage_known) {
    if (! BUG(d->usage < removed_file_size)) {
      /* This bug can also be triggered if an external process resized a file
       * between the call to storage_dir_get_usage() that last checked
       * actual usage (rather than relaying on cached usage), and the call to
       * this function. */
      d->usage -= removed_file_size;
    } else {
      /* If we underflowed the cached directory size, re-check the sizes of all
       * the files in the directory. This makes storage_dir_shrink() quadratic,
       * but only if a process is continually changing file sizes in the
       * storage directory (in which case, we have bigger issues).
       *
       * We can't just reset usage_known, because storage_dir_shrink() relies
       * on knowing the usage. */
      storage_dir_rescan(d);
      (void)storage_dir_get_usage(d);
    }
  }
}

/**
 * Remove the file called <b>fname</b> from <b>d</b>.
 */
void
storage_dir_remove_file(storage_dir_t *d,
                        const char *fname)
{
  char *path = NULL;
  tor_asprintf(&path, "%s/%s", d->directory, fname);
  const char *ipath = sandbox_intern_string(path);

  uint64_t size = 0;
  if (d->usage_known) {
    struct stat st;
    if (stat(ipath, &st) == 0) {
      size = st.st_size;
    }
  }
  if (unlink(ipath) == 0) {
    storage_dir_reduce_usage(d, size);
  } else {
    log_warn(LD_FS, "Unable to unlink %s while removing file: %s",
             escaped(path), strerror(errno));
    tor_free(path);
    return;
  }
  if (d->contents) {
    smartlist_string_remove(d->contents, fname);
  }

  tor_free(path);
}

/** Helper type: used to sort the members of storage directory by mtime. */
typedef struct shrinking_dir_entry_t {
  time_t mtime;
  uint64_t size;
  char *path;
} shrinking_dir_entry_t;

/** Helper: use with qsort to sort shrinking_dir_entry_t structs. */
static int
shrinking_dir_entry_compare(const void *a_, const void *b_)
{
  const shrinking_dir_entry_t *a = a_;
  const shrinking_dir_entry_t *b = b_;

  if (a->mtime < b->mtime)
    return -1;
  else if (a->mtime > b->mtime)
    return 1;
  else
    return 0;
}

/**
 * Try to free space by removing the oldest files in <b>d</b>. Delete
 * until no more than <b>target_size</b> bytes are left, and at least
 * <b>min_to_remove</b> files have been removed... or until there is
 * nothing left to remove.
 *
 * Return 0 on success; -1 on failure.
 */
int
storage_dir_shrink(storage_dir_t *d,
                   uint64_t target_size,
                   int min_to_remove)
{
  if (d->usage_known && d->usage <= target_size && !min_to_remove) {
    /* Already small enough. */
    return 0;
  }

  if (storage_dir_rescan(d) < 0)
    return -1;

  const uint64_t orig_usage = storage_dir_get_usage(d);
  if (orig_usage <= target_size && !min_to_remove) {
    /* Okay, small enough after rescan! */
    return 0;
  }

  const int n = smartlist_len(d->contents);
  shrinking_dir_entry_t *ents = tor_calloc(n, sizeof(shrinking_dir_entry_t));
  SMARTLIST_FOREACH_BEGIN(d->contents, const char *, fname) {
    shrinking_dir_entry_t *ent = &ents[fname_sl_idx];
    struct stat st;
    tor_asprintf(&ent->path, "%s/%s", d->directory, fname);
    if (stat(sandbox_intern_string(ent->path), &st) == 0) {
      ent->mtime = st.st_mtime;
      ent->size = st.st_size;
    }
  } SMARTLIST_FOREACH_END(fname);

  qsort(ents, n, sizeof(shrinking_dir_entry_t), shrinking_dir_entry_compare);

  int idx = 0;
  while ((d->usage > target_size || min_to_remove > 0) && idx < n) {
    if (unlink(sandbox_intern_string(ents[idx].path)) == 0) {
      storage_dir_reduce_usage(d, ents[idx].size);
      --min_to_remove;
    }
    ++idx;
  }

  for (idx = 0; idx < n; ++idx) {
    tor_free(ents[idx].path);
  }
  tor_free(ents);

  storage_dir_rescan(d);

  return 0;
}

/** Remove all files in <b>d</b>. */
int
storage_dir_remove_all(storage_dir_t *d)
{
  return storage_dir_shrink(d, 0, d->max_files);
}

/**
 * Return the largest number of non-temporary files we're willing to
 * store in <b>d</b>.
 */
int
storage_dir_get_max_files(storage_dir_t *d)
{
  return d->max_files;
}
