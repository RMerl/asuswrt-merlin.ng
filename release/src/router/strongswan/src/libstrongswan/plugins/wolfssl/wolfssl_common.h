/*
 * Copyright (C) 2020 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef WOLFSSL_PLUGIN_COMMON_H_
#define WOLFSSL_PLUGIN_COMMON_H_

#include <library.h>

/* Undefine these as they are enum entries in wolfSSL - same values */
#ifdef AES_BLOCK_SIZE
#undef AES_BLOCK_SIZE
#endif

#ifdef CAMELLIA_BLOCK_SIZE
#undef CAMELLIA_BLOCK_SIZE
#endif

#ifdef DES_BLOCK_SIZE
#undef DES_BLOCK_SIZE
#endif

#ifdef RSA_PSS_SALT_LEN_DEFAULT
#undef RSA_PSS_SALT_LEN_DEFAULT
#endif

/* PARSE_ERROR is an enum entry in wolfSSL - not used in this plugin */
#define PARSE_ERROR	WOLFSSL_PARSE_ERROR

/* Remap unused enums from the OpenSSL compatibility layer to avoid conflicts */
#define ASN1_BOOLEAN         REMAP_ASN1_BOOLEAN
#define ASN1_OID             REMAP_ASN1_OID
#define ASN1_INTEGER         REMAP_ASN1_INTEGER
#define ASN1_BIT_STRING      REMAP_ASN1_BIT_STRING
#define ASN1_IA5STRING       REMAP_ASN1_IA5STRING
#define ASN1_OCTET_STRING    REMAP_ASN1_OCTET_STRING
#define ASN1_UTCTIME         REMAP_ASN1_UTCTIME
#define ASN1_GENERALIZEDTIME REMAP_ASN1_GENERALIZEDTIME

#ifndef WOLFSSL_USER_SETTINGS
	#include <wolfssl/options.h>
#endif
#include <wolfssl/ssl.h>

/* Special type used to handle EdDSA keys depending on config options */
#if defined(HAVE_ED25519) || defined(HAVE_ED448)
#ifdef HAVE_ED25519
#include <wolfssl/wolfcrypt/ed25519.h>
#endif
#ifdef HAVE_ED448
#include <wolfssl/wolfcrypt/ed448.h>
#endif
typedef union {
#ifdef HAVE_ED25519
	ed25519_key ed25519;
#endif
#ifdef HAVE_ED448
	ed448_key ed448;
#endif
} wolfssl_ed_key;
#endif /* HAVE_ED25519 || HAVE_ED448 */

#undef PARSE_ERROR

#undef ASN1_BOOLEAN
#undef ASN1_OID
#undef ASN1_INTEGER
#undef ASN1_BIT_STRING
#undef ASN1_IA5STRING
#undef ASN1_OCTET_STRING
#undef ASN1_UTCTIME
#undef ASN1_GENERALIZEDTIME

/* Eliminate macro conflicts */
#undef RNG

#endif /* WOLFSSL_PLUGIN_COMMON_H_ */
