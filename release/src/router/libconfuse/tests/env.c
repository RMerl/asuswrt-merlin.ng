/* Test cfg_include when called from a buffer
 */

#include "config.h"
#include <string.h>
#include <stdlib.h>
#include "check_confuse.h"

cfg_opt_t opts[] = {
	CFG_STR("parameter", NULL, CFGF_NONE),
	CFG_END()
};

static int testconfig(const char *buf, const char *parameter)
{
	char *param;
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	if (!cfg)
		return 0;

	if (cfg_parse_buf(cfg, buf) != CFG_SUCCESS)
		return 0;

	param = cfg_getstr(cfg, "parameter");
	if (!param)
		return 0;

	if (strcmp(param, parameter) != 0)
		return 0;

	cfg_free(cfg);
	return 1;
}

int main(void)
{
#if defined(HAVE_SETENV) && defined(HAVE_UNSETENV)
	fail_unless(setenv("MYVAR", "testing", 1) == 0);
	fail_unless(unsetenv("MYUNSETVAR") == 0);
#elif defined(HAVE__PUTENV)
	fail_unless(_putenv("MYVAR=testing") == 0);
	fail_unless(_putenv("MYUNSETVAR=") == 0);
#else
#error "Not sure how to set environment variables."
#endif

	/* Check basic string parsing */
	fail_unless(testconfig("parameter=\"abc\\ndef\"", "abc\ndef"));
	fail_unless(testconfig("parameter=\"abc\\adef\"", "abc\adef"));
	fail_unless(testconfig("parameter=\"abc\\040def\"", "abc def"));
	fail_unless(testconfig("parameter=\"abc\\x20def\"", "abc def"));
	fail_unless(testconfig("parameter=\"${}\"", ""));

	/* Check unquoted environment variable handling */
	fail_unless(testconfig("parameter=${MYVAR}", "testing"));
	fail_unless(testconfig("parameter=${MYVAR:-default}", "testing"));
	fail_unless(testconfig("parameter=${MYUNSETVAR}", ""));
	fail_unless(testconfig("parameter=${MYUNSETVAR:-default}", "default"));

	/* Check quoted environment variable handling */
	fail_unless(testconfig("parameter=\"${MYVAR}\"", "testing"));
	fail_unless(testconfig("parameter=\"${MYVAR:-default}\"", "testing"));
	fail_unless(testconfig("parameter=\"${MYUNSETVAR}\"", ""));
	fail_unless(testconfig("parameter=\"${MYUNSETVAR:-default}\"", "default"));

	/* Check quoted environment variable handling in the middle of strings */
	fail_unless(testconfig("parameter=\"text_${MYVAR}\"", "text_testing"));
	fail_unless(testconfig("parameter=\"${MYVAR}_text\"", "testing_text"));
	fail_unless(testconfig("parameter=\"start_${MYVAR}_end\"", "start_testing_end"));

	/* Check single quoted environment variable handling */
	fail_unless(testconfig("parameter='${MYVAR}'", "${MYVAR}"));

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
