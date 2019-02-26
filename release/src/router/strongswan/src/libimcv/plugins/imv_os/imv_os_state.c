/*
 * Copyright (C) 2012-2017 Andreas Steffen
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

#include "imv_os_state.h"

#include "imv/imv_lang_string.h"
#include "imv/imv_reason_string.h"
#include "imv/imv_remediation_string.h"
#include "imv/imv_os_info.h"

#include <tncif_policy.h>

#include <utils/debug.h>
#include <collections/linked_list.h>

typedef struct private_imv_os_state_t private_imv_os_state_t;
typedef struct package_entry_t package_entry_t;
typedef struct entry_t entry_t;
typedef struct instruction_entry_t instruction_entry_t;

/**
 * Private data of an imv_os_state_t object.
 */
struct private_imv_os_state_t {

	/**
	 * Public members of imv_os_state_t
	 */
	imv_os_state_t public;

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
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * IMV evaluation result
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * IMV OS handshake state
	 */
	imv_os_handshake_state_t handshake_state;

	/**
	 * List of blacklisted packages to be removed
	 */
	linked_list_t *remove_packages;

	/**
	 h* List of vulnerable packages to be updated
	 */
	linked_list_t *update_packages;

	/**
	 * TNC Reason String
	 */
	imv_reason_string_t *reason_string;

	/**
	 * IETF Remediation Instructions String
	 */
	imv_remediation_string_t *remediation_string;

	/**
	 * Number of processed packages
	 */
	int count;

	/**
	 * Number of vulnerable packages
	 */
	int count_security;

	/**
	 * Number of blacklisted packages
	 */
	int count_blacklist;

	/**
	 * Number of whitelisted packages
	 */
	int count_ok;

	/**
	 * OS Settings
	 */
	u_int os_settings;

	/**
	 * Number of installed packages still missing
	 */
	uint16_t missing;

};

/**
 * Supported languages
 */
static char* languages[] = { "en", "de", "pl" };

/**
 * Reason strings for "OS settings"
 */
static imv_lang_string_t reason_settings[] = {
	{ "en", "Improper OS settings were detected" },
	{ "de", "Unzulässige OS Einstellungen wurden festgestellt" },
	{ "pl", "Stwierdzono niewłaściwe ustawienia OS" },
	{ NULL, NULL }
};

/**
 * Reason strings for "installed software packages"
 */
static imv_lang_string_t reason_packages[] = {
	{ "en", "Vulnerable or blacklisted software packages were found" },
	{ "de", "Schwachstellenbehaftete oder gesperrte Softwarepakete wurden gefunden" },
	{ "pl", "Znaleziono pakiety podatne na atak lub będące na czarnej liście" },
	{ NULL, NULL }
};

/**
 * Instruction strings for "Software Security Updates"
 */
static imv_lang_string_t instr_update_packages_title[] = {
	{ "en", "Software Security Updates" },
	{ "de", "Software Sicherheitsupdates" },
	{ "pl", "Aktualizacja softwaru zabezpieczającego" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_update_packages_descr[] = {
	{ "en", "Packages with security vulnerabilities were found" },
	{ "de", "Softwarepakete mit Sicherheitsschwachstellen wurden gefunden" },
	{ "pl", "Znaleziono pakiety podatne na atak" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_update_packages_header[] = {
	{ "en", "Please update the following software packages:" },
	{ "de", "Bitte updaten Sie die folgenden Softwarepakete:" },
	{ "pl", "Proszę zaktualizować następujące pakiety:" },
	{ NULL, NULL }
};

/**
 * Instruction strings for "Blacklisted Software Packages"
 */
static imv_lang_string_t instr_remove_packages_title[] = {
	{ "en", "Blacklisted Software Packages" },
	{ "de", "Gesperrte Softwarepakete" },
	{ "pl", "Pakiety będące na czarnej liście" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_remove_packages_descr[] = {
	{ "en", "Dangerous software packages were found" },
	{ "de", "Gefährliche Softwarepakete wurden gefunden" },
	{ "pl", "Znaleziono niebezpieczne pakiety" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_remove_packages_header[] = {
	{ "en", "Please remove the following software packages:" },
	{ "de", "Bitte entfernen Sie die folgenden Softwarepakete:" },
	{ "pl", "Proszę usunąć następujące pakiety:" },
	{ NULL, NULL }
};

;/**
 * Instruction strings for "Forwarding Enabled"
 */
static imv_lang_string_t instr_fwd_enabled_title[] = {
	{ "en", "IP Packet Forwarding" },
	{ "de", "Weiterleitung von IP Paketen" },
	{ "pl", "Przekazywanie pakietów IP" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_fwd_enabled_descr[] = {
	{ "en", "Please disable the forwarding of IP packets" },
	{ "de", "Bitte deaktivieren Sie das Forwarding von IP Paketen" },
	{ "pl", "Proszę zdezaktywować przekazywanie pakietów IP" },
	{ NULL, NULL }
};

/**
 * Instruction strings for "Default Password Enabled"
 */
static imv_lang_string_t instr_default_pwd_enabled_title[] = {
	{ "en", "Default Password" },
	{ "de", "Default Passwort" },
	{ "pl", "Hasło domyślne" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_default_pwd_enabled_descr[] = {
	{ "en", "Please change the default password" },
	{ "de", "Bitte ändern Sie das Default Passwort" },
	{ "pl", "Proszę zmienić domyślne hasło" },
	{ NULL, NULL }
};

/**
 * Instruction strings for  "Unknown Source"
 */
static imv_lang_string_t instr_unknown_source_title[] = {
	{ "en", "Unknown Software Origin" },
	{ "de", "Unbekannte Softwareherkunft" },
	{ "pl", "Nieznane pochodzenie softwaru" },
	{ NULL, NULL }
};

static imv_lang_string_t instr_unknown_source_descr[] = {
	{ "en", "Do not allow the installation of apps from unknown sources" },
	{ "de", "Erlauben Sie nicht die Installation von Apps aus unbekannten Quellen" },
	{ "pl", "Proszę nie dopuszczać do instalacji Apps z nieznanych źródeł" },
	{ NULL, NULL }
};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_os_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_os_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_os_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_os_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_os_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_os_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_action_flags, void,
	private_imv_os_state_t *this, uint32_t flags)
{
	this->action_flags |= flags;
}

METHOD(imv_state_t, get_action_flags, uint32_t,
	private_imv_os_state_t *this)
{
	return this->action_flags;
}

METHOD(imv_state_t, set_session, void,
	private_imv_os_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_os_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_os_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_os_state_t *this, TNC_IMV_Action_Recommendation *rec,
								  TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_os_state_t *this, TNC_IMV_Action_Recommendation rec,
								  TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_os_state_t *this, TNC_IMV_Action_Recommendation rec,
								  TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_state_t, change_state, TNC_ConnectionState,
	private_imv_os_state_t *this, TNC_ConnectionState new_state)
{
	TNC_ConnectionState old_state;

	old_state = this->state;
	this->state = new_state;
	return old_state;
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_os_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	if (!this->count_security && !this->count_blacklist & !this->os_settings)
	{
		return FALSE;
	}
	*reason_language = imv_lang_string_select_lang(language_enumerator,
											  languages, countof(languages));

	/* Instantiate a TNC Reason String object */
	DESTROY_IF(this->reason_string);
	this->reason_string = imv_reason_string_create(*reason_language, "\n");

	if (this->count_security || this->count_blacklist)
	{
		this->reason_string->add_reason(this->reason_string, reason_packages);
	}
	if (this->os_settings)
	{
		this->reason_string->add_reason(this->reason_string, reason_settings);
	}
	*reason_string = this->reason_string->get_encoding(this->reason_string);

	return TRUE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_os_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	imv_os_info_t *os_info;
	bool as_xml = FALSE;

	if (!this->count_security && !this->count_blacklist & !this->os_settings)
	{
		return FALSE;
	}
	*lang_code = imv_lang_string_select_lang(language_enumerator,
										languages, countof(languages));

	/* Instantiate an IETF Remediation Instructions String object */
	DESTROY_IF(this->remediation_string);
	if (this->session)
	{
		os_info = this->session->get_os_info(this->session);
		as_xml = os_info->get_type(os_info) == OS_TYPE_ANDROID;
	}
	this->remediation_string = imv_remediation_string_create(as_xml, *lang_code);

	/* List of blacklisted packages to be removed, if any */
	if (this->count_blacklist)
	{
		this->remediation_string->add_instruction(this->remediation_string,
							instr_remove_packages_title,
							instr_remove_packages_descr,
							instr_remove_packages_header,
							this->remove_packages);
	}

	/* List of packages in need of an update, if any */
	if (this->count_security)
	{
		this->remediation_string->add_instruction(this->remediation_string,
							instr_update_packages_title,
							instr_update_packages_descr,
							instr_update_packages_header,
							this->update_packages);
	}

	/* Add instructions concerning improper OS settings */
	if (this->os_settings & OS_SETTINGS_FWD_ENABLED)
	{
		this->remediation_string->add_instruction(this->remediation_string,
							instr_fwd_enabled_title,
							instr_fwd_enabled_descr, NULL, NULL);
	}
	if (this->os_settings & OS_SETTINGS_DEFAULT_PWD_ENABLED)
	{
		this->remediation_string->add_instruction(this->remediation_string,
							instr_default_pwd_enabled_title,
							instr_default_pwd_enabled_descr, NULL, NULL);
	}
	if (this->os_settings & OS_SETTINGS_UNKNOWN_SOURCE)
	{
		this->remediation_string->add_instruction(this->remediation_string,
							instr_unknown_source_title,
							instr_unknown_source_descr, NULL, NULL);
	}

	*string = this->remediation_string->get_encoding(this->remediation_string);
	*uri = lib->settings->get_str(lib->settings,
							"%s.plugins.imv-os.remediation_uri", NULL, lib->ns);

	return TRUE;
}

METHOD(imv_state_t, reset, void,
	private_imv_os_state_t *this)
{
	DESTROY_IF(this->reason_string);
	DESTROY_IF(this->remediation_string);
	this->reason_string = NULL;
	this->remediation_string = NULL;
	this->rec  = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
	this->eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;

	this->action_flags = 0;

	this->handshake_state = IMV_OS_STATE_INIT;
	this->count = 0;
	this->count_security = 0;
	this->count_blacklist = 0;
	this->count_ok = 0;
	this->os_settings = 0;
	this->missing = 0;

	this->update_packages->destroy_function(this->update_packages, free);
	this->remove_packages->destroy_function(this->remove_packages, free);
	this->update_packages = linked_list_create();
	this->remove_packages = linked_list_create();
}

METHOD(imv_state_t, destroy, void,
	private_imv_os_state_t *this)
{
	DESTROY_IF(this->session);
	DESTROY_IF(this->reason_string);
	DESTROY_IF(this->remediation_string);
	this->contracts->destroy(this->contracts);
	this->update_packages->destroy_function(this->update_packages, free);
	this->remove_packages->destroy_function(this->remove_packages, free);
	free(this);
}

METHOD(imv_os_state_t, set_handshake_state, void,
	private_imv_os_state_t *this, imv_os_handshake_state_t new_state)
{
	this->handshake_state = new_state;
}

METHOD(imv_os_state_t, get_handshake_state, imv_os_handshake_state_t,
	private_imv_os_state_t *this)
{
	return this->handshake_state;
}


METHOD(imv_os_state_t, set_count, void,
	private_imv_os_state_t *this, int count, int count_security,
	int count_blacklist, int count_ok)
{
	this->count           += count;
	this->count_security  += count_security;
	this->count_blacklist += count_blacklist;
	this->count_ok        += count_ok;
}

METHOD(imv_os_state_t, get_count, void,
	private_imv_os_state_t *this, int *count, int *count_security,
	int *count_blacklist, int *count_ok)
{
	if (count)
	{
		*count = this->count;
	}
	if (count_security)
	{
		*count_security = this->count_security;
	}
	if (count_blacklist)
	{
		*count_blacklist = this->count_blacklist;
	}
	if (count_ok)
	{
		*count_ok = this->count_ok;
	}
}

METHOD(imv_os_state_t, set_os_settings, void,
	private_imv_os_state_t *this, u_int settings)
{
	this->os_settings |= settings;
}

METHOD(imv_os_state_t, get_os_settings, u_int,
	private_imv_os_state_t *this)
{
	return this->os_settings;
}

METHOD(imv_os_state_t, set_missing, void,
	private_imv_os_state_t *this, uint16_t missing)
{
	this->missing = missing;
}

METHOD(imv_os_state_t, get_missing, uint16_t,
	private_imv_os_state_t *this)
{
	return this->missing;
}

METHOD(imv_os_state_t, add_bad_package, void,
	private_imv_os_state_t *this, char *package,
	os_package_state_t package_state)
{
	package = strdup(package);

	if (package_state == OS_PACKAGE_STATE_BLACKLIST)
	{
		this->remove_packages->insert_last(this->remove_packages, package);
	}
	else
	{
		this->update_packages->insert_last(this->update_packages, package);
	}
}

/**
 * Described in header.
 */
imv_state_t *imv_os_state_create(TNC_ConnectionID connection_id)
{
	private_imv_os_state_t *this;

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
				.reset = _reset,
				.destroy = _destroy,
			},
			.set_handshake_state = _set_handshake_state,
			.get_handshake_state = _get_handshake_state,
			.set_count = _set_count,
			.get_count = _get_count,
			.set_os_settings = _set_os_settings,
			.get_os_settings = _get_os_settings,
			.set_missing = _set_missing,
			.get_missing = _get_missing,
			.add_bad_package = _add_bad_package,
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
		.update_packages = linked_list_create(),
		.remove_packages = linked_list_create(),
	);

	return &this->public.interface;
}


