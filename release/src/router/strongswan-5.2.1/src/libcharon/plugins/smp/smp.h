/*
 * Copyright (C) 2007-2008 Martin Willi
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
 * @defgroup smp smp
 * @ingroup cplugins
 *
 * @defgroup smp_i smp
 * @{ @ingroup smp
 */

#ifndef SMP_H_
#define SMP_H_

#include <plugins/plugin.h>

typedef struct smp_t smp_t;

/**
 * SMP configuration and control interface.
 *
 * The SMP interface uses a socket and a to communicate. The syntax is strict
 * XML, defined in the schema.xml specification.
 */
struct smp_t {

	/**
	 * implements the plugin interface.
	 */
	plugin_t plugin;
};

#endif /** XML_H_ @}*/
