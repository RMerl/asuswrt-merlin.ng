/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup certexpire_export certexpire_export
 * @{ @ingroup certexpire
 */

#ifndef CERTEXPIRE_EXPORT_H_
#define CERTEXPIRE_EXPORT_H_

typedef struct certexpire_export_t certexpire_export_t;

#include <collections/linked_list.h>

/**
 * Caches and exports trustchain information to CSV files.
 */
struct certexpire_export_t {

	/**
	 * Add trustchain to cache for export.
	 *
	 * @param trustchain		trustchain, sorted list of certificate_t
	 * @param local				TRUE for own chain, FALSE for remote chain
	 */
	void (*add)(certexpire_export_t *this, linked_list_t *trustchain, bool local);

	/**
	 * Destroy a certexpire_export_t.
	 */
	void (*destroy)(certexpire_export_t *this);
};

/**
 * Create a certexpire_export instance.
 */
certexpire_export_t *certexpire_export_create();

#endif /** CERTEXPIRE_EXPORT_H_ @}*/
