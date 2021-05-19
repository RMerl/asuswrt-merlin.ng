/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file env.c
 * \brief Inspect and manipulate the environment variables.
 **/

#include "orconfig.h"
#include "lib/process/env.h"

#include "lib/malloc/malloc.h"
#include "lib/ctime/di_ops.h"
#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/log/log.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_CRT_EXTERNS_H
/* For _NSGetEnviron on macOS */
#include <crt_externs.h>
#endif

#ifndef HAVE__NSGETENVIRON
#ifndef HAVE_EXTERN_ENVIRON_DECLARED
/* Some platforms declare environ under some circumstances, others don't. */
#ifndef RUNNING_DOXYGEN
extern char **environ;
#endif
#endif /* !defined(HAVE_EXTERN_ENVIRON_DECLARED) */
#endif /* !defined(HAVE__NSGETENVIRON) */

/** Return the current environment. This is a portable replacement for
 * 'environ'. */
char **
get_environment(void)
{
#ifdef HAVE__NSGETENVIRON
  /* This is for compatibility between OSX versions.  Otherwise (for example)
   * when we do a mostly-static build on OSX 10.7, the resulting binary won't
   * work on OSX 10.6. */
  return *_NSGetEnviron();
#else /* !defined(HAVE__NSGETENVIRON) */
  return environ;
#endif /* defined(HAVE__NSGETENVIRON) */
}

/** Helper: return the number of characters in <b>s</b> preceding the first
 * occurrence of <b>ch</b>. If <b>ch</b> does not occur in <b>s</b>, return
 * the length of <b>s</b>. Should be equivalent to strspn(s, "ch"). */
static inline size_t
str_num_before(const char *s, char ch)
{
  const char *cp = strchr(s, ch);
  if (cp)
    return cp - s;
  else
    return strlen(s);
}

/** Return non-zero iff getenv would consider <b>s1</b> and <b>s2</b>
 * to have the same name as strings in a process's environment. */
int
environment_variable_names_equal(const char *s1, const char *s2)
{
  size_t s1_name_len = str_num_before(s1, '=');
  size_t s2_name_len = str_num_before(s2, '=');

  return (s1_name_len == s2_name_len &&
          tor_memeq(s1, s2, s1_name_len));
}

/** Free <b>env</b> (assuming it was produced by
 * process_environment_make). */
void
process_environment_free_(process_environment_t *env)
{
  if (env == NULL) return;

  /* As both an optimization hack to reduce consing on Unixoid systems
   * and a nice way to ensure that some otherwise-Windows-specific
   * code will always get tested before changes to it get merged, the
   * strings which env->unixoid_environment_block points to are packed
   * into env->windows_environment_block. */
  tor_free(env->unixoid_environment_block);
  tor_free(env->windows_environment_block);

  tor_free(env);
}

/** Make a process_environment_t containing the environment variables
 * specified in <b>env_vars</b> (as C strings of the form
 * "NAME=VALUE"). */
process_environment_t *
process_environment_make(struct smartlist_t *env_vars)
{
  process_environment_t *env = tor_malloc_zero(sizeof(process_environment_t));
  int n_env_vars = smartlist_len(env_vars);
  int i;
  size_t total_env_length;
  smartlist_t *env_vars_sorted;

  tor_assert(n_env_vars + 1 != 0);
  env->unixoid_environment_block = tor_calloc(n_env_vars + 1, sizeof(char *));
  /* env->unixoid_environment_block is already NULL-terminated,
   * because we assume that NULL == 0 (and check that during compilation). */

  total_env_length = 1; /* terminating NUL of terminating empty string */
  for (i = 0; i < n_env_vars; ++i) {
    const char *s = smartlist_get(env_vars, (int)i);
    size_t slen = strlen(s);

    tor_assert(slen + 1 != 0);
    tor_assert(slen + 1 < SIZE_MAX - total_env_length);
    total_env_length += slen + 1;
  }

  env->windows_environment_block = tor_malloc_zero(total_env_length);
  /* env->windows_environment_block is already
   * (NUL-terminated-empty-string)-terminated. */

  /* Some versions of Windows supposedly require that environment
   * blocks be sorted.  Or maybe some Windows programs (or their
   * runtime libraries) fail to look up strings in non-sorted
   * environment blocks.
   *
   * Also, sorting strings makes it easy to find duplicate environment
   * variables and environment-variable strings without an '=' on all
   * OSes, and they can cause badness.  Let's complain about those. */
  env_vars_sorted = smartlist_new();
  smartlist_add_all(env_vars_sorted, env_vars);
  smartlist_sort_strings(env_vars_sorted);

  /* Now copy the strings into the environment blocks. */
  {
    char *cp = env->windows_environment_block;
    const char *prev_env_var = NULL;

    for (i = 0; i < n_env_vars; ++i) {
      const char *s = smartlist_get(env_vars_sorted, (int)i);
      size_t slen = strlen(s);
      size_t s_name_len = str_num_before(s, '=');

      if (s_name_len == slen) {
        log_warn(LD_GENERAL,
                 "Preparing an environment containing a variable "
                 "without a value: %s",
                 s);
      }
      if (prev_env_var != NULL &&
          environment_variable_names_equal(s, prev_env_var)) {
        log_warn(LD_GENERAL,
                 "Preparing an environment containing two variables "
                 "with the same name: %s and %s",
                 prev_env_var, s);
      }

      prev_env_var = s;

      /* Actually copy the string into the environment. */
      memcpy(cp, s, slen+1);
      env->unixoid_environment_block[i] = cp;
      cp += slen+1;
    }

    tor_assert(cp == env->windows_environment_block + total_env_length - 1);
  }

  smartlist_free(env_vars_sorted);

  return env;
}

/** Return a newly allocated smartlist containing every variable in
 * this process's environment, as a NUL-terminated string of the form
 * "NAME=VALUE".  Note that on some/many/most/all OSes, the parent
 * process can put strings not of that form in our environment;
 * callers should try to not get crashed by that.
 *
 * The returned strings are heap-allocated, and must be freed by the
 * caller. */
struct smartlist_t *
get_current_process_environment_variables(void)
{
  smartlist_t *sl = smartlist_new();

  char **environ_tmp; /* Not const char ** ? Really? */
  for (environ_tmp = get_environment(); *environ_tmp; ++environ_tmp) {
    smartlist_add_strdup(sl, *environ_tmp);
  }

  return sl;
}

/** For each string s in <b>env_vars</b> such that
 * environment_variable_names_equal(s, <b>new_var</b>), remove it; if
 * <b>free_p</b> is non-zero, call <b>free_old</b>(s).  If
 * <b>new_var</b> contains '=', insert it into <b>env_vars</b>. */
void
set_environment_variable_in_smartlist(struct smartlist_t *env_vars,
                                      const char *new_var,
                                      void (*free_old)(void*),
                                      int free_p)
{
  SMARTLIST_FOREACH_BEGIN(env_vars, const char *, s) {
    if (environment_variable_names_equal(s, new_var)) {
      SMARTLIST_DEL_CURRENT(env_vars, s);
      if (free_p) {
        free_old((void *)s);
      }
    }
  } SMARTLIST_FOREACH_END(s);

  if (strchr(new_var, '=') != NULL) {
    smartlist_add(env_vars, (void *)new_var);
  }
}
