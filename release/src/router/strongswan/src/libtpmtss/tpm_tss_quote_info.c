/*
 * Copyright (C) 2016 Andreas Steffen
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

#include <tpm_tss_quote_info.h>

#include <bio/bio_writer.h>

#ifndef TPM_TAG_QUOTE_INFO2
#define TPM_TAG_QUOTE_INFO2 0x0036
#endif
#ifndef TPM_LOC_ZERO
#define TPM_LOC_ZERO 0x01
#endif

typedef struct private_tpm_tss_quote_info_t private_tpm_tss_quote_info_t;

/**
 * Private data of an tpm_tss_quote_info_t object.
 */
struct private_tpm_tss_quote_info_t {

	/**
	 * Public tpm_tss_quote_info_t interface.
	 */
	tpm_tss_quote_info_t public;

	/**
	 * TPM Quote Mode
	 */
	tpm_quote_mode_t quote_mode;

	/**
	 * TPM Qualified Signer
	 */
	chunk_t qualified_signer;

	/**
	 * TPM Clock Info
	 */
	chunk_t clock_info;

	/**
	 * TPM Version Info
	 */
	chunk_t	version_info;

	/**
	 * TPM PCR Selection
	 */
	chunk_t pcr_select;

	/**
	 * TPM PCR Composite Hash
	 */
	chunk_t pcr_digest;

	/**
	 * TPM PCR Composite Hash algorithm
	 */
	hash_algorithm_t pcr_digest_alg;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(tpm_tss_quote_info_t, get_quote_mode, tpm_quote_mode_t,
	private_tpm_tss_quote_info_t *this)
{
	return this->quote_mode;
}

METHOD(tpm_tss_quote_info_t, get_pcr_digest_alg, hash_algorithm_t,
	private_tpm_tss_quote_info_t *this)
{
	return this->pcr_digest_alg;
}

METHOD(tpm_tss_quote_info_t, get_pcr_digest, chunk_t,
	private_tpm_tss_quote_info_t *this)
{
	return this->pcr_digest;
}

METHOD(tpm_tss_quote_info_t, get_quote, bool,
	private_tpm_tss_quote_info_t *this, chunk_t nonce,
	tpm_tss_pcr_composite_t *composite, chunk_t *quoted)
{
	chunk_t pcr_composite, pcr_digest;
	bio_writer_t *writer;
	hasher_t *hasher;
	bool equal_digests;

	/* Construct PCR Composite */
	writer = bio_writer_create(32);

	switch (this->quote_mode)
	{
		case TPM_QUOTE:
		case TPM_QUOTE2:
		case TPM_QUOTE2_VERSION_INFO:
			writer->write_data16(writer, composite->pcr_select);
			writer->write_data32(writer, composite->pcr_composite);

			break;
		case TPM_QUOTE_TPM2:
			writer->write_data(writer, composite->pcr_composite);
			break;
		case TPM_QUOTE_NONE:
			break;
	}

	pcr_composite = writer->extract_buf(writer);
	writer->destroy(writer);

	DBG2(DBG_PTS, "constructed PCR Composite: %B", &pcr_composite);

	/* Compute PCR Composite Hash */
	hasher = lib->crypto->create_hasher(lib->crypto, this->pcr_digest_alg);
	if (!hasher || !hasher->allocate_hash(hasher, pcr_composite, &pcr_digest))
	{
		DESTROY_IF(hasher);
		chunk_free(&pcr_composite);
		return FALSE;
	}
	hasher->destroy(hasher);
	chunk_free(&pcr_composite);

	DBG2(DBG_PTS, "constructed PCR Composite digest: %B", &pcr_digest);

	equal_digests = chunk_equals(pcr_digest, this->pcr_digest);

	/* Construct Quote Info */
	writer = bio_writer_create(32);

	switch (this->quote_mode)
	{
		case TPM_QUOTE:
			/* Version number */
			writer->write_data(writer, chunk_from_chars(1, 1, 0, 0));

			/* Magic QUOT value */
			writer->write_data(writer, chunk_from_str("QUOT"));

			/* PCR Composite Hash */
			writer->write_data(writer, pcr_digest);

			/* Secret assessment value 20 bytes (nonce) */
			writer->write_data(writer, nonce);
			break;
		case TPM_QUOTE2:
		case TPM_QUOTE2_VERSION_INFO:
			/* TPM Structure Tag */
			writer->write_uint16(writer, TPM_TAG_QUOTE_INFO2);

			/* Magic QUT2 value */
			writer->write_data(writer, chunk_from_str("QUT2"));

			/* Secret assessment value 20 bytes (nonce) */
			writer->write_data(writer, nonce);

			/* PCR selection */
			writer->write_data16(writer, composite->pcr_select);

			/* TPM Locality Selection */
			writer->write_uint8(writer, TPM_LOC_ZERO);

			/* PCR Composite Hash */
			writer->write_data(writer, pcr_digest);

			if (this->quote_mode == TPM_QUOTE2_VERSION_INFO)
			{
				/* TPM version Info */
				writer->write_data(writer, this->version_info);
			}
			break;
		case TPM_QUOTE_TPM2:
			/* Magic */
			writer->write_data(writer, chunk_from_chars(0xff,0x54,0x43,0x47));

			/* Type */
			writer->write_uint16(writer, 0x8018);

			/* Qualified Signer */
			writer->write_data16(writer, this->qualified_signer);

			/* Extra Data */
			writer->write_data16(writer, nonce);

			/* Clock Info */
			writer->write_data(writer, this->clock_info);

			/* Firmware Version */
			writer->write_data(writer, this->version_info);

			/* PCR Selection */
			writer->write_data(writer, this->pcr_select);

			/* PCR Composite Hash */
			writer->write_data16(writer, pcr_digest);
			break;
		case TPM_QUOTE_NONE:
			break;
	}
	chunk_free(&pcr_digest);
	*quoted = writer->extract_buf(writer);
	writer->destroy(writer);

	DBG2(DBG_PTS, "constructed TPM Quote Info: %B", quoted);

	if (!equal_digests)
	{
		DBG1(DBG_IMV, "received PCR Composite digest does not match "
					  "constructed one");
		chunk_free(quoted);
	}
	return equal_digests;
}

METHOD(tpm_tss_quote_info_t, set_version_info, void,
	private_tpm_tss_quote_info_t *this, chunk_t version_info)
{
	chunk_free(&this->version_info);
	this->version_info = chunk_clone(version_info);
}

METHOD(tpm_tss_quote_info_t, get_version_info, chunk_t,
	private_tpm_tss_quote_info_t *this)
{
	return this->version_info;
}

METHOD(tpm_tss_quote_info_t, set_tpm2_info, void,
	private_tpm_tss_quote_info_t *this, chunk_t qualified_signer,
	chunk_t clock_info, chunk_t pcr_select)
{
	chunk_free(&this->qualified_signer);
	this->qualified_signer = chunk_clone(qualified_signer);

	chunk_free(&this->clock_info);
	this->clock_info = chunk_clone(clock_info);

	chunk_free(&this->pcr_select);
	this->pcr_select = chunk_clone(pcr_select);
}

METHOD(tpm_tss_quote_info_t, get_tpm2_info, void,
	private_tpm_tss_quote_info_t *this, chunk_t *qualified_signer,
	chunk_t *clock_info, chunk_t *pcr_select)
{
	if (qualified_signer)
	{
		*qualified_signer = this->qualified_signer;
	}
	if (clock_info)
	{
		*clock_info = this->clock_info;
	}
	if (pcr_select)
	{
		*pcr_select = this->pcr_select;
	}
}

METHOD(tpm_tss_quote_info_t, get_ref, tpm_tss_quote_info_t*,
	private_tpm_tss_quote_info_t *this)
{
	ref_get(&this->ref);

	return &this->public;
}

METHOD(tpm_tss_quote_info_t, destroy, void,
	private_tpm_tss_quote_info_t *this)
{
	if (ref_put(&this->ref))
	{
		chunk_free(&this->qualified_signer);
		chunk_free(&this->clock_info);
		chunk_free(&this->version_info);
		chunk_free(&this->pcr_select);
		chunk_free(&this->pcr_digest);
		free(this);
	}
}

/**
 * See header
 */
tpm_tss_quote_info_t *tpm_tss_quote_info_create(tpm_quote_mode_t quote_mode,
						hash_algorithm_t pcr_digest_alg, chunk_t pcr_digest)

{
	private_tpm_tss_quote_info_t *this;

	INIT(this,
		.public = {
			.get_quote_mode = _get_quote_mode,
			.get_pcr_digest_alg = _get_pcr_digest_alg,
			.get_pcr_digest = _get_pcr_digest,
			.get_quote = _get_quote,
			.set_version_info = _set_version_info,
			.get_version_info = _get_version_info,
			.set_tpm2_info = _set_tpm2_info,
			.get_tpm2_info = _get_tpm2_info,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.quote_mode = quote_mode,
		.pcr_digest_alg = pcr_digest_alg,
		.pcr_digest = chunk_clone(pcr_digest),
		.ref = 1,
	);

	return &this->public;
}
