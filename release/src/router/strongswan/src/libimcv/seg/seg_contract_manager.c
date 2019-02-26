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

#include "seg_contract_manager.h"

typedef struct private_seg_contract_manager_t private_seg_contract_manager_t;

/**
 * Private data of a seg_contract_manager_t object.
 *
 */
struct private_seg_contract_manager_t {

	/**
	 * Public seg_contract_manager_t interface.
	 */
	seg_contract_manager_t public;

	/**
	 * List of PA-TNC segmentation contracts
	 */
	linked_list_t *contracts;

};

METHOD(seg_contract_manager_t, add_contract, void,
	private_seg_contract_manager_t *this, seg_contract_t *contract)
{
	this->contracts->insert_last(this->contracts, contract);
}

METHOD(seg_contract_manager_t, get_contract, seg_contract_t*,
	private_seg_contract_manager_t *this, pen_type_t msg_type, bool is_issuer,
	TNC_UInt32 id)
{
	enumerator_t *enumerator;
	seg_contract_t *contract, *found = NULL;

	enumerator = this->contracts->create_enumerator(this->contracts);
	while (enumerator->enumerate(enumerator, &contract))
	{
		if (contract->is_issuer(contract) == is_issuer &&
			pen_type_equals(contract->get_msg_type(contract), msg_type) &&
			id == (is_issuer ? contract->get_responder(contract) :
							   contract->get_issuer(contract)))
		{
			found = contract;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

METHOD(seg_contract_manager_t, destroy, void,
	private_seg_contract_manager_t *this)
{
	this->contracts->destroy_offset(this->contracts,
									offsetof(seg_contract_t, destroy));
	free(this);
}

/**
 * See header
 */
seg_contract_manager_t *seg_contract_manager_create(void)
{
	private_seg_contract_manager_t *this;

	INIT(this,
		.public = {
			.add_contract = _add_contract,
			.get_contract = _get_contract,
			.destroy = _destroy,
		},
		.contracts = linked_list_create(),
	);

	return &this->public;
}

