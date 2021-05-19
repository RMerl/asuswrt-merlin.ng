/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_common.c
 * \brief Common pieces to implement unit tests.
 **/

#define MAINLOOP_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"
#include "feature/control/control.h"
#include "feature/control/control_events.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/stats/predict_ports.h"
#include "feature/stats/bwhist.h"
#include "feature/stats/rephist.h"
#include "lib/err/backtrace.h"
#include "test/test.h"
#include "core/or/channelpadding.h"
#include "core/mainloop/mainloop.h"
#include "lib/compress/compress.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/crypt_ops/crypto_init.h"
#include "lib/version/torversion.h"
#include "app/main/subsysmgr.h"

#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

/** Temporary directory (set up by setup_directory) under which we store all
 * our files during testing. */
static char temp_dir[256];
#ifdef _WIN32
#define pid_t int
#endif
static pid_t temp_dir_setup_in_pid = 0;

/** Select and create the temporary directory we'll use to run our unit tests.
 * Store it in <b>temp_dir</b>.  Exit immediately if we can't create it.
 * idempotent. */
static void
setup_directory(void)
{
  static int is_setup = 0;
  int r;
  char rnd[256], rnd32[256];
  if (is_setup) return;

/* Due to base32 limitation needs to be a multiple of 5. */
#define RAND_PATH_BYTES 5
  crypto_rand(rnd, RAND_PATH_BYTES);
  base32_encode(rnd32, sizeof(rnd32), rnd, RAND_PATH_BYTES);

#ifdef _WIN32
  {
    char buf[MAX_PATH];
    const char *tmp = buf;
    const char *extra_backslash = "";
    /* If this fails, we're probably screwed anyway */
    if (!GetTempPathA(sizeof(buf),buf))
      tmp = "c:\\windows\\temp\\";
    if (strcmpend(tmp, "\\")) {
      /* According to MSDN, it should be impossible for GetTempPath to give us
       * an answer that doesn't end with \.  But let's make sure. */
      extra_backslash = "\\";
    }
    tor_snprintf(temp_dir, sizeof(temp_dir),
                 "%s%stor_test_%d_%s", tmp, extra_backslash,
                 (int)getpid(), rnd32);
    r = mkdir(temp_dir);
  }
#elif defined(__ANDROID__)
  /* tor might not like the default perms, so create a subdir */
  tor_snprintf(temp_dir, sizeof(temp_dir),
               "/data/local/tmp/tor_%d_%d_%s",
               (int) getuid(), (int) getpid(), rnd32);
  r = mkdir(temp_dir, 0700);
  if (r) {
    fprintf(stderr, "Can't create directory %s:", temp_dir);
    perror("");
    exit(1);
  }
#else /* !defined(_WIN32) */
  tor_snprintf(temp_dir, sizeof(temp_dir), "/tmp/tor_test_%d_%s",
               (int) getpid(), rnd32);
  r = mkdir(temp_dir, 0700);
  if (!r) {
    /* undo sticky bit so tests don't get confused. */
    r = chown(temp_dir, getuid(), getgid());
  }
#endif /* defined(_WIN32) || ... */
  if (r) {
    fprintf(stderr, "Can't create directory %s:", temp_dir);
    perror("");
    exit(1);
  }
  is_setup = 1;
  temp_dir_setup_in_pid = getpid();
}

/** Return a filename relative to our testing temporary directory, based on
 * name and suffix. If name is NULL, return the name of the testing temporary
 * directory. */
static const char *
get_fname_suffix(const char *name, const char *suffix)
{
  static char buf[1024];
  setup_directory();
  if (!name)
    return temp_dir;
  tor_snprintf(buf,sizeof(buf),"%s%s%s%s%s", temp_dir, PATH_SEPARATOR, name,
               suffix ? "_" : "", suffix ? suffix : "");
  return buf;
}

/** Return a filename relative to our testing temporary directory. If name is
 * NULL, return the name of the testing temporary directory. */
const char *
get_fname(const char *name)
{
  return get_fname_suffix(name, NULL);
}

/** Return a filename with a random suffix, relative to our testing temporary
 * directory. If name is NULL, return the name of the testing temporary
 * directory, without any suffix. */
const char *
get_fname_rnd(const char *name)
{
  char rnd[256], rnd32[256];
  crypto_rand(rnd, RAND_PATH_BYTES);
  base32_encode(rnd32, sizeof(rnd32), rnd, RAND_PATH_BYTES);
  return get_fname_suffix(name, rnd32);
}

/* Remove a directory and all of its subdirectories */
static void
rm_rf(const char *dir)
{
  struct stat st;
  smartlist_t *elements;

  elements = tor_listdir(dir);
  if (elements) {
    SMARTLIST_FOREACH_BEGIN(elements, const char *, cp) {
         char *tmp = NULL;
         tor_asprintf(&tmp, "%s"PATH_SEPARATOR"%s", dir, cp);
         if (0 == stat(tmp,&st) && (st.st_mode & S_IFDIR)) {
           rm_rf(tmp);
         } else {
           if (unlink(tmp)) {
             fprintf(stderr, "Error removing %s: %s\n", tmp, strerror(errno));
           }
         }
         tor_free(tmp);
    } SMARTLIST_FOREACH_END(cp);
    SMARTLIST_FOREACH(elements, char *, cp, tor_free(cp));
    smartlist_free(elements);
  }
  if (rmdir(dir))
    fprintf(stderr, "Error removing directory %s: %s\n", dir, strerror(errno));
}

/** Remove all files stored under the temporary directory, and the directory
 * itself.  Called by atexit(). */
static void
remove_directory(void)
{
  if (getpid() != temp_dir_setup_in_pid) {
    /* Only clean out the tempdir when the main process is exiting. */
    return;
  }

  rm_rf(temp_dir);
}

static void *
passthrough_test_setup(const struct testcase_t *testcase)
{
  /* Make sure the passthrough doesn't unintentionally fail or skip tests */
  tor_assert(testcase->setup_data);
  tor_assert(testcase->setup_data != (void*)TT_SKIP);
  return testcase->setup_data;
}
static int
passthrough_test_cleanup(const struct testcase_t *testcase, void *ptr)
{
  (void)testcase;
  (void)ptr;
  return 1;
}

static void *
ed25519_testcase_setup(const struct testcase_t *testcase)
{
  crypto_ed25519_testing_force_impl(testcase->setup_data);
  return testcase->setup_data;
}
static int
ed25519_testcase_cleanup(const struct testcase_t *testcase, void *ptr)
{
  (void)testcase;
  (void)ptr;
  crypto_ed25519_testing_restore_impl();
  return 1;
}
const struct testcase_setup_t ed25519_test_setup = {
  ed25519_testcase_setup, ed25519_testcase_cleanup
};

const struct testcase_setup_t passthrough_setup = {
  passthrough_test_setup, passthrough_test_cleanup
};

static void
an_assertion_failed(void)
{
  tinytest_set_test_failed_();
}

void tinytest_prefork(void);
void tinytest_postfork(void);
void
tinytest_prefork(void)
{
  free_pregenerated_keys();
  subsystems_prefork();
}
void
tinytest_postfork(void)
{
  subsystems_postfork();
  init_pregenerated_keys();
}

static void
log_callback_failure(int severity, log_domain_mask_t domain, const char *msg)
{
  (void)msg;
  if (severity == LOG_ERR || (domain & LD_BUG)) {
    tinytest_set_test_failed_();
  }
}

/** Main entry point for unit test code: parse the command line, and run
 * some unit tests. */
int
main(int c, const char **v)
{
  or_options_t *options;
  char *errmsg = NULL;
  int i, i_out;
  int loglevel = LOG_ERR;
  int accel_crypto = 0;

  subsystems_init();

  options = options_new();

  struct tor_libevent_cfg_t cfg;
  memset(&cfg, 0, sizeof(cfg));
  tor_libevent_initialize(&cfg);

  control_initialize_event_queue();

  /* Don't add default logs; the tests manage their own. */
  quiet_level = QUIET_SILENT;

  unsigned num=1, den=1;

  for (i_out = i = 1; i < c; ++i) {
    if (!strcmp(v[i], "--warn")) {
      loglevel = LOG_WARN;
    } else if (!strcmp(v[i], "--notice")) {
      loglevel = LOG_NOTICE;
    } else if (!strcmp(v[i], "--info")) {
      loglevel = LOG_INFO;
    } else if (!strcmp(v[i], "--debug")) {
      loglevel = LOG_DEBUG;
    } else if (!strcmp(v[i], "--accel")) {
      accel_crypto = 1;
    } else if (!strcmp(v[i], "--fraction")) {
      if (i+1 == c) {
        printf("--fraction needs an argument.\n");
        return 1;
      }
      const char *fracstr = v[++i];
      char ch;
      if (sscanf(fracstr, "%u/%u%c", &num, &den, &ch) != 2) {
        printf("--fraction expects a fraction as an input.\n");
      }
      if (den == 0 || num == 0 || num > den) {
        printf("--fraction expects a valid fraction as an input.\n");
      }
    } else {
      v[i_out++] = v[i];
    }
  }
  c = i_out;

  {
    /* setup logs to stdout */
    log_severity_list_t s;
    memset(&s, 0, sizeof(s));
    set_log_severity_config(loglevel, LOG_ERR, &s);
    /* ALWAYS log bug warnings. */
    s.masks[SEVERITY_MASK_IDX(LOG_WARN)] |= LD_BUG;
    add_stream_log(&s, "", fileno(stdout));
  }
  {
    /* Setup logs that cause failure. */
    log_severity_list_t s;
    memset(&s, 0, sizeof(s));
    set_log_severity_config(LOG_ERR, LOG_ERR, &s);
    s.masks[SEVERITY_MASK_IDX(LOG_WARN)] |= LD_BUG;
    add_callback_log(&s, log_callback_failure);
  }
  flush_log_messages_from_startup();
  init_protocol_warning_severity_level();

  options->command = CMD_RUN_UNITTESTS;
  if (crypto_global_init(accel_crypto, NULL, NULL)) {
    printf("Can't initialize crypto subsystem; exiting.\n");
    return 1;
  }
  if (crypto_seed_rng() < 0) {
    printf("Couldn't seed RNG; exiting.\n");
    return 1;
  }
  rep_hist_init();
  bwhist_init();
  setup_directory();
  initialize_mainloop_events();
  options_init(options);
  options->DataDirectory = tor_strdup(temp_dir);
  options->DataDirectory_option = tor_strdup(temp_dir);
  tor_asprintf(&options->KeyDirectory, "%s"PATH_SEPARATOR"keys",
               options->DataDirectory);
  options->CacheDirectory = tor_strdup(temp_dir);
  options->EntryStatistics = 1;
  if (set_options(options, &errmsg) < 0) {
    printf("Failed to set initial options: %s\n", errmsg);
    tor_free(errmsg);
    return 1;
  }

  tor_set_failed_assertion_callback(an_assertion_failed);

  init_pregenerated_keys();

  channelpadding_new_consensus_params(NULL);

  predicted_ports_init();

  atexit(remove_directory);

  /* Look for TOR_SKIP_TESTCASES: a space-separated list of tests to skip. */
  const char *skip_tests = getenv("TOR_SKIP_TESTCASES");
  if (skip_tests) {
    smartlist_t *skip = smartlist_new();
    smartlist_split_string(skip, skip_tests, NULL,
                           SPLIT_IGNORE_BLANK, -1);
    int n = 0;
    SMARTLIST_FOREACH_BEGIN(skip, char *, cp) {
      n += tinytest_skip(testgroups, cp);
      tor_free(cp);
    } SMARTLIST_FOREACH_END(cp);
    printf("Skipping %d testcases.\n", n);
    smartlist_free(skip);
  }

  if (den != 1) {
    // count the tests. Linear but fast.
    unsigned n_tests = 0;
    struct testgroup_t *tg;
    struct testcase_t *tc;
    for (tg = testgroups; tg->prefix != NULL; ++tg) {
      for (tc = tg->cases; tc->name != NULL; ++tc) {
        ++n_tests;
      }
    }
    // Which tests should we run?  This can give iffy results if den is huge
    // but it doesn't actually matter in practice.
    unsigned tests_per_chunk = CEIL_DIV(n_tests, den);
    unsigned start_at = (num-1) * tests_per_chunk;

    // Skip the tests that are outside of the range.
    unsigned idx = 0;
    for (tg = testgroups; tg->prefix != NULL; ++tg) {
      for (tc = tg->cases; tc->name != NULL; ++tc) {
        if (idx < start_at || idx >= start_at + tests_per_chunk) {
          tc->flags |= TT_SKIP;
        }
        ++idx;
      }
    }
  }

  int have_failed = (tinytest_main(c, v, testgroups) != 0);

  free_pregenerated_keys();

  if (have_failed)
    return 1;
  else
    return 0;
}
