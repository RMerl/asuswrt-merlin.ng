/* sexp.c  -  S-Expression handling
 * Copyright (C) 1999, 2000, 2001, 2002, 2003,
 *               2004, 2006, 2007, 2008, 2011  Free Software Foundation, Inc.
 * Copyright (C) 2013, 2014, 2017 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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
 * SPDX-License-Identifier: LGPL-2.1+
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#define GCRYPT_NO_MPI_MACROS 1
#include "g10lib.h"


/* Notes on the internal memory layout.

   We store an S-expression as one memory buffer with tags, length and
   value.  The simplest list would thus be:

   /----------+----------+---------+------+-----------+----------\
   | open_tag | data_tag | datalen | data | close_tag | stop_tag |
   \----------+----------+---------+------+-----------+----------/

   Expressed more compact and with an example:

   /----+----+----+---+----+----\
   | OT | DT | DL | D | CT | ST |  "(foo)"
   \----+----+----+---+----+----/

   The open tag must always be the first tag of a list as requires by
   the S-expression specs.  At least data element (data_tag, datalen,
   data) is required as well.  The close_tag finishes the list and
   would actually be sufficient.  For fail-safe reasons a final stop
   tag is always the last byte in a buffer; it has a value of 0 so
   that string function accidentally applied to an S-expression will
   never access unallocated data.  We do not support display hints and
   thus don't need to represent them.  A list may have more an
   arbitrary number of data elements but at least one is required.
   The length of each data must be greater than 0 and has a current
   limit to 65535 bytes (by means of the DATALEN type).

   A list with two data elements:

   /----+----+----+---+----+----+---+----+----\
   | OT | DT | DL | D | DT | DL | D | CT | ST |  "(foo bar)"
   \----+----+----+---+----+----+---+----+----/

   In the above example both DL fields have a value of 3.
   A list of a list with one data element:

   /----+----+----+----+---+----+----+----\
   | OT | OT | DT | DL | D | CT | CT | ST |  "((foo))"
   \----+----+----+----+---+----+----+----/

   A list with one element followed by another list:

   /----+----+----+---+----+----+----+---+----+----+----\
   | OT | DT | DL | D | OT | DT | DL | D | CT | CT | ST |  "(foo (bar))"
   \----+----+----+---+----+----+----+---+----+----+----/

 */

typedef unsigned short DATALEN;

struct gcry_sexp
{
  byte d[1];
};

#define ST_STOP  0
#define ST_DATA  1  /* datalen follows */
/*#define ST_HINT  2   datalen follows (currently not used) */
#define ST_OPEN  3
#define ST_CLOSE 4

/* The atoi macros assume that the buffer has only valid digits.  */
#define atoi_1(p)   (*(p) - '0' )
#define xtoi_1(p)   (*(p) <= '9'? (*(p)- '0'): \
                     *(p) <= 'F'? (*(p)-'A'+10):(*(p)-'a'+10))
#define xtoi_2(p)   ((xtoi_1(p) * 16) + xtoi_1((p)+1))

#define TOKEN_SPECIALS  "-./_:*+="

static gcry_err_code_t
do_vsexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
                const char *buffer, size_t length, int argflag,
                void **arg_list, va_list arg_ptr);

static gcry_err_code_t
do_sexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
               const char *buffer, size_t length, int argflag,
               void **arg_list, ...);

/* Return true if P points to a byte containing a whitespace according
   to the S-expressions definition. */
#undef whitespacep
static GPG_ERR_INLINE int
whitespacep (const char *p)
{
  switch (*p)
    {
    case ' ': case '\t': case '\v': case '\f': case '\r': case '\n': return 1;
    default: return 0;
    }
}


#if 0
static void
dump_mpi( gcry_mpi_t a )
{
    char buffer[1000];
    size_t n = 1000;

    if( !a )
	fputs("[no MPI]", stderr );
    else if( gcry_mpi_print( GCRYMPI_FMT_HEX, buffer, &n, a ) )
	fputs("[MPI too large to print]", stderr );
    else
	fputs( buffer, stderr );
}
#endif

static void
dump_string (const byte *p, size_t n, int delim )
{
  for (; n; n--, p++ )
    {
      if ((*p & 0x80) || iscntrl( *p ) || *p == delim )
        {
          if( *p == '\n' )
            log_printf ("\\n");
          else if( *p == '\r' )
            log_printf ("\\r");
          else if( *p == '\f' )
            log_printf ("\\f");
          else if( *p == '\v' )
            log_printf ("\\v");
	    else if( *p == '\b' )
              log_printf ("\\b");
          else if( !*p )
            log_printf ("\\0");
          else
            log_printf ("\\x%02x", *p );
	}
      else
        log_printf ("%c", *p);
    }
}


void
_gcry_sexp_dump (const gcry_sexp_t a)
{
  const byte *p;
  int indent = 0;
  int type;

  if (!a)
    {
      log_printf ( "[nil]\n");
      return;
    }

  p = a->d;
  while ( (type = *p) != ST_STOP )
    {
      p++;
      switch ( type )
        {
        case ST_OPEN:
          log_printf ("%*s[open]\n", 2*indent, "");
          indent++;
          break;
        case ST_CLOSE:
          if( indent )
            indent--;
          log_printf ("%*s[close]\n", 2*indent, "");
          break;
        case ST_DATA: {
          DATALEN n;
          memcpy ( &n, p, sizeof n );
          p += sizeof n;
          log_printf ("%*s[data=\"", 2*indent, "" );
          dump_string (p, n, '\"' );
          log_printf ("\"]\n");
          p += n;
        }
        break;
        default:
          log_printf ("%*s[unknown tag %d]\n", 2*indent, "", type);
          break;
	}
    }
}


/* Pass list through except when it is an empty list - in that case
 * return NULL and release the passed list.  This is used to make sure
 * that no forbidden empty lists are created.
 */
static gcry_sexp_t
normalize ( gcry_sexp_t list )
{
  unsigned char *p;

  if ( !list )
    return NULL;
  p = list->d;
  if ( *p == ST_STOP )
    {
      /* this is "" */
      sexp_release ( list );
      return NULL;
    }
  if ( *p == ST_OPEN && p[1] == ST_CLOSE )
    {
      /* this is "()" */
      sexp_release ( list );
      return NULL;
    }

  return list;
}

/* Create a new S-expression object by reading LENGTH bytes from
   BUFFER, assuming it is canonical encoded or autodetected encoding
   when AUTODETECT is set to 1.  With FREEFNC not NULL, ownership of
   the buffer is transferred to the newly created object.  FREEFNC
   should be the freefnc used to release BUFFER; there is no guarantee
   at which point this function is called; most likey you want to use
   free() or gcry_free().

   Passing LENGTH and AUTODETECT as 0 is allowed to indicate that
   BUFFER points to a valid canonical encoded S-expression.  A LENGTH
   of 0 and AUTODETECT 1 indicates that buffer points to a
   null-terminated string.

   This function returns 0 and and the pointer to the new object in
   RETSEXP or an error code in which case RETSEXP is set to NULL.  */
gcry_err_code_t
_gcry_sexp_create (gcry_sexp_t *retsexp, void *buffer, size_t length,
                  int autodetect, void (*freefnc)(void*) )
{
  gcry_err_code_t errcode;
  gcry_sexp_t se;

  if (!retsexp)
    return GPG_ERR_INV_ARG;
  *retsexp = NULL;
  if (autodetect < 0 || autodetect > 1 || !buffer)
    return GPG_ERR_INV_ARG;

  if (!length && !autodetect)
    { /* What a brave caller to assume that there is really a canonical
         encoded S-expression in buffer */
      length = _gcry_sexp_canon_len (buffer, 0, NULL, &errcode);
      if (!length)
        return errcode;
    }
  else if (!length && autodetect)
    { /* buffer is a string */
      length = strlen ((char *)buffer);
    }

  errcode = do_sexp_sscan (&se, NULL, buffer, length, 0, NULL);
  if (errcode)
    return errcode;

  *retsexp = se;
  if (freefnc)
    {
      /* For now we release the buffer immediately.  As soon as we
         have changed the internal represenation of S-expression to
         the canoncial format - which has the advantage of faster
         parsing - we will use this function as a closure in our
         GCRYSEXP object and use the BUFFER directly.  */
      freefnc (buffer);
    }
  return 0;
}

/* Same as gcry_sexp_create but don't transfer ownership */
gcry_err_code_t
_gcry_sexp_new (gcry_sexp_t *retsexp, const void *buffer, size_t length,
               int autodetect)
{
  return _gcry_sexp_create (retsexp, (void *)buffer, length, autodetect, NULL);
}


/****************
 * Release resource of the given SEXP object.
 */
void
_gcry_sexp_release( gcry_sexp_t sexp )
{
  if (sexp)
    {
      if (_gcry_is_secure (sexp))
        {
          /* Extra paranoid wiping. */
          const byte *p = sexp->d;
          int type;

          while ( (type = *p) != ST_STOP )
            {
              p++;
              switch ( type )
                {
                case ST_OPEN:
                  break;
                case ST_CLOSE:
                  break;
                case ST_DATA:
                  {
                    DATALEN n;
                    memcpy ( &n, p, sizeof n );
                    p += sizeof n;
                    p += n;
                  }
                  break;
                default:
                  break;
                }
            }
          wipememory (sexp->d, p - sexp->d);
        }
      xfree ( sexp );
    }
}


/****************
 * Make a pair from lists a and b, don't use a or b later on.
 * Special behaviour:  If one is a single element list we put the
 * element straight into the new pair.
 */
gcry_sexp_t
_gcry_sexp_cons( const gcry_sexp_t a, const gcry_sexp_t b )
{
  (void)a;
  (void)b;

  /* NYI: Implementation should be quite easy with our new data
     representation */
  BUG ();
  return NULL;
}


/****************
 * Make a list from all items in the array the end of the array is marked
 * with a NULL.
 */
gcry_sexp_t
_gcry_sexp_alist( const gcry_sexp_t *array )
{
  (void)array;

  /* NYI: Implementation should be quite easy with our new data
     representation. */
  BUG ();
  return NULL;
}

/****************
 * Make a list from all items, the end of list is indicated by a NULL
 */
gcry_sexp_t
_gcry_sexp_vlist( const gcry_sexp_t a, ... )
{
  (void)a;
  /* NYI: Implementation should be quite easy with our new data
     representation. */
  BUG ();
  return NULL;
}


/****************
 * Append n to the list a
 * Returns: a new list (which maybe a)
 */
gcry_sexp_t
_gcry_sexp_append( const gcry_sexp_t a, const gcry_sexp_t n )
{
  (void)a;
  (void)n;
  /* NYI: Implementation should be quite easy with our new data
     representation. */
  BUG ();
  return NULL;
}

gcry_sexp_t
_gcry_sexp_prepend( const gcry_sexp_t a, const gcry_sexp_t n )
{
  (void)a;
  (void)n;
  /* NYI: Implementation should be quite easy with our new data
     representation. */
  BUG ();
  return NULL;
}



/****************
 * Locate token in a list. The token must be the car of a sublist.
 * Returns: A new list with this sublist or NULL if not found.
 */
gcry_sexp_t
_gcry_sexp_find_token( const gcry_sexp_t list, const char *tok, size_t toklen )
{
  const byte *p;
  DATALEN n;

  if ( !list )
    return NULL;

  if ( !toklen )
    toklen = strlen(tok);

  p = list->d;
  while ( *p != ST_STOP )
    {
      if ( *p == ST_OPEN && p[1] == ST_DATA )
        {
          const byte *head = p;

          p += 2;
          memcpy ( &n, p, sizeof n );
          p += sizeof n;
          if ( n == toklen && !memcmp( p, tok, toklen ) )
            { /* found it */
              gcry_sexp_t newlist;
              byte *d;
              int level = 1;

              /* Look for the end of the list.  */
              for ( p += n; level; p++ )
                {
                  if ( *p == ST_DATA )
                    {
			memcpy ( &n, ++p, sizeof n );
			p += sizeof n + n;
			p--; /* Compensate for later increment. */
		    }
                  else if ( *p == ST_OPEN )
                    {
                      level++;
		    }
                  else if ( *p == ST_CLOSE )
                    {
                      level--;
		    }
                  else if ( *p == ST_STOP )
                    {
                      BUG ();
		    }
		}
              n = p - head;

              newlist = xtrymalloc ( sizeof *newlist + n );
              if (!newlist)
                {
                  /* No way to return an error code, so we can only
                     return Not Found. */
                  return NULL;
                }
              d = newlist->d;
              memcpy ( d, head, n ); d += n;
              *d++ = ST_STOP;
              return normalize ( newlist );
	    }
          p += n;
	}
      else if ( *p == ST_DATA )
        {
          memcpy ( &n, ++p, sizeof n ); p += sizeof n;
          p += n;
	}
      else
        p++;
    }
  return NULL;
}

/****************
 * Return the length of the given list
 */
int
_gcry_sexp_length (const gcry_sexp_t list)
{
  const byte *p;
  DATALEN n;
  int type;
  int length = 0;
  int level = 0;

  if (!list)
    return 0;

  p = list->d;
  while ((type=*p) != ST_STOP)
    {
      p++;
      if (type == ST_DATA)
        {
          memcpy (&n, p, sizeof n);
          p += sizeof n + n;
          if (level == 1)
            length++;
	}
      else if (type == ST_OPEN)
        {
          if (level == 1)
            length++;
          level++;
	}
      else if (type == ST_CLOSE)
        {
          level--;
	}
    }
  return length;
}


/* Return the internal lengths offset of LIST.  That is the size of
   the buffer from the first ST_OPEN, which is returned at R_OFF, to
   the corresponding ST_CLOSE inclusive.  */
static size_t
get_internal_buffer (const gcry_sexp_t list, size_t *r_off)
{
  const unsigned char *p;
  DATALEN n;
  int type;
  int level = 0;

  *r_off = 0;
  if (list)
    {
      p = list->d;
      while ( (type=*p) != ST_STOP )
        {
          p++;
          if (type == ST_DATA)
            {
              memcpy (&n, p, sizeof n);
              p += sizeof n + n;
            }
          else if (type == ST_OPEN)
            {
              if (!level)
                *r_off = (p-1) - list->d;
              level++;
            }
          else if ( type == ST_CLOSE )
            {
              level--;
              if (!level)
                return p - list->d;
            }
        }
    }
  return 0; /* Not a proper list.  */
}



/* Extract the n-th element of the given LIST.  Returns NULL for
   no-such-element, a corrupt list, or memory failure.  */
gcry_sexp_t
_gcry_sexp_nth (const gcry_sexp_t list, int number)
{
  const byte *p;
  DATALEN n;
  gcry_sexp_t newlist;
  byte *d;
  int level = 0;

  if (!list || list->d[0] != ST_OPEN)
    return NULL;
  p = list->d;

  while (number > 0)
    {
      p++;
      if (*p == ST_DATA)
        {
          memcpy (&n, ++p, sizeof n);
          p += sizeof n + n;
          p--;
          if (!level)
            number--;
	}
      else if (*p == ST_OPEN)
        {
          level++;
	}
      else if (*p == ST_CLOSE)
        {
          level--;
          if ( !level )
            number--;
	}
      else if (*p == ST_STOP)
        {
          return NULL;
	}
    }
  p++;

  if (*p == ST_DATA)
    {
      memcpy (&n, p+1, sizeof n);
      newlist = xtrymalloc (sizeof *newlist + 1 + 1 + sizeof n + n + 1);
      if (!newlist)
        return NULL;
      d = newlist->d;
      *d++ = ST_OPEN;
      memcpy (d, p, 1 + sizeof n + n);
      d += 1 + sizeof n + n;
      *d++ = ST_CLOSE;
      *d = ST_STOP;
    }
  else if (*p == ST_OPEN)
    {
      const byte *head = p;

      level = 1;
      do {
        p++;
        if (*p == ST_DATA)
          {
            memcpy (&n, ++p, sizeof n);
            p += sizeof n + n;
            p--;
          }
        else if (*p == ST_OPEN)
          {
            level++;
          }
        else if (*p == ST_CLOSE)
          {
            level--;
          }
        else if (*p == ST_STOP)
          {
            BUG ();
          }
      } while (level);
      n = p + 1 - head;

      newlist = xtrymalloc (sizeof *newlist + n);
      if (!newlist)
        return NULL;
      d = newlist->d;
      memcpy (d, head, n);
      d += n;
      *d++ = ST_STOP;
    }
  else
    newlist = NULL;

  return normalize (newlist);
}


gcry_sexp_t
_gcry_sexp_car (const gcry_sexp_t list)
{
  return _gcry_sexp_nth (list, 0);
}


/* Helper to get data from the car.  The returned value is valid as
   long as the list is not modified. */
static const char *
do_sexp_nth_data (const gcry_sexp_t list, int number, size_t *datalen)
{
  const byte *p;
  DATALEN n;
  int level = 0;

  *datalen = 0;
  if ( !list )
    return NULL;

  p = list->d;
  if ( *p == ST_OPEN )
    p++;	     /* Yep, a list. */
  else if (number)
    return NULL;     /* Not a list but N > 0 requested. */

  /* Skip over N elements. */
  while (number > 0)
    {
      if (*p == ST_DATA)
        {
          memcpy ( &n, ++p, sizeof n );
          p += sizeof n + n;
          p--;
          if ( !level )
            number--;
	}
      else if (*p == ST_OPEN)
        {
          level++;
	}
      else if (*p == ST_CLOSE)
        {
          level--;
          if ( !level )
            number--;
	}
      else if (*p == ST_STOP)
        {
          return NULL;
	}
      p++;
    }

  /* If this is data, return it.  */
  if (*p == ST_DATA)
    {
      memcpy ( &n, ++p, sizeof n );
      *datalen = n;
      return (const char*)p + sizeof n;
    }

  return NULL;
}


/* Get data from the car.  The returned value is valid as long as the
   list is not modified.  */
const char *
_gcry_sexp_nth_data (const gcry_sexp_t list, int number, size_t *datalen )
{
  return do_sexp_nth_data (list, number, datalen);
}


/* Get the nth element of a list which needs to be a simple object.
   The returned value is a malloced buffer and needs to be freed by
   the caller.  This is basically the same as gcry_sexp_nth_data but
   with an allocated result. */
void *
_gcry_sexp_nth_buffer (const gcry_sexp_t list, int number, size_t *rlength)
{
  const char *s;
  size_t n;
  char *buf;

  *rlength = 0;
  s = do_sexp_nth_data (list, number, &n);
  if (!s || !n)
    return NULL;
  buf = xtrymalloc (n);
  if (!buf)
    return NULL;
  memcpy (buf, s, n);
  *rlength = n;
  return buf;
}


/* Get a string from the car.  The returned value is a malloced string
   and needs to be freed by the caller.  */
char *
_gcry_sexp_nth_string (const gcry_sexp_t list, int number)
{
  const char *s;
  size_t n;
  char *buf;

  s = do_sexp_nth_data (list, number, &n);
  if (!s || n < 1 || (n+1) < 1)
    return NULL;
  buf = xtrymalloc (n+1);
  if (!buf)
    return NULL;
  memcpy (buf, s, n);
  buf[n] = 0;
  return buf;
}


/*
 * Get a MPI from the car
 */
gcry_mpi_t
_gcry_sexp_nth_mpi (gcry_sexp_t list, int number, int mpifmt)
{
  size_t n;
  gcry_mpi_t a;

  if (mpifmt == GCRYMPI_FMT_OPAQUE)
    {
      char *p;

      p = _gcry_sexp_nth_buffer (list, number, &n);
      if (!p)
        return NULL;

      a = _gcry_is_secure (list)? _gcry_mpi_snew (0) : _gcry_mpi_new (0);
      if (a)
        mpi_set_opaque (a, p, n*8);
      else
        xfree (p);
    }
  else
    {
      const char *s;

      if (!mpifmt)
        mpifmt = GCRYMPI_FMT_STD;

      s = do_sexp_nth_data (list, number, &n);
      if (!s)
        return NULL;

      if (_gcry_mpi_scan (&a, mpifmt, s, n, NULL))
        return NULL;
    }

  return a;
}


/****************
 * Get the CDR
 */
gcry_sexp_t
_gcry_sexp_cdr(const gcry_sexp_t list)
{
  const byte *p;
  const byte *head;
  DATALEN n;
  gcry_sexp_t newlist;
  byte *d;
  int level = 0;
  int skip = 1;

  if (!list || list->d[0] != ST_OPEN)
    return NULL;
  p = list->d;

  while (skip > 0)
    {
      p++;
      if (*p == ST_DATA)
        {
          memcpy ( &n, ++p, sizeof n );
          p += sizeof n + n;
          p--;
          if ( !level )
            skip--;
	}
      else if (*p == ST_OPEN)
        {
          level++;
	}
      else if (*p == ST_CLOSE)
        {
          level--;
          if ( !level )
            skip--;
	}
      else if (*p == ST_STOP)
        {
          return NULL;
	}
    }
  p++;

  head = p;
  level = 0;
  do {
    if (*p == ST_DATA)
      {
        memcpy ( &n, ++p, sizeof n );
        p += sizeof n + n;
        p--;
      }
    else if (*p == ST_OPEN)
      {
        level++;
      }
    else if (*p == ST_CLOSE)
      {
        level--;
      }
    else if (*p == ST_STOP)
      {
        return NULL;
      }
    p++;
  } while (level);
  n = p - head;

  newlist = xtrymalloc (sizeof *newlist + n + 2);
  if (!newlist)
    return NULL;
  d = newlist->d;
  *d++ = ST_OPEN;
  memcpy (d, head, n);
  d += n;
  *d++ = ST_CLOSE;
  *d++ = ST_STOP;

  return normalize (newlist);
}


gcry_sexp_t
_gcry_sexp_cadr ( const gcry_sexp_t list )
{
  gcry_sexp_t a, b;

  a = _gcry_sexp_cdr (list);
  b = _gcry_sexp_car (a);
  sexp_release (a);
  return b;
}


static GPG_ERR_INLINE int
hextonibble (int s)
{
  if (s >= '0' && s <= '9')
    return s - '0';
  else if (s >= 'A' && s <= 'F')
    return 10 + s - 'A';
  else if (s >= 'a' && s <= 'f')
    return 10 + s - 'a';
  else
    return 0;
}


struct make_space_ctx
{
  gcry_sexp_t sexp;
  size_t allocated;
  byte *pos;
};


static gpg_err_code_t
make_space ( struct make_space_ctx *c, size_t n )
{
  size_t used = c->pos - c->sexp->d;

  if ( used + n + sizeof(DATALEN) + 1 >= c->allocated )
    {
      gcry_sexp_t newsexp;
      byte *newhead;
      size_t newsize;

      newsize = c->allocated + 2*(n+sizeof(DATALEN)+1);
      if (newsize <= c->allocated)
        return GPG_ERR_TOO_LARGE;
      newsexp = xtryrealloc ( c->sexp, sizeof *newsexp + newsize - 1);
      if (!newsexp)
        return gpg_err_code_from_errno (errno);
      c->allocated = newsize;
      newhead = newsexp->d;
      c->pos = newhead + used;
      c->sexp = newsexp;
    }
  return 0;
}


/* Unquote STRING of LENGTH and store it into BUF.  The surrounding
   quotes are must already be removed from STRING.  We assume that the
   quoted string is syntacillay correct.  */
static size_t
unquote_string (const char *string, size_t length, unsigned char *buf)
{
  int esc = 0;
  const unsigned char *s = (const unsigned char*)string;
  unsigned char *d = buf;
  size_t n = length;

  for (; n; n--, s++)
    {
      if (esc)
        {
          switch (*s)
            {
            case 'b':  *d++ = '\b'; break;
            case 't':  *d++ = '\t'; break;
            case 'v':  *d++ = '\v'; break;
            case 'n':  *d++ = '\n'; break;
            case 'f':  *d++ = '\f'; break;
            case 'r':  *d++ = '\r'; break;
            case '"':  *d++ = '\"'; break;
            case '\'': *d++ = '\''; break;
            case '\\': *d++ = '\\'; break;

            case '\r':  /* ignore CR[,LF] */
              if (n>1 && s[1] == '\n')
                {
                  s++; n--;
                }
              break;

            case '\n':  /* ignore LF[,CR] */
              if (n>1 && s[1] == '\r')
                {
                  s++; n--;
                }
              break;

            case 'x': /* hex value */
              if (n>2 && hexdigitp (s+1) && hexdigitp (s+2))
                {
                  s++; n--;
                  *d++ = xtoi_2 (s);
                  s++; n--;
                }
              break;

            default:
              if (n>2 && octdigitp (s) && octdigitp (s+1) && octdigitp (s+2))
                {
                  *d++ = (atoi_1 (s)*64) + (atoi_1 (s+1)*8) + atoi_1 (s+2);
                  s += 2;
                  n -= 2;
                }
              break;
	    }
          esc = 0;
        }
      else if( *s == '\\' )
        esc = 1;
      else
        *d++ = *s;
    }

  return d - buf;
}

/****************
 * Scan the provided buffer and return the S expression in our internal
 * format.  Returns a newly allocated expression.  If erroff is not NULL and
 * a parsing error has occurred, the offset into buffer will be returned.
 * If ARGFLAG is true, the function supports some printf like
 * expressions.
 *  These are:
 *	%m - MPI
 *	%s - string (no autoswitch to secure allocation)
 *	%d - integer stored as string (no autoswitch to secure allocation)
 *      %b - memory buffer; this takes _two_ arguments: an integer with the
 *           length of the buffer and a pointer to the buffer.
 *      %S - Copy an gcry_sexp_t here.  The S-expression needs to be a
 *           regular one, starting with a parenthesis.
 *           (no autoswitch to secure allocation)
 *  all other format elements are currently not defined and return an error.
 *  this includes the "%%" sequence becauce the percent sign is not an
 *  allowed character.
 * FIXME: We should find a way to store the secure-MPIs not in the string
 * but as reference to somewhere - this can help us to save huge amounts
 * of secure memory.  The problem is, that if only one element is secure, all
 * other elements are automagicaly copied to secure memory too, so the most
 * common operation gcry_sexp_cdr_mpi() will always return a secure MPI
 * regardless whether it is needed or not.
 */
static gpg_err_code_t
do_vsexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
                const char *buffer, size_t length, int argflag,
                void **arg_list, va_list arg_ptr)
{
  gcry_err_code_t err = 0;
  static const char tokenchars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789-./_:*+=";
  const char *p;
  size_t n;
  const char *digptr = NULL;
  const char *quoted = NULL;
  const char *tokenp = NULL;
  const char *hexfmt = NULL;
  const char *base64 = NULL;
  const char *disphint = NULL;
  const char *percent = NULL;
  int hexcount = 0;
  int b64count = 0;
  int quoted_esc = 0;
  size_t datalen = 0;
  size_t dummy_erroff;
  struct make_space_ctx c;
  int arg_counter = 0;
  int level = 0;

  if (!retsexp)
    return GPG_ERR_INV_ARG;
  *retsexp = NULL;

  if (!buffer)
    return GPG_ERR_INV_ARG;

  if (!erroff)
    erroff = &dummy_erroff;

  /* Depending on whether ARG_LIST is non-zero or not, this macro gives
     us the next argument, either from the variable argument list as
     specified by ARG_PTR or from the argument array ARG_LIST.  */
#define ARG_NEXT(storage, type)                          \
  do                                                     \
    {                                                    \
      if (!arg_list)                                     \
	storage = va_arg (arg_ptr, type);                \
      else                                               \
	storage = *((type *) (arg_list[arg_counter++])); \
    }                                                    \
  while (0)

  /* The MAKE_SPACE macro is used before each store operation to
     ensure that the buffer is large enough.  It requires a global
     context named C and jumps out to the label LEAVE on error! It
     also sets ERROFF using the variables BUFFER and P.  */
#define MAKE_SPACE(n)  do {                                                \
                            gpg_err_code_t _ms_err = make_space (&c, (n)); \
                            if (_ms_err)                                   \
                              {                                            \
                                err = _ms_err;                             \
                                *erroff = p - buffer;                      \
                                goto leave;                                \
                              }                                            \
                       } while (0)

  /* The STORE_LEN macro is used to store the length N at buffer P. */
#define STORE_LEN(p,n) do {						   \
			    DATALEN ashort = (n);			   \
			    memcpy ( (p), &ashort, sizeof(ashort) );	   \
			    (p) += sizeof (ashort);			   \
			} while (0)

  /* We assume that the internal representation takes less memory than
     the provided one.  However, we add space for one extra datalen so
     that the code which does the ST_CLOSE can use MAKE_SPACE */
  c.allocated = length + sizeof(DATALEN);
  if (length && _gcry_is_secure (buffer))
    c.sexp = xtrymalloc_secure (sizeof *c.sexp + c.allocated - 1);
  else
    c.sexp = xtrymalloc (sizeof *c.sexp + c.allocated - 1);
  if (!c.sexp)
    {
      err = gpg_err_code_from_errno (errno);
      *erroff = 0;
      goto leave;
    }
  c.pos = c.sexp->d;

  for (p = buffer, n = length; n; p++, n--)
    {
      if (tokenp && !hexfmt)
	{
	  if (strchr (tokenchars, *p))
	    continue;
	  else
	    {
	      datalen = p - tokenp;
	      MAKE_SPACE (datalen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, datalen);
	      memcpy (c.pos, tokenp, datalen);
	      c.pos += datalen;
	      tokenp = NULL;
	    }
	}

      if (quoted)
	{
	  if (quoted_esc)
	    {
	      switch (*p)
		{
		case 'b': case 't': case 'v': case 'n': case 'f':
		case 'r': case '"': case '\'': case '\\':
		  quoted_esc = 0;
		  break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7':
		  if (!((n > 2)
                        && (p[1] >= '0') && (p[1] <= '7')
                        && (p[2] >= '0') && (p[2] <= '7')))
		    {
		      *erroff = p - buffer;
		      /* Invalid octal value.  */
		      err = GPG_ERR_SEXP_BAD_QUOTATION;
                      goto leave;
		    }
		  p += 2;
		  n -= 2;
		  quoted_esc = 0;
		  break;

		case 'x':
		  if (!((n > 2) && hexdigitp (p+1) && hexdigitp (p+2)))
		    {
		      *erroff = p - buffer;
		      /* Invalid hex value.  */
		      err = GPG_ERR_SEXP_BAD_QUOTATION;
                      goto leave;
		    }
		  p += 2;
		  n -= 2;
		  quoted_esc = 0;
		  break;

		case '\r':
		  /* ignore CR[,LF] */
		  if (n && (p[1] == '\n'))
		    {
		      p++;
		      n--;
		    }
		  quoted_esc = 0;
		  break;

		case '\n':
		  /* ignore LF[,CR] */
		  if (n && (p[1] == '\r'))
		    {
		      p++;
		      n--;
		    }
		  quoted_esc = 0;
		  break;

		default:
		  *erroff = p - buffer;
		  /* Invalid quoted string escape.  */
		  err = GPG_ERR_SEXP_BAD_QUOTATION;
                  goto leave;
		}
	    }
	  else if (*p == '\\')
	    quoted_esc = 1;
	  else if (*p == '\"')
	    {
	      /* Keep it easy - we know that the unquoted string will
		 never be larger. */
	      unsigned char *save;
	      size_t len;

	      quoted++; /* Skip leading quote.  */
	      MAKE_SPACE (p - quoted);
	      *c.pos++ = ST_DATA;
	      save = c.pos;
	      STORE_LEN (c.pos, 0); /* Will be fixed up later.  */
	      len = unquote_string (quoted, p - quoted, c.pos);
	      c.pos += len;
	      STORE_LEN (save, len);
	      quoted = NULL;
	    }
	}
      else if (hexfmt)
	{
	  if (isxdigit (*p))
	    hexcount++;
	  else if (*p == '#')
	    {
	      if ((hexcount & 1))
		{
		  *erroff = p - buffer;
		  err = GPG_ERR_SEXP_ODD_HEX_NUMBERS;
                  goto leave;
		}

	      datalen = hexcount / 2;
	      MAKE_SPACE (datalen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, datalen);
	      for (hexfmt++; hexfmt < p; hexfmt++)
		{
                  int tmpc;

		  if (whitespacep (hexfmt))
		    continue;
		  tmpc = hextonibble (*(const unsigned char*)hexfmt);
                  for (hexfmt++; hexfmt < p && whitespacep (hexfmt); hexfmt++)
		    ;
                  if (hexfmt < p)
                    {
                      tmpc *= 16;
                      tmpc += hextonibble (*(const unsigned char*)hexfmt);
                    }
                  *c.pos++ = tmpc;
		}
	      hexfmt = NULL;
	    }
	  else if (!whitespacep (p))
	    {
	      *erroff = p - buffer;
	      err = GPG_ERR_SEXP_BAD_HEX_CHAR;
              goto leave;
	    }
	}
      else if (base64)
        {
          if (digitp (p) || alphap (p) || *p == '+' || *p == '/' || *p == '=')
            b64count++;
          else if (*p == '|')
            {
              gpgrt_b64state_t b64state;
              char *b64buf;
              int i;

              base64++;         /* Skip beginning '|' */
              b64buf = xtrymalloc (b64count);
              if (!b64buf)
                {
                  err = gpg_err_code_from_syserror ();
                  goto leave;
                }
              memcpy (b64buf, base64, b64count);

              b64state = gpgrt_b64dec_start (NULL);
              if (!b64state)
                {
                  err = gpg_err_code_from_syserror ();
                  xfree (b64buf);
                  goto leave;
                }
              err = gpgrt_b64dec_proc (b64state, b64buf, b64count,
                                       &datalen);
              if (err && gpg_err_code (err) != GPG_ERR_EOF)
                {
                  xfree (b64state);
                  xfree (b64buf);
                  goto leave;
                }
              err = gpgrt_b64dec_finish (b64state);
              if (err)
                {
                  xfree (b64buf);
                  goto leave;
                }

              MAKE_SPACE (datalen);
              *c.pos++ = ST_DATA;
              STORE_LEN (c.pos, datalen);
              for (i = 0; i < datalen; i++)
                *c.pos++ = b64buf[i];

              xfree (b64buf);
              base64 = NULL;
            }
          else
            {
              *erroff = p - buffer;
              err = GPG_ERR_SEXP_BAD_CHARACTER;
              goto leave;
            }
	}
      else if (digptr)
	{
	  if (digitp (p))
	    ;
	  else if (*p == ':')
	    {
	      datalen = atoi (digptr); /* FIXME: check for overflow.  */
	      digptr = NULL;
	      if (datalen > n - 1)
		{
		  *erroff = p - buffer;
		  /* Buffer too short.  */
		  err = GPG_ERR_SEXP_STRING_TOO_LONG;
                  goto leave;
		}
	      /* Make a new list entry.  */
	      MAKE_SPACE (datalen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, datalen);
	      memcpy (c.pos, p + 1, datalen);
	      c.pos += datalen;
	      n -= datalen;
	      p += datalen;
	    }
	  else if (*p == '\"')
	    {
	      digptr = NULL; /* We ignore the optional length.  */
	      quoted = p;
	      quoted_esc = 0;
	    }
	  else if (*p == '#')
	    {
	      digptr = NULL; /* We ignore the optional length.  */
	      hexfmt = p;
	      hexcount = 0;
	    }
          else if (*p == '|')
            {
              digptr = NULL; /* We ignore the optional length.  */
              base64 = p;
              b64count = 0;
            }
	  else
	    {
	      *erroff = p - buffer;
	      err = GPG_ERR_SEXP_INV_LEN_SPEC;
              goto leave;
	    }
	}
      else if (percent)
	{
	  if (*p == 'm' || *p == 'M')
	    {
	      /* Insert an MPI.  */
	      gcry_mpi_t m;
	      size_t nm = 0;
              int mpifmt = *p == 'm'? GCRYMPI_FMT_STD: GCRYMPI_FMT_USG;

	      ARG_NEXT (m, gcry_mpi_t);

              if (mpi_get_flag (m, GCRYMPI_FLAG_OPAQUE))
                {
                  void *mp;
                  unsigned int nbits;

                  mp = mpi_get_opaque (m, &nbits);
                  nm = (nbits+7)/8;
                  if (mp && nm)
                    {
                      MAKE_SPACE (nm);
                      if (!_gcry_is_secure (c.sexp->d)
                          && mpi_get_flag (m, GCRYMPI_FLAG_SECURE))
                        {
                          /* We have to switch to secure allocation.  */
                          gcry_sexp_t newsexp;
                          byte *newhead;

                          newsexp = xtrymalloc_secure (sizeof *newsexp
                                                       + c.allocated - 1);
                          if (!newsexp)
                            {
                              err = gpg_err_code_from_errno (errno);
                              goto leave;
                            }
                          newhead = newsexp->d;
                          memcpy (newhead, c.sexp->d, (c.pos - c.sexp->d));
                          c.pos = newhead + (c.pos - c.sexp->d);
                          xfree (c.sexp);
                          c.sexp = newsexp;
                        }

                      *c.pos++ = ST_DATA;
                      STORE_LEN (c.pos, nm);
                      memcpy (c.pos, mp, nm);
                      c.pos += nm;
                    }
                }
              else
                {
                  if (mpifmt == GCRYMPI_FMT_USG && mpi_cmp_ui (m, 0) < 0)
                    {
                      err = GPG_ERR_INV_ARG;
                      goto leave;
                    }

                  err = _gcry_mpi_print (mpifmt, NULL, 0, &nm, m);
                  if (err)
                    goto leave;

                  MAKE_SPACE (nm);
                  if (!_gcry_is_secure (c.sexp->d)
                      && mpi_get_flag ( m, GCRYMPI_FLAG_SECURE))
                    {
                      /* We have to switch to secure allocation.  */
                      gcry_sexp_t newsexp;
                      byte *newhead;

                      newsexp = xtrymalloc_secure (sizeof *newsexp
                                                   + c.allocated - 1);
                      if (!newsexp)
                        {
                          err = gpg_err_code_from_errno (errno);
                          goto leave;
                        }
                      newhead = newsexp->d;
                      memcpy (newhead, c.sexp->d, (c.pos - c.sexp->d));
                      c.pos = newhead + (c.pos - c.sexp->d);
                      xfree (c.sexp);
                      c.sexp = newsexp;
                    }

                  *c.pos++ = ST_DATA;
                  STORE_LEN (c.pos, nm);
                  err = _gcry_mpi_print (mpifmt, c.pos, nm, &nm, m);
                  if (err)
                    goto leave;
                  c.pos += nm;
                }
	    }
	  else if (*p == 's')
	    {
	      /* Insert an string.  */
	      const char *astr;
	      size_t alen;

	      ARG_NEXT (astr, const char *);
	      alen = strlen (astr);

	      MAKE_SPACE (alen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, alen);
	      memcpy (c.pos, astr, alen);
	      c.pos += alen;
	    }
	  else if (*p == 'b')
	    {
	      /* Insert a memory buffer.  */
	      const char *astr;
	      int alen;

	      ARG_NEXT (alen, int);
	      ARG_NEXT (astr, const char *);

              if (alen < 0)
                {
                  *erroff = p - buffer;
		  err = GPG_ERR_INV_ARG;
                  goto leave;
                }

	      MAKE_SPACE (alen);
	      if (alen
                  && !_gcry_is_secure (c.sexp->d)
		  && _gcry_is_secure (astr))
              {
		  /* We have to switch to secure allocation.  */
		  gcry_sexp_t newsexp;
		  byte *newhead;

		  newsexp = xtrymalloc_secure (sizeof *newsexp
                                               + c.allocated - 1);
                  if (!newsexp)
                    {
                      err = gpg_err_code_from_errno (errno);
                      goto leave;
                    }
		  newhead = newsexp->d;
		  memcpy (newhead, c.sexp->d, (c.pos - c.sexp->d));
		  c.pos = newhead + (c.pos - c.sexp->d);
		  xfree (c.sexp);
		  c.sexp = newsexp;
		}

	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, alen);
	      memcpy (c.pos, astr, alen);
	      c.pos += alen;
	    }
	  else if (*p == 'd')
	    {
	      /* Insert an integer as string.  */
	      int aint;
	      size_t alen;
	      char buf[35];

	      ARG_NEXT (aint, int);
	      snprintf (buf, sizeof buf, "%d", aint);
	      alen = strlen (buf);
	      MAKE_SPACE (alen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, alen);
	      memcpy (c.pos, buf, alen);
	      c.pos += alen;
	    }
	  else if (*p == 'u')
	    {
	      /* Insert an unsigned integer as string.  */
	      unsigned int aint;
	      size_t alen;
	      char buf[35];

	      ARG_NEXT (aint, unsigned int);
	      snprintf (buf, sizeof buf, "%u", aint);
	      alen = strlen (buf);
	      MAKE_SPACE (alen);
	      *c.pos++ = ST_DATA;
	      STORE_LEN (c.pos, alen);
	      memcpy (c.pos, buf, alen);
	      c.pos += alen;
	    }
	  else if (*p == 'S')
	    {
	      /* Insert a gcry_sexp_t.  */
	      gcry_sexp_t asexp;
	      size_t alen, aoff;

	      ARG_NEXT (asexp, gcry_sexp_t);
              alen = get_internal_buffer (asexp, &aoff);
              if (alen)
                {
                  MAKE_SPACE (alen);
                  memcpy (c.pos, asexp->d + aoff, alen);
                  c.pos += alen;
                }
	    }
	  else
	    {
	      *erroff = p - buffer;
	      /* Invalid format specifier.  */
	      err = GPG_ERR_SEXP_INV_LEN_SPEC;
              goto leave;
	    }
	  percent = NULL;
	}
      else if (*p == '(')
	{
	  if (disphint)
	    {
	      *erroff = p - buffer;
	      /* Open display hint.  */
	      err = GPG_ERR_SEXP_UNMATCHED_DH;
              goto leave;
	    }
	  MAKE_SPACE (0);
	  *c.pos++ = ST_OPEN;
	  level++;
	}
      else if (*p == ')')
	{
	  /* Walk up.  */
	  if (disphint)
	    {
	      *erroff = p - buffer;
	      /* Open display hint.  */
	      err = GPG_ERR_SEXP_UNMATCHED_DH;
              goto leave;
	    }

	  if (level == 0)
	    {
	      *erroff = p - buffer;
	      err = GPG_ERR_SEXP_UNMATCHED_PAREN;
	      goto leave;
	    }
	  MAKE_SPACE (0);
	  *c.pos++ = ST_CLOSE;
	  level--;
	}
      else if (*p == '\"')
	{
	  quoted = p;
	  quoted_esc = 0;
	}
      else if (*p == '#')
	{
	  hexfmt = p;
	  hexcount = 0;
	}
      else if (*p == '|')
        {
          base64 = p;
          b64count = 0;
        }
      else if (*p == '[')
	{
	  if (disphint)
	    {
	      *erroff = p - buffer;
	      /* Open display hint.  */
	      err = GPG_ERR_SEXP_NESTED_DH;
              goto leave;
	    }
	  disphint = p;
	}
      else if (*p == ']')
	{
	  if (!disphint)
	    {
	      *erroff = p - buffer;
	      /* Open display hint.  */
	      err = GPG_ERR_SEXP_UNMATCHED_DH;
              goto leave;
	    }
	  disphint = NULL;
	}
      else if (digitp (p))
	{
	  if (*p == '0')
	    {
	      /* A length may not begin with zero.  */
	      *erroff = p - buffer;
	      err = GPG_ERR_SEXP_ZERO_PREFIX;
              goto leave;
	    }
	  digptr = p;
	}
      else if (strchr (tokenchars, *p))
	tokenp = p;
      else if (whitespacep (p))
	;
      else if (*p == '{')
	{
	  /* fixme: handle rescanning: we can do this by saving our
	     current state and start over at p+1 -- Hmmm. At this
	     point here we are in a well defined state, so we don't
	     need to save it.  Great.  */
	  *erroff = p - buffer;
	  err = GPG_ERR_SEXP_UNEXPECTED_PUNC;
          goto leave;
	}
      else if (strchr ("&\\", *p))
	{
	  /* Reserved punctuation.  */
	  *erroff = p - buffer;
	  err = GPG_ERR_SEXP_UNEXPECTED_PUNC;
          goto leave;
	}
      else if (argflag && (*p == '%'))
	percent = p;
      else
	{
	  /* Bad or unavailable.  */
	  *erroff = p - buffer;
	  err = GPG_ERR_SEXP_BAD_CHARACTER;
          goto leave;
	}
    }
  MAKE_SPACE (0);
  *c.pos++ = ST_STOP;

  if (level && !err)
    err = GPG_ERR_SEXP_UNMATCHED_PAREN;

 leave:
  if (err)
    {
      /* Error -> deallocate.  */
      if (c.sexp)
        {
          /* Extra paranoid wipe on error. */
          if (_gcry_is_secure (c.sexp))
            wipememory (c.sexp, sizeof (struct gcry_sexp) + c.allocated - 1);
          xfree (c.sexp);
        }
    }
  else
    *retsexp = normalize (c.sexp);

  return err;
#undef MAKE_SPACE
#undef STORE_LEN
}


static gpg_err_code_t
do_sexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
               const char *buffer, size_t length, int argflag,
               void **arg_list, ...)
{
  gcry_err_code_t rc;
  va_list arg_ptr;

  va_start (arg_ptr, arg_list);
  rc = do_vsexp_sscan (retsexp, erroff, buffer, length, argflag,
                       arg_list, arg_ptr);
  va_end (arg_ptr);

  return rc;
}


gpg_err_code_t
_gcry_sexp_build (gcry_sexp_t *retsexp, size_t *erroff, const char *format, ...)
{
  gcry_err_code_t rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = do_vsexp_sscan (retsexp, erroff, format, strlen(format), 1,
                       NULL, arg_ptr);
  va_end (arg_ptr);

  return rc;
}


gcry_err_code_t
_gcry_sexp_vbuild (gcry_sexp_t *retsexp, size_t *erroff,
                   const char *format, va_list arg_ptr)
{
  return do_vsexp_sscan (retsexp, erroff, format, strlen(format), 1,
                         NULL, arg_ptr);
}


/* Like gcry_sexp_build, but uses an array instead of variable
   function arguments.  */
gcry_err_code_t
_gcry_sexp_build_array (gcry_sexp_t *retsexp, size_t *erroff,
                        const char *format, void **arg_list)
{
  return do_sexp_sscan (retsexp, erroff, format, strlen(format), 1, arg_list);
}


gcry_err_code_t
_gcry_sexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
                  const char *buffer, size_t length)
{
  return do_sexp_sscan (retsexp, erroff, buffer, length, 0, NULL);
}


/* Figure out a suitable encoding for BUFFER of LENGTH.
   Returns: 0 = Binary
            1 = String possible
            2 = Token possible
*/
static int
suitable_encoding (const unsigned char *buffer, size_t length)
{
  const unsigned char *s;
  int maybe_token = 1;

  if (!length)
    return 1;

  if (*buffer & 0x80)
    return 0; /* If the MSB is set we assume that buffer represents a
                 negative number.  */
  if (!*buffer)
    return 0; /* Starting with a zero is pretty much a binary string.  */

  for (s=buffer; length; s++, length--)
    {
      if ( (*s < 0x20 || (*s >= 0x7f && *s <= 0xa0))
           && !strchr ("\b\t\v\n\f\r\"\'\\", *s))
        return 0; /*binary*/
      if ( maybe_token
           && !alphap (s) && !digitp (s)  && !strchr (TOKEN_SPECIALS, *s))
        maybe_token = 0;
    }
  s = buffer;
  if ( maybe_token && !digitp (s) )
    return 2;
  return 1;
}


static int
convert_to_hex (const unsigned char *src, size_t len, char *dest)
{
  int i;

  if (dest)
    {
      *dest++ = '#';
      for (i=0; i < len; i++, dest += 2 )
        snprintf (dest, 3, "%02X", src[i]);
      *dest++ = '#';
    }
  return len*2+2;
}

static int
convert_to_string (const unsigned char *s, size_t len, char *dest)
{
  if (dest)
    {
      char *p = dest;
      *p++ = '\"';
      for (; len; len--, s++ )
        {
          switch (*s)
            {
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            case '\v': *p++ = '\\'; *p++ = 'v';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\"': *p++ = '\\'; *p++ = '\"';  break;
            case '\'': *p++ = '\\'; *p++ = '\'';  break;
            case '\\': *p++ = '\\'; *p++ = '\\';  break;
            default:
              if ( (*s < 0x20 || (*s >= 0x7f && *s <= 0xa0)))
                {
                  snprintf (p, 5, "\\x%02x", *s);
                  p += 4;
                }
              else
                *p++ = *s;
            }
        }
      *p++ = '\"';
      return p - dest;
    }
  else
    {
      int count = 2;
      for (; len; len--, s++ )
        {
          switch (*s)
            {
            case '\b':
            case '\t':
            case '\v':
            case '\n':
            case '\f':
            case '\r':
            case '\"':
            case '\'':
            case '\\': count += 2; break;
            default:
              if ( (*s < 0x20 || (*s >= 0x7f && *s <= 0xa0)))
                count += 4;
              else
                count++;
            }
        }
      return count;
    }
}



static int
convert_to_token (const unsigned char *src, size_t len, char *dest)
{
  if (dest)
    memcpy (dest, src, len);
  return len;
}


/****************
 * Print SEXP to buffer using the MODE.  Returns the length of the
 * SEXP in buffer or 0 if the buffer is too short (We have at least an
 * empty list consisting of 2 bytes).  If a buffer of NULL is provided,
 * the required length is returned.
 */
size_t
_gcry_sexp_sprint (const gcry_sexp_t list, int mode,
                   void *buffer, size_t maxlength )
{
  static unsigned char empty[3] = { ST_OPEN, ST_CLOSE, ST_STOP };
  const unsigned char *s;
  char *d;
  DATALEN n;
  char numbuf[20];
  size_t len = 0;
  int i, indent = 0;

  s = list? list->d : empty;
  d = buffer;
  while ( *s != ST_STOP )
    {
      switch ( *s )
        {
        case ST_OPEN:
          s++;
          if ( mode != GCRYSEXP_FMT_CANON )
            {
              if (indent)
                len++;
              len += indent;
            }
          len++;
          if ( buffer )
            {
              if ( len >= maxlength )
                return 0;
              if ( mode != GCRYSEXP_FMT_CANON )
                {
                  if (indent)
                    *d++ = '\n';
                  for (i=0; i < indent; i++)
                    *d++ = ' ';
                }
              *d++ = '(';
	    }
          indent++;
          break;
        case ST_CLOSE:
          s++;
          len++;
          if ( buffer )
            {
              if ( len >= maxlength )
                return 0;
              *d++ = ')';
	    }
          indent--;
          if (*s != ST_OPEN && *s != ST_STOP && mode != GCRYSEXP_FMT_CANON)
            {
              len++;
              len += indent;
              if (buffer)
                {
                  if (len >= maxlength)
                    return 0;
                  *d++ = '\n';
                  for (i=0; i < indent; i++)
                    *d++ = ' ';
                }
            }
          break;
        case ST_DATA:
          s++;
          memcpy ( &n, s, sizeof n ); s += sizeof n;
          if (mode == GCRYSEXP_FMT_ADVANCED)
            {
              int type;
              size_t nn;

              switch ( (type=suitable_encoding (s, n)))
                {
                case 1: nn = convert_to_string (s, n, NULL); break;
                case 2: nn = convert_to_token (s, n, NULL); break;
                default: nn = convert_to_hex (s, n, NULL); break;
                }
              len += nn;
              if (buffer)
                {
                  if (len >= maxlength)
                    return 0;
                  switch (type)
                    {
                    case 1: convert_to_string (s, n, d); break;
                    case 2: convert_to_token (s, n, d); break;
                    default: convert_to_hex (s, n, d); break;
                    }
                  d += nn;
                }
              if (s[n] != ST_CLOSE)
                {
                  len++;
                  if (buffer)
                    {
                      if (len >= maxlength)
                        return 0;
                      *d++ = ' ';
                    }
                }
            }
          else
            {
              snprintf (numbuf, sizeof numbuf, "%u:", (unsigned int)n );
              len += strlen (numbuf) + n;
              if ( buffer )
                {
                  if ( len >= maxlength )
		    return 0;
                  d = stpcpy ( d, numbuf );
                  memcpy ( d, s, n ); d += n;
                }
            }
          s += n;
          break;
        default:
          BUG ();
	}
    }
  if ( mode != GCRYSEXP_FMT_CANON )
    {
      len++;
      if (buffer)
        {
          if ( len >= maxlength )
            return 0;
          *d++ = '\n';
        }
    }
  if (buffer)
    {
      if ( len >= maxlength )
        return 0;
      *d++ = 0; /* for convenience we make a C string */
    }
  else
    len++; /* we need one byte more for this */

  return len;
}


/* Scan a canonical encoded buffer with implicit length values and
   return the actual length this S-expression uses.  For a valid S-Exp
   it should never return 0.  If LENGTH is not zero, the maximum
   length to scan is given - this can be used for syntax checks of
   data passed from outside. errorcode and erroff may both be passed as
   NULL.  */
size_t
_gcry_sexp_canon_len (const unsigned char *buffer, size_t length,
                      size_t *erroff, gcry_err_code_t *errcode)
{
  const unsigned char *p;
  const unsigned char *disphint = NULL;
  unsigned int datalen = 0;
  size_t dummy_erroff;
  gcry_err_code_t dummy_errcode;
  size_t count = 0;
  int level = 0;

  if (!erroff)
    erroff = &dummy_erroff;
  if (!errcode)
    errcode = &dummy_errcode;

  *errcode = GPG_ERR_NO_ERROR;
  *erroff = 0;
  if (!buffer)
    return 0;
  if (*buffer != '(')
    {
      *errcode = GPG_ERR_SEXP_NOT_CANONICAL;
      return 0;
    }

  for (p=buffer; ; p++, count++ )
    {
      if (length && count >= length)
        {
          *erroff = count;
          *errcode = GPG_ERR_SEXP_STRING_TOO_LONG;
          return 0;
        }

      if (datalen)
        {
          if (*p == ':')
            {
              if (length && (count+datalen) >= length)
                {
                  *erroff = count;
                  *errcode = GPG_ERR_SEXP_STRING_TOO_LONG;
                  return 0;
                }
              count += datalen;
              p += datalen;
              datalen = 0;
	    }
          else if (digitp(p))
            datalen = datalen*10 + atoi_1(p);
          else
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_INV_LEN_SPEC;
              return 0;
	    }
	}
      else if (*p == '(')
        {
          if (disphint)
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_UNMATCHED_DH;
              return 0;
	    }
          level++;
	}
      else if (*p == ')')
        { /* walk up */
          if (!level)
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_UNMATCHED_PAREN;
              return 0;
	    }
          if (disphint)
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_UNMATCHED_DH;
              return 0;
	    }
          if (!--level)
            return ++count; /* ready */
	}
      else if (*p == '[')
        {
          if (disphint)
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_NESTED_DH;
              return 0;
            }
          disphint = p;
	}
      else if (*p == ']')
        {
          if ( !disphint )
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_UNMATCHED_DH;
              return 0;
	    }
          disphint = NULL;
	}
      else if (digitp (p) )
        {
          if (*p == '0')
            {
              *erroff = count;
              *errcode = GPG_ERR_SEXP_ZERO_PREFIX;
              return 0;
	    }
          datalen = atoi_1 (p);
	}
      else if (*p == '&' || *p == '\\')
        {
          *erroff = count;
          *errcode = GPG_ERR_SEXP_UNEXPECTED_PUNC;
          return 0;
	}
      else
        {
          *erroff = count;
          *errcode = GPG_ERR_SEXP_BAD_CHARACTER;
          return 0;
	}
    }
}


/* Extract MPIs from an s-expression using a list of parameters.  The
 * names of these parameters are given by the string LIST.  Some
 * special characters may be given to control the conversion:
 *
 *   +   :: Switch to unsigned integer format (default).
 *   -   :: Switch to standard signed format.
 *   /   :: Switch to opaque format
 *   &   :: Switch to buffer descriptor mode - see below.
 *   %s  :: Switch to allocated string arguments.
 *   %#s :: Switch to allocated string arguments for a list of string flags.
 *   %u  :: Switch to unsigned integer arguments.
 *   %lu :: Switch to unsigned long integer arguments.
 *   %zu :: Switch to size_t arguments.
 *   %d  :: Switch to signed integer arguments.
 *   %ld :: Switch to signed long integer arguments.
 *   ?   :: The previous parameter is optional.
 *
 * In general parameter names are single letters.  To use a string for
 * a parameter name, enclose the name in single quotes.
 *
 * Unless in gcry_buffer_t mode for each parameter name a pointer to
 * an MPI variable is expected that must be set to NULL prior to
 * invoking this function, and finally a NULL is expected.  Example:
 *
 *   _gcry_sexp_extract_param (key, NULL, "n/x+ed",
 *                             &mpi_n, &mpi_x, &mpi_e, NULL)
 *
 * This stores the parameter "N" from KEY as an unsigned MPI into
 * MPI_N, the parameter "X" as an opaque MPI into MPI_X, and the
 * parameter "E" again as an unsigned MPI into MPI_E.
 *
 * If in buffer descriptor mode a pointer to gcry_buffer_t descriptor
 * is expected instead of a pointer to an MPI.  The caller may use two
 * different operation modes: If the DATA field of the provided buffer
 * descriptor is NULL, the function allocates a new buffer and stores
 * it at DATA; the other fields are set accordingly with OFF being 0.
 * If DATA is not NULL, the function assumes that DATA, SIZE, and OFF
 * describe a buffer where to but the data; on return the LEN field
 * receives the number of bytes copied to that buffer; if the buffer
 * is too small, the function immediately returns with an error code
 * (and LEN set to 0).
 *
 * For a flag list ("%#s") which has other lists as elements these
 * sub-lists are skipped and a indicated by "()" in the output.
 *
 * PATH is an optional string used to locate a token.  The exclamation
 * mark separated tokens are used to via gcry_sexp_find_token to find
 * a start point inside SEXP.
 *
 * The function returns 0 on success.  On error an error code is
 * returned, all passed MPIs that might have been allocated up to this
 * point are deallocated and set to NULL, and all passed buffers are
 * either truncated if the caller supplied the buffer, or deallocated
 * if the function allocated the buffer.
 */
gpg_err_code_t
_gcry_sexp_vextract_param (gcry_sexp_t sexp, const char *path,
                           const char *list, va_list arg_ptr)
{
  gpg_err_code_t rc;
  const char *s, *s2;
  void **array[20];
  char arrayisdesc[20];
  int idx, i;
  gcry_sexp_t l1 = NULL;
  int mode = '+'; /* Default to GCRYMPI_FMT_USG.  */
  int submode = 0;
  gcry_sexp_t freethis = NULL;
  char *tmpstr = NULL;

  /* Values in ARRAYISDESC describing what the ARRAY holds.
   *  0  - MPI
   *  1  - gcry_buffer_t provided by caller.
   *  2  - gcry_buffer_t allocated by us.
   * 's' - String allocated by us.
   * 'x' - Ignore
   */
  memset (arrayisdesc, 0, sizeof arrayisdesc);

  /* First copy all the args into an array.  This is required so that
     we are able to release already allocated MPIs if later an error
     was found.  */
  for (s=list, idx=0; *s && idx < DIM (array); s++)
    {
      if (*s == '&' || *s == '+' || *s == '-' || *s == '/' || *s == '?')
        ;
      else if (*s == '%')
        {
          s++;
          if (*s == 'l' && (s[1] == 'u' || s[1] == 'd'))
            s++;
          else if (*s == 'z' && s[1] == 'u')
            s++;
          else if (*s == '#' && s[1] == 's')
            s++;
          continue;
        }
      else if (whitespacep (s))
        ;
      else
        {
          if (*s == '\'')
            {
              s++;
              s2 = strchr (s, '\'');
              if (!s2 || s2 == s)
                {
                  /* Closing quote not found or empty string.  */
                  return GPG_ERR_SYNTAX;
                }
              s = s2;
            }
          array[idx] = va_arg (arg_ptr, void *);
          if (!array[idx])
            return GPG_ERR_MISSING_VALUE; /* NULL pointer given.  */
          idx++;
        }
    }
  if (*s)
    return GPG_ERR_LIMIT_REACHED;  /* Too many list elements.  */
  if (va_arg (arg_ptr, gcry_mpi_t *))
    return GPG_ERR_INV_ARG;  /* Not enough list elemends.  */

  /* Drill down.  */
  while (path && *path)
    {
      size_t n;

      s = strchr (path, '!');
      if (s == path)
        {
          rc = GPG_ERR_NOT_FOUND;
          goto cleanup;
        }
      n = s? s - path : 0;
      l1 = _gcry_sexp_find_token (sexp, path, n);
      if (!l1)
        {
          rc = GPG_ERR_NOT_FOUND;
          goto cleanup;
        }
      sexp = l1; l1 = NULL;
      sexp_release (freethis);
      freethis = sexp;
      if (n)
        path += n + 1;
      else
        path = NULL;
    }


  /* Now extract all parameters.  */
  for (s=list, idx=0; *s; s++)
    {
      if (*s == '&' || *s == '+' || *s == '-' || *s == '/')
        mode = *s;
      else if (*s == '%')
        {
          s++;
          if (!*s)
            continue;  /* Ignore at end of format.  */
          if (*s == 's' || *s == 'd' || *s == 'u')
            {
              mode = *s;
              submode = 0;
            }
          else if (*s == 'l' && (s[1] == 'u' || s[1] == 'd'))
            {
              mode = s[1];
              submode = 'l';
              s++;
            }
          else if (*s == 'z' && s[1] == 'u')
            {
              mode = s[1];
              submode = 'z';
              s++;
            }
          else if (*s == '#' && s[1] == 's')
            {
              mode = s[1];
              submode = '#';
              s++;
            }
          continue;
        }
      else if (whitespacep (s))
        ;
      else if (*s == '?')
        ; /* Only used via lookahead.  */
      else
        {
          if (*s == '\'')
            {
              /* Find closing quote, find token, set S to closing quote.  */
              s++;
              s2 = strchr (s, '\'');
              if (!s2 || s2 == s)
                {
                  /* Closing quote not found or empty string.  */
                  rc = GPG_ERR_SYNTAX;
                  goto cleanup;
                }
              l1 = _gcry_sexp_find_token (sexp, s, s2 - s);
              s = s2;
            }
          else
            l1 = _gcry_sexp_find_token (sexp, s, 1);

          if (!l1 && s[1] == '?')
            {
              /* Optional element not found.  */
              if (mode == '&')
                {
                  gcry_buffer_t *spec = (gcry_buffer_t*)array[idx];
                  if (!spec->data)
                    {
                      spec->size = 0;
                      spec->off = 0;
                    }
                  spec->len = 0;
                }
              else if (mode == 's')
                {
                  *array[idx] = NULL;
                  arrayisdesc[idx] = 's';
                }
              else if (mode == 'd')
                {
                  if (submode == 'l')
                    *(long *)array[idx] = 0;
                  else
                    *(int *)array[idx] = 0;
                  arrayisdesc[idx] = 'x';
                }
              else if (mode == 'u')
                {
                  if (submode == 'l')
                    *(unsigned long *)array[idx] = 0;
                  else if (submode == 'z')
                    *(size_t *)array[idx] = 0;
                  else
                    *(unsigned int *)array[idx] = 0;
                  arrayisdesc[idx] = 'x';
                }
              else
                *array[idx] = NULL;
            }
          else if (!l1)
            {
              rc = GPG_ERR_NO_OBJ;  /* List element not found.  */
              goto cleanup;
            }
           else
            {
              if (mode == '&')
                {
                  gcry_buffer_t *spec = (gcry_buffer_t*)array[idx];

                  if (spec->data)
                    {
                      const char *pbuf;
                      size_t nbuf;

                      pbuf = _gcry_sexp_nth_data (l1, 1, &nbuf);
                      if (!pbuf || !nbuf)
                        {
                          rc = GPG_ERR_INV_OBJ;
                          goto cleanup;
                        }
                      if (spec->off + nbuf > spec->size)
                        {
                          rc = GPG_ERR_BUFFER_TOO_SHORT;
                          goto cleanup;
                        }
                      memcpy ((char*)spec->data + spec->off, pbuf, nbuf);
                      spec->len = nbuf;
                      arrayisdesc[idx] = 1;
                    }
                  else
                    {
                      spec->data = _gcry_sexp_nth_buffer (l1, 1, &spec->size);
                      if (!spec->data)
                        {
                          rc = GPG_ERR_INV_OBJ; /* Or out of core.  */
                          goto cleanup;
                        }
                      spec->len = spec->size;
                      spec->off = 0;
                      arrayisdesc[idx] = 2;
                    }
                }
              else if (mode == 's')
                {
                  if (submode == '#')
                    {
                      size_t needed = 0;
                      size_t n;
                      int l1len;
                      char *p;

                      l1len = l1? sexp_length (l1) : 0;
                      for (i = 1; i < l1len; i++)
                        {
                          s2 = sexp_nth_data (l1, i, &n);
                          if (!s2)
                            n = 2; /* Not a data element; we use "()". */
                          needed += n + 1;
                        }
                      if (!needed)
                        {
                          *array[idx] = p = xtrymalloc (1);
                          if (p)
                            *p = 0;
                        }
                      else if ((*array[idx] = p = xtrymalloc (needed)))
                        {
                          for (i = 1; i < l1len; i++)
                            {
                              s2 = sexp_nth_data (l1, i, &n);
                              if (!s2)
                                memcpy (p, "()", (n=2));
                              else
                                memcpy (p, s2, n);
                              p[n] = ' ';
                              p += n + 1;
                            }
                          if (p != *array[idx])
                            p[-1] = 0;
                        }
                    }
                  else
                    *array[idx] = _gcry_sexp_nth_string (l1, 1);
                  if (!*array[idx])
                    {
                      rc = gpg_err_code_from_syserror ();
                      goto cleanup;
                    }
                  arrayisdesc[idx] = 's';
                }
              else if (mode == 'd')
                {
                  long along;

                  xfree (tmpstr);
                  tmpstr = _gcry_sexp_nth_string (l1, 1);
                  if (!tmpstr)
                    {
                      rc = gpg_err_code_from_syserror ();
                      goto cleanup;
                    }
                  along = strtol (tmpstr, NULL, 10);
                  if (submode == 'l')
                    *(long *)array[idx] = along;
                  else
                    *(int *)array[idx] = along;
                  arrayisdesc[idx] = 'x';
                }
              else if (mode == 'u')
                {
                  long aulong;

                  xfree (tmpstr);
                  tmpstr = _gcry_sexp_nth_string (l1, 1);
                  if (!tmpstr)
                    {
                      rc = gpg_err_code_from_syserror ();
                      goto cleanup;
                    }
                  aulong = strtoul (tmpstr, NULL, 10);
                  if (submode == 'l')
                    *(unsigned long *)array[idx] = aulong;
                  else if (submode == 'z')
                    *(size_t *)array[idx] = aulong;
                  else
                    *(unsigned int *)array[idx] = aulong;
                  arrayisdesc[idx] = 'x';
                }
              else
                {
                  if (mode == '/')
                    *array[idx] = _gcry_sexp_nth_mpi (l1,1,GCRYMPI_FMT_OPAQUE);
                  else if (mode == '-')
                    *array[idx] = _gcry_sexp_nth_mpi (l1,1,GCRYMPI_FMT_STD);
                  else
                    *array[idx] = _gcry_sexp_nth_mpi (l1,1,GCRYMPI_FMT_USG);
                  if (!*array[idx])
                    {
                      rc = GPG_ERR_INV_OBJ;  /* Conversion failed.  */
                      goto cleanup;
                    }
                }
              sexp_release (l1); l1 = NULL;
            }
          idx++;
        }
    }

  xfree (tmpstr);
  sexp_release (freethis);
  return 0;

 cleanup:
  xfree (tmpstr);
  sexp_release (freethis);
  sexp_release (l1);
  while (idx--)
    {
      if (!arrayisdesc[idx])
        {
          _gcry_mpi_release (*array[idx]);
          *array[idx] = NULL;
        }
      else if (arrayisdesc[idx] == 1)
        {
          /* Caller provided buffer.  */
          gcry_buffer_t *spec = (gcry_buffer_t*)array[idx];
          spec->len = 0;
        }
      else if (arrayisdesc[idx] == 2)
        {
          /* We might have allocated a buffer.  */
          gcry_buffer_t *spec = (gcry_buffer_t*)array[idx];
          xfree (spec->data);
          spec->data = NULL;
          spec->size = spec->off = spec->len = 0;
        }
      else if (arrayisdesc[idx] == 's')
        {
          /* We might have allocated a buffer.  */
          xfree (*array[idx]);
          *array[idx] = NULL;
        }
     }
  return rc;
}

gpg_err_code_t
_gcry_sexp_extract_param (gcry_sexp_t sexp, const char *path,
                          const char *list, ...)
{
  gcry_err_code_t rc;
  va_list arg_ptr;

  va_start (arg_ptr, list);
  rc = _gcry_sexp_vextract_param (sexp, path, list, arg_ptr);
  va_end (arg_ptr);
  return rc;
}
