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

#include "imv_attestation_state.h"

#include <imcv.h>
#include <imv/imv_lang_string.h>
#include "imv/imv_reason_string.h"

#include <tncif_policy.h>

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_imv_attestation_state_t private_imv_attestation_state_t;
typedef struct file_meas_request_t file_meas_request_t;
typedef struct func_comp_t func_comp_t;

/**
 * Private data of an imv_attestation_state_t object.
 */
struct private_imv_attestation_state_t {

	/**
	 * Public members of imv_attestation_state_t
	 */
	imv_attestation_state_t public;

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
	 * Flags set for completed actions
	 */
	uint32_t action_flags;

	/**
	 * IMV database session associated with TNCCS connection
	 */
	imv_session_t *session;

	/**
	 * PA-TNC attribute segmentation contracts associated with TNCCS connection
	 */
	seg_contract_manager_t *contracts;

	/**
	 * IMV Attestation handshake state
	 */
	imv_attestation_handshake_state_t handshake_state;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * IMV evaluation result
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * List of Functional Components
	 */
	linked_list_t *components;

	/**
	 * PTS object
	 */
	pts_t *pts;

	/**
	 * Measurement error flags
	 */
	uint32_t measurement_error;

	/**
	 * TNC Reason String
	 */
	imv_reason_string_t *reason_string;

};

/**
 * PTS Functional Component entry
 */
struct func_comp_t {
	pts_component_t *comp;
	pts_comp_func_name_t* name;
};

/**
 * Frees a func_comp_t object
 */
static void free_func_comp(func_comp_t *this)
{
	this->comp->destroy(this->comp);
	this->name->destroy(this->name);
	free(this);
}

/**
 * Supported languages
 */
static char* languages[] = { "en", "de", "mn" };

/**
 * Table of reason strings
 */
static imv_lang_string_t reason_file_meas_fail[] = {
	{ "en", "Incorrect file measurement" },
	{ "de", "Falsche Dateimessung" },
	{ "mn", "Буруу байгаа файл" },
	{ NULL, NULL }
};

static imv_lang_string_t reason_file_meas_pend[] = {
	{ "en", "Pending file measurement" },
	{ "de", "Ausstehende Dateimessung" },
	{ "mn", "Xүлээгдэж байгаа файл" },
	{ NULL, NULL }
};

static imv_lang_string_t reason_no_trusted_aik[] = {
	{ "en", "No trusted AIK available" },
	{ "de", "Kein vetrauenswürdiger AIK verfügbar" },
	{ NULL, NULL }
};

static imv_lang_string_t reason_comp_evid_fail[] = {
	{ "en", "Incorrect component evidence" },
	{ "de", "Falsche Komponenten-Evidenz" },
	{ "mn", "Буруу компонент хэмжилт" },
	{ NULL, NULL }
};

static imv_lang_string_t reason_comp_evid_pend[] = {
	{ "en", "Pending component evidence" },
	{ "de", "Ausstehende Komponenten-Evidenz" },
	{ "mn", "Xүлээгдэж компонент хэмжилт" },
	{ NULL, NULL }
};

static imv_lang_string_t reason_tpm_quote_fail[] = {
	{ "en", "Invalid TPM Quote signature received" },
	{ "de", "Falsche TPM Quote Signature erhalten" },
	{ "mn", "Буруу TPM Quote гарын үсэг" },
	{ NULL, NULL }
};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_attestation_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_attestation_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_attestation_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_attestation_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_attestation_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_attestation_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_action_flags, void,
	private_imv_attestation_state_t *this, uint32_t flags)
{
	this->action_flags |= flags;
}

METHOD(imv_state_t, get_action_flags, uint32_t,
	private_imv_attestation_state_t *this)
{
	return this->action_flags;
}

METHOD(imv_state_t, set_session, void,
	private_imv_attestation_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_attestation_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_attestation_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, change_state, void,
	private_imv_attestation_state_t *this, TNC_ConnectionState new_state)
{
	this->state = new_state;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_attestation_state_t *this, TNC_IMV_Action_Recommendation *rec,
										   TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_attestation_state_t *this, TNC_IMV_Action_Recommendation rec,
										   TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_attestation_state_t *this, TNC_IMV_Action_Recommendation rec,
										   TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_attestation_state_t, add_file_meas_reasons, void,
	private_imv_attestation_state_t *this, imv_reason_string_t *reason_string)
{
	if (this->measurement_error & IMV_ATTESTATION_ERROR_FILE_MEAS_FAIL)
	{
		reason_string->add_reason(reason_string, reason_file_meas_fail);
	}
	if (this->measurement_error & IMV_ATTESTATION_ERROR_FILE_MEAS_PEND)
	{
		reason_string->add_reason(reason_string, reason_file_meas_pend);
	}
}

METHOD(imv_attestation_state_t, add_comp_evid_reasons, void,
	private_imv_attestation_state_t *this, imv_reason_string_t *reason_string)
{
	if (this->measurement_error & IMV_ATTESTATION_ERROR_NO_TRUSTED_AIK)
	{
		reason_string->add_reason(reason_string, reason_no_trusted_aik);
	}
	if (this->measurement_error & IMV_ATTESTATION_ERROR_COMP_EVID_FAIL)
	{
		reason_string->add_reason(reason_string, reason_comp_evid_fail);
	}
	if (this->measurement_error & IMV_ATTESTATION_ERROR_COMP_EVID_PEND)
	{
		reason_string->add_reason(reason_string, reason_comp_evid_pend);
	}
	if (this->measurement_error & IMV_ATTESTATION_ERROR_TPM_QUOTE_FAIL)
	{
		reason_string->add_reason(reason_string, reason_tpm_quote_fail);
	}
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_attestation_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	*reason_language = imv_lang_string_select_lang(language_enumerator,
											  languages, countof(languages));

	/* Instantiate a TNC Reason String object */
	DESTROY_IF(this->reason_string);
	this->reason_string = imv_reason_string_create(*reason_language, "\n");
	add_file_meas_reasons(this, this->reason_string);
	add_comp_evid_reasons(this, this->reason_string);
	*reason_string = this->reason_string->get_encoding(this->reason_string);

	return TRUE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_attestation_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	return FALSE;
}

METHOD(imv_state_t, destroy, void,
	private_imv_attestation_state_t *this)
{
	DESTROY_IF(this->session);
	DESTROY_IF(this->reason_string);
	this->components->destroy_function(this->components, (void *)free_func_comp);
	this->pts->destroy(this->pts);
	this->contracts->destroy(this->contracts);
	free(this);
}

METHOD(imv_attestation_state_t, get_handshake_state,
	   imv_attestation_handshake_state_t, private_imv_attestation_state_t *this)
{
	return this->handshake_state;
}

METHOD(imv_attestation_state_t, set_handshake_state, void,
	private_imv_attestation_state_t *this,
	imv_attestation_handshake_state_t new_state)
{
	this->handshake_state = new_state;
}

METHOD(imv_attestation_state_t, get_pts, pts_t*,
	private_imv_attestation_state_t *this)
{
	return this->pts;
}

METHOD(imv_attestation_state_t, create_component, pts_component_t*,
	private_imv_attestation_state_t *this, pts_comp_func_name_t *name,
	uint32_t depth, pts_database_t *pts_db)
{
	enumerator_t *enumerator;
	func_comp_t *entry, *new_entry;
	pts_component_t *component;
	bool found = FALSE;

	enumerator = this->components->create_enumerator(this->components);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (name->equals(name, entry->comp->get_comp_func_name(entry->comp)))
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		if (name->equals(name, entry->name))
		{
			/* duplicate entry */
			return NULL;
		}
		new_entry = malloc_thing(func_comp_t);
		new_entry->name = name->clone(name);
		new_entry->comp = entry->comp->get_ref(entry->comp);
		this->components->insert_last(this->components, new_entry);
		return entry->comp;
	}
	else
	{
		component = imcv_pts_components->create(imcv_pts_components,
												name, depth, pts_db);
		if (!component)
		{
			/* unsupported component */
			return NULL;
		}
		new_entry = malloc_thing(func_comp_t);
		new_entry->name = name->clone(name);
		new_entry->comp = component;
		this->components->insert_last(this->components, new_entry);
		return component;
	}
}

/**
 * Enumerate file measurement entries
 */
static bool entry_filter(void *null, func_comp_t **entry, uint8_t *flags,
						 void *i2, uint32_t *depth,
						 void *i3, pts_comp_func_name_t **comp_name)
{
	pts_component_t *comp;
	pts_comp_func_name_t *name;

	comp = (*entry)->comp;
	name = (*entry)->name;

	*flags = comp->get_evidence_flags(comp);
	*depth = comp->get_depth(comp);
	*comp_name = name;

	return TRUE;
}

METHOD(imv_attestation_state_t, create_component_enumerator, enumerator_t*,
	private_imv_attestation_state_t *this)
{
	return enumerator_create_filter(
				this->components->create_enumerator(this->components),
				(void*)entry_filter, NULL, NULL);
}

METHOD(imv_attestation_state_t, get_component, pts_component_t*,
	private_imv_attestation_state_t *this, pts_comp_func_name_t *name)
{
	enumerator_t *enumerator;
	func_comp_t *entry;
	pts_component_t *found = NULL;

	enumerator = this->components->create_enumerator(this->components);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (name->equals(name, entry->name))
		{
			found = entry->comp;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(imv_attestation_state_t, get_measurement_error, uint32_t,
	private_imv_attestation_state_t *this)
{
	return this->measurement_error;
}

METHOD(imv_attestation_state_t, set_measurement_error, void,
	private_imv_attestation_state_t *this, uint32_t error)
{
	this->measurement_error |= error;
}

METHOD(imv_attestation_state_t, finalize_components, void,
	private_imv_attestation_state_t *this, bio_writer_t *result)
{
	func_comp_t *entry;
	bool first = TRUE;

	while (this->components->remove_last(this->components,
										(void**)&entry) == SUCCESS)
	{
		if (first)
		{
			first = FALSE;
		}
		else
		{
			result->write_data(result, chunk_from_str("; "));
		}
		if (!entry->comp->finalize(entry->comp,
								   entry->name->get_qualifier(entry->name),
								   result))
		{
			set_measurement_error(this, IMV_ATTESTATION_ERROR_COMP_EVID_PEND);
		}
		free_func_comp(entry);
	}
}

/**
 * Described in header.
 */
imv_state_t *imv_attestation_state_create(TNC_ConnectionID connection_id)
{
	private_imv_attestation_state_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_connection_id = _get_connection_id,
				.has_long = _has_long,
				.has_excl = _has_excl,
				.set_flags = _set_flags,
				.set_max_msg_len = _set_max_msg_len,
				.get_max_msg_len = _get_max_msg_len,
				.set_action_flags = _set_action_flags,
				.get_action_flags = _get_action_flags,
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
			.get_handshake_state = _get_handshake_state,
			.set_handshake_state = _set_handshake_state,
			.get_pts = _get_pts,
			.create_component = _create_component,
			.create_component_enumerator = _create_component_enumerator,
			.get_component = _get_component,
			.finalize_components = _finalize_components,
			.get_measurement_error = _get_measurement_error,
			.set_measurement_error = _set_measurement_error,
			.add_file_meas_reasons = _add_file_meas_reasons,
			.add_comp_evid_reasons = _add_comp_evid_reasons,
		},
		.connection_id = connection_id,
		.state = TNC_CONNECTION_STATE_CREATE,
		.handshake_state = IMV_ATTESTATION_STATE_INIT,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.contracts = seg_contract_manager_create(),
		.components = linked_list_create(),
		.pts = pts_create(FALSE),
	);

	return &this->public.interface;
}
