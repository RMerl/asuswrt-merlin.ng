/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup seg_contract seg_contract
 * @{ @ingroup libimcv_seg
 */

#ifndef SEG_CONTRACT_H_
#define SEG_CONTRACT_H_

typedef struct seg_contract_t seg_contract_t;

#include "pa_tnc/pa_tnc_attr.h"

#include <library.h>
#include <pen/pen.h>

#include <tncif.h>

#define SEG_CONTRACT_MAX_SIZE_VALUE		0xffffffff
#define SEG_CONTRACT_NO_FRAGMENTATION	SEG_CONTRACT_MAX_SIZE_VALUE

/**
 * Interface for a PA-TNC attribute segmentation contract
 *
 */
struct seg_contract_t {

	/**
	 * Get the PA-TNC message type.
	 *
	 * @return					PA-TNC Message type
	 */
	pen_type_t (*get_msg_type)(seg_contract_t *this);

	/**
	 * Set maximum PA-TNC attribute and segment size in octets
	 *
	 * @param max_attr_size	Maximum PA-TNC attribute size in octets
	 * @param max_seg_size	Maximum PA-TNC attribute segment size in octets
	 */
	void (*set_max_size)(seg_contract_t *this, uint32_t max_attr_size,
											   uint32_t max_seg_size);

	/**
	 * Get maximum PA-TNC attribute and segment size in octets
	 *
	 * @param max_attr_size	Maximum PA-TNC attribute size in octets
	 * @param max_seg_size	Maximum PA-TNC attribute segment size in octets
	 */
	void (*get_max_size)(seg_contract_t *this, uint32_t *max_attr_size,
											   uint32_t *max_seg_size);

	/**
	 * Check if a PA-TNC attribute must be segmented or is oversized
	 *
	 * @param attr			PA-TNC attribute to be checked
	 * @param oversize		PA-TNC attribute is larger than maximum size
	 * @return				TRUE if PA-TNC attribute must be segmented
	 */
	bool (*check_size)(seg_contract_t *this, pa_tnc_attr_t *attr,
											 bool *oversize);

	/**
	 * Generate first segment of a PA-TNC attribute according to the contract
	 *
	 * @param attr			PA-TNC attribute to be segmented
	 * @return				First segment envelope attribute
	 */
	pa_tnc_attr_t* (*first_segment)(seg_contract_t *this, pa_tnc_attr_t *attr);

	/**
	 * Generate next segment of a PA-TNC attribute according to the contract
	 *
	 * @param base_attr_id	Base Attribute ID
	 * @return				Next segment envelope attribute
	 */
	pa_tnc_attr_t* (*next_segment)(seg_contract_t *this, uint32_t base_attr_id);

	/**
	 * Add an attribute segments until the PA-TNC attribute is reconstructed
	 *
	 * @param attr			Segment envelope attribute
	 * @param error			Error attribute if an error occurred or NULL
	 * @param more			Need more segments
	 * @return				Completed PA-TNC attribute or NULL
	 */
	pa_tnc_attr_t* (*add_segment)(seg_contract_t *this,
								 pa_tnc_attr_t *attr, pa_tnc_attr_t **error,
								 bool *more);

	/**
	 * Get contract role
	 *
	 * @return				TRUE:  contracting party (issuer),
	 *						FALSE: contracted party
	 */
	bool (*is_issuer)(seg_contract_t *this);

	/**
	 * Is this a null contract ?
	 *
	 * @return				TRUE if null contract
	 */
	bool (*is_null)(seg_contract_t *this);

	/**
	 * Set the responder ID
	 *
	 * @param responder		IMC or IMV ID of responder
	 */
	void (*set_responder)(seg_contract_t *this, TNC_UInt32 responder);

	/**
	 * Get the responder ID
	 *
	 * @return				IMC or IMV ID of responder
	 */
	TNC_UInt32 (*get_responder)(seg_contract_t *this);

	/**
	 * Get the issuer ID
	 *
	 * @return				IMC or IMV ID of issuer
	 */
	TNC_UInt32 (*get_issuer)(seg_contract_t *this);

	/**
	 * Clone a contract
	 *
	 * @return				Cloned contract
	 */
	seg_contract_t* (*clone)(seg_contract_t *this);

	/**
	 * Get an info string about the contract
	 *
	 * @param buf			String buffer of at least size len
	 * @param len			Size of string buffer
	 * @param request		TRUE if contract request, FALSE if response
	 */
	void (*get_info_string)(seg_contract_t *this, char *buf, size_t len,
							bool request);

	/**
	 * Destroys a seg_contract_t object.
	 */
	void (*destroy)(seg_contract_t *this);
};

/**
 * Create a PA-TNC attribute segmentation contract
 *
 * @param msg_type			PA-TNC message type
 * @param max_attr_size		Maximum PA-TNC attribute size in octets
 * @param max_seg_size		Maximum PA-TNC attribute segment size in octets
 * @param is_issuer			TRUE if issuer of the contract
 * @param issuer_id			IMC or IMV ID of issuer
 * @param is_imc			TRUE if IMC, FALSE if IMV
 */
seg_contract_t* seg_contract_create(pen_type_t msg_type,
									uint32_t max_attr_size,
									uint32_t max_seg_size,
									bool is_issuer, TNC_UInt32 issuer_id,
									bool is_imc);

#endif /** SEG_CONTRACT_H_ @}*/
