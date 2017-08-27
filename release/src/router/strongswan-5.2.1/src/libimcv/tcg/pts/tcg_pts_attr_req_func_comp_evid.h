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
 * @defgroup tcg_pts_attr_req_func_comp_evid tcg_pts_attr_req_func_comp_evid
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_REQ_FUNC_COMP_EVID_H_
#define TCG_PTS_ATTR_REQ_FUNC_COMP_EVID_H_

typedef struct tcg_pts_attr_req_func_comp_evid_t tcg_pts_attr_req_func_comp_evid_t;

#include "tcg/tcg_attr.h"
#include "pts/components/pts_comp_func_name.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the TCG PTS Request Functional Component Evidence attribute
 *
 */
struct tcg_pts_attr_req_func_comp_evid_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Add a component to the Functional Component Evidence Request
	 *
	 * @param flags				Component Evidence Request Flags
	 * @param depth				Sub-component Depth
	 * @param name				Functional Component Name
	 */
	void (*add_component)(tcg_pts_attr_req_func_comp_evid_t *this,
						  u_int8_t flags, u_int32_t depth,
						  pts_comp_func_name_t *name);

	/**
	 * Returns the number of Functional Component entries
	 *
	 * @return					Number of entries
	 */
	int (*get_count)(tcg_pts_attr_req_func_comp_evid_t *this);

	/**
	 * Enumerator over Functional Component entries
	 *
	 * @return					Entry enumerator
	 */
	enumerator_t* (*create_enumerator)(tcg_pts_attr_req_func_comp_evid_t *this);

};

/**
 * Creates a tcg_pts_attr_req_func_comp_evid_t object
 */
pa_tnc_attr_t* tcg_pts_attr_req_func_comp_evid_create(void);

/**
 * Creates a tcg_pts_attr_req_func_comp_evid_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_pts_attr_req_func_comp_evid_create_from_data(size_t length,
																chunk_t value);

#endif /** TCG_PTS_ATTR_REQ_FUNC_COMP_EVID_H_ @}*/
