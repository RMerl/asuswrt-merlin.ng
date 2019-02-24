/*
 * Copyright (C) 2011-2015 Andreas Steffen
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

#include "imcv.h"
#include "imv_agent.h"
#include "imv_session.h"

#include "ietf/ietf_attr_assess_result.h"

#include <tncif_names.h>
#include <tncif_identity.h>

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <bio/bio_reader.h>
#include <threading/rwlock.h>

typedef struct private_imv_agent_t private_imv_agent_t;

/**
 * Private data of an imv_agent_t object.
 */
struct private_imv_agent_t {

	/**
	 * Public members of imv_agent_t
	 */
	imv_agent_t public;

	/**
	 * name of IMV
	 */
	const char *name;

	/**
	 * message types registered by IMV
	 */
	pen_type_t *supported_types;

	/**
	 * number of message types registered by IMV
	 */
	uint32_t type_count;

	/**
	 * ID of IMV as assigned by TNCS
	 */
	TNC_IMVID id;

	/**
	 * List of additional IMV IDs assigned by TNCS
	 */
	linked_list_t *additional_ids;

	/**
	 * list of non-fatal unsupported PA-TNC attribute types
	 */
	linked_list_t *non_fatal_attr_types;

	/**
	 * list of TNCS connection entries
	 */
	linked_list_t *connections;

	/**
	 * rwlock to lock TNCS connection entries
	 */
	rwlock_t *connection_lock;

	/**
	 * Inform a TNCS about the set of message types the IMV is able to receive
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param supported_types	list of supported message types
	 * @param type_count		number of list elements
	 * @return					TNC result code
	 */
	TNC_Result (*report_message_types)(TNC_IMVID imv_id,
									   TNC_MessageTypeList supported_types,
									   TNC_UInt32 type_count);

	/**
	 * Inform a TNCS about the set of message types the IMV is able to receive
	 *
	 * @param imv_id				IMV ID assigned by TNCS
	 * @param supported_vids		list of supported message vendor IDs
	 * @param supported_subtypes	list of supported message subtypes
	 * @param type_count			number of list elements
	 * @return						TNC result code
	 */
	TNC_Result (*report_message_types_long)(TNC_IMVID imv_id,
									TNC_VendorIDList supported_vids,
									TNC_MessageSubtypeList supported_subtypes,
									TNC_UInt32 type_count);

	/**
	 * Deliver IMV Action Recommendation and IMV Evaluation Results to the TNCS
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 # @param connection_id		network connection ID assigned by TNCS
	 * @param rec				IMV action recommendation
	 * @param eval				IMV evaluation result
	 * @return					TNC result code
	 */
	TNC_Result (*provide_recommendation)(TNC_IMVID imv_id,
										 TNC_ConnectionID connection_id,
										 TNC_IMV_Action_Recommendation rec,
										 TNC_IMV_Evaluation_Result eval);

	/**
	 * Get the value of an attribute associated with a connection
	 * or with the TNCS as a whole.
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param attribute_id		attribute ID
	 * @param buffer_len		length of buffer in bytes
	 * @param buffer			buffer
	 * @param out_value_len		size in bytes of attribute stored in buffer
	 * @return					TNC result code
	 */
	TNC_Result (*get_attribute)(TNC_IMVID imv_id,
								TNC_ConnectionID connection_id,
								TNC_AttributeID attribute_id,
								TNC_UInt32 buffer_len,
								TNC_BufferReference buffer,
								TNC_UInt32 *out_value_len);

	/**
	 * Set the value of an attribute associated with a connection
	 * or with the TNCS as a whole.
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param attribute_id		attribute ID
	 * @param buffer_len		length of buffer in bytes
	 * @param buffer			buffer
	 * @return					TNC result code
	 */
	TNC_Result (*set_attribute)(TNC_IMVID imv_id,
								TNC_ConnectionID connection_id,
								TNC_AttributeID attribute_id,
								TNC_UInt32 buffer_len,
								TNC_BufferReference buffer);

	/**
	 * Reserve an additional IMV ID
	 *
	 * @param imv_id			primary IMV ID assigned by TNCS
	 * @param out_imv_id		additional IMV ID assigned by TNCS
	 * @return					TNC result code
	 */
	TNC_Result (*reserve_additional_id)(TNC_IMVID imv_id,
										TNC_UInt32 *out_imv_id);

};

METHOD(imv_agent_t, bind_functions, TNC_Result,
	private_imv_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	if (!bind_function)
	{
		DBG1(DBG_IMV, "TNC server failed to provide bind function");
		return TNC_RESULT_INVALID_PARAMETER;
	}
	if (bind_function(this->id, "TNC_TNCS_ReportMessageTypes",
			(void**)&this->report_message_types) != TNC_RESULT_SUCCESS)
	{
		this->report_message_types = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_ReportMessageTypesLong",
			(void**)&this->report_message_types_long) != TNC_RESULT_SUCCESS)
	{
		this->report_message_types_long = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_RequestHandshakeRetry",
			(void**)&this->public.request_handshake_retry) != TNC_RESULT_SUCCESS)
	{
		this->public.request_handshake_retry = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_SendMessage",
			(void**)&this->public.send_message) != TNC_RESULT_SUCCESS)
	{
		this->public.send_message = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_SendMessageLong",
			(void**)&this->public.send_message_long) != TNC_RESULT_SUCCESS)
	{
		this->public.send_message_long = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_ProvideRecommendation",
			(void**)&this->provide_recommendation) != TNC_RESULT_SUCCESS)
	{
		this->provide_recommendation = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_GetAttribute",
			(void**)&this->get_attribute) != TNC_RESULT_SUCCESS)
	{
		this->get_attribute = NULL;
	}
	if (bind_function(this->id, "TNC_TNCS_SetAttribute",
			(void**)&this->set_attribute) != TNC_RESULT_SUCCESS)
	{
		this->set_attribute = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_ReserveAdditionalIMVID",
			(void**)&this->reserve_additional_id) != TNC_RESULT_SUCCESS)
	{
		this->reserve_additional_id = NULL;
	}

	if (this->report_message_types_long)
	{
		TNC_VendorIDList vendor_id_list;
		TNC_MessageSubtypeList subtype_list;
		int i;

		vendor_id_list = malloc(this->type_count * sizeof(TNC_UInt32));
		subtype_list   = malloc(this->type_count * sizeof(TNC_UInt32));

		for (i = 0; i < this->type_count; i++)
		{
			vendor_id_list[i] = this->supported_types[i].vendor_id;
			subtype_list[i]   = this->supported_types[i].type;
		}
		this->report_message_types_long(this->id, vendor_id_list, subtype_list,
										this->type_count);
		free(vendor_id_list);
		free(subtype_list);
	}
	else if (this->report_message_types)
	{
		TNC_MessageTypeList type_list;
		int i;

		type_list = malloc(this->type_count * sizeof(TNC_UInt32));

		for (i = 0; i < this->type_count; i++)
		{
			type_list[i] = (this->supported_types[i].vendor_id << 8) |
						   (this->supported_types[i].type & 0xff);
		}
		this->report_message_types(this->id, type_list, this->type_count);
		free(type_list);
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * finds a connection state based on its Connection ID
 */
static imv_state_t* find_connection(private_imv_agent_t *this,
									 TNC_ConnectionID id)
{
	enumerator_t *enumerator;
	imv_state_t *state, *found = NULL;

	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &state))
	{
		if (id == state->get_connection_id(state))
		{
			found = state;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	return found;
}

/**
 * delete a connection state with a given Connection ID
 */
static bool delete_connection(private_imv_agent_t *this, TNC_ConnectionID id)
{
	enumerator_t *enumerator;
	imv_state_t *state;
	imv_session_t *session;
	bool found = FALSE;

	this->connection_lock->write_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &state))
	{
		if (id == state->get_connection_id(state))
		{
			found = TRUE;
			session = state->get_session(state);
			imcv_sessions->remove_session(imcv_sessions, session);
			state->destroy(state);
			this->connections->remove_at(this->connections, enumerator);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	return found;
}

/**
 * Read a boolean attribute
 */
static bool get_bool_attribute(private_imv_agent_t *this, TNC_ConnectionID id,
							   TNC_AttributeID attribute_id)
{
	TNC_UInt32 len;
	char buf[4];

	return this->get_attribute  &&
		   this->get_attribute(this->id, id, attribute_id, 4, buf, &len) ==
							   TNC_RESULT_SUCCESS && len == 1 && *buf == 0x01;
 }

/**
 * Read a string attribute
 */
static char* get_str_attribute(private_imv_agent_t *this, TNC_ConnectionID id,
								TNC_AttributeID attribute_id)
{
	TNC_UInt32 len;
	char buf[BUF_LEN];

	if (this->get_attribute  &&
		this->get_attribute(this->id, id, attribute_id, BUF_LEN, buf, &len) ==
							TNC_RESULT_SUCCESS && len <= BUF_LEN)
	{
		return strdup(buf);
	}
	return NULL;
 }

/**
 * Read an UInt32 attribute
 */
static uint32_t get_uint_attribute(private_imv_agent_t *this, TNC_ConnectionID id,
									TNC_AttributeID attribute_id)
{
	TNC_UInt32 len;
	char buf[4];

	if (this->get_attribute  &&
		this->get_attribute(this->id, id, attribute_id, 4, buf, &len) ==
							TNC_RESULT_SUCCESS && len == 4)
	{
		return untoh32(buf);
	}
	return 0;
 }

/**
 * Read a TNC identity attribute
 */
static linked_list_t* get_identity_attribute(private_imv_agent_t *this,
											 TNC_ConnectionID id,
											 TNC_AttributeID attribute_id)
{
	TNC_UInt32 len;
	char buf[2048];
	uint32_t count;
	tncif_identity_t *tnc_id;
	bio_reader_t *reader;
	linked_list_t *list;

	list = linked_list_create();

	if (!this->get_attribute ||
		 this->get_attribute(this->id, id, attribute_id, sizeof(buf), buf, &len)
				!= TNC_RESULT_SUCCESS || len > sizeof(buf))
	{
		return list;
	}

	reader = bio_reader_create(chunk_create(buf, len));
	if (!reader->read_uint32(reader, &count))
	{
			goto end;
	}
	while (count--)
	{
		tnc_id = tncif_identity_create_empty();
		if (!tnc_id->process(tnc_id, reader))
		{
			tnc_id->destroy(tnc_id);
			goto end;
		}
		list->insert_last(list, tnc_id);
	}

end:
	reader->destroy(reader);
	return list;
 }

METHOD(imv_agent_t, create_state, TNC_Result,
	private_imv_agent_t *this, imv_state_t *state)
{
	TNC_ConnectionID conn_id;
	char *tnccs_p = NULL, *tnccs_v = NULL, *t_p = NULL, *t_v = NULL;
	bool has_long = FALSE, has_excl = FALSE, has_soh = FALSE;
	linked_list_t *ar_identities;
	imv_session_t *session;
	uint32_t max_msg_len;

	conn_id = state->get_connection_id(state);
	if (find_connection(this, conn_id))
	{
		DBG1(DBG_IMV, "IMV %u \"%s\" already created a state for Connection ID %u",
					   this->id, this->name, conn_id);
		state->destroy(state);
		return TNC_RESULT_OTHER;
	}

	/* Get and display attributes from TNCS via IF-IMV */
	has_long = get_bool_attribute(this, conn_id,
									TNC_ATTRIBUTEID_HAS_LONG_TYPES);
	has_excl = get_bool_attribute(this, conn_id,
									TNC_ATTRIBUTEID_HAS_EXCLUSIVE);
	has_soh  = get_bool_attribute(this, conn_id,
									TNC_ATTRIBUTEID_HAS_SOH);
	tnccs_p = get_str_attribute(this, conn_id,
									TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL);
	tnccs_v = get_str_attribute(this, conn_id,
									TNC_ATTRIBUTEID_IFTNCCS_VERSION);
	t_p = get_str_attribute(this, conn_id,
									TNC_ATTRIBUTEID_IFT_PROTOCOL);
	t_v = get_str_attribute(this, conn_id,
									TNC_ATTRIBUTEID_IFT_VERSION);
	max_msg_len = get_uint_attribute(this, conn_id,
									TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE);
	ar_identities = get_identity_attribute(this, conn_id,
									TNC_ATTRIBUTEID_AR_IDENTITIES);

	state->set_flags(state, has_long, has_excl);
	state->set_max_msg_len(state, max_msg_len);

	DBG2(DBG_IMV, "IMV %u \"%s\" created a state for %s %s Connection ID %u: "
				  "%slong %sexcl %ssoh", this->id, this->name,
				  tnccs_p ? tnccs_p:"?", tnccs_v ? tnccs_v:"?", conn_id,
			      has_long ? "+":"-", has_excl ? "+":"-", has_soh ? "+":"-");
	DBG2(DBG_IMV, "  over %s %s with maximum PA-TNC message size of %u bytes",
				  t_p ? t_p:"?", t_v ? t_v :"?", max_msg_len);

	session = imcv_sessions->add_session(imcv_sessions, conn_id, ar_identities);
	state->set_session(state, session);

	free(tnccs_p);
	free(tnccs_v);
	free(t_p);
	free(t_v);

	/* insert state in connection list */
	this->connection_lock->write_lock(this->connection_lock);
	this->connections->insert_last(this->connections, state);
	this->connection_lock->unlock(this->connection_lock);

	return TNC_RESULT_SUCCESS;
}

METHOD(imv_agent_t, delete_state, TNC_Result,
	private_imv_agent_t *this, TNC_ConnectionID connection_id)
{
	if (!delete_connection(this, connection_id))
	{
		DBG1(DBG_IMV, "IMV %u \"%s\" has no state for Connection ID %u",
					  this->id, this->name, connection_id);
		return TNC_RESULT_FATAL;
	}
	DBG2(DBG_IMV, "IMV %u \"%s\" deleted the state of Connection ID %u",
				  this->id, this->name, connection_id);
	return TNC_RESULT_SUCCESS;
}

METHOD(imv_agent_t, change_state, TNC_Result,
	private_imv_agent_t *this, TNC_ConnectionID connection_id,
							   TNC_ConnectionState new_state,
							   imv_state_t **state_p)
{
	imv_state_t *state;
	TNC_ConnectionState old_state;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_HANDSHAKE:
		case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			state = find_connection(this, connection_id);
			if (!state)
			{
				DBG1(DBG_IMV, "IMV %u \"%s\" has no state for Connection ID %u",
							  this->id, this->name, connection_id);
				return TNC_RESULT_FATAL;
			}
			old_state = state->change_state(state, new_state);
			DBG2(DBG_IMV, "IMV %u \"%s\" changed state of Connection ID %u to '%N'",
						  this->id, this->name, connection_id,
						  TNC_Connection_State_names, new_state);
			if (state_p)
			{
				*state_p = state;
			}
			if (new_state == TNC_CONNECTION_STATE_HANDSHAKE &&
				old_state != TNC_CONNECTION_STATE_CREATE)
			{
				state->reset(state);
				DBG2(DBG_IMV, "IMV %u \"%s\" reset state of Connection ID %u",
							   this->id, this->name, connection_id);
			}
			break;
		case TNC_CONNECTION_STATE_CREATE:
			DBG1(DBG_IMV, "state '%N' should be handled by create_state()",
						  TNC_Connection_State_names, new_state);
				return TNC_RESULT_FATAL;
		case TNC_CONNECTION_STATE_DELETE:
			DBG1(DBG_IMV, "state '%N' should be handled by delete_state()",
						  TNC_Connection_State_names, new_state);
				return TNC_RESULT_FATAL;
		default:
			DBG1(DBG_IMV, "IMV %u \"%s\" was notified of unknown state %u "
				 		  "for Connection ID %u",
						  this->id, this->name, new_state, connection_id);
			return TNC_RESULT_INVALID_PARAMETER;
	}
	return TNC_RESULT_SUCCESS;
}

METHOD(imv_agent_t, get_state, bool,
	private_imv_agent_t *this, TNC_ConnectionID connection_id,
							   imv_state_t **state)
{
	*state = find_connection(this, connection_id);
	if (!*state)
	{
		DBG1(DBG_IMV, "IMV %u \"%s\" has no state for Connection ID %u",
					  this->id, this->name, connection_id);
		return FALSE;
	}
	return TRUE;
}

METHOD(imv_agent_t, get_name, const char*,
	private_imv_agent_t *this)
{
	return	this->name;
}

METHOD(imv_agent_t, get_id, TNC_IMVID,
	private_imv_agent_t *this)
{
	return	this->id;
}

METHOD(imv_agent_t, reserve_additional_ids, TNC_Result,
	private_imv_agent_t *this, int count)
{
	TNC_Result result;
	TNC_UInt32 id;
	void *pointer;

	if (!this->reserve_additional_id)
	{
		DBG1(DBG_IMV, "IMV %u \"%s\" did not detect the capability to reserve "
					  "additional IMV IDs from the TNCS", this->id, this->name);
		return TNC_RESULT_ILLEGAL_OPERATION;
	}
	while (count > 0)
	{
		result = this->reserve_additional_id(this->id, &id);
		if (result != TNC_RESULT_SUCCESS)
		{
			DBG1(DBG_IMV, "IMV %u \"%s\" failed to reserve %d additional IMV IDs",
						  this->id, this->name, count);
			return result;
		}
		count--;

		/* store the scalar value in the pointer */
		pointer = (void*)(uintptr_t)id;
		this->additional_ids->insert_last(this->additional_ids, pointer);
		DBG2(DBG_IMV, "IMV %u \"%s\" reserved additional ID %u",
					  this->id, this->name, id);
	}
	return TNC_RESULT_SUCCESS;
}

METHOD(imv_agent_t, count_additional_ids, int,
	private_imv_agent_t *this)
{
	return	this->additional_ids->get_count(this->additional_ids);
}

METHOD(imv_agent_t, create_id_enumerator, enumerator_t*,
	private_imv_agent_t *this)
{
	return this->additional_ids->create_enumerator(this->additional_ids);
}

typedef struct {
	/**
	 * implements enumerator_t
	 */
	enumerator_t public;

	/**
	 * language length
	 */
	TNC_UInt32 lang_len;

	/**
	 * language buffer
	 */
	char lang_buf[BUF_LEN];

	/**
	 * position pointer into language buffer
	 */
	char *lang_pos;

} language_enumerator_t;

METHOD(enumerator_t, language_enumerator_enumerate, bool,
	language_enumerator_t *this, va_list args)
{
	char *pos, *cur_lang, **lang;
	TNC_UInt32 len;

	VA_ARGS_VGET(args, lang);

	if (!this->lang_len)
	{
		return FALSE;
	}
	cur_lang = this->lang_pos;
	pos = strchr(this->lang_pos, ',');
	if (pos)
	{
		len = pos - this->lang_pos;
		this->lang_pos += len + 1;
		this->lang_len -= len + 1;
	}
	else
	{
		len = this->lang_len;
		pos = this->lang_pos + len;
		this->lang_pos = NULL;
		this->lang_len = 0;
	}

	/* remove preceding whitespace */
	while (*cur_lang == ' ' && len--)
	{
		cur_lang++;
	}

	/* remove trailing whitespace */
	while (len && *(--pos) == ' ')
	{
		len--;
	}
	cur_lang[len] = '\0';

	*lang = cur_lang;
	return TRUE;
}

METHOD(imv_agent_t, create_language_enumerator, enumerator_t*,
	private_imv_agent_t *this, imv_state_t *state)
{
	language_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _language_enumerator_enumerate,
			.destroy = (void*)free,
		},
	);

	if (!this->get_attribute ||
		 this->get_attribute(this->id, state->get_connection_id(state),
						TNC_ATTRIBUTEID_PREFERRED_LANGUAGE, BUF_LEN,
						e->lang_buf, &e->lang_len) != TNC_RESULT_SUCCESS ||
		e->lang_len >= BUF_LEN)
	{
		e->lang_len = 0;
	}
	e->lang_buf[e->lang_len] = '\0';
	e->lang_pos = e->lang_buf;

	return (enumerator_t*)e;
}

METHOD(imv_agent_t, provide_recommendation, TNC_Result,
	private_imv_agent_t *this, imv_state_t *state)
{
	TNC_IMV_Action_Recommendation rec;
	TNC_IMV_Evaluation_Result eval;
	TNC_ConnectionID connection_id;
	chunk_t reason_string;
	char *reason_lang;
	enumerator_t *e;

	state->get_recommendation(state, &rec, &eval);
	connection_id = state->get_connection_id(state);

	/* send a reason string if action recommendation is not allow */
	if (rec != TNC_IMV_ACTION_RECOMMENDATION_ALLOW)
	{
		/* find a reason string for the preferred language and set it */
		if (this->set_attribute)
		{
			e = create_language_enumerator(this, state);
			if (state->get_reason_string(state, e, &reason_string, &reason_lang))
			{
				this->set_attribute(this->id, connection_id,
									TNC_ATTRIBUTEID_REASON_STRING,
									reason_string.len, reason_string.ptr);
				this->set_attribute(this->id, connection_id,
									TNC_ATTRIBUTEID_REASON_LANGUAGE,
									strlen(reason_lang), reason_lang);
			}
			e->destroy(e);
		}
	}
	return this->provide_recommendation(this->id, connection_id, rec, eval);
}

METHOD(imv_agent_t, add_non_fatal_attr_type, void,
	private_imv_agent_t *this, pen_type_t type)
{
	pen_type_t *type_p;

	type_p = malloc_thing(pen_type_t);
	*type_p = type;
	this->non_fatal_attr_types->insert_last(this->non_fatal_attr_types, type_p);
}

METHOD(imv_agent_t, get_non_fatal_attr_types, linked_list_t*,
	private_imv_agent_t *this)
{
	return this->non_fatal_attr_types;
}

METHOD(imv_agent_t, destroy, void,
	private_imv_agent_t *this)
{
	DBG1(DBG_IMV, "IMV %u \"%s\" terminated", this->id, this->name);
	this->additional_ids->destroy(this->additional_ids);
	this->non_fatal_attr_types->destroy_function(this->non_fatal_attr_types,
												 free);
	this->connections->destroy_offset(this->connections,
									  offsetof(imv_state_t, destroy));
	this->connection_lock->destroy(this->connection_lock);
	free(this);

	/* decrease the reference count or terminate */
	libimcv_deinit();
}

/**
 * Described in header.
 */
imv_agent_t *imv_agent_create(const char *name,
							  pen_type_t *supported_types, uint32_t type_count,
							  TNC_IMVID id, TNC_Version *actual_version)
{
	private_imv_agent_t *this;

	/* initialize  or increase the reference count */
	if (!libimcv_init(TRUE))
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.bind_functions = _bind_functions,
			.create_state = _create_state,
			.delete_state = _delete_state,
			.change_state = _change_state,
			.get_state = _get_state,
			.get_name = _get_name,
			.get_id = _get_id,
			.reserve_additional_ids = _reserve_additional_ids,
			.count_additional_ids = _count_additional_ids,
			.create_id_enumerator = _create_id_enumerator,
			.create_language_enumerator = _create_language_enumerator,
			.provide_recommendation = _provide_recommendation,
			.add_non_fatal_attr_type = _add_non_fatal_attr_type,
			.get_non_fatal_attr_types = _get_non_fatal_attr_types,
			.destroy = _destroy,
		},
		.name = name,
		.supported_types = supported_types,
		.type_count = type_count,
		.id = id,
		.additional_ids = linked_list_create(),
		.non_fatal_attr_types = linked_list_create(),
		.connections = linked_list_create(),
		.connection_lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	*actual_version = TNC_IFIMV_VERSION_1;
	DBG1(DBG_IMV, "IMV %u \"%s\" initialized", this->id, this->name);

	return &this->public;
}
