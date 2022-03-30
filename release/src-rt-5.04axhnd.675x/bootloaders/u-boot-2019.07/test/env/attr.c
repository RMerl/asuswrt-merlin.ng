// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <common.h>
#include <command.h>
#include <env_attr.h>
#include <test/env.h>
#include <test/ut.h>

static int env_test_attrs_lookup(struct unit_test_state *uts)
{
	char attrs[32];

	ut_assertok(env_attr_lookup("foo:bar", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(",foo:bar", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(",foo:bar,", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(" foo:bar", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup("foo : bar", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(" foo: bar ", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup("foo:bar ", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(",foo:bar,goo:baz", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_asserteq(-ENOENT, env_attr_lookup(",,", "foo", attrs));

	ut_asserteq(-ENOENT, env_attr_lookup("goo:baz", "foo", attrs));

	ut_assertok(env_attr_lookup("foo:bar,foo:bat,foo:baz", "foo", attrs));
	ut_asserteq_str("baz", attrs);

	ut_assertok(env_attr_lookup(
		" foo : bar , foo : bat , foot : baz ", "foo", attrs));
	ut_asserteq_str("bat", attrs);

	ut_assertok(env_attr_lookup(
		" foo : bar , foo : bat , ufoo : baz ", "foo", attrs));
	ut_asserteq_str("bat", attrs);

	ut_asserteq(-EINVAL, env_attr_lookup(NULL, "foo", attrs));
	ut_asserteq(-EINVAL, env_attr_lookup("foo:bar", "foo", NULL));

	return 0;
}
ENV_TEST(env_test_attrs_lookup, 0);

#ifdef CONFIG_REGEX
static int env_test_attrs_lookup_regex(struct unit_test_state *uts)
{
	char attrs[32];

	ut_assertok(env_attr_lookup("foo1?:bar", "foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup("foo1?:bar", "foo1", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(".foo:bar", ".foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup(".foo:bar", "ufoo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_assertok(env_attr_lookup("\\.foo:bar", ".foo", attrs));
	ut_asserteq_str("bar", attrs);

	ut_asserteq(-ENOENT, env_attr_lookup("\\.foo:bar", "ufoo", attrs));

	return 0;
}
ENV_TEST(env_test_attrs_lookup_regex, 0);
#endif
