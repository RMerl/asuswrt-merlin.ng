/* balloon-test.c

   Copyright (C) 2022 Zoltan Fridrich
   Copyright (C) 2022 Red Hat, Inc.

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#include "testutils.h"
#include "balloon.h"

static void
test_balloon(const struct nettle_hash *alg,
             size_t password_len, const char *password,
             size_t salt_len, const char *salt,
             unsigned s_cost, unsigned t_cost,
             const struct tstring *expected)
{
  void *ctx = xalloc(alg->context_size);
  uint8_t *buf = xalloc(balloon_itch(alg->digest_size, s_cost));

  alg->init(ctx);
  balloon(ctx, alg->update, alg->digest, alg->digest_size,
          s_cost, t_cost, password_len, (const uint8_t *)password,
          salt_len, (const uint8_t *)salt, buf, buf);

  if (!MEMEQ(alg->digest_size, buf, expected->data))
    {
      fprintf(stderr, "test_balloon: result doesn't match the expectation:");
      fprintf(stderr, "\nOutput: ");
      print_hex(alg->digest_size, buf);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(expected);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(ctx);
  free(buf);
}

static void
test_balloon_sha(const struct nettle_hash *alg,
                 size_t password_len, const char *password,
                 size_t salt_len, const char *salt,
                 unsigned s_cost, unsigned t_cost,
                 const struct tstring *expected)
{
  uint8_t *buf = xalloc(balloon_itch(alg->digest_size, s_cost));

  if (alg == &nettle_sha1)
    balloon_sha1(s_cost, t_cost, password_len, (const uint8_t *)password,
                 salt_len, (const uint8_t *)salt, buf, buf);
  else if (alg == &nettle_sha256)
    balloon_sha256(s_cost, t_cost, password_len, (const uint8_t *)password,
                   salt_len, (const uint8_t *)salt, buf, buf);
  else if (alg == &nettle_sha384)
    balloon_sha384(s_cost, t_cost, password_len, (const uint8_t *)password,
                   salt_len, (const uint8_t *)salt, buf, buf);
  else if (alg == &nettle_sha512)
    balloon_sha512(s_cost, t_cost, password_len, (const uint8_t *)password,
                   salt_len, (const uint8_t *)salt, buf, buf);
  else
    {
      fprintf(stderr, "test_balloon_sha: bad test\n");
      FAIL();
    }

  if (!MEMEQ(alg->digest_size, buf, expected->data))
    {
      fprintf(stderr, "test_balloon_sha: result doesn't match the expectation:");
      fprintf(stderr, "\nOutput: ");
      print_hex(alg->digest_size, buf);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(expected);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(buf);
}

/* Test vectors are taken from:
 * <https://github.com/nachonavarro/balloon-hashing>
 * <https://github.com/RustCrypto/password-hashes/tree/master/balloon-hash>
 */
void
test_main(void)
{
  test_balloon(&nettle_sha256, 8, "hunter42", 11, "examplesalt", 1024, 3,
               SHEX("716043dff777b44aa7b88dcbab12c078abecfac9d289c5b5195967aa63440dfb"));
  test_balloon(&nettle_sha256, 0, "", 4, "salt", 3, 3,
               SHEX("5f02f8206f9cd212485c6bdf85527b698956701ad0852106f94b94ee94577378"));
  test_balloon(&nettle_sha256, 8, "password", 0, "", 3, 3,
               SHEX("20aa99d7fe3f4df4bd98c655c5480ec98b143107a331fd491deda885c4d6a6cc"));
  test_balloon(&nettle_sha256, 1, "", 1, "", 3, 3,
               SHEX("4fc7e302ffa29ae0eac31166cee7a552d1d71135f4e0da66486fb68a749b73a4"));
  test_balloon(&nettle_sha256, 8, "password", 4, "salt", 1, 1,
               SHEX("eefda4a8a75b461fa389c1dcfaf3e9dfacbc26f81f22e6f280d15cc18c417545"));

  test_balloon_sha(&nettle_sha1, 8, "password", 4, "salt", 3, 3,
                   SHEX("99393c091fdd3136f85864099ec49a439dcacc21"));
  test_balloon_sha(&nettle_sha256, 8, "password", 4, "salt", 3, 3,
                   SHEX("a4df347f5a312e8b2b14c32164f61a81758c807f1bdcda44f4930e2b80ab2154"));
  test_balloon_sha(&nettle_sha384, 8, "password", 4, "salt", 3, 3,
                   SHEX("78da235f7d0f84aba98b50a432fa6c8f7f3ecb7ea0858cfb316c7e5356aae6c8"
                        "d7e7b3924c54c4ed71a3d0d68cb0ad68"));
  test_balloon_sha(&nettle_sha512, 8, "password", 4, "salt", 3, 3,
                   SHEX("9baf289dfa42990f4b189d96d4ede0f2610ba71fb644169427829d696f6866d8"
                        "7af41eb68f9e14fd4b1f1a7ce4832f1ed6117c16e8eae753f9e1d054a7c0a7eb"));
}
