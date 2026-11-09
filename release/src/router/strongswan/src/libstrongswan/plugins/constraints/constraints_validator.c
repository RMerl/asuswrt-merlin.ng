/*
 * Copyright (C) 2023-2024 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
#include <collections/array.h>
#include <collections/hashtable.h>
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
	if (!chunk_equals(c, chunk_skip(i, diff.len)))
	{
		return FALSE;
	}
	if (!diff.len)
	{
		return TRUE;
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
 * Check if a new permitted or excluded NameConstraint is matching an
 * existing one
 */
static bool name_constraint_matches(identification_t *existing,
									identification_t *new, bool permitted)
{
	identification_t *a, *b;
	bool matching = FALSE;

	if (permitted)
	{	/* permitted constraint can be narrowed */
		a = existing;
		b = new;
	}
	else
	{	/* excluded constraint can be widened */
		a = new;
		b = existing;
	}
	switch (existing->get_type(existing))
	{
		case ID_FQDN:
			matching = fqdn_matches(a, b);
			break;
		case ID_RFC822_ADDR:
			matching = email_matches(a, b);
			break;
		case ID_DER_ASN1_DN:
			matching = dn_matches(a, b);
			break;
		case ID_IPV4_ADDR_SUBNET:
		case ID_IPV6_ADDR_SUBNET:
			matching = b->matches(b, a);
			break;
		default:
			/* shouldn't happen */
			matching = FALSE;
			break;
	}
	return matching;
}

/**
 * Get the name constraint type from an identity type
 */
static id_type_t constraint_type_from_id(id_type_t id)
{
	switch (id)
	{
		case ID_IPV4_ADDR:
			return ID_IPV4_ADDR_SUBNET;
		case ID_IPV6_ADDR:
			return ID_IPV6_ADDR_SUBNET;
		default:
			return id;
	}
}

/**
 * Check if the given identity matches any of the given name constraints
 */
static bool id_matches_constraints(certificate_t *cert, identification_t *id,
								   array_t *constraints, bool permitted)
{
	enumerator_t *enumerator;
	identification_t *subject, *constraint;
	id_type_t type;
	bool matches = FALSE;

	subject = cert->get_subject(cert);
 	type = id->get_type(id);

	enumerator = array_create_enumerator(constraints);
	while (enumerator->enumerate(enumerator, &constraint))
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
			case ID_IPV4_ADDR:
			case ID_IPV6_ADDR:
				matches = id->matches(id, constraint);
				break;
			default:
				/* shouldn't happen */
				break;
		}
		if (matches)
		{
			if (!permitted)
			{
				if (id->equals(id, subject))
				{
					DBG1(DBG_CFG, "subject of certificate '%Y' matches excluded "
						 "name constraint '%Y'", subject, constraint);
				}
				else
				{
					DBG1(DBG_CFG, "subject alternative name '%Y' of certificate "
						 "'%Y' matches excluded name constraint '%Y'",
						 id, subject, constraint);
				}
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!matches && permitted)
	{
		if (id->equals(id, subject))
		{
			DBG1(DBG_CFG, "subject of certificate '%Y' does not match any "
				 "permitted name constraints", subject);
		}
		else
		{
			DBG1(DBG_CFG, "subject alternative name '%Y' of certificate '%Y' "
				 "does not match any permitted name constraints", id, subject);
		}
	}
	return matches;
}

/**
 * Check if a certificate matches the given permitted/excluded name constraints
 */
static bool cert_matches_constraints(x509_t *x509, hashtable_t *types,
									 bool permitted)
{
	certificate_t *cert = (certificate_t*)x509;
	array_t *constraints;
	enumerator_t *enumerator;
	identification_t *id;
	id_type_t type;
	bool matches = permitted;

	constraints = types->get(types, (void*)(uintptr_t)ID_DER_ASN1_DN);
	if (constraints)
	{
		matches = id_matches_constraints(cert, cert->get_subject(cert),
										 constraints, permitted);
		if (matches != permitted)
		{
			return matches;
		}
	}

	enumerator = x509->create_subjectAltName_enumerator(x509);
	while (enumerator->enumerate(enumerator, &id))
	{
		type = constraint_type_from_id(id->get_type(id));
		constraints = types->get(types, (void*)(uintptr_t)type);
		if (constraints)
		{
			matches = id_matches_constraints(cert, id, constraints, permitted);
			if (matches != permitted)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	return matches;
}

/**
 * Validate the names in the given certificate against the current constraints
 */
static bool name_constraints_match(x509_t *x509, hashtable_t *permitted,
								   hashtable_t *excluded)
{
	if (permitted && !cert_matches_constraints(x509, permitted, TRUE))
	{
		return FALSE;
	}
	if (excluded && cert_matches_constraints(x509, excluded, FALSE))
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Destroy name constraints (callback for hashtable_t::destroy_function())
 */
CALLBACK(destroy_constraints, void,
	array_t *this, const void *key)
{
	array_destroy(this);
}

/**
 * Hashtable hash function
 */
static u_int id_type_hash(const void *key)
{
	uintptr_t id = (uintptr_t)key;
	return chunk_hash(chunk_from_thing(id));
}

/**
 * Hashtable equals function
 */
static bool id_type_equals(const void *a, const void *b)
{
	return (uintptr_t)a == (uintptr_t)b;
}

/**
 * Collect name constraints (permitted or excluded) of each supported type
 * from the given certificate
 */
static bool collect_constraints(x509_t *x509, bool permitted, hashtable_t **out)
{
	hashtable_t *collected;
	enumerator_t *enumerator;
	identification_t *constraint;
	array_t *constraints;
	id_type_t type;
	bool success = TRUE;

	collected = hashtable_create(id_type_hash, id_type_equals, 8);

	enumerator = x509->create_name_constraint_enumerator(x509, permitted);
	while (enumerator->enumerate(enumerator, &constraint))
	{
		type = constraint->get_type(constraint);
		switch (type)
		{
			case ID_FQDN:
			case ID_RFC822_ADDR:
			case ID_DER_ASN1_DN:
			case ID_IPV4_ADDR_SUBNET:
			case ID_IPV6_ADDR_SUBNET:
				break;
			default:
				DBG1(DBG_CFG, "%N NameConstraint not supported",
					 id_type_names, type);
				success = FALSE;
				break;
		}
		if (!success)
		{
			break;
		}
		constraints = collected->get(collected, (void*)(uintptr_t)type);
		if (!constraints)
		{
			constraints = array_create(0, 8);
			collected->put(collected, (void*)(uintptr_t)type, constraints);
		}
		array_insert(constraints, ARRAY_TAIL, constraint);
	}
	enumerator->destroy(enumerator);

	if (success)
	{
		*out = collected;
	}
	else
	{
		collected->destroy_function(collected, destroy_constraints);
	}
	return success;
}

/**
 * Merge existing and new permitted/excluded name constraints
 */
static void merge_constraints(certificate_t *cert, array_t *existing_constraints,
							  array_t *new_constraints, bool permitted)
{
	enumerator_t *enumerator, *new;
	identification_t *constraint, *new_constraint;

	if (permitted)
	{
		array_t *to_move = NULL;

		enumerator = array_create_enumerator(existing_constraints);
		while (enumerator->enumerate(enumerator, &constraint))
		{
			new = array_create_enumerator(new_constraints);
			while (new->enumerate(new, &new_constraint))
			{
				if (name_constraint_matches(constraint, new_constraint, TRUE))
				{
					array_insert_create(&to_move, ARRAY_TAIL, new_constraint);
					array_remove_at(new_constraints, new);
				}
			}
			new->destroy(new);

			/* remove the existing constraint.  if it was matched, it gets
			 * replaced by the moved equal/narrower constraints, if not, it's
			 * not permitted anymore */
			array_remove_at(existing_constraints, enumerator);
		}
		enumerator->destroy(enumerator);

		if (to_move)
		{
			while (array_remove(to_move, ARRAY_HEAD, &new_constraint))
			{
				array_insert(existing_constraints, ARRAY_TAIL, new_constraint);
			}
			array_destroy(to_move);
		}
		/* report ignored constraints that would widen the permitted set */
		while (array_remove(new_constraints, ARRAY_HEAD, &new_constraint))
		{
			DBG1(DBG_CFG, "ignoring name constraint '%Y' in certificate "
				 "'%Y' that's not permitted by parent CAs",
				 new_constraint, cert->get_subject(cert));
		}
	}
	else
	{
		/* this is simpler as we basically adopt all new constraints, we just
		 * check if we can remove a constraint that gets widened */
		enumerator = array_create_enumerator(existing_constraints);
		while (enumerator->enumerate(enumerator, &constraint))
		{
			new = array_create_enumerator(new_constraints);
			while (new->enumerate(new, &new_constraint))
			{
				if (name_constraint_matches(constraint, new_constraint, FALSE))
				{
					/* remove the existing constraint if it is matched, it
					 * gets replaced by an equal/wider constraint */
					array_remove_at(existing_constraints, enumerator);
					break;
				}
			}
			new->destroy(new);
		}
		enumerator->destroy(enumerator);

		/* add all new constraints to the list */
		while (array_remove(new_constraints, ARRAY_HEAD, &new_constraint))
		{
			array_insert(existing_constraints, ARRAY_TAIL, new_constraint);
		}
	}
}

/**
 * Update the set of permitted/excluded name constraints
 */
static bool update_name_constraints(x509_t *x509, hashtable_t **existing,
									bool permitted)
{
	enumerator_t *enumerator;
	hashtable_t *collected;
	array_t *existing_constraints, *new_constraints;
	void *type;

	if (!(x509->get_flags(x509) & X509_CA))
	{
		/* ignore end-entity certificates */
		return TRUE;
	}

	if (!collect_constraints(x509, permitted, &collected))
	{
		return FALSE;
	}
	if (collected->get_count(collected))
	{
		if (!*existing)
		{
			/* adopt all constraints if we haven't any yet */
			*existing = collected;
			collected = NULL;
		}
		else
		{
			/* merge sets of constraints for each type */
			enumerator = collected->create_enumerator(collected);
			while (enumerator->enumerate(enumerator, &type, &new_constraints))
			{
				existing_constraints = (*existing)->get(*existing, type);
				if (existing_constraints)
				{
					/* merge constraints of known types, either allowing them to
					 * get narrowed or widened */
					merge_constraints((certificate_t*)x509, existing_constraints,
									  new_constraints, permitted);
				}
				else
				{
					/* adopt constraints for new types */
					collected->remove_at(collected, enumerator);
					(*existing)->put(*existing, type, new_constraints);
				}
			}
			enumerator->destroy(enumerator);
		}
	}
	DESTROY_FUNCTION_IF(collected, destroy_constraints);
	return TRUE;
}

/**
 * Check name constraints
 */
static bool check_name_constraints(x509_t *issuer, u_int pathlen,
								   auth_cfg_t *auth, certificate_t **violator)
{
	enumerator_t *enumerator;
	linked_list_t *chain;
	hashtable_t *permitted = NULL, *excluded = NULL;
	certificate_t *subject, *cert;
	auth_rule_t rule;
	x509_t *x509;
	int len = 0;
	bool valid = TRUE;

	subject = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (!subject || subject->get_type(subject) != CERT_X509)
	{
		return TRUE;
	}

	/* prepare trustchain to validate name constraints top-down */
	chain = linked_list_create_with_items(subject, NULL);
	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &rule, &cert))
	{
		if (rule == AUTH_RULE_IM_CERT &&
			cert->get_type(cert) == CERT_X509)
		{
			chain->insert_first(chain, cert);
		}
	}
	enumerator->destroy(enumerator);
	chain->insert_first(chain, issuer);

	enumerator = chain->create_enumerator(chain);
	while (enumerator->enumerate(enumerator, &x509))
	{
		if ((len > 0 && !name_constraints_match(x509, permitted, excluded)) ||
			!update_name_constraints(x509, &permitted, TRUE) ||
			!update_name_constraints(x509, &excluded, FALSE))
		{
			valid = FALSE;
			*violator = (certificate_t*)x509;
			break;
		}
		len++;
	}
	enumerator->destroy(enumerator);

	DESTROY_FUNCTION_IF(permitted, destroy_constraints);
	DESTROY_FUNCTION_IF(excluded, destroy_constraints);
	chain->destroy(chain);
	return valid;
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
	certificate_t *cert DBG_UNUSED = (certificate_t*)subject;
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
	certificate_t *cert DBG_UNUSED;
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
	certificate_t *cert DBG_UNUSED;
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
				/* remove the self-signed root certificate from the chain if it
				 * does not contain any certificate policies, in accordance with
				 * RFC 5280, sections 6.1 and 6.2 */
				x509 = (x509_t*)issuer;
				enumerator = x509->create_cert_policy_enumerator(x509);
				if ((x509->get_flags(x509) & X509_SELF_SIGNED) &&
					!enumerator->enumerate(enumerator, &policy))
				{
					chain->remove_last(chain, (void**)&x509);
				}
				enumerator->destroy(enumerator);

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
	certificate_t *issuer, u_int pathlen, bool anchor, auth_cfg_t *auth)
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
		if (anchor)
		{
			certificate_t *violator;

			if (!check_name_constraints((x509_t*)issuer, pathlen, auth, &violator))
			{
				lib->credmgr->call_hook(lib->credmgr,
										CRED_HOOK_POLICY_VIOLATION, violator);
				return FALSE;
			}
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
