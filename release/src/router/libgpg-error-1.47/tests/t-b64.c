/* t-b64.c - b64dec tests.
 * Copyright (C) 2017, 2018 g10 Code GmbH
 *
 * This file is part of Libgpg-error.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#define PGM "t-b64"
#include "t-common.h"

static const char *test_string = "libgpg-error is free software; "
  "you can redistribute it and/or modify it under the terms of "
  "the GNU Lesser General Public License as published by the Free "
  "Software Foundation; either version 2.1 of the License, or "
  "(at your option) any later version.";

static const char *test_string_b64_0 = "bGliZ3BnLWVycm9yIGlzIGZyZWUgc29"
  "mdHdhcmU7IHlvdSBjYW4gcmVkaXN0cmlidXRlIGl0IGFuZC9vciBtb2RpZnkgaXQgd"
  "W5kZXIgdGhlIHRlcm1zIG9mIHRoZSBHTlUgTGVzc2VyIEdlbmVyYWwgUHVibGljIEx"
  "pY2Vuc2UgYXMgcHVibGlzaGVkIGJ5IHRoZSBGcmVlIFNvZnR3YXJlIEZvdW5kYXRpb"
  "247IGVpdGhlciB2ZXJzaW9uIDIuMSBvZiB0aGUgTGljZW5zZSwgb3IgKGF0IHlvdXI"
  "gb3B0aW9uKSBhbnkgbGF0ZXIgdmVyc2lvbi4=";

static const char *test_string_b64_1 =
  "bGliZ3BnLWVycm9yIGlzIGZyZWUgc29mdHdhcmU7IHlvdSBjYW4gcmVkaXN0cmli\n"
  "dXRlIGl0IGFuZC9vciBtb2RpZnkgaXQgdW5kZXIgdGhlIHRlcm1zIG9mIHRoZSBH\n"
  "TlUgTGVzc2VyIEdlbmVyYWwgUHVibGljIExpY2Vuc2UgYXMgcHVibGlzaGVkIGJ5\n"
  "IHRoZSBGcmVlIFNvZnR3YXJlIEZvdW5kYXRpb247IGVpdGhlciB2ZXJzaW9uIDIu\n"
  "MSBvZiB0aGUgTGljZW5zZSwgb3IgKGF0IHlvdXIgb3B0aW9uKSBhbnkgbGF0ZXIg\n"
  "dmVyc2lvbi4=\n";

static const char *test_string_b64_2 =
  "-----BEGIN DATA-----\n"
  "bGliZ3BnLWVycm9yIGlzIGZyZWUgc29mdHdhcmU7IHlvdSBjYW4gcmVkaXN0cmli\n"
  "dXRlIGl0IGFuZC9vciBtb2RpZnkgaXQgdW5kZXIgdGhlIHRlcm1zIG9mIHRoZSBH\n"
  "TlUgTGVzc2VyIEdlbmVyYWwgUHVibGljIExpY2Vuc2UgYXMgcHVibGlzaGVkIGJ5\n"
  "IHRoZSBGcmVlIFNvZnR3YXJlIEZvdW5kYXRpb247IGVpdGhlciB2ZXJzaW9uIDIu\n"
  "MSBvZiB0aGUgTGljZW5zZSwgb3IgKGF0IHlvdXIgb3B0aW9uKSBhbnkgbGF0ZXIg\n"
  "dmVyc2lvbi4=\n"
  "-----END DATA-----\n";

static const char *test_string_b64_3 =
  "-----BEGIN PGP ARMORED FILE-----\n"
  "\n"
  "bGliZ3BnLWVycm9yIGlzIGZyZWUgc29mdHdhcmU7IHlvdSBjYW4gcmVkaXN0cmli\n"
  "dXRlIGl0IGFuZC9vciBtb2RpZnkgaXQgdW5kZXIgdGhlIHRlcm1zIG9mIHRoZSBH\n"
  "TlUgTGVzc2VyIEdlbmVyYWwgUHVibGljIExpY2Vuc2UgYXMgcHVibGlzaGVkIGJ5\n"
  "IHRoZSBGcmVlIFNvZnR3YXJlIEZvdW5kYXRpb247IGVpdGhlciB2ZXJzaW9uIDIu\n"
  "MSBvZiB0aGUgTGljZW5zZSwgb3IgKGF0IHlvdXIgb3B0aW9uKSBhbnkgbGF0ZXIg\n"
  "dmVyc2lvbi4=\n"
  "=4BMJ\n"
  "-----END PGP ARMORED FILE-----\n";

static const char *test_blob_1 = "\x01\x03\x04\xff";
static const char *test_blob_1_b64_0 = "AQME/w==";
static const char *test_blob_2 = "\x01\x03\x04\xff""A";
static const char *test_blob_2_b64_0 = "AQME/0E=";
static const char *test_blob_3 = "\x01\x03\x04\xff""AB";
static const char *test_blob_3_b64_0 = "AQME/0FC";


#define FAIL(a)  do { fail ("line %d: test %d failed\n", __LINE__, (a));  \
                    } while(0)

static gpg_error_t
test_b64enc_string (const char *string, const char *expected, const char *title)
{
  gpg_err_code_t err;
  estream_t fp;
  gpgrt_b64state_t state;
  char *result;

  fp = es_fopenmem (0, "rwb");
  if (!fp)
    die ("es_fopenmem failed: %s\n", gpg_strerror (gpg_error_from_syserror ()));

  state = gpgrt_b64enc_start (fp, title);
  if (!state)
    {
      err = gpg_err_code_from_syserror ();
      fail ("gpgrt_b64enc_start failed: %s\n", gpg_strerror (err));
      return err;
    }

  err = gpgrt_b64enc_write (state, string, strlen (string));
  if (err)
    {
      free (state);
      fail ("gpgrt_b64enc_write failed: %s\n", gpg_strerror (err));
      return err;
    }

  err = gpgrt_b64enc_finish (state);
  if (err)
    {
      fail ("gpgrt_b64enc_finish failed: %s\n", gpg_strerror (err));
      return err;
    }

  es_fputc (0, fp);
  if (es_fclose_snatch (fp, (void**)&result, NULL))
    die ("es_fclose_snatch failed: %s\n",
         gpg_strerror (gpg_error_from_syserror ()));

  if (strcmp (result, expected))
    {
      if (verbose)
        {
          gpgrt_log_debug_string (result,   "result: ");
          gpgrt_log_debug_string (expected, "expect: ");
        }
      return GPG_ERR_FALSE;
    }

  es_free (result);
  return 0;
}


static gpg_error_t
test_b64dec_string (const char *string, const char *expected, const char *title)
{
  gpg_error_t err;
  gpgrt_b64state_t state;
  char *buffer;
  size_t len;

  len = strlen (string);
  buffer = malloc (strlen (string) + 1);
  if (!buffer)
    {
      err = gpg_error_from_syserror ();
      return err;
    }
  strcpy (buffer, string);

  state = gpgrt_b64dec_start (title);
  if (!state)
    {
      err = gpg_err_code_from_syserror ();
      fail ("gpgrt_b64dec_start failed: %s\n", gpg_strerror (err));
      free (buffer);
      return err;
    }

  err = gpgrt_b64dec_proc (state, buffer, len, &len);
  if (err)
    {
      if (gpg_err_code (err) != GPG_ERR_EOF)
        {
          free (buffer);
          free (state);
          return err;
        }
    }

  err = gpgrt_b64dec_finish (state);
  if (err)
    {
      free (buffer);
      return err;
    }

  if (len != strlen (expected) || strncmp (buffer, expected, len))
    {
      if (verbose)
        {
          gpgrt_log_debug_string (buffer,   "result(len=%zu): ", len);
          gpgrt_log_debug_string (expected, "expect(len=%zu): ",
                                  strlen (expected));
        }
      free (buffer);
      return GPG_ERR_FALSE;
    }

  free (buffer);
  return 0;
}


static void
encoder_tests (void)
{
  gpg_err_code_t err;

  if (verbose)
    show ("running encoder tests\n");

  err = test_b64enc_string (test_string, test_string_b64_0, "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string (test_string, test_string_b64_1, NULL);
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string (test_string, test_string_b64_2, "DATA");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string (test_string, test_string_b64_3, "PGP ARMORED FILE");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  /* Note that the _test_ function dows not allow to provide a string
   * with an empdded Nul.  */
  err = test_b64enc_string (test_blob_1, test_blob_1_b64_0, "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string (test_blob_2, test_blob_2_b64_0, "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string (test_blob_3, test_blob_3_b64_0, "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  err = test_b64enc_string ("@", "QA==", "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string ("@", "QA==\n", NULL);
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string ("@",
                            "-----BEGIN PGP SOMETHING-----\n"
                            "\n"
                            "QA==\n"
                            "=eMoB\n"
                            "-----END PGP SOMETHING-----\n",
                            "PGP SOMETHING");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  err = test_b64enc_string ("", "", "");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string ("", "", NULL);
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
  err = test_b64enc_string ("", "", "PGP SOMETHING");
  if (err)
    fail ("encoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));
}


static void
decoder_tests (void)
{
  gpg_err_code_t err;

  if (verbose)
    show ("running decoder tests\n");

  err = test_b64dec_string (test_string_b64_0, test_string, NULL);
  if (err)
    fail ("decoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  err = test_b64dec_string (test_string_b64_1, test_string, NULL);
  if (err)
    fail ("decoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  err = test_b64dec_string (test_string_b64_2, test_string, "");
  if (err)
    fail ("decoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

  err = test_b64dec_string (test_string_b64_2, test_string, NULL);
  if (err != GPG_ERR_BAD_DATA)
    fail ("decoder test at line %d failed: %s\n", __LINE__, gpg_strerror (err));

}


static gpg_error_t
extra_tests (void)
{
  gpg_err_code_t err;
  gpgrt_b64state_t state;

  if (verbose)
    show ("running extra tests\n");

  /* Check that we detect mismacthed use of enc and dec functions.  */
  state = gpgrt_b64enc_start (es_stdout, NULL);
  if (!state)
    {
      err = gpg_err_code_from_syserror ();
      fail ("gpgrt_b64enc_start failed: %s\n", gpg_strerror (err));
      return err;
    }

  err = gpgrt_b64dec_finish (state);
  if (err != GPG_ERR_CONFLICT)
    {
      fail ("gpgrt_b64dec_finish failed: %s\n", gpg_strerror (err));
      return err;
    }

  state = gpgrt_b64dec_start (NULL);
  if (!state)
    {
      err = gpg_err_code_from_syserror ();
      fail ("gpgrt_b64dec_start failed: %s\n", gpg_strerror (err));
      return err;
    }

  err = gpgrt_b64enc_finish (state);
  if (err != GPG_ERR_CONFLICT)
    {
      fail ("gpgrt_b64enc_finish failed: %s\n", gpg_strerror (err));
      return err;
    }

  return 0;
}


int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--help"))
        {
          puts (
"usage: ./" PGM " [options]\n"
"\n"
"Options:\n"
"  --verbose      Show what is going on\n"
"  --debug        Flyswatter\n"
);
          exit (0);
        }
      if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
    }

  encoder_tests ();
  decoder_tests ();
  extra_tests ();

  return !!errorcount;
}
