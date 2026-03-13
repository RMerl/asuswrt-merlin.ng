/*
 * Copyright (C) 2024 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "ml_params.h"

/*
 * Described in header
 */
const uint16_t ml_kem_zetas[128] = {
	   1, 1729, 2580, 3289, 2642,  630, 1897,  848,
	1062, 1919,  193,  797, 2786, 3260,  569, 1746,
	 296, 2447, 1339, 1476, 3046,   56, 2240, 1333,
	1426, 2094,  535, 2882, 2393, 2879, 1974,  821,
	 289,  331, 3253, 1756, 1197, 2304, 2277, 2055,
	 650, 1977, 2513,  632, 2865,   33, 1320, 1915,
	2319, 1435,  807,  452, 1438, 2868, 1534, 2402,
	2647, 2617, 1481,  648, 2474, 3110, 1227,  910,
	  17, 2761,  583, 2649, 1637,  723, 2288, 1100,
	1409, 2662, 3281,  233,  756, 2156, 3015, 3050,
	1703, 1651, 2789, 1789, 1847,  952, 1461, 2687,
	 939, 2308, 2437, 2388,  733, 2337,  268,  641,
	1584, 2298, 2037, 3220,  375, 2549, 2090, 1645,
	1063,  319, 2773,  757, 2099,  561, 2466, 2594,
	2804, 1092,  403, 1026, 1143, 2150, 2775,  886,
	1722, 1212, 1874, 1029, 2110, 2935,  885, 2154,
};

/**
 * Parameter sets for ML-KEM.
 */
static const ml_kem_params_t ml_kem_params[] = {
	{
		.method = ML_KEM_512,
		.k = 2,
		.eta1 = 3,
		.eta2 = 2,
		.du = 10,
		.dv = 4,
		.pk_len = 800,
		.ct_len = 768,
	},
	{
		.method = ML_KEM_768,
		.k = 3,
		.eta1 = 2,
		.eta2 = 2,
		.du = 10,
		.dv = 4,
		.pk_len = 1184,
		.ct_len = 1088,
	},
	{
		.method = ML_KEM_1024,
		.k = 4,
		.eta1 = 2,
		.eta2 = 2,
		.du = 11,
		.dv = 5,
		.pk_len = 1568,
		.ct_len = 1568,
	},
};

/*
 * Described in header
 */
const ml_kem_params_t *ml_kem_params_get(key_exchange_method_t method)
{
	int i;

	for (i = 0; i < countof(ml_kem_params); i++)
	{
		if (ml_kem_params[i].method == method)
		{
			return &ml_kem_params[i];
		}
	}
	return NULL;
}
