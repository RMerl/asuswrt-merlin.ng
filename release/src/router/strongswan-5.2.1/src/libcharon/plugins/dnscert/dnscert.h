/*
 * Copyright (C) 2013 Ruslan Marchenko
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

/**
 * @defgroup dnscert_i dnscert
 * @{ @ingroup dnscert
 */

#ifndef DNSCERT_H_
#define DNSCERT_H_

typedef struct dnscert_t dnscert_t;
typedef enum dnscert_algorithm_t dnscert_algorithm_t;
typedef enum dnscert_type_t dnscert_type_t;

#include <library.h>

/**
 * DNS CERT types as defined in RFC 4398.
 */
enum dnscert_type_t {
	/** Reserved value */
	DNSCERT_TYPE_RESERVED = 0,
	/** An x509 PKIX certificate */
	DNSCERT_TYPE_PKIX = 1,
	/** A SKPI certificate */
	DNSCERT_TYPE_SKPI = 2,
	/** A PGP certificate */
	DNSCERT_TYPE_PGP = 3,
	/** An x509 PKIX cert URL */
	DNSCERT_TYPE_IPKIX = 4,
	/** A SKPI cert URL */
	DNSCERT_TYPE_ISKPI = 5,
	/** A PGP cert fingerprint and URL */
	DNSCERT_TYPE_IPGP = 6,
	/** An attribute Certificate */
	DNSCERT_TYPE_ACPKIX = 7,
	/** An attribute cert URL */
	DNSCERT_TYPE_IACKPIX = 8
};

/**
 * DNSCERT algorithms as defined in http://www.iana.org/assignments/dns-sec-alg-numbers/dns-sec-alg-numbers.xhtml#dns-sec-alg-numbers-1
 */
enum dnscert_algorithm_t {
	/** No defined */
	DNSCERT_ALGORITHM_UNDEFINED = 0,
	/** RSA/MD5 */
	DNSCERT_ALGORITHM_RSAMD5 = 1,
	/** Diffie-Hellman */
	DNSCERT_ALGORITHM_DH = 2,
	/** DSA/SHA1 */
	DNSCERT_ALGORITHM_DSASHA = 3,
	/** Reserved */
	DNSCERT_ALGORITHM_RSRVD4 = 4,
	/** RSA/SHA1 */
	DNSCERT_ALGORITHM_RSASHA = 5,
	/** DSA/NSEC3/SHA */
	DNSCERT_ALGORITHM_DSANSEC3 = 6,
	/** RSA/NSEC3/SHA */
	DNSCERT_ALGORITHM_RSANSEC3 = 7,
	/** RSA/SHA256 */
	DNSCERT_ALGORITHM_RSASHA256 = 8,
	/** Reserved */
	DNSCERT_ALGORITHM_RSRVD9 = 9,
	/** RSA/SHA512 */
	DNSCERT_ALGORITHM_RSASHA512 = 10,
};

/**
 * DNS CERT RR as defined in RFC 4398.
 *
 * The CERT resource record (RR) has the structure given below.  Its RR
 * type code is 37.
 *
 *                      1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             type              |             key tag           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   algorithm   |                                               /
 * +---------------+            certificate or CRL                 /
 * /                                                               /
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
 */
struct dnscert_t {

	/**
	 * Get the type of the certificate body.
	 *
	 * The certificate "type" determines the format of the body
	 * of the CERT data.
	 *
	 * @return			certificate type
	 */
	dnscert_type_t (*get_cert_type)(dnscert_t *this);

	/**
	 * Get the tag of the key part of the CERT.
	 *
	 * @return			keytag
	 */
	u_int16_t (*get_key_tag)(dnscert_t *this);

	/**
	 * Get the algorithm.
	 *
	 * The "algorithm" determines the format of the public key field
	 * of the DNS CERT.
	 *
	 * @return			algorithm
	 */
	dnscert_algorithm_t (*get_algorithm)(dnscert_t *this);

	/**
	 * Get the content of the certificate field as chunk.
	 *
	 * The format of the certificate depends on the type.
	 *
	 * The data pointed by the chunk is still owned by the DNSCERT.
	 * Clone it if necessary.
	 *
	 * @return			certificate field as chunk
	 */
	chunk_t (*get_certificate)(dnscert_t *this);

	/**
	 * Destroy the DNSCERT.
	 */
	void (*destroy) (dnscert_t *this);
};

/**
 * Create a dnscert instance out of a resource record.
 *
 * @param	rr		resource record which contains a DNSCERT
 * @return			dnscert, NULL on failure
 */
dnscert_t *dnscert_create_frm_rr(rr_t *rr);

#endif /** DNSCERT_H_ @}*/
