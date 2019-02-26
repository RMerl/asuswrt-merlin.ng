/*
 * Copyright (C) 2006-2008 Martin Willi
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
 * @defgroup des_crypter des_crypter
 * @{ @ingroup des_p
 */

#ifndef DES_CRYPTER_H_
#define DES_CRYPTER_H_

typedef struct des_crypter_t des_crypter_t;

#include <crypto/crypters/crypter.h>


/**
 * Class implementing the DES and 3DES encryption algorithms.
 */
struct des_crypter_t {

	/**
	 * Implements crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Constructor to create des_crypter_t objects.
 *
 * @param algo		ENCR_DES for single DES, ENCR_3DES for triple DES
 * @return			des_crypter_t object, NULL if algo not supported
 */
des_crypter_t *des_crypter_create(encryption_algorithm_t algo);


#endif /** DES_CRYPTER_H_ @}*/
