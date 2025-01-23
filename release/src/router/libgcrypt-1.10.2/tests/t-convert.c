/* t-convert.c  - Tests for mpi print and scna functions
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
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define PGM "t-convert"
#include "t-common.h"


static void
showhex (const char *prefix, const void *buffer, size_t buflen)
{
  const unsigned char *s;

  if (!verbose)
    return;
  fprintf (stderr, "%s: %s ", PGM, prefix);
  for (s= buffer; buflen; buflen--, s++)
    fprintf (stderr, "%02x", *s);
  putc ('\n', stderr);
}


/* Allocate a bit string consisting of '0' and '1' from the MPI A.  Do
   not return any leading zero bits.  Caller needs to gcry_free the
   result. */
static char *
mpi2bitstr_nlz (gcry_mpi_t a)
{
  char *p, *buf;
  size_t length = gcry_mpi_get_nbits (a);

  if (!length)
    {
      buf = p = xmalloc (3);
      *p++ = ' ';
      *p++ = '0';
    }
  else
    {
      buf = p = xmalloc (length + 1 + 1);
      *p++ = gcry_mpi_is_neg (a)? '-':' ';
      while (length-- > 1)
        *p++ = gcry_mpi_test_bit (a, length) ? '1':'0';
      *p++ = gcry_mpi_test_bit (a, 0) ? '1':'0';
    }
  *p = 0;
  return buf;
}


static void
showmpi (const char *prefix, gcry_mpi_t a)
{
  char *bitstr;

  if (!verbose)
    return;
  bitstr = mpi2bitstr_nlz (a);
  fprintf (stderr, "%s: %s%s\n", PGM, prefix, bitstr);
  xfree (bitstr);
}


/* Check that mpi_print does not return a negative zero.  */
static void
negative_zero (void)
{
  gpg_error_t err;
  gcry_mpi_t a;
  char *buf;
  void *bufaddr = &buf;
  struct { const char *name; enum gcry_mpi_format format; } fmts[] =
    {
      { "STD", GCRYMPI_FMT_STD },
      { "PGP", GCRYMPI_FMT_PGP },
      { "SSH", GCRYMPI_FMT_SSH },
      { "HEX", GCRYMPI_FMT_HEX },
      { "USG", GCRYMPI_FMT_USG },
      { NULL, 0 }
    };
  int i;

  if (debug)
    info ("negative zero printing\n");

  a = gcry_mpi_new (0);
  for (i=0; fmts[i].name; i++)
    {
      err = gcry_mpi_aprint (fmts[i].format, bufaddr, NULL, a);
      if (err)
        fail ("error printing a zero as %s: %s\n",
              fmts[i].name,gpg_strerror (err) );
      else
        gcry_free (buf);
    }

  /* With the current version of libgcrypt the next two statements
     should set a to -0. */
  gcry_mpi_sub_ui (a, a, 1);
  gcry_mpi_add_ui (a, a, 1);

  for (i=0; fmts[i].name; i++)
    {
      err = gcry_mpi_aprint (fmts[i].format, bufaddr, NULL, a);
      if (err)
        fail ("error printing a negative zero as %s: %s\n",
              fmts[i].name,gpg_strerror (err) );
      else
        gcry_free (buf);
    }

  gcry_mpi_release (a);
}


static void
check_formats (void)
{
  static struct {
    int have_value;
    int value;
    struct {
      const char *hex;
      size_t stdlen;
      const char *std;
      size_t sshlen;
      const char *ssh;
      size_t usglen;
      const char *usg;
      size_t pgplen;
      const char *pgp;
    } a;
  } data[] = {
    {
      1, 0,
      { "00",
	0, "",
	4, "\x00\x00\x00\x00",
	0, "",
	2, "\x00\x00" }
    },
    {
      1, 1,
      { "01",
	1, "\x01",
	5, "\x00\x00\x00\x01\x01",
	1, "\x01",
	3, "\x00\x01\x01" }
    },
    {
      1, 2,
      { "02",
	1, "\x02",
	5, "\x00\x00\x00\x01\x02",
	1, "\x02",
	3, "\x00\x02\x02" }
    },
    {
      1, 127,
      { "7F",
	1, "\x7f",
	5, "\x00\x00\x00\x01\x7f",
	1, "\x7f",
	3, "\x00\x07\x7f" }
    },
    {
      1, 128,
      { "0080",
	2, "\x00\x80",
	6, "\x00\x00\x00\x02\x00\x80",
	1, "\x80",
	3, "\x00\x08\x80" }
    },
    {
      1, 129,
      { "0081",
	2, "\x00\x81",
	6, "\x00\x00\x00\x02\x00\x81",
	1, "\x81",
	3, "\x00\x08\x81" }
    },
    {
      1, 255,
      { "00FF",
	2, "\x00\xff",
	6, "\x00\x00\x00\x02\x00\xff",
	1, "\xff",
	3, "\x00\x08\xff" }
    },
    {
      1, 256,
      { "0100",
	2, "\x01\x00",
	6, "\x00\x00\x00\x02\x01\x00",
	2, "\x01\x00",
	4, "\x00\x09\x01\x00" }
    },
    {
      1, 257,
      { "0101",
	2, "\x01\x01",
	6, "\x00\x00\x00\x02\x01\x01",
	2, "\x01\x01",
	4, "\x00\x09\x01\x01" }
    },
    {
      1, -1,
      { "-01",
	1, "\xff",
	5, "\x00\x00\x00\x01\xff",
	1,"\x01" }
    },
    {
      1, -2,
      { "-02",
	1, "\xfe",
	5, "\x00\x00\x00\x01\xfe",
	1, "\x02" }
    },
    {
      1, -127,
      { "-7F",
	1, "\x81",
	5, "\x00\x00\x00\x01\x81",
	1, "\x7f" }
    },
    {
      1, -128,
      { "-0080",
	1, "\x80",
	5, "\x00\x00\x00\x01\x80",
	1, "\x80" }
    },
    {
      1, -129,
      { "-0081",
	2, "\xff\x7f",
	6, "\x00\x00\x00\x02\xff\x7f",
	1, "\x81" }
    },
    {
      1, -255,
      { "-00FF",
	2, "\xff\x01",
	6, "\x00\x00\x00\x02\xff\x01",
	1, "\xff" }
    },
    {
      1, -256,
      { "-0100",
	2, "\xff\x00",
	6, "\x00\x00\x00\x02\xff\x00",
	2, "\x01\x00" }
    },
    {
      1, -257,
      { "-0101",
	2, "\xfe\xff",
	6, "\x00\x00\x00\x02\xfe\xff",
	2, "\x01\x01" }
    },
    {
      1, 65535,
      { "00FFFF",
	3, "\x00\xff\xff",
	7, "\x00\x00\x00\x03\x00\xff\xff",
	2, "\xff\xff",
	4, "\x00\x10\xff\xff" }
    },
    {
      1, 65536,
      { "010000",
	3, "\x01\00\x00",
	7, "\x00\x00\x00\x03\x01\x00\x00",
	3, "\x01\x00\x00",
	5, "\x00\x11\x01\x00\x00 "}
    },
    {
      1, 65537,
      { "010001",
	3, "\x01\00\x01",
	7, "\x00\x00\x00\x03\x01\x00\x01",
	3, "\x01\x00\x01",
	5, "\x00\x11\x01\x00\x01" }
    },
    {
      1, -65537,
      { "-010001",
	3, "\xfe\xff\xff",
	7, "\x00\x00\x00\x03\xfe\xff\xff",
	3, "\x01\x00\x01" }
    },
    {
      1, -65536,
      { "-010000",
	3, "\xff\x00\x00",
	7, "\x00\x00\x00\x03\xff\x00\x00",
	3, "\x01\x00\x00" }
    },
    {
      1, -65535,
      { "-00FFFF",
	3, "\xff\x00\x01",
	7, "\x00\x00\x00\x03\xff\x00\x01",
	2, "\xff\xff" }
    },
    {
      1, 0x7fffffff,
      { "7FFFFFFF",
	4, "\x7f\xff\xff\xff",
	8, "\x00\x00\x00\x04\x7f\xff\xff\xff",
	4, "\x7f\xff\xff\xff",
	6, "\x00\x1f\x7f\xff\xff\xff" }
    },
    { 1, -0x7fffffff,
      { "-7FFFFFFF",
	4, "\x80\x00\x00\x01",
	8, "\x00\x00\x00\x04\x80\x00\x00\x01",
	4, "\x7f\xff\xff\xff" }
    },
    {
      1, (int)0x800000ffU,
      { "-7FFFFF01",
	4, "\x80\x00\x00\xff",
	8, "\x00\x00\x00\x04\x80\x00\x00\xff",
	4, "\x7f\xff\xff\x01" }
    },
    {
      1, (int)0x800000feU,
      { "-7FFFFF02",
	4, "\x80\x00\x00\xfe",
	8, "\x00\x00\x00\x04\x80\x00\x00\xfe",
	4, "\x7f\xff\xff\x02" }
    },
    {
      1, (int)0x800000fcU,
      { "-7FFFFF04",
	4, "\x80\x00\x00\xfc",
	8, "\x00\x00\x00\x04\x80\x00\x00\xfc",
	4, "\x7f\xff\xff\x04" }
    },
    {
      1, (int)0x800000f8U,
      { "-7FFFFF08",
	4, "\x80\x00\x00\xf8",
	8, "\x00\x00\x00\x04\x80\x00\x00\xf8",
	4, "\x7f\xff\xff\x08" }
    },
    {
      1, (int)0x800000f0U,
      { "-7FFFFF10",
	4, "\x80\x00\x00\xf0",
	8, "\x00\x00\x00\x04\x80\x00\x00\xf0",
	4, "\x7f\xff\xff\x10" }
    },
    {
      1, (int)0x800000e0U,
      { "-7FFFFF20",
	4, "\x80\x00\x00\xe0",
	8, "\x00\x00\x00\x04\x80\x00\x00\xe0",
	4, "\x7f\xff\xff\x20" }
    },
    {
      1, (int)0x800000c0U,
      { "-7FFFFF40",
	4, "\x80\x00\x00\xc0",
	8, "\x00\x00\x00\x04\x80\x00\x00\xc0",
	4, "\x7f\xff\xff\x40" }
    },
    {
      1, (int)0x80000080U,
      { "-7FFFFF80",
	4, "\x80\x00\x00\x80",
	8, "\x00\x00\x00\x04\x80\x00\x00\x80",
	4, "\x7f\xff\xff\x80" }
    },
    {
      1, (int)0x80000100U,
      { "-7FFFFF00",
	4, "\x80\x00\x01\x00",
	8, "\x00\x00\x00\x04\x80\x00\x01\x00",
	4, "\x7f\xff\xff\x00" }
    },
    {
      0, 0,
      { "076543210FEDCBA9876543210123456789ABCDEF00112233",
	24, "\x07\x65\x43\x21\x0f\xed\xcb\xa9\x87\x65\x43\x21\x01\x23"
	    "\x45\x67\x89\xab\xcd\xef\x00\x11\x22\x33",
	28, "\x00\x00\x00\x18\x07\x65\x43\x21\x0f\xed\xcb\xa9\x87\x65"
	    "\x43\x21\x01\x23\x45\x67\x89\xab\xcd\xef\x00\x11\x22\x33"
	    "\x44",
	24, "\x07\x65\x43\x21\x0f\xed\xcb\xa9\x87\x65\x43\x21\x01\x23"
	    "\x45\x67\x89\xab\xcd\xef\x00\x11\x22\x33",
	26, "\x00\xbb\x07\x65\x43\x21\x0f\xed\xcb\xa9\x87\x65\x43\x21"
	    "\x01\x23\x45\x67\x89\xab\xcd\xef\x00\x11\x22\x33" }
    },
    {
      0, 0,
      { "-07FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF01",
	24, "\xf8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff",
	28, "\x00\x00\x00\x18\xf8\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff",
	24, "\x07\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01" }
    }
  };
  gpg_error_t err;
  gcry_mpi_t a, b;
  char valuestr[128];
  char *buf;
  void *bufaddr = &buf;
  int idx;
  size_t buflen;

  a = gcry_mpi_new (0);
  for (idx=0; idx < DIM(data); idx++)
    {
      if (debug)
        info ("print test %d\n", data[idx].value);

      if (data[idx].have_value)
	{
	  snprintf(valuestr, sizeof(valuestr), "%d", data[idx].value);
	  if (data[idx].value < 0)
	    {
	      gcry_mpi_set_ui (a, -data[idx].value);
	      gcry_mpi_neg (a, a);
	    }
	  else
	    gcry_mpi_set_ui (a, data[idx].value);
	}
      else
	{
	  /* Use hex-format as source test vector. */
	  snprintf(valuestr, sizeof(valuestr), "%s", data[idx].a.hex);
          gcry_mpi_release (a);
	  err = gcry_mpi_scan (&a, GCRYMPI_FMT_HEX, data[idx].a.hex, 0,
			       &buflen);
	  if (err)
	    fail ("error scanning value %s from %s: %s\n",
		  valuestr, "HEX", gpg_strerror (err));
	}

      err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
      if (err)
        fail ("error printing value %s as %s: %s\n",
              valuestr, "HEX", gpg_strerror (err));
      else
        {
          if (strcmp (buf, data[idx].a.hex))
            {
              fail ("error printing value %s as %s: %s\n",
                    valuestr, "HEX", "wrong result");
              info ("expected: '%s'\n", data[idx].a.hex);
              info ("     got: '%s'\n", buf);
            }
          gcry_free (buf);
        }

      err = gcry_mpi_aprint (GCRYMPI_FMT_STD, bufaddr, &buflen, a);
      if (err)
        fail ("error printing value %s as %s: %s\n",
              valuestr, "STD", gpg_strerror (err));
      else
        {
          if (buflen != data[idx].a.stdlen
              || memcmp (buf, data[idx].a.std, data[idx].a.stdlen))
            {
              fail ("error printing value %s as %s: %s\n",
                    valuestr, "STD", "wrong result");
              showhex ("expected:", data[idx].a.std, data[idx].a.stdlen);
              showhex ("     got:", buf, buflen);
            }
          gcry_free (buf);
        }

      err = gcry_mpi_aprint (GCRYMPI_FMT_SSH, bufaddr, &buflen, a);
      if (err)
        fail ("error printing value %s as %s: %s\n",
              valuestr, "SSH", gpg_strerror (err));
      else
        {
          if (buflen != data[idx].a.sshlen
              || memcmp (buf, data[idx].a.ssh, data[idx].a.sshlen))
            {
              fail ("error printing value %s as %s: %s\n",
                    valuestr, "SSH", "wrong result");
              showhex ("expected:", data[idx].a.ssh, data[idx].a.sshlen);
              showhex ("     got:", buf, buflen);
            }
          gcry_free (buf);
        }

      err = gcry_mpi_aprint (GCRYMPI_FMT_USG, bufaddr, &buflen, a);
      if (err)
        fail ("error printing value %s as %s: %s\n",
              valuestr, "USG", gpg_strerror (err));
      else
        {
          if (buflen != data[idx].a.usglen
              || memcmp (buf, data[idx].a.usg, data[idx].a.usglen))
            {
              fail ("error printing value %s as %s: %s\n",
                    valuestr, "USG", "wrong result");
              showhex ("expected:", data[idx].a.usg, data[idx].a.usglen);
              showhex ("     got:", buf, buflen);
            }
          gcry_free (buf);
        }

      err = gcry_mpi_aprint (GCRYMPI_FMT_PGP, bufaddr, &buflen, a);
      if (gcry_mpi_is_neg (a))
        {
          if (gpg_err_code (err) != GPG_ERR_INV_ARG)
            fail ("error printing value %s as %s: %s\n",
                  valuestr, "PGP", "Expected error not returned");
        }
      else if (err)
        fail ("error printing value %s as %s: %s\n",
              valuestr, "PGP", gpg_strerror (err));
      else
        {
          if (buflen != data[idx].a.pgplen
              || memcmp (buf, data[idx].a.pgp, data[idx].a.pgplen))
            {
              fail ("error printing value %s as %s: %s\n",
                    valuestr, "PGP", "wrong result");
              showhex ("expected:", data[idx].a.pgp, data[idx].a.pgplen);
              showhex ("     got:", buf, buflen);
            }
          gcry_free (buf);
        }
    }


  /* Now for the other direction.  */
  for (idx=0; idx < DIM(data); idx++)
    {
      if (debug)
        info ("scan test %d\n", data[idx].value);

      if (data[idx].have_value)
	{
	  snprintf(valuestr, sizeof(valuestr), "%d", data[idx].value);
	  if (data[idx].value < 0)
	    {
	      gcry_mpi_set_ui (a, -data[idx].value);
	      gcry_mpi_neg (a, a);
	    }
	  else
	    gcry_mpi_set_ui (a, data[idx].value);
	}
      else
	{
	  /* Use hex-format as source test vector. */
	  snprintf(valuestr, sizeof(valuestr), "%s", data[idx].a.hex);
          gcry_mpi_release (a);
	  err = gcry_mpi_scan (&a, GCRYMPI_FMT_HEX, data[idx].a.hex, 0,
			       &buflen);
	  if (err)
	    fail ("error scanning value %s from %s: %s\n",
		  valuestr, "HEX", gpg_strerror (err));
	}

      err = gcry_mpi_scan (&b, GCRYMPI_FMT_HEX, data[idx].a.hex, 0, &buflen);
      if (err)
        fail ("error scanning value %s from %s: %s\n",
              valuestr, "HEX", gpg_strerror (err));
      else
        {
          if (gcry_mpi_cmp (a, b))
            {
              fail ("error scanning value %s from %s: %s\n",
                    valuestr, "HEX", "wrong result");
              showmpi ("expected:", a);
              showmpi ("     got:", b);
            }
          gcry_mpi_release (b);
        }

      err = gcry_mpi_scan (&b, GCRYMPI_FMT_STD,
                           data[idx].a.std, data[idx].a.stdlen, &buflen);
      if (err)
        fail ("error scanning value %s as %s: %s\n",
              valuestr, "STD", gpg_strerror (err));
      else
        {
          if (gcry_mpi_cmp (a, b) || data[idx].a.stdlen != buflen)
            {
              fail ("error scanning value %s from %s: %s (%lu)\n",
                    valuestr, "STD", "wrong result",
                    (long unsigned int)buflen);
              showmpi ("expected:", a);
              showmpi ("     got:", b);
            }
          gcry_mpi_release (b);
        }

      err = gcry_mpi_scan (&b, GCRYMPI_FMT_SSH,
                           data[idx].a.ssh, data[idx].a.sshlen, &buflen);
      if (err)
        fail ("error scanning value %s as %s: %s\n",
              valuestr, "SSH", gpg_strerror (err));
      else
        {
          if (gcry_mpi_cmp (a, b) || data[idx].a.sshlen != buflen)
            {
              fail ("error scanning value %s from %s: %s (%lu)\n",
                    valuestr, "SSH", "wrong result",
                    (long unsigned int)buflen);
              showmpi ("expected:", a);
              showmpi ("     got:", b);
            }
          gcry_mpi_release (b);
        }

      err = gcry_mpi_scan (&b, GCRYMPI_FMT_USG,
                           data[idx].a.usg, data[idx].a.usglen, &buflen);
      if (err)
        fail ("error scanning value %s as %s: %s\n",
              valuestr, "USG", gpg_strerror (err));
      else
        {
          if (gcry_mpi_is_neg (a))
            gcry_mpi_neg (b, b);
          if (gcry_mpi_cmp (a, b) || data[idx].a.usglen != buflen)
            {
              fail ("error scanning value %s from %s: %s (%lu)\n",
                    valuestr, "USG", "wrong result",
                    (long unsigned int)buflen);
              showmpi ("expected:", a);
              showmpi ("     got:", b);
            }
          gcry_mpi_release (b);
        }

      /* Negative values are not supported by PGP, thus we don't have
         an samples.  */
      if (!gcry_mpi_is_neg (a))
        {
          err = gcry_mpi_scan (&b, GCRYMPI_FMT_PGP,
                               data[idx].a.pgp, data[idx].a.pgplen, &buflen);
          if (err)
            fail ("error scanning value %s as %s: %s\n",
                  valuestr, "PGP", gpg_strerror (err));
          else
            {
              if (gcry_mpi_cmp (a, b) || data[idx].a.pgplen != buflen)
                {
                  fail ("error scanning value %s from %s: %s (%lu)\n",
                        valuestr, "PGP", "wrong result",
                        (long unsigned int)buflen);
                  showmpi ("expected:", a);
                  showmpi ("     got:", b);
                }
              gcry_mpi_release (b);
            }
        }
    }

  gcry_mpi_release (a);
}


int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  negative_zero ();
  check_formats ();

  info ("All tests completed. Errors: %d\n", error_count);
  return error_count ? 1 : 0;
}
