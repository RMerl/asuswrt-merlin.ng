/*
 * md4.h	Structures and prototypes for md4.
 *
 * Version:     $Id$
 * License:		LGPL, but largely derived from a public domain source.
 *
 */

#ifndef _FR_MD4_H
#define _FR_MD4_H

RCSIDH(md4_h, "$Id$")

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <string.h>

#ifdef WITH_OPENSSL_MD4
#include <openssl/md4.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void fr_md4_calc (unsigned char *, unsigned char const *, unsigned int);

#ifndef WITH_OPENSSL_MD4
/*  The below was retrieved from
 *  http://www.openbsd.org/cgi-bin/cvsweb/src/include/md4.h?rev=1.12
 *  With the following changes: uint64_t => uint32_t[2]
 *  Commented out #include <sys/cdefs.h>
 *  Commented out the __BEGIN and __END _DECLS, and the __attributes.
 *  Commented out MD4End, MD4File, MD4Data
 *  Commented out header file protection #ifndef,#define,#endif
 */

/*	$OpenBSD: md4.h,v 1.12 2004/04/28 16:54:00 millert Exp $	*/

/*
 * This code implements the MD4 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 * Todd C. Miller modified the MD5 code to do MD4 based on RFC 1186.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

/*#ifndef _MD4_H_*/
/*#define _MD4_H_*/

#define	MD4_BLOCK_LENGTH		64
#define	MD4_DIGEST_LENGTH		16
#define	MD4_DIGEST_STRING_LENGTH	(MD4_DIGEST_LENGTH * 2 + 1)

typedef struct FR_MD4Context {
	uint32_t state[4];			/* state */
	uint32_t count[2];			/* number of bits, mod 2^64 */
	uint8_t buffer[MD4_BLOCK_LENGTH];	/* input buffer */
} FR_MD4_CTX;

/*__BEGIN_DECLS*/
void	 fr_MD4Init(FR_MD4_CTX *);
void	 fr_MD4Update(FR_MD4_CTX *, uint8_t const *, size_t)
/*		__attribute__((__bounded__(__string__,2,3)))*/;
void	 fr_MD4Final(uint8_t [MD4_DIGEST_LENGTH], FR_MD4_CTX *)
/*		__attribute__((__bounded__(__minbytes__,1,MD4_DIGEST_LENGTH)))*/;
void	 fr_MD4Transform(uint32_t [4], uint8_t const [MD4_BLOCK_LENGTH])
/*		__attribute__((__bounded__(__minbytes__,1,4)))
		__attribute__((__bounded__(__minbytes__,2,MD4_BLOCK_LENGTH)))*/;
/*__END_DECLS*/
#else  /* WITH_OPENSSL_MD4 */
USES_APPLE_DEPRECATED_API
#define FR_MD4_CTX	MD4_CTX
#define fr_MD4Init	MD4_Init
#define fr_MD4Update	MD4_Update
#define fr_MD4Final	MD4_Final
#define fr_MD4Transform MD4_Transform
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FR_MD4_H */
