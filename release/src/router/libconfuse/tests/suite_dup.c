#include "check_confuse.h"
#include <string.h>

static cfg_t *create_config(void)
{
	static cfg_opt_t sec_opts[] = {
		CFG_INT("a", 1, CFGF_NONE),
		CFG_INT("b", 2, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_SEC("sec", sec_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

	return cfg_init(opts, 0);
}

int main(void)
{
	cfg_t *acfg, *bcfg;
	cfg_t *sec;

	acfg = create_config();
	fail_unless(cfg_parse(acfg, SRC_DIR "/a.conf") == 0);

	bcfg = create_config();
	fail_unless(cfg_parse(bcfg, SRC_DIR "/b.conf") == 0);

	sec = cfg_getnsec(acfg, "sec", 0);
	fail_unless(sec != 0);
	fail_unless(cfg_size(acfg, "sec") == 1);
	fail_unless(strcmp(cfg_title(sec), "acfg") == 0);
	fail_unless(cfg_getint(sec, "a") == 5);
	fail_unless(cfg_getint(sec, "b") == 2);

	sec = cfg_getnsec(bcfg, "sec", 0);
	fail_unless(sec != 0);
	fail_unless(cfg_size(bcfg, "sec") == 1);
	fail_unless(strcmp(cfg_title(sec), "bcfg") == 0);
	fail_unless(cfg_getint(sec, "a") == 1);
	fail_unless(cfg_getint(sec, "b") == 9);

	cfg_free(acfg);
	cfg_free(bcfg);

	return 0;
}
