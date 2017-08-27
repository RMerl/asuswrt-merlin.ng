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
 * @defgroup imv_attestation_process_t imv_attestation_process
 * @{ @ingroup imv_attestation
 */

#ifndef IMV_ATTESTATION_PROCESS_H_
#define IMV_ATTESTATION_PROCESS_H_

#include "imv_attestation_state.h"

#include <library.h>
#include <collections/linked_list.h>
#include <credentials/credential_manager.h>
#include <crypto/hashers/hasher.h>

#include <imv/imv_msg.h>
#include <pa_tnc/pa_tnc_attr.h>

#include <pts/pts_database.h>
#include <pts/pts_dh_group.h>
#include <pts/pts_meas_algo.h>

/**
 * Process a TCG PTS attribute
 *
 * @param attr					PA-TNC attribute to be processed
 * @param out_msg				PA-TNC message containing error messages
 * @param state					state of a given connection
 * @param supported_algorithms	supported PTS measurement algorithms
 * @param supported_dh_groups	supported DH groups
 * @param pts_db				PTS configuration database
 * @param pts_credmgr			PTS credential manager
 * @return						TRUE if successful
 */
bool imv_attestation_process(pa_tnc_attr_t *attr, imv_msg_t *out_msg,
							 imv_state_t *state,
							 pts_meas_algorithms_t supported_algorithms,
							 pts_dh_group_t supported_dh_groups,
							 pts_database_t *pts_db,
							 credential_manager_t *pts_credmgr);

#endif /** IMV_ATTESTATION_PROCESS_H_ @}*/
