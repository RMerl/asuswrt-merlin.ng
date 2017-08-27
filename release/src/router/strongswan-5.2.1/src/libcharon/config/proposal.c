/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2006-2010 Martin Willi
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

#include <string.h>

#include "proposal.h"

#include <daemon.h>
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
	 * senders SPI
	 */
	u_int64_t spi;

	/**
	 * Proposal number
	 */
	u_int number;
};

/**
 * Struct used to store different kinds of algorithms.
 */
typedef struct {
	/** Type of the transform */
	transform_type_t type;
	/** algorithm identifier */
	u_int16_t alg;
	/** key size in bits, or zero if not needed */
	u_int16_t key_size;
} entry_t;

METHOD(proposal_t, add_algorithm, void,
	private_proposal_t *this, transform_type_t type,
	u_int16_t alg, u_int16_t key_size)
{
	entry_t entry = {
		.type = type,
		.alg = alg,
		.key_size = key_size,
	};

	array_insert(this->transforms, ARRAY_TAIL, &entry);
}

/**
 * filter function for peer configs
 */
static bool alg_filter(uintptr_t type, entry_t **in, u_int16_t *alg,
					   void **unused, u_int16_t *key_size)
{
	entry_t *entry = *in;

	if (entry->type != type)
	{
		return FALSE;
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

METHOD(proposal_t, create_enumerator, enumerator_t*,
	private_proposal_t *this, transform_type_t type)
{
	return enumerator_create_filter(
						array_create_enumerator(this->transforms),
						(void*)alg_filter, (void*)(uintptr_t)type, NULL);
}

METHOD(proposal_t, get_algorithm, bool,
	private_proposal_t *this, transform_type_t type,
	u_int16_t *alg, u_int16_t *key_size)
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
	u_int16_t current;

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

METHOD(proposal_t, strip_dh, void,
	private_proposal_t *this, diffie_hellman_group_t keep)
{
	enumerator_t *enumerator;
	entry_t *entry;

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->type == DIFFIE_HELLMAN_GROUP &&
			entry->alg != keep)
		{
			array_remove_at(this->transforms, enumerator);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Select a matching proposal from this and other, insert into selected.
 */
static bool select_algo(private_proposal_t *this, proposal_t *other,
						proposal_t *selected, transform_type_t type, bool priv)
{
	enumerator_t *e1, *e2;
	u_int16_t alg1, alg2, ks1, ks2;
	bool found = FALSE, optional = FALSE;

	if (type == INTEGRITY_ALGORITHM &&
		selected->get_algorithm(selected, ENCRYPTION_ALGORITHM, &alg1, NULL) &&
		encryption_algorithm_is_aead(alg1))
	{
		/* no integrity algorithm required, we have an AEAD */
		return TRUE;
	}
	if (type == DIFFIE_HELLMAN_GROUP)
	{
		optional = this->protocol == PROTO_ESP || this->protocol == PROTO_AH;
	}

	e1 = create_enumerator(this, type);
	e2 = other->create_enumerator(other, type);
	if (!e1->enumerate(e1, NULL, NULL))
	{
		if (!e2->enumerate(e2, &alg2, NULL))
		{
			found = TRUE;
		}
		else if (optional)
		{
			do
			{	/* if the other peer proposes NONE, we accept the proposal */
				found = !alg2;
			}
			while (!found && e2->enumerate(e2, &alg2, NULL));
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
					/* accept private use algorithms only if requested */
					DBG1(DBG_CFG, "an algorithm from private space would match, "
						 "but peer implementation is unknown, skipped");
					continue;
				}
				/* ok, we have an algorithm */
				selected->add_algorithm(selected, type, alg1, ks1);
				found = TRUE;
				break;
			}
		}
	}
	/* no match in all comparisons */
	e1->destroy(e1);
	e2->destroy(e2);

	if (!found)
	{
		DBG2(DBG_CFG, "  no acceptable %N found", transform_type_names, type);
	}
	return found;
}

METHOD(proposal_t, select_proposal, proposal_t*,
	private_proposal_t *this, proposal_t *other, bool private)
{
	proposal_t *selected;

	DBG2(DBG_CFG, "selecting proposal:");

	if (this->protocol != other->get_protocol(other))
	{
		DBG2(DBG_CFG, "  protocol mismatch, skipping");
		return NULL;
	}

	selected = proposal_create(this->protocol, other->get_number(other));

	if (!select_algo(this, other, selected, ENCRYPTION_ALGORITHM, private) ||
		!select_algo(this, other, selected, PSEUDO_RANDOM_FUNCTION, private) ||
		!select_algo(this, other, selected, INTEGRITY_ALGORITHM, private) ||
		!select_algo(this, other, selected, DIFFIE_HELLMAN_GROUP, private) ||
		!select_algo(this, other, selected, EXTENDED_SEQUENCE_NUMBERS, private))
	{
		selected->destroy(selected);
		return NULL;
	}

	DBG2(DBG_CFG, "  proposal matches");

	selected->set_spi(selected, other->get_spi(other));

	return selected;
}

METHOD(proposal_t, get_protocol, protocol_id_t,
	private_proposal_t *this)
{
	return this->protocol;
}

METHOD(proposal_t, set_spi, void,
	private_proposal_t *this, u_int64_t spi)
{
	this->spi = spi;
}

METHOD(proposal_t, get_spi, u_int64_t,
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
	u_int16_t alg1, alg2, ks1, ks2;
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
	if (&this->public == other)
	{
		return TRUE;
	}
	return (
		algo_list_equals(this, other, ENCRYPTION_ALGORITHM) &&
		algo_list_equals(this, other, INTEGRITY_ALGORITHM) &&
		algo_list_equals(this, other, PSEUDO_RANDOM_FUNCTION) &&
		algo_list_equals(this, other, DIFFIE_HELLMAN_GROUP) &&
		algo_list_equals(this, other, EXTENDED_SEQUENCE_NUMBERS));
}

METHOD(proposal_t, clone_, proposal_t*,
	private_proposal_t *this)
{
	private_proposal_t *clone;
	enumerator_t *enumerator;
	entry_t *entry;

	clone = (private_proposal_t*)proposal_create(this->protocol, 0);

	enumerator = array_create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &entry))
	{
		array_insert(clone->transforms, ARRAY_TAIL, entry);
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
	{AUTH_HMAC_SHA2_256_128,			PRF_HMAC_SHA2_256				},
	{AUTH_HMAC_SHA2_384_192,			PRF_HMAC_SHA2_384				},
	{AUTH_HMAC_SHA2_512_256,			PRF_HMAC_SHA2_512				},
	{AUTH_HMAC_MD5_96,					PRF_HMAC_MD5					},
	{AUTH_AES_XCBC_96,					PRF_AES128_XCBC					},
	{AUTH_CAMELLIA_XCBC_96,				PRF_CAMELLIA128_XCBC			},
	{AUTH_AES_CMAC_96,					PRF_AES128_CMAC					},
};

/**
 * Checks the proposal read from a string.
 */
static void check_proposal(private_proposal_t *this)
{
	enumerator_t *e;
	entry_t *entry;
	u_int16_t alg, ks;
	bool all_aead = TRUE;
	int i;

	if (this->protocol == PROTO_IKE)
	{
		e = create_enumerator(this, PSEUDO_RANDOM_FUNCTION);
		if (!e->enumerate(e, &alg, &ks))
		{
			/* No explicit PRF found. We assume the same algorithm as used
			 * for integrity checking */
			e->destroy(e);
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
		}
		e->destroy(e);
	}

	if (this->protocol == PROTO_ESP)
	{
		e = create_enumerator(this, ENCRYPTION_ALGORITHM);
		while (e->enumerate(e, &alg, &ks))
		{
			if (!encryption_algorithm_is_aead(alg))
			{
				all_aead = FALSE;
				break;
			}
		}
		e->destroy(e);

		if (all_aead)
		{
			/* if all encryption algorithms in the proposal are AEADs,
			 * we MUST NOT propose any integrity algorithms */
			e = array_create_enumerator(this->transforms);
			while (e->enumerate(e, &entry))
			{
				if (entry->type == INTEGRITY_ALGORITHM)
				{
					array_remove_at(this->transforms, e);
				}
			}
			e->destroy(e);
		}
	}

	if (this->protocol == PROTO_AH || this->protocol == PROTO_ESP)
	{
		e = create_enumerator(this, EXTENDED_SEQUENCE_NUMBERS);
		if (!e->enumerate(e, NULL, NULL))
		{	/* ESN not specified, assume not supported */
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS, 0);
		}
		e->destroy(e);
	}

	array_compress(this->transforms);
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
 * print all algorithms of a kind to buffer
 */
static int print_alg(private_proposal_t *this, printf_hook_data_t *data,
					 u_int kind, void *names, bool *first)
{
	enumerator_t *enumerator;
	size_t written = 0;
	u_int16_t alg, size;

	enumerator = create_enumerator(this, kind);
	while (enumerator->enumerate(enumerator, &alg, &size))
	{
		if (*first)
		{
			written += print_in_hook(data, "%N", names, alg);
			*first = FALSE;
		}
		else
		{
			written += print_in_hook(data, "/%N", names, alg);
		}
		if (size)
		{
			written += print_in_hook(data, "_%u", size);
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
		{	/* call recursivly */
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
	written += print_alg(this, data, ENCRYPTION_ALGORITHM,
						 encryption_algorithm_names, &first);
	written += print_alg(this, data, INTEGRITY_ALGORITHM,
						 integrity_algorithm_names, &first);
	written += print_alg(this, data, PSEUDO_RANDOM_FUNCTION,
						 pseudo_random_function_names, &first);
	written += print_alg(this, data, DIFFIE_HELLMAN_GROUP,
						 diffie_hellman_group_names, &first);
	written += print_alg(this, data, EXTENDED_SEQUENCE_NUMBERS,
						 extended_sequence_numbers_names, &first);
	return written;
}

METHOD(proposal_t, destroy, void,
	private_proposal_t *this)
{
	array_destroy(this->transforms);
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
			.strip_dh = _strip_dh,
			.select = _select_proposal,
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
		enumerator = lib->crypto->create_aead_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
		{
			switch (encryption)
			{
				case ENCR_AES_CCM_ICV8:
				case ENCR_AES_CCM_ICV12:
				case ENCR_AES_CCM_ICV16:
				case ENCR_AES_GCM_ICV8:
				case ENCR_AES_GCM_ICV12:
				case ENCR_AES_GCM_ICV16:
				case ENCR_CAMELLIA_CCM_ICV8:
				case ENCR_CAMELLIA_CCM_ICV12:
				case ENCR_CAMELLIA_CCM_ICV16:
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

		enumerator = lib->crypto->create_signer_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &integrity, &plugin_name))
		{
			switch (integrity)
			{
				case AUTH_HMAC_SHA1_96:
				case AUTH_HMAC_SHA2_256_128:
				case AUTH_HMAC_SHA2_384_192:
				case AUTH_HMAC_SHA2_512_256:
				case AUTH_HMAC_MD5_96:
				case AUTH_AES_XCBC_96:
				case AUTH_AES_CMAC_96:
					add_algorithm(this, INTEGRITY_ALGORITHM, integrity, 0);
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}

	enumerator = lib->crypto->create_prf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &prf, &plugin_name))
	{
		switch (prf)
		{
			case PRF_HMAC_SHA1:
			case PRF_HMAC_SHA2_256:
			case PRF_HMAC_SHA2_384:
			case PRF_HMAC_SHA2_512:
			case PRF_HMAC_MD5:
			case PRF_AES128_XCBC:
			case PRF_AES128_CMAC:
				add_algorithm(this, PSEUDO_RANDOM_FUNCTION, prf, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	enumerator = lib->crypto->create_dh_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		switch (group)
		{
			case MODP_NULL:
				/* only for testing purposes */
				break;
			case MODP_768_BIT:
				/* weak */
				break;
			case MODP_1024_BIT:
			case MODP_1536_BIT:
			case MODP_2048_BIT:
			case MODP_3072_BIT:
			case MODP_4096_BIT:
			case MODP_8192_BIT:
			case ECP_256_BIT:
			case ECP_384_BIT:
			case ECP_521_BIT:
			case MODP_1024_160:
			case MODP_2048_224:
			case MODP_2048_256:
			case ECP_192_BIT:
			case ECP_224_BIT:
			case ECP_224_BP:
			case ECP_256_BP:
			case ECP_384_BP:
			case ECP_512_BP:
			case NTRU_112_BIT:
			case NTRU_128_BIT:
			case NTRU_192_BIT:
			case NTRU_256_BIT:
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
			add_algorithm(this, ENCRYPTION_ALGORITHM,   ENCR_AES_CBC,         128);
			add_algorithm(this, ENCRYPTION_ALGORITHM,   ENCR_AES_CBC,         192);
			add_algorithm(this, ENCRYPTION_ALGORITHM,   ENCR_AES_CBC,         256);
			add_algorithm(this, ENCRYPTION_ALGORITHM,   ENCR_3DES,              0);
			add_algorithm(this, ENCRYPTION_ALGORITHM,   ENCR_BLOWFISH,        256);
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_HMAC_SHA1_96,      0);
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_AES_XCBC_96,       0);
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_HMAC_MD5_96,       0);
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS,  0);
			break;
		case PROTO_AH:
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_HMAC_SHA1_96,      0);
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_AES_XCBC_96,       0);
			add_algorithm(this, INTEGRITY_ALGORITHM,    AUTH_HMAC_MD5_96,       0);
			add_algorithm(this, EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS,  0);
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

	if (failed)
	{
		destroy(this);
		return NULL;
	}

	check_proposal(this);

	return &this->public;
}
