/*
 * Copyright (C) 2016 Andreas Steffen
 * Copyright (C) 2009-2015 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "identification.h"

#include <utils/utils.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <crypto/hashers/hasher.h>

ENUM_BEGIN(id_match_names, ID_MATCH_NONE, ID_MATCH_MAX_WILDCARDS,
	"MATCH_NONE",
	"MATCH_ANY",
	"MATCH_MAX_WILDCARDS");
ENUM_NEXT(id_match_names, ID_MATCH_PERFECT, ID_MATCH_PERFECT, ID_MATCH_MAX_WILDCARDS,
	"MATCH_PERFECT");
ENUM_END(id_match_names, ID_MATCH_PERFECT);

ENUM_BEGIN(id_type_names, ID_ANY, ID_KEY_ID,
	"ID_ANY",
	"ID_IPV4_ADDR",
	"ID_FQDN",
	"ID_RFC822_ADDR",
	"ID_IPV4_ADDR_SUBNET",
	"ID_IPV6_ADDR",
	"ID_IPV6_ADDR_SUBNET",
	"ID_IPV4_ADDR_RANGE",
	"ID_IPV6_ADDR_RANGE",
	"ID_DER_ASN1_DN",
	"ID_DER_ASN1_GN",
	"ID_KEY_ID");
ENUM_NEXT(id_type_names, ID_DER_ASN1_GN_URI, ID_DER_ASN1_GN_URI, ID_KEY_ID,
	"ID_DER_ASN1_GN_URI");
ENUM_END(id_type_names, ID_DER_ASN1_GN_URI);

/**
 * coding of X.501 distinguished name
 */
typedef struct {
	const u_char *name;
	int oid;
	u_char type;
} x501rdn_t;

static const x501rdn_t x501rdns[] = {
	{"ND", 					OID_NAME_DISTINGUISHER,		ASN1_PRINTABLESTRING},
	{"UID", 				OID_PILOT_USERID,			ASN1_PRINTABLESTRING},
	{"DC", 					OID_PILOT_DOMAIN_COMPONENT, ASN1_PRINTABLESTRING},
	{"CN",					OID_COMMON_NAME,			ASN1_PRINTABLESTRING},
	{"S", 					OID_SURNAME,				ASN1_PRINTABLESTRING},
	{"SN", 					OID_SERIAL_NUMBER,			ASN1_PRINTABLESTRING},
	{"serialNumber", 		OID_SERIAL_NUMBER,			ASN1_PRINTABLESTRING},
	{"C", 					OID_COUNTRY,				ASN1_PRINTABLESTRING},
	{"L", 					OID_LOCALITY,				ASN1_PRINTABLESTRING},
	{"ST",					OID_STATE_OR_PROVINCE,		ASN1_PRINTABLESTRING},
	{"STREET",				OID_STREET_ADDRESS,			ASN1_PRINTABLESTRING},
	{"O", 					OID_ORGANIZATION,			ASN1_PRINTABLESTRING},
	{"OU", 					OID_ORGANIZATION_UNIT,		ASN1_PRINTABLESTRING},
	{"T", 					OID_TITLE,					ASN1_PRINTABLESTRING},
	{"D", 					OID_DESCRIPTION,			ASN1_PRINTABLESTRING},
	{"postalAddress",		OID_POSTAL_ADDRESS,			ASN1_PRINTABLESTRING},
	{"postalCode",			OID_POSTAL_CODE,			ASN1_PRINTABLESTRING},
	{"N", 					OID_NAME,					ASN1_PRINTABLESTRING},
	{"G", 					OID_GIVEN_NAME,				ASN1_PRINTABLESTRING},
	{"I", 					OID_INITIALS,				ASN1_PRINTABLESTRING},
	{"dnQualifier", 		OID_DN_QUALIFIER,			ASN1_PRINTABLESTRING},
	{"dmdName", 			OID_DMD_NAME,				ASN1_PRINTABLESTRING},
	{"pseudonym", 			OID_PSEUDONYM,				ASN1_PRINTABLESTRING},
	{"ID", 					OID_UNIQUE_IDENTIFIER,		ASN1_PRINTABLESTRING},
	{"EN", 					OID_EMPLOYEE_NUMBER,		ASN1_PRINTABLESTRING},
	{"employeeNumber",		OID_EMPLOYEE_NUMBER,		ASN1_PRINTABLESTRING},
	{"E",					OID_EMAIL_ADDRESS,			ASN1_IA5STRING},
	{"Email", 				OID_EMAIL_ADDRESS,			ASN1_IA5STRING},
	{"emailAddress",		OID_EMAIL_ADDRESS,			ASN1_IA5STRING},
	{"UN",					OID_UNSTRUCTURED_NAME,		ASN1_IA5STRING},
	{"unstructuredName",	OID_UNSTRUCTURED_NAME,		ASN1_IA5STRING},
	{"UA",					OID_UNSTRUCTURED_ADDRESS,	ASN1_PRINTABLESTRING},
	{"unstructuredAddress", OID_UNSTRUCTURED_ADDRESS,	ASN1_PRINTABLESTRING},
	{"TCGID", 				OID_TCGID,					ASN1_PRINTABLESTRING}
};

/**
 * maximum number of RDNs in atodn()
 */
#define RDN_MAX			20


typedef struct private_identification_t private_identification_t;

/**
 * Private data of an identification_t object.
 */
struct private_identification_t {
	/**
	 * Public interface.
	 */
	identification_t public;

	/**
	 * Encoded representation of this ID.
	 */
	chunk_t encoded;

	/**
	 * Type of this ID.
	 */
	id_type_t type;
};

/**
 * Enumerator over RDNs
 */
typedef struct {
	/* implements enumerator interface */
	enumerator_t public;
	/* next set to parse, if any */
	chunk_t sets;
	/* next sequence in set, if any */
	chunk_t seqs;
} rdn_enumerator_t;

METHOD(enumerator_t, rdn_enumerate, bool,
	rdn_enumerator_t *this, va_list args)
{
	chunk_t rdn, *oid, *data;
	u_char *type;

	VA_ARGS_VGET(args, oid, type, data);

	/* a DN contains one or more SET, each containing one or more SEQUENCES,
	 * each containing a OID/value RDN */
	if (!this->seqs.len)
	{
		/* no SEQUENCEs in current SET, parse next SET */
		if (asn1_unwrap(&this->sets, &this->seqs) != ASN1_SET)
		{
			return FALSE;
		}
	}
	if (asn1_unwrap(&this->seqs, &rdn) == ASN1_SEQUENCE &&
		asn1_unwrap(&rdn, oid) == ASN1_OID)
	{
		int t = asn1_unwrap(&rdn, data);

		if (t != ASN1_INVALID)
		{
			*type = t;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Create an enumerator over all RDNs (oid, string type, data) of a DN
 */
static enumerator_t* create_rdn_enumerator(chunk_t dn)
{
	rdn_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _rdn_enumerate,
			.destroy = (void*)free,
		},
	);

	/* a DN is a SEQUENCE, get the first SET of it */
	if (asn1_unwrap(&dn, &e->sets) == ASN1_SEQUENCE)
	{
		e->seqs = chunk_empty;
		return &e->public;
	}
	free(e);
	return enumerator_create_empty();
}

/**
 * Part enumerator over RDNs
 */
typedef struct {
	/* implements enumerator interface */
	enumerator_t public;
	/* inner RDN enumerator */
	enumerator_t *inner;
} rdn_part_enumerator_t;

METHOD(enumerator_t, rdn_part_enumerate, bool,
	rdn_part_enumerator_t *this, va_list args)
{
	int i, known_oid, strtype;
	chunk_t oid, inner_data, *data;
	id_part_t *type;
	static const struct {
		int oid;
		id_part_t type;
	} oid2part[] = {
		{OID_COMMON_NAME,		ID_PART_RDN_CN},
		{OID_SURNAME,			ID_PART_RDN_S},
		{OID_SERIAL_NUMBER,		ID_PART_RDN_SN},
		{OID_COUNTRY,			ID_PART_RDN_C},
		{OID_LOCALITY,			ID_PART_RDN_L},
		{OID_STATE_OR_PROVINCE,	ID_PART_RDN_ST},
		{OID_ORGANIZATION,		ID_PART_RDN_O},
		{OID_ORGANIZATION_UNIT,	ID_PART_RDN_OU},
		{OID_TITLE,				ID_PART_RDN_T},
		{OID_DESCRIPTION,		ID_PART_RDN_D},
		{OID_NAME,				ID_PART_RDN_N},
		{OID_GIVEN_NAME,		ID_PART_RDN_G},
		{OID_INITIALS,			ID_PART_RDN_I},
		{OID_DN_QUALIFIER,		ID_PART_RDN_DNQ},
		{OID_DMD_NAME,			ID_PART_RDN_DMDN},
		{OID_PSEUDONYM,			ID_PART_RDN_PN},
		{OID_UNIQUE_IDENTIFIER,	ID_PART_RDN_ID},
		{OID_EMAIL_ADDRESS,		ID_PART_RDN_E},
		{OID_EMPLOYEE_NUMBER,	ID_PART_RDN_EN},
	};

	VA_ARGS_VGET(args, type, data);

	while (this->inner->enumerate(this->inner, &oid, &strtype, &inner_data))
	{
		known_oid = asn1_known_oid(oid);
		for (i = 0; i < countof(oid2part); i++)
		{
			if (oid2part[i].oid == known_oid)
			{
				*type = oid2part[i].type;
				*data = inner_data;
				return TRUE;
			}
		}
	}
	return FALSE;
}

METHOD(enumerator_t, rdn_part_enumerator_destroy, void,
	rdn_part_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(identification_t, create_part_enumerator, enumerator_t*,
	private_identification_t *this)
{
	switch (this->type)
	{
		case ID_DER_ASN1_DN:
		{
			rdn_part_enumerator_t *e;

			INIT(e,
				.inner = create_rdn_enumerator(this->encoded),
				.public = {
					.enumerate = enumerator_enumerate_default,
					.venumerate = _rdn_part_enumerate,
					.destroy = _rdn_part_enumerator_destroy,
				},
			);
			return &e->public;
		}
		case ID_RFC822_ADDR:
			/* TODO */
		case ID_FQDN:
			/* TODO */
		default:
			return enumerator_create_empty();
	}
}

/**
 * Print a separator between two RDNs
 */
static inline bool print_separator(char **buf, size_t *len)
{
	int written;

	written = snprintf(*buf, *len, ", ");
	if (written < 0 || written >= *len)
	{
		return FALSE;
	}
	*buf += written;
	*len -= written;
	return TRUE;
}

/**
 * Print a DN with all its RDN in a buffer to present it to the user
 */
static void dntoa(chunk_t dn, char *buf, size_t len)
{
	enumerator_t *e;
	chunk_t oid_data, data, printable;
	u_char type;
	int oid, written;
	bool finished = FALSE, empty = TRUE;

	e = create_rdn_enumerator(dn);
	while (e->enumerate(e, &oid_data, &type, &data))
	{
		empty = FALSE;

		/* previous RDN was empty but it wasn't the last one */
		if (finished && !print_separator(&buf, &len))
		{
			break;
		}
		finished = FALSE;

		oid = asn1_known_oid(oid_data);
		if (oid == OID_UNKNOWN)
		{
			written = snprintf(buf, len, "%#B=", &oid_data);
		}
		else
		{
			written = snprintf(buf, len,"%s=", oid_names[oid].name);
		}
		if (written < 0 || written >= len)
		{
			break;
		}
		buf += written;
		len -= written;

		written = 0;
		chunk_printable(data, &printable, '?');
		if (printable.ptr)
		{
			written = snprintf(buf, len, "%.*s", (int)printable.len,
							   printable.ptr);
		}
		chunk_free(&printable);
		if (written < 0 || written >= len)
		{
			break;
		}
		buf += written;
		len -= written;

		if (!data.ptr)
		{	/* we can't calculate if we're finished, assume we are */
			finished = TRUE;
		}
		else if (data.ptr + data.len == dn.ptr + dn.len)
		{
			finished = TRUE;
			break;
		}
		else if (!print_separator(&buf, &len))
		{
			break;
		}
	}
	if (empty)
	{
		snprintf(buf, len, "");
	}
	else if (!finished)
	{
		snprintf(buf, len, "(invalid ID_DER_ASN1_DN)");
	}
	e->destroy(e);
}

/**
 * Converts an LDAP-style human-readable ASCII-encoded
 * ASN.1 distinguished name into binary DER-encoded format
 */
static status_t atodn(char *src, chunk_t *dn)
{
	/* finite state machine for atodn */
	typedef enum {
		SEARCH_OID =	0,
		READ_OID =		1,
		SEARCH_NAME =	2,
		READ_NAME =		3,
		UNKNOWN_OID =	4
	} state_t;

	chunk_t oid  = chunk_empty;
	chunk_t name = chunk_empty;
	chunk_t rdns[RDN_MAX];
	int rdn_count = 0;
	int dn_len = 0;
	int whitespace = 0;
	int i = 0;
	asn1_t rdn_type;
	state_t state = SEARCH_OID;
	status_t status = SUCCESS;
	char sep = '\0';

	do
	{
		switch (state)
		{
			case SEARCH_OID:
				if (!sep && *src == '/')
				{	/* use / as separator if the string starts with a slash */
					sep = '/';
					break;
				}
				if (*src != ' ' && *src != '\0')
				{
					if (!sep)
					{	/* use , as separator by default */
						sep = ',';
					}
					oid.ptr = src;
					oid.len = 1;
					state = READ_OID;
				}
				break;
			case READ_OID:
				if (*src != ' ' && *src != '=')
				{
					oid.len++;
				}
				else
				{
					bool found = FALSE;

					for (i = 0; i < countof(x501rdns); i++)
					{
						if (strlen(x501rdns[i].name) == oid.len &&
							strncasecmp(x501rdns[i].name, oid.ptr, oid.len) == 0)
						{
							found = TRUE;
							break;
						}
					}
					if (!found)
					{
						status = NOT_SUPPORTED;
						state = UNKNOWN_OID;
						break;
					}
					/* reset oid and change state */
					oid = chunk_empty;
					state = SEARCH_NAME;
				}
				break;
			case SEARCH_NAME:
				if (*src == ' ' || *src == '=')
				{
					break;
				}
				else if (*src != sep && *src != '\0')
				{
					name.ptr = src;
					name.len = 1;
					whitespace = 0;
					state = READ_NAME;
					break;
				}
				name = chunk_empty;
				whitespace = 0;
				state = READ_NAME;
				/* fall-through */
			case READ_NAME:
				if (*src != sep && *src != '\0')
				{
					name.len++;
					if (*src == ' ')
						whitespace++;
					else
						whitespace = 0;
				}
				else
				{
					name.len -= whitespace;
					rdn_type = (x501rdns[i].type == ASN1_PRINTABLESTRING
								&& !asn1_is_printablestring(name))
								? ASN1_UTF8STRING : x501rdns[i].type;

					if (rdn_count < RDN_MAX)
					{
						chunk_t rdn_oid;

						rdn_oid = asn1_build_known_oid(x501rdns[i].oid);
						if (rdn_oid.len)
						{
							rdns[rdn_count] =
									asn1_wrap(ASN1_SET, "m",
										asn1_wrap(ASN1_SEQUENCE, "mm",
											rdn_oid,
											asn1_wrap(rdn_type, "c", name)
										)
									);
							dn_len += rdns[rdn_count++].len;
						}
						else
						{
							status = INVALID_ARG;
						}
					}
					else
					{
						status = OUT_OF_RES;
					}
					/* reset name and change state */
					name = chunk_empty;
					state = SEARCH_OID;
				}
				break;
			case UNKNOWN_OID:
				break;
		}
	} while (*src++ != '\0');

	if (state == READ_OID)
	{	/* unterminated OID */
		status = INVALID_ARG;
	}

	/* build the distinguished name sequence */
	{
		int i;
		u_char *pos = asn1_build_object(dn, ASN1_SEQUENCE, dn_len);

		for (i = 0; i < rdn_count; i++)
		{
			memcpy(pos, rdns[i].ptr, rdns[i].len);
			pos += rdns[i].len;
			free(rdns[i].ptr);
		}
	}
	if (status != SUCCESS)
	{
		free(dn->ptr);
		*dn = chunk_empty;
	}
	return status;
}

METHOD(identification_t, get_encoding, chunk_t,
	private_identification_t *this)
{
	return this->encoded;
}

METHOD(identification_t, get_type, id_type_t,
	private_identification_t *this)
{
	return this->type;
}

METHOD(identification_t, contains_wildcards_dn, bool,
	private_identification_t *this)
{
	enumerator_t *enumerator;
	bool contains = FALSE;
	id_part_t type;
	chunk_t data;

	enumerator = create_part_enumerator(this);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (data.len == 1 && data.ptr[0] == '*')
		{
			contains = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return contains;
}

METHOD(identification_t, contains_wildcards_memchr, bool,
	private_identification_t *this)
{
	return memchr(this->encoded.ptr, '*', this->encoded.len) != NULL;
}

METHOD(identification_t, hash_binary, u_int,
	private_identification_t *this, u_int inc)
{
	u_int hash;

	hash = chunk_hash_inc(chunk_from_thing(this->type), inc);
	if (this->type != ID_ANY)
	{
		hash = chunk_hash_inc(this->encoded, hash);
	}
	return hash;
}

METHOD(identification_t, equals_binary, bool,
	private_identification_t *this, identification_t *other)
{
	if (this->type == other->get_type(other))
	{
		if (this->type == ID_ANY)
		{
			return TRUE;
		}
		return chunk_equals(this->encoded, other->get_encoding(other));
	}
	return FALSE;
}

/**
 * Compare to DNs, for equality if wc == NULL, for match otherwise
 */
static bool compare_dn(chunk_t t_dn, chunk_t o_dn, int *wc)
{
	enumerator_t *t, *o;
	chunk_t t_oid, o_oid, t_data, o_data;
	u_char t_type, o_type;
	bool t_next, o_next, finished = FALSE;

	if (wc)
	{
		*wc = 0;
	}
	else
	{
		if (t_dn.len != o_dn.len)
		{
			return FALSE;
		}
	}
	/* try a binary compare */
	if (chunk_equals(t_dn, o_dn))
	{
		return TRUE;
	}

	t = create_rdn_enumerator(t_dn);
	o = create_rdn_enumerator(o_dn);
	while (TRUE)
	{
		t_next = t->enumerate(t, &t_oid, &t_type, &t_data);
		o_next = o->enumerate(o, &o_oid, &o_type, &o_data);

		if (!o_next && !t_next)
		{
			break;
		}
		finished = FALSE;
		if (o_next != t_next)
		{
			break;
		}
		if (!chunk_equals(t_oid, o_oid))
		{
			break;
		}
		if (wc && o_data.len == 1 && o_data.ptr[0] == '*')
		{
			(*wc)++;
		}
		else
		{
			if (t_data.len != o_data.len)
			{
				break;
			}
			if (t_type == o_type &&
				(t_type == ASN1_PRINTABLESTRING ||
				 (t_type == ASN1_IA5STRING &&
				  asn1_known_oid(t_oid) == OID_EMAIL_ADDRESS)))
			{	/* ignore case for printableStrings and email RDNs */
				if (strncasecmp(t_data.ptr, o_data.ptr, t_data.len) != 0)
				{
					break;
				}
			}
			else
			{	/* respect case and length for everything else */
				if (!memeq(t_data.ptr, o_data.ptr, t_data.len))
				{
					break;
				}
			}
		}
		/* the enumerator returns FALSE on parse error, we are finished
		 * if we have reached the end of the DN only */
		if ((t_data.ptr + t_data.len == t_dn.ptr + t_dn.len) &&
			(o_data.ptr + o_data.len == o_dn.ptr + o_dn.len))
		{
			finished = TRUE;
		}
	}
	t->destroy(t);
	o->destroy(o);
	return finished;
}

METHOD(identification_t, equals_dn, bool,
	private_identification_t *this, identification_t *other)
{
	return compare_dn(this->encoded, other->get_encoding(other), NULL);
}

METHOD(identification_t, hash_dn, u_int,
	private_identification_t *this, u_int inc)
{
	enumerator_t *rdns;
	chunk_t oid, data;
	u_char type;
	u_int hash;

	hash = chunk_hash_inc(chunk_from_thing(this->type), inc);
	rdns = create_rdn_enumerator(this->encoded);
	while (rdns->enumerate(rdns, &oid, &type, &data))
	{
		hash = chunk_hash_inc(data, chunk_hash_inc(oid, hash));
	}
	rdns->destroy(rdns);
	return hash;
}

METHOD(identification_t, equals_strcasecmp,  bool,
	private_identification_t *this, identification_t *other)
{
	chunk_t encoded = other->get_encoding(other);

	/* we do some extra sanity checks to check for invalid IDs with a
	 * terminating null in it. */
	if (this->type == other->get_type(other) &&
		this->encoded.len == encoded.len &&
		memchr(this->encoded.ptr, 0, this->encoded.len) == NULL &&
		memchr(encoded.ptr, 0, encoded.len) == NULL &&
		strncasecmp(this->encoded.ptr, encoded.ptr, this->encoded.len) == 0)
	{
		return TRUE;
	}
	return FALSE;
}

METHOD(identification_t, matches_binary, id_match_t,
	private_identification_t *this, identification_t *other)
{
	if (other->get_type(other) == ID_ANY)
	{
		return ID_MATCH_ANY;
	}
	if (this->type == other->get_type(other) &&
		chunk_equals(this->encoded, other->get_encoding(other)))
	{
		return ID_MATCH_PERFECT;
	}
	return ID_MATCH_NONE;
}

METHOD(identification_t, matches_string, id_match_t,
	private_identification_t *this, identification_t *other)
{
	chunk_t encoded = other->get_encoding(other);
	u_int len = encoded.len;

	if (other->get_type(other) == ID_ANY)
	{
		return ID_MATCH_ANY;
	}
	if (this->type != other->get_type(other))
	{
		return ID_MATCH_NONE;
	}
	/* try a equals check first */
	if (equals_strcasecmp(this, other))
	{
		return ID_MATCH_PERFECT;
	}
	if (len == 0 || this->encoded.len < len)
	{
		return ID_MATCH_NONE;
	}

	/* check for single wildcard at the head of the string */
	if (*encoded.ptr == '*')
	{
		/* single asterisk matches any string */
		if (len-- == 1)
		{	/* not better than ID_ANY */
			return ID_MATCH_ANY;
		}
		if (strncasecmp(this->encoded.ptr + this->encoded.len - len,
						encoded.ptr + 1, len) == 0)
		{
			return ID_MATCH_ONE_WILDCARD;
		}
	}
	return ID_MATCH_NONE;
}

METHOD(identification_t, matches_any, id_match_t,
	private_identification_t *this, identification_t *other)
{
	if (other->get_type(other) == ID_ANY)
	{
		return ID_MATCH_ANY;
	}
	return ID_MATCH_NONE;
}

METHOD(identification_t, matches_dn, id_match_t,
	private_identification_t *this, identification_t *other)
{
	int wc;

	if (other->get_type(other) == ID_ANY)
	{
		return ID_MATCH_ANY;
	}

	if (this->type == other->get_type(other))
	{
		if (compare_dn(this->encoded, other->get_encoding(other), &wc))
		{
			wc = min(wc, ID_MATCH_ONE_WILDCARD - ID_MATCH_MAX_WILDCARDS);
			return ID_MATCH_PERFECT - wc;
		}
	}
	return ID_MATCH_NONE;
}

/**
 * Transform netmask to CIDR bits
 */
static int netmask_to_cidr(char *netmask, size_t address_size)
{
	uint8_t byte;
	int i, netbits = 0;

	for (i = 0; i < address_size; i++)
	{
		byte = netmask[i];

		if (byte == 0x00)
		{
			break;
		}
		if (byte == 0xff)
		{
			netbits += 8;
		}
		else
		{
			while (byte & 0x80)
			{
				netbits++;
				byte <<= 1;
			}
		}
	}
	return netbits;
}

METHOD(identification_t, matches_range, id_match_t,
	private_identification_t *this, identification_t *other)
{
	chunk_t other_encoding;
	uint8_t *address, *from, *to, *network, *netmask;
	size_t address_size = 0;
	int netbits, range_sign, i;

	if (other->get_type(other) == ID_ANY)
	{
		return ID_MATCH_ANY;
	}
	if (this->type == other->get_type(other) &&
		chunk_equals(this->encoded, other->get_encoding(other)))
	{
		return ID_MATCH_PERFECT;
	}
	if ((this->type == ID_IPV4_ADDR &&
		 other->get_type(other) == ID_IPV4_ADDR_SUBNET))
	{
		address_size = sizeof(struct in_addr);
	}
	else if ((this->type == ID_IPV6_ADDR &&
		 other->get_type(other) == ID_IPV6_ADDR_SUBNET))
	{
		address_size = sizeof(struct in6_addr);
	}
	if (address_size)
	{
		other_encoding = other->get_encoding(other);
		if (this->encoded.len != address_size ||
			other_encoding.len != 2 * address_size)
		{
			return ID_MATCH_NONE;
		}
		address = this->encoded.ptr;
		network = other_encoding.ptr;
		netmask = other_encoding.ptr + address_size;
		netbits = netmask_to_cidr(netmask, address_size);

		if (netbits == 0)
		{
			return ID_MATCH_MAX_WILDCARDS;
		}
		if (netbits == 8 * address_size)
		{
			return memeq(address, network, address_size) ?
				   ID_MATCH_PERFECT : ID_MATCH_NONE;
		}
		for (i = 0; i < (netbits + 7)/8; i++)
		{
			if ((address[i] ^ network[i]) & netmask[i])
			{
				return ID_MATCH_NONE;
			}
		}
		return ID_MATCH_ONE_WILDCARD;
	}
	if ((this->type == ID_IPV4_ADDR &&
		 other->get_type(other) == ID_IPV4_ADDR_RANGE))
	{
		address_size = sizeof(struct in_addr);
	}
	else if ((this->type == ID_IPV6_ADDR &&
		 other->get_type(other) == ID_IPV6_ADDR_RANGE))
	{
		address_size = sizeof(struct in6_addr);
	}
	if (address_size)
	{
		other_encoding = other->get_encoding(other);
		if (this->encoded.len != address_size ||
			other_encoding.len != 2 * address_size)
		{
			return ID_MATCH_NONE;
		}
		address = this->encoded.ptr;
		from = other_encoding.ptr;
		to = other_encoding.ptr + address_size;

		range_sign = memcmp(to, from, address_size);
		if (range_sign < 0)
		{	/* to is smaller than from */
			return ID_MATCH_NONE;
		}

		/* check lower bound */
		for (i = 0; i < address_size; i++)
		{
			if (address[i] != from[i])
			{
				if (address[i] < from[i])
				{
					return ID_MATCH_NONE;
				}
				break;
			}
		}

		/* check upper bound */
		for (i = 0; i < address_size; i++)
		{
			if (address[i] != to[i])
			{
				if (address[i] > to[i])
				{
					return ID_MATCH_NONE;
				}
				break;
			}
		}
		return range_sign ? ID_MATCH_ONE_WILDCARD : ID_MATCH_PERFECT;
	}
	return ID_MATCH_NONE;
}

/**
 * Described in header.
 */
int identification_printf_hook(printf_hook_data_t *data,
							printf_hook_spec_t *spec, const void *const *args)
{
	private_identification_t *this = *((private_identification_t**)(args[0]));
	chunk_t proper;
	char buf[BUF_LEN], *pos;
	size_t len, address_size;
	int written;

	if (this == NULL)
	{
		return print_in_hook(data, "%*s", spec->width, "(null)");
	}

	switch (this->type)
	{
		case ID_ANY:
			snprintf(buf, BUF_LEN, "%%any");
			break;
		case ID_IPV4_ADDR:
			if (this->encoded.len < sizeof(struct in_addr) ||
				inet_ntop(AF_INET, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV4_ADDR)");
			}
			break;
		case ID_IPV4_ADDR_SUBNET:
			address_size = sizeof(struct in_addr);
			if (this->encoded.len < 2 * address_size ||
				inet_ntop(AF_INET, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV4_ADDR_SUBNET)");
				break;
			}
			written = strlen(buf);
			snprintf(buf + written, BUF_LEN - written, "/%d",
					 netmask_to_cidr(this->encoded.ptr + address_size,
														 address_size));
			break;
		case ID_IPV4_ADDR_RANGE:
			address_size = sizeof(struct in_addr);
			if (this->encoded.len < 2 * address_size ||
				inet_ntop(AF_INET, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV4_ADDR_RANGE)");
				break;
			}
			written = strlen(buf);
			pos = buf + written;
			len = BUF_LEN - written;
			written = snprintf(pos, len, "-");
			if (written < 0 || written >= len ||
			    inet_ntop(AF_INET, this->encoded.ptr + address_size,
						  pos + written, len - written) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV4_ADDR_RANGE)");
			}
			break;
		case ID_IPV6_ADDR:
			if (this->encoded.len < sizeof(struct in6_addr) ||
				inet_ntop(AF_INET6, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV6_ADDR)");
			}
			break;
		case ID_IPV6_ADDR_SUBNET:
			address_size = sizeof(struct in6_addr);
			if (this->encoded.len < 2 * address_size ||
				inet_ntop(AF_INET6, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV6_ADDR_SUBNET)");
			}
			else
			{
				written = strlen(buf);
				snprintf(buf + written, BUF_LEN - written, "/%d",
						 netmask_to_cidr(this->encoded.ptr + address_size,
															 address_size));
			}
			break;
		case ID_IPV6_ADDR_RANGE:
			address_size = sizeof(struct in6_addr);
			if (this->encoded.len < 2 * address_size ||
				inet_ntop(AF_INET6, this->encoded.ptr, buf, BUF_LEN) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV6_ADDR_RANGE)");
				break;
			}
			written = strlen(buf);
			pos = buf + written;
			len = BUF_LEN - written;
			written = snprintf(pos, len, "-");
			if (written < 0 || written >= len ||
			    inet_ntop(AF_INET6, this->encoded.ptr + address_size,
						  pos + written, len - written) == NULL)
			{
				snprintf(buf, BUF_LEN, "(invalid ID_IPV6_ADDR_RANGE)");
			}
			break;
		case ID_FQDN:
		case ID_RFC822_ADDR:
		case ID_DER_ASN1_GN_URI:
			chunk_printable(this->encoded, &proper, '?');
			snprintf(buf, BUF_LEN, "%.*s", (int)proper.len, proper.ptr);
			chunk_free(&proper);
			break;
		case ID_DER_ASN1_DN:
			dntoa(this->encoded, buf, BUF_LEN);
			break;
		case ID_DER_ASN1_GN:
			snprintf(buf, BUF_LEN, "(ASN.1 general name)");
			break;
		case ID_KEY_ID:
			if (chunk_printable(this->encoded, NULL, '?') &&
				this->encoded.len != HASH_SIZE_SHA1)
			{	/* fully printable, use ascii version */
				snprintf(buf, BUF_LEN, "%.*s", (int)this->encoded.len,
						 this->encoded.ptr);
			}
			else
			{	/* not printable, hex dump */
				snprintf(buf, BUF_LEN, "%#B", &this->encoded);
			}
			break;
		default:
			snprintf(buf, BUF_LEN, "(unknown ID type: %d)", this->type);
			break;
	}
	if (spec->minus)
	{
		return print_in_hook(data, "%-*s", spec->width, buf);
	}
	return print_in_hook(data, "%*s", spec->width, buf);
}

METHOD(identification_t, clone_, identification_t*,
	private_identification_t *this)
{
	private_identification_t *clone = malloc_thing(private_identification_t);

	memcpy(clone, this, sizeof(private_identification_t));
	if (this->encoded.len)
	{
		clone->encoded = chunk_clone(this->encoded);
	}
	return &clone->public;
}

METHOD(identification_t, destroy, void,
	private_identification_t *this)
{
	chunk_free(&this->encoded);
	free(this);
}

/**
 * Generic constructor used for the other constructors.
 */
static private_identification_t *identification_create(id_type_t type)
{
	private_identification_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_type = _get_type,
			.create_part_enumerator = _create_part_enumerator,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.type = type,
	);

	switch (type)
	{
		case ID_ANY:
			this->public.hash = _hash_binary;
			this->public.equals = _equals_binary;
			this->public.matches = _matches_any;
			this->public.contains_wildcards = return_true;
			break;
		case ID_FQDN:
		case ID_RFC822_ADDR:
			this->public.hash = _hash_binary;
			this->public.equals = _equals_strcasecmp;
			this->public.matches = _matches_string;
			this->public.contains_wildcards = _contains_wildcards_memchr;
			break;
		case ID_DER_ASN1_DN:
			this->public.hash = _hash_dn;
			this->public.equals = _equals_dn;
			this->public.matches = _matches_dn;
			this->public.contains_wildcards = _contains_wildcards_dn;
			break;
		case ID_IPV4_ADDR:
		case ID_IPV6_ADDR:
			this->public.hash = _hash_binary;
			this->public.equals = _equals_binary;
			this->public.matches = _matches_range;
			this->public.contains_wildcards = return_false;
			break;
		default:
			this->public.hash = _hash_binary;
			this->public.equals = _equals_binary;
			this->public.matches = _matches_binary;
			this->public.contains_wildcards = return_false;
			break;
	}
	return this;
}

/**
 * Create an identity for a specific type, determined by prefix
 */
static private_identification_t* create_from_string_with_prefix_type(char *str)
{
	struct {
		const char *str;
		id_type_t type;
	} prefixes[] = {
		{ "ipv4:",			ID_IPV4_ADDR			},
		{ "ipv6:",			ID_IPV6_ADDR			},
		{ "ipv4net:",		ID_IPV4_ADDR_SUBNET		},
		{ "ipv6net:",		ID_IPV6_ADDR_SUBNET		},
		{ "ipv4range:",		ID_IPV4_ADDR_RANGE		},
		{ "ipv6range:",		ID_IPV6_ADDR_RANGE		},
		{ "rfc822:",		ID_RFC822_ADDR			},
		{ "email:",			ID_RFC822_ADDR			},
		{ "userfqdn:",		ID_USER_FQDN			},
		{ "fqdn:",			ID_FQDN					},
		{ "dns:",			ID_FQDN					},
		{ "asn1dn:",		ID_DER_ASN1_DN			},
		{ "asn1gn:",		ID_DER_ASN1_GN			},
		{ "xmppaddr:",		ID_DER_ASN1_GN          },
		{ "keyid:",			ID_KEY_ID				},
	};
	private_identification_t *this;
	int i;

	for (i = 0; i < countof(prefixes); i++)
	{
		if (strcasepfx(str, prefixes[i].str))
		{
			this = identification_create(prefixes[i].type);
			str += strlen(prefixes[i].str);

			if (*str == '#')
			{
				this->encoded = chunk_from_hex(chunk_from_str(str + 1), NULL);
			}
			else
			{
				this->encoded = chunk_clone(chunk_from_str(str));
			}

			if (prefixes[i].type == ID_DER_ASN1_GN &&
				strcasepfx(prefixes[i].str, "xmppaddr:"))
			{
				this->encoded = asn1_wrap(ASN1_CONTEXT_C_0, "mm",
									asn1_build_known_oid(OID_XMPP_ADDR),
									asn1_wrap(ASN1_CONTEXT_C_0, "m",
										asn1_wrap(ASN1_UTF8STRING, "m",
											this->encoded)));
			}

			return this;
		}
	}
	return NULL;
}

/**
 * Create an identity for a specific type, determined by a numerical prefix
 *
 * The prefix is of the form "{x}:", where x denotes the numerical identity
 * type.
 */
static private_identification_t* create_from_string_with_num_type(char *str)
{
	private_identification_t *this;
	u_long type;

	if (*str++ != '{')
	{
		return NULL;
	}
	errno = 0;
	type = strtoul(str, &str, 0);
	if (errno || *str++ != '}' || *str++ != ':')
	{
		return NULL;
	}
	this = identification_create(type);
	if (*str == '#')
	{
		this->encoded = chunk_from_hex(chunk_from_str(str + 1), NULL);
	}
	else
	{
		this->encoded = chunk_clone(chunk_from_str(str));
	}
	return this;
}

/**
 * Convert to an IPv4/IPv6 host address, subnet or address range
 */
static private_identification_t* create_ip_address_from_string(char *string,
															   bool is_ipv4)
{
	private_identification_t *this;
	uint8_t encoding[32];
	uint8_t *str, *pos, *address, *to_address, *netmask;
	size_t address_size;
	int bits, bytes, i;
	bool has_subnet = FALSE, has_range = FALSE;

	address = encoding;
	address_size = is_ipv4 ? sizeof(struct in_addr) : sizeof(struct in6_addr);

	str = strdup(string);
	pos = strchr(str, '/');
	if (pos)
	{	/* separate IP address from optional netmask */

		*pos = '\0';
		has_subnet = TRUE;
	}
	else
	{
		pos = strchr(str, '-');
		if (pos)
		{	/* separate lower address from upper address of IP range */
			*pos = '\0';
			has_range = TRUE;
		}
	}

	if (inet_pton(is_ipv4 ? AF_INET : AF_INET6, str, address) != 1)
	{
		free(str);
		return NULL;
	}

	if (has_subnet)
	{	/* is IP subnet */
		bits = atoi(pos + 1);
		if (bits > 8 * address_size)
		{
			free(str);
			return NULL;
		}
		bytes = bits / 8;
		bits -= 8 * bytes;
		netmask = encoding + address_size;

		for (i = 0; i < address_size; i++)
		{
			if (bytes)
			{
				*netmask = 0xff;
				bytes--;
			}
			else if (bits)
			{
				*netmask = 0xff << (8 - bits);
				bits = 0;
			}
			else
			{
				*netmask = 0x00;
			}
			*address++ &= *netmask++;
		}
		this = identification_create(is_ipv4 ? ID_IPV4_ADDR_SUBNET :
											   ID_IPV6_ADDR_SUBNET);
		this->encoded = chunk_clone(chunk_create(encoding, 2 * address_size));
	}
	else if (has_range)
	{	/* is IP range */
		to_address = encoding + address_size;

		if (inet_pton(is_ipv4 ? AF_INET : AF_INET6, pos + 1, to_address) != 1)
		{
			free(str);
			return NULL;
		}
		for (i = 0; i < address_size; i++)
		{
			if (address[i] != to_address[i])
			{
				if (address[i] > to_address[i])
				{
					free(str);
					return NULL;
				}
				break;
			}
		}
		this = identification_create(is_ipv4 ? ID_IPV4_ADDR_RANGE :
											   ID_IPV6_ADDR_RANGE);
		this->encoded = chunk_clone(chunk_create(encoding, 2 * address_size));
	}
	else
	{	/* is IP host address */
		this = identification_create(is_ipv4 ? ID_IPV4_ADDR : ID_IPV6_ADDR);
		this->encoded = chunk_clone(chunk_create(encoding, address_size));
	}
	free(str);

	return this;
}

/*
 * Described in header.
 */
identification_t *identification_create_from_string(char *string)
{
	private_identification_t *this;
	chunk_t encoded;

	if (string == NULL)
	{
		string = "%any";
	}
	this = create_from_string_with_prefix_type(string);
	if (this)
	{
		return &this->public;
	}
	this = create_from_string_with_num_type(string);
	if (this)
	{
		return &this->public;
	}
	if (strchr(string, '=') != NULL)
	{
		/* we interpret this as an ASCII X.501 ID_DER_ASN1_DN.
		 * convert from LDAP style or openssl x509 -subject style to ASN.1 DN
		 */
		if (atodn(string, &encoded) == SUCCESS)
		{
			this = identification_create(ID_DER_ASN1_DN);
			this->encoded = encoded;
		}
		else
		{
			this = identification_create(ID_KEY_ID);
			this->encoded = chunk_from_str(strdup(string));
		}
		return &this->public;
	}
	else if (strchr(string, '@') == NULL)
	{
		if (streq(string, "")
		||	streq(string, "%any")
		||	streq(string, "%any6")
		||	streq(string, "0.0.0.0")
		||	streq(string, "*")
		||	streq(string, "::")
		||	streq(string, "0::0"))
		{
			/* any ID will be accepted */
			this = identification_create(ID_ANY);
			return &this->public;
		}
		else
		{
			if (strchr(string, ':') == NULL)
			{
				/* IPv4 address or subnet */
				this = create_ip_address_from_string(string, TRUE);
				if (!this)
				{	/* not IPv4, mostly FQDN */
					this = identification_create(ID_FQDN);
					this->encoded = chunk_from_str(strdup(string));
				}
				return &this->public;
			}
			else
			{
				/* IPv6 address or subnet */
				this = create_ip_address_from_string(string, FALSE);
				if (!this)
				{	/* not IPv4/6 fallback to KEY_ID */
					this = identification_create(ID_KEY_ID);
					this->encoded = chunk_from_str(strdup(string));
				}
				return &this->public;
			}
		}
	}
	else
	{
		if (*string == '@')
		{
			string++;
			if (*string == '#')
			{
				this = identification_create(ID_KEY_ID);
				this->encoded = chunk_from_hex(chunk_from_str(string + 1), NULL);
				return &this->public;
			}
			else if (*string == '@')
			{
				this = identification_create(ID_USER_FQDN);
				this->encoded = chunk_clone(chunk_from_str(string + 1));
				return &this->public;
			}
			else
			{
				this = identification_create(ID_FQDN);
				this->encoded = chunk_clone(chunk_from_str(string));
				return &this->public;
			}
		}
		else
		{
			this = identification_create(ID_RFC822_ADDR);
			this->encoded = chunk_from_str(strdup(string));
			return &this->public;
		}
	}
}

/*
 * Described in header.
 */
identification_t * identification_create_from_data(chunk_t data)
{
	char buf[data.len + 1];

	if (is_asn1(data))
	{
		return identification_create_from_encoding(ID_DER_ASN1_DN, data);
	}
	else
	{
		/* use string constructor */
		snprintf(buf, sizeof(buf), "%.*s", (int)data.len, data.ptr);
		return identification_create_from_string(buf);
	}
}

/*
 * Described in header.
 */
identification_t *identification_create_from_encoding(id_type_t type,
													  chunk_t encoded)
{
	private_identification_t *this = identification_create(type);

	/* apply encoded chunk */
	if (type != ID_ANY)
	{
		this->encoded = chunk_clone(encoded);
	}
	return &(this->public);
}

/*
 * Described in header.
 */
identification_t *identification_create_from_sockaddr(sockaddr_t *sockaddr)
{
	switch (sockaddr->sa_family)
	{
		case AF_INET:
		{
			struct in_addr *addr = &(((struct sockaddr_in*)sockaddr)->sin_addr);

			return identification_create_from_encoding(ID_IPV4_ADDR,
					chunk_create((u_char*)addr, sizeof(struct in_addr)));
		}
		case AF_INET6:
		{
			struct in6_addr *addr = &(((struct sockaddr_in6*)sockaddr)->sin6_addr);

			return identification_create_from_encoding(ID_IPV6_ADDR,
					chunk_create((u_char*)addr, sizeof(struct in6_addr)));
		}
		default:
		{
			private_identification_t *this = identification_create(ID_ANY);

			return &(this->public);
		}
	}
}
