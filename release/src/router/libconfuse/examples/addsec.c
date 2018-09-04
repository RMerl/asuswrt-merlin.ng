#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <string.h>
#include <locale.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "confuse.h"

cfg_t *cfg = NULL;
const char *config_filename = "./reread.conf";

void read_config(void)
{
	static cfg_opt_t arg_opts[] = {
		CFG_STR("value", "default", CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT("delay", 3, CFGF_NONE),
		CFG_STR("message", "This is a message", CFGF_NONE),
		CFG_SEC("argument", arg_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

	cfg = cfg_init(opts, CFGF_NONE);
	if (cfg_parse(cfg, config_filename) != CFG_SUCCESS) {
		fprintf(stderr, "Failed parsing configuration: %s\n", strerror(errno));
		exit(1);
	}
}

void print_message()
{
	size_t i;

	printf("Message: %s", cfg_getstr(cfg, "message"));
	for (i = 0; i < cfg_size(cfg, "argument"); i++) {
		cfg_t *arg;

		arg = cfg_getnsec(cfg, "argument", i);
		if (arg)
			printf(", %s", cfg_getstr(arg, "value"));
	}
	printf("\n");
}

int main(void)
{
	cfg_t* sec;

	/* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
#endif

	read_config();
	print_message();

	/* Add a new section */
	sec = cfg_addtsec(cfg, "argument", "two");
	cfg_setstr(sec, "value", "foo");
	print_message();

	cfg_free(cfg);

	return 0;

}
