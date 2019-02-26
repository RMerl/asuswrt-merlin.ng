/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup pts_component_manager pts_component_manager
 * @{ @ingroup pts
 */

#ifndef PTS_COMPONENT_MANAGER_H_
#define PTS_COMPONENT_MANAGER_H_

typedef struct pts_component_manager_t pts_component_manager_t;

#include "pts/pts_database.h"
#include "pts/components/pts_component.h"
#include "pts/components/pts_comp_func_name.h"

#include <library.h>
#include <pen/pen.h>

typedef pts_component_t* (*pts_component_create_t)(uint32_t depth,
												   pts_database_t *pts_db);

/**
 * Manages PTS Functional Components
 */
struct pts_component_manager_t {

	/**
	 * Add vendor-specific functional component names
	 *
	 * @param vendor_id				Private Enterprise Number (PEN)
	 * @param comp_func_names		Vendor-specific Component Functional names
	 * @param qualifier_type_size	Vendor-specific Qualifier Type size
	 * @param qualifier_flag_names	Vendor-specific Qualifier Flag names
	 * @param qualifier_type_names	Vendor-specific Qualifier Type names
	 */
	void (*add_vendor)(pts_component_manager_t *this, pen_t vendor_id,
					   enum_name_t *comp_func_names,
					   int qualifier_type_size,
					   char *qualifier_flag_names,
					   enum_name_t *qualifier_type_names);

	/**
	 * Add vendor-specific functional component
	 *
	 * @param vendor_id				Private Enterprise Number (PEN)
	 * @param names					Component Functional Name
	 * @param create				Functional Component creation method
	 */
	void (*add_component)(pts_component_manager_t *this, pen_t vendor_id,
						  uint32_t name, pts_component_create_t create);

	/**
	 * Remove vendor-specific components and associated namespace
	 *
	 * @param vendor_id				Private Enterprise Number (PEN)
	 */
	void (*remove_vendor)(pts_component_manager_t *this, pen_t vendor_id);

	/**
	 * Return the Functional Component names for a given vendor ID
	 *
	 * @param vendor_id				Private Enterprise Number (PEN)
	 * @return 						Comp. Func. names if found, NULL else
	 */
	enum_name_t* (*get_comp_func_names)(pts_component_manager_t *this,
										pen_t vendor_id);

	/**
	 * Return the Functional Component Qualifier Type names for a given vendor ID
	 *
	 * @param vendor_id				Private Enterprise Number (PEN)
	 * @return 						Qualifier Type names if found, NULL else
	 */
	enum_name_t* (*get_qualifier_type_names)(pts_component_manager_t *this,
											 pen_t vendor_id);

	/**
	 * Return the Qualifier Type and Flags
	 *
	 * @param name					Component Functional Name
	 * @param flags					Qualifier Flags as a string in a char buffer
	 * @return						Qualifier Type
	 */
	uint8_t (*get_qualifier)(pts_component_manager_t *this,
							  pts_comp_func_name_t *name, char *flags);

	/**
	 * Create a PTS Component object from a Functional Component Name object
	 *
	 * @param name					Component Functional Name
	 * @param depth					Sub-component Depth
	 * @param pts_db				PTS measurement database
	 * @return						Component object if supported, NULL else
	 */
	pts_component_t* (*create)(pts_component_manager_t *this,
							   pts_comp_func_name_t *name, uint32_t depth,
							   pts_database_t *pts_db);

	/**
	 * Destroys a pts_component_manager_t object.
	 */
	void (*destroy)(pts_component_manager_t *this);
};

/**
 * Create a PA-TNC attribute manager
 */
pts_component_manager_t* pts_component_manager_create(void);

#endif /** PTS_COMPONENT_MANAGER_H_ @}*/
