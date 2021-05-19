/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#define CRYPTO_ED25519_PRIVATE
#define CONFIG_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"
#include "app/main/subsysmgr.h"
#include "lib/err/backtrace.h"
#include "app/config/config.h"
#include "test/fuzz/fuzzing.h"
#include "lib/compress/compress.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_init.h"
#include "lib/version/torversion.h"

static or_options_t *mock_options = NULL;
static const or_options_t *
mock_get_options(void)
{
  return mock_options;
}

static int
mock_crypto_pk_public_checksig__nocheck(const crypto_pk_t *env, char *to,
                                        size_t tolen,
                                        const char *from, size_t fromlen)
{
  tor_assert(env && to && from);
  (void)fromlen;
  /* We could look at from[0..fromlen-1] ... */
  tor_assert(tolen >= crypto_pk_keysize(env));
  size_t siglen = MIN(20, crypto_pk_keysize(env));
  memset(to, 0x01, siglen);
  return (int)siglen;
}

static int
mock_crypto_pk_public_checksig_digest__nocheck(crypto_pk_t *env,
                                               const char *data,
                                               size_t datalen,
                                               const char *sig,
                                               size_t siglen)
{
  tor_assert(env && data && sig);
  (void)datalen;
  (void)siglen;
  /* We could look at data[..] and sig[..] */
  return 0;
}

static int
mock_ed25519_checksig__nocheck(const ed25519_signature_t *signature,
                      const uint8_t *msg, size_t len,
                      const ed25519_public_key_t *pubkey)
{
  tor_assert(signature && msg && pubkey);
  /* We could look at msg[0..len-1] ... */
  (void)len;
  return 0;
}

static int
mock_ed25519_checksig_batch__nocheck(int *okay_out,
                                     const ed25519_checkable_t *checkable,
                                     int n_checkable)
{
  tor_assert(checkable);
  int i;
  for (i = 0; i < n_checkable; ++i) {
    /* We could look at messages and signatures XXX */
    tor_assert(checkable[i].pubkey);
    tor_assert(checkable[i].msg);
    if (okay_out)
      okay_out[i] = 1;
  }
  return 0;
}

static int
mock_ed25519_impl_spot_check__nocheck(void)
{
  return 0;
}

void
disable_signature_checking(void)
{
  MOCK(crypto_pk_public_checksig,
       mock_crypto_pk_public_checksig__nocheck);
  MOCK(crypto_pk_public_checksig_digest,
       mock_crypto_pk_public_checksig_digest__nocheck);
  MOCK(ed25519_checksig, mock_ed25519_checksig__nocheck);
  MOCK(ed25519_checksig_batch, mock_ed25519_checksig_batch__nocheck);
  MOCK(ed25519_impl_spot_check, mock_ed25519_impl_spot_check__nocheck);
}

static void
global_init(void)
{
  subsystems_init_upto(SUBSYS_LEVEL_LIBS);
  flush_log_messages_from_startup();

  tor_compress_init();

  if (crypto_global_init(0, NULL, NULL) < 0)
    abort();

  {
    struct sipkey sipkey = { 1337, 7331 };
    siphash_unset_global_key();
    siphash_set_global_key(&sipkey);
  }

  /* set up the options. */
  mock_options = options_new();
  MOCK(get_options, mock_get_options);

  /* Make BUG() and nonfatal asserts crash */
  tor_set_failed_assertion_callback(abort);

  /* Make protocol warnings handled correctly. */
  init_protocol_warning_severity_level();
}

#ifdef LLVM_FUZZ
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
  static int initialized = 0;
  if (!initialized) {
    global_init();
    if (fuzz_init() < 0)
      abort();
    initialized = 1;
  }

  return fuzz_main(Data, Size);
}

#else /* !defined(LLVM_FUZZ) */

int
main(int argc, char **argv)
{
  size_t size;

  global_init();

  /* Disable logging by default to speed up fuzzing. */
  int loglevel = LOG_ERR;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--warn")) {
      loglevel = LOG_WARN;
    } else if (!strcmp(argv[i], "--notice")) {
      loglevel = LOG_NOTICE;
    } else if (!strcmp(argv[i], "--info")) {
      loglevel = LOG_INFO;
    } else if (!strcmp(argv[i], "--debug")) {
      loglevel = LOG_DEBUG;
    }
  }

  {
    log_severity_list_t s;
    memset(&s, 0, sizeof(s));
    set_log_severity_config(loglevel, LOG_ERR, &s);
    /* ALWAYS log bug warnings. */
    s.masks[SEVERITY_MASK_IDX(LOG_WARN)] |= LD_BUG;
    add_stream_log(&s, "", fileno(stdout));
  }

  if (fuzz_init() < 0)
    abort();

#ifdef __AFL_HAVE_MANUAL_CONTROL
  /* Tell AFL to pause and fork here - ignored if not using AFL */
  __AFL_INIT();
#endif

#define MAX_FUZZ_SIZE (128*1024)
  char *input = read_file_to_str_until_eof(0, MAX_FUZZ_SIZE, &size);
  tor_assert(input);
  char *raw = tor_memdup(input, size); /* Because input is nul-terminated */
  tor_free(input);
  fuzz_main((const uint8_t*)raw, size);
  tor_free(raw);

  if (fuzz_cleanup() < 0)
    abort();

  or_options_free(mock_options);
  UNMOCK(get_options);
  return 0;
}

#endif /* defined(LLVM_FUZZ) */
