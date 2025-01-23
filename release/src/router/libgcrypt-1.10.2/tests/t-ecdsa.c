/* t-ecdsa.c - Check the ECDSA crypto
 * Copyright (C) 2021 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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

#include "stopwatch.h"

#define PGM "t-ecdsa"
#include "t-common.h"
#define N_TESTS 320

static int no_verify;
static int custom_data_file;
static int in_fips_mode;


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


/* Prepend FNAME with the srcdir environment variable's value and
 * return an allocated filename.  */
char *
prepend_srcdir (const char *fname)
{
  static const char *srcdir;
  char *result;

  if (!srcdir && !(srcdir = getenv ("srcdir")))
    srcdir = ".";

  result = xmalloc (strlen (srcdir) + 1 + strlen (fname) + 1);
  strcpy (result, srcdir);
  strcat (result, "/");
  strcat (result, fname);
  return result;
}


/* Read next line but skip over empty and comment lines.  Caller must
   xfree the result.  */
static char *
read_textline (FILE *fp, int *lineno)
{
  char line[4096];
  char *p;

  do
    {
      if (!fgets (line, sizeof line, fp))
        {
          if (feof (fp))
            return NULL;
          die ("error reading input line: %s\n", strerror (errno));
        }
      ++*lineno;
      p = strchr (line, '\n');
      if (!p)
        die ("input line %d not terminated or too long\n", *lineno);
      *p = 0;
      for (p--;p > line && my_isascii (*p) && isspace (*p); p--)
        *p = 0;
    }
  while (!*line || *line == '#');
  /* if (debug) */
  /*   info ("read line: '%s'\n", line); */
  return xstrdup (line);
}


/*
 * The input line is like:
 *
 *      [P-224,SHA-224]
 *
 */
static void
parse_annotation (char **buffer, char **buffer2, const char *line, int lineno)
{
  const char *s;

  xfree (*buffer);
  *buffer = NULL;

  xfree (*buffer2);
  *buffer2 = NULL;

  s = strchr (line, '-');
  if (!s)
    {
      fail ("syntax error at input line %d", lineno);
      return;
    }
  *buffer = xstrdup (s-1);
  (*buffer)[5] = 0; /* Remove ','.  */

  s = strchr (s+1, ',');
  if (!s)
    {
      fail ("syntax error at input line %d", lineno);
      return;
    }
  *buffer2 = xstrdup (s+1);
  (*buffer2)[strlen (*buffer2) - 1] = 0; /* Remove ']'.  */

}

/* Copy the data after the tag to BUFFER.  BUFFER will be allocated as
   needed.  */
static void
copy_data (char **buffer, const char *line, int lineno)
{
  const char *s;

  xfree (*buffer);
  *buffer = NULL;

  s = strchr (line, '=');
  if (!s)
    {
      fail ("syntax error at input line %d", lineno);
      return;
    }
  for (s++; my_isascii (*s) && isspace (*s); s++)
    ;
  *buffer = xstrdup (s);
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
  int odd;

  odd = ((strlen (string) & 1));

  buffer = xmalloc (strlen (string)/2 + odd + 1);
  if (odd)
    {
      length = 1;
      s = string;
      buffer[0] = xtoi_1 (s);
      s++;
    }
  else
    {
      length = 0;
      s = string;
    }
  for (; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        {
          xfree (buffer);
          return NULL;           /* Invalid hex digits. */
        }
      buffer[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}


static void
one_test_sexp (const char *curvename, const char *sha_alg,
	       const char *x, const char *y, const char *d,
	       const char *msg, const char *k,
	       const char *r, const char *s)
{
  gpg_error_t err;
  int i;
  char *p0;
  void *buffer = NULL;
  void *buffer2 = NULL;
  void *buffer3 = NULL;
  size_t buflen, buflen2, buflen3;
  unsigned char *pkbuffer = NULL;
  size_t pkbuflen;
  char curvename_gcrypt[11];
  gcry_ctx_t ctx = NULL;
  int md_algo;
  const char *data_tmpl;
  char data_tmpl2[256];
  gcry_md_hd_t hd = NULL;
  gcry_sexp_t s_pk = NULL;
  gcry_sexp_t s_sk = NULL;
  gcry_sexp_t s_sig = NULL, s_sig2 = NULL;
  gcry_sexp_t s_tmp, s_tmp2;
  unsigned char *out_r = NULL;
  unsigned char *out_s = NULL;
  size_t out_r_len, out_s_len;
  char *sig_r_string = NULL;
  char *sig_s_string = NULL;

  if (verbose > 1)
    info ("Running test %s\n", sha_alg);

  if (!strcmp (curvename, "P-224")
      || !strcmp (curvename, "P-256")
      || !strcmp (curvename, "P-384")
      || !strcmp (curvename, "P-521"))
    {
      memcpy (curvename_gcrypt, "NIST ", 5);
      strcpy (curvename_gcrypt+5, curvename);
    }
  else
    {
      fail ("error for test, %s: %s: %s",
            "ECC curve", "invalid", curvename);
      goto leave;
    }

  if (!strcmp (sha_alg, "SHA-224"))
    md_algo = GCRY_MD_SHA224;
  else if (!strcmp (sha_alg, "SHA-256"))
    md_algo = GCRY_MD_SHA256;
  else if (!strcmp (sha_alg, "SHA-384"))
    md_algo = GCRY_MD_SHA384;
  else if (!strcmp (sha_alg, "SHA-512"))
    md_algo = GCRY_MD_SHA512;
  else if (!strcmp (sha_alg, "SHA-512224"))
    md_algo = GCRY_MD_SHA512_224;
  else if (!strcmp (sha_alg, "SHA-512256"))
    md_algo = GCRY_MD_SHA512_256;
  else
    {
      fail ("error for test, %s: %s: %s",
            "SHA algo", "invalid", sha_alg);
      goto leave;
    }

  err = gcry_md_open (&hd, md_algo, 0);
  if (err)
    {
      fail ("algo %d, gcry_md_open failed: %s\n", md_algo, gpg_strerror (err));
      return;
    }

  if (!(buffer = hex2buffer (x, &buflen)))
    {
      fail ("error parsing for test, %s: %s",
            "Qx", "invalid hex string");
      goto leave;
    }
  if (!(buffer2 = hex2buffer (y, &buflen2)))
    {
      fail ("error parsing for test, %s: %s",
            "Qy", "invalid hex string");
      goto leave;
    }
  if (!(buffer3 = hex2buffer (d, &buflen3)))
    {
      fail ("error parsing for test, %s: %s",
            "d", "invalid hex string");
      goto leave;
    }

  pkbuflen = buflen + buflen2 + 1;
  pkbuffer = xmalloc (pkbuflen);
  pkbuffer[0] = 0x04;
  memcpy (pkbuffer+1, buffer, buflen);
  memcpy (pkbuffer+1+buflen, buffer2, buflen2);

  err = gcry_sexp_build (&s_sk, NULL,
                         "(private-key (ecc (curve %s)(q %b)(d %b)))",
			 curvename_gcrypt,
                         (int)pkbuflen, pkbuffer,
                         (int)buflen3, buffer3);
  if (err)
    {
      fail ("error building SEXP for test, %s: %s",
            "sk", gpg_strerror (err));
      goto leave;
    }

  err = gcry_sexp_build (&s_pk, NULL,
                         "(public-key (ecc (curve %s)(q %b)))",
			 curvename_gcrypt,
                         (int)pkbuflen, pkbuffer);
  if (err)
    {
      fail ("error building SEXP for test, %s: %s",
            "pk", gpg_strerror (err));
      goto leave;
    }

  xfree (buffer);
  xfree (buffer2);
  xfree (buffer3);
  buffer = buffer2 = buffer3 = NULL;
  xfree (pkbuffer);
  pkbuffer = NULL;

  if (!(buffer = hex2buffer (msg, &buflen)))
    {
      fail ("error parsing for test, %s: %s",
            "msg", "invalid hex string");
      goto leave;
    }

  gcry_md_write (hd, buffer, buflen);
  xfree (buffer);
  buffer = NULL;

  if (!(buffer2 = hex2buffer (k, &buflen2)))
    {
      fail ("error parsing for test, %s: %s",
            "salt_val", "invalid hex string");
      goto leave;
    }

  err = gcry_pk_random_override_new (&ctx, buffer2, buflen2);
  if (err)
    {
      fail ("error setting salt for test: %s",
	    gpg_strerror (err));
      goto leave;
    }

  xfree (buffer2);
  buffer2 = NULL;

  data_tmpl = "(data(flags raw)(hash %s %b)(label %b))";

  err = gcry_pk_hash_sign (&s_sig, data_tmpl, s_sk, hd, ctx);
  if (err)
    {
      fail ("gcry_pk_hash_sign failed: %s", gpg_strerror (err));
      goto leave;
    }

  if (snprintf (data_tmpl2, sizeof(data_tmpl2),
                "(data(flags raw)(hash %s %%b)(label %%b))",
                gcry_md_algo_name(md_algo)) >= sizeof(data_tmpl2))
    {
      fail ("snprintf out of bounds");
      goto leave;
    }
  err = gcry_pk_hash_sign (&s_sig2, data_tmpl2, s_sk, hd, ctx);
  if (err)
    {
      fail ("gcry_pk_hash_sign with explicit hash algorithm %s failed: %s",
            gcry_md_algo_name (md_algo), gpg_strerror (err));
      goto leave;
    }

  out_r_len = out_s_len = 0;
  out_s = out_r = NULL;
  s_tmp2 = NULL;
  s_tmp = gcry_sexp_find_token (s_sig, "sig-val", 0);
  if (s_tmp)
    {
      s_tmp2 = s_tmp;
      s_tmp = gcry_sexp_find_token (s_tmp2, "ecdsa", 0);
      if (s_tmp)
        {
          gcry_sexp_release (s_tmp2);
          s_tmp2 = s_tmp;
          s_tmp = gcry_sexp_find_token (s_tmp2, "r", 0);
          if (s_tmp)
            {
              const char *p;
              size_t n;

              out_r_len = buflen3;
              out_r = xmalloc (out_r_len);
              if (!out_r)
                {
                  err = gpg_error_from_syserror ();
                  gcry_sexp_release (s_tmp);
                  gcry_sexp_release (s_tmp2);
		  goto leave;
                }

              p = gcry_sexp_nth_data (s_tmp, 1, &n);
              if (n == out_r_len)
                memcpy (out_r, p, out_r_len);
              else
                {
                  memset (out_r, 0, out_r_len - n);
                  memcpy (out_r + out_r_len - n, p, n);
                }
              gcry_sexp_release (s_tmp);
            }
          s_tmp = gcry_sexp_find_token (s_tmp2, "s", 0);
          if (s_tmp)
            {
              const char *p;
              size_t n;

              out_s_len = out_r_len;
              out_s = xmalloc (out_s_len);
              if (!out_s)
                {
                  err = gpg_error_from_syserror ();
                  gcry_sexp_release (s_tmp);
                  gcry_sexp_release (s_tmp2);
		  goto leave;
                }

              p = gcry_sexp_nth_data (s_tmp, 1, &n);
              if (n == out_s_len)
                memcpy (out_s, p, out_s_len);
              else
                {
                  memset (out_s, 0, out_s_len - n);
                  memcpy (out_s + out_s_len - n, p, n);
                }
              gcry_sexp_release (s_tmp);
            }
        }
    }
  gcry_sexp_release (s_tmp2);

  sig_r_string = xmalloc (2*out_r_len+1);
  p0 = sig_r_string;
  *p0 = 0;
  for (i=0; i < out_r_len; i++, p0 += 2)
    snprintf (p0, 3, "%02x", out_r[i]);

  sig_s_string = xmalloc (2*out_s_len+1);
  p0 = sig_s_string;
  *p0 = 0;
  for (i=0; i < out_s_len; i++, p0 += 2)
    snprintf (p0, 3, "%02x", out_s[i]);

  if (strcmp (sig_r_string + (strcmp (curvename, "P-521") == 0), r)
      || strcmp (sig_s_string + (strcmp (curvename, "P-521") == 0), s))
    {
      fail ("gcry_pkey_op failed: %s",
            "wrong value returned");
      info ("  expected: '%s'", r);
      info ("       got: '%s'", sig_r_string);
      info ("  expected: '%s'", s);
      info ("       got: '%s'", sig_s_string);
    }

  if (!no_verify)
    {
      err = gcry_pk_hash_verify (s_sig, data_tmpl, s_pk, hd, ctx);
      if (err)
        fail ("gcry_pk_hash_verify failed for test: %s",
              gpg_strerror (err));

      err = gcry_pk_hash_verify (s_sig2, data_tmpl2, s_pk, hd, ctx);
      if (err)
        fail ("gcry_pk_hash_verify with explicit hash algorithm %s failed: %s",
              gcry_md_algo_name (md_algo), gpg_strerror (err));
    }

 leave:
  gcry_ctx_release (ctx);
  gcry_sexp_release (s_sig);
  gcry_sexp_release (s_sig2);
  gcry_sexp_release (s_sk);
  gcry_sexp_release (s_pk);
  if (hd)
    gcry_md_close (hd);
  xfree (buffer);
  xfree (buffer2);
  xfree (buffer3);
  xfree (out_r);
  xfree (out_s);
  xfree (sig_r_string);
  xfree (sig_s_string);
  xfree (pkbuffer);
}


static void
check_ecdsa (const char *fname)
{
  FILE *fp;
  int lineno, ntests;
  char *line;
  char *curve, *sha_alg;
  char *x, *y;
  char *d;
  char *msg, *k, *r, *s;

  info ("Checking ECDSA.\n");

  fp = fopen (fname, "r");
  if (!fp)
    die ("error opening '%s': %s\n", fname, strerror (errno));

  curve = NULL;
  sha_alg = NULL;
  x = y = d = NULL;
  msg = k = r = s = NULL;
  lineno = ntests = 0;
  while ((line = read_textline (fp, &lineno)))
    {
      if (!strncmp (line, "[", 1))
        parse_annotation (&curve, &sha_alg, line, lineno);
      else if (!strncmp (line, "Msg =", 5))
        copy_data (&msg, line, lineno);
      else if (!strncmp (line, "d =", 3))
        copy_data (&d, line, lineno);
      else if (!strncmp (line, "Qx =", 4))
        copy_data (&x, line, lineno);
      else if (!strncmp (line, "Qy =", 4))
        copy_data (&y, line, lineno);
      else if (!strncmp (line, "k =", 3))
        copy_data (&k, line, lineno);
      else if (!strncmp (line, "R =", 3))
        copy_data (&r, line, lineno);
      else if (!strncmp (line, "S =", 3))
        copy_data (&s, line, lineno);
      else
        fail ("unknown tag at input line %d", lineno);

      xfree (line);
      if (curve && sha_alg && x && y && d && msg && k && r && s)
        {
          one_test_sexp (curve, sha_alg, x, y, d, msg, k, r, s);
          ntests++;
          if (!(ntests % 256))
            show_note ("%d of %d tests done\n", ntests, N_TESTS);
          xfree (msg); msg = NULL;
          xfree (x); x = NULL;
          xfree (y); y = NULL;
          xfree (d); d = NULL;
          xfree (k); k = NULL;
          xfree (r); r = NULL;
          xfree (s); s = NULL;
        }

    }
  xfree (curve);
  xfree (sha_alg);
  xfree (x);
  xfree (y);
  xfree (d);
  xfree (msg);
  xfree (k);
  xfree (r);
  xfree (s);

  if (ntests != N_TESTS && !custom_data_file)
    fail ("did %d tests but expected %d", ntests, N_TESTS);
  else if ((ntests % 256))
    show_note ("%d tests done\n", ntests);

  fclose (fp);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  char *fname = NULL;

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
          fputs ("usage: " PGM " [options]\n"
                 "Options:\n"
                 "  --verbose       print timings etc.\n"
                 "  --debug         flyswatter\n"
                 "  --no-verify     skip the verify test\n"
                 "  --data FNAME    take test data from file FNAME\n",
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
      else if (!strcmp (*argv, "--no-verify"))
        {
          no_verify = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--data"))
        {
          argc--; argv++;
          if (argc)
            {
              xfree (fname);
              fname = xstrdup (*argv);
              argc--; argv++;
            }
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);

    }

  if (!fname)
    fname = prepend_srcdir ("t-ecdsa.inp");
  else
    custom_data_file = 1;

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  start_timer ();
  check_ecdsa (fname);
  stop_timer ();

  xfree (fname);

  info ("All tests completed in %s.  Errors: %d\n",
        elapsed_time (1), error_count);
  return !!error_count;
}
