/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "test/test.h"
#include "feature/nodelist/torcert.h"

#include "feature/hs/hs_common.h"
#include "test/hs_test_helpers.h"

hs_desc_intro_point_t *
hs_helper_build_intro_point(const ed25519_keypair_t *signing_kp, time_t now,
                            const char *addr, int legacy)
{
  int ret;
  ed25519_keypair_t auth_kp;
  hs_desc_intro_point_t *intro_point = NULL;
  hs_desc_intro_point_t *ip = hs_desc_intro_point_new();

  /* For a usable intro point we need at least two link specifiers: One legacy
   * keyid and one ipv4 */
  {
    hs_desc_link_specifier_t *ls_legacy = tor_malloc_zero(sizeof(*ls_legacy));
    hs_desc_link_specifier_t *ls_v4 = tor_malloc_zero(sizeof(*ls_v4));
    ls_legacy->type = LS_LEGACY_ID;
    memcpy(ls_legacy->u.legacy_id, "0299F268FCA9D55CD157976D39AE92B4B455B3A8",
           DIGEST_LEN);
    ls_v4->u.ap.port = 9001;
    int family = tor_addr_parse(&ls_v4->u.ap.addr, addr);
    switch (family) {
    case AF_INET:
          ls_v4->type = LS_IPV4;
          break;
        case AF_INET6:
          ls_v4->type = LS_IPV6;
          break;
        default:
          /* Stop the test, not suppose to have an error. */
          tt_int_op(family, OP_EQ, AF_INET);
    }
    smartlist_add(ip->link_specifiers, ls_legacy);
    smartlist_add(ip->link_specifiers, ls_v4);
  }

  ret = ed25519_keypair_generate(&auth_kp, 0);
  tt_int_op(ret, ==, 0);
  ip->auth_key_cert = tor_cert_create(signing_kp, CERT_TYPE_AUTH_HS_IP_KEY,
                                      &auth_kp.pubkey, now,
                                      HS_DESC_CERT_LIFETIME,
                                      CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(ip->auth_key_cert);

  if (legacy) {
    ip->legacy.key = crypto_pk_new();
    tt_assert(ip->legacy.key);
    ret = crypto_pk_generate_key(ip->legacy.key);
    tt_int_op(ret, ==, 0);
    ssize_t cert_len = tor_make_rsa_ed25519_crosscert(
                                    &signing_kp->pubkey, ip->legacy.key,
                                    now + HS_DESC_CERT_LIFETIME,
                                    &ip->legacy.cert.encoded);
    tt_assert(ip->legacy.cert.encoded);
    tt_u64_op(cert_len, OP_GT, 0);
    ip->legacy.cert.len = cert_len;
  }

  /* Encryption key. */
  {
    int signbit;
    curve25519_keypair_t curve25519_kp;
    ed25519_keypair_t ed25519_kp;
    tor_cert_t *cross_cert;

    ret = curve25519_keypair_generate(&curve25519_kp, 0);
    tt_int_op(ret, ==, 0);
    ed25519_keypair_from_curve25519_keypair(&ed25519_kp, &signbit,
                                            &curve25519_kp);
    cross_cert = tor_cert_create(signing_kp, CERT_TYPE_CROSS_HS_IP_KEYS,
                                 &ed25519_kp.pubkey, time(NULL),
                                 HS_DESC_CERT_LIFETIME,
                                 CERT_FLAG_INCLUDE_SIGNING_KEY);
    tt_assert(cross_cert);
    ip->enc_key_cert = cross_cert;
  }

  intro_point = ip;
 done:
  if (intro_point == NULL)
    tor_free(ip);

  return intro_point;
}

/* Return a valid hs_descriptor_t object. If no_ip is set, no introduction
 * points are added. */
static hs_descriptor_t *
hs_helper_build_hs_desc_impl(unsigned int no_ip,
                             const ed25519_keypair_t *signing_kp)
{
  int ret;
  int i;
  time_t now = approx_time();
  ed25519_keypair_t blinded_kp;
  curve25519_keypair_t auth_ephemeral_kp;
  hs_descriptor_t *descp = NULL, *desc = tor_malloc_zero(sizeof(*desc));

  desc->plaintext_data.version = HS_DESC_SUPPORTED_FORMAT_VERSION_MAX;

  /* Copy only the public key into the descriptor. */
  memcpy(&desc->plaintext_data.signing_pubkey, &signing_kp->pubkey,
         sizeof(ed25519_public_key_t));

  uint64_t current_time_period = hs_get_time_period_num(0);
  hs_build_blinded_keypair(signing_kp, NULL, 0,
                           current_time_period, &blinded_kp);
  /* Copy only the public key into the descriptor. */
  memcpy(&desc->plaintext_data.blinded_pubkey, &blinded_kp.pubkey,
         sizeof(ed25519_public_key_t));

  desc->plaintext_data.signing_key_cert =
    tor_cert_create(&blinded_kp, CERT_TYPE_SIGNING_HS_DESC,
                    &signing_kp->pubkey, now, 3600,
                    CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(desc->plaintext_data.signing_key_cert);
  desc->plaintext_data.revision_counter = 42;
  desc->plaintext_data.lifetime_sec = 3 * 60 * 60;

  hs_get_subcredential(&signing_kp->pubkey, &blinded_kp.pubkey,
                    desc->subcredential);

  /* Setup superencrypted data section. */
  ret = curve25519_keypair_generate(&auth_ephemeral_kp, 0);
  tt_int_op(ret, ==, 0);
  memcpy(&desc->superencrypted_data.auth_ephemeral_pubkey,
         &auth_ephemeral_kp.pubkey,
         sizeof(curve25519_public_key_t));

  desc->superencrypted_data.clients = smartlist_new();
  for (i = 0; i < HS_DESC_AUTH_CLIENT_MULTIPLE; i++) {
    hs_desc_authorized_client_t *desc_client =
      hs_desc_build_fake_authorized_client();
    smartlist_add(desc->superencrypted_data.clients, desc_client);
  }

  /* Setup encrypted data section. */
  desc->encrypted_data.create2_ntor = 1;
  desc->encrypted_data.intro_auth_types = smartlist_new();
  desc->encrypted_data.single_onion_service = 1;
  smartlist_add(desc->encrypted_data.intro_auth_types, tor_strdup("ed25519"));
  desc->encrypted_data.intro_points = smartlist_new();
  if (!no_ip) {
    /* Add four intro points. */
    smartlist_add(desc->encrypted_data.intro_points,
              hs_helper_build_intro_point(signing_kp, now, "1.2.3.4", 0));
    smartlist_add(desc->encrypted_data.intro_points,
              hs_helper_build_intro_point(signing_kp, now, "[2600::1]", 0));
    smartlist_add(desc->encrypted_data.intro_points,
              hs_helper_build_intro_point(signing_kp, now, "3.2.1.4", 1));
    smartlist_add(desc->encrypted_data.intro_points,
              hs_helper_build_intro_point(signing_kp, now, "5.6.7.8", 1));
  }

  descp = desc;
 done:
  if (descp == NULL)
    tor_free(desc);

  return descp;
}

/** Helper function to get the HS subcredential using the identity keypair of
 *  an HS. Used to decrypt descriptors in unittests. */
void
hs_helper_get_subcred_from_identity_keypair(ed25519_keypair_t *signing_kp,
                                            uint8_t *subcred_out)
{
  ed25519_keypair_t blinded_kp;
  uint64_t current_time_period = hs_get_time_period_num(approx_time());
  hs_build_blinded_keypair(signing_kp, NULL, 0,
                           current_time_period, &blinded_kp);

  hs_get_subcredential(&signing_kp->pubkey, &blinded_kp.pubkey,
                       subcred_out);
}

/* Build a descriptor with introduction points. */
hs_descriptor_t *
hs_helper_build_hs_desc_with_ip(const ed25519_keypair_t *signing_kp)
{
  return hs_helper_build_hs_desc_impl(0, signing_kp);
}

/* Build a descriptor without any introduction points. */
hs_descriptor_t *
hs_helper_build_hs_desc_no_ip(const ed25519_keypair_t *signing_kp)
{
  return hs_helper_build_hs_desc_impl(1, signing_kp);
}

void
hs_helper_desc_equal(const hs_descriptor_t *desc1,
                     const hs_descriptor_t *desc2)
{
  char *addr1 = NULL, *addr2 = NULL;
  /* Plaintext data section. */
  tt_int_op(desc1->plaintext_data.version, OP_EQ,
            desc2->plaintext_data.version);
  tt_uint_op(desc1->plaintext_data.lifetime_sec, OP_EQ,
             desc2->plaintext_data.lifetime_sec);
  tt_assert(tor_cert_eq(desc1->plaintext_data.signing_key_cert,
                        desc2->plaintext_data.signing_key_cert));
  tt_mem_op(desc1->plaintext_data.signing_pubkey.pubkey, OP_EQ,
            desc2->plaintext_data.signing_pubkey.pubkey,
            ED25519_PUBKEY_LEN);
  tt_mem_op(desc1->plaintext_data.blinded_pubkey.pubkey, OP_EQ,
            desc2->plaintext_data.blinded_pubkey.pubkey,
            ED25519_PUBKEY_LEN);
  tt_u64_op(desc1->plaintext_data.revision_counter, ==,
            desc2->plaintext_data.revision_counter);

  /* NOTE: We can't compare the encrypted blob because when encoding the
   * descriptor, the object is immutable thus we don't update it with the
   * encrypted blob. As contrast to the decoding process where we populate a
   * descriptor object. */

  /* Superencrypted data section. */
  tt_mem_op(desc1->superencrypted_data.auth_ephemeral_pubkey.public_key, OP_EQ,
            desc2->superencrypted_data.auth_ephemeral_pubkey.public_key,
            CURVE25519_PUBKEY_LEN);

  /* Auth clients. */
  {
    tt_assert(desc1->superencrypted_data.clients);
    tt_assert(desc2->superencrypted_data.clients);
    tt_int_op(smartlist_len(desc1->superencrypted_data.clients), ==,
              smartlist_len(desc2->superencrypted_data.clients));
    for (int i=0;
         i < smartlist_len(desc1->superencrypted_data.clients);
         i++) {
      hs_desc_authorized_client_t
        *client1 = smartlist_get(desc1->superencrypted_data.clients, i),
        *client2 = smartlist_get(desc2->superencrypted_data.clients, i);
      tt_mem_op(client1->client_id, OP_EQ, client2->client_id,
                sizeof(client1->client_id));
      tt_mem_op(client1->iv, OP_EQ, client2->iv,
                sizeof(client1->iv));
      tt_mem_op(client1->encrypted_cookie, OP_EQ, client2->encrypted_cookie,
                sizeof(client1->encrypted_cookie));
    }
  }

  /* Encrypted data section. */
  tt_uint_op(desc1->encrypted_data.create2_ntor, ==,
             desc2->encrypted_data.create2_ntor);

  /* Authentication type. */
  tt_int_op(!!desc1->encrypted_data.intro_auth_types, ==,
            !!desc2->encrypted_data.intro_auth_types);
  if (desc1->encrypted_data.intro_auth_types &&
      desc2->encrypted_data.intro_auth_types) {
    tt_int_op(smartlist_len(desc1->encrypted_data.intro_auth_types), ==,
              smartlist_len(desc2->encrypted_data.intro_auth_types));
    for (int i = 0;
         i < smartlist_len(desc1->encrypted_data.intro_auth_types);
         i++) {
      tt_str_op(smartlist_get(desc1->encrypted_data.intro_auth_types, i),OP_EQ,
                smartlist_get(desc2->encrypted_data.intro_auth_types, i));
    }
  }

  /* Introduction points. */
  {
    tt_assert(desc1->encrypted_data.intro_points);
    tt_assert(desc2->encrypted_data.intro_points);
    tt_int_op(smartlist_len(desc1->encrypted_data.intro_points), ==,
              smartlist_len(desc2->encrypted_data.intro_points));
    for (int i=0; i < smartlist_len(desc1->encrypted_data.intro_points); i++) {
      hs_desc_intro_point_t *ip1 = smartlist_get(desc1->encrypted_data
                                                 .intro_points, i),
                            *ip2 = smartlist_get(desc2->encrypted_data
                                                 .intro_points, i);
      tt_assert(tor_cert_eq(ip1->auth_key_cert, ip2->auth_key_cert));
      if (ip1->legacy.key) {
        tt_int_op(crypto_pk_cmp_keys(ip1->legacy.key, ip2->legacy.key),
                  OP_EQ, 0);
      } else {
        tt_mem_op(&ip1->enc_key, OP_EQ, &ip2->enc_key, CURVE25519_PUBKEY_LEN);
      }

      tt_int_op(smartlist_len(ip1->link_specifiers), ==,
                smartlist_len(ip2->link_specifiers));
      for (int j = 0; j < smartlist_len(ip1->link_specifiers); j++) {
        hs_desc_link_specifier_t *ls1 = smartlist_get(ip1->link_specifiers, j),
                                 *ls2 = smartlist_get(ip2->link_specifiers, j);
        tt_int_op(ls1->type, ==, ls2->type);
        switch (ls1->type) {
          case LS_IPV4:
          case LS_IPV6:
            {
              addr1 = tor_addr_to_str_dup(&ls1->u.ap.addr);
              addr2 = tor_addr_to_str_dup(&ls2->u.ap.addr);
              tt_str_op(addr1, OP_EQ, addr2);
              tor_free(addr1);
              tor_free(addr2);
              tt_int_op(ls1->u.ap.port, ==, ls2->u.ap.port);
            }
            break;
          case LS_LEGACY_ID:
            tt_mem_op(ls1->u.legacy_id, OP_EQ, ls2->u.legacy_id,
                      sizeof(ls1->u.legacy_id));
            break;
          default:
            /* Unknown type, caught it and print its value. */
            tt_int_op(ls1->type, OP_EQ, -1);
        }
      }
    }
  }

 done:
  tor_free(addr1);
  tor_free(addr2);
}

