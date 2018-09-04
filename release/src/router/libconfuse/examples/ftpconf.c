/*
 * Parses and prints the configuration options for a fictous ftp client
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <confuse.h>

/* valid values for the auto-create-bookmark option */
#define ACB_YES 1
#define ACB_NO 2
#define ACB_ASK 3

/* called on alias() functions in the config file */
int conf_alias(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv)
{
	if (argc < 2) {
		cfg_error(cfg, "function '%s' requires 2 arguments", cfg_opt_name(opt));
		return -1;
	}
	printf("got alias '%s' = '%s'\n", argv[0], argv[1]);
	return 0;
}

/* parse values for the auto-create-bookmark option */
int conf_parse_acb(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	if (strcmp(value, "yes") == 0)
		*(int *)result = ACB_YES;
	else if (strcmp(value, "no") == 0)
		*(int *)result = ACB_NO;
	else if (strcmp(value, "ask") == 0)
		*(int *)result = ACB_ASK;
	else {
		cfg_error(cfg, "invalid value for option '%s': %s", cfg_opt_name(opt), value);
		return -1;
	}
	return 0;
}

/* validates a port option (must be positive) */
int conf_validate_port(cfg_t *cfg, cfg_opt_t *opt)
{
	int value = cfg_opt_getnint(opt, 0);

	if (value <= 0) {
		cfg_error(cfg, "invalid port %d in section '%s'", value, cfg_name(cfg));
		return -1;
	}
	return 0;
}

/* validates a bookmark section (host option required) */
int conf_validate_bookmark(cfg_t *cfg, cfg_opt_t *opt)
{
	cfg_t *bookmark = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

	if (cfg_size(bookmark, "host") == 0) {
		cfg_error(cfg, "missing required option 'host' in bookmark");
		return -1;
	}
	return 0;
}

cfg_t *parse_conf(const char *filename)
{
	static cfg_opt_t bookmark_opts[] = {
		CFG_STR("host", 0, CFGF_NODEFAULT),
		CFG_INT("port", 21, CFGF_NONE),
		CFG_STR("login", "anonymous", CFGF_NONE),
		CFG_STR("password", "anonymous@", CFGF_NONE),
		CFG_STR("directory", 0, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_SEC("bookmark", bookmark_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_BOOL("passive-mode", cfg_false, CFGF_NONE),
		CFG_BOOL("remote-completion", cfg_true, CFGF_NONE),
		CFG_FUNC("alias", conf_alias),
		CFG_STR_LIST("xterm-terminals", "{xterm, rxvt}", CFGF_NONE),
		CFG_INT_CB("auto-create-bookmark", ACB_YES, CFGF_NONE, conf_parse_acb),
		CFG_FUNC("include-file", cfg_include),
		CFG_END()
	};

	cfg_t *cfg = cfg_init(opts, CFGF_NONE);

	cfg_set_validate_func(cfg, "bookmark|port", conf_validate_port);
	cfg_set_validate_func(cfg, "bookmark", conf_validate_bookmark);

	switch (cfg_parse(cfg, filename)) {
	case CFG_FILE_ERROR:
		printf("warning: configuration file '%s' could not be read: %s\n", filename, strerror(errno));
		printf("continuing with default values...\n\n");
	case CFG_SUCCESS:
		break;
	case CFG_PARSE_ERROR:
		return 0;
	}

	return cfg;
}

/* Parse the file ftp.conf and print the parsed configuration options */
int main(int argc, char **argv)
{
	cfg_t *cfg;

	/* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
#endif

	cfg = parse_conf(argc > 1 ? argv[1] : "ftp.conf");
	if (cfg) {
		unsigned int i;

		printf("passive-mode = %s\n", cfg_getbool(cfg, "passive-mode") ? "true" : "false");
		printf("remote-completion = %s\n", cfg_getbool(cfg, "remote-completion") ? "true" : "false");

		printf("number of bookmarks: %d\n", cfg_size(cfg, "bookmark"));
		for (i = 0; i < cfg_size(cfg, "bookmark"); i++) {
			cfg_t *bookmark = cfg_getnsec(cfg, "bookmark", i);

			printf("  bookmark #%d: %s:%s@%s:%ld%s\n", i + 1,
			       cfg_getstr(bookmark, "login"),
			       cfg_getstr(bookmark, "password"),
			       cfg_getstr(bookmark, "host"), cfg_getint(bookmark, "port"), cfg_getstr(bookmark, "directory"));
		}

		for (i = 0; i < cfg_size(cfg, "xterm-terminals"); i++) {
			printf("xterm-terminal #%d: %s\n", i + 1, cfg_getnstr(cfg, "xterm-terminals", i));
		}

		printf("auto-create-bookmark = %ld\n", cfg_getint(cfg, "auto-create-bookmark"));
		cfg_free(cfg);
	}

	return 0;
}
