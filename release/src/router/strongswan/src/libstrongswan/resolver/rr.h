/*
 * Copyright (C) 2012 Reto Guadagnini
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup rr rr
 * @{ @ingroup resolver
 */

#ifndef RR_H_
#define RR_H_

typedef struct rr_t rr_t;
typedef enum rr_type_t rr_type_t;
typedef enum rr_class_t rr_class_t;

#include <library.h>

/**
 * Resource Record types.
 *
 * According to www.iana.org/assignments/dns-parameters (version 2012-03-13).
 */
enum rr_type_t {
	/** a host address */
	RR_TYPE_A = 1,
	/** an authoritative name server */
	RR_TYPE_NS = 2,
	//** a mail destination (OBSOLETE - use MX */
	RR_TYPE_MD = 3,
	/** a mail forwarder (OBSOLETE - use MX) */
	RR_TYPE_MF = 4,
	/** the canonical name for an alias */
	RR_TYPE_CNAME = 5,
	/** marks the start of a zone of authority */
	RR_TYPE_SOA = 6,
	/** a mailbox domain name (EXPERIMENTAL) */
	RR_TYPE_MB = 7,
	/** a mail group member (EXPERIMENTAL) */
	RR_TYPE_MG = 8,
	/** a mail rename domain name (EXPERIMENTAL) */
	RR_TYPE_MR = 9,
	/** a null RR (EXPERIMENTAL) */
	RR_TYPE_NULL = 10,
	/** a well known service description */
	RR_TYPE_WKS = 11,
	/**  a domain name pointer */
	RR_TYPE_PTR = 12,
	/**  host information */
	RR_TYPE_HINFO = 13,
	/**  mailbox or mail list information */
	RR_TYPE_MINFO = 14,
	/**  mail exchange */
	RR_TYPE_MX = 15,
	/**  text strings */
	RR_TYPE_TXT = 16,
	/** for Responsible Person */
	RR_TYPE_RP = 17,
	/** for AFS Data Base location */
	RR_TYPE_AFSDB = 18,
	/** for X.25 PSDN address */
	RR_TYPE_X25 = 19,
	/** for ISDN address */
	RR_TYPE_ISDN = 20,
	/** for Route Through */
	RR_TYPE_RT = 21,
	/** for NSAP address, NSAP style A record */
	RR_TYPE_NSAP = 22,
	/** for domain name pointer, NSAP style  */
	RR_TYPE_NSAP_PTR = 23,
	/** for security signature */
	RR_TYPE_SIG = 24,
	/** for security key */
	RR_TYPE_KEY = 25,
	/** X.400 mail mapping information  */
	RR_TYPE_PX = 26,
	/**  Geographical Position  */
	RR_TYPE_GPOS = 27,
	/** ipv6 address */
	RR_TYPE_AAAA = 28,
	/** Location Information  */
	RR_TYPE_LOC = 29,
	/** Next Domain (OBSOLETE) */
	RR_TYPE_NXT = 30,
	/** Endpoint Identifier  */
	RR_TYPE_EID = 31,
	/** Nimrod Locator */
	RR_TYPE_NIMLOC = 32,
	/** Server Selection */
	RR_TYPE_SRV = 33,
	/** ATM Address */
	RR_TYPE_ATMA = 34,
	/** Naming Authority Pointer */
	RR_TYPE_NAPTR = 35,
	/** Key Exchanger */
	RR_TYPE_KX = 36,
	/** CERT */
	RR_TYPE_CERT = 37,
	/** A6 (OBSOLETE - use AAAA) */
	RR_TYPE_A6 = 38,
	/** DNAME */
	RR_TYPE_DNAME = 39,
	/** SINK */
	RR_TYPE_SINK = 40,
	/** OPT */
	RR_TYPE_OPT = 41,
	/** APL */
	RR_TYPE_APL = 42,
	/** Delegation Signer */
	RR_TYPE_DS = 43,
	/** SSH Key Fingerprint */
	RR_TYPE_SSHFP = 44,
	/** IPSECKEY */
	RR_TYPE_IPSECKEY = 45,
	/** RRSIG */
	RR_TYPE_RRSIG = 46,
	/** NSEC */
	RR_TYPE_NSEC = 47,
	/** DNSKEY */
	RR_TYPE_DNSKEY = 48,
	/** DHCID */
	RR_TYPE_DHCID = 49,
	/** NSEC3 */
	RR_TYPE_NSEC3 = 50,
	/** NSEC3PARAM */
	RR_TYPE_NSEC3PARAM = 51,

	/** Unassigned   52-54 */

	/** Host Identity Protocol */
	RR_TYPE_HIP =  55,
	/** NINFO */
	RR_TYPE_NINFO = 56,
	/** RKEY */
	RR_TYPE_RKEY = 57,
	/** Trust Anchor LINK */
	RR_TYPE_TALINK = 58,
	/** Child DS */
	RR_TYPE_CDS = 59,

	/** Unassigned   60-98 */

	/** SPF */
	RR_TYPE_SPF = 99,
	/** UINFO */
	RR_TYPE_UINFO = 100,
	/** UID */
	RR_TYPE_UID = 101,
	/** GID */
	RR_TYPE_GID = 102,
	/** UNSPEC */
	RR_TYPE_UNSPEC = 103,

	/** Unassigned   104-248 */

	/** Transaction Key */
	RR_TYPE_TKEY = 249,
	/** Transaction Signature */
	RR_TYPE_TSIG = 250,
	/** incremental transfer */
	RR_TYPE_IXFR = 251,
	/** transfer of an entire zone */
	RR_TYPE_AXFR = 252,
	/** mailbox-related RRs (MB, MG or MR) */
	RR_TYPE_MAILB = 253,
	/** mail agent RRs (OBSOLETE - see MX) */
	RR_TYPE_MAILA = 254,
	/** A request for all records */
	RR_TYPE_ANY = 255,
	/** URI */
	RR_TYPE_URI = 256,
	/** Certification Authority Authorization */
	RR_TYPE_CAA = 257,

	/** Unassigned   258-32767 */

	/** DNSSEC Trust Authorities */
	RR_TYPE_TA = 32768,
	/** DNSSEC Lookaside Validation */
	RR_TYPE_DLV = 32769,

	/** Unassigned   32770-65279 */

	/** Private use  65280-65534 */

	/** Reserved     65535 */
};


/**
 * Resource Record CLASSes
 */
enum rr_class_t {
	/** Internet */
	RR_CLASS_IN = 1,
	/** Chaos */
	RR_CLASS_CH = 3,
	/** Hesiod */
	RR_CLASS_HS = 4,
	/** further CLASSes: http://wwwiana.org/assignments/dns-parameters */
};


/**
 * A DNS Resource Record.
 *
 * Represents a Resource Record of the Domain Name System
 * as defined in RFC 1035.
 *
 */
struct rr_t {

	/**
	 * Get the NAME of the owner of this RR.
	 *
	 * @return			owner name as string
	 */
	char *(*get_name)(rr_t *this);

	/**
	 * Get the type of this RR.
	 *
	 * @return			RR type
	 */
	rr_type_t (*get_type)(rr_t *this);

	/**
	 * Get the class of this RR.
	 *
	 * @return			RR class
	 */
	rr_class_t (*get_class)(rr_t *this);

	/**
	 * Get the Time to Live (TTL) of this RR.
	 *
	 * @return			Time to Live
	 */
	uint32_t (*get_ttl)(rr_t *this);

	/**
	 * Get the content of the RDATA field as chunk.
	 *
	 * The data pointed by the chunk is still owned by the RR.
	 * Clone it if needed.
	 *
	 * @return			RDATA field as chunk
	 */
	chunk_t (*get_rdata)(rr_t *this);

	/**
	 * Destroy the Resource Record.
	 */
	void (*destroy) (rr_t *this);
};

#endif /** RR_H_ @}*/
