/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2011-2016 Andreas Steffen
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

#include "tcg_pts_attr_simple_evid_final.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_simple_evid_final_t private_tcg_pts_attr_simple_evid_final_t;

/**
 * Simple Evidence Final
 * see section 3.15.2 of PTS Protocol: Binding to TNC IF-M Specification
 * plus non-standard extensions to cover the TPM 2.0 Quote Info format
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Flags      |   Reserved    | Optional Composite Hash Alg   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |               Optional TPM PCR Composite Length               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~          Optional TPM PCR Composite (Variable Length)         ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Opt. TPM Qual. Signer Length  | Optional TPM Qualified Signer ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM Qualified Signer (Variable Length)        ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Opt. TPM Clock Info Length    | Optional TPM Clock Info       ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM Clock Info (Variable Length)              ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Opt. TPM Version Info Length  | Optional TPM Version Info     ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM Version Info (Variable Length)            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Opt. TPM PCR Selection Length | Opt. TPM PCR Selection        ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM PCR Selection (Variable Length)           ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |              Optional TPM Quote Signature Length              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM Quote Signature (Variable Length)         ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional Evidence Signature (Variable Length)          ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

#define PTS_SIMPLE_EVID_FINAL_SIZE			2
#define PTS_SIMPLE_EVID_FINAL_RESERVED		0x00

/**
 * PTS Simple Evidence Final Flags
 */
enum pts_simple_evid_final_flag_t {
	/** TPM PCR Composite and TPM Quote Signature not included   */
	PTS_SIMPLE_EVID_FINAL_NO =						0x00,
	/** TPM Quote Info and TPM Quite Signature included
	  * using TPM 2.0 Quote Info format                          */
	PTS_SIMPLE_EVID_FINAL_EVID_QUOTE_INFO_TPM2 =	0x10,
    /** Evidence Signature included                              */
	PTS_SIMPLE_EVID_FINAL_EVID_SIG =				0x20,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO                                     */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO =			 	0x40,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO2, TPM_CAP_VERSION_INFO not appended */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2 =				0x80,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO2, TPM_CAP_VERSION_INFO appended     */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2_CAP_VER =	 	0xC0,
	/** Mask for the TPM Quote Info flags                        */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO_MASK =			0xD0
};

/**
 * Private data of an tcg_pts_attr_simple_evid_final_t object.
 */
struct private_tcg_pts_attr_simple_evid_final_t {

	/**
	 * Public members of tcg_pts_attr_simple_evid_final_t
	 */
	tcg_pts_attr_simple_evid_final_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Attribute value or segment
	 */
	chunk_t value;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * Optional TPM Quote Info
	 */
	tpm_tss_quote_info_t *quote_info;

	/**
	 * Optional TPM Quote Signature
	 */
	chunk_t quote_sig;

	/**
	 * Is Evidence Signature included?
	 */
	bool has_evid_sig;

	/**
	 * Optional Evidence Signature
	 */
	chunk_t evid_sig;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_simple_evid_final_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_simple_evid_final_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->quote_info);
		free(this->value.ptr);
		free(this->quote_sig.ptr);
		free(this->evid_sig.ptr);
		free(this);
	}
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	chunk_t pcr_digest, pcr_select, qualified_signer, clock_info, version_info;
	hash_algorithm_t pcr_digest_alg;
	tpm_quote_mode_t quote_mode;
	bio_writer_t *writer;
	uint8_t flags;

	if (this->value.ptr)
	{
		return;
	}

	quote_mode = this->quote_info->get_quote_mode(this->quote_info);
	switch (quote_mode)
	{
		case TPM_QUOTE:
			flags = PTS_SIMPLE_EVID_FINAL_QUOTE_INFO;
			break;
		case TPM_QUOTE2:
			flags = PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2;
			break;
		case TPM_QUOTE2_VERSION_INFO:
			flags = PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2_CAP_VER;
			break;
		case TPM_QUOTE_TPM2:
			flags = PTS_SIMPLE_EVID_FINAL_EVID_QUOTE_INFO_TPM2;
			break;
		case TPM_QUOTE_NONE:
		default:
			flags = PTS_SIMPLE_EVID_FINAL_NO;
	}

	if (this->has_evid_sig)
	{
		flags |= PTS_SIMPLE_EVID_FINAL_EVID_SIG;
	}

	writer = bio_writer_create(PTS_SIMPLE_EVID_FINAL_SIZE);
	writer->write_uint8 (writer, flags);
	writer->write_uint8 (writer, PTS_SIMPLE_EVID_FINAL_RESERVED);

	pcr_digest_alg = this->quote_info->get_pcr_digest_alg(this->quote_info);
	pcr_digest     = this->quote_info->get_pcr_digest(this->quote_info);

	writer->write_uint16(writer, pts_meas_algo_from_hash(pcr_digest_alg));

	/* Optional fields */
	if (quote_mode != TPM_QUOTE_NONE)
	{
		writer->write_data32(writer, pcr_digest);
	}

	if (quote_mode == TPM_QUOTE_TPM2)
	{
		version_info = this->quote_info->get_version_info(this->quote_info);
		this->quote_info->get_tpm2_info(this->quote_info, &qualified_signer,
										&clock_info, &pcr_select);
		writer->write_data16(writer, qualified_signer);
		writer->write_data16(writer, clock_info);
		writer->write_data16(writer, version_info);
		writer->write_data16(writer, pcr_select);
	}

	if (quote_mode != TPM_QUOTE_NONE)
	{
		writer->write_data32(writer, this->quote_sig);
		if (this->has_evid_sig)
		{
			writer->write_data(writer, this->evid_sig);
		}
	}

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_simple_evid_final_t *this, uint32_t *offset)
{
	hash_algorithm_t pcr_digest_alg;
	tpm_quote_mode_t quote_mode;
	bio_reader_t *reader;
	uint8_t flags, reserved;
	uint16_t algorithm;
	uint32_t evid_sig_len;
	chunk_t pcr_digest = chunk_empty, quote_sig, evid_sig;
	chunk_t qualified_signer, clock_info, version_info, pcr_select;
	status_t status = FAILED;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_SIMPLE_EVID_FINAL_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for Simple Evidence Final");
		return FAILED;
	}
	reader = bio_reader_create(this->value);

	reader->read_uint8(reader, &flags);
	reader->read_uint8(reader, &reserved);

	this->has_evid_sig = (flags & PTS_SIMPLE_EVID_FINAL_EVID_SIG) != 0;

	flags &= PTS_SIMPLE_EVID_FINAL_QUOTE_INFO_MASK;

	switch (flags)
	{
		case PTS_SIMPLE_EVID_FINAL_QUOTE_INFO:
			quote_mode = TPM_QUOTE;
			break;
		case PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2:
			quote_mode = TPM_QUOTE2;
			break;
		case PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2_CAP_VER:
			quote_mode = TPM_QUOTE2_VERSION_INFO;
			break;
		case PTS_SIMPLE_EVID_FINAL_EVID_QUOTE_INFO_TPM2:
			quote_mode = TPM_QUOTE_TPM2;
			break;
		case PTS_SIMPLE_EVID_FINAL_NO:
		default:
			quote_mode = TPM_QUOTE_NONE;
			break;
	}

	/** Optional Composite Hash Algorithm field is always present
	 * Field has value of all zeroes if not used.
	 * Implemented adhering the suggestion of Paul Sangster 28.Oct.2011
	 */
	reader->read_uint16(reader, &algorithm);
	pcr_digest_alg = pts_meas_algo_to_hash(algorithm);

	/*  Optional fields */
	if (quote_mode != TPM_QUOTE_NONE)
	{
		if (!reader->read_data32(reader, &pcr_digest))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "PCR Composite");
			goto end;
		}
	}
	this->quote_info = tpm_tss_quote_info_create(quote_mode, pcr_digest_alg,
															 pcr_digest);

	if (quote_mode == TPM_QUOTE_TPM2)
	{
		if (!reader->read_data16(reader, &qualified_signer))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "Qualified Signer");
			goto end;
		}
		if (!reader->read_data16(reader, &clock_info))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "Clock Info");
			goto end;
		}
		if (!reader->read_data16(reader, &version_info))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "Version Info");
			goto end;
		}
		if (!reader->read_data16(reader, &pcr_select))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "PCR select");
			goto end;
		}
		this->quote_info->set_tpm2_info(this->quote_info, qualified_signer,
										clock_info, pcr_select);
		this->quote_info->set_version_info(this->quote_info, version_info);
	}


	if (quote_mode != TPM_QUOTE_NONE)
	{
		if (!reader->read_data32(reader, &quote_sig))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "TPM Quote Singature");
			goto end;
		}
		this->quote_sig = chunk_clone(quote_sig);
	}

	/*  Optional Evidence Signature field */
	if (this->has_evid_sig)
	{
		evid_sig_len = reader->remaining(reader);
		reader->read_data(reader, evid_sig_len, &evid_sig);
		this->evid_sig = chunk_clone(evid_sig);
	}

	reader->destroy(reader);
	return SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(tcg_pts_attr_simple_evid_final_t, get_quote_info, void,
	private_tcg_pts_attr_simple_evid_final_t *this,
	tpm_tss_quote_info_t **quote_info, chunk_t *quote_sig)
{
	if (quote_info)
	{
		*quote_info = this->quote_info;
	}
	if (quote_sig)
	{
		*quote_sig = this->quote_sig;
	}
}

METHOD(tcg_pts_attr_simple_evid_final_t, get_evid_sig, bool,
	private_tcg_pts_attr_simple_evid_final_t *this, chunk_t *evid_sig)
{
	if (evid_sig)
	{
		*evid_sig = this->evid_sig;
	}
	return this->has_evid_sig;
}

METHOD(tcg_pts_attr_simple_evid_final_t, set_evid_sig, void,
	private_tcg_pts_attr_simple_evid_final_t *this, chunk_t evid_sig)
{
	this->evid_sig = evid_sig;
	this->has_evid_sig = TRUE;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_simple_evid_final_create(
						tpm_tss_quote_info_t *quote_info, chunk_t quote_sig)
{
	private_tcg_pts_attr_simple_evid_final_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_quote_info = _get_quote_info,
			.get_evid_sig = _get_evid_sig,
			.set_evid_sig = _set_evid_sig,
		},
		.type = { PEN_TCG, TCG_PTS_SIMPLE_EVID_FINAL },
		.quote_info = quote_info,
		.quote_sig = quote_sig,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_simple_evid_final_create_from_data(size_t length,
															   chunk_t data)
{
	private_tcg_pts_attr_simple_evid_final_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_quote_info = _get_quote_info,
			.get_evid_sig = _get_evid_sig,
			.set_evid_sig = _set_evid_sig,
		},
		.type = { PEN_TCG, TCG_PTS_SIMPLE_EVID_FINAL },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
