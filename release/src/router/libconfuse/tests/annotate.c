#include "check_confuse.h"
#include <string.h>

int main(void)
{
	char *comment;
	char *expect = "Now, is it this comment that goes with the option?";
	cfg_t *cfg;
	cfg_opt_t *opt;
	cfg_opt_t section_opts[] = {
		CFG_INT("key", 0, CFGF_NONE),
		CFG_BOOL("bool", 0, CFGF_NONE),
		CFG_STR("option", NULL, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_STR("option", NULL, CFGF_NONE),
		CFG_SEC("section", section_opts, CFGF_MULTI),
		CFG_END()
	};

	cfg = cfg_init(opts, CFGF_COMMENTS);
	fail_unless(cfg != NULL);

	fail_unless(cfg_parse(cfg, SRC_DIR "/annotate.conf") == CFG_SUCCESS);

	/* Verify the parser read the correct comment for this tricky option */
	opt = cfg_getopt(cfg, "section|option");
	fail_unless(opt != NULL);
	comment = cfg_opt_getcomment(opt);
	fail_unless(comment != NULL);
	fail_unless(strcmp(comment, expect) == 0);

	expect = "But what's the worst poetry in the universe?";
	fail_unless(cfg_opt_setcomment(opt, expect) == CFG_SUCCESS);

	cfg_opt_setnstr(opt, "Paula Nancy Millstone Jennings was a poet who wrote the worst poetry in "
			"the universe. In fact, her poetry is still considered to be the worst in "
			"the Galaxy, closely followed by that of the Azgoths of Kria and the "
			"Vogons, in that order.", 0);

	/* Verify that the comment is not reset when changing option value */
	comment = cfg_opt_getcomment(opt);
	fail_unless(comment != NULL);
	fail_unless(strcmp(comment, expect) == 0);

	cfg_print(cfg, stdout);
	fail_unless(cfg_free(cfg) == CFG_SUCCESS);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
