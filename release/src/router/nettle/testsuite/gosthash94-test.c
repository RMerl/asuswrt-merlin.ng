#include "testutils.h"
#include "gosthash94.h"

/* Using test vectors from Wikipedia article on GOST */

void
test_main(void)
{
  test_hash(&nettle_gosthash94, SDATA("The quick brown fox jumps over the lazy dog"),
	    SHEX("77b7fa410c9ac58a25f49bca7d0468c9296529315eaca76bd1a10f376d1f4294"));

  test_hash(&nettle_gosthash94, SDATA("message digest"),
	    SHEX("ad4434ecb18f2c99b60cbe59ec3d2469582b65273f48de72db2fde16a4889a4d"));

  test_hash(&nettle_gosthash94, SDATA("a"),
	    SHEX("d42c539e367c66e9c88a801f6649349c21871b4344c6a573f849fdce62f314dd"));

  test_hash(&nettle_gosthash94, SDATA(""),
	    SHEX("ce85b99cc46752fffee35cab9a7b0278abb4c2d2055cff685af4912c49490f8d"));

  test_hash(&nettle_gosthash94cp, SDATA("The quick brown fox jumps over the lazy dog"),
	    SHEX("9004294a361a508c586fe53d1f1b02746765e71b765472786e4770d565830a76"));

  test_hash(&nettle_gosthash94cp, SDATA("message digest"),
	    SHEX("bc6041dd2aa401ebfa6e9886734174febdb4729aa972d60f549ac39b29721ba0"));

  test_hash(&nettle_gosthash94cp, SDATA("a"),
	    SHEX("e74c52dd282183bf37af0079c9f78055715a103f17e3133ceff1aacf2f403011"));

  test_hash(&nettle_gosthash94cp, SDATA(""),
	    SHEX("981e5f3ca30c841487830f84fb433e13ac1101569b9c13584ac483234cd656c0"));
}
