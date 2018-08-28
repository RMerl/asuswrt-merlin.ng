#define _GNU_SOURCE
#include "check_confuse.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../src/compat.h"

static cfg_t *cfg;

int parse_ip_address(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	int i;
	unsigned int e[4];
	unsigned char *addr;

	if (sscanf(value, "%u.%u.%u.%u", e + 0, e + 1, e + 2, e + 3) != 4) {
		/*cfg_error(cfg, "invalid IP address %s in section %s", value, cfg->name); */
		return 1;
	}
	addr = (unsigned char *)malloc(sizeof(4));
	for (i = 0; i < 4; i++) {
		if (e[i] <= 0xff) {
			addr[i] = e[i];
		} else {
			free(addr);
			return 1;
		}
	}
	*(void **)result = (void *)addr;
	return 0;
}

static unsigned char *my_ether_aton(const char *addr)
{
	int i;
	static unsigned int e[6];
	static unsigned char ec[6];

	if (sscanf(addr, "%x:%x:%x:%x:%x:%x", &e[0], &e[1], &e[2], &e[3], &e[4], &e[5]) != 6) {
		return NULL;
	}
	for (i = 0; i < 6; i++) {
		if (e[i] <= 0xff)
			ec[i] = e[i];
		else
			return NULL;
	}
	return ec;
}

static char *my_ether_ntoa(unsigned char *addr)
{
	static char buf[18];

	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return buf;
}

static char *my_inet_ntoa(unsigned char *addr)
{
	static char buf[16];

	sprintf(buf, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

	return buf;
}

int parse_ether_address(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	unsigned char *tmp;

	tmp = my_ether_aton(value);
	if (tmp == 0) {
		/*cfg_error(cfg, "invalid Ethernet address %s in section %s", value, cfg->name); */
		return 1;
	}
	*(void **)result = malloc(6);
	memcpy(*(void **)result, tmp, 6);
	return 0;
}

void single_setup(void)
{
	static cfg_opt_t subsec_opts[] = {
		CFG_STR("subsubstring", "subsubdefault", CFGF_NONE),
		CFG_INT("subsubinteger", -42, CFGF_NONE),
		CFG_FLOAT("subsubfloat", 19923.1234, CFGF_NONE),
		CFG_BOOL("subsubbool", cfg_false, CFGF_NONE),
		CFG_END()
	};

	static cfg_opt_t sec_opts[] = {
		CFG_STR("substring", "subdefault", CFGF_NONE),
		CFG_INT("subinteger", 17, CFGF_NONE),
		CFG_FLOAT("subfloat", 8.37, CFGF_NONE),
		CFG_BOOL("subbool", cfg_true, CFGF_NONE),
		CFG_SEC("subsection", subsec_opts, CFGF_NONE),
		CFG_END()
	};

	static cfg_opt_t nodef_opts[] = {
		CFG_STR("string", "defvalue", CFGF_NONE),
		CFG_INT("int", -17, CFGF_NODEFAULT),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_STR("string", "default", CFGF_NONE),
		CFG_INT("integer", 4711, CFGF_NONE),
		CFG_FLOAT("float", 0.42, CFGF_NONE),
		CFG_BOOL("bool", cfg_false, CFGF_NONE),
		CFG_PTR_CB("ip-address", 0, CFGF_NONE, parse_ip_address, free),
		CFG_PTR_CB("ethernet-address", 0, CFGF_NONE, parse_ether_address, free),
		CFG_SEC("section", sec_opts, CFGF_NONE),
		CFG_STR("nodefstring", "not used", CFGF_NODEFAULT),
		CFG_SEC("nodefsec", nodef_opts, CFGF_NODEFAULT),
		CFG_END()
	};

	cfg = cfg_init(opts, 0);

	memset(opts, 0, sizeof(opts));
	memset(sec_opts, 0, sizeof(sec_opts));
	memset(subsec_opts, 0, sizeof(subsec_opts));
}

void single_teardown(void)
{
	cfg_free(cfg);
}

void single_string_test(void)
{
	char *buf;

	fail_unless(cfg_size(cfg, "string") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "string")) == 1);
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "default") == 0);
	fail_unless(cfg_getnstr(cfg, "string", 0) == cfg_getstr(cfg, "string"));
	fail_unless(cfg_getnstr(cfg, "string", 1) == 0);
	buf = "string = 'set'";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "set") == 0);
	cfg_setstr(cfg, "string", "manually set");
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "manually set") == 0);
	buf = "multi set";
	fail_unless(cfg_setmulti(cfg, "string", 1, &buf) == 0);
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "multi set") == 0);
}

void single_integer_test(void)
{
	char *buf;

	fail_unless(cfg_size(cfg, "integer") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "integer")) == 1);
	fail_unless(cfg_getint(cfg, "integer") == 4711);
	fail_unless(cfg_getnint(cfg, "integer", 0) == cfg_getint(cfg, "integer"));
	fail_unless(cfg_getnint(cfg, "integer", 1) == 0);
	buf = "integer = -46";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getint(cfg, "integer") == -46);
	cfg_setint(cfg, "integer", 999999);
	fail_unless(cfg_getint(cfg, "integer") == 999999);
	buf = "42";
	fail_unless(cfg_setmulti(cfg, "integer", 1, &buf) == 0);
	fail_unless(cfg_getint(cfg, "integer") == 42);
	buf = "ouch";
	fail_unless(cfg_setmulti(cfg, "integer", 1, &buf) == -1);
	fail_unless(cfg_getint(cfg, "integer") == 42);
}

void single_float_test(void)
{
	char *buf;

	fail_unless(cfg_size(cfg, "float") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "float")) == 1);
	fail_unless(cfg_getfloat(cfg, "float") == 0.42);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == cfg_getfloat(cfg, "float"));
	fail_unless(cfg_getnfloat(cfg, "float", 1) == 0);
	buf = "float = -46.777";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getfloat(cfg, "float") == -46.777);
	cfg_setfloat(cfg, "float", 5.1234e2);
	fail_unless(cfg_getfloat(cfg, "float") == 5.1234e2);
	buf = "4.2";
	fail_unless(cfg_setmulti(cfg, "float", 1, &buf) == 0);
	fail_unless(cfg_getfloat(cfg, "float") == 4.2);
	buf = "ouch";
	fail_unless(cfg_setmulti(cfg, "float", 1, &buf) == -1);
	fail_unless(cfg_getfloat(cfg, "float") == 4.2);
}

void single_bool_test(void)
{
	char *buf;

	fail_unless(cfg_size(cfg, "bool") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "bool")) == 1);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);

	buf = "bool = yes";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	buf = "bool = no";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);
	buf = "bool = true";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	buf = "bool = false";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);
	buf = "bool = on";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	buf = "bool = off";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);

	cfg_setbool(cfg, "bool", cfg_true);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	cfg_setbool(cfg, "bool", cfg_false);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);

	buf = "true";
	fail_unless(cfg_setmulti(cfg, "bool", 1, &buf) == 0);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	buf = "later";
	fail_unless(cfg_setmulti(cfg, "bool", 1, &buf) == -1);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
}

void single_section_test(void)
{
	char *buf;
	cfg_t *sec, *subsec;

	fail_unless(cfg_size(cfg, "section") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "section")) == 1);

	fail_unless(cfg_size(cfg, "section|subsection") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "section|subsection")) == 1);

	fail_unless(cfg_size(cfg, "section|subsection|subsubstring") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "section|subsection|subsubinteger")) == 1);

	fail_unless(cfg_title(cfg_getsec(cfg, "section")) == 0);
	fail_unless(cfg_title(cfg_getsec(cfg, "section|subsection")) == 0);

	fail_unless(strcmp(cfg_getstr(cfg, "section|substring"), "subdefault") == 0);
	sec = cfg_getsec(cfg, "section|subsection");
	fail_unless(sec != 0);
	fail_unless(cfg_getint(sec, "subsubinteger") == -42);

	fail_unless(cfg_getnsec(cfg, "section", 0) == cfg_getsec(cfg, "section"));
	fail_unless(cfg_getnsec(cfg, "section", 1) == 0);

	sec = cfg_getsec(cfg, "section");
	fail_unless(sec != 0);
	subsec = cfg_getsec(sec, "subsection");
	fail_unless(subsec != 0);
	fail_unless(cfg_getfloat(subsec, "subsubfloat") == 19923.1234);

	buf = "section { substring = \"foo\" subsection { subsubstring = \"bar\"} }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
}

void single_ptr_test(void)
{
	char *buf;
	char tmp[80];
	char addr[] = { 0xC0, 0xA8, 0x00, 0x01, 0 };	/* 192.168.0.1 */
	struct in_addr *ipaddr;
	unsigned char *etheraddr, *cmpether;

	fail_unless(cfg_size(cfg, "ip-address") == 0);

	/* Test valid IPv4 address */
	snprintf(tmp, sizeof(tmp), "ip-address = %s", my_inet_ntoa(addr));
	fail_unless(cfg_parse_buf(cfg, tmp) == CFG_SUCCESS);
	ipaddr = cfg_getptr(cfg, "ip-address");
	fail_unless(ipaddr != 0);
	fail_unless(memcmp(addr, ipaddr, 4) == 0);

	/* Test invalid IPv4 address */
	buf = "ip-address = 192.168.0.325";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "ethernet-address = '00:03:93:d4:05:58'";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	etheraddr = cfg_getptr(cfg, "ethernet-address");
	fail_unless(etheraddr != 0);
	fail_unless(my_ether_ntoa(etheraddr) != 0);
	cmpether = my_ether_aton("00:03:93:d4:05:58");
	fail_unless(memcmp(etheraddr, cmpether, 6) == 0);

	buf = "ethernet-address = '00:03:93:d4:05'";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
}

void parse_buf_test(void)
{
	char *buf;

	fail_unless(cfg_parse_buf(cfg, 0) == CFG_SUCCESS);
	fail_unless(cfg_parse_buf(cfg, "") == CFG_SUCCESS);

	buf = "bool = wrong";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "string = ";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "option = 'value'";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
}

void nonexistent_option_test(void)
{
	fail_unless(cfg_getopt(cfg, "nonexistent") == 0);
	fail_unless(cfg_getopt(cfg, "section|subnonexistent") == 0);
}

void nodefault_test(void)
{
	char *buf;
	cfg_t *nodefsec;

	fail_unless(cfg_size(cfg, "nodefstring") == 0);
	cfg_setstr(cfg, "nodefstring", "manually set");
	fail_unless(cfg_size(cfg, "nodefstring") == 1);
	fail_unless(cfg_size(cfg, "nodefsec") == 0);

	buf = "nodefsec {}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	nodefsec = cfg_getsec(cfg, "nodefsec");
	fail_unless(nodefsec != 0);
	fail_unless(cfg_size(nodefsec, "string") == 1);
	fail_unless(cfg_size(nodefsec, "int") == 0);
}

int main(void)
{
	single_setup();

	single_string_test();
	single_integer_test();
	single_float_test();
	single_bool_test();
	single_section_test();
	single_ptr_test();
	parse_buf_test();
	nonexistent_option_test();
	nodefault_test();

	single_teardown();

	return 0;
}
