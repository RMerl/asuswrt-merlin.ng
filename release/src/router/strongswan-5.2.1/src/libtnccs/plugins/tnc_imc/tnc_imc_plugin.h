/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup tnc_imc tnc_imc
 * @ingroup cplugins
 *
 * @defgroup tnc_imc_plugin tnc_imc_plugin
 * @{ @ingroup tnc_imc
 */

#ifndef TNC_IMC_PLUGIN_H_
#define TNC_IMC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnc_imc_plugin_t tnc_imc_plugin_t;

/**
 * TNC IMC plugin
 */
struct tnc_imc_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNC_IMC_PLUGIN_H_ @}*/
