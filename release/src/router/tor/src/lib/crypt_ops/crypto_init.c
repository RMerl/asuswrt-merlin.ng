/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_init.c
 *
 * \brief Initialize and shut down Tor's crypto library and subsystem.
 **/

#include "orconfig.h"

#define CRYPTO_PRIVATE

#include "lib/crypt_ops/crypto_init.h"

#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_openssl_mgt.h"
#include "lib/crypt_ops/crypto_nss_mgt.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_sys.h"

#include "lib/subsys/subsys.h"

#include "ext/siphash.h"

/** Boolean: has our crypto library been initialized? (early phase) */
static int crypto_early_initialized_ = 0;

/** Boolean: has our crypto library been initialized? (late phase) */
static int crypto_global_initialized_ = 0;

static int have_seeded_siphash = 0;

/** Set up the siphash key if we haven't already done so. */
int
crypto_init_siphash_key(void)
{
  struct sipkey key;
  if (have_seeded_siphash)
    return 0;

  crypto_rand((char*) &key, sizeof(key));
  siphash_set_global_key(&key);
  have_seeded_siphash = 1;
  return 0;
}

/** Initialize the crypto library.  Return 0 on success, -1 on failure.
 */
int
crypto_early_init(void)
{
  if (!crypto_early_initialized_) {

    crypto_early_initialized_ = 1;

#ifdef ENABLE_OPENSSL
    crypto_openssl_early_init();
#endif
#ifdef ENABLE_NSS
    crypto_nss_early_init(0);
#endif

    if (crypto_seed_rng() < 0)
      return -1;
    if (crypto_init_siphash_key() < 0)
      return -1;

    crypto_rand_fast_init();

    curve25519_init();
    ed25519_init();
  }
  return 0;
}

/** Initialize the crypto library.  Return 0 on success, -1 on failure.
 */
int
crypto_global_init(int useAccel, const char *accelName, const char *accelDir)
{
  if (!crypto_global_initialized_) {
    if (crypto_early_init() < 0)
      return -1;

    crypto_global_initialized_ = 1;

    crypto_dh_init();

#ifdef ENABLE_OPENSSL
    if (crypto_openssl_late_init(useAccel, accelName, accelDir) < 0)
      return -1;
#else
    (void)useAccel;
    (void)accelName;
    (void)accelDir;
#endif /* defined(ENABLE_OPENSSL) */
#ifdef ENABLE_NSS
    if (crypto_nss_late_init() < 0)
      return -1;
#endif
  }
  return 0;
}

/** Free crypto resources held by this thread. */
void
crypto_thread_cleanup(void)
{
#ifdef ENABLE_OPENSSL
  crypto_openssl_thread_cleanup();
#endif
  destroy_thread_fast_rng();
}

/**
 * Uninitialize the crypto library. Return 0 on success. Does not detect
 * failure.
 */
int
crypto_global_cleanup(void)
{
  crypto_dh_free_all();

#ifdef ENABLE_OPENSSL
  crypto_openssl_global_cleanup();
#endif
#ifdef ENABLE_NSS
  crypto_nss_global_cleanup();
#endif

  crypto_rand_fast_shutdown();

  crypto_early_initialized_ = 0;
  crypto_global_initialized_ = 0;
  have_seeded_siphash = 0;
  siphash_unset_global_key();

  return 0;
}

/** Run operations that the crypto library requires to be happy again
 * after forking. */
void
crypto_prefork(void)
{
#ifdef ENABLE_NSS
  crypto_nss_prefork();
#endif
  /* It is not safe to share a fast_rng object across a fork boundary unless
   * we actually have zero-on-fork support in map_anon.c.  If we have
   * drop-on-fork support, we will crash; if we have neither, we will yield
   * a copy of the parent process's rng, which is scary and insecure.
   */
  destroy_thread_fast_rng();
}

/** Run operations that the crypto library requires to be happy again
 * after forking. */
void
crypto_postfork(void)
{
#ifdef ENABLE_NSS
  crypto_nss_postfork();
#endif
}

/** Return the name of the crypto library we're using. */
const char *
crypto_get_library_name(void)
{
#ifdef ENABLE_OPENSSL
  return "OpenSSL";
#endif
#ifdef ENABLE_NSS
  return "NSS";
#endif
}

/** Return the version of the crypto library we are using, as given in the
 * library. */
const char *
crypto_get_library_version_string(void)
{
#ifdef ENABLE_OPENSSL
  return crypto_openssl_get_version_str();
#endif
#ifdef ENABLE_NSS
  return crypto_nss_get_version_str();
#endif
}

/** Return the version of the crypto library we're using, as given in the
 * headers. */
const char *
crypto_get_header_version_string(void)
{
#ifdef ENABLE_OPENSSL
  return crypto_openssl_get_header_version_str();
#endif
#ifdef ENABLE_NSS
  return crypto_nss_get_header_version_str();
#endif
}

/** Return true iff Tor is using the NSS library. */
int
tor_is_using_nss(void)
{
#ifdef ENABLE_NSS
  return 1;
#else
  return 0;
#endif
}

static int
subsys_crypto_initialize(void)
{
  if (crypto_early_init() < 0)
    return -1;
  crypto_dh_init();
  return 0;
}

static void
subsys_crypto_shutdown(void)
{
  crypto_global_cleanup();
}

static void
subsys_crypto_prefork(void)
{
  crypto_prefork();
}

static void
subsys_crypto_postfork(void)
{
  crypto_postfork();
}

static void
subsys_crypto_thread_cleanup(void)
{
  crypto_thread_cleanup();
}

const struct subsys_fns_t sys_crypto = {
  .name = "crypto",
  .supported = true,
  .level = -60,
  .initialize = subsys_crypto_initialize,
  .shutdown = subsys_crypto_shutdown,
  .prefork = subsys_crypto_prefork,
  .postfork = subsys_crypto_postfork,
  .thread_cleanup = subsys_crypto_thread_cleanup,
};
