/* Test cfg_include when called from a buffer
 */

#include <string.h>
#include "check_confuse.h"

/* reuse suite_dup.c profile so that a.conf could be used for testing */
cfg_opt_t sec_opts[] = {
	CFG_INT("a", 1, CFGF_NONE),
	CFG_INT("b", 2, CFGF_NONE),
	CFG_STR_LIST("list", "{}", CFGF_NONE),
	CFG_END()
};

cfg_opt_t opts[] = {
	CFG_SEC("sec", sec_opts, CFGF_MULTI | CFGF_TITLE),
	CFG_FUNC("include", &cfg_include),
	CFG_END()
};

int main(void)
{
	char *buf = "include (\"" SRC_DIR "/a.conf\")\n";
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	fail_unless(cfg);
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "sec") == 1);
	fail_unless(cfg_getint(cfg, "sec|a") == 5);
	fail_unless(cfg_getint(cfg, "sec|b") == 2);
	cfg_free(cfg);

	return 0;
}
