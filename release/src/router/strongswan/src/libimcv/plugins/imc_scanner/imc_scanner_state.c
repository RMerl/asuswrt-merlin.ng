/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "imc_scanner_state.h"

#include <tncif_names.h>

#include <utils/debug.h>

typedef struct private_imc_scanner_state_t private_imc_scanner_state_t;

/**
 * Private data of an imc_scanner_state_t object.
 */
struct private_imc_scanner_state_t {

	/**
	 * Public members of imc_scanner_state_t
	 */
	imc_scanner_state_t public;

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID connection_id;

	/**
	 * TNCCS connection state
	 */
	TNC_ConnectionState state;

	/**
	 * Assessment/Evaluation Result
	 */
	TNC_IMV_Evaluation_Result result;

	/**
	 * Does the TNCCS connection support long message types?
	 */
	bool has_long;

	/**
	 * Does the TNCCS connection support exclusive delivery?
	 */
	bool has_excl;

	/**
	 * Maximum PA-TNC message size for this TNCCS connection
	 */
	uint32_t max_msg_len;

	/**
	 * PA-TNC attribute segmentation contracts associated with TNCCS connection
	 */
	seg_contract_manager_t *contracts;
};

METHOD(imc_state_t, get_connection_id, TNC_ConnectionID,
	private_imc_scanner_state_t *this)
{
	return this->connection_id;
}

METHOD(imc_state_t, has_long, bool,
	private_imc_scanner_state_t *this)
{
	return this->has_long;
}

METHOD(imc_state_t, has_excl, bool,
	private_imc_scanner_state_t *this)
{
	return this->has_excl;
}

METHOD(imc_state_t, set_flags, void,
	private_imc_scanner_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imc_state_t, set_max_msg_len, void,
	private_imc_scanner_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imc_state_t, get_max_msg_len, uint32_t,
	private_imc_scanner_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imc_state_t, get_contracts, seg_contract_manager_t*,
	private_imc_scanner_state_t *this)
{
	return this->contracts;
}

METHOD(imc_state_t, change_state, TNC_ConnectionState,
	private_imc_scanner_state_t *this, TNC_ConnectionState new_state)
{
	TNC_ConnectionState old_state;

	old_state = this->state;
	this->state = new_state;
	return old_state;
}

METHOD(imc_state_t, set_result, void,
	private_imc_scanner_state_t *this, TNC_IMCID id,
	TNC_IMV_Evaluation_Result result)
{
	this->result = result;
}

METHOD(imc_state_t, get_result, bool,
	private_imc_scanner_state_t *this, TNC_IMCID id,
	TNC_IMV_Evaluation_Result *result)
{
	if (result)
	{
		*result = this->result;
	}
	return this->result != TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
}

METHOD(imc_state_t, reset, void,
	private_imc_scanner_state_t *this)
{
	this->result = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
}

METHOD(imc_state_t, destroy, void,
	private_imc_scanner_state_t *this)
{
	this->contracts->destroy(this->contracts);
	free(this);
}

/**
 * Described in header.
 */
imc_state_t *imc_scanner_state_create(TNC_ConnectionID connection_id)
{
	private_imc_scanner_state_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_connection_id = _get_connection_id,
				.has_long = _has_long,
				.has_excl = _has_excl,
				.set_flags = _set_flags,
				.set_max_msg_len = _set_max_msg_len,
				.get_max_msg_len = _get_max_msg_len,
				.get_contracts = _get_contracts,
				.change_state = _change_state,
				.set_result = _set_result,
				.get_result = _get_result,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.result = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
	);

	return &this->public.interface;
}


