/*
* <:label-BRCM:2012:NONE:standard
* 
* :>
*/

/**********************************************************************
 *  
 *  bcm_secPrivate.h    
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *  
 *  This software is furnished under license and may be used and 
 *  copied only in accordance with the following terms and 
 *  conditions.  Subject to these conditions, you may download, 
 *  copy, install, use, modify and distribute modified or unmodified 
 *  copies of this software in source and/or binary form.  No title 
 *  or ownership is transferred hereby.
 *  
 *  1) Any source code used, modified or distributed must reproduce 
 *     and retain this copyright notice and list of conditions 
 *     as they appear in the source file.
 *  
 *  2) No right is granted to use any trade name, trademark, or 
 *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
 *     name may not be used to endorse or promote products derived 
 *     from this software without the prior written permission of 
 *     Broadcom Corporation.
 *  
 *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
 *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
 *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
 *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
 *     THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************* 
 */
#ifndef SECPRIVATE_H
#define SECPRIVATE_H

/* 
 * Multiple-Precision Modular Arithmetic Routines
 */
#include "lib_types.h"

/* Data structure for big numbers */

typedef uint32_t* mp;			/* we represent each mp number as an array of 
					 * 30-bit long numbers (each group of 30-bit 
					 * will be called a 'chunk') */
/* Other types needed */

#define CHARL		8			/* number of bits in a byte */
#define SIZEOFLONG	4			/* number of bytes in a long */
#define BITSOFLONG  (CHARL*SIZEOFLONG)		/* number of bits in a long */

#define NBITS		30			/* number of bits used per chunk, It must be even and <32 */
#define RADIX		(1L << NBITS)		/* this is 'b' in the algorithms */
#define UNUSEDBITS	(BITSOFLONG-NBITS)	/* no. bits unused in chunk */

#define NBITSH		(NBITS >> 1L)		/* half the number of bits/chunk */
#define RADIXM      	(RADIX-1)		/* RADIX mask */
#define RADIXROOT   	(1L<<NBITSH)		/* "RADIX" for half chunk */
#define RADIXROOTM  	(RADIXROOT-1)		/* RADIXROOT mask */
#define NBITSM1	    	(NBITS-1)					
#define MSBMASK		(1L << NBITSM1)		/* most significant bit mask */
#define MASK		RADIXM			/* marks which bits are used in chunk */

#define MPSIZE		(2048)			/* size in bits of mp numbers */

#define SIZE		((MPSIZE+NBITS-1)/NBITS)+1	/* size in chunks of mp numbers */
#define EXTSIZE		(SIZE+2)		/* include space to store actual size */
#define LONGEXTSIZE	((EXTSIZE*2)+2)		/* for temp results in multiplications */

#define CEILDIV(value,divisor) (((value) + (divisor)-1) / (divisor))

#define SHA256_S_DIGEST     256                  /* Bits in a sha-256 digest  */
#define SHA256_S_DIGEST8    (SHA256_S_DIGEST/8)  /* Bytes in a sha-256 digest */
#define SHA256_S_DIGEST32   (SHA256_S_DIGEST/32) /* Words in a sha-256 digest */

#define RSA_S_MODULUS       2048                 /* Bits in a key modulus     */
#define RSA_S_MODULUS8      (RSA_S_MODULUS/8)    /* Bytes in a key modulus    */
#define RSA_S_MODULUS32     (RSA_S_MODULUS/32)   /* Words in a key modulus    */

#define RSA_S_SALT          SHA256_S_DIGEST      /* Bits in the PSS salt      */
#define RSA_S_HLEN          SHA256_S_DIGEST8     /* Bytes in the hash value   */
#define RSA_S_SLEN          (RSA_S_SALT/8)       /* Bytes in the PSS salt     */

/* For a message representative with no recoverable message (that is, MB1 is
 * null), the encoded message will be
 * 
 * S || 1 || salt || H || 0xbc
 * 
 * where S (the leading zeros added to pad the message) might be null. Hence, 
 * the minimum length of this message in bits will be 0 + 1 + length of salt + 
 * length of hash + 8:
 */
#define RSA_S_EMBITS        (8*RSA_S_HLEN + 8*RSA_S_SLEN + 9)

/* We need to round it up to a whole number of bytes */
#define RSA_S_EMLEN         CEILDIV(RSA_S_EMBITS, 8)

									

/* Swaps two mp numbers */
#define mpswap(a,b)	\
{			\
	mp tmp = a;	\
	a = b;		\
	b = tmp;	\
}

/** Count leading zeros in a 32 bit value
 * 
 * Count the number of leading zeros in the second value given and store the
 * result in the first
 * 
 * rd       location to be set to the count
 * rs       32-bit value to be inspected
 */
#define CLZ(rd, rs)			\
{					\
	int32_t _i=0;	    		\
	rd = 32;			\
	for (_i=31; _i>=0; _i--)	\
		if (rs & (1L << _i))	\
		{			\
			rd = 31-_i;	\
			break;		\
		}			\
}

#define iszero(a)				( *(a)==1L && (a)[1] == 0L )

#define ROTL32(a,n)				((((a) << (n)) & 0xFFFFFFFFUL) \
								| ((a) >> (32 - (n))))

/* We need to be able to convert 32-bit values from big-endian (network) byte
 * order to the host byte order and vice versa. This may be a no-op on some
 * hosts. On the mips, its a no-op operation
 */

#if defined(_BCM963138_) || defined(_BCM963148_) || \
    defined(_BCM94908_) || defined(_BCM96858_) || \
    defined(_BCM963158_) || defined(_BCM96856_) || defined(_BCM96846_)


/* Use this inline below if running code on a linux pc host and not mips, or */
static inline uint32_t HTONL(uint32_t a)
{
    uint32_t ret=a & 0xff;
    a>>=8; ret<<=8; ret |= a & 0xff;
    a>>=8; ret<<=8; ret |= a & 0xff;
    a>>=8; ret<<=8; ret |= a & 0xff;
    return ret;
}

#else

/* Use this inline below if cross compiling into mips */
static inline uint32_t HTONL(uint32_t a)
{
    uint32_t ret=a;
    return ret;
}

#endif

#endif
