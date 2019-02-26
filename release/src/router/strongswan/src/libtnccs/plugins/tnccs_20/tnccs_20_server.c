/*
 * Copyright (C) 2015 Andreas Steffen
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

#include "tnccs_20_server.h"
#include "messages/pb_tnc_msg.h"
#include "messages/ietf/pb_pa_msg.h"
#include "messages/ietf/pb_error_msg.h"
#include "messages/ietf/pb_assessment_result_msg.h"
#include "messages/ietf/pb_access_recommendation_msg.h"
#include "messages/ietf/pb_remediation_parameters_msg.h"
#include "messages/ietf/pb_reason_string_msg.h"
#include "messages/ietf/pb_language_preference_msg.h"
#include "messages/ita/pb_mutual_capability_msg.h"
#include "messages/ita/pb_noskip_test_msg.h"
#include "messages/tcg/pb_pdp_referral_msg.h"
#include "state_machine/pb_tnc_state_machine.h"

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <tnc/tnc.h>
#include <tnc/tnccs/tnccs_manager.h>
#include <tnc/imv/imv_manager.h>

#include <threading/mutex.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <pen/pen.h>

typedef struct private_tnccs_20_server_t private_tnccs_20_server_t;

/**
 * Private data of a tnccs_20_server_t object.
 */
struct private_tnccs_20_server_t {

	/**
	 * Public tnccs_20_server_t interface.
	 */
	tnccs_20_server_t public;

	/**
	 * PB-TNC State Machine
	 */
	pb_tnc_state_machine_t *state_machine;

	/**
	 * Connection ID assigned to this TNCCS connection
	 */
	TNC_ConnectionID connection_id;

	/**
	 * PB-TNC messages to be sent
	 */
	linked_list_t *messages;

	/**
	 * Type of PB-TNC batch being constructed
	 */
	pb_tnc_batch_type_t batch_type;

	/**
	 * Maximum PB-TNC batch size
	 */
	size_t max_batch_len;

	/**
	 * Mutex locking the batch in construction
	 */
	mutex_t *mutex;

	/**
	 * Flag set while processing
	 */
	bool fatal_error;

	/**
	 * Flag set by IMC/IMV RequestHandshakeRetry() function
	 */
	bool request_handshake_retry;

	/**
	 * Flag set after sending SRETRY batch
	 */
	bool retry_handshake;

	/**
	  * SendMessage() by IMV only allowed if flag is set
	  */
	bool send_msg;

	/**
	 * Set of IMV recommendations
	 */
	recommendations_t *recs;

	/**
	 *  TNC IF-T transport protocol for EAP methods
	 */
	bool eap_transport;

	/**
	 * Mutual PB-TNC protocol enabled
	 */
	bool mutual;

	/**
	 * Mutual Capability message sent
	 */
	bool sent_mutual_capability;

};

/**
 * The following two functions are shared with the tnccs_20_server class
 */
extern void tnccs_20_handle_ietf_error_msg(pb_tnc_msg_t *msg,
										   bool *fatal_error);
extern bool tnccs_20_handle_ita_mutual_capability_msg(pb_tnc_msg_t *msg);

/**
 * If the batch type changes then delete all accumulated PB-TNC messages
 */
static void change_batch_type(private_tnccs_20_server_t *this,
							  pb_tnc_batch_type_t batch_type)
{
	pb_tnc_msg_t *msg;

	if (batch_type != this->batch_type)
	{
		if (this->batch_type != PB_BATCH_NONE)
		{
			DBG1(DBG_TNC, "cancelling PB-TNC %N batch",
				 pb_tnc_batch_type_names, this->batch_type);

			while (this->messages->remove_last(this->messages,
											  (void**)&msg) == SUCCESS)
			{
				msg->destroy(msg);
			}
		}
		this->batch_type = batch_type;
	}
}

/**
 * Handle a single PB-TNC IETF standard message according to its type
 */
static void handle_ietf_message(private_tnccs_20_server_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.type)
	{
		case PB_MSG_EXPERIMENTAL:
			/* nothing to do */
			break;
		case PB_MSG_PA:
		{
			pb_pa_msg_t *pa_msg;
			pen_type_t msg_subtype;
			uint16_t imc_id, imv_id;
			chunk_t msg_body;
			bool excl;
			enum_name_t *pa_subtype_names;

			pa_msg = (pb_pa_msg_t*)msg;
			msg_subtype = pa_msg->get_subtype(pa_msg);
			msg_body = pa_msg->get_body(pa_msg);
			imc_id = pa_msg->get_collector_id(pa_msg);
			imv_id = pa_msg->get_validator_id(pa_msg);
			excl = pa_msg->get_exclusive_flag(pa_msg);

			pa_subtype_names = get_pa_subtype_names(msg_subtype.vendor_id);
			if (pa_subtype_names)
			{
				DBG2(DBG_TNC, "handling PB-PA message type '%N/%N' 0x%06x/0x%08x",
					 pen_names, msg_subtype.vendor_id, pa_subtype_names,
					 msg_subtype.type, msg_subtype.vendor_id, msg_subtype.type);
			}
			else
			{
				DBG2(DBG_TNC, "handling PB-PA message type '%N' 0x%06x/0x%08x",
					 pen_names, msg_subtype.vendor_id, msg_subtype.vendor_id,
					 msg_subtype.type);
			}
			this->send_msg = TRUE;
			tnc->imvs->receive_message(tnc->imvs, this->connection_id,
									   excl, msg_body.ptr, msg_body.len,
									   msg_subtype.vendor_id,
									   msg_subtype.type, imc_id, imv_id);
			this->send_msg = FALSE;
			break;
		}
		case PB_MSG_ERROR:
			tnccs_20_handle_ietf_error_msg(msg, &this->fatal_error);
			break;
		case PB_MSG_LANGUAGE_PREFERENCE:
		{
			pb_language_preference_msg_t *lang_msg;
			chunk_t lang;

			lang_msg = (pb_language_preference_msg_t*)msg;
			lang = lang_msg->get_language_preference(lang_msg);
			DBG2(DBG_TNC, "setting language preference to '%.*s'",
				 (int)lang.len, lang.ptr);
			this->recs->set_preferred_language(this->recs, lang);
			break;
		}
		default:
			break;
	}
}

/**
 * Handle a single PB-TNC ITA standard message according to its type
 */
static void handle_ita_message(private_tnccs_20_server_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.type)
	{
		case PB_ITA_MSG_MUTUAL_CAPABILITY:
			this->mutual = tnccs_20_handle_ita_mutual_capability_msg(msg);

			/* Respond with PB-TNC Mutual Capability message if necessary */
			if (this->mutual && !this->sent_mutual_capability)
			{
				msg = pb_mutual_capability_msg_create(PB_MUTUAL_HALF_DUPLEX);
				this->mutex->lock(this->mutex);
				this->messages->insert_last(this->messages, msg);
				this->mutex->unlock(this->mutex);
				this->sent_mutual_capability = TRUE;
			}
			break;
		default:
			break;
	}
}

/**
 * Handle a single PB-TNC message according to its type
 */
static void handle_message(private_tnccs_20_server_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.vendor_id)
	{
		case PEN_IETF:
			handle_ietf_message(this, msg);
			break;
		case PEN_ITA:
			handle_ita_message(this, msg);
			break;
		default:
			break;
	}
}

/**
 *  Build an SRETRY batch
 */
static void build_retry_batch(private_tnccs_20_server_t *this)
{
	if (this->batch_type == PB_BATCH_SRETRY)
	{
		/* retry batch has already been selected */
		return;
	}
	change_batch_type(this, PB_BATCH_SRETRY);

	this->recs->clear_recommendation(this->recs);

	/* Handshake will be retried with next incoming CDATA batch */
	this->retry_handshake = TRUE;
}

METHOD(tnccs_20_handler_t, process, status_t,
	private_tnccs_20_server_t *this, pb_tnc_batch_t *batch)
{
	pb_tnc_batch_type_t batch_type;
	status_t status;

	batch_type = batch->get_type(batch);

	DBG1(DBG_TNC, "processing PB-TNC %N batch for Connection ID %d",
		 pb_tnc_batch_type_names, batch_type, this->connection_id);
	status = batch->process(batch, this->state_machine);

	if (status != FAILED)
	{
		enumerator_t *enumerator;
		pb_tnc_msg_t *msg;
		bool empty = TRUE;

		if (batch_type == PB_BATCH_CDATA)
		{
			/* retry handshake after a previous SRETRY batch */
			if (this->retry_handshake)
			{
				tnc->imvs->notify_connection_change(tnc->imvs,
						this->connection_id, TNC_CONNECTION_STATE_HANDSHAKE);
				this->retry_handshake = FALSE;
			}
		}
		else if (batch_type == PB_BATCH_CRETRY)
		{
			/* Send an SRETRY batch in response */
			this->mutex->lock(this->mutex);
			build_retry_batch(this);
			this->mutex->unlock(this->mutex);
		}

		enumerator = batch->create_msg_enumerator(batch);
		while (enumerator->enumerate(enumerator, &msg))
		{
			handle_message(this, msg);
			empty = FALSE;
		}
		enumerator->destroy(enumerator);

		/* received a CLOSE batch from PB-TNC client */
		if (batch_type == PB_BATCH_CLOSE)
		{
			return empty ? SUCCESS : FAILED;
		}

		this->send_msg = TRUE;
		tnc->imvs->batch_ending(tnc->imvs, this->connection_id);
		this->send_msg = FALSE;
	}

	switch (status)
	{
		case FAILED:
			this->fatal_error = TRUE;
			status = VERIFY_ERROR;
			break;
		case VERIFY_ERROR:
			break;
		case SUCCESS:
		default:
			status = NEED_MORE;
			break;
	}

	return status;
}

/**
 *  Build a RESULT batch if a final recommendation is available
 */
static void check_and_build_recommendation(private_tnccs_20_server_t *this)
{
	TNC_IMV_Action_Recommendation rec;
	TNC_IMV_Evaluation_Result eval;
	TNC_ConnectionState state;
	TNC_IMVID id;
	chunk_t reason, language;
	enumerator_t *enumerator;
	pb_tnc_msg_t *msg;
	pb_access_recommendation_code_t pb_rec;

	if (!this->recs->have_recommendation(this->recs, &rec, &eval))
	{
		tnc->imvs->solicit_recommendation(tnc->imvs, this->connection_id);
	}
	if (this->recs->have_recommendation(this->recs, &rec, &eval))
	{
		this->batch_type = PB_BATCH_RESULT;

		msg = pb_assessment_result_msg_create(eval);
		this->messages->insert_last(this->messages, msg);

		/**
		 * Map IMV Action Recommendation codes to PB Access Recommendation codes
		 * and communicate Access Recommendation to IMVs
		 */
		switch (rec)
		{
			case TNC_IMV_ACTION_RECOMMENDATION_ALLOW:
				state = TNC_CONNECTION_STATE_ACCESS_ALLOWED;
				pb_rec = PB_REC_ACCESS_ALLOWED;
				break;
			case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
				state = TNC_CONNECTION_STATE_ACCESS_ISOLATED;
				pb_rec = PB_REC_QUARANTINED;
				break;
			case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
			case TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION:
			default:
				state = TNC_CONNECTION_STATE_ACCESS_NONE;
				pb_rec = PB_REC_ACCESS_DENIED;
		}
		tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
											state);

		msg = pb_access_recommendation_msg_create(pb_rec);
		this->messages->insert_last(this->messages, msg);

		enumerator = this->recs->create_reason_enumerator(this->recs);
		while (enumerator->enumerate(enumerator, &id, &reason, &language))
		{
			msg = pb_reason_string_msg_create(reason, language);
			this->messages->insert_last(this->messages, msg);
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(tnccs_20_handler_t, build, status_t,
	private_tnccs_20_server_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	status_t status;
	pb_tnc_state_t state;

	state = this->state_machine->get_state(this->state_machine);

	if (this->fatal_error && state == PB_STATE_END)
	{
		DBG1(DBG_TNC, "a fatal PB-TNC error occurred, terminating connection");
		return FAILED;
	}

	/* Do not allow any asynchronous IMVs to add additional messages */
	this->mutex->lock(this->mutex);

	if (this->request_handshake_retry)
	{
		if (state != PB_STATE_INIT)
		{
			build_retry_batch(this);
		}

		/* Reset the flag for the next handshake retry request */
		this->request_handshake_retry = FALSE;
	}

	if (state == PB_STATE_SERVER_WORKING &&
		this->recs->have_recommendation(this->recs, NULL, NULL))
	{
		check_and_build_recommendation(this);
	}

	if (this->batch_type == PB_BATCH_NONE)
	{
		if (state == PB_STATE_SERVER_WORKING)
		{
			if (this->state_machine->get_empty_cdata(this->state_machine))
			{
				check_and_build_recommendation(this);
			}
			else
			{
				DBG2(DBG_TNC, "no recommendation available yet, "
							  "sending empty PB-TNC SDATA batch");
				this->batch_type = PB_BATCH_SDATA;
			}
		}
	}

	if (this->batch_type != PB_BATCH_NONE)
	{
		pb_tnc_batch_t *batch;
		pb_tnc_msg_t *msg;
		chunk_t data;
		int msg_count;
		enumerator_t *enumerator;

		if (this->state_machine->send_batch(this->state_machine, this->batch_type))
		{
			batch = pb_tnc_batch_create(TRUE, this->batch_type,
										min(this->max_batch_len, *buflen));

			enumerator = this->messages->create_enumerator(this->messages);
			while (enumerator->enumerate(enumerator, &msg))
			{
				if (batch->add_msg(batch, msg))
				{
					this->messages->remove_at(this->messages, enumerator);
				}
				else
				{
					break;
				}
			}
			enumerator->destroy(enumerator);

			batch->build(batch);
			data = batch->get_encoding(batch);
			DBG1(DBG_TNC, "sending PB-TNC %N batch (%d bytes) for Connection ID %u",
						   pb_tnc_batch_type_names, this->batch_type, data.len,
						   this->connection_id);
			DBG3(DBG_TNC, "%B", &data);

			*buflen = data.len;
			*msglen = 0;
			memcpy(buf, data.ptr, *buflen);
			batch->destroy(batch);

			msg_count = this->messages->get_count(this->messages);
			if (msg_count)
			{
				DBG2(DBG_TNC, "queued %d PB-TNC message%s for next %N batch",
					 msg_count, (msg_count == 1) ? "" : "s",
					 pb_tnc_batch_type_names, this->batch_type);
			}
			else
			{
				this->batch_type = PB_BATCH_NONE;
			}

			status = ALREADY_DONE;
		}
		else
		{
			change_batch_type(this, PB_BATCH_NONE);
			status = INVALID_STATE;
		}
	}
	else
	{
		DBG1(DBG_TNC, "no PB-TNC batch to send");
		status = INVALID_STATE;
	}
	this->mutex->unlock(this->mutex);

	return status;
}

METHOD(tnccs_20_handler_t, begin_handshake, void,
	private_tnccs_20_server_t *this, bool mutual)
{
	pb_tnc_msg_t *msg;
	identification_t *pdp_server;
	uint16_t *pdp_port;

	tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
										TNC_CONNECTION_STATE_HANDSHAKE);

	/* Send a PB-TNC TCG PDP Referral message if PDP is known */
	pdp_server = (identification_t*)lib->get(lib, "pt-tls-server");
	pdp_port = (uint16_t*)lib->get(lib, "pt-tls-port");

	if (this->eap_transport && pdp_server && pdp_port)
	{
		msg = pb_pdp_referral_msg_create_from_fqdn(
						pdp_server->get_encoding(pdp_server), *pdp_port);
		this->mutex->lock(this->mutex);
		this->messages->insert_last(this->messages, msg);
		this->mutex->unlock(this->mutex);
	}

	/* Send a PB-Noskip-Test message for testing purposes */
	if (lib->settings->get_bool(lib->settings,
				"%s.plugins.tnccs-20.tests.pb_tnc_noskip", FALSE, lib->ns))
	{
		msg = pb_noskip_test_msg_create();
		this->mutex->lock(this->mutex);
		this->messages->insert_last(this->messages, msg);
		this->mutex->unlock(this->mutex);
	}
}

METHOD(tnccs_20_handler_t, get_send_flag, bool,
	private_tnccs_20_server_t *this)
{
	return this->send_msg;
}

METHOD(tnccs_20_handler_t, get_mutual, bool,
	private_tnccs_20_server_t *this)
{
	return this->mutual;
}

METHOD(tnccs_20_handler_t, get_state, pb_tnc_state_t,
	private_tnccs_20_server_t *this)
{
	return this->state_machine->get_state(this->state_machine);
}

METHOD(tnccs_20_handler_t, add_msg, void,
	private_tnccs_20_server_t *this, pb_tnc_msg_t *msg)
{
	/* adding PA message to SDATA batch only */
	this->mutex->lock(this->mutex);
	if (this->batch_type == PB_BATCH_NONE)
	{
		this->batch_type = PB_BATCH_SDATA;
	}
	if (this->batch_type == PB_BATCH_SDATA)
	{
		this->messages->insert_last(this->messages, msg);
	}
	else
	{
		msg->destroy(msg);
	}
	this->mutex->unlock(this->mutex);
}

METHOD(tnccs_20_handler_t, handle_errors, void,
	private_tnccs_20_server_t *this,  pb_tnc_batch_t *batch,
	bool fatal_header_error)
{
	pb_tnc_msg_t *msg;
	enumerator_t *enumerator;

	if (fatal_header_error || this->fatal_error)
	{
		this->mutex->lock(this->mutex);
		change_batch_type(this, PB_BATCH_CLOSE);
		this->mutex->unlock(this->mutex);
	}

	enumerator = batch->create_error_enumerator(batch);
	while (enumerator->enumerate(enumerator, &msg))
	{
		this->mutex->lock(this->mutex);
		this->messages->insert_last(this->messages, msg->get_ref(msg));
		this->mutex->unlock(this->mutex);
	}
	enumerator->destroy(enumerator);
}

METHOD(tnccs_20_handler_t, destroy, void,
	private_tnccs_20_server_t *this)
{
	if (this->connection_id)
	{
		tnc->tnccs->remove_connection(tnc->tnccs, this->connection_id, TRUE);
	}
	this->state_machine->destroy(this->state_machine);
	this->mutex->destroy(this->mutex);
	this->messages->destroy_offset(this->messages,
								   offsetof(pb_tnc_msg_t, destroy));
	free(this);
}

METHOD(tnccs_20_server_t, have_recommendation, bool,
	private_tnccs_20_server_t *this, TNC_IMV_Action_Recommendation *rec,
	TNC_IMV_Evaluation_Result *eval)
{
	return this->recs->have_recommendation(this->recs, rec, eval);
}

/**
 * See header
 */
tnccs_20_handler_t* tnccs_20_server_create(tnccs_t *tnccs,
										   tnccs_send_message_t send_msg,
										   size_t max_batch_len,
										   size_t max_msg_len,
										   bool eap_transport)
{
	private_tnccs_20_server_t *this;

	INIT(this,
		.public = {
			.handler = {
				.process = _process,
				.build = _build,
				.begin_handshake = _begin_handshake,
				.get_send_flag = _get_send_flag,
				.get_mutual = _get_mutual,
				.get_state = _get_state,
				.add_msg = _add_msg,
				.handle_errors = _handle_errors,
				.destroy = _destroy,
			},
			.have_recommendation = _have_recommendation,
		},
		.state_machine = pb_tnc_state_machine_create(TRUE),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.messages = linked_list_create(),
		.batch_type = PB_BATCH_SDATA,
		.max_batch_len = max_batch_len,
		.eap_transport = eap_transport,
	);

	this->connection_id = tnc->tnccs->create_connection(tnc->tnccs,
											TNCCS_2_0, tnccs, send_msg,
											&this->request_handshake_retry,
											max_msg_len, &this->recs);
	if (!this->connection_id)
	{
		destroy(this);
		return NULL;
	}
	tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
										TNC_CONNECTION_STATE_CREATE);

	return &this->public.handler;
}
