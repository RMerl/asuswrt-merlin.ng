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

#include "pwg_attr.h"

#include "generic/generic_attr_bool.h"
#include "generic/generic_attr_chunk.h"
#include "generic/generic_attr_string.h"
#include "ietf/ietf_attr_fwd_enabled.h"
#include "ietf/ietf_attr_port_filter.h"
#include "pwg/pwg_attr_vendor_smi_code.h"

ENUM_BEGIN(pwg_attr_names,	PWG_HCD_ATTRS_NATURAL_LANG,
							PWG_HCD_VENDOR_SMI_CODE,
	"HCD AttributesNaturalLanguage",
	"HCD MachineTypeModel",
	"HCD VendorName",
	"HCD VendorSMICode");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_DEFAULT_PWD_ENABLED,
							PWG_HCD_FORWARDING_ENABLED,
							PWG_HCD_VENDOR_SMI_CODE,
	"HCD DefaultPasswordEnabled",
	"HCD FirewallSetting",
	"HCD ForwardingEnabled");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_PSTN_FAX_ENABLED,
							PWG_HCD_PSTN_FAX_ENABLED,
							PWG_HCD_FORWARDING_ENABLED,
	"HCD PSTNFaxEnabled");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_TIME_SOURCE,
							PWG_HCD_TIME_SOURCE,
							PWG_HCD_PSTN_FAX_ENABLED,
	"HCD TimeSource");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_FIRMWARE_NAME,
							PWG_HCD_FIRMWARE_VERSION,
							PWG_HCD_TIME_SOURCE,
	"HCD FirmwareName",
	"HCD FirmwarePatches",
	"HCD FirmwareStringVersion",
	"HCD FirmwareVersion");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_RESIDENT_APP_NAME,
							PWG_HCD_RESIDENT_APP_VERSION,
							PWG_HCD_FIRMWARE_VERSION,
	"HCD ResidentApplicationName",
	"HCD ResidentApplicationPatches",
	"HCD ResidentApplicationStringVersion",
	"HCD ResidentApplicationVersion");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_USER_APP_NAME,
							PWG_HCD_USER_APP_PERSIST_ENABLED,
							PWG_HCD_RESIDENT_APP_VERSION,
	"HCD UserApplicationName",
	"HCD UserApplicationPatches",
	"HCD UserApplicationStringVersion",
	"HCD UserApplicationVersion",
	"HCD UserApplicationEnabled",
	"HCD UserApplicationPersistenceEnabled");
ENUM_NEXT(pwg_attr_names,	PWG_HCD_CERTIFICATION_STATE,
							PWG_HCD_CONFIGURATION_STATE,
							PWG_HCD_USER_APP_PERSIST_ENABLED,
	"HCD CertificationState",
	"HCD ConfigurationState");
ENUM_END(pwg_attr_names,	PWG_HCD_CONFIGURATION_STATE);

/**
 * See header
 */
pa_tnc_attr_t* pwg_attr_create_from_data(uint32_t type, size_t length, chunk_t value)
{
	switch (type)
	{
		case PWG_HCD_DEFAULT_PWD_ENABLED:
		case PWG_HCD_USER_APP_ENABLED:
		case PWG_HCD_USER_APP_PERSIST_ENABLED:
		case PWG_HCD_PSTN_FAX_ENABLED:
			return generic_attr_bool_create_from_data(length, value,
									pen_type_create(PEN_PWG, type));
		case PWG_HCD_ATTRS_NATURAL_LANG:
		case PWG_HCD_MACHINE_TYPE_MODEL:
		case PWG_HCD_VENDOR_NAME:
		case PWG_HCD_FIRMWARE_NAME:
		case PWG_HCD_FIRMWARE_PATCHES:
		case PWG_HCD_FIRMWARE_STRING_VERSION:
		case PWG_HCD_TIME_SOURCE:
		case PWG_HCD_USER_APP_NAME:
		case PWG_HCD_USER_APP_PATCHES:
		case PWG_HCD_USER_APP_STRING_VERSION:
		case PWG_HCD_RESIDENT_APP_NAME:
		case PWG_HCD_RESIDENT_APP_PATCHES:
		case PWG_HCD_RESIDENT_APP_STRING_VERSION:
			return generic_attr_string_create_from_data(length, value,
									pen_type_create(PEN_PWG, type));
		case PWG_HCD_FIRMWARE_VERSION:
		case PWG_HCD_RESIDENT_APP_VERSION:
		case PWG_HCD_USER_APP_VERSION:
			return generic_attr_chunk_create_from_data(length, value, 16,
									pen_type_create(PEN_PWG, type));
		case PWG_HCD_CERTIFICATION_STATE:
		case PWG_HCD_CONFIGURATION_STATE:
			return generic_attr_chunk_create_from_data(length, value, 0,
									pen_type_create(PEN_PWG, type));
		case PWG_HCD_VENDOR_SMI_CODE:
			return pwg_attr_vendor_smi_code_create_from_data(length, value);
		case PWG_HCD_FORWARDING_ENABLED:
			return ietf_attr_fwd_enabled_create_from_data(length, value,
									pen_type_create(PEN_PWG, type));
		case PWG_HCD_FIREWALL_SETTING:
			return ietf_attr_port_filter_create_from_data(length, value,
									pen_type_create(PEN_PWG, type));
		default:
			return NULL;
	}
}
