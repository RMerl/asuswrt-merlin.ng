/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup imc_scanner imc_scanner
 * @ingroup libimcv_plugins
 *
 * @defgroup imc_scanner_state_t imc_scanner_state
 * @{ @ingroup imc_scanner
 */

#ifndef IMC_SCANNER_STATE_H_
#define IMC_SCANNER_STATE_H_

#include <imc/imc_state.h>
#include <library.h>

typedef struct imc_scanner_state_t imc_scanner_state_t;

/**
 * Internal state of an imc_scanner_t connection instance
 */
struct imc_scanner_state_t {

	/**
	 * imc_state_t interface
	 */
	imc_state_t interface;
};

/**
 * Create an imc_scanner_state_t instance
 *
 * @param id		connection ID
 */
imc_state_t* imc_scanner_state_create(TNC_ConnectionID id);

#endif /** IMC_SCANNER_STATE_H_ @}*/
