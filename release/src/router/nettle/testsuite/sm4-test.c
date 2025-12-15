#include "testutils.h"
#include "sm4.h"

void
test_main(void)
{
  /* test vectors from:
   * https://tools.ietf.org/id/draft-ribose-cfrg-sm4-10.html
   */
  test_cipher(&nettle_sm4,
	      SHEX("0123456789ABCDEF FEDCBA9876543210"),
	      SHEX("0123456789ABCDEF FEDCBA9876543210"),
	      SHEX("681EDF34D206965E 86B3E94F536E4246"));

  test_cipher(&nettle_sm4,
	      SHEX("FEDCBA9876543210 0123456789ABCDEF"),
	      SHEX("0001020304050607 08090A0B0C0D0E0F"),
	      SHEX("F766678F13F01ADE AC1B3EA955ADB594"));
}
