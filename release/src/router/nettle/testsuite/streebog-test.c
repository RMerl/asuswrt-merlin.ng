#include "testutils.h"
#include "streebog.h"

void
test_main(void)
{
  /* Using test vectors from the standard itself */

  /* RFC 6986 provides all data in "Integer" big-endian format, while all
   * constructs expects the data in little-endian format. Thus these examples
   * (as the rest of the code) has data buffers reversed compared to the RFC
   * 6986. */
  /* 10.1.1 */
  test_hash(&nettle_streebog512,
            SDATA("012345678901234567890123456789012345678901234567890123456789012"),
            SHEX("1b54d01a4af5b9d5 cc3d86d68d285462"
                 "b19abc2475222f35 c085122be4ba1ffa"
                 "00ad30f8767b3a82 384c6574f024c311"
                 "e2a481332b08ef7f 41797891c1646f48"));

  /* 10.1.2 */
  test_hash(&nettle_streebog256,
            SDATA("012345678901234567890123456789012345678901234567890123456789012"),
            SHEX("9d151eefd8590b89 daa6ba6cb74af927"
                 "5dd051026bb149a4 52fd84e5e57b5500"));

  /* 10.2.1 */
  test_hash(&nettle_streebog512,
            SHEX("d1e520e2e5f2f0e82c20d1f2f0e8e1ee"
                 "e6e820e2edf3f6e82c20e2e5fef2fa20"
                 "f120eceef0ff20f1f2f0e5ebe0ece820"
                 "ede020f5f0e0e1f0fbff20efebfaeafb"
                 "20c8e3eef0e5e2fb"),
            SHEX("1e88e62226bfca6f 9994f1f2d51569e0"
                 "daf8475a3b0fe61a 5300eee46d961376"
                 "035fe83549ada2b8 620fcd7c496ce5b3"
                 "3f0cb9dddc2b6460 143b03dabac9fb28"));

  /* 10.2.2 */
  test_hash(&nettle_streebog256,
            SHEX("d1e520e2e5f2f0e82c20d1f2f0e8e1ee"
                 "e6e820e2edf3f6e82c20e2e5fef2fa20"
                 "f120eceef0ff20f1f2f0e5ebe0ece820"
                 "ede020f5f0e0e1f0fbff20efebfaeafb"
                 "20c8e3eef0e5e2fb"),
            SHEX("9dd2fe4e90409e5d a87f53976d7405b0"
                 "c0cac628fc669a74 1d50063c557e8f50"));

  /* Additional tests to verify long integer addition with carry */
  test_hash(&nettle_streebog512,
	    SHEX("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "16111111111111111111111111111111"
		 "11111111111111111111111111111111"
		 "11111111111111111111111111111111"
		 "11111111111111111111111111111116"),
	    SHEX("8b06f41e59907d9636e892caf5942fcd"
		 "fb71fa31169a5e70f0edb873664df41c"
		 "2cce6e06dc6755d15a61cdeb92bd607c"
		 "c4aaca6732bf3568a23a210dd520fd41"));

  test_hash(&nettle_streebog256,
	    SHEX("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
		 "16111111111111111111111111111111"
		 "11111111111111111111111111111111"
		 "11111111111111111111111111111111"
		 "11111111111111111111111111111116"),
	    SHEX("81bb632fa31fcc38b4c379a662dbc58b"
		 "9bed83f50d3a1b2ce7271ab02d25babb"));

  test_hash(&nettle_streebog512,
	    SHEX("ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"
		 "ffffffffffffffffffffffffffffffff"),
	    SHEX("90a161d12ad309498d3fe5d48202d8a4"
		 "e9c406d6a264aeab258ac5ecc37a7962"
		 "aaf9587a5abb09b6bb81ec4b3752a3ff"
		 "5a838ef175be5772056bc5fe54fcfc7e"));

}
