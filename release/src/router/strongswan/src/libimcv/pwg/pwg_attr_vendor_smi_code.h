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

/**
 * @defgroup pwg_attr_vendor_smi_codet pwg_attr_vendor_smi_code
 * @{ @ingroup ietf_attr
 */

#ifndef PWG_ATTR_VENDOR_SMI_CODE_H_
#define PWG_ATTR_VENDOR_SMI_CODE_H_

typedef struct pwg_attr_vendor_smi_code_t pwg_attr_vendor_smi_code_t;

#include "pwg_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the PWG HCD PA-TNC Vendor SMI Code attribute.
 *
 */
struct pwg_attr_vendor_smi_code_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Vendor SMI Code
	 *
	 * @return				Vendor SMI Code
	 */
	pen_t (*get_vendor_smi_code)(pwg_attr_vendor_smi_code_t *this);

};

/**
 * Creates an pwg_attr_vendor_smi_code_t object
 *
 */
pa_tnc_attr_t* pwg_attr_vendor_smi_code_create(pen_t vendor_smi_code);

/**
 * Creates an pwg_attr_vendor_smi_code_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* pwg_attr_vendor_smi_code_create_from_data(size_t length,
														 chunk_t value);

#endif /** PWG_ATTR_VENDOR_SMI_CODE_H_ @}*/
