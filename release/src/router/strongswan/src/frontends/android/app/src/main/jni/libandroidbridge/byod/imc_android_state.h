/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup imc_android_state imc_android_state
 * @{ @ingroup android_byod
 */

#ifndef IMC_ANDROID_STATE_H_
#define IMC_ANDROID_STATE_H_

#include <imc/imc_state.h>
#include <pts/pts.h>

typedef struct imc_android_state_t imc_android_state_t;

/**
 * Internal state of an imc_android_t connection instance
 */
struct imc_android_state_t {

	/**
	 * imc_state_t interface
	 */
	imc_state_t interface;

	/**
	 * Get TCG Platform Trust Service (PTS) object
	 */
	pts_t *(*get_pts)(imc_android_state_t *this);
};

/**
 * Create an imc_android_state_t instance
 *
 * @param id		connection ID
 */
imc_state_t* imc_android_state_create(TNC_ConnectionID id);

#endif /** IMC_ANDROID_STATE_H_ @}*/
