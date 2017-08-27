/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu, Andreas Steffen
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
 * @defgroup imv_attestation imv_attestation
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_attestation_state_t imv_attestation_state
 * @{ @ingroup imv_attestation
 */

#ifndef IMV_ATTESTATION_STATE_H_
#define IMV_ATTESTATION_STATE_H_

#include <imv/imv_state.h>
#include <imv/imv_reason_string.h>
#include <pts/pts.h>
#include <pts/pts_database.h>
#include <pts/components/pts_component.h>

#include <library.h>
#include <bio/bio_writer.h>

typedef struct imv_attestation_state_t imv_attestation_state_t;
typedef enum imv_attestation_flag_t imv_attestation_flag_t;
typedef enum imv_attestation_handshake_state_t imv_attestation_handshake_state_t;
typedef enum imv_meas_error_t imv_meas_error_t;

/**
 * IMV Attestation Flags set for completed actions
 */
enum imv_attestation_flag_t {
	IMV_ATTESTATION_ATTR_PRODUCT_INFO =   (1<<0),
	IMV_ATTESTATION_ATTR_STRING_VERSION = (1<<1),
	IMV_ATTESTATION_ATTR_DEVICE_ID =      (1<<2),
	IMV_ATTESTATION_ATTR_MUST =           (1<<3)-1,
	IMV_ATTESTATION_ATTR_REQ =            (1<<3),
	IMV_ATTESTATION_ALGO =                (1<<4),
	IMV_ATTESTATION_DH_NONCE =            (1<<5),
	IMV_ATTESTATION_AIK =                 (1<<6),
	IMV_ATTESTATION_FILE_MEAS =           (1<<7),
	IMV_ATTESTATION_REC =                 (1<<8)
};

/**
 * IMV Attestation Handshake States (state machine)
 */
enum imv_attestation_handshake_state_t {
	IMV_ATTESTATION_STATE_INIT,
	IMV_ATTESTATION_STATE_DISCOVERY,
	IMV_ATTESTATION_STATE_NONCE_REQ,
	IMV_ATTESTATION_STATE_TPM_INIT,
	IMV_ATTESTATION_STATE_COMP_EVID,
	IMV_ATTESTATION_STATE_EVID_FINAL,
	IMV_ATTESTATION_STATE_END,
};

/**
 * IMV Measurement Error Types
 */
enum imv_meas_error_t {
	IMV_ATTESTATION_ERROR_FILE_MEAS_FAIL =  1,
	IMV_ATTESTATION_ERROR_FILE_MEAS_PEND =  2,
	IMV_ATTESTATION_ERROR_NO_TRUSTED_AIK =  4,
	IMV_ATTESTATION_ERROR_COMP_EVID_FAIL =  8,
	IMV_ATTESTATION_ERROR_COMP_EVID_PEND = 16,
	IMV_ATTESTATION_ERROR_TPM_QUOTE_FAIL = 32
};

/**
 * Internal state of an imv_attestation_t connection instance
 */
struct imv_attestation_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Get state of the handshake
	 *
	 * @return					the handshake state of IMV
	 */
	imv_attestation_handshake_state_t (*get_handshake_state)(
		imv_attestation_state_t *this);

	/**
	 * Set state of the handshake
	 *
	 * @param new_state			the handshake state of IMV
	 */
	void (*set_handshake_state)(imv_attestation_state_t *this,
								imv_attestation_handshake_state_t new_state);

	/**
	 * Get the PTS object
	 *
	 * @return					PTS object
	 */
	pts_t* (*get_pts)(imv_attestation_state_t *this);

	/**
	 * Create and add an entry to the list of Functional Components
	 *
	 * @param name				Component Functional Name
	 * @param depth				Sub-component Depth
	 * @param pts_db			PTS measurement database
	 * @return					created functional component instance or NULL
	 */
	pts_component_t* (*create_component)(imv_attestation_state_t *this,
										 pts_comp_func_name_t *name,
										 uint32_t depth,
										 pts_database_t *pts_db);

	/**
	 * Enumerate over all Functional Components
	 *
	 * @return					Functional Component enumerator
	 */
	enumerator_t* (*create_component_enumerator)(imv_attestation_state_t *this);

	/**
	 * Get a Functional Component with a given name
	 *
	 * @param name				Name of the requested Functional Component
	 * @return					Functional Component if found, NULL otherwise
	 */
	pts_component_t* (*get_component)(imv_attestation_state_t *this,
									  pts_comp_func_name_t *name);

	/**
	 * Tell the Functional Components to finalize any measurement registrations
	 * and to check if all expected measurements were received
	 *
	 * @param result			Writer appending component measurement results
	 */
	void (*finalize_components)(imv_attestation_state_t *this,
								bio_writer_t *result);

	/**
	 * Indicates the types of measurement errors that occurred
	 *
	 * @return					Measurement error flags
	 */
	uint32_t (*get_measurement_error)(imv_attestation_state_t *this);

	/**
	 * Call if a measurement error is encountered
	 *
	 * @param error				Measurement error type
	 */
	void (*set_measurement_error)(imv_attestation_state_t *this,
								  uint32_t error);

	/**
	 * Returns a concatenation of File Measurement reason strings
	 *
	 * @param reason_string		Concatenated reason strings
	 */
	void (*add_file_meas_reasons)(imv_attestation_state_t *this,
								  imv_reason_string_t *reason_string);

	/**
	 * Returns a concatenation of Component Evidence reason strings
	 *
	 * @param reason_string		Concatenated reason strings
	 */
	void (*add_comp_evid_reasons)(imv_attestation_state_t *this,
								  imv_reason_string_t *reason_string);
};

/**
 * Create an imv_attestation_state_t instance
 *
 * @param id					connection ID
 */
imv_state_t* imv_attestation_state_create(TNC_ConnectionID id);

#endif /** IMV_ATTESTATION_STATE_H_ @}*/
