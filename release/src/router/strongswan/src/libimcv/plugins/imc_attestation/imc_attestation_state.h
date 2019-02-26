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
 * @defgroup imc_attestation imc_attestation
 * @ingroup libimcv_plugins
 *
 * @defgroup imc_attestation_state_t imc_attestation_state
 * @{ @ingroup imc_attestation
 */

#ifndef IMC_ATTESTATION_STATE_H_
#define IMC_ATTESTATION_STATE_H_

#include <imc/imc_state.h>
#include <pts/pts.h>
#include <pts/components/pts_component.h>
#include <pts/components/pts_comp_evidence.h>
#include <library.h>

typedef struct imc_attestation_state_t imc_attestation_state_t;

/**
 * Internal state of an imc_attestation_t connection instance
 */
struct imc_attestation_state_t {

	/**
	 * imc_state_t interface
	 */
	imc_state_t interface;

	/**
	 * Get the PTS object
	 *
	 * @return					PTS object
	 */
	pts_t* (*get_pts)(imc_attestation_state_t *this);

	/**
	 * Create and add an entry to the list of Functional Components
	 *
	 * @param name				Component Functional Name
	 * @param depth				Sub-component Depth
	 * @return					created functional component instance or NULL
	 */
	pts_component_t* (*create_component)(imc_attestation_state_t *this,
							 pts_comp_func_name_t *name, uint32_t depth);

	/**
	 * Add an entry to the Component Evidence cache list
	 *
	 * @param evid				Component Evidence entry
	 */
	void (*add_evidence)(imc_attestation_state_t *this, pts_comp_evidence_t *evid);

	/**
	 * Removes next entry from the Component Evidence cache list and returns it
	 *
	 * @param evid				Next Component Evidence entry
	 * @return					TRUE if next entry is available
	 */
	bool (*next_evidence)(imc_attestation_state_t *this, pts_comp_evidence_t** evid);

};

/**
 * Create an imc_attestation_state_t instance
 *
 * @param id					connection ID
 */
imc_state_t* imc_attestation_state_create(TNC_ConnectionID id);

#endif /** IMC_ATTESTATION_STATE_H_ @}*/
