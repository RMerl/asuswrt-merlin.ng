/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2012-2016 Andreas Steffen
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

#include "pts.h"

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>

#include <tpm_tss.h>
#include <tpm_tss_trousers.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>

#ifndef TPM_TAG_QUOTE_INFO2
#define TPM_TAG_QUOTE_INFO2 0x0036
#endif
#ifndef TPM_LOC_ZERO
#define TPM_LOC_ZERO 0x01
#endif

typedef struct private_pts_t private_pts_t;

/**
 * Private data of a pts_t object.
 *
 */
struct private_pts_t {

	/**
	 * Public pts_t interface.
	 */
	pts_t public;

	/**
	 * PTS Protocol Capabilities
	 */
	pts_proto_caps_flag_t proto_caps;

	/**
	 * PTS Measurement Algorithm
	 */
	pts_meas_algorithms_t algorithm;

	/**
	 * DH Hash Algorithm
	 */
	pts_meas_algorithms_t dh_hash_algorithm;

	/**
	 * PTS Diffie-Hellman Secret
	 */
	diffie_hellman_t *dh;

	/**
	 * PTS Diffie-Hellman Initiator Nonce
	 */
	chunk_t initiator_nonce;

	/**
	 * PTS Diffie-Hellman Responder Nonce
	 */
	chunk_t responder_nonce;

	/**
	 * Secret assessment value to be used for TPM Quote as an external data
	 */
	chunk_t secret;

	/**
	 * Primary key of platform entry in database
	 */
	int platform_id;

	/**
	 * TRUE if IMC-PTS, FALSE if IMV-PTS
	 */
	bool is_imc;

	/**
	 * Active TPM
	 */
	tpm_tss_t *tpm;

	/**
	 * Contains a TPM_CAP_VERSION_INFO struct
	 */
	chunk_t tpm_version_info;

	/**
	 * AIK object handle
	 */
	uint32_t aik_handle;

	/**
	 * Contains an Attestation Identity Key Certificate
	 */
	certificate_t *aik_cert;

	/**
	 * Primary key referening AIK in database
	 */
	int aik_id;

	/**
	 * Shadow PCR set
	 */
	pts_pcr_t *pcrs;

};

METHOD(pts_t, get_proto_caps, pts_proto_caps_flag_t,
	   private_pts_t *this)
{
	return this->proto_caps;
}

METHOD(pts_t, set_proto_caps, void,
	private_pts_t *this, pts_proto_caps_flag_t flags)
{
	this->proto_caps = flags;
	DBG2(DBG_PTS, "supported PTS protocol capabilities: %s%s%s%s%s",
		 flags & PTS_PROTO_CAPS_C ? "C" : ".",
		 flags & PTS_PROTO_CAPS_V ? "V" : ".",
		 flags & PTS_PROTO_CAPS_D ? "D" : ".",
		 flags & PTS_PROTO_CAPS_T ? "T" : ".",
		 flags & PTS_PROTO_CAPS_X ? "X" : ".");
}

METHOD(pts_t, get_meas_algorithm, pts_meas_algorithms_t,
	private_pts_t *this)
{
	return this->algorithm;
}

METHOD(pts_t, set_meas_algorithm, void,
	private_pts_t *this, pts_meas_algorithms_t algorithm)
{
	hash_algorithm_t hash_alg;

	hash_alg = pts_meas_algo_to_hash(algorithm);
	DBG2(DBG_PTS, "selected PTS measurement algorithm is %N",
				   hash_algorithm_names, hash_alg);
	if (hash_alg != HASH_UNKNOWN)
	{
		this->algorithm = algorithm;
	}
}

METHOD(pts_t, get_dh_hash_algorithm, pts_meas_algorithms_t,
	private_pts_t *this)
{
	return this->dh_hash_algorithm;
}

METHOD(pts_t, set_dh_hash_algorithm, void,
	private_pts_t *this, pts_meas_algorithms_t algorithm)
{
	hash_algorithm_t hash_alg;

	hash_alg = pts_meas_algo_to_hash(algorithm);
	DBG2(DBG_PTS, "selected DH hash algorithm is %N",
				   hash_algorithm_names, hash_alg);
	if (hash_alg != HASH_UNKNOWN)
	{
		this->dh_hash_algorithm = algorithm;
	}
}

METHOD(pts_t, create_dh_nonce, bool,
	private_pts_t *this, pts_dh_group_t group, int nonce_len)
{
	diffie_hellman_group_t dh_group;
	chunk_t *nonce;
	rng_t *rng;

	dh_group = pts_dh_group_to_ike(group);
	DBG2(DBG_PTS, "selected PTS DH group is %N",
				   diffie_hellman_group_names, dh_group);
	DESTROY_IF(this->dh);
	this->dh = lib->crypto->create_dh(lib->crypto, dh_group);

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng)
	{
		DBG1(DBG_PTS, "no rng available");
		return FALSE;
	}
	DBG2(DBG_PTS, "nonce length is %d", nonce_len);
	nonce = this->is_imc ? &this->responder_nonce : &this->initiator_nonce;
	chunk_free(nonce);
	if (!rng->allocate_bytes(rng, nonce_len, nonce))
	{
		DBG1(DBG_PTS, "failed to allocate nonce");
		rng->destroy(rng);
		return FALSE;
	}
	rng->destroy(rng);
	return TRUE;
}

METHOD(pts_t, get_my_public_value, bool,
	private_pts_t *this, chunk_t *value, chunk_t *nonce)
{
	if (!this->dh->get_my_public_value(this->dh, value))
	{
		return FALSE;
	}
	*nonce = this->is_imc ? this->responder_nonce : this->initiator_nonce;
	return TRUE;
}

METHOD(pts_t, set_peer_public_value, bool,
	private_pts_t *this, chunk_t value, chunk_t nonce)
{
	if (!this->dh->set_other_public_value(this->dh, value))
	{
		return FALSE;
	}

	nonce = chunk_clone(nonce);
	if (this->is_imc)
	{
		this->initiator_nonce = nonce;
	}
	else
	{
		this->responder_nonce = nonce;
	}
	return TRUE;
}

METHOD(pts_t, calculate_secret, bool,
	private_pts_t *this)
{
	hasher_t *hasher;
	hash_algorithm_t hash_alg;
	chunk_t shared_secret;

	/* Check presence of nonces */
	if (!this->initiator_nonce.len || !this->responder_nonce.len)
	{
		DBG1(DBG_PTS, "initiator and/or responder nonce is not available");
		return FALSE;
	}
	DBG3(DBG_PTS, "initiator nonce: %B", &this->initiator_nonce);
	DBG3(DBG_PTS, "responder nonce: %B", &this->responder_nonce);

	/* Calculate the DH secret */
	if (!this->dh->get_shared_secret(this->dh, &shared_secret))
	{
		DBG1(DBG_PTS, "shared DH secret computation failed");
		return FALSE;
	}
	DBG3(DBG_PTS, "shared DH secret: %B", &shared_secret);

	/* Calculate the secret assessment value */
	hash_alg = pts_meas_algo_to_hash(this->dh_hash_algorithm);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_alg);

	if (!hasher ||
		!hasher->get_hash(hasher, chunk_from_chars('1'), NULL) ||
		!hasher->get_hash(hasher, this->initiator_nonce, NULL) ||
		!hasher->get_hash(hasher, this->responder_nonce, NULL) ||
		!hasher->allocate_hash(hasher, shared_secret, &this->secret))
	{
		DESTROY_IF(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	/* The DH secret must be destroyed */
	chunk_clear(&shared_secret);

	/*
	 * Truncate the hash to 20 bytes to fit the ExternalData
	 * argument of the TPM Quote command
	 */
	this->secret.len = min(this->secret.len, 20);
	DBG3(DBG_PTS, "secret assessment value: %B", &this->secret);
	return TRUE;
}

METHOD(pts_t, get_platform_id, int,
	private_pts_t *this)
{
	return this->platform_id;
}

METHOD(pts_t, set_platform_id, void,
	private_pts_t *this, int pid)
{
	this->platform_id = pid;
}

METHOD(pts_t, get_tpm_version_info, bool,
	private_pts_t *this, chunk_t *info)
{
	*info = this->tpm ? this->tpm->get_version_info(this->tpm) :
						this->tpm_version_info;
	return info->len > 0;
}

METHOD(pts_t, set_tpm_version_info, void,
	private_pts_t *this, chunk_t info)
{
	this->tpm_version_info = chunk_clone(info);
}

/**
 * Load an AIK handle and an optional AIK certificate and
 * in the case of a TPM 1.2 an AIK private key blob plus matching public key,
 * the certificate having precedence over the public key if both are present
 */
static void load_aik(private_pts_t *this)
{
	char *handle_str, *cert_path, *key_path, *blob_path;
	chunk_t aik_pubkey = chunk_empty;

	handle_str = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.aik_handle", NULL, lib->ns);
	cert_path = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.aik_cert", NULL, lib->ns);
	key_path = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.aik_pubkey", NULL, lib->ns);
	blob_path = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.aik_blob", NULL, lib->ns);

	if (handle_str)
	{
		this->aik_handle = strtoll(handle_str, NULL, 16);
	}
	if (cert_path)
	{
		this->aik_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
									   CERT_X509, BUILD_FROM_FILE,
									   cert_path, BUILD_END);
		if (this->aik_cert)
		{
			DBG2(DBG_PTS, "loaded AIK certificate from '%s'", cert_path);
		}
	}

	if (this->tpm->get_version(this->tpm) == TPM_VERSION_1_2)
	{
		tpm_tss_trousers_t *tpm_12;
		chunk_t aik_blob = chunk_empty;
		chunk_t *map;

		/* get AIK private key blob */
		if (blob_path)
		{
			map = chunk_map(blob_path, FALSE);
			if (map)
			{
				DBG2(DBG_PTS, "loaded AIK Blob from '%s'", blob_path);
				DBG3(DBG_PTS, "AIK Blob: %B", map);
				aik_blob = chunk_clone(*map);
				chunk_unmap(map);
			}
			else
			{
				DBG1(DBG_PTS, "unable to map AIK Blob file '%s': %s",
							   blob_path, strerror(errno));
			}
		}
		else
		{
			DBG1(DBG_PTS, "AIK Blob is not available");
		}

		/* get AIK public key if no AIK certificate is available */
		if (!this->aik_cert)
		{
			if (key_path)
			{
				map = chunk_map(key_path, FALSE);
				if (map)
				{
					DBG2(DBG_PTS, "loaded AIK public key from '%s'", key_path);
					aik_pubkey = chunk_clone(*map);
					chunk_unmap(map);
				}
				else
				{
					DBG1(DBG_PTS, "unable to map AIK public key file '%s': %s",
								   key_path, strerror(errno));
				}
			}
			else
			{
				DBG1(DBG_PTS, "AIK public key is not available");
			}
		}

		/* Load AIK item into TPM 1.2 object */
		tpm_12 = (tpm_tss_trousers_t *)this->tpm;
		tpm_12->load_aik(tpm_12, aik_blob, aik_pubkey, this->aik_handle);
	}

	/* if no signed X.509 AIK certificate is available use public key instead */
	if (!this->aik_cert)
	{
		aik_pubkey = this->tpm->get_public(this->tpm, this->aik_handle);
		if (aik_pubkey.len > 0)
		{
			this->aik_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
									   CERT_TRUSTED_PUBKEY, BUILD_BLOB,
									   aik_pubkey, BUILD_END);
			chunk_free(&aik_pubkey);
		}
		else
		{
			DBG1(DBG_PTS, "neither AIK certificate nor public key is available");
		}
	}
}

METHOD(pts_t, get_aik, certificate_t*,
	private_pts_t *this)
{
	return this->aik_cert;
}

METHOD(pts_t, set_aik, void,
	private_pts_t *this, certificate_t *aik, int aik_id)
{
	DESTROY_IF(this->aik_cert);
	this->aik_cert = aik->get_ref(aik);
	this->aik_id = aik_id;
}

METHOD(pts_t, get_aik_id, int,
	private_pts_t *this)
{
	return this->aik_id;
}

METHOD(pts_t, is_path_valid, bool,
	private_pts_t *this, char *path, pts_error_code_t *error_code)
{
	struct stat st;

	*error_code = 0;

	if (!stat(path, &st))
	{
		return TRUE;
	}
	else if (errno == ENOENT || errno == ENOTDIR)
	{
		DBG1(DBG_PTS, "file/directory does not exist %s", path);
		*error_code = TCG_PTS_FILE_NOT_FOUND;
	}
	else if (errno == EFAULT)
	{
		DBG1(DBG_PTS, "bad address %s", path);
		*error_code = TCG_PTS_INVALID_PATH;
	}
	else
	{
		DBG1(DBG_PTS, "error: %s occurred while validating path: %s",
			 		   strerror(errno), path);
		return FALSE;
	}

	return TRUE;
}

/**
 * Obtain statistical information describing a file
 */
static bool file_metadata(char *pathname, pts_file_metadata_t **entry)
{
	struct stat st;
	pts_file_metadata_t *this;

	this = malloc_thing(pts_file_metadata_t);

	if (stat(pathname, &st))
	{
		DBG1(DBG_PTS, "unable to obtain statistics about '%s'", pathname);
		free(this);
		return FALSE;
	}

	if (S_ISREG(st.st_mode))
	{
		this->type = PTS_FILE_REGULAR;
	}
	else if (S_ISDIR(st.st_mode))
	{
		this->type = PTS_FILE_DIRECTORY;
	}
	else if (S_ISCHR(st.st_mode))
	{
		this->type = PTS_FILE_CHAR_SPEC;
	}
	else if (S_ISBLK(st.st_mode))
	{
		this->type = PTS_FILE_BLOCK_SPEC;
	}
	else if (S_ISFIFO(st.st_mode))
	{
		this->type = PTS_FILE_FIFO;
	}
#ifndef WIN32
	else if (S_ISLNK(st.st_mode))
	{
		this->type = PTS_FILE_SYM_LINK;
	}
	else if (S_ISSOCK(st.st_mode))
	{
		this->type = PTS_FILE_SOCKET;
	}
#endif /* WIN32 */
	else
	{
		this->type = PTS_FILE_OTHER;
	}

	this->filesize = st.st_size;
	this->created =  st.st_ctime;
	this->modified = st.st_mtime;
	this->accessed = st.st_atime;
	this->owner =    st.st_uid;
	this->group =    st.st_gid;

	*entry = this;
	return TRUE;
}

METHOD(pts_t, get_metadata, pts_file_meta_t*,
	private_pts_t *this, char *pathname, bool is_directory)
{
	pts_file_meta_t *metadata;
	pts_file_metadata_t *entry;

	/* Create a metadata object */
	metadata = pts_file_meta_create();

	if (is_directory)
	{
		enumerator_t *enumerator;
		char *rel_name, *abs_name;
		struct stat st;

		enumerator = enumerator_create_directory(pathname);
		if (!enumerator)
		{
			DBG1(DBG_PTS,"  directory '%s' can not be opened, %s", pathname,
				 strerror(errno));
			metadata->destroy(metadata);
			return NULL;
		}
		while (enumerator->enumerate(enumerator, &rel_name, &abs_name, &st))
		{
			/* measure regular files only */
			if (S_ISREG(st.st_mode) && *rel_name != '.')
			{
				if (!file_metadata(abs_name, &entry))
				{
					enumerator->destroy(enumerator);
					metadata->destroy(metadata);
					return NULL;
				}
				entry->filename = strdup(rel_name);
				metadata->add(metadata, entry);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		if (!file_metadata(pathname, &entry))
		{
			metadata->destroy(metadata);
			return NULL;
		}
		entry->filename = path_basename(pathname);
		metadata->add(metadata, entry);
	}

	return metadata;
}

METHOD(pts_t, read_pcr, bool,
	private_pts_t *this, uint32_t pcr_num, chunk_t *pcr_value,
	hash_algorithm_t alg)
{
	return this->tpm ? this->tpm->read_pcr(this->tpm, pcr_num, pcr_value, alg)
				     : FALSE;
}

METHOD(pts_t, extend_pcr, bool,
	private_pts_t *this, uint32_t pcr_num, chunk_t *pcr_value, chunk_t data,
	hash_algorithm_t alg)
{
	if (!this->tpm->extend_pcr(this->tpm, pcr_num, pcr_value, data, alg))
	{
		return FALSE;
	}
	DBG3(DBG_PTS, "PCR %d extended with:   %#B", pcr_num, &data);
	DBG3(DBG_PTS, "PCR %d after extension: %#B", pcr_num, pcr_value);

	return TRUE;
}

METHOD(pts_t, quote, bool,
	private_pts_t *this, tpm_quote_mode_t *quote_mode,
	tpm_tss_quote_info_t **quote_info, chunk_t *quote_sig)
{
	chunk_t pcr_value, pcr_computed;
	uint32_t pcr, pcr_sel = 0;
	enumerator_t *enumerator;

	/* select PCRs */
	DBG2(DBG_PTS, "PCR values hashed into PCR Composite:");
	enumerator = this->pcrs->create_enumerator(this->pcrs);
	while (enumerator->enumerate(enumerator, &pcr))
	{
		if (this->tpm->read_pcr(this->tpm, pcr, &pcr_value, HASH_SHA1))
		{
			pcr_computed = this->pcrs->get(this->pcrs, pcr);
			DBG2(DBG_PTS, "PCR %2d %#B  %s", pcr, &pcr_value,
				 chunk_equals(pcr_value, pcr_computed) ? "ok" : "differs");
			chunk_free(&pcr_value);
		};

		/* add PCR to selection list */
		pcr_sel |= (1 << pcr);
	}
	enumerator->destroy(enumerator);

	/* TPM Quote */
	return this->tpm->quote(this->tpm, this->aik_handle, pcr_sel, HASH_SHA1,
							this->secret, quote_mode, quote_info, quote_sig);
}

METHOD(pts_t, get_quote, bool,
	private_pts_t *this, tpm_tss_quote_info_t *quote_info, chunk_t *quoted)
{
	tpm_tss_pcr_composite_t *pcr_composite;
	bool success;

	if (!this->pcrs->get_count(this->pcrs))
	{
		DBG1(DBG_PTS, "No extended PCR entries available, "
					  "unable to construct TPM Quote Info");
		return FALSE;
	}
	if (!this->secret.ptr)
	{
		DBG1(DBG_PTS, "Secret assessment value unavailable, ",
					  "unable to construct TPM Quote Info");
		return FALSE;
	}
	if (quote_info->get_quote_mode(quote_info) == TPM_QUOTE2_VERSION_INFO)
	{
		if (!this->tpm_version_info.ptr)
		{
			DBG1(DBG_PTS, "TPM Version Information unavailable, ",
						  "unable to construct TPM Quote Info2");
			return FALSE;
		}
		quote_info->set_version_info(quote_info, this->tpm_version_info);
	}
	pcr_composite = this->pcrs->get_composite(this->pcrs);

	success = quote_info->get_quote(quote_info, this->secret,
									pcr_composite, quoted);
	chunk_free(&pcr_composite->pcr_select);
	chunk_free(&pcr_composite->pcr_composite);
	free(pcr_composite);

	return success;
}

METHOD(pts_t, verify_quote_signature, bool,
	private_pts_t *this, hash_algorithm_t digest_alg, chunk_t digest,
	chunk_t signature)
{
	public_key_t *aik_pubkey;
	signature_scheme_t scheme;

	aik_pubkey = this->aik_cert->get_public_key(this->aik_cert);
	if (!aik_pubkey)
	{
		DBG1(DBG_PTS, "failed to get public key from AIK certificate");
		return FALSE;
	}

	/* Determine signing scheme */
	switch (aik_pubkey->get_type(aik_pubkey))
	{
		case KEY_RSA:
			switch (digest_alg)
			{
				case HASH_SHA1:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
					break;
				case HASH_SHA256:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256;
					break;
				case HASH_SHA384:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA2_384;
					break;
				case HASH_SHA512:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA2_512;
					break;
				case HASH_SHA3_256:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA3_256;
					break;
				case HASH_SHA3_384:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA3_384;
					break;
				case HASH_SHA3_512:
					scheme = SIGN_RSA_EMSA_PKCS1_SHA3_512;
					break;
				default:
					scheme = SIGN_UNKNOWN;
			}
			break;
		case KEY_ECDSA:
			switch (digest_alg)
			{
				case HASH_SHA256:
					scheme = SIGN_ECDSA_256;
					break;
				case HASH_SHA384:
					scheme = SIGN_ECDSA_384;
					break;
				case HASH_SHA512:
					scheme = SIGN_ECDSA_521;
					break;
				default:
					scheme = SIGN_UNKNOWN;
			}
			break;
		default:
			DBG1(DBG_PTS, "%N AIK key type not supported", key_type_names,
						   aik_pubkey->get_type(aik_pubkey));
			return FALSE;
	}

	if (!aik_pubkey->verify(aik_pubkey, scheme, NULL, digest, signature))
	{
		DBG1(DBG_PTS, "signature verification failed for TPM Quote Info");
		DESTROY_IF(aik_pubkey);
		return FALSE;
	}

	aik_pubkey->destroy(aik_pubkey);
	return TRUE;
}

METHOD(pts_t, get_pcrs, pts_pcr_t*,
	private_pts_t *this)
{
	return this->pcrs;
}

METHOD(pts_t, destroy, void,
	private_pts_t *this)
{
	DESTROY_IF(this->tpm);
	DESTROY_IF(this->pcrs);
	DESTROY_IF(this->aik_cert);
	DESTROY_IF(this->dh);
	free(this->initiator_nonce.ptr);
	free(this->responder_nonce.ptr);
	free(this->secret.ptr);
	free(this->tpm_version_info.ptr);
	free(this);
}

/**
 * See header
 */
pts_t *pts_create(bool is_imc)
{
	private_pts_t *this;
	pts_pcr_t *pcrs;

	pcrs = pts_pcr_create();
	if (!pcrs)
	{
		DBG1(DBG_PTS, "shadow PCR set could not be created");
		return NULL;
	}

	INIT(this,
		.public = {
			.get_proto_caps = _get_proto_caps,
			.set_proto_caps = _set_proto_caps,
			.get_meas_algorithm = _get_meas_algorithm,
			.set_meas_algorithm = _set_meas_algorithm,
			.get_dh_hash_algorithm = _get_dh_hash_algorithm,
			.set_dh_hash_algorithm = _set_dh_hash_algorithm,
			.create_dh_nonce = _create_dh_nonce,
			.get_my_public_value = _get_my_public_value,
			.set_peer_public_value = _set_peer_public_value,
			.calculate_secret = _calculate_secret,
			.get_platform_id = _get_platform_id,
			.set_platform_id = _set_platform_id,
			.get_tpm_version_info = _get_tpm_version_info,
			.set_tpm_version_info = _set_tpm_version_info,
			.get_aik = _get_aik,
			.set_aik = _set_aik,
			.get_aik_id = _get_aik_id,
			.is_path_valid = _is_path_valid,
			.get_metadata = _get_metadata,
			.read_pcr = _read_pcr,
			.extend_pcr = _extend_pcr,
			.quote = _quote,
			.get_pcrs = _get_pcrs,
			.get_quote = _get_quote,
			.verify_quote_signature  = _verify_quote_signature,
			.destroy = _destroy,
		},
		.is_imc = is_imc,
		.proto_caps = PTS_PROTO_CAPS_V,
		.algorithm = PTS_MEAS_ALGO_SHA256,
		.dh_hash_algorithm = PTS_MEAS_ALGO_SHA256,
		.pcrs = pcrs,
	);

	if (is_imc)
	{
		this->tpm = tpm_tss_probe(TPM_VERSION_ANY);
		if (this->tpm)
		{
			this->proto_caps |= PTS_PROTO_CAPS_T | PTS_PROTO_CAPS_D;
			load_aik(this);
		}
	}
	else
	{
		this->proto_caps |= PTS_PROTO_CAPS_T | PTS_PROTO_CAPS_D;
	}

	return &this->public;
}
