/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2006-2010 Martin Willi
 * Copyright (C) 2013-2015 Andreas Steffen
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

#include "proposal.h"

#include <collections/array.h>
#include <utils/identification.h>

#include <crypto/transform.h>
#include <crypto/prfs/prf.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>

ENUM(protocol_id_names, PROTO_NONE, PROTO_IPCOMP,
	"PROTO_NONE",
	"IKE",
	"AH",
	"ESP",
	"IPCOMP",
);

typedef struct private_proposal_t private_proposal_t;

/**
 * Private data of an proposal_t object
 */
struct private_proposal_t {

	/**
	 * Public part
	 */
	proposal_t public;

	/**
	 * protocol (ESP or AH)
	 */
	protocol_id_t protocol;

	/**
	 * Priority ordered list of transforms, as entry_t
	 */
	array_t *transforms;

	/**
	 * Types of transforms contained, as transform_type_t
	 */
	array_t *types;

	/**
	 * senders SPI
	 */
	uint64_t spi;

	/**
	 * Proposal number
	 */
	u_int number;
};

/**
 * This is a hack to not change the previous order when printing proposals
 */
static transform_type_t type_for_sort(const void *type)
{
	const transform_type_t *t = type;

	switch (*t)
	{
		case PSEUDO_RANDOM_FUNCTION:
			return INTEGRITY_ALGORITHM;
		case INTEGRITY_ALGORITHM:
			return PSEUDO_RANDOM_FUNCTION;
		default:
			return *t;
	}
}

/**
 * Sort transform types
 */
static int type_sort(const void *a, const void *b, void *user)
{
	transform_type_t ta = type_for_sort(a), tb = type_for_sort(b);
	return ta - tb;
}

/**
 * Find a transform type
 */
static int type_find(const void *a, const void *b)
{
	return type_sort(a, b, NULL);
}

/**
 * Check if the given transform type is already in the set
 */
static bool contains_type(array_t *types, transform_type_t type)
{
	return array_bsearch(types, &type, type_find, NULL) != -1;
}

/**
 * Add the given transform type to the set
 */
static void add_type(array_t *types, transform_type_t type)
{
	if (!contains_type(types, type))
	{
		array_insert(types, ARRAY_TAIL, &type);
		array_sort(types, type_sort, NULL);
	}
}

/**
 * Merge two sets of transform types into a new array
 */
static array_t *merge_types(private_proposal_t *this, private_proposal_t *other)
{
	array_t *types;
	transform_type_t type;
	int i, count;

	count = max(array_count(this->types), array_count(other->types));
	types = array_create(sizeof(transform_type_t), count);

	for (i = 0; i < count; i++)
	{
		if (array_get(this->types, i, &type))
		{
			add_type(types, type);
		}
		if (array_get(other->types, i, &type))
		{
			add_type(types, type);
		}
	}
	return types;
}

/**
 * Remove the given transform type from the set
 */
static void remove_type(private_proposal_t *this, transform_type_t type)
{
	int i;

	i = array_bsearch(this->types, &type, type_find, NULL);
	if (i >= 0)
	{
		array_remove(this->types, i, NULL);
	}
}

/**
 * Struct used to store different kinds of algorithms.
 */
typedef struct {
	/** Type of the transform */
	transform_type_t type;
	/** algorithm identifier */
	uint16_t alg;
	/** key size in bits, or zero if not needed */
	uint16_t key_size;
} entry_t;

METHOD(proposal_t, add_algorithm, void,
	private_proposal_t *this, transform_type_t type,
	uint16_t alg, uint16_t key_size)
{
	entry_t entry = {
		.type = type,
		.alg = alg,
		.key_size = key_size,
	};

	array_insert(this->transforms, ARRAY_TAIL, &entry);
	add_type(this->types, type);
}

CALLBACK(alg_filter, bool,
	uintptr_t type, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	uint16_t *alg, *key_size;

	VA_ARGS_VGET(args, alg, key_size);

	while (orig->enumerate(orig, &entry))
	{
		if (entry->type != type)
		{
			continue;
		}
		if (alg)
		{
			*alg = entry->alg;
		}
		if (key_size)
		{
			*key_size = entry->key_size;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(proposal_t, create_enumerator, enumerator_t*,
	private_proposal_t *this, transform_type_t type)
{
	return enumerator_create_filter(
						array_create_enumerator(this->transforms),
						alg_filter, (void*)(uintptr_t)type, NULL);
}

METHOD(proposal_t, get_algorithm, bool,
	private_proposal_t *this, transform_type_t type,
	uint16_t *alg, uint16_t *key_size)
{
	enumerator_t *enumerator;
	bool found = FALSE;

	enumerator = create_enumerator(this, type);
	if (enumerator->enumerate(enumerator, alg, key_size))
	{
		found = TRUE;
	}
	enumerator->destroy(enumerator);

	return found;
}

METHOD(proposal_t, has_dh_group, bool,
	private_proposal_t *this, diffie_hellman_group_t group)
{
	bool found = FALSE, any = FALSE;
	enumerator_t *enumerator;
	uint16_t current;

	enumerator = create_enumerator(this, DIFFIE_HELLMAN_GROUP);
	while (enumerator->enumerate(enumerator, &current, NULL))
	{
		any = TRUE;
		if (current == group)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!any && group == MODP_NONE)
	{
		found = TRUE;
	}
	return found;
}

METHOD(proposal_t, promote_dh_group, bool,
	private_proposal_t *this, diffie_hellman_group_t group)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool found = FALSE;

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->type == DIFFIE_HELLMAN_GROUP &&
			entry->alg == group)
		{
			array_remove_at(this->transforms, enumerator);
			found = TRUE;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		entry_t entry = {
			.type = DIFFIE_HELLMAN_GROUP,
			.alg = group,
		};
		array_insert(this->transforms, ARRAY_HEAD, &entry);
	}
	return found;
}

METHOD(proposal_t, strip_dh, void,
	private_proposal_t *this, diffie_hellman_group_t keep)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool found = FALSE;

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->type == DIFFIE_HELLMAN_GROUP)
		{
			if (entry->alg != keep)
			{
				array_remove_at(this->transforms, enumerator);
			}
			else
			{
				found = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
	array_compress(this->transforms);

	if (keep == MODP_NONE || !found)
	{
		remove_type(this, DIFFIE_HELLMAN_GROUP);
		array_compress(this->types);
	}
}

/**
 * Select a matching proposal from this and other.
 */
static bool select_algo(private_proposal_t *this, proposal_t *other,
						transform_type_t type, bool priv, bool log,
						uint16_t *alg, uint16_t *ks)
{
	enumerator_t *e1, *e2;
	uint16_t alg1, alg2, ks1, ks2;
	bool found = FALSE, optional = FALSE;

	if (type == DIFFIE_HELLMAN_GROUP)
	{
		optional = this->protocol == PROTO_ESP || this->protocol == PROTO_AH;
	}

	e1 = create_enumerator(this, type);
	e2 = other->create_enumerator(other, type);
	if (!e1->enumerate(e1, &alg1, NULL))
	{
		if (!e2->enumerate(e2, &alg2, NULL))
		{
			found = TRUE;
		}
		else if (optional)
		{
			do
			{	/* if NONE is proposed, we accept the proposal */
				found = !alg2;
			}
			while (!found && e2->enumerate(e2, &alg2, NULL));
		}
	}
	else if (!e2->enumerate(e2, NULL, NULL))
	{
		if (optional)
		{
			do
			{	/* if NONE is proposed, we accept the proposal */
				found = !alg1;
			}
			while (!found && e1->enumerate(e1, &alg1, NULL));
		}
	}

	e1->destroy(e1);
	e1 = create_enumerator(this, type);
	/* compare algs, order of algs in "first" is preferred */
	while (!found && e1->enumerate(e1, &alg1, &ks1))
	{
		e2->destroy(e2);
		e2 = other->create_enumerator(other, type);
		while (e2->enumerate(e2, &alg2, &ks2))
		{
			if (alg1 == alg2 && ks1 == ks2)
			{
				if (!priv && alg1 >= 1024)
				{
					if (log)
					{
						DBG1(DBG_CFG, "an algorithm from private space would "
							 "match, but peer implementation is unknown, "
							 "skipped");
					}
					continue;
				}
				*alg = alg1;
				*ks = ks1;
				found = TRUE;
				break;
			}
		}
	}
	e1->destroy(e1);
	e2->destroy(e2);
	return found;
}

/**
 * Select algorithms from the given proposals, if selected is given, the result
 * is stored there and errors are logged.
 */
static bool select_algos(private_proposal_t *this, proposal_t *other,
						 proposal_t *selected, bool private)
{
	transform_type_t type;
	array_t *types;
	bool skip_integrity = FALSE;
	int i;

	types = merge_types(this, (private_proposal_t*)other);
	for (i = 0; i < array_count(types); i++)
	{
		uint16_t alg = 0, ks = 0;

		array_get(types, i, &type);
		if (type == INTEGRITY_ALGORITHM && skip_integrity)
		{
			continue;
		}
		if (select_algo(this, other, type, private, selected != NULL, &alg, &ks))
		{
			if (alg == 0 && type != EXTENDED_SEQUENCE_NUMBERS)
			{	/* 0 is "valid" for extended sequence numbers, for other
				 * transforms it either means NONE or is reserved */
				continue;
			}
			if (selected)
			{
				selected->add_algorithm(selected, type, alg, ks);
			}
			if (type == ENCRYPTION_ALGORITHM &&
				encryption_algorithm_is_aead(alg))
			{
				/* no integrity algorithm required, we have an AEAD */
				skip_integrity = TRUE;
			}
		}
		else
		{
			if (selected)
			{
				DBG2(DBG_CFG, "  no acceptable %N found", transform_type_names,
					 type);
			}
			array_destroy(types);
			return FALSE;
		}
	}
	array_destroy(types);
	return TRUE;
}

METHOD(proposal_t, select_proposal, proposal_t*,
	private_proposal_t *this, proposal_t *other, bool other_remote,
	bool private)
{
	proposal_t *selected;

	DBG2(DBG_CFG, "selecting proposal:");

	if (this->protocol != other->get_protocol(other))
	{
		DBG2(DBG_CFG, "  protocol mismatch, skipping");
		return NULL;
	}

	if (other_remote)
	{
		selected = proposal_create(this->protocol, other->get_number(other));
		selected->set_spi(selected, other->get_spi(other));
	}
	else
	{
		selected = proposal_create(this->protocol, this->number);
		selected->set_spi(selected, this->spi);
	}

	if (!select_algos(this, other, selected, private))
	{
		selected->destroy(selected);
		return NULL;
	}
	DBG2(DBG_CFG, "  proposal matches");
	return selected;
}

METHOD(proposal_t, matches, bool,
	private_proposal_t *this, proposal_t *other, bool private)
{
	if (this->protocol != other->get_protocol(other))
	{
		return FALSE;
	}
	return select_algos(this, other, NULL, private);
}

METHOD(proposal_t, get_protocol, protocol_id_t,
	private_proposal_t *this)
{
	return this->protocol;
}

METHOD(proposal_t, set_spi, void,
	private_proposal_t *this, uint64_t spi)
{
	this->spi = spi;
}

METHOD(proposal_t, get_spi, uint64_t,
	private_proposal_t *this)
{
	return this->spi;
}

/**
 * Check if two proposals have the same algorithms for a given transform type
 */
static bool algo_list_equals(private_proposal_t *this, proposal_t *other,
							 transform_type_t type)
{
	enumerator_t *e1, *e2;
	uint16_t alg1, alg2, ks1, ks2;
	bool equals = TRUE;

	e1 = create_enumerator(this, type);
	e2 = other->create_enumerator(other, type);
	while (e1->enumerate(e1, &alg1, &ks1))
	{
		if (!e2->enumerate(e2, &alg2, &ks2))
		{
			/* this has more algs */
			equals = FALSE;
			break;
		}
		if (alg1 != alg2 || ks1 != ks2)
		{
			equals = FALSE;
			break;
		}
	}
	if (e2->enumerate(e2, &alg2, &ks2))
	{
		/* other has more algs */
		equals = FALSE;
	}
	e1->destroy(e1);
	e2->destroy(e2);

	return equals;
}

METHOD(proposal_t, get_number, u_int,
	private_proposal_t *this)
{
	return this->number;
}

METHOD(proposal_t, equals, bool,
	private_proposal_t *this, proposal_t *other)
{
	transform_type_t type;
	array_t *types;
	int i;

	if (&this->public == other)
	{
		return TRUE;
	}

	types = merge_types(this, (private_proposal_t*)other);
	for (i = 0; i < array_count(types); i++)
	{
		array_get(types, i, &type);
		if (!algo_list_equals(this, other, type))
		{
			array_destroy(types);
			return FALSE;
		}
	}
	array_destroy(types);
	return TRUE;
}

METHOD(proposal_t, clone_, proposal_t*,
	private_proposal_t *this)
{
	private_proposal_t *clone;
	enumerator_t *enumerator;
	entry_t *entry;
	transform_type_t *type;

	clone = (private_proposal_t*)proposal_create(this->protocol, 0);

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		array_insert(clone->transforms, ARRAY_TAIL, entry);
	}
	enumerator->destroy(enumerator);
	enumerator = array_create_enumerator(this->types);
	while (enumerator->enumerate(enumerator, &type))
	{
		array_insert(clone->types, ARRAY_TAIL, type);
	}
	enumerator->destroy(enumerator);

	clone->spi = this->spi;
	clone->number = this->number;

	return &clone->public;
}

/**
 * Map integrity algorithms to the PRF functions using the same algorithm.
 */
static const struct {
	integrity_algorithm_t integ;
	pseudo_random_function_t prf;
} integ_prf_map[] = {
	{AUTH_HMAC_SHA1_96,					PRF_HMAC_SHA1					},
	{AUTH_HMAC_SHA1_160,				PRF_HMAC_SHA1					},
	{AUTH_HMAC_SHA2_256_128,			PRF_HMAC_SHA2_256				},
	{AUTH_HMAC_SHA2_384_192,			PRF_HMAC_SHA2_384				},
	{AUTH_HMAC_SHA2_512_256,			PRF_HMAC_SHA2_512				},
	{AUTH_HMAC_MD5_96,					PRF_HMAC_MD5					},
	{AUTH_HMAC_MD5_128,					PRF_HMAC_MD5					},
	{AUTH_AES_XCBC_96,					PRF_AES128_XCBC					},
	{AUTH_CAMELLIA_XCBC_96,				PRF_CAMELLIA128_XCBC			},
	{AUTH_AES_CMAC_96,					PRF_AES128_CMAC					},
};

/**
 * Remove all entries of the given transform type
 */
static void remove_transform(private_proposal_t *this, transform_type_t type)
{
	enumerator_t *e;
	entry_t *entry;

	e = array_create_enumerator(this->transforms);
	while (e->enumerate(e, &entry))
	{
		if (entry->type == type)
		{
			array_remove_at(this->transforms, e);
		}
	}
	e->destroy(e);
	remove_type(this, type);
}

/**
 * Checks the proposal read from a string.
 */
static bool check_proposal(private_proposal_t *this)
{
	enumerator_t *e;
	entry_t *entry;
	uint16_t alg, ks;
	bool all_aead = TRUE, any_aead = FALSE, any_enc = FALSE;
	int i;

	if (this->protocol == PROTO_IKE)
	{
		if (!get_algorithm(this, PSEUDO_RANDOM_FUNCTION, NULL, NULL))
		{	/* No explicit PRF found. We assume the same algorithm as used
			 * for integrity checking. */
			e = create_enumerator(this, INTEGRITY_ALGORITHM);
			while (e->enumerate(e, &alg, &ks))
			{
				for (i = 0; i < countof(integ_prf_map); i++)
				{
					if (alg == integ_prf_map[i].integ)
					{
						add_algorithm(this, PSEUDO_RANDOM_FUNCTION,
									  integ_prf_map[i].prf, 0);
						break;
					}
				}
			}
			e->destroy(e);
		}
		if (!get_algorithm(this, PSEUDO_RANDOM_FUNCTION, NULL, NULL))
		{
			DBG1(DBG_CFG, "a PRF algorithm is mandatory in IKE proposals");
			return FALSE;
		}
		/* remove MODP_NONE from IKE proposal */
		e = array_create_enumerator(this->transforms);
		while (e->enumerate(e, &entry))
		{
			if (entry->type == DIFFIE_HELLMAN_GROUP && !entry->alg)
			{
				array_remove_at(this->transforms, e);
			}
		}
		e->destroy(e);
		if (!get_algorithm(this, DIFFIE_HELLMAN_GROUP, NULL, NULL))
		{
			DBG1(DBG_CFG, "a DH group is mandatory in IKE proposals");
			return FALSE;
		}
	}
	else
	{	/* remove PRFs from ESP/AH proposals */
		remove_transform(this, PSEUDO_RANDOM_FUNCTION);
	}

	if (this->protocol == PROTO_IKE || this->protocol == PROTO_ESP)
	{
		e = create_enumerator(this, ENCRYPTION_ALGORITHM);
		while (e->enumerate(e, &alg, &ks))
		{
			any_enc = TRUE;
			if (encryption_algorithm_is_aead(alg))
			{
				any_aead = TRUE;
				continue;
			}
			all_aead = FALSE;
		}
		e->destroy(e);

		if (!any_enc)
		{
			DBG1(DBG_CFG, "an encryption algorithm is mandatory in %N proposals",
				 protocol_id_names, this->protocol);
			return FALSE;
		}
		else if (any_aead && !all_aead)
		{
			DBG1(DBG_CFG, "classic and combined-mode (AEAD) encryption "
				 "algorithms can't be contained in the same %N proposal",
				 protocol_id_names, this->protocol);
			return FALSE;
		}
		else if (all_aead)
		{	/* if all encryption algorithms in the proposal are AEADs,
			 * we MUST NOT propose any integrity algorithms */
			remove_transform(this, INTEGRITY_ALGORITHM);
		}
		else if (this->protocol == PROTO_IKE &&
				 !get_algorithm(this, INTEGRITY_ALGORITHM, NULL, NULL))
		{
			DBG1(DBG_CFG, "an integrity algorithm is mandatory in %N proposals "
				 "with classic (non-AEAD) encryption algorithms",
				 protocol_id_names, this->protocol);
			return FALSE;
		}
	}
	else
	{	/* AES-GMAC is parsed as encryption algorithm, so we map that to the
		 * proper integrity algorithm */
		e = array_create_enumerator(this->transforms);
		while (e->enumerate(e, &entry))
		{
			if (entry->type == ENCRYPTION_ALGORITHM)
			{
				if (entry->alg == ENCR_NULL_AUTH_AES_GMAC)
				{
					entry->type = INTEGRITY_ALGORITHM;
					ks = entry->key_size;
					entry->key_size = 0;
					switch (ks)
					{
						case 128:
							entry->alg = AUTH_AES_128_GMAC;
							continue;
						case 192:
							entry->alg = AUTH_AES_192_GMAC;
							continue;
						case 256:
							entry->alg = AUTH_AES_256_GMAC;
							continue;
						default:
							break;
					}
				}
				/* remove all other encryption algorithms */
				array_remove_at(this->transforms, e);
			}
		}
		e->destroy(e);
		remove_type(this, ENCRYPTION_ALGORITHM);

		if (!get_algorithm(this, INTEGRITY_ALGORITHM, NULL, NULL))
		{
			DBG1(DBG_CFG, "an integrity algorithm is mandatory in AH "
				 "proposals");
			return FALSE;
		}
	}

	if (this->protocol == PROTO_AH || this->protocol == PROTO_ESP)
	{
		if (!get_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NULL, NULL))
		{	/* ESN not specified, assume not supported */
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS, 0);
		}
	}

	array_compress(this->transforms);
	array_compress(this->types);
	return TRUE;
}

/**
 * add a algorithm identified by a string to the proposal.
 */
static bool add_string_algo(private_proposal_t *this, const char *alg)
{
	const proposal_token_t *token;

	token = lib->proposal->get_token(lib->proposal, alg);
	if (token == NULL)
	{
		DBG1(DBG_CFG, "algorithm '%s' not recognized", alg);
		return FALSE;
	}

	add_algorithm(this, token->type, token->algorithm, token->keysize);

	return TRUE;
}

/**
 * Print all algorithms of the given type
 */
static int print_alg(private_proposal_t *this, printf_hook_data_t *data,
					 transform_type_t type, bool *first)
{
	enumerator_t *enumerator;
	size_t written = 0;
	entry_t *entry;
	enum_name_t *names;

	names = transform_get_enum_names(type);

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		char *prefix = "/";

		if (type != entry->type)
		{
			continue;
		}
		if (*first)
		{
			prefix = "";
			*first = FALSE;
		}
		if (names)
		{
			written += print_in_hook(data, "%s%N", prefix, names, entry->alg);
		}
		else
		{
			written += print_in_hook(data, "%sUNKNOWN_%u_%u", prefix,
									 entry->type, entry->alg);
		}
		if (entry->key_size)
		{
			written += print_in_hook(data, "_%u", entry->key_size);
		}
	}
	enumerator->destroy(enumerator);
	return written;
}

/**
 * Described in header.
 */
int proposal_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						 const void *const *args)
{
	private_proposal_t *this = *((private_proposal_t**)(args[0]));
	linked_list_t *list = *((linked_list_t**)(args[0]));
	enumerator_t *enumerator;
	transform_type_t *type;
	size_t written = 0;
	bool first = TRUE;

	if (this == NULL)
	{
		return print_in_hook(data, "(null)");
	}

	if (spec->hash)
	{
		enumerator = list->create_enumerator(list);
		while (enumerator->enumerate(enumerator, &this))
		{	/* call recursively */
			if (first)
			{
				written += print_in_hook(data, "%P", this);
				first = FALSE;
			}
			else
			{
				written += print_in_hook(data, ", %P", this);
			}
		}
		enumerator->destroy(enumerator);
		return written;
	}

	written = print_in_hook(data, "%N:", protocol_id_names, this->protocol);
	enumerator = array_create_enumerator(this->types);
	while (enumerator->enumerate(enumerator, &type))
	{
		written += print_alg(this, data, *type, &first);
	}
	enumerator->destroy(enumerator);
	return written;
}

METHOD(proposal_t, destroy, void,
	private_proposal_t *this)
{
	array_destroy(this->transforms);
	array_destroy(this->types);
	free(this);
}

/*
 * Described in header
 */
proposal_t *proposal_create(protocol_id_t protocol, u_int number)
{
	private_proposal_t *this;

	INIT(this,
		.public = {
			.add_algorithm = _add_algorithm,
			.create_enumerator = _create_enumerator,
			.get_algorithm = _get_algorithm,
			.has_dh_group = _has_dh_group,
			.promote_dh_group = _promote_dh_group,
			.strip_dh = _strip_dh,
			.select = _select_proposal,
			.matches = _matches,
			.get_protocol = _get_protocol,
			.set_spi = _set_spi,
			.get_spi = _get_spi,
			.get_number = _get_number,
			.equals = _equals,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.protocol = protocol,
		.number = number,
		.transforms = array_create(sizeof(entry_t), 0),
		.types = array_create(sizeof(transform_type_t), 0),
	);

	return &this->public;
}

/**
 * Add supported IKE algorithms to proposal
 */
static bool proposal_add_supported_ike(private_proposal_t *this, bool aead)
{
	enumerator_t *enumerator;
	encryption_algorithm_t encryption;
	integrity_algorithm_t integrity;
	pseudo_random_function_t prf;
	diffie_hellman_group_t group;
	const char *plugin_name;

	if (aead)
	{
		/* Round 1 adds algorithms with at least 128 bit security strength */
		enumerator = lib->crypto->create_aead_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
		{
			switch (encryption)
			{
				case ENCR_AES_GCM_ICV16:
				case ENCR_AES_CCM_ICV16:
				case ENCR_CAMELLIA_CCM_ICV16:
					/* we assume that we support all AES/Camellia sizes */
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 128);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 192);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 256);
					break;
				case ENCR_CHACHA20_POLY1305:
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 0);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);

		/* Round 2 adds algorithms with less than 128 bit security strength */
		enumerator = lib->crypto->create_aead_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
		{
			switch (encryption)
			{
				case ENCR_AES_GCM_ICV12:
				case ENCR_AES_GCM_ICV8:
				case ENCR_AES_CCM_ICV12:
				case ENCR_AES_CCM_ICV8:
				case ENCR_CAMELLIA_CCM_ICV12:
				case ENCR_CAMELLIA_CCM_ICV8:
					/* we assume that we support all AES/Camellia sizes */
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 128);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 192);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 256);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);

		if (!array_count(this->transforms))
		{
			return FALSE;
		}
	}
	else
	{
		/* Round 1 adds algorithms with at least 128 bit security strength */
		enumerator = lib->crypto->create_crypter_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
		{
			switch (encryption)
			{
				case ENCR_AES_CBC:
				case ENCR_AES_CTR:
				case ENCR_CAMELLIA_CBC:
				case ENCR_CAMELLIA_CTR:
					/* we assume that we support all AES/Camellia sizes */
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 128);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 192);
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 256);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);

		/* Round 2 adds algorithms with less than 128 bit security strength */
		enumerator = lib->crypto->create_crypter_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
		{
			switch (encryption)
			{
				case ENCR_3DES:
					add_algorithm(this, ENCRYPTION_ALGORITHM, encryption, 0);
					break;
				case ENCR_DES:
					/* no, thanks */
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);

		if (!array_count(this->transforms))
		{
			return FALSE;
		}

		/* Round 1 adds algorithms with at least 128 bit security strength */
		enumerator = lib->crypto->create_signer_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &integrity, &plugin_name))
		{
			switch (integrity)
			{
				case AUTH_HMAC_SHA2_256_128:
				case AUTH_HMAC_SHA2_384_192:
				case AUTH_HMAC_SHA2_512_256:
					add_algorithm(this, INTEGRITY_ALGORITHM, integrity, 0);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);

		/* Round 2 adds algorithms with less than 128 bit security strength */
		enumerator = lib->crypto->create_signer_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &integrity, &plugin_name))
		{
			switch (integrity)
			{
				case AUTH_AES_XCBC_96:
				case AUTH_AES_CMAC_96:
				case AUTH_HMAC_SHA1_96:
					add_algorithm(this, INTEGRITY_ALGORITHM, integrity, 0);
					break;
				case AUTH_HMAC_MD5_96:
					/* no, thanks */
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}

	/* Round 1 adds algorithms with at least 128 bit security strength */
	enumerator = lib->crypto->create_prf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &prf, &plugin_name))
	{
		switch (prf)
		{
			case PRF_HMAC_SHA2_256:
			case PRF_HMAC_SHA2_384:
			case PRF_HMAC_SHA2_512:
			case PRF_AES128_XCBC:
			case PRF_AES128_CMAC:
				add_algorithm(this, PSEUDO_RANDOM_FUNCTION, prf, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* Round 2 adds algorithms with less than 128 bit security strength */
	enumerator = lib->crypto->create_prf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &prf, &plugin_name))
	{
		switch (prf)
		{
			case PRF_HMAC_SHA1:
				add_algorithm(this, PSEUDO_RANDOM_FUNCTION, prf, 0);
				break;
			case PRF_HMAC_MD5:
				/* no, thanks */
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* Round 1 adds ECC and NTRU algorithms with at least 128 bit security strength */
	enumerator = lib->crypto->create_dh_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		switch (group)
		{
			case ECP_256_BIT:
			case ECP_384_BIT:
			case ECP_521_BIT:
			case ECP_256_BP:
			case ECP_384_BP:
			case ECP_512_BP:
			case CURVE_25519:
			case CURVE_448:
			case NTRU_128_BIT:
			case NTRU_192_BIT:
			case NTRU_256_BIT:
			case NH_128_BIT:
				add_algorithm(this, DIFFIE_HELLMAN_GROUP, group, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* Round 2 adds other algorithms with at least 128 bit security strength */
	enumerator = lib->crypto->create_dh_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		switch (group)
		{
			case MODP_3072_BIT:
			case MODP_4096_BIT:
			case MODP_6144_BIT:
			case MODP_8192_BIT:
				add_algorithm(this, DIFFIE_HELLMAN_GROUP, group, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* Round 3 adds algorithms with less than 128 bit security strength */
	enumerator = lib->crypto->create_dh_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		switch (group)
		{
			case MODP_NULL:
				/* only for testing purposes */
				break;
			case MODP_768_BIT:
			case MODP_1024_BIT:
			case MODP_1536_BIT:
				/* weak */
				break;
			case MODP_1024_160:
			case MODP_2048_224:
			case MODP_2048_256:
				/* RFC 5114 primes are of questionable source */
				break;
			case ECP_224_BIT:
			case ECP_224_BP:
			case ECP_192_BIT:
			case NTRU_112_BIT:
				/* rarely used */
				break;
			case MODP_2048_BIT:
				add_algorithm(this, DIFFIE_HELLMAN_GROUP, group, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

/*
 * Described in header
 */
proposal_t *proposal_create_default(protocol_id_t protocol)
{
	private_proposal_t *this = (private_proposal_t*)proposal_create(protocol, 0);

	switch (protocol)
	{
		case PROTO_IKE:
			if (!proposal_add_supported_ike(this, FALSE))
			{
				destroy(this);
				return NULL;
			}
			break;
		case PROTO_ESP:
			add_algorithm(this, ENCRYPTION_ALGORITHM, ENCR_AES_CBC,          128);
			add_algorithm(this, ENCRYPTION_ALGORITHM, ENCR_AES_CBC,          192);
			add_algorithm(this, ENCRYPTION_ALGORITHM, ENCR_AES_CBC,          256);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_128,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_384_192,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_512_256,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA1_96,       0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_AES_XCBC_96,        0);
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS, 0);
			break;
		case PROTO_AH:
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_128,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_384_192,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_512_256,  0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA1_96,       0);
			add_algorithm(this, INTEGRITY_ALGORITHM,  AUTH_AES_XCBC_96,        0);
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS, 0);
			break;
		default:
			break;
	}
	return &this->public;
}

/*
 * Described in header
 */
proposal_t *proposal_create_default_aead(protocol_id_t protocol)
{
	private_proposal_t *this;

	switch (protocol)
	{
		case PROTO_IKE:
			this = (private_proposal_t*)proposal_create(protocol, 0);
			if (!proposal_add_supported_ike(this, TRUE))
			{
				destroy(this);
				return NULL;
			}
			return &this->public;
		case PROTO_ESP:
			/* we currently don't include any AEAD proposal for ESP, as we
			 * don't know if our kernel backend actually supports it. */
			return NULL;
		case PROTO_AH:
		default:
			return NULL;
	}
}

/*
 * Described in header
 */
proposal_t *proposal_create_from_string(protocol_id_t protocol, const char *algs)
{
	private_proposal_t *this;
	enumerator_t *enumerator;
	bool failed = TRUE;
	char *alg;

	this = (private_proposal_t*)proposal_create(protocol, 0);

	/* get all tokens, separated by '-' */
	enumerator = enumerator_create_token(algs, "-", " ");
	while (enumerator->enumerate(enumerator, &alg))
	{
		if (!add_string_algo(this, alg))
		{
			failed = TRUE;
			break;
		}
		failed = FALSE;
	}
	enumerator->destroy(enumerator);

	if (failed || !check_proposal(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
