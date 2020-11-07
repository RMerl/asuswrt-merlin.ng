/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
//**************************************************************************
// File Name  : spumd5.c
//
// Description: IPSec SPU MD5 Authentication routines.
//               
//**************************************************************************
#include <linux/module.h>
#include "bcmipsec.h"
#include "bcmsad.h"
#include "spuipsec.h"
#include "spumd5.h"
/* 
 * ubsmd5.c: MD5 key manipulation functions.
 */

extern unsigned int rol(unsigned int x, int n);
static void MD5Update(struct MD5Context *ctx, unsigned char *HashBlock,
              int len);

/*
 * We use a SHA-style formulation of MD5 algorithm -- this is simpler and much
 * faster in HW, although it is slower in SW. The SHA-style algorithm avoids
 * a set of muxes in front of each arithmetic unit, and also
 * simplifies the HW control logic a lot.
 *
 * This code is exactly equivalent to the more verbose 64-round formulation
 * with FF's, GG's, HH's and II's shown in Schneier's "Applied Cryptography."
 * This code will only work on a little endian machine. Constants and boolean 
 * functions are out of Schneier's "Applied Cryptography". Also, MD5Update() 
 * must only be called once, followed by a call to MD5Final() -- the length 
 * field is not being properly carried over within MD5Update().
 * This implementation does not handle large block sizes per the MD5 RFC1321.
 * It is meant for packets of less than 2**28 bytes each.
 *
*/

/* Boolean functions for internal rounds */
#define F(x,y,z) ( ((x) & (y)) | ((~(x)) & (z)) )
#define G(x,y,z) ( ((x) & (z)) | ((y) & (~(z))) )
#define H(x,y,z) ( (x) ^ (y) ^ (z) )
#define I(x,y,z) ( (y) ^ ((x) | (~(z))) )

static unsigned int MD5_T[] = {    /* The 64 magic constants */
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static int MD5_S[] = {        /* Shift amounts for each round */
    /* Rounds  0-15 */ 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12,
        17, 22,
    /* Rounds 16-31 */ 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14,
        20,
    /* Rounds 32-47 */ 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11,
        16, 23,
    /* Rounds 48-63 */ 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10,
        15, 21
};

static int MD5_P[] = {        /* Permutation index to access message blocks */
    /* Rounds  0-15 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    /* Rounds 16-31 */ 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
    /* Rounds 32-47 */ 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
    /* Rounds 48-63 */ 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};

static unsigned char ipad[64] =
    { 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
};

static unsigned char opad[64] =
    { 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
    0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c
};

static void
bytnxor(unsigned char *dest, unsigned char *src1, unsigned char *src2,
    unsigned char len)
{
    while (len-- > 0)
        *dest++ = *src1++ ^ *src2++;
}

void
InitMD5State(ubsec_HMAC_State_pt HMAC_State, unsigned char *HashKey, int donxor)
{
    struct MD5Context ctx;
    unsigned char pad[64];

    /* First prepare the inner block. */
    if (donxor)
        bytnxor((unsigned char *) pad, (unsigned char *) HashKey, ipad,
            64);
    else {
        memcpy(pad, HashKey, 16);
        memcpy(&pad[16], ipad, 48);
    }
    memset(&ctx, 0, sizeof (struct MD5Context));

    /* The initial values in memory are 01 23 45 ... per RFC1321 */
    ctx.buf[0] = 0x67452301;
    ctx.buf[1] = 0xefcdab89;
    ctx.buf[2] = 0x98badcfe;
    ctx.buf[3] = 0x10325476;

    MD5Update(&ctx, pad, 64);

    /* ctx comes out as an array of long ints. The byte order of ctx
       is dependent on the CPU endianess. The byte order of the memory destination 
       is dependent on the CryptoNet memory endianess. Based on our MD5 algorithm's
       CPU endianess assumptions, the net result is that we do a straight copy if
       the CPU and CryptoNet memory are of the same endianess. If the CPU and 
       CryptoNet memory are of opposite endianess, we'll do 32-bit byteswapping
       during the copy, taken care of by the copywords() routine. */

    memcpy(&HMAC_State->InnerState[0], &ctx.buf[0], MD5_HASH_LENGTH);

    /* Now prepare the Outer block. */
    if (donxor)
        bytnxor((unsigned char *) pad, (unsigned char *) HashKey, opad,
            64);
    else {
        memcpy(pad, HashKey, 16);
        memcpy(&pad[16], opad, 48);
    }
    memset(&ctx, 0, sizeof (struct MD5Context));

    /* The initial values in memory are 01 23 45 ... per RFC1321 */
    ctx.buf[0] = 0x67452301;
    ctx.buf[1] = 0xefcdab89;
    ctx.buf[2] = 0x98badcfe;
    ctx.buf[3] = 0x10325476;

    MD5Update(&ctx, pad, 64);

    /* ctx comes out as an array of long ints. The byte order of ctx
       is dependent on the CPU endianess. The byte order of the memory destination 
       is dependent on the CryptoNet memory endianess. Based on our MD5 algorithm's
       CPU endianess assumptions, the net result is that we do a straight copy if
       the CPU and CryptoNet memory are of the same endianess. If the CPU and 
       CryptoNet memory are of opposite endianess, we'll do 32-bit byteswapping
       during the copy, taken care of by the copywords() routine. */

    memcpy(&HMAC_State->OuterState[0], &ctx.buf[0], MD5_HASH_LENGTH);

}

static void
MD5Update(struct MD5Context *ctx, unsigned char *HashBlock, int len)
{
    /* ctx was built (CPU endian-independent) as unsigned longs       */
    /* HashBlock was built (CPU endian-independent) as unsigned chars */
    unsigned int a, b, c, d;
    int block, i;
    /* Process as many blocks as possible */
    for (block = 0; len >= 64; len -= 64, ++block) {
        unsigned int m[16], A, B, C, D;

        memcpy(m, HashBlock + 64 * block, 64);    /* Get one block of data */

        /* At this point m[] is built as a char array. However, m[] will
           be operated on from here on out as an array of unsigned longs.
           This algorithm's definition assumes a little endian CPU, so 
           we'll endian-adjust the byte array if we have a big endian CPU */
        for (i = 0; i < 16; ++i)
            m[i] = BYTESWAPLONG(m[i]);    /* View data as big endian */

        a = ctx->buf[0];
        b = ctx->buf[1];
        c = ctx->buf[2];
        d = ctx->buf[3];
        /* Now for the hyper-compact SHA-style formulation of MD5 */
        A = b;
        B = c;
        C = d;
        D = a;
        /*printk(KERN_ERR "pre round 0: 0x%x 0x%x 0x%x 0x%x\n", A, B, C, D);*/
        for (i = 0; i < 64; ++i) {
            unsigned int temp, fn = 0, rolt, prerolt;

            switch (i / 16) {
            case 0:
                fn = F(A, B, C);
                break;
            case 1:
                fn = G(A, B, C);
                break;
            case 2:
                fn = H(A, B, C);
                break;
            case 3:
                fn = I(A, B, C);
                break;
            default:;
            }
            prerolt = D + fn + m[MD5_P[i]] + MD5_T[i];
            rolt = rol(D + fn + m[MD5_P[i]] + MD5_T[i], MD5_S[i]);
            temp =
                A + rol(D + fn + m[MD5_P[i]] + MD5_T[i], MD5_S[i]);
            D = C;
            C = B;
            B = A;
            A = temp;
            /*printk(KERN_ERR "post round %d: 0x%x 0x%x 0x%x 0x%x\n",
                               i, A, B, C, D);
            printk(KERN_ERR "fn=0x%x, before rol=0x%x, rol()=0x%x\n",
                   fn, prerolt, rolt);
            printk(KERN_ERR "\tcte=0x%x, data=0x%x, shift amt=%d\n\n",
                   MD5_T[i], m[MD5_P[i]], MD5_S[i]);*/
        }
        b = A;
        c = B;
        d = C;
        a = D;

        ctx->buf[0] += a;
        ctx->buf[1] += b;
        ctx->buf[2] += c;
        ctx->buf[3] += d;
    }
}

MODULE_LICENSE("GPL");
