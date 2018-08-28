#include "check_confuse.h"
#include <string.h>
#include <sys/types.h>

void suppress_errors(cfg_t *cfg, const char *fmt, va_list ap);

static cfg_t *cfg = 0;
static int func_alias_called = 0;

static int func_alias(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv)
{
	func_alias_called = 1;

	fail_unless(cfg != 0);
	fail_unless(opt != 0);
	fail_unless(strcmp(opt->name, "alias") == 0);
	fail_unless(opt->type == CFGT_FUNC);
	fail_unless(argv != 0);
	fail_unless(strcmp(argv[0], "ll") == 0);
	fail_unless(strcmp(argv[1], "ls -l") == 0);

	if (argc != 2)
		return -1;

	return 0;
}

static void func_setup(void)
{
	cfg_opt_t opts[] = {
		CFG_FUNC("alias", func_alias),
		CFG_END()
	};

	cfg = cfg_init(opts, 0);
	/* cfg_set_error_function(cfg, suppress_errors); */
}

static void func_teardown(void)
{
	cfg_free(cfg);
}

static void func_test(void)
{
	char *buf;

	func_alias_called = 0;
	buf = "alias(ll, 'ls -l')";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(func_alias_called == 1);

	func_alias_called = 0;
	buf = "alias(ll, 'ls -l', 2)";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
	fail_unless(func_alias_called == 1);

	buf = "unalias(ll, 'ls -l')";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
}

int main(void)
{
	func_setup();
	func_test();
	func_teardown();

	return 0;
}
