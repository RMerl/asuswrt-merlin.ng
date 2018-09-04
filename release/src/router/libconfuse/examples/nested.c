/* Example by Thomas Adam */

#include <confuse.h>
#include <stdio.h>
#include <string.h>

#define CONF "nested.conf"

int main(void)
{
	cfg_opt_t group_opts[] = {
		CFG_INT("number", 0, CFGF_NONE),
		CFG_INT("total", 0, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t groups_opts[] = {
		CFG_STR("name", "Esmé", CFGF_NONE),
		CFG_SEC("group", group_opts, CFGF_TITLE | CFGF_MULTI),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_SEC("groups", groups_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_t *cfg, *sec;
	size_t i, j;

	cfg = cfg_init(opts, CFGF_NONE);
	if (!cfg || cfg_parse(cfg, CONF) == CFG_PARSE_ERROR) {
		perror("Failed parsing " CONF);
		return 1;
	}

	/* Iterate over the sections and print fields from each section. */
	for (i = 0; i < cfg_size(cfg, "groups"); i++) {
		sec = cfg_getnsec(cfg, "groups", i);

		for (j = 0; j < cfg_size(sec, "group"); j++) {
			cfg_t *opt = cfg_getnsec(sec, "group", j);

			printf("group title:  '%s'\n", cfg_title(opt));
			printf("group number:  %ld\n", cfg_getint(opt, "number"));
			printf("group total:   %ld\n", cfg_getint(opt, "total"));
			printf("\n");
		}
	}

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
