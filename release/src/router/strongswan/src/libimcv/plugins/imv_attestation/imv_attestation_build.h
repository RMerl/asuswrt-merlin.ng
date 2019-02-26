/*
 * Copyright (C) 2011 Sansar Choinyambuu
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
 * @defgroup imv_attestation_build_t imv_attestation_build
 * @{ @ingroup imv_attestation
 */

#ifndef IMV_ATTESTATION_BUILD_H_
#define IMV_ATTESTATION_BUILD_H_

#include "imv_attestation_state.h"

#include <imv/imv_msg.h>
#include <library.h>

#include <pts/pts_database.h>
#include <pts/pts_dh_group.h>
#include <pts/pts_meas_algo.h>

/**
 * Process a TCG PTS attribute
 *
 * @param out_msg				outbound PA-TNC message to be built
 * @param state					state of a given connection
 * @param supported_dh_groups	supported DH groups
 * @param pts_db				PTS configuration database
 * @return						TRUE if successful
 */
bool imv_attestation_build(imv_msg_t *out_msg, imv_state_t *state,
						   pts_dh_group_t supported_dh_groups,
						   pts_database_t *pts_db);

#endif /** IMV_ATTESTATION_BUILD_H_ @}*/
