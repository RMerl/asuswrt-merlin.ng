/* misc.c
 * Copyright (C) 1999, 2001, 2002, 2003, 2007,
 *               2008 Free Software Foundation, Inc.
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

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "g10lib.h"
#include "secmem.h"
#include "mpi.h"

static int verbosity_level = 0;

#ifndef HAVE_EXPLICIT_MEMSET
/* Prevent compiler from optimizing away the call to memset by accessing
   memset through volatile pointer. */
static void *(*volatile memset_ptr)(void *, int, size_t) = (void *)memset;
#endif

static void (*fatal_error_handler)(void*,int, const char*) = NULL;
static void *fatal_error_handler_value = 0;
static void (*log_handler)(void*,int, const char*, va_list) = NULL;
static void *log_handler_value = 0;

static const char *(*user_gettext_handler)( const char * ) = NULL;

void
_gcry_set_gettext_handler (const char *(*f)(const char*))
{
    user_gettext_handler = f;
}


const char *
_gcry_gettext( const char *key )
{
    if( user_gettext_handler )
	return user_gettext_handler( key );
    /* FIXME: switch the domain to gnupg and restore later */
    return key;
}

void
_gcry_set_fatalerror_handler( void (*fnc)(void*,int, const char*), void *value)
{
    fatal_error_handler_value = value;
    fatal_error_handler = fnc;
}

static void
write2stderr( const char *s )
{
  /* Dummy variable to silence gcc warning.  */
  int res = write( 2, s, strlen(s) );
  (void) res;
}

/*
 * This function is called for fatal errors.  A caller might want to
 * set his own handler because this function simply calls abort().
 */
void
_gcry_fatal_error (int rc, const char *text)
{
  if ( !text ) /* get a default text */
    text = gpg_strerror (rc);

  if (fatal_error_handler && !fips_mode () )
    fatal_error_handler (fatal_error_handler_value, rc, text);

  fips_signal_fatal_error (text);
  write2stderr("\nFatal error: ");
  write2stderr(text);
  write2stderr("\n");
  _gcry_secmem_term ();
  abort ();
}

void
_gcry_set_log_handler (void (*f)(void*,int, const char*, va_list), void *opaque)
{
    log_handler = f;
    log_handler_value = opaque;
}

void
_gcry_set_log_verbosity( int level )
{
    verbosity_level = level;
}

int
_gcry_log_verbosity( int level )
{
    return verbosity_level >= level;
}

/****************
 * This is our log function which prints all log messages to stderr or
 * using the function defined with gcry_set_log_handler().
 */
void
_gcry_logv( int level, const char *fmt, va_list arg_ptr )
{
  if (log_handler)
    log_handler (log_handler_value, level, fmt, arg_ptr);
  else
    {
      switch (level)
        {
        case GCRY_LOG_CONT:  break;
        case GCRY_LOG_INFO:  break;
        case GCRY_LOG_WARN:  break;
        case GCRY_LOG_ERROR: break;
        case GCRY_LOG_FATAL: fputs("Fatal: ",stderr ); break;
        case GCRY_LOG_BUG:   fputs("Ohhhh jeeee: ", stderr); break;
        case GCRY_LOG_DEBUG: fputs("DBG: ", stderr ); break;
        default: fprintf(stderr,"[Unknown log level %d]: ", level ); break;
	}
      vfprintf(stderr,fmt,arg_ptr) ;
    }

  if ( level == GCRY_LOG_FATAL || level == GCRY_LOG_BUG )
    {
      fips_signal_fatal_error ("internal error (fatal or bug)");
      _gcry_secmem_term ();
      abort ();
    }
}


void
_gcry_log( int level, const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( level, fmt, arg_ptr );
    va_end(arg_ptr);
}


#if defined(JNLIB_GCC_M_FUNCTION) || __STDC_VERSION__ >= 199901L
void
_gcry_bug( const char *file, int line, const char *func )
{
    _gcry_log( GCRY_LOG_BUG,
	     ("... this is a bug (%s:%d:%s)\n"), file, line, func );
    abort(); /* never called, but it makes the compiler happy */
}
void
_gcry_assert_failed (const char *expr, const char *file, int line,
                     const char *func)
{
  _gcry_log (GCRY_LOG_BUG,
             ("Assertion `%s' failed (%s:%d:%s)\n"), expr, file, line, func );
  abort(); /* Never called, but it makes the compiler happy. */
}
#else
void
_gcry_bug( const char *file, int line )
{
    _gcry_log( GCRY_LOG_BUG,
	     _("you found a bug ... (%s:%d)\n"), file, line);
    abort(); /* never called, but it makes the compiler happy */
}
void
_gcry_assert_failed (const char *expr, const char *file, int line)
{
  _gcry_log (GCRY_LOG_BUG,
             ("Assertion `%s' failed (%s:%d)\n"), expr, file, line);
  abort(); /* Never called, but it makes the compiler happy. */
}
#endif

void
_gcry_log_info( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_INFO, fmt, arg_ptr );
    va_end(arg_ptr);
}

void
_gcry_log_error( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_ERROR, fmt, arg_ptr );
    va_end(arg_ptr);
}


void
_gcry_log_fatal( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_FATAL, fmt, arg_ptr );
    va_end(arg_ptr);
    abort(); /* never called, but it makes the compiler happy */
}

void
_gcry_log_bug( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_BUG, fmt, arg_ptr );
    va_end(arg_ptr);
    abort(); /* never called, but it makes the compiler happy */
}

void
_gcry_log_debug( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_DEBUG, fmt, arg_ptr );
    va_end(arg_ptr);
}


void
_gcry_log_printf (const char *fmt, ...)
{
  va_list arg_ptr;

  if (fmt)
    {
      va_start( arg_ptr, fmt ) ;
      _gcry_logv (GCRY_LOG_CONT, fmt, arg_ptr);
      va_end(arg_ptr);
    }
}


/* Helper for _gcry_log_printhex and _gcry_log_printmpi.  */
static void
do_printhex (const char *text, const char *text2,
             const void *buffer, size_t length)
{
  int wrap = 0;
  int cnt = 0;

  if (text && *text)
    {
      wrap = 1;
      log_debug ("%s:%s", text, text2);
      if (text2[1] == '[' && length && buffer)
        {
          /* Start with a new line so that we get nice output for
             opaque MPIS:
               "value: [31 bit]"
               "        01020300"  */
          log_printf ("\n");
          text2 = " ";
          log_debug ("%*s  ", (int)strlen(text), "");
        }
    }
  if (length && buffer)
    {
      const unsigned char *p = buffer;
      for (; length--; p++)
        {
          log_printf ("%02x", *p);
          if (wrap && ++cnt == 32 && length)
            {
              cnt = 0;
              log_printf (" \\\n");
              log_debug ("%*s %*s",
                         (int)strlen(text), "", (int)strlen(text2), "");
            }
        }
    }
  if (text)
    log_printf ("\n");
}


/* Print a hexdump of BUFFER.  With TEXT of NULL print just the raw
   dump without any wrappping, with TEXT an empty string, print a
   trailing linefeed, otherwise print an entire debug line. */
void
_gcry_log_printhex (const char *text, const void *buffer, size_t length)
{
  do_printhex (text, " ", buffer, length);
}


/* Print MPI in hex notation.  To make clear that the output is an MPI
   a sign is always printed. With TEXT of NULL print just the raw dump
   without any wrapping, with TEXT an empty string, print a trailing
   linefeed, otherwise print an entire debug line. */
void
_gcry_log_printmpi (const char *text, gcry_mpi_t mpi)
{
  unsigned char *rawmpi;
  unsigned int rawmpilen;
  int sign;

  if (!mpi)
    do_printhex (text? text:" ", " (null)", NULL, 0);
  else if (mpi_is_opaque (mpi))
    {
      unsigned int nbits;
      const unsigned char *p;
      char prefix[30];

      p = mpi_get_opaque (mpi, &nbits);
      snprintf (prefix, sizeof prefix, " [%u bit]", nbits);
      do_printhex (text? text:" ", prefix, p, (nbits+7)/8);
    }
  else
    {
      rawmpi = _gcry_mpi_get_buffer (mpi, 0, &rawmpilen, &sign);
      if (!rawmpi)
        do_printhex (text? text:" ", " [out of core]", NULL, 0);
      else
        {
          if (!rawmpilen)
            do_printhex (text, sign? "-":"+", "", 1);
          else
            do_printhex (text, sign? "-":"+", rawmpi, rawmpilen);
          xfree (rawmpi);
        }
    }
}


static int
count_closing_parens (const char *p)
{
  int count = 0;

  for (; *p; p++)
    if (*p == ')')
      count++;
    else if (!strchr ("\n \t", *p))
      return 0;

  return count;
}


/* Print SEXP in human readabale format.  With TEXT of NULL print just the raw
   dump without any wrappping, with TEXT an empty string, print a
   trailing linefeed, otherwise print the full debug output. */
void
_gcry_log_printsxp (const char *text, gcry_sexp_t sexp)
{
  int with_lf = 0;

  if (text && *text)
    {
      if ((with_lf = !!strchr (text, '\n')))
        log_debug ("%s", text);
      else
        log_debug ("%s: ", text);
    }
  if (sexp)
    {
      int any = 0;
      int n_closing;
      char *buf, *pend;
      const char *p;
      size_t size;

      size = sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, NULL, 0);
      p = buf = xmalloc (size);
      sexp_sprint (sexp, GCRYSEXP_FMT_ADVANCED, buf, size);

      do
        {
          if (any && !with_lf)
            log_debug ("%*s  ", text?(int)strlen(text):0, "");
          else
            any = 1;
          pend = strchr (p, '\n');
          size = pend? (pend - p) : strlen (p);
          if (with_lf)
            log_debug ("%.*s", (int)size, p);
          else
            log_printf ("%.*s", (int)size, p);
          if (pend)
            p = pend + 1;
          else
            p += size;
          n_closing = count_closing_parens (p);
          if (n_closing)
            {
              while (n_closing--)
                log_printf (")");
              p = "";
            }
          log_printf ("\n");
        }
      while (*p);
      xfree (buf);
    }
  else if (text)
    log_printf ("\n");
}


/*
 * Tokenize STRING using the set of delimiters in DELIM.  Leading
 * white spaces are removed from all tokens.  The caller must xfree
 * the result.
 *
 * Returns: A malloced and NULL delimited array with the tokens.  On
 *          memory error NULL is returned and ERRNO is set.
 */
char **
_gcry_strtokenize (const char *string, const char *delim)
{
  const char *s;
  size_t fields;
  size_t bytes, n;
  char *buffer;
  char *p, *px, *pend;
  char **result;
  char const ws[] = " \t\v\f\r\n";

  if (!delim)
    delim = ws;

  /* Count the number of fields.  */
  for (fields = 1, s = strpbrk (string, delim); s; s = strpbrk (s + 1, delim))
    fields++;
  fields++; /* Add one for the terminating NULL.  */

  /* Allocate an array for all fields, a terminating NULL, and space
     for a copy of the string.  */
  bytes = fields * sizeof *result;
  if (bytes / sizeof *result != fields)
    {
      gpg_err_set_errno (ENOMEM);
      return NULL;
    }
  n = strlen (string) + 1;
  bytes += n;
  if (bytes < n)
    {
      gpg_err_set_errno (ENOMEM);
      return NULL;
    }
  result = xtrymalloc (bytes);
  if (!result)
    return NULL;
  buffer = (char*)(result + fields);

  /* Copy and parse the string.  */
  strcpy (buffer, string);
  for (n = 0, p = buffer; (pend = strpbrk (p, delim)); p = pend + 1)
    {
      *pend = 0;
      while (strchr (ws, *(byte*)p))
        p++;
      for (px = pend - 1; px >= p && strchr (ws, *(byte*)px); px--)
        *px = 0;
      result[n++] = p;
    }
  while (*p && strchr (ws, *(byte*)p))
    p++;
  for (px = p + strlen (p) - 1; px >= p && strchr (ws, *(byte*)px); px--)
    *px = 0;
  /* Traling spaces may result in an empty field.  We do not want to
     store that.  */
  result[n++] = *p? p : NULL;
  result[n] = NULL;

  gcry_assert ((char*)(result + n + 1) == buffer);

  return result;
}


void
_gcry_fast_wipememory (void *ptr, size_t len)
{
  /* Note: This function is called from wipememory/wipememory2 only if LEN
     is large or unknown at compile time. New wipe function alternatives
     need to be checked before adding to this function. New implementations
     need to be faster than wipememory/wipememory2 macros in 'g10lib.h'.

     Following implementations were found to have suboptimal performance:

     - [_WIN32/mingw32] SecureZeroMemory; Inline function, equivalent to
       volatile byte buffer set: while(buflen--) (volatile char *)(buf++)=set;
   */
#ifdef HAVE_EXPLICIT_BZERO
  explicit_bzero (ptr, len);
#elif defined(HAVE_EXPLICIT_MEMSET)
  explicit_memset (ptr, 0, len);
#else
  memset_ptr (ptr, 0, len);
#endif
}


void
_gcry_fast_wipememory2 (void *ptr, int set, size_t len)
{
#ifdef HAVE_EXPLICIT_MEMSET
  explicit_memset (ptr, set, len);
#else
#ifdef HAVE_EXPLICIT_BZERO
  if (set == 0)
    {
      explicit_bzero (ptr, len);
      return;
    }
#endif

  memset_ptr (ptr, set, len);
#endif
}


void NOINLINE_FUNC
__gcry_burn_stack (unsigned int bytes)
{
#ifdef HAVE_VLA
  /* (bytes == 0 ? 1 : bytes) == (!bytes + bytes) */
  unsigned int buflen = ((!bytes + bytes) + 63) & ~63;
  char buf[buflen];

  _gcry_fast_wipememory (buf, buflen);
#else
  volatile char buf[64];

  _gcry_fast_wipememory (buf, sizeof buf);

  if (bytes > sizeof buf)
      _gcry_burn_stack (bytes - sizeof buf);
#endif
}

#ifndef HAVE_GCC_ASM_VOLATILE_MEMORY
void
__gcry_burn_stack_dummy (void)
{
}
#endif

void
_gcry_divide_by_zero (void)
{
    gpg_err_set_errno (EDOM);
    _gcry_fatal_error (gpg_err_code_from_errno (errno), "divide by zero");
}
