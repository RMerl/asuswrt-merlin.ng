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
 * @defgroup imc_attestation_process_t imc_attestation_process
 * @{ @ingroup imc_attestation
 */

#ifndef IMC_ATTESTATION_PROCESS_H_
#define IMC_ATTESTATION_PROCESS_H_

#include "imc_attestation_state.h"

#include <library.h>

#include <imc/imc_msg.h>
#include <pa_tnc/pa_tnc_attr.h>

#include <pts/pts_dh_group.h>
#include <pts/pts_meas_algo.h>

/**
 * Process a TCG PTS attribute
 *
 * @param attr					PA-TNC attribute to be processed
 * @param msg					outbound PA-TNC message to be assembled
 * @param attestation_state		attestation state of a given connection
 * @param supported_algorithms	supported PTS measurement algorithms
 * @param supported_dh_groups	supported DH groups
 * @return						TRUE if successful
 */
bool imc_attestation_process(pa_tnc_attr_t *attr, imc_msg_t *msg,
							 imc_attestation_state_t *attestation_state,
							 pts_meas_algorithms_t supported_algorithms,
							 pts_dh_group_t supported_dh_groups);

#endif /** IMC_ATTESTATION_PROCESS_H_ @}*/
