/*
 * Copyright (C) 2011 Andreas Steffen, HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup imv_test imv_test
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_test_state_t imv_test_state
 * @{ @ingroup imv_test
 */

#ifndef IMV_TEST_STATE_H_
#define IMV_TEST_STATE_H_

#include <imv/imv_state.h>
#include <library.h>

typedef struct imv_test_state_t imv_test_state_t;

/**
 * Internal state of an imv_test_t connection instance
 */
struct imv_test_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Add an IMC
	 *
	 * @param imc_id	ID of the IMC to be added
	 * @param rounds	number of re-measurement rounds
	 */
	void (*add_imc)(imv_test_state_t *this, TNC_UInt32 imc_id, int rounds);

	/**
	 * Set the IMC-IMV round-trip count
	 *
	 * @param rounds	number of re-measurement rounds
	 */
	void (*set_rounds)(imv_test_state_t *this, int rounds);

	/**
	 * Check and decrease IMC-IMV round-trip count
	 *
	 * @param imc_id	ID of the IMC to be checked
	 * @return			new connection state
	 */
	bool (*another_round)(imv_test_state_t *this, TNC_UInt32 imc_id);
};

/**
 * Create an imv_test_state_t instance
 *
 * @param id			connection ID
 */
imv_state_t* imv_test_state_create(TNC_ConnectionID id);

#endif /** IMV_TEST_STATE_H_ @}*/
