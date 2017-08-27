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
 * @defgroup seg_contract_manager seg_contract_manager
 * @{ @ingroup libimcv_seg
 */

#ifndef SEG_CONTRACT_MANAGER_H_
#define SEG_CONTRACT_MANAGER_H_

typedef struct seg_contract_manager_t seg_contract_manager_t;

#include "seg_contract.h"

/**
 * Interface for a PA-TNC attribute segmentation contract manager
 *
 */
struct seg_contract_manager_t {

	/**
	 * Add segmentation contract
	 *
	 * @param contract			Segmentation contract to be added
	 */
	void (*add_contract)(seg_contract_manager_t *this, seg_contract_t *contract);

	/**
	 * Get segmentation contract
	 *
	 * @param msg_type			PA-TNC message type governed by contract
	 * @param is_issuer			If TRUE get only issuer contracts
	 * @param id				Match either issuer or responder ID
	 */
	seg_contract_t* (*get_contract)(seg_contract_manager_t *this,
									pen_type_t msg_type, bool is_issuer,
									TNC_UInt32 id);

	/**
	 * Destroys a seg_contract_manager_t object.
	 */
	void (*destroy)(seg_contract_manager_t *this);
};

/**
 * Create a PA-TNC attribute segmentation contract manager
 */
seg_contract_manager_t* seg_contract_manager_create();

#endif /** SEG_CONTRACT_MANAGER_H_ @}*/
