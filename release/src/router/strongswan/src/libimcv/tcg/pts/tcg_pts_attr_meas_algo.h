/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup tcg_pts_attr_meas_algo tcg_pts_attr_meas_algo
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_MEAS_ALGO_H_
#define TCG_PTS_ATTR_MEAS_ALGO_H_

typedef struct tcg_pts_attr_meas_algo_t tcg_pts_attr_meas_algo_t;

#include "tcg/tcg_attr.h"
#include "pts/pts_meas_algo.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the TCG Measurement Algorithm Attribute
 *
 */
struct tcg_pts_attr_meas_algo_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get PTS Measurement Algorithm Set
	 *
	 * @return				set of algorithms
	 */
	pts_meas_algorithms_t (*get_algorithms)(tcg_pts_attr_meas_algo_t *this);

};

/**
 * Creates an tcg_pts_attr_meas_algo_t object
 *
 * @param algorithms		set of algorithms
 * @param selection			TRUE if a selection
 */
pa_tnc_attr_t* tcg_pts_attr_meas_algo_create(pts_meas_algorithms_t algorithms,
											 bool selection);

/**
 * Creates an tcg_pts_attr_meas_algo_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 * @param selection			TRUE if a selection
 */
pa_tnc_attr_t* tcg_pts_attr_meas_algo_create_from_data(size_t length,
													   chunk_t value,
													   bool selection);

#endif /** TCG_PTS_ATTR_MEAS_ALGO_H_ @}*/
