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

#include "imc_attestation_state.h"
#include "imc_attestation_process.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ietf/ietf_attr_product_info.h>
#include <ietf/ietf_attr_string_version.h>
#include <ietf/ietf_attr_assess_result.h>
#include <tcg/pts/tcg_pts_attr_proto_caps.h>
#include <tcg/pts/tcg_pts_attr_meas_algo.h>
#include <os_info/os_info.h>
#include <pts/pts_error.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>
#include <collections/linked_list.h>

/* IMC definitions */

static const char imc_name[] = "Attestation";

static pen_type_t msg_types[] = {
	{ PEN_TCG, PA_SUBTYPE_TCG_PTS }
};

static imc_agent_t *imc_attestation;

/**
 * Supported PTS measurement algorithms
 */
static pts_meas_algorithms_t supported_algorithms = PTS_MEAS_ALGO_NONE;

/**
 * Supported PTS Diffie Hellman Groups
 */
static pts_dh_group_t supported_dh_groups = PTS_DH_GROUP_NONE;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_Initialize(TNC_IMCID imc_id,
										  TNC_Version min_version,
										  TNC_Version max_version,
										  TNC_Version *actual_version)
{
	bool mandatory_dh_groups;

	if (imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_attestation = imc_agent_create(imc_name, msg_types, countof(msg_types),
									   imc_id, actual_version);
	if (!imc_attestation)
	{
		return TNC_RESULT_FATAL;
	}

	mandatory_dh_groups = lib->settings->get_bool(lib->settings,
			"%s.plugins.imc-attestation.mandatory_dh_groups", TRUE, lib->ns);

	if (!pts_meas_algo_probe(&supported_algorithms) ||
		!pts_dh_group_probe(&supported_dh_groups, mandatory_dh_groups))
	{
		imc_attestation->destroy(imc_attestation);
		imc_attestation = NULL;
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
TNC_Result TNC_IMC_API TNC_IMC_NotifyConnectionChange(TNC_IMCID imc_id,
												TNC_ConnectionID connection_id,
												TNC_ConnectionState new_state)
{
	imc_state_t *state;

	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_attestation_state_create(connection_id);
			return imc_attestation->create_state(imc_attestation, state);
		case TNC_CONNECTION_STATE_HANDSHAKE:
			if (imc_attestation->change_state(imc_attestation, connection_id,
				new_state, &state) != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			state->set_result(state, imc_id,
							  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			return TNC_RESULT_SUCCESS;
		case TNC_CONNECTION_STATE_DELETE:
			return imc_attestation->delete_state(imc_attestation, connection_id);
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
		default:
			return imc_attestation->change_state(imc_attestation, connection_id,
												  new_state, NULL);
	}
}


/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
											  TNC_ConnectionID connection_id)
{
	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}

	return TNC_RESULT_SUCCESS;
}

static TNC_Result receive_message(imc_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	imc_attestation_state_t *attestation_state;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t type;
	TNC_Result result;
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
		type = attr->get_type(attr);

		if (type.vendor_id == PEN_IETF)
		{
			if (type.type == IETF_ATTR_PA_TNC_ERROR)
			{
				ietf_attr_pa_tnc_error_t *error_attr;
				pen_type_t error_code;
				chunk_t msg_info;

				error_attr = (ietf_attr_pa_tnc_error_t*)attr;
				error_code = error_attr->get_error_code(error_attr);

				if (error_code.vendor_id == PEN_TCG)
				{
					msg_info = error_attr->get_msg_info(error_attr);

					DBG1(DBG_IMC, "received TCG-PTS error '%N'",
						 pts_error_code_names, error_code.type);
					DBG1(DBG_IMC, "error information: %B", &msg_info);

					result = TNC_RESULT_FATAL;
				}
			}
		}
		else if (type.vendor_id == PEN_TCG)
		{
			attestation_state = (imc_attestation_state_t*)state;

			if (!imc_attestation_process(attr, out_msg, attestation_state,
				supported_algorithms, supported_dh_groups))
			{
				result = TNC_RESULT_FATAL;
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (result == TNC_RESULT_SUCCESS)
	{
		/* send PA-TNC message with the EXCL flag set */
		result = out_msg->send(out_msg, TRUE);
	}
	out_msg->destroy(out_msg);

	return result;
}

/**
 * see section 3.8.4 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_ReceiveMessage(TNC_IMCID imc_id,
											  TNC_ConnectionID connection_id,
											  TNC_BufferReference msg,
											  TNC_UInt32 msg_len,
											  TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_attestation->get_state(imc_attestation, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	in_msg = imc_msg_create_from_data(imc_attestation, state, connection_id,
									  msg_type, chunk_create(msg, msg_len));
	result = receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_ReceiveMessageLong(TNC_IMCID imc_id,
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

	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_attestation->get_state(imc_attestation, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_attestation, state, connection_id,
								src_imv_id, dst_imc_id, msg_vid, msg_subtype,
								chunk_create(msg, msg_len));
	result =receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.7 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_BatchEnding(TNC_IMCID imc_id,
										   TNC_ConnectionID connection_id)
{
	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.8 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_Terminate(TNC_IMCID imc_id)
{
	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_attestation->destroy(imc_attestation);
	imc_attestation = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
								TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_attestation)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_attestation->bind_functions(imc_attestation, bind_function);
}
