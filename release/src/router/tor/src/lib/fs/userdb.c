/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file userdb.c
 *
 * \brief Access the POSIX user database.
 **/

#include "lib/fs/userdb.h"

#ifndef _WIN32
#include "lib/malloc/malloc.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#include <pwd.h>
#include <stddef.h>
#include <string.h>

/** Cached struct from the last getpwname() call we did successfully. */
static struct passwd *passwd_cached = NULL;

/** Helper: copy a struct passwd object.
 *
 * We only copy the fields pw_uid, pw_gid, pw_name, pw_dir.  Tor doesn't use
 * any others, and I don't want to run into incompatibilities.
 */
static struct passwd *
tor_passwd_dup(const struct passwd *pw)
{
  struct passwd *new_pw = tor_malloc_zero(sizeof(struct passwd));
  if (pw->pw_name)
    new_pw->pw_name = tor_strdup(pw->pw_name);
  if (pw->pw_dir)
    new_pw->pw_dir = tor_strdup(pw->pw_dir);
  new_pw->pw_uid = pw->pw_uid;
  new_pw->pw_gid = pw->pw_gid;

  return new_pw;
}

#define tor_passwd_free(pw) \
  FREE_AND_NULL(struct passwd, tor_passwd_free_, (pw))

/** Helper: free one of our cached 'struct passwd' values. */
static void
tor_passwd_free_(struct passwd *pw)
{
  if (!pw)
    return;

  tor_free(pw->pw_name);
  tor_free(pw->pw_dir);
  tor_free(pw);
}

/** Wrapper around getpwnam() that caches result. Used so that we don't need
 * to give the sandbox access to /etc/passwd.
 *
 * The following fields alone will definitely be copied in the output: pw_uid,
 * pw_gid, pw_name, pw_dir.  Other fields are not present in cached values.
 *
 * When called with a NULL argument, this function clears storage associated
 * with static variables it uses.
 **/
const struct passwd *
tor_getpwnam(const char *username)
{
  struct passwd *pw;

  if (username == NULL) {
    tor_passwd_free(passwd_cached);
    passwd_cached = NULL;
    return NULL;
  }

  if ((pw = getpwnam(username))) {
    tor_passwd_free(passwd_cached);
    passwd_cached = tor_passwd_dup(pw);
    log_info(LD_GENERAL, "Caching new entry %s for %s",
             passwd_cached->pw_name, username);
    return pw;
  }

  /* Lookup failed */
  if (! passwd_cached || ! passwd_cached->pw_name)
    return NULL;

  if (! strcmp(username, passwd_cached->pw_name))
    return passwd_cached; // LCOV_EXCL_LINE - would need to make getpwnam flaky

  return NULL;
}

/** Wrapper around getpwnam() that can use cached result from
 * tor_getpwnam(). Used so that we don't need to give the sandbox access to
 * /etc/passwd.
 *
 * The following fields alone will definitely be copied in the output: pw_uid,
 * pw_gid, pw_name, pw_dir.  Other fields are not present in cached values.
 */
const struct passwd *
tor_getpwuid(uid_t uid)
{
  struct passwd *pw;

  if ((pw = getpwuid(uid))) {
    return pw;
  }

  /* Lookup failed */
  if (! passwd_cached)
    return NULL;

  if (uid == passwd_cached->pw_uid)
    return passwd_cached; // LCOV_EXCL_LINE - would need to make getpwnam flaky

  return NULL;
}

/** Allocate and return a string containing the home directory for the
 * user <b>username</b>. Only works on posix-like systems. */
char *
get_user_homedir(const char *username)
{
  const struct passwd *pw;
  tor_assert(username);

  if (!(pw = tor_getpwnam(username))) {
    log_err(LD_CONFIG,"User \"%s\" not found.", username);
    return NULL;
  }
  return tor_strdup(pw->pw_dir);
}
#endif /* !defined(_WIN32) */
