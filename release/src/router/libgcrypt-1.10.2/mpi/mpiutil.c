/* mpiutil.ac  -  Utility functions for MPI
 * Copyright (C) 1998, 2000, 2001, 2002, 2003,
 *               2007  Free Software Foundation, Inc.
 * Copyright (C) 2013  g10 Code GmbH
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "mpi-internal.h"
#include "mod-source-info.h"


#if SIZEOF_UNSIGNED_INT == 2
# define MY_UINT_MAX 0xffff
/* (visual check:      0123 ) */
#elif SIZEOF_UNSIGNED_INT == 4
# define MY_UINT_MAX 0xffffffff
/* (visual check:      01234567 ) */
#elif SIZEOF_UNSIGNED_INT == 8
# define MY_UINT_MAX 0xffffffffffffffff
/* (visual check:      0123456789abcdef ) */
#else
# error Need MY_UINT_MAX for this limb size
#endif


/* Constants allocated right away at startup.  */
static gcry_mpi_t constants[MPI_NUMBER_OF_CONSTANTS];

/* These variables are used to generate masks from conditional operation
 * flag parameters.  Use of volatile prevents compiler optimizations from
 * converting AND-masking to conditional branches.  */
static volatile mpi_limb_t vzero = 0;
static volatile mpi_limb_t vone = 1;


const char *
_gcry_mpi_get_hw_config (void)
{
  return mod_source_info + 1;
}


/* Initialize the MPI subsystem.  This is called early and allows to
   do some initialization without taking care of threading issues.  */
gcry_err_code_t
_gcry_mpi_init (void)
{
  int idx;
  unsigned long value;

  for (idx=0; idx < MPI_NUMBER_OF_CONSTANTS; idx++)
    {
      switch (idx)
        {
        case MPI_C_ZERO:  value = 0; break;
        case MPI_C_ONE:   value = 1; break;
        case MPI_C_TWO:   value = 2; break;
        case MPI_C_THREE: value = 3; break;
        case MPI_C_FOUR:  value = 4; break;
        case MPI_C_EIGHT: value = 8; break;
        default: log_bug ("invalid mpi_const selector %d\n", idx);
        }
      constants[idx] = mpi_alloc_set_ui (value);
      constants[idx]->flags = (16|32);
    }

  return 0;
}


/****************
 * Note:  It was a bad idea to use the number of limbs to allocate
 *	  because on a alpha the limbs are large but we normally need
 *	  integers of n bits - So we should change this to bits (or bytes).
 *
 *	  But mpi_alloc is used in a lot of places :-(.  New code
 *	  should use mpi_new.
 */
gcry_mpi_t
_gcry_mpi_alloc( unsigned nlimbs )
{
    gcry_mpi_t a;

    a = xmalloc( sizeof *a );
    a->d = nlimbs? mpi_alloc_limb_space( nlimbs, 0 ) : NULL;
    a->alloced = nlimbs;
    a->nlimbs = 0;
    a->sign = 0;
    a->flags = 0;
    return a;
}

void
_gcry_mpi_m_check( gcry_mpi_t a )
{
    _gcry_check_heap(a);
    _gcry_check_heap(a->d);
}

gcry_mpi_t
_gcry_mpi_alloc_secure( unsigned nlimbs )
{
    gcry_mpi_t a;

    a = xmalloc( sizeof *a );
    a->d = nlimbs? mpi_alloc_limb_space( nlimbs, 1 ) : NULL;
    a->alloced = nlimbs;
    a->flags = 1;
    a->nlimbs = 0;
    a->sign = 0;
    return a;
}



mpi_ptr_t
_gcry_mpi_alloc_limb_space( unsigned int nlimbs, int secure )
{
    mpi_ptr_t p;
    size_t len;

    len = (nlimbs ? nlimbs : 1) * sizeof (mpi_limb_t);
    p = secure ? xmalloc_secure (len) : xmalloc (len);
    if (! nlimbs)
      *p = 0;

    return p;
}

void
_gcry_mpi_free_limb_space( mpi_ptr_t a, unsigned int nlimbs)
{
  if (a)
    {
      size_t len = nlimbs * sizeof(mpi_limb_t);

      /* If we have information on the number of allocated limbs, we
         better wipe that space out.  This is a failsafe feature if
         secure memory has been disabled or was not properly
         implemented in user provided allocation functions. */
      if (len)
        wipememory (a, len);
      xfree(a);
    }
}


void
_gcry_mpi_assign_limb_space( gcry_mpi_t a, mpi_ptr_t ap, unsigned int nlimbs )
{
  _gcry_mpi_free_limb_space (a->d, a->alloced);
  a->d = ap;
  a->alloced = nlimbs;
}



/****************
 * Resize the array of A to NLIMBS. The additional space is cleared
 * (set to 0).
 */
void
_gcry_mpi_resize (gcry_mpi_t a, unsigned nlimbs)
{
  size_t i;

  if (nlimbs <= a->alloced)
    {
      /* We only need to clear the new space (this is a nop if the
         limb space is already of the correct size. */
      for (i=a->nlimbs; i < a->alloced; i++)
        a->d[i] = 0;
      return;
    }

  /* Actually resize the limb space.  */
  if (a->d)
    {
      a->d = xrealloc (a->d, nlimbs * sizeof (mpi_limb_t));
      for (i=a->nlimbs; i < nlimbs; i++)
        a->d[i] = 0;
    }
  else
    {
      if (a->flags & 1)
	/* Secure memory is wanted.  */
	a->d = xcalloc_secure (nlimbs , sizeof (mpi_limb_t));
      else
	/* Standard memory.  */
	a->d = xcalloc (nlimbs , sizeof (mpi_limb_t));
    }
  a->alloced = nlimbs;
}

void
_gcry_mpi_clear( gcry_mpi_t a )
{
  if (mpi_is_immutable (a))
    {
      mpi_immutable_failed ();
      return;
    }
  a->nlimbs = 0;
  a->flags = 0;
}


void
_gcry_mpi_free( gcry_mpi_t a )
{
  if (!a )
    return;
  if ((a->flags & 32))
  {
#if GPGRT_VERSION_NUMBER >= 0x011600  /* 1.22 */
    gpgrt_annotate_leaked_object(a);
#endif
    return; /* Never release a constant. */
  }
  if ((a->flags & 4))
    xfree( a->d );
  else
    {
      _gcry_mpi_free_limb_space(a->d, a->alloced);
    }
  /* Check that the flags makes sense.  We better allow for bit 1
     (value 2) for backward ABI compatibility.  */
  if ((a->flags & ~(1|2|4|16
                    |GCRYMPI_FLAG_USER1
                    |GCRYMPI_FLAG_USER2
                    |GCRYMPI_FLAG_USER3
                    |GCRYMPI_FLAG_USER4)))
    log_bug("invalid flag value in mpi_free\n");
  xfree (a);
}


void
_gcry_mpi_immutable_failed (void)
{
  log_info ("Warning: trying to change an immutable MPI\n");
}


static void
mpi_set_secure( gcry_mpi_t a )
{
  mpi_ptr_t ap, bp;

  if ( (a->flags & 1) )
    return;
  a->flags |= 1;
  ap = a->d;
  if (!a->nlimbs)
    {
      gcry_assert (!ap);
      return;
    }
  bp = mpi_alloc_limb_space (a->alloced, 1);
  MPN_COPY( bp, ap, a->nlimbs );
  a->d = bp;
  _gcry_mpi_free_limb_space (ap, a->alloced);
}


gcry_mpi_t
_gcry_mpi_set_opaque (gcry_mpi_t a, void *p, unsigned int nbits)
{
  if (!a)
    a = mpi_alloc(0);

  if (mpi_is_immutable (a))
    {
      mpi_immutable_failed ();
      return a;
    }

  if( a->flags & 4 )
    xfree (a->d);
  else
    _gcry_mpi_free_limb_space (a->d, a->alloced);

  a->d = p;
  a->alloced = 0;
  a->nlimbs = 0;
  a->sign  = nbits;
  a->flags = 4 | (a->flags & (GCRYMPI_FLAG_USER1|GCRYMPI_FLAG_USER2
                              |GCRYMPI_FLAG_USER3|GCRYMPI_FLAG_USER4));
  if (_gcry_is_secure (a->d))
    a->flags |= 1;
  return a;
}


gcry_mpi_t
_gcry_mpi_set_opaque_copy (gcry_mpi_t a, const void *p, unsigned int nbits)
{
  void *d;
  unsigned int n;

  n = (nbits+7)/8;
  d = _gcry_is_secure (p)? xtrymalloc_secure (n) : xtrymalloc (n);
  if (!d)
    return NULL;
  memcpy (d, p, n);
  return mpi_set_opaque (a, d, nbits);
}


void *
_gcry_mpi_get_opaque (gcry_mpi_t a, unsigned int *nbits)
{
    if( !(a->flags & 4) )
	log_bug("mpi_get_opaque on normal mpi\n");
    if( nbits )
	*nbits = a->sign;
    return a->d;
}


void *
_gcry_mpi_get_opaque_copy (gcry_mpi_t a, unsigned int *nbits)
{
  const void *s;
  void *d;
  unsigned int n;

  s = mpi_get_opaque (a, nbits);
  if (!s && nbits)
    return NULL;
  n = (*nbits+7)/8;
  d = _gcry_is_secure (s)? xtrymalloc_secure (n) : xtrymalloc (n);
  if (d)
    memcpy (d, s, n);
  return d;
}

/****************
 * Note: This copy function should not interpret the MPI
 *	 but copy it transparently.
 */
gcry_mpi_t
_gcry_mpi_copy (gcry_mpi_t a)
{
    int i;
    gcry_mpi_t b;

    if( a && (a->flags & 4) ) {
        void *p = NULL;
        if (a->sign) {
            p = _gcry_is_secure(a->d)? xmalloc_secure ((a->sign+7)/8)
                                     : xmalloc ((a->sign+7)/8);
            if (a->d)
                memcpy( p, a->d, (a->sign+7)/8 );
        }
        b = mpi_set_opaque( NULL, p, a->sign );
        b->flags = a->flags;
        b->flags &= ~(16|32); /* Reset the immutable and constant flags.  */
    }
    else if( a ) {
	b = mpi_is_secure(a)? mpi_alloc_secure( a->nlimbs )
			    : mpi_alloc( a->nlimbs );
	b->nlimbs = a->nlimbs;
	b->sign = a->sign;
	b->flags  = a->flags;
        b->flags &= ~(16|32); /* Reset the immutable and constant flags.  */
	for(i=0; i < b->nlimbs; i++ )
	    b->d[i] = a->d[i];
    }
    else
	b = NULL;
    return b;
}


/* Return true if A is negative.  */
int
_gcry_mpi_is_neg (gcry_mpi_t a)
{
  if (a->sign && _gcry_mpi_cmp_ui (a, 0))
    return 1;
  else
    return 0;
}


/* W = - U */
void
_gcry_mpi_neg (gcry_mpi_t w, gcry_mpi_t u)
{
  if (w != u)
    mpi_set (w, u);
  else if (mpi_is_immutable (w))
    {
      mpi_immutable_failed ();
      return;
    }

  w->sign = !u->sign;
}


/* W = [W] */
void
_gcry_mpi_abs (gcry_mpi_t w)
{
  if (mpi_is_immutable (w))
    {
      mpi_immutable_failed ();
      return;
    }

  w->sign = 0;
}


/****************
 * This function allocates an MPI which is optimized to hold
 * a value as large as the one given in the argument and allocates it
 * with the same flags as A.
 */
gcry_mpi_t
_gcry_mpi_alloc_like( gcry_mpi_t a )
{
    gcry_mpi_t b;

    if( a && (a->flags & 4) ) {
	int n = (a->sign+7)/8;
	void *p = _gcry_is_secure(a->d)? xtrymalloc_secure (n)
                                       : xtrymalloc (n);
	memcpy( p, a->d, n );
	b = mpi_set_opaque( NULL, p, a->sign );
    }
    else if( a ) {
	b = mpi_is_secure(a)? mpi_alloc_secure( a->nlimbs )
			    : mpi_alloc( a->nlimbs );
	b->nlimbs = 0;
	b->sign = 0;
	b->flags = a->flags;
    }
    else
	b = NULL;
    return b;
}


/* Set U into W and release U.  If W is NULL only U will be released. */
void
_gcry_mpi_snatch (gcry_mpi_t w, gcry_mpi_t u)
{
  if (w)
    {
      if (mpi_is_immutable (w))
        {
          mpi_immutable_failed ();
          return;
        }
      _gcry_mpi_assign_limb_space (w, u->d, u->alloced);
      w->nlimbs = u->nlimbs;
      w->sign   = u->sign;
      w->flags  = u->flags;
      u->alloced = 0;
      u->nlimbs = 0;
      u->d = NULL;
    }
  _gcry_mpi_free (u);
}


gcry_mpi_t
_gcry_mpi_set (gcry_mpi_t w, gcry_mpi_t u)
{
  mpi_ptr_t wp, up;
  mpi_size_t usize = u->nlimbs;
  int usign = u->sign;

  if (!w)
    w = _gcry_mpi_alloc( mpi_get_nlimbs(u) );
  if (mpi_is_immutable (w))
    {
      mpi_immutable_failed ();
      return w;
    }
  RESIZE_IF_NEEDED(w, usize);
  wp = w->d;
  up = u->d;
  MPN_COPY( wp, up, usize );
  w->nlimbs = usize;
  w->flags = u->flags;
  w->flags &= ~(16|32); /* Reset the immutable and constant flags.  */
  w->sign = usign;
  return w;
}

/****************
 * Set the value of W by the one of U, when SET is 1.
 * Leave the value when SET is 0.
 * This implementation should be constant-time regardless of SET.
 */
gcry_mpi_t
_gcry_mpi_set_cond (gcry_mpi_t w, const gcry_mpi_t u, unsigned long set)
{
  mpi_size_t i;
  mpi_size_t nlimbs = u->alloced;
  mpi_limb_t mask1 = vzero - set;
  mpi_limb_t mask2 = set - vone;
  mpi_limb_t xu;
  mpi_limb_t xw;
  mpi_limb_t *uu = u->d;
  mpi_limb_t *uw = w->d;

  if (w->alloced != u->alloced)
    log_bug ("mpi_set_cond: different sizes\n");

  for (i = 0; i < nlimbs; i++)
    {
      xu = uu[i];
      xw = uw[i];
      uw[i] = (xw & mask2) | (xu & mask1);
    }

  xu = u->nlimbs;
  xw = w->nlimbs;
  w->nlimbs = (xw & mask2) | (xu & mask1);

  xu = u->sign;
  xw = w->sign;
  w->sign = (xw & mask2) | (xu & mask1);
  return w;
}


gcry_mpi_t
_gcry_mpi_set_ui (gcry_mpi_t w, unsigned long u)
{
  if (!w)
    w = _gcry_mpi_alloc (1);
  /* FIXME: If U is 0 we have no need to resize and thus possible
     allocating the the limbs. */
  if (mpi_is_immutable (w))
    {
      mpi_immutable_failed ();
      return w;
    }
  RESIZE_IF_NEEDED(w, 1);
  w->d[0] = u;
  w->nlimbs = u? 1:0;
  w->sign = 0;
  w->flags = 0;
  return w;
}

/* If U is non-negative and small enough store it as an unsigned int
 * at W.  If the value does not fit into an unsigned int or is
 * negative return GPG_ERR_ERANGE.  Note that we return an unsigned
 * int so that the value can be used with the bit test functions; in
 * contrast the other _ui functions take an unsigned long so that on
 * some platforms they may accept a larger value.  On error the value
 * at W is not changed. */
gcry_err_code_t
_gcry_mpi_get_ui (unsigned int *w, gcry_mpi_t u)
{
  mpi_limb_t x;

  if (u->nlimbs > 1 || u->sign)
    return GPG_ERR_ERANGE;

  x = (u->nlimbs == 1) ? u->d[0] : 0;
  if (sizeof (x) > sizeof (unsigned int) && x > MY_UINT_MAX)
    return GPG_ERR_ERANGE;

  *w = x;
  return 0;
}


gcry_mpi_t
_gcry_mpi_alloc_set_ui( unsigned long u)
{
    gcry_mpi_t w = mpi_alloc(1);
    w->d[0] = u;
    w->nlimbs = u? 1:0;
    w->sign = 0;
    return w;
}

void
_gcry_mpi_swap (gcry_mpi_t a, gcry_mpi_t b)
{
    struct gcry_mpi tmp;

    tmp = *a; *a = *b; *b = tmp;
}


/****************
 * Swap the value of A and B, when SWAP is 1.
 * Leave the value when SWAP is 0.
 * This implementation should be constant-time regardless of SWAP.
 */
void
_gcry_mpi_swap_cond (gcry_mpi_t a, gcry_mpi_t b, unsigned long swap)
{
  mpi_size_t i;
  mpi_size_t nlimbs;
  mpi_limb_t mask1 = vzero - swap;
  mpi_limb_t mask2 = swap - vone;
  mpi_limb_t *ua = a->d;
  mpi_limb_t *ub = b->d;
  mpi_limb_t xa;
  mpi_limb_t xb;

  if (a->alloced > b->alloced)
    nlimbs = b->alloced;
  else
    nlimbs = a->alloced;
  if (a->nlimbs > nlimbs || b->nlimbs > nlimbs)
    log_bug ("mpi_swap_cond: different sizes\n");

  for (i = 0; i < nlimbs; i++)
    {
      xa = ua[i];
      xb = ub[i];
      ua[i] = (xa & mask2) | (xb & mask1);
      ub[i] = (xa & mask1) | (xb & mask2);
    }

  xa = a->nlimbs;
  xb = b->nlimbs;
  a->nlimbs = (xa & mask2) | (xb & mask1);
  b->nlimbs = (xa & mask1) | (xb & mask2);

  xa = a->sign;
  xb = b->sign;
  a->sign = (xa & mask2) | (xb & mask1);
  b->sign = (xa & mask1) | (xb & mask2);
}


/****************
 * Set bit N of A, when SET is 1.
 * This implementation should be constant-time regardless of SET.
 */
void
_gcry_mpi_set_bit_cond (gcry_mpi_t a, unsigned int n, unsigned long set)
{
  unsigned int limbno, bitno;
  mpi_limb_t set_the_bit = !!set;

  limbno = n / BITS_PER_MPI_LIMB;
  bitno  = n % BITS_PER_MPI_LIMB;

  a->d[limbno] |= (set_the_bit<<bitno);
}


gcry_mpi_t
_gcry_mpi_new (unsigned int nbits)
{
    return _gcry_mpi_alloc ( (nbits+BITS_PER_MPI_LIMB-1)
                             / BITS_PER_MPI_LIMB );
}


gcry_mpi_t
_gcry_mpi_snew (unsigned int nbits)
{
  return _gcry_mpi_alloc_secure ( (nbits+BITS_PER_MPI_LIMB-1)
                                  / BITS_PER_MPI_LIMB );
}

void
_gcry_mpi_release( gcry_mpi_t a )
{
    _gcry_mpi_free( a );
}

void
_gcry_mpi_randomize (gcry_mpi_t w,
                     unsigned int nbits, enum gcry_random_level level)
{
  unsigned char *p;
  size_t nbytes = (nbits+7)/8;

  if (mpi_is_immutable (w))
    {
      mpi_immutable_failed ();
      return;
    }
  if (level == GCRY_WEAK_RANDOM)
    {
      p = mpi_is_secure(w) ? xmalloc_secure (nbytes)
                           : xmalloc (nbytes);
      _gcry_create_nonce (p, nbytes);
    }
  else
    {
      p = mpi_is_secure(w) ? _gcry_random_bytes_secure (nbytes, level)
                           : _gcry_random_bytes (nbytes, level);
    }
  _gcry_mpi_set_buffer( w, p, nbytes, 0 );
  xfree (p);
}


void
_gcry_mpi_set_flag (gcry_mpi_t a, enum gcry_mpi_flag flag)
{
  switch (flag)
    {
    case GCRYMPI_FLAG_SECURE:     mpi_set_secure(a); break;
    case GCRYMPI_FLAG_CONST:      a->flags |= (16|32); break;
    case GCRYMPI_FLAG_IMMUTABLE:  a->flags |= 16; break;

    case GCRYMPI_FLAG_USER1:
    case GCRYMPI_FLAG_USER2:
    case GCRYMPI_FLAG_USER3:
    case GCRYMPI_FLAG_USER4:      a->flags |= flag; break;

    case GCRYMPI_FLAG_OPAQUE:
    default: log_bug("invalid flag value\n");
    }
}

void
_gcry_mpi_clear_flag (gcry_mpi_t a, enum gcry_mpi_flag flag)
{
  (void)a; /* Not yet used. */

  switch (flag)
    {
    case GCRYMPI_FLAG_IMMUTABLE:
      if (!(a->flags & 32))
        a->flags &= ~16;
      break;

    case GCRYMPI_FLAG_USER1:
    case GCRYMPI_FLAG_USER2:
    case GCRYMPI_FLAG_USER3:
    case GCRYMPI_FLAG_USER4:
      a->flags &= ~flag;
      break;

    case GCRYMPI_FLAG_CONST:
    case GCRYMPI_FLAG_SECURE:
    case GCRYMPI_FLAG_OPAQUE:
    default: log_bug("invalid flag value\n");
    }
}

int
_gcry_mpi_get_flag (gcry_mpi_t a, enum gcry_mpi_flag flag)
{
  switch (flag)
    {
    case GCRYMPI_FLAG_SECURE:    return !!(a->flags & 1);
    case GCRYMPI_FLAG_OPAQUE:    return !!(a->flags & 4);
    case GCRYMPI_FLAG_IMMUTABLE: return !!(a->flags & 16);
    case GCRYMPI_FLAG_CONST:     return !!(a->flags & 32);
    case GCRYMPI_FLAG_USER1:
    case GCRYMPI_FLAG_USER2:
    case GCRYMPI_FLAG_USER3:
    case GCRYMPI_FLAG_USER4:     return !!(a->flags & flag);
    default: log_bug("invalid flag value\n");
    }
  /*NOTREACHED*/
  return 0;
}


/* Return a constant MPI descripbed by NO which is one of the
   MPI_C_xxx macros.  There is no need to copy this returned value; it
   may be used directly.  */
gcry_mpi_t
_gcry_mpi_const (enum gcry_mpi_constants no)
{
  if ((int)no < 0 || no > MPI_NUMBER_OF_CONSTANTS)
    log_bug("invalid mpi_const selector %d\n", no);
  if (!constants[no])
    log_bug("MPI subsystem not initialized\n");
  return constants[no];
}
