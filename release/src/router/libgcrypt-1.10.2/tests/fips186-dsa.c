/* fips186-dsa.c - FIPS 186 DSA tests
 *	Copyright (C) 2008 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _GCRYPT_IN_LIBGCRYPT
# include "../src/gcrypt-int.h"
#else
# include <gcrypt.h>
#endif

#define PGM "fips186-dsa"
#include "t-common.h"

static int in_fips_mode;

static void
show_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  if (prefix)
    fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = gcry_xmalloc (size);

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
  gcry_free (buf);
}

static gcry_mpi_t
mpi_from_string (const char *string)
{
  gpg_error_t err;
  gcry_mpi_t a;

  err = gcry_mpi_scan (&a, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    die ("error converting string to mpi: %s\n", gpg_strerror (err));
  return a;
}

/* Convert STRING consisting of hex characters into its binary
   representation and return it as an allocated buffer. The valid
   length of the buffer is returned at R_LENGTH.  The string is
   delimited by end of string.  The function returns NULL on
   error.  */
static void *
data_from_hex (const char *string, size_t *r_length)
{
  const char *s;
  unsigned char *buffer;
  size_t length;

  buffer = gcry_xmalloc (strlen(string)/2+1);
  length = 0;
  for (s=string; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        die ("error parsing hex string `%s'\n", string);
      ((unsigned char*)buffer)[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}


static void
extract_cmp_mpi (gcry_sexp_t sexp, const char *name, const char *expected)
{
  gcry_sexp_t l1;
  gcry_mpi_t a, b;

  l1 = gcry_sexp_find_token (sexp, name, 0);
  a = gcry_sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
  b = mpi_from_string (expected);
  if (!a)
    fail ("parameter \"%s\" missing in key\n", name);
  else if ( gcry_mpi_cmp (a, b) )
    fail ("parameter \"%s\" does not match expected value\n", name);
  gcry_mpi_release (b);
  gcry_mpi_release (a);
  gcry_sexp_release (l1);
}


static void
extract_cmp_data (gcry_sexp_t sexp, const char *name, const char *expected)
{
  gcry_sexp_t l1;
  const void *a;
  size_t alen;
  void *b;
  size_t blen;

  l1 = gcry_sexp_find_token (sexp, name, 0);
  a = gcry_sexp_nth_data (l1, 1, &alen);
  b = data_from_hex (expected, &blen);
  if (!a)
    fail ("parameter \"%s\" missing in key\n", name);
  else if ( alen != blen || memcmp (a, b, alen) )
    fail ("parameter \"%s\" does not match expected value\n", name);
  gcry_free (b);
  gcry_sexp_release (l1);
}

static void
extract_cmp_int (gcry_sexp_t sexp, const char *name, int expected)
{
  gcry_sexp_t l1;
  char *a;

  l1 = gcry_sexp_find_token (sexp, name, 0);
  a = gcry_sexp_nth_string (l1, 1);
  if (!a)
    fail ("parameter \"%s\" missing in key\n", name);
  else if ( strtoul (a, NULL, 10) != expected )
    fail ("parameter \"%s\" does not match expected value\n", name);
  gcry_free (a);
  gcry_sexp_release (l1);
}


static void
check_dsa_gen_186_2 (void)
{
  static struct {
    int nbits;
    const char *p, *q, *g;
    const char *seed;
    int counter;
    const char *h;
  } tbl[] = {
    /* These tests are from FIPS 186-2, DSAVS B.3.1 PQGGen.rsp.  CAVS 2.2.  */
    {
      1024,
      "d3aed1876054db831d0c1348fbb1ada72507e5fbf9a62cbd47a63aeb7859d6921"
      "4adeb9146a6ec3f43520f0fd8e3125dd8bbc5d87405d1ac5f82073cd762a3f8d7"
      "74322657c9da88a7d2f0e1a9ceb84a39cb40876179e6a76e400498de4bb9379b0"
      "5f5feb7b91eb8fea97ee17a955a0a8a37587a272c4719d6feb6b54ba4ab69",
      "9c916d121de9a03f71fb21bc2e1c0d116f065a4f",
      "8157c5f68ca40b3ded11c353327ab9b8af3e186dd2e8dade98761a0996dda99ab"
      "0250d3409063ad99efae48b10c6ab2bba3ea9a67b12b911a372a2bba260176fad"
      "b4b93247d9712aad13aa70216c55da9858f7a298deb670a403eb1e7c91b847f1e"
      "ccfbd14bd806fd42cf45dbb69cd6d6b43add2a78f7d16928eaa04458dea44",
      "0cb1990c1fd3626055d7a0096f8fa99807399871",
      98,
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000002"
    },
    {
      1024,
      "f5c73304080353357de1b5967597c27d65f70aa2fe9b6aed1d0afc2b499adf22f"
      "8e37937096d88548ac36c4a067f8353c7fed73f96f0d688b19b0624aedbae5dbb"
      "0ee8835a4c269288c0e1d69479e701ee266bb767af39d748fe7d6afc73fdf44be"
      "3eb6e661e599670061203e75fc8b3dbd59e40b54f358d0097013a0f3867f9",
      "f8751166cf4f6f3b07c081fd2a9071f23ca1988d",
      "1e288a442e02461c418ed67a66d24cacbeb8936fbde62ff995f5fd569dee6be62"
      "4e4f0f9f8c8093f5d192ab3b3f9ae3f2665d95d27fb10e382f45cd356e7f4eb7a"
      "665db432113ed06478f93b7cf188ec7a1ee97aec8f91ea7bfceaf8b6e7e5a349c"
      "4ad3225362ef440c57cbc6e69df15b6699caac85f733555075f04781b2b33",
      "34b3520d45d240a8861b82c8b61ffa16e67b5cce",
      622,
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000002",
    },
    {
      1024,
      "c6c6f4f4eed927fb1c3b0c81010967e530658e6f9698ebe058b4f47b2dc8fcbc7"
      "b69296b9e8b6cf55681181fe72492668061b262b0046a0d409902e269b0cb69a4"
      "55ed1a086caf41927f5912bf0e0cbc45ee81a4f98bf6146f6168a228aec80e9cc"
      "1162d6f6aa412efe82d4f18b95e34ab790daac5bd7aef0b22fa08ba5dbaad",
      "d32b29f065c1394a30490b6fcbf812a32a8634ab",
      "06f973c879e2e89345d0ac04f9c34ad69b9eff1680f18d1c8f3e1596c2e8fa8e1"
      "ecef6830409e9012d4788bef6ec7414d09c981b47c941b77f39dfc49caff5e714"
      "c97abe25a7a8b5d1fe88700bb96eff91cca64d53700a28b1146d81bad1212d231"
      "80154c95a01f5aeebb553a8365c38a5ebe05539b51734233776ce9aff98b2",
      "b6ec750da2f824cb42c5f7e28c81350d97f75125",
      185,
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000002",
    },
    {
      1024,
      "b827a9dc9221a6ed1bec7b64d61232aacb2812f888b0a0b3a95033d7a22e77d0b"
      "ff23bfeed0fb1281b21b8ff7421f0c727d1fb8aa2b843d6885f067e763f83d41f"
      "d800ab15a7e2b12f71ec2058ee7bd62cd72c26989b272e519785da57bfa1f974b"
      "c652e1a2d6cfb68477de5635fd019b37add656cff0b802558b31b6d2851e5",
      "de822c03445b77cec4ad3a6fb0ca39ff97059ddf",
      "65a9e2d43a378d7063813104586868cacf2fccd51aec1e0b6af8ba3e66dee6371"
      "681254c3fb5e3929d65e3c4bcd20abd4ddc7cf815623e17b9fc92f02b8d44278b"
      "848480ffd193104cf5612639511e45bd247708ff6028bd3824f8844c263b46c69"
      "1f2076f8cd13c5d0be95f1f2a1a17ab1f7e5bc73500bac27d57b473ba9748",
      "cd2221dd73815a75224e9fde7faf52829b81ac7a",
      62,
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000002",
    },
    {
      1024,
      "898a8d93e295c8ef2ffd46976225a1543640640d155a576fafa0be32136165803"
      "ba2eff2782a2be75cc9ec65db6bd3238cca695b3a5a14726a2a314775c377d891"
      "354b3de6c89e714a05599ca04132c987f889f72c4fe298ccb31f711c03b07e1d9"
      "8d72af590754cf3847398b60cecd55a4611692b308809560a83880404c227",
      "c6d786643d2acfc6b8d576863fda8cfbfbd5e03f",
      "2fd38b8d21c58e8fb5315a177b8d5dc4c450d574e69348b7b9da367c26e72438d"
      "af8372e7f0bee84ef5dcbbc3727194a2228431192f1779be24837f22a0e14d10d"
      "5344da1b8b403df9f9b2655095b3d0f67418ed6cd989f35aa4232e4b7001764fb"
      "e85d6b2c716980f13272fc4271ac1e234f7e24c023cfc2d2dc0aa1e9af2fb",
      "73483e697599871af983a281e3afa22e0ed86b68",
      272,
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000002",
    },

    /* These tests are generated by the OpenSSL FIPS version.  */
    {
      1024,
      "A404363903FDCE86839BCFD953AAD2DA2B0E70CAED3B5FF5D68F15A1C4BB0A793C"
      "A9D58FC956804C5901DE0AF99F345ED1A8617C687864BAC044B7C3C3E732A2B255"
      "EC986AA76EA8CB0E0815B3E0E605650AF7D8058EE7E8EBCDEFFDAB8100D3FC1033"
      "11BA3AB232EF06BB74BA9A949EC0C7ED324C19B202F4AB725BBB4080C9",
      "C643946CEA8748E12D430C48DB038F9165814389",
      "59B7E7BA0033CCE8E6837173420FBB382A784D4154A3C166043F5A68CB92945D16"
      "892D4CC5585F2D28C780E75A6C20A379E2B58304C1E5FC0D8C15E4E89C4498C8BC"
      "B90FB36ED8DC0489B9D0BC09EC4411FB0BFADF25485EEAB6700BE0ACF5C44A6ED7"
      "44A015382FF9B8DA7EAA00DEA135FADC59212DBBFFC1537336FA4B7225",
      "02708ab36e3f0bfd67ec3b8bd8829d03b84f56bd",
      50,
      "02"
    },
    {
      1024,
      "9C664033DB8B203D826F896D2293C62EF9351D5CFD0F4C0AD7EFDA4DDC7F15987"
      "6A3C68CAB2586B44FD1BD4DEF7A17905D88D321DD77C4E1720D848CA21D79F9B3"
      "D8F537338E09B44E9F481E8DA3C56569F63146596A050EF8FAEE8ACA32C666450"
      "04F675C8806EB4025B0A5ECC39CE89983EA40A183A7CF5208BA958045ABD5",
      "AD0D8CBA369AF6CD0D2BAC0B4CFCAF0A1F9BCDF7",
      "74D717F7092A2AF725FDD6C2561D1DBE5AEE40203C638BA8B9F49003857873701"
      "95A44E515C4E8B344F5CDC7F4A6D38097CD57675E7643AB9700692C69F0A99B0E"
      "039FDDDFCA8CEB607BDB4ADF2834DE1690F5823FC8199FB8F6F29E5A583B6786A"
      "C14C7E67106C3B30568CBB9383F89287D578159778EB18216799D16D46498",
      "6481a12a50384888ee84b61024f7c9c685d6ac96",
      289,
      "02"
    },
    {
      1024,

      "B0DFB602EB8462B1DC8C2214A52B587D3E6842CCF1C38D0F7C7F967ED30CF6828"
      "1E2675B3BAB594755FB1634E66B4C23936F0725A358F8DFF3C307E2601FD66D63"
      "5B17270450C50BD2BEC29E0E9A471DF1C15B0191517952268A2763D4BD28B8503"
      "B3399686272B76B11227F693D7833105EF70C2289C3194CF4527024B272DF",
      "EA649C04911FAB5A41440287A517EF752A40354B",
      "88C5A4563ECB949763E0B696CD04B21321360F54C0EE7B23E2CEDC30E9E486162"
      "01BFB1619E7C54B653D1F890C50E04B29205F5E3E2F93A13B0751AF25491C5194"
      "93C09DDF6B9C173B3846DFB0E7A5C870BBFC78419260C90E20315410691C8326C"
      "858D7063E7921F3F601158E912C7EE487FF259202BEEB10F6D9E99190F696",
      "5bf9d17bc62fbbf3d569c92bd4505586b2e5ef1a",
      626,
      "02"
    },
    {
      1024,
      "F783C08D7F9463E48BA87893805C4B34B63C85DF7EBDD9EBEE94DB4AF4E4A415C"
      "F0F3793AE55096BA1199598798FA8403B28DED7F7C7AFD54FD535861A0150EF4D"
      "5871465B13837CCF46BEB0A22F8D38DC7D6AE0E14A3845FD0C027CFA97791B977"
      "CE2808BAD9B43CE69390C0F40016056722D82C0D7B1B27413D026A39D7DAD",
      "A40D9EE456AED4C8A653FDB47B6629C0B843FE8F",
      "DF876263E21F263AE6DA57409BD517DCEADB9216048F066D6B58867F8E59A5EEE"
      "700283A946C1455534618979BE6C227673C1B803910262BD93BC94D5089850614"
      "F3E29AB64E8C989A7E3E28FE670FFA3EE21DEEEC1AB0B60E1D8E2AA39663BADD7"
      "2C9F957D7F3D4F17D9FDAD050EB373A6DEFD09F5DA752EAFE046836E14B67",
      "8a9a57706f69f4f566252cdf6d5cbfdf2020150b",
      397,
      "02"
    },
    {
      1024,
      "D40E4F6461E145859CCF60FD57962840BD75FFF12C22F76626F566842252AD068"
      "29745F0147056354F6C016CF12762B0E331787925B8128CF5AF81F9B176A51934"
      "96D792430FF83C7B79BD595BDA10787B34600787FA552EFE3662F37B99AAD3F3A"
      "093732680A01345192A19BECCE6BF5D498E44ED6BED5B0BA72AAD49E8276B",
      "D12F1BD0AA78B99247FD9F18EAFEE5C136686EA5",
      "468EBD20C99449C1E440E6F8E452C6A6BC7551C555FE5E94996E20CFD4DA3B9CC"
      "58499D6CC2374CCF9C392715A537DE10CFCA8A6A37AFBD187CF6B88D26881E5F5"
      "7521D9D2C9BBA51E7B87B070BBE73F5C5FE31E752CAF88183516D8503BAAC1159"
      "928EF50DEE52D96F396B93FB4138D786464C315401A853E57C9A0F9D25839",
      "30b3599944a914a330a3f49d11ec88f555422aef",
      678,
      "02"
    }
  };
  gpg_error_t err;
  int tno;
  gcry_sexp_t key_spec, key, pub_key, sec_key, seed_values;
  gcry_sexp_t l1;

  for (tno = 0; tno < DIM (tbl); tno++)
    {
      if (verbose)
        info ("generating FIPS 186-2 test key %d\n", tno);

      {
        void *data;
        size_t datalen;

        data = data_from_hex (tbl[tno].seed, &datalen);
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (dsa (nbits %d)(use-fips186-2)"
                               "(derive-parms(seed %b))))",
                               tbl[tno].nbits, (int)datalen, data);
        gcry_free (data);
      }
      if (err)
        die ("error creating S-expression %d: %s\n", tno, gpg_strerror (err));

      err = gcry_pk_genkey (&key, key_spec);
      gcry_sexp_release (key_spec);
      if (err)
        {
          if (in_fips_mode)
            {
              if (verbose > 1)
                fprintf (stderr, "DSA keys are not available in FIPS mode");
            }
          else
            fail ("error generating key %d: %s\n", tno, gpg_strerror (err));
          continue;
        }

      if (verbose > 1)
        show_sexp ("generated key:\n", key);

      pub_key = gcry_sexp_find_token (key, "public-key", 0);
      if (!pub_key)
        fail ("public part missing in key %d\n", tno);

      sec_key = gcry_sexp_find_token (key, "private-key", 0);
      if (!sec_key)
        fail ("private part missing in key %d\n", tno);

      l1 = gcry_sexp_find_token (key, "misc-key-info", 0);
      if (!l1)
        fail ("misc_key_info part missing in key %d\n", tno);
      seed_values = gcry_sexp_find_token (l1, "seed-values", 0);
      if (!seed_values)
        fail ("seed-values part missing in key %d\n", tno);
      gcry_sexp_release (l1);

      extract_cmp_mpi (sec_key, "p", tbl[tno].p);
      extract_cmp_mpi (sec_key, "q", tbl[tno].q);
      extract_cmp_mpi (sec_key, "g", tbl[tno].g);

      extract_cmp_data (seed_values, "seed", tbl[tno].seed);
      extract_cmp_int (seed_values, "counter", tbl[tno].counter);
      extract_cmp_mpi (seed_values, "h", tbl[tno].h);

      gcry_sexp_release (seed_values);
      gcry_sexp_release (sec_key);
      gcry_sexp_release (pub_key);
      gcry_sexp_release (key);
    }
}


static void
check_dsa_gen_186_3 (void)
{
  static struct {
    int nbits, qbits;
    const char *p, *q;
    const char *seed;
    int counter;
  } tbl[] = {
    /* These tests are from FIPS 186-3 Test Vectors, PQGGen.rsp.  CAVS 11.1.  */
    {
      2048,
      256,
      "8e2266d5cb5b1e9ad34ac6380e3d166fd4d60dadc6dfa1be8492a5642c91fdf7"
      "e81b9634a4eeff59e7e93b1b0e8f49ded45a72788866dff71b1329feeb4b6cdb"
      "f2c7166c7cbca20b04300ae127c9940233e891712ac905ed6b43495717a2998e"
      "a8c4eef4ec6c32dc9e774e8e66476f17d9c39abac59e8b583b1107b679e0bed0"
      "78476e933a90cfcf80c89b831c0e054f86eac7ca848e059662d938a4e12947e2"
      "e73b1ffedd7125dd54ba463217abc9c5f3399132aec77b946c806429f6f812c1"
      "0716d57dde7b5d45cb2e5eb6e4dbb81d5a465054fa17e613cbe01afb49ea593f"
      "33f1a696a774941ca1ff6f208577fe529f5e7592f39698c63bf6ae9d56cd2d93",
      "b19c6d094e1210c92910f49aa083957fbe68c0ca4602896f50123fd776786275",
      "f770a4598ff756931fc529764513b103ce57d85f4ad8c5cf297c9b4d48241c5b",
      105
    },
    {
      2048,
      256,
      "b636e5970383cecab68840cca8a909a29325c3924e2c187dd034222f9e1a4334"
      "1061ca620f82787bd349fb8f380fc3f0adb84be116c695529114aecee8a0a1b0"
      "9e7ebb6888e6da71f48eefb3e9990e2d7bd36c1aa24fb10e011a193d6b5a1b22"
      "6cf97fab302e237ecb1dc824264dba2e2285930005717c4e9a12cc1a1ac336c2"
      "0619c4d06ec4e1e02e0d1d2d285661a7472d30c4282646506487cbe6a5c988ee"
      "8402d474713a7d8213eeb19a0719996bbfd3835eb8832eead5a3a340e61c52f0"
      "0dde1c98655a13839ad215d8f43c8e482317af8b086c3d555fc8dbb2f595f256"
      "3520a0c6387661774e1e6ca5fe2626b26a2c4f99b7aff043a091434dfd3275b7",
      "fe9f06fa1901182ab00bf063bff8fd4f736922ce830fd50fee47ebbd21e291e9",
      "3a66a430f23374ce3d2e758881c411c23dad4a8cd6ad697056d24b8cfcc8c353",
      720
    },
    {
      2048,
      256,
      "8d636640981c2ce1935bd16ad3aa3ce2a6efa26f23f07ceda92766f80e82fa03"
      "5c6cf44dc41e08fea242c5cd5846d839bdf0c11d3a29ebaca00aad844cd33a80"
      "448f1f96cebe66b9963f7e3b5c976e29dc430bc5ddf5d2c198eb736339adc14d"
      "5c8a3d22533d7c6a861b6a8b31c55e46804e4c2f95e2e9cc2bbb23bbc833995a"
      "7afe619127d28fa53b0712b17da4786f9116cc39e2c6254845e85513c220e368"
      "fe9c92bc71eabfa831062f01e66e8a970f043112ca0af175f64d13fcff2f087f"
      "ff9198a9fe9732001ab49b2a48d0e39f99d036698703aa853ac02c65f3d55993"
      "5a72c8bbc6ab2fa59ff9a2fcd837a4675229abed23d42badc12a60b34a3bf0f5",
      "b5f3c535e7f48d3251d353b73b3a05c4bdb4591a8c2f2ba4a6a945a889f5aeff",
      "77eb88f087bfbbc312bca7572bafd36f2a7aca2e4d7378dd923b0b277f3d730f",
      137
    },
    {
      2048,
      256,
      "8fa95228b848a9533375e4789c88bb7df505c4478ed3c79545c5d2b04f0e0efb"
      "ac8d3f603603a48b203e1cc67ded22b840ac21bc41b7ab78c73a9cd0773148ca"
      "7c87a5a51564164f683e8f8a77b97cf7d91f989aa3668819bca8f54e0ec8f10c"
      "78ecd26982048cf0ab0446a6de154bbed8891be916627d470061811caf51bef1"
      "b5be8ef2b560cf981c2a097b3769bed61d6ee9b66221e956fe2c49f1809a2d5f"
      "6996be7b39f41afea5184a73c049f3abbd28fddbf37bcae6c4aa4a7255464c2e"
      "ee915c44b8d90d76e5d9e3d8e6cf4ac7c5d9436d19ccc27c5bc1b65dbb56723b"
      "5e77624489652313f9da2ce38554401fdbd61c78f2a4fa69bcc2f5aaffbfed2d",
      "ed3f52bce81572d126b27fb1e9c02346ae523532af82b79943565593d6f46d45",
      "e0ed96bf5e7d78754b5095ed766a1bbc4338eaa8f3d00e9906ef51a8798bc1c2",
      40
    },
    {
      2048,
      256,
      "a80f2481a814d07eb47a7c67e24bc3f8f1ccebc6cf684a0bc9fbb0054cc24cef"
      "24872315b566630d5147184980b4bce3f0849660c84b22dfacb785446c0f6314"
      "b7a53a92cf821bcceb325e03dc9e404832146d34ff8a9b112ed0e69efe69c619"
      "5de03373e590eba88fc5b9d337d6566dc7e82e326a28343f644779f6784159eb"
      "3d33f2ddf1157a02f2f91d0897a4e8ad53f614186a5fe043187510316904bd95"
      "6966e10735d6ef01c195b7dd7fd245a83c18af7908fef0bced2f454e1954f2a3"
      "2c35658f4e0f5811a3d06c81cca715537debabbbc65ba4dd0e7fb0c08397622f"
      "039a51df69f5b10dda61f57bbb84c55f25eacd0f3d8b40ae016ed0ba856837e7",
      "9e3b5a7939082c95069902d3833df8421871ca2dab8a34f7be6cd39151291d07",
      "c7bb440d973189ca07464b037fd309f68ec38baba390988a2e986ecee281e2f5",
      722
    }
  };
  gpg_error_t err;
  int tno;
  gcry_sexp_t key_spec, key, pub_key, sec_key, seed_values;
  gcry_sexp_t l1;

  for (tno = 0; tno < DIM (tbl); tno++)
    {
      if (verbose)
        info ("generating FIPS 186-3 test key %d\n", tno);

      {
        void *data;
        size_t datalen;

        data = data_from_hex (tbl[tno].seed, &datalen);
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (dsa (nbits %d)(qbits %d)(use-fips186)"
                               "(derive-parms(seed %b))))",
                               tbl[tno].nbits, tbl[tno].qbits,
                               (int)datalen, data);
        gcry_free (data);
      }
      if (err)
        die ("error creating S-expression %d: %s\n", tno, gpg_strerror (err));

      err = gcry_pk_genkey (&key, key_spec);
      gcry_sexp_release (key_spec);
      if (err)
        {
          if (in_fips_mode)
            {
              if (verbose > 1)
                fprintf (stderr, "DSA keys are not available in FIPS mode");
            }
          else
            fail ("error generating key %d: %s\n", tno, gpg_strerror (err));
          continue;
        }

      if (verbose > 1)
        show_sexp ("generated key:\n", key);

      pub_key = gcry_sexp_find_token (key, "public-key", 0);
      if (!pub_key)
        fail ("public part missing in key %d\n", tno);

      sec_key = gcry_sexp_find_token (key, "private-key", 0);
      if (!sec_key)
        fail ("private part missing in key %d\n", tno);

      l1 = gcry_sexp_find_token (key, "misc-key-info", 0);
      if (!l1)
        fail ("misc_key_info part missing in key %d\n", tno);
      seed_values = gcry_sexp_find_token (l1, "seed-values", 0);
      if (!seed_values)
        fail ("seed-values part missing in key %d\n", tno);
      gcry_sexp_release (l1);

      extract_cmp_mpi (sec_key, "p", tbl[tno].p);
      extract_cmp_mpi (sec_key, "q", tbl[tno].q);

      extract_cmp_data (seed_values, "seed", tbl[tno].seed);
      extract_cmp_int (seed_values, "counter", tbl[tno].counter);

      gcry_sexp_release (seed_values);
      gcry_sexp_release (sec_key);
      gcry_sexp_release (pub_key);
      gcry_sexp_release (key);
    }
}


int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    {
      verbose = 2;
      debug = 1;
    }

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  /* No valuable keys are create, so we can speed up our RNG. */
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));


  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  check_dsa_gen_186_2 ();
  check_dsa_gen_186_3 ();


  return error_count ? 1 : 0;
}
