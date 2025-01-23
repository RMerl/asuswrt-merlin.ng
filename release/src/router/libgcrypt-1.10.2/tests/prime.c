/* prime.c - part of the Libgcrypt test suite.
   Copyright (C) 2001, 2002, 2003, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PGM "prime"
#include "t-common.h"

static void
check_primes (void)
{
  gcry_error_t err = GPG_ERR_NO_ERROR;
  gcry_mpi_t *factors = NULL;
  gcry_mpi_t prime = NULL;
  gcry_mpi_t g;
  unsigned int i = 0;
  struct prime_spec
  {
    unsigned int prime_bits;
    unsigned int factor_bits;
    unsigned int flags;
  } prime_specs[] =
    {
      { 1024, 100, GCRY_PRIME_FLAG_SPECIAL_FACTOR },
      { 128, 0, 0 },
      { 0 },
    };

  for (i = 0; prime_specs[i].prime_bits; i++)
    {
      err = gcry_prime_generate (&prime,
				 prime_specs[i].prime_bits,
				 prime_specs[i].factor_bits,
				 &factors,
				 NULL, NULL,
				 GCRY_WEAK_RANDOM,
				 prime_specs[i].flags);
      assert (! err);
      if (verbose)
        {
          fprintf (stderr, "test %d: p = ", i);
          gcry_mpi_dump (prime);
          putc ('\n', stderr);
        }

      err = gcry_prime_check (prime, 0);
      assert (! err);

      err = gcry_prime_group_generator (&g, prime, factors, NULL);
      assert (!err);
      gcry_prime_release_factors (factors); factors = NULL;

      if (verbose)
        {
          fprintf (stderr, "     %d: g = ", i);
          gcry_mpi_dump (g);
          putc ('\n', stderr);
        }
      gcry_mpi_release (g);


      gcry_mpi_add_ui (prime, prime, 1);
      err = gcry_prime_check (prime, 0);
      assert (err);
      gcry_mpi_release (prime); prime = NULL;
    }
}


/* Print an MPI S-expression.  */
static void
print_mpi (const char *name, gcry_mpi_t a)
{
  gcry_error_t err;
  unsigned char *buf;
  int writerr = 0;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, &buf, NULL, a);
  if (err)
    die ("gcry_mpi_aprint failed: %s\n", gcry_strerror (err));

  printf ("  (%s #%s#)\n", name, buf);
  if (ferror (stdout))
    writerr++;
  if (!writerr && fflush (stdout) == EOF)
    writerr++;
  if (writerr)
    die ("writing output failed\n");
  gcry_free (buf);
}


/* Create the key for our public standard dummy CA.  */
static void
create_42prime (void)
{
  gcry_error_t err;
  char string[128*2+1];
  int i;
  gcry_mpi_t start = NULL;
  gcry_mpi_t p, q, n, t1, t2, phi, f, g, e, d, u;


  /* Our start value is a string of 0x42 values, with the exception
     that the two high order bits are set.  This is to resemble the
     way Lingcrypt generates RSA primes.  */
  for (i=0; i < 128;)
    {
      string[i++] = '4';
      string[i++] = '2';
    }
  string[i] = 0;
  string[0] = 'C';

  err = gcry_mpi_scan (&start, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    die ("gcry_mpi_scan failed: %s\n", gcry_strerror (err));
  fputs ("start:", stderr); gcry_mpi_dump (start); putc ('\n', stderr);

  /* Generate two primes with p < q.  We take the first primes below
     and above a start value. */
  p = gcry_mpi_copy (start);
  gcry_mpi_sub_ui (p, p, 1);
  while (gcry_prime_check (p, 0))
    gcry_mpi_sub_ui (p, p, 2);
  fputs ("    p:", stderr); gcry_mpi_dump (p); putc ('\n', stderr);
  q = gcry_mpi_copy (start);
  gcry_mpi_add_ui (q, q, 1);
  while (gcry_prime_check (q, 0))
    gcry_mpi_add_ui (q, q, 2);
  fputs ("    q:", stderr); gcry_mpi_dump (q); putc ('\n', stderr);

  /* Compute the modulus.  */
  n = gcry_mpi_new (1024);
  gcry_mpi_mul (n, p, q);
  fputs ("    n:", stderr); gcry_mpi_dump (n); putc ('\n', stderr);
  if (gcry_mpi_get_nbits (n) != 1024)
    die ("Oops: the size of N is not 1024 but %u\n", gcry_mpi_get_nbits (n));

  /* Calculate Euler totient: phi = (p-1)(q-1) */
  t1 = gcry_mpi_new (0);
  t2 = gcry_mpi_new (0);
  phi = gcry_mpi_new (0);
  g   = gcry_mpi_new (0);
  f   = gcry_mpi_new (0);
  gcry_mpi_sub_ui (t1, p, 1);
  gcry_mpi_sub_ui (t2, q, 1);
  gcry_mpi_mul (phi, t1, t2);
  gcry_mpi_gcd (g, t1, t2);
  gcry_mpi_div (f, NULL, phi, g, -1);

  /* Check the public exponent.  */
  e = gcry_mpi_set_ui (NULL, 65537);
  if (!gcry_mpi_gcd (t1, e, phi))
    die ("Oops: E is not a generator\n");
  fputs ("    e:", stderr); gcry_mpi_dump (e); putc ('\n', stderr);

  /* Compute the secret key:  d = e^-1 mod phi */
  d = gcry_mpi_new (0);
  gcry_mpi_invm (d, e, f );
  fputs ("    d:", stderr); gcry_mpi_dump (d); putc ('\n', stderr);

  /* Compute the inverse of p and q. */
  u = gcry_mpi_new (0);
  gcry_mpi_invm (u, p, q);
  fputs ("    u:", stderr); gcry_mpi_dump (u); putc ('\n', stderr);

  /* Print the S-expression.  */
  fputs ("(private-key\n (rsa\n", stdout);
  print_mpi ("n", n);
  print_mpi ("e", e);
  print_mpi ("d", d);
  print_mpi ("p", p);
  print_mpi ("q", q);
  print_mpi ("u", u);
  fputs ("))\n", stdout);

  gcry_mpi_release (p);
  gcry_mpi_release (q);
  gcry_mpi_release (n);
  gcry_mpi_release (t1);
  gcry_mpi_release (t2);
  gcry_mpi_release (phi);
  gcry_mpi_release (f);
  gcry_mpi_release (g);
  gcry_mpi_release (e);
  gcry_mpi_release (d);
  gcry_mpi_release (u);
}




int
main (int argc, char **argv)
{
  int mode42 = 0;

  if ((argc > 1) && (! strcmp (argv[1], "--verbose")))
    verbose = 1;
  else if ((argc > 1) && (! strcmp (argv[1], "--debug")))
    verbose = debug = 1;
  else if ((argc > 1) && (! strcmp (argv[1], "--42")))
    verbose = debug = mode42 = 1;

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (! gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  if (mode42)
    create_42prime ();
  else
    check_primes ();

  return 0;
}
