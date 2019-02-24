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

#include "imc_swima_state.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include "ietf/swima/ietf_swima_attr_req.h"
#include "ietf/swima/ietf_swima_attr_sw_inv.h"
#include "ietf/swima/ietf_swima_attr_sw_ev.h"
#include "swima/swima_inventory.h"
#include "swima/swima_collector.h"
#include "swima/swima_error.h"
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"

#include <tncif_pa_subtypes.h>
#include <pen/pen.h>
#include <utils/debug.h>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#ifndef SW_COLLECTOR
#define SW_COLLECTOR NULL
#endif

/* IMC definitions */

static const char imc_name[] = "SWIMA";

static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_SWIMA }
};

static imc_agent_t *imc_swima;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Initialize(TNC_IMCID imc_id,
							  TNC_Version min_version,
							  TNC_Version max_version,
							  TNC_Version *actual_version)
{
	if (imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_swima = imc_agent_create(imc_name, msg_types, countof(msg_types),
								 imc_id, actual_version);
	if (!imc_swima)
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
 * Poll for IN_CLOSE_WRITE event on the apt history.log
 */
static bool poll_history_log(void)
{
	int fd, wd, res;
	nfds_t nfds;
	struct pollfd fds[1];
	char *history_path;
	bool success = FALSE;

	history_path = lib->settings->get_str(lib->settings, "sw-collector.history",
										  NULL);
	if (!history_path)
	{
		DBG1(DBG_IMC, "sw-collector.history path not set");
		return FALSE;
	}

	/* Create the file descriptor for accessing the inotify API */
	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1)
	{
		DBG1(DBG_IMC, "inotify file descriptor could not be created");
		return FALSE;
	}

	/* Watch for CLOSE_WRITE events on history log */
	wd = inotify_add_watch(fd, history_path, IN_CLOSE_WRITE);
	if (wd == -1)
	{
		DBG1(DBG_IMC, "cannot watch '%s'", history_path);
		goto end;
	}

	/* Prepare for polling */
	nfds = 1;

	/* Inotify input */
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	while (1)
	{
		DBG1(DBG_IMC, "  waiting for write event on history.log ...");

		res = poll(fds, nfds, -1);
		if (res == -1)
		{
			DBG1(DBG_IMC, "  poll failed: %s", strerror(errno));
			if (errno == EINTR)
			{
				continue;
			}
			goto end;
		}
		if (res > 0 &&	fds[0].revents & POLLIN)
		{
			DBG1(DBG_IMC, "  poll successful");
			success = TRUE;
			break;
		}
	}

end:
	close(fd);
	return success;
}

/**
 * see section 3.8.2 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_NotifyConnectionChange(TNC_IMCID imc_id,
										  TNC_ConnectionID connection_id,
										  TNC_ConnectionState new_state)
{
	imc_state_t *state;
	imc_swima_state_t *swima_state;
	imc_swima_subscription_t *subscription;
	TNC_IMV_Evaluation_Result res;
	TNC_Result result;
	uint32_t eid, eid_epoch;

	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_swima_state_create(connection_id);
			return imc_swima->create_state(imc_swima, state);
		case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
		case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		case TNC_CONNECTION_STATE_ACCESS_NONE:
			/* get updated IMC state */
			result = imc_swima->change_state(imc_swima, connection_id,
											 new_state, &state);
			if (result != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			swima_state = (imc_swima_state_t*)state;

			/* do a handshake retry? */
			if (swima_state->get_subscription(swima_state, &subscription))
			{
				/* update earliest EID in subscription target */
				if (state->get_result(state, imc_id, &res) &&
					res == TNC_IMV_EVALUATION_RESULT_COMPLIANT)
				{
					eid = subscription->targets->get_eid(subscription->targets,
												   &eid_epoch);
					if (eid > 0)
					{
						eid = swima_state->get_earliest_eid(swima_state);
						subscription->targets->set_eid(subscription->targets, eid,
													   eid_epoch);
					}
				}
				DBG1(DBG_IMC, "SWIMA subscription %u:", subscription->request_id);
				if (!poll_history_log())
				{
					return TNC_RESULT_FATAL;
				}
				return imc_swima->request_handshake_retry(imc_id, connection_id,
												TNC_RETRY_REASON_IMC_PERIODIC);
			}
			return TNC_RESULT_SUCCESS;
		case TNC_CONNECTION_STATE_DELETE:
			return imc_swima->delete_state(imc_swima, connection_id);
		default:
			return imc_swima->change_state(imc_swima, connection_id,
										   new_state, NULL);
	}
}

/**
 * Add SWID Inventory or Event attribute to the send queue
 */
static void fulfill_request(imc_state_t *state, imc_msg_t *msg,
							   uint32_t request_id, bool sw_id_only,
							   swima_inventory_t *targets)
{
	pa_tnc_attr_t *attr;
	swima_collector_t  *collector;
	size_t msg_len = 64;
	char error_msg[msg_len], *id_str;
	bool collect_inventory = TRUE;
	int items;

	collector = swima_collector_create();
	id_str = sw_id_only ? " ID" : "";

	if (targets->get_eid(targets, NULL) > 0)
	{
		swima_events_t *sw_ev;
		ietf_swima_attr_sw_ev_t *sw_ev_attr;
		imc_swima_state_t *swima_state;
		uint32_t eid_epoch, last_eid = 0;

		sw_ev = collector->collect_events(collector, sw_id_only, targets);
		if (!sw_ev)
		{
			snprintf(error_msg, msg_len, "failed to collect SW%s events, "
					 "fallback to SW%s inventory", id_str, id_str);
			attr = swima_error_create(PA_ERROR_SWIMA, request_id, 0, error_msg);
			msg->add_attribute(msg, attr);
		}
		else {
			items = sw_ev->get_count(sw_ev);
			last_eid = sw_ev->get_eid(sw_ev, &eid_epoch, NULL);

			DBG1(DBG_IMC, "collected %d SW%s event%s at last eid %d of epoch 0x%08x",
				 items, id_str, items == 1 ? "" : "s", last_eid, eid_epoch);

			/* Store the earliest EID for the next subscription round */
			swima_state = (imc_swima_state_t*)state;
			swima_state->set_earliest_eid(swima_state, last_eid + 1);

			/* Send an IETF SW [Identity] Events attribute */
			attr = ietf_swima_attr_sw_ev_create(IETF_SWIMA_ATTR_SW_INV_FLAG_NONE,
											 request_id, sw_id_only);
			sw_ev_attr = (ietf_swima_attr_sw_ev_t*)attr;
			sw_ev_attr->set_events(sw_ev_attr, sw_ev);
			collect_inventory = FALSE;
		}
	}

	if (collect_inventory)
	{
		swima_inventory_t *sw_inv;
		ietf_swima_attr_sw_inv_t *sw_inv_attr;

		sw_inv = collector->collect_inventory(collector, sw_id_only, targets);
		if (!sw_inv)
		{
			snprintf(error_msg, msg_len, "failed to collect SW%s inventory",
					 id_str);
			attr = swima_error_create(PA_ERROR_SWIMA, request_id, 0, error_msg);
		}
		else
		{
			items = sw_inv->get_count(sw_inv);
			DBG1(DBG_IMC, "collected %d SW%s record%s", items, id_str,
														items == 1 ? "" : "s");

			/* Send an IETF SW [Identity] Inventory attribute */
			attr = ietf_swima_attr_sw_inv_create(IETF_SWIMA_ATTR_SW_INV_FLAG_NONE,
												 request_id, sw_id_only);
			sw_inv_attr = (ietf_swima_attr_sw_inv_t*)attr;
			sw_inv_attr->set_inventory(sw_inv_attr, sw_inv);
		}
	}
	msg->add_attribute(msg, attr);
	collector->destroy(collector);
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_swima_state_t *swima_state;
	imc_msg_t *out_msg;
	pa_tnc_attr_t *attr;
	seg_contract_t *contract;
	seg_contract_manager_t *contracts;
	imc_swima_subscription_t *subscription;
	size_t max_attr_size = SWIMA_MAX_ATTR_SIZE;
	size_t max_seg_size;
	char buf[BUF_LEN];
	TNC_Result result = TNC_RESULT_SUCCESS;

	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swima->get_state(imc_swima, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	swima_state = (imc_swima_state_t*)state;

	if (swima_state->get_subscription(swima_state, &subscription))
	{
		if (system(SW_COLLECTOR) != 0)
		{
			DBG1(DBG_IMC, "calling %s failed", SW_COLLECTOR);
			return TNC_RESULT_FATAL;
		}
		out_msg = imc_msg_create(imc_swima, state, connection_id, imc_id,
								 subscription->imv_id, msg_types[0]);
		fulfill_request(state, out_msg, subscription->request_id,
						subscription->sw_id_only, subscription->targets);
	}
	else
	{
		/* Determine maximum PA-TNC attribute segment size */
		max_seg_size = state->get_max_msg_len(state) - PA_TNC_HEADER_SIZE
													 - PA_TNC_ATTR_HEADER_SIZE
												 - TCG_SEG_ATTR_SEG_ENV_HEADER;

		/* Announce support of PA-TNC segmentation to IMV */
		contract = seg_contract_create(msg_types[0], max_attr_size, max_seg_size,
									   TRUE, imc_id, TRUE);
		contract->get_info_string(contract, buf, BUF_LEN, TRUE);
		DBG2(DBG_IMC, "%s", buf);
		contracts = state->get_contracts(state);
		contracts->add_contract(contracts, contract);
		attr = tcg_seg_attr_max_size_create(max_attr_size, max_seg_size, TRUE);

		/* send PA-TNC message with the excl flag not set */
		out_msg = imc_msg_create(imc_swima, state, connection_id, imc_id,
								 TNC_IMVID_ANY, msg_types[0]);
		out_msg->add_attribute(out_msg, attr);
	}
	result = out_msg->send(out_msg, FALSE);
	out_msg->destroy(out_msg);

	return result;
}

static TNC_Result receive_message(imc_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	imc_swima_state_t *swima_state;
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
		ietf_swima_attr_req_t *attr_req;
		uint8_t flags;
		uint32_t request_id;
		bool sw_id_only;
		swima_inventory_t *targets;
		type = attr->get_type(attr);

		if (type.vendor_id != PEN_IETF || type.type != IETF_ATTR_SWIMA_REQUEST)
		{
			continue;
		}

		attr_req = (ietf_swima_attr_req_t*)attr;
		flags = attr_req->get_flags(attr_req);
		request_id = attr_req->get_request_id(attr_req);
		targets = attr_req->get_targets(attr_req);
		sw_id_only = (flags & IETF_SWIMA_ATTR_REQ_FLAG_R);

		if (flags & (IETF_SWIMA_ATTR_REQ_FLAG_S | IETF_SWIMA_ATTR_REQ_FLAG_C))
		{
			if (imc_swima->has_pt_tls(imc_swima) &&
				lib->settings->get_bool(lib->settings,
					"%s.plugins.imc-swima.subscriptions", FALSE, lib->ns))
			{
				imc_swima_subscription_t *subscription;

				swima_state = (imc_swima_state_t*)state;

				if (flags & IETF_SWIMA_ATTR_REQ_FLAG_C)
				{
					if (swima_state->get_subscription(swima_state, &subscription))
					{
						DBG1(DBG_IMC, "SWIMA subscription %u cleared",
									   subscription->request_id);
						swima_state->set_subscription(swima_state, NULL, FALSE);
					}
				}
				else
				{
					INIT(subscription,
						.imv_id = in_msg->get_src_id(in_msg),
						.request_id = request_id,
						.targets = targets->get_ref(targets),
						.sw_id_only = sw_id_only,
					);

					swima_state->set_subscription(swima_state, subscription,
												  TRUE);
					DBG1(DBG_IMC, "SWIMA subscription %u established",
								   subscription->request_id);
					if (system(SW_COLLECTOR) != 0)
					{
						DBG1(DBG_IMC, "calling %s failed", SW_COLLECTOR);
						out_msg->destroy(out_msg);
						return TNC_RESULT_FATAL;
					}
				}
			}
			else
			{
				attr = swima_error_create(PA_ERROR_SWIMA_SUBSCRIPTION_DENIED,
							request_id, 0, "subscriptions not enabled");
				out_msg->add_attribute(out_msg, attr);
			}
		}

		fulfill_request(state, out_msg, request_id, sw_id_only, targets);
		break;
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

	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swima->get_state(imc_swima, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_data(imc_swima, state, connection_id, msg_type,
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

	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_swima->get_state(imc_swima, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_swima, state, connection_id,
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
	if (!imc_swima)
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
	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_swima->destroy(imc_swima);
	imc_swima = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
									   TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_swima)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_swima->bind_functions(imc_swima, bind_function);
}
