/*
 * Copyright (C) 2013-2015 Andreas Steffen
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

#include "imv_scanner_agent.h"
#include "imv_scanner_state.h"

#include <imcv.h>
#include <imv/imv_agent.h>
#include <imv/imv_msg.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ietf/ietf_attr_port_filter.h>

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>
#include <utils/lexparser.h>

typedef struct private_imv_scanner_agent_t private_imv_scanner_agent_t;

/* Subscribed PA-TNC message subtypes */
static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_FIREWALL }
};

/**
 * Private data of an imv_scanner_agent_t object.
 */
struct private_imv_scanner_agent_t {

	/**
	 * Public members of imv_scanner_agent_t
	 */
	imv_agent_if_t public;

	/**
	 * IMV agent responsible for generic functions
	 */
	imv_agent_t *agent;

};

METHOD(imv_agent_if_t, bind_functions, TNC_Result,
	private_imv_scanner_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	return this->agent->bind_functions(this->agent, bind_function);
}

METHOD(imv_agent_if_t, notify_connection_change, TNC_Result,
	private_imv_scanner_agent_t *this, TNC_ConnectionID id,
	TNC_ConnectionState new_state)
{
	imv_state_t *state;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imv_scanner_state_create(id);
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
static TNC_Result receive_msg(private_imv_scanner_agent_t *this,
							  imv_state_t *state, imv_msg_t *in_msg)
{
	imv_msg_t *out_msg;
	imv_scanner_state_t *scanner_state;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t type;
	TNC_Result result;
	ietf_attr_port_filter_t *port_filter_attr;
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

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		type = attr->get_type(attr);

		if (type.vendor_id == PEN_IETF && type.type == IETF_ATTR_PORT_FILTER)
		{
			scanner_state = (imv_scanner_state_t*)state;
			port_filter_attr = (ietf_attr_port_filter_t*)attr->get_ref(attr);
			scanner_state->set_port_filter_attr(scanner_state, port_filter_attr);
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
	private_imv_scanner_agent_t *this, TNC_ConnectionID id,
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
	private_imv_scanner_agent_t *this, TNC_ConnectionID id,
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

typedef struct port_range_t port_range_t;

struct port_range_t {
	uint16_t start, stop;
};

/**
 * Parse a TCP or UDP port list from an argument string
 */
static linked_list_t* get_port_list(uint8_t protocol_family,
									bool closed_port_policy, char *arg_str)
{
	chunk_t port_list, port_item, port_start;
	port_range_t *port_range;
	linked_list_t *list;

	list = linked_list_create();

	port_list = chunk_from_str(arg_str);
	DBG2(DBG_IMV, "list of %s ports that %s:",
		 (protocol_family == IPPROTO_TCP) ? "tcp" : "udp",
		 closed_port_policy ? "are allowed to be open" : "must be closed");

	while (eat_whitespace(&port_list))
	{
		if (!extract_token(&port_item, ' ', &port_list))
		{
			/* reached last port item */
			port_item = port_list;
			port_list = chunk_empty;
		}
		port_range = malloc_thing(port_range_t);
		port_range->start = atoi(port_item.ptr);

		if (extract_token(&port_start, '-', &port_item) && port_item.len)
		{
			port_range->stop = atoi(port_item.ptr);
		}
		else
		{
			port_range->stop = port_range->start;
		}
		DBG2(DBG_IMV, "%5u - %5u", port_range->start, port_range->stop);
		list->insert_last(list, port_range);
	}

	return list;
}

METHOD(imv_agent_if_t, batch_ending, TNC_Result,
	private_imv_scanner_agent_t *this, TNC_ConnectionID id)
{
	imv_msg_t *out_msg;
	imv_state_t *state;
	imv_session_t *session;
	imv_workitem_t *workitem;
	imv_scanner_state_t *scanner_state;
	imv_scanner_handshake_state_t handshake_state;
	pa_tnc_attr_t *attr;
	ietf_attr_port_filter_t *port_filter_attr;
	TNC_IMVID imv_id;
	TNC_Result result = TNC_RESULT_SUCCESS;
	bool no_workitems = TRUE;
	enumerator_t *enumerator;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	scanner_state = (imv_scanner_state_t*)state;
	handshake_state = scanner_state->get_handshake_state(scanner_state);
	port_filter_attr = scanner_state->get_port_filter_attr(scanner_state);
	session = state->get_session(state);
	imv_id = this->agent->get_id(this->agent);

	if (handshake_state == IMV_SCANNER_STATE_END)
	{
		return TNC_RESULT_SUCCESS;
	}

	/* create an empty out message - we might need it */
	out_msg = imv_msg_create(this->agent, state, id, imv_id, TNC_IMCID_ANY,
							 msg_types[0]);

	if (!imcv_db)
	{
		DBG2(DBG_IMV, "no workitems available - no evaluation possible");
		state->set_recommendation(state,
							TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
							TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
		result = out_msg->send_assessment(out_msg);
		out_msg->destroy(out_msg);
		scanner_state->set_handshake_state(scanner_state, IMV_SCANNER_STATE_END);

		if (result != TNC_RESULT_SUCCESS)
		{
			return result;
		}
		return this->agent->provide_recommendation(this->agent, state);
	}

	if (handshake_state == IMV_SCANNER_STATE_INIT &&
		session->get_policy_started(session))
	{
		enumerator = session->create_workitem_enumerator(session);
		if (enumerator)
		{
			while (enumerator->enumerate(enumerator, &workitem))
			{
				if (workitem->get_imv_id(workitem) != TNC_IMVID_ANY)
				{
					continue;
				}

				switch (workitem->get_type(workitem))
				{
					case IMV_WORKITEM_TCP_PORT_OPEN:
					case IMV_WORKITEM_TCP_PORT_BLOCK:
					case IMV_WORKITEM_UDP_PORT_OPEN:
					case IMV_WORKITEM_UDP_PORT_BLOCK:
						if (!port_filter_attr &&
							handshake_state != IMV_SCANNER_STATE_ATTR_REQ)
						{
							attr = ietf_attr_attr_request_create(PEN_IETF,
											IETF_ATTR_PORT_FILTER);
							out_msg->add_attribute(out_msg, attr);
							handshake_state = IMV_SCANNER_STATE_ATTR_REQ;
						}
						break;
					default:
						continue;
				}
				workitem->set_imv_id(workitem, imv_id);
				no_workitems = FALSE;
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
			handshake_state = IMV_SCANNER_STATE_WORKITEMS;
			scanner_state->set_handshake_state(scanner_state, handshake_state);
		}
	}

	if (handshake_state == IMV_SCANNER_STATE_WORKITEMS && port_filter_attr)
	{
		TNC_IMV_Evaluation_Result eval;
		TNC_IMV_Action_Recommendation rec;
		uint8_t protocol_family, protocol;
		uint16_t port;
		bool closed_port_policy, blocked, first;
		char result_str[BUF_LEN], *pos, *protocol_str;
		size_t len;
		int written;
		linked_list_t *port_list;
		enumerator_t *e1, *e2;

		enumerator = session->create_workitem_enumerator(session);
		while (enumerator->enumerate(enumerator, &workitem))
		{
			if (workitem->get_imv_id(workitem) != imv_id)
			{
				continue;
			}
			eval = TNC_IMV_EVALUATION_RESULT_COMPLIANT;

			switch (workitem->get_type(workitem))
			{
				case IMV_WORKITEM_TCP_PORT_OPEN:
					protocol_family = IPPROTO_TCP;
					closed_port_policy = TRUE;
					break;
				case IMV_WORKITEM_TCP_PORT_BLOCK:
					protocol_family = IPPROTO_TCP;
					closed_port_policy = FALSE;
					break;
				case IMV_WORKITEM_UDP_PORT_OPEN:
					protocol_family = IPPROTO_UDP;
					closed_port_policy = TRUE;
					break;
				case IMV_WORKITEM_UDP_PORT_BLOCK:
					protocol_family = IPPROTO_UDP;
					closed_port_policy = FALSE;
					break;
				default:
					continue;
			}
			port_list = get_port_list(protocol_family, closed_port_policy,
									  workitem->get_arg_str(workitem));
			protocol_str = (protocol_family == IPPROTO_TCP) ? "tcp" : "udp";
			result_str[0] = '\0';
			pos = result_str;
			len = BUF_LEN;
			first = TRUE;

			e1 = port_filter_attr->create_port_enumerator(port_filter_attr);
			while (e1->enumerate(e1, &blocked, &protocol, &port))
			{
				port_range_t *port_range;
				bool passed, found = FALSE;
				char buf[20];

				if (blocked || protocol != protocol_family)
				{
					/* ignore closed ports or non-matching protocols */
					continue;
				}

				e2 = port_list->create_enumerator(port_list);
				while (e2->enumerate(e2, &port_range))
				{
					if (port >= port_range->start && port <= port_range->stop)
					{
						found = TRUE;
						break;
					}
				}
				e2->destroy(e2);

				passed = (closed_port_policy == found);
				DBG2(DBG_IMV, "%s port %5u open: %s", protocol_str, port,
							   passed ? "ok" : "fatal");
				if (!passed)
				{
					eval = TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR;
					snprintf(buf, sizeof(buf), "%s/%u", protocol_str, port);
					scanner_state->add_violating_port(scanner_state, strdup(buf));
					if (first)
					{
						written = snprintf(pos, len, "violating %s ports:",
													  protocol_str);
						if (written > 0 && written < len)
						{
							pos += written;
							len -= written;
						}
						first = FALSE;
					}
					written = snprintf(pos, len, " %u", port);
					if (written < 0 || written >= len)
					{
						pos += len - 1;
						*pos = '\0';
					}
					else
					{
						pos += written;
						len -= written;
					}
				}
			}
			e1->destroy(e1);

			if (first)
			{
				snprintf(pos, len, "no violating %s ports", protocol_str);
			}
			port_list->destroy(port_list);

			session->remove_workitem(session, enumerator);
			rec = workitem->set_result(workitem, result_str, eval);
			state->update_recommendation(state, rec, eval);
			imcv_db->finalize_workitem(imcv_db, workitem);
			workitem->destroy(workitem);
		}
		enumerator->destroy(enumerator);
	}

	/* finalized all workitems ? */
	if (handshake_state == IMV_SCANNER_STATE_WORKITEMS &&
		session->get_workitem_count(session, imv_id) == 0)
	{
		result = out_msg->send_assessment(out_msg);
		out_msg->destroy(out_msg);
		scanner_state->set_handshake_state(scanner_state, IMV_SCANNER_STATE_END);

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
	private_imv_scanner_agent_t *this, TNC_ConnectionID id)
{
	imv_state_t *state;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	return this->agent->provide_recommendation(this->agent, state);
}

METHOD(imv_agent_if_t, destroy, void,
	private_imv_scanner_agent_t *this)
{
	this->agent->destroy(this->agent);
	free(this);
}

/**
 * Described in header.
 */
imv_agent_if_t *imv_scanner_agent_create(const char *name, TNC_IMVID id,
										 TNC_Version *actual_version)
{
	private_imv_scanner_agent_t *this;
	imv_agent_t *agent;

	agent = imv_agent_create(name, msg_types, countof(msg_types), id,
							 actual_version);
	if (!agent)
	{
		return NULL;
	}

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

	return &this->public;
}

