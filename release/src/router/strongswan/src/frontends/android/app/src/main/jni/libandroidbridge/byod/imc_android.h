/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup android_imc android_imc
 * @{ @ingroup android_byod
 */

#ifndef ANDROID_IMC_H_
#define ANDROID_IMC_H_

/**
 * Callback for the Android IMC plugin
 */
bool imc_android_register(plugin_t *plugin, plugin_feature_t *feature,
						  bool reg, void *data);

#endif /** ANDROID_IMC_H_ @}*/
