/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define HS_CLIENT_PRIVATE

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "test/test.h"
#include "feature/nodelist/torcert.h"

#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_service.h"
#include "test/hs_test_helpers.h"

/**
 * Create an introduction point taken straight out of an HSv3 descriptor.
 *
 * Use 'signing_kp' to sign the introduction point certificates.
 *
 * If 'intro_auth_kp' is provided use that as the introduction point
 * authentication keypair, otherwise generate one on the fly.
 *
 * If 'intro_enc_kp' is provided use that as the introduction point encryption
 * keypair, otherwise generate one on the fly.
 */
hs_desc_intro_point_t *
hs_helper_build_intro_point(const ed25519_keypair_t *signing_kp, time_t now,
                            const char *addr, int legacy,
                            const ed25519_keypair_t *intro_auth_kp,
                            const curve25519_keypair_t *intro_enc_kp)
{
  int ret;
  ed25519_keypair_t auth_kp;
  hs_desc_intro_point_t *intro_point = NULL;
  hs_desc_intro_point_t *ip = hs_desc_intro_point_new();

  /* For a usable intro point we need at least two link specifiers: One legacy
   * keyid and one ipv4 */
  {
    tor_addr_t a;
    tor_addr_make_unspec(&a);
    link_specifier_t *ls_legacy = link_specifier_new();
    link_specifier_t *ls_ip = link_specifier_new();
    link_specifier_set_ls_type(ls_legacy, LS_LEGACY_ID);
    memset(link_specifier_getarray_un_legacy_id(ls_legacy), 'C',
           link_specifier_getlen_un_legacy_id(ls_legacy));
    int family = tor_addr_parse(&a, addr);
    switch (family) {
    case AF_INET:
          link_specifier_set_ls_type(ls_ip, LS_IPV4);
          link_specifier_set_un_ipv4_addr(ls_ip, tor_addr_to_ipv4h(&a));
          link_specifier_set_un_ipv4_port(ls_ip, 9001);
          break;
        case AF_INET6:
          link_specifier_set_ls_type(ls_ip, LS_IPV6);
          memcpy(link_specifier_getarray_un_ipv6_addr(ls_ip),
                 tor_addr_to_in6_addr8(&a),
                 link_specifier_getlen_un_ipv6_addr(ls_ip));
          link_specifier_set_un_ipv6_port(ls_ip, 9001);
          break;
        default:
          /* Stop the test, not supposed to have an error.
           * Compare with -1 to show the actual family.
           */
          tt_int_op(family, OP_EQ, -1);
    }
    smartlist_add(ip->link_specifiers, ls_legacy);
    smartlist_add(ip->link_specifiers, ls_ip);
  }

  if (intro_auth_kp) {
    memcpy(&auth_kp, intro_auth_kp, sizeof(ed25519_keypair_t));
  } else {
    ret = ed25519_keypair_generate(&auth_kp, 0);
    tt_int_op(ret, OP_EQ, 0);
  }
  ip->auth_key_cert = tor_cert_create_ed25519(signing_kp,
                                      CERT_TYPE_AUTH_HS_IP_KEY,
                                      &auth_kp.pubkey, now,
                                      HS_DESC_CERT_LIFETIME,
                                      CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(ip->auth_key_cert);

  if (legacy) {
    ip->legacy.key = crypto_pk_new();
    tt_assert(ip->legacy.key);
    ret = crypto_pk_generate_key(ip->legacy.key);
    tt_int_op(ret, OP_EQ, 0);
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

    if (intro_enc_kp) {
      memcpy(&curve25519_kp, intro_enc_kp, sizeof(curve25519_keypair_t));
    } else {
      ret = curve25519_keypair_generate(&curve25519_kp, 0);
      tt_int_op(ret, OP_EQ, 0);
    }
    ed25519_keypair_from_curve25519_keypair(&ed25519_kp, &signbit,
                                            &curve25519_kp);
    cross_cert = tor_cert_create_ed25519(signing_kp,
                                 CERT_TYPE_CROSS_HS_IP_KEYS,
                                 &ed25519_kp.pubkey, time(NULL),
                                 HS_DESC_CERT_LIFETIME,
                                 CERT_FLAG_INCLUDE_SIGNING_KEY);
    tt_assert(cross_cert);
    ip->enc_key_cert = cross_cert;
    memcpy(ip->enc_key.public_key, curve25519_kp.pubkey.public_key,
           CURVE25519_PUBKEY_LEN);
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
    tor_cert_create_ed25519(&blinded_kp, CERT_TYPE_SIGNING_HS_DESC,
                    &signing_kp->pubkey, now, 3600,
                    CERT_FLAG_INCLUDE_SIGNING_KEY);
  tt_assert(desc->plaintext_data.signing_key_cert);
  desc->plaintext_data.revision_counter = 42;
  desc->plaintext_data.lifetime_sec = 3 * 60 * 60;

  hs_get_subcredential(&signing_kp->pubkey, &blinded_kp.pubkey,
                    &desc->subcredential);

  /* Setup superencrypted data section. */
  ret = curve25519_keypair_generate(&auth_ephemeral_kp, 0);
  tt_int_op(ret, OP_EQ, 0);
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
                  hs_helper_build_intro_point(signing_kp, now, "1.2.3.4", 0,
                                              NULL, NULL));
    smartlist_add(desc->encrypted_data.intro_points,
                  hs_helper_build_intro_point(signing_kp, now, "[2600::1]", 0,
                                              NULL, NULL));
    smartlist_add(desc->encrypted_data.intro_points,
                  hs_helper_build_intro_point(signing_kp, now, "3.2.1.4", 1,
                                              NULL, NULL));
    smartlist_add(desc->encrypted_data.intro_points,
                  hs_helper_build_intro_point(signing_kp, now, "5.6.7.8", 1,
                                              NULL, NULL));
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
                                            hs_subcredential_t *subcred_out)
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

hs_descriptor_t *
hs_helper_build_hs_desc_with_client_auth(
                        const uint8_t *descriptor_cookie,
                        const curve25519_public_key_t *client_pk,
                        const ed25519_keypair_t *signing_kp)
{
  curve25519_keypair_t auth_ephemeral_kp;
  hs_descriptor_t *desc = hs_helper_build_hs_desc_impl(0, signing_kp);
  hs_desc_authorized_client_t *desc_client;

  /* The number of client authorized auth has tobe a multiple of
   * HS_DESC_AUTH_CLIENT_MULTIPLE so remove one that we'll replace. */
  desc_client = smartlist_get(desc->superencrypted_data.clients, 0);
  smartlist_remove(desc->superencrypted_data.clients, desc_client);
  hs_desc_authorized_client_free(desc_client);

  desc_client = tor_malloc_zero(sizeof(hs_desc_authorized_client_t));

  curve25519_keypair_generate(&auth_ephemeral_kp, 0);
  memcpy(&desc->superencrypted_data.auth_ephemeral_pubkey,
         &auth_ephemeral_kp.pubkey, sizeof(curve25519_public_key_t));

  hs_desc_build_authorized_client(&desc->subcredential, client_pk,
                                  &auth_ephemeral_kp.seckey,
                                  descriptor_cookie, desc_client);
  smartlist_add(desc->superencrypted_data.clients, desc_client);
  return desc;
}

void
hs_helper_desc_equal(const hs_descriptor_t *desc1,
                     const hs_descriptor_t *desc2)
{
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
  tt_u64_op(desc1->plaintext_data.revision_counter, OP_EQ,
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
    tt_int_op(smartlist_len(desc1->superencrypted_data.clients), OP_EQ,
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
  tt_uint_op(desc1->encrypted_data.create2_ntor, OP_EQ,
             desc2->encrypted_data.create2_ntor);

  /* Authentication type. */
  tt_int_op(!!desc1->encrypted_data.intro_auth_types, OP_EQ,
            !!desc2->encrypted_data.intro_auth_types);
  if (desc1->encrypted_data.intro_auth_types &&
      desc2->encrypted_data.intro_auth_types) {
    tt_int_op(smartlist_len(desc1->encrypted_data.intro_auth_types), OP_EQ,
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
    tt_int_op(smartlist_len(desc1->encrypted_data.intro_points), OP_EQ,
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

      tt_int_op(smartlist_len(ip1->link_specifiers), OP_EQ,
                smartlist_len(ip2->link_specifiers));
      for (int j = 0; j < smartlist_len(ip1->link_specifiers); j++) {
        link_specifier_t *ls1 = smartlist_get(ip1->link_specifiers, j),
                         *ls2 = smartlist_get(ip2->link_specifiers, j);
        tt_int_op(link_specifier_get_ls_type(ls1), OP_EQ,
                  link_specifier_get_ls_type(ls2));
        switch (link_specifier_get_ls_type(ls1)) {
          case LS_IPV4:
            {
              uint32_t addr1 = link_specifier_get_un_ipv4_addr(ls1);
              uint32_t addr2 = link_specifier_get_un_ipv4_addr(ls2);
              tt_int_op(addr1, OP_EQ, addr2);
              uint16_t port1 = link_specifier_get_un_ipv4_port(ls1);
              uint16_t port2 = link_specifier_get_un_ipv4_port(ls2);
              tt_int_op(port1, OP_EQ, port2);
            }
            break;
          case LS_IPV6:
            {
              const uint8_t *addr1 =
                link_specifier_getconstarray_un_ipv6_addr(ls1);
              const uint8_t *addr2 =
                link_specifier_getconstarray_un_ipv6_addr(ls2);
              tt_int_op(link_specifier_getlen_un_ipv6_addr(ls1), OP_EQ,
                        link_specifier_getlen_un_ipv6_addr(ls2));
              tt_mem_op(addr1, OP_EQ, addr2,
                        link_specifier_getlen_un_ipv6_addr(ls1));
              uint16_t port1 = link_specifier_get_un_ipv6_port(ls1);
              uint16_t port2 = link_specifier_get_un_ipv6_port(ls2);
              tt_int_op(port1, OP_EQ, port2);
            }
            break;
          case LS_LEGACY_ID:
            {
              const uint8_t *id1 =
                link_specifier_getconstarray_un_legacy_id(ls1);
              const uint8_t *id2 =
                link_specifier_getconstarray_un_legacy_id(ls2);
              tt_int_op(link_specifier_getlen_un_legacy_id(ls1), OP_EQ,
                        link_specifier_getlen_un_legacy_id(ls2));
              tt_mem_op(id1, OP_EQ, id2,
                        link_specifier_getlen_un_legacy_id(ls1));
            }
            break;
          default:
            /* Unknown type, caught it and print its value. */
            tt_int_op(link_specifier_get_ls_type(ls1), OP_EQ, -1);
        }
      }
    }
  }

 done:
  ;
}

void
hs_helper_add_client_auth(const ed25519_public_key_t *service_pk,
                          const curve25519_secret_key_t *client_sk)
{
  digest256map_t *client_auths = get_hs_client_auths_map();
  if (client_auths == NULL) {
    client_auths = digest256map_new();
    set_hs_client_auths_map(client_auths);
  }

  hs_client_service_authorization_t *auth =
    tor_malloc_zero(sizeof(hs_client_service_authorization_t));
  memcpy(&auth->enc_seckey, client_sk, sizeof(curve25519_secret_key_t));
  hs_build_address(service_pk, HS_VERSION_THREE, auth->onion_address);
  digest256map_set(client_auths, service_pk->pubkey, auth);
}
