/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the Licenseor (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be usefulbut
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <crypto/crypto_tester.h>

#include <utils/debug.h>

/**
 * Monobit test
 */
typedef struct {
	int lower;
	int upper;
} monobit_t;

monobit_t monobit_all = {
	.lower = 9654,
	.upper = 10346
};

static bool test_monobit(monobit_t *param, chunk_t data)
{
	int i, j, bits = 0;

	for (i = 0; i < data.len; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (data.ptr[i] & (1<<j))
			{
				bits++;
			}
		}
	}
	DBG2(DBG_LIB, "  Monobit: %d/%d bits set", bits, data.len * 8);
	if (bits > param->lower && bits < param->upper)
	{
		return TRUE;
	}
	return FALSE;
}

rng_test_vector_t rng_monobit_1 = {
	RNG_WEAK, .len = 2500,
	.test = (void*)test_monobit,
	.user = &monobit_all
};

rng_test_vector_t rng_monobit_2 = {
	RNG_STRONG, .len = 2500,
	.test = (void*)test_monobit,
	.user = &monobit_all
};

rng_test_vector_t rng_monobit_3 = {
	RNG_TRUE, .len = 2500,
	.test = (void*)test_monobit,
	.user = &monobit_all
};

/**
 * Poker test
 */
typedef struct {
	double lower;
	double upper;
} poker_t;

poker_t poker_all = {
	.lower = 1.03,
	.upper = 57.4
};

static bool test_poker(poker_t *param, chunk_t data)
{
	int i, counter[16];
	double sum = 0.0;

	memset(counter, 0, sizeof(counter));

	for (i = 0; i < data.len; i++)
	{
		counter[data.ptr[i] & 0x0F]++;
		counter[(data.ptr[i] & 0xF0) >> 4]++;
	}

	for (i = 0; i < countof(counter); i++)
	{
		sum += (counter[i] * counter[i]) / 5000.0 * 16.0;
	}
	sum -= 5000.0;
	DBG2(DBG_LIB, "  Poker: %f", sum);
	if (sum > param->lower && sum < param->upper)
	{
		return TRUE;
	}
	return FALSE;
}

rng_test_vector_t rng_poker_1 = {
	RNG_WEAK, .len = 2500,
	.test = (void*)test_poker,
	.user = &poker_all
};

rng_test_vector_t rng_poker_2 = {
	RNG_STRONG, .len = 2500,
	.test = (void*)test_poker,
	.user = &poker_all
};

rng_test_vector_t rng_poker_3 = {
	RNG_TRUE, .len = 2500,
	.test = (void*)test_poker,
	.user = &poker_all
};

/**
 * Runs test
 */
typedef struct {
	int longrun;
	int lower[7];
	int upper[7];
} runs_t;

runs_t runs_all = {
	.longrun = 34,
	.lower = {-1, 2267, 1079, 502, 223,  90,  90},
	.upper = {-1, 2733, 1421, 748, 402, 223, 223},
};

static bool test_runs(runs_t *param, chunk_t data)
{
	int i, j, zero_runs[7], one_runs[7], zero = 0, one = 0, longrun = 0;

	memset(one_runs, 0, sizeof(zero_runs));
	memset(zero_runs, 0, sizeof(one_runs));

	for (i = 0; i < data.len; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (data.ptr[i] & (1<<j))
			{
				if (one)
				{
					if (++one >= param->longrun)
					{
						longrun++;
						break;
					}
				}
				else
				{
					zero_runs[min(6, zero)]++;
					zero = 0;
					one = 1;
				}
			}
			else
			{
				if (zero)
				{
					if (++zero >= param->longrun)
					{
						longrun++;
						break;
					}
				}
				else
				{
					one_runs[min(6, one)]++;
					one = 0;
					zero = 1;
				}
			}
		}
	}

	DBG2(DBG_LIB, "  Runs: zero: %d/%d/%d/%d/%d/%d, one: %d/%d/%d/%d/%d/%d, "
		 "longruns: %d",
		 zero_runs[1], zero_runs[2], zero_runs[3],
		 zero_runs[4], zero_runs[5], zero_runs[6],
		 one_runs[1], one_runs[2], one_runs[3],
		 one_runs[4], one_runs[5], one_runs[6],
		 longrun);

	if (longrun)
	{
		return FALSE;
	}

	for (i = 1; i < countof(zero_runs); i++)
	{
		if (zero_runs[i] <= param->lower[i] ||
			zero_runs[i] >=  param->upper[i] ||
			one_runs[i] <= param->lower[i] ||
			one_runs[i] >=  param->upper[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

rng_test_vector_t rng_runs_1 = {
	RNG_WEAK, .len = 2500,
	.test = (void*)test_runs,
	.user = &runs_all
};

rng_test_vector_t rng_runs_2 = {
	RNG_STRONG, .len = 2500,
	.test = (void*)test_runs,
	.user = &runs_all
};

rng_test_vector_t rng_runs_3 = {
	RNG_TRUE, .len = 2500,
	.test = (void*)test_runs,
	.user = &runs_all
};

