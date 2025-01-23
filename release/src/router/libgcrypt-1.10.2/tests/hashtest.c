/* hashtest.c - Check the hash functions
 * Copyright (C) 2013 g10 Code GmbH
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
#include <config.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../src/gcrypt-int.h"

#include "stopwatch.h"

#define PGM "hashtest"
#include "t-common.h"

static int missing_test_vectors;

static struct {
  int algo;
  int gigs;
  int bytes;
  const char *hex;
} testvectors[] = {
  { GCRY_MD_SHA1, 256, -64, "92fc51850c7b750e6e774b75f294f6979d4059f0" },
  { GCRY_MD_SHA1, 256,  -1, "4bddeeb4c08683f02d4944d93dbcb02ebab50134" },
  { GCRY_MD_SHA1, 256,  -0, "71b923afde1c8c040884c723a2e3335b333e64c6" },
  { GCRY_MD_SHA1, 256,   1, "2d99f9b5b86e9c9c937104f4242bd6b8bc0927ef" },
  { GCRY_MD_SHA1, 256,  64, "a60dabe8d749f798b7ec3a684cc3eab487451482" },

  { GCRY_MD_SHA224, 256, -64,
    "b5672b54d2480a5688a2dc727a1ad4db7a81ef31ce8999e0bbaeffdc" },
  { GCRY_MD_SHA224, 256,  -1,
    "814ea7159473e6ffc1c64b90026a542e13ac6980f7f3ca3c4582a9b8" },
  { GCRY_MD_SHA224, 256,   0,
    "9ec0e1829455db8650ec7a8b06912196f97a7358bc3a73c79911cd4e" },
  { GCRY_MD_SHA224, 256,   1,
    "e578d5d523320876565bbbc892511a485427caee6dd754d57e3e58c2" },
  { GCRY_MD_SHA224, 256,  64,
    "ff0464df248cd298b63765bc4f87f21e25c93c657fdf3656d3c878e5" },

  { GCRY_MD_SHA256, 256, -64,
    "87a9828d3de78d55d252341db2a622908c4e0ceaee9961ecf9768700fc799ec8" },
  { GCRY_MD_SHA256, 256,  -1,
    "823bf95f64ef04a4a77579c38760b1d401b56bf3a8e664bdf56ca15afb468a03" },
  { GCRY_MD_SHA256, 256,   0,
    "2d0723878cb2c3d5c59dfad910cdb857f4430a6ba2a7d687938d7a20e63dde47" },
  { GCRY_MD_SHA256, 256,   1,
    "5a2e21b1e79cd866acf53a2a18ca76bd4e02c4b01bf4627354171824c812d95f" },
  { GCRY_MD_SHA256, 256,  64,
    "34444808af8e9d995e67f9e155ed94bf55f195a51dc1d8a989e6bcf95511c8a2" },

  { GCRY_MD_SHA512, 256, -64,
    "e01bf8140874bf240e8426cb2bcbc377cbed2e6037334116637149e1cd8cd462"
    "96828b71f32b9f002771d4cb51172ce578b73b7939221e4df655ecd08601e655" },
  { GCRY_MD_SHA512, 256,  -1,
    "4917ff94514b1757705c289fdc3e7d6ffcce5771b20ae237ebc03d2ec9eb435f"
    "b7ce9f0e27272be8cced77a5edae1a01a0ad62b0a44169d88bbee45474a17734" },
  { GCRY_MD_SHA512, 256,   0,
    "1e28e8b3c79f2f47da11f3c0b7da4e7981e7d932db6d17d528a31e191922edda"
    "8fc4bb2df10ea876232db5a1c606bc41886e8b2c570a3e721221f60c8c7dc4ab" },
  { GCRY_MD_SHA512, 256,   1,
    "027d3324dd1cf127770ceb53681f4c70937c9bca4e3acd5fd76cb266c7d4527d"
    "58140290a1822e8d60c4d3ae9725fb923183230d6dfd2d7d73c0d74a4757f34a" },
  { GCRY_MD_SHA512, 256,  64,
    "49920704ea9d6ee19f0742d6c868110fa3eda8ac09f026e9ef22cc731af53020"
    "de40eedef66cb1afd94c61e285fa9327e01336e804903740a9145ab1f065c2d5" },

  { GCRY_MD_SHA3_512, 256, -64,
    "c6e082b3db996dbe5f2c5709818a7f325ef4febd883d7e9c545c06bfa7225198"
    "1ecf40103788913cd5a5bdf13246b952ded6651043684b24197eb23544882a97" },
  { GCRY_MD_SHA3_512, 256,  -1,
    "d7bf28e8216bf7d3d0d3969e34078e94b98598e17b6f21f256379389e4eba8ee"
    "74eb288774797263fec00bdfd357d132cea9e408be36b982f5a60ab56ad01613" },
  { GCRY_MD_SHA3_512, 256,  +0,
    "c1270852ba7b1e1a3eaa777969b8a65be28c3894537c61eb8cd22b1df6af703d"
    "b59939f6adadeb64317faece8167d4817e73daf73e28a5ccd26bebee0a35c322" },
  { GCRY_MD_SHA3_512, 256,  +1,
    "8bdfeb3a1a9a1cdcef21172cbc5bb3b87c0d8f7111df0aaf7f1bc03ad4775bd6"
    "a03e0a875c4e7d02d2230c213562c6a57be28d92eaf6e4bea4bc24690454c8ef" },
  { GCRY_MD_SHA3_512, 256, +64,
    "0c91b91665ceaf7af5102e0ed31aa4f050668ab3c57b1f4763946d567efe66b3"
    "ab9a2016cf238dee5b44eae9f0cdfbf7b7a6eb1e759986273243dc35894706b6" },

  { GCRY_MD_SM3, 256, -64,
    "4ceb893abeb43965d4cac7626da9a4be895585b5b2f16f302626801308b1c02a" },
  { GCRY_MD_SM3, 256, -1,
    "825f01e4f2b6084136abc356fa1b343a9411d844a4dc1474293aad817cd2a48f" },
  { GCRY_MD_SM3, 256, +0,
    "d948a4025ac3ea0aa8989f43203411bd22ad17eaa5fd92ebdf9cabf869f1ba1b" },
  { GCRY_MD_SM3, 256, +1,
    "4f6d0e260299c1f286ef1dbb4638a0770979f266b6c007c55410ee6849cba2a8" },
  { GCRY_MD_SM3, 256, +64,
    "ed34869dbadd62e3bec1f511004d7bbfc9cafa965477cc48843b248293bbe867" },

  { 0 }
};


static void
showhex (const void *buffer, size_t buflen, const char *format, ...)
{
  va_list arg_ptr;
  const unsigned char *s;

  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);

  for (s=buffer; buflen; buflen--, s++)
    fprintf (stderr, "%02x", *s);
  putc ('\n', stderr);
}


static void
show_note (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose && getenv ("srcdir"))
    fputs ("      ", stderr);  /* To align above "PASS: ".  */
  else
    fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
  va_end (arg_ptr);
}

/* Convert STRING consisting of hex characters into its binary
   representation and return it as an allocated buffer. The valid
   length of the buffer is returned at R_LENGTH.  The string is
   delimited by end of string.  The function returns NULL on
   error.  */
static void *
hex2buffer (const char *string, size_t *r_length)
{
  const char *s;
  unsigned char *buffer;
  size_t length;

  buffer = xmalloc (strlen(string)/2+1);
  length = 0;
  for (s=string; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        return NULL;           /* Invalid hex digits. */
      ((unsigned char*)buffer)[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}


static void
run_selftest (int algo)
{
  gpg_error_t err;
  size_t n;

  n = 1;
  err = gcry_md_algo_info (algo, GCRYCTL_SELFTEST, NULL, &n);
  if (err && gpg_err_code (err) != GPG_ERR_NOT_IMPLEMENTED)
    fail ("extended selftest for %s (%d) failed: %s",
          gcry_md_algo_name (algo), algo, gpg_strerror (err));
  else if (err && verbose)
    info ("extended selftest for %s (%d) not implemented",
          gcry_md_algo_name (algo), algo);
  else if (verbose)
    info ("extended selftest for %s (%d) passed",
          gcry_md_algo_name (algo), algo);
}

/* Compare DIGEST of length DIGESTLEN generated using ALGO and GIGS
   plus BYTES with the test vector and print an error message if the
   don't match.  Return 0 on match.  */
static int
cmp_digest (const unsigned char *digest, size_t digestlen,
            int algo, int gigs, int bytes)
{
  int idx;
  unsigned char *tv_digest;
  size_t tv_digestlen = 0;

  for (idx=0; testvectors[idx].algo; idx++)
    {
      if (testvectors[idx].algo == algo
          && testvectors[idx].gigs == gigs
          && testvectors[idx].bytes == bytes)
        break;
    }
  if (!testvectors[idx].algo)
    {
      info ("%d GiB %+3d %-10s warning: %s",
            gigs, bytes, gcry_md_algo_name (algo), "no test vector");
      missing_test_vectors++;
      return 1;
    }

  tv_digest = hex2buffer (testvectors[idx].hex, &tv_digestlen);
  if (tv_digestlen != digestlen) /* Ooops.  */
    {
      fail ("%d GiB %+3d %-10s error: %s",
            gigs, bytes, gcry_md_algo_name (algo), "digest length mismatch");
      xfree (tv_digest);
      return 1;
    }
  if (memcmp (tv_digest, digest, tv_digestlen))
    {
      fail ("%d GiB %+3d %-10s error: %s",
            gigs, bytes, gcry_md_algo_name (algo), "mismatch");
      xfree (tv_digest);
      return 1;
    }
  xfree (tv_digest);

  return 0;
}


static void
run_longtest (int algo, int gigs)
{
  gpg_error_t err;
  gcry_md_hd_t hd;
  gcry_md_hd_t hd_pre = NULL;
  gcry_md_hd_t hd_pre2 = NULL;
  gcry_md_hd_t hd_post = NULL;
  gcry_md_hd_t hd_post2 = NULL;
  char pattern[1024];
  int i, g;
  const unsigned char *digest;
  unsigned int digestlen;

  memset (pattern, 'a', sizeof pattern);

  err = gcry_md_open (&hd, algo, 0);
  if (err)
    {
      fail ("gcry_md_open failed for %s (%d): %s",
            gcry_md_algo_name (algo), algo, gpg_strerror (err));
      return;
    }

  digestlen = gcry_md_get_algo_dlen (algo);


  for (g=0; g < gigs; g++)
    {
      if (g == gigs - 1)
        {
          for (i = 0; i < 1024*1023; i++)
            gcry_md_write (hd, pattern, sizeof pattern);
          for (i = 0; i < 1023; i++)
            gcry_md_write (hd, pattern, sizeof pattern);
          err = gcry_md_copy (&hd_pre, hd);
          if (!err)
            err = gcry_md_copy (&hd_pre2, hd);
          if (err)
            die ("gcry_md_copy failed for %s (%d): %s",
                 gcry_md_algo_name (algo), algo, gpg_strerror (err));
          gcry_md_write (hd, pattern, sizeof pattern);
        }
      else
        {
          for (i = 0; i < 1024*1024; i++)
            gcry_md_write (hd, pattern, sizeof pattern);
        }
      if (g && !(g % 16))
        show_note ("%d GiB so far hashed with %s", g, gcry_md_algo_name (algo));
    }
  if (g >= 16)
    show_note ("%d GiB hashed with %s", g, gcry_md_algo_name (algo));

  err = gcry_md_copy (&hd_post, hd);
  if (err)
    die ("gcry_md_copy failed for %s (%d): %s",
         gcry_md_algo_name (algo), algo, gpg_strerror (err));
  err = gcry_md_copy (&hd_post2, hd);
  if (err)
    die ("gcry_md_copy failed for %s (%d): %s",
         gcry_md_algo_name (algo), algo, gpg_strerror (err));

  gcry_md_write (hd_pre2, pattern, sizeof pattern - 64);
  gcry_md_write (hd_pre, pattern, sizeof pattern - 1);
  gcry_md_write (hd_post, pattern, 1);
  gcry_md_write (hd_post2, pattern, 64);

  digest = gcry_md_read (hd_pre2, algo);
  if (cmp_digest (digest, digestlen, algo, gigs, -64) || verbose)
    showhex (digest, digestlen, "%d GiB %+3d %-10s ",
             gigs, -64, gcry_md_algo_name (algo));
  digest = gcry_md_read (hd_pre, algo);
  if (cmp_digest (digest, digestlen, algo, gigs, -1) || verbose)
    showhex (digest, digestlen, "%d GiB %+3d %-10s ",
             gigs, -1, gcry_md_algo_name (algo));
  digest = gcry_md_read (hd, algo);
  if (cmp_digest (digest, digestlen, algo, gigs, 0) || verbose)
    showhex (digest, digestlen, "%d GiB %+3d %-10s ",
             gigs, 0, gcry_md_algo_name (algo));
  digest = gcry_md_read (hd_post, algo);
  if (cmp_digest (digest, digestlen, algo, gigs, 1) || verbose)
    showhex (digest, digestlen, "%d GiB %+3d %-10s ",
             gigs, 1, gcry_md_algo_name (algo));
  digest = gcry_md_read (hd_post2, algo);
  if (cmp_digest (digest, digestlen, algo, gigs, 64) || verbose)
    showhex (digest, digestlen, "%d GiB %+3d %-10s ",
             gigs, 64, gcry_md_algo_name (algo));

  gcry_md_close (hd);
  gcry_md_close (hd_pre);
  gcry_md_close (hd_pre2);
  gcry_md_close (hd_post);
  gcry_md_close (hd_post2);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  int gigs = 0;
  int algo = 0;
  int idx;

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
      else if (!strcmp (*argv, "--help"))
        {
          fputs ("usage: " PGM " [options] [algos]\n"
                 "Options:\n"
                 "  --verbose       print timings etc.\n"
                 "  --debug         flyswatter\n"
                 "  --gigs N        Run a test on N GiB\n",
                 stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose += 2;
          debug++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--gigs"))
        {
          argc--; argv++;
          if (argc)
            {
              gigs = atoi (*argv);
              argc--; argv++;
            }
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);
    }

  if (gigs < 0 || gigs > 1024*1024)
    die ("value for --gigs must be in the range 0 to %d", 1024*1024);

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  /* A quick check that all given algorithms are valid.  */
  for (idx=0; idx < argc; idx++)
    {
      algo = gcry_md_map_name (argv[idx]);
      if (!algo)
        fail ("invalid algorithm '%s'", argv[idx]);
    }
  if (error_count)
    exit (1);

  /* Start checking.  */
  start_timer ();
  if (!argc)
    {
      for (algo=1; algo < 400; algo++)
        if (!gcry_md_test_algo (algo))
          {
            if (!gigs)
              run_selftest (algo);
            else
              run_longtest (algo, gigs);
          }
     }
  else
    {
      for (idx=0; idx < argc; idx++)
        {
          algo = gcry_md_map_name (argv[idx]);
          if (!algo)
            die ("invalid algorithm '%s'", argv[idx]);

          if (!gigs)
            run_selftest (algo);
          else
            run_longtest (algo, gigs);
        }
    }
  stop_timer ();

  if (missing_test_vectors)
    fail ("Some test vectors are missing");

  if (verbose)
    info ("All tests completed in %s.  Errors: %d\n",
          elapsed_time (1), error_count);
  return !!error_count;
}
