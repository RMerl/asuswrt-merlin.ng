#include "testutils.h"
#include "nettle-meta.h"
#include "nettle-internal.h"

const char* ciphers[] = {
  "aes128",
  "aes192",
  "aes256",
  "arctwo40",
  "arctwo64",
  "arctwo128",
  "arctwo_gutmann128",
  "camellia128",
  "camellia192",
  "camellia256",
  "cast128",
  "serpent128",
  "serpent192",
  "serpent256",
  "twofish128",
  "twofish192",
  "twofish256",
  "sm4"
};

void
test_main(void)
{
  int i,j;
  int count = sizeof(ciphers)/sizeof(*ciphers);
  for (i = 0; i < count; i++) {
    for (j = 0; NULL != nettle_ciphers[j]; j++) {
      if (0 == strcmp(ciphers[i], nettle_ciphers[j]->name))
        break;
    }
    ASSERT(NULL != nettle_ciphers[j]); /* make sure we found a matching cipher */
  }
  j = 0;
  for (j = 0; NULL != nettle_ciphers[j]; j++)
    {
      ASSERT(nettle_ciphers[j]->block_size <= NETTLE_MAX_CIPHER_BLOCK_SIZE);
      ASSERT(nettle_ciphers[j]->key_size <= NETTLE_MAX_CIPHER_KEY_SIZE);
    }
  ASSERT(j == count); /* we are not missing testing any ciphers */
}
  
