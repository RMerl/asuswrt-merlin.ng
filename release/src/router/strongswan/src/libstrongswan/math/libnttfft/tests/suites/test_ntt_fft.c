/*
 * Copyright (C) 2014-2016 Andreas Steffen
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

#include <ntt_fft.h>
#include <ntt_fft_reduce.h>

#include <time.h>

static const ntt_fft_params_t *fft_params[] = {
	&ntt_fft_17_8,
	&ntt_fft_12289_512,
	&ntt_fft_12289_1024
};

START_TEST(test_ntt_fft_impulse)
{
	ntt_fft_t *fft;
	uint16_t n = fft_params[_i]->n;
	uint32_t rq = (1 << fft_params[_i]->rlog) % fft_params[_i]->q;
	uint32_t x[n], X[n];
	int i;

	for (i = 0; i < n; i++)
	{
		x[i] = 0;
	}
	x[0] = 1;
 
	fft = ntt_fft_create(fft_params[_i]);
	fft->transform(fft, x, X, FALSE);

	for (i = 0; i < n; i++)
	{
		ck_assert(X[i] == rq);
	}
	fft->transform(fft, X, x, TRUE);

	for (i = 0; i < n; i++)
	{
		ck_assert(x[i] == (i == 0));
	}
	fft->destroy(fft);
}
END_TEST

START_TEST(test_ntt_fft_wrap)
{
	ntt_fft_t *fft;
	uint16_t n = fft_params[_i]->n;
	uint16_t q = fft_params[_i]->q;
	uint32_t x[n],y[n], X[n], Y[n];
	int i, j;

	for (i = 0; i < n; i++)
	{
		x[i] = i;
		y[i] = 0;
	}
	fft = ntt_fft_create(fft_params[_i]);
	ck_assert(fft->get_size(fft) == n);
	ck_assert(fft->get_modulus(fft) == q); 
	fft->transform(fft, x, X, FALSE);

	for (j = 0; j < n; j++)
	{
		y[j] = 1;
		fft->transform(fft, y, Y, FALSE);

		for (i = 0; i < n; i++)
		{
			Y[i] = ntt_fft_mreduce(X[i] * Y[i], fft_params[_i]);
		}
		fft->transform(fft, Y, Y, TRUE);

		for (i = 0; i < n; i++)
		{
			ck_assert(Y[i] == ( i < j ? q - n - i + j : i - j));
		}
		y[j] = 0;
	}
	fft->destroy(fft);  
}
END_TEST

START_TEST(test_ntt_fft_speed)
{
	ntt_fft_t *fft;
	struct timespec start, stop;
	int i, m, count = 10000;
	int n = fft_params[_i]->n;
	uint32_t x[n], X[n];

	for (i = 0; i < n; i++)
	{
		x[i] = i;
	}
	fft = ntt_fft_create(fft_params[_i]);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	for (m = 0; m < count; m++)
	{
		fft->transform(fft, x, X, FALSE);
		fft->transform(fft, X, x, TRUE);
	}
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);

	DBG0(DBG_LIB, "%d FFT-%d loops in %d ms\n", count, n,
				  (stop.tv_nsec - start.tv_nsec) / 1000000 +
				  (stop.tv_sec - start.tv_sec) * 1000);

	for (i = 0; i < n; i++)
	{
		ck_assert(x[i] == i);
	}
	fft->destroy(fft);
}
END_TEST

START_TEST(test_ntt_fft_init)
{
	libnttfft_init();
}
END_TEST

Suite *ntt_fft_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ntt_fft");

	tc = tcase_create("init");
	tcase_add_test(tc, test_ntt_fft_init);
	suite_add_tcase(s, tc);

	tc = tcase_create("impulse");
	tcase_add_loop_test(tc, test_ntt_fft_impulse, 0, countof(fft_params));
	suite_add_tcase(s, tc);

	tc = tcase_create("negative_wrap");
	tcase_add_loop_test(tc, test_ntt_fft_wrap, 0, countof(fft_params));
	suite_add_tcase(s, tc);

	tc = tcase_create("speed");
	tcase_set_timeout(tc, 10);
	tcase_add_loop_test(tc, test_ntt_fft_speed, 1, countof(fft_params));
	suite_add_tcase(s, tc);

	return s;
}
