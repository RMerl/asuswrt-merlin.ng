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

#include "imc_hcd_state.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include <imc/imc_os_info.h>
#include <generic/generic_attr_bool.h>
#include <generic/generic_attr_chunk.h>
#include <generic/generic_attr_string.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include "ietf/ietf_attr_fwd_enabled.h"
#include <pwg/pwg_attr.h>
#include <pwg/pwg_attr_vendor_smi_code.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/debug.h>

/* IMC definitions */

static const char imc_name[] = "HCD";

static pen_type_t msg_types[] = {
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_SYSTEM },
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_CONSOLE },
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_MARKER },
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_FINISHER },
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_INTERFACE },
	{ PEN_PWG, PA_SUBTYPE_PWG_HCD_SCANNER }
};

static imc_agent_t *imc_hcd;
static imc_os_info_t *os;

typedef struct section_subtype_t section_subtype_t;

struct section_subtype_t {
	char *section;
	pa_subtype_pwg_t subtype;
};

static section_subtype_t section_subtypes[] = {
	{ "system",    PA_SUBTYPE_PWG_HCD_SYSTEM    },
	{ "console",   PA_SUBTYPE_PWG_HCD_CONSOLE   },
	{ "marker",    PA_SUBTYPE_PWG_HCD_MARKER    },
	{ "finisher",  PA_SUBTYPE_PWG_HCD_FINISHER  },
	{ "interface", PA_SUBTYPE_PWG_HCD_INTERFACE },
	{ "scanner"  , PA_SUBTYPE_PWG_HCD_SCANNER   }
};

typedef struct quadruple_t quadruple_t;

struct quadruple_t {
	char *section;
	pwg_attr_t name_attr;
	pwg_attr_t patches_attr;
	pwg_attr_t string_version_attr;
	pwg_attr_t version_attr;
};

static quadruple_t quadruples[] = {
	{ "firmware",
	   PWG_HCD_FIRMWARE_NAME, PWG_HCD_FIRMWARE_PATCHES,
	   PWG_HCD_FIRMWARE_STRING_VERSION, PWG_HCD_FIRMWARE_VERSION },
	{ "resident_application",
	   PWG_HCD_RESIDENT_APP_NAME, PWG_HCD_RESIDENT_APP_PATCHES,
	   PWG_HCD_RESIDENT_APP_STRING_VERSION, PWG_HCD_RESIDENT_APP_VERSION },
	{ "user_application",
	   PWG_HCD_USER_APP_NAME, PWG_HCD_USER_APP_PATCHES,
	   PWG_HCD_USER_APP_STRING_VERSION, PWG_HCD_USER_APP_VERSION }
};

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_Initialize(TNC_IMCID imc_id,
										  TNC_Version min_version,
										  TNC_Version max_version,
										  TNC_Version *actual_version)
{
	if (imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_hcd = imc_agent_create(imc_name, msg_types, countof(msg_types),
							  imc_id, actual_version);
	if (!imc_hcd)
	{
		return TNC_RESULT_FATAL;
	}

	os = imc_os_info_create();
	if (!os)
	{
		imc_hcd->destroy(imc_hcd);
		imc_hcd = NULL;

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
 * see section 3.8.2 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_NotifyConnectionChange(TNC_IMCID imc_id,
				TNC_ConnectionID connection_id, TNC_ConnectionState new_state)
{
	imc_state_t *state;

	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_hcd_state_create(connection_id);
			return imc_hcd->create_state(imc_hcd, state);
		case TNC_CONNECTION_STATE_DELETE:
			return imc_hcd->delete_state(imc_hcd, connection_id);
		default:
			return imc_hcd->change_state(imc_hcd, connection_id,
											 new_state, NULL);
	}
}

/**
 * Add AttributesNaturalLanguage attribute to send queue
 */
static void add_attrs_natural_lang(imc_msg_t *msg, char *section)
{
	pa_tnc_attr_t *attr;
	char *string;

	string = lib->settings->get_str(lib->settings,
				"%s.plugins.imc-hcd.subtypes.%s.attributes_natural_language",
				"en", lib->ns, section);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_ATTRS_NATURAL_LANG,
				string);
	attr = generic_attr_string_create(chunk_from_str(string),
				pen_type_create(PEN_PWG, PWG_HCD_ATTRS_NATURAL_LANG));
	msg->add_attribute(msg, attr);
}

/**
 * Add DefaultPasswordEnabled attribute to send queue
 */
static void add_default_pwd_enabled(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	bool status;

	status = os->get_default_pwd_status(os);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_DEFAULT_PWD_ENABLED,
				status ? "yes" : "no");
	attr = generic_attr_bool_create(status,
				pen_type_create(PEN_PWG, PWG_HCD_DEFAULT_PWD_ENABLED));
	msg->add_attribute(msg, attr);
}

/**
 * Add ForwardingEnabled attribute to send queue
 */
static void add_forwarding_enabled(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	os_fwd_status_t fwd_status;

	fwd_status = os->get_fwd_status(os);
	DBG2(DBG_IMC, "  %N: %N", pwg_attr_names, PWG_HCD_FORWARDING_ENABLED,
				os_fwd_status_names, fwd_status);
	attr = ietf_attr_fwd_enabled_create(fwd_status,
				pen_type_create(PEN_PWG, PWG_HCD_FORWARDING_ENABLED));
	msg->add_attribute(msg, attr);
}

/**
 * Add MachineTypeModel attribute to send queue
 */
static void add_machine_type_model(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	char *string;

	string = lib->settings->get_str(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.machine_type_model",
				"", lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_MACHINE_TYPE_MODEL,
				string);
	attr = generic_attr_string_create(chunk_from_str(string),
				pen_type_create(PEN_PWG, PWG_HCD_MACHINE_TYPE_MODEL));
	msg->add_attribute(msg, attr);
}

/**
 * Add PSTNFaxEnabled attribute to send queue
 */
static void add_pstn_fax_enabled(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	bool status;

	status = lib->settings->get_bool(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.pstn_fax_enabled",
				FALSE, lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_PSTN_FAX_ENABLED,
				status ? "yes" : "no");
	attr = generic_attr_bool_create(status,
				pen_type_create(PEN_PWG, PWG_HCD_PSTN_FAX_ENABLED));
	msg->add_attribute(msg, attr);
}

/**
 * Add TimeSource attribute to send queue
 */
static void add_time_source(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	char *string;

	string = lib->settings->get_str(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.time_source",
				"", lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_TIME_SOURCE,
				string);
	attr = generic_attr_string_create(chunk_from_str(string),
				pen_type_create(PEN_PWG, PWG_HCD_TIME_SOURCE));
	msg->add_attribute(msg, attr);
}

/**
 * Add UserApplicationEnabled attribute to send queue
 */
static void add_user_app_enabled(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	bool status;

	status = lib->settings->get_bool(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.user_application_enabled",
				FALSE, lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_USER_APP_ENABLED,
				status ? "yes" : "no");
	attr = generic_attr_bool_create(status,
				pen_type_create(PEN_PWG, PWG_HCD_USER_APP_ENABLED));
	msg->add_attribute(msg, attr);
}

/**
 * Add UserApplicationPersistenceEnabled attribute to send queue
 */
static void add_user_app_persist_enabled(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	bool status;

	status = lib->settings->get_bool(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.user_application_persistence.enabled",
				FALSE, lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_USER_APP_PERSIST_ENABLED,
				status ? "yes" : "no");
	attr = generic_attr_bool_create(status,
				pen_type_create(PEN_PWG, PWG_HCD_USER_APP_PERSIST_ENABLED));
	msg->add_attribute(msg, attr);
}

/**
 * Add VendorName attribute to send queue
 */
static void add_vendor_name(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	char *string;

	string = lib->settings->get_str(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.vendor_name",
				"", lib->ns);
	DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, PWG_HCD_VENDOR_NAME,
				string);
	attr = generic_attr_string_create(chunk_from_str(string),
				pen_type_create(PEN_PWG, PWG_HCD_VENDOR_NAME));
	msg->add_attribute(msg, attr);
}

/**
 * Add VendorSMICode attribute to send queue
 */
static void add_vendor_smi_code(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	int smi_code;

	smi_code = lib->settings->get_int(lib->settings,
				"%s.plugins.imc-hcd.subtypes.system.vendor_smi_code",
				0, lib->ns);
	DBG2(DBG_IMC, "  %N: 0x%06x (%d)", pwg_attr_names, PWG_HCD_VENDOR_SMI_CODE,
				smi_code, smi_code);
	attr = pwg_attr_vendor_smi_code_create(smi_code);
	msg->add_attribute(msg, attr);
}

/**
 * Add CertificationState attribute to send queue
 */
static void add_certification_state(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	char *hex_string;
	chunk_t blob;

	hex_string = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.system.certification_state",
					NULL, lib->ns);
	if (hex_string)
	{
		blob = chunk_from_hex(chunk_from_str(hex_string), NULL);

		DBG2(DBG_IMC, "  %N: %B", pwg_attr_names, PWG_HCD_CERTIFICATION_STATE,
					&blob);
		attr = generic_attr_chunk_create(blob,
					pen_type_create(PEN_PWG, PWG_HCD_CERTIFICATION_STATE));
		msg->add_attribute(msg, attr);
		chunk_free(&blob);
	}
}

/**
 * Add CertificationState attribute to send queue
 */
static void add_configuration_state(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	char *hex_string;
	chunk_t blob;

	hex_string = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.system.configuration_state",
					NULL, lib->ns);
	if (hex_string)
	{
		blob = chunk_from_hex(chunk_from_str(hex_string), NULL);

		DBG2(DBG_IMC, "  %N: %B", pwg_attr_names, PWG_HCD_CONFIGURATION_STATE,
					&blob);
		attr = generic_attr_chunk_create(blob,
					pen_type_create(PEN_PWG, PWG_HCD_CONFIGURATION_STATE));
		msg->add_attribute(msg, attr);
		chunk_free(&blob);
	}
}

/**
 * Add Correlated Attributes to send queue
 */
static void add_quadruple(imc_msg_t *msg, char *section, quadruple_t *quad)
{
	pa_tnc_attr_t *attr;
	const size_t version_len = 16;
	char version[version_len];
	char hex_version_default[] = "00000000000000000000000000000000";
	char *app, *name, *patches, *string_version, *hex_version;
	size_t len;
	chunk_t num_version;
	enumerator_t *enumerator;

	enumerator = lib->settings->create_section_enumerator(lib->settings,
					"%s.plugins.imc-hcd.subtypes.%s.%s",
					lib->ns, section, quad->section);
	while (enumerator->enumerate(enumerator, &app))
	{
		name = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.%s.%s.%s.name",
					"",	lib->ns, section, quad->section, app);
		patches = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.%s.%s.%s.patches",
					"", lib->ns, section, quad->section, app);
		string_version = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.%s.%s.%s.string_version",
					"",	lib->ns, section, quad->section, app);
		hex_version = lib->settings->get_str(lib->settings,
					"%s.plugins.imc-hcd.subtypes.%s.%s.%s.version",
					hex_version_default, lib->ns, section, quad->section, app);

		/* convert hex string into binary chunk */
		if (strlen(hex_version) > 2 * version_len)
		{
			hex_version = hex_version_default;
		}
		num_version = chunk_from_hex(chunk_from_str(hex_version), version);

		DBG2(DBG_IMC, "--- %s ---", app);

		DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, quad->name_attr, name);
		attr = generic_attr_string_create(chunk_from_str(name),
						pen_type_create(PEN_PWG, quad->name_attr));
		msg->add_attribute(msg, attr);

		/* remove any trailing LF from patches string for logging */
		len = strlen(patches);
		if (len && (patches[len - 1] == '\n'))
		{
			len--;
		}
		DBG2(DBG_IMC, "  %N:%s%.*s", pwg_attr_names, quad->patches_attr,
						len ? "\n" : " ", len, patches);
		attr = generic_attr_string_create(chunk_from_str(patches),
						pen_type_create(PEN_PWG, quad->patches_attr));
		msg->add_attribute(msg, attr);

		DBG2(DBG_IMC, "  %N: %s", pwg_attr_names, quad->string_version_attr,
						string_version);
		attr = generic_attr_string_create(chunk_from_str(string_version),
						pen_type_create(PEN_PWG, quad->string_version_attr));
		msg->add_attribute(msg, attr);

		DBG2(DBG_IMC, "  %N: %#B", pwg_attr_names, quad->version_attr, &num_version);
		attr = generic_attr_chunk_create(num_version,
						pen_type_create(PEN_PWG, quad->version_attr));
		msg->add_attribute(msg, attr);
	}
	enumerator->destroy(enumerator);
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
											  TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_msg_t *out_msg;
	TNC_Result result = TNC_RESULT_SUCCESS;
	pa_subtype_pwg_t subtype;
	pen_type_t msg_type;
	enumerator_t *enumerator;
	char *section;
	int i;

	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_hcd->get_state(imc_hcd, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	/* Enumerate over all HCD subtype sections */
	enumerator = lib->settings->create_section_enumerator(lib->settings,
						"%s.plugins.imc-hcd.subtypes", lib->ns);
	while (enumerator->enumerate(enumerator, &section) &&
		   result == TNC_RESULT_SUCCESS)
	{
		subtype = PA_SUBTYPE_PWG_HCD_UNKNOWN;

		for (i = 0; i < countof(section_subtypes); i++)
		{
			if (streq(section, section_subtypes[i].section))
			{
				subtype = section_subtypes[i].subtype;
				break;
			}
		}
		if (subtype == PA_SUBTYPE_PWG_HCD_UNKNOWN)
		{
			DBG1(DBG_IMC, "HCD subtype '%s' not supported", section);
			continue;
		}
		DBG2(DBG_IMC, "retrieving attributes for PA subtype %N/%N",
			 pen_names, PEN_PWG, pa_subtype_pwg_names, subtype);

		msg_type = pen_type_create(PEN_PWG, subtype);
		out_msg = imc_msg_create(imc_hcd, state, connection_id, imc_id,
								 TNC_IMVID_ANY, msg_type);

		/* mandatory attributes that are always sent without request */
		add_attrs_natural_lang(out_msg, section);
		if (subtype == PA_SUBTYPE_PWG_HCD_SYSTEM)
		{
			add_default_pwd_enabled(out_msg);
			add_forwarding_enabled(out_msg);
			add_machine_type_model(out_msg);
			add_pstn_fax_enabled(out_msg);
			add_time_source(out_msg);
			add_vendor_name(out_msg);
			add_vendor_smi_code(out_msg);
			add_user_app_enabled(out_msg);
			add_user_app_persist_enabled(out_msg);
		}
		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.imc-hcd.push_info", FALSE, lib->ns))
		{
			/* correlated attributes */
			for (i = 0; i < countof(quadruples); i++)
			{
				add_quadruple(out_msg, section, &quadruples[i]);
			}
		}

		/* send PA-TNC message with the excl flag not set */
		result = out_msg->send(out_msg, FALSE);
		out_msg->destroy(out_msg);
	}
	enumerator->destroy(enumerator);

	return result;
}

static TNC_Result receive_message(imc_state_t *state, imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t type, msg_type;
	TNC_Result result;
	char *section = NULL;
	int i;
	bool fatal_error = FALSE, pushed_info;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imc_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}
	msg_type = in_msg->get_msg_type(in_msg);

	for (i = 0; i < countof(section_subtypes); i++)
	{
		if (msg_type.type == section_subtypes[i].subtype)
		{
			section = section_subtypes[i].section;
			break;
		}
	}
	pushed_info = lib->settings->get_bool(lib->settings,
						"%s.plugins.imc-hcd.push_info", FALSE, lib->ns);

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		type = attr->get_type(attr);

		if (type.vendor_id == PEN_IETF)
		{
			if (type.type == IETF_ATTR_ATTRIBUTE_REQUEST)
			{
				ietf_attr_attr_request_t *attr_cast;
				pen_type_t *entry;
				enumerator_t *e;

				attr_cast = (ietf_attr_attr_request_t*)attr;

				e = attr_cast->create_enumerator(attr_cast);
				while (e->enumerate(e, &entry))
				{
					if (entry->vendor_id == PEN_PWG)
					{
						switch (entry->type)
						{
							case PWG_HCD_ATTRS_NATURAL_LANG:
								add_attrs_natural_lang(out_msg, section);
								break;
							case PWG_HCD_DEFAULT_PWD_ENABLED:
								add_default_pwd_enabled(out_msg);
								break;
							case PWG_HCD_FORWARDING_ENABLED:
								add_forwarding_enabled(out_msg);
								break;
							case PWG_HCD_MACHINE_TYPE_MODEL:
								add_machine_type_model(out_msg);
								break;
							case PWG_HCD_PSTN_FAX_ENABLED:
								add_pstn_fax_enabled(out_msg);
								break;
							case PWG_HCD_TIME_SOURCE:
								add_time_source(out_msg);
								break;
							case PWG_HCD_USER_APP_ENABLED:
								add_user_app_enabled(out_msg);
								break;
							case PWG_HCD_USER_APP_PERSIST_ENABLED:
								add_user_app_persist_enabled(out_msg);
								break;
							case PWG_HCD_VENDOR_NAME:
								add_vendor_name(out_msg);
								break;
							case PWG_HCD_VENDOR_SMI_CODE:
								add_vendor_smi_code(out_msg);
								break;
							case PWG_HCD_CERTIFICATION_STATE:
								add_certification_state(out_msg);
								break;
							case PWG_HCD_CONFIGURATION_STATE:
								add_configuration_state(out_msg);
								break;
							default:
								if (pushed_info)
								{
									continue;
								}
						}

						/* if not pushed, deliver on request */
						switch (entry->type)
						{
							case PWG_HCD_FIRMWARE_NAME:
								add_quadruple(out_msg, section, &quadruples[0]);
								break;
							case PWG_HCD_RESIDENT_APP_NAME:
								add_quadruple(out_msg, section, &quadruples[1]);
								break;
							case PWG_HCD_USER_APP_NAME:
								add_quadruple(out_msg, section, &quadruples[2]);
								break;
							default:
								break;
						}
					}
				}
				e->destroy(e);
			}
		}
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
TNC_Result TNC_IMC_API TNC_IMC_ReceiveMessage(TNC_IMCID imc_id,
											  TNC_ConnectionID connection_id,
											  TNC_BufferReference msg,
											  TNC_UInt32 msg_len,
											  TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_hcd->get_state(imc_hcd, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_data(imc_hcd, state, connection_id, msg_type,
									  chunk_create(msg, msg_len));
	result = receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_ReceiveMessageLong(TNC_IMCID imc_id,
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

	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_hcd->get_state(imc_hcd, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_hcd, state, connection_id,
								src_imv_id, dst_imc_id,msg_vid, msg_subtype,
								chunk_create(msg, msg_len));
	result =receive_message(state, in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.7 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_BatchEnding(TNC_IMCID imc_id,
										   TNC_ConnectionID connection_id)
{
	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.8 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_Terminate(TNC_IMCID imc_id)
{
	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_hcd->destroy(imc_hcd);
	imc_hcd = NULL;

	os->destroy(os);
	os = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_API TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
									TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_hcd)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_hcd->bind_functions(imc_hcd, bind_function);
}
