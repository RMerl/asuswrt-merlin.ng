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

#define _GNU_SOURCE
#include <stdio.h>

#include "imv_swima_agent.h"
#include "imv_swima_state.h"

#include <imcv.h>
#include <imv/imv_agent.h>
#include <imv/imv_msg.h>
#include "rest/rest.h"
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"
#include "ietf/swima/ietf_swima_attr_req.h"
#include "ietf/swima/ietf_swima_attr_sw_inv.h"
#include "ietf/swima/ietf_swima_attr_sw_ev.h"
#include "swima/swima_error.h"
#include "swima/swima_inventory.h"
#include "swima/swima_events.h"
#include "swima/swima_data_model.h"

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>
#include <bio/bio_reader.h>

typedef struct private_imv_swima_agent_t private_imv_swima_agent_t;

/* Subscribed PA-TNC message subtypes */
static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_SWIMA }
};

/**
 * Flag set when corresponding attribute has been received
 */
enum imv_swima_attr_t {
	IMV_SWIMA_ATTR_SW_INV =    (1<<0),
	IMV_SWIMA_ATTR_SW_ID_INV = (1<<1),
	IMV_SWIMA_ATTR_SW_EV =     (1<<2),
	IMV_SWIMA_ATTR_SW_ID_EV =  (1<<2)
};

/**
 * Private data of an imv_swima_agent_t object.
 */
struct private_imv_swima_agent_t {

	/**
	 * Public members of imv_swima_agent_t
	 */
	imv_agent_if_t public;

	/**
	 * IMV agent responsible for generic functions
	 */
	imv_agent_t *agent;

	/**
	 * REST API to strongTNC manager
	 */
	rest_t *rest_api;

};

METHOD(imv_agent_if_t, bind_functions, TNC_Result,
	private_imv_swima_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	return this->agent->bind_functions(this->agent, bind_function);
}

METHOD(imv_agent_if_t, notify_connection_change, TNC_Result,
	private_imv_swima_agent_t *this, TNC_ConnectionID id,
	TNC_ConnectionState new_state)
{
	imv_state_t *state;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imv_swima_state_create(id);
			return this->agent->create_state(this->agent, state);
		case TNC_CONNECTION_STATE_DELETE:
			return this->agent->delete_state(this->agent, id);
		default:
			return this->agent->change_state(this->agent, id, new_state, NULL);
	}
}

/**
 * Process a received message
 */
static TNC_Result receive_msg(private_imv_swima_agent_t *this,
							  imv_state_t *state, imv_msg_t *in_msg)
{
	imv_swima_state_t *swima_state;
	imv_msg_t *out_msg;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	TNC_Result result;
	bool fatal_error = FALSE;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imv_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}

	swima_state = (imv_swima_state_t*)state;

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		uint32_t request_id = 0, last_eid, eid_epoch;
		swima_inventory_t *inventory;
		swima_events_t *events;
		pen_type_t type;

		type = attr->get_type(attr);

        if (type.vendor_id != PEN_IETF)
        {
            continue;
        }

		switch (type.type)
		{
			case IETF_ATTR_PA_TNC_ERROR:
			{
				ietf_attr_pa_tnc_error_t *error_attr;
				pen_type_t error_code;
				chunk_t msg_info, description;
				bio_reader_t *reader;
				uint32_t max_attr_size;
				bool success;

				error_attr = (ietf_attr_pa_tnc_error_t*)attr;
				error_code = error_attr->get_error_code(error_attr);

				if (error_code.vendor_id != PEN_IETF ||
					error_code.type <= PA_ERROR_PA_TNC_MSG_ROOF)
				{
					continue;
				}
				msg_info = error_attr->get_msg_info(error_attr);
				reader = bio_reader_create(msg_info);
				success = reader->read_uint32(reader, &request_id);

				DBG1(DBG_IMV, "received PA-TNC error '%N' for request %d",
					 pa_tnc_error_code_names, error_code.type, request_id);
				if (!success)
				{
					reader->destroy(reader);
					continue;
				}
				if (error_code.type == PA_ERROR_SWIMA_RESPONSE_TOO_LARGE)
				{
					if (!reader->read_uint32(reader, &max_attr_size))
					{
						reader->destroy(reader);
						continue;
					}
					DBG1(DBG_IMV, "  maximum PA-TNC attribute size is %u bytes",
						max_attr_size);
				}
				description = reader->peek(reader);
				if (description.len)
				{
					DBG1(DBG_IMV, "  description: %.*s", description.len,
														 description.ptr);
				}
				reader->destroy(reader);
				if (error_code.type == PA_ERROR_SWIMA_SUBSCRIPTION_DENIED)
				{
					swima_state->set_subscription(swima_state, FALSE);
					DBG1(DBG_IMV, "SWIMA subscription %u cleared",
								   swima_state->get_request_id(swima_state));
				}
				break;
			}
			case IETF_ATTR_SW_ID_INVENTORY:
			{
				ietf_swima_attr_sw_inv_t *attr_cast;
				uint32_t missing;
				int sw_id_count;

				state->set_action_flags(state, IMV_SWIMA_ATTR_SW_ID_INV);

				attr_cast = (ietf_swima_attr_sw_inv_t*)attr;
				request_id = attr_cast->get_request_id(attr_cast);
				inventory = attr_cast->get_inventory(attr_cast);
				last_eid = inventory->get_eid(inventory, &eid_epoch);
				sw_id_count = inventory->get_count(inventory);
				missing = attr_cast->get_record_count(attr_cast);
				swima_state->set_missing(swima_state, missing);

				DBG2(DBG_IMV, "received software ID inventory with "
					 "%d item%s for request %d at last eid %d of epoch 0x%08x, "
					 "%d item%s to follow", sw_id_count,
					 (sw_id_count == 1) ? "" : "s", request_id, last_eid,
					 eid_epoch, missing, (missing == 1) ? "" : "s");

				if (request_id == swima_state->get_request_id(swima_state))
				{
					swima_state->set_inventory(swima_state, inventory);
					swima_state->set_count(swima_state, sw_id_count, 0,
										   in_msg->get_src_id(in_msg));
				}
				else
				{
					DBG1(DBG_IMV, "no workitem found for software ID "
								  "inventory with request ID %d", request_id);
				}
				attr_cast->clear_inventory(attr_cast);
				break;
			 }
			case IETF_ATTR_SW_INVENTORY:
			{
				ietf_swima_attr_sw_inv_t *attr_cast;
				swima_record_t *sw_record;
				json_object *jobj, *jarray, *jstring;
				pen_type_t data_model;
				chunk_t tag;
				char *tag_str;
				uint32_t missing;
				int sw_count;
				enumerator_t *e;

				state->set_action_flags(state, IMV_SWIMA_ATTR_SW_INV);

				attr_cast = (ietf_swima_attr_sw_inv_t*)attr;
				request_id = attr_cast->get_request_id(attr_cast);
				inventory = attr_cast->get_inventory(attr_cast);
				last_eid = inventory->get_eid(inventory, &eid_epoch);
				sw_count = inventory->get_count(inventory);
				missing = attr_cast->get_record_count(attr_cast);
				swima_state->set_missing(swima_state, missing);

				DBG2(DBG_IMV, "received software inventory with %d item%s for "
					 "request %d at last eid %d of epoch 0x%08x, %d item%s to "
					 "follow", sw_count, (sw_count == 1) ? "" : "s", request_id,
					  last_eid, eid_epoch, missing, (missing == 1) ? "" : "s");

				if (request_id == swima_state->get_request_id(swima_state))
				{
					swima_state->set_count(swima_state, 0, sw_count,
										  in_msg->get_src_id(in_msg));

					if (this->rest_api)
					{
						jobj = json_object_new_object();
						jarray = json_object_new_array();
						json_object_object_add(jobj, "data", jarray);

						e = inventory->create_enumerator(inventory);
						while (e->enumerate(e, &sw_record))
						{
							tag = sw_record->get_record(sw_record);
							DBG3(DBG_IMV, "%.*s", tag.len, tag.ptr);

							data_model = sw_record->get_data_model(sw_record);
							if (!pen_type_equals(data_model,
										swima_data_model_iso_2015_swid_xml))
							{
								DBG1(DBG_IMV, "only ISO/IEC 19770-2-2015 XML "
											  "data model supported");
								continue;
							}

							tag_str = strndup(tag.ptr, tag.len);
							jstring = json_object_new_string(tag_str);
							json_object_array_add(jarray, jstring);
							free(tag_str);
						}
						e->destroy(e);

						if (this->rest_api->post(this->rest_api,
								"swid/add-tags/", jobj, NULL) != SUCCESS)
						{
							DBG1(DBG_IMV, "error in REST API add-tags request");
						}
						json_object_put(jobj);
					}
				}
				else
				{
					DBG1(DBG_IMV, "no workitem found for SWID tag inventory "
								  "with request ID %d", request_id);
				}
				attr_cast->clear_inventory(attr_cast);
				break;
			}
			case IETF_ATTR_SW_ID_EVENTS:
			{
				ietf_swima_attr_sw_ev_t *attr_cast;
				uint32_t missing;
				int sw_ev_count;

				state->set_action_flags(state, IMV_SWIMA_ATTR_SW_ID_EV);

				attr_cast = (ietf_swima_attr_sw_ev_t*)attr;
				request_id = attr_cast->get_request_id(attr_cast);
				events = attr_cast->get_events(attr_cast);
				last_eid = events->get_eid(events, &eid_epoch, NULL);
				sw_ev_count = events->get_count(events);
				missing = attr_cast->get_event_count(attr_cast);
				swima_state->set_missing(swima_state, missing);

				DBG2(DBG_IMV, "received software ID events with "
					 "%d item%s for request %d at last eid %d of epoch 0x%08x, "
					 "%d item%s to follow", sw_ev_count,
					 (sw_ev_count == 1) ? "" : "s", request_id, last_eid,
					 eid_epoch, missing, (missing == 1) ? "" : "s");

				if (request_id == swima_state->get_request_id(swima_state))
				{
					swima_state->set_events(swima_state, events);
					swima_state->set_count(swima_state, sw_ev_count, 0,
										   in_msg->get_src_id(in_msg));
				}
				else
				{
					DBG1(DBG_IMV, "no workitem found for software ID events "
								  "with request ID %d", request_id);
				}
				attr_cast->clear_events(attr_cast);
				break;

			}
			default:
				break;
		 }
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
								TNC_IMV_EVALUATION_RESULT_ERROR);
		result = out_msg->send_assessment(out_msg);
		if (result == TNC_RESULT_SUCCESS)
		{
			result = this->agent->provide_recommendation(this->agent, state);
		}
	}
	else
	{
		/* send PA-TNC message with the EXCL flag set */
		result = out_msg->send(out_msg, TRUE);
	}
	out_msg->destroy(out_msg);

	return result;
}

METHOD(imv_agent_if_t, receive_message, TNC_Result,
	private_imv_swima_agent_t *this, TNC_ConnectionID id,
	TNC_MessageType msg_type, chunk_t msg)
{
	imv_state_t *state;
	imv_msg_t *in_msg;
	TNC_Result result;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imv_msg_create_from_data(this->agent, state, id, msg_type, msg);
	result = receive_msg(this, state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

METHOD(imv_agent_if_t, receive_message_long, TNC_Result,
	private_imv_swima_agent_t *this, TNC_ConnectionID id,
	TNC_UInt32 src_imc_id, TNC_UInt32 dst_imv_id,
	TNC_VendorID msg_vid, TNC_MessageSubtype msg_subtype, chunk_t msg)
{
	imv_state_t *state;
	imv_msg_t *in_msg;
	TNC_Result result;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imv_msg_create_from_long_data(this->agent, state, id,
					src_imc_id, dst_imv_id, msg_vid, msg_subtype, msg);
	result = receive_msg(this, state, in_msg);
	in_msg->destroy(in_msg);

	return result;

}

METHOD(imv_agent_if_t, batch_ending, TNC_Result,
	private_imv_swima_agent_t *this, TNC_ConnectionID id)
{
	imv_msg_t *out_msg;
	imv_state_t *state;
	imv_session_t *session;
	imv_workitem_t *workitem;
	imv_swima_state_t *swima_state;
	imv_swima_handshake_state_t handshake_state;
	pa_tnc_attr_t *attr;
	TNC_IMVID imv_id;
	TNC_Result result = TNC_RESULT_SUCCESS;
	bool no_workitems = TRUE;
	uint32_t request_id, received;
	uint8_t flags;
	enumerator_t *enumerator;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	swima_state = (imv_swima_state_t*)state;
	handshake_state = swima_state->get_handshake_state(swima_state);
	session = state->get_session(state);
	imv_id = this->agent->get_id(this->agent);

	if (handshake_state == IMV_SWIMA_STATE_END)
	{
		return TNC_RESULT_SUCCESS;
	}

	/* Create an empty out message - we might need it */
	out_msg = imv_msg_create(this->agent, state, id, imv_id,
							 swima_state->get_imc_id(swima_state),
							 msg_types[0]);

	if (!imcv_db)
	{
		DBG2(DBG_IMV, "no workitems available - no evaluation possible");
		state->set_recommendation(state,
							TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
							TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
		result = out_msg->send_assessment(out_msg);
		out_msg->destroy(out_msg);
		swima_state->set_handshake_state(swima_state, IMV_SWIMA_STATE_END);

		if (result != TNC_RESULT_SUCCESS)
		{
			return result;
		}
		return this->agent->provide_recommendation(this->agent, state);
	}

	/* Look for SWID tag workitem and create SWID tag request */
	if (handshake_state == IMV_SWIMA_STATE_INIT &&
		session->get_policy_started(session))
	{
		size_t max_attr_size = SWIMA_MAX_ATTR_SIZE;
		size_t max_seg_size;
		ietf_swima_attr_req_t *cast_attr;
		seg_contract_t *contract;
		seg_contract_manager_t *contracts;
		swima_inventory_t *targets;
		uint32_t old_request_id = 0, earliest_eid = 0;
		char buf[BUF_LEN];

		enumerator = session->create_workitem_enumerator(session);
		if (enumerator)
		{
			while (enumerator->enumerate(enumerator, &workitem))
			{
				if (workitem->get_imv_id(workitem) != TNC_IMVID_ANY ||
					workitem->get_type(workitem) != IMV_WORKITEM_SWID_TAGS)
				{
					continue;
				}

				earliest_eid = workitem->get_arg_int(workitem);
				request_id = workitem->get_id(workitem);
				workitem->set_imv_id(workitem, imv_id);
				no_workitems = FALSE;
				old_request_id = swima_state->get_request_id(swima_state);

				flags = IETF_SWIMA_ATTR_REQ_FLAG_NONE;
				if (strchr(workitem->get_arg_str(workitem), 'R'))
				{
					flags |= IETF_SWIMA_ATTR_REQ_FLAG_R;
				}
				if (strchr(workitem->get_arg_str(workitem), 'S'))
				{
					flags |= IETF_SWIMA_ATTR_REQ_FLAG_S;
					swima_state->set_subscription(swima_state, TRUE);
					if (!old_request_id)
					{
						DBG1(DBG_IMV, "SWIMA subscription %u requested",
									   request_id);
					}
				}
				if (strchr(workitem->get_arg_str(workitem), 'C'))
				{
					flags |= IETF_SWIMA_ATTR_REQ_FLAG_C;
					swima_state->set_subscription(swima_state, FALSE);
				}

				if (!old_request_id)
				{
					/* Determine maximum PA-TNC attribute segment size */
					max_seg_size = state->get_max_msg_len(state)
									- PA_TNC_HEADER_SIZE
									- PA_TNC_ATTR_HEADER_SIZE
									- TCG_SEG_ATTR_SEG_ENV_HEADER;

					/* Announce support of PA-TNC segmentation to IMC */
					contract = seg_contract_create(msg_types[0], max_attr_size,
										max_seg_size, TRUE, imv_id, FALSE);
					contract->get_info_string(contract, buf, BUF_LEN, TRUE);
					DBG2(DBG_IMV, "%s", buf);
					contracts = state->get_contracts(state);
					contracts->add_contract(contracts, contract);
					attr = tcg_seg_attr_max_size_create(max_attr_size,
														max_seg_size, TRUE);
					out_msg->add_attribute(out_msg, attr);
				}

				if (!old_request_id ||
					!swima_state->get_subscription(swima_state))
				{
					/* Issue a SWID request */
					swima_state->set_request_id(swima_state, request_id);
					attr = ietf_swima_attr_req_create(flags, request_id);

					/* Request software identifier events */
					targets = swima_inventory_create();
					targets->set_eid(targets, earliest_eid, 0);
					cast_attr = (ietf_swima_attr_req_t*)attr;
					cast_attr->set_targets(cast_attr, targets);
					targets->destroy(targets);

					out_msg->add_attribute(out_msg, attr);
					DBG2(DBG_IMV, "IMV %d issues sw request %d with earliest "
								  "eid %d", imv_id, request_id, earliest_eid);
				}
				break;
			}
			enumerator->destroy(enumerator);

			if (no_workitems)
			{
				DBG2(DBG_IMV, "IMV %d has no workitems - "
							  "no evaluation requested", imv_id);
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
								TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			}
			handshake_state = IMV_SWIMA_STATE_WORKITEMS;
			swima_state->set_handshake_state(swima_state, handshake_state);
		}
	}

	received = state->get_action_flags(state);

	if (handshake_state == IMV_SWIMA_STATE_WORKITEMS &&
	   (received & (IMV_SWIMA_ATTR_SW_INV|IMV_SWIMA_ATTR_SW_ID_INV|
					IMV_SWIMA_ATTR_SW_EV |IMV_SWIMA_ATTR_SW_ID_EV)) &&
		swima_state->get_missing(swima_state) == 0)
	{
		TNC_IMV_Evaluation_Result eval;
		TNC_IMV_Action_Recommendation rec;
		char result_str[BUF_LEN], *format = NULL, *cmd = NULL, *command;
		char *target_str, *error_str = "";
		int sw_id_count, tag_count, i, res, written;
		json_object *jrequest, *jresponse, *jvalue;
		ietf_swima_attr_req_t *cast_attr;
		swima_inventory_t *targets;
		swima_record_t *target;
		status_t status = SUCCESS;

		if (received & IMV_SWIMA_ATTR_SW_ID_INV)
		{
			cmd = "swid-measurement";
			format = "received inventory of %d SW ID%s and %d SWID tag%s";
		}
		else if (received & IMV_SWIMA_ATTR_SW_ID_EV)
		{
			cmd = "swid-events";
			format = "received %d SW ID event%s and %d SWID tag%s";
		}

		if (cmd && this->rest_api)
		{
			res = asprintf(&command, "sessions/%d/%s/",
					 session->get_session_id(session, NULL, NULL), cmd);
			if (res < 0)
			{
				error_str = "allocation of command string failed";
				status = FAILED;
			}
			else
			{
				jrequest = swima_state->get_jrequest(swima_state);
				status = this->rest_api->post(this->rest_api, command,
											  jrequest, &jresponse);
				if (status == FAILED)
				{
						error_str = "error in REST API request";
				}
				free(command);
			}
		}

		switch (status)
		{
			case SUCCESS:
				enumerator = session->create_workitem_enumerator(session);
				while (enumerator->enumerate(enumerator, &workitem))
				{
					if (workitem->get_type(workitem) == IMV_WORKITEM_SWID_TAGS)
					{
						swima_state->get_count(swima_state, &sw_id_count,
														  &tag_count);
						if (format)
						{
							written = snprintf(result_str, BUF_LEN, format,
								sw_id_count, (sw_id_count == 1) ? "" : "s",
								tag_count,   (tag_count   == 1) ? "" : "s");
						}
						else
						{
							written = snprintf(result_str, BUF_LEN,
								"received %d SWID tag%s",
								tag_count, (tag_count == 1) ? "" : "s");

						}
						if (swima_state->get_subscription(swima_state) &&
							written > 0 && written < BUF_LEN)
						{
							snprintf(result_str + written, BUF_LEN - written,
								" from subscription %u",
								swima_state->get_request_id(swima_state));
						}
						session->remove_workitem(session, enumerator);

						eval = TNC_IMV_EVALUATION_RESULT_COMPLIANT;
						rec = workitem->set_result(workitem, result_str, eval);
						state->update_recommendation(state, rec, eval);
						imcv_db->finalize_workitem(imcv_db, workitem);
						workitem->destroy(workitem);
						break;
					}
				}
				enumerator->destroy(enumerator);
				break;
			case NEED_MORE:
				if (received & IMV_SWIMA_ATTR_SW_INV)
				{
					error_str = "not all requested SWID tags were received";
					status = FAILED;
					json_object_put(jresponse);
					break;
				}
				if (json_object_get_type(jresponse) != json_type_array)
				{
					error_str = "response was not a json_array";
					status = FAILED;
					json_object_put(jresponse);
					break;
				}

				/* Create an IETF SW Request attribute */
				attr = ietf_swima_attr_req_create(IETF_SWIMA_ATTR_REQ_FLAG_NONE,
								swima_state->get_request_id(swima_state));
				sw_id_count = json_object_array_length(jresponse);
				DBG1(DBG_IMV, "%d SWID tag target%s", sw_id_count,
							  (sw_id_count == 1) ? "" : "s");
				swima_state->set_missing(swima_state, sw_id_count);
				targets = swima_inventory_create();

				for (i = 0; i < sw_id_count; i++)
				{
					jvalue = json_object_array_get_idx(jresponse, i);
					if (json_object_get_type(jvalue) != json_type_string)
					{
						error_str = "json_string element expected in json_array";
						status = FAILED;
						json_object_put(jresponse);
						break;
					}
					target_str = (char*)json_object_get_string(jvalue);
					DBG1(DBG_IMV, "  %s", target_str);
					target = swima_record_create(0, chunk_from_str(target_str),
													chunk_empty);
					targets->add(targets, target);
				}
				json_object_put(jresponse);

				cast_attr = (ietf_swima_attr_req_t*)attr;
				cast_attr->set_targets(cast_attr, targets);
				targets->destroy(targets);
				out_msg->add_attribute(out_msg, attr);
				break;
			case FAILED:
			default:
				break;
		}

		if (status == FAILED)
		{
			enumerator = session->create_workitem_enumerator(session);
			while (enumerator->enumerate(enumerator, &workitem))
			{
				if (workitem->get_type(workitem) == IMV_WORKITEM_SWID_TAGS)
				{
					session->remove_workitem(session, enumerator);
					eval = TNC_IMV_EVALUATION_RESULT_ERROR;
					rec = workitem->set_result(workitem, error_str, eval);
					state->update_recommendation(state, rec, eval);
					imcv_db->finalize_workitem(imcv_db, workitem);
					workitem->destroy(workitem);
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
	}

	/* finalized all workitems ? */
	if (handshake_state == IMV_SWIMA_STATE_WORKITEMS &&
		session->get_workitem_count(session, imv_id) == 0)
	{
		result = out_msg->send_assessment(out_msg);
		out_msg->destroy(out_msg);
		swima_state->set_handshake_state(swima_state, IMV_SWIMA_STATE_END);

		if (result != TNC_RESULT_SUCCESS)
		{
			return result;
		}
		return this->agent->provide_recommendation(this->agent, state);
	}

	/* send non-empty PA-TNC message with excl flag not set */
	if (out_msg->get_attribute_count(out_msg))
	{
		result = out_msg->send(out_msg, FALSE);
	}
	out_msg->destroy(out_msg);

	return result;
}

METHOD(imv_agent_if_t, solicit_recommendation, TNC_Result,
	private_imv_swima_agent_t *this, TNC_ConnectionID id)
{
	imv_state_t *state;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	return this->agent->provide_recommendation(this->agent, state);
}

METHOD(imv_agent_if_t, destroy, void,
	private_imv_swima_agent_t *this)
{
	DESTROY_IF(this->rest_api);
	this->agent->destroy(this->agent);
	free(this);
}

/**
 * Described in header.
 */
imv_agent_if_t *imv_swima_agent_create(const char *name, TNC_IMVID id,
										 TNC_Version *actual_version)
{
	private_imv_swima_agent_t *this;
	imv_agent_t *agent;
	char *uri;
	u_int timeout;

	agent = imv_agent_create(name, msg_types, countof(msg_types), id,
							 actual_version);
	if (!agent)
	{
		return NULL;
	}
	agent->add_non_fatal_attr_type(agent,
				pen_type_create(PEN_TCG, TCG_SEG_MAX_ATTR_SIZE_REQ));

	INIT(this,
		.public = {
			.bind_functions = _bind_functions,
			.notify_connection_change = _notify_connection_change,
			.receive_message = _receive_message,
			.receive_message_long = _receive_message_long,
			.batch_ending = _batch_ending,
			.solicit_recommendation = _solicit_recommendation,
			.destroy = _destroy,
		},
		.agent = agent,
	);

	uri = lib->settings->get_str(lib->settings,
					"%s.plugins.imv-swima.rest_api.uri", NULL, lib->ns);
	timeout = lib->settings->get_int(lib->settings,
					"%s.plugins.imv-swima.rest_api.timeout", 120, lib->ns);
	if (uri)
	{
		this->rest_api = rest_create(uri, timeout);
	}

	return &this->public;
}

