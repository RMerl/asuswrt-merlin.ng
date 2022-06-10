/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#define SIGCOMMON_PRIVATE
#include "core/or/or.h"
#include "feature/dirparse/routerparse.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/dirparse/unparseable.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/routerkeys.h"
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

int
fuzz_init(void)
{
  disable_signature_checking();
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
  extrainfo_t *ei;
  const char *str = (const char*) data;
  int again = 0;
  ei = extrainfo_parse_entry_from_string((const char *)str,
                                         str+sz,
                                         0, NULL, &again);
  if (ei) {
    log_debug(LD_GENERAL, "Parsing okay");
    extrainfo_free(ei);
  } else {
    log_debug(LD_GENERAL, "Parsing failed");
  }
  return 0;
}

