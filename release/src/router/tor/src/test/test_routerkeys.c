/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#define ROUTER_PRIVATE
#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_format.h"
#include "feature/keymgt/loadkey.h"
#include "feature/nodelist/torcert.h"
#include "test/test.h"

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

static void
test_routerkeys_write_fingerprint(void *arg)
{
  crypto_pk_t *key = pk_generate(2);
  or_options_t *options = get_options_mutable();
  const char *ddir = get_fname("write_fingerprint");
  char *cp = NULL, *cp2 = NULL;
  char fp[FINGERPRINT_LEN+1];

  (void)arg;

  tt_assert(key);

  options->ORPort_set = 1; /* So that we can get the server ID key */
  tor_free(options->DataDirectory);
  options->DataDirectory = tor_strdup(ddir);
  options->Nickname = tor_strdup("haflinger");
  set_server_identity_key(key);
  set_client_identity_key(crypto_pk_dup_key(key));

  tt_int_op(0, OP_EQ, check_private_dir(ddir, CPD_CREATE, NULL));
  tt_int_op(crypto_pk_cmp_keys(get_server_identity_key(),key),OP_EQ,0);

  /* Write fingerprint file */
  tt_int_op(0, OP_EQ, router_write_fingerprint(0, 0));
  cp = read_file_to_str(get_fname("write_fingerprint/fingerprint"),
                        0, NULL);
  crypto_pk_get_fingerprint(key, fp, 0);
  tor_asprintf(&cp2, "haflinger %s\n", fp);
  tt_str_op(cp, OP_EQ, cp2);
  tor_free(cp);
  tor_free(cp2);

  /* Write hashed-fingerprint file */
  tt_int_op(0, OP_EQ, router_write_fingerprint(1, 0));
  cp = read_file_to_str(get_fname("write_fingerprint/hashed-fingerprint"),
                        0, NULL);
  crypto_pk_get_hashed_fingerprint(key, fp);
  tor_asprintf(&cp2, "haflinger %s\n", fp);
  tt_str_op(cp, OP_EQ, cp2);
  tor_free(cp);
  tor_free(cp2);

  /* Replace outdated file */
  write_str_to_file(get_fname("write_fingerprint/hashed-fingerprint"),
                    "junk goes here", 0);
  tt_int_op(0, OP_EQ, router_write_fingerprint(1, 0));
  cp = read_file_to_str(get_fname("write_fingerprint/hashed-fingerprint"),
                        0, NULL);
  crypto_pk_get_hashed_fingerprint(key, fp);
  tor_asprintf(&cp2, "haflinger %s\n", fp);
  tt_str_op(cp, OP_EQ, cp2);
  tor_free(cp);
  tor_free(cp2);

 done:
  crypto_pk_free(key);
  set_client_identity_key(NULL);
  tor_free(cp);
  tor_free(cp2);
}

static void
test_routerkeys_write_ed25519_identity(void *arg)
{
  crypto_pk_t *key = pk_generate(2);
  or_options_t *options = get_options_mutable();
  time_t now = time(NULL);
  const char *ddir = get_fname("write_fingerprint");
  char *cp = NULL, *cp2 = NULL;
  char ed25519_id[BASE64_DIGEST256_LEN + 1];

  (void) arg;

  tt_assert(key);

  options->ORPort_set = 1; /* So that we can get the server ID key */
  tor_free(options->DataDirectory);
  options->DataDirectory = tor_strdup(ddir);
  options->Nickname = tor_strdup("haflinger");
  set_server_identity_key(key);
  set_client_identity_key(crypto_pk_dup_key(key));

  load_ed_keys(options, now);
  tt_assert(get_master_identity_key());

  tt_int_op(0, OP_EQ, check_private_dir(ddir, CPD_CREATE, NULL));

  /* Write fingerprint file */
  tt_int_op(0, OP_EQ, router_write_fingerprint(0, 1));
  cp = read_file_to_str(get_fname("write_fingerprint/fingerprint-ed25519"),
                        0, NULL);
  digest256_to_base64(ed25519_id,
                      (const char *) get_master_identity_key()->pubkey);
  tor_asprintf(&cp2, "haflinger %s\n", ed25519_id);
  tt_str_op(cp, OP_EQ, cp2);
  tor_free(cp);
  tor_free(cp2);

 done:
  crypto_pk_free(key);
  set_client_identity_key(NULL);
  tor_free(cp);
  tor_free(cp2);
  routerkeys_free_all();
}

static void
test_routerkeys_ed_certs(void *args)
{
  (void)args;
  ed25519_keypair_t kp1, kp2;
  tor_cert_t *cert[2] = {NULL, NULL}, *nocert = NULL;
  tor_cert_t *parsed_cert[2] = {NULL, NULL};
  time_t now = 1412094534;
  uint8_t *junk = NULL;
  char *base64 = NULL;

  tt_int_op(0,OP_EQ,ed25519_keypair_generate(&kp1, 0));
  tt_int_op(0,OP_EQ,ed25519_keypair_generate(&kp2, 0));

  for (int i = 0; i <= 1; ++i) {
    uint32_t flags = i ? CERT_FLAG_INCLUDE_SIGNING_KEY : 0;

    cert[i] = tor_cert_create_ed25519(&kp1, 5, &kp2.pubkey, now, 10000, flags);
    tt_assert(cert[i]);

    tt_uint_op(cert[i]->sig_bad, OP_EQ, 0);
    tt_uint_op(cert[i]->sig_ok, OP_EQ, 1);
    tt_uint_op(cert[i]->cert_expired, OP_EQ, 0);
    tt_uint_op(cert[i]->cert_valid, OP_EQ, 1);
    tt_int_op(cert[i]->cert_type, OP_EQ, 5);
    tt_mem_op(cert[i]->signed_key.pubkey, OP_EQ, &kp2.pubkey.pubkey, 32);
    tt_mem_op(cert[i]->signing_key.pubkey, OP_EQ, &kp1.pubkey.pubkey, 32);
    tt_int_op(cert[i]->signing_key_included, OP_EQ, i);

    tt_assert(cert[i]->encoded);
    tt_int_op(cert[i]->encoded_len, OP_EQ, 104 + 36 * i);
    tt_int_op(cert[i]->encoded[0], OP_EQ, 1);
    tt_int_op(cert[i]->encoded[1], OP_EQ, 5);

    parsed_cert[i] = tor_cert_parse(cert[i]->encoded, cert[i]->encoded_len);
    tt_assert(parsed_cert[i]);
    tt_int_op(cert[i]->encoded_len, OP_EQ, parsed_cert[i]->encoded_len);
    tt_mem_op(cert[i]->encoded, OP_EQ, parsed_cert[i]->encoded,
              cert[i]->encoded_len);
    tt_uint_op(parsed_cert[i]->sig_bad, OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->sig_ok, OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->cert_expired, OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->cert_valid, OP_EQ, 0);

    /* Expired */
    tt_int_op(tor_cert_checksig(parsed_cert[i], &kp1.pubkey, now + 30000),
              OP_LT, 0);
    tt_uint_op(parsed_cert[i]->cert_expired, OP_EQ, 1);
    parsed_cert[i]->cert_expired = 0;

    /* Wrong key */
    tt_int_op(tor_cert_checksig(parsed_cert[i], &kp2.pubkey, now), OP_LT, 0);
    tt_uint_op(parsed_cert[i]->sig_bad, OP_EQ, 1);
    parsed_cert[i]->sig_bad = 0;

    /* Missing key */
    int ok = tor_cert_checksig(parsed_cert[i], NULL, now);
    tt_int_op(ok < 0, OP_EQ, i == 0);
    tt_uint_op(parsed_cert[i]->sig_bad, OP_EQ, 0);
    tt_assert(parsed_cert[i]->sig_ok == (i != 0));
    tt_assert(parsed_cert[i]->cert_valid == (i != 0));
    parsed_cert[i]->sig_bad = 0;
    parsed_cert[i]->sig_ok = 0;
    parsed_cert[i]->cert_valid = 0;

    /* Right key */
    tt_int_op(tor_cert_checksig(parsed_cert[i], &kp1.pubkey, now), OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->sig_bad, OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->sig_ok, OP_EQ, 1);
    tt_uint_op(parsed_cert[i]->cert_expired, OP_EQ, 0);
    tt_uint_op(parsed_cert[i]->cert_valid, OP_EQ, 1);
  }

  /* Now try some junky certs. */
  /* - Truncated */
  nocert = tor_cert_parse(cert[0]->encoded, cert[0]->encoded_len-1);
  tt_ptr_op(NULL, OP_EQ, nocert);

  /* - First byte modified */
  cert[0]->encoded[0] = 99;
  nocert = tor_cert_parse(cert[0]->encoded, cert[0]->encoded_len);
  tt_ptr_op(NULL, OP_EQ, nocert);
  cert[0]->encoded[0] = 1;

  /* - Extra byte at the end*/
  junk = tor_malloc_zero(cert[0]->encoded_len + 1);
  memcpy(junk, cert[0]->encoded, cert[0]->encoded_len);
  nocert = tor_cert_parse(junk, cert[0]->encoded_len+1);
  tt_ptr_op(NULL, OP_EQ, nocert);

  /* - Multiple signing key instances */
  tor_free(junk);
  junk = tor_malloc_zero(104 + 36 * 2);
  junk[0] = 1; /* version */
  junk[1] = 5; /* cert type */
  junk[6] = 1; /* key type */
  junk[39] = 2; /* n_extensions */
  junk[41] = 32; /* extlen */
  junk[42] = 4; /* exttype */
  junk[77] = 32; /* extlen */
  junk[78] = 4; /* exttype */
  nocert = tor_cert_parse(junk, 104 + 36 * 2);
  tt_ptr_op(NULL, OP_EQ, nocert);

 done:
  tor_cert_free(cert[0]);
  tor_cert_free(cert[1]);
  tor_cert_free(parsed_cert[0]);
  tor_cert_free(parsed_cert[1]);
  tor_cert_free(nocert);
  tor_free(junk);
  tor_free(base64);
}

static void
test_routerkeys_ed_key_create(void *arg)
{
  (void)arg;
  tor_cert_t *cert = NULL;
  ed25519_keypair_t *kp1 = NULL, *kp2 = NULL;
  time_t now = time(NULL);

  /* This is a simple alias for 'make a new keypair' */
  kp1 = ed_key_new(NULL, 0, 0, 0, 0, &cert);
  tt_assert(kp1);

  /* Create a new certificate signed by kp1. */
  kp2 = ed_key_new(kp1, INIT_ED_KEY_NEEDCERT, now, 3600, 4, &cert);
  tt_assert(kp2);
  tt_assert(cert);
  tt_mem_op(&cert->signed_key, OP_EQ, &kp2->pubkey,
            sizeof(ed25519_public_key_t));
  tt_assert(! cert->signing_key_included);

  tt_int_op(cert->valid_until, OP_GE, now);
  tt_int_op(cert->valid_until, OP_LE, now+7200);

  /* Create a new key-including certificate signed by kp1 */
  ed25519_keypair_free(kp2);
  tor_cert_free(cert);
  cert = NULL; kp2 = NULL;
  kp2 = ed_key_new(kp1, (INIT_ED_KEY_NEEDCERT|
                         INIT_ED_KEY_INCLUDE_SIGNING_KEY_IN_CERT),
                   now, 3600, 4, &cert);
  tt_assert(kp2);
  tt_assert(cert);
  tt_assert(cert->signing_key_included);
  tt_mem_op(&cert->signed_key, OP_EQ, &kp2->pubkey,
            sizeof(ed25519_public_key_t));
  tt_mem_op(&cert->signing_key, OP_EQ, &kp1->pubkey,
            sizeof(ed25519_public_key_t));

 done:
  ed25519_keypair_free(kp1);
  ed25519_keypair_free(kp2);
  tor_cert_free(cert);
}

static void
test_routerkeys_ed_key_init_basic(void *arg)
{
  (void) arg;

  tor_cert_t *cert = NULL, *cert2 = NULL;
  ed25519_keypair_t *kp1 = NULL, *kp2 = NULL, *kp3 = NULL;
  time_t now = time(NULL);
  char *fname1 = tor_strdup(get_fname("test_ed_key_1"));
  char *fname2 = tor_strdup(get_fname("test_ed_key_2"));
  struct stat st;

  unlink(fname1);
  unlink(fname2);

  /* Fail to load a key that isn't there. */
  kp1 = ed_key_init_from_file(fname1, 0, LOG_INFO, NULL, now, 0, 7, &cert,
                              NULL);
  tt_assert(kp1 == NULL);
  tt_assert(cert == NULL);

  /* Create the key if requested to do so. */
  kp1 = ed_key_init_from_file(fname1, INIT_ED_KEY_CREATE, LOG_INFO,
                              NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp1 != NULL);
  tt_assert(cert == NULL);
  tt_int_op(stat(get_fname("test_ed_key_1_cert"), &st), OP_LT, 0);
  tt_int_op(stat(get_fname("test_ed_key_1_secret_key"), &st), OP_EQ, 0);

  /* Fail to load if we say we need a cert */
  kp2 = ed_key_init_from_file(fname1, INIT_ED_KEY_NEEDCERT, LOG_INFO,
                              NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 == NULL);

  /* Fail to load if we say the wrong key type */
  kp2 = ed_key_init_from_file(fname1, 0, LOG_INFO,
                              NULL, now, 0, 6, &cert, NULL);
  tt_assert(kp2 == NULL);

  /* Load successfully if we're not picky, whether we say "create" or not. */
  kp2 = ed_key_init_from_file(fname1, INIT_ED_KEY_CREATE, LOG_INFO,
                              NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert == NULL);
  tt_mem_op(kp1, OP_EQ, kp2, sizeof(*kp1));
  ed25519_keypair_free(kp2); kp2 = NULL;

  kp2 = ed_key_init_from_file(fname1, 0, LOG_INFO,
                              NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert == NULL);
  tt_mem_op(kp1, OP_EQ, kp2, sizeof(*kp1));
  ed25519_keypair_free(kp2); kp2 = NULL;

  /* Now create a key with a cert. */
  kp2 = ed_key_init_from_file(fname2, (INIT_ED_KEY_CREATE|
                                       INIT_ED_KEY_NEEDCERT),
                              LOG_INFO, kp1, now, 7200, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert != NULL);
  tt_mem_op(kp1, OP_NE, kp2, sizeof(*kp1));
  tt_int_op(stat(get_fname("test_ed_key_2_cert"), &st), OP_EQ, 0);
  tt_int_op(stat(get_fname("test_ed_key_2_secret_key"), &st), OP_EQ, 0);

  tt_assert(cert->cert_valid == 1);
  tt_mem_op(&cert->signed_key, OP_EQ, &kp2->pubkey, 32);

  /* Now verify we can load the cert... */
  kp3 = ed_key_init_from_file(fname2, (INIT_ED_KEY_CREATE|
                                       INIT_ED_KEY_NEEDCERT),
                              LOG_INFO, kp1, now, 7200, 7, &cert2, NULL);
  tt_mem_op(kp2, OP_EQ, kp3, sizeof(*kp2));
  tt_mem_op(cert2->encoded, OP_EQ, cert->encoded, cert->encoded_len);
  ed25519_keypair_free(kp3); kp3 = NULL;
  tor_cert_free(cert2); cert2 = NULL;

  /* ... even without create... */
  kp3 = ed_key_init_from_file(fname2, INIT_ED_KEY_NEEDCERT,
                              LOG_INFO, kp1, now, 7200, 7, &cert2, NULL);
  tt_mem_op(kp2, OP_EQ, kp3, sizeof(*kp2));
  tt_mem_op(cert2->encoded, OP_EQ, cert->encoded, cert->encoded_len);
  ed25519_keypair_free(kp3); kp3 = NULL;
  tor_cert_free(cert2); cert2 = NULL;

  /* ... but that we don't crash or anything if we say we don't want it. */
  kp3 = ed_key_init_from_file(fname2, INIT_ED_KEY_NEEDCERT,
                              LOG_INFO, kp1, now, 7200, 7, NULL, NULL);
  tt_mem_op(kp2, OP_EQ, kp3, sizeof(*kp2));
  ed25519_keypair_free(kp3); kp3 = NULL;

  /* Fail if we're told the wrong signing key */
  kp3 = ed_key_init_from_file(fname2, INIT_ED_KEY_NEEDCERT,
                              LOG_INFO, kp2, now, 7200, 7, &cert2, NULL);
  tt_assert(kp3 == NULL);
  tt_assert(cert2 == NULL);

 done:
  ed25519_keypair_free(kp1);
  ed25519_keypair_free(kp2);
  ed25519_keypair_free(kp3);
  tor_cert_free(cert);
  tor_cert_free(cert2);
  tor_free(fname1);
  tor_free(fname2);
}

static void
test_routerkeys_ed_key_init_split(void *arg)
{
  (void) arg;

  tor_cert_t *cert = NULL;
  ed25519_keypair_t *kp1 = NULL, *kp2 = NULL;
  time_t now = time(NULL);
  char *fname1 = tor_strdup(get_fname("test_ed_key_3"));
  char *fname2 = tor_strdup(get_fname("test_ed_key_4"));
  struct stat st;
  const uint32_t flags = INIT_ED_KEY_SPLIT|INIT_ED_KEY_MISSING_SECRET_OK;

  unlink(fname1);
  unlink(fname2);

  /* Can't load key that isn't there. */
  kp1 = ed_key_init_from_file(fname1, flags, LOG_INFO, NULL, now, 0, 7, &cert,
                              NULL);
  tt_assert(kp1 == NULL);
  tt_assert(cert == NULL);

  /* Create a split key */
  kp1 = ed_key_init_from_file(fname1, flags|INIT_ED_KEY_CREATE,
                              LOG_INFO, NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp1 != NULL);
  tt_assert(cert == NULL);
  tt_int_op(stat(get_fname("test_ed_key_3_cert"), &st), OP_LT, 0);
  tt_int_op(stat(get_fname("test_ed_key_3_secret_key"), &st), OP_EQ, 0);
  tt_int_op(stat(get_fname("test_ed_key_3_public_key"), &st), OP_EQ, 0);

  /* Load it. */
  kp2 = ed_key_init_from_file(fname1, flags|INIT_ED_KEY_CREATE,
                              LOG_INFO, NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert == NULL);
  tt_mem_op(kp1, OP_EQ, kp2, sizeof(*kp2));
  ed25519_keypair_free(kp2); kp2 = NULL;

  /* Okay, try killing the secret key and loading it. */
  unlink(get_fname("test_ed_key_3_secret_key"));
  kp2 = ed_key_init_from_file(fname1, flags,
                              LOG_INFO, NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert == NULL);
  tt_mem_op(&kp1->pubkey, OP_EQ, &kp2->pubkey, sizeof(kp2->pubkey));
  tt_assert(fast_mem_is_zero((char*)kp2->seckey.seckey,
                            sizeof(kp2->seckey.seckey)));
  ed25519_keypair_free(kp2); kp2 = NULL;

  /* Even when we're told to "create", don't create if there's a public key */
  kp2 = ed_key_init_from_file(fname1, flags|INIT_ED_KEY_CREATE,
                              LOG_INFO, NULL, now, 0, 7, &cert, NULL);
  tt_assert(kp2 != NULL);
  tt_assert(cert == NULL);
  tt_mem_op(&kp1->pubkey, OP_EQ, &kp2->pubkey, sizeof(kp2->pubkey));
  tt_assert(fast_mem_is_zero((char*)kp2->seckey.seckey,
                            sizeof(kp2->seckey.seckey)));
  ed25519_keypair_free(kp2); kp2 = NULL;

  /* Make sure we fail on a tag mismatch, though */
  kp2 = ed_key_init_from_file(fname1, flags,
                              LOG_INFO, NULL, now, 0, 99, &cert, NULL);
  tt_assert(kp2 == NULL);

 done:
  ed25519_keypair_free(kp1);
  ed25519_keypair_free(kp2);
  tor_cert_free(cert);
  tor_free(fname1);
  tor_free(fname2);
}

static void
test_routerkeys_ed_keys_init_all(void *arg)
{
  (void)arg;
  char *dir = tor_strdup(get_fname("test_ed_keys_init_all"));
  char *keydir = tor_strdup(get_fname("test_ed_keys_init_all/KEYS"));
  or_options_t *options = tor_malloc_zero(sizeof(or_options_t));
  time_t now = time(NULL);
  ed25519_public_key_t id;
  ed25519_keypair_t sign, auth;
  tor_cert_t *link_cert = NULL;

  get_options_mutable()->ORPort_set = 1;

  crypto_pk_t *rsa = pk_generate(0);

  set_server_identity_key(rsa);
  set_client_identity_key(rsa);

  router_initialize_tls_context();

  options->SigningKeyLifetime = 30*86400;
  options->TestingAuthKeyLifetime = 2*86400;
  options->TestingLinkCertLifetime = 2*86400;
  options->TestingSigningKeySlop = 2*86400;
  options->TestingAuthKeySlop = 2*3600;
  options->TestingLinkKeySlop = 2*3600;

#ifdef _WIN32
  tt_int_op(0, OP_EQ, mkdir(dir));
  tt_int_op(0, OP_EQ, mkdir(keydir));
#else
  tt_int_op(0, OP_EQ, mkdir(dir, 0700));
  tt_int_op(0, OP_EQ, mkdir(keydir, 0700));
#endif /* defined(_WIN32) */

  options->DataDirectory = dir;
  options->KeyDirectory = keydir;

  tt_int_op(1, OP_EQ, load_ed_keys(options, now));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now, 0));
  tt_assert(get_master_identity_key());
  tt_assert(get_master_identity_key());
  tt_assert(get_master_signing_keypair());
  tt_assert(get_current_auth_keypair());
  tt_assert(get_master_signing_key_cert());
  tt_assert(get_current_link_cert_cert());
  tt_assert(get_current_auth_key_cert());
  memcpy(&id, get_master_identity_key(), sizeof(id));
  memcpy(&sign, get_master_signing_keypair(), sizeof(sign));
  memcpy(&auth, get_current_auth_keypair(), sizeof(auth));
  link_cert = tor_cert_dup(get_current_link_cert_cert());

  /* Call load_ed_keys again, but nothing has changed. */
  tt_int_op(0, OP_EQ, load_ed_keys(options, now));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now, 0));
  tt_mem_op(&id, OP_EQ, get_master_identity_key(), sizeof(id));
  tt_mem_op(&sign, OP_EQ, get_master_signing_keypair(), sizeof(sign));
  tt_mem_op(&auth, OP_EQ, get_current_auth_keypair(), sizeof(auth));
  tt_assert(tor_cert_eq(link_cert, get_current_link_cert_cert()));

  /* Force a reload: we make new link/auth keys. */
  routerkeys_free_all();
  tt_int_op(1, OP_EQ, load_ed_keys(options, now));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now, 0));
  tt_mem_op(&id, OP_EQ, get_master_identity_key(), sizeof(id));
  tt_mem_op(&sign, OP_EQ, get_master_signing_keypair(), sizeof(sign));
  tt_assert(tor_cert_eq(link_cert, get_current_link_cert_cert()));
  tt_mem_op(&auth, OP_NE, get_current_auth_keypair(), sizeof(auth));
  tt_assert(get_master_signing_key_cert());
  tt_assert(get_current_link_cert_cert());
  tt_assert(get_current_auth_key_cert());
  tor_cert_free(link_cert);
  link_cert = tor_cert_dup(get_current_link_cert_cert());
  memcpy(&auth, get_current_auth_keypair(), sizeof(auth));

  /* Force a link/auth-key regeneration by advancing time. */
  tt_int_op(0, OP_EQ, load_ed_keys(options, now+3*86400));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now+3*86400, 0));
  tt_mem_op(&id, OP_EQ, get_master_identity_key(), sizeof(id));
  tt_mem_op(&sign, OP_EQ, get_master_signing_keypair(), sizeof(sign));
  tt_assert(! tor_cert_eq(link_cert, get_current_link_cert_cert()));
  tt_mem_op(&auth, OP_NE, get_current_auth_keypair(), sizeof(auth));
  tt_assert(get_master_signing_key_cert());
  tt_assert(get_current_link_cert_cert());
  tt_assert(get_current_auth_key_cert());
  tor_cert_free(link_cert);
  link_cert = tor_cert_dup(get_current_link_cert_cert());
  memcpy(&auth, get_current_auth_keypair(), sizeof(auth));

  /* Force a signing-key regeneration by advancing time. */
  tt_int_op(1, OP_EQ, load_ed_keys(options, now+100*86400));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now+100*86400, 0));
  tt_mem_op(&id, OP_EQ, get_master_identity_key(), sizeof(id));
  tt_mem_op(&sign, OP_NE, get_master_signing_keypair(), sizeof(sign));
  tt_assert(! tor_cert_eq(link_cert, get_current_link_cert_cert()));
  tt_mem_op(&auth, OP_NE, get_current_auth_keypair(), sizeof(auth));
  tt_assert(get_master_signing_key_cert());
  tt_assert(get_current_link_cert_cert());
  tt_assert(get_current_auth_key_cert());
  memcpy(&sign, get_master_signing_keypair(), sizeof(sign));
  tor_cert_free(link_cert);
  link_cert = tor_cert_dup(get_current_link_cert_cert());
  memcpy(&auth, get_current_auth_keypair(), sizeof(auth));

  /* Demonstrate that we can start up with no secret identity key */
  routerkeys_free_all();
  unlink(get_fname("test_ed_keys_init_all/KEYS/"
                   "ed25519_master_id_secret_key"));
  tt_int_op(1, OP_EQ, load_ed_keys(options, now));
  tt_int_op(0, OP_EQ, generate_ed_link_cert(options, now, 0));
  tt_mem_op(&id, OP_EQ, get_master_identity_key(), sizeof(id));
  tt_mem_op(&sign, OP_EQ, get_master_signing_keypair(), sizeof(sign));
  tt_assert(! tor_cert_eq(link_cert, get_current_link_cert_cert()));
  tt_mem_op(&auth, OP_NE, get_current_auth_keypair(), sizeof(auth));
  tt_assert(get_master_signing_key_cert());
  tt_assert(get_current_link_cert_cert());
  tt_assert(get_current_auth_key_cert());

  /* But we're in trouble if we have no id key and our signing key has
     expired. */
  log_global_min_severity_ = LOG_ERR; /* Suppress warnings.
                                       * XXX (better way to do this)? */
  routerkeys_free_all();
  tt_int_op(-1, OP_EQ, load_ed_keys(options, now+200*86400));

 done:
  tor_free(dir);
  tor_free(keydir);
  tor_free(options);
  tor_cert_free(link_cert);
  routerkeys_free_all();
}

static void
test_routerkeys_cross_certify_ntor(void *args)
{
  (void) args;

  tor_cert_t *cert = NULL;
  curve25519_keypair_t onion_keys;
  ed25519_public_key_t master_key;
  ed25519_public_key_t onion_check_key;
  time_t now = time(NULL);
  int sign;

  tt_int_op(0, OP_EQ, ed25519_public_from_base64(&master_key,
                               "IamwritingthesetestsOnARainyAfternoonin2014"));
  tt_int_op(0, OP_EQ, curve25519_keypair_generate(&onion_keys, 0));
  cert = make_ntor_onion_key_crosscert(&onion_keys,
                                       &master_key,
                                       now, 10000,
                                       &sign);
  tt_assert(cert);
  tt_assert(sign == 0 || sign == 1);
  tt_int_op(cert->cert_type, OP_EQ, CERT_TYPE_ONION_ID);
  tt_int_op(1, OP_EQ, ed25519_pubkey_eq(&cert->signed_key, &master_key));
  tt_int_op(0, OP_EQ, ed25519_public_key_from_curve25519_public_key(
                               &onion_check_key, &onion_keys.pubkey, sign));
  tt_int_op(0, OP_EQ, tor_cert_checksig(cert, &onion_check_key, now));

 done:
  tor_cert_free(cert);
}

static void
test_routerkeys_cross_certify_tap(void *args)
{
  (void)args;
  uint8_t *cc = NULL;
  int cc_len;
  ed25519_public_key_t master_key;
  crypto_pk_t *onion_key = pk_generate(2), *id_key = pk_generate(1);
  char digest[20];
  char buf[128];
  int n;

  tt_int_op(0, OP_EQ, ed25519_public_from_base64(&master_key,
                               "IAlreadyWroteTestsForRouterdescsUsingTheseX"));

  cc = make_tap_onion_key_crosscert(onion_key,
                                    &master_key,
                                    id_key, &cc_len);
  tt_assert(cc);
  tt_assert(cc_len);

  n = crypto_pk_public_checksig(onion_key, buf, sizeof(buf),
                                (char*)cc, cc_len);
  tt_int_op(n,OP_GT,0);
  tt_int_op(n,OP_EQ,52);

  crypto_pk_get_digest(id_key, digest);
  tt_mem_op(buf,OP_EQ,digest,20);
  tt_mem_op(buf+20,OP_EQ,master_key.pubkey,32);

  tt_int_op(0, OP_EQ, check_tap_onion_key_crosscert(cc, cc_len,
                                    onion_key, &master_key, (uint8_t*)digest));

 done:
  tor_free(cc);
  crypto_pk_free(id_key);
  crypto_pk_free(onion_key);
}

static void
test_routerkeys_rsa_ed_crosscert(void *arg)
{
  (void)arg;
  ed25519_public_key_t ed;
  crypto_pk_t *rsa = pk_generate(2);

  uint8_t *cc = NULL;
  ssize_t cc_len;
  time_t expires_in = 1470846177;

  tt_int_op(0, OP_EQ, ed25519_public_from_base64(&ed,
                        "ThisStringCanContainAnythingSoNoKeyHereNowX"));
  cc_len = tor_make_rsa_ed25519_crosscert(&ed, rsa, expires_in, &cc);

  tt_int_op(cc_len, OP_GT, 0);
  tt_int_op(cc_len, OP_GT, 37); /* key, expires, siglen */
  tt_mem_op(cc, OP_EQ, ed.pubkey, 32);
  time_t expires_out = 3600 * ntohl(get_uint32(cc+32));
  tt_int_op(expires_out, OP_GE, expires_in);
  tt_int_op(expires_out, OP_LE, expires_in + 3600);

  tt_int_op(cc_len, OP_EQ, 37 + get_uint8(cc+36));

  tt_int_op(0, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len, rsa, &ed,
                                                  expires_in - 10));

  /* Now try after it has expired */
  tt_int_op(-4, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len, rsa, &ed,
                                                  expires_out + 1));

  /* Truncated object */
  tt_int_op(-2, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len - 2, rsa, &ed,
                                                  expires_in - 10));

  /* Key not as expected */
  cc[0] ^= 3;
  tt_int_op(-3, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len, rsa, &ed,
                                                  expires_in - 10));
  cc[0] ^= 3;

  /* Bad signature */
  cc[40] ^= 3;
  tt_int_op(-5, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len, rsa, &ed,
                                                   expires_in - 10));
  cc[40] ^= 3;

  /* Signature of wrong data */
  cc[0] ^= 3;
  ed.pubkey[0] ^= 3;
  tt_int_op(-6, OP_EQ, rsa_ed25519_crosscert_check(cc, cc_len, rsa, &ed,
                                                  expires_in - 10));
  cc[0] ^= 3;
  ed.pubkey[0] ^= 3;

 done:
  crypto_pk_free(rsa);
  tor_free(cc);
}

#define TEST(name, flags)                                       \
  { #name , test_routerkeys_ ## name, (flags), NULL, NULL }

struct testcase_t routerkeys_tests[] = {
  TEST(write_fingerprint, TT_FORK),
  TEST(write_ed25519_identity, TT_FORK),
  TEST(ed_certs, TT_FORK),
  TEST(ed_key_create, TT_FORK),
  TEST(ed_key_init_basic, TT_FORK),
  TEST(ed_key_init_split, TT_FORK),
  TEST(ed_keys_init_all, TT_FORK),
  TEST(cross_certify_ntor, 0),
  TEST(cross_certify_tap, 0),
  TEST(rsa_ed_crosscert, 0),
  END_OF_TESTCASES
};
