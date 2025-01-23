/* keygrip.c - verifies that keygrips are calculated as expected
 *	Copyright (C) 2005 Free Software Foundation, Inc.
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define PGM "keygrip"
#include "t-common.h"

static int repetitions;

/* Whether fips mode was active at startup.  */
static int in_fips_mode;



static void
print_hex (const char *text, const void *buf, size_t n)
{
  const unsigned char *p = buf;

  fputs (text, stdout);
  for (; n; n--, p++)
    printf ("%02X", *p);
  putchar ('\n');
}




static struct
{
  int algo;
  const char *key;
  const unsigned char grip[20];
  int skip_when_fips;
} key_grips[] =
  {
    {
      GCRY_PK_RSA,
      "(private-key"
      " (rsa"
      "  (n #00B6B509596A9ECABC939212F891E656A626BA07DA8521A9CAD4C08E640C04052FBB87F424EF1A0275A48A9299AC9DB69ABE3D0124E6C756B1F7DFB9B842D6251AEA6EE85390495CADA73D671537FCE5850A932F32BAB60AB1AC1F852C1F83C625E7A7D70CDA9EF16D5C8E47739D77DF59261ABE8454807FF441E143FBD37F8545#)"
      "  (e #010001#)"
      "  (d #077AD3DE284245F4806A1B82B79E616FBDE821C82D691A65665E57B5FAD3F34E67F401E7BD2E28699E89D9C496CF821945AE83AC7A1231176A196BA6027E77D85789055D50404A7A2A95B1512F91F190BBAEF730ED550D227D512F89C0CDB31AC06FA9A19503DDF6B66D0B42B9691BFD6140EC1720FFC48AE00C34796DC899E5#)"
      "  (p #00D586C78E5F1B4BF2E7CD7A04CA091911706F19788B93E44EE20AAF462E8363E98A72253ED845CCBF2481BB351E8557C85BCFFF0DABDBFF8E26A79A0938096F27#)"
      "  (q #00DB0CDF60F26F2A296C88D6BF9F8E5BE45C0DDD713C96CC73EBCB48B061740943F21D2A93D6E42A7211E7F02A95DCED6C390A67AD21ECF739AE8A0CA46FF2EBB3#)"
      "  (u #33149195F16912DB20A48D020DBC3B9E3881B39D722BF79378F6340F43148A6E9FC5F53E2853B7387BA4443BA53A52FCA8173DE6E85B42F9783D4A7817D0680B#)))",
      "\x32\xCF\xFA\x85\xB1\x79\x1F\xBB\x26\x14\xE9\x1A\xFD\xF3\xAF\xE3\x32\x08\x2E\x25"
    },
    {
      GCRY_PK_DSA,
      " (public-key"
      " (dsa"
      "  (p #0084E4C626E16005770BD9509ABF7354492E85B8C0060EFAAAEC617F725B592FAA59DF5460575F41022776A9718CE62EDD542AB73C7720869EBDBC834D174ADCD7136827DF51E2613545A25CA573BC502A61B809000B6E35F5EB7FD6F18C35678C23EA1C3638FB9CFDBA2800EE1B62F41A4479DE824F2834666FBF8DC5B53C2617#)"
      "  (q #00B0E6F710051002A9F425D98A677B18E0E5B038AB#)"
      "  (g #44370CEE0FE8609994183DBFEBA7EEA97D466838BCF65EFF506E35616DA93FA4E572A2F08886B74977BC00CA8CD3DBEA7AEB7DB8CBB180E6975E0D2CA76E023E6DE9F8CCD8826EBA2F72B8516532F6001DEFFAE76AA5E59E0FA33DBA3999B4E92D1703098CDEDCC416CF008801964084CDE1980132B2B78CB4CE9C15A559528B#)"
      "  (y #3D5DD14AFA2BF24A791E285B90232213D0E3BA74AB1109E768AED19639A322F84BB7D959E2BA92EF73DE4C7F381AA9F4053CFA3CD4527EF9043E304E5B95ED0A3A5A9D590AA641C13DB2B6E32B9B964A6A2C730DD3EA7C8E13F7A140AFF1A91CE375E9B9B960384779DC4EA180FA1F827C52288F366C0770A220F50D6D8FD6F6#)))",
      "\x04\xA3\x4F\xA0\x2B\x03\x94\xD7\x32\xAD\xD5\x9B\x50\xAF\xDB\x5D\x57\x22\xA6\x10"

    },
    {
      GCRY_PK_DSA,
      "(private-key"
      " (dsa"
      "  (p #0084E4C626E16005770BD9509ABF7354492E85B8C0060EFAAAEC617F725B592FAA59DF5460575F41022776A9718CE62EDD542AB73C7720869EBDBC834D174ADCD7136827DF51E2613545A25CA573BC502A61B809000B6E35F5EB7FD6F18C35678C23EA1C3638FB9CFDBA2800EE1B62F41A4479DE824F2834666FBF8DC5B53C2617#)"
      "  (q #00B0E6F710051002A9F425D98A677B18E0E5B038AB#)"
      "  (g #44370CEE0FE8609994183DBFEBA7EEA97D466838BCF65EFF506E35616DA93FA4E572A2F08886B74977BC00CA8CD3DBEA7AEB7DB8CBB180E6975E0D2CA76E023E6DE9F8CCD8826EBA2F72B8516532F6001DEFFAE76AA5E59E0FA33DBA3999B4E92D1703098CDEDCC416CF008801964084CDE1980132B2B78CB4CE9C15A559528B#)"
      "  (y #3D5DD14AFA2BF24A791E285B90232213D0E3BA74AB1109E768AED19639A322F84BB7D959E2BA92EF73DE4C7F381AA9F4053CFA3CD4527EF9043E304E5B95ED0A3A5A9D590AA641C13DB2B6E32B9B964A6A2C730DD3EA7C8E13F7A140AFF1A91CE375E9B9B960384779DC4EA180FA1F827C52288F366C0770A220F50D6D8FD6F6#)"
      "  (x #0087F9E91BFBCC1163DE71ED86D557708E32F8ADDE#)))",
      "\x04\xA3\x4F\xA0\x2B\x03\x94\xD7\x32\xAD\xD5\x9B\x50\xAF\xDB\x5D\x57\x22\xA6\x10"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa(flags param)"
      " (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)"
      " (a #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC#)"
      " (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)"
      " (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)"
      " (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)"
      " (h #000000000000000000000000000000000000000000000000000000000000000001#)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa(flags param)"
      " (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)"
      " (curve \"NIST P-256\")"
      " (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)"
      " (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)"
      " (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)"
      " (h #000000000000000000000000000000000000000000000000000000000000000001#)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa"
      " (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)"
      " (curve \"NIST P-256\")"
      " (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)"
      " (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)"
      " (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)"
      " (h #000000000000000000000000000000000000000000000000000000000000000001#)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa"
      " (curve secp256r1)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve secp256r1)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve brainpoolP256r1)"
      " (q #042ECD8679930BE2DB4AD42B8600BA3F80"
      /*   */"2D4D539BFF2F69B83EC9B7BBAA7F3406"
      /*   */"436DD11A1756AFE56CD93408410FCDA9"
      /*   */"BA95024EB613BD481A14FCFEC27A448A#)))",
      "\x52\xBA\xD4\xB4\xA3\x2D\x32\xA1\xDD\x06"
      "\x5E\x99\x0B\xF1\xAB\xC1\x13\x3D\x84\xD4",
      1
    },
    { /* Compressed form of above.  */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve brainpoolP256r1)"
      " (q #022ECD8679930BE2DB4AD42B8600BA3F80"
      /*   */"2D4D539BFF2F69B83EC9B7BBAA7F3406#)))",
      "\x52\xBA\xD4\xB4\xA3\x2D\x32\xA1\xDD\x06"
      "\x5E\x99\x0B\xF1\xAB\xC1\x13\x3D\x84\xD4",
      1
    },
    {
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve brainpoolP256r1)"
      " (q #045B784CA008EE64AB3D85017EE0D2BE87"
      /*   */"558762C7300E0C8E06B1F9AF7C031458"
      /*   */"9EBBA41915313417BA54218EB0569C59"
      /*   */"0B156C76DBCAB6E84575E6EF68CE7B87#)))",
      "\x99\x38\x6A\x82\x41\x96\x29\x9C\x89\x74"
      "\xD6\xE1\xBF\x43\xAC\x9B\x9A\x12\xE7\x3F",
      1
    },
    { /* Compressed form of above.  */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve brainpoolP256r1)"
      " (q #035B784CA008EE64AB3D85017EE0D2BE87"
      /*   */"558762C7300E0C8E06B1F9AF7C031458#)))",
      "\x99\x38\x6A\x82\x41\x96\x29\x9C\x89\x74"
      "\xD6\xE1\xBF\x43\xAC\x9B\x9A\x12\xE7\x3F",
      1
    },
    { /* Ed25519 standard */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve Ed25519)"
      " (q #04"
      "     1CC662926E7EFF4982B7FB8B928E61CD74CCDD85277CC57196C3AD20B611085F"
      "     47BD24842905C049257673B3F5249524E0A41FAA17B25B818D0F97E625F1A1D0#)"
      "     ))",
      "\x0C\xCA\xB2\xFD\x48\x9A\x33\x40\x2C\xE8"
      "\xE0\x4A\x1F\xB2\x45\xEA\x80\x3D\x0A\xF1",
      1
    },
    { /* Ed25519+EdDSA */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve Ed25519)(flags eddsa)"
      " (q #773E72848C1FD5F9652B29E2E7AF79571A04990E96F2016BF4E0EC1890C2B7DB#)"
      " ))",
      "\x9D\xB6\xC6\x4A\x38\x83\x0F\x49\x60\x70"
      "\x17\x89\x47\x55\x20\xBE\x8C\x82\x1F\x47",
      1
    },
    { /* Ed25519+EdDSA (with compression prefix) */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve Ed25519)(flags eddsa)"
      " (q #40"
      "     773E72848C1FD5F9652B29E2E7AF79571A04990E96F2016BF4E0EC1890C2B7DB#)"
      " ))",
      "\x9D\xB6\xC6\x4A\x38\x83\x0F\x49\x60\x70"
      "\x17\x89\x47\x55\x20\xBE\x8C\x82\x1F\x47",
      1
    },
    { /* Ed25519+EdDSA  (same but uncompressed)*/
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve Ed25519)(flags eddsa)"
      " (q #04"
      "     629ad237d1ed04dcd4abe1711dd699a1cf51b1584c4de7a4ef8b8a640180b26f"
      "     5bb7c29018ece0f46b01f2960e99041a5779afe7e2292b65f9d51f8c84723e77#)"
      " ))",
      "\x9D\xB6\xC6\x4A\x38\x83\x0F\x49\x60\x70"
      "\x17\x89\x47\x55\x20\xBE\x8C\x82\x1F\x47",
      1
    },
    { /* Cv25519 */
      GCRY_PK_ECC,
      "(public-key"
      " (ecc"
      " (curve Curve25519)(flags djb-tweak)"
      " (q #40"
      "     918C1733127F6BF2646FAE3D081A18AE77111C903B906310B077505EFFF12740#)"
      " ))",
      "\x0F\x89\xA5\x65\xD3\xEA\x18\x7C\xE8\x39"
      "\x33\x23\x98\xF5\xD4\x80\x67\x7D\xF4\x9C",
      1
    },
    { /* Random key  */
      GCRY_PK_RSA,
      "(shadowed-private-key"
      " (rsa"
      " (n #00B493C79928398DA9D99AC0E949FE6EB62F683CB974FFFBFBC01066F5C9A89B"
      "     D3DC48EAD7C65F36EA943C2B2C865C26C4884FF9EDFDA8C99C855B737D77EEF6"
      "     B85DBC0CCEC0E900C1F89A6893A2A93E8B31028469B6927CEB2F08687E547C68"
      "     6B0A2F7E50A194FF7AB7637E03DE0912EF7F6E5F1EC37625BD1620CCC2E7A564"
      "     31E168CDAFBD1D9E61AE47A69A6FA03EF22F844528A710B2392F262B95A3078C"
      "     F321DC8325F92A5691EF69F34FD0DE0B22C79D29DC87723FCADE463829E8E5F7"
      "     D196D73D6C9C180F6A6A0DDBF7B9D8F7FA293C36163B12199EF6A1A95CAE4051"
      "     E3069C522CC6C4A7110F663A5DAD20F66C13A1674D050088208FAE4F33B3AB51"
      "     03#)"
      " (e #00010001#)"
      " (shadowed t1-v1"
      " (#D2760001240102000005000123350000# OPENPGP.1)"
      ")))",
      "\xE5\x6E\xE6\xEE\x5A\x2F\xDC\x3E\x98\x9D"
      "\xFE\x49\xDA\xF5\x67\x43\xE3\x27\x28\x33"
    }
  };


static void
check (void)
{
  unsigned char buf[20];
  unsigned char *ret;
  gcry_error_t err;
  gcry_sexp_t sexp;
  unsigned int i;
  int repn;

  for (i = 0; i < (sizeof (key_grips) / sizeof (*key_grips)); i++)
    {
      if (in_fips_mode && key_grips[i].skip_when_fips)
        continue;

      if (gcry_pk_test_algo (key_grips[i].algo))
        {
          if (verbose)
            fprintf (stderr, "algo %d not available; test skipped\n",
                     key_grips[i].algo);
          continue;
        }
      err = gcry_sexp_sscan (&sexp, NULL, key_grips[i].key,
			     strlen (key_grips[i].key));
      if (err)
        die ("scanning data %d failed: %s\n", i, gpg_strerror (err));

      if (debug)
        info ("check(%d): s-exp='%s'\n", i, key_grips[i].key);

      for (repn=0; repn < repetitions; repn++)
        {
          ret = gcry_pk_get_keygrip (sexp, buf);
          if (!ret)
            die ("gcry_pk_get_keygrip failed for %d\n", i);

          if ( memcmp (key_grips[i].grip, buf, sizeof (buf)) )
            {
              print_hex ("keygrip: ", buf, sizeof buf);
              die ("keygrip for %d does not match\n", i);
            }
          else if (debug && !repn)
            print_hex ("keygrip: ", buf, sizeof buf);
        }

      gcry_sexp_release (sexp);
    }
}



static void
progress_handler (void *cb_data, const char *what, int printchar,
		  int current, int total)
{
  (void)cb_data;
  (void)what;
  (void)current;
  (void)total;

  putchar (printchar);
}

int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    { argc--; argv++; }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = 1;
          debug = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--repetitions"))
        {
          argc--; argv++;
          if (argc)
            {
              repetitions = atoi(*argv);
              argc--; argv++;
            }
        }
    }

  if (repetitions < 1)
    repetitions = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  gcry_set_progress_handler (progress_handler, NULL);

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  check ();

  return 0;
}
