/*
 * Copyright (C) 2014 Tobias Brunner
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

#include "test_suite.h"

#include <crypto/crypto_factory.h>

static rng_t *rng_create(rng_quality_t quality)
{
	rng_quality_t *q = malloc_thing(rng_quality_t);
	*q = quality;
	return (rng_t*)q;
}

static rng_t *rng_create_weak(rng_quality_t quality)
{
	ck_assert(quality == RNG_WEAK);
	return rng_create(RNG_WEAK);
}

static rng_t *rng_create_strong(rng_quality_t quality)
{
	ck_assert(quality <= RNG_STRONG);
	return rng_create(RNG_STRONG);
}

static rng_t *rng_create_true(rng_quality_t quality)
{
	ck_assert(quality <= RNG_TRUE);
	return rng_create(RNG_TRUE);
}

static rng_t *rng_create_true_second(rng_quality_t quality)
{
	fail("should never be called");
	return rng_create(RNG_TRUE);
}

static rng_quality_t rng_weak = RNG_WEAK;
static rng_quality_t rng_strong = RNG_STRONG;
static rng_quality_t rng_true = RNG_TRUE;

static struct {
	rng_quality_t *exp_weak;
	rng_quality_t *exp_strong;
	rng_quality_t *exp_true;
	struct {
		rng_quality_t *q;
		rng_constructor_t create;
	} data[4];
} rng_data[] = {
	{ NULL, NULL, NULL, {
		{ NULL, NULL }
	}},
	{ &rng_weak, NULL, NULL, {
		{ &rng_weak, rng_create_weak },
		{ NULL, NULL }
	}},
	{ &rng_strong, &rng_strong, NULL, {
		{ &rng_strong, rng_create_strong },
		{ NULL, NULL }
	}},
	{ &rng_true, &rng_true, &rng_true, {
		{ &rng_true, rng_create_true },
		{ NULL, NULL }
	}},
	{ &rng_true, &rng_true, &rng_true, {
		{ &rng_true, rng_create_true },
		{ &rng_true, rng_create_true_second },
		{ NULL, NULL }
	}},
	{ &rng_weak, &rng_true, &rng_true, {
		{ &rng_weak, rng_create_weak },
		{ &rng_true, rng_create_true },
		{ NULL, NULL }
	}},
	{ &rng_weak, &rng_strong, &rng_true, {
		{ &rng_true, rng_create_true },
		{ &rng_strong, rng_create_strong },
		{ &rng_weak, rng_create_weak },
		{ NULL, NULL }
	}},
	{ &rng_weak, &rng_strong, &rng_true, {
		{ &rng_weak, rng_create_weak },
		{ &rng_strong, rng_create_strong },
		{ &rng_true, rng_create_true },
		{ NULL, NULL }
	}},
};

static void verify_rng(crypto_factory_t *factory, rng_quality_t request,
					   rng_quality_t *expected)
{
	rng_quality_t *res;

	res = (rng_quality_t*)factory->create_rng(factory, request);
	if (!expected)
	{
		ck_assert(!res);
	}
	else
	{
		ck_assert(res);
		ck_assert_int_eq(*expected, *res);
		free(res);
	}
}

START_TEST(test_create_rng)
{
	crypto_factory_t *factory;
	int i;

	factory = crypto_factory_create();
	for (i = 0; rng_data[_i].data[i].q; i++)
	{
		ck_assert(factory->add_rng(factory, *rng_data[_i].data[i].q, "test",
								   rng_data[_i].data[i].create));
	}
	verify_rng(factory, RNG_WEAK, rng_data[_i].exp_weak);
	verify_rng(factory, RNG_STRONG, rng_data[_i].exp_strong);
	verify_rng(factory, RNG_TRUE, rng_data[_i].exp_true);
	for (i = 0; rng_data[_i].data[i].q; i++)
	{
		factory->remove_rng(factory, rng_data[_i].data[i].create);
	}
	factory->destroy(factory);
}
END_TEST

static key_exchange_t *ke_create(char *plugin)
{
	return (key_exchange_t*)plugin;
}

static key_exchange_t *ke_create_modp1024(key_exchange_method_t group, ...)
{
	ck_assert(group == MODP_1024_BIT);
	return ke_create("plugin1");
}

static key_exchange_t *ke_create_modp1024_second(key_exchange_method_t group,
												 ...)
{
	ck_assert(group == MODP_1024_BIT);
	return ke_create("plugin2");
}

static key_exchange_t *ke_create_modp2048(key_exchange_method_t group, ...)
{
	ck_assert(group == MODP_2048_BIT);
	return ke_create("plugin1");
}

static key_exchange_t *ke_create_modp2048_second(key_exchange_method_t group,
												 ...)
{
	ck_assert(group == MODP_2048_BIT);
	return ke_create("plugin2");
}

static struct {
	char *exp1024;
	char *exp2048;
	struct {
		key_exchange_method_t ke;
		ke_constructor_t create;
		char *plugin;
	} data[4];
} ke_data[] = {
	{ NULL, NULL, {
		{ KE_NONE, NULL, NULL }
	}},
	{ "plugin1", NULL, {
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ KE_NONE, NULL, NULL }
	}},
	{ "plugin1", NULL, {
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ MODP_1024_BIT, ke_create_modp1024_second, "plugin2" },
		{ KE_NONE, NULL, NULL }
	}},
	{ "plugin2", NULL, {
		{ MODP_1024_BIT, ke_create_modp1024_second, "plugin2" },
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ KE_NONE, NULL, NULL }
	}},
	{ "plugin1", "plugin1", {
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ MODP_2048_BIT, ke_create_modp2048, "plugin1" },
		{ KE_NONE, NULL }
	}},
	{ "plugin1", "plugin1", {
		{ MODP_2048_BIT, ke_create_modp2048, "plugin1" },
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ KE_NONE, NULL }
	}},
	{ "plugin1", "plugin1", {
		{ MODP_2048_BIT, ke_create_modp2048, "plugin1" },
		{ MODP_2048_BIT, ke_create_modp2048_second, "plugin2" },
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ KE_NONE, NULL }
	}},
	{ "plugin1", "plugin2", {
		{ MODP_2048_BIT, ke_create_modp2048_second, "plugin2" },
		{ MODP_2048_BIT, ke_create_modp2048, "plugin1" },
		{ MODP_1024_BIT, ke_create_modp1024, "plugin1" },
		{ KE_NONE, NULL }
	}},
};

static void verify_ke(crypto_factory_t *factory, key_exchange_method_t request,
					  char *expected)
{
	char *plugin;

	plugin = (char*)factory->create_ke(factory, request);
	if (!expected)
	{
		ck_assert(!plugin);
	}
	else
	{
		ck_assert(plugin);
		ck_assert_str_eq(expected, plugin);
	}
}

START_TEST(test_create_ke)
{
	enumerator_t *enumerator;
	crypto_factory_t *factory;
	key_exchange_method_t ke;
	char *plugin;
	int i, len = 0;


	factory = crypto_factory_create();
	for (i = 0; ke_data[_i].data[i].ke != KE_NONE; i++)
	{
		ck_assert(factory->add_ke(factory, ke_data[_i].data[i].ke,
								  ke_data[_i].data[i].plugin,
								  ke_data[_i].data[i].create));
	}
	verify_ke(factory, MODP_1024_BIT, ke_data[_i].exp1024);
	verify_ke(factory, MODP_2048_BIT, ke_data[_i].exp2048);

	len = countof(ke_data[_i].data);
	enumerator = factory->create_ke_enumerator(factory);
	for (i = 0; enumerator->enumerate(enumerator, &ke, &plugin) && i < len;)
	{
		ck_assert_int_eq(ke_data[_i].data[i].ke, ke);
		while (ke_data[_i].data[i].ke == ke)
		{	/* skip other entries of the same method */
			i++;
		}
		switch (ke)
		{
			case MODP_1024_BIT:
				ck_assert(ke_data[_i].exp1024);
				ck_assert_str_eq(ke_data[_i].exp1024, plugin);
				break;
			case MODP_2048_BIT:
				ck_assert(ke_data[_i].exp2048);
				ck_assert_str_eq(ke_data[_i].exp2048, plugin);
				break;
			default:
				fail("unexpected key exchange method");
				break;
		}
	}
	ck_assert(!enumerator->enumerate(enumerator));
	ck_assert_int_eq(ke_data[_i].data[i].ke, KE_NONE);
	enumerator->destroy(enumerator);

	for (i = 0; ke_data[_i].data[i].ke != KE_NONE; i++)
	{
		factory->remove_ke(factory, ke_data[_i].data[i].create);
	}
	factory->destroy(factory);
}
END_TEST

Suite *crypto_factory_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("crypto-factory");

	tc = tcase_create("create_rng");
	tcase_add_loop_test(tc, test_create_rng, 0, countof(rng_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("create_ke");
	tcase_add_loop_test(tc, test_create_ke, 0, countof(ke_data));
	suite_add_tcase(s, tc);

	return s;
}
