/*
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

#include "imcv.h"
#include "imc_agent.h"

#include <tncif_names.h>

#include <utils/debug.h>
#include <threading/rwlock.h>

typedef struct private_imc_agent_t private_imc_agent_t;

/**
 * Private data of an imc_agent_t object.
 */
struct private_imc_agent_t {

	/**
	 * Public members of imc_agent_t
	 */
	imc_agent_t public;

	/**
	 * name of IMC
	 */
	const char *name;

	/**
	 * message types registered by IMC
	 */
	pen_type_t *supported_types;

	/**
	 * number of message types registered by IMC
	 */
	uint32_t type_count;

	/**
	 * ID of IMC as assigned by TNCC
	 */
	TNC_IMCID id;

	/**
	 * List of additional IMC IDs assigned by TNCC
	 */
	linked_list_t *additional_ids;

	/**
	 * list of non-fatal unsupported PA-TNC attribute types
	 */
	linked_list_t *non_fatal_attr_types;

	/**
	 * list of TNCC connection entries
	 */
	linked_list_t *connections;

	/**
	 * rwlock to lock TNCC connection entries
	 */
	rwlock_t *connection_lock;

	/**
	 * Is the transport protocol PT-TLS?
	 */
	bool has_pt_tls;

	/**
	 * Inform a TNCC about the set of message types the IMC is able to receive
	 *
	 * @param imc_id			IMC ID assigned by TNCC
	 * @param supported_types	list of supported message types
	 * @param type_count		number of list elements
	 * @return					TNC result code
	 */
	TNC_Result (*report_message_types)(TNC_IMCID imc_id,
									   TNC_MessageTypeList supported_types,
									   TNC_UInt32 type_count);

	/**
	 * Inform a TNCC about the set of message types the IMC is able to receive
	 *
	 * @param imc_id				IMC ID assigned by TNCC
	 * @param supported_vids		list of supported message vendor IDs
	 * @param supported_subtypes	list of supported message subtypes
	 * @param type_count			number of list elements
	 * @return						TNC result code
	 */
	TNC_Result (*report_message_types_long)(TNC_IMCID imc_id,
									TNC_VendorIDList supported_vids,
									TNC_MessageSubtypeList supported_subtypes,
									TNC_UInt32 type_count);

	/**
	 * Get the value of an attribute associated with a connection
	 * or with the TNCC as a whole.
	 *
	 * @param imc_id			IMC ID assigned by TNCC
	 * @param connection_id		network connection ID assigned by TNCC
	 * @param attribute_id		attribute ID
	 * @param buffer_len		length of buffer in bytes
	 * @param buffer			buffer
	 * @param out_value_len		size in bytes of attribute stored in buffer
	 * @return					TNC result code
	 */
	TNC_Result (*get_attribute)(TNC_IMCID imc_id,
								TNC_ConnectionID connection_id,
								TNC_AttributeID attribute_id,
								TNC_UInt32 buffer_len,
								TNC_BufferReference buffer,
								TNC_UInt32 *out_value_len);

	/**
	 * Set the value of an attribute associated with a connection
	 * or with the TNCC as a whole.
	 *
	 * @param imc_id			IMV ID assigned by TNCC
	 * @param connection_id		network connection ID assigned by TNCC
	 * @param attribute_id		attribute ID
	 * @param buffer_len		length of buffer in bytes
	 * @param buffer			buffer
	 * @return					TNC result code
	 */
	TNC_Result (*set_attribute)(TNC_IMCID imc_id,
								TNC_ConnectionID connection_id,
								TNC_AttributeID attribute_id,
								TNC_UInt32 buffer_len,
								TNC_BufferReference buffer);

	/**
	 * Reserve an additional IMC ID
	 *
	 * @param imc_id			primary IMC ID assigned by TNCC
	 * @param out_imc_id		additional IMC ID assigned by TNCC
	 * @return					TNC result code
	 */
	TNC_Result (*reserve_additional_id)(TNC_IMCID imc_id,
										TNC_UInt32 *out_imc_id);

};

METHOD(imc_agent_t, bind_functions, TNC_Result,
	private_imc_agent_t *this, TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!bind_function)
	{
		DBG1(DBG_IMC, "TNC client failed to provide bind function");
		return TNC_RESULT_INVALID_PARAMETER;
	}
	if (bind_function(this->id, "TNC_TNCC_ReportMessageTypes",
			(void**)&this->report_message_types) != TNC_RESULT_SUCCESS)
	{
		this->report_message_types = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_ReportMessageTypesLong",
			(void**)&this->report_message_types_long) != TNC_RESULT_SUCCESS)
	{
		this->report_message_types_long = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_RequestHandshakeRetry",
			(void**)&this->public.request_handshake_retry) != TNC_RESULT_SUCCESS)
	{
		this->public.request_handshake_retry = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_SendMessage",
			(void**)&this->public.send_message) != TNC_RESULT_SUCCESS)
	{
		this->public.send_message = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_SendMessageLong",
			(void**)&this->public.send_message_long) != TNC_RESULT_SUCCESS)
	{
		this->public.send_message_long = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_GetAttribute",
			(void**)&this->get_attribute) != TNC_RESULT_SUCCESS)
	{
		this->get_attribute = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_SetAttribute",
			(void**)&this->set_attribute) != TNC_RESULT_SUCCESS)
	{
		this->set_attribute = NULL;
	}
	if (bind_function(this->id, "TNC_TNCC_ReserveAdditionalIMCID",
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
static imc_state_t* find_connection(private_imc_agent_t *this,
									 TNC_ConnectionID id)
{
	enumerator_t *enumerator;
	imc_state_t *state, *found = NULL;

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
static bool delete_connection(private_imc_agent_t *this, TNC_ConnectionID id)
{
	enumerator_t *enumerator;
	imc_state_t *state;
	bool found = FALSE;

	this->connection_lock->write_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &state))
	{
		if (id == state->get_connection_id(state))
		{
			found = TRUE;
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
static bool get_bool_attribute(private_imc_agent_t *this, TNC_ConnectionID id,
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
static char* get_str_attribute(private_imc_agent_t *this, TNC_ConnectionID id,
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
static uint32_t get_uint_attribute(private_imc_agent_t *this, TNC_ConnectionID id,
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

METHOD(imc_agent_t, create_state, TNC_Result,
	private_imc_agent_t *this, imc_state_t *state)
{
	TNC_ConnectionID conn_id;
	char *tnccs_p = NULL, *tnccs_v = NULL, *t_p = NULL, *t_v = NULL;
	bool has_long = FALSE, has_excl = FALSE, has_soh = FALSE;
	uint32_t max_msg_len;

	conn_id = state->get_connection_id(state);
	if (find_connection(this, conn_id))
	{
		DBG1(DBG_IMC, "IMC %u \"%s\" already created a state for Connection ID %u",
					   this->id, this->name, conn_id);
		state->destroy(state);
		return TNC_RESULT_OTHER;
	}

	/* Get and display attributes from TNCC via IF-IMC */
	has_long = get_bool_attribute(this, conn_id, TNC_ATTRIBUTEID_HAS_LONG_TYPES);
	has_excl = get_bool_attribute(this, conn_id, TNC_ATTRIBUTEID_HAS_EXCLUSIVE);
	has_soh  = get_bool_attribute(this, conn_id, TNC_ATTRIBUTEID_HAS_SOH);
	tnccs_p = get_str_attribute(this, conn_id, TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL);
	tnccs_v = get_str_attribute(this, conn_id, TNC_ATTRIBUTEID_IFTNCCS_VERSION);
	t_p = get_str_attribute(this, conn_id, TNC_ATTRIBUTEID_IFT_PROTOCOL);
	t_v = get_str_attribute(this, conn_id, TNC_ATTRIBUTEID_IFT_VERSION);
	max_msg_len = get_uint_attribute(this, conn_id, TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE);

	state->set_flags(state, has_long, has_excl);
	state->set_max_msg_len(state, max_msg_len);

	DBG2(DBG_IMC, "IMC %u \"%s\" created a state for %s %s Connection ID %u: "
				  "%slong %sexcl %ssoh", this->id, this->name,
				  tnccs_p ? tnccs_p:"?", tnccs_v ? tnccs_v:"?", conn_id,
			      has_long ? "+":"-", has_excl ? "+":"-", has_soh ? "+":"-");
	DBG2(DBG_IMC, "  over %s %s with maximum PA-TNC message size of %u bytes",
				  t_p ? t_p:"?", t_v ? t_v :"?", max_msg_len);

	this->has_pt_tls = streq(t_p, "IF-T for TLS");

	free(tnccs_p);
	free(tnccs_v);
	free(t_p);
	free(t_v);

	this->connection_lock->write_lock(this->connection_lock);
	this->connections->insert_last(this->connections, state);
	this->connection_lock->unlock(this->connection_lock);
	return TNC_RESULT_SUCCESS;
}

METHOD(imc_agent_t, delete_state, TNC_Result,
	private_imc_agent_t *this, TNC_ConnectionID connection_id)
{
	if (!delete_connection(this, connection_id))
	{
		DBG1(DBG_IMC, "IMC %u \"%s\" has no state for Connection ID %u",
					  this->id, this->name, connection_id);
		return TNC_RESULT_FATAL;
	}
	DBG2(DBG_IMC, "IMC %u \"%s\" deleted the state of Connection ID %u",
				  this->id, this->name, connection_id);
	return TNC_RESULT_SUCCESS;
}

METHOD(imc_agent_t, change_state, TNC_Result,
	private_imc_agent_t *this, TNC_ConnectionID connection_id,
							   TNC_ConnectionState new_state,
							   imc_state_t **state_p)
{
	imc_state_t *state;
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
				DBG1(DBG_IMC, "IMC %u \"%s\" has no state for Connection ID %u",
							  this->id, this->name, connection_id);
				return TNC_RESULT_FATAL;
			}
			old_state = state->change_state(state, new_state);
			DBG2(DBG_IMC, "IMC %u \"%s\" changed state of Connection ID %u to '%N'",
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
				DBG2(DBG_IMC, "IMC %u \"%s\" reset state of Connection ID %u",
							   this->id, this->name, connection_id);
			}
			break;
		case TNC_CONNECTION_STATE_CREATE:
			DBG1(DBG_IMC, "state '%N' should be handled by create_state()",
						  TNC_Connection_State_names, new_state);
				return TNC_RESULT_FATAL;
		case TNC_CONNECTION_STATE_DELETE:
			DBG1(DBG_IMC, "state '%N' should be handled by delete_state()",
						  TNC_Connection_State_names, new_state);
				return TNC_RESULT_FATAL;
		default:
			DBG1(DBG_IMC, "IMC %u \"%s\" was notified of unknown state %u "
				 		  "for Connection ID %u",
						  this->id, this->name, new_state, connection_id);
			return TNC_RESULT_INVALID_PARAMETER;
	}
	return TNC_RESULT_SUCCESS;
}

METHOD(imc_agent_t, get_state, bool,
	private_imc_agent_t *this, TNC_ConnectionID connection_id,
							   imc_state_t **state)
{
	*state = find_connection(this, connection_id);
	if (!*state)
	{
		DBG1(DBG_IMC, "IMC %u \"%s\" has no state for Connection ID %u",
					  this->id, this->name, connection_id);
		return FALSE;
	}
	return TRUE;
}

METHOD(imc_agent_t, get_name, const char*,
	private_imc_agent_t *this)
{
	return	this->name;
}

METHOD(imc_agent_t, get_id, TNC_IMCID,
	private_imc_agent_t *this)
{
	return	this->id;
}

METHOD(imc_agent_t, reserve_additional_ids, TNC_Result,
	private_imc_agent_t *this, int count)
{
	TNC_Result result;
	TNC_UInt32 id;
	void *pointer;

	if (!this->reserve_additional_id)
	{
		DBG1(DBG_IMC, "IMC %u \"%s\" did not detect the capability to reserve "
					  "additional IMC IDs from the TNCC", this->id, this->name);
		return TNC_RESULT_ILLEGAL_OPERATION;
	}
	while (count > 0)
	{
		result = this->reserve_additional_id(this->id, &id);
		if (result != TNC_RESULT_SUCCESS)
		{
			DBG1(DBG_IMC, "IMC %u \"%s\" failed to reserve %d additional IMC IDs",
						  this->id, this->name, count);
			return result;
		}
		count--;

		/* store the scalar value in the pointer */
		pointer = (void*)(uintptr_t)id;
		this->additional_ids->insert_last(this->additional_ids, pointer);
		DBG2(DBG_IMC, "IMC %u \"%s\" reserved additional ID %u",
					  this->id, this->name, id);
	}
	return TNC_RESULT_SUCCESS;
}

METHOD(imc_agent_t, count_additional_ids, int,
	private_imc_agent_t *this)
{
	return	this->additional_ids->get_count(this->additional_ids);
}

METHOD(imc_agent_t, create_id_enumerator, enumerator_t*,
	private_imc_agent_t *this)
{
	return this->additional_ids->create_enumerator(this->additional_ids);
}

METHOD(imc_agent_t, add_non_fatal_attr_type, void,
	private_imc_agent_t *this, pen_type_t type)
{
	pen_type_t *type_p;

	type_p = malloc_thing(pen_type_t);
	*type_p = type;
	this->non_fatal_attr_types->insert_last(this->non_fatal_attr_types, type_p);
}

METHOD(imc_agent_t, get_non_fatal_attr_types, linked_list_t*,
	private_imc_agent_t *this)
{
	return this->non_fatal_attr_types;
}

METHOD(imc_agent_t, has_pt_tls, bool,
	private_imc_agent_t *this)
{
	return	this->has_pt_tls;
}

METHOD(imc_agent_t, destroy, void,
	private_imc_agent_t *this)
{
	DBG1(DBG_IMC, "IMC %u \"%s\" terminated", this->id, this->name);
	this->additional_ids->destroy(this->additional_ids);
	this->non_fatal_attr_types->destroy_function(this->non_fatal_attr_types,
												 free);
	this->connections->destroy_function(this->connections, free);
	this->connection_lock->destroy(this->connection_lock);
	free(this);

	/* decrease the reference count or terminate */
	libimcv_deinit();
}

/**
 * Described in header.
 */
imc_agent_t *imc_agent_create(const char *name,
							  pen_type_t *supported_types, uint32_t type_count,
							  TNC_IMCID id, TNC_Version *actual_version)
{
	private_imc_agent_t *this;

	/* initialize  or increase the reference count */
	if (!libimcv_init(FALSE))
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
			.add_non_fatal_attr_type = _add_non_fatal_attr_type,
			.get_non_fatal_attr_types = _get_non_fatal_attr_types,
			.has_pt_tls = _has_pt_tls,
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

	*actual_version = TNC_IFIMC_VERSION_1;
	DBG1(DBG_IMC, "IMC %u \"%s\" initialized", this->id, this->name);

	return &this->public;
}
