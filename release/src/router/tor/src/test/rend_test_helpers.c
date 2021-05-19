/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "core/or/extendinfo.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "test/test.h"
#include "feature/rend/rendcommon.h"
#include "test/rend_test_helpers.h"

#include "core/or/extend_info_st.h"
#include "feature/rend/rend_intro_point_st.h"
#include "feature/rend/rend_service_descriptor_st.h"

void
generate_desc(int time_diff, rend_encoded_v2_service_descriptor_t **desc,
              char **service_id, int intro_points)
{
  rend_service_descriptor_t *generated = NULL;
  smartlist_t *descs = smartlist_new();
  time_t now;

  now = time(NULL) + time_diff;
  create_descriptor(&generated, service_id, intro_points);
  generated->timestamp = now;

  rend_encode_v2_descriptors(descs, generated, now, 0, REND_NO_AUTH, NULL,
                             NULL);
  tor_assert(smartlist_len(descs) > 1);
  *desc = smartlist_get(descs, 0);
  smartlist_set(descs, 0, NULL);

  SMARTLIST_FOREACH(descs, rend_encoded_v2_service_descriptor_t *, d,
                    rend_encoded_v2_service_descriptor_free(d));
  smartlist_free(descs);
  rend_service_descriptor_free(generated);
}

void
create_descriptor(rend_service_descriptor_t **generated, char **service_id,
                  int intro_points)
{
  crypto_pk_t *pk1 = NULL;
  crypto_pk_t *pk2 = NULL;
  int i;

  *service_id = tor_malloc(REND_SERVICE_ID_LEN_BASE32+1);
  pk1 = pk_generate(0);
  pk2 = pk_generate(1);

  *generated = tor_malloc_zero(sizeof(rend_service_descriptor_t));
  (*generated)->pk = crypto_pk_dup_key(pk1);
  rend_get_service_id((*generated)->pk, *service_id);

  (*generated)->version = 2;
  (*generated)->protocols = 42;
  (*generated)->intro_nodes = smartlist_new();

  for (i = 0; i < intro_points; i++) {
    rend_intro_point_t *intro = tor_malloc_zero(sizeof(rend_intro_point_t));
    crypto_pk_t *okey = pk_generate(2 + i);
    intro->extend_info =
      extend_info_new(NULL, NULL, NULL, NULL, NULL, NULL, 0);
    intro->extend_info->onion_key = okey;
    crypto_pk_get_digest(intro->extend_info->onion_key,
                         intro->extend_info->identity_digest);
    intro->extend_info->nickname[0] = '$';
    base16_encode(intro->extend_info->nickname + 1,
                  sizeof(intro->extend_info->nickname) - 1,
                  intro->extend_info->identity_digest, DIGEST_LEN);
    tor_addr_t addr;
    uint16_t port;
    /* Does not cover all IP addresses. */
    tor_addr_from_ipv4h(&addr, crypto_rand_int(65536) + 1);
    port = 1 + crypto_rand_int(65535);
    extend_info_add_orport(intro->extend_info, &addr, port);
    intro->intro_key = crypto_pk_dup_key(pk2);
    smartlist_add((*generated)->intro_nodes, intro);
  }

  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

rend_data_t *
mock_rend_data(const char *onion_address)
{
  rend_data_v2_t *v2_data = tor_malloc_zero(sizeof(*v2_data));
  rend_data_t *rend_query = &v2_data->base_;
  rend_query->version = 2;

  strlcpy(v2_data->onion_address, onion_address,
          sizeof(v2_data->onion_address));
  v2_data->auth_type = REND_NO_AUTH;
  rend_query->hsdirs_fp = smartlist_new();
  smartlist_add(rend_query->hsdirs_fp, tor_memdup("aaaaaaaaaaaaaaaaaaaaaaaa",
                                                 DIGEST_LEN));
  return rend_query;
}
