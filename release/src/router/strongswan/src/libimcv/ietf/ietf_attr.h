/*
 * Copyright (C) 2011-2018 Andreas Steffen
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

/**
 * @defgroup ietf_attr ietf_attr
 * @{ @ingroup libimcv
 */

#ifndef IETF_ATTR_H_
#define IETF_ATTR_H_

#include "pa_tnc/pa_tnc_attr.h"

#include <library.h>

typedef enum ietf_attr_t ietf_attr_t;

/**
 * IETF standard PA-TNC attribute types
 */
enum ietf_attr_t {

	/* RFC 5792 PA-TNC */
	IETF_ATTR_TESTING =                            0,
	IETF_ATTR_ATTRIBUTE_REQUEST =                  1,
	IETF_ATTR_PRODUCT_INFORMATION =                2,
	IETF_ATTR_NUMERIC_VERSION =                    3,
	IETF_ATTR_STRING_VERSION =                     4,
	IETF_ATTR_OPERATIONAL_STATUS =                 5,
	IETF_ATTR_PORT_FILTER =                        6,
	IETF_ATTR_INSTALLED_PACKAGES =                 7,
	IETF_ATTR_PA_TNC_ERROR =                       8,
	IETF_ATTR_ASSESSMENT_RESULT =                  9,
	IETF_ATTR_REMEDIATION_INSTRUCTIONS =          10,
	IETF_ATTR_FORWARDING_ENABLED =                11,
	IETF_ATTR_FACTORY_DEFAULT_PWD_ENABLED =       12,

	/* RFC 8412 SWIMA */
	IETF_ATTR_SWIMA_REQUEST =                     13,
	IETF_ATTR_SW_ID_INVENTORY =                   14,
	IETF_ATTR_SW_ID_EVENTS =                      15,
	IETF_ATTR_SW_INVENTORY =                      16,
	IETF_ATTR_SW_EVENTS =                         17,
	IETF_ATTR_SUBSCRIPTION_STATUS_REQ =           18,
	IETF_ATTR_SUBSCRIPTION_STATUS_RESP =          19,
	IETF_ATTR_SRC_METADATA_REQ =                  20,
	IETF_ATTR_SRC_METADATA_RESP =                 21,

	IETF_ATTR_RESERVED =                  0xffffffff,
};

/**
 * enum name for ietf_attr_t.
 */
extern enum_name_t *ietf_attr_names;

/**
 * Create an IETF PA-TNC attribute from data
 *
 * @param type				attribute type
 * @param length			attribute length
 * @param value				attribute value or segment
 */
pa_tnc_attr_t* ietf_attr_create_from_data(uint32_t type, size_t length,
										  chunk_t value);

#endif /** IETF_ATTR_H_ @}*/
