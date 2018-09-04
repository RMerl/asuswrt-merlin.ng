/* Check that double quotes gets escaped when printing the config with
 * cfg_print. Also backslashes needs to be escaped.
 */

#include <stdio.h>
#include <string.h>

#include "check_confuse.h"

cfg_opt_t opts[] = {
	CFG_STR("parameter", NULL, CFGF_NONE),
	CFG_END()
};

int main(void)
{
	FILE *fp;
	char *param;
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	fail_unless(cfg);

	/* set a string parameter to a string including a quote character
	 */
	cfg_setstr(cfg, "parameter", "text \" with quotes and \\");

	/* print the config to a temporary file
	 */
	fp = tmpfile();
	fail_unless(fp);
	cfg_print(cfg, fp);
	cfg_free(cfg);

	/* read it back, we expect 'parameter' to include a quote character
	 */
	rewind(fp);
	cfg = cfg_init(opts, CFGF_NONE);
	fail_unless(cfg);
	fail_unless(cfg_parse_fp(cfg, fp) == CFG_SUCCESS);
	fail_unless(fclose(fp) == 0);

	param = cfg_getstr(cfg, "parameter");
	fail_unless(param);

	fail_unless(strcmp(param, "text \" with quotes and \\") == 0);
	cfg_free(cfg);

	return 0;
}
