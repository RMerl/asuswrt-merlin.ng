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
 * @defgroup tcg_pts_attr_dh_nonce_finish tcg_pts_attr_dh_nonce_finish
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_DH_NONCE_FINISH_H_
#define TCG_PTS_ATTR_DH_NONCE_FINISH_H_

typedef struct tcg_pts_attr_dh_nonce_finish_t tcg_pts_attr_dh_nonce_finish_t;

#include "tcg/tcg_attr.h"
#include "pa_tnc/pa_tnc_attr.h"
#include "pts/pts_meas_algo.h"

/**
 * Class implementing the TCG PTS DH Nonce Finish Attribute
 */
struct tcg_pts_attr_dh_nonce_finish_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get nonce length
	 *
	 * @return				Length of nonce
	 */
	uint8_t (*get_nonce_len)(tcg_pts_attr_dh_nonce_finish_t *this);

	/**
	 * Get selected hash algorithm
	 *
	 * @return				Selected hash algorithm
	 */
	pts_meas_algorithms_t (*get_hash_algo)(tcg_pts_attr_dh_nonce_finish_t *this);

	/**
	 * Get DH Initiator Public Value
	 *
	 * @return				DH Initiator Public Value
	 */
	chunk_t (*get_initiator_value)(tcg_pts_attr_dh_nonce_finish_t *this);

	/**
	 * Get DH Initiator Nonce
	 *
	 * @return				DH Initiator Nonce
	 */
	chunk_t (*get_initiator_nonce)(tcg_pts_attr_dh_nonce_finish_t *this);

};

/**
 * Creates an tcg_pts_attr_dh_nonce_finish_t object
 *
 * @param hash_algo					Selected hash algorithm
 * @param initiator_value			DH Initiator Public Value
 * @param initiator_nonce			DH Initiator Nonce
 */
pa_tnc_attr_t* tcg_pts_attr_dh_nonce_finish_create(
										pts_meas_algorithms_t hash_algo,
										chunk_t initiator_value,
										chunk_t initiator_nonce);

/**
 * Creates an tcg_pts_attr_dh_nonce_finish_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_pts_attr_dh_nonce_finish_create_from_data(size_t length,
															 chunk_t value);

#endif /** TCG_PTS_ATTR_DH_NONCE_FINISH_H_ @}*/
