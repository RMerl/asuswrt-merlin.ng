#include "testutils.h"
#include "sm3.h"

void
test_main(void)
{
    /* test vectors from:
     * https://datatracker.ietf.org/doc/html/draft-shen-sm3-hash-01
     */
  test_hash(&nettle_sm3,
            SDATA("abc"),
            SHEX("66c7f0f462eeedd9 d1f2d46bdc10e4e2"
                 "4167c4875cf2f7a2 297da02b8f4ba8e0"));

  test_hash(&nettle_sm3,
            SDATA("abcdabcdabcdabcdabcdabcdabcdabcd"
                  "abcdabcdabcdabcdabcdabcdabcdabcd"),
            SHEX("debe9ff92275b8a1 38604889c18e5a4d"
                 "6fdb70e5387e5765 293dcba39c0c5732"));
}
