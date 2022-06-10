/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file setuid.c
 * \brief Change the user ID after Tor has started (Unix only)
 **/

#include "orconfig.h"
#include "lib/process/setuid.h"

#if defined(HAVE_SYS_CAPABILITY_H) && defined(HAVE_CAP_SET_PROC)
#define HAVE_LINUX_CAPABILITIES
#endif

#include "lib/container/smartlist.h"
#include "lib/fs/userdb.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif
#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#include <errno.h>
#include <string.h>

#ifndef _WIN32
/** Log details of current user and group credentials. Return 0 on
 * success. Logs and return -1 on failure.
 */
static int
log_credential_status(void)
{
/** Log level to use when describing non-error UID/GID status. */
#define CREDENTIAL_LOG_LEVEL LOG_INFO
  /* Real, effective and saved UIDs */
  uid_t ruid, euid, suid;
  /* Read, effective and saved GIDs */
  gid_t rgid, egid, sgid;
  /* Supplementary groups */
  gid_t *sup_gids = NULL;
  int sup_gids_size;
  /* Number of supplementary groups */
  int ngids;

  /* log UIDs */
#ifdef HAVE_GETRESUID
  if (getresuid(&ruid, &euid, &suid) != 0) {
    log_warn(LD_GENERAL, "Error getting changed UIDs: %s", strerror(errno));
    return -1;
  } else {
    log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL,
           "UID is %u (real), %u (effective), %u (saved)",
           (unsigned)ruid, (unsigned)euid, (unsigned)suid);
  }
#else /* !defined(HAVE_GETRESUID) */
  /* getresuid is not present on MacOS X, so we can't get the saved (E)UID */
  ruid = getuid();
  euid = geteuid();
  (void)suid;

  log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL,
         "UID is %u (real), %u (effective), unknown (saved)",
         (unsigned)ruid, (unsigned)euid);
#endif /* defined(HAVE_GETRESUID) */

  /* log GIDs */
#ifdef HAVE_GETRESGID
  if (getresgid(&rgid, &egid, &sgid) != 0) {
    log_warn(LD_GENERAL, "Error getting changed GIDs: %s", strerror(errno));
    return -1;
  } else {
    log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL,
           "GID is %u (real), %u (effective), %u (saved)",
           (unsigned)rgid, (unsigned)egid, (unsigned)sgid);
  }
#else /* !defined(HAVE_GETRESGID) */
  /* getresgid is not present on MacOS X, so we can't get the saved (E)GID */
  rgid = getgid();
  egid = getegid();
  (void)sgid;
  log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL,
         "GID is %u (real), %u (effective), unknown (saved)",
         (unsigned)rgid, (unsigned)egid);
#endif /* defined(HAVE_GETRESGID) */

  /* log supplementary groups */
  sup_gids_size = 64;
  sup_gids = tor_calloc(64, sizeof(gid_t));
  while ((ngids = getgroups(sup_gids_size, sup_gids)) < 0 &&
         errno == EINVAL &&
         sup_gids_size < NGROUPS_MAX) {
    sup_gids_size *= 2;
    sup_gids = tor_reallocarray(sup_gids, sizeof(gid_t), sup_gids_size);
  }

  if (ngids < 0) {
    log_warn(LD_GENERAL, "Error getting supplementary GIDs: %s",
             strerror(errno));
    tor_free(sup_gids);
    return -1;
  } else {
    int i, retval = 0;
    char *s = NULL;
    smartlist_t *elts = smartlist_new();

    for (i = 0; i<ngids; i++) {
      smartlist_add_asprintf(elts, "%u", (unsigned)sup_gids[i]);
    }

    s = smartlist_join_strings(elts, " ", 0, NULL);

    log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL, "Supplementary groups are: %s",s);

    tor_free(s);
    SMARTLIST_FOREACH(elts, char *, cp, tor_free(cp));
    smartlist_free(elts);
    tor_free(sup_gids);

    return retval;
  }

  return 0;
}
#endif /* !defined(_WIN32) */

/** Return true iff we were compiled with capability support, and capabilities
 * seem to work. **/
int
have_capability_support(void)
{
#ifdef HAVE_LINUX_CAPABILITIES
  cap_t caps = cap_get_proc();
  if (caps == NULL)
    return 0;
  cap_free(caps);
  return 1;
#else /* !defined(HAVE_LINUX_CAPABILITIES) */
  return 0;
#endif /* defined(HAVE_LINUX_CAPABILITIES) */
}

#ifdef HAVE_LINUX_CAPABILITIES
/** Helper. Drop all capabilities but a small set, and set PR_KEEPCAPS as
 * appropriate.
 *
 * If pre_setuid, retain only CAP_NET_BIND_SERVICE, CAP_SETUID, and
 * CAP_SETGID, and use PR_KEEPCAPS to ensure that capabilities persist across
 * setuid().
 *
 * If not pre_setuid, retain only CAP_NET_BIND_SERVICE, and disable
 * PR_KEEPCAPS.
 *
 * Return 0 on success, and -1 on failure.
 */
static int
drop_capabilities(int pre_setuid)
{
  /* We keep these three capabilities, and these only, as we setuid.
   * After we setuid, we drop all but the first. */
  const cap_value_t caplist[] = {
    CAP_NET_BIND_SERVICE, CAP_SETUID, CAP_SETGID
  };
  const char *where = pre_setuid ? "pre-setuid" : "post-setuid";
  const int n_effective = pre_setuid ? 3 : 1;
  const int n_permitted = pre_setuid ? 3 : 1;
  const int n_inheritable = 1;
  const int keepcaps = pre_setuid ? 1 : 0;

  /* Sets whether we keep capabilities across a setuid. */
  if (prctl(PR_SET_KEEPCAPS, keepcaps) < 0) {
    log_warn(LD_CONFIG, "Unable to call prctl() %s: %s",
             where, strerror(errno));
    return -1;
  }

  cap_t caps = cap_get_proc();
  if (!caps) {
    log_warn(LD_CONFIG, "Unable to call cap_get_proc() %s: %s",
             where, strerror(errno));
    return -1;
  }
  cap_clear(caps);

  cap_set_flag(caps, CAP_EFFECTIVE, n_effective, caplist, CAP_SET);
  cap_set_flag(caps, CAP_PERMITTED, n_permitted, caplist, CAP_SET);
  cap_set_flag(caps, CAP_INHERITABLE, n_inheritable, caplist, CAP_SET);

  int r = cap_set_proc(caps);
  cap_free(caps);
  if (r < 0) {
    log_warn(LD_CONFIG, "No permission to set capabilities %s: %s",
             where, strerror(errno));
    return -1;
  }

  return 0;
}
#endif /* defined(HAVE_LINUX_CAPABILITIES) */

/** Call setuid and setgid to run as <b>user</b> and switch to their
 * primary group.  Return 0 on success.  On failure, log and return -1.
 *
 * If SWITCH_ID_KEEP_BINDLOW is set in 'flags', try to use the capability
 * system to retain the abilitity to bind low ports.
 *
 * If SWITCH_ID_WARN_IF_NO_CAPS is set in flags, also warn if we have
 * don't have capability support.
 */
int
switch_id(const char *user, const unsigned flags)
{
#ifndef _WIN32
  const struct passwd *pw = NULL;
  uid_t old_uid;
  gid_t old_gid;
  static int have_already_switched_id = 0;
  const int keep_bindlow = !!(flags & SWITCH_ID_KEEP_BINDLOW);
  const int warn_if_no_caps = !!(flags & SWITCH_ID_WARN_IF_NO_CAPS);

  tor_assert(user);

  if (have_already_switched_id)
    return 0;

  /* Log the initial credential state */
  if (log_credential_status())
    return -1;

  log_fn(CREDENTIAL_LOG_LEVEL, LD_GENERAL, "Changing user and groups");

  /* Get old UID/GID to check if we changed correctly */
  old_uid = getuid();
  old_gid = getgid();

  /* Lookup the user and group information, if we have a problem, bail out. */
  pw = tor_getpwnam(user);
  if (pw == NULL) {
    log_warn(LD_CONFIG, "Error setting configured user: %s not found", user);
    return -1;
  }

#ifdef HAVE_LINUX_CAPABILITIES
  (void) warn_if_no_caps;
  if (keep_bindlow) {
    if (drop_capabilities(1))
      return -1;
  }
#else /* !defined(HAVE_LINUX_CAPABILITIES) */
  (void) keep_bindlow;
  if (warn_if_no_caps) {
    log_warn(LD_CONFIG, "KeepBindCapabilities set, but no capability support "
             "on this system.");
  }
#endif /* defined(HAVE_LINUX_CAPABILITIES) */

  /* Properly switch egid,gid,euid,uid here or bail out */
  if (setgroups(1, &pw->pw_gid)) {
    log_warn(LD_GENERAL, "Error setting groups to gid %d: \"%s\".",
             (int)pw->pw_gid, strerror(errno));
    if (old_uid == pw->pw_uid) {
      log_warn(LD_GENERAL, "Tor is already running as %s.  You do not need "
               "the \"User\" option if you are already running as the user "
               "you want to be.  (If you did not set the User option in your "
               "torrc, check whether it was specified on the command line "
               "by a startup script.)", user);
    } else {
      log_warn(LD_GENERAL, "If you set the \"User\" option, you must start Tor"
               " as root.");
    }
    return -1;
  }

  if (setegid(pw->pw_gid)) {
    log_warn(LD_GENERAL, "Error setting egid to %d: %s",
             (int)pw->pw_gid, strerror(errno));
    return -1;
  }

  if (setgid(pw->pw_gid)) {
    log_warn(LD_GENERAL, "Error setting gid to %d: %s",
             (int)pw->pw_gid, strerror(errno));
    return -1;
  }

  if (setuid(pw->pw_uid)) {
    log_warn(LD_GENERAL, "Error setting configured uid to %s (%d): %s",
             user, (int)pw->pw_uid, strerror(errno));
    return -1;
  }

  if (seteuid(pw->pw_uid)) {
    log_warn(LD_GENERAL, "Error setting configured euid to %s (%d): %s",
             user, (int)pw->pw_uid, strerror(errno));
    return -1;
  }

  /* This is how OpenBSD rolls:
  if (setgroups(1, &pw->pw_gid) || setegid(pw->pw_gid) ||
      setgid(pw->pw_gid) || setuid(pw->pw_uid) || seteuid(pw->pw_uid)) {
      setgid(pw->pw_gid) || seteuid(pw->pw_uid) || setuid(pw->pw_uid)) {
    log_warn(LD_GENERAL, "Error setting configured UID/GID: %s",
    strerror(errno));
    return -1;
  }
  */

  /* We've properly switched egid, gid, euid, uid, and supplementary groups if
   * we're here. */
#ifdef HAVE_LINUX_CAPABILITIES
  if (keep_bindlow) {
    if (drop_capabilities(0))
      return -1;
  }
#endif /* defined(HAVE_LINUX_CAPABILITIES) */

#if !defined(CYGWIN) && !defined(__CYGWIN__)
  /* If we tried to drop privilege to a group/user other than root, attempt to
   * restore root (E)(U|G)ID, and abort if the operation succeeds */

  /* Only check for privilege dropping if we were asked to be non-root */
  if (pw->pw_uid) {
    /* Try changing GID/EGID */
    if (pw->pw_gid != old_gid &&
        (setgid(old_gid) != -1 || setegid(old_gid) != -1)) {
      log_warn(LD_GENERAL, "Was able to restore group credentials even after "
               "switching GID: this means that the setgid code didn't work.");
      return -1;
    }

    /* Try changing UID/EUID */
    if (pw->pw_uid != old_uid &&
        (setuid(old_uid) != -1 || seteuid(old_uid) != -1)) {
      log_warn(LD_GENERAL, "Was able to restore user credentials even after "
               "switching UID: this means that the setuid code didn't work.");
      return -1;
    }
  }
#endif /* !defined(CYGWIN) && !defined(__CYGWIN__) */

  /* Check what really happened */
  if (log_credential_status()) {
    return -1;
  }

  have_already_switched_id = 1; /* mark success so we never try again */

#if defined(__linux__) && defined(HAVE_SYS_PRCTL_H) && \
  defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
  if (pw->pw_uid) {
    /* Re-enable core dumps if we're not running as root. */
    log_info(LD_CONFIG, "Re-enabling coredumps");
    if (prctl(PR_SET_DUMPABLE, 1)) {
      log_warn(LD_CONFIG, "Unable to re-enable coredumps: %s",strerror(errno));
    }
  }
#endif /* defined(__linux__) && defined(HAVE_SYS_PRCTL_H) && ... */
  return 0;

#else /* defined(_WIN32) */
  (void)user;
  (void)flags;

  log_warn(LD_CONFIG, "Switching users is unsupported on your OS.");
  return -1;
#endif /* !defined(_WIN32) */
}
