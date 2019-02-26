/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup soup_fetcher soup_fetcher
 * @{ @ingroup soup_p
 */

#ifndef SOUP_FETCHER_H_
#define SOUP_FETCHER_H_

#include <library.h>

typedef struct soup_fetcher_t soup_fetcher_t;

/**
 * Fetcher implementation for HTTP using libsoup.
 */
struct soup_fetcher_t {

	/**
	 * Implements fetcher interface.
	 */
	fetcher_t interface;
};

/**
 * Create a soup_fetcher instance.
 */
soup_fetcher_t *soup_fetcher_create();

#endif /** SOUP_FETCHER_H_ @}*/
