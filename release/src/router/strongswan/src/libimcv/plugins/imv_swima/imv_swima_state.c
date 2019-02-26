/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "imv_swima_state.h"

#include <imv/imv_lang_string.h>
#include <imv/imv_reason_string.h>
#include <imv/imv_remediation_string.h>

#include <tncif_policy.h>

#include <utils/debug.h>

typedef struct private_imv_swima_state_t private_imv_swima_state_t;

/**
 * Private data of an imv_swima_state_t object.
 */
struct private_imv_swima_state_t {

	/**
	 * Public members of imv_swima_state_t
	 */
	imv_swima_state_t public;

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID connection_id;

	/**
	 * TNCCS connection state
	 */
	TNC_ConnectionState state;

	/**
	 * Does the TNCCS connection support long message types?
	 */
	bool has_long;

	/**
	 * Does the TNCCS connection support exclusive delivery?
	 */
	bool has_excl;

	/**
	 * Maximum PA-TNC message size for this TNCCS connection
	 */
	uint32_t max_msg_len;

	/**
	 * Flags set for completed actions
	 */
	uint32_t action_flags;

	/**
	 * IMV database session associated with TNCCS connection
	 */
	imv_session_t *session;

	/**
	 * PA-TNC attribute segmentation contracts associated with TNCCS connection
	 */
	seg_contract_manager_t *contracts;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * IMV evaluation result
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * IMV Scanner handshake state
	 */
	imv_swima_handshake_state_t handshake_state;

	/**
	 * TNC Reason String
	 */
	imv_reason_string_t *reason_string;

	/**
	 * IETF Remediation Instructions String
	 */
	imv_remediation_string_t *remediation_string;

	/**
	 * Has a subscription been established?
	 */
	bool has_subscription;

	/**
	 * SWID Tag Request ID
	 */
	uint32_t request_id;

	/**
	 * Number of processed Software Identifiers
	 */
	int sw_id_count;

	/**
	 * Number of processed SWID Tags
	 */
	int tag_count;

	/**
	 * Number of missing Software Identifiers or SWID Tags
	 */
	uint32_t missing;

	/**
	 * SWID IMC ID
	 */
	TNC_UInt32 imc_id;

	/**
	 * Top level JSON object
	 */
	json_object *jobj;

	/**
	 * JSON array containing either a SW [ID] inventory or SW ID events
	 */
	json_object *jarray;

};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_swima_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_swima_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_swima_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_swima_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_swima_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_swima_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_action_flags, void,
	private_imv_swima_state_t *this, uint32_t flags)
{
	this->action_flags |= flags;
}

METHOD(imv_state_t, get_action_flags, uint32_t,
	private_imv_swima_state_t *this)
{
	return this->action_flags;
}

METHOD(imv_state_t, set_session, void,
	private_imv_swima_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_swima_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_swima_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, change_state, TNC_ConnectionState,
	private_imv_swima_state_t *this, TNC_ConnectionState new_state)
{
	TNC_ConnectionState old_state;

	old_state = this->state;
	this->state = new_state;
	return old_state;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_swima_state_t *this, TNC_IMV_Action_Recommendation *rec,
									   TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_swima_state_t *this, TNC_IMV_Action_Recommendation rec,
									   TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_swima_state_t *this, TNC_IMV_Action_Recommendation rec,
									   TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_swima_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	return FALSE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_swima_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	return FALSE;
}

METHOD(imv_state_t, reset, void,
	private_imv_swima_state_t *this)
{
	this->rec  = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
	this->eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;

	this->action_flags = 0;

	this->handshake_state = IMV_SWIMA_STATE_INIT;
	this->sw_id_count = 0;
	this->tag_count = 0;
	this->missing = 0;

	json_object_put(this->jobj);
	this->jobj = json_object_new_object();
}

METHOD(imv_state_t, destroy, void,
	private_imv_swima_state_t *this)
{
	json_object_put(this->jobj);
	DESTROY_IF(this->session);
	this->contracts->destroy(this->contracts);
	free(this);
}

METHOD(imv_swima_state_t, set_handshake_state, void,
	private_imv_swima_state_t *this, imv_swima_handshake_state_t new_state)
{
	this->handshake_state = new_state;
}

METHOD(imv_swima_state_t, get_handshake_state, imv_swima_handshake_state_t,
	private_imv_swima_state_t *this)
{
	return this->handshake_state;
}

METHOD(imv_swima_state_t, set_request_id, void,
	private_imv_swima_state_t *this, uint32_t request_id)
{
	this->request_id = request_id;
}

METHOD(imv_swima_state_t, get_request_id, uint32_t,
	private_imv_swima_state_t *this)
{
	return this->request_id;
}

METHOD(imv_swima_state_t, set_inventory, void,
    private_imv_swima_state_t *this, swima_inventory_t *inventory)
{
	chunk_t sw_id, sw_locator;
	uint32_t record_id;
	char *sw_id_str;
	json_object *jstring;
	swima_record_t *sw_record;
	enumerator_t *enumerator;

	if (this->sw_id_count == 0)
	{
		this->jarray = json_object_new_array();
		json_object_object_add(this->jobj, "data", this->jarray);
	}

	enumerator = inventory->create_enumerator(inventory);
	while (enumerator->enumerate(enumerator, &sw_record))
	{
		record_id = sw_record->get_record_id(sw_record);
		sw_id = sw_record->get_sw_id(sw_record, &sw_locator);
		sw_id_str = strndup(sw_id.ptr, sw_id.len);
		if (sw_locator.len)
		{
			DBG3(DBG_IMV, "%6u: %s @ %.*s", record_id, sw_id_str,
						   sw_locator.len, sw_locator.ptr);
		}
		else
		{
			DBG3(DBG_IMV, "%6u: %s", record_id, sw_id_str);
		}

		/* Add software identity to JSON array */
		jstring = json_object_new_string(sw_id_str);
		json_object_array_add(this->jarray, jstring);
		free(sw_id_str);
	}
	enumerator->destroy(enumerator);
}

METHOD(imv_swima_state_t, set_events, void,
    private_imv_swima_state_t *this, swima_events_t *events)
{
	chunk_t sw_id, timestamp;
	uint32_t record_id, eid, last_eid, epoch, source_id, action;
	char *sw_id_str, *timestamp_str;
	json_object *jevent, *jvalue, *jstring;
	swima_event_t *sw_event;
	swima_record_t *sw_record;
	enumerator_t *enumerator;

	if (this->sw_id_count == 0)
	{
		last_eid = events->get_eid(events, &epoch, NULL);
		jvalue = json_object_new_int(epoch);
		json_object_object_add(this->jobj, "epoch", jvalue);
		jvalue = json_object_new_int(last_eid);
		json_object_object_add(this->jobj, "lastEid", jvalue);
		this->jarray = json_object_new_array();
		json_object_object_add(this->jobj, "events", this->jarray);
	}

	enumerator = events->create_enumerator(events);
	while (enumerator->enumerate(enumerator, &sw_event))
	{
		eid = sw_event->get_eid(sw_event, &timestamp);
		timestamp_str = strndup(timestamp.ptr, timestamp.len);
		action = sw_event->get_action(sw_event);
		sw_record = sw_event->get_sw_record(sw_event);
		record_id = sw_record->get_record_id(sw_record);
		source_id = sw_record->get_source_id(sw_record);
		sw_id = sw_record->get_sw_id(sw_record, NULL);
		sw_id_str = strndup(sw_id.ptr, sw_id.len);
		DBG3(DBG_IMV, "%3u %.*s %u %5u: %s", eid, timestamp.len, timestamp.ptr,
											 action, record_id, sw_id_str);

		/* Add software event to JSON array */
		jevent = json_object_new_object();
		jvalue = json_object_new_int(eid);
		json_object_object_add(jevent, "eid", jvalue);
		jstring = json_object_new_string(timestamp_str);
		json_object_object_add(jevent, "timestamp", jstring);
		jvalue = json_object_new_int(record_id);
		json_object_object_add(jevent, "recordId", jvalue);
		jvalue = json_object_new_int(source_id);
		json_object_object_add(jevent, "sourceId", jvalue);
		jvalue = json_object_new_int(action);
		json_object_object_add(jevent, "action", jvalue);
		jstring = json_object_new_string(sw_id_str);
		json_object_object_add(jevent, "softwareId", jstring);
		json_object_array_add(this->jarray, jevent);
		free(timestamp_str);
		free(sw_id_str);
	}
	enumerator->destroy(enumerator);
}

METHOD(imv_swima_state_t, get_jrequest, json_object*,
	private_imv_swima_state_t *this)
{
	return this->jobj;
}

METHOD(imv_swima_state_t, set_missing, void,
	private_imv_swima_state_t *this, uint32_t count)
{
	this->missing = count;
}

METHOD(imv_swima_state_t, get_missing, uint32_t,
	private_imv_swima_state_t *this)
{
	return this->missing;
}

METHOD(imv_swima_state_t, set_count, void,
	private_imv_swima_state_t *this, int sw_id_count, int tag_count,
	TNC_UInt32 imc_id)
{
	this->sw_id_count += sw_id_count;
	this->tag_count += tag_count;
	this->imc_id = imc_id;
}

METHOD(imv_swima_state_t, get_count, void,
	private_imv_swima_state_t *this, int *sw_id_count, int *tag_count)
{
	if (sw_id_count)
	{
		*sw_id_count = this->sw_id_count;
	}
	if (tag_count)
	{
		*tag_count = this->tag_count;
	}
}

METHOD(imv_swima_state_t, get_imc_id, TNC_UInt32,
	private_imv_swima_state_t *this)
{
	return this->imc_id;
}

METHOD(imv_swima_state_t, set_subscription, void,
	private_imv_swima_state_t *this, bool set)
{
	this->has_subscription = set;
}

METHOD(imv_swima_state_t, get_subscription, bool,
	private_imv_swima_state_t *this)
{
	return this->has_subscription;
}

/**
 * Described in header.
 */
imv_state_t *imv_swima_state_create(TNC_ConnectionID connection_id)
{
	private_imv_swima_state_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_connection_id = _get_connection_id,
				.has_long = _has_long,
				.has_excl = _has_excl,
				.set_flags = _set_flags,
				.set_max_msg_len = _set_max_msg_len,
				.get_max_msg_len = _get_max_msg_len,
				.set_action_flags = _set_action_flags,
				.get_action_flags = _get_action_flags,
				.set_session = _set_session,
				.get_session= _get_session,
				.get_contracts = _get_contracts,
				.change_state = _change_state,
				.get_recommendation = _get_recommendation,
				.set_recommendation = _set_recommendation,
				.update_recommendation = _update_recommendation,
				.get_reason_string = _get_reason_string,
				.get_remediation_instructions = _get_remediation_instructions,
				.reset = _reset,
				.destroy = _destroy,
			},
			.set_handshake_state = _set_handshake_state,
			.get_handshake_state = _get_handshake_state,
			.set_request_id = _set_request_id,
			.get_request_id = _get_request_id,
			.set_inventory = _set_inventory,
			.set_events = _set_events,
			.get_jrequest = _get_jrequest,
			.set_missing = _set_missing,
			.get_missing = _get_missing,
			.set_count = _set_count,
			.get_count = _get_count,
			.get_imc_id = _get_imc_id,
			.set_subscription = _set_subscription,
			.get_subscription = _get_subscription,
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
		.imc_id = TNC_IMCID_ANY,
		.jobj = json_object_new_object(),
	);

	return &this->public.interface;
}


