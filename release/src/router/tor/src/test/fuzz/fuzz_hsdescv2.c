/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */
#include "core/or/or.h"
#include "feature/dirparse/unparseable.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendparse.h"
#include "lib/crypt_ops/crypto_ed25519.h"
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
  rend_service_descriptor_t *desc = NULL;
  char desc_id[64];
  char *ipts = NULL;
  size_t ipts_size, esize;
  const char *next;
  char *str = tor_memdup_nulterm(data, sz);
  (void) rend_parse_v2_service_descriptor(&desc, desc_id, &ipts, &ipts_size,
                                          &esize, &next, str, 1);
  if (desc) {
    log_debug(LD_GENERAL, "Parsing okay");
    rend_service_descriptor_free(desc);
  } else {
    log_debug(LD_GENERAL, "Parsing failed");
  }
  tor_free(ipts);
  tor_free(str);
  return 0;
}
