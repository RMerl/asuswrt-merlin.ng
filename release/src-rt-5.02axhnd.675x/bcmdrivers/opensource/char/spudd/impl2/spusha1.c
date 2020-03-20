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
// Description: IPSec SPU SHA Authentication routines.
//               
//**************************************************************************
#include <linux/module.h>
#include "bcmipsec.h"
#include "bcmsad.h"
#include "spusha1.h"
#include "spuipsec.h"

/* 
 * ubssha1.c: SHA1 key manipulation functions.
 */

static void SHAUpdate(SHA_CTX * ctx, unsigned char *HashBlock, int len);

/* Reasonably fast rotate left of x by 'n' bits */
unsigned int
rol(unsigned int x, int n)
{
        unsigned long result = (x << n) | (x >> (32 - n));
        return (result);
}

static void
bytnxor(unsigned char *dest, unsigned char *src1, unsigned char *src2,
    unsigned char len)
{
    while (len-- > 0)
        *dest++ = *src1++ ^ *src2++;
}

/*
 * This code will only work on a little endian machine. Constants and boolean functions
 * are out of Schneier's "Applied Cryptography". Also, SHAUpdate() must only be called
 * once, followed by a call to SHAFinal() -- the length field is not being properly
 * carried over within SHAUpdate().
 *
 * This implementation does not handle large block sizes per the spec.
 * It is meant for packets of less than 2**28 bytes each.
*/

/* Boolean functions for internal rounds */
#define F(x,y,z) ( ((x) & (y)) | ((~(x)) & (z)) )
#define G(x,y,z) ( (x) ^ (y) ^ (z) )
#define H(x,y,z) ( ((x)&(y)) | ((x)&(z)) | ((y)&(z)) )

/* The 4 magic constants */
static unsigned int SHA1_K[] = {
    0x5a827999, 0x6ed9eba1,
    0x8f1bbcdc, 0xca62c1d6
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

void
InitSHA1State(ubsec_HMAC_State_pt HMAC_State, unsigned char *HashKey)
{
    SHA_CTX ctx;
    unsigned char pad[64];

    /* First prepare the inner block. */
    bytnxor((unsigned char *) pad, (unsigned char *) HashKey, ipad, 64);

    /* Init the context, the initial values in memory are 01 23 45 ... */
    memset(&ctx, 0, sizeof (SHA_CTX));
#if 1 // Pavan little endian
    ctx.buffer[0] = 0x67452301;
    ctx.buffer[1] = 0xefcdab89;
    ctx.buffer[2] = 0x98badcfe;
    ctx.buffer[3] = 0x10325476;
    ctx.buffer[4] = 0xc3d2e1f0;
    SHAUpdate(&ctx, pad, 64);
#endif // Pavan
#if 0 // Pavan big endian
    ctx.buffer[0] = 0x01234567;
    ctx.buffer[1] = 0x89abcdef;
    ctx.buffer[2] = 0xfedcba98;
    ctx.buffer[3] = 0x76543210;
    ctx.buffer[4] = 0xf0e1d2c3;
    SHAUpdate(&ctx, pad, 64);
#endif // Pavan

    /* ctx comes out as an array of long ints. The byte order of ctx
       is dependent on the CPU endianess. The byte order of the memory destination 
       is dependent on the CryptoNet memory endianess. Based on our SHA1 algorithm's
       CPU endianess assumptions, the net result is that we do a straight copy if
       the CPU and CryptoNet memory are of the same endianess. If the CPU and 
       CryptoNet memory are of opposite endianess, we'll do 32-bit byteswapping
       during the copy, taken care of by the copywords() routine. */

    memcpy(&HMAC_State->InnerState[0], &ctx.buffer[0], SHA_HASH_LENGTH);

    /* Do do the same for the outer block */
    bytnxor((unsigned char *) pad, (unsigned char *) HashKey, opad, 64);
    memset(&ctx, 0, sizeof (SHA_CTX));
#if 1 // Pavan little endian
    ctx.buffer[0] = 0x67452301;
    ctx.buffer[1] = 0xefcdab89;
    ctx.buffer[2] = 0x98badcfe;
    ctx.buffer[3] = 0x10325476;
    ctx.buffer[4] = 0xc3d2e1f0;

    SHAUpdate(&ctx, pad, 64);
#endif // Pavan
#if 0 // Pavan big endian
    ctx.buffer[0] = 0x01234567;
    ctx.buffer[1] = 0x89abcdef;
    ctx.buffer[2] = 0xfedcba98;
    ctx.buffer[3] = 0x76543210;
    ctx.buffer[4] = 0xf0e1d2c3;

    SHAUpdate(&ctx, pad, 64);
#endif // Pavan

    /* ctx comes out as an array of long ints. The byte order of ctx
       is dependent on the CPU endianess. The byte order of the memory destination 
       is dependent on the CryptoNet memory endianess. Based on our SHA1 algorithm's
       CPU endianess assumptions, the net result is that we do a straight copy if
       the CPU and CryptoNet memory are of the same endianess. If the CPU and 
       CryptoNet memory are of opposite endianess, we'll do 32-bit byteswapping
       during the copy, taken care of by the copywords() routine. */

    memcpy(&HMAC_State->OuterState[0], &ctx.buffer[0], SHA_HASH_LENGTH);

}

static void
SHAUpdate(SHA_CTX * ctx, unsigned char *HashBlock, int len)
{
    unsigned int a, b, c, d, e;
    int block, i;

    /* Process as many blocks as possible */
    for (block = 0; len >= 64; len -= 64, ++block) {
        unsigned int m[16];

        /* A 16-byte buffer is sufficient -- we build needed words beyond the first 16 on the fly */
        memcpy(m, HashBlock + 64 * block, 64);    /* Get one block of data */

        /* At this point m[] is built as a char array. However, m[] will
           be operated on from here on out as an array of unsigned longs.
           This algorithm assumes m[] is arranged in big-endian byte order, so 
           we'll endian-adjust the byte array if we have a little endian CPU */

        a = ctx->buffer[0];
        b = ctx->buffer[1];
        c = ctx->buffer[2];
        d = ctx->buffer[3];
        e = ctx->buffer[4];

#ifdef SPU_DEBUG
        printk(KERN_ERR "pre round 0: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                       a, b, c, d, e);
#endif

        /* Four sets of 20 rounds each */
        for (i = 0; i < 80; ++i) {
            unsigned long fn = 0, temp, K = 0;
            switch (i / 20) {
            case 0:
                K = SHA1_K[0];
                fn = F(b, c, d);
                break;
            case 1:
                K = SHA1_K[1];
                fn = G(b, c, d);
                break;
            case 2:
                K = SHA1_K[2];
                fn = H(b, c, d);
                break;
            case 3:
                K = SHA1_K[3];
                fn = G(b, c, d);
                break;
            default:;
            }

            /* Build needed words beyond original 16 on the fly */
            if (i >= 16)
                m[i % 16] =
                    rol(m[(i - 3) % 16] ^ m[(i - 8) % 16] ^
                    m[(i - 14) % 16] ^ m[(i - 16) % 16], 1);
            temp = rol(a, 5) + fn + e + m[i % 16] + K;
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;

#ifdef SPU_DEBUG
            printk(KERN_ERR "post round %d: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                               i, a, b, c, d, e);
#endif
        }

        ctx->buffer[0] += a;
        ctx->buffer[1] += b;
        ctx->buffer[2] += c;
        ctx->buffer[3] += d;
        ctx->buffer[4] += e;
    }

}

MODULE_LICENSE("GPL");
