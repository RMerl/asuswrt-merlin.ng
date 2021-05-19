/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#define SIGCOMMON_PRIVATE
#include "core/or/or.h"
#include "feature/dirparse/ns_parse.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/networkstatus.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "feature/nodelist/networkstatus_st.h"
#include "test/fuzz/fuzzing.h"

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

static int
mock_signed_digest_equals__yes(const uint8_t *d1, const uint8_t *d2,
                               size_t len)
{
  (void) tor_memeq(d1, d2, len);
  return 1;
}

int
fuzz_init(void)
{
  disable_signature_checking();
  MOCK(dump_desc, mock_dump_desc__nodump);
  MOCK(router_compute_hash_final, mock_router_produce_hash_final__nohash);
  MOCK(signed_digest_equals, mock_signed_digest_equals__yes);
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
  networkstatus_t *ns;
  const char *eos = NULL;
  networkstatus_type_t tp = NS_TYPE_CONSENSUS;
  if (tor_memstr(data, MIN(sz, 1024), "tus vote"))
    tp = NS_TYPE_VOTE;
  const char *what = (tp == NS_TYPE_CONSENSUS) ? "consensus" : "vote";
  ns = networkstatus_parse_vote_from_string((const char *)data,
                                            sz,
                                            &eos,
                                            tp);
  if (ns) {
    log_debug(LD_GENERAL, "Parsing as %s okay", what);
    networkstatus_vote_free(ns);
  } else {
    log_debug(LD_GENERAL, "Parsing as %s failed", what);
  }

  return 0;
}
