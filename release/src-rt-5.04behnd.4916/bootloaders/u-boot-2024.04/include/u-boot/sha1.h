/* SPDX-License-Identifier: LGPL-2.1 */
/**
 * \file sha1.h
 * based from http://xyssl.org/code/source/sha1/
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  Copyright (C) 2003-2006  Christophe Devine
 */
/*
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */
#ifndef _SHA1_H
#define _SHA1_H

#ifdef __cplusplus
extern "C" {
#endif

#define SHA1_SUM_POS	-0x20
#define SHA1_SUM_LEN	20
#define SHA1_DER_LEN	15

extern const uint8_t sha1_der_prefix[];

/**
 * \brief	   SHA-1 context structure
 */
typedef struct
{
    unsigned long total[2];	/*!< number of bytes processed	*/
    uint32_t state[5];		/*!< intermediate digest state	*/
    unsigned char buffer[64];	/*!< data block being processed */
}
sha1_context;

/**
 * \brief	   SHA-1 context setup
 *
 * \param ctx	   SHA-1 context to be initialized
 */
void sha1_starts( sha1_context *ctx );

/**
 * \brief	   SHA-1 process buffer
 *
 * \param ctx	   SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen	   length of the input data
 */
void sha1_update(sha1_context *ctx, const unsigned char *input,
		 unsigned int ilen);

/**
 * \brief	   SHA-1 final digest
 *
 * \param ctx	   SHA-1 context
 * \param output   SHA-1 checksum result
 */
void sha1_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * \brief	   Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen	   length of the input data
 * \param output   SHA-1 checksum result
 */
void sha1_csum(const unsigned char *input, unsigned int ilen,
		unsigned char *output);

/**
 * \brief	   Output = SHA-1( input buffer ), with watchdog triggering
 *
 * \param input    buffer holding the  data
 * \param ilen	   length of the input data
 * \param output   SHA-1 checksum result
 * \param chunk_sz watchdog triggering period (in bytes of input processed)
 */
void sha1_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);

/**
 * \brief	   Output = HMAC-SHA-1( input buffer, hmac key )
 *
 * \param key	   HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen	   length of the input data
 * \param output   HMAC-SHA-1 result
 */
void sha1_hmac(const unsigned char *key, int keylen,
		const unsigned char *input, unsigned int ilen,
		unsigned char *output);

/**
 * \brief	   Checkup routine
 *
 * \return	   0 if successful, or 1 if the test failed
 */
int sha1_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* sha1.h */
