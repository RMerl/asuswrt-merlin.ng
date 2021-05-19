/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file unparseable.c
 * @brief Dump unparseable objects to disk.
 **/

#define UNPARSEABLE_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dirparse/unparseable.h"
#include "lib/sandbox/sandbox.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* Dump mechanism for unparseable descriptors */

/** List of dumped descriptors for FIFO cleanup purposes */
STATIC smartlist_t *descs_dumped = NULL;
/** Total size of dumped descriptors for FIFO cleanup */
STATIC uint64_t len_descs_dumped = 0;
/** Directory to stash dumps in */
static int have_dump_desc_dir = 0;
static int problem_with_dump_desc_dir = 0;

#define DESC_DUMP_DATADIR_SUBDIR "unparseable-descs"
#define DESC_DUMP_BASE_FILENAME "unparseable-desc"

/** Find the dump directory and check if we'll be able to create it */
void
dump_desc_init(void)
{
  char *dump_desc_dir;

  dump_desc_dir = get_datadir_fname(DESC_DUMP_DATADIR_SUBDIR);

  /*
   * We just check for it, don't create it at this point; we'll
   * create it when we need it if it isn't already there.
   */
  if (check_private_dir(dump_desc_dir, CPD_CHECK, get_options()->User) < 0) {
    /* Error, log and flag it as having a problem */
    log_notice(LD_DIR,
               "Doesn't look like we'll be able to create descriptor dump "
               "directory %s; dumps will be disabled.",
               dump_desc_dir);
    problem_with_dump_desc_dir = 1;
    tor_free(dump_desc_dir);
    return;
  }

  /* Check if it exists */
  switch (file_status(dump_desc_dir)) {
    case FN_DIR:
      /* We already have a directory */
      have_dump_desc_dir = 1;
      break;
    case FN_NOENT:
      /* Nothing, we'll need to create it later */
      have_dump_desc_dir = 0;
      break;
    case FN_ERROR:
      /* Log and flag having a problem */
      log_notice(LD_DIR,
                 "Couldn't check whether descriptor dump directory %s already"
                 " exists: %s",
                 dump_desc_dir, strerror(errno));
      problem_with_dump_desc_dir = 1;
      break;
    case FN_FILE:
    case FN_EMPTY:
    default:
      /* Something else was here! */
      log_notice(LD_DIR,
                 "Descriptor dump directory %s already exists and isn't a "
                 "directory",
                 dump_desc_dir);
      problem_with_dump_desc_dir = 1;
  }

  if (have_dump_desc_dir && !problem_with_dump_desc_dir) {
    dump_desc_populate_fifo_from_directory(dump_desc_dir);
  }

  tor_free(dump_desc_dir);
}

/** Create the dump directory if needed and possible */
static void
dump_desc_create_dir(void)
{
  char *dump_desc_dir;

  /* If the problem flag is set, skip it */
  if (problem_with_dump_desc_dir) return;

  /* Do we need it? */
  if (!have_dump_desc_dir) {
    dump_desc_dir = get_datadir_fname(DESC_DUMP_DATADIR_SUBDIR);

    if (check_private_dir(dump_desc_dir, CPD_CREATE,
                          get_options()->User) < 0) {
      log_notice(LD_DIR,
                 "Failed to create descriptor dump directory %s",
                 dump_desc_dir);
      problem_with_dump_desc_dir = 1;
    }

    /* Okay, we created it */
    have_dump_desc_dir = 1;

    tor_free(dump_desc_dir);
  }
}

/** Dump desc FIFO/cleanup; take ownership of the given filename, add it to
 * the FIFO, and clean up the oldest entries to the extent they exceed the
 * configured cap.  If any old entries with a matching hash existed, they
 * just got overwritten right before this was called and we should adjust
 * the total size counter without deleting them.
 */
static void
dump_desc_fifo_add_and_clean(char *filename, const uint8_t *digest_sha256,
                             size_t len)
{
  dumped_desc_t *ent = NULL, *tmp;
  uint64_t max_len;

  tor_assert(filename != NULL);
  tor_assert(digest_sha256 != NULL);

  if (descs_dumped == NULL) {
    /* We better have no length, then */
    tor_assert(len_descs_dumped == 0);
    /* Make a smartlist */
    descs_dumped = smartlist_new();
  }

  /* Make a new entry to put this one in */
  ent = tor_malloc_zero(sizeof(*ent));
  ent->filename = filename;
  ent->len = len;
  ent->when = time(NULL);
  memcpy(ent->digest_sha256, digest_sha256, DIGEST256_LEN);

  /* Do we need to do some cleanup? */
  max_len = get_options()->MaxUnparseableDescSizeToLog;
  /* Iterate over the list until we've freed enough space */
  while (len > max_len - len_descs_dumped &&
         smartlist_len(descs_dumped) > 0) {
    /* Get the oldest thing on the list */
    tmp = (dumped_desc_t *)(smartlist_get(descs_dumped, 0));

    /*
     * Check if it matches the filename we just added, so we don't delete
     * something we just emitted if we get repeated identical descriptors.
     */
    if (strcmp(tmp->filename, filename) != 0) {
      /* Delete it and adjust the length counter */
      tor_unlink(tmp->filename);
      tor_assert(len_descs_dumped >= tmp->len);
      len_descs_dumped -= tmp->len;
      log_info(LD_DIR,
               "Deleting old unparseable descriptor dump %s due to "
               "space limits",
               tmp->filename);
    } else {
      /*
       * Don't delete, but do adjust the counter since we will bump it
       * later
       */
      tor_assert(len_descs_dumped >= tmp->len);
      len_descs_dumped -= tmp->len;
      log_info(LD_DIR,
               "Replacing old descriptor dump %s with new identical one",
               tmp->filename);
    }

    /* Free it and remove it from the list */
    smartlist_del_keeporder(descs_dumped, 0);
    tor_free(tmp->filename);
    tor_free(tmp);
  }

  /* Append our entry to the end of the list and bump the counter */
  smartlist_add(descs_dumped, ent);
  len_descs_dumped += len;
}

/** Check if we already have a descriptor for this hash and move it to the
 * head of the queue if so.  Return 1 if one existed and 0 otherwise.
 */
static int
dump_desc_fifo_bump_hash(const uint8_t *digest_sha256)
{
  dumped_desc_t *match = NULL;

  tor_assert(digest_sha256);

  if (descs_dumped) {
    /* Find a match if one exists */
    SMARTLIST_FOREACH_BEGIN(descs_dumped, dumped_desc_t *, ent) {
      if (ent &&
          tor_memeq(ent->digest_sha256, digest_sha256, DIGEST256_LEN)) {
        /*
         * Save a pointer to the match and remove it from its current
         * position.
         */
        match = ent;
        SMARTLIST_DEL_CURRENT_KEEPORDER(descs_dumped, ent);
        break;
      }
    } SMARTLIST_FOREACH_END(ent);

    if (match) {
      /* Update the timestamp */
      match->when = time(NULL);
      /* Add it back at the end of the list */
      smartlist_add(descs_dumped, match);

      /* Indicate we found one */
      return 1;
    }
  }

  return 0;
}

/** Clean up on exit; just memory, leave the dumps behind
 */
void
dump_desc_fifo_cleanup(void)
{
  if (descs_dumped) {
    /* Free each descriptor */
    SMARTLIST_FOREACH_BEGIN(descs_dumped, dumped_desc_t *, ent) {
      tor_assert(ent);
      tor_free(ent->filename);
      tor_free(ent);
    } SMARTLIST_FOREACH_END(ent);
    /* Free the list */
    smartlist_free(descs_dumped);
    descs_dumped = NULL;
    len_descs_dumped = 0;
  }
}

/** Handle one file for dump_desc_populate_fifo_from_directory(); make sure
 * the filename is sensibly formed and matches the file content, and either
 * return a dumped_desc_t for it or remove the file and return NULL.
 */
MOCK_IMPL(STATIC dumped_desc_t *,
dump_desc_populate_one_file, (const char *dirname, const char *f))
{
  dumped_desc_t *ent = NULL;
  char *path = NULL, *desc = NULL;
  const char *digest_str;
  char digest[DIGEST256_LEN], content_digest[DIGEST256_LEN];
  /* Expected prefix before digest in filenames */
  const char *f_pfx = DESC_DUMP_BASE_FILENAME ".";
  /*
   * Stat while reading; this is important in case the file
   * contains a NUL character.
   */
  struct stat st;

  /* Sanity-check args */
  tor_assert(dirname != NULL);
  tor_assert(f != NULL);

  /* Form the full path */
  tor_asprintf(&path, "%s" PATH_SEPARATOR "%s", dirname, f);

  /* Check that f has the form DESC_DUMP_BASE_FILENAME.<digest256> */

  if (!strcmpstart(f, f_pfx)) {
    /* It matches the form, but is the digest parseable as such? */
    digest_str = f + strlen(f_pfx);
    if (base16_decode(digest, DIGEST256_LEN,
                      digest_str, strlen(digest_str)) != DIGEST256_LEN) {
      /* We failed to decode it */
      digest_str = NULL;
    }
  } else {
    /* No match */
    digest_str = NULL;
  }

  if (!digest_str) {
    /* We couldn't get a sensible digest */
    log_notice(LD_DIR,
               "Removing unrecognized filename %s from unparseable "
               "descriptors directory", f);
    tor_unlink(path);
    /* We're done */
    goto done;
  }

  /*
   * The filename has the form DESC_DUMP_BASE_FILENAME "." <digest256> and
   * we've decoded the digest.  Next, check that we can read it and the
   * content matches this digest.  We are relying on the fact that if the
   * file contains a '\0', read_file_to_str() will allocate space for and
   * read the entire file and return the correct size in st.
   */
  desc = read_file_to_str(path, RFTS_IGNORE_MISSING|RFTS_BIN, &st);
  if (!desc) {
    /* We couldn't read it */
    log_notice(LD_DIR,
               "Failed to read %s from unparseable descriptors directory; "
               "attempting to remove it.", f);
    tor_unlink(path);
    /* We're done */
    goto done;
  }

#if SIZE_MAX > UINT64_MAX
  if (BUG((uint64_t)st.st_size > (uint64_t)SIZE_MAX)) {
    /* LCOV_EXCL_START
     * Should be impossible since RFTS above should have failed to read the
     * huge file into RAM. */
    goto done;
    /* LCOV_EXCL_STOP */
  }
#endif /* SIZE_MAX > UINT64_MAX */
  if (BUG(st.st_size < 0)) {
    /* LCOV_EXCL_START
     * Should be impossible, since the OS isn't supposed to be b0rken. */
    goto done;
    /* LCOV_EXCL_STOP */
  }
  /* (Now we can be sure that st.st_size is safe to cast to a size_t.) */

  /*
   * We got one; now compute its digest and check that it matches the
   * filename.
   */
  if (crypto_digest256((char *)content_digest, desc, (size_t) st.st_size,
                       DIGEST_SHA256) < 0) {
    /* Weird, but okay */
    log_info(LD_DIR,
             "Unable to hash content of %s from unparseable descriptors "
             "directory", f);
    tor_unlink(path);
    /* We're done */
    goto done;
  }

  /* Compare the digests */
  if (tor_memneq(digest, content_digest, DIGEST256_LEN)) {
    /* No match */
    log_info(LD_DIR,
             "Hash of %s from unparseable descriptors directory didn't "
             "match its filename; removing it", f);
    tor_unlink(path);
    /* We're done */
    goto done;
  }

  /* Okay, it's a match, we should prepare ent */
  ent = tor_malloc_zero(sizeof(dumped_desc_t));
  ent->filename = path;
  memcpy(ent->digest_sha256, digest, DIGEST256_LEN);
  ent->len = (size_t) st.st_size;
  ent->when = st.st_mtime;
  /* Null out path so we don't free it out from under ent */
  path = NULL;

 done:
  /* Free allocations if we had them */
  tor_free(desc);
  tor_free(path);

  return ent;
}

/** Sort helper for dump_desc_populate_fifo_from_directory(); compares
 * the when field of dumped_desc_ts in a smartlist to put the FIFO in
 * the correct order after reconstructing it from the directory.
 */
static int
dump_desc_compare_fifo_entries(const void **a_v, const void **b_v)
{
  const dumped_desc_t **a = (const dumped_desc_t **)a_v;
  const dumped_desc_t **b = (const dumped_desc_t **)b_v;

  if ((a != NULL) && (*a != NULL)) {
    if ((b != NULL) && (*b != NULL)) {
      /* We have sensible dumped_desc_ts to compare */
      if ((*a)->when < (*b)->when) {
        return -1;
      } else if ((*a)->when == (*b)->when) {
        return 0;
      } else {
        return 1;
      }
    } else {
      /*
       * We shouldn't see this, but what the hell, NULLs precede everythin
       * else
       */
      return 1;
    }
  } else {
    return -1;
  }
}

/** Scan the contents of the directory, and update FIFO/counters; this will
 * consistency-check descriptor dump filenames against hashes of descriptor
 * dump file content, and remove any inconsistent/unreadable dumps, and then
 * reconstruct the dump FIFO as closely as possible for the last time the
 * tor process shut down.  If a previous dump was repeated more than once and
 * moved ahead in the FIFO, the mtime will not have been updated and the
 * reconstructed order will be wrong, but will always be a permutation of
 * the original.
 */
STATIC void
dump_desc_populate_fifo_from_directory(const char *dirname)
{
  smartlist_t *files = NULL;
  dumped_desc_t *ent = NULL;

  tor_assert(dirname != NULL);

  /* Get a list of files */
  files = tor_listdir(dirname);
  if (!files) {
    log_notice(LD_DIR,
               "Unable to get contents of unparseable descriptor dump "
               "directory %s",
               dirname);
    return;
  }

  /*
   * Iterate through the list and decide which files should go in the
   * FIFO and which should be purged.
   */

  SMARTLIST_FOREACH_BEGIN(files, char *, f) {
    /* Try to get a FIFO entry */
    ent = dump_desc_populate_one_file(dirname, f);
    if (ent) {
      /*
       * We got one; add it to the FIFO.  No need for duplicate checking
       * here since we just verified the name and digest match.
       */

      /* Make sure we have a list to add it to */
      if (!descs_dumped) {
        descs_dumped = smartlist_new();
        len_descs_dumped = 0;
      }

      /* Add it and adjust the counter */
      smartlist_add(descs_dumped, ent);
      len_descs_dumped += ent->len;
    }
    /*
     * If we didn't, we will have unlinked the file if necessary and
     * possible, and emitted a log message about it, so just go on to
     * the next.
     */
  } SMARTLIST_FOREACH_END(f);

  /* Did we get anything? */
  if (descs_dumped != NULL) {
    /* Sort the FIFO in order of increasing timestamp */
    smartlist_sort(descs_dumped, dump_desc_compare_fifo_entries);

    /* Log some stats */
    log_info(LD_DIR,
             "Reloaded unparseable descriptor dump FIFO with %d dump(s) "
             "totaling %"PRIu64 " bytes",
             smartlist_len(descs_dumped), (len_descs_dumped));
  }

  /* Free the original list */
  SMARTLIST_FOREACH(files, char *, f, tor_free(f));
  smartlist_free(files);
}

/** For debugging purposes, dump unparseable descriptor *<b>desc</b> of
 * type *<b>type</b> to file $DATADIR/unparseable-desc. Do not write more
 * than one descriptor to disk per minute. If there is already such a
 * file in the data directory, overwrite it. */
MOCK_IMPL(void,
dump_desc,(const char *desc, const char *type))
{
  tor_assert(desc);
  tor_assert(type);
#ifndef TOR_UNIT_TESTS
  /* For now, we are disabling this function, since it can be called with
   * strings that are far too long.  We can turn it back on if we fix it
   * someday, but we'd need to give it a length argument. A likelier
   * resolution here is simply to remove this module entirely.  See tor#40286
   * for background. */
  if (1)
    return;
#endif
  size_t len;
  /* The SHA256 of the string */
  uint8_t digest_sha256[DIGEST256_LEN];
  char digest_sha256_hex[HEX_DIGEST256_LEN+1];
  /* Filename to log it to */
  char *debugfile, *debugfile_base;

  /* Get the hash for logging purposes anyway */
  len = strlen(desc);
  if (crypto_digest256((char *)digest_sha256, desc, len,
                       DIGEST_SHA256) < 0) {
    log_info(LD_DIR,
             "Unable to parse descriptor of type %s, and unable to even hash"
             " it!", type);
    goto err;
  }

  base16_encode(digest_sha256_hex, sizeof(digest_sha256_hex),
                (const char *)digest_sha256, sizeof(digest_sha256));

  /*
   * We mention type and hash in the main log; don't clutter up the files
   * with anything but the exact dump.
   */
  tor_asprintf(&debugfile_base,
               DESC_DUMP_BASE_FILENAME ".%s", digest_sha256_hex);
  debugfile = get_datadir_fname2(DESC_DUMP_DATADIR_SUBDIR, debugfile_base);

  /*
   * Check if the sandbox is active or will become active; see comment
   * below at the log message for why.
   */
  if (!(sandbox_is_active() || get_options()->Sandbox)) {
    if (len <= get_options()->MaxUnparseableDescSizeToLog) {
      if (!dump_desc_fifo_bump_hash(digest_sha256)) {
        /* Create the directory if needed */
        dump_desc_create_dir();
        /* Make sure we've got it */
        if (have_dump_desc_dir && !problem_with_dump_desc_dir) {
          /* Write it, and tell the main log about it */
          write_str_to_file(debugfile, desc, 1);
          log_info(LD_DIR,
                   "Unable to parse descriptor of type %s with hash %s and "
                   "length %lu. See file %s in data directory for details.",
                   type, digest_sha256_hex, (unsigned long)len,
                   debugfile_base);
          dump_desc_fifo_add_and_clean(debugfile, digest_sha256, len);
          /* Since we handed ownership over, don't free debugfile later */
          debugfile = NULL;
        } else {
          /* Problem with the subdirectory */
          log_info(LD_DIR,
                   "Unable to parse descriptor of type %s with hash %s and "
                   "length %lu. Descriptor not dumped because we had a "
                   "problem creating the " DESC_DUMP_DATADIR_SUBDIR
                   " subdirectory",
                   type, digest_sha256_hex, (unsigned long)len);
          /* We do have to free debugfile in this case */
        }
      } else {
        /* We already had one with this hash dumped */
        log_info(LD_DIR,
                 "Unable to parse descriptor of type %s with hash %s and "
                 "length %lu. Descriptor not dumped because one with that "
                 "hash has already been dumped.",
                 type, digest_sha256_hex, (unsigned long)len);
        /* We do have to free debugfile in this case */
      }
    } else {
      /* Just log that it happened without dumping */
      log_info(LD_DIR,
               "Unable to parse descriptor of type %s with hash %s and "
               "length %lu. Descriptor not dumped because it exceeds maximum"
               " log size all by itself.",
               type, digest_sha256_hex, (unsigned long)len);
      /* We do have to free debugfile in this case */
    }
  } else {
    /*
     * Not logging because the sandbox is active and seccomp2 apparently
     * doesn't have a sensible way to allow filenames according to a pattern
     * match.  (If we ever figure out how to say "allow writes to /regex/",
     * remove this checK).
     */
    log_info(LD_DIR,
             "Unable to parse descriptor of type %s with hash %s and "
             "length %lu. Descriptor not dumped because the sandbox is "
             "configured",
             type, digest_sha256_hex, (unsigned long)len);
  }

  tor_free(debugfile_base);
  tor_free(debugfile);

 err:
  return;
}
