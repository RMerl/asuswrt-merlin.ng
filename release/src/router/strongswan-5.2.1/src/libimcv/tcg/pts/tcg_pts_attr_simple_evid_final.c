/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2011-2014 Andreas Steffen
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
#include "pts/pts_simple_evid_final.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_simple_evid_final_t private_tcg_pts_attr_simple_evid_final_t;

/**
 * Simple Evidence Final
 * see section 3.15.2 of PTS Protocol: Binding to TNC IF-M Specification
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
 *  |              Optional TPM Quote Signature Length              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional TPM Quote Signature (Variable Length)         ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~        Optional Evidence Signature (Variable Length)          ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PTS_SIMPLE_EVID_FINAL_SIZE			2
#define PTS_SIMPLE_EVID_FINAL_RESERVED		0x00
#define PTS_SIMPLE_EVID_FINAL_FLAG_MASK		0xC0
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
	 * Set of flags for Simple Evidence Final
	 */
	u_int8_t flags;

	/**
	 * Optional Composite Hash Algorithm
	 */
	pts_meas_algorithms_t comp_hash_algorithm;

	/**
	 * Optional TPM PCR Composite
	 */
	chunk_t pcr_comp;

	/**
	 * Optional TPM Quote Signature
	 */
	chunk_t tpm_quote_sig;

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
		free(this->value.ptr);
		free(this->pcr_comp.ptr);
		free(this->tpm_quote_sig.ptr);
		free(this->evid_sig.ptr);
		free(this);
	}
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_simple_evid_final_t *this)
{
	bio_writer_t *writer;
	u_int8_t flags;

	if (this->value.ptr)
	{
		return;
	}
	flags = this->flags & PTS_SIMPLE_EVID_FINAL_FLAG_MASK;

	if (this->has_evid_sig)
	{
		flags |= PTS_SIMPLE_EVID_FINAL_EVID_SIG;
	}

	writer = bio_writer_create(PTS_SIMPLE_EVID_FINAL_SIZE);
	writer->write_uint8 (writer, flags);
	writer->write_uint8 (writer, PTS_SIMPLE_EVID_FINAL_RESERVED);

	/** Optional Composite Hash Algorithm field is always present
	 * Field has value of all zeroes if not used.
	 * Implemented adhering the suggestion of Paul Sangster 28.Oct.2011
	 */
	writer->write_uint16(writer, this->comp_hash_algorithm);

	/* Optional fields */
	if (this->flags != PTS_SIMPLE_EVID_FINAL_NO)
	{
		writer->write_uint32 (writer, this->pcr_comp.len);
		writer->write_data (writer, this->pcr_comp);

		writer->write_uint32 (writer, this->tpm_quote_sig.len);
		writer->write_data (writer, this->tpm_quote_sig);
	}

	if (this->has_evid_sig)
	{
		writer->write_data (writer, this->evid_sig);
	}

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_simple_evid_final_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int8_t flags, reserved;
	u_int16_t algorithm;
	u_int32_t pcr_comp_len, tpm_quote_sig_len, evid_sig_len;
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

	this->flags = flags & PTS_SIMPLE_EVID_FINAL_FLAG_MASK;

	this->has_evid_sig = (flags & PTS_SIMPLE_EVID_FINAL_EVID_SIG) != 0;

	/** Optional Composite Hash Algorithm field is always present
	 * Field has value of all zeroes if not used.
	 * Implemented adhering the suggestion of Paul Sangster 28.Oct.2011
	 */

	reader->read_uint16(reader, &algorithm);
	this->comp_hash_algorithm = algorithm;

	/*  Optional Composite Hash Algorithm and TPM PCR Composite fields */
	if (this->flags != PTS_SIMPLE_EVID_FINAL_NO)
	{
		if (!reader->read_uint32(reader, &pcr_comp_len))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "PCR Composite Length");
			goto end;
		}
		if (!reader->read_data(reader, pcr_comp_len, &this->pcr_comp))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "PCR Composite");
			goto end;
		}
		this->pcr_comp = chunk_clone(this->pcr_comp);

		if (!reader->read_uint32(reader, &tpm_quote_sig_len))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "TPM Quote Singature Length");
			goto end;
		}
		if (!reader->read_data(reader, tpm_quote_sig_len, &this->tpm_quote_sig))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Evidence Final "
						  "TPM Quote Singature");
			goto end;
		}
		this->tpm_quote_sig = chunk_clone(this->tpm_quote_sig);
	}

	/*  Optional Evidence Signature field */
	if (this->has_evid_sig)
	{
		evid_sig_len = reader->remaining(reader);
		reader->read_data(reader, evid_sig_len, &this->evid_sig);
		this->evid_sig = chunk_clone(this->evid_sig);
	}

	reader->destroy(reader);
	return SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(tcg_pts_attr_simple_evid_final_t, get_quote_info, u_int8_t,
	private_tcg_pts_attr_simple_evid_final_t *this,
	pts_meas_algorithms_t *comp_hash_algo, chunk_t *pcr_comp, chunk_t *tpm_quote_sig)
{
	if (comp_hash_algo)
	{
		*comp_hash_algo = this->comp_hash_algorithm;
	}
	if (pcr_comp)
	{
		*pcr_comp = this->pcr_comp;
	}
	if (tpm_quote_sig)
	{
		*tpm_quote_sig = this->tpm_quote_sig;
	}
	return this->flags;
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
pa_tnc_attr_t *tcg_pts_attr_simple_evid_final_create(u_int8_t flags,
							pts_meas_algorithms_t comp_hash_algorithm,
							chunk_t pcr_comp, chunk_t tpm_quote_sig)
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
		.flags = flags,
		.comp_hash_algorithm = comp_hash_algorithm,
		.pcr_comp = pcr_comp,
		.tpm_quote_sig = tpm_quote_sig,
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
