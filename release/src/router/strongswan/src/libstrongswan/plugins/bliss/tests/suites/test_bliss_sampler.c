/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "test_suite.h"

#include <bliss_sampler.h>

static u_int key_size[] = { 1, 3, 4};

START_TEST(test_bliss_sampler_gaussian)
{
	bliss_sampler_t *sampler;
	const bliss_param_set_t *set;
	int i, k, count;
	uint32_t hist[8], sign[3];
	int32_t z;
	ext_out_function_t alg;
	size_t seed_len;
	chunk_t seed;

	set = bliss_param_set_get_by_id(key_size[_i]);
	alg = XOF_MGF1_SHA256;
	seed_len = 32;
	count = 10000000;

	seed = chunk_alloc(seed_len);
	memset(seed.ptr, 0xcc, seed_len);

	for (k = 0; k < 3; k++)
	{
		sign[k] = 0;
	}
	for (k = 0; k < 8; k++)
	{
		hist[k] = 0;
	}

	sampler = bliss_sampler_create(alg, seed, set);
	for (i = 0; i < count; i++)
	{
		ck_assert(sampler->gaussian(sampler, &z));
		if (z == 0)
		{
			sign[1]++;
			hist[0]++;
		}
		else if (z > 0)
		{
			sign[2]++;
			hist[z/256]++;
		}
		else
		{
			sign[0]++;
			hist[(-z)/256]++;
		}
	}
	sampler->destroy(sampler);
	free(seed.ptr);

	DBG1(DBG_LIB, "histogram");
	for (k = 0; k < 8; k++)
	{
		DBG1(DBG_LIB, "%d %7d", k, hist[k]);
	}
	DBG1(DBG_LIB, "- %7d", sign[0]);
	DBG1(DBG_LIB, "0 %7d", sign[1]);
	DBG1(DBG_LIB, "+ %7d", sign[2]);
}
END_TEST

Suite *bliss_sampler_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bliss_sampler");

	tc = tcase_create("sampler_gaussian");
	tcase_set_timeout(tc, 30);
	tcase_add_loop_test(tc, test_bliss_sampler_gaussian, 0, countof(key_size));
	suite_add_tcase(s, tc);

	return s;
}
