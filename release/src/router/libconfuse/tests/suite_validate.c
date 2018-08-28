#define _GNU_SOURCE
#include "check_confuse.h"
#include <string.h>
#include <sys/types.h>

static cfg_t *cfg = 0;

#define ACTION_NONE 0
#define ACTION_RUN 1
#define ACTION_WALK 2
#define ACTION_CRAWL 3
#define ACTION_JUMP 4

static int parse_action(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	if (strcmp(value, "run") == 0)
		*(int *)result = ACTION_RUN;
	else if (strcmp(value, "walk") == 0)
		*(int *)result = ACTION_WALK;
	else if (strcmp(value, "crawl") == 0)
		*(int *)result = ACTION_CRAWL;
	else if (strcmp(value, "jump") == 0)
		*(int *)result = ACTION_JUMP;
	else {
		/* cfg_error(cfg, "Invalid action value '%s'", value); */
		return -1;
	}
	return 0;
}

int validate_speed(cfg_t *cfg, cfg_opt_t *opt)
{
	unsigned int i;

	for (i = 0; i < cfg_opt_size(opt); i++) {
		if (cfg_opt_getnint(opt, i) <= 0) {
			/* cfg_error(cfg, "speed must be positive in section %s", cfg->name); */
			return 1;
		}
	}
	return 0;
}

int validate_speed2(cfg_t *cfg, cfg_opt_t *opt, void *value)
{
	int *val = (int *)value;

	if (*val <= 0)
		return 1;	/* Speed must be positive */

	if (*val >= 42)
		*val = 42;	/* Allow, but adjust MAX */

	return 0;
}

int validate_name2(cfg_t *cfg, cfg_opt_t *opt, void *value)
{
	char *str = (char *)value;

	if (!str || !str[0])
		return 1;	/* Must be a valid string */

	if (strlen(str) > 42)
		return 1;	/* Cannot be longer than ... */

	return 0;
}

int validate_ip(cfg_t *cfg, cfg_opt_t *opt)
{
	unsigned int i, j;

	for (i = 0; i < cfg_opt_size(opt); i++) {
		unsigned int v[4];
		char *ip = cfg_opt_getnstr(opt, i);

		if (sscanf(ip, "%u.%u.%u.%u", v + 0, v + 1, v + 2, v + 3) != 4) {
			/* cfg_error(cfg, "invalid IP address %s in section %s", ip, cfg->name); */
			return 1;
		}
		for (j = 0; j < 4; j++) {
			if (v[j] > 0xff) {
				return 1;
			}
		}
	}
	return 0;
}

int validate_action(cfg_t *cfg, cfg_opt_t *opt)
{
	cfg_opt_t *name_opt;
	cfg_t *action_sec = cfg_opt_getnsec(opt, 0);

	fail_unless(action_sec != 0);

	name_opt = cfg_getopt(action_sec, "name");

	fail_unless(name_opt != 0);
	fail_unless(cfg_opt_size(name_opt) == 1);

	if (cfg_opt_getnstr(name_opt, 0) == NULL) {
		/* cfg_error(cfg, "missing required option 'name' in section %s", opt->name); */
		return 1;
	}
	return 0;
}

void validate_setup(void)
{
	cfg_opt_t *opt = 0;

	static cfg_opt_t action_opts[] = {
		CFG_INT("speed", 0, CFGF_NONE),
		CFG_STR("name", 0, CFGF_NONE),
		CFG_INT("xspeed", 0, CFGF_NONE),
		CFG_END()
	};

	static cfg_opt_t multi_opts[] = {
		CFG_INT_LIST("speeds", 0, CFGF_NONE),
		CFG_SEC("options", action_opts, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_STR_LIST("ip-address", 0, CFGF_NONE),
		CFG_INT_CB("action", ACTION_NONE, CFGF_NONE, parse_action),
		CFG_SEC("options", action_opts, CFGF_NONE),
		CFG_SEC("multi_options", multi_opts, CFGF_MULTI),
		CFG_END()
	};

	cfg = cfg_init(opts, 0);

	cfg_set_validate_func(cfg, "ip-address", validate_ip);
	fail_unless(cfg_set_validate_func(cfg, "ip-address", validate_ip) == validate_ip);
	opt = cfg_getopt(cfg, "ip-address");
	fail_unless(opt != 0);
	fail_unless(opt->validcb == validate_ip);

	cfg_set_validate_func(cfg, "options", validate_action);
	fail_unless(cfg_set_validate_func(cfg, "options", validate_action) == validate_action);
	opt = cfg_getopt(cfg, "options");
	fail_unless(opt != 0);
	fail_unless(opt->validcb == validate_action);

	cfg_set_validate_func(cfg, "options|speed", validate_speed);
	fail_unless(cfg_set_validate_func(cfg, "options|speed", validate_speed) == validate_speed);
	opt = cfg_getopt(cfg, "options|speed");
	fail_unless(opt != 0);
	fail_unless(opt->validcb == validate_speed);

	cfg_set_validate_func(cfg, "multi_options|speeds", validate_speed);
	fail_unless(cfg_set_validate_func(cfg, "multi_options|speeds", validate_speed) == validate_speed);

	cfg_set_validate_func(cfg, "multi_options|options|xspeed", validate_speed);
	fail_unless(cfg_set_validate_func(cfg, "multi_options|options|xspeed", validate_speed) == validate_speed);

	/* Validate callbacks for *set*() functions, i.e. not when parsing file content */
	cfg_set_validate_func2(cfg, "multi_options|speed", validate_speed2);
	cfg_set_validate_func2(cfg, "multi_options|options|name", validate_name2);
}

void validate_teardown(void)
{
	cfg_free(cfg);
}

void validate_test(void)
{
	char *buf;
	unsigned int i;

	buf = "action = wlak";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "action = walk";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "action = run" " options { speed = 6 }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "action = jump" " options { speed = 2 name = 'Joe' }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "action = crawl" " options { speed = -2 name = 'Smith' }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "ip-address = { 0.0.0.0 , 1.2.3.4 , 192.168.0.254 , 10.0.0.255 , 20.30.40.256}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
	buf = "ip-address = { 0.0.0.0 , 1.2.3.4 , 192.168.0.254 , 10.0.0.255 , 20.30.40.250}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	buf = "ip-address = { 1.2.3. }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "action = run" " multi_options { speeds = {1, 2, 3, 4, 5} }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	for (i = 0; i < cfg_size(cfg, "multi_options"); i++) {
		cfg_t *multisec = cfg_getnsec(cfg, "multi_options", i);
		cfg_opt_t *speeds_opt = cfg_getopt(multisec, "speeds");

		fail_unless(speeds_opt != 0);
		fail_unless(speeds_opt->validcb == validate_speed);
	}

	buf = "action = run" " multi_options { speeds = {1, 2, 3, -4, 5} }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "action = run" " multi_options { speeds = {1, 2, 3, 4, 0} }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "action = run" " multi_options { options { xspeed = 3 } }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "action = run" " multi_options { options { xspeed = -3 } }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
}

int main(void)
{
	validate_setup();
	validate_test();
	validate_teardown();

	return 0;
}
