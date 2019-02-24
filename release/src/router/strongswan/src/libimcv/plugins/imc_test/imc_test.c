/*
 * Copyright (C) 2011-2012 Andreas Steffen
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

#include "imc_test_state.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include <ietf/ietf_attr.h>
#include <ita/ita_attr.h>
#include <ita/ita_attr_command.h>
#include <ita/ita_attr_dummy.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

/* IMC definitions */

static const char imc_name[] = "Test";

static pen_type_t msg_types[] = {
	{ PEN_ITA, PA_SUBTYPE_ITA_TEST }
};

static imc_agent_t *imc_test;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Initialize(TNC_IMCID imc_id,
							  TNC_Version min_version,
							  TNC_Version max_version,
							  TNC_Version *actual_version)
{
	if (imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_test = imc_agent_create(imc_name, msg_types, countof(msg_types),
								imc_id, actual_version);
	if (!imc_test)
	{
		return TNC_RESULT_FATAL;
	}
	if (min_version > TNC_IFIMC_VERSION_1 || max_version < TNC_IFIMC_VERSION_1)
	{
		DBG1(DBG_IMC, "no common IF-IMC version");
		return TNC_RESULT_NO_COMMON_VERSION;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.2 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_NotifyConnectionChange(TNC_IMCID imc_id,
										  TNC_ConnectionID connection_id,
										  TNC_ConnectionState new_state)
{
	imc_state_t *state;
	imc_test_state_t *test_state;
	TNC_Result result;
	TNC_UInt32 additional_id;
	char *command;
	bool retry;
	void *pointer;
	enumerator_t *enumerator;
	int dummy_size, additional_ids;

	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			command = lib->settings->get_str(lib->settings,
								"%s.plugins.imc-test.command", "none", lib->ns);
			dummy_size = lib->settings->get_int(lib->settings,
								"%s.plugins.imc-test.dummy_size", 0, lib->ns);
			retry = lib->settings->get_bool(lib->settings,
								"%s.plugins.imc-test.retry", FALSE, lib->ns);
			state = imc_test_state_create(connection_id, command, dummy_size,
										  retry);

			result = imc_test->create_state(imc_test, state);
			if (result != TNC_RESULT_SUCCESS)
			{
				return result;
			}

			/* Optionally reserve additional IMC IDs */
			additional_ids = lib->settings->get_int(lib->settings,
							"%s.plugins.imc-test.additional_ids", 0, lib->ns);
			imc_test->reserve_additional_ids(imc_test, additional_ids -
								imc_test->count_additional_ids(imc_test));

			return TNC_RESULT_SUCCESS;

		case TNC_CONNECTION_STATE_HANDSHAKE:
			/* get updated IMC state */
			result = imc_test->change_state(imc_test, connection_id,
											new_state, &state);
			if (result != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			test_state = (imc_test_state_t*)state;

			/* is it the first handshake or a retry ? */
			if (!test_state->is_first_handshake(test_state))
			{
				command = lib->settings->get_str(lib->settings,
								"%s.plugins.imc-test.retry_command",
								test_state->get_command(test_state), lib->ns);
				test_state->set_command(test_state, command);
			}

			state->set_result(state, imc_id, TNC_IMV_EVALUATION_RESULT_DONT_KNOW);

			/* Exit if there are no additional IMC IDs */
			if (!imc_test->count_additional_ids(imc_test))
			{
				return result;
			}

			enumerator = imc_test->create_id_enumerator(imc_test);
			while (enumerator->enumerate(enumerator, &pointer))
			{
				/* interpret pointer as scalar value */
				additional_id = (TNC_UInt32)pointer;

				state->set_result(state, additional_id,
								  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			}
			enumerator->destroy(enumerator);

			return TNC_RESULT_SUCCESS;

		case TNC_CONNECTION_STATE_DELETE:
			return imc_test->delete_state(imc_test, connection_id);

		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			/* get updated IMC state */
			result = imc_test->change_state(imc_test, connection_id,
											new_state, &state);
			if (result != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			test_state = (imc_test_state_t*)state;

			/* do a handshake retry? */
			if (test_state->do_handshake_retry(test_state))
			{
				return imc_test->request_handshake_retry(imc_id, connection_id,
									TNC_RETRY_REASON_IMC_REMEDIATION_COMPLETE);
			}
			return TNC_RESULT_SUCCESS;

		default:
			return imc_test->change_state(imc_test, connection_id,
										  new_state, NULL);
	}
}

static void create_message(imc_state_t *state, imc_msg_t *out_msg)
{
	imc_test_state_t *test_state;
	pa_tnc_attr_t *attr;

	test_state = (imc_test_state_t*)state;
	if (test_state->get_dummy_size(test_state))
	{
		attr = ita_attr_dummy_create(test_state->get_dummy_size(test_state));
		attr->set_noskip_flag(attr, TRUE);
		out_msg->add_attribute(out_msg, attr);
	}
	attr = ita_attr_command_create(test_state->get_command(test_state));
	attr->set_noskip_flag(attr, TRUE);
	out_msg->add_attribute(out_msg, attr);
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_msg_t *out_msg;
	enumerator_t *enumerator;
	void *pointer;
	TNC_UInt32 additional_id;
	TNC_Result result;

	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_test->get_state(imc_test, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	/* send PA message for primary IMC ID with the EXCL flag set */
	out_msg = imc_msg_create(imc_test, state, connection_id, imc_id,
							 TNC_IMVID_ANY, msg_types[0]);
	create_message(state, out_msg);
	result = out_msg->send(out_msg, TRUE);
	out_msg->destroy(out_msg);

	/* Exit if there are no additional IMC IDs */
	if (!imc_test->count_additional_ids(imc_test))
	{
		return result;
	}

	/* Do we have support for transporting multiple IMC IDs? */
	if (!state->has_long(state))
	{
		DBG1(DBG_IMC, "IMC %u \"%s\" did not detect support for transporting "
					  "multiple IMC IDs", imc_id, imc_name);
		return result;
	}

	/* send PA messages for additional IMC IDs */
	enumerator = imc_test->create_id_enumerator(imc_test);
	while (result == TNC_RESULT_SUCCESS &&
		   enumerator->enumerate(enumerator, &pointer))
	{
		/* interpret pointer as scalar value */
		additional_id = (TNC_UInt32)pointer;
		out_msg = imc_msg_create(imc_test, state, connection_id, additional_id,
								 TNC_IMVID_ANY, msg_types[0]);
		create_message(state, out_msg);
		result = out_msg->send(out_msg, TRUE);
		out_msg->destroy(out_msg);
	}
	enumerator->destroy(enumerator);

	return result;
}

static TNC_Result receive_message(imc_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t attr_type;
	TNC_Result result = TNC_RESULT_SUCCESS;
	bool fatal_error = FALSE;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imc_msg_create_as_reply(in_msg);

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
		attr_type = attr->get_type(attr);

		if (attr_type.vendor_id != PEN_ITA)
		{
			continue;
		}
		if (attr_type.type == ITA_ATTR_COMMAND)
		{
			ita_attr_command_t *ita_attr;

			ita_attr = (ita_attr_command_t*)attr;
			DBG1(DBG_IMC, "received command '%s'",
				 ita_attr->get_command(ita_attr));
		}
		else if (attr_type.type == ITA_ATTR_DUMMY)
		{
			ita_attr_dummy_t *ita_attr;

			ita_attr = (ita_attr_dummy_t*)attr;
			DBG1(DBG_IMC, "received dummy attribute value (%d bytes)",
				 ita_attr->get_size(ita_attr));
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		result = TNC_RESULT_FATAL;
	}
	else
	{
		/* if no assessment result is known then repeat the measurement */
		if (!state->get_result(state, in_msg->get_dst_id(in_msg), NULL))
		{
			create_message(state, out_msg);
		}
		result = out_msg->send(out_msg, TRUE);
	}
	out_msg->destroy(out_msg);

	return result;
}

/**
 * see section 3.8.4 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_ReceiveMessage(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id,
								  TNC_BufferReference msg,
								  TNC_UInt32 msg_len,
								  TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_test->get_state(imc_test, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	in_msg = imc_msg_create_from_data(imc_test, state, connection_id, msg_type,
									  chunk_create(msg, msg_len));
	result = receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMC_ReceiveMessageLong(TNC_IMCID imc_id,
									  TNC_ConnectionID connection_id,
									  TNC_UInt32 msg_flags,
									  TNC_BufferReference msg,
									  TNC_UInt32 msg_len,
									  TNC_VendorID msg_vid,
									  TNC_MessageSubtype msg_subtype,
									  TNC_UInt32 src_imv_id,
									  TNC_UInt32 dst_imc_id)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_test->get_state(imc_test, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_test, state, connection_id,
								src_imv_id, dst_imc_id, msg_vid, msg_subtype,
								chunk_create(msg, msg_len));
	result =receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.7 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BatchEnding(TNC_IMCID imc_id,
							   TNC_ConnectionID connection_id)
{
	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.8 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Terminate(TNC_IMCID imc_id)
{
	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_test->destroy(imc_test);
	imc_test = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
									   TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_test)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_test->bind_functions(imc_test, bind_function);
}
