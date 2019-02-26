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
 * @defgroup pts_creds pts_creds
 * @{ @ingroup pts
 */

#ifndef PTS_CREDS_H_
#define PTS_CREDS_H_

typedef struct pts_creds_t pts_creds_t;

#include <library.h>
#include <credentials/credential_set.h>

/**
 * Class implementing a PTS credentials set
 */
struct pts_creds_t {

	/**
	 * Get the credential set
	 *
	 * @return				credential set
	 */
	credential_set_t* (*get_set)(pts_creds_t *this);

	/**
	 * Destroys a pts_creds_t object.
	 */
	void (*destroy)(pts_creds_t *this);

};

/**
 * Creates an pts_creds_t object
 *
 * @param path				path to the PTS cacerts directory
 */
pts_creds_t* pts_creds_create(char *path);

#endif /** PTS_CREDS_H_ @}*/
