/*
 * Copyright (C) 2011-2018 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "ietf_attr.h"
#include "ietf/ietf_attr_assess_result.h"
#include "ietf/ietf_attr_attr_request.h"
#include "ietf/ietf_attr_fwd_enabled.h"
#include "ietf/ietf_attr_installed_packages.h"
#include "ietf/ietf_attr_numeric_version.h"
#include "ietf/ietf_attr_op_status.h"
#include "ietf/ietf_attr_pa_tnc_error.h"
#include "ietf/ietf_attr_port_filter.h"
#include "ietf/ietf_attr_product_info.h"
#include "ietf/ietf_attr_remediation_instr.h"
#include "ietf/ietf_attr_string_version.h"
#include "ietf/swima/ietf_swima_attr_req.h"
#include "ietf/swima/ietf_swima_attr_sw_inv.h"
#include "ietf/swima/ietf_swima_attr_sw_ev.h"
#include "generic/generic_attr_bool.h"

ENUM(ietf_attr_names, IETF_ATTR_TESTING, IETF_ATTR_SRC_METADATA_RESP,
	"Testing",
	"Attribute Request",
	"Product Information",
	"Numeric Version",
	"String Version",
	"Operational Status",
	"Port Filter",
	"Installed Packages",
	"PA-TNC Error",
	"Assessment Result",
	"Remediation Instructions",
	"Forwarding Enabled",
	"Factory Default Password Enabled",
	"SWIMA Request",
	"SW Identifier Inventory",
	"SW Identifier Events",
	"SW Inventory",
	"SW Events",
	"SW Subscription Status Request",
	"SW Subscription Status Response",
	"SW Source Metadata Request",
	"SW Source Metadata Response",
);

/**
 * See header
 */
pa_tnc_attr_t* ietf_attr_create_from_data(uint32_t type, size_t length,
										  chunk_t value)
{
	switch (type)
	{
		case IETF_ATTR_ATTRIBUTE_REQUEST:
			return ietf_attr_attr_request_create_from_data(length, value);
		case IETF_ATTR_PRODUCT_INFORMATION:
			return ietf_attr_product_info_create_from_data(length, value);
		case IETF_ATTR_NUMERIC_VERSION:
			return ietf_attr_numeric_version_create_from_data(length, value);
		case IETF_ATTR_STRING_VERSION:
			return ietf_attr_string_version_create_from_data(length, value);
		case IETF_ATTR_OPERATIONAL_STATUS:
			return ietf_attr_op_status_create_from_data(length, value);
		case IETF_ATTR_PORT_FILTER:
			return ietf_attr_port_filter_create_from_data(length, value,
									pen_type_create(PEN_IETF, type));
		case IETF_ATTR_INSTALLED_PACKAGES:
			return ietf_attr_installed_packages_create_from_data(length, value);
		case IETF_ATTR_PA_TNC_ERROR:
			return ietf_attr_pa_tnc_error_create_from_data(length, value);
		case IETF_ATTR_ASSESSMENT_RESULT:
			return ietf_attr_assess_result_create_from_data(length, value);
		case IETF_ATTR_REMEDIATION_INSTRUCTIONS:
			return ietf_attr_remediation_instr_create_from_data(length, value);
		case IETF_ATTR_FORWARDING_ENABLED:
			return ietf_attr_fwd_enabled_create_from_data(length, value,
									pen_type_create(PEN_IETF, type));
		case IETF_ATTR_FACTORY_DEFAULT_PWD_ENABLED:
			return generic_attr_bool_create_from_data(length, value,
									pen_type_create(PEN_IETF, type));
		case IETF_ATTR_SWIMA_REQUEST:
			return ietf_swima_attr_req_create_from_data(length, value);
		case IETF_ATTR_SW_ID_INVENTORY:
			return ietf_swima_attr_sw_inv_create_from_data(length, value, TRUE);
		case IETF_ATTR_SW_INVENTORY:
			return ietf_swima_attr_sw_inv_create_from_data(length, value, FALSE);
		case IETF_ATTR_SW_ID_EVENTS:
			return ietf_swima_attr_sw_ev_create_from_data(length, value, TRUE);
		case IETF_ATTR_SW_EVENTS:
			return ietf_swima_attr_sw_ev_create_from_data(length, value, FALSE);
		case IETF_ATTR_TESTING:
		case IETF_ATTR_RESERVED:
		/* unsupported IETF/SWIMA attributes */
		case IETF_ATTR_SUBSCRIPTION_STATUS_REQ:
		case IETF_ATTR_SUBSCRIPTION_STATUS_RESP:
		case IETF_ATTR_SRC_METADATA_REQ:
		case IETF_ATTR_SRC_METADATA_RESP:
		default:
			return NULL;
	}
}
