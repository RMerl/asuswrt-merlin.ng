#include "check_confuse.h"
#include <string.h>

int main(void)
{
	cfg_opt_t opts[] = {
		CFG_STR_LIST("stringproperty", 0, CFGF_NONE),
		CFG_END()
	};

	int rc;
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	fail_unless(cfg);

	rc = cfg_parse_buf(cfg,
			   " stringproperty = {\"this\"}\n"
			   " stringproperty += {\"that\"}\n"
			   " stringproperty += {\"other\"}\n");

	fail_unless(rc == CFG_SUCCESS);

	fail_unless(cfg_size(cfg, "stringproperty") == 3);

	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 0), "this") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 1), "that") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 2), "other") == 0);

	rc = cfg_parse_buf(cfg,
			   " stringproperty = \"this\"\n"
			   " stringproperty += \"that\"\n"
			   " stringproperty += \"other\"\n");

	fail_unless(rc == CFG_SUCCESS);

	fail_unless(cfg_size(cfg, "stringproperty") == 3);

	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 0), "this") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 1), "that") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "stringproperty", 2), "other") == 0);

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
