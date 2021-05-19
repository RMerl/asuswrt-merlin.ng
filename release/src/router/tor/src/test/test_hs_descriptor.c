/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_descriptor.c
 * \brief Test hidden service descriptor encoding and decoding.
 */

#define HS_DESCRIPTOR_PRIVATE

#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "trunnel/ed25519_cert.h"
#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/hs/hs_descriptor.h"
#include "test/test.h"
#include "feature/nodelist/torcert.h"

#include "test/hs_test_helpers.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/rng_test_helpers.h"

#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
DISABLE_GCC_WARNING("-Woverlength-strings")
/* We allow huge string constants in the unit tests, but not in the code
 * at large. */
#endif
#include "test_hs_descriptor.inc"
ENABLE_GCC_WARNING("-Woverlength-strings")

/* Test certificate encoding put in a descriptor. */
static void
test_cert_encoding(void *arg)
{
  int ret;
  char *encoded = NULL;
  ed25519_keypair_t kp;
  ed25519_public_key_t signed_key;
  ed25519_secret_key_t secret_key;
  tor_cert_t *cert = NULL;

  (void) arg;

  /* Change time to 03-01-2002 23:36 UTC */
  update_approx_time(1010101010);
  time_t now = approx_time();

  ret = ed25519_keypair_generate(&kp, 0);
  tt_int_op(ret, == , 0);
  ret = ed25519_secret_key_generate(&secret_key, 0);
  tt_int_op(ret, == , 0);
  ret = ed25519_public_key_generate(&signed_key, &secret_key);
  tt_int_op(ret, == , 0);

  cert = tor_cert_create_ed25519(&kp, CERT_TYPE_SIGNING_AUTH, &signed_key,
                         now, 3600 * 2, CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(cert);

  /* Test the certificate encoding function. */
  ret = tor_cert_encode_ed22519(cert, &encoded);
  tt_int_op(ret, OP_EQ, 0);

  /* Validated the certificate string. */
  {
    char *end, *pos = encoded;
    char *b64_cert, buf[256];
    size_t b64_cert_len;
    tor_cert_t *parsed_cert;

    tt_int_op(strcmpstart(pos, "-----BEGIN ED25519 CERT-----\n"), OP_EQ, 0);
    pos += strlen("-----BEGIN ED25519 CERT-----\n");

    /* Isolate the base64 encoded certificate and try to decode it. */
    end = strstr(pos, "-----END ED25519 CERT-----");
    tt_assert(end);
    b64_cert = pos;
    b64_cert_len = end - pos;
    ret = base64_decode(buf, sizeof(buf), b64_cert, b64_cert_len);
    tt_int_op(ret, OP_GT, 0);
    /* Parseable? */
    parsed_cert = tor_cert_parse((uint8_t *) buf, ret);
    tt_assert(parsed_cert);
    /* Signature is valid? */
    ret = tor_cert_checksig(parsed_cert, &kp.pubkey, now + 10);
    tt_int_op(ret, OP_EQ, 0);
    ret = tor_cert_eq(cert, parsed_cert);
    tt_int_op(ret, OP_EQ, 1);
    /* The cert did have the signing key? */
    ret= ed25519_pubkey_eq(&parsed_cert->signing_key, &kp.pubkey);
    tt_int_op(ret, OP_EQ, 1);

    /* Get to the end part of the certificate. */
    pos += b64_cert_len;
    tt_int_op(strcmpstart(pos, "-----END ED25519 CERT-----"), OP_EQ, 0);
    pos += strlen("-----END ED25519 CERT-----");
    tt_str_op(pos, OP_EQ, "");

    /* Check that certificate expiry works properly and emits the right log
       message */
    const char *msg = "fire";
    /* Move us forward 4 hours so that the the certificate is definitely
       expired */
    update_approx_time(approx_time() + 3600*4);
    setup_full_capture_of_logs(LOG_PROTOCOL_WARN);
    ret = cert_is_valid(parsed_cert, CERT_TYPE_SIGNING_AUTH, msg);
    tt_int_op(ret, OP_EQ, 0);
    /* Since the current time at the creation of the cert was "03-01-2002
     * 23:36", and the expiration date of the cert was two hours, the Tor code
     * will ceiling that and make it 02:00. Make sure that the right log
     * message is emitted */
    expect_log_msg_containing("Invalid signature for fire: expired"
                              " (2002-01-04 02:00:00)");
    teardown_capture_of_logs();

    tor_cert_free(parsed_cert);
  }

 done:
  tor_cert_free(cert);
  tor_free(encoded);
}

/* Test the descriptor padding. */
static void
test_descriptor_padding(void *arg)
{
  char *plaintext;
  size_t plaintext_len, padded_len;
  uint8_t *padded_plaintext = NULL;

/* Example: if l = 129, the ceiled division gives 2 and then multiplied by 128
 * to give 256. With l = 127, ceiled division gives 1 then times 128. */
#define PADDING_EXPECTED_LEN(l) \
  CEIL_DIV(l, HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE) * \
  HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE

  (void) arg;

  { /* test #1: no padding */
    plaintext_len = HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE;
    plaintext = tor_malloc(plaintext_len);
    padded_len = build_plaintext_padding(plaintext, plaintext_len,
                                         &padded_plaintext);
    tt_assert(padded_plaintext);
    tor_free(plaintext);
    /* Make sure our padding has been zeroed. */
    tt_int_op(fast_mem_is_zero((char *) padded_plaintext + plaintext_len,
                              padded_len - plaintext_len), OP_EQ, 1);
    tor_free(padded_plaintext);
    /* Never never have a padded length smaller than the plaintext. */
    tt_int_op(padded_len, OP_GE, plaintext_len);
    tt_int_op(padded_len, OP_EQ, PADDING_EXPECTED_LEN(plaintext_len));
  }

  { /* test #2: one byte padding? */
    plaintext_len = HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE - 1;
    plaintext = tor_malloc(plaintext_len);
    padded_plaintext = NULL;
    padded_len = build_plaintext_padding(plaintext, plaintext_len,
                                         &padded_plaintext);
    tt_assert(padded_plaintext);
    tor_free(plaintext);
    /* Make sure our padding has been zeroed. */
    tt_int_op(fast_mem_is_zero((char *) padded_plaintext + plaintext_len,
                              padded_len - plaintext_len), OP_EQ, 1);
    tor_free(padded_plaintext);
    /* Never never have a padded length smaller than the plaintext. */
    tt_int_op(padded_len, OP_GE, plaintext_len);
    tt_int_op(padded_len, OP_EQ, PADDING_EXPECTED_LEN(plaintext_len));
  }

  { /* test #3: Lots more bytes of padding? */
    plaintext_len = HS_DESC_SUPERENC_PLAINTEXT_PAD_MULTIPLE + 1;
    plaintext = tor_malloc(plaintext_len);
    padded_plaintext = NULL;
    padded_len = build_plaintext_padding(plaintext, plaintext_len,
                                         &padded_plaintext);
    tt_assert(padded_plaintext);
    tor_free(plaintext);
    /* Make sure our padding has been zeroed. */
    tt_int_op(fast_mem_is_zero((char *) padded_plaintext + plaintext_len,
                              padded_len - plaintext_len), OP_EQ, 1);
    tor_free(padded_plaintext);
    /* Never never have a padded length smaller than the plaintext. */
    tt_int_op(padded_len, OP_GE, plaintext_len);
    tt_int_op(padded_len, OP_EQ, PADDING_EXPECTED_LEN(plaintext_len));
  }

 done:
  return;
}

static void
test_encode_descriptor(void *arg)
{
  int ret;
  ed25519_keypair_t signing_kp;
  hs_descriptor_t *desc = NULL;

  (void) arg;

  ret = ed25519_keypair_generate(&signing_kp, 0);
  tt_int_op(ret, OP_EQ, 0);
  desc = hs_helper_build_hs_desc_with_ip(&signing_kp);

  {
    char *encoded = NULL;
    ret = hs_desc_encode_descriptor(desc, &signing_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    tor_free(encoded);
  }

  {
    char *encoded = NULL;
    uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];

    crypto_strongest_rand(descriptor_cookie, sizeof(descriptor_cookie));

    ret = hs_desc_encode_descriptor(desc, &signing_kp,
                                   descriptor_cookie, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    tor_free(encoded);
  }
 done:
  hs_descriptor_free(desc);
}

static void
test_decode_descriptor(void *arg)
{
  int ret;
  int i;
  char *encoded = NULL;
  ed25519_keypair_t signing_kp;
  hs_descriptor_t *desc = NULL;
  hs_descriptor_t *decoded = NULL;
  hs_descriptor_t *desc_no_ip = NULL;
  hs_subcredential_t subcredential;

  (void) arg;

  ret = ed25519_keypair_generate(&signing_kp, 0);
  tt_int_op(ret, OP_EQ, 0);
  desc = hs_helper_build_hs_desc_with_ip(&signing_kp);

  hs_helper_get_subcred_from_identity_keypair(&signing_kp,
                                              &subcredential);

  /* Give some bad stuff to the decoding function. */
  ret = hs_desc_decode_descriptor("hladfjlkjadf", &subcredential,
                                  NULL, &decoded);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);

  ret = hs_desc_encode_descriptor(desc, &signing_kp, NULL, &encoded);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
  tt_assert(encoded);

  ret = hs_desc_decode_descriptor(encoded, &subcredential, NULL, &decoded);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
  tt_assert(decoded);

  hs_helper_desc_equal(desc, decoded);

  /* Decode a descriptor with _no_ introduction points. */
  {
    ed25519_keypair_t signing_kp_no_ip;
    ret = ed25519_keypair_generate(&signing_kp_no_ip, 0);
    tt_int_op(ret, OP_EQ, 0);
    hs_helper_get_subcred_from_identity_keypair(&signing_kp_no_ip,
                                                &subcredential);
    desc_no_ip = hs_helper_build_hs_desc_no_ip(&signing_kp_no_ip);
    tt_assert(desc_no_ip);
    tor_free(encoded);
    ret = hs_desc_encode_descriptor(desc_no_ip, &signing_kp_no_ip,
                                    NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);
    hs_descriptor_free(decoded);
    ret = hs_desc_decode_descriptor(encoded, &subcredential, NULL, &decoded);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
    tt_assert(decoded);
  }

  /* Decode a descriptor with auth clients. */
  {
    uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];
    curve25519_keypair_t auth_ephemeral_kp;
    curve25519_keypair_t client_kp, invalid_client_kp;
    smartlist_t *clients;
    hs_desc_authorized_client_t *client, *fake_client;
    client = tor_malloc_zero(sizeof(hs_desc_authorized_client_t));

    /* Prepare all the keys needed to build the auth client. */
    curve25519_keypair_generate(&auth_ephemeral_kp, 0);
    curve25519_keypair_generate(&client_kp, 0);
    curve25519_keypair_generate(&invalid_client_kp, 0);
    crypto_strongest_rand(descriptor_cookie, HS_DESC_DESCRIPTOR_COOKIE_LEN);

    memcpy(&desc->superencrypted_data.auth_ephemeral_pubkey,
           &auth_ephemeral_kp.pubkey, CURVE25519_PUBKEY_LEN);

    hs_helper_get_subcred_from_identity_keypair(&signing_kp,
                                                &subcredential);

    /* Build and add the auth client to the descriptor. */
    clients = desc->superencrypted_data.clients;
    if (!clients) {
      clients = smartlist_new();
    }
    hs_desc_build_authorized_client(&subcredential,
                                    &client_kp.pubkey,
                                    &auth_ephemeral_kp.seckey,
                                    descriptor_cookie, client);
    smartlist_add(clients, client);

    /* We need to add fake auth clients here. */
    for (i=0; i < 15; ++i) {
      fake_client = hs_desc_build_fake_authorized_client();
      smartlist_add(clients, fake_client);
    }
    desc->superencrypted_data.clients = clients;

    /* Test the encoding/decoding in the following lines. */
    tor_free(encoded);
    ret = hs_desc_encode_descriptor(desc, &signing_kp,
                                    descriptor_cookie, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    /* If we do not have the client secret key, the decoding must fail. */
    hs_descriptor_free(decoded);
    ret = hs_desc_decode_descriptor(encoded, &subcredential,
                                    NULL, &decoded);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_NEED_CLIENT_AUTH);
    tt_assert(!decoded);

    /* If we have an invalid client secret key, the decoding must fail. */
    hs_descriptor_free(decoded);
    ret = hs_desc_decode_descriptor(encoded, &subcredential,
                                    &invalid_client_kp.seckey, &decoded);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_BAD_CLIENT_AUTH);
    tt_assert(!decoded);

    /* If we have the client secret key, the decoding must succeed and the
     * decoded descriptor must be correct. */
    ret = hs_desc_decode_descriptor(encoded, &subcredential,
                                    &client_kp.seckey, &decoded);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
    tt_assert(decoded);

    hs_helper_desc_equal(desc, decoded);
  }

 done:
  hs_descriptor_free(desc);
  hs_descriptor_free(desc_no_ip);
  hs_descriptor_free(decoded);
  tor_free(encoded);
}

static void
test_supported_version(void *arg)
{
  int ret;

  (void) arg;

  /* Unsupported. */
  ret = hs_desc_is_supported_version(42);
  tt_int_op(ret, OP_EQ, 0);
  /* To early. */
  ret = hs_desc_is_supported_version(HS_DESC_SUPPORTED_FORMAT_VERSION_MIN - 1);
  tt_int_op(ret, OP_EQ, 0);
  /* One too new. */
  ret = hs_desc_is_supported_version(HS_DESC_SUPPORTED_FORMAT_VERSION_MAX + 1);
  tt_int_op(ret, OP_EQ, 0);
  /* Valid version. */
  ret = hs_desc_is_supported_version(3);
  tt_int_op(ret, OP_EQ, 1);

 done:
  ;
}

static void
test_encrypted_data_len(void *arg)
{
  int ret;
  size_t value;

  (void) arg;

  /* No length, error. */
  ret = encrypted_data_length_is_valid(0);
  tt_int_op(ret, OP_EQ, 0);
  /* Valid value. */
  value = HS_DESC_ENCRYPTED_SALT_LEN + DIGEST256_LEN + 1;
  ret = encrypted_data_length_is_valid(value);
  tt_int_op(ret, OP_EQ, 1);

 done:
  ;
}

static void
test_decode_invalid_intro_point(void *arg)
{
  int ret;
  char *encoded_ip = NULL;
  size_t len_out;
  hs_desc_intro_point_t *ip = NULL;
  ed25519_keypair_t signing_kp;
  hs_descriptor_t *desc = NULL;

  (void) arg;

  /* Separate pieces of a valid encoded introduction point. */
  const char *intro_point =
    "introduction-point AQIUMDI5OUYyNjhGQ0E5RDU1Q0QxNTc=";
  const char *auth_key =
    "auth-key\n"
    "-----BEGIN ED25519 CERT-----\n"
    "AQkACOhAAQW8ltYZMIWpyrfyE/b4Iyi8CNybCwYs6ADk7XfBaxsFAQAgBAD3/BE4\n"
    "XojGE/N2bW/wgnS9r2qlrkydGyuCKIGayYx3haZ39LD4ZTmSMRxwmplMAqzG/XNP\n"
    "0Kkpg4p2/VnLFJRdU1SMFo1lgQ4P0bqw7Tgx200fulZ4KUM5z5V7m+a/mgY=\n"
    "-----END ED25519 CERT-----";
  const char *enc_key =
    "enc-key ntor bpZKLsuhxP6woDQ3yVyjm5gUKSk7RjfAijT2qrzbQk0=";
  const char *enc_key_cert =
    "enc-key-cert\n"
    "-----BEGIN ED25519 CERT-----\n"
    "AQsACOhZAUpNvCZ1aJaaR49lS6MCdsVkhVGVrRqoj0Y2T4SzroAtAQAgBABFOcGg\n"
    "lbTt1DF5nKTE/gU3Fr8ZtlCIOhu1A+F5LM7fqCUupfesg0KTHwyIZOYQbJuM5/he\n"
    "/jDNyLy9woPJdjkxywaY2RPUxGjLYtMQV0E8PUxWyICV+7y52fTCYaKpYQw=\n"
    "-----END ED25519 CERT-----";

  /* Try to decode a junk string. */
  {
    hs_descriptor_free(desc);
    desc = NULL;
    ret = ed25519_keypair_generate(&signing_kp, 0);
    tt_int_op(ret, OP_EQ, 0);
    desc = hs_helper_build_hs_desc_with_ip(&signing_kp);
    const char *junk = "this is not a descriptor";
    ip = decode_introduction_point(desc, junk);
    tt_ptr_op(ip, OP_EQ, NULL);
    hs_desc_intro_point_free(ip);
    ip = NULL;
  }

  /* Invalid link specifiers. */
  {
    smartlist_t *lines = smartlist_new();
    const char *bad_line = "introduction-point blah";
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) auth_key);
    smartlist_add(lines, (char *) enc_key);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
    hs_desc_intro_point_free(ip);
    ip = NULL;
  }

  /* Invalid auth key type. */
  {
    smartlist_t *lines = smartlist_new();
    /* Try to put a valid object that our tokenize function will be able to
     * parse but that has nothing to do with the auth_key. */
    const char *bad_line =
      "auth-key\n"
      "-----BEGIN UNICORN CERT-----\n"
      "MIGJAoGBAO4bATcW8kW4h6RQQAKEgg+aXCpF4JwbcO6vGZtzXTDB+HdPVQzwqkbh\n"
      "XzFM6VGArhYw4m31wcP1Z7IwULir7UMnAFd7Zi62aYfU6l+Y1yAoZ1wzu1XBaAMK\n"
      "ejpwQinW9nzJn7c2f69fVke3pkhxpNdUZ+vplSA/l9iY+y+v+415AgMBAAE=\n"
      "-----END UNICORN CERT-----";
    /* Build intro point text. */
    smartlist_add(lines, (char *) intro_point);
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) enc_key);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
  }

  /* Invalid enc-key. */
  {
    smartlist_t *lines = smartlist_new();
    const char *bad_line =
      "enc-key unicorn bpZKLsuhxP6woDQ3yVyjm5gUKSk7RjfAijT2qrzbQk0=";
    /* Build intro point text. */
    smartlist_add(lines, (char *) intro_point);
    smartlist_add(lines, (char *) auth_key);
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
  }

  /* Invalid enc-key object. */
  {
    smartlist_t *lines = smartlist_new();
    const char *bad_line = "enc-key ntor";
    /* Build intro point text. */
    smartlist_add(lines, (char *) intro_point);
    smartlist_add(lines, (char *) auth_key);
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
  }

  /* Invalid enc-key base64 curv25519 key. */
  {
    smartlist_t *lines = smartlist_new();
    const char *bad_line = "enc-key ntor blah===";
    /* Build intro point text. */
    smartlist_add(lines, (char *) intro_point);
    smartlist_add(lines, (char *) auth_key);
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
  }

  /* Invalid enc-key invalid legacy. */
  {
    smartlist_t *lines = smartlist_new();
    const char *bad_line = "legacy-key blah===";
    /* Build intro point text. */
    smartlist_add(lines, (char *) intro_point);
    smartlist_add(lines, (char *) auth_key);
    smartlist_add(lines, (char *) bad_line);
    smartlist_add(lines, (char *) enc_key_cert);
    encoded_ip = smartlist_join_strings(lines, "\n", 0, &len_out);
    tt_assert(encoded_ip);
    ip = decode_introduction_point(desc, encoded_ip);
    tt_ptr_op(ip, OP_EQ, NULL);
    tor_free(encoded_ip);
    smartlist_free(lines);
  }

 done:
  hs_descriptor_free(desc);
  hs_desc_intro_point_free(ip);
}

/** Make sure we fail gracefully when decoding the bad desc from #23233. */
static void
test_decode_bad_signature(void *arg)
{
  hs_desc_plaintext_data_t desc_plaintext;
  int ret;

  (void) arg;

  memset(&desc_plaintext, 0, sizeof(desc_plaintext));

  /* Update approx time to dodge cert expiration */
  update_approx_time(1502661599);

  setup_full_capture_of_logs(LOG_WARN);
  ret = hs_desc_decode_plaintext(HS_DESC_BAD_SIG, &desc_plaintext);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  expect_log_msg_containing("Malformed signature line. Rejecting.");
  teardown_capture_of_logs();

 done:
  hs_desc_plaintext_data_free_contents(&desc_plaintext);
}

static void
test_decode_plaintext(void *arg)
{
  int ret;
  hs_desc_plaintext_data_t desc_plaintext;
  const char *bad_value = "unicorn";

  (void) arg;

#define template \
    "hs-descriptor %s\n" \
    "descriptor-lifetime %s\n" \
    "descriptor-signing-key-cert\n" \
    "-----BEGIN ED25519 CERT-----\n" \
    "AQgABjvPAQaG3g+dc6oV/oJV4ODAtkvx56uBnPtBT9mYVuHVOhn7AQAgBABUg3mQ\n" \
    "myBr4bu5LCr53wUEbW2EXui01CbUgU7pfo9LvJG3AcXRojj6HlfsUs9BkzYzYdjF\n" \
    "A69Apikgu0ewHYkFFASt7Il+gB3w6J8YstQJZT7dtbtl+doM7ug8B68Qdg8=\n" \
    "-----END ED25519 CERT-----\n" \
    "revision-counter %s\n" \
    "encrypted\n" \
    "-----BEGIN %s-----\n" \
    "UNICORN\n" \
    "-----END MESSAGE-----\n" \
    "signature m20WJH5agqvwhq7QeuEZ1mYyPWQDO+eJOZUjLhAiKu8DbL17DsDfJE6kXbWy" \
    "HimbNj2we0enV3cCOOAsmPOaAw\n"

  /* Invalid version. */
  {
    char *plaintext;
    tor_asprintf(&plaintext, template, bad_value, "180", "42", "MESSAGE");
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Missing fields. */
  {
    const char *plaintext = "hs-descriptor 3\n";
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Max length. */
  {
    size_t big = 64000;
    /* Must always be bigger than HS_DESC_MAX_LEN. */
    tt_int_op(HS_DESC_MAX_LEN, OP_LT, big);
    char *plaintext = tor_malloc_zero(big);
    memset(plaintext, 'a', big);
    plaintext[big - 1] = '\0';
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Bad lifetime value. */
  {
    char *plaintext;
    tor_asprintf(&plaintext, template, "3", bad_value, "42", "MESSAGE");
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Huge lifetime value. */
  {
    char *plaintext;
    tor_asprintf(&plaintext, template, "3", "7181615", "42", "MESSAGE");
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Invalid encrypted section. */
  {
    char *plaintext;
    tor_asprintf(&plaintext, template, "3", "180", "42", bad_value);
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

  /* Invalid revision counter. */
  {
    char *plaintext;
    tor_asprintf(&plaintext, template, "3", "180", bad_value, "MESSAGE");
    ret = hs_desc_decode_plaintext(plaintext, &desc_plaintext);
    tor_free(plaintext);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_PLAINTEXT_ERROR);
  }

 done:
  ;
}

static void
test_validate_cert(void *arg)
{
  int ret;
  time_t now = time(NULL);
  ed25519_keypair_t kp;
  tor_cert_t *cert = NULL;

  (void) arg;

  ret = ed25519_keypair_generate(&kp, 0);
  tt_int_op(ret, OP_EQ, 0);

  /* Cert of type CERT_TYPE_AUTH_HS_IP_KEY. */
  cert = tor_cert_create_ed25519(&kp, CERT_TYPE_AUTH_HS_IP_KEY,
                                     &kp.pubkey, now, 3600,
                                     CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(cert);
  /* Test with empty certificate. */
  ret = cert_is_valid(NULL, CERT_TYPE_AUTH_HS_IP_KEY, "unicorn");
  tt_int_op(ret, OP_EQ, 0);
  /* Test with a bad type. */
  ret = cert_is_valid(cert, CERT_TYPE_SIGNING_HS_DESC, "unicorn");
  tt_int_op(ret, OP_EQ, 0);
  /* Normal validation. */
  ret = cert_is_valid(cert, CERT_TYPE_AUTH_HS_IP_KEY, "unicorn");
  tt_int_op(ret, OP_EQ, 1);
  /* Break signing key so signature verification will fails. */
  memset(&cert->signing_key, 0, sizeof(cert->signing_key));
  ret = cert_is_valid(cert, CERT_TYPE_AUTH_HS_IP_KEY, "unicorn");
  tt_int_op(ret, OP_EQ, 0);
  tor_cert_free(cert);

  /* Try a cert without including the signing key. */
  cert = tor_cert_create_ed25519(&kp, CERT_TYPE_AUTH_HS_IP_KEY,
                                 &kp.pubkey, now, 3600, 0);

  tt_assert(cert);
  /* Test with a bad type. */
  ret = cert_is_valid(cert, CERT_TYPE_AUTH_HS_IP_KEY, "unicorn");
  tt_int_op(ret, OP_EQ, 0);

 done:
  tor_cert_free(cert);
}

static void
test_desc_signature(void *arg)
{
  int ret;
  char *data = NULL, *desc = NULL;
  char sig_b64[ED25519_SIG_BASE64_LEN + 1];
  ed25519_keypair_t kp;
  ed25519_signature_t sig;

  (void) arg;

  ed25519_keypair_generate(&kp, 0);
  /* Setup a phoony descriptor but with a valid signature token that is the
   * signature is verifiable. */
  tor_asprintf(&data, "This is a signed descriptor\n");
  ret = ed25519_sign_prefixed(&sig, (const uint8_t *) data, strlen(data),
                              "Tor onion service descriptor sig v3", &kp);
  tt_int_op(ret, OP_EQ, 0);
  ed25519_signature_to_base64(sig_b64, &sig);
  /* Build the descriptor that should be valid. */
  tor_asprintf(&desc, "%ssignature %s\n", data, sig_b64);
  ret = desc_sig_is_valid(sig_b64, &kp.pubkey, desc, strlen(desc));
  tt_int_op(ret, OP_EQ, 1);
  /* Junk signature. */
  ret = desc_sig_is_valid("JUNK", &kp.pubkey, desc, strlen(desc));
  tt_int_op(ret, OP_EQ, 0);

 done:
  tor_free(desc);
  tor_free(data);
}

static void
test_build_authorized_client(void *arg)
{
  int ret;
  hs_desc_authorized_client_t *desc_client = NULL;
  uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];
  curve25519_secret_key_t auth_ephemeral_sk;
  curve25519_secret_key_t client_auth_sk;
  curve25519_public_key_t client_auth_pk;
  const char ephemeral_sk_b16[] =
    "d023b674d993a5c8446bd2ca97e9961149b3c0e88c7dc14e8777744dd3468d6a";
  const char descriptor_cookie_b16[] =
    "07d087f1d8c68393721f6e70316d3b29";
  const char client_pubkey_b16[] =
    "8c1298fa6050e372f8598f6deca32e27b0ad457741422c2629ebb132cf7fae37";
  hs_subcredential_t subcredential;
  char *mem_op_hex_tmp=NULL;

  (void) arg;

  ret = curve25519_secret_key_generate(&auth_ephemeral_sk, 0);
  tt_int_op(ret, OP_EQ, 0);

  ret = curve25519_secret_key_generate(&client_auth_sk, 0);
  tt_int_op(ret, OP_EQ, 0);
  curve25519_public_key_generate(&client_auth_pk, &client_auth_sk);

  memset(subcredential.subcred, 42, sizeof(subcredential));

  desc_client = tor_malloc_zero(sizeof(hs_desc_authorized_client_t));

  base16_decode((char *) &auth_ephemeral_sk,
                sizeof(auth_ephemeral_sk),
                ephemeral_sk_b16,
                strlen(ephemeral_sk_b16));

  base16_decode((char *) descriptor_cookie,
                sizeof(descriptor_cookie),
                descriptor_cookie_b16,
                strlen(descriptor_cookie_b16));

  base16_decode((char *) &client_auth_pk,
                sizeof(client_auth_pk),
                client_pubkey_b16,
                strlen(client_pubkey_b16));

  testing_enable_prefilled_rng("\x01", 1);

  hs_desc_build_authorized_client(&subcredential,
                                  &client_auth_pk, &auth_ephemeral_sk,
                                  descriptor_cookie, desc_client);

  test_memeq_hex((char *) desc_client->client_id,
                 "EC19B7FF4D2DDA13");
  test_memeq_hex((char *) desc_client->iv,
                "01010101010101010101010101010101");
  test_memeq_hex((char *) desc_client->encrypted_cookie,
                "B21222BE13F385F355BD07B2381F9F29");

 done:
  tor_free(desc_client);
  tor_free(mem_op_hex_tmp);
  testing_disable_prefilled_rng();
}

struct testcase_t hs_descriptor[] = {
  /* Encoding tests. */
  { "cert_encoding", test_cert_encoding, TT_FORK,
    NULL, NULL },
  { "encode_descriptor", test_encode_descriptor, TT_FORK,
    NULL, NULL },
  { "descriptor_padding", test_descriptor_padding, TT_FORK,
    NULL, NULL },

  /* Decoding tests. */
  { "decode_descriptor", test_decode_descriptor, TT_FORK,
    NULL, NULL },
  { "encrypted_data_len", test_encrypted_data_len, TT_FORK,
    NULL, NULL },
  { "decode_invalid_intro_point", test_decode_invalid_intro_point, TT_FORK,
    NULL, NULL },
  { "decode_plaintext", test_decode_plaintext, TT_FORK,
    NULL, NULL },
  { "decode_bad_signature", test_decode_bad_signature, TT_FORK,
    NULL, NULL },

  /* Misc. */
  { "version", test_supported_version, TT_FORK,
    NULL, NULL },
  { "validate_cert", test_validate_cert, TT_FORK,
    NULL, NULL },
  { "desc_signature", test_desc_signature, TT_FORK,
    NULL, NULL },
  { "build_authorized_client", test_build_authorized_client, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
