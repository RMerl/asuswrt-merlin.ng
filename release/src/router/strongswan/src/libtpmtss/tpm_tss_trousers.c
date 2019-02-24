/*
 * Copyright (C) 2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (c) 2008 Hal Finney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "tpm_tss_trousers.h"

#ifdef TSS_TROUSERS

#ifdef _BASETSD_H_
/* MinGW defines _BASETSD_H_, but TSS checks for _BASETSD_H */
# define _BASETSD_H
#endif

#include <trousers/tss.h>
#include <trousers/trousers.h>

#define LABEL	"TPM 1.2 -"

/* size in bytes of a TSS AIK public key blob */
#define AIK_PUBKEY_BLOB_SIZE		284

/* maximum number of PCR registers */
#define PCR_NUM_MAX					24

typedef struct private_tpm_tss_trousers_t private_tpm_tss_trousers_t;
typedef struct aik_t aik_t;

/**
 * Private data of an tpm_tss_trousers_t object.
 */
struct private_tpm_tss_trousers_t {

	/**
	 * Public tpm_tss_trousers_t interface.
	 */
	tpm_tss_trousers_t interface;

	/**
	 * TSS context
	 */
	TSS_HCONTEXT hContext;

	/**
	 * TPM handle
	 */
	TSS_HTPM hTPM;

	/**
	 * TPM version info
	 */
	chunk_t version_info;

	/**
	 * List of AIKs retrievable by an object handle
	 */
	linked_list_t *aik_list;

};

struct aik_t {
	/** AIK object handle */
	uint32_t handle;

	/** AIK private key blob */
	chunk_t blob;

	/** AIK public key */
	chunk_t pubkey;
};

static void free_aik(aik_t *this)
{
	free(this->blob.ptr);
	free(this->pubkey.ptr);
	free(this);
}

/**
 * Initialize TSS context
 *
 * TPM 1.2 Specification, Part 2 TPM Structures, 21.6 TPM_CAP_VERSION_INFO
 *
 *   typedef struct tdTPM_VERSION {
 *       TPM_VERSION_BYTE major;
 *       TPM_VERSION_BYTE minor;
 *       BYTE revMajor;
 *       BYTE revMinor;
 *   } TPM_VERSION;
 *
 *   typedef struct tdTPM_CAP_VERSION_INFO {
 *       TPM_STRUCTURE_TAG tag;
 *       TPM_VERSION version;
 *       UINT16 specLevel;
 *       BYTE errataRev;
 *       BYTE tpmVendorID[4];
 *       UINT16 vendorSpecificSize;
 *       [size_is(vendorSpecificSize)] BYTE* vendorSpecific;
 *   } TPM_CAP_VERSION_INFO;
 */
static bool initialize_context(private_tpm_tss_trousers_t *this)
{
	uint8_t *version_ptr;
	uint32_t version_len;

	TSS_RESULT result;
	TPM_CAP_VERSION_INFO *info;

	result = Tspi_Context_Create(&this->hContext);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not created context: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	result = Tspi_Context_Connect(this->hContext, NULL);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not connect with context: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	result = Tspi_Context_GetTpmObject (this->hContext, &this->hTPM);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s could not get TPM object: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	result = Tspi_TPM_GetCapability(this->hTPM, TSS_TPMCAP_VERSION_VAL,  0,
									NULL, &version_len, &version_ptr);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_TPM_GetCapability failed: 0x%x",
						LABEL, result);
		return FALSE;
	}

	info = (TPM_CAP_VERSION_INFO *)version_ptr;
	DBG2(DBG_PTS, "TPM Version Info: Chip Version: %u.%u.%u.%u, "
		 "Spec Level: %u, Errata Rev: %u, Vendor ID: %.4s",
		 info->version.major, info->version.minor,
		 info->version.revMajor, info->version.revMinor,
		 untoh16(&info->specLevel), info->errataRev, info->tpmVendorID);

	this->version_info = chunk_clone(chunk_create(version_ptr, version_len));

	return TRUE;
}

/**
 * Finalize TSS context
 */
static void finalize_context(private_tpm_tss_trousers_t *this)
{
	if (this->hContext)
	{
		Tspi_Context_FreeMemory(this->hContext, NULL);
		Tspi_Context_Close(this->hContext);
	}
}

METHOD(tpm_tss_t, get_version, tpm_version_t,
	private_tpm_tss_trousers_t *this)
{
	return TPM_VERSION_1_2;
}

METHOD(tpm_tss_t, get_version_info, chunk_t,
	private_tpm_tss_trousers_t *this)
{
	return this->version_info;
}

METHOD(tpm_tss_t, generate_aik, bool,
	private_tpm_tss_trousers_t *this, chunk_t ca_modulus, chunk_t *aik_blob,
	chunk_t *aik_pubkey, chunk_t *identity_req)
{
	chunk_t aik_pubkey_blob;
	chunk_t aik_modulus;
	chunk_t aik_exponent;

	TSS_RESULT   result;
	TSS_HKEY     hSRK;
	TSS_HKEY     hPCAKey;
	TSS_HPOLICY  hSrkPolicy;
	TSS_HPOLICY  hTPMPolicy;
	TSS_HKEY     hIdentKey;
	TSS_UUID     SRK_UUID = TSS_UUID_SRK;
	BYTE         secret[] = TSS_WELL_KNOWN_SECRET;
	BYTE        *IdentityReq;
	UINT32       IdentityReqLen;
	BYTE        *blob;
	UINT32       blobLen;

	/* get SRK plus SRK policy and set SRK secret */
	result = Tspi_Context_LoadKeyByUUID(this->hContext, TSS_PS_TYPE_SYSTEM,
										SRK_UUID, &hSRK);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_LoadKeyByUUID for SRK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_GetPolicyObject(hSRK, TSS_POLICY_USAGE, &hSrkPolicy);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_GetPolicyObject or SRK failed: 0x%x ",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_Policy_SetSecret(hSrkPolicy, TSS_SECRET_MODE_SHA1, 20, secret);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Policy_SetSecret for SRK failed: 0x%x ",
					   LABEL, result);
		return FALSE;
	}

	/* get TPM plus TPM policy and set TPM secret */
	result = Tspi_Context_GetTpmObject (this->hContext, &this->hTPM);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_GetTpmObject failed:  0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_GetPolicyObject(this->hTPM, TSS_POLICY_USAGE, &hTPMPolicy);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_GetPolicyObject for TPM failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_Policy_SetSecret(hTPMPolicy, TSS_SECRET_MODE_SHA1, 20, secret);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tspi_Policy_SetSecret for TPM failed: 0x%x",
					  LABEL, result);
		return FALSE;
	}

	/* create context for a 2048 bit AIK */
	result = Tspi_Context_CreateObject(this->hContext, TSS_OBJECT_TYPE_RSAKEY,
					TSS_KEY_TYPE_IDENTITY | TSS_KEY_SIZE_2048 |
					TSS_KEY_VOLATILE | TSS_KEY_NOT_MIGRATABLE, &hIdentKey);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_CreateObject for key failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	/* create context for the Privacy CA public key and assign modulus */
	result = Tspi_Context_CreateObject(this->hContext, TSS_OBJECT_TYPE_RSAKEY,
					TSS_KEY_TYPE_LEGACY|TSS_KEY_SIZE_2048, &hPCAKey);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_CreateObject for PCA failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_SetAttribData (hPCAKey, TSS_TSPATTRIB_RSAKEY_INFO,
					TSS_TSPATTRIB_KEYINFO_RSA_MODULUS, ca_modulus.len,
					ca_modulus.ptr);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_SetAttribData for PCA modulus failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_SetAttribUint32(hPCAKey, TSS_TSPATTRIB_KEY_INFO,
					TSS_TSPATTRIB_KEYINFO_ENCSCHEME, TSS_ES_RSAESPKCSV15);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS,"%s Tspi_SetAttribUint32 for PCA encryption scheme "
					 "failed: 0x%x", LABEL, result);
		return FALSE;
	}

	/* generate AIK */
	DBG1(DBG_LIB, "Generating identity key...");
	result = Tspi_TPM_CollateIdentityRequest(this->hTPM, hSRK, hPCAKey, 0, NULL,
					hIdentKey, TSS_ALG_AES,	&IdentityReqLen, &IdentityReq);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_TPM_CollateIdentityRequest failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	*identity_req = chunk_create(IdentityReq, IdentityReqLen);
	DBG3(DBG_LIB, "%s Identity Request: %B", LABEL, identity_req);

	/* load identity key */
	result = Tspi_Key_LoadKey (hIdentKey, hSRK);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Key_LoadKey for AIK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	/* output AIK private key in TSS blob format */
	result = Tspi_GetAttribData (hIdentKey, TSS_TSPATTRIB_KEY_BLOB,
					TSS_TSPATTRIB_KEYBLOB_BLOB, &blobLen, &blob);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_GetAttribData for private key blob failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	*aik_blob = chunk_create(blob, blobLen);
	DBG3(DBG_LIB, "%s AIK private key blob: %B", LABEL, aik_blob);

	/* output AIK Public Key in TSS blob format */
	result = Tspi_GetAttribData (hIdentKey, TSS_TSPATTRIB_KEY_BLOB,
					TSS_TSPATTRIB_KEYBLOB_PUBLIC_KEY, &blobLen, &blob);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_GetAttribData for public key blob failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	aik_pubkey_blob = chunk_create(blob, blobLen);
	DBG3(DBG_LIB, "%s AIK public key blob: %B", LABEL, &aik_pubkey_blob);

	/* create a trusted AIK public key */
	if (aik_pubkey_blob.len != AIK_PUBKEY_BLOB_SIZE)
	{
		DBG1(DBG_PTS, "%s AIK public key is not in TSS blob format",
					   LABEL);
		return FALSE;
	}
	aik_modulus = chunk_skip(aik_pubkey_blob, AIK_PUBKEY_BLOB_SIZE - 256);
	aik_exponent = chunk_from_chars(0x01, 0x00, 0x01);

	/* output subjectPublicKeyInfo encoding of AIK public key */
	if (!lib->encoding->encode(lib->encoding, PUBKEY_SPKI_ASN1_DER, NULL,
					aik_pubkey, CRED_PART_RSA_MODULUS, aik_modulus,
					CRED_PART_RSA_PUB_EXP, aik_exponent, CRED_PART_END))
	{
		DBG1(DBG_PTS, "%s subjectPublicKeyInfo encoding of AIK key failed",
					   LABEL);
		return FALSE;
	}
	return TRUE;
}

METHOD(tpm_tss_t, get_public, chunk_t,
	private_tpm_tss_trousers_t *this, uint32_t handle)
{
	enumerator_t *enumerator;
	chunk_t aik_pubkey = chunk_empty;
	aik_t *aik;

	enumerator = this->aik_list->create_enumerator(this->aik_list);
	while (enumerator->enumerate(enumerator, &aik))
	{
		if (aik->handle == handle)
		{
			aik_pubkey = chunk_clone(aik->pubkey);
			break;
		}
	}
	enumerator->destroy(enumerator);

	return aik_pubkey;
}

METHOD(tpm_tss_t, supported_signature_schemes, enumerator_t*,
	private_tpm_tss_trousers_t *this, uint32_t handle)
{
	return enumerator_create_empty();
}

METHOD(tpm_tss_t, read_pcr, bool,
	private_tpm_tss_trousers_t *this, uint32_t pcr_num, chunk_t *pcr_value,
	hash_algorithm_t alg)
{
	TSS_RESULT result;
	uint8_t *value;
	uint32_t len;

	result = Tspi_TPM_PcrRead(this->hTPM, pcr_num, &len, &value);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_TPM_PcrRead failed: 0x%x", LABEL, result);
		return FALSE;
	}
	*pcr_value = chunk_clone(chunk_create(value, len));

	return TRUE;
}

METHOD(tpm_tss_t, extend_pcr, bool,
	private_tpm_tss_trousers_t *this, uint32_t pcr_num, chunk_t *pcr_value,
	chunk_t data, hash_algorithm_t alg)
{
	TSS_RESULT result;
	uint32_t pcr_len;
	uint8_t *pcr_ptr;

	result = Tspi_TPM_PcrExtend(this->hTPM, pcr_num, data.len, data.ptr,
								NULL, &pcr_len, &pcr_ptr);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_TPM_PcrExtend failed: 0x%x", LABEL, result);
		return FALSE;
	}
	*pcr_value = chunk_clone(chunk_create(pcr_ptr, pcr_len));

	return TRUE;
}

METHOD(tpm_tss_t, quote, bool,
	private_tpm_tss_trousers_t *this, uint32_t aik_handle, uint32_t pcr_sel,
	hash_algorithm_t alg, chunk_t data, tpm_quote_mode_t *quote_mode,
	tpm_tss_quote_info_t **quote_info, chunk_t *quote_sig)
{
	TSS_HKEY hAIK;
	TSS_HKEY hSRK;
	TSS_HPOLICY srkUsagePolicy;
	TSS_UUID SRK_UUID = TSS_UUID_SRK;
	TSS_HPCRS hPcrComposite;
	TSS_VALIDATION valData;
	TSS_RESULT result;
	uint8_t secret[] = TSS_WELL_KNOWN_SECRET;
	uint8_t *version_info, *comp_hash;
	uint32_t version_info_size, pcr;
	aik_t *aik;
	chunk_t aik_blob = chunk_empty;
	chunk_t quote_chunk, pcr_digest;
	enumerator_t *enumerator;
	bool success = FALSE;

	/* Retrieve SRK from TPM and set the authentication to well known secret*/
	result = Tspi_Context_LoadKeyByUUID(this->hContext, TSS_PS_TYPE_SYSTEM,
										SRK_UUID, &hSRK);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_LoadKeyByUUID for SRK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_GetPolicyObject(hSRK, TSS_POLICY_USAGE, &srkUsagePolicy);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_GetPolicyObject for SRK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}
	result = Tspi_Policy_SetSecret(srkUsagePolicy, TSS_SECRET_MODE_SHA1,
					20, secret);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Policy_SetSecret for SRK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	/* Retrieve AIK using its handle and load private key into TPM 1.2 */
	enumerator = this->aik_list->create_enumerator(this->aik_list);
	while (enumerator->enumerate(enumerator, &aik))
	{
		if (aik->handle == aik_handle)
		{
			aik_blob = aik->blob;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (aik_blob.len == 0)
	{
		DBG1(DBG_PTS, "%s AIK private key for handle 0x%80x not found", LABEL);
		return FALSE;
	}
	result = Tspi_Context_LoadKeyByBlob(this->hContext, hSRK, aik_blob.len,
										aik_blob.ptr, &hAIK);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_LoadKeyByBlob for AIK failed: 0x%x",
					   LABEL, result);
		return FALSE;
	}

	/* Create PCR composite object */
	result = Tspi_Context_CreateObject(this->hContext, TSS_OBJECT_TYPE_PCRS,
					(*quote_mode == TPM_QUOTE) ? TSS_PCRS_STRUCT_INFO :
												 TSS_PCRS_STRUCT_INFO_SHORT,
					&hPcrComposite);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_Context_CreateObject for pcrComposite failed: "
					  "0x%x", LABEL, result);
		goto err1;
	}

	/* Select PCRs */
	for (pcr = 0; pcr < PCR_NUM_MAX; pcr++)
	{
		if (pcr_sel & (1 << pcr))
		{
			result = (*quote_mode == TPM_QUOTE) ?
				Tspi_PcrComposite_SelectPcrIndex(hPcrComposite, pcr) :
				Tspi_PcrComposite_SelectPcrIndexEx(hPcrComposite, pcr,
										TSS_PCRS_DIRECTION_RELEASE);
			if (result != TSS_SUCCESS)
			{
				DBG1(DBG_PTS, "%s Tspi_PcrComposite_SelectPcrIndex failed: "
							  "0x%x", LABEL, result);
				goto err2;
			}
		}
	}

	/* Set the Validation Data */
	valData.ulExternalDataLength = data.len;
	valData.rgbExternalData      = data.ptr;

	/* TPM Quote */
	result = (*quote_mode == TPM_QUOTE) ?
			Tspi_TPM_Quote (this->hTPM, hAIK, hPcrComposite, &valData) :
			Tspi_TPM_Quote2(this->hTPM, hAIK,
							*quote_mode == TPM_QUOTE2_VERSION_INFO,
						    hPcrComposite, &valData, &version_info_size,
							&version_info);
	if (result != TSS_SUCCESS)
	{
		DBG1(DBG_PTS, "%s Tspi_TPM_Quote%s failed: 0x%x", LABEL,
					  (*quote_mode == TPM_QUOTE) ? "" : "2", result);
		goto err2;
	}

	if (*quote_mode == TPM_QUOTE)
	{
		/* TPM_Composite_Hash starts at byte 8 of TPM_Quote_Info structure */
		comp_hash = valData.rgbData + 8;
	}
	else
	{
		/* TPM_Composite_Hash is last 20 bytes of TPM_Quote_Info2 structure */
		comp_hash = valData.rgbData + valData.ulDataLength - version_info_size -
					HASH_SIZE_SHA1;
	}
	pcr_digest = chunk_create(comp_hash, HASH_SIZE_SHA1);
	DBG2(DBG_PTS, "PCR composite digest: %B", &pcr_digest);

	quote_chunk = chunk_create(valData.rgbData, valData.ulDataLength);
	DBG2(DBG_PTS, "TPM Quote Info: %B", &quote_chunk);

	*quote_info = tpm_tss_quote_info_create(*quote_mode, HASH_SHA1, pcr_digest);

	*quote_sig = chunk_clone(chunk_create(valData.rgbValidationData,
										  valData.ulValidationDataLength));
	DBG2(DBG_PTS, "TPM Quote Signature: %B", quote_sig);

	success = TRUE;

err2:
	Tspi_Context_CloseObject(this->hContext, hPcrComposite);
err1:
	Tspi_Context_CloseObject(this->hContext, hAIK);

	return success;
}

METHOD(tpm_tss_t, sign, bool,
	private_tpm_tss_trousers_t *this, uint32_t hierarchy, uint32_t handle,
	signature_scheme_t scheme, void *params, chunk_t data, chunk_t pin,
	chunk_t *signature)
{
	return FALSE;
}

METHOD(tpm_tss_t, get_random, bool,
	private_tpm_tss_trousers_t *this, size_t bytes, uint8_t *buffer)
{
	return FALSE;
}

METHOD(tpm_tss_t, get_data, bool,
	private_tpm_tss_trousers_t *this, uint32_t hierarchy, uint32_t handle,
	chunk_t pin, chunk_t *data)
{
	return FALSE;
}

METHOD(tpm_tss_t, destroy, void,
	private_tpm_tss_trousers_t *this)
{
	finalize_context(this);
	this->aik_list->destroy_function(this->aik_list, (void*)free_aik);
	free(this->version_info.ptr);
	free(this);
}

METHOD(tpm_tss_trousers_t, load_aik, void,
	private_tpm_tss_trousers_t *this, chunk_t blob, chunk_t pubkey,
	uint32_t handle)
{
	aik_t *item;

	INIT(item,
		.handle = handle,
		.blob = blob,
		.pubkey = pubkey,
	);

	this->aik_list->insert_last(this->aik_list, item);
}

/**
 * See header
 */
tpm_tss_t *tpm_tss_trousers_create()
{
	private_tpm_tss_trousers_t *this;
	bool available;

	INIT(this,
		.interface = {
			.public = {
				.get_version = _get_version,
				.get_version_info = _get_version_info,
				.generate_aik = _generate_aik,
				.get_public = _get_public,
				.supported_signature_schemes = _supported_signature_schemes,
				.read_pcr = _read_pcr,
				.extend_pcr = _extend_pcr,
				.quote = _quote,
				.sign = _sign,
				.get_random = _get_random,
				.get_data = _get_data,
				.destroy = _destroy,
			},
			.load_aik = _load_aik,
		},
		.aik_list = linked_list_create(),
	);

	available = initialize_context(this);
	DBG1(DBG_PTS, "TPM 1.2 via TrouSerS %savailable", available ? "" : "not ");

	if (!available)
	{
		destroy(this);
		return NULL;
	}
	return &this->interface.public;
}

#else /* TSS_TROUSERS */

tpm_tss_t *tpm_tss_trousers_create()
{
	return NULL;
}

#endif /* TSS_TROUSERS */



