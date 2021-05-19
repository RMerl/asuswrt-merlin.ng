/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#define NS_PARSE_PRIVATE
#define NETWORKSTATUS_PRIVATE
#include "core/or/or.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirparse/ns_parse.h"
#include "feature/dirparse/unparseable.h"
#include "lib/memarea/memarea.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"

#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/vote_routerstatus_st.h"
#include "lib/crypt_ops/crypto_ed25519.h"

#include "test/fuzz/fuzzing.h"

static void
mock_dump_desc__nodump(const char *desc, const char *type)
{
  (void)desc;
  (void)type;
}

static networkstatus_t *dummy_vote = NULL;
static memarea_t *area = NULL;

int
fuzz_init(void)
{
  disable_signature_checking();
  MOCK(dump_desc, mock_dump_desc__nodump);
  ed25519_init();
  area = memarea_new();
  dummy_vote = tor_malloc_zero(sizeof(*dummy_vote));
  dummy_vote->known_flags = smartlist_new();
  smartlist_split_string(dummy_vote->known_flags,
                         DIRVOTE_UNIVERSAL_FLAGS,
                         " ", 0, 0);
  smartlist_split_string(dummy_vote->known_flags,
                         DIRVOTE_OPTIONAL_FLAGS,
                         " ", 0, 0);
  smartlist_sort_strings(dummy_vote->known_flags);
  return 0;
}

int
fuzz_cleanup(void)
{
  SMARTLIST_FOREACH(dummy_vote->known_flags, char *, cp, tor_free(cp));
  smartlist_free(dummy_vote->known_flags);
  tor_free(dummy_vote);
  return 0;
}

int
fuzz_main(const uint8_t *data, size_t sz)
{
  const char *s;
  routerstatus_t *rs_ns = NULL, *rs_md = NULL, *rs_vote = NULL;
  vote_routerstatus_t *vrs = tor_malloc_zero(sizeof(*vrs));
  smartlist_t *tokens = smartlist_new();
  const char *eos = (const char *)data + sz;

  s = (const char *)data;
  rs_ns = routerstatus_parse_entry_from_string(area, &s, eos, tokens,
                                               NULL, NULL, 26, FLAV_NS);
  tor_assert(smartlist_len(tokens) == 0);

  s = (const char *)data;
  rs_md = routerstatus_parse_entry_from_string(area, &s, eos, tokens,
                                               NULL, NULL, 26, FLAV_MICRODESC);
  tor_assert(smartlist_len(tokens) == 0);

  s = (const char *)data;
  rs_vote = routerstatus_parse_entry_from_string(area, &s, eos, tokens,
                                              dummy_vote, vrs, 26, FLAV_NS);
  tor_assert(smartlist_len(tokens) == 0);

  log_debug(LD_GENERAL,
            "ns=%p, md=%p, vote=%p", rs_ns, rs_md, rs_vote);

  routerstatus_free(rs_md);
  routerstatus_free(rs_ns);
  vote_routerstatus_free(vrs);
  memarea_clear(area);
  smartlist_free(tokens);

  return 0;
}
