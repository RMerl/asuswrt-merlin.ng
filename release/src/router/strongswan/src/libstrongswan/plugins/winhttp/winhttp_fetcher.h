/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup winhttp_fetcher winhttp_fetcher
 * @{ @ingroup winhttp_p
 */

#ifndef WINHTTP_FETCHER_H_
#define WINHTTP_FETCHER_H_

#include <library.h>

typedef struct winhttp_fetcher_t winhttp_fetcher_t;

/**
 * Fetcher implementation using Microsofts WinHTTP.
 */
struct winhttp_fetcher_t {

	/**
	 * Implements fetcher interface.
	 */
	fetcher_t interface;
};

/**
 * Create a winhttp_fetcher instance
 *
 * @return		WinHTTP based fetcher
 */
winhttp_fetcher_t *winhttp_fetcher_create();

#endif /** WINHTTP_FETCHER_H_ @}*/
