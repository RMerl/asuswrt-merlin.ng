/*
 * Copyright (C) 2008-2012 Tobias Brunner
 * Copyright (C) 2007-2009 Martin Willi
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
 * @defgroup auth_cfg auth_cfg
 * @{ @ingroup credentials
 */

#ifndef AUTH_CFG_H_
#define AUTH_CFG_H_

#include <collections/enumerator.h>

typedef struct auth_cfg_t auth_cfg_t;
typedef enum auth_rule_t auth_rule_t;
typedef enum auth_class_t auth_class_t;

/**
 * Class of authentication to use. This is different to auth_method_t in that
 * it does not specify a method, but a class of acceptable methods. The found
 * certificate finally dictates which method is used.
 */
enum auth_class_t {
	/** any class acceptable */
	AUTH_CLASS_ANY = 0,
	/** authentication using public keys (RSA, ECDSA) */
	AUTH_CLASS_PUBKEY = 1,
	/** authentication using a pre-shared secrets */
	AUTH_CLASS_PSK = 2,
	/** authentication using EAP */
	AUTH_CLASS_EAP = 3,
	/** authentication using IKEv1 XAUTH */
	AUTH_CLASS_XAUTH = 4,
};

/**
 * enum strings for auth_class_t
 */
extern enum_name_t *auth_class_names;

/**
 * Authentication config to use during authentication process.
 *
 * Each authentication config contains a set of rules. These rule-sets are used
 * in two ways:
 * - For configs specifying local authentication behavior, the rules define
 *   which authentication method in which way.
 * - For configs specifying remote peer authentication, the rules define
 *   constraints the peer has to fulfill.
 *
 * Additionally to the rules, there is a set of helper items. These are used
 * to transport credentials during the authentication process.
 */
enum auth_rule_t {
	/** identity to use for IKEv2 authentication exchange, identification_t* */
	AUTH_RULE_IDENTITY,
	/** if TRUE don't send IDr as initiator, but verify the identity after
	 * receiving IDr (but also verify it against subjectAltNames), bool */
	AUTH_RULE_IDENTITY_LOOSE,
	/** authentication class, auth_class_t */
	AUTH_RULE_AUTH_CLASS,
	/** AAA-backend identity for EAP methods supporting it, identification_t* */
	AUTH_RULE_AAA_IDENTITY,
	/** EAP identity to use within EAP-Identity exchange, identification_t* */
	AUTH_RULE_EAP_IDENTITY,
	/** EAP type to propose for peer authentication, eap_type_t */
	AUTH_RULE_EAP_TYPE,
	/** EAP vendor for vendor specific type, u_int32_t */
	AUTH_RULE_EAP_VENDOR,
	/** XAUTH backend name to use, char* */
	AUTH_RULE_XAUTH_BACKEND,
	/** XAuth identity to use or require, identification_t* */
	AUTH_RULE_XAUTH_IDENTITY,
	/** certificate authority, certificate_t* */
	AUTH_RULE_CA_CERT,
	/** intermediate certificate in trustchain, certificate_t* */
	AUTH_RULE_IM_CERT,
	/** subject certificate, certificate_t* */
	AUTH_RULE_SUBJECT_CERT,
	/** result of a CRL validation, cert_validation_t */
	AUTH_RULE_CRL_VALIDATION,
	/** result of a OCSP validation, cert_validation_t */
	AUTH_RULE_OCSP_VALIDATION,
	/** subject is member of a group, identification_t*
	 * The group membership constraint is fulfilled if the subject is member of
	 * one group defined in the constraints. */
	AUTH_RULE_GROUP,
	/** required RSA public key strength, u_int in bits */
	AUTH_RULE_RSA_STRENGTH,
	/** required ECDSA public key strength, u_int in bits */
	AUTH_RULE_ECDSA_STRENGTH,
	/** required signature scheme, signature_scheme_t */
	AUTH_RULE_SIGNATURE_SCHEME,
	/** certificatePolicy constraint, numerical OID as char* */
	AUTH_RULE_CERT_POLICY,

	/** intermediate certificate, certificate_t* */
	AUTH_HELPER_IM_CERT,
	/** subject certificate, certificate_t* */
	AUTH_HELPER_SUBJECT_CERT,
	/** Hash and URL of a intermediate certificate, char* */
	AUTH_HELPER_IM_HASH_URL,
	/** Hash and URL of a end-entity certificate, char* */
	AUTH_HELPER_SUBJECT_HASH_URL,
	/** revocation certificate (CRL, OCSP), certificate_t* */
	AUTH_HELPER_REVOCATION_CERT,
	/** attribute certificate for authorization decisions, certificate_t */
	AUTH_HELPER_AC_CERT,

	/** helper to determine the number of elements in this enum */
	AUTH_RULE_MAX,
};

/**
 * enum name for auth_rule_t.
 */
extern enum_name_t *auth_rule_names;

/**
 * Authentication/Authorization round.
 *
 * RFC4739 defines multiple authentication rounds. This class defines such
 * a round from a configuration perspective, either for the local or the remote
 * peer. Local configs are called "rulesets". They define how we authenticate.
 * Remote peer configs are called "constraits". They define what is needed to
 * complete the authentication round successfully.
 *
 * @verbatim

   [Repeat for each configuration]
   +--------------------------------------------------+
   |                                                  |
   |                                                  |
   |   +----------+     IKE_AUTH       +--------- +   |
   |   |  config  |   ----------->     |          |   |
   |   |  ruleset |                    |          |   |
   |   +----------+ [ <----------- ]   |          |   |
   |                [ optional EAP ]   |   Peer   |   |
   |   +----------+ [ -----------> ]   |          |   |
   |   |  config  |                    |          |   |
   |   |  constr. |   <-----------     |          |   |
   |   +----------+     IKE_AUTH       +--------- +   |
   |                                                  |
   |                                                  |
   +--------------------------------------------------+

   @endverbatim
 *
 * Values for each item are either pointers (casted to void*) or short
 * integers (use uintptr_t cast).
 */
struct auth_cfg_t {

	/**
	 * Add a rule to the set.
	 *
	 * Rules we expect only once (e.g. identities) implicitly replace previous
	 * rules of the same type (but pointers to previous values will remain
	 * valid until the auth_cfg_t object is destroyed).
	 * Rules that may occur multiple times (e.g. CA certificates) are inserted
	 * so that they can be enumerated in the order in which they were added.
	 * For these get() will return the value added first.
	 *
	 * @param rule		rule type
	 * @param ...		associated value to rule
	 */
	void (*add)(auth_cfg_t *this, auth_rule_t rule, ...);

	/**
	 * Get a rule value.
	 *
	 * For rules we expect only once the latest value is returned.
	 *
	 * @param rule		rule type
	 * @return			rule or NULL (or an appropriate default) if not found
	 */
	void* (*get)(auth_cfg_t *this, auth_rule_t rule);

	/**
	 * Create an enumerator over added rules.
	 *
	 * Refer to add() regarding the order in which rules are enumerated.
	 * For rules we expect only once the latest value is enumerated only.
	 *
	 * @return			enumerator over (auth_rule_t, union{void*,uintpr_t})
	 */
	enumerator_t* (*create_enumerator)(auth_cfg_t *this);

	/**
	 * Replace a rule at enumerator position.
	 *
	 * @param pos		enumerator position
	 * @param rule		rule type
	 * @param ...		associated value to rule
	 */
	void (*replace)(auth_cfg_t *this, enumerator_t *pos,
					auth_rule_t rule, ...);

	/**
	 * Check if a used config fulfills a set of configured constraints.
	 *
	 * @param constraints	required authorization rules
	 * @param log_error		whether to log compliance errors
	 * @return				TRUE if this complies with constraints
	 */
	bool (*complies)(auth_cfg_t *this, auth_cfg_t *constraints, bool log_error);

	/**
	 * Merge items from other into this.
	 *
	 * @param other		items to read for merge
	 * @param copy		TRUE to copy items, FALSE to move them
	 */
	void (*merge)(auth_cfg_t *this, auth_cfg_t *other, bool copy);

	/**
	 * Purge all rules in a config.
	 *
	 * @param keep_ca	whether to keep AUTH_RULE_CA_CERT entries
	 */
	void (*purge)(auth_cfg_t *this, bool keep_ca);

	/**
	 * Check two configs for equality.
	 *
	 * For rules we expect only once the latest value is compared only.
	 *
	 * @param other		other config to compare against this
	 * @return			TRUE if auth infos identical
	 */
	bool (*equals)(auth_cfg_t *this, auth_cfg_t *other);

	/**
	 * Clone an authentication config, including all rules.
	 *
	 * @return			cloned configuration
	 */
	auth_cfg_t* (*clone)(auth_cfg_t *this);

	/**
	 * Destroy a config with all associated rules/values.
	 */
	void (*destroy)(auth_cfg_t *this);
};

/**
 * Create a authentication config.
 */
auth_cfg_t *auth_cfg_create();

#endif /** AUTH_CFG_H_ @}*/
