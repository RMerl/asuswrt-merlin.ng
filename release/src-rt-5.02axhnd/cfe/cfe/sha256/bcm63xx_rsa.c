/**********************************************************************
 *  
 *  bcm63xx_rsa.c       
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
 *
 ********************************************************************* 
 */
#include "bcm_secPrivate.h"
#include "bcm_sec.h"
#include "bcm_stdint.h"

#include "lib_string.h"

#include <stdbool.h>

#define assert(x)

/* -----------------------------------------------------------------------------
 * The following functions provide multiple-precision arithemetic for integers
 * of up to MPSIZE bits.
 * 
 * For a multiple-precision number held in a vector a,
 *  |a[0]| stores the actual size (in 32-bit words), say n.
 *  a[1] ... a[n] store the big number, a[1] is the least significant digit
 *
 *  The absolute value of the number represented by 'a' is the sum over i=1..n 
 *  of a[i]*2^((i-1)*NBITS).
 *  
 * The sign of this number is the sign of a[0] [Note: most of the time, only
 * positive numbers are used]
 *
 * Vectors holding multiple-precision numbers are normally EXTSIZE words.If the
 * vector is to hold the result of a multiplication, it will be LONGEXTSIZE
 * words.
 * 
 * This code was provided by the STB team.
 */

/** Set a multiple-precision value to zero
 * 
 * @param a     the value to set to zero
 */
static void mpzero (mp a)
{
    *(a++) = 1L; 
    *a = 0L;
}


/** Multiple precision comparison - return the sign of a - b
 * 
 * @param a     the first value to compare
 * @param b     the second value to compare
 * @return      -1, 0, 1 as a <, =, > b
 */
static int32_t mpcompare (mp a, mp b)
{
    uint32_t *aa = a;
    uint32_t *bb = b;
    uint32_t ina;
    uint32_t inb;

    uint32_t sa = *aa;
    uint32_t sb = *bb;
    uint32_t isazero = iszero(aa);
    uint32_t isbzero = iszero(bb);

    /* Any zero? */
    if (isazero || isbzero)
        return (isazero-isbzero);

    /* different length? */
    if ( sa!= sb)
        return (sa < sb)? (-1): (1);
    
    aa += sa;
    bb += sa;
    for (; sa>0; sa--, aa--, bb--)
    {
        ina = *aa;
        inb = *bb;
        if (ina != inb) 
            return (ina > inb) ? 1 : -1;
    }
    
    return 0;
}


/** multiple-precision addition c = a + b
 * 
 * Given two multiple-precision integers a and b, return their sum in the 
 * multiple-precision integer c, which must be at least as long a the longer of
 * a and b.
 * 
 * @param a     the first summand as a multi-precision integer
 * @param b     the other summand as a multi-precision integer
 * @param c     the sum as a multi-precision integer
 */
static void mpadd (mp a, mp b, mp c)
{
	uint32_t *aa = a;
	uint32_t *bb = b;
	uint32_t *cc = c;
	uint32_t i = 0;
	uint32_t carry=0;
	uint32_t sa;
	uint32_t sb;
	uint32_t bdigit;

	/* ensure length(a) >= length(b) */
	if (mpcompare(aa, bb)<0)
		mpswap(aa,bb);

	sa = *aa;
	sb = *bb;

	/* Addition loop */	
	aa++;
	bb++;
	cc++;
	for (; i<sa; i++, aa++, bb++, cc++)
	{
		bdigit = (i>=sb)? 0L : *bb;

		*cc = (uint32_t) (*aa + bdigit + carry);
		carry = 0;
		if (*cc >= RADIX) /* overflow */
		{
			*cc -= RADIX;
			carry = 1;
		}
	}

	if (carry) {
		*cc = carry;
		sa++;
	}

	/* Set length of sum c */
	*c = sa; 
}


/** multiple-precision subtraction c = a - b
 * 
 * Given two multiple-precision integers a and b, return their difference in the 
 * multiple-precision integer c, which must be at least as a.
 * 
 * @param a     the minuend as a multi-precision integer
 * @param b     the subtrahend as a multi-precision integer
 * @param c     the difference as a multi-precision integer
 */
static void mpsub (mp a, mp b, mp c)
{
	uint32_t *aa = a;
	uint32_t *bb = b;
	uint32_t *cc = c;
	uint32_t sa = *aa;
	uint32_t sb = *bb;
	uint32_t ina;

	uint32_t bdigit;
	uint32_t i = 1;
	uint32_t carry=0;

	uint32_t lastnz=1;
	uint32_t v, w;

	if (aa==bb) 
	{
		mpzero(cc);
		return;
	}
	
	/* Subtraction loop */	
	aa++;
	bb++;
	cc++;
	for (; i<=sa; i++, aa++, bb++, cc++)
	{
		bdigit = (i>sb)? 0L : *bb;
		w = bdigit+carry;

		/* check for underflow */
		carry = 0;
		ina = *aa;
		if (ina < w)
		{
			v = (uint32_t)(ina+(RADIX-w));
			carry = 1;
		}else
			v = (uint32_t)(ina - w);

		if (v) lastnz=i;
		*cc = v;
	}

	/* Set length of c */
	*c = lastnz;
}


/** Copy one multiple-precision value to another
 * 
 * The first multiple-precision value is overwritten by copying from the second.
 * Unused words in the second value are not copied.
 * 
 * @param a
 * @param b
 */
static void mpcopy (mp a, mp b)
{
    uint32_t i = (*a)+1;
    for (; i > 0; i--, a++, b++)
        *b = *a;
}


/** Shift a multiple-precision value left by a multiple of NBITS bits
 * 
 * Because this is a multiple of NBITs (the number of bits used per word), this
 * is equivalent to shifting complete words.
 * 
 * @param a     the multiple-precision value to be shifted
 * @param k     the number of multiples of NBITS to shift
 * @param b     place to return the shifted value
 */
static void mpclshift (mp a, uint32_t k, mp b)
{
    uint32_t *aa = a;
    uint32_t *bb = b;
    uint32_t kk = k;
    uint32_t sa = *a;
    uint32_t sb = sa+kk;

    if (iszero(aa)) return;

    /* set new length */
    *bb = sb;

    /* shift the remaining chunks */
    for (aa+=sa, bb+=sb; sb>0; sb--, aa--, bb--)
        *bb = (sb>kk)? *aa : 0L;
}

/*
    smalldiv returns 'q' and 'r' so

    q = (hi*RADIX + lo)/d;
    r = (hi*RADIX + lo)%d;
  
    Assumes 0 <= hi < d < RADIX and 0 <= lo < RADIX.
*/
static int smalldiv (uint32_t hi, uint32_t lo, uint32_t d, uint32_t *q, uint32_t *r)
{
    uint32_t tmp; 
    uint32_t i; 
    uint32_t numbits = (NBITS << 1); 

    uint32_t lr = 0;
    uint32_t lq = 0; 

    if (d==0L) return 0;
    
    //printf("x= %d, hi = %x (%d), lo= %x (%d), d= %x (%d)\n", hi*RADIX+lo, hi, hi, lo, lo, d, d);
    if (hi)
    {
        while ( (tmp=((lr << 1) | ((hi & MSBMASK) >> NBITSM1))) < d )
        {
            lr = tmp; 
            hi = (hi << 1) & RADIXM;;
            lo <<= 1;
            if (lo>=RADIX) 
            {
                hi |= 0x00000001L;
                lo &= RADIXM;
            }
            numbits--;
            //printf("x = %d, hi= %x (%d), lo= %x (%d), lr= %x (%d), numbits= %d\n", 
            //      hi*RADIX+lo, hi, hi, lo, lo, lr, lr, numbits);
        }
        for (i=0; i<numbits; i++)
        {
            lr = (lr << 1) | ((hi & MSBMASK) >> NBITSM1);
            lq = (lq << 1) & RADIXM;
            //printf("i= %d, lq = %x (%d), lr= %x (%d)\n", i, lq, lq, lr,lr);
            if (lr >= d) 
            {
                //printf ("lr > d: lr= %x (%d), d= %d\n", lr, lr, d);
                lq |= 1;
                lr -= d;
                //printf ("lr > d: lq= %x (%d), lr= %x (%d)\n", lq, lq, lr, lr);
            }
        
            hi = (hi << 1) & RADIXM;;
            lo <<= 1;
            if (lo>=RADIX) 
            {
                hi |= 0x00000001L;
                lo &= RADIXM;
            }
            //printf("x << 1: hi= %x (%d), lo= %x (%d)\n", hi, hi, lo, lo);
        }
    }
    else
    {
        lr = lo % d;
        lq = lo / d;
    }
    //printf("lq= %d, lr= %d\n", lq, lr);
    *r = lr; 
    *q = lq;

    return 1;
}


/*
 * w = (w + c + x * y) / RADIX; 
 * c = (w + c + x * y) % RADIX;
 */
static void addmul(uint32_t *w, uint32_t *c, uint32_t x, uint32_t y)
{ 
    uint32_t lx = x; 
    uint32_t ly = y; 
    uint32_t x1 = x & RADIXROOTM; 
    uint32_t y1 = y & RADIXROOTM; 
    uint32_t cc = (*c) + x1 * y1;

    x1 = x1 * (ly >>= NBITSH) + y1 * (lx >>= NBITSH) + (cc >> NBITSH); 
    cc = (cc & RADIXROOTM) + ((x1 & RADIXROOTM) << NBITSH) + *w; 
    *w = ly * lx + (x1 >> NBITSH) + (cc >> NBITS); 
    *c = (cc & RADIXM); 
    
    return;
}

static void mpmul (mp x, mp y, mp w)
{
	uint32_t n = (*x)-1;
	uint32_t t = (*y)-1;
	uint32_t i,j;
	uint32_t *ww = w;
	uint32_t c, tt=t+1;

	uint32_t sw = n+t+2;

	for (i=sw; i>0; i--)
		*(ww++) = 0L;
	for (i=1; i<=tt; i++)
	{
		c = 0L;
		for (j=0; j<=n; j++)
			addmul (&c, &(w[i+j]), x[j+1], y[i]);
		w[i+j] = c;
	}

	for (i=sw; i>1 && !w[i]; i--) 
		; 
	*w = i;
	return;
} 

/* Left shift 0<= k <30 bits */
static void mplshift (mp a, uint32_t k, mp b)
{
	uint32_t sa = *a;
	uint32_t *aa = a+sa;
	uint32_t *bb = b+sa;
	uint32_t kk = k;
	
	uint32_t w;

	*b = sa;
	
	/* shifting last chunk may yield an additional chunk */
	if ((w = (((uint32_t)*aa) >> (NBITS-kk))) != 0L)
	{
		*(bb+1) = w; 
		*b = sa+1;
	}

	/* shift the remaining chunks */
	for (; sa>1; sa--, aa--,bb--)
		*bb = ( ((((uint32_t)(*aa)) << kk) & MASK) | 
				((((uint32_t)(*(aa-1))) >> (NBITS-kk))) );

	*bb = (((uint32_t)*aa) << kk) & MASK; /* last iteration, when sa == 1 */
	return;
}

/* Right shift 0<= k <30 bits */
static void mprshift (mp a, uint32_t k, mp b)
{
	uint32_t *aa = a+1;
	uint32_t *bb = b+1;
	uint32_t kk = k;
	uint32_t sa = *a;
	uint32_t i=1;

	/* shift the remaining chunks */
	for (; i<sa; i++, aa++, bb++)
		*bb = ( (((uint32_t)(*aa)) >> kk) | ((((uint32_t)(*(aa+1))) << (NBITS-kk)) & MASK) );

	/* shifting last chunk may clear it */
	if (((*bb = (((uint32_t)(*aa)) >> kk)) == 0L) && (sa>1))
		sa--;
	*b = sa;
	return;
}

/*	Division of mp numbers:			
 *
 *  q = x / y, 
 *  r = x % y 
 *  - Assume y!=0
 *  - always y*q + r == x, 
 *  - unless y | x, sign(r) == sign(y)
 *
 *  Result undefined if y = 0.
 */
static int mpdiv (mp x1, mp y1, mp q, mp r)
{
	uint32_t i;
	uint32_t ii;
	uint32_t tt;
	uint32_t imt;

	uint32_t n,t,nt1,was_corrected;
	uint32_t sx, sy;
	uint32_t  val, rem, leadingZeros;
	uint32_t qq[2] = {1L, 0L};

	uint32_t xx[LONGEXTSIZE];
	uint32_t yy[EXTSIZE];
	uint32_t ybnt[LONGEXTSIZE];
	uint32_t aux[LONGEXTSIZE];

	CLZ (leadingZeros, y1[*y1]);
	leadingZeros -= UNUSEDBITS;

	/* normalization */
	mplshift(x1, leadingZeros, xx);
	mplshift(y1, leadingZeros, yy);

	sx = *xx;
	sy = *yy;
	n = sx-1;
	t = sy-1;
	tt = t+1;
	if (n<t)
	{
		mpcopy(x1,r);
		mpzero(q);
		return 1;
	}
	nt1 = n-t+1;

	for (i=0; i<=nt1; i++)
		q[i] = 0L;

	mpclshift(yy,nt1-1,ybnt);

	while (mpcompare(xx,ybnt)>=0)
	{
		q[nt1]++;
		mpsub (xx, ybnt, xx);
	}

	for (i=n; i>=tt; i--)
	{
		ii = i+1;
		imt = i-t;

		if (xx[ii] == yy[tt]) 
		{
			q[imt] = RADIX -1;
		}
		else
		{
			if (!smalldiv (xx[ii], xx[i], yy[tt], &(q[imt]),&rem))
				return 0; 
		}
		if (t>0) {
		do
		{
			/* create xx bar */
			val = yy[t-1];
			yy[t-1]= (yy[tt]==0)? 1: 2;
			/* create qq */
			qq[1] = q[imt];
			mpmul (yy+t-1,qq,aux);

			yy[t-1] = val;

			/* create xxp */
			val = xx[i-2];
			xx[i-2] = ((xx[ii]==0)? ((xx[i]==0)? 1: 2): 3);

			if ( mpcompare (aux,xx+i-2) > 0 )
			{
				was_corrected = 1;
			}
			else
			{
				was_corrected = 0;
			}
			
			if (was_corrected == 1)
			{
				q[imt]--; 
			}
			
			xx[i-2] = val; /* restore xx[i-2] value */
		} 
		while (was_corrected);
		}
		
		mpclshift(yy,imt-1,aux);
		qq[1] = q[imt];

		mpmul (aux, qq, ybnt);
		if (mpcompare(ybnt,xx)>0)
		{
			mpadd (xx, aux, xx);
			q[imt]--;
		}
		mpsub (xx, ybnt, xx);
	}
	mprshift (xx, leadingZeros, xx); /* because of normalization */

	/* adjust lengths */
	for (i=nt1; i>1 && !q[i]; i--)
		;
	*q = i;

	for (i=*xx; i>1 && !xx[i]; i--)
		;
	*xx = i;

	/* copy the remainder to r */
	mpcopy (xx, r);
	return 1;
} 


static int unpack(mp src30, uint32_t* dst, uint32_t dstlen)
{
    // Work out how many bits there are in the number
    uint_fast16_t const bitlen = src30[0] * NBITS;
 
    // Work out how many uint32_t this will use in the destination, rounding up
    int_fast16_t const wordcount = (bitlen + BITSOFLONG - 1) / BITSOFLONG;
   
    // Work out if we will leave any of the destination empty and if so, set
    // them to zero. We will start unpacking at the last element, dst[dstlen-1]
    // and fill wordcount elements up to and including dst[dstlen-wordcount-1],
    // so we need to fill elements dst[0..dstlen-wordcount-1] with zeros.
    int_fast16_t const lowestIndex = (int)dstlen - (int)wordcount;
    {
        int i;
        for (i = 0; i < lowestIndex; ++i)
        {
            dst[i] = 0;
        }
    }
    // Now set the remaining wordcount words with bits pulled from our source,
    // padding the last word if necessary.    
    uint_fast8_t numBitsFromThisWord = NBITS;
    uint_fast8_t numBitsFromNextWord = BITSOFLONG - numBitsFromThisWord;
    uint_fast16_t srcIndex = 1;
    {

        int i;
        for ( i = dstlen - 1; i >= lowestIndex; --i)
        {
            // If we are to take no bits from the current word, it means we used
            // them all up last time, so skip to the next word
            if (numBitsFromThisWord == 0)
            {
                srcIndex += 1;
                numBitsFromThisWord = NBITS;
                numBitsFromNextWord = BITSOFLONG - numBitsFromThisWord;
                assert (srcIndex < src30[0]);
            }

            // If there isn't a source word after this one, we will need to pad with
            // zeros. Set up nextWords to be the data or padding. If src30[0] says
            // there are N words, these are held in elements 1..N
            uint32_t const nextWord = srcIndex + 1 > src30[0] ? 0 : src30[srcIndex+1];

            // Take the required number of bits from the topmost of the 30 in the
            // current word by shifting out the bits not wanted
            uint32_t const lowerBits = (src30[srcIndex] & 0x3fffffff) >> (NBITS - numBitsFromThisWord);

            // Take the remaining bits from the next word
            uint32_t const bottomMask = (1 << numBitsFromNextWord) - 1;
            uint32_t const upperBits = nextWord & bottomMask;

            // Combine these, putting the bits from the current word at the bottom
            // and the ones from the next word at the top
            uint32_t result = (upperBits << numBitsFromThisWord) | lowerBits;

            // On a little-endian machine, reverse the byte ordering to give a big-
            // endian bytestream
            result = HTONL(result);

            // Now store this and advance to the next. If we have run out of space
            // to store the result and it isn't zero, abandon the conversion
            if (i < 0 && result != 0) return 0;

            dst[i] = result;

            numBitsFromThisWord -= 2;
            numBitsFromNextWord += 2;
            srcIndex += 1;
        }
    }
    // Return the number of words we used
    return wordcount;
}


/* Pack a sequence of 32-bit words into 30-bit chunks, reversing
 * the order in the process (so it follows the ordering specified in
 * PKCS#1 v2.1 standard. See the OS2IP primitive).
 *
 * The resulting sequence is a valid 
 * representation of the given sequence as a mp number.
 *
 * n is the size of the input in 32-bit words.
 * The implementation assumes src32[n] is a "readable" address and n>=1.
 */

static void pack (uint32_t *src32, mp dest30, uint32_t n)
{
	uint32_t *lsrc32  = src32;
	uint32_t *ldest30 = dest30;
	uint32_t i;
	uint32_t shift;
	uint32_t mask, u, w=0L;
	
	lsrc32 += (n-1);
	ldest30++;

	/* First piece is always rightmost */
	u = HTONL(*lsrc32);
	w = u & MASK;	
	shift = 0;
	for (i=1; i<=n; i++, lsrc32--, ldest30++)
	{
		*ldest30 = ROTL32(w, shift);
	
		/* Compute shifting offset */
		shift = ((shift+UNUSEDBITS) % 32);

		/* Leftmost piece */
		mask  = (0xFFFFFFFFUL >> shift);		
		w = (u & ~mask); /* If shift==0 then ~mask==0 so w==0 */
		if (i<n)
		{	
			if ( shift==0 )
			{
				lsrc32++;
				n++; 
			} 
			/* Add rightmost piece */
			/* Note: If shift==30 then value in *(lsrc32-1) is not used, u is always 0 */
			u = HTONL(*(lsrc32-1));
			w |= ( u & (mask >> UNUSEDBITS));
		}
	}
	/* store last leftmost piece */
	*ldest30 = ROTL32(w, shift);

	/* adjust length, just in case most significant bits are zero */
	for (i=n+1; i>1 && !(*ldest30); i--, ldest30--)
		;
	*dest30 = i;
	
	return;
}



/** Decrypt a RSASSA-PSS signature to yield the encoded message
 * 
 * Given an RSASSA-PSS signature and the public key corresponding to the private
 * key that was used to produce the signature, decrypt the signature and return
 * the resulting bitstream.
 * 
 * All three buffers are expected to be RSA_S_MODULUS8 bytes long.
 * The public exponent is assumed to be F4
 * 
 * @param signature     address of the signature
 * @param siglen        number of bytes in the signature 
 * @param publickey     address of the public key modulus
 * @param publen        number of bytes in the modulus
 * @param em            address where the encoded message is to be returned
 * @param emLen         length in bytes of the encoded message buffer
 * @return              true if decryption succeeded
 */
static SecStatus rsapss_rsavp1(uint32_t const* signature, int siglen, uint32_t const* publickey, int publen, uint32_t* em, int emLen)
{
    // 1. Length checking: if the length of the signature S(signature) is not 
    //    k(RSA_S_MODULUSBITS), output 'invalid signature' and stop
    if (siglen != SEC_S_SIGNATURE) return SEC_E_BADSIGLEN;
    
    // We also check the length of the public key modulus and the length we 
    // expect the encoded message to be
    if (publen != RSA_S_MODULUS8) return SEC_E_BADPUBLEN;
    if (emLen  != RSA_S_MODULUS8) return SEC_E_BADEMLEN;
    
    // emBits is the maximal bit length of the integer EM and is at least 
    // 8.hLen + 8.sLen + 9:
    if (emLen*8 < 8*RSA_S_HLEN + 8*RSA_S_SLEN + 9) return SEC_E_BADEMLEN;
    
    // 2. RSA verification:
    
    uint32_t i;
    static uint32_t input[EXTSIZE];    // The original encrypted message
    static uint32_t modulus[EXTSIZE];  // The modulus component of the public key
    static uint32_t output[EXTSIZE];   // The result of input^i mod modulus so far
    static uint32_t tmp[LONGEXTSIZE];  // Intermediate result during each iteration
        
    // a. Convert the signature S(signature) to an integer signature 
    //    representative 
    pack((uint32_t *)signature, input, RSA_S_MODULUS32);
    
    // Repeat this for the public key
    pack((uint32_t *)publickey, modulus, RSA_S_MODULUS32);
    
    // b. Apply the RSAVP1 verification primitive to the RSA public key (n, e)
    //    (modulus, 65537) and the signature representative s(input) to produce
    //    an integer message representative m(output).
    
    // output = input ^ 1
    mpcopy(input, output);
    
    /* Square output sixteen times, giving input^(2^16) and multiply once by
     * input, giving input^(2^16+1) = input^65537. Taking the modulus each time
     * gives us the result input^F4 mod pubkey, the original secret.
     */
    for (i=0; i<17; i++)
    {
        mpmul(output, (i == 16) ? input : output, tmp);        
        if (!mpdiv(tmp, modulus, tmp, output)) return SEC_E_DIVFAIL;
    }

    // c. Convert the message representative m(output) to an encoded message EM
    //    (em) of length emLen = ceil((modBits - 1) / 8) octets
    unpack(output, em, emLen/sizeof(uint32_t));
    
    return SEC_S_SUCCESS;
}
   

/** Implementation of mask generation function MGF1
 * 
 * IEEE 1363a specified this algorithm as part of the specification of the EMSA4
 * method for signatures with appendix (based on the Probabilistic Signature
 * Scheme).
 * 
 * It generates a mask of the required length using an input bit string and a
 * hash function - in this case SHA-256.
 * 
 * The algorithm is described in section 14.2.1 of IEEE 1363a. We constrain the
 * length of mask we will generate to the size of the public key modulus we are
 * using. We also constrain the seed bit string to have the length of the hash
 * value.
 * 
 * @param mgfSeed   the seed bit string
 * @param seedLen   the length of the bit string in octets
 * @param maskLen   the length of the required mask in octets
 * @param mask      the place to return the mask
 */
static SecStatus rsapss_mgf1(uint8_t const* mgfSeed, int seedLen, int maskLen, uint8_t* mask)
{
    // 1. If maskLen > 2**32.hLen, output 'mask too long' and stop
    //
    // We actually have a tighter constraint so we can determine the size at 
    // compilation time and allocate it on the stack.
    if (maskLen > RSA_S_MODULUS8) return SEC_E_BADMASKLEN;
    if (seedLen != RSA_S_HLEN) return SEC_E_BADSEEDLEN;
    
    // We need to copy the seed to a place where we can append the counter
    // each time in step 3:
    uint8_t seedAndCounter[RSA_S_HLEN+4];
    memcpy(seedAndCounter, mgfSeed, seedLen);
    
    // 4. Let MB be the empty octet string ...
    uint8_t MB[RSA_S_MODULUS8];
    uint8_t* nextMB = &MB[0];
    
    //    ... Let cThreshold = ceil(oBits/hBits)
    //
    // We know we are handling octets, so this is:
    int const cThreshold = (maskLen + RSA_S_HLEN - 1) / RSA_S_HLEN;
    
    // 3. For counter from 0 to ceil(maskLen / hLen) - 1 do the following:
    
    // There must be room in T
    assert(cThreshold * RSA_S_HLEN <= sizeof(MB));
    {
        uint32_t counter;
        for(counter = 0; counter < cThreshold; ++counter)
        {
            // 5.1 Convert counter to a bit string CB of length 32 bits using I2BSP
            uint8_t CB[4] = {
                counter >> 24,
                (counter >> 16) & 0xff,
                (counter >> 8) & 0xff,
                counter & 0xff
            };

            // 5.2 Compute Hash(ZB || CB) with the selected hash function to produce
            //     a bit string HB of length hBits
            memcpy(seedAndCounter + seedLen, CB, sizeof(CB));
            uint32_t HB[RSA_S_HLEN/sizeof(uint32_t)];
            Sec_sha256(seedAndCounter, 8*(seedLen+sizeof(CB)), HB);

            // Let MB = MB || HB
            memcpy(nextMB, HB, sizeof(HB));
            nextMB += sizeof(HB);
        }
    }
    
    // 5.5 Let maskB be the leftmost oBits bits of MB
    memcpy(mask, MB, maskLen);    
    
    return SEC_S_SUCCESS;
}


/** EMSA4 verification
 * 
 * This function implements the EMSA4 decoding operation described in IEEE 1363a
 * but assumes that we have been passed the SHA-256 hash of the non-recoverable
 * (appended) message M2.
 *
 * For our use, we know the following:
 * - For decryption, the public exponent will always be F4.
 * - We know that the salt length is the same as the hash length
 * - Because this is EMSA4, not EMSR4, we know there is no recoverable message
 * 
 * The encoding steps took a message M2, a zero-length recoverable message MB1
 * and a 64-bit value C1 representing the length of MB1 (0) and then:
 * 
 * 1. H2 = Hash(M2)
 * 2. M' = C1 || MB1 || H2 || salt, where salt is randomly generated
 * 3. DB = S || 1 || MB1 || salt, where S is enough zero bits to fill the buffer
 * 4. H = Hash(M')
 * 5. dbMask = MGF1(H)
 * 6. maskedDB = DB ^ dbMask
 * 7. T = maskedDB || H || 0xBC
 * 
 * This signature T was then encrypted using a private key. To verify that T is
 * the signature for the appended message M2, we perform the following steps:
 * 
 * 1. Decrypt the signature we were passed to recover T = maskedDB || H || 0xBC
 * 2. Use H to regenerate the mask used during encoding, dbMask = MGF1(H)
 * 3. Recover DB = maskedDB ^ dbMask
 * 4. Knowing that MB1 is null, strip the leading S || 1 from DB to recover salt
 * 5. Use H2 (Hash(M2) from the M2) that our caller passed
 * 6. Knowing C1 is 8*0x00 and MB1 is null, form M' = C1 || MB1 || H2 || salt
 * 7. H' = Hash(M')
 * 8. Check that H' = H
 * 
 * @param signature     The signature encrypted by a private key
 * @param siglen        The length of the signature in octets
 * @param publickey     The modulus of the public key
 * @param publen        The length of the modulus
 * @param H2            The SHA-256 digest of the non-recoverable message M2
 * @param H2len         The length of digest in octets
 * @return              SEC_S_SUCCESS iff the signature signed the message
 */
SecStatus Sec_verifyDigest(uint32_t const* signature, int siglen, uint32_t const* publickey, int publen, uint8_t const* H2, int H2len)
{
    // We expect the signature and public key to be the length of the public key 
    // modulus
    if (siglen != RSA_S_MODULUS8) return SEC_E_BADSIGLEN;
    if (publen != RSA_S_MODULUS8) return SEC_E_BADPUBLEN;
    if (H2len  != SEC_S_DIGEST)   return SEC_E_BADDIGLEN;
    
    // Recover the message representative T from the signature by decrypting it
    // using the given public key modulus. The buffer needs to be aligned
    // so that we can perform the xor operation on 64-bit values later.
    uint8_t T[RSA_S_MODULUS8] __attribute__ ((aligned (8)));
    SecStatus const rc = rsapss_rsavp1(signature, siglen, publickey, publen, (uint32_t*)T, sizeof T);
    if (rc != SEC_S_SUCCESS) return rc;
    
    // The comments here reflect the steps described in section 12.3.3.2 of the
    // IEEE 1363a-2004 description of EMSA4 as EMSR3 with a zero recovered
    // message (that is, the message is the appendix).
    

    // If emlen < hLen + sLen + 2, output 'inconsistent' and stop
    if (sizeof(T) < RSA_S_HLEN + RSA_S_SLEN + 2) return SEC_E_TSHORT;
    
    // What we have now in T should be maskedDB || H || 0xbc
    //
    // 5. ... If hash function identification is not desired then verify that 
    //    the rightmost octet of T has the hexadecimal value 0xBC. If 
    //    verification fails, output 'invalid' and stop.
    if (T[sizeof(T)-1] != 0xBC) return SEC_E_TENDNOTBC;
    
    
    // 6. Let maskedDB be the leftmost emLen - hLen - 1 octets of T and let H
    //    be the next hLen octets
    uint8_t const* const maskedDB = &T[0];
    uint8_t const* const H = &T[sizeof(T) - RSA_S_HLEN - 1];
    
    // 7. Verify that the leftmost t bits of maskedDB are zero
    //
    // Here, we know emLen is a multiple of eight and so 8*emLen - emBits is 
    // zero so there is nothing to check
    
    // 8. Apply the mask generation function G to the string H to produce a bit
    //    string dbMask of length 8.oLen - hBits - 8 bits.
    uint8_t dbMask[sizeof(T) - RSA_S_HLEN - 1] __attribute__ ((aligned (8)));
    SecStatus const mgfrc = rsapss_mgf1(H, RSA_S_HLEN, sizeof dbMask, dbMask); 
    if (mgfrc != SEC_S_SUCCESS) return mgfrc;
    
    // 9. Let DB = maskedDB ^ dbMask
    //
    // Process this 8 bytes at a time and then handle the remaining 0-7 bytes
    // specially - so make sure the buffer is aligned correctly.
    uint8_t DB[sizeof(T) - RSA_S_HLEN - 1] __attribute__ ((aligned (8)));
    
    uint64_t*       db8       = (uint64_t*)DB;
    uint64_t const* maskedDB8 = (uint64_t const*)maskedDB;
    uint64_t const* dbMask8   = (uint64_t const*)dbMask;
    
    int const numLongs = sizeof(DB) / sizeof(uint64_t);
    int const numBytesLeft = sizeof(DB) - numLongs*sizeof(uint64_t);
    {
        int i;
        for (i = 0; i < numLongs; ++i)
        {
            *db8++ = (*maskedDB8++) ^ (*dbMask8++);
        }
    }
    
    uint8_t*       dbLeft       = (uint8_t*)db8;
    uint8_t const* maskedDBLeft = (uint8_t const*)maskedDB8;
    uint8_t const* dbMaskLeft   = (uint8_t const*)dbMask8;
    {
        int i;
        for (i = 0; i < numBytesLeft; ++i)
        {
            *dbLeft++ = (*maskedDBLeft++) ^ (*dbMaskLeft++);
        }
    }
    // What we have now in DB is S || 1 || MB1 || salt, where we expect MB1 to
    // be empty, as there is no recoverable message.
    
    // 10. Set the leftmost t bits of the leftmost octet in DB to zero.
    //
    // As above, we know emLen is a multiple of eight and so the value of t,
    // 8*emLen - emBits is zero so there should be nothing to do. However,
    // OpenSSL calculates t as 1 (to make sure that its integers are unsigned),
    // so we need to clear the top bit.
    DB[0] &= 0x7f;
    
     
    // 11. Let MB1' be the leftmost 8.oLen - saltBits - hBits of DB and let
    //     salt be the rightmost saltBits bits:
    uint8_t const* const MB1dash = DB;
    uint8_t const* const salt = DB + sizeof(DB) - RSA_S_SLEN;
        
    // Now we have MB1' = S || 1 || MB1, where MB1 should be null and so we 
    // expect MB1' = 00 00 .... 01
    
   // 12. Let U be the leftmost non-zero bit of MB1'. If MB1' has no non-zero
    //     bits, output 'invalid' and stop
    //
    // Advance over the leading zeros. We should eventually find an 0x01 just
    // before we run into the salt.
    uint8_t const* U = MB1dash;
    while (*U == 0x00 && U < salt) U += 1;
    if (*U != 0x01 || U + 1 != salt) return SEC_E_BADLEADER;
    
    // 13. Remove U and all the bits to the left of it (which are all zero) to
    //     produce a bit string MB1. (The bit string MB1 may be empty.) Let
    //     m1bits be the length in bits of MB1
    //
    // Here, we know U is the first bit of DB and so the bit string MB1 will be
    // empty and so its length will be zero. 
    
    // 14. Convert m1Bits to a bit string C1 of length 64 bits using I2BSP:
    uint64_t const C1 = 0;
    
    // 15. Let M' = C1 || MB1 || H2 || salt
    //
    // We know MB1 is the null string and C1 = 0
    uint8_t Mdash[(8 + RSA_S_HLEN + RSA_S_SLEN)];
    memcpy(Mdash,                           &C1,  sizeof(C1));
    memcpy(Mdash + sizeof(C1),              H2,   RSA_S_HLEN);
    memcpy(Mdash + sizeof(C1) + RSA_S_HLEN, salt, RSA_S_SLEN);
    
    // 16. Compute Hash(M') with the selected hash function to produce an octet
    //     string H' of ength hBits bits.
    uint32_t Hdash[RSA_S_HLEN/sizeof(uint32_t)];
    Sec_sha256(Mdash, 8*sizeof(Mdash), Hdash);
        
    // 17. If H = H', output 'invalid' 
    // Note that there is no recoverable message MB1 to return
    return memcmp(H, Hdash, RSA_S_HLEN) == 0 ? SEC_S_SUCCESS : SEC_E_MISMATCH;
}

/** EMSA4 verification
 * 
 * This function implements the EMSA4 decoding operation described in IEEE 1363a
 *
 * For our use, we know the following:
 * - For decryption, the public exponent will always be F4.
 * - We know that the salt length is the same as the hash length
 * - Because this is EMSA4, not EMSR4, we know there is no recoverable message
 * 
 * The encoding steps took a message M2, a zero-length recoverable message MB1
 * and a 64-bit value C1 representing the length of MB1 (0) and then:
 * 
 * 1. H2 = Hash(M2)
 * 2. M' = C1 || MB1 || H2 || salt, where salt is randomly generated
 * 3. DB = S || 1 || MB1 || salt, where S is enough zero bits to fill the buffer
 * 4. H = Hash(M')
 * 5. dbMask = MGF1(H)
 * 6. maskedDB = DB ^ dbMask
 * 7. T = maskedDB || H || 0xBC
 * 
 * This signature T was then encrypted using a private key. To verify that T is
 * the signature for the appended message M2, we perform the following steps:
 * 
 * 1. Decrypt the signature we were passed to recover T = maskedDB || H || 0xBC
 * 2. Use H to regenerate the mask used during encoding, dbMask = MGF1(H)
 * 3. Recover DB = maskedDB ^ dbMask
 * 4. Knowing that MB1 is null, strip the leading S || 1 from DB to recover salt
 * 5. Form H2 = Hash(M2) from the M2 our caller passed
 * 6. Knowing C1 is 8*0x00 and MB1 is null, form M' = C1 || MB1 || H2 || salt
 * 7. H' = Hash(M')
 * 8. Check that H' = H
 * 
 * @param signature     The signature encrypted by a private key
 * @param siglen        The length of the signature in octets
 * @param publickey     The modulus of the public key
 * @param publen        The length of the modulus
 * @param M2            The message the signature is expected to have signed
 * @param M2len         The length of the message in octets
 * @return              true iff the signature signed the message
 */
SecStatus Sec_verify(uint32_t const* signature, int siglen, uint32_t const* publickey, int publen, uint8_t const* M2, int M2len)
{
    // If the length of M2 is greater than the input limitiation for the hash
    // function, output 'inconsistent' and stop. Section 14.1.3 (SHA-256) says
    // that M2len shall be at most 2**61 -1 octets.
    if (M2len > (1ull << 61) - 1) return SEC_E_BADMESLEN;
    
    // 2. Compute Hash(M2) with the selected hash function to produce a bit 
    // string H2 of length hBits bits.
    uint8_t H2[SHA256_S_DIGEST8] __attribute__ ((aligned (4)));
    Sec_sha256(M2, M2len*8, (uint32_t*)H2);
    
    // The rest of the algorithm is delegated.
    return Sec_verifyDigest(signature, siglen, publickey, publen, H2, sizeof H2);
}
