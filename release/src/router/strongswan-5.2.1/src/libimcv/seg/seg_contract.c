/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "seg_contract.h"
#include "seg_env.h"
#include "ietf/ietf_attr_pa_tnc_error.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"

#include <utils/debug.h>
#include <bio/bio_writer.h>

#include <tncif_pa_subtypes.h>

typedef struct private_seg_contract_t private_seg_contract_t;

/**
 * Private data of a seg_contract_t object.
 */
struct private_seg_contract_t {

	/**
	 * Public seg_contract_t interface.
	 */
	seg_contract_t public;

	/**
	 * PA-TNC message type
	 */
	pen_type_t msg_type;

	/**
	 * Maximum PA-TNC attribute size
	 */
	uint32_t max_attr_size;

	/**
	 * Maximum PA-TNC attribute segment size
	 */
	uint32_t max_seg_size;

	/**
	 * Maximum PA-TNC attribute segment size
	 */
	uint32_t last_base_attr_id;

	/**
	 * List of attribute segment envelopes
	 */

	linked_list_t *seg_envs;

	/**
	 * Is this a null contract?
	 */
	bool is_null;

	/**
	 * Contract role
	 */
	bool is_issuer;

	/**
	 * Issuer ID (either IMV or IMC ID)
	 */
	TNC_UInt32 issuer_id;

	/**
	 * Responder ID (either IMC or IMV ID)
	 */
	TNC_UInt32 responder_id;

	/**
	 * IMC/IMV role
	 */
	bool is_imc;

};

METHOD(seg_contract_t, get_msg_type, pen_type_t,
	private_seg_contract_t *this)
{
	return this->msg_type;
}

METHOD(seg_contract_t, set_max_size, void,
	private_seg_contract_t *this, uint32_t max_attr_size, uint32_t max_seg_size)
{
	this->max_attr_size = max_attr_size;
	this->max_seg_size = max_seg_size;
	this->is_null = max_attr_size == SEG_CONTRACT_MAX_SIZE_VALUE &&
					max_seg_size  == SEG_CONTRACT_MAX_SIZE_VALUE;
}

METHOD(seg_contract_t, get_max_size, void,
	private_seg_contract_t *this, uint32_t *max_attr_size, uint32_t *max_seg_size)
{
	if (max_attr_size)
	{
		*max_attr_size = this->max_attr_size;
	}
	if (max_seg_size)
	{
		*max_seg_size = this->max_seg_size;
	}
}

METHOD(seg_contract_t, check_size, bool,
	private_seg_contract_t *this, pa_tnc_attr_t *attr, bool *oversize)
{
	chunk_t attr_value;
	size_t attr_len;

	*oversize = FALSE;

	if (this->is_null)
	{
		/* null segmentation contract */
		return FALSE;
	}
	attr->build(attr);
	attr_value = attr->get_value(attr);
	attr_len = PA_TNC_ATTR_HEADER_SIZE + attr_value.len;

	if (attr_len > this->max_attr_size)
	{
		/* oversize attribute */
		*oversize = TRUE;
		return FALSE;
	}
	if (this->max_seg_size == SEG_CONTRACT_NO_FRAGMENTATION)
	{
		/* no fragmentation wanted */
		return FALSE;
	}
	return attr_value.len > this->max_seg_size + TCG_SEG_ATTR_SEG_ENV_HEADER;
}

METHOD(seg_contract_t, first_segment, pa_tnc_attr_t*,
	private_seg_contract_t *this, pa_tnc_attr_t *attr)
{
	seg_env_t *seg_env;

	seg_env = seg_env_create(++this->last_base_attr_id, attr,
							 this->max_seg_size);
	if (!seg_env)
	{
		return NULL;
	}
	this->seg_envs->insert_last(this->seg_envs, seg_env);

	return seg_env->first_segment(seg_env);
}

METHOD(seg_contract_t, next_segment, pa_tnc_attr_t*,
	private_seg_contract_t *this, uint32_t base_attr_id)
{
	pa_tnc_attr_t *seg_env_attr = NULL;
	seg_env_t *seg_env;
	bool last_segment = FALSE;
	enumerator_t *enumerator;

	enumerator = this->seg_envs->create_enumerator(this->seg_envs);
	while (enumerator->enumerate(enumerator, &seg_env))
	{
		if (seg_env->get_base_attr_id(seg_env) == base_attr_id)
		{
			seg_env_attr = seg_env->next_segment(seg_env, &last_segment);
			if (!seg_env_attr)
			{
				break;
			}
			if (last_segment)
			{
				this->seg_envs->remove_at(this->seg_envs, enumerator);
				seg_env->destroy(seg_env);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	return seg_env_attr;
}

METHOD(seg_contract_t, add_segment, pa_tnc_attr_t*,
	private_seg_contract_t *this, pa_tnc_attr_t *attr, pa_tnc_attr_t **error,
	bool *more)
{
	tcg_seg_attr_seg_env_t *seg_env_attr;
	seg_env_t *current, *seg_env = NULL;
	pa_tnc_attr_t *base_attr;
	pen_type_t error_code;
	uint32_t base_attr_id;
	uint8_t flags;
	chunk_t segment_data, msg_info;
	enumerator_t *enumerator;

	seg_env_attr = (tcg_seg_attr_seg_env_t*)attr;
	base_attr_id = seg_env_attr->get_base_attr_id(seg_env_attr);
	segment_data = seg_env_attr->get_segment(seg_env_attr, &flags);
	*more = flags & SEG_ENV_FLAG_MORE;
	*error = NULL;

	enumerator = this->seg_envs->create_enumerator(this->seg_envs);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->get_base_attr_id(current) == base_attr_id)
		{
			seg_env = current;
			this->seg_envs->remove_at(this->seg_envs, enumerator);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (flags & SEG_ENV_FLAG_START)
	{
		if (seg_env)
		{
			DBG1(DBG_TNC, "base attribute ID %d is already in use",
						   base_attr_id);
			this->seg_envs->insert_last(this->seg_envs, seg_env);
			return NULL;
		}
		DBG2(DBG_TNC, "received first segment for base attribute ID %d "
					  "(%d bytes)", base_attr_id, segment_data.len);
		seg_env = seg_env_create_from_data(base_attr_id, segment_data,
										   this->max_seg_size, error);
		if (!seg_env)
		{
			return NULL;
		}
	}
	else
	{
		if (!seg_env)
		{
			DBG1(DBG_TNC, "base attribute ID %d not found", base_attr_id);
			return NULL;
		}
		DBG2(DBG_TNC, "received %s segment for base attribute ID %d "
					  "(%d bytes)", (*more) ? "next" : "last", base_attr_id,
					   segment_data.len);
		if (!seg_env->add_segment(seg_env, segment_data, error))
		{
			seg_env->destroy(seg_env);
			return NULL;
		}
	}
	base_attr = seg_env->get_base_attr(seg_env);

	if (*more)
	{
		/* reinsert into list since more segments are to come */
		this->seg_envs->insert_last(this->seg_envs, seg_env);
	}
	else
	{
		/* added the last segment */
		if (!base_attr)
		{
			/* base attribute waits for more data */
			DBG1(DBG_TNC, "insufficient bytes for PA-TNC attribute value");
			msg_info = seg_env->get_base_attr_info(seg_env);
			error_code = pen_type_create(PEN_IETF, PA_ERROR_INVALID_PARAMETER);
			*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
										msg_info, PA_TNC_ATTR_INFO_SIZE);
		}
		seg_env->destroy(seg_env);
	}
	return base_attr;
}

METHOD(seg_contract_t, is_issuer, bool,
	private_seg_contract_t *this)
{
	return this->is_issuer;
}

METHOD(seg_contract_t, is_null, bool,
	private_seg_contract_t *this)
{
	return this->is_null;
}

METHOD(seg_contract_t, set_responder, void,
	private_seg_contract_t *this, TNC_UInt32 responder_id)
{
	this->responder_id = responder_id;
}

METHOD(seg_contract_t, get_responder, TNC_UInt32,
	private_seg_contract_t *this)
{
	return this->responder_id;
}

METHOD(seg_contract_t, get_issuer, TNC_UInt32,
	private_seg_contract_t *this)
{
	return this->issuer_id;
}

METHOD(seg_contract_t, clone_, seg_contract_t*,
	private_seg_contract_t *this)
{
	private_seg_contract_t *clone;

	clone = malloc_thing(private_seg_contract_t);
	memcpy(clone, this, sizeof(private_seg_contract_t));
	clone->seg_envs = linked_list_create();

	return &clone->public;
}

METHOD(seg_contract_t, get_info_string, void,
	private_seg_contract_t *this, char *buf, size_t len, bool request)
{
	enum_name_t *pa_subtype_names;
	uint32_t msg_vid, msg_subtype;
	char *pos = buf;
	int written;

	/* nul-terminate the string buffer */
	buf[--len] = '\0';

	if (this->is_issuer && request)
	{
		written = snprintf(pos, len, "%s %d requests",
						  this->is_imc ? "IMC" : "IMV", this->issuer_id);
	}
	else
	{
		written = snprintf(pos, len, "%s %d received",
						   this->is_imc ? "IMC" : "IMV",
						   this->is_issuer ? this->issuer_id :
											 this->responder_id);
	}
	if (written < 0 || written > len)
	{
		return;
	}
	pos += written;
	len -= written;

	written = snprintf(pos, len, " a %ssegmentation contract%s ",
					   this->is_null ? "null" : "", request ?
					  (this->is_issuer ? "" : " request") : " response");
	if (written < 0 || written > len)
	{
		return;
	}
	pos += written;
	len -= written;

	if ((!this->is_issuer && this->issuer_id != TNC_IMVID_ANY) ||
		( this->is_issuer && this->responder_id != TNC_IMVID_ANY))
	{
		written = snprintf(pos, len, "from %s %d ",
						   this->is_imc ? "IMV" : "IMC",
						   this->is_issuer ? this->responder_id :
											 this->issuer_id);
		if (written < 0 || written > len)
		{
			return;
		}
		pos += written;
		len -= written;
	}

	msg_vid     = this->msg_type.vendor_id;
	msg_subtype = this->msg_type.type;
	pa_subtype_names = get_pa_subtype_names(msg_vid);
	if (pa_subtype_names)
	{
		written = snprintf(pos, len, "for PA message type '%N/%N' "
						   "0x%06x/0x%08x", pen_names, msg_vid,
						   pa_subtype_names, msg_subtype, msg_vid,
						   msg_subtype);
	}
	else
	{
		written = snprintf(pos, len, "for PA message type '%N' "
						   "0x%06x/0x%08x", pen_names, msg_vid,
						   msg_vid, msg_subtype);
	}
	if (written < 0 || written > len)
	{
		return;
	}
	pos += written;
	len -= written;

	if (!this->is_null)
	{
		written = snprintf(pos, len, "\n  maximum attribute size of %u bytes "
						   "with ", this->max_attr_size);
		if (written < 0 || written > len)
		{
			return;
		}
		pos += written;
		len -= written;

		if (this->max_seg_size == SEG_CONTRACT_MAX_SIZE_VALUE)
		{
			written = snprintf(pos, len, "no segmentation");
		}
		else
		{
			written = snprintf(pos, len, "maximum segment size of %u bytes",
							   this->max_seg_size);
		}
	}
}

METHOD(seg_contract_t, destroy, void,
	private_seg_contract_t *this)
{
	this->seg_envs->destroy_offset(this->seg_envs, offsetof(seg_env_t, destroy));
	free(this);
}

/**
 * See header
 */
seg_contract_t *seg_contract_create(pen_type_t msg_type,
								    uint32_t max_attr_size,
									uint32_t max_seg_size,
									bool is_issuer, TNC_UInt32 issuer_id,
									bool is_imc)
{
	private_seg_contract_t *this;

	INIT(this,
		.public = {
			.get_msg_type = _get_msg_type,
			.set_max_size = _set_max_size,
			.get_max_size = _get_max_size,
			.check_size = _check_size,
			.first_segment = _first_segment,
			.next_segment = _next_segment,
			.add_segment = _add_segment,
			.is_issuer = _is_issuer,
			.is_null = _is_null,
			.set_responder = _set_responder,
			.get_responder = _get_responder,
			.get_issuer = _get_issuer,
			.clone = _clone_,
			.get_info_string = _get_info_string,
			.destroy = _destroy,
		},
		.msg_type = msg_type,
		.max_attr_size = max_attr_size,
		.max_seg_size = max_seg_size,
		.seg_envs = linked_list_create(),
		.is_issuer = is_issuer,
		.issuer_id = issuer_id,
		.responder_id = is_imc ? TNC_IMVID_ANY : TNC_IMCID_ANY,
		.is_imc = is_imc,
		.is_null = max_attr_size == SEG_CONTRACT_MAX_SIZE_VALUE &&
				   max_seg_size  == SEG_CONTRACT_MAX_SIZE_VALUE,
	);

	return &this->public;
}

