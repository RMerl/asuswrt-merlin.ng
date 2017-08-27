/*
 * $Id$
 *
 * @file md5.h
 * @brief Structures and prototypes for md5.
 *
 * @license LGPL, but largely derived from a public domain source.
 */

#ifndef _FR_MD5_H
#define _FR_MD5_H

RCSIDH(md5_h, "$Id$")

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

#ifdef WITH_OPENSSL_MD5
#include <openssl/md5.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WITH_OPENSSL_MD5
/*  The below was retrieved from
 *  http://www.openbsd.org/cgi-bin/cvsweb/~checkout~/src/sys/crypto/md5.h?rev=1.1
 *  With the following changes: uint64_t => uint32_t[2]
 *  Commented out #include <sys/cdefs.h>
 *  Commented out the __BEGIN and __END _DECLS, and the __attributes.
 */

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16

typedef struct FR_MD5Context {
	uint32_t state[4];			/* state */
	uint32_t count[2];			/* number of bits, mod 2^64 */
	uint8_t buffer[MD5_BLOCK_LENGTH];	/* input buffer */
} FR_MD5_CTX;

/* include <sys/cdefs.h> */

/* __BEGIN_DECLS */
void	 fr_MD5Init(FR_MD5_CTX *);
void	 fr_MD5Update(FR_MD5_CTX *, uint8_t const *, size_t)
/*		__attribute__((__bounded__(__string__,2,3)))*/;
void	 fr_MD5Final(uint8_t [MD5_DIGEST_LENGTH], FR_MD5_CTX *)
/*		__attribute__((__bounded__(__minbytes__,1,MD5_DIGEST_LENGTH)))*/;
void	 fr_MD5Transform(uint32_t [4], uint8_t const [MD5_BLOCK_LENGTH])
/*		__attribute__((__bounded__(__minbytes__,1,4)))*/
/*		__attribute__((__bounded__(__minbytes__,2,MD5_BLOCK_LENGTH)))*/;
/* __END_DECLS */

#else  /* WITH_OPENSSL_HASH */

USES_APPLE_DEPRECATED_API
#define FR_MD5_CTX	MD5_CTX
#define fr_MD5Init	MD5_Init
#define fr_MD5Update	MD5_Update
#define fr_MD5Final	MD5_Final
#define fr_MD5Transform MD5_Transform
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FR_MD5_H */
