/*
 * Copyright (C) 2015 Reto Buerki
 * Copyright (C) 2015 Adrian-Ken Rueegsegger
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
 * @defgroup tkm-spi-generator spi generator
 * @{ @ingroup tkm
 */

#ifndef TKM_SPI_GENERATOR_H_
#define TKM_SPI_GENERATOR_H_

#include <plugins/plugin.h>

/**
 * Register the TKM SPI generator callback.
 *
 * @return			TRUE on success
 */
bool tkm_spi_generator_register(plugin_t *plugin,
                                plugin_feature_t *feature,
                                bool reg, void *cb_data);

#endif /** TKM_SPI_GENERATOR_H_ @}*/
