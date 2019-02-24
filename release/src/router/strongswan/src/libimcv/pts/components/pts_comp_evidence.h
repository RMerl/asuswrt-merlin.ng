/*
 * Copyright (C) 2011 Sansar Choinyambuu, Andreas Steffen
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
 * @defgroup pts_comp_evidence pts_comp_evidence
 * @{ @ingroup pts
 */

#ifndef PTS_COMP_EVIDENCE_H_
#define PTS_COMP_EVIDENCE_H_

typedef struct pts_comp_evidence_t pts_comp_evidence_t;
typedef enum pts_pcr_transform_t pts_pcr_transform_t;
typedef enum pts_comp_evid_validation_t pts_comp_evid_validation_t;

#include "pts/pts_meas_algo.h"
#include "pts/components/pts_comp_func_name.h"

#include <library.h>

/**
 * PTS PCR Transformations
 */
enum pts_pcr_transform_t {
	/** No Transformation */
	PTS_PCR_TRANSFORM_NO =		0,
	/** Hash Value matched PCR size */
	PTS_PCR_TRANSFORM_MATCH =	1,
	/** Hash value shorter than PCR size */
	PTS_PCR_TRANSFORM_SHORT =	2,
	/** Hash value longer than PCR size */
	PTS_PCR_TRANSFORM_LONG =	3,
};

/**
 * PTS Component Evidence Validation Result Flags
 */
enum pts_comp_evid_validation_t {
	/** No Validation was attempted */		
	PTS_COMP_EVID_VALIDATION_NONE =		0x00,
	/** Attempted validation, unable to verify */
	PTS_COMP_EVID_VALIDATION_UNABLE =	0x20,
	/** Attempted validation, verification failed */
	PTS_COMP_EVID_VALIDATION_FAILED =	0x40,
	/** Attempted validation, verification passed */
	PTS_COMP_EVID_VALIDATION_PASSED =	0x60,
};

/**
 * PTS Functional Component Interface 
 */
struct pts_comp_evidence_t {

	/**
	 * Gets the Component Functional Name and Sub-Component Depth
	 *
	 * @param depth				Sub-Component Depth
	 * @result					Component Functional Name
	 */
	pts_comp_func_name_t* (*get_comp_func_name)(pts_comp_evidence_t *this,
							   					uint32_t *depth);

	/**
	 * Gets the PCR the measurement was extended into
	 *
	 * @result					PCR the measurement was extended into
	 */
	uint32_t (*get_extended_pcr)(pts_comp_evidence_t *this);

	/**
	 * Gets the measurement and the algorithms used
	 *
	 * @param extended_pcr		PCR the measurement was extended into
	 * @param algo				Measurement hash algorithm
	 * @param transform			Transformation used for PCR extension
	 * @param measurement_time	Time the measurement was taken
	 * @result					Measurement hash value
	 */
	chunk_t (*get_measurement)(pts_comp_evidence_t *this,
							   uint32_t *extended_pcr,
							   pts_meas_algorithms_t *algo,
							   pts_pcr_transform_t *transform,
							   time_t *measurement_time);

	/**
	 * Gets the PCR information if available
	 *
	 * @param pcr_before		PCR value before extension
	 * @param pcr_after			PCR value after extension
	 * @result					TRUE if PCR information is available
	 */
	bool (*get_pcr_info)(pts_comp_evidence_t *this, chunk_t *pcr_before,
													chunk_t *pcr_after);

	/**
	 * Sets PCR information if available
	 *
	 * @param pcr_before		PCR value before extension
	 * @param pcr_after			PCR value after extension
	 */
	void (*set_pcr_info)(pts_comp_evidence_t *this, chunk_t pcr_before,
													chunk_t pcr_after);

	/**
	 * Gets Validation Result if available
	 *
	 * @param uri				Verification Policy URI
	 * @return validation		Validation Result
	 */
	pts_comp_evid_validation_t (*get_validation)(pts_comp_evidence_t *this,
								char **uri);

	/**
	 * Sets Validation Result if available
	 *
	 * @param validation		Validation Result
	 * @param uri				Verification Policy URI
	 */
	void (*set_validation)(pts_comp_evidence_t *this,
						   pts_comp_evid_validation_t validation, char* uri);

	/**
	 * Destroys a pts_comp_evidence_t object.
	 */
	void (*destroy)(pts_comp_evidence_t *this);

};

/**
 * Creates a pts_comp_evidence_t object
 * 
 * @param name					Component Functional Name
 * @param depth					Sub-component depth
 * @param extended_pcr			PCR the measurement was extended into
 * @param algo					Measurement hash algorithm
 * @param transform				Transformation used for PCR extension
 * @param measurement_time		Time the measurement was taken, 0 if unknown
 * @param measurement			Measurement hash value
 */
pts_comp_evidence_t* pts_comp_evidence_create(pts_comp_func_name_t *name,
											  uint32_t depth,
											  uint32_t extended_pcr,
											  pts_meas_algorithms_t algo,
											  pts_pcr_transform_t transform,
											  time_t measurement_time,
											  chunk_t measurement);

/**
 * Determine transform to fit measurement hash into PCR register
 *
 * @param algo					Measurement hash algorithm 
 * @param pcr_len				Length of the PCR registers in bytes
 * @return						PCR transform type
 */
pts_pcr_transform_t pts_meas_algo_to_pcr_transform(pts_meas_algorithms_t algo,
												   size_t pcr_len);

#endif /** PTS_COMP_EVIDENCE_H_ @}*/
