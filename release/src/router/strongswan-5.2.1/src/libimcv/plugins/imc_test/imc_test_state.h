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
 * @defgroup imc_test imc_test
 * @ingroup libimcv_plugins
 *
 * @defgroup imc_test_state_t imc_test_state
 * @{ @ingroup imc_test
 */

#ifndef IMC_TEST_STATE_H_
#define IMC_TEST_STATE_H_

#include <tncifimc.h>
#include <imc/imc_state.h>
#include <library.h>

typedef struct imc_test_state_t imc_test_state_t;

/**
 * Internal state of an imc_test_t connection instance
 */
struct imc_test_state_t {

	/**
	 * imc_state_t interface
	 */
	imc_state_t interface;

	/**
	 * get the command to send to IMV
	 *
	 * @return				commmand to send to IMV
	 */
	char* (*get_command)(imc_test_state_t *this);

	/**
	 * set the command to send to IMV
	 *
	 * @param command		commmand to send to IMV
	 */
	void (*set_command)(imc_test_state_t *this, char *command);

	/**
	 * get the value size of a dummy attribute to send to IMV
	 *
	 * @return				size of the dummy attribute value to send to IMV
	 */
	int (*get_dummy_size)(imc_test_state_t *this);

	/**
	 * Test and reset the first handshake flag
	 *
	 * @return				TRUE if first handshake
	 */
	bool (*is_first_handshake)(imc_test_state_t *this);

	/**
	 * Test and reset the retry handshake flag
	 *
	 * @return				TRUE if a handshake retry should be done
	 */
	bool (*do_handshake_retry)(imc_test_state_t *this);

};

/**
 * Create an imc_test_state_t instance
 *
 * @param id			connection ID
 * @param command		command to send to IMV
 * @param dummy_size	size of the dummy attribute to send (only if > 0)
 * @param retry			TRUE if a handshake retry should be done
 */
imc_state_t* imc_test_state_create(TNC_ConnectionID id, char* command,
								   int dummy_size, bool retry);

#endif /** IMC_TEST_STATE_H_ @}*/
