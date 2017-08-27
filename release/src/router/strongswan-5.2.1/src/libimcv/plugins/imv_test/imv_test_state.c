/*
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

#include "imv_test_state.h"
#include "imv/imv_lang_string.h"
#include "imv/imv_reason_string.h"

#include <tncif_policy.h>

#include <utils/lexparser.h>
#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_imv_test_state_t private_imv_test_state_t;

/**
 * Private data of an imv_test_state_t object.
 */
struct private_imv_test_state_t {

	/**
	 * Public members of imv_test_state_t
	 */
	imv_test_state_t public;

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID connection_id;

	/**
	 * TNCCS connection state
	 */
	TNC_ConnectionState state;

	/**
	 * Does the TNCCS connection support long message types?
	 */
	bool has_long;

	/**
	 * Does the TNCCS connection support exclusive delivery?
	 */
	bool has_excl;

	/**
	 * Maximum PA-TNC message size for this TNCCS connection
	 */
	uint32_t max_msg_len;

	/**
	 * IMV database session associated with TNCCS connection
	 */
	imv_session_t *session;

	/**
	 * PA-TNC attribute segmentation contracts associated with TNCCS connection
	 */
	seg_contract_manager_t *contracts;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * IMV evaluation result
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * TNC Reason String
	 */
	imv_reason_string_t *reason_string;

	/**
	 * List of IMCs
	 */
	linked_list_t *imcs;

};

typedef struct imc_entry_t imc_entry_t;

/**
 * Define an internal IMC entry
 */
struct imc_entry_t {
	TNC_UInt32 imc_id;
	int rounds;
};

/**
 * Supported languages
 */
static char* languages[] = { "en", "de", "fr", "pl" };

/**
 * Table of reason strings
 */
static imv_lang_string_t reasons[] = {
	{ "en", "IMC Test was not configured with \"command = allow\"" },
	{ "de", "IMC Test wurde nicht mit \"command = allow\" konfiguriert" },
	{ "fr", "IMC Test n'etait pas configuré avec \"command = allow\"" },
	{ "pl", "IMC Test nie zostało skonfigurowany z \"command = allow\"" },
	{ NULL, NULL }
};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_test_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_test_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_test_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_test_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_test_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_test_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_session, void,
	private_imv_test_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_test_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_test_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, change_state, void,
	private_imv_test_state_t *this, TNC_ConnectionState new_state)
{
	this->state = new_state;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_test_state_t *this, TNC_IMV_Action_Recommendation *rec,
									TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_test_state_t *this, TNC_IMV_Action_Recommendation rec,
									TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_test_state_t *this, TNC_IMV_Action_Recommendation rec,
									TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_test_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	*reason_language = imv_lang_string_select_lang(language_enumerator,
											  languages, countof(languages));

	/* Instantiate a TNC Reason String object */
	DESTROY_IF(this->reason_string);
	this->reason_string = imv_reason_string_create(*reason_language, "\n");
	this->reason_string->add_reason(this->reason_string, reasons);
	*reason_string = this->reason_string->get_encoding(this->reason_string);

	return TRUE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_test_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	return FALSE;
}

METHOD(imv_state_t, destroy, void,
	private_imv_test_state_t *this)
{
	DESTROY_IF(this->session);
	DESTROY_IF(this->reason_string);
	this->contracts->destroy(this->contracts);
	this->imcs->destroy_function(this->imcs, free);
	free(this);
}

METHOD(imv_test_state_t, add_imc, void,
	private_imv_test_state_t *this, TNC_UInt32 imc_id, int rounds)
{
	enumerator_t *enumerator;
	imc_entry_t *imc_entry;
	bool found = FALSE;

	enumerator = this->imcs->create_enumerator(this->imcs);
	while (enumerator->enumerate(enumerator, &imc_entry))
	{
		if (imc_entry->imc_id == imc_id)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		imc_entry = malloc_thing(imc_entry_t);
		imc_entry->imc_id = imc_id;
		imc_entry->rounds = rounds;
		this->imcs->insert_last(this->imcs, imc_entry);
	}
}

METHOD(imv_test_state_t, set_rounds, void,
	private_imv_test_state_t *this, int rounds)
{
	enumerator_t *enumerator;
	imc_entry_t *imc_entry;

	enumerator = this->imcs->create_enumerator(this->imcs);
	while (enumerator->enumerate(enumerator, &imc_entry))
	{
		imc_entry->rounds = rounds;
	}
	enumerator->destroy(enumerator);
}

METHOD(imv_test_state_t, another_round, bool,
	private_imv_test_state_t *this, TNC_UInt32 imc_id)
{
	enumerator_t *enumerator;
	imc_entry_t *imc_entry;
	bool not_finished = FALSE;

	enumerator = this->imcs->create_enumerator(this->imcs);
	while (enumerator->enumerate(enumerator, &imc_entry))
	{
		if (imc_entry->rounds > 0)
		{
			not_finished = TRUE;
		}
		if (imc_entry->imc_id == imc_id)
		{
			imc_entry->rounds--;
		}
	}
	enumerator->destroy(enumerator);

	return not_finished;
}

/**
 * Described in header.
 */
imv_state_t *imv_test_state_create(TNC_ConnectionID connection_id)
{
	private_imv_test_state_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_connection_id = _get_connection_id,
				.has_long = _has_long,
				.has_excl = _has_excl,
				.set_flags = _set_flags,
				.set_max_msg_len = _set_max_msg_len,
				.get_max_msg_len = _get_max_msg_len,
				.set_session = _set_session,
				.get_session = _get_session,
				.get_contracts = _get_contracts,
				.change_state = _change_state,
				.get_recommendation = _get_recommendation,
				.set_recommendation = _set_recommendation,
				.update_recommendation = _update_recommendation,
				.get_reason_string = _get_reason_string,
				.get_remediation_instructions = _get_remediation_instructions,
				.destroy = _destroy,
			},
			.add_imc = _add_imc,
			.set_rounds = _set_rounds,
			.another_round = _another_round,
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
		.imcs = linked_list_create(),
	);

	return &this->public.interface;
}


