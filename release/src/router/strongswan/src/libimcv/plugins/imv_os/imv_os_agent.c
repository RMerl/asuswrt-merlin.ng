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

#define _GNU_SOURCE
#include <stdio.h>

#include "imv_os_agent.h"
#include "imv_os_state.h"
#include "imv_os_database.h"

#include <imcv.h>
#include <imv/imv_agent.h>
#include <imv/imv_msg.h>
#include <generic/generic_attr_bool.h>
#include <generic/generic_attr_string.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include <ietf/ietf_attr_installed_packages.h>
#include <ietf/ietf_attr_numeric_version.h>
#include <ietf/ietf_attr_op_status.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ietf/ietf_attr_product_info.h>
#include <ietf/ietf_attr_remediation_instr.h>
#include <ietf/ietf_attr_string_version.h>
#include <ita/ita_attr.h>
#include <ita/ita_attr_get_settings.h>
#include <ita/ita_attr_settings.h>
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

#define INSTALLED_PACKAGES_MAX_ATTR_SIZE	100000000

typedef struct private_imv_os_agent_t private_imv_os_agent_t;
typedef enum imv_os_attr_t imv_os_attr_t;

/* Subscribed PA-TNC message subtypes */
static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_OPERATING_SYSTEM }
};

static char unknown_source_str[] = "install_non_market_apps";

/**
 * Flag set when corresponding attribute has been received
 */
enum imv_os_attr_t {
	IMV_OS_ATTR_PRODUCT_INFORMATION =         (1<<0),
	IMV_OS_ATTR_STRING_VERSION =              (1<<1),
	IMV_OS_ATTR_NUMERIC_VERSION =             (1<<2),
	IMV_OS_ATTR_OPERATIONAL_STATUS =          (1<<3),
	IMV_OS_ATTR_FORWARDING_ENABLED =          (1<<4),
	IMV_OS_ATTR_FACTORY_DEFAULT_PWD_ENABLED = (1<<5),
	IMV_OS_ATTR_DEVICE_ID =                   (1<<6),
	IMV_OS_ATTR_MUST =                        (1<<7)-1,
	IMV_OS_ATTR_INSTALLED_PACKAGES =          (1<<7),
	IMV_OS_ATTR_SETTINGS =                    (1<<8)
};

/**
 * Private data of an imv_os_agent_t object.
 */
struct private_imv_os_agent_t {

	/**
	 * Public members of imv_os_agent_t
	 */
	imv_agent_if_t public;

	/**
	 * IMV agent responsible for generic functions
	 */
	imv_agent_t *agent;

	/**
	 * IMV OS database
	 */
	imv_os_database_t *db;

};

METHOD(imv_agent_if_t, bind_functions, TNC_Result,
	private_imv_os_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	return this->agent->bind_functions(this->agent, bind_function);
}

METHOD(imv_agent_if_t, notify_connection_change, TNC_Result,
	private_imv_os_agent_t *this, TNC_ConnectionID id,
	TNC_ConnectionState new_state)
{
	TNC_IMV_Action_Recommendation rec;
	imv_state_t *state;
	imv_session_t *session;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imv_os_state_create(id);
			return this->agent->create_state(this->agent, state);
		case TNC_CONNECTION_STATE_DELETE:
			return this->agent->delete_state(this->agent, id);
		case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			if (this->agent->get_state(this->agent, id, &state) && imcv_db)
			{
				session = state->get_session(state);

				if (session->get_policy_started(session))
				{
					switch (new_state)
					{
						case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
							rec = TNC_IMV_ACTION_RECOMMENDATION_ALLOW;
							break;
						case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
							rec = TNC_IMV_ACTION_RECOMMENDATION_ISOLATE;
							break;
						case TNC_CONNECTION_STATE_ACCESS_NONE:
						default:
							rec = TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS;
					}
					imcv_db->add_recommendation(imcv_db, session, rec);
					if (!imcv_db->policy_script(imcv_db, session, FALSE))
					{
						DBG1(DBG_IMV, "error in policy script stop");
					}
				}
			}
			/* fall through to default state */
		default:
			return this->agent->change_state(this->agent, id, new_state, NULL);
	}
}

/**
 * Process a received message
 */
static TNC_Result receive_msg(private_imv_os_agent_t *this, imv_state_t *state,
							  imv_msg_t *in_msg)
{
	imv_msg_t *out_msg;
	imv_os_state_t *os_state;
	imv_session_t *session;
	imv_os_info_t *os_info = NULL;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t type;
	TNC_Result result;
	chunk_t os_name = chunk_empty;
	chunk_t os_version = chunk_empty;
	bool fatal_error = FALSE, assessment = FALSE;
	uint16_t missing;

	os_state = (imv_os_state_t*)state;
	session = state->get_session(state);
	os_info = session->get_os_info(session);

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imv_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg,out_msg, &fatal_error);
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
			switch (type.type)
			{
				case IETF_ATTR_PRODUCT_INFORMATION:
				{
					ietf_attr_product_info_t *attr_cast;
					pen_t vendor_id;

					state->set_action_flags(state,
											IMV_OS_ATTR_PRODUCT_INFORMATION);
					attr_cast = (ietf_attr_product_info_t*)attr;
					os_name = attr_cast->get_info(attr_cast, &vendor_id, NULL);
					os_info->set_name(os_info, os_name);

					if (vendor_id != PEN_IETF)
					{
						DBG1(DBG_IMV, "operating system name is '%.*s' "
									  "from vendor %N", os_name.len, os_name.ptr,
									   pen_names, vendor_id);
					}
					else
					{
						DBG1(DBG_IMV, "operating system name is '%.*s'",
									   os_name.len, os_name.ptr);
					}
					break;
				}
				case IETF_ATTR_STRING_VERSION:
				{
					ietf_attr_string_version_t *attr_cast;

					state->set_action_flags(state,
											IMV_OS_ATTR_STRING_VERSION);
					attr_cast = (ietf_attr_string_version_t*)attr;
					os_version = attr_cast->get_version(attr_cast, NULL, NULL);
					os_info->set_version(os_info, os_version);

					if (os_version.len)
					{
						DBG1(DBG_IMV, "operating system version is '%.*s'",
									   os_version.len, os_version.ptr);
					}
					break;
				}
				case IETF_ATTR_NUMERIC_VERSION:
				{
					ietf_attr_numeric_version_t *attr_cast;
					uint32_t major, minor;

					state->set_action_flags(state,
											IMV_OS_ATTR_NUMERIC_VERSION);
					attr_cast = (ietf_attr_numeric_version_t*)attr;
					attr_cast->get_version(attr_cast, &major, &minor);
					DBG1(DBG_IMV, "operating system numeric version is %d.%d",
								   major, minor);
					break;
				}
				case IETF_ATTR_OPERATIONAL_STATUS:
				{
					ietf_attr_op_status_t *attr_cast;
					op_status_t op_status;
					op_result_t op_result;
					time_t last_boot;

					state->set_action_flags(state,
											IMV_OS_ATTR_OPERATIONAL_STATUS);
					attr_cast = (ietf_attr_op_status_t*)attr;
					op_status = attr_cast->get_status(attr_cast);
					op_result = attr_cast->get_result(attr_cast);
					last_boot = attr_cast->get_last_use(attr_cast);
					DBG1(DBG_IMV, "operational status: %N, result: %N",
						 op_status_names, op_status, op_result_names, op_result);
					DBG1(DBG_IMV, "last boot: %T", &last_boot, TRUE);
					break;
				}
				case IETF_ATTR_FORWARDING_ENABLED:
				{
					generic_attr_bool_t *attr_cast;
					os_fwd_status_t fwd_status;

					state->set_action_flags(state,
											IMV_OS_ATTR_FORWARDING_ENABLED);
					attr_cast = (generic_attr_bool_t*)attr;
					fwd_status = attr_cast->get_status(attr_cast);
					DBG1(DBG_IMV, "IPv4 forwarding is %N",
								   os_fwd_status_names, fwd_status);
					if (fwd_status == OS_FWD_ENABLED)
					{
						os_state->set_os_settings(os_state,
											OS_SETTINGS_FWD_ENABLED);
					}
					break;
				}
				case IETF_ATTR_FACTORY_DEFAULT_PWD_ENABLED:
				{
					generic_attr_bool_t *attr_cast;
					bool default_pwd_status;

					state->set_action_flags(state,
									IMV_OS_ATTR_FACTORY_DEFAULT_PWD_ENABLED);
					attr_cast = (generic_attr_bool_t*)attr;
					default_pwd_status = attr_cast->get_status(attr_cast);
					DBG1(DBG_IMV, "factory default password is %sabled",
								   default_pwd_status ? "en":"dis");
					if (default_pwd_status)
					{
						os_state->set_os_settings(os_state,
											OS_SETTINGS_DEFAULT_PWD_ENABLED);
					}
					break;
				}
				case IETF_ATTR_INSTALLED_PACKAGES:
				{
					ietf_attr_installed_packages_t *attr_cast;
					enumerator_t *e;
					status_t status;

					state->set_action_flags(state,
											IMV_OS_ATTR_INSTALLED_PACKAGES);
					if (!this->db)
					{
						break;
					}
					attr_cast = (ietf_attr_installed_packages_t*)attr;

					e = attr_cast->create_enumerator(attr_cast);
					status = this->db->check_packages(this->db, os_state, e);
					e->destroy(e);

					if (status == FAILED)
					{
						state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
								TNC_IMV_EVALUATION_RESULT_ERROR);
						assessment = TRUE;
					}
					missing = attr_cast->get_count(attr_cast);
					os_state->set_missing(os_state, missing);
					attr_cast->clear_packages(attr_cast);
					break;
				}
				default:
					break;
			}
		}
		else if (type.vendor_id == PEN_ITA)
		{
			switch (type.type)
			{
				case ITA_ATTR_SETTINGS:
				{
					ita_attr_settings_t *attr_cast;
					enumerator_t *e;
					char *name;
					chunk_t value;

					state->set_action_flags(state, IMV_OS_ATTR_SETTINGS);

					attr_cast = (ita_attr_settings_t*)attr;
					e = attr_cast->create_enumerator(attr_cast);
					while (e->enumerate(e, &name, &value))
					{
						if (streq(name, unknown_source_str) &&
							chunk_equals(value, chunk_from_chars('1')))
						{
							os_state->set_os_settings(os_state,
												OS_SETTINGS_UNKNOWN_SOURCE);
						}
						DBG1(DBG_IMV, "setting '%s'\n  %.*s",
							 name, value.len, value.ptr);
					}
					e->destroy(e);
					break;
				}
				case ITA_ATTR_DEVICE_ID:
				{
					chunk_t value;

					state->set_action_flags(state, IMV_OS_ATTR_DEVICE_ID);

					value = attr->get_value(attr);
					DBG1(DBG_IMV, "device ID is %.*s", value.len, value.ptr);
					session->set_device_id(session, value);
					break;
				}
				default:
					break;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
								TNC_IMV_EVALUATION_RESULT_ERROR);
		assessment = TRUE;
	}

	if (assessment)
	{
		os_state->set_handshake_state(os_state, IMV_OS_STATE_END);
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
	private_imv_os_agent_t *this, TNC_ConnectionID id,
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
	private_imv_os_agent_t *this, TNC_ConnectionID id,
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

/**
 * Build an IETF Attribute Request attribute for missing attributes
 */
static pa_tnc_attr_t* build_attr_request(uint32_t received)
{
	pa_tnc_attr_t *attr;
	ietf_attr_attr_request_t *attr_cast;

	attr = ietf_attr_attr_request_create(PEN_RESERVED, 0);
	attr_cast = (ietf_attr_attr_request_t*)attr;

	if (!(received & IMV_OS_ATTR_PRODUCT_INFORMATION) ||
		!(received & IMV_OS_ATTR_STRING_VERSION))
	{
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_PRODUCT_INFORMATION);
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_STRING_VERSION);
	}
	if (!(received & IMV_OS_ATTR_NUMERIC_VERSION))
	{
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_NUMERIC_VERSION);
	}
	if (!(received & IMV_OS_ATTR_OPERATIONAL_STATUS))
	{
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_OPERATIONAL_STATUS);
	}
	if (!(received & IMV_OS_ATTR_FORWARDING_ENABLED))
	{
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_FORWARDING_ENABLED);
	}
	if (!(received & IMV_OS_ATTR_FACTORY_DEFAULT_PWD_ENABLED))
	{
		attr_cast->add(attr_cast, PEN_IETF,
								  IETF_ATTR_FACTORY_DEFAULT_PWD_ENABLED);
	}
	if (!(received & IMV_OS_ATTR_DEVICE_ID))
	{
		attr_cast->add(attr_cast, PEN_ITA,  ITA_ATTR_DEVICE_ID);
	}

	return attr;
}

METHOD(imv_agent_if_t, batch_ending, TNC_Result,
	private_imv_os_agent_t *this, TNC_ConnectionID id)
{
	imv_msg_t *out_msg;
	imv_state_t *state;
	imv_session_t *session;
	imv_workitem_t *workitem;
	imv_os_state_t *os_state;
	imv_os_handshake_state_t handshake_state;
	pa_tnc_attr_t *attr;
	TNC_IMVID imv_id;
	TNC_Result result = TNC_RESULT_SUCCESS;
	bool no_workitems = TRUE;
	enumerator_t *enumerator;
	uint32_t received;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	os_state = (imv_os_state_t*)state;
	handshake_state = os_state->get_handshake_state(os_state);
	received = state->get_action_flags(state);
	session = state->get_session(state);
	imv_id = this->agent->get_id(this->agent);

	if (handshake_state == IMV_OS_STATE_END)
	{
		return TNC_RESULT_SUCCESS;
	}

	/* create an empty out message - we might need it */
	out_msg = imv_msg_create(this->agent, state, id, imv_id, TNC_IMCID_ANY,
							 msg_types[0]);

	if (handshake_state == IMV_OS_STATE_INIT)
	{
		size_t max_attr_size = INSTALLED_PACKAGES_MAX_ATTR_SIZE;
		size_t max_seg_size;
		seg_contract_t *contract;
		seg_contract_manager_t *contracts;
		char buf[BUF_LEN];

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
		attr = tcg_seg_attr_max_size_create(max_attr_size, max_seg_size, TRUE);
		out_msg->add_attribute(out_msg, attr);

		if ((received & IMV_OS_ATTR_MUST) != IMV_OS_ATTR_MUST)
		{
			/* create attribute request for missing mandatory attributes */
			out_msg->add_attribute(out_msg, build_attr_request(received));
		}
	}

	if (handshake_state < IMV_OS_STATE_POLICY_START)
	{
		if (session->get_policy_started(session))
		{
			/* the policy script has already been started by another IMV */
			handshake_state = IMV_OS_STATE_POLICY_START;
		}
		else
		{
			if (((received & IMV_OS_ATTR_PRODUCT_INFORMATION) &&
				 (received & IMV_OS_ATTR_STRING_VERSION)) &&
				((received & IMV_OS_ATTR_DEVICE_ID) ||
				 (handshake_state == IMV_OS_STATE_ATTR_REQ)))
			{
				if (!session->get_device_id(session, NULL))
				{
					session->set_device_id(session, chunk_empty);
				}
				if (imcv_db)
				{
					/* start the policy script */
					if (!imcv_db->policy_script(imcv_db, session, TRUE))
					{
						DBG1(DBG_IMV, "error in policy script start");
					}
				}
				else
				{
					DBG2(DBG_IMV, "no workitems available - "
								  "no evaluation possible");
					state->set_recommendation(state,
									TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
									TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
					session->set_policy_started(session, TRUE);
				}
				handshake_state = IMV_OS_STATE_POLICY_START;
			}
			else if (handshake_state == IMV_OS_STATE_ATTR_REQ)
			{
				/**
				 * both the IETF Product Information and IETF String Version
				 * attribute should have been present
				 */
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
								TNC_IMV_EVALUATION_RESULT_ERROR);

				/* send assessment */
				result = out_msg->send_assessment(out_msg);
				out_msg->destroy(out_msg);

				if (result != TNC_RESULT_SUCCESS)
				{
					return result;
				}
				return this->agent->provide_recommendation(this->agent, state);
			}
			else
			{
				handshake_state = IMV_OS_STATE_ATTR_REQ;
			}
		}
		os_state->set_handshake_state(os_state, handshake_state);
	}

	if (handshake_state == IMV_OS_STATE_POLICY_START)
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
					case IMV_WORKITEM_PACKAGES:
						attr = ietf_attr_attr_request_create(PEN_IETF,
										IETF_ATTR_INSTALLED_PACKAGES);
						out_msg->add_attribute(out_msg, attr);
						break;
					case IMV_WORKITEM_UNKNOWN_SOURCE:
						attr = ita_attr_get_settings_create(unknown_source_str);
						out_msg->add_attribute(out_msg, attr);
						break;
					case IMV_WORKITEM_FORWARDING:
					case IMV_WORKITEM_DEFAULT_PWD:
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
			handshake_state = IMV_OS_STATE_WORKITEMS;
			os_state->set_handshake_state(os_state, handshake_state);
		}
	}

	if (handshake_state == IMV_OS_STATE_WORKITEMS)
	{
		TNC_IMV_Evaluation_Result eval;
		TNC_IMV_Action_Recommendation rec;
		char result_str[BUF_LEN];
		bool fail;

		enumerator = session->create_workitem_enumerator(session);
		while (enumerator->enumerate(enumerator, &workitem))
		{
			if (workitem->get_imv_id(workitem) != imv_id)
			{
				continue;
			}

			switch (workitem->get_type(workitem))
			{
				case IMV_WORKITEM_PACKAGES:
				{
					int count, count_update, count_blacklist, count_ok;

					if (!(received & IMV_OS_ATTR_INSTALLED_PACKAGES) ||
						os_state->get_missing(os_state) > 0)
					{
						continue;
					}
					os_state->get_count(os_state, &count, &count_update,
										&count_blacklist, &count_ok);
					fail = count_update || count_blacklist;
					eval = fail ? TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR :
								  TNC_IMV_EVALUATION_RESULT_COMPLIANT;
					snprintf(result_str, BUF_LEN, "processed %d packages: "
							"%d vulnerable, %d blacklisted, %d ok, %d unknown",
							count, count_update, count_blacklist, count_ok,
							count - count_update - count_blacklist - count_ok);
					break;
				}
				case IMV_WORKITEM_UNKNOWN_SOURCE:
					if (!(received & IMV_OS_ATTR_SETTINGS))
					{
						continue;
					}
					fail = os_state->get_os_settings(os_state) &
								OS_SETTINGS_UNKNOWN_SOURCE;
					eval = fail ? TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR :
								  TNC_IMV_EVALUATION_RESULT_COMPLIANT;
					snprintf(result_str, BUF_LEN, "unknown sources%s enabled",
							 fail ? "" : " not");
					break;
				case IMV_WORKITEM_FORWARDING:
					if (!(received & IMV_OS_ATTR_FORWARDING_ENABLED))
					{
						continue;
					}
					fail = os_state->get_os_settings(os_state) &
								OS_SETTINGS_FWD_ENABLED;
					eval = fail ? TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR :
								  TNC_IMV_EVALUATION_RESULT_COMPLIANT;
					snprintf(result_str, BUF_LEN, "forwarding%s enabled",
							 fail ? "" : " not");
					break;
				case IMV_WORKITEM_DEFAULT_PWD:
					if (!(received & IMV_OS_ATTR_FACTORY_DEFAULT_PWD_ENABLED))
					{
						continue;
					}
					fail = os_state->get_os_settings(os_state) &
								OS_SETTINGS_DEFAULT_PWD_ENABLED;
					eval = fail ? TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR :
								  TNC_IMV_EVALUATION_RESULT_COMPLIANT;
					snprintf(result_str, BUF_LEN, "factory default password%s enabled",
							 fail ? "" : " not");
					break;
				default:
					continue;
			}
			session->remove_workitem(session, enumerator);
			rec = workitem->set_result(workitem, result_str, eval);
			state->update_recommendation(state, rec, eval);
			imcv_db->finalize_workitem(imcv_db, workitem);
			workitem->destroy(workitem);
		}
		enumerator->destroy(enumerator);

		/* finalized all workitems ? */
		if (session->get_workitem_count(session, imv_id) == 0)
		{
			os_state->set_handshake_state(os_state, IMV_OS_STATE_END);

			result = out_msg->send_assessment(out_msg);
			out_msg->destroy(out_msg);
			if (result != TNC_RESULT_SUCCESS)
			{
				return result;
			}
			return this->agent->provide_recommendation(this->agent, state);
		}
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
	private_imv_os_agent_t *this, TNC_ConnectionID id)
{
	imv_state_t *state;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	return this->agent->provide_recommendation(this->agent, state);
}

METHOD(imv_agent_if_t, destroy, void,
	private_imv_os_agent_t *this)
{
	DESTROY_IF(this->agent);
	DESTROY_IF(this->db);
	free(this);
}

/**
 * Described in header.
 */
imv_agent_if_t *imv_os_agent_create(const char *name, TNC_IMVID id,
									TNC_Version *actual_version)
{
	private_imv_os_agent_t *this;
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
		.db = imv_os_database_create(imcv_db),
	);

	return &this->public;
}

