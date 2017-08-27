/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup x509_p x509
 * @ingroup plugins
 *
 * @defgroup x509_plugin x509_plugin
 * @{ @ingroup x509_p
 */

#ifndef X509_PLUGIN_H_
#define X509_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct x509_plugin_t x509_plugin_t;

/**
 * Plugin implementing x509, CRL and OCSP certificates.
 */
struct x509_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** X509_PLUGIN_H_ @}*/
