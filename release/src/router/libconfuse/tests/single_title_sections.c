/* Test sections with the CFGF_TITLE flag set but without CFGF_MULTI.
 * There's no reason this shouldn't work. Noticed by Josh Kropf.
 */

#include "check_confuse.h"

cfg_opt_t root_opts[] = {
	CFG_END()
};

cfg_opt_t opts[] = {
	CFG_SEC("root", root_opts, CFGF_TITLE),
	CFG_END()
};

int main(void)
{
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	fail_unless(cfg);
	cfg_free(cfg);

	return 0;
}
