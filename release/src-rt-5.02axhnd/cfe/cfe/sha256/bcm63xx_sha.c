/**********************************************************************
 *  
 *  bcm63xx_sha.c       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *  
 * <:label-BRCM:2012:proprietary:standard
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 ********************************************************************* 
 */
#include "bcm_secPrivate.h"
#include "bcm_sec.h"
#include "lib_string.h"

#define assert(x)


/** Rotate a 32-bit value to the right
 * 
 * @param x     the value to be rotated
 * @param n     the number of bits to rotate
 * @return      the result
 */
static uint32_t ROTR32(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (32 - n));
}


/** Process one message block of data updating the SHA-256 digest
 * 
 * Given the SHA-256 message digest computed so far, process a block containing
 * the next 512 bits of the message. These 512 bits are presented as 16 32-bit 
 * words. These 16 words are extended to an array of 64 and then each of these
 * 64 words is processed in turn, updating the digest each time. On exit, the
 * digest provided by the caller will have been updated to include the given
 * block.
 * 
 * @param digest    the 256-bit digest as 8 32-bit values
 * @param block     the next 512-bit block to add, as 16 32-bit values
 */
static void SHA256Transform(uint32_t digest[8], uint32_t const block[16])
{
    // SHA-256 uses a sequence of sixty-four constant 32-bit words. These words 
    // represent the first thirty-two bits of the fractional parts of the cube 
    // roots of the first sixty-four prime numbers. See section 4.2.2 of "Secure 
    // Hash Signature Standard (SHS) (FIPS PUB 180-2)".
    static uint32_t const k[64] =
    {
       0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 
       0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
       0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 
       0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
       0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 
       0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
       0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 
       0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
       0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 
       0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
       0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 
       0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
       0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 
       0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
       0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 
       0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    };

    // Extend the sixteen 32-bit words in block into sixty-four 32-bit words in 
    // w:
    uint32_t w[64];
    {
        int i;
        for (i = 0; i < 16; ++i) w[i] = block[i];
    }
    
    {

        int i;
        for (i = 16; i < 64; ++i)
        {
            uint32_t const s0 = ROTR32(w[i-15], 7) ^ ROTR32(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t const s1 = ROTR32(w[i-2], 17) ^ ROTR32(w[i-2], 19)  ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
    }
    
    // Copy the initial state of the digest into the eight working variables
    uint32_t a = digest[0];
    uint32_t b = digest[1];
    uint32_t c = digest[2];
    uint32_t d = digest[3];
    uint32_t e = digest[4];
    uint32_t f = digest[5];
    uint32_t g = digest[6];
    uint32_t h = digest[7];

    // Process each word in the extended 64-word block
    {
        int i;
        for(i = 0; i < 64; i++ )
        {
            uint32_t const s0  = ROTR32(a, 2) ^ ROTR32(a, 13) ^ ROTR32(a, 22);
            uint32_t const maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t const t2  = s0 + maj;

            uint32_t const s1  = ROTR32(e, 6) ^ ROTR32(e, 11) ^ ROTR32(e, 25);
            uint32_t const ch  = (e & f) ^ ((~ e) & g);
            uint32_t const t1  = h + s1 + ch + k[i] + w[i];

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
    }

    // Copy out the updated state of the digest
    digest[0] += a;
    digest[1] += b;
    digest[2] += c;
    digest[3] += d;
    digest[4] += e;
    digest[5] += f;
    digest[6] += g;
    digest[7] += h;            
}


/** Initialise a SHA-256 digest before use
 * 
 * Prepare a SHA-256 digest for use, setting it to the initial hash value H(0).
 * This function must be called before the digest can be passed to the
 * Sec_sha256Update and Sec_sha256Final functions. (It is not needed before a
 * call to Sec_sha256, however).
 * 
 * @param digest    the digest to be initialised
 */
void Sec_sha256Init(uint32_t* digest)
{
    // For SHA-256, the initial hash value, H(0), shall consist of the following 
    // eight 32-bit words. See section 5.3.2 of "Secure Hash Signature Standard 
    // (SHS) (FIPS PUB 180-2)".
    digest[0] = 0x6a09e667;
    digest[1] = 0xbb67ae85;
    digest[2] = 0x3c6ef372;
    digest[3] = 0xa54ff53a;
    digest[4] = 0x510e527f;
    digest[5] = 0x9b05688c;
    digest[6] = 0x1f83d9ab;
    digest[7] = 0x5be0cd19;     
}


/** Update an existing SHA-256 digest using a new block of data
 * 
 * Given a SHA-256 digest which has already been initialised (and possibly 
 * already updated with data), hash into it as many 64-byte blocks of data as we 
 * can from the block of data provided. The function returns the number of bits
 * at the end of the block that were unused - this value will always be less 
 * that 512.
 *  
 * This restriction allows us to assume that on entry we are about to start a
 * new block of 512 bits which will speed up the hashing significantly.
 * 
 * This function can be called as many times as required to process the bulk of
 * a message. Sec_sha256Final must be called at the end to complete the 
 * operation, even if there are no bits left over.
 * 
 * @param digest        the digest to be updated
 * @param buffer        the data to be hashed into the digest
 * @param bufferLength  the length of the data in bits
 * @return              the number of bits left unused at the end
 */
int Sec_sha256Update(uint32_t* digest, uint8_t const* buffer, int bufferLength)
{
    // We will process the data in blocks of 512 bits (64 bytes) built here.
    uint32_t block[16];
    
    // We will need to keep hold of the original length of the data for the end
    int bitsRemaining = bufferLength;
    
    // Process complete blocks of 512 bits (64 bytes)
    while (bitsRemaining / 512 > 0) 
    {
        // Extract the next block of words from the data we were given
        {
            int i;
            for (i = 0; i < 16; i++)
            {
                // The SHA-256 algorithm is specified in terms of big-endian data and we
                // are a little-endian machine, so flip the byte order when the data is
                // passed in and when the digest is returned at the end.
                uint32_t const temp = *(uint32_t const*)buffer;
                block[i] = HTONL(temp);

                buffer += 4;
            }
        }

        // Process this block into the digest
        SHA256Transform(digest, block);
        
        // Account for the bits we've processed
        bitsRemaining -= 512;
    }
    
    // We no longer have a complete block of 512 bits to add - return the number
    // of unused bits to the caller
    return bitsRemaining;
}


/** Conclude the calculation of a SHA-256 digest
 * 
 * Given a digest that has been initialised using Sec_sha256Init and possibly
 * updated with calls to Sec_sha256Update, conclude the calculation by
 * 
 * a) Hashing all of the data passed to this function
 * b) Adding the final padding and buffer length into the hash
 * 
 * This function processes all of the data passed to it. There is no requirement
 * that it consist of a multiple of 512 bits.
 * 
 * Once this function has been called, the digest cannot be updated again but it
 * can be reused by calling Sec_sha256Init to start a new digest.
 * 
 * @param digest        the digest to be updated
 * @param buffer        the data to be hashed into the digest
 * @param bufferLength  the total length of data that has been hashed (in all
 *                      previous calls to Sec_sha256Init and this call) in bits 
 * @param bitsRemaining the length of data in buffer to be hashed in bits
 */
void Sec_sha256Final(uint32_t* digest, uint8_t const* buffer, int bufferLength, int bitsRemaining)
{
    // Deal with any complete blocks we are passed
    int const bitsUsed = bitsRemaining - Sec_sha256Update(digest, buffer, bitsRemaining);
    
    // Step over the data we consumed
    buffer += bitsUsed / 8;
    bitsRemaining -= bitsUsed;
    
    // When we are left with fewer than 512 bits, we need to deal with that data
    // plus the suffix to be added which is:
    // 
    // a) a single '1' bit
    // b) a sequence of '0' bits
    // c) the length in bits of the data we were given as a uint64_t
    //
    // where (b) is long enough to result in a block which is 512 bits long. See
    // section 5.1.1 of "Secure Hash Signature Standard (SHS) (FIPS PUB 180-2)".
    //
    // The minimum length of this suffix is 65 bits and so we have two cases:
    //
    // 1. If the number of bits of data remaining is 447 bits or less, we can add
    //    the suffix to form a single block.
    // 2. If the number of bits of data remaining is more than 447 bits, we must
    //    add (a) and enough of (b) to form a first 512 bit block and then form a
    //    second 512 bit block consisting of the rest of (b) and (c).
    //
    // Place the remaining data in place. First of all, copy entire 32-bit values
    // across. We know there are fewer than 512 bits and that we are handling
    // entire bytes so there are 512-8= 504 bits or 63 bytes or less.
    uint32_t block[16];
    
    assert(bitsRemaining <= 512-8);
    int nwords = bitsRemaining / 32;
    int const nbytes = (bitsRemaining / 8) % sizeof(uint32_t);
    
    {
        int i;
        for (i = 0; i < nwords; ++i)
        {
            uint32_t const temp = *(uint32_t const*)buffer;
            block[i] = HTONL(temp);
            buffer += sizeof(uint32_t);
        }
    }

    // Then deal with any runt, combining it with (a) and enough of (b) to form
    // a 32-bit value. Note that we are deliberately falling through each case 
    // of the switch
    uint32_t value = 0;
    assert(0 <= nbytes && nbytes <= 3);
    switch (nbytes)
    {
        case 3:  value |= buffer[2] <<  8;
        case 2:  value |= buffer[1] << 16;
        case 1:  value |= buffer[0] << 24;
    }    
    value |= 0x80 << (24 - nbytes*8);
    block[nwords++] = value;

    // Is there room left in this block to add the final length? The length is 64
    // bits (8 bytes, 2 uint32_ts) and the block is 16 uint32_ts
    if (nwords > 14)
    {
        // There is no room for the length in this block so pad it to 512 bits
        // and process it now.
        while (nwords < 16) block[nwords++] = 0;
        SHA256Transform(digest, block);

        // Start again with an empty block
        nwords = 0;
    }

    // Here, we have nwords of data and guaranteed space for the final length.
    // Add padding (b) as needed to fill up all but the last 64 bits of the block
    assert(nwords <= 14);
    while (nwords < 14) block[nwords++] = 0;

    // Put the length in position. The length isn't large enough to spill into
    // the top word
    assert(sizeof(bufferLength) <= sizeof(uint32_t));
    block[14] = 0;
    block[15] = bufferLength;

    // Process the final block
    SHA256Transform(digest, block);

    // Restore the endianness
    {
        int i;
        for (i = 0; i < 8; ++i) digest[i] = HTONL(digest[i]);
    }
}


/** Compute the SHA-256 digest of a buffer of data
 * 
 * Compute from scratch the SHA-256 digest of a buffer containing an arbitrary
 * number of bits.
 * 
 * This is simply a convenience function wrapping the other SHA-256 functions
 * together.
 * 
 * @param buffer        the buffer of data to be hashed
 * @param bufferLength  the length of the buffer in bits
 * @param digest        the place where the digest is to be stored
 */
void Sec_sha256(uint8_t const* buffer, int bufferLength, uint32_t* digest)
{
    Sec_sha256Init(digest);
    int const bitsRemaining = Sec_sha256Update(digest, buffer, bufferLength);
    int const bytesUsed = (bufferLength - bitsRemaining) / 8;
    Sec_sha256Final(digest, buffer + bytesUsed, bufferLength, bitsRemaining);
}

int sec_verify_sha256(uint8_t const * buf, int size, uint8_t const * dgst)
{
   uint8_t digest[SHA256_S_DIGEST8]  __attribute__ ((aligned (4)));
   Sec_sha256((uint8_t const*)buf, size*8, (uint32_t*)digest);
   return memcmp((const void*)dgst, (const void*)digest, SHA256_S_DIGEST8);
}
