/*
 * Copyright (C) 2012-2014 Andreas Steffen
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
 * @defgroup ietf_attr_remediation_instrt ietf_attr_remediation_instr
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_REMEDIATION_INSTR_H_
#define IETF_ATTR_REMEDIATION_INSTR_H_

typedef struct ietf_attr_remediation_instr_t ietf_attr_remediation_instr_t;
typedef enum ietf_remediation_parameters_t ietf_remediation_parameters_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

enum ietf_remediation_parameters_t {
	IETF_REMEDIATION_PARAMETERS_URI =    1,
	IETF_REMEDIATION_PARAMETERS_STRING = 2
};

/**
 * Class implementing the IETF PA-TNC Remediation Instructions attribute.
 *
 */
struct ietf_attr_remediation_instr_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get the Remediation Parameters Type (Vendor ID and Type)
	 *
	 * @return				Remediation Parameters Type
	 */
	pen_type_t (*get_parameters_type)(ietf_attr_remediation_instr_t *this);

	/**
	 * Get the Remediation Parameters
	 *
	 * @return				Remediation Parameters
	 */
	chunk_t (*get_parameters)(ietf_attr_remediation_instr_t *this);

	/**
	 * Get the Remediation URI
	 *
	 * @return				Remediation URI
	 */
	chunk_t (*get_uri)(ietf_attr_remediation_instr_t *this);

	/**
	 * Get the Remediation String
	 *
	 * @param lang_code		Optional Language Code
	 * @return				Remediation String
	 */
	chunk_t (*get_string)(ietf_attr_remediation_instr_t *this,
						  chunk_t *lang_code);
};

/**
 * Creates a general ietf_attr_remediation_instr_t object
 *
 * @param parameters_type	Remediation Parameters Type
 * @param parameters		Remediation Parameters
 */
pa_tnc_attr_t* ietf_attr_remediation_instr_create(pen_type_t parameters_type,
												  chunk_t parameters);

/**
 * Creates an ietf_attr_remediation_instr_t object of Remediation URI Type
 *
 * @param uri				Remediation URI
 */
pa_tnc_attr_t* ietf_attr_remediation_instr_create_from_uri(chunk_t uri);

/**
 * Creates an ietf_attr_remediation_instr_t object of Remediation String Type
 *
 * @param string			Remediation String
 * @param lang_code			Remediation String Language Code
 */
pa_tnc_attr_t* ietf_attr_remediation_instr_create_from_string(chunk_t string,
															  chunk_t lang_code);

/**
 * Creates an ietf_attr_remediation_instr_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_remediation_instr_create_from_data(size_t length,
															chunk_t value);

#endif /** IETF_ATTR_REMEDIATION_INSTR_H_ @}*/
