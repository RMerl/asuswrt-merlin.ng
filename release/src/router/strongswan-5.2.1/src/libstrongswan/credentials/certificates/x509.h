/*
 * Copyright (C) 2007-2008 Martin Willi
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
 * @defgroup x509 x509
 * @{ @ingroup certificates
 */

#ifndef X509_H_
#define X509_H_

#include <collections/enumerator.h>
#include <credentials/certificates/certificate.h>

/* constraints are currently restricted to the range 0..127 */
#define X509_NO_CONSTRAINT	255

typedef struct x509_t x509_t;
typedef struct x509_cert_policy_t x509_cert_policy_t;
typedef struct x509_policy_mapping_t x509_policy_mapping_t;
typedef struct x509_cdp_t x509_cdp_t;
typedef enum x509_flag_t x509_flag_t;
typedef enum x509_constraint_t x509_constraint_t;

/**
 * X.509 certificate flags.
 */
enum x509_flag_t {
	/** cert has no constraints */
	X509_NONE =	               0,
	/** cert has CA constraint */
	X509_CA =                 (1<<0),
	/** cert has AA constraint */
	X509_AA =                 (1<<1),
	/** cert has OCSP signer constraint */
	X509_OCSP_SIGNER =        (1<<2),
	/** cert has serverAuth key usage */
	X509_SERVER_AUTH =        (1<<3),
	/** cert has clientAuth key usage */
	X509_CLIENT_AUTH =        (1<<4),
	/** cert is self-signed */
	X509_SELF_SIGNED =        (1<<5),
	/** cert has an ipAddrBlocks extension */
	X509_IP_ADDR_BLOCKS =     (1<<6),
	/** cert has CRL sign key usage */
	X509_CRL_SIGN =           (1<<7),
	/** cert has iKEIntermediate key usage */
	X509_IKE_INTERMEDIATE =   (1<<8),
	/** cert has Microsoft Smartcard Logon usage */
	X509_MS_SMARTCARD_LOGON = (1<<9),
};

/**
 * Different numerical X.509 constraints.
 */
enum x509_constraint_t {
	/** pathLenConstraint basicConstraints */
	X509_PATH_LEN,
	/** inhibitPolicyMapping policyConstraint */
	X509_INHIBIT_POLICY_MAPPING,
	/** requireExplicitPolicy policyConstraint */
	X509_REQUIRE_EXPLICIT_POLICY,
	/** inhibitAnyPolicy constraint */
	X509_INHIBIT_ANY_POLICY,
};

/**
 * X.509 certPolicy extension.
 */
struct x509_cert_policy_t {
	/** Certification Practice Statement URI qualifier */
	char *cps_uri;
	/** UserNotice Text qualifier */
	char *unotice_text;
	/** OID of certPolicy */
	chunk_t oid;
};

/**
 * X.509 policyMapping extension
 */
struct x509_policy_mapping_t {
	/** OID of issuerDomainPolicy */
	chunk_t issuer;
	/** OID of subjectDomainPolicy */
	chunk_t subject;
};

/**
 * X.509 CRL distributionPoint
 */
struct x509_cdp_t {
	/** CDP URI, as string */
	char *uri;
	/** CRL issuer */
	identification_t *issuer;
};

/**
 * X.509 certificate interface.
 *
 * This interface adds additional methods to the certificate_t type to
 * allow further operations on these certificates.
 */
struct x509_t {

	/**
	 * Implements certificate_t.
	 */
	certificate_t interface;

	/**
	 * Get the flags set for this certificate.
	 *
	 * @return			set of flags
	 */
	x509_flag_t (*get_flags)(x509_t *this);

	/**
	 * Get the certificate serial number.
	 *
	 * @return			chunk pointing to internal serial number
	 */
	chunk_t (*get_serial)(x509_t *this);

	/**
	 * Get the the subjectKeyIdentifier.
	 *
	 * @return			subjectKeyIdentifier as chunk_t, internal data
	 */
	chunk_t (*get_subjectKeyIdentifier)(x509_t *this);

	/**
	 * Get the the authorityKeyIdentifier.
	 *
	 * @return			authKeyIdentifier as chunk_t, internal data
	 */
	chunk_t (*get_authKeyIdentifier)(x509_t *this);

	/**
	 * Get a numerical X.509 constraint.
	 *
	 * @param type		type of constraint to get
	 * @return			constraint, X509_NO_CONSTRAINT if none found
	 */
	u_int (*get_constraint)(x509_t *this, x509_constraint_t type);

	/**
	 * Create an enumerator over all subjectAltNames.
	 *
	 * @return			enumerator over subjectAltNames as identification_t*
	 */
	enumerator_t* (*create_subjectAltName_enumerator)(x509_t *this);

	/**
	 * Create an enumerator over all CRL URIs and CRL Issuers.
	 *
	 * @return			enumerator over x509_cdp_t
	 */
	enumerator_t* (*create_crl_uri_enumerator)(x509_t *this);

	/**
	 * Create an enumerator over all OCSP URIs.
	 *
	 * @return			enumerator over URIs as char*
	 */
	enumerator_t* (*create_ocsp_uri_enumerator)(x509_t *this);

	/**
	 * Create an enumerator over all ipAddrBlocks.
	 *
	 * @return			enumerator over ipAddrBlocks as traffic_selector_t*
	 */
	enumerator_t* (*create_ipAddrBlock_enumerator)(x509_t *this);

	/**
	 * Create an enumerator over name constraints.
	 *
	 * @param perm		TRUE for permitted, FALSE for excluded subtrees
	 * @return			enumerator over subtrees as identification_t
	 */
	enumerator_t* (*create_name_constraint_enumerator)(x509_t *this, bool perm);

	/**
	 * Create an enumerator over certificate policies.
	 *
	 * @return			enumerator over x509_cert_policy_t
	 */
	enumerator_t* (*create_cert_policy_enumerator)(x509_t *this);

	/**
	 * Create an enumerator over policy mappings.
	 *
	 * @return			enumerator over x509_policy_mapping
	 */
	enumerator_t* (*create_policy_mapping_enumerator)(x509_t *this);


};

#endif /** X509_H_ @}*/
