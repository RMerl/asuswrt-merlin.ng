/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup tpm tpm
 * @ingroup plugins
 *
 * @defgroup tpm_plugin tpm_plugin
 * @{ @ingroup tpm
 */

#ifndef TPM_PLUGIN_H_
#define TPM_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tpm_plugin_t tpm_plugin_t;

/**
 * Plugin providing TPM token support.
 */
struct tpm_plugin_t {

	/**
	 * Implements plugin interface,
	 */
	plugin_t plugin;
};

#endif /** TPM_PLUGIN_H_ @}*/
