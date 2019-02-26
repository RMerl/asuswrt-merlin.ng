/*
 * Copyright (C) 2007-2008 Andreas Steffen
 *
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
 * @defgroup optionsfrom optionsfrom
 * @{ @ingroup utils
 */

#ifndef OPTIONSFROM_H_
#define OPTIONSFROM_H_

typedef struct options_t options_t;

/**
 * Reads additional command line arguments from a file
 */
struct options_t {

	/**
	 * Check if the PKCS#7 contentType is data
	 *
	 * @param filename		file containing the options
	 * @param argcp			pointer to argc
	 * @param argvp			pointer to argv[]
	 * @param optind		current optind, number of next argument
	 * @return				TRUE if optionsfrom parsing successful
	 */
	bool (*from) (options_t * this, char *filename,
				  int *argcp, char **argvp[], int optind);

	/**
	 * Destroys the options_t object.
	 */
	void (*destroy) (options_t *this);
};

/**
 * Create an options object.
 *
 * @return					created options_t object
 */
options_t *options_create(void);

#endif /** OPTIONSFROM_H_ @}*/
