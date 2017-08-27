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

#include "imc_swid_state.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"
#include "tcg/swid/tcg_swid_attr_req.h"
#include "tcg/swid/tcg_swid_attr_tag_inv.h"
#include "tcg/swid/tcg_swid_attr_tag_id_inv.h"
#include "swid/swid_inventory.h"
#include "swid/swid_error.h"

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

#define SWID_GENERATOR	"/usr/local/bin/swid_generator"

/* IMC definitions */

static const char imc_name[] = "SWID";

static pen_type_t msg_types[] = {
	{ PEN_TCG, PA_SUBTYPE_TCG_SWID }
};

static imc_agent_t *imc_swid;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Initialize(TNC_IMCID imc_id,
							  TNC_Version min_version,
							  TNC_Version max_version,
							  TNC_Version *actual_version)
{
	if (imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_swid = imc_agent_create(imc_name, msg_types, countof(msg_types),
							  imc_id, actual_version);
	if (!imc_swid)
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

	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_swid_state_create(connection_id);
			return imc_swid->create_state(imc_swid, state);
		case TNC_CONNECTION_STATE_HANDSHAKE:
			if (imc_swid->change_state(imc_swid, connection_id, new_state,
				&state) != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			state->set_result(state, imc_id,
							  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			return TNC_RESULT_SUCCESS;
		case TNC_CONNECTION_STATE_DELETE:
			return imc_swid->delete_state(imc_swid, connection_id);
		default:
			return imc_swid->change_state(imc_swid, connection_id,
										  new_state, NULL);
	}
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_msg_t *out_msg;
	pa_tnc_attr_t *attr;
	seg_contract_t *contract;
	seg_contract_manager_t *contracts;
	size_t max_attr_size = SWID_MAX_ATTR_SIZE;
	size_t max_seg_size;
	char buf[BUF_LEN];
	TNC_Result result = TNC_RESULT_SUCCESS;

	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swid->get_state(imc_swid, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	/* Determine maximum PA-TNC attribute segment size */
	max_seg_size = state->get_max_msg_len(state) - PA_TNC_HEADER_SIZE
												 - PA_TNC_ATTR_HEADER_SIZE
												 - TCG_SEG_ATTR_SEG_ENV_HEADER
												 - PA_TNC_ATTR_HEADER_SIZE
												 - TCG_SEG_ATTR_MAX_SIZE_SIZE;

	/* Announce support of PA-TNC segmentation to IMV */
	contract = seg_contract_create(msg_types[0], max_attr_size, max_seg_size,
									 TRUE, imc_id, TRUE);
	contract->get_info_string(contract, buf, BUF_LEN, TRUE);
	DBG2(DBG_IMC, "%s", buf);
	contracts = state->get_contracts(state);
	contracts->add_contract(contracts, contract);
	attr = tcg_seg_attr_max_size_create(max_attr_size, max_seg_size, TRUE);

	/* send PA-TNC message with the excl flag not set */
	out_msg = imc_msg_create(imc_swid, state, connection_id, imc_id,
							 TNC_IMVID_ANY, msg_types[0]);
	out_msg->add_attribute(out_msg, attr);
	result = out_msg->send(out_msg, FALSE);
	out_msg->destroy(out_msg);

	return result;
}

/**
 * Add one or multiple SWID Inventory attributes to the send queue
 */
static bool add_swid_inventory(imc_state_t *state, imc_msg_t *msg,
							   uint32_t request_id, bool full_tags,
							   swid_inventory_t *targets)
{
	pa_tnc_attr_t *attr, *attr_error;
	imc_swid_state_t *swid_state;
	swid_inventory_t *swid_inventory;
	char *swid_directory, *swid_generator;
	uint32_t eid_epoch;
	bool swid_pretty, swid_full;
	enumerator_t *enumerator;

	swid_directory = lib->settings->get_str(lib->settings,
								"%s.plugins.imc-swid.swid_directory",
								 SWID_DIRECTORY, lib->ns);
	swid_generator = lib->settings->get_str(lib->settings,
								"%s.plugins.imc-swid.swid_generator",
								 SWID_GENERATOR, lib->ns);
	swid_pretty = lib->settings->get_bool(lib->settings,
								"%s.plugins.imc-swid.swid_pretty",
								 FALSE, lib->ns);
	swid_full = lib->settings->get_bool(lib->settings,
								"%s.plugins.imc-swid.swid_full",
								 FALSE, lib->ns);

	swid_inventory = swid_inventory_create(full_tags);
	if (!swid_inventory->collect(swid_inventory, swid_directory, swid_generator,
								 targets, swid_pretty, swid_full))
	{
		swid_inventory->destroy(swid_inventory);
		attr_error = swid_error_create(TCG_SWID_ERROR, request_id,
								 0, "error in SWID tag collection");
		msg->add_attribute(msg, attr_error);
		return FALSE;
	}
	DBG1(DBG_IMC, "collected %d SWID tag%s%s",
		 swid_inventory->get_count(swid_inventory), full_tags ? "" : " ID",
		 swid_inventory->get_count(swid_inventory) == 1 ? "" : "s");

	swid_state = (imc_swid_state_t*)state;
	eid_epoch = swid_state->get_eid_epoch(swid_state);

	if (full_tags)
	{
		tcg_swid_attr_tag_inv_t *swid_attr;
		swid_tag_t *tag;

		/* Send a TCG SWID Tag Inventory attribute */
		attr = tcg_swid_attr_tag_inv_create(request_id, eid_epoch, 1);
		swid_attr = (tcg_swid_attr_tag_inv_t*)attr;

		enumerator = swid_inventory->create_enumerator(swid_inventory);
		while (enumerator->enumerate(enumerator, &tag))
		{
			swid_attr->add(swid_attr, tag->get_ref(tag));
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		tcg_swid_attr_tag_id_inv_t *swid_id_attr;
		swid_tag_id_t *tag_id;

		/* Send a TCG SWID Tag ID Inventory attribute */
		attr = tcg_swid_attr_tag_id_inv_create(request_id, eid_epoch, 1);
		swid_id_attr = (tcg_swid_attr_tag_id_inv_t*)attr;

		enumerator = swid_inventory->create_enumerator(swid_inventory);
		while (enumerator->enumerate(enumerator, &tag_id))
		{
			swid_id_attr->add(swid_id_attr, tag_id->get_ref(tag_id));
		}
		enumerator->destroy(enumerator);
	}

	msg->add_attribute(msg, attr);
	swid_inventory->destroy(swid_inventory);

	return TRUE;
}

static TNC_Result receive_message(imc_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	pa_tnc_attr_t *attr;
	enumerator_t *enumerator;
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
		tcg_swid_attr_req_t *attr_req;
		uint8_t flags;
		uint32_t request_id;
		bool full_tags;
		swid_inventory_t *targets;

		type = attr->get_type(attr);

		if (type.vendor_id != PEN_TCG || type.type != TCG_SWID_REQUEST)
		{
			continue;
		}

		attr_req = (tcg_swid_attr_req_t*)attr;
		flags = attr_req->get_flags(attr_req);
		request_id = attr_req->get_request_id(attr_req);
		targets = attr_req->get_targets(attr_req);

		if (flags & (TCG_SWID_ATTR_REQ_FLAG_S | TCG_SWID_ATTR_REQ_FLAG_C))
		{
			attr = swid_error_create(TCG_SWID_SUBSCRIPTION_DENIED, request_id,
									 0, "no subscription available yet");
			out_msg->add_attribute(out_msg, attr);
			break;
		}
		full_tags = (flags & TCG_SWID_ATTR_REQ_FLAG_R) == 0;

		if (!add_swid_inventory(state, out_msg, request_id, full_tags, targets))
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		result = TNC_RESULT_FATAL;
	}
	else
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
TNC_Result TNC_IMC_ReceiveMessage(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id,
								  TNC_BufferReference msg,
								  TNC_UInt32 msg_len,
								  TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swid->get_state(imc_swid, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_data(imc_swid, state, connection_id, msg_type,
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

	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swid->get_state(imc_swid, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_swid, state, connection_id,
								src_imv_id, dst_imc_id,msg_vid, msg_subtype,
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
	if (!imc_swid)
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
	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_swid->destroy(imc_swid);
	imc_swid = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
									   TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_swid)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_swid->bind_functions(imc_swid, bind_function);
}
