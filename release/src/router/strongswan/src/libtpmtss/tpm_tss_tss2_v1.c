/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2016-2020 Andreas Steffen
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

#include "tpm_tss_tss2.h"
#include "tpm_tss_tss2_names.h"

#ifdef TSS_TSS2_V1

#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <threading/mutex.h>

#include <tpm20.h>

#include <unistd.h>

#ifdef TSS2_TCTI_TABRMD
#include <tcti/tcti-tabrmd.h>
#endif /* TSS2_TCTI_TABRMD */

#ifdef TSS2_TCTI_SOCKET
#include <tcti_socket.h>

#define TCTI_SOCKET_DEFAULT_ADDRESS "127.0.0.1"
#define TCTI_SOCKET_DEFAULT_PORT     2323
#endif /* TSS2_TCTI_SOCKET */

#define LABEL	"TPM 2.0 -"

#define PLATFORM_PCR	24
#define MAX_PCR_BANKS	 4

typedef struct private_tpm_tss_tss2_t private_tpm_tss_tss2_t;

/**
 * Private data of an tpm_tss_tss2_t object.
 */
struct private_tpm_tss_tss2_t {

	/**
	 * Public tpm_tss_tss2_t interface.
	 */
	tpm_tss_t public;

	/**
	 * TCTI context
	 */
	TSS2_TCTI_CONTEXT *tcti_context;

	/**
	 * SYS context
	 */
	TSS2_SYS_CONTEXT  *sys_context;

	/**
	 * TPM version info
	 */
	chunk_t version_info;

	/**
	 * Number of supported algorithms
	 */
	size_t supported_algs_count;

	/**
	 * List of supported algorithms
	 */
	TPM_ALG_ID supported_algs[TPM_PT_ALGORITHM_SET];

		/**
	 * Number of assigned PCR banks
	 */
	size_t assigned_pcrs_count;

	/**
	 * List of assigned PCR banks
	 */
	TPM_ALG_ID assigned_pcrs[MAX_PCR_BANKS];

	/**
	 * Is TPM FIPS 186-4 compliant ?
	 */
	bool fips_186_4;

	/**
	 * Mutex controlling access to the TPM 2.0 context
	 */
	mutex_t *mutex;

};

/**
 * Some symbols required by libtctisocket
 */
FILE *outFp;
uint8_t simulator = 1;

int TpmClientPrintf (uint8_t type, const char *format, ...)
{
    return 0;
}

/**
 * Convert hash algorithm to TPM_ALG_ID
 */
static TPM_ALG_ID hash_alg_to_tpm_alg_id(hash_algorithm_t alg)
{
	switch (alg)
	{
		case HASH_SHA1:
			return TPM_ALG_SHA1;
		case HASH_SHA256:
			return TPM_ALG_SHA256;
		case HASH_SHA384:
			return TPM_ALG_SHA384;
		case HASH_SHA512:
			return TPM_ALG_SHA512;
		default:
			return TPM_ALG_ERROR;
	}
}

/**
 * Convert TPM_ALG_ID to hash algorithm
 */
static hash_algorithm_t hash_alg_from_tpm_alg_id(TPM_ALG_ID alg)
{
	switch (alg)
	{
		case TPM_ALG_SHA1:
			return HASH_SHA1;
		case TPM_ALG_SHA256:
			return HASH_SHA256;
		case TPM_ALG_SHA384:
			return HASH_SHA384;
		case TPM_ALG_SHA512:
			return HASH_SHA512;
		default:
			return HASH_UNKNOWN;
	}
}

/**
 * Check if an algorithm given by its TPM_ALG_ID is supported by the TPM
 */
static bool is_supported_alg(private_tpm_tss_tss2_t *this, TPM_ALG_ID alg_id)
{
	int i;

	if (alg_id == TPM_ALG_ERROR)
	{
		return FALSE;
	}

	for (i = 0; i < this->supported_algs_count; i++)
	{
		if (this->supported_algs[i] == alg_id)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Get the TPM version_info and a list of supported algorithms
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  TPM 2.0 Version_Info Tag     |   Reserved    |   Locality    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                            Revision                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                              Year                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                             Vendor                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define TPM2_VERSION_INFO_TAG       0x0200
#define TPM2_VERSION_INFO_RESERVED  0x00
#define TPM2_VERSION_INFO_SIZE      16
#define TPM2_DEFAULT_LOCALITY       3

static bool get_algs_capability(private_tpm_tss_tss2_t *this)
{
	TPMS_CAPABILITY_DATA cap_data;
	TPMS_TAGGED_PROPERTY tp;
	TPMI_YES_NO more_data;
	TPM_ALG_ID alg;
	bio_writer_t *writer;
	bool fips_140_2 = FALSE;
	uint32_t rval, i, offset, revision = 0, year = 0, vendor = 0;
	uint8_t locality = TPM2_DEFAULT_LOCALITY;
	size_t len = BUF_LEN;
	char buf[BUF_LEN], manufacturer[5], vendor_string[17];
	char *pos = buf;
	int written;

	/* get fixed properties */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_GetCapability(this->sys_context, 0, TPM_CAP_TPM_PROPERTIES,
						PT_FIXED, MAX_TPM_PROPERTIES, &more_data, &cap_data, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s GetCapability failed for TPM_CAP_TPM_PROPERTIES: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}
	memset(manufacturer,  '\0', sizeof(manufacturer));
	memset(vendor_string, '\0', sizeof(vendor_string));

	/* print fixed properties */
	for (i = 0; i < cap_data.data.tpmProperties.count; i++)
	{
		tp = cap_data.data.tpmProperties.tpmProperty[i];
		switch (tp.property)
		{
			case TPM_PT_REVISION:
				revision = tp.value;
				break;
			case TPM_PT_YEAR:
				year = tp.value;
				break;
			case TPM_PT_MANUFACTURER:
				vendor = tp.value;
				htoun32(manufacturer, tp.value);
				break;
			case TPM_PT_VENDOR_STRING_1:
			case TPM_PT_VENDOR_STRING_2:
			case TPM_PT_VENDOR_STRING_3:
			case TPM_PT_VENDOR_STRING_4:
				offset = 4 * (tp.property - TPM_PT_VENDOR_STRING_1);
				htoun32(vendor_string + offset, tp.value);
				break;
			case TPM_PT_MODES:
				if (tp.value & TPMA_MODES_FIPS_140_2)
				{
					this->fips_186_4 = fips_140_2 = TRUE;
				}
				break;
			default:
				break;
		}
	}

	if (!fips_140_2)
	{
		this->fips_186_4 = lib->settings->get_bool(lib->settings,
					"%s.plugins.tpm.fips_186_4", FALSE, lib->ns);
	}
	DBG2(DBG_PTS, "%s manufacturer: %s (%s) rev: %05.2f %u %s", LABEL,
		 manufacturer, vendor_string, (float)revision/100, year,
		 fips_140_2 ? "FIPS 140-2" : (this->fips_186_4 ? "FIPS 186-4" : ""));

	/* determine if TPM uses old event digest format and a different locality */
	if (streq(manufacturer, "INTC") && revision == 116 && year == 2016)
	{
		locality = 0;
	}

	/* construct TPM 2.0 version_info object */
	writer = bio_writer_create(  TPM2_VERSION_INFO_SIZE);
	writer->write_uint16(writer, TPM2_VERSION_INFO_TAG);
	writer->write_uint8(writer,  TPM2_VERSION_INFO_RESERVED);
	writer->write_uint8(writer,  locality);
	writer->write_uint32(writer, revision);
	writer->write_uint32(writer, year);
	writer->write_uint32(writer, vendor);
	this->version_info = writer->extract_buf(writer);
	writer->destroy(writer);

	/* get supported algorithms */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_GetCapability(this->sys_context, 0, TPM_CAP_ALGS,
						0, TPM_PT_ALGORITHM_SET, &more_data, &cap_data, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s GetCapability failed for TPM_CAP_ALGS: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* Number of supported algorithms */
	this->supported_algs_count = cap_data.data.algorithms.count;

	/* store and print supported algorithms */
	for (i = 0; i < this->supported_algs_count; i++)
	{
		alg = cap_data.data.algorithms.algProperties[i].alg;
		this->supported_algs[i] = alg;

		written = snprintf(pos, len, " %N", tpm_alg_id_names, alg);
		if (written < 0 || written >= len)
		{
			break;
		}
		pos += written;
		len -= written;
	}
	DBG2(DBG_PTS, "%s algorithms:%s", LABEL, buf);

	/* get supported ECC curves */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_GetCapability(this->sys_context, 0, TPM_CAP_ECC_CURVES,
						0, TPM_PT_LOADED_CURVES, &more_data, &cap_data, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s GetCapability failed for TPM_CAP_ECC_CURVES: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* reset print buffer */
	pos = buf;
	len = BUF_LEN;

	/* print supported ECC curves */
	for (i = 0; i < cap_data.data.eccCurves.count; i++)
	{
		written = snprintf(pos, len, " %N", tpm_ecc_curve_names,
						   cap_data.data.eccCurves.eccCurves[i]);
		if (written < 0 || written >= len)
		{
			break;
		}
		pos += written;
		len -= written;
	}
	DBG2(DBG_PTS, "%s ECC curves:%s", LABEL, buf);

	/* get assigned PCR banks */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_GetCapability(this->sys_context, 0, TPM_CAP_PCRS,
						0, MAX_PCR_BANKS, &more_data, &cap_data, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s GetCapability failed for TPM_CAP_PCRS: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* Number of assigned PCR banks */
	this->assigned_pcrs_count = cap_data.data.assignedPCR.count;

	/* reset print buffer */
	pos = buf;
	len = BUF_LEN;

	/* store and print assigned PCR banks */
	for (i = 0; i < cap_data.data.assignedPCR.count; i++)
	{
		alg = cap_data.data.assignedPCR.pcrSelections[i].hash;
		this->assigned_pcrs[i] = alg;
		written = snprintf(pos, len, " %N", tpm_alg_id_names, alg);
		if (written < 0 || written >= len)
		{
			break;
		}
		pos += written;
		len -= written;
	}
	DBG2(DBG_PTS, "%s PCR banks:%s", LABEL, buf);

	return TRUE;
}

/**
 * Initialize TSS2 TCTI TABRMD context
 */
static bool initialize_tcti_tabrmd_context(private_tpm_tss_tss2_t *this)
{
#ifdef TSS2_TCTI_TABRMD
	size_t   tcti_context_size;
	uint32_t rval;

	/* determine size of tcti context */
	rval = tss2_tcti_tabrmd_init(NULL, &tcti_context_size);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get tcti_context size: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* allocate and initialize memory for tcti context */
	this->tcti_context = (TSS2_TCTI_CONTEXT*)malloc(tcti_context_size);
	memset(this->tcti_context, 0x00, tcti_context_size);

	/* initialize tcti context */
	rval = tss2_tcti_tabrmd_init(this->tcti_context, &tcti_context_size);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get tcti_context: 0x%06x "
					  "via tabrmd interface", LABEL, rval);
		return FALSE;
	}
	return TRUE;
#else /* TSS2_TCTI_TABRMD */
	return FALSE;
#endif /* TSS2_TCTI_TABRMD */
}

/**
 * Initialize TSS2 TCTI Socket context
 */
static bool initialize_tcti_socket_context(private_tpm_tss_tss2_t *this)
{
#ifdef TSS2_TCTI_SOCKET
	size_t   tcti_context_size;
	uint32_t rval;

	TCTI_SOCKET_CONF rm_if_config = { TCTI_SOCKET_DEFAULT_ADDRESS,
									  TCTI_SOCKET_DEFAULT_PORT
									};

	/* determine size of tcti context */
	rval = InitSocketTcti(NULL, &tcti_context_size, &rm_if_config, 0);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get tcti_context size: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* allocate memory for tcti context */
	this->tcti_context = (TSS2_TCTI_CONTEXT*)malloc(tcti_context_size);

	/* initialize tcti context */
	rval = InitSocketTcti(this->tcti_context, &tcti_context_size,
						  &rm_if_config, 0);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get tcti_context: 0x%06x "
					  "via socket interface", LABEL, rval);
		return FALSE;
	}
	return TRUE;
#else /* TSS2_TCTI_SOCKET */
	return FALSE;
#endif /* TSS2_TCTI_SOCKET */
}

/**
 * Initialize TSS2 Sys context
 */
static bool initialize_sys_context(private_tpm_tss_tss2_t *this)
{
	uint32_t sys_context_size;
	uint32_t rval;

	TSS2_ABI_VERSION abi_version = { TSSWG_INTEROP,
									 TSS_SAPI_FIRST_FAMILY,
									 TSS_SAPI_FIRST_LEVEL,
									 TSS_SAPI_FIRST_VERSION
								   };

	/* determine size of sys context */
	sys_context_size = Tss2_Sys_GetContextSize(0);

	/* allocate memory for sys context */
	this->sys_context = malloc(sys_context_size);

	/* initialize sys context */
	rval = Tss2_Sys_Initialize(this->sys_context, sys_context_size,
							   this->tcti_context, &abi_version);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get sys_context: 0x%06x",
					   LABEL, rval);
		return FALSE;
	}

	/* get a list of supported algorithms and ECC curves */
	return get_algs_capability(this);
}

/**
 * Finalize TSS context
 */
static void finalize_context(private_tpm_tss_tss2_t *this)
{
	if (this->tcti_context)
	{
		tss2_tcti_finalize(this->tcti_context);
		free(this->tcti_context);
	}
	if (this->sys_context)
	{
		Tss2_Sys_Finalize(this->sys_context);
		free(this->sys_context);
	}
}

METHOD(tpm_tss_t, get_version, tpm_version_t,
	private_tpm_tss_tss2_t *this)
{
	return TPM_VERSION_2_0;
}

METHOD(tpm_tss_t, get_version_info, chunk_t,
	private_tpm_tss_tss2_t *this)
{
	return this->version_info;
}

/**
 * read the public key portion of a TSS 2.0 key from NVRAM
 */
bool read_public(private_tpm_tss_tss2_t *this, TPMI_DH_OBJECT handle,
	TPM2B_PUBLIC *public)
{
	uint32_t rval;

	TPM2B_NAME name = { { sizeof(TPM2B_NAME)-2, } };
	TPM2B_NAME qualified_name = { { sizeof(TPM2B_NAME)-2, } };

	TPMS_AUTH_RESPONSE session_data;
	TSS2_SYS_RSP_AUTHS sessions_data;
	TPMS_AUTH_RESPONSE *session_data_array[1];

	session_data_array[0]  = &session_data;
	sessions_data.rspAuths = &session_data_array[0];
	sessions_data.rspAuthsCount = 1;

	/* read public key for a given object handle from TPM 2.0 NVRAM */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_ReadPublic(this->sys_context, handle, 0, public, &name,
							   &qualified_name, &sessions_data);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not read public key from handle 0x%08x: 0x%06x",
					   LABEL, handle, rval);
		return FALSE;
	}
	return TRUE;
}

METHOD(tpm_tss_t, generate_aik, bool,
	private_tpm_tss_tss2_t *this, chunk_t ca_modulus, chunk_t *aik_blob,
	chunk_t *aik_pubkey, chunk_t *identity_req)
{
	return FALSE;
}

METHOD(tpm_tss_t, get_public, chunk_t,
	private_tpm_tss_tss2_t *this, uint32_t handle)
{
	TPM2B_PUBLIC public = { { 0, } };
	TPM_ALG_ID sig_alg, digest_alg;
	chunk_t aik_pubkey = chunk_empty;

	if (!read_public(this, handle, &public))
	{
		return chunk_empty;
	}

	/* convert TSS 2.0 public key blot into PKCS#1 format */
	switch (public.t.publicArea.type)
	{
		case TPM_ALG_RSA:
		{
			TPM2B_PUBLIC_KEY_RSA *rsa;
			TPMT_RSA_SCHEME *scheme;
			chunk_t aik_exponent = chunk_from_chars(0x01, 0x00, 0x01);
			chunk_t aik_modulus;
			uint32_t exponent;

			scheme = &public.t.publicArea.parameters.rsaDetail.scheme;
			sig_alg   = scheme->scheme;
			digest_alg = scheme->details.anySig.hashAlg;

			rsa = &public.t.publicArea.unique.rsa;
			aik_modulus = chunk_create(rsa->t.buffer, rsa->t.size);
			exponent = htonl(public.t.publicArea.parameters.rsaDetail.exponent);
			if (exponent)
			{
				aik_exponent = chunk_from_thing(exponent);
			}

			/* subjectPublicKeyInfo encoding of RSA public key */
			if (!lib->encoding->encode(lib->encoding, PUBKEY_SPKI_ASN1_DER,
					NULL, &aik_pubkey, CRED_PART_RSA_MODULUS, aik_modulus,
					CRED_PART_RSA_PUB_EXP, aik_exponent, CRED_PART_END))
			{
				DBG1(DBG_PTS, "%s subjectPublicKeyInfo encoding of public key "
							  "failed", LABEL);
				return chunk_empty;
			}
			break;
		}
		case TPM_ALG_ECC:
		{
			TPMS_ECC_POINT *ecc;
			TPMT_ECC_SCHEME *scheme;
			chunk_t ecc_point;
			uint8_t *pos;

			scheme = &public.t.publicArea.parameters.eccDetail.scheme;
			sig_alg   = scheme->scheme;
			digest_alg = scheme->details.anySig.hashAlg;

			ecc = &public.t.publicArea.unique.ecc;

			/* allocate space for bit string */
			pos = asn1_build_object(&ecc_point, ASN1_BIT_STRING,
									2 + ecc->x.t.size + ecc->y.t.size);
			/* bit string length is a multiple of octets */
			*pos++ = 0x00;
			/* uncompressed ECC point format */
			*pos++ = 0x04;
			/* copy x coordinate of ECC point */
			memcpy(pos, ecc->x.t.buffer, ecc->x.t.size);
			pos += ecc->x.t.size;
			/* copy y coordinate of ECC point */
			memcpy(pos, ecc->y.t.buffer, ecc->y.t.size);
			/* subjectPublicKeyInfo encoding of ECC public key */
			aik_pubkey = asn1_wrap(ASN1_SEQUENCE, "mm",
							asn1_wrap(ASN1_SEQUENCE, "mm",
								asn1_build_known_oid(OID_EC_PUBLICKEY),
								asn1_build_known_oid(ecc->x.t.size == 32 ?
										OID_PRIME256V1 : OID_SECT384R1)),
							ecc_point);
			break;
		}
		default:
			DBG1(DBG_PTS, "%s unsupported key type", LABEL);
			return chunk_empty;
	}
	DBG1(DBG_PTS, "signature algorithm is %N with %N hash",
		 tpm_alg_id_names, sig_alg, tpm_alg_id_names, digest_alg);
	return aik_pubkey;
}

METHOD(tpm_tss_t, supported_signature_schemes, enumerator_t*,
	private_tpm_tss_tss2_t *this, uint32_t handle)
{
	TPM2B_PUBLIC public = { { 0, } };
	hash_algorithm_t digest;
	signature_params_t supported_scheme;

	if (!read_public(this, handle, &public))
	{
		return enumerator_create_empty();
	}

	switch (public.t.publicArea.type)
	{
		case TPM_ALG_RSA:
		{
			TPMS_RSA_PARMS *rsa;
			TPMT_RSA_SCHEME *scheme;

			rsa = &public.t.publicArea.parameters.rsaDetail;
			scheme = &rsa->scheme;
			digest = hash_alg_from_tpm_alg_id(scheme->details.anySig.hashAlg);

			switch (scheme->scheme)
			{
				case TPM_ALG_RSAPSS:
				{
					ssize_t salt_len;

					salt_len = this->fips_186_4 ? RSA_PSS_SALT_LEN_DEFAULT :
												  RSA_PSS_SALT_LEN_MAX;
					rsa_pss_params_t pss_params = {
						.hash = digest,
						.mgf1_hash = digest,
						.salt_len = salt_len,
					};
					supported_scheme = (signature_params_t){
						.scheme = SIGN_RSA_EMSA_PSS,
						.params = &pss_params,
					};
					if (!rsa_pss_params_set_salt_len(&pss_params, rsa->keyBits))
					{
						return enumerator_create_empty();
					}
					break;
				}
				case TPM_ALG_RSASSA:
					supported_scheme = (signature_params_t){
						.scheme = signature_scheme_from_oid(
									hasher_signature_algorithm_to_oid(digest,
																	  KEY_RSA)),
					};
					break;
				default:
					return enumerator_create_empty();
			}
			break;
		}
		case TPM_ALG_ECC:
		{
			TPMT_ECC_SCHEME *scheme;

			scheme = &public.t.publicArea.parameters.eccDetail.scheme;
			digest = hash_alg_from_tpm_alg_id(scheme->details.anySig.hashAlg);

			switch (scheme->scheme)
			{
				case TPM_ALG_ECDSA:
					supported_scheme = (signature_params_t){
						.scheme = signature_scheme_from_oid(
									hasher_signature_algorithm_to_oid(digest,
																	KEY_ECDSA)),
					};
					break;
				default:
					return enumerator_create_empty();
			}
			break;
		}
		default:
			DBG1(DBG_PTS, "%s unsupported key type", LABEL);
			return enumerator_create_empty();
	}
	return enumerator_create_single(signature_params_clone(&supported_scheme),
									(void*)signature_params_destroy);
}

METHOD(tpm_tss_t, has_pcr_bank, bool,
	private_tpm_tss_tss2_t *this, hash_algorithm_t alg)
{
	TPM_ALG_ID alg_id;
	int i;

	alg_id = hash_alg_to_tpm_alg_id(alg);

	for (i = 0; i < this->assigned_pcrs_count; i++)
	{
		if (this->assigned_pcrs[i] == alg_id)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Configure a PCR Selection assuming a maximum of 24 registers
 */
static bool init_pcr_selection(private_tpm_tss_tss2_t *this, uint32_t pcrs,
							   hash_algorithm_t alg, TPML_PCR_SELECTION *pcr_sel)
{
	uint32_t pcr;

	/* check if there is an assigned PCR bank for this hash algorithm */
	if (!has_pcr_bank(this, alg))
	{
		DBG1(DBG_PTS, "%s %N hash algorithm not supported by any PCR bank",
			 LABEL, hash_algorithm_short_names, alg);
		return FALSE;
	}

	/* initialize the PCR Selection structure,*/
	pcr_sel->count = 1;
	pcr_sel->pcrSelections[0].hash = hash_alg_to_tpm_alg_id(alg);
	pcr_sel->pcrSelections[0].sizeofSelect = 3;
	pcr_sel->pcrSelections[0].pcrSelect[0] = 0;
	pcr_sel->pcrSelections[0].pcrSelect[1] = 0;
	pcr_sel->pcrSelections[0].pcrSelect[2] = 0;

	/* set the selected PCRs */
	for (pcr = 0; pcr < PLATFORM_PCR; pcr++)
	{
		if (pcrs & (1 << pcr))
		{
			pcr_sel->pcrSelections[0].pcrSelect[pcr / 8] |= ( 1 << (pcr % 8) );
		}
	}
	return TRUE;
}

METHOD(tpm_tss_t, read_pcr, bool,
	private_tpm_tss_tss2_t *this, uint32_t pcr_num, chunk_t *pcr_value,
	hash_algorithm_t alg)
{
	TPML_PCR_SELECTION pcr_selection;
	TPML_DIGEST pcr_values;

	uint32_t pcr_update_counter, rval;
	uint8_t *pcr_value_ptr;
	size_t   pcr_value_len;

	if (pcr_num >= PLATFORM_PCR)
	{
		DBG1(DBG_PTS, "%s maximum number of supported PCR is %d",
					   LABEL, PLATFORM_PCR);
		return FALSE;
	}

	if (!init_pcr_selection(this, (1 << pcr_num), alg, &pcr_selection))
	{
		return FALSE;
	}

	/* initialize the PCR Digest structure */
	memset(&pcr_values, 0, sizeof(TPML_DIGEST));

	/* read the PCR value */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_PCR_Read(this->sys_context, 0, &pcr_selection,
				&pcr_update_counter, &pcr_selection, &pcr_values, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s PCR bank could not be read: 0x%60x",
					   LABEL, rval);
		return FALSE;
	}
	pcr_value_ptr = (uint8_t *)pcr_values.digests[0].t.buffer;
	pcr_value_len = (size_t)   pcr_values.digests[0].t.size;

	*pcr_value = chunk_clone(chunk_create(pcr_value_ptr, pcr_value_len));

	return TRUE;
}

METHOD(tpm_tss_t, extend_pcr, bool,
	private_tpm_tss_tss2_t *this, uint32_t pcr_num, chunk_t *pcr_value,
	chunk_t data, hash_algorithm_t alg)
{
	uint32_t rval;
	TPML_DIGEST_VALUES digest_values;
	TPMS_AUTH_COMMAND  session_data_cmd;
	TPMS_AUTH_RESPONSE session_data_rsp;
	TSS2_SYS_CMD_AUTHS sessions_data_cmd;
	TSS2_SYS_RSP_AUTHS sessions_data_rsp;
	TPMS_AUTH_COMMAND  *session_data_cmd_array[1];
	TPMS_AUTH_RESPONSE *session_data_rsp_array[1];

	session_data_cmd_array[0] = &session_data_cmd;
	session_data_rsp_array[0] = &session_data_rsp;

	sessions_data_cmd.cmdAuths = &session_data_cmd_array[0];
	sessions_data_rsp.rspAuths = &session_data_rsp_array[0];

	sessions_data_cmd.cmdAuthsCount = 1;
	sessions_data_rsp.rspAuthsCount = 1;

	session_data_cmd.sessionHandle = TPM_RS_PW;
	session_data_cmd.hmac.t.size = 0;
	session_data_cmd.nonce.t.size = 0;

	*( (uint8_t *)((void *)&session_data_cmd.sessionAttributes ) ) = 0;

	/* check if there is an assigned PCR bank for this hash algorithm */
	if (!has_pcr_bank(this, alg))
	{
		DBG1(DBG_PTS, "%s %N hash algorithm not supported by any PCR bank",
			 LABEL, hash_algorithm_short_names, alg);
		return FALSE;
	}

	digest_values.count = 1;
	digest_values.digests[0].hashAlg = hash_alg_to_tpm_alg_id(alg);

	switch (alg)
	{
		case HASH_SHA1:
			if (data.len != HASH_SIZE_SHA1)
			{
				return FALSE;
			}
			memcpy(digest_values.digests[0].digest.sha1, data.ptr,
				   HASH_SIZE_SHA1);
			break;
		case HASH_SHA256:
			if (data.len != HASH_SIZE_SHA256)
			{
				return FALSE;
			}
			memcpy(digest_values.digests[0].digest.sha256, data.ptr,
				    HASH_SIZE_SHA256);
			break;
		case HASH_SHA384:
			if (data.len != HASH_SIZE_SHA384)
			{
				return FALSE;
			}
			memcpy(digest_values.digests[0].digest.sha384, data.ptr,
				    HASH_SIZE_SHA384);
			break;
		case HASH_SHA512:
			if (data.len != HASH_SIZE_SHA512)
			{
				return FALSE;
			}
			memcpy(digest_values.digests[0].digest.sha512, data.ptr,
				    HASH_SIZE_SHA512);
			break;
		default:
			return FALSE;
	}

	/* extend PCR */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_PCR_Extend(this->sys_context, pcr_num, &sessions_data_cmd,
							   &digest_values, &sessions_data_rsp);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS, "%s PCR %02u could not be extended: 0x%06x",
			 LABEL, pcr_num, rval);
		return FALSE;
	}

	/* get updated PCR value */
	return read_pcr(this, pcr_num, pcr_value, alg);
}

METHOD(tpm_tss_t, quote, bool,
	private_tpm_tss_tss2_t *this, uint32_t aik_handle, uint32_t pcr_sel,
	hash_algorithm_t alg, chunk_t data, tpm_quote_mode_t *quote_mode,
	tpm_tss_quote_info_t **quote_info, chunk_t *quote_sig)
{
	chunk_t quoted_chunk, qualified_signer, extra_data, clock_info,
			firmware_version, pcr_select, pcr_digest;
	hash_algorithm_t pcr_digest_alg;
	bio_reader_t *reader;
	uint32_t rval;

	TPM2B_DATA qualifying_data;
	TPML_PCR_SELECTION  pcr_selection;
	TPM2B_ATTEST quoted = { { sizeof(TPM2B_ATTEST)-2, } };
	TPMT_SIG_SCHEME scheme;
	TPMT_SIGNATURE sig;
	TPMI_ALG_HASH hash_alg;
	TPMS_AUTH_COMMAND  session_data_cmd;
	TPMS_AUTH_RESPONSE session_data_rsp;
	TSS2_SYS_CMD_AUTHS sessions_data_cmd;
	TSS2_SYS_RSP_AUTHS sessions_data_rsp;
	TPMS_AUTH_COMMAND  *session_data_cmd_array[1];
	TPMS_AUTH_RESPONSE *session_data_rsp_array[1];

	session_data_cmd_array[0] = &session_data_cmd;
	session_data_rsp_array[0] = &session_data_rsp;

	sessions_data_cmd.cmdAuths = &session_data_cmd_array[0];
	sessions_data_rsp.rspAuths = &session_data_rsp_array[0];

	sessions_data_cmd.cmdAuthsCount = 1;
	sessions_data_rsp.rspAuthsCount = 1;

	session_data_cmd.sessionHandle = TPM_RS_PW;
	session_data_cmd.hmac.t.size = 0;
	session_data_cmd.nonce.t.size = 0;

	*( (uint8_t *)((void *)&session_data_cmd.sessionAttributes ) ) = 0;

	qualifying_data.t.size = data.len;
	memcpy(qualifying_data.t.buffer, data.ptr, data.len);

	scheme.scheme = TPM_ALG_NULL;
	memset(&sig, 0x00, sizeof(sig));

	/* set Quote mode */
	*quote_mode = TPM_QUOTE_TPM2;

	if (!init_pcr_selection(this, pcr_sel, alg, &pcr_selection))
	{
		return FALSE;
	}

	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_Quote(this->sys_context, aik_handle, &sessions_data_cmd,
						  &qualifying_data, &scheme, &pcr_selection,  &quoted,
						  &sig, &sessions_data_rsp);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tss2_Sys_Quote failed: 0x%06x", LABEL, rval);
		return FALSE;
	}
	quoted_chunk = chunk_create(quoted.t.attestationData, quoted.t.size);

	reader = bio_reader_create(chunk_skip(quoted_chunk, 6));
	if (!reader->read_data16(reader, &qualified_signer) ||
		!reader->read_data16(reader, &extra_data) ||
		!reader->read_data  (reader, 17, &clock_info) ||
		!reader->read_data  (reader,  8, &firmware_version) ||
		!reader->read_data  (reader, 10, &pcr_select) ||
		!reader->read_data16(reader, &pcr_digest))
	{
		DBG1(DBG_PTS, "%s parsing of quoted struct failed", LABEL);
		reader->destroy(reader);
		return FALSE;
	}
	reader->destroy(reader);

	DBG2(DBG_PTS, "PCR Composite digest: %B", &pcr_digest);
	DBG2(DBG_PTS, "TPM Quote Info: %B", &quoted_chunk);
	DBG2(DBG_PTS, "qualifiedSigner: %B", &qualified_signer);
	DBG2(DBG_PTS, "extraData: %B", &extra_data);
	DBG2(DBG_PTS, "clockInfo: %B", &clock_info);
	DBG2(DBG_PTS, "firmwareVersion: %B", &firmware_version);
	DBG2(DBG_PTS, "pcrSelect: %B", &pcr_select);

	/* extract signature */
	switch (sig.sigAlg)
	{
		case TPM_ALG_RSASSA:
		case TPM_ALG_RSAPSS:
			*quote_sig = chunk_clone(
							chunk_create(
								sig.signature.rsassa.sig.t.buffer,
								sig.signature.rsassa.sig.t.size));
			hash_alg = sig.signature.rsassa.hash;
			break;
		case TPM_ALG_ECDSA:
		case TPM_ALG_ECDAA:
		case TPM_ALG_SM2:
		case TPM_ALG_ECSCHNORR:
			*quote_sig = chunk_cat("cc",
							chunk_create(
								sig.signature.ecdsa.signatureR.t.buffer,
								sig.signature.ecdsa.signatureR.t.size),
							chunk_create(
								sig.signature.ecdsa.signatureS.t.buffer,
								sig.signature.ecdsa.signatureS.t.size));
			hash_alg = sig.signature.ecdsa.hash;
			break;
		default:
			DBG1(DBG_PTS, "%s unsupported %N signature algorithm",
						   LABEL, tpm_alg_id_names, sig.sigAlg);
			return FALSE;
	}

	DBG2(DBG_PTS, "PCR digest algorithm is %N", tpm_alg_id_names, hash_alg);
	pcr_digest_alg = hash_alg_from_tpm_alg_id(hash_alg);

	DBG2(DBG_PTS, "TPM Quote Signature: %B", quote_sig);

	/* Create and initialize Quote Info object */
	*quote_info = tpm_tss_quote_info_create(*quote_mode, pcr_digest_alg,
														 pcr_digest);
	(*quote_info)->set_tpm2_info(*quote_info, qualified_signer, clock_info,
														 pcr_select);
	(*quote_info)->set_version_info(*quote_info, firmware_version);

	return TRUE;
}

METHOD(tpm_tss_t, sign, bool,
	private_tpm_tss_tss2_t *this, uint32_t hierarchy, uint32_t handle,
	signature_scheme_t scheme, void *params, chunk_t data, chunk_t pin,
	chunk_t *signature)
{
	key_type_t key_type;
	hash_algorithm_t hash_alg;
	rsa_pss_params_t *rsa_pss_params;
	uint32_t rval;

	TPM_ALG_ID alg_id;
	TPM2B_MAX_BUFFER buffer;
	TPM2B_DIGEST hash = { { sizeof(TPM2B_DIGEST)-2, } };
	TPMT_TK_HASHCHECK validation;
	TPM2B_PUBLIC public = { { 0, } };
	TPMT_SIG_SCHEME sig_scheme;
	TPMT_SIGNATURE sig;
	TPMS_AUTH_COMMAND  session_data_cmd;
	TPMS_AUTH_RESPONSE session_data_rsp;
	TSS2_SYS_CMD_AUTHS sessions_data_cmd;
	TSS2_SYS_RSP_AUTHS sessions_data_rsp;
	TPMS_AUTH_COMMAND  *session_data_cmd_array[1];
	TPMS_AUTH_RESPONSE *session_data_rsp_array[1];

	session_data_cmd_array[0] = &session_data_cmd;
	session_data_rsp_array[0] = &session_data_rsp;

	sessions_data_cmd.cmdAuths = &session_data_cmd_array[0];
	sessions_data_rsp.rspAuths = &session_data_rsp_array[0];

	sessions_data_cmd.cmdAuthsCount = 1;
	sessions_data_rsp.rspAuthsCount = 1;

	session_data_cmd.sessionHandle = TPM_RS_PW;
	session_data_cmd.nonce.t.size = 0;
	session_data_cmd.hmac.t.size = 0;

	if (pin.len > 0)
	{
		session_data_cmd.hmac.t.size = min(sizeof(session_data_cmd.hmac.t) - 2,
										   pin.len);
		memcpy(session_data_cmd.hmac.t.buffer, pin.ptr,
			   session_data_cmd.hmac.t.size);
	}
	*( (uint8_t *)((void *)&session_data_cmd.sessionAttributes ) ) = 0;

	if (scheme == SIGN_RSA_EMSA_PSS)
	{
		key_type = KEY_RSA;
		rsa_pss_params = (rsa_pss_params_t *)params;
		hash_alg = rsa_pss_params->hash;
	}
	else
	{
		key_type = key_type_from_signature_scheme(scheme);
		hash_alg = hasher_from_signature_scheme(scheme, NULL);
	}

	/* Check if hash algorithm is supported by TPM */
	alg_id = hash_alg_to_tpm_alg_id(hash_alg);
	if (!is_supported_alg(this, alg_id))
	{
		DBG1(DBG_PTS, "%s %N hash algorithm not supported by TPM",
			 LABEL, hash_algorithm_short_names, hash_alg);
		return FALSE;
	}

	/* Get public key */
	if (!read_public(this, handle, &public))
	{
		return FALSE;
	}

	if (key_type == KEY_RSA && public.t.publicArea.type == TPM_ALG_RSA)
	{
		if (scheme == SIGN_RSA_EMSA_PSS)
		{
			sig_scheme.scheme = TPM_ALG_RSAPSS;
			sig_scheme.details.rsapss.hashAlg = alg_id;
		}
		else
		{
			sig_scheme.scheme = TPM_ALG_RSASSA;
			sig_scheme.details.rsassa.hashAlg = alg_id;
		}
	}
	else if (key_type == KEY_ECDSA && public.t.publicArea.type == TPM_ALG_ECC)
	{
		sig_scheme.scheme = TPM_ALG_ECDSA;
		sig_scheme.details.ecdsa.hashAlg = alg_id;

	}
	else
	{
		DBG1(DBG_PTS, "%s signature scheme %N not supported by TPM key",
			 LABEL, signature_scheme_names, scheme);
		return FALSE;
	}

	if (data.len <= MAX_DIGEST_BUFFER)
	{
		memcpy(buffer.t.buffer, data.ptr, data.len);
		buffer.t.size = data.len;

		this->mutex->lock(this->mutex);
		rval = Tss2_Sys_Hash(this->sys_context, 0, &buffer, alg_id, hierarchy,
							 &hash, &validation, 0);
		this->mutex->unlock(this->mutex);
		if (rval != TPM_RC_SUCCESS)
		{
			DBG1(DBG_PTS,"%s Tss2_Sys_Hash failed: 0x%06x", LABEL, rval);
			return FALSE;
		}
	}
	else
	{
	    TPMI_DH_OBJECT sequence_handle;
	    TPM2B_AUTH null_auth;

		null_auth.t.size = 0;
		this->mutex->lock(this->mutex);
		rval = Tss2_Sys_HashSequenceStart(this->sys_context, 0, &null_auth,
										  alg_id, &sequence_handle, 0);
		if (rval != TPM_RC_SUCCESS)
		{
			DBG1(DBG_PTS,"%s Tss2_Sys_HashSequenceStart failed: 0x%06x",
				 LABEL, rval);
			this->mutex->unlock(this->mutex);
			return FALSE;
		}

		while (data.len > 0)
		{
			buffer.t.size = min(data.len, MAX_DIGEST_BUFFER);
			memcpy(buffer.t.buffer, data.ptr, buffer.t.size);
			data.ptr += buffer.t.size;
			data.len -= buffer.t.size;

			rval = Tss2_Sys_SequenceUpdate(this->sys_context, sequence_handle,
										   &sessions_data_cmd, &buffer, 0);
			if (rval != TPM_RC_SUCCESS)
			{
				DBG1(DBG_PTS,"%s Tss2_Sys_SequenceUpdate failed: 0x%06x",
					 LABEL, rval);
				this->mutex->unlock(this->mutex);
				return FALSE;
			}
		}
		buffer.t.size = 0;

		rval = Tss2_Sys_SequenceComplete(this->sys_context, sequence_handle,
										 &sessions_data_cmd, &buffer, hierarchy,
										 &hash, &validation, 0);
		this->mutex->unlock(this->mutex);
		if (rval != TPM_RC_SUCCESS)
		{
			DBG1(DBG_PTS,"%s Tss2_Sys_SequenceComplete failed: 0x%06x",
				 LABEL, rval);
			return FALSE;
		}
	}

	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_Sign(this->sys_context, handle, &sessions_data_cmd, &hash,
						 &sig_scheme, &validation, &sig, &sessions_data_rsp);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tss2_Sys_Sign failed: 0x%06x", LABEL, rval);
		return FALSE;
	}

	/* extract signature */
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_SHA1:
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			*signature = chunk_clone(
							chunk_create(
								sig.signature.rsassa.sig.t.buffer,
								sig.signature.rsassa.sig.t.size));
			break;
		case SIGN_RSA_EMSA_PSS:
			*signature = chunk_clone(
							chunk_create(
								sig.signature.rsapss.sig.t.buffer,
								sig.signature.rsapss.sig.t.size));
			break;
		case SIGN_ECDSA_256:
		case SIGN_ECDSA_384:
		case SIGN_ECDSA_521:
			*signature = chunk_cat("cc",
							chunk_create(
								sig.signature.ecdsa.signatureR.t.buffer,
								sig.signature.ecdsa.signatureR.t.size),
							chunk_create(
								sig.signature.ecdsa.signatureS.t.buffer,
								sig.signature.ecdsa.signatureS.t.size));
			break;
		case SIGN_ECDSA_WITH_SHA256_DER:
		case SIGN_ECDSA_WITH_SHA384_DER:
		case SIGN_ECDSA_WITH_SHA512_DER:
			*signature = asn1_wrap(ASN1_SEQUENCE, "mm",
							asn1_integer("c",
								chunk_create(
									sig.signature.ecdsa.signatureR.t.buffer,
									sig.signature.ecdsa.signatureR.t.size)),
							asn1_integer("c",
								chunk_create(
									sig.signature.ecdsa.signatureS.t.buffer,
									sig.signature.ecdsa.signatureS.t.size)));
			break;
		default:
			DBG1(DBG_PTS, "%s unsupported %N signature scheme",
						   LABEL, signature_scheme_names, scheme);
			return FALSE;
	}

	return TRUE;
}

METHOD(tpm_tss_t, get_random, bool,
	private_tpm_tss_tss2_t *this, size_t bytes, uint8_t *buffer)
{
	size_t len, random_len= sizeof(TPM2B_DIGEST)-2;
	TPM2B_DIGEST random = { { random_len, } };
	uint8_t *pos = buffer;
	uint32_t rval;

	while (bytes > 0)
	{
		len = min(bytes, random_len);

		this->mutex->lock(this->mutex);
		rval = Tss2_Sys_GetRandom(this->sys_context, NULL, len, &random, NULL);
		this->mutex->unlock(this->mutex);
		if (rval != TSS2_RC_SUCCESS)
		{
			DBG1(DBG_PTS,"%s Tss2_Sys_GetRandom failed: 0x%06x", LABEL, rval);
			return FALSE;
	    }
		memcpy(pos, random.t.buffer, random.t.size);
		pos   += random.t.size;
		bytes -= random.t.size;
	}

	return TRUE;
}

METHOD(tpm_tss_t, get_data, bool,
	private_tpm_tss_tss2_t *this, uint32_t hierarchy, uint32_t handle,
	chunk_t pin, chunk_t *data)
{
	uint16_t max_data_size, nv_size, nv_offset = 0;
	uint32_t rval;

	TPMS_CAPABILITY_DATA cap_data;
	TPMI_YES_NO more_data;
	TPM2B_NAME nv_name = { { sizeof(TPM2B_NAME)-2, } };
	TPM2B_NV_PUBLIC nv_public = { { 0, } };
	TPM2B_MAX_NV_BUFFER nv_data = { { MAX_NV_BUFFER_SIZE, } };
	TPMS_AUTH_COMMAND  session_data_cmd;
	TPMS_AUTH_RESPONSE session_data_rsp;
	TSS2_SYS_CMD_AUTHS sessions_data_cmd;
	TSS2_SYS_RSP_AUTHS sessions_data_rsp;
	TPMS_AUTH_COMMAND  *session_data_cmd_array[1];
	TPMS_AUTH_RESPONSE *session_data_rsp_array[1];

	/* query maximum TPM data transmission size */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_GetCapability(this->sys_context, 0, TPM_CAP_TPM_PROPERTIES,
				TPM_PT_NV_BUFFER_MAX, 1, &more_data, &cap_data, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tss2_Sys_GetCapability failed for "
					 "TPM_CAP_TPM_PROPERTIES: 0x%06x", LABEL, rval);
		return FALSE;
	}
	max_data_size = min(cap_data.data.tpmProperties.tpmProperty[0].value,
						MAX_NV_BUFFER_SIZE);

	/* get size of NV object */
	this->mutex->lock(this->mutex);
	rval = Tss2_Sys_NV_ReadPublic(this->sys_context, handle, 0, &nv_public,
																&nv_name, 0);
	this->mutex->unlock(this->mutex);
	if (rval != TPM_RC_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tss2_Sys_NV_ReadPublic failed: 0x%06x", LABEL, rval);
		return FALSE;
	}
	nv_size = nv_public.t.nvPublic.dataSize;
	*data = chunk_alloc(nv_size);

	/*prepare NV read session */
	session_data_cmd_array[0] = &session_data_cmd;
	session_data_rsp_array[0] = &session_data_rsp;

	sessions_data_cmd.cmdAuths = &session_data_cmd_array[0];
	sessions_data_rsp.rspAuths = &session_data_rsp_array[0];

	sessions_data_cmd.cmdAuthsCount = 1;
	sessions_data_rsp.rspAuthsCount = 1;

	session_data_cmd.sessionHandle = TPM_RS_PW;
	session_data_cmd.nonce.t.size = 0;
	session_data_cmd.hmac.t.size = 0;

	if (pin.len > 0)
	{
		session_data_cmd.hmac.t.size = min(sizeof(session_data_cmd.hmac.t) - 2,
										   pin.len);
		memcpy(session_data_cmd.hmac.t.buffer, pin.ptr,
			   session_data_cmd.hmac.t.size);
	}
	*( (uint8_t *)((void *)&session_data_cmd.sessionAttributes ) ) = 0;

	/* read NV data a maximum data size block at a time */
	while (nv_size > 0)
	{
		this->mutex->lock(this->mutex);
		rval = Tss2_Sys_NV_Read(this->sys_context, hierarchy, handle,
					&sessions_data_cmd, min(nv_size, max_data_size),
					nv_offset, &nv_data, &sessions_data_rsp);
		this->mutex->unlock(this->mutex);
		if (rval != TPM_RC_SUCCESS)
		{
			DBG1(DBG_PTS,"%s Tss2_Sys_NV_Read failed: 0x%06x", LABEL, rval);
			chunk_free(data);
			return FALSE;
		}
		memcpy(data->ptr + nv_offset, nv_data.t.buffer, nv_data.t.size);
		nv_offset += nv_data.t.size;
		nv_size   -= nv_data.t.size;
	}

	return TRUE;
}

METHOD(tpm_tss_t, get_event_digest, bool,
	private_tpm_tss_tss2_t *this, int fd, hash_algorithm_t alg, chunk_t *digest)
{
	uint8_t digest_buf[HASH_SIZE_SHA512];
	uint32_t digest_count;
	size_t digest_len = 0;
	hash_algorithm_t hash_alg;
	TPM_ALG_ID alg_id;

	if (read(fd, &digest_count, 4) != 4)
	{
		return FALSE;
	}
	while (digest_count--)
	{
		if (read(fd, &alg_id, 2) != 2)
		{
			return FALSE;
		}
		hash_alg = hash_alg_from_tpm_alg_id(alg_id);

		switch (hash_alg)
		{
			case HASH_SHA1:
				digest_len = HASH_SIZE_SHA1;
				break;
			case HASH_SHA256:
				digest_len = HASH_SIZE_SHA256;
				break;
			case HASH_SHA384:
				digest_len = HASH_SIZE_SHA384;
				break;
			case HASH_SHA512:
				digest_len = HASH_SIZE_SHA512;
				break;
			default:
				DBG2(DBG_PTS, "alg_id: 0x%04x", alg_id);
				return FALSE;
		}
		if (hash_alg == alg)
		{
			*digest = chunk_alloc(digest_len);
			if (read(fd, digest->ptr, digest_len) != digest_len)
			{
				return FALSE;
			}
		}
		else
		{
			/* read without storing */
			if (read(fd, digest_buf, digest_len) != digest_len)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

METHOD(tpm_tss_t, destroy, void,
	private_tpm_tss_tss2_t *this)
{
	finalize_context(this);
	this->mutex->destroy(this->mutex);
	free(this->version_info.ptr);
	free(this);
}

/**
 * See header
 */
tpm_tss_t *tpm_tss_tss2_create()
{
	private_tpm_tss_tss2_t *this;
	bool available;

	INIT(this,
		.public = {
			.get_version = _get_version,
			.get_version_info = _get_version_info,
			.generate_aik = _generate_aik,
			.get_public = _get_public,
			.supported_signature_schemes = _supported_signature_schemes,
			.has_pcr_bank = _has_pcr_bank,
			.read_pcr = _read_pcr,
			.extend_pcr = _extend_pcr,
			.quote = _quote,
			.sign = _sign,
			.get_random = _get_random,
			.get_data = _get_data,
			.get_event_digest = _get_event_digest,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	available = initialize_tcti_tabrmd_context(this);
	if (!available)
	{
		available = initialize_tcti_socket_context(this);
	}
	if (available)
	{
		available = initialize_sys_context(this);
	}
	DBG1(DBG_PTS, "TPM 2.0 via TSS2 v1 %savailable", available ? "" : "not ");

	if (!available)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#else /* TSS_TSS2_V1 */

#ifndef TSS_TSS2_V2
tpm_tss_t *tpm_tss_tss2_create(void)
{
	return NULL;
}
#endif /* !TSS_TSS2_V2 */

#endif /* TSS_TSS2_V1 */


