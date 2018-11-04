#include "testutils.h"
#include "nettle-internal.h"
#include "nettle-meta.h"
/* For NETTLE_MAX_HASH_CONTEXT_SIZE */
#include "sha3.h"

const char* hashes[] = {
  "md2",
  "md4",
  "md5",
  "ripemd160",
  "sha1",
  "sha224",
  "sha256",
  "sha384",
  "sha512",
  "sha3_224",
  "sha3_256",
  "sha3_384",
  "sha3_512",
};

void
test_main(void)
{
  int i;
  int count = sizeof(hashes)/sizeof(*hashes);
  for (i = 0; i < count; i++) {
    /* make sure we found a matching hash */
    ASSERT(nettle_lookup_hash(hashes[i]) != NULL);
  }

  for (i = 0; NULL != nettle_hashes[i]; i++) {
    ASSERT(nettle_hashes[i]->digest_size <= NETTLE_MAX_HASH_DIGEST_SIZE);
    ASSERT(nettle_hashes[i]->context_size <= NETTLE_MAX_HASH_CONTEXT_SIZE);
  }
  ASSERT(i == count); /* we are not missing testing any hashes */
}
