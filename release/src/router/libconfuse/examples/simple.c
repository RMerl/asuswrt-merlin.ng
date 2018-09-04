#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "confuse.h"

int main(void)
{
	static cfg_bool_t verbose = cfg_false;
	static char *server = NULL;
	static double delay = 1.356e-32;
	static char *username = NULL;

	/* Although the macro used to specify an integer option is called
	 * CFG_SIMPLE_INT(), it actually expects a long int. On a 64 bit system
	 * where ints are 32 bit and longs 64 bit (such as the x86-64 or amd64
	 * architectures), you will get weird effects if you use an int here.
	 *
	 * If you use the regular (non-"simple") options, ie CFG_INT() and use
	 * cfg_getint(), this is not a problem as the data types are implicitly
	 * cast.
	 */
	static long int debug = 1;

	cfg_opt_t opts[] = {
		CFG_SIMPLE_BOOL("verbose", &verbose),
		CFG_SIMPLE_STR("server", &server),
		CFG_SIMPLE_STR("user", &username),
		CFG_SIMPLE_INT("debug", &debug),
		CFG_SIMPLE_FLOAT("delay", &delay),
		CFG_END()
	};
	cfg_t *cfg;

	/* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
#endif

	/* set default value for the server option */
	server = strdup("gazonk");

	cfg = cfg_init(opts, 0);
	cfg_parse(cfg, "simple.conf");

	printf("verbose: %s\n", verbose ? "true" : "false");
	printf("server: %s\n", server);
	printf("username: %s\n", username);
	printf("debug: %ld\n", debug);
	printf("delay: %G\n", delay);

	printf("setting username to 'foo'\n");
	/* using cfg_setstr here is not necessary at all, the equivalent
	 * code is:
	 *   free(username);
	 *   username = strdup("foo");
	 */
	cfg_setstr(cfg, "user", "foo");
	printf("username: %s\n", username);

	/* print the parsed values to another file */
	{
		FILE *fp = fopen("simple.conf.out", "w");

		cfg_print(cfg, fp);
		fclose(fp);
	}

	cfg_free(cfg);

	/* You are responsible for freeing string values. */
	free(server);
	free(username);

	return 0;
}
