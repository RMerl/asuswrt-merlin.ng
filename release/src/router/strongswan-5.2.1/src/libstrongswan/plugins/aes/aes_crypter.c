/*
 * Copyright (C) 2001 Dr B. R. Gladman <brg@gladman.uk.net>
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "aes_crypter.h"

/*
 * The number of key schedule words for different block and key lengths
 * allowing for method of computation which requires the length to be a
 * multiple of the key length. This version of AES implementation supports
 * all three keylengths 16, 24 and 32 bytes!
 *
 * Nk =       4   6   8
 *        -------------
 * Nb = 4 |  60  60  64
 *      6 |  96  90  96
 *      8 | 120 120 120
 */
#define AES_KS_LENGTH   120
#define AES_RC_LENGTH    29

typedef struct private_aes_crypter_t private_aes_crypter_t;

/**
 * Class implementing the AES symmetric encryption algorithm.
 *
 * @ingroup crypters
 */
struct private_aes_crypter_t {

	/**
	 * Public part of this class.
	 */
	aes_crypter_t public;

	/**
	 * Number of words in the key input block.
	 */
	u_int32_t aes_Nkey;

	/**
	 * The number of cipher rounds.
	 */
	u_int32_t aes_Nrnd;

	/**
	* The encryption key schedule.
	*/
	u_int32_t aes_e_key[AES_KS_LENGTH];

	/**
	* The decryption key schedule.
	*/
	u_int32_t aes_d_key[AES_KS_LENGTH];

	/**
	* Key size of this AES cypher object.
	*/
	u_int32_t key_size;
};

/**
 * Rotates bytes within words by n positions, moving bytes
 * to higher index positions with wrap around into low positions.
 */
#define upr(x,n)        (((x) << 8 * (n)) | ((x) >> (32 - 8 * (n))))
/**
 * Moves bytes by n positions to higher index positions in
 * words but without wrap around.
 */
#define ups(x,n)        ((x) << 8 * (n))

/**
 * Extracts a byte from a word.
 */
#define bval(x,n)       ((unsigned char)((x) >> 8 * (n)))
#define bytes2word(b0, b1, b2, b3)  \
        ((u_int32_t)(b3) << 24 | (u_int32_t)(b2) << 16 | (u_int32_t)(b1) << 8 | (b0))


/* little endian processor without data alignment restrictions: AES_LE_OK */
/* original code: i386 */
#if defined(i386) || defined(_I386) || defined(__i386__) || defined(__i386)
#define	AES_LE_OK 1
/* added (tested): alpha  --jjo */
#elif defined(__alpha__)|| defined (__alpha)
#define AES_LE_OK 1
/* added (tested): ia64  --jjo */
#elif defined(__ia64__)|| defined (__ia64)
#define AES_LE_OK 1
#endif

#ifdef AES_LE_OK
/* little endian processor without data alignment restrictions */
#define word_in(x)      *(u_int32_t*)(x)
#define const_word_in(x)      *(const u_int32_t*)(x)
#define word_out(x,v)   *(u_int32_t*)(x) = (v)
#define const_word_out(x,v)   *(const u_int32_t*)(x) = (v)
#else
/* slower but generic big endian or with data alignment restrictions */
/* some additional "const" touches to stop "gcc -Wcast-qual" complains --jjo */
#define word_in(x)      ((u_int32_t)(((unsigned char *)(x))[0])|((u_int32_t)(((unsigned char *)(x))[1])<<8)|((u_int32_t)(((unsigned char *)(x))[2])<<16)|((u_int32_t)(((unsigned char *)(x))[3])<<24))
#define const_word_in(x)      ((const u_int32_t)(((const unsigned char *)(x))[0])|((const u_int32_t)(((const unsigned char *)(x))[1])<<8)|((const u_int32_t)(((const unsigned char *)(x))[2])<<16)|((const u_int32_t)(((const unsigned char *)(x))[3])<<24))
#define word_out(x,v)   ((unsigned char *)(x))[0]=(v),((unsigned char *)(x))[1]=((v)>>8),((unsigned char *)(x))[2]=((v)>>16),((unsigned char *)(x))[3]=((v)>>24)
#define const_word_out(x,v)   ((const unsigned char *)(x))[0]=(v),((const unsigned char *)(x))[1]=((v)>>8),((const unsigned char *)(x))[2]=((v)>>16),((const unsigned char *)(x))[3]=((v)>>24)
#endif

// the finite field modular polynomial and elements

#define ff_poly 0x011b
#define ff_hi   0x80

// multiply four bytes in GF(2^8) by 'x' {02} in parallel

#define m1  0x80808080
#define m2  0x7f7f7f7f
#define m3  0x0000001b
#define FFmulX(x)  ((((x) & m2) << 1) ^ ((((x) & m1) >> 7) * m3))

// The following defines provide alternative definitions of FFmulX that might
// give improved performance if a fast 32-bit multiply is not available. Note
// that a temporary variable u needs to be defined where FFmulX is used.

// #define FFmulX(x) (u = (x) & m1, u |= (u >> 1), ((x) & m2) << 1) ^ ((u >> 3) | (u >> 6))
// #define m4  0x1b1b1b1b
// #define FFmulX(x) (u = (x) & m1, ((x) & m2) << 1) ^ ((u - (u >> 7)) & m4)

// perform column mix operation on four bytes in parallel

#define fwd_mcol(x) (f2 = FFmulX(x), f2 ^ upr(x ^ f2,3) ^ upr(x,2) ^ upr(x,1))

#define w0(p)          0x000000##p

// Number of elements required in this table for different
// block and key lengths is:
//
// Nk =      4  6  8
//        ----------
// Nb = 4 | 10  8  7
//      6 | 19 12 11
//      8 | 29 19 14
//
// this table can be a table of bytes if the key schedule
// code is adjusted accordingly

static const u_int32_t rcon_tab[29] =
{
    w0(01), w0(02), w0(04), w0(08),
    w0(10), w0(20), w0(40), w0(80),
    w0(1b), w0(36), w0(6c), w0(d8),
    w0(ab), w0(4d), w0(9a), w0(2f),
    w0(5e), w0(bc), w0(63), w0(c6),
    w0(97), w0(35), w0(6a), w0(d4),
    w0(b3), w0(7d), w0(fa), w0(ef),
    w0(c5)
};

#undef  w0

#define r0(p,q,r,s) 0x##p##q##r##s
#define r1(p,q,r,s) 0x##q##r##s##p
#define r2(p,q,r,s) 0x##r##s##p##q
#define r3(p,q,r,s) 0x##s##p##q##r
#define w0(p)          0x000000##p
#define w1(p)        0x0000##p##00
#define w2(p)        0x00##p##0000
#define w3(p)        0x##p##000000

//  data for forward tables (other than last round)

#define f_table \
    r(a5,63,63,c6), r(84,7c,7c,f8), r(99,77,77,ee), r(8d,7b,7b,f6),\
    r(0d,f2,f2,ff), r(bd,6b,6b,d6), r(b1,6f,6f,de), r(54,c5,c5,91),\
    r(50,30,30,60), r(03,01,01,02), r(a9,67,67,ce), r(7d,2b,2b,56),\
    r(19,fe,fe,e7), r(62,d7,d7,b5), r(e6,ab,ab,4d), r(9a,76,76,ec),\
    r(45,ca,ca,8f), r(9d,82,82,1f), r(40,c9,c9,89), r(87,7d,7d,fa),\
    r(15,fa,fa,ef), r(eb,59,59,b2), r(c9,47,47,8e), r(0b,f0,f0,fb),\
    r(ec,ad,ad,41), r(67,d4,d4,b3), r(fd,a2,a2,5f), r(ea,af,af,45),\
    r(bf,9c,9c,23), r(f7,a4,a4,53), r(96,72,72,e4), r(5b,c0,c0,9b),\
    r(c2,b7,b7,75), r(1c,fd,fd,e1), r(ae,93,93,3d), r(6a,26,26,4c),\
    r(5a,36,36,6c), r(41,3f,3f,7e), r(02,f7,f7,f5), r(4f,cc,cc,83),\
    r(5c,34,34,68), r(f4,a5,a5,51), r(34,e5,e5,d1), r(08,f1,f1,f9),\
    r(93,71,71,e2), r(73,d8,d8,ab), r(53,31,31,62), r(3f,15,15,2a),\
    r(0c,04,04,08), r(52,c7,c7,95), r(65,23,23,46), r(5e,c3,c3,9d),\
    r(28,18,18,30), r(a1,96,96,37), r(0f,05,05,0a), r(b5,9a,9a,2f),\
    r(09,07,07,0e), r(36,12,12,24), r(9b,80,80,1b), r(3d,e2,e2,df),\
    r(26,eb,eb,cd), r(69,27,27,4e), r(cd,b2,b2,7f), r(9f,75,75,ea),\
    r(1b,09,09,12), r(9e,83,83,1d), r(74,2c,2c,58), r(2e,1a,1a,34),\
    r(2d,1b,1b,36), r(b2,6e,6e,dc), r(ee,5a,5a,b4), r(fb,a0,a0,5b),\
    r(f6,52,52,a4), r(4d,3b,3b,76), r(61,d6,d6,b7), r(ce,b3,b3,7d),\
    r(7b,29,29,52), r(3e,e3,e3,dd), r(71,2f,2f,5e), r(97,84,84,13),\
    r(f5,53,53,a6), r(68,d1,d1,b9), r(00,00,00,00), r(2c,ed,ed,c1),\
    r(60,20,20,40), r(1f,fc,fc,e3), r(c8,b1,b1,79), r(ed,5b,5b,b6),\
    r(be,6a,6a,d4), r(46,cb,cb,8d), r(d9,be,be,67), r(4b,39,39,72),\
    r(de,4a,4a,94), r(d4,4c,4c,98), r(e8,58,58,b0), r(4a,cf,cf,85),\
    r(6b,d0,d0,bb), r(2a,ef,ef,c5), r(e5,aa,aa,4f), r(16,fb,fb,ed),\
    r(c5,43,43,86), r(d7,4d,4d,9a), r(55,33,33,66), r(94,85,85,11),\
    r(cf,45,45,8a), r(10,f9,f9,e9), r(06,02,02,04), r(81,7f,7f,fe),\
    r(f0,50,50,a0), r(44,3c,3c,78), r(ba,9f,9f,25), r(e3,a8,a8,4b),\
    r(f3,51,51,a2), r(fe,a3,a3,5d), r(c0,40,40,80), r(8a,8f,8f,05),\
    r(ad,92,92,3f), r(bc,9d,9d,21), r(48,38,38,70), r(04,f5,f5,f1),\
    r(df,bc,bc,63), r(c1,b6,b6,77), r(75,da,da,af), r(63,21,21,42),\
    r(30,10,10,20), r(1a,ff,ff,e5), r(0e,f3,f3,fd), r(6d,d2,d2,bf),\
    r(4c,cd,cd,81), r(14,0c,0c,18), r(35,13,13,26), r(2f,ec,ec,c3),\
    r(e1,5f,5f,be), r(a2,97,97,35), r(cc,44,44,88), r(39,17,17,2e),\
    r(57,c4,c4,93), r(f2,a7,a7,55), r(82,7e,7e,fc), r(47,3d,3d,7a),\
    r(ac,64,64,c8), r(e7,5d,5d,ba), r(2b,19,19,32), r(95,73,73,e6),\
    r(a0,60,60,c0), r(98,81,81,19), r(d1,4f,4f,9e), r(7f,dc,dc,a3),\
    r(66,22,22,44), r(7e,2a,2a,54), r(ab,90,90,3b), r(83,88,88,0b),\
    r(ca,46,46,8c), r(29,ee,ee,c7), r(d3,b8,b8,6b), r(3c,14,14,28),\
    r(79,de,de,a7), r(e2,5e,5e,bc), r(1d,0b,0b,16), r(76,db,db,ad),\
    r(3b,e0,e0,db), r(56,32,32,64), r(4e,3a,3a,74), r(1e,0a,0a,14),\
    r(db,49,49,92), r(0a,06,06,0c), r(6c,24,24,48), r(e4,5c,5c,b8),\
    r(5d,c2,c2,9f), r(6e,d3,d3,bd), r(ef,ac,ac,43), r(a6,62,62,c4),\
    r(a8,91,91,39), r(a4,95,95,31), r(37,e4,e4,d3), r(8b,79,79,f2),\
    r(32,e7,e7,d5), r(43,c8,c8,8b), r(59,37,37,6e), r(b7,6d,6d,da),\
    r(8c,8d,8d,01), r(64,d5,d5,b1), r(d2,4e,4e,9c), r(e0,a9,a9,49),\
    r(b4,6c,6c,d8), r(fa,56,56,ac), r(07,f4,f4,f3), r(25,ea,ea,cf),\
    r(af,65,65,ca), r(8e,7a,7a,f4), r(e9,ae,ae,47), r(18,08,08,10),\
    r(d5,ba,ba,6f), r(88,78,78,f0), r(6f,25,25,4a), r(72,2e,2e,5c),\
    r(24,1c,1c,38), r(f1,a6,a6,57), r(c7,b4,b4,73), r(51,c6,c6,97),\
    r(23,e8,e8,cb), r(7c,dd,dd,a1), r(9c,74,74,e8), r(21,1f,1f,3e),\
    r(dd,4b,4b,96), r(dc,bd,bd,61), r(86,8b,8b,0d), r(85,8a,8a,0f),\
    r(90,70,70,e0), r(42,3e,3e,7c), r(c4,b5,b5,71), r(aa,66,66,cc),\
    r(d8,48,48,90), r(05,03,03,06), r(01,f6,f6,f7), r(12,0e,0e,1c),\
    r(a3,61,61,c2), r(5f,35,35,6a), r(f9,57,57,ae), r(d0,b9,b9,69),\
    r(91,86,86,17), r(58,c1,c1,99), r(27,1d,1d,3a), r(b9,9e,9e,27),\
    r(38,e1,e1,d9), r(13,f8,f8,eb), r(b3,98,98,2b), r(33,11,11,22),\
    r(bb,69,69,d2), r(70,d9,d9,a9), r(89,8e,8e,07), r(a7,94,94,33),\
    r(b6,9b,9b,2d), r(22,1e,1e,3c), r(92,87,87,15), r(20,e9,e9,c9),\
    r(49,ce,ce,87), r(ff,55,55,aa), r(78,28,28,50), r(7a,df,df,a5),\
    r(8f,8c,8c,03), r(f8,a1,a1,59), r(80,89,89,09), r(17,0d,0d,1a),\
    r(da,bf,bf,65), r(31,e6,e6,d7), r(c6,42,42,84), r(b8,68,68,d0),\
    r(c3,41,41,82), r(b0,99,99,29), r(77,2d,2d,5a), r(11,0f,0f,1e),\
    r(cb,b0,b0,7b), r(fc,54,54,a8), r(d6,bb,bb,6d), r(3a,16,16,2c)

//  data for inverse tables (other than last round)

#define i_table \
    r(50,a7,f4,51), r(53,65,41,7e), r(c3,a4,17,1a), r(96,5e,27,3a),\
    r(cb,6b,ab,3b), r(f1,45,9d,1f), r(ab,58,fa,ac), r(93,03,e3,4b),\
    r(55,fa,30,20), r(f6,6d,76,ad), r(91,76,cc,88), r(25,4c,02,f5),\
    r(fc,d7,e5,4f), r(d7,cb,2a,c5), r(80,44,35,26), r(8f,a3,62,b5),\
    r(49,5a,b1,de), r(67,1b,ba,25), r(98,0e,ea,45), r(e1,c0,fe,5d),\
    r(02,75,2f,c3), r(12,f0,4c,81), r(a3,97,46,8d), r(c6,f9,d3,6b),\
    r(e7,5f,8f,03), r(95,9c,92,15), r(eb,7a,6d,bf), r(da,59,52,95),\
    r(2d,83,be,d4), r(d3,21,74,58), r(29,69,e0,49), r(44,c8,c9,8e),\
    r(6a,89,c2,75), r(78,79,8e,f4), r(6b,3e,58,99), r(dd,71,b9,27),\
    r(b6,4f,e1,be), r(17,ad,88,f0), r(66,ac,20,c9), r(b4,3a,ce,7d),\
    r(18,4a,df,63), r(82,31,1a,e5), r(60,33,51,97), r(45,7f,53,62),\
    r(e0,77,64,b1), r(84,ae,6b,bb), r(1c,a0,81,fe), r(94,2b,08,f9),\
    r(58,68,48,70), r(19,fd,45,8f), r(87,6c,de,94), r(b7,f8,7b,52),\
    r(23,d3,73,ab), r(e2,02,4b,72), r(57,8f,1f,e3), r(2a,ab,55,66),\
    r(07,28,eb,b2), r(03,c2,b5,2f), r(9a,7b,c5,86), r(a5,08,37,d3),\
    r(f2,87,28,30), r(b2,a5,bf,23), r(ba,6a,03,02), r(5c,82,16,ed),\
    r(2b,1c,cf,8a), r(92,b4,79,a7), r(f0,f2,07,f3), r(a1,e2,69,4e),\
    r(cd,f4,da,65), r(d5,be,05,06), r(1f,62,34,d1), r(8a,fe,a6,c4),\
    r(9d,53,2e,34), r(a0,55,f3,a2), r(32,e1,8a,05), r(75,eb,f6,a4),\
    r(39,ec,83,0b), r(aa,ef,60,40), r(06,9f,71,5e), r(51,10,6e,bd),\
    r(f9,8a,21,3e), r(3d,06,dd,96), r(ae,05,3e,dd), r(46,bd,e6,4d),\
    r(b5,8d,54,91), r(05,5d,c4,71), r(6f,d4,06,04), r(ff,15,50,60),\
    r(24,fb,98,19), r(97,e9,bd,d6), r(cc,43,40,89), r(77,9e,d9,67),\
    r(bd,42,e8,b0), r(88,8b,89,07), r(38,5b,19,e7), r(db,ee,c8,79),\
    r(47,0a,7c,a1), r(e9,0f,42,7c), r(c9,1e,84,f8), r(00,00,00,00),\
    r(83,86,80,09), r(48,ed,2b,32), r(ac,70,11,1e), r(4e,72,5a,6c),\
    r(fb,ff,0e,fd), r(56,38,85,0f), r(1e,d5,ae,3d), r(27,39,2d,36),\
    r(64,d9,0f,0a), r(21,a6,5c,68), r(d1,54,5b,9b), r(3a,2e,36,24),\
    r(b1,67,0a,0c), r(0f,e7,57,93), r(d2,96,ee,b4), r(9e,91,9b,1b),\
    r(4f,c5,c0,80), r(a2,20,dc,61), r(69,4b,77,5a), r(16,1a,12,1c),\
    r(0a,ba,93,e2), r(e5,2a,a0,c0), r(43,e0,22,3c), r(1d,17,1b,12),\
    r(0b,0d,09,0e), r(ad,c7,8b,f2), r(b9,a8,b6,2d), r(c8,a9,1e,14),\
    r(85,19,f1,57), r(4c,07,75,af), r(bb,dd,99,ee), r(fd,60,7f,a3),\
    r(9f,26,01,f7), r(bc,f5,72,5c), r(c5,3b,66,44), r(34,7e,fb,5b),\
    r(76,29,43,8b), r(dc,c6,23,cb), r(68,fc,ed,b6), r(63,f1,e4,b8),\
    r(ca,dc,31,d7), r(10,85,63,42), r(40,22,97,13), r(20,11,c6,84),\
    r(7d,24,4a,85), r(f8,3d,bb,d2), r(11,32,f9,ae), r(6d,a1,29,c7),\
    r(4b,2f,9e,1d), r(f3,30,b2,dc), r(ec,52,86,0d), r(d0,e3,c1,77),\
    r(6c,16,b3,2b), r(99,b9,70,a9), r(fa,48,94,11), r(22,64,e9,47),\
    r(c4,8c,fc,a8), r(1a,3f,f0,a0), r(d8,2c,7d,56), r(ef,90,33,22),\
    r(c7,4e,49,87), r(c1,d1,38,d9), r(fe,a2,ca,8c), r(36,0b,d4,98),\
    r(cf,81,f5,a6), r(28,de,7a,a5), r(26,8e,b7,da), r(a4,bf,ad,3f),\
    r(e4,9d,3a,2c), r(0d,92,78,50), r(9b,cc,5f,6a), r(62,46,7e,54),\
    r(c2,13,8d,f6), r(e8,b8,d8,90), r(5e,f7,39,2e), r(f5,af,c3,82),\
    r(be,80,5d,9f), r(7c,93,d0,69), r(a9,2d,d5,6f), r(b3,12,25,cf),\
    r(3b,99,ac,c8), r(a7,7d,18,10), r(6e,63,9c,e8), r(7b,bb,3b,db),\
    r(09,78,26,cd), r(f4,18,59,6e), r(01,b7,9a,ec), r(a8,9a,4f,83),\
    r(65,6e,95,e6), r(7e,e6,ff,aa), r(08,cf,bc,21), r(e6,e8,15,ef),\
    r(d9,9b,e7,ba), r(ce,36,6f,4a), r(d4,09,9f,ea), r(d6,7c,b0,29),\
    r(af,b2,a4,31), r(31,23,3f,2a), r(30,94,a5,c6), r(c0,66,a2,35),\
    r(37,bc,4e,74), r(a6,ca,82,fc), r(b0,d0,90,e0), r(15,d8,a7,33),\
    r(4a,98,04,f1), r(f7,da,ec,41), r(0e,50,cd,7f), r(2f,f6,91,17),\
    r(8d,d6,4d,76), r(4d,b0,ef,43), r(54,4d,aa,cc), r(df,04,96,e4),\
    r(e3,b5,d1,9e), r(1b,88,6a,4c), r(b8,1f,2c,c1), r(7f,51,65,46),\
    r(04,ea,5e,9d), r(5d,35,8c,01), r(73,74,87,fa), r(2e,41,0b,fb),\
    r(5a,1d,67,b3), r(52,d2,db,92), r(33,56,10,e9), r(13,47,d6,6d),\
    r(8c,61,d7,9a), r(7a,0c,a1,37), r(8e,14,f8,59), r(89,3c,13,eb),\
    r(ee,27,a9,ce), r(35,c9,61,b7), r(ed,e5,1c,e1), r(3c,b1,47,7a),\
    r(59,df,d2,9c), r(3f,73,f2,55), r(79,ce,14,18), r(bf,37,c7,73),\
    r(ea,cd,f7,53), r(5b,aa,fd,5f), r(14,6f,3d,df), r(86,db,44,78),\
    r(81,f3,af,ca), r(3e,c4,68,b9), r(2c,34,24,38), r(5f,40,a3,c2),\
    r(72,c3,1d,16), r(0c,25,e2,bc), r(8b,49,3c,28), r(41,95,0d,ff),\
    r(71,01,a8,39), r(de,b3,0c,08), r(9c,e4,b4,d8), r(90,c1,56,64),\
    r(61,84,cb,7b), r(70,b6,32,d5), r(74,5c,6c,48), r(42,57,b8,d0)

// generate the required tables in the desired endian format

#undef  r
#define r   r0

static const u_int32_t ft_tab[4][256] =
{   {   f_table },
#undef  r
#define r   r1
    {   f_table },
#undef  r
#define r   r2
    {   f_table },
#undef  r
#define r   r3
    {   f_table }
};

#undef  r
#define r   r0
static const u_int32_t it_tab[4][256] =
{   {   i_table },
#undef  r
#define r   r1
    {   i_table },
#undef  r
#define r   r2
    {   i_table },
#undef  r
#define r   r3
    {   i_table }
};

#define li_table    \
    w(52), w(09), w(6a), w(d5), w(30), w(36), w(a5), w(38),\
    w(bf), w(40), w(a3), w(9e), w(81), w(f3), w(d7), w(fb),\
    w(7c), w(e3), w(39), w(82), w(9b), w(2f), w(ff), w(87),\
    w(34), w(8e), w(43), w(44), w(c4), w(de), w(e9), w(cb),\
    w(54), w(7b), w(94), w(32), w(a6), w(c2), w(23), w(3d),\
    w(ee), w(4c), w(95), w(0b), w(42), w(fa), w(c3), w(4e),\
    w(08), w(2e), w(a1), w(66), w(28), w(d9), w(24), w(b2),\
    w(76), w(5b), w(a2), w(49), w(6d), w(8b), w(d1), w(25),\
    w(72), w(f8), w(f6), w(64), w(86), w(68), w(98), w(16),\
    w(d4), w(a4), w(5c), w(cc), w(5d), w(65), w(b6), w(92),\
    w(6c), w(70), w(48), w(50), w(fd), w(ed), w(b9), w(da),\
    w(5e), w(15), w(46), w(57), w(a7), w(8d), w(9d), w(84),\
    w(90), w(d8), w(ab), w(00), w(8c), w(bc), w(d3), w(0a),\
    w(f7), w(e4), w(58), w(05), w(b8), w(b3), w(45), w(06),\
    w(d0), w(2c), w(1e), w(8f), w(ca), w(3f), w(0f), w(02),\
    w(c1), w(af), w(bd), w(03), w(01), w(13), w(8a), w(6b),\
    w(3a), w(91), w(11), w(41), w(4f), w(67), w(dc), w(ea),\
    w(97), w(f2), w(cf), w(ce), w(f0), w(b4), w(e6), w(73),\
    w(96), w(ac), w(74), w(22), w(e7), w(ad), w(35), w(85),\
    w(e2), w(f9), w(37), w(e8), w(1c), w(75), w(df), w(6e),\
    w(47), w(f1), w(1a), w(71), w(1d), w(29), w(c5), w(89),\
    w(6f), w(b7), w(62), w(0e), w(aa), w(18), w(be), w(1b),\
    w(fc), w(56), w(3e), w(4b), w(c6), w(d2), w(79), w(20),\
    w(9a), w(db), w(c0), w(fe), w(78), w(cd), w(5a), w(f4),\
    w(1f), w(dd), w(a8), w(33), w(88), w(07), w(c7), w(31),\
    w(b1), w(12), w(10), w(59), w(27), w(80), w(ec), w(5f),\
    w(60), w(51), w(7f), w(a9), w(19), w(b5), w(4a), w(0d),\
    w(2d), w(e5), w(7a), w(9f), w(93), w(c9), w(9c), w(ef),\
    w(a0), w(e0), w(3b), w(4d), w(ae), w(2a), w(f5), w(b0),\
    w(c8), w(eb), w(bb), w(3c), w(83), w(53), w(99), w(61),\
    w(17), w(2b), w(04), w(7e), w(ba), w(77), w(d6), w(26),\
    w(e1), w(69), w(14), w(63), w(55), w(21), w(0c), w(7d),

// generate the required tables in the desired endian format

#undef  r
#define r(p,q,r,s)  w0(q)
static const u_int32_t fl_tab[4][256] =
{   {   f_table    },
#undef  r
#define r(p,q,r,s)   w1(q)
    {   f_table    },
#undef  r
#define r(p,q,r,s)   w2(q)
    {   f_table    },
#undef  r
#define r(p,q,r,s)   w3(q)
    {   f_table    }
};

#undef  w
#define w   w0
static const u_int32_t il_tab[4][256] =
{   {   li_table    },
#undef  w
#define w   w1
    {   li_table    },
#undef  w
#define w   w2
    {   li_table    },
#undef  w
#define w   w3
    {   li_table    }
};

#define m_table \
    r(00,00,00,00), r(0b,0d,09,0e), r(16,1a,12,1c), r(1d,17,1b,12),\
    r(2c,34,24,38), r(27,39,2d,36), r(3a,2e,36,24), r(31,23,3f,2a),\
    r(58,68,48,70), r(53,65,41,7e), r(4e,72,5a,6c), r(45,7f,53,62),\
    r(74,5c,6c,48), r(7f,51,65,46), r(62,46,7e,54), r(69,4b,77,5a),\
    r(b0,d0,90,e0), r(bb,dd,99,ee), r(a6,ca,82,fc), r(ad,c7,8b,f2),\
    r(9c,e4,b4,d8), r(97,e9,bd,d6), r(8a,fe,a6,c4), r(81,f3,af,ca),\
    r(e8,b8,d8,90), r(e3,b5,d1,9e), r(fe,a2,ca,8c), r(f5,af,c3,82),\
    r(c4,8c,fc,a8), r(cf,81,f5,a6), r(d2,96,ee,b4), r(d9,9b,e7,ba),\
    r(7b,bb,3b,db), r(70,b6,32,d5), r(6d,a1,29,c7), r(66,ac,20,c9),\
    r(57,8f,1f,e3), r(5c,82,16,ed), r(41,95,0d,ff), r(4a,98,04,f1),\
    r(23,d3,73,ab), r(28,de,7a,a5), r(35,c9,61,b7), r(3e,c4,68,b9),\
    r(0f,e7,57,93), r(04,ea,5e,9d), r(19,fd,45,8f), r(12,f0,4c,81),\
    r(cb,6b,ab,3b), r(c0,66,a2,35), r(dd,71,b9,27), r(d6,7c,b0,29),\
    r(e7,5f,8f,03), r(ec,52,86,0d), r(f1,45,9d,1f), r(fa,48,94,11),\
    r(93,03,e3,4b), r(98,0e,ea,45), r(85,19,f1,57), r(8e,14,f8,59),\
    r(bf,37,c7,73), r(b4,3a,ce,7d), r(a9,2d,d5,6f), r(a2,20,dc,61),\
    r(f6,6d,76,ad), r(fd,60,7f,a3), r(e0,77,64,b1), r(eb,7a,6d,bf),\
    r(da,59,52,95), r(d1,54,5b,9b), r(cc,43,40,89), r(c7,4e,49,87),\
    r(ae,05,3e,dd), r(a5,08,37,d3), r(b8,1f,2c,c1), r(b3,12,25,cf),\
    r(82,31,1a,e5), r(89,3c,13,eb), r(94,2b,08,f9), r(9f,26,01,f7),\
    r(46,bd,e6,4d), r(4d,b0,ef,43), r(50,a7,f4,51), r(5b,aa,fd,5f),\
    r(6a,89,c2,75), r(61,84,cb,7b), r(7c,93,d0,69), r(77,9e,d9,67),\
    r(1e,d5,ae,3d), r(15,d8,a7,33), r(08,cf,bc,21), r(03,c2,b5,2f),\
    r(32,e1,8a,05), r(39,ec,83,0b), r(24,fb,98,19), r(2f,f6,91,17),\
    r(8d,d6,4d,76), r(86,db,44,78), r(9b,cc,5f,6a), r(90,c1,56,64),\
    r(a1,e2,69,4e), r(aa,ef,60,40), r(b7,f8,7b,52), r(bc,f5,72,5c),\
    r(d5,be,05,06), r(de,b3,0c,08), r(c3,a4,17,1a), r(c8,a9,1e,14),\
    r(f9,8a,21,3e), r(f2,87,28,30), r(ef,90,33,22), r(e4,9d,3a,2c),\
    r(3d,06,dd,96), r(36,0b,d4,98), r(2b,1c,cf,8a), r(20,11,c6,84),\
    r(11,32,f9,ae), r(1a,3f,f0,a0), r(07,28,eb,b2), r(0c,25,e2,bc),\
    r(65,6e,95,e6), r(6e,63,9c,e8), r(73,74,87,fa), r(78,79,8e,f4),\
    r(49,5a,b1,de), r(42,57,b8,d0), r(5f,40,a3,c2), r(54,4d,aa,cc),\
    r(f7,da,ec,41), r(fc,d7,e5,4f), r(e1,c0,fe,5d), r(ea,cd,f7,53),\
    r(db,ee,c8,79), r(d0,e3,c1,77), r(cd,f4,da,65), r(c6,f9,d3,6b),\
    r(af,b2,a4,31), r(a4,bf,ad,3f), r(b9,a8,b6,2d), r(b2,a5,bf,23),\
    r(83,86,80,09), r(88,8b,89,07), r(95,9c,92,15), r(9e,91,9b,1b),\
    r(47,0a,7c,a1), r(4c,07,75,af), r(51,10,6e,bd), r(5a,1d,67,b3),\
    r(6b,3e,58,99), r(60,33,51,97), r(7d,24,4a,85), r(76,29,43,8b),\
    r(1f,62,34,d1), r(14,6f,3d,df), r(09,78,26,cd), r(02,75,2f,c3),\
    r(33,56,10,e9), r(38,5b,19,e7), r(25,4c,02,f5), r(2e,41,0b,fb),\
    r(8c,61,d7,9a), r(87,6c,de,94), r(9a,7b,c5,86), r(91,76,cc,88),\
    r(a0,55,f3,a2), r(ab,58,fa,ac), r(b6,4f,e1,be), r(bd,42,e8,b0),\
    r(d4,09,9f,ea), r(df,04,96,e4), r(c2,13,8d,f6), r(c9,1e,84,f8),\
    r(f8,3d,bb,d2), r(f3,30,b2,dc), r(ee,27,a9,ce), r(e5,2a,a0,c0),\
    r(3c,b1,47,7a), r(37,bc,4e,74), r(2a,ab,55,66), r(21,a6,5c,68),\
    r(10,85,63,42), r(1b,88,6a,4c), r(06,9f,71,5e), r(0d,92,78,50),\
    r(64,d9,0f,0a), r(6f,d4,06,04), r(72,c3,1d,16), r(79,ce,14,18),\
    r(48,ed,2b,32), r(43,e0,22,3c), r(5e,f7,39,2e), r(55,fa,30,20),\
    r(01,b7,9a,ec), r(0a,ba,93,e2), r(17,ad,88,f0), r(1c,a0,81,fe),\
    r(2d,83,be,d4), r(26,8e,b7,da), r(3b,99,ac,c8), r(30,94,a5,c6),\
    r(59,df,d2,9c), r(52,d2,db,92), r(4f,c5,c0,80), r(44,c8,c9,8e),\
    r(75,eb,f6,a4), r(7e,e6,ff,aa), r(63,f1,e4,b8), r(68,fc,ed,b6),\
    r(b1,67,0a,0c), r(ba,6a,03,02), r(a7,7d,18,10), r(ac,70,11,1e),\
    r(9d,53,2e,34), r(96,5e,27,3a), r(8b,49,3c,28), r(80,44,35,26),\
    r(e9,0f,42,7c), r(e2,02,4b,72), r(ff,15,50,60), r(f4,18,59,6e),\
    r(c5,3b,66,44), r(ce,36,6f,4a), r(d3,21,74,58), r(d8,2c,7d,56),\
    r(7a,0c,a1,37), r(71,01,a8,39), r(6c,16,b3,2b), r(67,1b,ba,25),\
    r(56,38,85,0f), r(5d,35,8c,01), r(40,22,97,13), r(4b,2f,9e,1d),\
    r(22,64,e9,47), r(29,69,e0,49), r(34,7e,fb,5b), r(3f,73,f2,55),\
    r(0e,50,cd,7f), r(05,5d,c4,71), r(18,4a,df,63), r(13,47,d6,6d),\
    r(ca,dc,31,d7), r(c1,d1,38,d9), r(dc,c6,23,cb), r(d7,cb,2a,c5),\
    r(e6,e8,15,ef), r(ed,e5,1c,e1), r(f0,f2,07,f3), r(fb,ff,0e,fd),\
    r(92,b4,79,a7), r(99,b9,70,a9), r(84,ae,6b,bb), r(8f,a3,62,b5),\
    r(be,80,5d,9f), r(b5,8d,54,91), r(a8,9a,4f,83), r(a3,97,46,8d)

#undef r
#define r   r0

static const u_int32_t im_tab[4][256] =
{   {   m_table },
#undef  r
#define r   r1
    {   m_table },
#undef  r
#define r   r2
    {   m_table },
#undef  r
#define r   r3
    {   m_table }
};

#define four_tables(x,tab,vf,rf,c) \
 (  tab[0][bval(vf(x,0,c),rf(0,c))] \
  ^ tab[1][bval(vf(x,1,c),rf(1,c))] \
  ^ tab[2][bval(vf(x,2,c),rf(2,c))] \
  ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((r-c)&3)

#define ls_box(x,c)     four_tables(x,fl_tab,vf1,rf2,c)
#define inv_mcol(x)     four_tables(x,im_tab,vf1,rf1,0)

#define nc   (AES_BLOCK_SIZE/4)

// Initialise the key schedule from the user supplied key. The key
// length is now specified in bytes - 16, 24 or 32 as appropriate.
// This corresponds to bit lengths of 128, 192 and 256 bits, and
// to Nk values of 4, 6 and 8 respectively.

#define mx(t,f) (*t++ = inv_mcol(*f),f++)
#define cp(t,f) *t++ = *f++

#if   AES_BLOCK_SIZE == 16
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s)
#elif AES_BLOCK_SIZE == 24
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s); \
                    cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s); \
                    mx(d,s); mx(d,s)
#elif AES_BLOCK_SIZE == 32
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s); \
                    cp(d,s); cp(d,s); cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s); \
                    mx(d,s); mx(d,s); mx(d,s); mx(d,s)
#else
#error bad AES_BLOCK_SIZE
#endif

// y = output word, x = input word, r = row, c = column
// for r = 0, 1, 2 and 3 = column accessed for row r

#if defined(ARRAYS)
#define s(x,c) x[c]
#else
#define s(x,c) x##c
#endif

// I am grateful to Frank Yellin for the following constructions
// which, given the column (c) of the output state variable that
// is being computed, return the input state variables which are
// needed for each row (r) of the state

// For the fixed block size options, compilers reduce these two
// expressions to fixed variable references. For variable block
// size code conditional clauses will sometimes be returned

#define unused  77  // Sunset Strip

#define fwd_var(x,r,c) \
 ( r==0 ?			\
    ( c==0 ? s(x,0) \
    : c==1 ? s(x,1) \
    : c==2 ? s(x,2) \
    : c==3 ? s(x,3) \
    : c==4 ? s(x,4) \
    : c==5 ? s(x,5) \
    : c==6 ? s(x,6) \
    : s(x,7))		\
 : r==1 ?			\
    ( c==0 ? s(x,1) \
    : c==1 ? s(x,2) \
    : c==2 ? s(x,3) \
    : c==3 ? nc==4 ? s(x,0) : s(x,4) \
    : c==4 ? s(x,5) \
    : c==5 ? nc==8 ? s(x,6) : s(x,0) \
    : c==6 ? s(x,7) \
    : s(x,0))		\
 : r==2 ?			\
    ( c==0 ? nc==8 ? s(x,3) : s(x,2) \
    : c==1 ? nc==8 ? s(x,4) : s(x,3) \
    : c==2 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4) \
    : c==3 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5) \
    : c==4 ? nc==8 ? s(x,7) : s(x,0) \
    : c==5 ? nc==8 ? s(x,0) : s(x,1) \
    : c==6 ? s(x,1) \
    : s(x,2))		\
 :					\
    ( c==0 ? nc==8 ? s(x,4) : s(x,3) \
    : c==1 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4) \
    : c==2 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5) \
    : c==3 ? nc==4 ? s(x,2) : nc==8 ? s(x,7) : s(x,0) \
    : c==4 ? nc==8 ? s(x,0) : s(x,1) \
    : c==5 ? nc==8 ? s(x,1) : s(x,2) \
    : c==6 ? s(x,2) \
    : s(x,3)))

#define inv_var(x,r,c) \
 ( r==0 ?			\
    ( c==0 ? s(x,0) \
    : c==1 ? s(x,1) \
    : c==2 ? s(x,2) \
    : c==3 ? s(x,3) \
    : c==4 ? s(x,4) \
    : c==5 ? s(x,5) \
    : c==6 ? s(x,6) \
    : s(x,7))		\
 : r==1 ?			\
    ( c==0 ? nc==4 ? s(x,3) : nc==8 ? s(x,7) : s(x,5) \
    : c==1 ? s(x,0) \
    : c==2 ? s(x,1) \
    : c==3 ? s(x,2) \
    : c==4 ? s(x,3) \
    : c==5 ? s(x,4) \
    : c==6 ? s(x,5) \
    : s(x,6))		\
 : r==2 ?			\
    ( c==0 ? nc==4 ? s(x,2) : nc==8 ? s(x,5) : s(x,4) \
    : c==1 ? nc==4 ? s(x,3) : nc==8 ? s(x,6) : s(x,5) \
    : c==2 ? nc==8 ? s(x,7) : s(x,0) \
    : c==3 ? nc==8 ? s(x,0) : s(x,1) \
    : c==4 ? nc==8 ? s(x,1) : s(x,2) \
    : c==5 ? nc==8 ? s(x,2) : s(x,3) \
    : c==6 ? s(x,3) \
    : s(x,4))		\
 :					\
    ( c==0 ? nc==4 ? s(x,1) : nc==8 ? s(x,4) : s(x,3) \
    : c==1 ? nc==4 ? s(x,2) : nc==8 ? s(x,5) : s(x,4) \
    : c==2 ? nc==4 ? s(x,3) : nc==8 ? s(x,6) : s(x,5) \
    : c==3 ? nc==8 ? s(x,7) : s(x,0) \
    : c==4 ? nc==8 ? s(x,0) : s(x,1) \
    : c==5 ? nc==8 ? s(x,1) : s(x,2) \
    : c==6 ? s(x,2) \
    : s(x,3)))

#define si(y,x,k,c) s(y,c) = const_word_in(x + 4 * c) ^ k[c]
#define so(y,x,c)   word_out(y + 4 * c, s(x,c))

#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,ft_tab,fwd_var,rf1,c)
#define inv_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,it_tab,inv_var,rf1,c)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,fl_tab,fwd_var,rf1,c)
#define inv_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,il_tab,inv_var,rf1,c)

#if AES_BLOCK_SIZE == 16

#if defined(ARRAYS)
#define locals(y,x)     x[4],y[4]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,y##0,y##1,y##2,y##3
// the following defines prevent the compiler requiring the declaration
// of generated but unused variables in the fwd_var and inv_var macros
#define b04 unused
#define b05 unused
#define b06 unused
#define b07 unused
#define b14 unused
#define b15 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

#elif AES_BLOCK_SIZE == 24

#if defined(ARRAYS)
#define locals(y,x)     x[6],y[6]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5, \
                        y##0,y##1,y##2,y##3,y##4,y##5
#define b06 unused
#define b07 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3); \
                        s(y,4) = s(x,4); s(y,5) = s(x,5);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); \
                        si(y,x,k,3); si(y,x,k,4); si(y,x,k,5)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); \
                        so(y,x,3); so(y,x,4); so(y,x,5)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); \
                        rm(y,x,k,3); rm(y,x,k,4); rm(y,x,k,5)
#else

#if defined(ARRAYS)
#define locals(y,x)     x[8],y[8]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5,x##6,x##7, \
                        y##0,y##1,y##2,y##3,y##4,y##5,y##6,y##7
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3); \
                        s(y,4) = s(x,4); s(y,5) = s(x,5); \
                        s(y,6) = s(x,6); s(y,7) = s(x,7);

#if AES_BLOCK_SIZE == 32

#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3); \
                        si(y,x,k,4); si(y,x,k,5); si(y,x,k,6); si(y,x,k,7)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3); \
                        so(y,x,4); so(y,x,5); so(y,x,6); so(y,x,7)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3); \
                        rm(y,x,k,4); rm(y,x,k,5); rm(y,x,k,6); rm(y,x,k,7)
#else
#error invalid AES_BLOCK_SIZE
#endif

#endif

/**
 * Encrypt a single block of data.
 */
static void encrypt_block(const private_aes_crypter_t *this,
						  const unsigned char in_blk[], unsigned char out_blk[])
{
	u_int32_t locals(b0, b1);
	const u_int32_t *kp = this->aes_e_key;

	state_in(b0, in_blk, kp); kp += nc;

	switch(this->aes_Nrnd)
	{
		case 14:
			round(fwd_rnd,  b1, b0, kp         );
			round(fwd_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
			/* fall */
		case 12:
			round(fwd_rnd,  b1, b0, kp         );
			round(fwd_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
			/* fall */
		case 10:
			round(fwd_rnd,  b1, b0, kp         );
			round(fwd_rnd,  b0, b1, kp +     nc);
			round(fwd_rnd,  b1, b0, kp + 2 * nc);
			round(fwd_rnd,  b0, b1, kp + 3 * nc);
			round(fwd_rnd,  b1, b0, kp + 4 * nc);
			round(fwd_rnd,  b0, b1, kp + 5 * nc);
			round(fwd_rnd,  b1, b0, kp + 6 * nc);
			round(fwd_rnd,  b0, b1, kp + 7 * nc);
			round(fwd_rnd,  b1, b0, kp + 8 * nc);
			round(fwd_lrnd, b0, b1, kp + 9 * nc);
	}

	state_out(out_blk, b0);
}

/**
 * Decrypt a single block of data.
 */
static void decrypt_block(const private_aes_crypter_t *this,
						  const unsigned char in_blk[], unsigned char out_blk[])
{
	u_int32_t locals(b0, b1);
	const u_int32_t *kp = this->aes_d_key;

	state_in(b0, in_blk, kp); kp += nc;

	switch(this->aes_Nrnd)
	{
		case 14:
			round(inv_rnd,  b1, b0, kp         );
			round(inv_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
			/* fall */
		case 12:
			round(inv_rnd,  b1, b0, kp         );
			round(inv_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
			/* fall */
		case 10:
			round(inv_rnd,  b1, b0, kp         );
			round(inv_rnd,  b0, b1, kp +     nc);
			round(inv_rnd,  b1, b0, kp + 2 * nc);
			round(inv_rnd,  b0, b1, kp + 3 * nc);
			round(inv_rnd,  b1, b0, kp + 4 * nc);
			round(inv_rnd,  b0, b1, kp + 5 * nc);
			round(inv_rnd,  b1, b0, kp + 6 * nc);
			round(inv_rnd,  b0, b1, kp + 7 * nc);
			round(inv_rnd,  b1, b0, kp + 8 * nc);
			round(inv_lrnd, b0, b1, kp + 9 * nc);
	}

	state_out(out_blk, b0);
}

METHOD(crypter_t, decrypt, bool,
	private_aes_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *decrypted)
{
	int pos;
	const u_int32_t *iv_i;
	u_int8_t *in, *out;

	if (decrypted)
	{
		*decrypted = chunk_alloc(data.len);
		out = decrypted->ptr;
	}
	else
	{
		out = data.ptr;
	}
	in = data.ptr;

	pos = data.len-16;
	in += pos;
	out += pos;
	while (pos >= 0)
	{
		decrypt_block(this, in, out);
		if (pos==0)
		{
			iv_i=(const u_int32_t*) (iv.ptr);
		}
		else
		{
			iv_i=(const u_int32_t*) (in-16);
		}
		*((u_int32_t *)(&out[ 0])) ^= iv_i[0];
		*((u_int32_t *)(&out[ 4])) ^= iv_i[1];
		*((u_int32_t *)(&out[ 8])) ^= iv_i[2];
		*((u_int32_t *)(&out[12])) ^= iv_i[3];
		in-=16;
		out-=16;
		pos-=16;
	}
	return TRUE;
}

METHOD(crypter_t, encrypt, bool,
	private_aes_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *encrypted)
{
	int pos;
	const u_int32_t *iv_i;
	u_int8_t *in, *out;

	in = data.ptr;
	out = data.ptr;
	if (encrypted)
	{
		*encrypted = chunk_alloc(data.len);
		out = encrypted->ptr;
	}

	pos=0;
	while(pos<data.len)
	{
		if (pos==0)
		{
			iv_i=(const u_int32_t*) iv.ptr;
		}
		else
		{
			iv_i=(const u_int32_t*) (out-16);
		}
		*((u_int32_t *)(&out[ 0])) = iv_i[0]^*((const u_int32_t *)(&in[ 0]));
		*((u_int32_t *)(&out[ 4])) = iv_i[1]^*((const u_int32_t *)(&in[ 4]));
		*((u_int32_t *)(&out[ 8])) = iv_i[2]^*((const u_int32_t *)(&in[ 8]));
		*((u_int32_t *)(&out[12])) = iv_i[3]^*((const u_int32_t *)(&in[12]));
		encrypt_block(this, out, out);
		in+=16;
		out+=16;
		pos+=16;
	}
	return TRUE;
}

METHOD(crypter_t, get_block_size, size_t,
	private_aes_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_aes_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_key_size, size_t,
	private_aes_crypter_t *this)
{
	return this->key_size;
}

METHOD(crypter_t, set_key, bool,
	private_aes_crypter_t *this, chunk_t key)
{
	u_int32_t    *kf, *kt, rci, f = 0;
	u_int8_t *in_key = key.ptr;

	this->aes_Nrnd = (this->aes_Nkey > (nc) ? this->aes_Nkey : (nc)) + 6;

	this->aes_e_key[0] = const_word_in(in_key     );
	this->aes_e_key[1] = const_word_in(in_key +  4);
	this->aes_e_key[2] = const_word_in(in_key +  8);
	this->aes_e_key[3] = const_word_in(in_key + 12);

	kf = this->aes_e_key;
	kt = kf + nc * (this->aes_Nrnd + 1) - this->aes_Nkey;
	rci = 0;

	switch(this->aes_Nkey)
	{
	case 4: do
			{   kf[4] = kf[0] ^ ls_box(kf[3],3) ^ rcon_tab[rci++];
				kf[5] = kf[1] ^ kf[4];
				kf[6] = kf[2] ^ kf[5];
				kf[7] = kf[3] ^ kf[6];
				kf += 4;
			}
			while(kf < kt);
			break;

	case 6: this->aes_e_key[4] = const_word_in(in_key + 16);
			this->aes_e_key[5] = const_word_in(in_key + 20);
			do
			{   kf[ 6] = kf[0] ^ ls_box(kf[5],3) ^ rcon_tab[rci++];
				kf[ 7] = kf[1] ^ kf[ 6];
				kf[ 8] = kf[2] ^ kf[ 7];
				kf[ 9] = kf[3] ^ kf[ 8];
				kf[10] = kf[4] ^ kf[ 9];
				kf[11] = kf[5] ^ kf[10];
				kf += 6;
			}
			while(kf < kt);
			break;

	case 8: this->aes_e_key[4] = const_word_in(in_key + 16);
			this->aes_e_key[5] = const_word_in(in_key + 20);
			this->aes_e_key[6] = const_word_in(in_key + 24);
			this->aes_e_key[7] = const_word_in(in_key + 28);
			do
			{   kf[ 8] = kf[0] ^ ls_box(kf[7],3) ^ rcon_tab[rci++];
				kf[ 9] = kf[1] ^ kf[ 8];
				kf[10] = kf[2] ^ kf[ 9];
				kf[11] = kf[3] ^ kf[10];
				kf[12] = kf[4] ^ ls_box(kf[11],0);
				kf[13] = kf[5] ^ kf[12];
				kf[14] = kf[6] ^ kf[13];
				kf[15] = kf[7] ^ kf[14];
				kf += 8;
			}
			while (kf < kt);
			break;
	}

	if(!f)
	{
		u_int32_t i;

		kt = this->aes_d_key + nc * this->aes_Nrnd;
		kf = this->aes_e_key;

		cpy(kt, kf); kt -= 2 * nc;

		for(i = 1; i < this->aes_Nrnd; ++i)
		{
			mix(kt, kf);
			kt -= 2 * nc;
		}
		cpy(kt, kf);
	}
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_aes_crypter_t *this)
{
	memwipe(this, sizeof(*this));
	free(this);
}

/*
 * Described in header
 */
aes_crypter_t *aes_crypter_create(encryption_algorithm_t algo, size_t key_size)
{
	private_aes_crypter_t *this;

	if (algo != ENCR_AES_CBC)
	{
		return NULL;
	}
	switch (key_size)
	{
		case 0:
			key_size = 16;
			break;
		case 32:
		case 24:
		case 16:
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.key_size = key_size,
		.aes_Nkey = key_size / 4,
	);

	return &this->public;
}
