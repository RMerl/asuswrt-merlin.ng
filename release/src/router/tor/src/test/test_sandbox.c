/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef _LARGEFILE64_SOURCE
/**
 * Temporarily required for O_LARGEFILE flag. Needs to be removed
 * with the libevent fix.
 */
#define _LARGEFILE64_SOURCE
#endif /* !defined(_LARGEFILE64_SOURCE) */

#include "orconfig.h"

#include "lib/sandbox/sandbox.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "ext/equix/include/equix.h"

#ifdef USE_LIBSECCOMP

#include <dirent.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "core/or/or.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

typedef struct {
  sandbox_cfg_t *cfg;

  char *file_ops_allowed;
  char *file_ops_blocked;

  char *file_rename_target_allowed;

  char *dir_ops_allowed;
  char *dir_ops_blocked;
} sandbox_data_t;

/* All tests are skipped when coverage support is enabled (see further below)
 * as the sandbox interferes with the use of gcov.  Prevent a compiler warning
 * by omitting these definitions in that case. */
#ifndef ENABLE_COVERAGE
static void *
setup_sandbox(const struct testcase_t *testcase)
{
  sandbox_data_t *data = tor_malloc_zero(sizeof(*data));

  (void)testcase;

  /* Establish common file and directory names within the test suite's
   * temporary directory. */
  data->file_ops_allowed = tor_strdup(get_fname("file_ops_allowed"));
  data->file_ops_blocked = tor_strdup(get_fname("file_ops_blocked"));

  data->file_rename_target_allowed =
    tor_strdup(get_fname("file_rename_target_allowed"));

  data->dir_ops_allowed = tor_strdup(get_fname("dir_ops_allowed"));
  data->dir_ops_blocked = tor_strdup(get_fname("dir_ops_blocked"));

  /* Create the corresponding filesystem objects. */
  creat(data->file_ops_allowed, S_IRWXU);
  creat(data->file_ops_blocked, S_IRWXU);
  mkdir(data->dir_ops_allowed, S_IRWXU);
  mkdir(data->dir_ops_blocked, S_IRWXU);

  /* Create the sandbox configuration. */
  data->cfg = sandbox_cfg_new();

  sandbox_cfg_allow_open_filename(&data->cfg,
                                  tor_strdup(data->file_ops_allowed));
  sandbox_cfg_allow_open_filename(&data->cfg,
                                  tor_strdup(data->dir_ops_allowed));

  sandbox_cfg_allow_chmod_filename(&data->cfg,
                                   tor_strdup(data->file_ops_allowed));
  sandbox_cfg_allow_chmod_filename(&data->cfg,
                                   tor_strdup(data->dir_ops_allowed));
  sandbox_cfg_allow_chown_filename(&data->cfg,
                                   tor_strdup(data->file_ops_allowed));
  sandbox_cfg_allow_chown_filename(&data->cfg,
                                   tor_strdup(data->dir_ops_allowed));

  sandbox_cfg_allow_rename(&data->cfg, tor_strdup(data->file_ops_allowed),
                           tor_strdup(data->file_rename_target_allowed));

  sandbox_cfg_allow_openat_filename(&data->cfg,
                                    tor_strdup(data->dir_ops_allowed));

  sandbox_cfg_allow_opendir_dirname(&data->cfg,
                                    tor_strdup(data->dir_ops_allowed));

  sandbox_cfg_allow_stat_filename(&data->cfg,
                                  tor_strdup(data->file_ops_allowed));
  sandbox_cfg_allow_stat_filename(&data->cfg,
                                  tor_strdup(data->dir_ops_allowed));

  /* Activate the sandbox, which will remain in effect until the process
   * terminates. */
  sandbox_init(data->cfg);

  return data;
}

static int
cleanup_sandbox(const struct testcase_t *testcase, void *data_)
{
  sandbox_data_t *data = data_;

  (void)testcase;

  tor_free(data->dir_ops_blocked);
  tor_free(data->dir_ops_allowed);
  tor_free(data->file_rename_target_allowed);
  tor_free(data->file_ops_blocked);
  tor_free(data->file_ops_allowed);

  tor_free(data);

  return 1;
}

static const struct testcase_setup_t sandboxed_testcase_setup = {
  .setup_fn = setup_sandbox,
  .cleanup_fn = cleanup_sandbox
};
#endif /* !defined(ENABLE_COVERAGE) */

static void
test_sandbox_is_active(void *ignored)
{
  (void)ignored;

  tt_assert(!sandbox_is_active());

  sandbox_init(sandbox_cfg_new());
  tt_assert(sandbox_is_active());

 done:
  (void)0;
}

static void
test_sandbox_open_filename(void *arg)
{
  sandbox_data_t *data = arg;
  int fd, errsv;

  fd = open(sandbox_intern_string(data->file_ops_allowed), O_RDONLY);
  if (fd == -1)
    tt_abort_perror("open");
  close(fd);

  /* It might be nice to use sandbox_intern_string() in the line below as well
   * (and likewise in the test cases that follow) but this would require
   * capturing the warning message it logs, and the mechanism for doing so
   * relies on system calls that are normally blocked by the sandbox and may
   * vary across architectures. */
  fd = open(data->file_ops_blocked, O_RDONLY);
  errsv = errno;
  tt_int_op(fd, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  if (fd >= 0)
    close(fd);
}

static void
test_sandbox_chmod_filename(void *arg)
{
  sandbox_data_t *data = arg;
  int rc, errsv;

  if (chmod(sandbox_intern_string(data->file_ops_allowed),
            S_IRUSR | S_IWUSR) != 0)
    tt_abort_perror("chmod");

  rc = chmod(data->file_ops_blocked, S_IRUSR | S_IWUSR);
  errsv = errno;
  tt_int_op(rc, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  (void)0;
}

static void
test_sandbox_chown_filename(void *arg)
{
  sandbox_data_t *data = arg;
  int rc, errsv;

  if (chown(sandbox_intern_string(data->file_ops_allowed), -1, -1) != 0)
    tt_abort_perror("chown");

  rc = chown(data->file_ops_blocked, -1, -1);
  errsv = errno;
  tt_int_op(rc, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  (void)0;
}

static void
test_sandbox_rename_filename(void *arg)
{
  sandbox_data_t *data = arg;
  const char *fname_old = sandbox_intern_string(data->file_ops_allowed),
    *fname_new = sandbox_intern_string(data->file_rename_target_allowed);
  int rc, errsv;

  if (rename(fname_old, fname_new) != 0)
    tt_abort_perror("rename");

  rc = rename(fname_new, fname_old);
  errsv = errno;
  tt_int_op(rc, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  (void)0;
}

static void
test_sandbox_openat_filename(void *arg)
{
  sandbox_data_t *data = arg;
  int flags = O_RDONLY | O_NONBLOCK | O_LARGEFILE | O_DIRECTORY | O_CLOEXEC;
  int fd, errsv;

  fd = openat(AT_FDCWD, sandbox_intern_string(data->dir_ops_allowed), flags);
  if (fd < 0)
    tt_abort_perror("openat");
  close(fd);

  fd = openat(AT_FDCWD, data->dir_ops_blocked, flags);
  errsv = errno;
  tt_int_op(fd, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  if (fd >= 0)
    close(fd);
}

static void
test_sandbox_opendir_dirname(void *arg)
{
  sandbox_data_t *data = arg;
  DIR *dir;
  int errsv;

  dir = opendir(sandbox_intern_string(data->dir_ops_allowed));
  if (dir == NULL)
    tt_abort_perror("opendir");
  closedir(dir);

  dir = opendir(data->dir_ops_blocked);
  errsv = errno;
  tt_ptr_op(dir, OP_EQ, NULL);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  if (dir)
    closedir(dir);
}

static void
test_sandbox_stat_filename(void *arg)
{
  sandbox_data_t *data = arg;
  struct stat st;

  if (stat(sandbox_intern_string(data->file_ops_allowed), &st) != 0)
    tt_abort_perror("stat");

  int rc = stat(data->file_ops_blocked, &st);
  int errsv = errno;
  tt_int_op(rc, OP_EQ, -1);
  tt_int_op(errsv, OP_EQ, EPERM);

 done:
  (void)0;
}

/** This is a simplified subset of test_crypto_equix(), running one solve
 * and one verify from inside the sandbox. The sandbox restricts mprotect, and
 * hashx will experience a failure at runtime which this test case exercises.
 * The result of the solve and verify should both still be correct, since we
 * expect it to cleanly fall back on an interpreted implementation which has
 * no operating system dependencies. */
static void
test_sandbox_crypto_equix(void *arg)
{
  (void)arg;

  const char *challenge_literal = "abce";
  const size_t challenge_len = strlen(challenge_literal);
  const size_t num_sols = 4;
  static const equix_solution sols_expected[EQUIX_MAX_SOLS] = {
    {{ 0x4fca, 0x72eb, 0x101f, 0xafab, 0x1add, 0x2d71, 0x75a3, 0xc978 }},
    {{ 0x17f1, 0x7aa6, 0x23e3, 0xab00, 0x7e2f, 0x917e, 0x16da, 0xda9e }},
    {{ 0x70ee, 0x7757, 0x8a54, 0xbd2b, 0x90e4, 0xe31e, 0x2085, 0xe47e }},
    {{ 0x62c5, 0x86d1, 0x5752, 0xe1f0, 0x12da, 0x8f33, 0x7336, 0xf161 }},
  };

  equix_solutions_buffer output;
  equix_ctx *solve_ctx = NULL, *verify_ctx = NULL;

  solve_ctx = equix_alloc(EQUIX_CTX_SOLVE | EQUIX_CTX_TRY_COMPILE);
  tt_ptr_op(solve_ctx, OP_NE, NULL);

  equix_result result;
  memset(&output, 0xEE, sizeof output);
  result = equix_solve(solve_ctx, challenge_literal, challenge_len, &output);
  tt_int_op(result, OP_EQ, EQUIX_OK);
  tt_int_op(output.count, OP_EQ, num_sols);
  tt_int_op(output.flags, OP_EQ, 0); /* EQUIX_SOLVER_DID_USE_COMPILER unset */
  tt_mem_op(output.sols, OP_EQ, sols_expected,
            num_sols * sizeof(equix_solution));

  verify_ctx = equix_alloc(EQUIX_CTX_VERIFY | EQUIX_CTX_TRY_COMPILE);
  tt_ptr_op(verify_ctx, OP_NE, NULL);

  /* Test one of the solutions randomly */
  const unsigned sol_i = crypto_rand_int(num_sols);
  equix_solution *sol = &output.sols[sol_i];

  result = equix_verify(verify_ctx, challenge_literal,
                        challenge_len, sol);
  tt_int_op(EQUIX_OK, OP_EQ, result);

 done:
  equix_free(solve_ctx);
  equix_free(verify_ctx);
}

#define SANDBOX_TEST_SKIPPED(name) \
  { #name, test_sandbox_ ## name, TT_SKIP, NULL, NULL }

/* Skip all tests when coverage support is enabled, as the sandbox interferes
 * with gcov and prevents it from producing any results. */
#ifdef ENABLE_COVERAGE
#define SANDBOX_TEST(name, flags) SANDBOX_TEST_SKIPPED(name)
#define SANDBOX_TEST_IN_SANDBOX(name) SANDBOX_TEST_SKIPPED(name)
#else
#define SANDBOX_TEST(name, flags) \
  { #name, test_sandbox_ ## name, flags, NULL, NULL }
#define SANDBOX_TEST_IN_SANDBOX(name) \
  { #name, test_sandbox_ ## name, TT_FORK, &sandboxed_testcase_setup, NULL }
#endif /* defined(ENABLE_COVERAGE) */

struct testcase_t sandbox_tests[] = {
  SANDBOX_TEST(is_active, TT_FORK),

/* When Tor is built with fragile compiler-hardening the sandbox is unable to
 * filter requests to open files or directories (on systems where glibc uses
 * the "open" system call to provide this functionality), as doing so would
 * interfere with the address sanitizer as it retrieves information about the
 * running process via the filesystem.  Skip these tests in that case as the
 * corresponding functions are likely to have no effect and this will cause the
 * tests to fail. */
#ifdef ENABLE_FRAGILE_HARDENING
  SANDBOX_TEST_SKIPPED(open_filename),
  SANDBOX_TEST_SKIPPED(opendir_dirname),
#else
  SANDBOX_TEST_IN_SANDBOX(open_filename),
  SANDBOX_TEST_IN_SANDBOX(opendir_dirname),
#endif /* defined(ENABLE_FRAGILE_HARDENING) */

  SANDBOX_TEST_IN_SANDBOX(openat_filename),
  SANDBOX_TEST_IN_SANDBOX(chmod_filename),
  SANDBOX_TEST_IN_SANDBOX(chown_filename),
  SANDBOX_TEST_IN_SANDBOX(rename_filename),

/* Currently the sandbox is unable to filter stat() calls on systems where
 * glibc implements this function using either of the legacy "stat" or "stat64"
 * system calls, or (in glibc version 2.33 and later) either of the newer
 * "newfstatat" or "statx" syscalls.
 *
 * Skip testing sandbox_cfg_allow_stat_filename() if it seems the likely the
 * function will have no effect and the test will therefore not succeed. */
#if !defined(__NR_stat) && !defined(__NR_stat64) && !defined(__NR_newfstatat) \
  && !(defined(__i386__) && defined(__NR_statx))
  SANDBOX_TEST_IN_SANDBOX(stat_filename),
#else
  SANDBOX_TEST_SKIPPED(stat_filename),
#endif

  SANDBOX_TEST_IN_SANDBOX(crypto_equix),
  END_OF_TESTCASES
};

#endif /* defined(USE_SECCOMP) */
