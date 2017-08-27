/*
 * Copyright (C) 2012 Reto Guadagnini
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup ipseckey_i ipseckey
 * @{ @ingroup ipseckey
 */

#ifndef IPSECKEY_H_
#define IPSECKEY_H_

typedef struct ipseckey_t ipseckey_t;
typedef enum ipseckey_algorithm_t ipseckey_algorithm_t;
typedef enum ipseckey_gw_type_t ipseckey_gw_type_t;

#include <library.h>

/**
 * IPSECKEY gateway types as defined in RFC 4025.
 */
enum ipseckey_gw_type_t {
	/** No gateway is present */
	IPSECKEY_GW_TP_NOT_PRESENT = 0,
	/** A 4-byte IPv4 address is present */
	IPSECKEY_GW_TP_IPV4 = 1,
	/** A 16-byte IPv6 address is present */
	IPSECKEY_GW_TP_IPV6 = 2,
	/** A wire-encoded domain name is present */
	IPSECKEY_GW_TP_WR_ENC_DNAME = 3,
};

/**
 * IPSECKEY algorithms as defined in RFC 4025.
 */
enum ipseckey_algorithm_t {
	/** No key present */
	IPSECKEY_ALGORITHM_NONE = 0,
	/** DSA key */
	IPSECKEY_ALGORITHM_DSA = 1,
	/** RSA key */
	IPSECKEY_ALGORITHM_RSA = 2,
};

/**
 * An IPSECKEY.
 *
 * Represents an IPSECKEY as defined in RFC 4025:
 *
 *      0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |  precedence   | gateway type  |  algorithm  |     gateway     |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------------+                 +
 *   ~                            gateway                            ~
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               /
 *   /                          public key                           /
 *   /                                                               /
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
 *
 *
 * Note: RFC 4025 defines that the algorithm field has a length of 7 bits.
 * 		 We use 8 bits instead, because the use of 7 bits is very uncommon
 * 		 in internet protocols and might be an error in RFC 4025
 * 		 (also the BIND DNS server uses 8 bits for the algorithm field of the
 * 		 IPSECKEY resource records).
 *
 */
struct ipseckey_t {

	/**
	 * Get the precedence of the IPSECKEY.
	 *
	 * @return		precedence
	 */
	u_int8_t (*get_precedence)(ipseckey_t *this);

	/**
	 * Get the type of the gateway.
	 *
	 * The "gateway type" determines the format of the gateway field
	 * of the IPSECKEY.
	 *
	 * @return		gateway type
	 */
	ipseckey_gw_type_t (*get_gateway_type)(ipseckey_t *this);

	/**
	 * Get the algorithm.
	 *
	 * The "algorithm" determines the format of the public key field
	 * of the IPSECKEY.
	 *
	 * @return			algorithm
	 */
	ipseckey_algorithm_t (*get_algorithm)(ipseckey_t *this);

	/**
	 * Get the content of the gateway field as chunk.
	 *
	 * The content is in network byte order and its format depends on the
	 * gateway type.
	 *
	 * The data pointed by the chunk is still owned by the IPSECKEY.
	 * Clone it if necessary.
	 *
	 * @return			gateway field as chunk
	 */
	chunk_t (*get_gateway)(ipseckey_t *this);

	/**
	 * Get the content of the public key field as chunk.
	 *
	 * The format of the public key depends on the algorithm type.
	 *
	 * The data pointed by the chunk is still owned by the IPSECKEY.
	 * Clone it if necessary.
	 *
	 * @return			public key field as chunk
	 */
	chunk_t (*get_public_key)(ipseckey_t *this);

	/**
	 * Destroy the IPSECKEY.
	 */
	void (*destroy) (ipseckey_t *this);
};

/**
 * Create an ipseckey instance out of a resource record.
 *
 * @param	rr		resource record which contains an IPSECKEY
 * @return			ipseckey, NULL on failure
 */
ipseckey_t *ipseckey_create_frm_rr(rr_t *rr);

#endif /** IPSECKEY_H_ @}*/
