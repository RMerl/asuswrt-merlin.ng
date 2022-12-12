#include "testutils.h"
#include "nettle-meta.h"

const char* macs[] = {
  "cmac_aes128",
  "cmac_aes256",
  "cmac_des3",
  "hmac_md5",
  "hmac_ripemd160",
  "hmac_sha1",
  "hmac_sha224",
  "hmac_sha256",
  "hmac_sha384",
  "hmac_sha512",
  "hmac_streebog256",
  "hmac_streebog512",
  "hmac_sm3",
};

void
test_main(void)
{
  int i, j;
  int count = sizeof(macs)/sizeof(*macs);
  for (i = 0; i < count; i++) {
    for (j = 0; NULL != nettle_macs[j]; j++) {
      if (0 == strcmp(macs[i], nettle_macs[j]->name))
	break;
    }
    ASSERT(NULL != nettle_macs[j]); /* make sure we found a matching mac */
  }
  i = 0;
  while (NULL != nettle_macs[i])
    i++;
  ASSERT(i == count); /* we are not missing testing any macs */
}
