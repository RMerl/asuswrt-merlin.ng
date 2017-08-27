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
	"RULE_GROUP",
	"RULE_RSA_STRENGTH",
	"RULE_ECDSA_STRENGTH",
	"RULE_SIGNATURE_SCHEME",
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
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_IDENTITY:
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_EAP_IDENTITY:
		case AUTH_RULE_AAA_IDENTITY:
		case AUTH_RULE_XAUTH_IDENTITY:
		case AUTH_RULE_XAUTH_BACKEND:
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
		case AUTH_RULE_SIGNATURE_SCHEME:
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

/**
 * enumerate function for item_enumerator_t
 */
static bool enumerate(entry_enumerator_t *this, auth_rule_t *type, void **value)
{
	entry_t *entry;

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

/**
 * destroy function for item_enumerator_t
 */
static void entry_enumerator_destroy(entry_enumerator_t *this)
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
			.enumerate = (void*)enumerate,
			.destroy = (void*)entry_enumerator_destroy,
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
		case AUTH_RULE_SIGNATURE_SCHEME:
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
		case AUTH_RULE_SIGNATURE_SCHEME:
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
		case AUTH_RULE_IDENTITY_LOOSE:
		case AUTH_RULE_AUTH_CLASS:
		case AUTH_RULE_EAP_TYPE:
		case AUTH_RULE_EAP_VENDOR:
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
		case AUTH_RULE_RSA_STRENGTH:
		case AUTH_RULE_ECDSA_STRENGTH:
		case AUTH_RULE_SIGNATURE_SCHEME:
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
			case AUTH_RULE_SIGNATURE_SCHEME:
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
			return (void*)0;
		case AUTH_RULE_SIGNATURE_SCHEME:
			return (void*)HASH_UNKNOWN;
		case AUTH_RULE_CRL_VALIDATION:
		case AUTH_RULE_OCSP_VALIDATION:
			return (void*)VALIDATION_FAILED;
		case AUTH_RULE_IDENTITY_LOOSE:
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

METHOD(auth_cfg_t, complies, bool,
	private_auth_cfg_t *this, auth_cfg_t *constraints, bool log_error)
{
	enumerator_t *e1, *e2;
	bool success = TRUE, group_match = FALSE, cert_match = FALSE;
	identification_t *require_group = NULL;
	certificate_t *require_cert = NULL;
	signature_scheme_t scheme = SIGN_UNKNOWN;
	u_int strength = 0;
	auth_rule_t t1, t2;
	void *value;

	e1 = constraints->create_enumerator(constraints);
	while (e1->enumerate(e1, &t1, &value))
	{
		switch (t1)
		{
			case AUTH_RULE_CA_CERT:
			case AUTH_RULE_IM_CERT:
			{
				certificate_t *c1, *c2;

				c1 = (certificate_t*)value;

				success = FALSE;
				e2 = create_enumerator(this);
				while (e2->enumerate(e2, &t2, &c2))
				{
					if ((t2 == AUTH_RULE_CA_CERT || t2 == AUTH_RULE_IM_CERT) &&
						c1->equals(c1, c2))
					{
						success = TRUE;
					}
				}
				e2->destroy(e2);
				if (!success && log_error)
				{
					DBG1(DBG_CFG, "constraint check failed: peer not "
						 "authenticated by CA '%Y'.", c1->get_subject(c1));
				}
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

						cert = get(this, AUTH_HELPER_SUBJECT_CERT);
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
				if ((uintptr_t)value != (uintptr_t)get(this, t1))
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
			{
				strength = (uintptr_t)value;
				break;
			}
			case AUTH_RULE_SIGNATURE_SCHEME:
			{
				scheme = (uintptr_t)value;
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
	if (success && scheme != SIGN_UNKNOWN)
	{
		e2 = create_enumerator(this);
		while (e2->enumerate(e2, &t2, &scheme))
		{
			if (t2 == AUTH_RULE_SIGNATURE_SCHEME)
			{
				success = FALSE;
				e1 = constraints->create_enumerator(constraints);
				while (e1->enumerate(e1, &t1, &value))
				{
					if (t1 == AUTH_RULE_SIGNATURE_SCHEME &&
						(uintptr_t)value == scheme)
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
						DBG1(DBG_CFG, "signature scheme %N not acceptable",
							 signature_scheme_names, (int)scheme);
					}
					break;
				}
			}
		}
		e2->destroy(e2);
	}

	/* Check if we have a matching constraint (or none at all) for used
	 * public key strength */
	if (success && strength)
	{
		e2 = create_enumerator(this);
		while (e2->enumerate(e2, &t2, &strength))
		{
			if (t2 == AUTH_RULE_RSA_STRENGTH ||
				t2 == AUTH_RULE_ECDSA_STRENGTH)
			{
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
							 t2 == AUTH_RULE_RSA_STRENGTH ? "RSA" : "ECDSA",
							 strength);
					}
					break;
				}
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

	if (require_cert && !cert_match)
	{
		if (log_error)
		{
			DBG1(DBG_CFG, "constraint check failed: peer not "
				 "authenticated with peer cert '%Y'.",
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
				case AUTH_RULE_SIGNATURE_SCHEME:
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

		while (array_remove(other->entries, ARRAY_HEAD, &entry))
		{
			array_insert(this->entries, ARRAY_TAIL, &entry);
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
			case AUTH_RULE_SIGNATURE_SCHEME:
				clone->add(clone, type, (uintptr_t)value);
				break;
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
