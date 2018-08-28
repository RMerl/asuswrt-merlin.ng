/* Example of how to deprecate and drop options by Sebastian Geiger */
#include "confuse.h"
#include <string.h>

int main(void)
{
	int repeat;
	size_t i;
	cfg_opt_t opts[] = {
		CFG_STR_LIST("targets", "{World}", CFGF_DEPRECATED),
		CFG_INT("repeat", 1, CFGF_DEPRECATED),
		CFG_INT("foobar", 1, CFGF_DEPRECATED | CFGF_DROP),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	if (cfg_parse(cfg, "deprecated.conf") == CFG_PARSE_ERROR)
		return 1;

	repeat = cfg_getint(cfg, "repeat");
	while (repeat--) {
		printf("Hello");
		for (i = 0; i < cfg_size(cfg, "targets"); i++)
			printf(", %s", cfg_getnstr(cfg, "targets", i));
		printf("!\n");
	}

	cfg_print_indent(cfg, stdout, 4);
	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
