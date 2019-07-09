/*
 * Copyright (C) 2017 Tobias Brunner
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
 * @defgroup android_fetcher android_fetcher
 * @{ @ingroup android_backend
 */

#ifndef ANDROID_FETCHER_H_
#define ANDROID_FETCHER_H_

#include <library.h>

/**
 * Create an Android-specific fetcher instance based on SimpleFetcher
 *
 * @return						fetcher_t instance
 */
fetcher_t *android_fetcher_create();

#endif /** ANDROID_FETCHER_H_ @}*/
