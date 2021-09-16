#include "testutils.h"
#include "hmac.h"
#include "pbkdf2.h"

/* NOTE: The salt argument is expected to expand to length, data */
#define PBKDF2_TEST(ctx, update, digest, size, c, salt, expect)	\
  do {									\
    dk[expect->length] = 17;						\
    PBKDF2 (ctx, update, digest, size, c, salt, expect->length, dk); \
    ASSERT(MEMEQ (expect->length, dk, expect->data));			\
    ASSERT(dk[expect->length] == 17);					\
  } while (0)

#define PBKDF2_HMAC_TEST(f, key, c, salt, expect)			\
  do {									\
    dk[expect->length] = 17;						\
    f (key, c, salt, expect->length, dk);				\
    ASSERT(MEMEQ (expect->length, dk, expect->data));			\
    ASSERT(dk[expect->length] == 17);					\
  } while (0)

/* Streebog test has particularly long testcase */
#define MAX_DKLEN 100

void
test_main (void)
{
  uint8_t dk[MAX_DKLEN + 1];
  struct hmac_sha1_ctx sha1ctx;
  struct hmac_sha256_ctx sha256ctx;
  struct hmac_sha512_ctx sha512ctx;
  struct hmac_gosthash94cp_ctx gosthash94cpctx;
  struct hmac_streebog512_ctx streebog512ctx;
  struct hmac_streebog256_ctx streebog256ctx;

  /* Test vectors for PBKDF2 from RFC 6070. */

  hmac_sha1_set_key (&sha1ctx, LDATA("password"));

  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       1, LDATA("salt"),
	       SHEX("0c60c80f961f0e71f3a9b524af6012062fe037a6"));

  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       2, LDATA("salt"),
	       SHEX("ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957"));

  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       4096, LDATA("salt"),
	       SHEX("4b007901b765489abead49d926f721d065a429c1"));

#if 0				/* too slow */
  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       16777216, LDATA("salt"),
	       SHEX("eefe3d61cd4da4e4e9945b3d6ba2158c2634e984"));
#endif

  hmac_sha1_set_key (&sha1ctx, LDATA("passwordPASSWORDpassword"));

  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       4096, LDATA("saltSALTsaltSALTsaltSALTsaltSALTsalt"),
	       SHEX("3d2eec4fe41c849b80c8d83662c0e44a8b291a964cf2f07038"));

  hmac_sha1_set_key (&sha1ctx, LDATA("pass\0word"));

  PBKDF2_TEST (&sha1ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE,
	       4096, LDATA("sa\0lt"),
	       SHEX("56fa6aa75548099dcc37d7f03425e0c3"));

  /* PBKDF2-HMAC-SHA-256 test vectors confirmed with another
     implementation.  */

  hmac_sha256_set_key (&sha256ctx, LDATA("passwd"));

  PBKDF2_TEST (&sha256ctx, hmac_sha256_update, hmac_sha256_digest,
	       SHA256_DIGEST_SIZE, 1, LDATA("salt"),
	       SHEX("55ac046e56e3089fec1691c22544b605"));

  hmac_sha256_set_key (&sha256ctx, LDATA("Password"));

  PBKDF2_TEST (&sha256ctx, hmac_sha256_update, hmac_sha256_digest,
	       SHA256_DIGEST_SIZE, 80000, LDATA("NaCl"),
	       SHEX("4ddcd8f60b98be21830cee5ef22701f9"));

  /* PBKDF2-HMAC-SHA-512 test vectors confirmed with another
     implementation (python-pbkdf2).

     >>> from pbkdf2 import PBKDF2
     >>> import hmac as HMAC
     >>> from hashlib import sha512 as SHA512
     >>> PBKDF2("password", "salt", 50, macmodule=HMAC, digestmodule=SHA512).read(64).encode('hex')
  */

  hmac_sha512_set_key (&sha512ctx, LDATA("password"));
  PBKDF2_TEST (&sha512ctx, hmac_sha512_update, hmac_sha512_digest,
	       SHA512_DIGEST_SIZE, 1, LDATA("NaCL"),
	       SHEX("73decfa58aa2e84f94771a75736bb88bd3c7b38270cfb50cb390ed78b305656af8148e52452b2216b2b8098b761fc6336060a09f76415e9f71ea47f9e9064306"));

  hmac_sha512_set_key (&sha512ctx, LDATA("pass\0word"));
  PBKDF2_TEST (&sha512ctx, hmac_sha512_update, hmac_sha512_digest,
	       SHA512_DIGEST_SIZE, 1, LDATA("sa\0lt"),
	       SHEX("71a0ec842abd5c678bcfd145f09d83522f93361560563c4d0d63b88329871090e76604a49af08fe7c9f57156c8790996b20f06bc535e5ab5440df7e878296fa7"));

  hmac_sha512_set_key (&sha512ctx, LDATA("passwordPASSWORDpassword"));
  PBKDF2_TEST (&sha512ctx, hmac_sha512_update, hmac_sha512_digest,
	       SHA512_DIGEST_SIZE, 50, LDATA("salt\0\0\0"),
	       SHEX("016871a4c4b75f96857fd2b9f8ca28023b30ee2a39f5adcac8c9375f9bda1ccd1b6f0b2fc3adda505412e79d890056c62e524c7d51154b1a8534575bd02dee39"));

  /* Test convenience functions. */

  PBKDF2_HMAC_TEST(pbkdf2_hmac_sha1, LDATA("password"), 1, LDATA("salt"),
		   SHEX("0c60c80f961f0e71f3a9b524af6012062fe037a6"));

  PBKDF2_HMAC_TEST(pbkdf2_hmac_sha256, LDATA("passwd"), 1, LDATA("salt"),
		   SHEX("55ac046e56e3089fec1691c22544b605"));

  PBKDF2_HMAC_TEST(pbkdf2_hmac_sha384, LDATA("passwd"), 1, LDATA("salt"),
		   SHEX("cd3443723a41cf1460cca9efeede428a"));

  PBKDF2_HMAC_TEST(pbkdf2_hmac_sha512, LDATA("passwd"), 1, LDATA("salt"),
		   SHEX("c74319d99499fc3e9013acff597c23c5"));

  /* From TC26 document, MR 26.2.001-2012 */

  hmac_gosthash94cp_set_key (&gosthash94cpctx, LDATA("password"));
  PBKDF2_TEST (&gosthash94cpctx, hmac_gosthash94cp_update, hmac_gosthash94cp_digest,
	       GOSTHASH94CP_DIGEST_SIZE, 1, LDATA("salt"),
	       SHEX("7314e7c04fb2e662c543674253f68bd0b73445d07f241bed872882da21662d58"));

  PBKDF2_TEST (&gosthash94cpctx, hmac_gosthash94cp_update, hmac_gosthash94cp_digest,
	       GOSTHASH94CP_DIGEST_SIZE, 4096, LDATA("salt"),
	       SHEX("1f1829a94bdff5be10d0aeb36af498e7a97467f3b31116a5a7c1afff9deadafe"));

  hmac_gosthash94cp_set_key (&gosthash94cpctx, LDATA("passwordPASSWORDpassword"));
  PBKDF2_TEST (&gosthash94cpctx, hmac_gosthash94cp_update, hmac_gosthash94cp_digest,
	       GOSTHASH94CP_DIGEST_SIZE, 4096, LDATA("saltSALTsaltSALTsaltSALTsaltSALTsalt"),
	       SHEX("788358c69cb2dbe251a7bb17d5f4241f265a792a35becde8d56f326b49c85047b7638acb4764b1fd"));

  hmac_gosthash94cp_set_key (&gosthash94cpctx, LDATA("pass\0word"));
  PBKDF2_TEST (&gosthash94cpctx, hmac_gosthash94cp_update, hmac_gosthash94cp_digest,
	       GOSTHASH94CP_DIGEST_SIZE, 4096, LDATA("sa\0lt"),
	       SHEX("43e06c5590b08c0225242373127edf9c8e9c3291"));

  PBKDF2_HMAC_TEST (pbkdf2_hmac_gosthash94cp, LDATA("password"), 1, LDATA("salt"),
	       SHEX("7314e7c04fb2e662c543674253f68bd0b73445d07f241bed872882da21662d58"));

  /* From TC26 document R 50.1.111-2016 */
  hmac_streebog512_set_key (&streebog512ctx, LDATA("password"));
  PBKDF2_TEST (&streebog512ctx, hmac_streebog512_update, hmac_streebog512_digest,
	       STREEBOG512_DIGEST_SIZE, 1, LDATA("salt"),
	       SHEX("64770af7f748c3b1c9ac831dbcfd85c26111b30a8a657ddc3056b80ca73e040d2854fd36811f6d825cc4ab66ec0a68a490a9e5cf5156b3a2b7eecddbf9a16b47"));
  PBKDF2_TEST (&streebog512ctx, hmac_streebog512_update, hmac_streebog512_digest,
	       STREEBOG512_DIGEST_SIZE, 4096, LDATA("salt"),
	       SHEX("e52deb9a2d2aaff4e2ac9d47a41f34c20376591c67807f0477e32549dc341bc7867c09841b6d58e29d0347c996301d55df0d34e47cf68f4e3c2cdaf1d9ab86c3"));

  hmac_streebog512_set_key (&streebog512ctx, LDATA("passwordPASSWORDpassword"));
  PBKDF2_TEST (&streebog512ctx, hmac_streebog512_update, hmac_streebog512_digest,
	       STREEBOG512_DIGEST_SIZE, 4096, LDATA("saltSALTsaltSALTsaltSALTsaltSALTsalt"),
	       SHEX("b2d8f1245fc4d29274802057e4b54e0a0753aa22fc53760b301cf008679e58fe4bee9addcae99ba2b0b20f431a9c5e50f395"
		    "c89387d0945aedeca6eb4015dfc2bd2421ee9bb71183ba882ceebfef259f33f9e27dc6178cb89dc37428cf9cc52a2baa2d3a"));

  hmac_streebog512_set_key (&streebog512ctx, LDATA("pass\0word"));
  PBKDF2_TEST (&streebog512ctx, hmac_streebog512_update, hmac_streebog512_digest,
	       STREEBOG512_DIGEST_SIZE, 4096, LDATA("sa\0lt"),
	       SHEX("50df062885b69801a3c10248eb0a27ab6e522ffeb20c991c660f001475d73a4e167f782c18e97e92976d9c1d970831ea78ccb879f67068cdac1910740844e830"));

  /* Generated */
  hmac_streebog256_set_key (&streebog256ctx, LDATA("password"));
  PBKDF2_TEST (&streebog256ctx, hmac_streebog256_update, hmac_streebog256_digest,
	       STREEBOG256_DIGEST_SIZE, 1, LDATA("salt"),
	       SHEX("d789458d143b9abebc4ef63ca8e576c72b13c7d4289db23fc1e946f84cd605bc"));
}
