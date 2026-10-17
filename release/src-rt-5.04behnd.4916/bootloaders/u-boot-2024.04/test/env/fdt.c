#include <common.h>
#include <command.h>
#include <env_attr.h>
#include <test/env.h>
#include <test/ut.h>

static int env_test_fdt_import(struct unit_test_state *uts)
{
	const char *val;

	val = env_get("from_fdt");
	ut_assertnonnull(val);
	ut_asserteq_str("yes", val);

	val = env_get("fdt_env_path");
	ut_assertnull(val);

	return 0;
}
ENV_TEST(env_test_fdt_import, 0);
