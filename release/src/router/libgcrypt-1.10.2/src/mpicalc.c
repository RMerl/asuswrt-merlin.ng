/* mpicalc.c - Simple RPN calculator using gcry_mpi functions
 * Copyright (C) 1997, 1998, 1999, 2004, 2006, 2013  Werner Koch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
   This program is a simple RPN calculator which was originally used
   to develop the mpi functions of GnuPG.  Values must be given in
   hex.  Operation is like dc(1) except that the input/output radix is
   always 16 and you can use a '-' to prefix a negative number.
   Addition operators: ++ and --.  All operators must be delimited by
   a blank.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _GCRYPT_IN_LIBGCRYPT
# undef _GCRYPT_IN_LIBGCRYPT
# include "gcrypt.h"
#else
# include <gcrypt.h>
#endif


#define MPICALC_VERSION "2.0"
#define NEED_LIBGCRYPT_VERSION "1.6.0"

#define STACKSIZE  500
static gcry_mpi_t stack[STACKSIZE];
static int stackidx;


static int
scan_mpi (gcry_mpi_t retval, const char *string)
{
  gpg_error_t err;
  gcry_mpi_t val;

  err = gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    {
      fprintf (stderr, "scanning input failed: %s\n", gpg_strerror (err));
      return -1;
    }
  mpi_set (retval, val);
  mpi_release (val);
  return 0;
}


static void
print_mpi (gcry_mpi_t a)
{
  gpg_error_t err;
  char *buf;
  void *bufaddr = &buf;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (err)
    fprintf (stderr, "[error printing number: %s]\n", gpg_strerror (err));
  else
    {
      fputs (buf, stdout);
      gcry_free (buf);
    }
}



static void
do_add (void)
{
  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_add (stack[stackidx - 2], stack[stackidx - 2], stack[stackidx - 1]);
  stackidx--;
}

static void
do_sub (void)
{
  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_sub (stack[stackidx - 2], stack[stackidx - 2], stack[stackidx - 1]);
  stackidx--;
}

static void
do_inc (void)
{
  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_add_ui (stack[stackidx - 1], stack[stackidx - 1], 1);
}

static void
do_dec (void)
{
  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  /* mpi_sub_ui( stack[stackidx-1], stack[stackidx-1], 1 ); */
}

static void
do_mul (void)
{
  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_mul (stack[stackidx - 2], stack[stackidx - 2], stack[stackidx - 1]);
  stackidx--;
}

static void
do_mulm (void)
{
  if (stackidx < 3)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_mulm (stack[stackidx - 3], stack[stackidx - 3],
	    stack[stackidx - 2], stack[stackidx - 1]);
  stackidx -= 2;
}

static void
do_div (void)
{
  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_fdiv (stack[stackidx - 2], NULL,
            stack[stackidx - 2], stack[stackidx - 1]);
  stackidx--;
}

static void
do_rem (void)
{
  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_mod (stack[stackidx - 2],
           stack[stackidx - 2], stack[stackidx - 1]);
  stackidx--;
}

static void
do_powm (void)
{
  gcry_mpi_t a;
  if (stackidx < 3)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  a = mpi_new (0);
  mpi_powm (a, stack[stackidx - 3], stack[stackidx - 2], stack[stackidx - 1]);
  mpi_release (stack[stackidx - 3]);
  stack[stackidx - 3] = a;
  stackidx -= 2;
}

static void
do_inv (void)
{
  gcry_mpi_t a;

  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  a = mpi_new (0);
  mpi_invm (a, stack[stackidx - 2], stack[stackidx - 1]);
  mpi_set (stack[stackidx - 2], a);
  mpi_release (a);
  stackidx--;
}

static void
do_gcd (void)
{
  gcry_mpi_t a;

  if (stackidx < 2)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  a = mpi_new (0);
  mpi_gcd (a, stack[stackidx - 2], stack[stackidx - 1]);
  mpi_set (stack[stackidx - 2], a);
  mpi_release (a);
  stackidx--;
}

static void
do_lshift (void)
{
  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_lshift (stack[stackidx - 1], stack[stackidx - 1], 1);
}

static void
do_rshift (void)
{
  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  mpi_rshift (stack[stackidx - 1], stack[stackidx - 1], 1);
}

static void
do_nbits (void)
{
  unsigned int n;

  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  n = mpi_get_nbits (stack[stackidx - 1]);
  mpi_set_ui (stack[stackidx - 1], n);
}


static void
do_primecheck (void)
{
  gpg_error_t err;

  if (stackidx < 1)
    {
      fputs ("stack underflow\n", stderr);
      return;
    }
  err = gcry_prime_check (stack[stackidx - 1], 0);
  mpi_set_ui (stack[stackidx - 1], !err);
  if (err && gpg_err_code (err) != GPG_ERR_NO_PRIME)
    fprintf (stderr, "checking prime failed: %s\n", gpg_strerror (err));
}


static int
my_getc (void)
{
  static int shown;
  int c;

  for (;;)
    {
      if ((c = getc (stdin)) == EOF)
        return EOF;
      if (!(c & 0x80))
        return c;

      if (!shown)
        {
          shown = 1;
          fputs ("note: Non ASCII characters are ignored\n", stderr);
        }
    }
}


static void
print_help (void)
{
  fputs ("+   add           [0] := [1] + [0]          {-1}\n"
         "-   subtract      [0] := [1] - [0]          {-1}\n"
         "*   multiply      [0] := [1] * [0]          {-1}\n"
         "/   divide        [0] := [1] / [0]          {-1}\n"
         "%   modulo        [0] := [1] % [0]          {-1}\n"
         "<   left shift    [0] := [0] << 1           {0}\n"
         ">   right shift   [0] := [0] >> 1           {0}\n"
         "++  increment     [0] := [0]++              {0}\n"
         "--  decrement     [0] := [0]--              {0}\n"
         "m   multiply mod  [0] := [2] * [1] mod [0]  {-2}\n"
         "^   power mod     [0] := [2] ^ [1] mod [0]  {-2}\n"
         "I   inverse mod   [0] := [1]^-1 mod [0]     {-1}\n"
         "G   gcd           [0] := gcd([1],[0])       {-1}\n"
         "i   remove item   [0] := [1]                {-1}\n"
         "d   dup item      [-1] := [0]               {+1}\n"
         "r   reverse       [0] := [1], [1] := [0]    {0}\n"
         "b   # of bits     [0] := nbits([0])         {0}\n"
         "P   prime check   [0] := is_prime([0])?1:0  {0}\n"
         "c   clear stack\n"
         "p   print top item\n"
         "f   print the stack\n"
         "#   ignore until end of line\n"
         "?   print this help\n"
         , stdout);
}



int
main (int argc, char **argv)
{
  const char *pgm;
  int last_argc = -1;
  int print_config = 0;
  int i, c;
  int state = 0;
  char strbuf[4096];
  int stridx = 0;

  if (argc)
    {
      pgm = strrchr (*argv, '/');
      if (pgm)
        pgm++;
      else
        pgm = *argv;
      argc--; argv++;
    }
  else
    pgm = "?";

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--version")
               || !strcmp (*argv, "--help"))
        {
          printf ("%s " MPICALC_VERSION "\n"
                  "libgcrypt %s\n"
                  "Copyright (C) 1997, 2013  Werner Koch\n"
                  "License LGPLv2.1+: GNU LGPL version 2.1 or later "
                  "<http://gnu.org/licenses/old-licenses/lgpl-2.1.html>\n"
                  "This is free software: you are free to change and "
                  "redistribute it.\n"
                  "There is NO WARRANTY, to the extent permitted by law.\n"
                  "\n"
                  "Syntax: mpicalc [options]\n"
                  "Simple interactive big integer RPN calculator\n"
                  "\n"
                  "Options:\n"
                  "  --version           print version information\n"
                  "  --print-config      print the Libgcrypt config\n"
                  "  --disable-hwf NAME  disable feature NAME\n",
                  pgm, gcry_check_version (NULL));
          exit (0);
        }
      else if (!strcmp (*argv, "--print-config"))
        {
          argc--; argv++;
          print_config = 1;
        }
      else if (!strcmp (*argv, "--disable-hwf"))
        {
          argc--; argv++;
          if (argc)
            {
              if (gcry_control (GCRYCTL_DISABLE_HWF, *argv, NULL))
                fprintf (stderr, "%s: unknown hardware feature `%s'"
                         " - option ignored\n", pgm, *argv);
              argc--; argv++;
            }
        }
    }

  if (argc)
    {
      fprintf (stderr, "usage: %s [options]  (--help for help)\n", pgm);
      exit (1);
    }

  if (!gcry_check_version (NEED_LIBGCRYPT_VERSION))
    {
      fprintf (stderr, "%s: Libgcrypt is too old (need %s, have %s)\n",
               pgm, NEED_LIBGCRYPT_VERSION, gcry_check_version (NULL) );
      exit (1);
    }
  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  if (print_config)
    {
      gcry_control (GCRYCTL_PRINT_CONFIG, stdout);
      exit (0);
    }

  for (i = 0; i < STACKSIZE; i++)
    stack[i] = NULL;
  stackidx = 0;

  while ((c = my_getc ()) != EOF)
    {
      if (!state) /* waiting */
	{
	  if (isdigit (c))
	    {
	      state = 1;
	      ungetc (c, stdin);
	      strbuf[0] = '0';
	      strbuf[1] = 'x';
	      stridx = 2;
	    }
	  else if (isspace (c))
	    ;
	  else
	    {
	      switch (c)
		{
                case '#':
                  state = 2;
                  break;
		case '+':
		  if ((c = my_getc ()) == '+')
		    do_inc ();
		  else
		    {
		      ungetc (c, stdin);
		      do_add ();
		    }
		  break;
                case '-':
		  if ((c = my_getc ()) == '-')
		    do_dec ();
		  else if (isdigit (c)
                           || (c >= 'A' && c <= 'F')
                           || (c >= 'a' && c <= 'f'))
		    {
		      state = 1;
		      ungetc (c, stdin);
		      strbuf[0] = '-';
		      strbuf[1] = '0';
		      strbuf[2] = 'x';
		      stridx = 3;
		    }
		  else
		    {
		      ungetc (c, stdin);
		      do_sub ();
		    }
		  break;
		case '*':
		  do_mul ();
		  break;
		case 'm':
		  do_mulm ();
		  break;
		case '/':
		  do_div ();
		  break;
		case '%':
		  do_rem ();
		  break;
		case '^':
		  do_powm ();
		  break;
		case '<':
		  do_lshift ();
		  break;
		case '>':
		  do_rshift ();
		  break;
		case 'I':
		  do_inv ();
		  break;
		case 'G':
		  do_gcd ();
		  break;
		case 'i':	/* dummy */
		  if (!stackidx)
		    fputs ("stack underflow\n", stderr);
		  else
		    {
		      mpi_release (stack[stackidx - 1]);
		      stackidx--;
		    }
		  break;
		case 'd':	/* duplicate the tos */
		  if (!stackidx)
		    fputs ("stack underflow\n", stderr);
		  else if (stackidx < STACKSIZE)
		    {
		      mpi_release (stack[stackidx]);
		      stack[stackidx] = mpi_copy (stack[stackidx - 1]);
		      stackidx++;
		    }
		  else
		    fputs ("stack overflow\n", stderr);
		  break;
		case 'r':	/* swap top elements */
		  if (stackidx < 2)
		    fputs ("stack underflow\n", stderr);
		  else if (stackidx < STACKSIZE)
		    {
		      gcry_mpi_t tmp = stack[stackidx-1];
                      stack[stackidx-1] = stack[stackidx - 2];
                      stack[stackidx-2] = tmp;
		    }
		  break;
                case 'b':
                  do_nbits ();
                  break;
                case 'P':
                  do_primecheck ();
                  break;
		case 'c':
		  for (i = 0; i < stackidx; i++)
                    {
                      mpi_release (stack[i]); stack[i] = NULL;
                    }
		  stackidx = 0;
		  break;
		case 'p':	/* print the tos */
		  if (!stackidx)
		    puts ("stack is empty");
		  else
		    {
		      print_mpi (stack[stackidx - 1]);
		      putchar ('\n');
		    }
		  break;
		case 'f':	/* print the stack */
		  for (i = stackidx - 1; i >= 0; i--)
		    {
		      printf ("[%2d]: ", i);
		      print_mpi (stack[i]);
		      putchar ('\n');
		    }
		  break;
                case '?':
                  print_help ();
                  break;
		default:
		  fputs ("invalid operator\n", stderr);
		}
	    }
	}
      else if (state == 1) /* In a number. */
	{
	  if (!isxdigit (c))
	    {
              /* Store the number */
	      state = 0;
	      ungetc (c, stdin);
	      if (stridx < sizeof strbuf)
		strbuf[stridx] = 0;

	      if (stackidx < STACKSIZE)
		{
		  if (!stack[stackidx])
		    stack[stackidx] = mpi_new (0);
		  if (scan_mpi (stack[stackidx], strbuf))
		    fputs ("invalid number\n", stderr);
		  else
		    stackidx++;
		}
	      else
		fputs ("stack overflow\n", stderr);
	    }
	  else
	    { /* Store a digit.  */
	      if (stridx < sizeof strbuf - 1)
		strbuf[stridx++] = c;
	      else if (stridx == sizeof strbuf - 1)
		{
		  strbuf[stridx] = 0;
		  fputs ("input too large - truncated\n", stderr);
		  stridx++;
		}
	    }
	}
      else if (state == 2) /* In a comment. */
        {
          if (c == '\n')
            state = 0;
        }

    }

  for (i = 0; i < stackidx; i++)
    mpi_release (stack[i]);
  return 0;
}
