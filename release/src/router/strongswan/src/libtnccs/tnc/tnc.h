/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup libtnccs libtnccs
 *
 * @defgroup tplugins plugins
 * @ingroup libtnccs
 *
 * @addtogroup libtnccs
 * @{
 */

#ifndef TNC_H_
#define TNC_H_

typedef struct tnc_t tnc_t;

#include "tnc/imc/imc_manager.h"
#include "tnc/imv/imv_manager.h"
#include "tnc/tnccs/tnccs_manager.h"

#include <library.h>

/**
 * TNC management support object.
 */
struct tnc_t {

	/**
	 * TNC-IMC manager controlling Integrity Measurement Collectors
	 */
	imc_manager_t *imcs;

	/**
	 * TNC-IMV manager controlling Integrity Measurement Verifiers
	 */
	imv_manager_t *imvs;

	/**
	 * TNC-TNCCS manager controlling the TNC Server and Client protocols
	 */
	tnccs_manager_t *tnccs;

};

/**
 * The single instance of tnc_t.
 *
 * Exists between calls to libtnccs_init() and libtnccs_deinit().
 */
extern tnc_t *tnc;

/**
 * Initialize libtnccs.
 */
void libtnccs_init(void);

/**
 * Deinitialize libtnccs
 */
void libtnccs_deinit(void);

/**
 * Helper function to (un-)register TNC managers from plugin features.
 *
 * This function is a plugin_feature_callback_t and can be used with the
 * PLUGIN_CALLBACK macro to register a TNC manager constructor.
 *
 * @param plugin		plugin registering the TNC manager
 * @param feature		associated plugin feature
 * @param reg			TRUE to register, FALSE to unregister.
 * @param data			data passed to callback, a TNC manager constructor
 */
bool tnc_manager_register(plugin_t *plugin, plugin_feature_t *feature,
						  bool reg, void *data);

#endif /** TNC_H_ @}*/
