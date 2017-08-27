/*
 * Copyright (C) 2012 Andreas Steffen
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
 * @defgroup pts_pcr pts_pcr
 * @{ @ingroup pts
 */

#ifndef PTS_PCR_H_
#define PTS_PCR_H_

typedef struct pts_pcr_t pts_pcr_t;

#include <library.h>

/**
 * Maximum number of PCR's of TPM, TPM Spec 1.2
 */
#define PTS_PCR_MAX_NUM				24

/**
 * Number of bytes that can be saved in a PCR of TPM, TPM Spec 1.2
 */
#define PTS_PCR_LEN					20

/**
 * Class implementing a shadow PCR register set
 */
struct pts_pcr_t {

	/**
	 * Get the number of selected PCRs
	 *
	 * @return				number of selected PCRs
	 */
	u_int32_t (*get_count)(pts_pcr_t *this);

	/**
	 * Mark a PCR as selected
	 *
	 * @param pcr			index of PCR
	 * @return				TRUE if PCR index exists
	 */
	bool (*select_pcr)(pts_pcr_t *this, u_int32_t pcr);

	/**
	 * Get the size of the selection field in bytes
	 *
	 * @return				number of bytes written
	 */
	size_t (*get_selection_size)(pts_pcr_t *this);

	/**
	 * Create an enumerator over all selected PCR indexes
	 *
	 * @return				enumerator
	 */
	enumerator_t* (*create_enumerator)(pts_pcr_t *this);

	/**
	 * Get the current content of a PCR
	 *
	 * @param pcr			index of PCR
	 * @return				content of PCR
	 */
	chunk_t (*get)(pts_pcr_t *this, u_int32_t pcr);

	/**
	 * Set the content of a PCR
	 *
	 * @param pcr			index of PCR
	 * @param value			new value of PCR
	 * @return				TRUE if value could be set
	 */
	bool (*set)(pts_pcr_t *this, u_int32_t pcr, chunk_t value);

	/**
	 * Extend the content of a PCR
	 *
	 * @param pcr			index of PCR
	 * @param measurement	measurment value to be extended into PCR
	 * @return				new content of PCR
	 */
	chunk_t (*extend)(pts_pcr_t *this, u_int32_t pcr, chunk_t measurement);

	/**
	 * Create a PCR Composite object over all selected PCRs
	 *
	 * @return				PCR Composite object (must be freed)
	 */
	chunk_t (*get_composite)(pts_pcr_t *this);

	/**

	 * Destroys a pts_pcr_t object.
	 */
	void (*destroy)(pts_pcr_t *this);

};

/**
 * Creates an pts_pcr_t object
 */
pts_pcr_t* pts_pcr_create(void);

#endif /** PTS_PCR_H_ @}*/
