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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "imv_attestation_agent.h"
#include "imv_attestation_state.h"
#include "imv_attestation_process.h"
#include "imv_attestation_build.h"

#include <imcv.h>
#include <imv/imv_agent.h>
#include <imv/imv_msg.h>
#include <imv/imv_session.h>
#include <imv/imv_os_info.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include <ietf/ietf_attr_pa_tnc_error.h>
#include <ietf/ietf_attr_product_info.h>
#include <ietf/ietf_attr_string_version.h>
#include <ita/ita_attr.h>
#include <ita/ita_attr_device_id.h>
#include <tcg/tcg_attr.h>
#include <tcg/pts/tcg_pts_attr_meas_algo.h>
#include <tcg/pts/tcg_pts_attr_proto_caps.h>
#include <tcg/pts/tcg_pts_attr_req_file_meas.h>
#include <tcg/pts/tcg_pts_attr_req_file_meta.h>
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"
#include <pts/pts.h>
#include <pts/pts_database.h>
#include <pts/pts_creds.h>
#include <pts/components/ita/ita_comp_func_name.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>
#include <credentials/credential_manager.h>
#include <collections/linked_list.h>

#define FILE_MEAS_MAX_ATTR_SIZE	100000000

typedef struct private_imv_attestation_agent_t private_imv_attestation_agent_t;

/* Subscribed PA-TNC message subtypes */
static pen_type_t msg_types[] = {
	{ PEN_TCG,  PA_SUBTYPE_TCG_PTS },
	{ PEN_IETF, PA_SUBTYPE_IETF_OPERATING_SYSTEM }
};

/**
 * Private data of an imv_attestation_agent_t object.
 */
struct private_imv_attestation_agent_t {

	/**
	 * Public members of imv_attestation_agent_t
	 */
	imv_agent_if_t public;

	/**
	 * IMV agent responsible for generic functions
	 */
	imv_agent_t *agent;

	/**
	 * Supported PTS measurement algorithms
	 */
	pts_meas_algorithms_t supported_algorithms;

	/**
	 * Supported PTS Diffie Hellman Groups
	 */
	pts_dh_group_t supported_dh_groups;

	/**
	 * PTS file measurement database
	 */
	pts_database_t *pts_db;

	/**
	 * PTS credentials
	 */
	pts_creds_t *pts_creds;

	/**
	 * PTS credential manager
	 */
	credential_manager_t *pts_credmgr;

};

METHOD(imv_agent_if_t, bind_functions, TNC_Result,
	private_imv_attestation_agent_t *this, TNC_TNCS_BindFunctionPointer bind_function)
{
	return this->agent->bind_functions(this->agent, bind_function);
}

METHOD(imv_agent_if_t, notify_connection_change, TNC_Result,
	private_imv_attestation_agent_t *this, TNC_ConnectionID id,
	TNC_ConnectionState new_state)
{
	TNC_IMV_Action_Recommendation rec;
	imv_state_t *state;
	imv_session_t *session;

	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imv_attestation_state_create(id);
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
static TNC_Result receive_msg(private_imv_attestation_agent_t *this,
							  imv_state_t *state, imv_msg_t *in_msg)
{
	imv_msg_t *out_msg;
	imv_session_t *session;
	imv_os_info_t *os_info;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t type;
	TNC_Result result;
	chunk_t os_name, os_version;
	bool fatal_error = FALSE;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imv_msg_create_as_reply(in_msg);
	out_msg->set_msg_type(out_msg, msg_types[0]);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}

	session = state->get_session(state);
	os_info = session->get_os_info(session);

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		type = attr->get_type(attr);

		if (type.vendor_id == PEN_IETF)
		{
			switch (type.type)
			{
				case IETF_ATTR_PA_TNC_ERROR:
				{
					ietf_attr_pa_tnc_error_t *error_attr;
					pen_type_t error_code;
					chunk_t msg_info;

					error_attr = (ietf_attr_pa_tnc_error_t*)attr;
					error_code = error_attr->get_error_code(error_attr);

					if (error_code.vendor_id == PEN_TCG)
					{
						msg_info = error_attr->get_msg_info(error_attr);

						DBG1(DBG_IMV, "received TCG-PTS error '%N'",
							 pts_error_code_names, error_code.type);
						DBG1(DBG_IMV, "error information: %B", &msg_info);
						fatal_error = TRUE;
					}
					break;
				}
				case IETF_ATTR_PRODUCT_INFORMATION:
				{
					ietf_attr_product_info_t *attr_cast;
					pen_t vendor_id;

					state->set_action_flags(state,
										IMV_ATTESTATION_ATTR_PRODUCT_INFO);
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

					break;
				}
				case IETF_ATTR_STRING_VERSION:
				{
					ietf_attr_string_version_t *attr_cast;

					state->set_action_flags(state,
										IMV_ATTESTATION_ATTR_STRING_VERSION);
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
				default:
					break;
			}
		}
		else if (type.vendor_id == PEN_ITA)
		{
			switch (type.type)
			{
				case ITA_ATTR_DEVICE_ID:
				{
					chunk_t value;

					state->set_action_flags(state,
										IMV_ATTESTATION_ATTR_DEVICE_ID);

					value = attr->get_value(attr);
					DBG1(DBG_IMV, "device ID is %.*s", value.len, value.ptr);
					session->set_device_id(session, value);
					break;
				}
				default:
					break;
			}
		}
		else if (type.vendor_id == PEN_TCG)
		{
			if (!imv_attestation_process(attr, out_msg, state,
				this->supported_algorithms, this->supported_dh_groups,
				this->pts_db, this->pts_credmgr))
			{
				result = TNC_RESULT_FATAL;
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error || result != TNC_RESULT_SUCCESS)
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
	private_imv_attestation_agent_t *this, TNC_ConnectionID id,
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
	private_imv_attestation_agent_t *this, TNC_ConnectionID id,
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

	if (!(received & IMV_ATTESTATION_ATTR_PRODUCT_INFO) ||
		!(received & IMV_ATTESTATION_ATTR_STRING_VERSION))
	{
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_PRODUCT_INFORMATION);
		attr_cast->add(attr_cast, PEN_IETF, IETF_ATTR_STRING_VERSION);
	}
	if (!(received & IMV_ATTESTATION_ATTR_DEVICE_ID))
	{
		attr_cast->add(attr_cast, PEN_ITA,  ITA_ATTR_DEVICE_ID);
	}

	return attr;
}

METHOD(imv_agent_if_t, batch_ending, TNC_Result,
	private_imv_attestation_agent_t *this, TNC_ConnectionID id)
{
	imv_msg_t *out_msg;
	imv_state_t *state;
	imv_session_t *session;
	imv_attestation_state_t *attestation_state;
	imv_attestation_handshake_state_t handshake_state;
	imv_workitem_t *workitem;
	TNC_IMV_Action_Recommendation rec;
	TNC_IMV_Evaluation_Result eval;
	TNC_IMVID imv_id;
	TNC_Result result = TNC_RESULT_SUCCESS;
	pts_t *pts;
	int pid;
	uint32_t actions;
	enumerator_t *enumerator;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	attestation_state = (imv_attestation_state_t*)state;
	pts = attestation_state->get_pts(attestation_state);
	handshake_state = attestation_state->get_handshake_state(attestation_state);
	actions = state->get_action_flags(state);
	session = state->get_session(state);
	imv_id = this->agent->get_id(this->agent);

	/* exit if a recommendation has already been provided */
	if (actions & IMV_ATTESTATION_REC)
	{
		return TNC_RESULT_SUCCESS;
	}

	/* send an IETF attribute request if no platform info was received */
	if (!(actions & IMV_ATTESTATION_ATTR_REQ))
	{
		if ((actions & IMV_ATTESTATION_ATTR_MUST) != IMV_ATTESTATION_ATTR_MUST)
		{
			imv_msg_t *os_msg;

			/* create attribute request for missing mandatory attributes */
			os_msg = imv_msg_create(this->agent, state, id, imv_id,
									TNC_IMCID_ANY, msg_types[1]);
			os_msg->add_attribute(os_msg, build_attr_request(actions));
			result = os_msg->send(os_msg, FALSE);
			os_msg->destroy(os_msg);

			if (result != TNC_RESULT_SUCCESS)
			{
				return result;
			}
		 }
		state->set_action_flags(state, IMV_ATTESTATION_ATTR_REQ);
	}

	if (!session->get_policy_started(session) &&
		(actions & IMV_ATTESTATION_ATTR_PRODUCT_INFO) &&
		(actions & IMV_ATTESTATION_ATTR_STRING_VERSION) &&
		(actions & IMV_ATTESTATION_ATTR_DEVICE_ID))
	{
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
			DBG2(DBG_IMV, "no workitems available - no evaluation possible");
			state->set_recommendation(state,
									  TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
									  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			session->set_policy_started(session, TRUE);
		}
	}

	if (handshake_state == IMV_ATTESTATION_STATE_INIT)
	{
		size_t max_attr_size = FILE_MEAS_MAX_ATTR_SIZE;
		size_t max_seg_size;
		seg_contract_t *contract;
		seg_contract_manager_t *contracts;
		pa_tnc_attr_t *attr;
		pts_proto_caps_flag_t flags;
		char buf[BUF_LEN];

		out_msg = imv_msg_create(this->agent, state, id, imv_id, TNC_IMCID_ANY,
								 msg_types[0]);

		/* Determine maximum PA-TNC attribute segment size */
		max_seg_size = state->get_max_msg_len(state)
								- PA_TNC_HEADER_SIZE
								- PA_TNC_ATTR_HEADER_SIZE
								- TCG_SEG_ATTR_SEG_ENV_HEADER
								- PA_TNC_ATTR_HEADER_SIZE
								- TCG_SEG_ATTR_MAX_SIZE_SIZE;

		/* Announce support of PA-TNC segmentation to IMC */
		contract = seg_contract_create(msg_types[0], max_attr_size,
										max_seg_size, TRUE, imv_id, FALSE);
		contract->get_info_string(contract, buf, BUF_LEN, TRUE);
		DBG2(DBG_IMV, "%s", buf);
		contracts = state->get_contracts(state);
		contracts->add_contract(contracts, contract);
		attr = tcg_seg_attr_max_size_create(max_attr_size, max_seg_size, TRUE);
		out_msg->add_attribute(out_msg, attr);

		/* Send Request Protocol Capabilities attribute */
		flags = pts->get_proto_caps(pts);
		attr = tcg_pts_attr_proto_caps_create(flags, TRUE);
		attr->set_noskip_flag(attr, TRUE);
		out_msg->add_attribute(out_msg, attr);

		/* Send Measurement Algorithms attribute */
		attr = tcg_pts_attr_meas_algo_create(this->supported_algorithms, FALSE);
		attr->set_noskip_flag(attr, TRUE);
		out_msg->add_attribute(out_msg, attr);

		attestation_state->set_handshake_state(attestation_state,
										IMV_ATTESTATION_STATE_DISCOVERY);

		/* send these initial PTS attributes and exit */
		result = out_msg->send(out_msg, FALSE);
		out_msg->destroy(out_msg);

		return result;
	}

	/* exit if we are not ready yet for PTS measurements */
	if (!(actions & IMV_ATTESTATION_ALGO))
	{
		return TNC_RESULT_SUCCESS;
	}

	session->get_session_id(session, &pid, NULL);
	pts->set_platform_id(pts, pid);

	/* create an empty out message - we might need it */
	out_msg = imv_msg_create(this->agent, state, id, imv_id, TNC_IMCID_ANY,
							 msg_types[0]);

	/* establish the PTS measurements to be taken */
	if (!(actions & IMV_ATTESTATION_FILE_MEAS))
	{
		bool is_dir, no_workitems = TRUE;
		uint32_t delimiter = SOLIDUS_UTF;
		uint16_t request_id;
		pa_tnc_attr_t *attr;
		char *pathname;

		attestation_state->set_handshake_state(attestation_state,
											   IMV_ATTESTATION_STATE_END);

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
					case IMV_WORKITEM_FILE_REF_MEAS:
					case IMV_WORKITEM_FILE_MEAS:
					case IMV_WORKITEM_FILE_META:
						is_dir = FALSE;
						break;
					case IMV_WORKITEM_DIR_REF_MEAS:
					case IMV_WORKITEM_DIR_MEAS:
					case IMV_WORKITEM_DIR_META:
						is_dir = TRUE;
						break;
					case IMV_WORKITEM_TPM_ATTEST:
					{
						pts_component_t *comp;
						pts_comp_func_name_t *comp_name;
						bool no_d_flag, no_t_flag;
						char result_str[BUF_LEN];

						workitem->set_imv_id(workitem, imv_id);
						no_workitems = FALSE;
						no_d_flag = !(pts->get_proto_caps(pts) & PTS_PROTO_CAPS_D);
						no_t_flag = !(pts->get_proto_caps(pts) & PTS_PROTO_CAPS_T);
						if (no_d_flag || no_t_flag)
						{
							snprintf(result_str, BUF_LEN, "%s%s%s",
									(no_t_flag) ? "no TPM available" : "",
									(no_t_flag && no_d_flag) ? ", " : "",
									(no_d_flag) ? "no DH nonce negotiation" : "");
							eval = TNC_IMV_EVALUATION_RESULT_ERROR;
							session->remove_workitem(session, enumerator);
							rec = workitem->set_result(workitem, result_str, eval);
							state->update_recommendation(state, rec, eval);
							imcv_db->finalize_workitem(imcv_db, workitem);
							workitem->destroy(workitem);
							continue;
						}

						/* do TPM BIOS measurements */
						if (strchr(workitem->get_arg_str(workitem), 'B'))
						{
							comp_name = pts_comp_func_name_create(PEN_ITA,
											PTS_ITA_COMP_FUNC_NAME_IMA,
											PTS_ITA_QUALIFIER_FLAG_KERNEL |
											PTS_ITA_QUALIFIER_TYPE_TRUSTED);
							comp = attestation_state->create_component(
											attestation_state, comp_name,
											0, this->pts_db);
							if (!comp)
							{
								comp_name->log(comp_name, "unregistered ");
								comp_name->destroy(comp_name);
							}
						}

						/* do TPM IMA measurements */
						if (strchr(workitem->get_arg_str(workitem), 'I'))
						{
							comp_name = pts_comp_func_name_create(PEN_ITA,
											PTS_ITA_COMP_FUNC_NAME_IMA,
											PTS_ITA_QUALIFIER_FLAG_KERNEL |
											PTS_ITA_QUALIFIER_TYPE_OS);
							comp = attestation_state->create_component(
											attestation_state, comp_name,
											0, this->pts_db);
							if (!comp)
							{
								comp_name->log(comp_name, "unregistered ");
								comp_name->destroy(comp_name);
							}
						}

						/* do TPM TRUSTED BOOT measurements */
						if (strchr(workitem->get_arg_str(workitem), 'T'))
						{
							comp_name = pts_comp_func_name_create(PEN_ITA,
											 PTS_ITA_COMP_FUNC_NAME_TBOOT,
											PTS_ITA_QUALIFIER_FLAG_KERNEL |
											PTS_ITA_QUALIFIER_TYPE_TRUSTED);
							comp = attestation_state->create_component(
											attestation_state, comp_name,
											0, this->pts_db);
							if (!comp)
							{
								comp_name->log(comp_name, "unregistered ");
								comp_name->destroy(comp_name);
							}
						}
						attestation_state->set_handshake_state(attestation_state,
											IMV_ATTESTATION_STATE_NONCE_REQ);
						continue;
					}
					default:
						continue;
				}

				/* initiate file and directory measurements */
				pathname = this->pts_db->get_pathname(this->pts_db, is_dir,
											workitem->get_arg_int(workitem));
				if (!pathname)
				{
					continue;
				}
				workitem->set_imv_id(workitem, imv_id);
				no_workitems = FALSE;

				if (workitem->get_type(workitem) == IMV_WORKITEM_FILE_META)
				{
					TNC_IMV_Action_Recommendation rec;
					TNC_IMV_Evaluation_Result eval;
					char result_str[BUF_LEN];

					DBG2(DBG_IMV, "IMV %d requests metadata for %s '%s'",
						 imv_id, is_dir ? "directory" : "file", pathname);

					/* currently just fire and forget metadata requests */
					attr = tcg_pts_attr_req_file_meta_create(is_dir,
												delimiter, pathname);
					snprintf(result_str, BUF_LEN, "%s metadata requested",
							 is_dir ? "directory" : "file");
					eval = TNC_IMV_EVALUATION_RESULT_COMPLIANT;
					session->remove_workitem(session, enumerator);
					rec = workitem->set_result(workitem, result_str, eval);
					state->update_recommendation(state, rec, eval);
					imcv_db->finalize_workitem(imcv_db, workitem);
					workitem->destroy(workitem);
				}
				else
				{
					/* use lower 16 bits of the workitem ID as request ID */
					request_id = workitem->get_id(workitem) & 0xffff;

					DBG2(DBG_IMV, "IMV %d requests measurement %d for %s '%s'",
						 imv_id, request_id, is_dir ? "directory" : "file",
						 pathname);
					attr = tcg_pts_attr_req_file_meas_create(is_dir, request_id,
												delimiter, pathname);
				}
				free(pathname);
				attr->set_noskip_flag(attr, TRUE);
				out_msg->add_attribute(out_msg, attr);
			}
			enumerator->destroy(enumerator);

			/* sent all file and directory measurement and metadata requests */
			state->set_action_flags(state, IMV_ATTESTATION_FILE_MEAS);

			if (no_workitems)
			{
				DBG2(DBG_IMV, "IMV %d has no workitems - "
							  "no evaluation requested", imv_id);
				state->set_recommendation(state,
								TNC_IMV_ACTION_RECOMMENDATION_ALLOW,
								TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			}
		}
	}

	/* check the IMV state for the next PA-TNC attributes to send */
	enumerator = session->create_workitem_enumerator(session);
	while (enumerator->enumerate(enumerator, &workitem))
	{
		if (workitem->get_type(workitem) == IMV_WORKITEM_TPM_ATTEST)
		{
			if (!imv_attestation_build(out_msg, state,
									   this->supported_dh_groups, this->pts_db))
			{
				imv_reason_string_t *reason_string;
				chunk_t result;
				char *result_str;

				reason_string = imv_reason_string_create("en", ", ");
				attestation_state->add_comp_evid_reasons(attestation_state,
													 reason_string);
				result = reason_string->get_encoding(reason_string);
				result_str = strndup(result.ptr, result.len);
				reason_string->destroy(reason_string);

				eval = TNC_IMV_EVALUATION_RESULT_ERROR;
				session->remove_workitem(session, enumerator);
				rec = workitem->set_result(workitem, result_str, eval);
				state->update_recommendation(state, rec, eval);
				imcv_db->finalize_workitem(imcv_db, workitem);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* finalized all workitems? */
	if (session->get_policy_started(session) &&
		session->get_workitem_count(session, imv_id) == 0 &&
		attestation_state->get_handshake_state(attestation_state) ==
			IMV_ATTESTATION_STATE_END)
	{
		result = out_msg->send_assessment(out_msg);
		out_msg->destroy(out_msg);
		state->set_action_flags(state, IMV_ATTESTATION_REC);

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
	private_imv_attestation_agent_t *this, TNC_ConnectionID id)
{
	TNC_IMVID imv_id;
	imv_state_t *state;
	imv_attestation_state_t *attestation_state;
	imv_session_t *session;

	if (!this->agent->get_state(this->agent, id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	attestation_state = (imv_attestation_state_t*)state;
	session = state->get_session(state);
	imv_id = this->agent->get_id(this->agent);

	if (imcv_db)
	{
		TNC_IMV_Evaluation_Result eval;
		TNC_IMV_Action_Recommendation rec;
		imv_workitem_t *workitem;
		enumerator_t *enumerator;
		int pending_file_meas = 0;
		char *result_str;
		chunk_t result_buf;
		bio_writer_t *result;

		enumerator = session->create_workitem_enumerator(session);
		if (enumerator)
		{
			while (enumerator->enumerate(enumerator, &workitem))
			{
				if (workitem->get_imv_id(workitem) != imv_id)
				{
					continue;
				}
				result = bio_writer_create(128);

				switch (workitem->get_type(workitem))
				{
					case IMV_WORKITEM_FILE_REF_MEAS:
					case IMV_WORKITEM_FILE_MEAS:
					case IMV_WORKITEM_DIR_REF_MEAS:
					case IMV_WORKITEM_DIR_MEAS:
						result_str = "pending file measurements";
						pending_file_meas++;
						break;
					case IMV_WORKITEM_TPM_ATTEST:
						attestation_state->finalize_components(attestation_state,
															   result);
						result->write_data(result,
								chunk_from_str("; pending component evidence"));
						result->write_uint8(result, '\0');
						result_buf = result->get_buf(result);
						result_str = result_buf.ptr;
						break;
					default:
						result->destroy(result);
						continue;
				}
				session->remove_workitem(session, enumerator);
				eval = TNC_IMV_EVALUATION_RESULT_ERROR;
				rec = workitem->set_result(workitem, result_str, eval);
				state->update_recommendation(state, rec, eval);
				imcv_db->finalize_workitem(imcv_db, workitem);
				workitem->destroy(workitem);
				result->destroy(result);
			}
			enumerator->destroy(enumerator);

			if (pending_file_meas)
			{
				DBG1(DBG_IMV, "failure due to %d pending file measurements",
							   pending_file_meas);
				attestation_state->set_measurement_error(attestation_state,
							   IMV_ATTESTATION_ERROR_FILE_MEAS_PEND);
			}
		}
	}
	return this->agent->provide_recommendation(this->agent, state);
}

METHOD(imv_agent_if_t, destroy, void,
	private_imv_attestation_agent_t *this)
{
	if (this->pts_creds)
	{
		this->pts_credmgr->remove_set(this->pts_credmgr,
						 			  this->pts_creds->get_set(this->pts_creds));
		this->pts_creds->destroy(this->pts_creds);
	}
	DESTROY_IF(this->pts_db);
	DESTROY_IF(this->pts_credmgr);
	DESTROY_IF(this->agent);
	free(this);
}

/**
 * Described in header.
 */
imv_agent_if_t *imv_attestation_agent_create(const char *name, TNC_IMVID id,
										 TNC_Version *actual_version)
{
	private_imv_attestation_agent_t *this;
	imv_agent_t *agent;
	char *hash_alg, *dh_group, *cadir;
	bool mandatory_dh_groups;

	agent = imv_agent_create(name, msg_types, countof(msg_types), id,
							 actual_version);
	if (!agent)
	{
		return NULL;
	}

	hash_alg = lib->settings->get_str(lib->settings,
				"%s.plugins.imv-attestation.hash_algorithm", "sha256", lib->ns);
	dh_group = lib->settings->get_str(lib->settings,
				"%s.plugins.imv-attestation.dh_group", "ecp256", lib->ns);
	mandatory_dh_groups = lib->settings->get_bool(lib->settings,
				"%s.plugins.imv-attestation.mandatory_dh_groups", TRUE, lib->ns);
	cadir = lib->settings->get_str(lib->settings,
				"%s.plugins.imv-attestation.cadir", NULL, lib->ns);

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
		.supported_algorithms = PTS_MEAS_ALGO_NONE,
		.supported_dh_groups = PTS_DH_GROUP_NONE,
		.pts_credmgr = credential_manager_create(),
		.pts_creds = pts_creds_create(cadir),
		.pts_db = pts_database_create(imcv_db),
	);

	if (!pts_meas_algo_probe(&this->supported_algorithms) ||
		!pts_dh_group_probe(&this->supported_dh_groups, mandatory_dh_groups) ||
		!pts_meas_algo_update(hash_alg, &this->supported_algorithms) ||
		!pts_dh_group_update(dh_group, &this->supported_dh_groups))
	{
		destroy(this);
		return NULL;
	}

	if (this->pts_creds)
	{
		this->pts_credmgr->add_set(this->pts_credmgr,
								   this->pts_creds->get_set(this->pts_creds));
	}

	return &this->public;
}
