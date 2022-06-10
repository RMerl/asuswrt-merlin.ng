/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#define SIGCOMMON_PRIVATE
#include "core/or/or.h"
#include "feature/dirparse/routerparse.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/torcert.h"
#include "feature/keymgt/loadkey.h"
#include "test/fuzz/fuzzing.h"

static int
mock_check_tap_onion_key_crosscert__nocheck(const uint8_t *crosscert,
                                   int crosscert_len,
                                   const crypto_pk_t *onion_pkey,
                                   const ed25519_public_key_t *master_id_pkey,
                                   const uint8_t *rsa_id_digest)
{
  tor_assert(crosscert && onion_pkey && master_id_pkey && rsa_id_digest);
  /* we could look at crosscert[..] */
  (void) crosscert_len;
  return 0;
}

static void
mock_dump_desc__nodump(const char *desc, const char *type)
{
  (void)desc;
  (void)type;
}

static int
mock_router_produce_hash_final__nohash(char *digest,
                                       const char *start, size_t len,
                                       digest_algorithm_t alg)
{
  (void)start;
  (void)len;
  /* we could look at start[..] */
  if (alg == DIGEST_SHA1)
    memset(digest, 0x01, 20);
  else
    memset(digest, 0x02, 32);
  return 0;
}

int
fuzz_init(void)
{
  disable_signature_checking();
  MOCK(check_tap_onion_key_crosscert,
       mock_check_tap_onion_key_crosscert__nocheck);
  MOCK(dump_desc, mock_dump_desc__nodump);
  MOCK(router_compute_hash_final, mock_router_produce_hash_final__nohash);
  ed25519_init();
  return 0;
}

int
fuzz_cleanup(void)
{
  return 0;
}

int
fuzz_main(const uint8_t *data, size_t sz)
{
  routerinfo_t *ri;
  const char *str = (const char*) data;
  ri = router_parse_entry_from_string((const char *)str,
                                      str+sz,
                                      0, 0, 0, NULL);
  if (ri) {
    log_debug(LD_GENERAL, "Parsing okay");
    routerinfo_free(ri);
  } else {
    log_debug(LD_GENERAL, "Parsing failed");
  }
  return 0;
}
