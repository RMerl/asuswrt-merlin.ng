/*
 * Copyright (C) 2012-14 Andreas Steffen
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
 * @defgroup ietf_attr_op_statust ietf_attr_op_status
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_OP_STATUS_H_
#define IETF_ATTR_OP_STATUS_H_

typedef struct ietf_attr_op_status_t ietf_attr_op_status_t;
typedef enum op_status_t op_status_t;
typedef enum op_result_t op_result_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Operational Status type
 */
enum op_status_t {
	OP_STATUS_UNKNOWN =       0,
	OP_STATUS_NOT_INSTALLED = 1,
	OP_STATUS_INSTALLED =     2,
	OP_STATUS_OPERATIONAL =   3,
	OP_STATUS_ROOF =          3
};

extern enum_name_t *op_status_names;

/**
 * Operational Result type
 */
enum op_result_t {
	OP_RESULT_UNKNOWN =      0,
	OP_RESULT_SUCCESSFUL =   1,
	OP_RESULT_ERRORED =      2,
	OP_RESULT_UNSUCCESSFUL = 3,
	OP_RESULT_ROOF         = 3
};

extern enum_name_t *op_result_names;

/**
 * Class implementing the IETF PA-TNC Operational Status attribute.
 *
 */
struct ietf_attr_op_status_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Operational Status
	 *
	 * @return				Operational Status
	 */
	u_int8_t (*get_status)(ietf_attr_op_status_t *this);

	/**
	 * Gets the Operational Result
	 *
	 * @return				Operational Result
	 */
	u_int8_t (*get_result)(ietf_attr_op_status_t *this);

	/**
	 * Gets the time of last use
	 *
	 * @return				Time of last use
	 */
	time_t (*get_last_use)(ietf_attr_op_status_t *this);
};

/**
 * Creates an ietf_attr_op_status_t object
 *
 * @param status			Operational Status
 * @param result			Operational Result
 * @param last_use			Time of last use
 */
pa_tnc_attr_t* ietf_attr_op_status_create(u_int8_t status, u_int8_t result,
										  time_t last_use);

/**
 * Creates an ietf_attr_op_status_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_op_status_create_from_data(size_t length,
													chunk_t value);

#endif /** IETF_ATTR_OP_STATUS_H_ @}*/
