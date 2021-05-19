/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "feature/dirparse/unparseable.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendparse.h"
#include "lib/crypt_ops/crypto_ed25519.h"

#include "feature/rend/rend_service_descriptor_st.h"

#include "test/fuzz/fuzzing.h"

static void
mock_dump_desc__nodump(const char *desc, const char *type)
{
  (void)desc;
  (void)type;
}

int
fuzz_init(void)
{
  disable_signature_checking();
  MOCK(dump_desc, mock_dump_desc__nodump);
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
  rend_service_descriptor_t *desc =
    tor_malloc_zero(sizeof(rend_service_descriptor_t));
  const char *str = (const char*) data;
  int r = rend_parse_introduction_points(desc, str, sz);
  if (r >= 0) {
    log_debug(LD_GENERAL, "Parsing okay: %d", r);
  } else {
    log_debug(LD_GENERAL, "Parsing failed");
  }
  rend_service_descriptor_free(desc);
  return 0;
}
