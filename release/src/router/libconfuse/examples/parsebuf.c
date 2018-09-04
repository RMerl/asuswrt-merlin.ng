/* Very simple example for parse buf example */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "confuse.h"

int main(void)
{
	cfg_t *cfg;
	cfg_opt_t arg_opts[] = {
		CFG_STR("value", "default", CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT("delay", 3, CFGF_NONE),
		CFG_STR("message", "This is a message", CFGF_NONE),
		CFG_SEC("argument", arg_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

	char *buf =
		" delay = 3\n"
		"# message = 'asdfasfasfd tersf'\n"
		" argument one { value = bar }\n"
		" argument two { value=foo}\n";

	cfg = cfg_init(opts, 0);
	if (cfg_parse_buf(cfg, buf) != CFG_SUCCESS) {
		fprintf(stderr, "Failed parsing configuration: %s\n", strerror(errno));
		exit(1);
	}
	cfg_print(cfg, stdout);

	return cfg_free(cfg);
}

