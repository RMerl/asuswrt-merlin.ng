/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup charon-nm charon-nm
 *
 * @defgroup nm nm
 * @ingroup charon-nm
 *
 * @defgroup nm_backend nm_backend
 * @{ @ingroup nm
 */

#ifndef NM_BACKEND_H_
#define NM_BACKEND_H_

/**
 * Initialize the NetworkManager backend.
 *
 * @return		TRUE, if initialization was successful
 */
void nm_backend_register();

#endif /** NM_BACKEND_H_ @}*/
