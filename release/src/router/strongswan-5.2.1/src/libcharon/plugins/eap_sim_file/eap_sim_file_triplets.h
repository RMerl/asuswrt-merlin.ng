/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_sim_file_triplets eap_sim_file_triplets
 * @{ @ingroup eap_sim_file
 */

#ifndef EAP_SIM_FILE_TRIPLETS_H_
#define EAP_SIM_FILE_TRIPLETS_H_

#include <collections/enumerator.h>

typedef struct eap_sim_file_triplets_t eap_sim_file_triplets_t;

/**
 * Reads triplets from a triplets.dat file.
 *
 * The file is in freeradius triplet file syntax:
 * http://www.freeradius.org/radiusd/doc/rlm_sim_triplets
 */
struct eap_sim_file_triplets_t {

	/**
	 * Create an enumerator over the file's triplets.
	 *
	 * @return			enumerator over (identity, rand, sres, kc)
	 */
	enumerator_t* (*create_enumerator)(eap_sim_file_triplets_t *this);

	/**
	 * Destroy a eap_sim_file_triplets_t.
	 */
	void (*destroy)(eap_sim_file_triplets_t *this);
};

/**
 * Create a eap_sim_file_triplets instance.
 *
 * @param file		triplet file to read from
 */
eap_sim_file_triplets_t *eap_sim_file_triplets_create(char *file);

#endif /** EAP_SIM_FILE_TRIPLETS_H_ @}*/
