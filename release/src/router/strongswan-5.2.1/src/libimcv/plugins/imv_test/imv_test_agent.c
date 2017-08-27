/*
 * Copyright (C) 2013-2014 Andreas Steffen
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

#include "imv_test_agent.h"
#include "imv_test_state.h"

#include <imv/imv_agent.h>
#include <imv/imv_msg.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ita/ita_attr.h>
#include <ita/ita_attr_get_settings.h>
#include <ita/ita_attr_command.h>
#include <ita/ita_attr_dummy.h>

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_imv_test_agent_t private_imv_test_agent_t;

/* Subscribed PA-TNC message subtypes */
static pen_type_t msg_types[] = {
	{ PEN_ITA, PA_SUBTYPE_ITA_TEST }
};

/**
 * Private data of an imv_test_agent_t object.
 */
struct private_imv_test_agent_t {

	/**
	 * Public members of imv_test_agent_t
	 */
	imv_agent_if_t public;

	/**
	 * IMV agent responsible for generic functions
	 */
	imv_agent_t *agent;

};

METHOD(imv_agent_if_t, bind_functions, TNC_Result,
	private_imv_test_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	return this->agent->bind_functions(this->agent, bind_function);
}

METHOD(imv_agent_if_t, notify_connection_change, TNC_Result,
	private_imv_test_agent_t *this, TNC_ConnectionID id,
	TNC_ConnectionState new_state)
{
	imv_state_t *state;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imv_test_state_create(id);
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
static TNC_Result receive_msg(private_imv_test_agent_t *this, imv_state_t *state,
							  imv_msg_t *in_msg)
{
	imv_msg_t *out_msg;
	imv_test_state_t *test_state;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t attr_type;
	TNC_Result result;
	int rounds;
	bool fatal_error = FALSE, received_command = FALSE, retry = FALSE;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imv_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}

	/* add any new IMC and set its number of rounds */
	rounds = lib->settings->get_int(lib->settings,
									"%s.plugins.imv-test.rounds", 0, lib->ns);
	test_state = (imv_test_state_t*)state;
	test_state->add_imc(test_state, in_msg->get_src_id(in_msg), rounds);

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		attr_type = attr->get_type(attr);

		if (attr_type.vendor_id != PEN_ITA)
		{
			continue;
		}
		if (attr_type.type == ITA_ATTR_COMMAND)
		{
			ita_attr_command_t *ita_attr;
			char *command;

			received_command = TRUE;
			ita_attr = (ita_attr_command_t*)attr;
			command = ita_attr->get_command(ita_attr);

			if (streq(command, "allow"))
			{
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
								TNC_IMV_EVALUATION_RESULT_COMPLIANT);
			}
			else if (streq(command, "isolate"))
			{
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_ISOLATE,
								TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR);
			}
			else if (streq(command, "block") || streq(command, "none"))
			{
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS,
								TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR);
			}
			else if (streq(command, "retry"))
			{
				retry = TRUE;
			}
			else
			{
				DBG1(DBG_IMV, "unsupported ITA Command '%s'", command);
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
								TNC_IMV_EVALUATION_RESULT_ERROR);
			}
		}
		else if (attr_type.type == ITA_ATTR_DUMMY)
		{
			ita_attr_dummy_t *ita_attr;

			ita_attr = (ita_attr_dummy_t*)attr;
			DBG1(DBG_IMV, "received dummy attribute value (%d bytes)",
						   ita_attr->get_size(ita_attr));
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
		return result;
	}

	/* request a handshake retry ? */
	if (retry)
	{
		test_state->set_rounds(test_state, rounds);
		return this->agent->request_handshake_retry(
									this->agent->get_id(this->agent),
									state->get_connection_id(state),
									TNC_RETRY_REASON_IMV_SERIOUS_EVENT);
	}

	/* repeat the measurement ? */
	if (test_state->another_round(test_state, in_msg->get_src_id(in_msg)))
	{
		attr = ita_attr_command_create("repeat");
		out_msg->add_attribute(out_msg, attr);

		/* send PA-TNC message with excl flag set */
		result = out_msg->send(out_msg, TRUE);
		out_msg->destroy(out_msg);

		return result;
	}

	if (received_command)
	{
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
	private_imv_test_agent_t *this, TNC_ConnectionID id,
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
	private_imv_test_agent_t *this, TNC_ConnectionID id,
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
	private_imv_test_agent_t *this, TNC_ConnectionID id)
{
	return TNC_RESULT_SUCCESS;
}

METHOD(imv_agent_if_t, solicit_recommendation, TNC_Result,
	private_imv_test_agent_t *this, TNC_ConnectionID id)
{
	imv_state_t *state;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	return this->agent->provide_recommendation(this->agent, state);
}

METHOD(imv_agent_if_t, destroy, void,
	private_imv_test_agent_t *this)
{
	DESTROY_IF(this->agent);
	free(this);
}

/**
 * Described in header.
 */
imv_agent_if_t *imv_test_agent_create(const char *name, TNC_IMVID id,
									  TNC_Version *actual_version)
{
	private_imv_test_agent_t *this;
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

