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

#include "imv_scanner_state.h"
#include "imv/imv_lang_string.h"
#include "imv/imv_reason_string.h"
#include "imv/imv_remediation_string.h"

#include <tncif_policy.h>

#include <utils/lexparser.h>
#include <utils/debug.h>

typedef struct private_imv_scanner_state_t private_imv_scanner_state_t;

/**
 * Private data of an imv_scanner_state_t object.
 */
struct private_imv_scanner_state_t {

	/**
	 * Public members of imv_scanner_state_t
	 */
	imv_scanner_state_t public;

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
	 * IMV database session associatied with TNCCS connection
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
	 * IMV Scanner handshake state
	 */
	imv_scanner_handshake_state_t handshake_state;

	/**
	 * Copy of the received IEEE Port Filter attribute
	 */
	 ietf_attr_port_filter_t *port_filter_attr;

	/**
	 * List with ports that should be closed
	 */
	 linked_list_t *violating_ports;

	/**
	 * TNC Reason String
	 */
	imv_reason_string_t *reason_string;

	/**
	 * IETF Remediation Instructions String
	 */
	imv_remediation_string_t *remediation_string;

};

/**
 * Supported languages
 */
static char* languages[] = { "en", "de", "fr", "pl" };

/**
 * Reason strings for "Port Filter"
 */
static imv_lang_string_t reasons[] = {
	{ "en", "Open server ports were detected" },
	{ "de", "Offene Serverports wurden festgestellt" },
	{ "fr", "Il y a des ports du serveur ouverts" },
	{ "pl", "Są otwarte porty serwera" },
	{ NULL, NULL }
};

/**
 * Instruction strings for "Port Filters"
 */
static imv_lang_string_t instr_ports_title[] = {
	{ "en", "Open Server Ports" },
	{ "de", "Offene Server Ports" },
	{ "fr", "Ports ouverts du serveur" },
	{ "pl", "Otwarte Porty Serwera" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_ports_descr[] = {
	{ "en", "Open Internet ports have been detected" },
	{ "de", "Offenen Internet-Ports wurden festgestellt" },
	{ "fr", "Il y'a des ports Internet ouverts" },
	{ "pl", "Porty internetowe są otwarte" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_ports_header[] = {
	{ "en", "Please close the following server ports:" },
	{ "de", "Bitte schliessen Sie die folgenden Serverports:" },
	{ "fr", "Fermez les ports du serveur suivants s'il vous plait:" },
	{ "pl", "Proszę zamknąć następujące porty serwera:" },
	{ NULL, NULL }
};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_scanner_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_scanner_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_scanner_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_scanner_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_scanner_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_scanner_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_action_flags, void,
	private_imv_scanner_state_t *this, uint32_t flags)
{
	this->action_flags |= flags;
}

METHOD(imv_state_t, get_action_flags, uint32_t,
	private_imv_scanner_state_t *this)
{
	return this->action_flags;
}

METHOD(imv_state_t, set_session, void,
	private_imv_scanner_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_scanner_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_scanner_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, change_state, void,
	private_imv_scanner_state_t *this, TNC_ConnectionState new_state)
{
	this->state = new_state;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_scanner_state_t *this, TNC_IMV_Action_Recommendation *rec,
									   TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_scanner_state_t *this, TNC_IMV_Action_Recommendation rec,
									   TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_scanner_state_t *this, TNC_IMV_Action_Recommendation rec,
									   TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_scanner_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	if (this->violating_ports->get_count(this->violating_ports) == 0)
	{
		return FALSE;
	}
	*reason_language = imv_lang_string_select_lang(language_enumerator,
											  languages, countof(languages));

	/* Instantiate a TNC Reason String object */
	DESTROY_IF(this->reason_string);
	this->reason_string = imv_reason_string_create(*reason_language, "\n");
	if (this->rec != TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION)
	{
		this->reason_string->add_reason(this->reason_string, reasons);
	}
	*reason_string = this->reason_string->get_encoding(this->reason_string);

	return TRUE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_scanner_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	if (this->violating_ports->get_count(this->violating_ports) == 0)
	{
		return FALSE;
	}
	*lang_code = imv_lang_string_select_lang(language_enumerator,
										languages, countof(languages));

	/* Instantiate an IETF Remediation Instructions String object */
	DESTROY_IF(this->remediation_string);
	this->remediation_string = imv_remediation_string_create(
									TRUE, *lang_code);	/* TODO get os_type */

	this->remediation_string->add_instruction(this->remediation_string,
									instr_ports_title,
									instr_ports_descr,
									instr_ports_header,
									this->violating_ports);
	*string = this->remediation_string->get_encoding(this->remediation_string);
	*uri = lib->settings->get_str(lib->settings,
					"%s.plugins.imv-scanner.remediation_uri", NULL, lib->ns);

	return TRUE;
}

METHOD(imv_state_t, destroy, void,
	private_imv_scanner_state_t *this)
{
	DESTROY_IF(this->session);
	DESTROY_IF(this->reason_string);
	DESTROY_IF(this->remediation_string);
	DESTROY_IF(&this->port_filter_attr->pa_tnc_attribute);
	this->contracts->destroy(this->contracts);
	this->violating_ports->destroy_function(this->violating_ports, free);
	free(this);
}

METHOD(imv_scanner_state_t, set_handshake_state, void,
	private_imv_scanner_state_t *this, imv_scanner_handshake_state_t new_state)
{
	this->handshake_state = new_state;
}

METHOD(imv_scanner_state_t, get_handshake_state, imv_scanner_handshake_state_t,
	private_imv_scanner_state_t *this)
{
	return this->handshake_state;
}

METHOD(imv_scanner_state_t, set_port_filter_attr, void,
	private_imv_scanner_state_t *this, ietf_attr_port_filter_t *attr)
{
	DESTROY_IF(&this->port_filter_attr->pa_tnc_attribute);
	this->port_filter_attr = attr;
}

METHOD(imv_scanner_state_t, get_port_filter_attr, ietf_attr_port_filter_t*,
	private_imv_scanner_state_t *this)
{
	return this->port_filter_attr;
}

METHOD(imv_scanner_state_t, add_violating_port, void,
	private_imv_scanner_state_t *this, char *port)
{
	this->violating_ports->insert_last(this->violating_ports, port);
}

/**
 * Described in header.
 */
imv_state_t *imv_scanner_state_create(TNC_ConnectionID connection_id)
{
	private_imv_scanner_state_t *this;

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
				.get_session= _get_session,
				.get_contracts = _get_contracts,
				.change_state = _change_state,
				.get_recommendation = _get_recommendation,
				.set_recommendation = _set_recommendation,
				.update_recommendation = _update_recommendation,
				.get_reason_string = _get_reason_string,
				.get_remediation_instructions = _get_remediation_instructions,
				.destroy = _destroy,
			},
			.set_handshake_state = _set_handshake_state,
			.get_handshake_state = _get_handshake_state,
			.set_port_filter_attr = _set_port_filter_attr,
			.get_port_filter_attr = _get_port_filter_attr,
			.add_violating_port = _add_violating_port,
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
		.violating_ports = linked_list_create(),
	);

	return &this->public.interface;
}


