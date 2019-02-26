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

#include "tnccs_20_client.h"
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
#include <tnc/imc/imc_manager.h>

#include <threading/mutex.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <pen/pen.h>

typedef struct private_tnccs_20_client_t private_tnccs_20_client_t;

/**
 * Private data of a tnccs_20_client_t object.
 */
struct private_tnccs_20_client_t {

	/**
	 * Public tnccs_20_client_t interface.
	 */
	tnccs_20_client_t public;

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
	 * Flag set by IMC RequestHandshakeRetry() function
	 */
	bool request_handshake_retry;

	/**
	  * SendMessage() by IMC only allowed if flag is set
	  */
	bool send_msg;

	/**
	 * PDP server FQDN
	 */
	chunk_t pdp_server;

	/**
	 * PDP server port
	 */
	uint16_t pdp_port;

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
void tnccs_20_handle_ietf_error_msg(pb_tnc_msg_t *msg, bool *fatal_error)
{
	pb_error_msg_t *err_msg;
	uint32_t vendor_id;
	uint16_t error_code;
	bool fatal;

	err_msg = (pb_error_msg_t*)msg;
	fatal = err_msg->get_fatal_flag(err_msg);
	vendor_id = err_msg->get_vendor_id(err_msg);
	error_code = err_msg->get_error_code(err_msg);

	if (fatal)
	{
		*fatal_error = TRUE;
	}

	if (vendor_id == PEN_IETF)
	{
		switch (error_code)
		{
			case PB_ERROR_INVALID_PARAMETER:
			case PB_ERROR_UNSUPPORTED_MANDATORY_MSG:
				DBG1(DBG_TNC, "received %s PB-TNC error '%N' (offset %u bytes)",
							  fatal ? "fatal" : "non-fatal",
							  pb_tnc_error_code_names, error_code,
							  err_msg->get_offset(err_msg));
				break;
			case PB_ERROR_VERSION_NOT_SUPPORTED:
				DBG1(DBG_TNC, "received %s PB-TNC error '%N' "
							  "caused by bad version 0x%02x",
							  fatal ? "fatal" : "non-fatal",
							  pb_tnc_error_code_names, error_code,
							  err_msg->get_bad_version(err_msg));
				break;
			case PB_ERROR_UNEXPECTED_BATCH_TYPE:
			case PB_ERROR_LOCAL_ERROR:
			default:
				DBG1(DBG_TNC, "received %s PB-TNC error '%N'",
							  fatal ? "fatal" : "non-fatal",
							  pb_tnc_error_code_names, error_code);
				break;
		}
	}
	else
	{
		DBG1(DBG_TNC, "received %s PB-TNC error (%u) with Vendor ID 0x%06x",
					  fatal ? "fatal" : "non-fatal", error_code, vendor_id);
	}
}

bool tnccs_20_handle_ita_mutual_capability_msg(pb_tnc_msg_t *msg)
{
	pb_mutual_capability_msg_t *mutual_msg;
	uint32_t protocols;

	if (!lib->settings->get_bool(lib->settings,
				"%s.plugins.tnccs-20.mutual", FALSE, lib->ns))
	{
		/* PB-TNC mutual capability disabled, ignore message */
		return FALSE;
	}

	mutual_msg = (pb_mutual_capability_msg_t*)msg;
	protocols = mutual_msg->get_protocols(mutual_msg);

	if (protocols & PB_MUTUAL_HALF_DUPLEX)
	{
		DBG1(DBG_TNC, "activating mutual PB-TNC %N protocol",
			 pb_tnc_mutual_protocol_type_names, PB_MUTUAL_HALF_DUPLEX);
		return TRUE;
	}

	return FALSE;
}

/**
 * If the batch type changes then delete all accumulated PB-TNC messages
 */
static void change_batch_type(private_tnccs_20_client_t *this,
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
static void handle_ietf_message(private_tnccs_20_client_t *this, pb_tnc_msg_t *msg)
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
			tnc->imcs->receive_message(tnc->imcs, this->connection_id,
									   excl, msg_body.ptr, msg_body.len,
									   msg_subtype.vendor_id,
									   msg_subtype.type, imv_id, imc_id);
			this->send_msg = FALSE;
			break;
		}
		case PB_MSG_ASSESSMENT_RESULT:
		{
			pb_assessment_result_msg_t *assess_msg;
			uint32_t result;

			assess_msg = (pb_assessment_result_msg_t*)msg;
			result = assess_msg->get_assessment_result(assess_msg);
			DBG1(DBG_TNC, "PB-TNC assessment result is '%N'",
				 TNC_IMV_Evaluation_Result_names, result);
			break;
		}
		case PB_MSG_ACCESS_RECOMMENDATION:
		{
			pb_access_recommendation_msg_t *rec_msg;
			pb_access_recommendation_code_t rec;
			TNC_ConnectionState state = TNC_CONNECTION_STATE_ACCESS_NONE;

			rec_msg = (pb_access_recommendation_msg_t*)msg;
			rec = rec_msg->get_access_recommendation(rec_msg);
			DBG1(DBG_TNC, "PB-TNC access recommendation is '%N'",
						   pb_access_recommendation_code_names, rec);
			switch (rec)
			{
				case PB_REC_ACCESS_ALLOWED:
					state = TNC_CONNECTION_STATE_ACCESS_ALLOWED;
					break;
				case PB_REC_ACCESS_DENIED:
					state = TNC_CONNECTION_STATE_ACCESS_NONE;
					break;
				case PB_REC_QUARANTINED:
					state = TNC_CONNECTION_STATE_ACCESS_ISOLATED;
			}
			tnc->imcs->notify_connection_change(tnc->imcs, this->connection_id,
												state);
			break;
		}
		case PB_MSG_REMEDIATION_PARAMETERS:
		{
			pb_remediation_parameters_msg_t *rem_msg;
			pen_type_t parameters_type;
			chunk_t parameters, string, lang_code;

			rem_msg = (pb_remediation_parameters_msg_t*)msg;
			parameters_type = rem_msg->get_parameters_type(rem_msg);
			parameters = rem_msg->get_parameters(rem_msg);

			if (parameters_type.vendor_id == PEN_IETF)
			{
				switch (parameters_type.type)
				{
					case PB_REMEDIATION_URI:
						DBG1(DBG_TNC, "remediation uri: %.*s",
									   parameters.len, parameters.ptr);
						break;
					case PB_REMEDIATION_STRING:
						string = rem_msg->get_string(rem_msg, &lang_code);
						DBG1(DBG_TNC, "remediation string: [%.*s]\n%.*s",
									   lang_code.len, lang_code.ptr,
									   string.len, string.ptr);
						break;
					default:
						DBG1(DBG_TNC, "remediation parameters: %B", &parameters);
				}
			}
			else
			{
				DBG1(DBG_TNC, "remediation parameters: %B", &parameters);
			}
			break;
		}
		case PB_MSG_ERROR:
			tnccs_20_handle_ietf_error_msg(msg, &this->fatal_error);
			break;
		case PB_MSG_REASON_STRING:
		{
			pb_reason_string_msg_t *reason_msg;
			chunk_t reason_string, language_code;

			reason_msg = (pb_reason_string_msg_t*)msg;
			reason_string = reason_msg->get_reason_string(reason_msg);
			language_code = reason_msg->get_language_code(reason_msg);
			DBG1(DBG_TNC, "reason string is '%.*s' [%.*s]",
				 (int)reason_string.len, reason_string.ptr,
				 (int)language_code.len, language_code.ptr);
			break;
		}
		default:
			break;
	}
}

/**
 * Handle a single PB-TNC TCG standard message according to its type
 */
static void handle_tcg_message(private_tnccs_20_client_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.type)
	{
		case PB_TCG_MSG_PDP_REFERRAL:
		{
			pb_pdp_referral_msg_t *pdp_msg;
			pen_type_t pdp_id_type;
			uint8_t pdp_protocol;

			pdp_msg = (pb_pdp_referral_msg_t*)msg;
			pdp_id_type = pdp_msg->get_identifier_type(pdp_msg);

			if (pdp_id_type.vendor_id == PEN_TCG &&
				pdp_id_type.type == PB_PDP_ID_FQDN)
			{
				this->pdp_server = chunk_clone(pdp_msg->get_fqdn(pdp_msg,
										 &pdp_protocol, &this->pdp_port));
				if (pdp_protocol != 0)
				{
					DBG1(DBG_TNC, "unsupported PDP transport protocol");
					break;
				}
				DBG1(DBG_TNC, "PDP server '%.*s' is listening on port %u",
							   this->pdp_server.len, this->pdp_server.ptr,
							   this->pdp_port);
			}
			break;
		}
		default:
			break;
	}
}

/**
 * Handle a single PB-TNC ITA standard message according to its type
 */
static void handle_ita_message(private_tnccs_20_client_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.type)
	{
		case PB_ITA_MSG_MUTUAL_CAPABILITY:
			this->mutual = tnccs_20_handle_ita_mutual_capability_msg(msg);
			break;
		default:
			break;
	}
}

/**
 * Handle a single PB-TNC message according to its type
 */
static void handle_message(private_tnccs_20_client_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.vendor_id)
	{
		case PEN_IETF:
			handle_ietf_message(this, msg);
			break;
		case PEN_TCG:
			handle_tcg_message(this, msg);
			break;
		case PEN_ITA:
			handle_ita_message(this, msg);
			break;
		default:
			break;
	}
}

/**
 *  Build a CRETRY batch
 */
static void build_retry_batch(private_tnccs_20_client_t *this)
{
	if (this->batch_type == PB_BATCH_CRETRY)
	{
		/* retry batch has already been selected */
		return;
	}
	change_batch_type(this, PB_BATCH_CRETRY);
}

METHOD(tnccs_20_handler_t, process, status_t,
	private_tnccs_20_client_t *this, pb_tnc_batch_t *batch)
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

		if (batch_type == PB_BATCH_SRETRY)
		{
			/* Restart the measurements */
			tnc->imcs->notify_connection_change(tnc->imcs,
						this->connection_id, TNC_CONNECTION_STATE_HANDSHAKE);
			this->send_msg = TRUE;
			tnc->imcs->begin_handshake(tnc->imcs, this->connection_id);
			this->send_msg = FALSE;
		}

		enumerator = batch->create_msg_enumerator(batch);
		while (enumerator->enumerate(enumerator, &msg))
		{
			handle_message(this, msg);
			empty = FALSE;
		}
		enumerator->destroy(enumerator);

		/* received a CLOSE batch from PB-TNC server */
		if (batch_type == PB_BATCH_CLOSE)
		{
			return empty ? SUCCESS : FAILED;
		}

 		this->send_msg = TRUE;
		tnc->imcs->batch_ending(tnc->imcs, this->connection_id);
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

METHOD(tnccs_20_handler_t, build, status_t,
	private_tnccs_20_client_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	status_t status;
	pb_tnc_state_t state;

	state = this->state_machine->get_state(this->state_machine);

	if (this->fatal_error && state == PB_STATE_END)
	{
		DBG1(DBG_TNC, "a fatal PB-TNC error occurred, terminating connection");
		return FAILED;
	}

	/* Do not allow any asynchronous IMCs to add additional messages */
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

	if (this->batch_type == PB_BATCH_NONE)
	{
		switch (state)
		{
			case PB_STATE_CLIENT_WORKING:
				DBG2(DBG_TNC, "no client data to send, "
							  "sending empty PB-TNC CDATA batch");
				this->batch_type = PB_BATCH_CDATA;
				break;
			case PB_STATE_DECIDED:
				/**
				 * In the DECIDED state and if no CRETRY is under way,
				 * a PB-TNC client replies with an empty CLOSE batch.
				 */
				this->batch_type = PB_BATCH_CLOSE;
				break;
			default:
				break;
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
			batch = pb_tnc_batch_create(FALSE, this->batch_type,
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
	private_tnccs_20_client_t *this, bool mutual)
{
	pb_tnc_msg_t *msg;
	char *pref_lang;

	tnc->imcs->notify_connection_change(tnc->imcs, this->connection_id,
										TNC_CONNECTION_STATE_HANDSHAKE);

	/* Announce PB-TNC Mutual Capability if activated */
	this->sent_mutual_capability = mutual;

	if (!mutual && lib->settings->get_bool(lib->settings,
				"%s.plugins.tnccs-20.mutual", FALSE, lib->ns))
	{
		pb_tnc_mutual_protocol_type_t protocols;

		protocols = PB_MUTUAL_HALF_DUPLEX;
		DBG2(DBG_TNC, "proposing PB-TNC mutual %N protocol",
			 pb_tnc_mutual_protocol_type_names, PB_MUTUAL_HALF_DUPLEX);
		msg = pb_mutual_capability_msg_create(protocols);
		this->mutex->lock(this->mutex);
		this->messages->insert_last(this->messages, msg);
		this->mutex->unlock(this->mutex);
		this->sent_mutual_capability = TRUE;
	}

	/* Create PB-TNC Language Preference message */
	pref_lang = tnc->imcs->get_preferred_language(tnc->imcs);
	msg = pb_language_preference_msg_create(chunk_create(pref_lang,
												strlen(pref_lang)));
	this->mutex->lock(this->mutex);
	this->messages->insert_last(this->messages, msg);
	this->mutex->unlock(this->mutex);

	this->send_msg = TRUE;
	tnc->imcs->begin_handshake(tnc->imcs, this->connection_id);
	this->send_msg = FALSE;

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
	private_tnccs_20_client_t *this)
{
	return this->send_msg;
}

METHOD(tnccs_20_handler_t, get_mutual, bool,
	private_tnccs_20_client_t *this)
{
	return this->mutual;
}

METHOD(tnccs_20_handler_t, get_state, pb_tnc_state_t,
	private_tnccs_20_client_t *this)
{
	return this->state_machine->get_state(this->state_machine);
}

METHOD(tnccs_20_handler_t, add_msg, void,
	private_tnccs_20_client_t *this, pb_tnc_msg_t *msg)
{
	/* adding PA message to CDATA batch only */
	this->mutex->lock(this->mutex);
	if (this->batch_type == PB_BATCH_NONE)
	{
		this->batch_type = PB_BATCH_CDATA;
	}
	if (this->batch_type == PB_BATCH_CDATA)
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
	private_tnccs_20_client_t *this, pb_tnc_batch_t *batch,
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
	private_tnccs_20_client_t *this)
{
	if (this->connection_id)
	{
		tnc->tnccs->remove_connection(tnc->tnccs, this->connection_id, FALSE);
	}
	this->state_machine->destroy(this->state_machine);
	this->mutex->destroy(this->mutex);
	this->messages->destroy_offset(this->messages,
								   offsetof(pb_tnc_msg_t, destroy));
	free(this->pdp_server.ptr);
	free(this);
}

METHOD(tnccs_20_client_t, get_pdp_server, chunk_t,
	private_tnccs_20_client_t *this, uint16_t *port)
{
	*port = this->pdp_port;

	return this->pdp_server;
}

/**
 * See header
 */
tnccs_20_handler_t* tnccs_20_client_create(tnccs_t *tnccs,
										   tnccs_send_message_t send_msg,
										   size_t max_batch_len,
										   size_t max_msg_len)
{
	private_tnccs_20_client_t *this;

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
			.get_pdp_server = _get_pdp_server,
		},
		.state_machine = pb_tnc_state_machine_create(FALSE),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.messages = linked_list_create(),
		.batch_type = PB_BATCH_CDATA,
		.max_batch_len = max_batch_len,
	);

	this->connection_id = tnc->tnccs->create_connection(tnc->tnccs,
											TNCCS_2_0, tnccs, send_msg,
											&this->request_handshake_retry,
											max_msg_len, NULL);
	if (!this->connection_id)
	{
		destroy(this);
		return NULL;
	}
	tnc->imcs->notify_connection_change(tnc->imcs, this->connection_id,
										TNC_CONNECTION_STATE_CREATE);

	return &this->public.handler;
}
