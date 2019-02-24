/*
 * Copyright (C) 2012-2014 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
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

#include <stddef.h>

#include "proposal_substructure.h"

#include <encoding/payloads/encodings.h>
#include <encoding/payloads/transform_substructure.h>
#include <library.h>
#include <collections/linked_list.h>
#include <daemon.h>

/**
 * IKEv2 Value for a proposal payload.
 */
#define PROPOSAL_TYPE_VALUE 2

typedef struct private_proposal_substructure_t private_proposal_substructure_t;

/**
 * Private data of an proposal_substructure_t object.
 */
struct private_proposal_substructure_t {

	/**
	 * Public proposal_substructure_t interface.
	 */
	proposal_substructure_t public;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;

	/**
	 * reserved byte
	 */
	uint8_t reserved;

	/**
	 * Length of this payload.
	 */
	uint16_t proposal_length;

	/**
	 * Proposal number.
	 */
	uint8_t proposal_number;

	/**
	 * Protocol ID.
	 */
	uint8_t protocol_id;

	/**
	 * SPI size of the following SPI.
	 */
	uint8_t  spi_size;

	/**
	 * Number of transforms.
	 */
	uint8_t  transforms_count;

	/**
	 * SPI is stored as chunk.
	 */
	chunk_t spi;

	/**
	 * Transforms are stored in a linked_list_t.
	 */
	linked_list_t *transforms;

	/**
	 * Type of this payload, PLV2_PROPOSAL_SUBSTRUCTURE or PLV1_PROPOSAL_SUBSTRUCTURE
	 */
	payload_type_t type;
};

/**
 * Encoding rules for a IKEv1 Proposal substructure.
 */
static encoding_rule_t encodings_v1[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, next_payload)		},
	/* 1 Reserved Byte */
	{ RESERVED_BYTE,	offsetof(private_proposal_substructure_t, reserved)			},
	/* Length of the whole proposal substructure payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_proposal_substructure_t, proposal_length)	},
	/* proposal number is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, proposal_number)	},
	/* protocol ID is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, protocol_id)		},
	/* SPI Size has its own type */
	{ SPI_SIZE,			offsetof(private_proposal_substructure_t, spi_size)			},
	/* Number of transforms is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, transforms_count)	},
	/* SPI is a chunk of variable size*/
	{ SPI,				offsetof(private_proposal_substructure_t, spi)				},
	/* Transforms are stored in a transform substructure list */
	{ PAYLOAD_LIST + PLV1_TRANSFORM_SUBSTRUCTURE,
						offsetof(private_proposal_substructure_t, transforms)		},
};

/**
 * Encoding rules for a IKEv2 Proposal substructure.
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, next_payload)		},
	/* 1 Reserved Byte */
	{ RESERVED_BYTE,	offsetof(private_proposal_substructure_t, reserved)			},
	/* Length of the whole proposal substructure payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_proposal_substructure_t, proposal_length)	},
	/* proposal number is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, proposal_number)	},
	/* protocol ID is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, protocol_id)		},
	/* SPI Size has its own type */
	{ SPI_SIZE,			offsetof(private_proposal_substructure_t, spi_size)			},
	/* Number of transforms is a number of 8 bit */
	{ U_INT_8,			offsetof(private_proposal_substructure_t, transforms_count)	},
	/* SPI is a chunk of variable size*/
	{ SPI,				offsetof(private_proposal_substructure_t, spi)				},
	/* Transforms are stored in a transform substructure list */
	{ PAYLOAD_LIST + PLV2_TRANSFORM_SUBSTRUCTURE,
						offsetof(private_proposal_substructure_t, transforms)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! 0 (last) or 2 !   RESERVED    !         Proposal Length       !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Proposal #    !  Protocol ID  !    SPI Size   !# of Transforms!
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ~                        SPI (variable)                         ~
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                        <Transforms>                           ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * Encryption.
 */
typedef enum {
	IKEV1_ENCR_DES_CBC = 1,
	IKEV1_ENCR_IDEA_CBC = 2,
	IKEV1_ENCR_BLOWFISH_CBC = 3,
	IKEV1_ENCR_RC5_R16_B64_CBC = 4,
	IKEV1_ENCR_3DES_CBC = 5,
	IKEV1_ENCR_CAST_CBC = 6,
	IKEV1_ENCR_AES_CBC = 7,
	IKEV1_ENCR_CAMELLIA_CBC = 8,
	/* FreeS/WAN proprietary */
	IKEV1_ENCR_SERPENT_CBC = 65004,
	IKEV1_ENCR_TWOFISH_CBC = 65005,
} ikev1_encryption_t;

/**
 * IKEv1 hash.
 */
typedef enum {
	IKEV1_HASH_MD5 = 1,
	IKEV1_HASH_SHA1 = 2,
	IKEV1_HASH_TIGER = 3,
	IKEV1_HASH_SHA2_256 = 4,
	IKEV1_HASH_SHA2_384 = 5,
	IKEV1_HASH_SHA2_512 = 6,
} ikev1_hash_t;

/**
 * IKEv1 Transform ID IKE.
 */
typedef enum {
	IKEV1_TRANSID_KEY_IKE = 1,
} ikev1_ike_transid_t;

/**
 * IKEv1 Transform ID ESP encryption algorithm.
 */
typedef enum {
	IKEV1_ESP_ENCR_DES_IV64 = 1,
	IKEV1_ESP_ENCR_DES = 2,
	IKEV1_ESP_ENCR_3DES = 3,
	IKEV1_ESP_ENCR_RC5 = 4,
	IKEV1_ESP_ENCR_IDEA = 5,
	IKEV1_ESP_ENCR_CAST = 6,
	IKEV1_ESP_ENCR_BLOWFISH = 7,
	IKEV1_ESP_ENCR_3IDEA = 8,
	IKEV1_ESP_ENCR_DES_IV32 = 9,
	IKEV1_ESP_ENCR_RC4 = 10,
	IKEV1_ESP_ENCR_NULL = 11,
	IKEV1_ESP_ENCR_AES_CBC = 12,
	IKEV1_ESP_ENCR_AES_CTR = 13,
	IKEV1_ESP_ENCR_AES_CCM_8 = 14,
	IKEV1_ESP_ENCR_AES_CCM_12 = 15,
	IKEV1_ESP_ENCR_AES_CCM_16 = 16,
	IKEV1_ESP_ENCR_AES_GCM_8 = 18,
	IKEV1_ESP_ENCR_AES_GCM_12 = 19,
	IKEV1_ESP_ENCR_AES_GCM_16 = 20,
	IKEV1_ESP_ENCR_SEED_CBC = 21,
	IKEV1_ESP_ENCR_CAMELLIA = 22,
	IKEV1_ESP_ENCR_NULL_AUTH_AES_GMAC = 23,
	/* FreeS/WAN proprietary */
	IKEV1_ESP_ENCR_SERPENT = 252,
	IKEV1_ESP_ENCR_TWOFISH = 253,
} ikev1_esp_transid_t;

/**
 * IKEv1 Transform ID AH authentication algorithm.
 */
typedef enum {
	IKEV1_AH_HMAC_MD5 = 2,
	IKEV1_AH_HMAC_SHA = 3,
	IKEV1_AH_DES_MAC = 4,
	IKEV1_AH_HMAC_SHA2_256 = 5,
	IKEV1_AH_HMAC_SHA2_384 = 6,
	IKEV1_AH_HMAC_SHA2_512 = 7,
	IKEV1_AH_RIPEMD = 8,
	IKEV1_AH_AES_XCBC_MAC = 9,
	IKEV1_AH_RSA = 10,
	IKEV1_AH_AES_128_GMAC = 11,
	IKEV1_AH_AES_192_GMAC = 12,
	IKEV1_AH_AES_256_GMAC = 13,
} ikev1_ah_transid_t;

/**
 * IKEv1 authentication algorithm.
 */
typedef enum {
	IKEV1_AUTH_HMAC_MD5 = 1,
	IKEV1_AUTH_HMAC_SHA = 2,
	IKEV1_AUTH_DES_MAC = 3,
	IKEV1_AUTH_KPDK = 4,
	IKEV1_AUTH_HMAC_SHA2_256 = 5,
	IKEV1_AUTH_HMAC_SHA2_384 = 6,
	IKEV1_AUTH_HMAC_SHA2_512 = 7,
	IKEV1_AUTH_HMAC_RIPEMD = 8,
	IKEV1_AUTH_AES_XCBC_MAC = 9,
	IKEV1_AUTH_SIG_RSA = 10,
	IKEV1_AUTH_AES_128_GMAC = 11,
	IKEV1_AUTH_AES_192_GMAC = 12,
	IKEV1_AUTH_AES_256_GMAC = 13,
} ikev1_auth_algo_t;

/**
 * IKEv1 ESP Encapsulation mode.
 */
typedef enum {
  IKEV1_ENCAP_TUNNEL = 1,
  IKEV1_ENCAP_TRANSPORT = 2,
  IKEV1_ENCAP_UDP_TUNNEL = 3,
  IKEV1_ENCAP_UDP_TRANSPORT = 4,
  IKEV1_ENCAP_UDP_TUNNEL_DRAFT_00_03 = 61443,
  IKEV1_ENCAP_UDP_TRANSPORT_DRAFT_00_03 = 61444,
} ikev1_esp_encap_t;

/**
 * IKEv1 Life duration types.
 */
typedef enum {
	IKEV1_LIFE_TYPE_SECONDS = 1,
	IKEV1_LIFE_TYPE_KILOBYTES = 2,
} ikev1_life_type_t;

/**
 * IKEv1 authentication methods
 */
typedef enum {
	IKEV1_AUTH_PSK = 1,
	IKEV1_AUTH_DSS_SIG = 2,
	IKEV1_AUTH_RSA_SIG = 3,
	IKEV1_AUTH_RSA_ENC = 4,
	IKEV1_AUTH_RSA_ENC_REV = 5,
	IKEV1_AUTH_ECDSA_256 = 9,
	IKEV1_AUTH_ECDSA_384 = 10,
	IKEV1_AUTH_ECDSA_521 = 11,
	/* XAuth Modes */
	IKEV1_AUTH_XAUTH_INIT_PSK = 65001,
	IKEV1_AUTH_XAUTH_RESP_PSK = 65002,
	IKEV1_AUTH_XAUTH_INIT_DSS = 65003,
	IKEV1_AUTH_XAUTH_RESP_DSS = 65004,
	IKEV1_AUTH_XAUTH_INIT_RSA = 65005,
	IKEV1_AUTH_XAUTH_RESP_RSA = 65006,
	IKEV1_AUTH_XAUTH_INIT_RSA_ENC = 65007,
	IKEV1_AUTH_XAUTH_RESP_RSA_ENC = 65008,
	IKEV1_AUTH_XAUTH_INIT_RSA_ENC_REV = 65009,
	IKEV1_AUTH_XAUTH_RESP_RSA_ENC_REV = 65010,
	/* Hybrid Modes */
	IKEV1_AUTH_HYBRID_INIT_RSA = 64221,
	IKEV1_AUTH_HYBRID_RESP_RSA = 64222,
	IKEV1_AUTH_HYBRID_INIT_DSS = 64223,
	IKEV1_AUTH_HYBRID_RESP_DSS = 64224,
} ikev1_auth_method_t;

/**
 * IKEv1 IPComp transform IDs
 */
typedef enum {
	IKEV1_IPCOMP_OUI = 1,
	IKEV1_IPCOMP_DEFLATE = 2,
	IKEV1_IPCOMP_LZS = 3,
} ikev1_ipcomp_transform_t;

METHOD(payload_t, verify, status_t,
	private_proposal_substructure_t *this)
{
	status_t status = SUCCESS;
	enumerator_t *enumerator;
	payload_t *current;

	if (this->next_payload != PL_NONE && this->next_payload != 2)
	{
		/* must be 0 or 2 */
		DBG1(DBG_ENC, "inconsistent next payload");
		return FAILED;
	}
	if (this->transforms_count != this->transforms->get_count(this->transforms))
	{
		/* must be the same! */
		DBG1(DBG_ENC, "transform count invalid");
		return FAILED;
	}

	switch (this->protocol_id)
	{
		case PROTO_IPCOMP:
			if (this->spi.len != 2 && this->spi.len != 4)
			{
				DBG1(DBG_ENC, "invalid CPI length in IPCOMP proposal");
				return FAILED;
			}
			break;
		case PROTO_AH:
		case PROTO_ESP:
			if (this->spi.len != 4)
			{
				DBG1(DBG_ENC, "invalid SPI length in %N proposal",
					 protocol_id_names, this->protocol_id);
				return FAILED;
			}
			break;
		case PROTO_IKE:
			if (this->type == PLV1_PROPOSAL_SUBSTRUCTURE)
			{
				if (this->spi.len <= 16)
				{	/* according to RFC 2409, section 3.5 anything between
					 * 0 and 16 is fine */
					break;
				}
			}
			else if (this->spi.len == 0 || this->spi.len  == 8)
			{
				break;
			}
			DBG1(DBG_ENC, "invalid SPI length in IKE proposal");
			return FAILED;
		default:
			break;
	}
	enumerator = this->transforms->create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &current))
	{
		status = current->verify(current);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "TRANSFORM_SUBSTRUCTURE verification failed");
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* proposal number is checked in SA payload */
	return status;
}

METHOD(payload_t, get_encoding_rules, int,
	private_proposal_substructure_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_PROPOSAL_SUBSTRUCTURE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_proposal_substructure_t *this)
{
	return 8 + this->spi_size;
}

METHOD(payload_t, get_type, payload_type_t,
	private_proposal_substructure_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_proposal_substructure_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_proposal_substructure_t *this, payload_type_t type)
{
}

/**
 * (re-)compute the length of the payload.
 */
static void compute_length(private_proposal_substructure_t *this)
{
	enumerator_t *enumerator;
	payload_t *transform;

	this->transforms_count = 0;
	this->proposal_length = get_header_length(this);
	enumerator = this->transforms->create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &transform))
	{
		this->proposal_length += transform->get_length(transform);
		this->transforms_count++;
	}
	enumerator->destroy(enumerator);
}

METHOD(payload_t, get_length, size_t,
	private_proposal_substructure_t *this)
{
	return this->proposal_length;
}

/**
 * Add a transform substructure to the proposal
 */
static void add_transform_substructure(private_proposal_substructure_t *this,
									   transform_substructure_t *transform)
{
	if (this->transforms->get_count(this->transforms) > 0)
	{
		transform_substructure_t *last;

		this->transforms->get_last(this->transforms, (void **)&last);
		last->set_is_last_transform(last, FALSE);
	}
	transform->set_is_last_transform(transform,TRUE);
	this->transforms->insert_last(this->transforms, transform);
	compute_length(this);
}

METHOD(proposal_substructure_t, set_is_last_proposal, void,
	private_proposal_substructure_t *this, bool is_last)
{
	this->next_payload = is_last ? 0 : PROPOSAL_TYPE_VALUE;
}

METHOD(proposal_substructure_t, set_proposal_number, void,
	private_proposal_substructure_t *this,uint8_t proposal_number)
{
	this->proposal_number = proposal_number;
}

METHOD(proposal_substructure_t, get_proposal_number, uint8_t,
	private_proposal_substructure_t *this)
{
	return this->proposal_number;
}

METHOD(proposal_substructure_t, set_protocol_id, void,
	private_proposal_substructure_t *this,uint8_t protocol_id)
{
	this->protocol_id = protocol_id;
}

METHOD(proposal_substructure_t, get_protocol_id, uint8_t,
	private_proposal_substructure_t *this)
{
	return this->protocol_id;
}

METHOD(proposal_substructure_t, set_spi, void,
	private_proposal_substructure_t *this, chunk_t spi)
{
	free(this->spi.ptr);
	this->spi = chunk_clone(spi);
	this->spi_size = spi.len;
	compute_length(this);
}

METHOD(proposal_substructure_t, get_spi, chunk_t,
	private_proposal_substructure_t *this)
{
	return this->spi;
}

METHOD(proposal_substructure_t, get_cpi, bool,
	private_proposal_substructure_t *this, uint16_t *cpi)
{

	transform_substructure_t *transform;
	enumerator_t *enumerator;

	if (this->protocol_id != PROTO_IPCOMP)
	{
		return FALSE;
	}

	enumerator = this->transforms->create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &transform))
	{
		if (transform->get_transform_id(transform) == IKEV1_IPCOMP_DEFLATE)
		{
			if (cpi)
			{
				*cpi = htons(untoh16(this->spi.ptr + this->spi.len - 2));
			}
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

/**
 * Add a transform to a proposal for IKEv2
 */
static void add_to_proposal_v2(proposal_t *proposal,
							   transform_substructure_t *transform)
{
	transform_attribute_t *tattr;
	enumerator_t *enumerator;
	uint16_t key_length = 0;

	enumerator = transform->create_attribute_enumerator(transform);
	while (enumerator->enumerate(enumerator, &tattr))
	{
		if (tattr->get_attribute_type(tattr) == TATTR_IKEV2_KEY_LENGTH)
		{
			key_length = tattr->get_value(tattr);
			break;
		}
	}
	enumerator->destroy(enumerator);

	proposal->add_algorithm(proposal,
						transform->get_transform_type_or_number(transform),
						transform->get_transform_id(transform), key_length);
}

/**
 * Map IKEv1 to IKEv2 algorithms
 */
typedef struct {
	uint16_t ikev1;
	uint16_t ikev2;
} algo_map_t;

/**
 * Encryption algorithm mapping
 */
static algo_map_t map_encr[] = {
	{ IKEV1_ENCR_DES_CBC,		ENCR_DES },
	{ IKEV1_ENCR_IDEA_CBC,		ENCR_IDEA },
	{ IKEV1_ENCR_BLOWFISH_CBC,	ENCR_BLOWFISH },
	{ IKEV1_ENCR_3DES_CBC,		ENCR_3DES },
	{ IKEV1_ENCR_CAST_CBC,		ENCR_CAST },
	{ IKEV1_ENCR_AES_CBC,		ENCR_AES_CBC },
	{ IKEV1_ENCR_CAMELLIA_CBC,	ENCR_CAMELLIA_CBC },
	{ IKEV1_ENCR_SERPENT_CBC,	ENCR_SERPENT_CBC },
	{ IKEV1_ENCR_TWOFISH_CBC,	ENCR_TWOFISH_CBC },
};

/**
 * Integrity algorithm mapping
 */
static algo_map_t map_integ[] = {
	{ IKEV1_HASH_MD5,			AUTH_HMAC_MD5_96 },
	{ IKEV1_HASH_SHA1,			AUTH_HMAC_SHA1_96 },
	{ IKEV1_HASH_SHA2_256,		AUTH_HMAC_SHA2_256_128 },
	{ IKEV1_HASH_SHA2_384,		AUTH_HMAC_SHA2_384_192 },
	{ IKEV1_HASH_SHA2_512,		AUTH_HMAC_SHA2_512_256 },
};

/**
 * PRF algorithm mapping
 */
static algo_map_t map_prf[] = {
	{ IKEV1_HASH_MD5,			PRF_HMAC_MD5 },
	{ IKEV1_HASH_SHA1,			PRF_HMAC_SHA1 },
	{ IKEV1_HASH_SHA2_256,		PRF_HMAC_SHA2_256 },
	{ IKEV1_HASH_SHA2_384,		PRF_HMAC_SHA2_384 },
	{ IKEV1_HASH_SHA2_512,		PRF_HMAC_SHA2_512 },
};

/**
 * ESP encryption algorithm mapping
 */
static algo_map_t map_esp[] = {
	{ IKEV1_ESP_ENCR_DES_IV64,				ENCR_DES_IV64 },
	{ IKEV1_ESP_ENCR_DES,					ENCR_DES },
	{ IKEV1_ESP_ENCR_3DES,					ENCR_3DES },
	{ IKEV1_ESP_ENCR_RC5,					ENCR_RC5 },
	{ IKEV1_ESP_ENCR_IDEA,					ENCR_IDEA },
	{ IKEV1_ESP_ENCR_CAST,					ENCR_CAST },
	{ IKEV1_ESP_ENCR_BLOWFISH,				ENCR_BLOWFISH },
	{ IKEV1_ESP_ENCR_3IDEA,					ENCR_3IDEA },
	{ IKEV1_ESP_ENCR_DES_IV32,				ENCR_DES_IV32 },
	{ IKEV1_ESP_ENCR_NULL,					ENCR_NULL },
	{ IKEV1_ESP_ENCR_AES_CBC,				ENCR_AES_CBC },
	{ IKEV1_ESP_ENCR_AES_CTR,				ENCR_AES_CTR },
	{ IKEV1_ESP_ENCR_AES_CCM_8,				ENCR_AES_CCM_ICV8 },
	{ IKEV1_ESP_ENCR_AES_CCM_12,			ENCR_AES_CCM_ICV12 },
	{ IKEV1_ESP_ENCR_AES_CCM_16,			ENCR_AES_CCM_ICV16 },
	{ IKEV1_ESP_ENCR_AES_GCM_8,				ENCR_AES_GCM_ICV8 },
	{ IKEV1_ESP_ENCR_AES_GCM_12,			ENCR_AES_GCM_ICV12 },
	{ IKEV1_ESP_ENCR_AES_GCM_16,			ENCR_AES_GCM_ICV16 },
	{ IKEV1_ESP_ENCR_CAMELLIA,				ENCR_CAMELLIA_CBC },
	{ IKEV1_ESP_ENCR_NULL_AUTH_AES_GMAC,	ENCR_NULL_AUTH_AES_GMAC },
	{ IKEV1_ESP_ENCR_SERPENT,				ENCR_SERPENT_CBC },
	{ IKEV1_ESP_ENCR_TWOFISH,				ENCR_TWOFISH_CBC },
};

/**
 * AH authentication algorithm mapping
 */
static algo_map_t map_ah[] = {
	{ IKEV1_AH_HMAC_MD5,		AUTH_HMAC_MD5_96 },
	{ IKEV1_AH_HMAC_SHA,		AUTH_HMAC_SHA1_96 },
	{ IKEV1_AH_DES_MAC,			AUTH_DES_MAC },
	{ IKEV1_AH_HMAC_SHA2_256,	AUTH_HMAC_SHA2_256_128 },
	{ IKEV1_AH_HMAC_SHA2_384,	AUTH_HMAC_SHA2_384_192 },
	{ IKEV1_AH_HMAC_SHA2_512,	AUTH_HMAC_SHA2_512_256 },
	{ IKEV1_AH_AES_XCBC_MAC,	AUTH_AES_XCBC_96 },
	{ IKEV1_AH_AES_128_GMAC,	AUTH_AES_128_GMAC },
	{ IKEV1_AH_AES_192_GMAC,	AUTH_AES_192_GMAC },
	{ IKEV1_AH_AES_256_GMAC,	AUTH_AES_256_GMAC },
};

/**
 * ESP/AH authentication algorithm mapping
 */
static algo_map_t map_auth[] = {
	{ IKEV1_AUTH_HMAC_MD5,			AUTH_HMAC_MD5_96 },
	{ IKEV1_AUTH_HMAC_SHA,			AUTH_HMAC_SHA1_96 },
	{ IKEV1_AUTH_DES_MAC,			AUTH_DES_MAC },
	{ IKEV1_AUTH_KPDK,				AUTH_KPDK_MD5 },
	{ IKEV1_AUTH_HMAC_SHA2_256,		AUTH_HMAC_SHA2_256_128 },
	{ IKEV1_AUTH_HMAC_SHA2_384,		AUTH_HMAC_SHA2_384_192 },
	{ IKEV1_AUTH_HMAC_SHA2_512,		AUTH_HMAC_SHA2_512_256 },
	{ IKEV1_AUTH_AES_XCBC_MAC,		AUTH_AES_XCBC_96 },
	{ IKEV1_AUTH_AES_128_GMAC,		AUTH_AES_128_GMAC },
	{ IKEV1_AUTH_AES_192_GMAC,		AUTH_AES_192_GMAC },
	{ IKEV1_AUTH_AES_256_GMAC,		AUTH_AES_256_GMAC },
};

/**
 * Map an IKEv1 to an IKEv2 identifier
 */
static uint16_t ikev2_from_ikev1(algo_map_t *map, int count, uint16_t def,
								  uint16_t value)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (map[i].ikev1 == value)
		{
			return map[i].ikev2;
		}
	}
	return def;
}

/**
 * Map an IKEv2 to an IKEv1 identifier
 */
static uint16_t ikev1_from_ikev2(algo_map_t *map, int count, uint16_t value)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (map[i].ikev2 == value)
		{
			return map[i].ikev1;
		}
	}
	return 0;
}

/**
 * Get IKEv2 algorithm from IKEv1 identifier
 */
static uint16_t get_alg_from_ikev1(transform_type_t type, uint16_t value)
{
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			return ikev2_from_ikev1(map_encr, countof(map_encr),
									ENCR_UNDEFINED, value);
		case INTEGRITY_ALGORITHM:
			return ikev2_from_ikev1(map_integ, countof(map_integ),
									AUTH_UNDEFINED, value);
		case PSEUDO_RANDOM_FUNCTION:
			return ikev2_from_ikev1(map_prf, countof(map_prf),
									PRF_UNDEFINED, value);
		default:
			return 0;
	}
}

/**
 * Get IKEv1 algorithm from IKEv2 identifier
 */
static uint16_t get_ikev1_from_alg(transform_type_t type, uint16_t value)
{
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			return ikev1_from_ikev2(map_encr, countof(map_encr), value);
		case INTEGRITY_ALGORITHM:
			return ikev1_from_ikev2(map_integ, countof(map_integ), value);
		case PSEUDO_RANDOM_FUNCTION:
			return ikev1_from_ikev2(map_prf, countof(map_prf), value);
		default:
			return 0;
	}
}

/**
 * Get IKEv2 algorithm from IKEv1 ESP/AH transform ID
 */
static uint16_t get_alg_from_ikev1_transid(transform_type_t type,
											uint16_t value)
{
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			return ikev2_from_ikev1(map_esp, countof(map_esp),
									ENCR_UNDEFINED, value);
		case INTEGRITY_ALGORITHM:
			return ikev2_from_ikev1(map_ah, countof(map_ah),
									AUTH_UNDEFINED, value);
		default:
			return 0;
	}
}

/**
 * Get IKEv1 ESP/AH transform ID from IKEv2 identifier
 */
static uint16_t get_ikev1_transid_from_alg(transform_type_t type,
											uint16_t value)
{
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			return ikev1_from_ikev2(map_esp, countof(map_esp), value);
		case INTEGRITY_ALGORITHM:
			return ikev1_from_ikev2(map_ah, countof(map_ah), value);
		default:
			return 0;
	}
}

/**
 * Get IKEv1 authentication algorithm from IKEv2 identifier
 */
static uint16_t get_alg_from_ikev1_auth(uint16_t value)
{
	return ikev2_from_ikev1(map_auth, countof(map_auth), AUTH_UNDEFINED, value);
}

/**
 * Get IKEv1 authentication algorithm from IKEv2 identifier
 */
static uint16_t get_ikev1_auth_from_alg(uint16_t value)
{
	return ikev1_from_ikev2(map_auth, countof(map_auth), value);
}

/**
 * Get IKEv1 authentication attribute from auth_method_t
 */
static uint16_t get_ikev1_auth(auth_method_t method)
{
	switch (method)
	{
		case AUTH_RSA:
			return IKEV1_AUTH_RSA_SIG;
		case AUTH_DSS:
			return IKEV1_AUTH_DSS_SIG;
		case AUTH_XAUTH_INIT_PSK:
			return IKEV1_AUTH_XAUTH_INIT_PSK;
		case AUTH_XAUTH_RESP_PSK:
			return IKEV1_AUTH_XAUTH_RESP_PSK;
		case AUTH_XAUTH_INIT_RSA:
			return IKEV1_AUTH_XAUTH_INIT_RSA;
		case AUTH_XAUTH_RESP_RSA:
			return IKEV1_AUTH_XAUTH_RESP_RSA;
		case AUTH_HYBRID_INIT_RSA:
			return IKEV1_AUTH_HYBRID_INIT_RSA;
		case AUTH_HYBRID_RESP_RSA:
			return IKEV1_AUTH_HYBRID_RESP_RSA;
		case AUTH_ECDSA_256:
			return IKEV1_AUTH_ECDSA_256;
		case AUTH_ECDSA_384:
			return IKEV1_AUTH_ECDSA_384;
		case AUTH_ECDSA_521:
			return IKEV1_AUTH_ECDSA_521;
		case AUTH_PSK:
		default:
			return IKEV1_AUTH_PSK;
	}
}

/**
 * Get IKEv1 encapsulation mode
 */
static uint16_t get_ikev1_mode(ipsec_mode_t mode, encap_t udp)
{
	switch (mode)
	{
		case MODE_TUNNEL:
			switch (udp)
			{
				case ENCAP_UDP:
					return IKEV1_ENCAP_UDP_TUNNEL;
				case ENCAP_UDP_DRAFT_00_03:
					return IKEV1_ENCAP_UDP_TUNNEL_DRAFT_00_03;
				default:
					return IKEV1_ENCAP_TUNNEL;
			}
		case MODE_TRANSPORT:
			switch (udp)
			{
				case ENCAP_UDP:
					return IKEV1_ENCAP_UDP_TRANSPORT;
				case ENCAP_UDP_DRAFT_00_03:
					return IKEV1_ENCAP_UDP_TRANSPORT_DRAFT_00_03;
				default:
					return IKEV1_ENCAP_TRANSPORT;
			}
		default:
			return IKEV1_ENCAP_TUNNEL;
	}
}

/**
 * Add an IKE transform to a proposal for IKEv1
 */
static void add_to_proposal_v1_ike(proposal_t *proposal,
								   transform_substructure_t *transform)
{
	transform_attribute_type_t type;
	transform_attribute_t *tattr;
	enumerator_t *enumerator;
	uint16_t value, key_length = 0;
	uint16_t encr = ENCR_UNDEFINED;

	enumerator = transform->create_attribute_enumerator(transform);
	while (enumerator->enumerate(enumerator, &tattr))
	{
		type = tattr->get_attribute_type(tattr);
		value = tattr->get_value(tattr);
		switch (type)
		{
			case TATTR_PH1_ENCRYPTION_ALGORITHM:
				encr = get_alg_from_ikev1(ENCRYPTION_ALGORITHM, value);
				break;
			case TATTR_PH1_KEY_LENGTH:
				key_length = value;
				break;
			case TATTR_PH1_HASH_ALGORITHM:
				proposal->add_algorithm(proposal, INTEGRITY_ALGORITHM,
						get_alg_from_ikev1(INTEGRITY_ALGORITHM, value), 0);
				proposal->add_algorithm(proposal, PSEUDO_RANDOM_FUNCTION,
						get_alg_from_ikev1(PSEUDO_RANDOM_FUNCTION, value), 0);
				break;
			case TATTR_PH1_GROUP:
				proposal->add_algorithm(proposal, DIFFIE_HELLMAN_GROUP,
						value, 0);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (encr != ENCR_UNDEFINED)
	{
		if (encr == ENCR_AES_CBC && !key_length)
		{	/* some implementations don't send a Key Length attribute for
			 * AES-128, early drafts of RFC 3602 allowed that */
			key_length = 128;
		}
		proposal->add_algorithm(proposal, ENCRYPTION_ALGORITHM, encr, key_length);
	}
}

/**
 * Add an ESP/AH transform to a proposal for IKEv1
 */
static void add_to_proposal_v1(proposal_t *proposal,
					transform_substructure_t *transform, protocol_id_t proto)
{
	transform_attribute_type_t type;
	transform_attribute_t *tattr;
	enumerator_t *enumerator;
	uint16_t encr, value, key_length = 0;
	extended_sequence_numbers_t esn = NO_EXT_SEQ_NUMBERS;

	enumerator = transform->create_attribute_enumerator(transform);
	while (enumerator->enumerate(enumerator, &tattr))
	{
		type = tattr->get_attribute_type(tattr);
		value = tattr->get_value(tattr);
		switch (type)
		{
			case TATTR_PH2_KEY_LENGTH:
				key_length = value;
				break;
			case TATTR_PH2_AUTH_ALGORITHM:
				proposal->add_algorithm(proposal, INTEGRITY_ALGORITHM,
										get_alg_from_ikev1_auth(value), 0);
				break;
			case TATTR_PH2_GROUP:
				proposal->add_algorithm(proposal, DIFFIE_HELLMAN_GROUP,
						value, 0);
				break;
			case TATTR_PH2_EXT_SEQ_NUMBER:
				esn = EXT_SEQ_NUMBERS;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	proposal->add_algorithm(proposal, EXTENDED_SEQUENCE_NUMBERS, esn, 0);
	if (proto == PROTO_ESP)
	{
		encr = get_alg_from_ikev1_transid(ENCRYPTION_ALGORITHM,
									transform->get_transform_id(transform));
		if (encr)
		{
			if (encr == ENCR_AES_CBC && !key_length)
			{	/* some implementations don't send a Key Length attribute for
				 * AES-128, early drafts of RFC 3602 allowed that for IKE, some
				 * also seem to do it for ESP */
				key_length = 128;
			}
			proposal->add_algorithm(proposal, ENCRYPTION_ALGORITHM, encr,
									key_length);
		}
	}
}

METHOD(proposal_substructure_t, get_proposals, void,
	private_proposal_substructure_t *this, linked_list_t *proposals)
{
	transform_substructure_t *transform;
	enumerator_t *enumerator;
	proposal_t *proposal = NULL;
	uint64_t spi = 0;

	switch (this->spi.len)
	{
		case 4:
			spi =  *((uint32_t*)this->spi.ptr);
			break;
		case 8:
			spi = *((uint64_t*)this->spi.ptr);
			break;
		default:
			break;
	}

	enumerator = this->transforms->create_enumerator(this->transforms);
	while (enumerator->enumerate(enumerator, &transform))
	{
		if (!proposal)
		{
			proposal = proposal_create(this->protocol_id, this->proposal_number);
			proposal->set_spi(proposal, spi);
			proposals->insert_last(proposals, proposal);
		}
		if (this->type == PLV2_PROPOSAL_SUBSTRUCTURE)
		{
			add_to_proposal_v2(proposal, transform);
		}
		else
		{
			switch (this->protocol_id)
			{
				case PROTO_IKE:
					add_to_proposal_v1_ike(proposal, transform);
					break;
				case PROTO_ESP:
				case PROTO_AH:
					add_to_proposal_v1(proposal, transform, this->protocol_id);
					break;
				default:
					break;
			}
			/* create a new proposal for each transform in IKEv1 */
			proposal = NULL;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(proposal_substructure_t, create_substructure_enumerator, enumerator_t*,
	private_proposal_substructure_t *this)
{
	return this->transforms->create_enumerator(this->transforms);
}

/**
 * Get an attribute from any transform, 0 if not found
 */
static uint64_t get_attr(private_proposal_substructure_t *this,
						  transform_attribute_type_t type)
{
	enumerator_t *transforms, *attributes;
	transform_substructure_t *transform;
	transform_attribute_t *attr;

	transforms = this->transforms->create_enumerator(this->transforms);
	while (transforms->enumerate(transforms, &transform))
	{
		attributes = transform->create_attribute_enumerator(transform);
		while (attributes->enumerate(attributes, &attr))
		{
			if (attr->get_attribute_type(attr) == type)
			{
				attributes->destroy(attributes);
				transforms->destroy(transforms);
				return attr->get_value(attr);
			}
		}
		attributes->destroy(attributes);
	}
	transforms->destroy(transforms);
	return 0;
}

/**
 * Look up a lifetime duration of a given kind in all transforms
 */
static uint64_t get_life_duration(private_proposal_substructure_t *this,
				transform_attribute_type_t type_attr, ikev1_life_type_t type,
				transform_attribute_type_t dur_attr)
{
	enumerator_t *transforms, *attributes;
	transform_substructure_t *transform;
	transform_attribute_t *attr;

	transforms = this->transforms->create_enumerator(this->transforms);
	while (transforms->enumerate(transforms, &transform))
	{
		attributes = transform->create_attribute_enumerator(transform);
		while (attributes->enumerate(attributes, &attr))
		{
			if (attr->get_attribute_type(attr) == type_attr &&
				attr->get_value(attr) == type)
			{	/* got type attribute, look for duration following next */
				while (attributes->enumerate(attributes, &attr))
				{
					if (attr->get_attribute_type(attr) == dur_attr)
					{
						attributes->destroy(attributes);
						transforms->destroy(transforms);
						return attr->get_value(attr);
					}
				}
			}
		}
		attributes->destroy(attributes);
	}
	transforms->destroy(transforms);
	return 0;
}

METHOD(proposal_substructure_t, get_lifetime, uint32_t,
	private_proposal_substructure_t *this)
{
	uint32_t duration;

	switch (this->protocol_id)
	{
		case PROTO_IKE:
			return get_life_duration(this, TATTR_PH1_LIFE_TYPE,
						IKEV1_LIFE_TYPE_SECONDS, TATTR_PH1_LIFE_DURATION);
		case PROTO_ESP:
		case PROTO_AH:
			duration = get_life_duration(this, TATTR_PH2_SA_LIFE_TYPE,
						IKEV1_LIFE_TYPE_SECONDS, TATTR_PH2_SA_LIFE_DURATION);
			if (!duration)
			{	/* default to 8 hours, RFC 2407 */
				return 28800;
			}
			return duration;
		default:
			return 0;
	}
}

METHOD(proposal_substructure_t, get_lifebytes, uint64_t,
	private_proposal_substructure_t *this)
{
	switch (this->protocol_id)
	{
		case PROTO_ESP:
		case PROTO_AH:
			return 1000 * get_life_duration(this, TATTR_PH2_SA_LIFE_TYPE,
					IKEV1_LIFE_TYPE_KILOBYTES, TATTR_PH2_SA_LIFE_DURATION);
		case PROTO_IKE:
		default:
			return 0;
	}
}

METHOD(proposal_substructure_t, get_auth_method, auth_method_t,
	private_proposal_substructure_t *this)
{
	switch (get_attr(this, TATTR_PH1_AUTH_METHOD))
	{
		case IKEV1_AUTH_PSK:
			return AUTH_PSK;
		case IKEV1_AUTH_RSA_SIG:
			return AUTH_RSA;
		case IKEV1_AUTH_DSS_SIG:
			return AUTH_DSS;
		case IKEV1_AUTH_XAUTH_INIT_PSK:
			return AUTH_XAUTH_INIT_PSK;
		case IKEV1_AUTH_XAUTH_RESP_PSK:
			return AUTH_XAUTH_RESP_PSK;
		case IKEV1_AUTH_XAUTH_INIT_RSA:
			return AUTH_XAUTH_INIT_RSA;
		case IKEV1_AUTH_XAUTH_RESP_RSA:
			return AUTH_XAUTH_RESP_RSA;
		case IKEV1_AUTH_HYBRID_INIT_RSA:
			return AUTH_HYBRID_INIT_RSA;
		case IKEV1_AUTH_HYBRID_RESP_RSA:
			return AUTH_HYBRID_RESP_RSA;
		case IKEV1_AUTH_ECDSA_256:
			return AUTH_ECDSA_256;
		case IKEV1_AUTH_ECDSA_384:
			return AUTH_ECDSA_384;
		case IKEV1_AUTH_ECDSA_521:
			return AUTH_ECDSA_521;
		default:
			return AUTH_NONE;
	}
}

METHOD(proposal_substructure_t, get_encap_mode, ipsec_mode_t,
	private_proposal_substructure_t *this, bool *udp)
{
	*udp = FALSE;
	switch (get_attr(this, TATTR_PH2_ENCAP_MODE))
	{
		case IKEV1_ENCAP_TRANSPORT:
			return MODE_TRANSPORT;
		case IKEV1_ENCAP_TUNNEL:
			return MODE_TUNNEL;
		case IKEV1_ENCAP_UDP_TRANSPORT:
		case IKEV1_ENCAP_UDP_TRANSPORT_DRAFT_00_03:
			*udp = TRUE;
			return MODE_TRANSPORT;
		case IKEV1_ENCAP_UDP_TUNNEL:
		case IKEV1_ENCAP_UDP_TUNNEL_DRAFT_00_03:
			*udp = TRUE;
			return MODE_TUNNEL;
		default:
			/* default to TUNNEL, RFC 2407 says implementation specific */
			return MODE_TUNNEL;
	}
}

METHOD2(payload_t, proposal_substructure_t, destroy, void,
	private_proposal_substructure_t *this)
{
	this->transforms->destroy_offset(this->transforms,
									 offsetof(payload_t, destroy));
	chunk_free(&this->spi);
	free(this);
}

/*
 * Described in header.
 */
proposal_substructure_t *proposal_substructure_create(payload_type_t type)
{
	private_proposal_substructure_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.set_proposal_number = _set_proposal_number,
			.get_proposal_number = _get_proposal_number,
			.set_protocol_id = _set_protocol_id,
			.get_protocol_id = _get_protocol_id,
			.set_is_last_proposal = _set_is_last_proposal,
			.get_proposals = _get_proposals,
			.create_substructure_enumerator = _create_substructure_enumerator,
			.set_spi = _set_spi,
			.get_spi = _get_spi,
			.get_cpi = _get_cpi,
			.get_lifetime = _get_lifetime,
			.get_lifebytes = _get_lifebytes,
			.get_auth_method = _get_auth_method,
			.get_encap_mode = _get_encap_mode,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.transforms = linked_list_create(),
		.type = type,
	);
	compute_length(this);

	return &this->public;
}

/**
 * Add an IKEv1 IKE proposal to the substructure
 */
static void set_from_proposal_v1_ike(private_proposal_substructure_t *this,
									 proposal_t *proposal, uint32_t lifetime,
									 auth_method_t method, int number)
{
	transform_substructure_t *transform;
	uint16_t alg, key_size;
	enumerator_t *enumerator;

	transform = transform_substructure_create_type(PLV1_TRANSFORM_SUBSTRUCTURE,
												number, IKEV1_TRANSID_KEY_IKE);

	enumerator = proposal->create_enumerator(proposal, ENCRYPTION_ALGORITHM);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		alg = get_ikev1_from_alg(ENCRYPTION_ALGORITHM, alg);
		if (alg)
		{
			transform->add_transform_attribute(transform,
				transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
									TATTR_PH1_ENCRYPTION_ALGORITHM, alg));
			if (key_size)
			{
				transform->add_transform_attribute(transform,
					transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
										TATTR_PH1_KEY_LENGTH, key_size));
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* encode the integrity algorithm as hash and assume use the same PRF */
	enumerator = proposal->create_enumerator(proposal, INTEGRITY_ALGORITHM);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		alg = get_ikev1_from_alg(INTEGRITY_ALGORITHM, alg);
		if (alg)
		{
			transform->add_transform_attribute(transform,
				transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
									TATTR_PH1_HASH_ALGORITHM, alg));
			break;
		}
	}
	enumerator->destroy(enumerator);

	enumerator = proposal->create_enumerator(proposal, DIFFIE_HELLMAN_GROUP);
	if (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
								TATTR_PH1_GROUP, alg));
	}
	enumerator->destroy(enumerator);

	transform->add_transform_attribute(transform,
		transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH1_AUTH_METHOD, get_ikev1_auth(method)));
	transform->add_transform_attribute(transform,
		transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH1_LIFE_TYPE, IKEV1_LIFE_TYPE_SECONDS));
	transform->add_transform_attribute(transform,
		transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH1_LIFE_DURATION, lifetime));

	add_transform_substructure(this, transform);
}

/**
 * Add an IKEv1 ESP/AH proposal to the substructure
 */
static void set_from_proposal_v1(private_proposal_substructure_t *this,
				proposal_t *proposal, uint32_t lifetime, uint64_t lifebytes,
				ipsec_mode_t mode, encap_t udp, int number)
{
	transform_substructure_t *transform = NULL;
	uint16_t alg, transid, key_size;
	enumerator_t *enumerator;

	enumerator = proposal->create_enumerator(proposal, ENCRYPTION_ALGORITHM);
	if (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transid = get_ikev1_transid_from_alg(ENCRYPTION_ALGORITHM, alg);
		if (transid)
		{
			transform = transform_substructure_create_type(
								PLV1_TRANSFORM_SUBSTRUCTURE, number, transid);
			if (key_size)
			{
				transform->add_transform_attribute(transform,
					transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
											TATTR_PH2_KEY_LENGTH, key_size));
			}
		}
	}
	enumerator->destroy(enumerator);

	enumerator = proposal->create_enumerator(proposal, INTEGRITY_ALGORITHM);
	if (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transid = get_ikev1_transid_from_alg(INTEGRITY_ALGORITHM, alg);
		alg = get_ikev1_auth_from_alg(alg);
		if (alg)
		{
			if (!transform && transid)
			{
				transform = transform_substructure_create_type(
								PLV1_TRANSFORM_SUBSTRUCTURE, number, transid);
			}
			if (transform)
			{
				transform->add_transform_attribute(transform,
					transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
										TATTR_PH2_AUTH_ALGORITHM, alg));
			}
		}
	}
	enumerator->destroy(enumerator);

	if (!transform)
	{
		return;
	}

	enumerator = proposal->create_enumerator(proposal, DIFFIE_HELLMAN_GROUP);
	if (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
									TATTR_PH2_GROUP, alg));
	}
	enumerator->destroy(enumerator);

	transform->add_transform_attribute(transform,
		transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_ENCAP_MODE, get_ikev1_mode(mode, udp)));
	if (lifetime)
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_TYPE, IKEV1_LIFE_TYPE_SECONDS));
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_DURATION, lifetime));
	}
	if (lifebytes)
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_TYPE, IKEV1_LIFE_TYPE_KILOBYTES));
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_DURATION, lifebytes / 1000));
	}

	enumerator = proposal->create_enumerator(proposal,
			EXTENDED_SEQUENCE_NUMBERS);
	while (enumerator->enumerate(enumerator, &alg, NULL))
	{
		if (alg == EXT_SEQ_NUMBERS)
		{
			transform->add_transform_attribute(transform,
				transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
								TATTR_PH2_EXT_SEQ_NUMBER, alg));
		}
	}
	enumerator->destroy(enumerator);
	add_transform_substructure(this, transform);
}

/**
 * Add an IKEv2 proposal to the substructure
 */
static void set_from_proposal_v2(private_proposal_substructure_t *this,
								 proposal_t *proposal)
{
	transform_substructure_t *transform;
	uint16_t alg, key_size;
	enumerator_t *enumerator;

	/* encryption algorithm is only available in ESP */
	enumerator = proposal->create_enumerator(proposal, ENCRYPTION_ALGORITHM);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transform = transform_substructure_create_type(PLV2_TRANSFORM_SUBSTRUCTURE,
												ENCRYPTION_ALGORITHM, alg);
		if (key_size)
		{
			transform->add_transform_attribute(transform,
				transform_attribute_create_value(PLV2_TRANSFORM_ATTRIBUTE,
											TATTR_IKEV2_KEY_LENGTH, key_size));
		}
		add_transform_substructure(this, transform);
	}
	enumerator->destroy(enumerator);

	/* integrity algorithms */
	enumerator = proposal->create_enumerator(proposal, INTEGRITY_ALGORITHM);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transform = transform_substructure_create_type(PLV2_TRANSFORM_SUBSTRUCTURE,
												INTEGRITY_ALGORITHM, alg);
		add_transform_substructure(this, transform);
	}
	enumerator->destroy(enumerator);

	/* prf algorithms */
	enumerator = proposal->create_enumerator(proposal, PSEUDO_RANDOM_FUNCTION);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		transform = transform_substructure_create_type(PLV2_TRANSFORM_SUBSTRUCTURE,
												PSEUDO_RANDOM_FUNCTION, alg);
		add_transform_substructure(this, transform);
	}
	enumerator->destroy(enumerator);

	/* dh groups */
	enumerator = proposal->create_enumerator(proposal, DIFFIE_HELLMAN_GROUP);
	while (enumerator->enumerate(enumerator, &alg, NULL))
	{
		transform = transform_substructure_create_type(PLV2_TRANSFORM_SUBSTRUCTURE,
												DIFFIE_HELLMAN_GROUP, alg);
		add_transform_substructure(this, transform);
	}
	enumerator->destroy(enumerator);

	/* extended sequence numbers */
	enumerator = proposal->create_enumerator(proposal, EXTENDED_SEQUENCE_NUMBERS);
	while (enumerator->enumerate(enumerator, &alg, NULL))
	{
		transform = transform_substructure_create_type(PLV2_TRANSFORM_SUBSTRUCTURE,
												EXTENDED_SEQUENCE_NUMBERS, alg);
		add_transform_substructure(this, transform);
	}
	enumerator->destroy(enumerator);
}

/**
 * Set SPI and other data from proposal, compute length
 */
static void set_data(private_proposal_substructure_t *this, proposal_t *proposal)
{
	uint64_t spi64;
	uint32_t spi32;

	/* add SPI, if necessary */
	switch (proposal->get_protocol(proposal))
	{
		case PROTO_AH:
		case PROTO_ESP:
			spi32 = proposal->get_spi(proposal);
			this->spi = chunk_clone(chunk_from_thing(spi32));
			this->spi_size = this->spi.len;
			break;
		case PROTO_IKE:
			spi64 = proposal->get_spi(proposal);
			if (spi64)
			{	/* IKE only uses SPIS when rekeying, but on initial setup */
				this->spi = chunk_clone(chunk_from_thing(spi64));
				this->spi_size = this->spi.len;
			}
			break;
		default:
			break;
	}
	this->proposal_number = proposal->get_number(proposal);
	this->protocol_id = proposal->get_protocol(proposal);
	compute_length(this);
}

/*
 * Described in header.
 */
proposal_substructure_t *proposal_substructure_create_from_proposal_v2(
														proposal_t *proposal)
{
	private_proposal_substructure_t *this;

	this = (private_proposal_substructure_t*)
							proposal_substructure_create(PLV2_SECURITY_ASSOCIATION);
	set_from_proposal_v2(this, proposal);
	set_data(this, proposal);

	return &this->public;
}

/**
 * See header.
 */
proposal_substructure_t *proposal_substructure_create_from_proposal_v1(
			proposal_t *proposal, uint32_t lifetime, uint64_t lifebytes,
			auth_method_t auth, ipsec_mode_t mode, encap_t udp)
{
	private_proposal_substructure_t *this;

	this = (private_proposal_substructure_t*)
						proposal_substructure_create(PLV1_PROPOSAL_SUBSTRUCTURE);
	switch (proposal->get_protocol(proposal))
	{
		case PROTO_IKE:
			set_from_proposal_v1_ike(this, proposal, lifetime, auth, 1);
			break;
		case PROTO_ESP:
		case PROTO_AH:
			set_from_proposal_v1(this, proposal, lifetime,
								 lifebytes, mode, udp, 1);
			break;
		default:
			break;
	}
	set_data(this, proposal);

	return &this->public;
}

/**
 * See header.
 */
proposal_substructure_t *proposal_substructure_create_from_proposals_v1(
			linked_list_t *proposals, uint32_t lifetime, uint64_t lifebytes,
			auth_method_t auth, ipsec_mode_t mode, encap_t udp)
{
	private_proposal_substructure_t *this = NULL;
	enumerator_t *enumerator;
	proposal_t *proposal;
	int number = 0;

	enumerator = proposals->create_enumerator(proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		if (!this)
		{
			this = (private_proposal_substructure_t*)
						proposal_substructure_create_from_proposal_v1(
								proposal, lifetime, lifebytes, auth, mode, udp);
			++number;
		}
		else
		{
			switch (proposal->get_protocol(proposal))
			{
				case PROTO_IKE:
					set_from_proposal_v1_ike(this, proposal, lifetime,
											 auth, ++number);
					break;
				case PROTO_ESP:
				case PROTO_AH:
					set_from_proposal_v1(this, proposal, lifetime,
										 lifebytes, mode, udp, ++number);
					break;
				default:
					break;
			}
		}
	}
	enumerator->destroy(enumerator);

	return &this->public;
}

/**
 * See header.
 */
proposal_substructure_t *proposal_substructure_create_for_ipcomp_v1(
			uint32_t lifetime, uint64_t lifebytes, uint16_t cpi,
			ipsec_mode_t mode, encap_t udp, uint8_t proposal_number)
{
	private_proposal_substructure_t *this;
	transform_substructure_t *transform;


	this = (private_proposal_substructure_t*)
						proposal_substructure_create(PLV1_PROPOSAL_SUBSTRUCTURE);

	/* we currently support DEFLATE only */
	transform = transform_substructure_create_type(PLV1_TRANSFORM_SUBSTRUCTURE,
												   1, IKEV1_IPCOMP_DEFLATE);

	transform->add_transform_attribute(transform,
		transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_ENCAP_MODE, get_ikev1_mode(mode, udp)));
	if (lifetime)
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_TYPE, IKEV1_LIFE_TYPE_SECONDS));
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_DURATION, lifetime));
	}
	if (lifebytes)
	{
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_TYPE, IKEV1_LIFE_TYPE_KILOBYTES));
		transform->add_transform_attribute(transform,
			transform_attribute_create_value(PLV1_TRANSFORM_ATTRIBUTE,
							TATTR_PH2_SA_LIFE_DURATION, lifebytes / 1000));
	}

	add_transform_substructure(this, transform);

	this->spi = chunk_clone(chunk_from_thing(cpi));
	this->spi_size = this->spi.len;
	this->protocol_id = PROTO_IPCOMP;
	this->proposal_number = proposal_number;

	compute_length(this);

	return &this->public;
}
