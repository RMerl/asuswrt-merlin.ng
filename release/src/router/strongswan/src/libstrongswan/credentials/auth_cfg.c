/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2007-2009 Martin Willi
 * Copyright (C) 2016 Andreas Steffen
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

#include "auth_cfg.h"

#include <library.h>
#include <utils/debug.h>
#include <collections/array.h>
#include <utils/identification.h>
#include <eap/eap.h>
#include <credentials/certificates/certificate.h>

ENUM(auth_class_names, AUTH_CLASS_ANY, AUTH_CLASS_XAUTH,
	"any",
	"public key",
	"pre-shared key",
	"EAP",
	"XAuth",
);

ENUM(auth_rule_names, AUTH_RULE_IDENTITY, AUTH_HELPER_AC_CERT,
	"RULE_IDENTITY",
	"RULE_IDENTITY_LOOSE",
	"RULE_AUTH_CLASS",
	"RULE_AAA_IDENTITY",
	"RULE_EAP_IDENTITY",
	"RULE_EAP_TYPE",
	"RULE_EAP_VENDOR",
	"RULE_XAUTH_BACKEND",
	"RULE_XAUTH_IDENTITY",
	"RULE_CA_CERT",
	"RULE_IM_CERT",
	"RULE_SUBJECT_CERT",
	"RULE_CRL_VALIDATION",
	"RULE_OCSP_VALIDATION",
	"RULE_CERT_VALIDATION_SUSPENDED",
	"RULE_GROUP",
	"RULE_RSA_STRENGTH",
	"RULE_ECDSA_STRENGTH",
	"RULE_BLISS_STRENGTH",
	"RULE_SIGNATURE_SCHEME",
	"RULE_IKE_SIGNATURE_SCHEME",
	"RULE_CERT_POLICY",
	"HELPER_IM_CERT",
	"HELPER_SUBJECT_CERT",
	"HELPER_IM_HASH_URL",
	"HELPER_SUBJECT_HASH_URL",
	"HELPER_REVOCATION_CERT",
	"HELPER_AC_CERT",
);

/**
 * Check if the given rule is a rule for which there may be multiple values.
 */
static inline bool is_multi_value_rule(auth_rule_t type)
{
	switch (type)
	{
		case AUTH_RULE_AUTH_CLASS:
		case AUTH_RULE_EAP_TYPE:
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_XAUTH_IDENTITY:
		case AUTH_RULE_XAUTH_BACKEND:
		case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
		case AUTH_HELPER_SUBJECT_CERT:
		case AUTH_HELPER_SUBJECT_HASH_URL:
		case AUTH_RULE_MAX:
			return FALSE;
		case AUTH_RULE_OCSP_VALIDATION:
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_GROUP:
		case AUTH_RULE_SUBJECT_CERT:
		case AUTH_RULE_CA_CERT:
		case AUTH_RULE_IM_CERT:
		case AUTH_RULE_CERT_POLICY:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_BLISS_STRENGTH:
		case AUTH_RULE_SIGNATURE_SCHEME:
		case AUTH_RULE_IKE_SIGNATURE_SCHEME:
		case AUTH_HELPER_IM_CERT:
		case AUTH_HELPER_IM_HASH_URL:
		case AUTH_HELPER_REVOCATION_CERT:
		case AUTH_HELPER_AC_CERT:
			return TRUE;
	}
	return FALSE;
}

typedef struct private_auth_cfg_t private_auth_cfg_t;

/**
 * private data of item_set
 */
struct private_auth_cfg_t {

	/**
	 * public functions
	 */
	auth_cfg_t public;

	/**
	 * Array of entry_t
	 */
	array_t *entries;
};

typedef struct entry_t entry_t;

struct entry_t {
	/** rule type */
	auth_rule_t type;
	/** associated value */
	void *value;
};

/**
 * enumerator for auth_cfg_t.create_enumerator()
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** inner enumerator from linked_list_t */
	enumerator_t *inner;
	/** current entry */
	entry_t *current;
	/** types we have already enumerated */
	bool enumerated[AUTH_RULE_MAX];
} entry_enumerator_t;

METHOD(enumerator_t, enumerate, bool,
	entry_enumerator_t *this, va_list args)
{
	auth_rule_t *type;
	entry_t *entry;
	void **value;

	VA_ARGS_VGET(args, type, value);

	while (this->inner->enumerate(this->inner, &entry))
	{
		if (!is_multi_value_rule(entry->type) && this->enumerated[entry->type])
		{
			continue;
		}
		this->enumerated[entry->type] = TRUE;
		this->current = entry;
		if (type)
		{
			*type = entry->type;
		}
		if (value)
		{
			*value = entry->value;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, entry_enumerator_destroy, void,
	entry_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(auth_cfg_t, create_enumerator, enumerator_t*,
	private_auth_cfg_t *this)
{
	entry_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _entry_enumerator_destroy,
		},
		.inner = array_create_enumerator(this->entries),
	);
	return &enumerator->public;
}

/**
 * Initialize an entry.
 */
static void init_entry(entry_t *this, auth_rule_t type, va_list args)
{
	this->type = type;
	switch (type)
	{
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_AUTH_CLASS:
		case AUTH_RULE_EAP_TYPE:
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_BLISS_STRENGTH:
		case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
			/* integer type */
			this->value = (void*)(uintptr_t)va_arg(args, u_int);
			break;
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_XAUTH_BACKEND:
		case AUTH_RULE_XAUTH_IDENTITY:
		case AUTH_RULE_GROUP:
		case AUTH_RULE_CA_CERT:
		case AUTH_RULE_IM_CERT:
		case AUTH_RULE_SUBJECT_CERT:
		case AUTH_RULE_CERT_POLICY:
		case AUTH_RULE_SIGNATURE_SCHEME:
		case AUTH_RULE_IKE_SIGNATURE_SCHEME:
		case AUTH_HELPER_IM_CERT:
		case AUTH_HELPER_SUBJECT_CERT:
		case AUTH_HELPER_IM_HASH_URL:
		case AUTH_HELPER_SUBJECT_HASH_URL:
		case AUTH_HELPER_REVOCATION_CERT:
		case AUTH_HELPER_AC_CERT:
			/* pointer type */
			this->value = va_arg(args, void*);
			break;
		case AUTH_RULE_MAX:
			this->value = NULL;
			break;
	}
}

/**
 * Compare two entries for equality.
 */
static bool entry_equals(entry_t *e1, entry_t *e2)
{
	if (e1->type != e2->type)
	{
		return FALSE;
	}
	switch (e1->type)
	{
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_AUTH_CLASS:
		case AUTH_RULE_EAP_TYPE:
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_BLISS_STRENGTH:
		case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
		{
			return e1->value == e2->value;
		}
		case AUTH_RULE_CA_CERT:
		case AUTH_RULE_IM_CERT:
		case AUTH_RULE_SUBJECT_CERT:
		case AUTH_HELPER_IM_CERT:
		case AUTH_HELPER_SUBJECT_CERT:
		case AUTH_HELPER_REVOCATION_CERT:
		case AUTH_HELPER_AC_CERT:
		{
			certificate_t *c1, *c2;

			c1 = (certificate_t*)e1->value;
			c2 = (certificate_t*)e2->value;

			return c1->equals(c1, c2);
		}
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_XAUTH_IDENTITY:
		case AUTH_RULE_GROUP:
		{
			identification_t *id1, *id2;

			id1 = (identification_t*)e1->value;
			id2 = (identification_t*)e2->value;

			return id1->equals(id1, id2);
		}
		case AUTH_RULE_SIGNATURE_SCHEME:
		case AUTH_RULE_IKE_SIGNATURE_SCHEME:
		{
			return signature_params_equal(e1->value, e2->value);
		}
		case AUTH_RULE_CERT_POLICY:
		case AUTH_RULE_XAUTH_BACKEND:
		case AUTH_HELPER_IM_HASH_URL:
		case AUTH_HELPER_SUBJECT_HASH_URL:
		{
			return streq(e1->value, e2->value);
		}
		case AUTH_RULE_MAX:
			break;
	}
	return FALSE;
}

/**
 * Destroy the value associated with an entry
 */
static void destroy_entry_value(entry_t *entry)
{
	switch (entry->type)
	{
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_GROUP:
		case AUTH_RULE_XAUTH_IDENTITY:
		{
			identification_t *id = (identification_t*)entry->value;
			id->destroy(id);
			break;
		}
		case AUTH_RULE_CA_CERT:
		case AUTH_RULE_IM_CERT:
		case AUTH_RULE_SUBJECT_CERT:
		case AUTH_HELPER_IM_CERT:
		case AUTH_HELPER_SUBJECT_CERT:
		case AUTH_HELPER_REVOCATION_CERT:
		case AUTH_HELPER_AC_CERT:
		{
			certificate_t *cert = (certificate_t*)entry->value;
			cert->destroy(cert);
			break;
		}
		case AUTH_RULE_CERT_POLICY:
		case AUTH_RULE_XAUTH_BACKEND:
		case AUTH_HELPER_IM_HASH_URL:
		case AUTH_HELPER_SUBJECT_HASH_URL:
		{
			free(entry->value);
			break;
		}
		case AUTH_RULE_SIGNATURE_SCHEME:
		case AUTH_RULE_IKE_SIGNATURE_SCHEME:
		{
			signature_params_destroy(entry->value);
			break;
		}
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_AUTH_CLASS:
		case AUTH_RULE_EAP_TYPE:
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_BLISS_STRENGTH:
		case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
		case AUTH_RULE_MAX:
			break;
	}
}

/**
 * Implementation of auth_cfg_t.replace.
 */
static void replace(private_auth_cfg_t *this, entry_enumerator_t *enumerator,
					auth_rule_t type, ...)
{
	if (enumerator->current)
	{
		entry_t *entry;
		va_list args;

		va_start(args, type);
		entry = enumerator->current;
		destroy_entry_value(entry);
		entry->type = type;
		switch (type)
		{
			case AUTH_RULE_IDENTITY_LOOSE:
			case AUTH_RULE_AUTH_CLASS:
			case AUTH_RULE_EAP_TYPE:
			case AUTH_RULE_EAP_VENDOR:
			case AUTH_RULE_CRL_VALIDATION:
			case AUTH_RULE_OCSP_VALIDATION:
			case AUTH_RULE_RSA_STRENGTH:
			case AUTH_RULE_ECDSA_STRENGTH:
			case AUTH_RULE_BLISS_STRENGTH:
			case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
				/* integer type */
				entry->value = (void*)(uintptr_t)va_arg(args, u_int);
				break;
			case AUTH_RULE_IDENTITY:
			case AUTH_RULE_EAP_IDENTITY:
			case AUTH_RULE_AAA_IDENTITY:
			case AUTH_RULE_XAUTH_BACKEND:
			case AUTH_RULE_XAUTH_IDENTITY:
			case AUTH_RULE_GROUP:
			case AUTH_RULE_CA_CERT:
			case AUTH_RULE_IM_CERT:
			case AUTH_RULE_SUBJECT_CERT:
			case AUTH_RULE_CERT_POLICY:
			case AUTH_RULE_SIGNATURE_SCHEME:
			case AUTH_RULE_IKE_SIGNATURE_SCHEME:
			case AUTH_HELPER_IM_CERT:
			case AUTH_HELPER_SUBJECT_CERT:
			case AUTH_HELPER_IM_HASH_URL:
			case AUTH_HELPER_SUBJECT_HASH_URL:
			case AUTH_HELPER_REVOCATION_CERT:
			case AUTH_HELPER_AC_CERT:
				/* pointer type */
				entry->value = va_arg(args, void*);
				break;
			case AUTH_RULE_MAX:
				entry->value = NULL;
				break;
		}
		va_end(args);
	}
}

METHOD(auth_cfg_t, get, void*,
	private_auth_cfg_t *this, auth_rule_t type)
{
	enumerator_t *enumerator;
	void *current_value, *best_value = NULL;
	auth_rule_t current_type;
	bool found = FALSE;

	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &current_type, &current_value))
	{
		if (type == current_type)
		{
			if (type == AUTH_RULE_CRL_VALIDATION ||
				type == AUTH_RULE_OCSP_VALIDATION)
			{	/* for CRL/OCSP validation, always get() the highest value */
				if (!found || current_value > best_value)
				{
					best_value = current_value;
				}
				found = TRUE;
				continue;
			}
			best_value = current_value;
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (found)
	{
		return best_value;
	}
	switch (type)
	{
		/* use some sane defaults if we don't find an entry */
		case AUTH_RULE_AUTH_CLASS:
			return (void*)AUTH_CLASS_ANY;
		case AUTH_RULE_EAP_TYPE:
			return (void*)EAP_NAK;
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_BLISS_STRENGTH:
			return (void*)0;
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
			return (void*)VALIDATION_FAILED;
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
			return (void*)FALSE;
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_XAUTH_BACKEND:
		case AUTH_RULE_XAUTH_IDENTITY:
		case AUTH_RULE_GROUP:
		case AUTH_RULE_CA_CERT:
		case AUTH_RULE_IM_CERT:
		case AUTH_RULE_SUBJECT_CERT:
		case AUTH_RULE_CERT_POLICY:
		case AUTH_RULE_SIGNATURE_SCHEME:
		case AUTH_RULE_IKE_SIGNATURE_SCHEME:
		case AUTH_HELPER_IM_CERT:
		case AUTH_HELPER_SUBJECT_CERT:
		case AUTH_HELPER_IM_HASH_URL:
		case AUTH_HELPER_SUBJECT_HASH_URL:
		case AUTH_HELPER_REVOCATION_CERT:
		case AUTH_HELPER_AC_CERT:
		case AUTH_RULE_MAX:
			break;
	}
	return NULL;
}

/**
 * Implementation of auth_cfg_t.add.
 */
static void add(private_auth_cfg_t *this, auth_rule_t type, ...)
{
	entry_t entry;
	va_list args;

	va_start(args, type);
	init_entry(&entry, type, args);
	va_end(args);

	if (is_multi_value_rule(type))
	{	/* insert rules that may occur multiple times at the end */
		array_insert(this->entries, ARRAY_TAIL, &entry);
	}
	else
	{	/* insert rules we expect only once at the front (get() will return
		 * the latest value) */
		array_insert(this->entries, ARRAY_HEAD, &entry);
	}
}

/**
 * Create a constraint for RSA/PSS signatures
 */
static signature_params_t *create_rsa_pss_constraint(char *token)
{
	signature_params_t *params = NULL;
	hash_algorithm_t hash;

	if (enum_from_name(hash_algorithm_short_names, token, &hash))
	{
		rsa_pss_params_t pss = {
			.hash = hash,
			.mgf1_hash = hash,
			.salt_len = RSA_PSS_SALT_LEN_DEFAULT,
		};
		signature_params_t pss_params = {
			.scheme = SIGN_RSA_EMSA_PSS,
			.params = &pss,
		};
		rsa_pss_params_set_salt_len(&pss, 0);
		params = signature_params_clone(&pss_params);
	}
	return params;
}

METHOD(auth_cfg_t, add_pubkey_constraints, void,
	private_auth_cfg_t *this, char* constraints, bool ike)
{
	enumerator_t *enumerator;
	bool ike_added = FALSE, rsa_pss;
	key_type_t expected_type = -1;
	auth_rule_t expected_strength = AUTH_RULE_MAX;
	signature_params_t *params;
	int strength;
	char *token, *key_token = NULL;
	auth_rule_t type;
	void *value;

	rsa_pss = lib->settings->get_bool(lib->settings, "%s.rsa_pss", FALSE,
									  lib->ns);

	enumerator = enumerator_create_token(constraints, "-", "");
	while (enumerator->enumerate(enumerator, &token))
	{
		bool found = FALSE;
		int i;
		struct {
			char *name;
			signature_scheme_t scheme;
			key_type_t key;
		} schemes[] = {
			{ "md5",		SIGN_RSA_EMSA_PKCS1_MD5,		KEY_RSA,	 },
			{ "sha1",		SIGN_RSA_EMSA_PKCS1_SHA1,		KEY_RSA,	 },
			{ "sha224",		SIGN_RSA_EMSA_PKCS1_SHA2_224,	KEY_RSA,	 },
			{ "sha256",		SIGN_RSA_EMSA_PKCS1_SHA2_256,	KEY_RSA,	 },
			{ "sha384",		SIGN_RSA_EMSA_PKCS1_SHA2_384,	KEY_RSA,	 },
			{ "sha512",		SIGN_RSA_EMSA_PKCS1_SHA2_512,	KEY_RSA,	 },
			{ "sha1",		SIGN_ECDSA_WITH_SHA1_DER,		KEY_ECDSA,	 },
			{ "sha256",		SIGN_ECDSA_WITH_SHA256_DER,		KEY_ECDSA,	 },
			{ "sha384",		SIGN_ECDSA_WITH_SHA384_DER,		KEY_ECDSA,	 },
			{ "sha512",		SIGN_ECDSA_WITH_SHA512_DER,		KEY_ECDSA,	 },
			{ "sha256",		SIGN_ECDSA_256,					KEY_ECDSA,	 },
			{ "sha384",		SIGN_ECDSA_384,					KEY_ECDSA,	 },
			{ "sha512",		SIGN_ECDSA_521,					KEY_ECDSA,	 },
			{ "sha256",		SIGN_BLISS_WITH_SHA2_256,		KEY_BLISS,	 },
			{ "sha384",		SIGN_BLISS_WITH_SHA2_384,		KEY_BLISS,	 },
			{ "sha512",		SIGN_BLISS_WITH_SHA2_512,		KEY_BLISS,	 },
			{ "identity",	SIGN_ED25519,					KEY_ED25519, },
			{ "identity",	SIGN_ED448,						KEY_ED448,	 },
		};

		if (expected_strength != AUTH_RULE_MAX)
		{	/* expecting a key strength token */
			strength = atoi(token);
			if (strength)
			{
				add(this, expected_strength, (uintptr_t)strength);
			}
			expected_strength = AUTH_RULE_MAX;
			if (strength)
			{
				continue;
			}
		}
		if (streq(token, "rsa") || streq(token, "ike:rsa"))
		{
			key_token = token;
			expected_type = KEY_RSA;
			expected_strength = AUTH_RULE_RSA_STRENGTH;
			continue;
		}
		if (streq(token, "rsa/pss") || streq(token, "ike:rsa/pss"))
		{
			key_token = token;
			expected_type = KEY_RSA;
			expected_strength = AUTH_RULE_RSA_STRENGTH;
			continue;
		}
		if (streq(token, "ecdsa") || streq(token, "ike:ecdsa"))
		{
			key_token = token;
			expected_type = KEY_ECDSA;
			expected_strength = AUTH_RULE_ECDSA_STRENGTH;
			continue;
		}
		if (streq(token, "ed25519") || streq(token, "ike:ed25519"))
		{
			key_token = token;
			expected_type = KEY_ED25519;
			continue;
		}
		if (streq(token, "ed448") || streq(token, "ike:ed448"))
		{
			key_token = token;
			expected_type = KEY_ED448;
			continue;
		}
		if (streq(token, "bliss") || streq(token, "ike:bliss"))
		{
			key_token = token;
			expected_type = KEY_BLISS;
			expected_strength = AUTH_RULE_BLISS_STRENGTH;
			continue;
		}
		if (streq(token, "pubkey") || streq(token, "ike:pubkey"))
		{
			key_token = token;
			expected_type = KEY_ANY;
			continue;
		}
		if (key_token && strpfx(key_token, "ike:") && !ike)
		{
			continue;
		}

		if (key_token && streq(key_token + strlen(key_token) - 3, "pss"))
		{
			params = create_rsa_pss_constraint(token);
			if (params)
			{
				if (strpfx(key_token, "ike:"))
				{
					add(this, AUTH_RULE_IKE_SIGNATURE_SCHEME, params);
					ike_added = TRUE;
				}
				else
				{
					add(this, AUTH_RULE_SIGNATURE_SCHEME, params);
				}
				found = TRUE;
			}
		}
		else
		{
			if (rsa_pss)
			{
				if (expected_type == KEY_ANY ||
					expected_type == KEY_RSA)
				{
					params = create_rsa_pss_constraint(token);
					if (params)
					{
						if (strpfx(key_token, "ike:"))
						{
							add(this, AUTH_RULE_IKE_SIGNATURE_SCHEME, params);
							ike_added = TRUE;
						}
						else
						{
							add(this, AUTH_RULE_SIGNATURE_SCHEME, params);
						}
						found = TRUE;
					}
				}
			}
			for (i = 0; i < countof(schemes); i++)
			{
				if (streq(schemes[i].name, token))
				{
					if (expected_type == KEY_ANY ||
						expected_type == schemes[i].key)
					{
						INIT(params,
							.scheme = schemes[i].scheme,
						);
						if (strpfx(key_token, "ike:"))
						{
							add(this, AUTH_RULE_IKE_SIGNATURE_SCHEME, params);
							ike_added = TRUE;
						}
						else
						{
							add(this, AUTH_RULE_SIGNATURE_SCHEME, params);
						}
					}
					found = TRUE;
				}
			}
		}
		if (!found)
		{
			DBG1(DBG_CFG, "ignoring invalid auth token: '%s'", token);
		}
	}
	enumerator->destroy(enumerator);

	/* if no explicit IKE signature constraints were added we add them for all
	 * configured signature constraints */
	if (ike && !ike_added &&
		lib->settings->get_bool(lib->settings,
							"%s.signature_authentication_constraints", TRUE,
							lib->ns))
	{
		enumerator = create_enumerator(this);
		while (enumerator->enumerate(enumerator, &type, &value))
		{
			if (type == AUTH_RULE_SIGNATURE_SCHEME)
			{
				add(this, AUTH_RULE_IKE_SIGNATURE_SCHEME,
					signature_params_clone(value));
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Check if signature schemes of a specific type are compliant
 */
static bool complies_scheme(private_auth_cfg_t *this, auth_cfg_t *constraints,
							auth_rule_t type, bool log_error)
{
	enumerator_t *e1, *e2;
	auth_rule_t t1, t2;
	signature_params_t *params, *constraint;
	bool success = TRUE;

	e2 = create_enumerator(this);
	while (e2->enumerate(e2, &t2, &params))
	{
		if (t2 == type)
		{
			success = FALSE;
			e1 = constraints->create_enumerator(constraints);
			while (e1->enumerate(e1, &t1, &constraint))
			{
				if (t1 == type &&
					signature_params_comply(constraint, params))
				{
					success = TRUE;
					break;
				}
			}
			e1->destroy(e1);
			if (!success)
			{
				if (log_error)
				{
					DBG1(DBG_CFG, "%s signature scheme %N not acceptable",
						 AUTH_RULE_SIGNATURE_SCHEME == type ? "X.509" : "IKE",
						 signature_scheme_names, params->scheme);
				}
				break;
			}
		}
	}
	e2->destroy(e2);
	return success;
}

METHOD(auth_cfg_t, complies, bool,
	private_auth_cfg_t *this, auth_cfg_t *constraints, bool log_error)
{
	enumerator_t *e1, *e2;
	bool success = TRUE, group_match = FALSE;
	bool ca_match = FALSE, cert_match = FALSE;
	identification_t *require_group = NULL;
	certificate_t *require_ca = NULL, *require_cert = NULL;
	signature_params_t *ike_scheme = NULL, *scheme = NULL;
	u_int strength = 0;
	auth_rule_t t1, t2;
	char *key_type;
	void *value;

	e1 = constraints->create_enumerator(constraints);
	while (e1->enumerate(e1, &t1, &value))
	{
		switch (t1)
		{
			case AUTH_RULE_CA_CERT:
			case AUTH_RULE_IM_CERT:
			{
				certificate_t *cert;

				/* for CA certs, a match of a single cert is sufficient */
				require_ca = (certificate_t*)value;

				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &cert))
				{
					if ((t2 == AUTH_RULE_CA_CERT || t2 == AUTH_RULE_IM_CERT) &&
						cert->equals(cert, require_ca))
					{
						ca_match = TRUE;
					}
				}
				e2->destroy(e2);
				break;
			}
			case AUTH_RULE_SUBJECT_CERT:
			{
				certificate_t *cert;

				/* for certs, a match of a single cert is sufficient */
				require_cert = (certificate_t*)value;

				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &cert))
				{
					if (t2 == AUTH_RULE_SUBJECT_CERT &&
						cert->equals(cert, require_cert))
					{
						cert_match = TRUE;
					}
				}
				e2->destroy(e2);
				break;
			}
			case AUTH_RULE_CRL_VALIDATION:
			case AUTH_RULE_OCSP_VALIDATION:
			{
				uintptr_t validated;

				if (get(this, AUTH_RULE_CERT_VALIDATION_SUSPENDED))
				{	/* skip validation, may happen later */
					break;
				}

				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &validated))
				{
					if (t2 == t1)
					{
						switch ((uintptr_t)value)
						{
							case VALIDATION_FAILED:
								/* no constraint */
								break;
							case VALIDATION_SKIPPED:
								if (validated == VALIDATION_SKIPPED)
								{
									break;
								}
								/* FALL */
							case VALIDATION_GOOD:
								if (validated == VALIDATION_GOOD)
								{
									break;
								}
								/* FALL */
							default:
								success = FALSE;
								if (log_error)
								{
									DBG1(DBG_CFG, "constraint check failed: "
										 "%N is %N, but requires at least %N",
										 auth_rule_names, t1,
										 cert_validation_names, validated,
										 cert_validation_names, (uintptr_t)value);
								}
								break;
						}
					}
				}
				e2->destroy(e2);
				break;
			}
			case AUTH_RULE_IDENTITY:
			case AUTH_RULE_EAP_IDENTITY:
			case AUTH_RULE_AAA_IDENTITY:
			case AUTH_RULE_XAUTH_IDENTITY:
			{
				identification_t *id1, *id2;

				id1 = (identification_t*)value;
				id2 = get(this, t1);
				if (!id2 || !id2->matches(id2, id1))
				{
					if (t1 == AUTH_RULE_IDENTITY &&
						constraints->get(constraints, AUTH_RULE_IDENTITY_LOOSE))
					{	/* also verify identity against subjectAltNames */
						certificate_t *cert;

						cert = get(this, AUTH_RULE_SUBJECT_CERT);
						if (cert && cert->has_subject(cert, id1))
						{
							break;
						}
					}
					success = FALSE;
					if (log_error)
					{
						DBG1(DBG_CFG, "constraint check failed: %sidentity '%Y'"
							 " required ", t1 == AUTH_RULE_IDENTITY ? "" :
							 "EAP ", id1);
					}
				}
				break;
			}
			case AUTH_RULE_AUTH_CLASS:
			{
				if ((uintptr_t)value != AUTH_CLASS_ANY &&
					(uintptr_t)value != (uintptr_t)get(this, t1))
				{
					success = FALSE;
					if (log_error)
					{
						DBG1(DBG_CFG, "constraint requires %N authentication, "
							 "but %N was used", auth_class_names, (uintptr_t)value,
							 auth_class_names, (uintptr_t)get(this, t1));
					}
				}
				break;
			}
			case AUTH_RULE_EAP_TYPE:
			{
				if ((uintptr_t)value != (uintptr_t)get(this, t1) &&
					(uintptr_t)value != EAP_DYNAMIC &&
					(uintptr_t)value != EAP_RADIUS)
				{
					success = FALSE;
					if (log_error)
					{
						DBG1(DBG_CFG, "constraint requires %N, "
							 "but %N was used", eap_type_names, (uintptr_t)value,
							 eap_type_names,  (uintptr_t)get(this, t1));
					}
				}
				break;
			}
			case AUTH_RULE_EAP_VENDOR:
			{
				if ((uintptr_t)value != (uintptr_t)get(this, t1))
				{
					success = FALSE;
					if (log_error)
					{
						DBG1(DBG_CFG, "constraint requires EAP vendor %d, "
							 "but %d was used", (uintptr_t)value,
							 (uintptr_t)get(this, t1));
					}
				}
				break;
			}
			case AUTH_RULE_GROUP:
			{
				identification_t *group;

				/* for groups, a match of a single group is sufficient */
				require_group = (identification_t*)value;
				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &group))
				{
					if (t2 == AUTH_RULE_GROUP &&
						group->matches(group, require_group))
					{
						group_match = TRUE;
					}
				}
				e2->destroy(e2);
				break;
			}
			case AUTH_RULE_RSA_STRENGTH:
			case AUTH_RULE_ECDSA_STRENGTH:
			case AUTH_RULE_BLISS_STRENGTH:
			{
				strength = (uintptr_t)value;
				break;
			}
			case AUTH_RULE_IKE_SIGNATURE_SCHEME:
			{
				ike_scheme = value;
				break;
			}
			case AUTH_RULE_SIGNATURE_SCHEME:
			{
				scheme = value;
				break;
			}
			case AUTH_RULE_CERT_POLICY:
			{
				char *oid1, *oid2;

				oid1 = (char*)value;
				success = FALSE;
				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &oid2))
				{
					if (t2 == t1 && streq(oid1, oid2))
					{
						success = TRUE;
						break;
					}
				}
				e2->destroy(e2);
				if (!success && log_error)
				{
					DBG1(DBG_CFG, "constraint requires cert policy %s", oid1);
				}
				break;
			}
			case AUTH_RULE_IDENTITY_LOOSE:
				/* just an indication when verifying AUTH_RULE_IDENTITY */
			case AUTH_RULE_XAUTH_BACKEND:
				/* not enforced, just a hint for local authentication */
			case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
				/* not a constraint */
			case AUTH_HELPER_IM_CERT:
			case AUTH_HELPER_SUBJECT_CERT:
			case AUTH_HELPER_IM_HASH_URL:
			case AUTH_HELPER_SUBJECT_HASH_URL:
			case AUTH_HELPER_REVOCATION_CERT:
			case AUTH_HELPER_AC_CERT:
			case AUTH_RULE_MAX:
				/* skip helpers */
				continue;
		}
		if (!success)
		{
			break;
		}
	}
	e1->destroy(e1);

	/* Check if we have a matching constraint (or none at all) for used
	 * signature schemes. */
	if (success && scheme)
	{
		success = complies_scheme(this, constraints,
								  AUTH_RULE_SIGNATURE_SCHEME, log_error);
	}
	if (success && ike_scheme)
	{
		success = complies_scheme(this, constraints,
								  AUTH_RULE_IKE_SIGNATURE_SCHEME, log_error);
	}

	/* Check if we have a matching constraint (or none at all) for used
	 * public key strength */
	if (success && strength)
	{
		e2 = create_enumerator(this);
		while (e2->enumerate(e2, &t2, &strength))
		{
			switch (t2)
			{
				default:
					continue;
				case AUTH_RULE_RSA_STRENGTH:
					key_type = "RSA";
					break;
				case AUTH_RULE_ECDSA_STRENGTH:
					key_type = "ECDSA";
					break;
				case AUTH_RULE_BLISS_STRENGTH:
					key_type = "BLISS";
					break;
			}
			success = FALSE;
			e1 = constraints->create_enumerator(constraints);
			while (e1->enumerate(e1, &t1, &value))
			{
				if (t1 == t2 && (uintptr_t)value <= strength)
				{
					success = TRUE;
					break;
				}
			}
			e1->destroy(e1);
			if (!success)
			{
				if (log_error)
				{
					DBG1(DBG_CFG, "%s-%d signatures not acceptable",
						 key_type, strength);
				}
				break;
			}
		}
		e2->destroy(e2);
	}

	if (require_group && !group_match)
	{
		if (log_error)
		{
			DBG1(DBG_CFG, "constraint check failed: group membership to "
				 "'%Y' required", require_group);
		}
		return FALSE;
	}
	if (require_ca && !ca_match)
	{
		if (log_error)
		{
			DBG1(DBG_CFG, "constraint check failed: peer not "
				 "authenticated by CA '%Y'",
				 require_ca->get_subject(require_ca));
		}
		return FALSE;
	}
	if (require_cert && !cert_match)
	{
		if (log_error)
		{
			DBG1(DBG_CFG, "constraint check failed: peer not "
				 "authenticated with peer cert '%Y'",
				 require_cert->get_subject(require_cert));
		}
		return FALSE;
	}
	return success;
}

/**
 * Implementation of auth_cfg_t.merge.
 */
static void merge(private_auth_cfg_t *this, private_auth_cfg_t *other, bool copy)
{
	if (!other)
	{	/* nothing to merge */
		return;
	}
	if (copy)
	{
		enumerator_t *enumerator;
		auth_rule_t type;
		void *value;

		/* this enumerator skips duplicates for rules we expect only once */
		enumerator = create_enumerator(other);
		while (enumerator->enumerate(enumerator, &type, &value))
		{
			switch (type)
			{
				case AUTH_RULE_CA_CERT:
				case AUTH_RULE_IM_CERT:
				case AUTH_RULE_SUBJECT_CERT:
				case AUTH_HELPER_IM_CERT:
				case AUTH_HELPER_SUBJECT_CERT:
				case AUTH_HELPER_REVOCATION_CERT:
				case AUTH_HELPER_AC_CERT:
				{
					certificate_t *cert = (certificate_t*)value;

					add(this, type, cert->get_ref(cert));
					break;
				}
				case AUTH_RULE_IDENTITY_LOOSE:
				case AUTH_RULE_CRL_VALIDATION:
				case AUTH_RULE_OCSP_VALIDATION:
				case AUTH_RULE_AUTH_CLASS:
				case AUTH_RULE_EAP_TYPE:
				case AUTH_RULE_EAP_VENDOR:
				case AUTH_RULE_RSA_STRENGTH:
				case AUTH_RULE_ECDSA_STRENGTH:
				case AUTH_RULE_BLISS_STRENGTH:
				case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
				{
					add(this, type, (uintptr_t)value);
					break;
				}
				case AUTH_RULE_IDENTITY:
				case AUTH_RULE_EAP_IDENTITY:
				case AUTH_RULE_AAA_IDENTITY:
				case AUTH_RULE_GROUP:
				case AUTH_RULE_XAUTH_IDENTITY:
				{
					identification_t *id = (identification_t*)value;

					add(this, type, id->clone(id));
					break;
				}
				case AUTH_RULE_SIGNATURE_SCHEME:
				case AUTH_RULE_IKE_SIGNATURE_SCHEME:
				{
					add(this, type, signature_params_clone(value));
					break;
				}
				case AUTH_RULE_XAUTH_BACKEND:
				case AUTH_RULE_CERT_POLICY:
				case AUTH_HELPER_IM_HASH_URL:
				case AUTH_HELPER_SUBJECT_HASH_URL:
				{
					add(this, type, strdup((char*)value));
					break;
				}
				case AUTH_RULE_MAX:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		entry_t entry;

		while (array_remove(other->entries, ARRAY_TAIL, &entry))
		{	/* keep order but prefer new values (esp. for single valued ones) */
			array_insert(this->entries, ARRAY_HEAD, &entry);
		}
		array_compress(other->entries);
	}
}

/**
 * Compare two auth_cfg_t objects for equality.
 */
static bool auth_cfg_equals(private_auth_cfg_t *this, private_auth_cfg_t *other)
{
	enumerator_t *e1, *e2;
	entry_t *i1, *i2;
	bool equal = TRUE, found;

	/* the rule count does not have to be equal for the two, as we only compare
	 * the first value found for some rules */
	e1 = array_create_enumerator(this->entries);
	while (e1->enumerate(e1, &i1))
	{
		found = FALSE;

		e2 = array_create_enumerator(other->entries);
		while (e2->enumerate(e2, &i2))
		{
			if (entry_equals(i1, i2))
			{
				found = TRUE;
				break;
			}
			else if (i1->type == i2->type && !is_multi_value_rule(i1->type))
			{	/* we continue our search, only for multi valued rules */
				break;
			}
		}
		e2->destroy(e2);
		if (!found)
		{
			equal = FALSE;
			break;
		}
	}
	e1->destroy(e1);
	return equal;
}

/**
 * Implementation of auth_cfg_t.equals.
 */
static bool equals(private_auth_cfg_t *this, private_auth_cfg_t *other)
{
	if (auth_cfg_equals(this, other))
	{
		/* as 'other' might contain entries that 'this' doesn't we also check
		 * the other way around */
		return auth_cfg_equals(other, this);
	}
	return FALSE;
}

METHOD(auth_cfg_t, purge, void,
	private_auth_cfg_t *this, bool keep_ca)
{
	enumerator_t *enumerator;
	entry_t *entry;

	enumerator = array_create_enumerator(this->entries);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (!keep_ca || entry->type != AUTH_RULE_CA_CERT)
		{
			destroy_entry_value(entry);
			array_remove_at(this->entries, enumerator);
		}
	}
	enumerator->destroy(enumerator);

	array_compress(this->entries);
}

METHOD(auth_cfg_t, clone_, auth_cfg_t*,
	private_auth_cfg_t *this)
{
	enumerator_t *enumerator;
	auth_cfg_t *clone;
	auth_rule_t type;
	void *value;

	clone = auth_cfg_create();
	/* this enumerator skips duplicates for rules we expect only once */
	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		switch (type)
		{
			case AUTH_RULE_IDENTITY:
			case AUTH_RULE_EAP_IDENTITY:
			case AUTH_RULE_AAA_IDENTITY:
			case AUTH_RULE_GROUP:
			case AUTH_RULE_XAUTH_IDENTITY:
			{
				identification_t *id = (identification_t*)value;
				clone->add(clone, type, id->clone(id));
				break;
			}
			case AUTH_RULE_CA_CERT:
			case AUTH_RULE_IM_CERT:
			case AUTH_RULE_SUBJECT_CERT:
			case AUTH_HELPER_IM_CERT:
			case AUTH_HELPER_SUBJECT_CERT:
			case AUTH_HELPER_REVOCATION_CERT:
			case AUTH_HELPER_AC_CERT:
			{
				certificate_t *cert = (certificate_t*)value;
				clone->add(clone, type, cert->get_ref(cert));
				break;
			}
			case AUTH_RULE_XAUTH_BACKEND:
			case AUTH_RULE_CERT_POLICY:
			case AUTH_HELPER_IM_HASH_URL:
			case AUTH_HELPER_SUBJECT_HASH_URL:
			{
				clone->add(clone, type, strdup(value));
				break;
			}
			case AUTH_RULE_IDENTITY_LOOSE:
			case AUTH_RULE_AUTH_CLASS:
			case AUTH_RULE_EAP_TYPE:
			case AUTH_RULE_EAP_VENDOR:
			case AUTH_RULE_CRL_VALIDATION:
			case AUTH_RULE_OCSP_VALIDATION:
			case AUTH_RULE_RSA_STRENGTH:
			case AUTH_RULE_ECDSA_STRENGTH:
			case AUTH_RULE_BLISS_STRENGTH:
			case AUTH_RULE_CERT_VALIDATION_SUSPENDED:
				clone->add(clone, type, (uintptr_t)value);
				break;
			case AUTH_RULE_SIGNATURE_SCHEME:
			case AUTH_RULE_IKE_SIGNATURE_SCHEME:
			{
				clone->add(clone, type, signature_params_clone(value));
				break;
			}
			case AUTH_RULE_MAX:
				break;
		}
	}
	enumerator->destroy(enumerator);
	return clone;
}

METHOD(auth_cfg_t, destroy, void,
	private_auth_cfg_t *this)
{
	purge(this, FALSE);
	array_destroy(this->entries);
	free(this);
}

/*
 * see header file
 */
auth_cfg_t *auth_cfg_create()
{
	private_auth_cfg_t *this;

	INIT(this,
		.public = {
			.add = (void(*)(auth_cfg_t*, auth_rule_t type, ...))add,
			.add_pubkey_constraints = _add_pubkey_constraints,
			.get = _get,
			.create_enumerator = _create_enumerator,
			.replace = (void(*)(auth_cfg_t*,enumerator_t*,auth_rule_t,...))replace,
			.complies = _complies,
			.merge = (void(*)(auth_cfg_t*,auth_cfg_t*,bool))merge,
			.purge = _purge,
			.equals = (bool(*)(auth_cfg_t*,auth_cfg_t*))equals,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.entries = array_create(sizeof(entry_t), 0),
	);

	return &this->public;
}
