/*
 * Copyright (C) 2010 Sansar Choinyanbuu
 * Copyright (C) 2010-2013 Andreas Steffen
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

#include "tnccs_20.h"
#include "batch/pb_tnc_batch.h"
#include "messages/pb_tnc_msg.h"
#include "messages/ietf/pb_pa_msg.h"
#include "messages/ietf/pb_error_msg.h"
#include "messages/ietf/pb_assessment_result_msg.h"
#include "messages/ietf/pb_access_recommendation_msg.h"
#include "messages/ietf/pb_remediation_parameters_msg.h"
#include "messages/ietf/pb_reason_string_msg.h"
#include "messages/ietf/pb_language_preference_msg.h"
#include "messages/tcg/pb_pdp_referral_msg.h"
#include "state_machine/pb_tnc_state_machine.h"

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <tnc/tnc.h>
#include <tnc/tnccs/tnccs_manager.h>
#include <tnc/imc/imc_manager.h>
#include <tnc/imv/imv_manager.h>

#include <threading/mutex.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <pen/pen.h>

typedef struct private_tnccs_20_t private_tnccs_20_t;

/**
 * Private data of a tnccs_20_t object.
 */
struct private_tnccs_20_t {

	/**
	 * Public tnccs_t interface.
	 */
	tnccs_t public;

	/**
	 * TNCC if TRUE, TNCS if FALSE
	 */
	bool is_server;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Client identity
	 */
	identification_t *peer;

	/**
	 * Underlying TNC IF-T transport protocol
	 */
	tnc_ift_type_t transport;

	/**
	 * Type of TNC client authentication
	 */
	u_int32_t auth_type;

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
	 * Maximum PA-TNC message size
	 */
	size_t max_msg_len;

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
	  * SendMessage() by IMC/IMV only allowed if flag is set
	  */
	bool send_msg;

	/**
	 * Set of IMV recommendations  (TNC Server only)
	 */
	recommendations_t *recs;

	/**
	 * Callback function to communicate recommendation (TNC Server only)
	 */
	tnccs_cb_t callback;

	/**
	 * Data to pass to callback function (TNC Server only)
	 */
	void *cb_data;

	/**
	 * PDP server FQDN
	 */
	chunk_t pdp_server;

	/**
	 * PDP server port
	 */
	u_int16_t pdp_port;

	/**
	 * reference count
	 */
	refcount_t ref;

};

/**
 * If the batch type changes then delete all accumulated PB-TNC messages
 */
void change_batch_type(private_tnccs_20_t *this, pb_tnc_batch_type_t batch_type)
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

METHOD(tnccs_t, send_msg, TNC_Result,
	private_tnccs_20_t* this, TNC_IMCID imc_id, TNC_IMVID imv_id,
						      TNC_UInt32 msg_flags,
							  TNC_BufferReference msg,
							  TNC_UInt32 msg_len,
						      TNC_VendorID msg_vid,
						      TNC_MessageSubtype msg_subtype)
{
	pb_tnc_msg_t *pb_tnc_msg;
	pb_tnc_batch_type_t batch_type;
	enum_name_t *pa_subtype_names;
	bool excl;

	if (!this->send_msg)
	{
		DBG1(DBG_TNC, "%s %u not allowed to call SendMessage()",
			this->is_server ? "IMV" : "IMC",
			this->is_server ? imv_id : imc_id);
		return TNC_RESULT_ILLEGAL_OPERATION;
	}
	excl = (msg_flags & TNC_MESSAGE_FLAGS_EXCLUSIVE) != 0;

	pb_tnc_msg = pb_pa_msg_create(msg_vid, msg_subtype, imc_id, imv_id,
								  excl, chunk_create(msg, msg_len));

	pa_subtype_names = get_pa_subtype_names(msg_vid);
	if (pa_subtype_names)
	{
		DBG2(DBG_TNC, "creating PB-PA message type '%N/%N' 0x%06x/0x%08x",
					   pen_names, msg_vid, pa_subtype_names, msg_subtype,
					   msg_vid, msg_subtype);
	}
	else
	{
		DBG2(DBG_TNC, "creating PB-PA message type '%N' 0x%06x/0x%08x",
					   pen_names, msg_vid, msg_vid, msg_subtype);
	}

	/* adding PA message to SDATA or CDATA batch only */
	batch_type = this->is_server ? PB_BATCH_SDATA : PB_BATCH_CDATA;
	this->mutex->lock(this->mutex);
	if (this->batch_type == PB_BATCH_NONE)
	{
		this->batch_type = batch_type;
	}
	if (this->batch_type == batch_type)
	{
		this->messages->insert_last(this->messages, pb_tnc_msg);
	}
	else
	{
		pb_tnc_msg->destroy(pb_tnc_msg);
	}
	this->mutex->unlock(this->mutex);
	return TNC_RESULT_SUCCESS;
}

/**
 * Handle a single PB-TNC IETF standard message according to its type
 */
static void handle_ietf_message(private_tnccs_20_t *this, pb_tnc_msg_t *msg)
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
			u_int16_t imc_id, imv_id;
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
			if (this->is_server)
			{
				tnc->imvs->receive_message(tnc->imvs, this->connection_id,
										   excl, msg_body.ptr, msg_body.len,
										   msg_subtype.vendor_id,
										   msg_subtype.type, imc_id, imv_id);
			}
			else
			{
				tnc->imcs->receive_message(tnc->imcs, this->connection_id,
										   excl, msg_body.ptr, msg_body.len,
										   msg_subtype.vendor_id,
										   msg_subtype.type, imv_id, imc_id);
			}
			this->send_msg = FALSE;
			break;
		}
		case PB_MSG_ASSESSMENT_RESULT:
		{
			pb_assessment_result_msg_t *assess_msg;
			u_int32_t result;

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
		{
			pb_error_msg_t *err_msg;
			bool fatal;
			u_int32_t vendor_id;
			u_int16_t error_code;

			err_msg = (pb_error_msg_t*)msg;
			fatal = err_msg->get_fatal_flag(err_msg);
			vendor_id = err_msg->get_vendor_id(err_msg);
			error_code = err_msg->get_error_code(err_msg);

			if (fatal)
			{
				this->fatal_error = TRUE;
			}

			if (vendor_id == PEN_IETF)
			{
				switch (error_code)
				{
					case PB_ERROR_INVALID_PARAMETER:
					case PB_ERROR_UNSUPPORTED_MANDATORY_MSG:
						DBG1(DBG_TNC, "received %s PB-TNC error '%N' "
									  "(offset %u bytes)",
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
				DBG1(DBG_TNC, "received %s PB-TNC error (%u) "
							  "with Vendor ID 0x%06x",
							  fatal ? "fatal" : "non-fatal",
							  error_code, vendor_id);
			}
			break;
		}
		case PB_MSG_LANGUAGE_PREFERENCE:
		{
			pb_language_preference_msg_t *lang_msg;
			chunk_t lang;

			lang_msg = (pb_language_preference_msg_t*)msg;
			lang = lang_msg->get_language_preference(lang_msg);

			if (this->recs)
			{
				DBG2(DBG_TNC, "setting language preference to '%.*s'",
					 (int)lang.len, lang.ptr);
				this->recs->set_preferred_language(this->recs, lang);
			}
			break;
		}
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
static void handle_tcg_message(private_tnccs_20_t *this, pb_tnc_msg_t *msg)
{
	pen_type_t msg_type = msg->get_type(msg);

	switch (msg_type.type)
	{
		case PB_TCG_MSG_PDP_REFERRAL:
		{
			pb_pdp_referral_msg_t *pdp_msg;
			pen_type_t pdp_id_type;
			u_int8_t pdp_protocol;

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
 * Handle a single PB-TNC message according to its type
 */
static void handle_message(private_tnccs_20_t *this, pb_tnc_msg_t *msg)
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
		default:
			break;
	}
}

/**
 *  Build a CRETRY or SRETRY batch
 */
static void build_retry_batch(private_tnccs_20_t *this)
{
	pb_tnc_batch_type_t batch_retry_type;

	batch_retry_type = this->is_server ? PB_BATCH_SRETRY : PB_BATCH_CRETRY;
	if (this->batch_type == batch_retry_type)
	{
		/* retry batch has already been selected */
		return;
	}

	change_batch_type(this, batch_retry_type);

	if (this->is_server)
	{
		this->recs->clear_recommendation(this->recs);
		tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
											TNC_CONNECTION_STATE_HANDSHAKE);
	}
}

METHOD(tls_t, process, status_t,
	private_tnccs_20_t *this, void *buf, size_t buflen)
{
	chunk_t data;
	pb_tnc_batch_t *batch;
	pb_tnc_msg_t *msg;
	enumerator_t *enumerator;
	identification_t *pdp_server;
	u_int16_t *pdp_port;
	status_t status;

	if (this->is_server && !this->connection_id)
	{
		this->connection_id = tnc->tnccs->create_connection(tnc->tnccs,
									TNCCS_2_0, (tnccs_t*)this, _send_msg,
									&this->request_handshake_retry,
									this->max_msg_len, &this->recs);
		if (!this->connection_id)
		{
			return FAILED;
		}
		tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
											TNC_CONNECTION_STATE_CREATE);
		tnc->imvs->notify_connection_change(tnc->imvs, this->connection_id,
											TNC_CONNECTION_STATE_HANDSHAKE);

		/* Send a PB-TNC TCG PDP Referral message if PDP is known */
		pdp_server = (identification_t*)lib->get(lib, "pt-tls-server");
		pdp_port = (u_int16_t*)lib->get(lib, "pt-tls-port");

		if ((this->transport ==	TNC_IFT_EAP_1_1 ||
			 this->transport == TNC_IFT_EAP_2_0) &&	pdp_server && pdp_port)
		{
			msg = pb_pdp_referral_msg_create_from_fqdn(
						pdp_server->get_encoding(pdp_server), *pdp_port);
			this->messages->insert_last(this->messages, msg);
		}

	}

	data = chunk_create(buf, buflen);
	DBG1(DBG_TNC, "received TNCCS batch (%u bytes) for Connection ID %u",
				   data.len, this->connection_id);
	DBG3(DBG_TNC, "%B", &data);
	batch = pb_tnc_batch_create_from_data(this->is_server, data);
	status = batch->process(batch, this->state_machine);

	if (status != FAILED)
	{
		enumerator_t *enumerator;
		pb_tnc_msg_t *msg;
		pb_tnc_batch_type_t batch_type;
		bool empty = TRUE;

		batch_type = batch->get_type(batch);

		if (batch_type == PB_BATCH_CRETRY)
		{
			/* Send an SRETRY batch in response */
			this->mutex->lock(this->mutex);
			build_retry_batch(this);
			this->mutex->unlock(this->mutex);
		}
		else if (batch_type == PB_BATCH_SRETRY)
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

		/* received an empty CLOSE batch from PB-TNC client */
		if (this->is_server && batch_type == PB_BATCH_CLOSE && empty)
		{
			batch->destroy(batch);
			if (this->fatal_error)
			{
				DBG1(DBG_TNC, "a fatal PB-TNC error occurred, "
							  "terminating connection");
				return FAILED;
			}
			else
			{
				return SUCCESS;
			}
		}

		this->send_msg = TRUE;
		if (this->is_server)
		{
			tnc->imvs->batch_ending(tnc->imvs, this->connection_id);
		}
		else
		{
			tnc->imcs->batch_ending(tnc->imcs, this->connection_id);
		}
		this->send_msg = FALSE;
	}

	switch (status)
	{
		case FAILED:
			this->fatal_error = TRUE;
			this->mutex->lock(this->mutex);
			change_batch_type(this, PB_BATCH_CLOSE);
			this->mutex->unlock(this->mutex);
			/* fall through to add error messages to outbound batch */
		case VERIFY_ERROR:
			enumerator = batch->create_error_enumerator(batch);
			while (enumerator->enumerate(enumerator, &msg))
			{
				this->mutex->lock(this->mutex);
				this->messages->insert_last(this->messages, msg->get_ref(msg));
				this->mutex->unlock(this->mutex);
			}
			enumerator->destroy(enumerator);
			break;
		case SUCCESS:
		default:
			break;
	}
	batch->destroy(batch);

	return NEED_MORE;
}

/**
 *  Build a RESULT batch if a final recommendation is available
 */
static void check_and_build_recommendation(private_tnccs_20_t *this)
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

METHOD(tls_t, build, status_t,
	private_tnccs_20_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	status_t status;
	pb_tnc_state_t state;

	/* Initialize the connection */
	if (!this->is_server && !this->connection_id)
	{
		pb_tnc_msg_t *msg;
		char *pref_lang;

		this->connection_id = tnc->tnccs->create_connection(tnc->tnccs,
										TNCCS_2_0, (tnccs_t*)this, _send_msg,
										&this->request_handshake_retry,
										this->max_msg_len, NULL);
		if (!this->connection_id)
		{
			return FAILED;
		}

		/* Create PB-TNC Language Preference message */
		pref_lang = tnc->imcs->get_preferred_language(tnc->imcs);
		msg = pb_language_preference_msg_create(chunk_create(pref_lang,
													strlen(pref_lang)));
		this->mutex->lock(this->mutex);
		this->batch_type = PB_BATCH_CDATA;
		this->messages->insert_last(this->messages, msg);
		this->mutex->unlock(this->mutex);

		tnc->imcs->notify_connection_change(tnc->imcs, this->connection_id,
											TNC_CONNECTION_STATE_CREATE);
		tnc->imcs->notify_connection_change(tnc->imcs, this->connection_id,
											TNC_CONNECTION_STATE_HANDSHAKE);
		this->send_msg = TRUE;
		tnc->imcs->begin_handshake(tnc->imcs, this->connection_id);
		this->send_msg = FALSE;
	}

	state = this->state_machine->get_state(this->state_machine);

	if (this->fatal_error && state == PB_STATE_END)
	{
		DBG1(DBG_TNC, "a fatal PB-TNC error occurred, terminating connection");
		return FAILED;
	}

	/* Do not allow any asynchronous IMCs or IMVs to add additional messages */
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

	if (this->is_server &&  state == PB_STATE_SERVER_WORKING &&
		this->recs->have_recommendation(this->recs, NULL, NULL))
	{
		check_and_build_recommendation(this);
	}

	if (this->batch_type == PB_BATCH_NONE)
	{
		if (this->is_server)
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
		else
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
			batch = pb_tnc_batch_create(this->is_server, this->batch_type,
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

METHOD(tls_t, is_server, bool,
	private_tnccs_20_t *this)
{
	return this->is_server;
}

METHOD(tls_t, get_server_id, identification_t*,
	private_tnccs_20_t *this)
{
	return this->server;
}

METHOD(tls_t, set_peer_id, void,
	private_tnccs_20_t *this, identification_t *id)
{
	DESTROY_IF(this->peer);
	this->peer = id->clone(id);
}

METHOD(tls_t, get_peer_id, identification_t*,
	private_tnccs_20_t *this)
{
	return this->peer;
}

METHOD(tls_t, get_purpose, tls_purpose_t,
	private_tnccs_20_t *this)
{
	return TLS_PURPOSE_EAP_TNC;
}

METHOD(tls_t, is_complete, bool,
	private_tnccs_20_t *this)
{
	TNC_IMV_Action_Recommendation rec;
	TNC_IMV_Evaluation_Result eval;

	if (this->recs && this->recs->have_recommendation(this->recs, &rec, &eval))
	{
		return this->callback ? this->callback(rec, eval) : TRUE;
	}
	else
	{
		return FALSE;
	}
}

METHOD(tls_t, get_eap_msk, chunk_t,
	private_tnccs_20_t *this)
{
	return chunk_empty;
}

METHOD(tls_t, destroy, void,
	private_tnccs_20_t *this)
{
	if (ref_put(&this->ref))
	{
		tnc->tnccs->remove_connection(tnc->tnccs, this->connection_id,
												  this->is_server);
		this->server->destroy(this->server);
		this->peer->destroy(this->peer);
		this->state_machine->destroy(this->state_machine);
		this->mutex->destroy(this->mutex);
		this->messages->destroy_offset(this->messages,
									   offsetof(pb_tnc_msg_t, destroy));
		free(this->pdp_server.ptr);
		free(this);
	}
}

METHOD(tnccs_t, get_transport, tnc_ift_type_t,
	private_tnccs_20_t *this)
{
	return this->transport;
}

METHOD(tnccs_t, set_transport, void,
	private_tnccs_20_t *this, tnc_ift_type_t transport)
{
	this->transport = transport;
}

METHOD(tnccs_t, get_auth_type, u_int32_t,
	private_tnccs_20_t *this)
{
	return this->auth_type;
}

METHOD(tnccs_t, set_auth_type, void,
	private_tnccs_20_t *this, u_int32_t auth_type)
{
	this->auth_type = auth_type;
}

METHOD(tnccs_t, get_pdp_server, chunk_t,
	private_tnccs_20_t *this, u_int16_t *port)
{
	*port = this->pdp_port;

	return this->pdp_server;
}

METHOD(tnccs_t, get_ref, tnccs_t*,
	private_tnccs_20_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

/**
 * See header
 */
tnccs_t* tnccs_20_create(bool is_server,
						 identification_t *server, identification_t *peer,
						 tnc_ift_type_t transport, tnccs_cb_t cb)
{
	private_tnccs_20_t *this;
	size_t max_batch_size, default_max_batch_size;
	size_t max_message_size, default_max_message_size;

	/* Determine the maximum PB-TNC batch size and PA-TNC message size */
	switch (transport)
	{
		case TNC_IFT_TLS_2_0:
		case TNC_IFT_TLS_1_0:
			default_max_batch_size = 128 * TLS_MAX_FRAGMENT_LEN - 16;
			break;
		case TNC_IFT_EAP_2_0:
		case TNC_IFT_EAP_1_1:
		case TNC_IFT_EAP_1_0:
		case TNC_IFT_UNKNOWN:
		default:
			default_max_batch_size =   4 * TLS_MAX_FRAGMENT_LEN - 14;
			break;
	}

	max_batch_size = min(default_max_batch_size,
						lib->settings->get_int(lib->settings,
								"%s.plugins.tnccs-20.max_batch_size",
								 default_max_batch_size, lib->ns));

	default_max_message_size = max_batch_size - PB_TNC_BATCH_HEADER_SIZE
											  - PB_TNC_MSG_HEADER_SIZE
											  - PB_PA_MSG_HEADER_SIZE;

	max_message_size = min(default_max_message_size,
							lib->settings->get_int(lib->settings,
								"%s.plugins.tnccs-20.max_message_size",
								 default_max_message_size, lib->ns));

	INIT(this,
		.public = {
			.tls = {
				.process = _process,
				.build = _build,
				.is_server = _is_server,
				.get_server_id = _get_server_id,
				.set_peer_id = _set_peer_id,
				.get_peer_id = _get_peer_id,
				.get_purpose = _get_purpose,
				.is_complete = _is_complete,
				.get_eap_msk = _get_eap_msk,
				.destroy = _destroy,
			},
			.get_transport = _get_transport,
			.set_transport = _set_transport,
			.get_auth_type = _get_auth_type,
			.set_auth_type = _set_auth_type,
			.get_pdp_server = _get_pdp_server,
			.get_ref = _get_ref,
		},
		.is_server = is_server,
		.server = server->clone(server),
		.peer = peer->clone(peer),
		.transport = transport,
		.callback = cb,
		.state_machine = pb_tnc_state_machine_create(is_server),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.messages = linked_list_create(),
		.max_batch_len = max_batch_size,
		.max_msg_len = max_message_size,
		.ref = 1,
	);

	return &this->public;
}
