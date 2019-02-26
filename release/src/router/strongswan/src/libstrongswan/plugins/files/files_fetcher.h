/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @defgroup files_fetcher files_fetcher
 * @{ @ingroup files_p
 */

#ifndef FILES_FETCHER_H_
#define FILES_FETCHER_H_

typedef struct files_fetcher_t files_fetcher_t;

/**
 * Fetcher implementation loading local files
 */
struct files_fetcher_t {

	/**
	 * Implements fetcher interface
	 */
	fetcher_t interface;
};

/**
 * Create a files_fetcher instance.
 */
files_fetcher_t *files_fetcher_create();

#endif /** FILES_FETCHER_H_ @}*/
