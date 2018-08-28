/* Test handling of parameters that should be ignored.
*/

#include <string.h>
#include <stdlib.h>
#include "check_confuse.h"

cfg_opt_t section_opts[] = {
	CFG_STR("section_parameter", NULL, CFGF_NONE),
	CFG_END()
};

cfg_opt_t opts[] = {
	CFG_STR("parameter", NULL, CFGF_NONE),
	CFG_SEC("section", section_opts, CFGF_TITLE | CFGF_MULTI),
	CFG_END()
};

static int testconfig(const char *buf)
{
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_IGNORE_UNKNOWN);
	if (!cfg)
		return 0;

	if (cfg_parse_buf(cfg, buf) != CFG_SUCCESS)
		return 0;

	cfg_free(cfg);

	return 1;
}

int main(void)
{
	/* Sanity check cases that don't need to ignore parameters. */
	fail_unless(testconfig("parameter=5"));
	fail_unless(testconfig("parameter=5\n"
			       "section mysection {\n"
			       "\tsection_parameter=1\n"
			       "}"));

	/* Ignore each type (no lists) */
	fail_unless(testconfig("unknown_int=1"));
	fail_unless(testconfig("unknown_string=\"hello\""));
	fail_unless(testconfig("unknown_float=1.1"));
	fail_unless(testconfig("unknown_bool=true"));

	/* Ignore multiple parameters */
	fail_unless(testconfig("unknown_one=1\n"
			       "unknown_two=2\n"
			       "unknown_three=3\n"));

	/* Test ignore parameters in a section */
	fail_unless(testconfig("section mysection {\n"
			       "\tunknown_section_parameter=1\n"
			       "}"));

	/* Ignore unknown section, with unknown parameters */
	fail_unless(testconfig("parameter=5\n"
			       "unknown section {\n"
			       "\tunknown_param=1\n"
			       "\tunknown_list={1,2,3,4}\n"
			       "}\n"
			       "section hej { section_parameter = \"gnejs\" }\n"
			       "parameter = \"ormbunke\""));

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
