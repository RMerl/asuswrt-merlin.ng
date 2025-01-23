/* rmd160.c  -	RIPE-MD160
 * Copyright (C) 1998, 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "hash-common.h"
#include "cipher.h" /* Only used for the rmd160_hash_buffer() prototype. */

#include "bithelp.h"
#include "bufhelp.h"

/*********************************
 * RIPEMD-160 is not patented, see (as of 25.10.97)
 *   http://www.esat.kuleuven.ac.be/~bosselae/ripemd160.html
 * Note that the code uses Little Endian byteorder, which is good for
 * 386 etc, but we must add some conversion when used on a big endian box.
 *
 *
 * Pseudo-code for RIPEMD-160
 *
 * RIPEMD-160 is an iterative hash function that operates on 32-bit words.
 * The round function takes as input a 5-word chaining variable and a 16-word
 * message block and maps this to a new chaining variable. All operations are
 * defined on 32-bit words. Padding is identical to that of MD4.
 *
 *
 * RIPEMD-160: definitions
 *
 *
 *   nonlinear functions at bit level: exor, mux, -, mux, -
 *
 *   f(j, x, y, z) = x XOR y XOR z		  (0 <= j <= 15)
 *   f(j, x, y, z) = (x AND y) OR (NOT(x) AND z)  (16 <= j <= 31)
 *   f(j, x, y, z) = (x OR NOT(y)) XOR z	  (32 <= j <= 47)
 *   f(j, x, y, z) = (x AND z) OR (y AND NOT(z))  (48 <= j <= 63)
 *   f(j, x, y, z) = x XOR (y OR NOT(z))	  (64 <= j <= 79)
 *
 *
 *   added constants (hexadecimal)
 *
 *   K(j) = 0x00000000	    (0 <= j <= 15)
 *   K(j) = 0x5A827999	   (16 <= j <= 31)	int(2**30 x sqrt(2))
 *   K(j) = 0x6ED9EBA1	   (32 <= j <= 47)	int(2**30 x sqrt(3))
 *   K(j) = 0x8F1BBCDC	   (48 <= j <= 63)	int(2**30 x sqrt(5))
 *   K(j) = 0xA953FD4E	   (64 <= j <= 79)	int(2**30 x sqrt(7))
 *   K'(j) = 0x50A28BE6     (0 <= j <= 15)      int(2**30 x cbrt(2))
 *   K'(j) = 0x5C4DD124    (16 <= j <= 31)      int(2**30 x cbrt(3))
 *   K'(j) = 0x6D703EF3    (32 <= j <= 47)      int(2**30 x cbrt(5))
 *   K'(j) = 0x7A6D76E9    (48 <= j <= 63)      int(2**30 x cbrt(7))
 *   K'(j) = 0x00000000    (64 <= j <= 79)
 *
 *
 *   selection of message word
 *
 *   r(j)      = j		      (0 <= j <= 15)
 *   r(16..31) = 7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8
 *   r(32..47) = 3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12
 *   r(48..63) = 1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2
 *   r(64..79) = 4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
 *   r0(0..15) = 5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12
 *   r0(16..31)= 6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2
 *   r0(32..47)= 15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13
 *   r0(48..63)= 8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14
 *   r0(64..79)= 12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
 *
 *
 *   amount for rotate left (rol)
 *
 *   s(0..15)  = 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8
 *   s(16..31) = 7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12
 *   s(32..47) = 11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5
 *   s(48..63) = 11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12
 *   s(64..79) = 9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
 *   s'(0..15) = 8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6
 *   s'(16..31)= 9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11
 *   s'(32..47)= 9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5
 *   s'(48..63)= 15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8
 *   s'(64..79)= 8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
 *
 *
 *   initial value (hexadecimal)
 *
 *   h0 = 0x67452301; h1 = 0xEFCDAB89; h2 = 0x98BADCFE; h3 = 0x10325476;
 *							h4 = 0xC3D2E1F0;
 *
 *
 * RIPEMD-160: pseudo-code
 *
 *   It is assumed that the message after padding consists of t 16-word blocks
 *   that will be denoted with X[i][j], with 0 <= i <= t-1 and 0 <= j <= 15.
 *   The symbol [+] denotes addition modulo 2**32 and rol_s denotes cyclic left
 *   shift (rotate) over s positions.
 *
 *
 *   for i := 0 to t-1 {
 *	 A := h0; B := h1; C := h2; D = h3; E = h4;
 *	 A' := h0; B' := h1; C' := h2; D' = h3; E' = h4;
 *	 for j := 0 to 79 {
 *	     T := rol_s(j)(A [+] f(j, B, C, D) [+] X[i][r(j)] [+] K(j)) [+] E;
 *	     A := E; E := D; D := rol_10(C); C := B; B := T;
 *	     T := rol_s'(j)(A' [+] f(79-j, B', C', D') [+] X[i][r'(j)]
						       [+] K'(j)) [+] E';
 *	     A' := E'; E' := D'; D' := rol_10(C'); C' := B'; B' := T;
 *	 }
 *	 T := h1 [+] C [+] D'; h1 := h2 [+] D [+] E'; h2 := h3 [+] E [+] A';
 *	 h3 := h4 [+] A [+] B'; h4 := h0 [+] B [+] C'; h0 := T;
 *   }
 */

/* Some examples:
 * ""                    9c1185a5c5e9fc54612808977ee8f548b2258d31
 * "a"                   0bdc9d2d256b3ee9daae347be6f4dc835a467ffe
 * "abc"                 8eb208f7e05d987a9b044a8e98c6b087f15a0bfc
 * "message digest"      5d0689ef49d2fae572b881b123a85ffa21595f36
 * "a...z"               f71c27109c692c1b56bbdceb5b9d2865b3708dbc
 * "abcdbcde...nopq"     12a053384a9c0c88e405a06c27dcf49ada62eb2b
 * "A...Za...z0...9"     b0e20b6e3116640286ed3a87a5713079b21f5189
 * 8 times "1234567890"  9b752e45573d4b39f4dbd3323cab82bf63326bfb
 * 1 million times "a"   52783243c1697bdbe16d37f97f68f08325dc1528
 */

typedef struct
{
  gcry_md_block_ctx_t bctx;
  u32  h0,h1,h2,h3,h4;
} RMD160_CONTEXT;


static unsigned int
transform ( void *ctx, const unsigned char *data, size_t nblks );

static void
rmd160_init (void *context, unsigned int flags)
{
  RMD160_CONTEXT *hd = context;

  (void)flags;

  hd->h0 = 0x67452301;
  hd->h1 = 0xEFCDAB89;
  hd->h2 = 0x98BADCFE;
  hd->h3 = 0x10325476;
  hd->h4 = 0xC3D2E1F0;

  hd->bctx.nblocks = 0;
  hd->bctx.nblocks_high = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize_shift = _gcry_ctz(64);
  hd->bctx.bwrite = transform;
}


/****************
 * Transform the message X which consists of 16 32-bit-words
 */
static unsigned int
transform_blk ( void *ctx, const unsigned char *data )
{
  RMD160_CONTEXT *hd = ctx;
  register u32 al, ar, bl, br, cl, cr, dl, dr, el, er;
  u32 x[16];
  int i;

  for ( i = 0; i < 16; i++ )
    x[i] = buf_get_le32(data + i * 4);

#define K0  0x00000000
#define K1  0x5A827999
#define K2  0x6ED9EBA1
#define K3  0x8F1BBCDC
#define K4  0xA953FD4E
#define KK0 0x50A28BE6
#define KK1 0x5C4DD124
#define KK2 0x6D703EF3
#define KK3 0x7A6D76E9
#define KK4 0x00000000
#define F0(x,y,z)   ( (x) ^ (y) ^ (z) )
#define F1(x,y,z)   ( ((x) & (y)) | (~(x) & (z)) )
#define F2(x,y,z)   ( ((x) | ~(y)) ^ (z) )
#define F3(x,y,z)   ( ((x) & (z)) | ((y) & ~(z)) )
#define F4(x,y,z)   ( (x) ^ ((y) | ~(z)) )
#define R(a,b,c,d,e,f,k,r,s) do { a += f(b,c,d) + k + x[r]; \
				  a = rol(a,s) + e;	       \
				  c = rol(c,10);	       \
				} while(0)

  /* left lane and right lanes interleaved */
  al = ar = hd->h0;
  bl = br = hd->h1;
  cl = cr = hd->h2;
  dl = dr = hd->h3;
  el = er = hd->h4;
  R( al, bl, cl, dl, el, F0, K0,  0, 11 );
  R( ar, br, cr, dr, er, F4, KK0,	5,  8);
  R( el, al, bl, cl, dl, F0, K0,  1, 14 );
  R( er, ar, br, cr, dr, F4, KK0, 14,  9);
  R( dl, el, al, bl, cl, F0, K0,  2, 15 );
  R( dr, er, ar, br, cr, F4, KK0,	7,  9);
  R( cl, dl, el, al, bl, F0, K0,  3, 12 );
  R( cr, dr, er, ar, br, F4, KK0,	0, 11);
  R( bl, cl, dl, el, al, F0, K0,  4,  5 );
  R( br, cr, dr, er, ar, F4, KK0,	9, 13);
  R( al, bl, cl, dl, el, F0, K0,  5,  8 );
  R( ar, br, cr, dr, er, F4, KK0,	2, 15);
  R( el, al, bl, cl, dl, F0, K0,  6,  7 );
  R( er, ar, br, cr, dr, F4, KK0, 11, 15);
  R( dl, el, al, bl, cl, F0, K0,  7,  9 );
  R( dr, er, ar, br, cr, F4, KK0,	4,  5);
  R( cl, dl, el, al, bl, F0, K0,  8, 11 );
  R( cr, dr, er, ar, br, F4, KK0, 13,  7);
  R( bl, cl, dl, el, al, F0, K0,  9, 13 );
  R( br, cr, dr, er, ar, F4, KK0,	6,  7);
  R( al, bl, cl, dl, el, F0, K0, 10, 14 );
  R( ar, br, cr, dr, er, F4, KK0, 15,  8);
  R( el, al, bl, cl, dl, F0, K0, 11, 15 );
  R( er, ar, br, cr, dr, F4, KK0,	8, 11);
  R( dl, el, al, bl, cl, F0, K0, 12,  6 );
  R( dr, er, ar, br, cr, F4, KK0,	1, 14);
  R( cl, dl, el, al, bl, F0, K0, 13,  7 );
  R( cr, dr, er, ar, br, F4, KK0, 10, 14);
  R( bl, cl, dl, el, al, F0, K0, 14,  9 );
  R( br, cr, dr, er, ar, F4, KK0,	3, 12);
  R( al, bl, cl, dl, el, F0, K0, 15,  8 );
  R( ar, br, cr, dr, er, F4, KK0, 12,  6);
  R( el, al, bl, cl, dl, F1, K1,  7,  7 );
  R( er, ar, br, cr, dr, F3, KK1,	6,  9);
  R( dl, el, al, bl, cl, F1, K1,  4,  6 );
  R( dr, er, ar, br, cr, F3, KK1, 11, 13);
  R( cl, dl, el, al, bl, F1, K1, 13,  8 );
  R( cr, dr, er, ar, br, F3, KK1,	3, 15);
  R( bl, cl, dl, el, al, F1, K1,  1, 13 );
  R( br, cr, dr, er, ar, F3, KK1,	7,  7);
  R( al, bl, cl, dl, el, F1, K1, 10, 11 );
  R( ar, br, cr, dr, er, F3, KK1,	0, 12);
  R( el, al, bl, cl, dl, F1, K1,  6,  9 );
  R( er, ar, br, cr, dr, F3, KK1, 13,  8);
  R( dl, el, al, bl, cl, F1, K1, 15,  7 );
  R( dr, er, ar, br, cr, F3, KK1,	5,  9);
  R( cl, dl, el, al, bl, F1, K1,  3, 15 );
  R( cr, dr, er, ar, br, F3, KK1, 10, 11);
  R( bl, cl, dl, el, al, F1, K1, 12,  7 );
  R( br, cr, dr, er, ar, F3, KK1, 14,  7);
  R( al, bl, cl, dl, el, F1, K1,  0, 12 );
  R( ar, br, cr, dr, er, F3, KK1, 15,  7);
  R( el, al, bl, cl, dl, F1, K1,  9, 15 );
  R( er, ar, br, cr, dr, F3, KK1,	8, 12);
  R( dl, el, al, bl, cl, F1, K1,  5,  9 );
  R( dr, er, ar, br, cr, F3, KK1, 12,  7);
  R( cl, dl, el, al, bl, F1, K1,  2, 11 );
  R( cr, dr, er, ar, br, F3, KK1,	4,  6);
  R( bl, cl, dl, el, al, F1, K1, 14,  7 );
  R( br, cr, dr, er, ar, F3, KK1,	9, 15);
  R( al, bl, cl, dl, el, F1, K1, 11, 13 );
  R( ar, br, cr, dr, er, F3, KK1,	1, 13);
  R( el, al, bl, cl, dl, F1, K1,  8, 12 );
  R( er, ar, br, cr, dr, F3, KK1,	2, 11);
  R( dl, el, al, bl, cl, F2, K2,  3, 11 );
  R( dr, er, ar, br, cr, F2, KK2, 15,  9);
  R( cl, dl, el, al, bl, F2, K2, 10, 13 );
  R( cr, dr, er, ar, br, F2, KK2,	5,  7);
  R( bl, cl, dl, el, al, F2, K2, 14,  6 );
  R( br, cr, dr, er, ar, F2, KK2,	1, 15);
  R( al, bl, cl, dl, el, F2, K2,  4,  7 );
  R( ar, br, cr, dr, er, F2, KK2,	3, 11);
  R( el, al, bl, cl, dl, F2, K2,  9, 14 );
  R( er, ar, br, cr, dr, F2, KK2,	7,  8);
  R( dl, el, al, bl, cl, F2, K2, 15,  9 );
  R( dr, er, ar, br, cr, F2, KK2, 14,  6);
  R( cl, dl, el, al, bl, F2, K2,  8, 13 );
  R( cr, dr, er, ar, br, F2, KK2,	6,  6);
  R( bl, cl, dl, el, al, F2, K2,  1, 15 );
  R( br, cr, dr, er, ar, F2, KK2,	9, 14);
  R( al, bl, cl, dl, el, F2, K2,  2, 14 );
  R( ar, br, cr, dr, er, F2, KK2, 11, 12);
  R( el, al, bl, cl, dl, F2, K2,  7,  8 );
  R( er, ar, br, cr, dr, F2, KK2,	8, 13);
  R( dl, el, al, bl, cl, F2, K2,  0, 13 );
  R( dr, er, ar, br, cr, F2, KK2, 12,  5);
  R( cl, dl, el, al, bl, F2, K2,  6,  6 );
  R( cr, dr, er, ar, br, F2, KK2,	2, 14);
  R( bl, cl, dl, el, al, F2, K2, 13,  5 );
  R( br, cr, dr, er, ar, F2, KK2, 10, 13);
  R( al, bl, cl, dl, el, F2, K2, 11, 12 );
  R( ar, br, cr, dr, er, F2, KK2,	0, 13);
  R( el, al, bl, cl, dl, F2, K2,  5,  7 );
  R( er, ar, br, cr, dr, F2, KK2,	4,  7);
  R( dl, el, al, bl, cl, F2, K2, 12,  5 );
  R( dr, er, ar, br, cr, F2, KK2, 13,  5);
  R( cl, dl, el, al, bl, F3, K3,  1, 11 );
  R( cr, dr, er, ar, br, F1, KK3,	8, 15);
  R( bl, cl, dl, el, al, F3, K3,  9, 12 );
  R( br, cr, dr, er, ar, F1, KK3,	6,  5);
  R( al, bl, cl, dl, el, F3, K3, 11, 14 );
  R( ar, br, cr, dr, er, F1, KK3,	4,  8);
  R( el, al, bl, cl, dl, F3, K3, 10, 15 );
  R( er, ar, br, cr, dr, F1, KK3,	1, 11);
  R( dl, el, al, bl, cl, F3, K3,  0, 14 );
  R( dr, er, ar, br, cr, F1, KK3,	3, 14);
  R( cl, dl, el, al, bl, F3, K3,  8, 15 );
  R( cr, dr, er, ar, br, F1, KK3, 11, 14);
  R( bl, cl, dl, el, al, F3, K3, 12,  9 );
  R( br, cr, dr, er, ar, F1, KK3, 15,  6);
  R( al, bl, cl, dl, el, F3, K3,  4,  8 );
  R( ar, br, cr, dr, er, F1, KK3,	0, 14);
  R( el, al, bl, cl, dl, F3, K3, 13,  9 );
  R( er, ar, br, cr, dr, F1, KK3,	5,  6);
  R( dl, el, al, bl, cl, F3, K3,  3, 14 );
  R( dr, er, ar, br, cr, F1, KK3, 12,  9);
  R( cl, dl, el, al, bl, F3, K3,  7,  5 );
  R( cr, dr, er, ar, br, F1, KK3,	2, 12);
  R( bl, cl, dl, el, al, F3, K3, 15,  6 );
  R( br, cr, dr, er, ar, F1, KK3, 13,  9);
  R( al, bl, cl, dl, el, F3, K3, 14,  8 );
  R( ar, br, cr, dr, er, F1, KK3,	9, 12);
  R( el, al, bl, cl, dl, F3, K3,  5,  6 );
  R( er, ar, br, cr, dr, F1, KK3,	7,  5);
  R( dl, el, al, bl, cl, F3, K3,  6,  5 );
  R( dr, er, ar, br, cr, F1, KK3, 10, 15);
  R( cl, dl, el, al, bl, F3, K3,  2, 12 );
  R( cr, dr, er, ar, br, F1, KK3, 14,  8);
  R( bl, cl, dl, el, al, F4, K4,  4,  9 );
  R( br, cr, dr, er, ar, F0, KK4, 12,  8);
  R( al, bl, cl, dl, el, F4, K4,  0, 15 );
  R( ar, br, cr, dr, er, F0, KK4, 15,  5);
  R( el, al, bl, cl, dl, F4, K4,  5,  5 );
  R( er, ar, br, cr, dr, F0, KK4, 10, 12);
  R( dl, el, al, bl, cl, F4, K4,  9, 11 );
  R( dr, er, ar, br, cr, F0, KK4,	4,  9);
  R( cl, dl, el, al, bl, F4, K4,  7,  6 );
  R( cr, dr, er, ar, br, F0, KK4,	1, 12);
  R( bl, cl, dl, el, al, F4, K4, 12,  8 );
  R( br, cr, dr, er, ar, F0, KK4,	5,  5);
  R( al, bl, cl, dl, el, F4, K4,  2, 13 );
  R( ar, br, cr, dr, er, F0, KK4,	8, 14);
  R( el, al, bl, cl, dl, F4, K4, 10, 12 );
  R( er, ar, br, cr, dr, F0, KK4,	7,  6);
  R( dl, el, al, bl, cl, F4, K4, 14,  5 );
  R( dr, er, ar, br, cr, F0, KK4,	6,  8);
  R( cl, dl, el, al, bl, F4, K4,  1, 12 );
  R( cr, dr, er, ar, br, F0, KK4,	2, 13);
  R( bl, cl, dl, el, al, F4, K4,  3, 13 );
  R( br, cr, dr, er, ar, F0, KK4, 13,  6);
  R( al, bl, cl, dl, el, F4, K4,  8, 14 );
  R( ar, br, cr, dr, er, F0, KK4, 14,  5);
  R( el, al, bl, cl, dl, F4, K4, 11, 11 );
  R( er, ar, br, cr, dr, F0, KK4,	0, 15);
  R( dl, el, al, bl, cl, F4, K4,  6,  8 );
  R( dr, er, ar, br, cr, F0, KK4,	3, 13);
  R( cl, dl, el, al, bl, F4, K4, 15,  5 );
  R( cr, dr, er, ar, br, F0, KK4,	9, 11);
  R( bl, cl, dl, el, al, F4, K4, 13,  6 );
  R( br, cr, dr, er, ar, F0, KK4, 11, 11);

  dr += cl + hd->h1;
  hd->h1 = hd->h2 + dl + er;
  hd->h2 = hd->h3 + el + ar;
  hd->h3 = hd->h4 + al + br;
  hd->h4 = hd->h0 + bl + cr;
  hd->h0 = dr;

  return /*burn_stack*/ 104+5*sizeof(void*);
}


static unsigned int
transform ( void *c, const unsigned char *data, size_t nblks )
{
  unsigned int burn;

  do
    {
      burn = transform_blk (c, data);
      data += 64;
    }
  while (--nblks);

  return burn;
}


/*
 * The routine terminates the computation
 */
static void
rmd160_final( void *context )
{
  RMD160_CONTEXT *hd = context;
  u32 t, th, msb, lsb;
  byte *p;
  unsigned int burn;

  t = hd->bctx.nblocks;
  if (sizeof t == sizeof hd->bctx.nblocks)
    th = hd->bctx.nblocks_high;
  else
    th = hd->bctx.nblocks >> 32;

  /* multiply by 64 to make a byte count */
  lsb = t << 6;
  msb = (th << 6) | (t >> 26);
  /* add the count */
  t = lsb;
  if( (lsb += hd->bctx.count) < t )
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 29;

  if (hd->bctx.count < 56)  /* enough room */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad */
      if (hd->bctx.count < 56)
	memset (&hd->bctx.buf[hd->bctx.count], 0, 56 - hd->bctx.count);

      /* append the 64 bit count */
      buf_put_le32(hd->bctx.buf + 56, lsb);
      buf_put_le32(hd->bctx.buf + 60, msb);
      burn = transform (hd, hd->bctx.buf, 1);
    }
  else /* need one extra block */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad character */
      /* fill pad and next block with zeroes */
      memset (&hd->bctx.buf[hd->bctx.count], 0, 64 - hd->bctx.count + 56);

      /* append the 64 bit count */
      buf_put_le32(hd->bctx.buf + 64 + 56, lsb);
      buf_put_le32(hd->bctx.buf + 64 + 60, msb);
      burn = transform (hd, hd->bctx.buf, 2);
    }

  p = hd->bctx.buf;
#define X(a) do { buf_put_le32(p, hd->h##a); p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
#undef X

  hd->bctx.count = 0;

  _gcry_burn_stack (burn);
}

static byte *
rmd160_read( void *context )
{
  RMD160_CONTEXT *hd = context;

  return hd->bctx.buf;
}



/****************
 * Shortcut functions which puts the hash value of the supplied buffer iov
 * into outbuf which must have a size of 20 bytes.
 */
static void
_gcry_rmd160_hash_buffers (void *outbuf, size_t nbytes,
			   const gcry_buffer_t *iov, int iovcnt)
{
  RMD160_CONTEXT hd;

  (void)nbytes;

  rmd160_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  rmd160_final ( &hd );
  memcpy ( outbuf, hd.bctx.buf, 20 );
}


static const byte asn[15] = /* Object ID is 1.3.36.3.2.1 */
  { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x24, 0x03,
    0x02, 0x01, 0x05, 0x00, 0x04, 0x14 };

static const gcry_md_oid_spec_t oid_spec_rmd160[] =
  {
    /* rsaSignatureWithripemd160 */
    { "1.3.36.3.3.1.2" },
    /* TeleTrust hash algorithm.  */
    { "1.3.36.3.2.1" },
    { NULL }
  };

const gcry_md_spec_t _gcry_digest_spec_rmd160 =
  {
    GCRY_MD_RMD160, {0, 0},
    "RIPEMD160", asn, DIM (asn), oid_spec_rmd160, 20,
    rmd160_init, _gcry_md_block_write, rmd160_final, rmd160_read, NULL,
    _gcry_rmd160_hash_buffers,
    sizeof (RMD160_CONTEXT)
  };
