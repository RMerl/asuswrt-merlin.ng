#include "check_confuse.h"
#include <string.h>

int main(void)
{
	static cfg_opt_t section_opts[] = {
		CFG_STR("prop", 0, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_SEC("section", section_opts, CFGF_TITLE | CFGF_MULTI),
		CFG_END()
	};

	const char *config_data =
		"section title_one { prop = 'value_one' }\n"
		"section title_two { prop = 'value_two' }\n";

	int rc;
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	fail_unless(cfg);

	rc = cfg_parse_buf(cfg, config_data);
	fail_unless(rc == CFG_SUCCESS);

	fail_unless(cfg_addtsec(cfg, "section", "title_three"));
	fail_unless(cfg_size(cfg, "section") == 3);
	fail_unless(cfg_title(cfg_gettsec(cfg, "section", "title_three")));

	/* attempt to add a pre-existing section should fail */
	fail_unless(!cfg_addtsec(cfg, "section", "title_three"));

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
