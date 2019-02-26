/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup sw_collector sw-collector
 *
 * @defgroup swid_gen_info_t swid_gen_info
 * @{ @ingroup sw_collector
 */

#ifndef SWID_GEN_INFO_H_
#define SWID_GEN_INFO_H_

typedef struct swid_gen_info_t swid_gen_info_t;

#include "imc/imc_os_info.h"

struct swid_gen_info_t {

	/**
	 * Get OS type
	 *
	 * @return				OS type
	 */
	os_type_t (*get_os_type)(swid_gen_info_t *this);

	/**
	 * Get OS and product strings
	 *
	 * @param product		Product string 'Name Version Arch'
	 * @return				OS string      'Name_Version-Arch'
	 */
	char* (*get_os)(swid_gen_info_t *this, char **product);

	/**
	 * Create software identifier including tagCreator and OS
	 *
	 * @param package		Package string
	 * @param version		Version string
	 * @return				Software Identifier string
	 */
	char* (*create_sw_id)(swid_gen_info_t *this, char *package,
												 char *version);

	/**
	 * Destroy swid_gen_info_t object
	 */
	void (*destroy)(swid_gen_info_t *this);

};

/**
 * Create an swid_gen_info_t instance
 */
swid_gen_info_t* swid_gen_info_create(void);

#endif /** SWID_GEN_INFO_H_ @}*/
