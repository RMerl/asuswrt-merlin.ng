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

#include <inttypes.h>
#include <library.h>
#include <daemon.h>

#include "tkm_spi_generator.h"

/**
 * Get SPI callback arguments
 */
typedef struct {
	rng_t *rng;
	uint64_t spi_mask;
	uint64_t spi_label;
} get_spi_args_t;

static get_spi_args_t *spi_args;

/**
 * Callback called to generate an IKE SPI.
 *
 * @param this			Callback args containing rng_t and spi mask & label
 * @return				labeled SPI
 */
CALLBACK(tkm_get_spi, uint64_t,
	const get_spi_args_t const *this)
{
	uint64_t spi;

	if (!this->rng->get_bytes(this->rng, sizeof(spi), (uint8_t*)&spi))
	{
		return 0;
	}

	return (spi & ~this->spi_mask) | this->spi_label;
}

bool tkm_spi_generator_register(plugin_t *plugin,
                                plugin_feature_t *feature,
                                bool reg, void *cb_data)
{
	uint64_t spi_mask, spi_label;
	char *spi_val;
	rng_t *rng;

	if (reg)
	{
		rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
		if (!rng)
		{
			return FALSE;
		}

		spi_val = lib->settings->get_str(lib->settings, "%s.spi_mask", NULL,
										 lib->ns);
		spi_mask = settings_value_as_uint64(spi_val, 0);

		spi_val = lib->settings->get_str(lib->settings, "%s.spi_label", NULL,
										 lib->ns);
		spi_label = settings_value_as_uint64(spi_val, 0);

		INIT(spi_args,
			.rng = rng,
			.spi_mask = spi_mask,
			.spi_label = spi_label,
		);

		charon->ike_sa_manager->set_spi_cb(charon->ike_sa_manager,
				tkm_get_spi, spi_args);
		DBG1(DBG_IKE, "using SPI label 0x%.16"PRIx64" and mask 0x%.16"PRIx64,
			 spi_label, spi_mask);
	}
	else
	{
		if (spi_args)
		{
			DESTROY_IF(spi_args->rng);
			free(spi_args);
		}
	}

	return TRUE;
}
