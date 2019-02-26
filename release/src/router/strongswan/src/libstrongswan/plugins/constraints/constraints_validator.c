/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "constraints_validator.h"

#include <utils/debug.h>
#include <asn1/asn1.h>
#include <collections/linked_list.h>
#include <credentials/certificates/x509.h>

typedef struct private_constraints_validator_t private_constraints_validator_t;

/**
 * Private data of an constraints_validator_t object.
 */
struct private_constraints_validator_t {

	/**
	 * Public constraints_validator_t interface.
	 */
	constraints_validator_t public;
};

/**
 * Check pathlen constraint of issuer certificate
 */
static bool check_pathlen(x509_t *issuer, int pathlen)
{
	u_int pathlen_constraint;

	pathlen_constraint = issuer->get_constraint(issuer, X509_PATH_LEN);
	if (pathlen_constraint != X509_NO_CONSTRAINT &&
		pathlen > pathlen_constraint)
	{
		DBG1(DBG_CFG, "path length of %d violates constraint of %d",
			 pathlen, pathlen_constraint);
		return FALSE;
	}
	return TRUE;
}

/**
 * Check if a FQDN constraint matches
 */
static bool fqdn_matches(identification_t *constraint, identification_t *id)
{
	chunk_t c, i, diff;

	c = constraint->get_encoding(constraint);
	i = id->get_encoding(id);

	if (!c.len || i.len < c.len)
	{
		return FALSE;
	}
	diff = chunk_create(i.ptr, i.len - c.len);
	if (!chunk_equals(c, chunk_skip(i, diff.len)))
	{
		return FALSE;
	}
	if (!diff.len)
	{
		return TRUE;
	}
	if (c.ptr[0] == '.' || diff.ptr[diff.len - 1] == '.')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Check if a RFC822 constraint matches
 */
static bool email_matches(identification_t *constraint, identification_t *id)
{
	chunk_t c, i, diff;

	c = constraint->get_encoding(constraint);
	i = id->get_encoding(id);

	if (!c.len || i.len < c.len)
	{
		return FALSE;
	}
	if (memchr(c.ptr, '@', c.len))
	{	/* constraint is a full email address */
		return chunk_equals(c, i);
	}
	diff = chunk_create(i.ptr, i.len - c.len);
	if (!diff.len || !chunk_equals(c, chunk_skip(i, diff.len)))
	{
		return FALSE;
	}
	if (c.ptr[0] == '.')
	{	/* constraint is domain, suffix match */
		return TRUE;
	}
	if (diff.ptr[diff.len - 1] == '@')
	{	/* constraint is host specific, only username can be appended */
		return TRUE;
	}
	return FALSE;
}

/**
 * Check if a DN constraint matches (RDN prefix match)
 */
static bool dn_matches(identification_t *constraint, identification_t *id)
{
	enumerator_t *ec, *ei;
	id_part_t pc, pi;
	chunk_t cc, ci;
	bool match = TRUE;

	ec = constraint->create_part_enumerator(constraint);
	ei = id->create_part_enumerator(id);
	while (ec->enumerate(ec, &pc, &cc))
	{
		if (!ei->enumerate(ei, &pi, &ci) ||
			pi != pc || !chunk_equals(cc, ci))
		{
			match = FALSE;
			break;
		}
	}
	ec->destroy(ec);
	ei->destroy(ei);

	return match;
}

/**
 * Check if a certificate matches to a NameConstraint
 */
static bool name_constraint_matches(identification_t *constraint,
									certificate_t *cert, bool permitted)
{
	x509_t *x509 = (x509_t*)cert;
	enumerator_t *enumerator;
	identification_t *id;
	id_type_t type;
	bool matches = permitted;

	type = constraint->get_type(constraint);
	if (type == ID_DER_ASN1_DN)
	{
		matches = dn_matches(constraint, cert->get_subject(cert));
		if (matches != permitted)
		{
			return matches;
		}
	}

	enumerator = x509->create_subjectAltName_enumerator(x509);
	while (enumerator->enumerate(enumerator, &id))
	{
		if (id->get_type(id) == type)
		{
			switch (type)
			{
				case ID_FQDN:
					matches = fqdn_matches(constraint, id);
					break;
				case ID_RFC822_ADDR:
					matches = email_matches(constraint, id);
					break;
				case ID_DER_ASN1_DN:
					matches = dn_matches(constraint, id);
					break;
				default:
					DBG1(DBG_CFG, "%N NameConstraint matching not implemented",
						 id_type_names, type);
					matches = FALSE;
					break;
			}
		}
		if (matches != permitted)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	return matches;
}

/**
 * Check if a permitted or excluded NameConstraint has been inherited to sub-CA
 */
static bool name_constraint_inherited(identification_t *constraint,
									  x509_t *x509, bool permitted)
{
	enumerator_t *enumerator;
	identification_t *id, *a, *b;
	bool inherited = FALSE;
	id_type_t type;

	if (!(x509->get_flags(x509) & X509_CA))
	{	/* not a sub-CA, not required */
		return TRUE;
	}

	type = constraint->get_type(constraint);
	enumerator = x509->create_name_constraint_enumerator(x509, permitted);
	while (enumerator->enumerate(enumerator, &id))
	{
		if (id->get_type(id) == type)
		{
			if (permitted)
			{	/* permitted constraint can be narrowed */
				a = constraint;
				b = id;
			}
			else
			{	/* excluded constraint can be widened */
				a = id;
				b = constraint;
			}
			switch (type)
			{
				case ID_FQDN:
					inherited = fqdn_matches(a, b);
					break;
				case ID_RFC822_ADDR:
					inherited = email_matches(a, b);
					break;
				case ID_DER_ASN1_DN:
					inherited = dn_matches(a, b);
					break;
				default:
					DBG1(DBG_CFG, "%N NameConstraint matching not implemented",
						 id_type_names, type);
					inherited = FALSE;
					break;
			}
		}
		if (inherited)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return inherited;
}

/**
 * Check name constraints
 */
static bool check_name_constraints(certificate_t *subject, x509_t *issuer)
{
	enumerator_t *enumerator;
	identification_t *constraint;

	enumerator = issuer->create_name_constraint_enumerator(issuer, TRUE);
	while (enumerator->enumerate(enumerator, &constraint))
	{
		if (!name_constraint_matches(constraint, subject, TRUE))
		{
			DBG1(DBG_CFG, "certificate '%Y' does not match permitted name "
				 "constraint '%Y'", subject->get_subject(subject), constraint);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		if (!name_constraint_inherited(constraint, (x509_t*)subject, TRUE))
		{
			DBG1(DBG_CFG, "intermediate CA '%Y' does not inherit permitted name "
				 "constraint '%Y'", subject->get_subject(subject), constraint);
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	enumerator = issuer->create_name_constraint_enumerator(issuer, FALSE);
	while (enumerator->enumerate(enumerator, &constraint))
	{
		if (name_constraint_matches(constraint, subject, FALSE))
		{
			DBG1(DBG_CFG, "certificate '%Y' matches excluded name "
				 "constraint '%Y'", subject->get_subject(subject), constraint);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		if (!name_constraint_inherited(constraint, (x509_t*)subject, FALSE))
		{
			DBG1(DBG_CFG, "intermediate CA '%Y' does not inherit excluded name "
				 "constraint '%Y'", subject->get_subject(subject), constraint);
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);
	return TRUE;
}

/**
 * Special OID for anyPolicy
 */
static chunk_t any_policy = chunk_from_chars(0x55,0x1d,0x20,0x00);

/**
 * Check if an issuer certificate has a given policy OID
 */
static bool has_policy(x509_t *issuer, chunk_t oid)
{
	x509_policy_mapping_t *mapping;
	x509_cert_policy_t *policy;
	enumerator_t *enumerator;

	enumerator = issuer->create_cert_policy_enumerator(issuer);
	while (enumerator->enumerate(enumerator, &policy))
	{
		if (chunk_equals(oid, policy->oid) ||
			chunk_equals(any_policy, policy->oid))
		{
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);

	/* fall back to a mapped policy */
	enumerator = issuer->create_policy_mapping_enumerator(issuer);
	while (enumerator->enumerate(enumerator, &mapping))
	{
		if (chunk_equals(mapping->subject, oid))
		{
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

/**
 * Check certificatePolicies.
 */
static bool check_policy(x509_t *subject, x509_t *issuer)
{
	certificate_t *cert = (certificate_t*)subject;
	x509_policy_mapping_t *mapping;
	x509_cert_policy_t *policy;
	enumerator_t *enumerator;
	char *oid;

	/* verify if policyMappings in subject are valid */
	enumerator = subject->create_policy_mapping_enumerator(subject);
	while (enumerator->enumerate(enumerator, &mapping))
	{
		if (!has_policy(issuer, mapping->issuer))
		{
			oid = asn1_oid_to_string(mapping->issuer);
			DBG1(DBG_CFG, "certificate '%Y' maps policy from %s, but issuer "
				 "misses it", cert->get_subject(cert), oid);
			free(oid);
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	enumerator = subject->create_cert_policy_enumerator(subject);
	while (enumerator->enumerate(enumerator, &policy))
	{
		if (!has_policy(issuer, policy->oid))
		{
			oid = asn1_oid_to_string(policy->oid);
			DBG1(DBG_CFG, "policy %s missing in issuing certificate '%Y'",
				 oid, cert->get_issuer(cert));
			free(oid);
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

/**
 * Check if a given policy is valid under a trustchain
 */
static bool is_policy_valid(linked_list_t *chain, chunk_t oid)
{
	x509_policy_mapping_t *mapping;
	x509_cert_policy_t *policy;
	x509_t *issuer;
	enumerator_t *issuers, *policies, *mappings;
	bool found = TRUE;

	issuers = chain->create_enumerator(chain);
	while (issuers->enumerate(issuers, &issuer))
	{
		int maxmap = 8;

		while (found)
		{
			found = FALSE;

			policies = issuer->create_cert_policy_enumerator(issuer);
			while (policies->enumerate(policies, &policy))
			{
				if (chunk_equals(oid, policy->oid) ||
					chunk_equals(any_policy, policy->oid))
				{
					found = TRUE;
					break;
				}
			}
			policies->destroy(policies);
			if (found)
			{
				break;
			}
			/* fall back to a mapped policy */
			mappings = issuer->create_policy_mapping_enumerator(issuer);
			while (mappings->enumerate(mappings, &mapping))
			{
				if (chunk_equals(mapping->subject, oid))
				{
					oid = mapping->issuer;
					found = TRUE;
					break;
				}
			}
			mappings->destroy(mappings);
			if (--maxmap == 0)
			{
				found = FALSE;
				break;
			}
		}
		if (!found)
		{
			break;
		}
	}
	issuers->destroy(issuers);

	return found;
}

/**
 * Check len certificates in trustchain for inherited policies
 */
static bool has_policy_chain(linked_list_t *chain, x509_t *subject, int len)
{
	enumerator_t *enumerator;
	x509_t *issuer;
	bool valid = TRUE;

	enumerator = chain->create_enumerator(chain);
	while (len-- > 0 && enumerator->enumerate(enumerator, &issuer))
	{
		if (!check_policy(subject, issuer))
		{
			valid = FALSE;
			break;
		}
		subject = issuer;
	}
	enumerator->destroy(enumerator);
	return valid;
}

/**
 * Check len certificates in trustchain to have no policyMappings
 */
static bool has_no_policy_mapping(linked_list_t *chain, int len)
{
	enumerator_t *enumerator, *mappings;
	x509_policy_mapping_t *mapping;
	certificate_t *cert;
	x509_t *x509;
	bool valid = TRUE;

	enumerator = chain->create_enumerator(chain);
	while (len-- > 0 && enumerator->enumerate(enumerator, &x509))
	{
		mappings = x509->create_policy_mapping_enumerator(x509);
		valid = !mappings->enumerate(mappings, &mapping);
		mappings->destroy(mappings);
		if (!valid)
		{
			cert = (certificate_t*)x509;
			DBG1(DBG_CFG, "found policyMapping in certificate '%Y', but "
				 "inhibitPolicyMapping in effect", cert->get_subject(cert));
			break;
		}
	}
	enumerator->destroy(enumerator);
	return valid;
}

/**
 * Check len certificates in trustchain to have no anyPolicies
 */
static bool has_no_any_policy(linked_list_t *chain, int len)
{
	enumerator_t *enumerator, *policies;
	x509_cert_policy_t *policy;
	certificate_t *cert;
	x509_t *x509;
	bool valid = TRUE;

	enumerator = chain->create_enumerator(chain);
	while (len-- > 0 && enumerator->enumerate(enumerator, &x509))
	{
		policies = x509->create_cert_policy_enumerator(x509);
		while (policies->enumerate(policies, &policy))
		{
			if (chunk_equals(policy->oid, any_policy))
			{
				cert = (certificate_t*)x509;
				DBG1(DBG_CFG, "found anyPolicy in certificate '%Y', but "
					 "inhibitAnyPolicy in effect", cert->get_subject(cert));
				valid = FALSE;
				break;
			}
		}
		policies->destroy(policies);
	}
	enumerator->destroy(enumerator);
	return valid;
}

/**
 * Check requireExplicitPolicy and inhibitPolicyMapping constraints
 */
static bool check_policy_constraints(x509_t *issuer, u_int pathlen,
									 auth_cfg_t *auth)
{
	certificate_t *subject;
	bool valid = TRUE;

	subject = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (subject)
	{
		if (subject->get_type(subject) == CERT_X509)
		{
			x509_cert_policy_t *policy;
			enumerator_t *enumerator;
			linked_list_t *chain;
			certificate_t *cert;
			auth_rule_t rule;
			x509_t *x509;
			int len = 0;
			u_int expl, inh;
			char *oid;

			/* prepare trustchain to validate */
			chain = linked_list_create();
			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &cert))
			{
				if (rule == AUTH_RULE_IM_CERT &&
					cert->get_type(cert) == CERT_X509)
				{
					chain->insert_last(chain, cert);
				}
			}
			enumerator->destroy(enumerator);
			chain->insert_last(chain, issuer);

			/* search for requireExplicitPolicy constraints */
			enumerator = chain->create_enumerator(chain);
			while (enumerator->enumerate(enumerator, &x509))
			{
				expl = x509->get_constraint(x509, X509_REQUIRE_EXPLICIT_POLICY);
				if (expl != X509_NO_CONSTRAINT)
				{
					if (!has_policy_chain(chain, (x509_t*)subject, len - expl))
					{
						valid = FALSE;
						break;
					}
				}
				len++;
			}
			enumerator->destroy(enumerator);

			/* search for inhibitPolicyMapping/inhibitAnyPolicy constraints */
			len = 0;
			chain->insert_first(chain, subject);
			enumerator = chain->create_enumerator(chain);
			while (enumerator->enumerate(enumerator, &x509))
			{
				inh = x509->get_constraint(x509, X509_INHIBIT_POLICY_MAPPING);
				if (inh != X509_NO_CONSTRAINT)
				{
					if (!has_no_policy_mapping(chain, len - inh))
					{
						valid = FALSE;
						break;
					}
				}
				inh = x509->get_constraint(x509, X509_INHIBIT_ANY_POLICY);
				if (inh != X509_NO_CONSTRAINT)
				{
					if (!has_no_any_policy(chain, len - inh))
					{
						valid = FALSE;
						break;
					}
				}
				len++;
			}
			enumerator->destroy(enumerator);

			if (valid)
			{
				x509 = (x509_t*)subject;

				enumerator = x509->create_cert_policy_enumerator(x509);
				while (enumerator->enumerate(enumerator, &policy))
				{
					oid = asn1_oid_to_string(policy->oid);
					if (oid)
					{
						if (is_policy_valid(chain, policy->oid))
						{
							auth->add(auth, AUTH_RULE_CERT_POLICY, oid);
						}
						else
						{
							DBG1(DBG_CFG, "certificate policy %s for '%Y' "
								 "not allowed by trustchain, ignored",
								 oid, subject->get_subject(subject));
							free(oid);
						}
					}
				}
				enumerator->destroy(enumerator);
			}
			chain->destroy(chain);
		}
	}
	return valid;
}

METHOD(cert_validator_t, validate, bool,
	private_constraints_validator_t *this, certificate_t *subject,
	certificate_t *issuer, bool online, u_int pathlen, bool anchor,
	auth_cfg_t *auth)
{
	if (issuer->get_type(issuer) == CERT_X509 &&
		subject->get_type(subject) == CERT_X509)
	{
		if (!check_pathlen((x509_t*)issuer, pathlen))
		{
			lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_EXCEEDED_PATH_LEN,
									subject);
			return FALSE;
		}
		if (!check_name_constraints(subject, (x509_t*)issuer))
		{
			lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_POLICY_VIOLATION,
									subject);
			return FALSE;
		}
		if (anchor)
		{
			if (!check_policy_constraints((x509_t*)issuer, pathlen, auth))
			{
				lib->credmgr->call_hook(lib->credmgr,
										CRED_HOOK_POLICY_VIOLATION, issuer);
				return FALSE;
			}
		}
	}
	return TRUE;
}

METHOD(constraints_validator_t, destroy, void,
	private_constraints_validator_t *this)
{
	free(this);
}

/**
 * See header
 */
constraints_validator_t *constraints_validator_create()
{
	private_constraints_validator_t *this;

	INIT(this,
		.public = {
			.validator.validate = _validate,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
