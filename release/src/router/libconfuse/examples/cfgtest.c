#include "confuse.h"
#include <string.h>

void print_func(cfg_opt_t *opt, unsigned int index, FILE *fp)
{
	fprintf(fp, "%s(foo)", opt->name);
}

void print_ask(cfg_opt_t *opt, unsigned int index, FILE *fp)
{
	int value = cfg_opt_getnint(opt, index);

	switch (value) {
	case 1:
		fprintf(fp, "yes");
		break;
	case 2:
		fprintf(fp, "no");
		break;
	case 3:
	default:
		fprintf(fp, "maybe");
		break;
	}
}

/* function callback
 */
int cb_func(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv)
{
	int i;

	/* at least one parameter is required */
	if (argc == 0) {
		cfg_error(cfg, "Too few parameters for the '%s' function", opt->name);
		return -1;
	}

	printf("cb_func() called with %d parameters:\n", argc);
	for (i = 0; i < argc; i++)
		printf("parameter %d: '%s'\n", i, argv[i]);
	return 0;
}

/* value parsing callback
 *
 * VALUE must be "yes", "no" or "maybe", and the corresponding results
 * are the integers 1, 2 and 3.
 */
int cb_verify_ask(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	if (strcmp(value, "yes") == 0)
		*(long int *)result = 1;
	else if (strcmp(value, "no") == 0)
		*(long int *)result = 2;
	else if (strcmp(value, "maybe") == 0)
		*(long int *)result = 3;
	else {
		cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
		return -1;
	}
	return 0;
}

int cb_validate_bookmark(cfg_t *cfg, cfg_opt_t *opt)
{
	/* only validate the last bookmark */
	cfg_t *sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

	if (!sec) {
		cfg_error(cfg, "section is NULL!?");
		return -1;
	}
	if (cfg_getstr(sec, "machine") == 0) {
		cfg_error(cfg, "machine option must be set for bookmark '%s'", cfg_title(sec));
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	unsigned int i;
	cfg_t *cfg;
	unsigned n;
	int ret;

	cfg_opt_t proxy_opts[] = {
		CFG_INT("type", 0, CFGF_NONE),
		CFG_STR("host", 0, CFGF_NONE),
		CFG_STR_LIST("exclude", "{localhost, .localnet}", CFGF_NONE),
		CFG_INT("port", 21, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t bookmark_opts[] = {
		CFG_STR("machine", 0, CFGF_NONE),
		CFG_INT("port", 21, CFGF_NONE),
		CFG_STR("login", 0, CFGF_NONE),
		CFG_STR("password", 0, CFGF_NONE),
		CFG_STR("directory", 0, CFGF_NONE),
		CFG_BOOL("passive-mode", cfg_false, CFGF_NONE),
		CFG_SEC("proxy", proxy_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT("backlog", 42, CFGF_NONE),
		CFG_STR("probe-device", "eth2", CFGF_NONE),
		CFG_SEC("bookmark", bookmark_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_FLOAT_LIST("delays", "{3.567e2, 0.2, -47.11}", CFGF_NONE),
		CFG_FUNC("func", &cb_func),
		CFG_INT_CB("ask-quit", 3, CFGF_NONE, &cb_verify_ask),
		CFG_INT_LIST_CB("ask-quit-array", "{maybe, yes, no}",
				CFGF_NONE, &cb_verify_ask),
		CFG_FUNC("include", &cfg_include),
		CFG_END()
	};

#ifndef _WIN32
	/* for some reason, MS Visual C++ chokes on this (?) */
	printf("Using %s\n\n", confuse_copyright);
#endif

	cfg = cfg_init(opts, CFGF_NOCASE);

	/* set a validating callback function for bookmark sections */
	cfg_set_validate_func(cfg, "bookmark", &cb_validate_bookmark);

	ret = cfg_parse(cfg, argc > 1 ? argv[1] : "test.conf");
	printf("ret == %d\n", ret);
	if (ret == CFG_FILE_ERROR) {
		perror("test.conf");
		return 1;
	} else if (ret == CFG_PARSE_ERROR) {
		fprintf(stderr, "parse error\n");
		return 2;
	}

	printf("backlog == %ld\n", cfg_getint(cfg, "backlog"));

	printf("probe device is %s\n", cfg_getstr(cfg, "probe-device"));
	cfg_setstr(cfg, "probe-device", "lo");
	printf("probe device is %s\n", cfg_getstr(cfg, "probe-device"));

	n = cfg_size(cfg, "bookmark");
	printf("%d configured bookmarks:\n", n);
	for (i = 0; i < n; i++) {
		cfg_t *pxy;
		cfg_t *bm = cfg_getnsec(cfg, "bookmark", i);

		printf("  bookmark #%u (%s):\n", i + 1, cfg_title(bm));
		printf("    machine = %s\n", cfg_getstr(bm, "machine"));
		printf("    port = %d\n", (int)cfg_getint(bm, "port"));
		printf("    login = %s\n", cfg_getstr(bm, "login"));
		printf("    passive-mode = %s\n", cfg_getbool(bm, "passive-mode") ? "true" : "false");
		printf("    directory = %s\n", cfg_getstr(bm, "directory"));
		printf("    password = %s\n", cfg_getstr(bm, "password"));

		pxy = cfg_getsec(bm, "proxy");
		if (pxy) {
			int j, m;

			if (cfg_getstr(pxy, "host") == 0) {
				printf("      no proxy host is set, setting it to 'localhost'...\n");
				/* For sections without CFGF_MULTI flag set, there is
				 * also an extended syntax to get an option in a
				 * subsection:
				 */
				cfg_setstr(bm, "proxy|host", "localhost");
			}
			printf("      proxy host is %s\n", cfg_getstr(pxy, "host"));
			printf("      proxy type is %ld\n", cfg_getint(pxy, "type"));
			printf("      proxy port is %ld\n", cfg_getint(pxy, "port"));

			m = cfg_size(pxy, "exclude");
			printf("      got %d hosts to exclude from proxying:\n", m);
			for (j = 0; j < m; j++) {
				printf("        exclude %s\n", cfg_getnstr(pxy, "exclude", j));
			}
		} else
			printf("    no proxy settings configured\n");
	}

	printf("delays are (%d):\n", cfg_size(cfg, "delays"));
	for (i = 0; i < cfg_size(cfg, "delays"); i++)
		printf(" %G\n", cfg_getnfloat(cfg, "delays", i));

	printf("ask-quit == %ld\n", cfg_getint(cfg, "ask-quit"));

	/* Using cfg_setint(), the integer value for the option ask-quit
	 * is not verified by the value parsing callback.
	 *
	 *
	 cfg_setint(cfg, "ask-quit", 4);
	 printf("ask-quit == %ld\n", cfg_getint(cfg, "ask-quit"));
	 */

	/* The following commented line will generate a failed assertion
	 * and abort, since the option "foo" is not declared
	 *
	 *
	 printf("foo == %ld\n", cfg_getint(cfg, "foo"));
	 */

	cfg_addlist(cfg, "ask-quit-array", 2, 1, 2);

	for (i = 0; i < cfg_size(cfg, "ask-quit-array"); i++)
		printf("ask-quit-array[%d] == %ld\n", i, cfg_getnint(cfg, "ask-quit-array", i));

	/* print the parsed values to another file */
	{
		FILE *fp = fopen("test.conf.out", "w");

		cfg_set_print_func(cfg, "func", print_func);
		cfg_set_print_func(cfg, "ask-quit", print_ask);
		cfg_set_print_func(cfg, "ask-quit-array", print_ask);
		cfg_print(cfg, fp);
		fclose(fp);
	}

	cfg_free(cfg);
	return 0;
}
