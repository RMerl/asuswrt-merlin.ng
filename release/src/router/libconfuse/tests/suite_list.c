#include "check_confuse.h"
#include <string.h>

static cfg_t *cfg;
static int numopts = 0;

static void list_setup(void)
{
	static cfg_opt_t subsec_opts[] = {
		CFG_STR_LIST("subsubstring", 0, CFGF_NONE),
		CFG_INT_LIST("subsubinteger", 0, CFGF_NONE),
		CFG_FLOAT_LIST("subsubfloat", 0, CFGF_NONE),
		CFG_BOOL_LIST("subsubbool", 0, CFGF_NONE),
		CFG_END()
	};

	static cfg_opt_t sec_opts[] = {
		CFG_STR_LIST("substring", "{subdefault1, subdefault2}", CFGF_NONE),
		CFG_INT_LIST("subinteger", "{17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 300}", CFGF_NONE),	/* 14 values */
		CFG_FLOAT_LIST("subfloat", "{8.37}", CFGF_NONE),
		CFG_BOOL_LIST("subbool", "{true}", CFGF_NONE),
		CFG_SEC("subsection", subsec_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_STR_LIST("string", "{default1, default2, default3}", CFGF_NONE),
		CFG_INT_LIST("integer", "{4711, 123456789}", CFGF_NONE),
		CFG_FLOAT_LIST("float", "{0.42}", CFGF_NONE),
		CFG_BOOL_LIST("bool", "{false, true, no, yes, off, on}", CFGF_NONE),
		CFG_SEC("section", sec_opts, CFGF_MULTI),
		CFG_END()
	};

	cfg = cfg_init(opts, 0);
	numopts = cfg_numopts(opts);
	fail_unless(numopts == 5);

	memset(opts, 0, sizeof(opts));
	memset(sec_opts, 0, sizeof(sec_opts));
	memset(subsec_opts, 0, sizeof(subsec_opts));
}

static void list_teardown(void)
{
	cfg_free(cfg);
}

static void list_string_test(void)
{
	char *buf;
	char *multi[2];

	fail_unless(cfg_size(cfg, "string") == 3);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "string")) == 3);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 0), "default1") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 1), "default2") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 2), "default3") == 0);
	buf = "string = {\"manually\", 'set'}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "string") == 2);
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "manually") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 1), "set") == 0);
	cfg_setstr(cfg, "string", "manually set");
	fail_unless(strcmp(cfg_getstr(cfg, "string"), "manually set") == 0);
	cfg_setnstr(cfg, "string", "foobar", 1);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 1), "foobar") == 0);

	cfg_addlist(cfg, "string", 3, "foo", "bar", "baz");
	fail_unless(cfg_size(cfg, "string") == 5);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 3), "bar") == 0);

	cfg_setlist(cfg, "string", 3, "baz", "foo", "bar");
	fail_unless(cfg_size(cfg, "string") == 3);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 0), "baz") == 0);

	buf = "string += {gaz, 'onk'}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "string") == 5);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 3), "gaz") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 4), "onk") == 0);

	multi[0] = "bar";
	multi[1] = "foo";
	fail_unless(cfg_setmulti(cfg, "string", 2, multi) == 0);
	fail_unless(cfg_size(cfg, "string") == 2);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 0), "bar") == 0);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 1), "foo") == 0);
}

static void list_integer_test(void)
{
	char *buf;
	char *multi[3];

	fail_unless(cfg_size(cfg, "integer") == 2);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "integer")) == 2);

	fail_unless(cfg_getint(cfg, "integer") == 4711);
	fail_unless(cfg_getnint(cfg, "integer", 1) == 123456789);
	buf = "integer = {-46}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "integer") == 1);
	fail_unless(cfg_getnint(cfg, "integer", 0) == -46);
	cfg_setnint(cfg, "integer", 999999, 1);
	fail_unless(cfg_size(cfg, "integer") == 2);
	fail_unless(cfg_getnint(cfg, "integer", 1) == 999999);

	cfg_addlist(cfg, "integer", 3, 11, 22, 33);
	fail_unless(cfg_size(cfg, "integer") == 5);
	fail_unless(cfg_getnint(cfg, "integer", 3) == 22);

	cfg_setlist(cfg, "integer", 3, 11, 22, 33);
	fail_unless(cfg_size(cfg, "integer") == 3);
	fail_unless(cfg_getnint(cfg, "integer", 0) == 11);

	buf = "integer += {-1234567890, 1234567890}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "integer") == 5);
	fail_unless(cfg_getnint(cfg, "integer", 3) == -1234567890);
	fail_unless(cfg_getnint(cfg, "integer", 4) == 1234567890);

	multi[0] = "42";
	multi[1] = "17";
	fail_unless(cfg_setmulti(cfg, "integer", 2, multi) == 0);
	fail_unless(cfg_size(cfg, "integer") == 2);
	fail_unless(cfg_getnint(cfg, "integer", 0) == 42);
	fail_unless(cfg_getnint(cfg, "integer", 1) == 17);

	multi[0] = "17";
	multi[1] = "bad";
	multi[2] = "42";
	fail_unless(cfg_setmulti(cfg, "integer", 3, multi) == -1);
	fail_unless(cfg_size(cfg, "integer") == 2);
	fail_unless(cfg_getnint(cfg, "integer", 0) == 42);
	fail_unless(cfg_getnint(cfg, "integer", 1) == 17);
}

static void list_float_test(void)
{
	char *buf;
	char *multi[3];

	fail_unless(cfg_size(cfg, "float") == 1);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "float")) == 1);

	fail_unless(cfg_getfloat(cfg, "float") == 0.42);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == 0.42);

	buf = "float = {-46.777, 0.1, 0.2, 0.17, 17.123}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "float")) == 5);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == -46.777);
	fail_unless(cfg_getnfloat(cfg, "float", 1) == 0.1);
	fail_unless(cfg_getnfloat(cfg, "float", 2) == 0.2);
	fail_unless(cfg_getnfloat(cfg, "float", 3) == 0.17);
	fail_unless(cfg_getnfloat(cfg, "float", 4) == 17.123);

	cfg_setnfloat(cfg, "float", 5.1234e2, 1);
	fail_unless(cfg_getnfloat(cfg, "float", 1) == 5.1234e2);

	cfg_addlist(cfg, "float", 1, 11.2233);
	fail_unless(cfg_size(cfg, "float") == 6);
	fail_unless(cfg_getnfloat(cfg, "float", 5) == 11.2233);

	cfg_setlist(cfg, "float", 2, .3, -18.17e-7);
	fail_unless(cfg_size(cfg, "float") == 2);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == 0.3);
	fail_unless(cfg_getnfloat(cfg, "float", 1) == -18.17e-7);

	buf = "float += {64.64, 1234.567890}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "float") == 4);
	fail_unless(cfg_getnfloat(cfg, "float", 2) == 64.64);
	fail_unless(cfg_getnfloat(cfg, "float", 3) == 1234.567890);

	multi[0] = "42";
	multi[1] = "0.17";
	fail_unless(cfg_setmulti(cfg, "float", 2, multi) == 0);
	fail_unless(cfg_size(cfg, "float") == 2);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == 42);
	fail_unless(cfg_getnfloat(cfg, "float", 1) == 0.17);

	multi[0] = "42";
	multi[1] = "bad";
	multi[2] = "0.17";
	fail_unless(cfg_setmulti(cfg, "float", 3, multi) == -1);
	fail_unless(cfg_size(cfg, "float") == 2);
	fail_unless(cfg_getnfloat(cfg, "float", 0) == 42);
	fail_unless(cfg_getnfloat(cfg, "float", 1) == 0.17);
}

static void list_bool_test(void)
{
	char *buf;
	char *multi[3];

	fail_unless(cfg_size(cfg, "bool") == 6);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "bool")) == 6);
	fail_unless(cfg_getnbool(cfg, "bool", 0) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 2) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 3) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 4) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 5) == cfg_true);

	buf = "bool = {yes, yes, no, false, true}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "bool") == 5);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 2) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 3) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 4) == cfg_true);

	cfg_setbool(cfg, "bool", cfg_false);
	fail_unless(cfg_getbool(cfg, "bool") == cfg_false);
	cfg_setnbool(cfg, "bool", cfg_false, 1);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_false);

	cfg_addlist(cfg, "bool", 2, cfg_true, cfg_false);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "bool")) == 7);
	fail_unless(cfg_getnbool(cfg, "bool", 5) == cfg_true);

	cfg_setlist(cfg, "bool", 4, cfg_true, cfg_true, cfg_false, cfg_true);
	fail_unless(cfg_size(cfg, "bool") == 4);
	fail_unless(cfg_getnbool(cfg, "bool", 0) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 2) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 3) == cfg_true);

	buf = "bool += {false, false}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "bool") == 6);
	fail_unless(cfg_getnbool(cfg, "bool", 4) == cfg_false);
	fail_unless(cfg_getnbool(cfg, "bool", 5) == cfg_false);

	multi[0] = "true";
	multi[1] = "no";
	fail_unless(cfg_setmulti(cfg, "bool", 2, multi) == 0);
	fail_unless(cfg_size(cfg, "bool") == 2);
	fail_unless(cfg_getnbool(cfg, "bool", 0) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_false);

	multi[0] = "false";
	multi[1] = "maybe";
	multi[2] = "yes";
	fail_unless(cfg_setmulti(cfg, "bool", 3, multi) == -1);
	fail_unless(cfg_size(cfg, "bool") == 2);
	fail_unless(cfg_getnbool(cfg, "bool", 0) == cfg_true);
	fail_unless(cfg_getnbool(cfg, "bool", 1) == cfg_false);
}

static void list_section_test(void)
{
	char *buf;
	cfg_t *sec, *subsec;
	cfg_opt_t *opt;

	/* size should be 0 before any section has been parsed. Since the
	 * CFGF_MULTI flag is set, there are no default sections.
	 */
	fail_unless(cfg_size(cfg, "section") == 0);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "section")) == 0);
	fail_unless(cfg_size(cfg, "section|subsection") == 0);
	fail_unless(cfg_opt_size(cfg_getopt(cfg, "section|subsection")) == 0);

	buf = "section {}";	/* add a section with default values */
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "section") == 1);

	sec = cfg_getsec(cfg, "section");
	fail_unless(sec != 0);
	fail_unless(strcmp(sec->name, "section") == 0);
	fail_unless(cfg_title(sec) == NULL);

	opt = cfg_getopt(sec, "subsection");
	fail_unless(opt != 0);
	fail_unless(cfg_opt_size(opt) == 0);
	fail_unless(cfg_size(sec, "subsection") == 0);

	fail_unless(strcmp(cfg_getnstr(sec, "substring", 0), "subdefault1") == 0);
	subsec = cfg_getsec(cfg, "section|subsection");
	fail_unless(subsec == 0);

	buf = "section { subsection 'foo' { subsubfloat = {1.2, 3.4, 5.6} } }";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "section") == 2);

	sec = cfg_getnsec(cfg, "section", 1);
	fail_unless(sec != 0);
	fail_unless(strcmp(cfg_title(cfg_getnsec(sec, "subsection", 0)), "foo") == 0);
	fail_unless(cfg_size(sec, "subinteger") == 14);

	subsec = cfg_getsec(sec, "subsection");
	fail_unless(cfg_size(subsec, "subsubfloat") == 3);
	fail_unless(cfg_getnfloat(subsec, "subsubfloat", 2) == 5.6);
	fail_unless(cfg_getnstr(subsec, "subsubstring", 0) == 0);

	sec = cfg_getnsec(cfg, "section", 0);
	fail_unless(sec != 0);
	fail_unless(cfg_size(sec, "subsection") == 0);
	buf = "subsection 'bar' {}";
	fail_unless(cfg_parse_buf(sec, buf) == CFG_SUCCESS);
	fail_unless(cfg_size(sec, "subsection") == 1);
	subsec = cfg_getnsec(sec, "subsection", 0);
	fail_unless(subsec != 0);
	fail_unless(strcmp(cfg_title(subsec), "bar") == 0);
	fail_unless(cfg_getnfloat(subsec, "subsubfloat", 2) == 0);

	buf = "subsection 'baz' {}";
	fail_unless(cfg_parse_buf(sec, buf) == CFG_SUCCESS);
	fail_unless(cfg_gettsec(sec, "subsection", "bar") == subsec);
	opt = cfg_getopt(sec, "subsection");
	fail_unless(opt != 0);
	fail_unless(cfg_gettsec(sec, "subsection", "baz") == cfg_opt_gettsec(opt, "baz"));
	fail_unless(cfg_opt_gettsec(opt, "bar") == subsec);
	fail_unless(cfg_opt_gettsec(opt, "foo") == 0);
	fail_unless(cfg_gettsec(sec, "subsection", "section") == 0);

	fail_unless(cfg_gettsec(cfg, "section", "baz") == 0);
}

static void parse_buf_test(void)
{
	char *buf;

	fail_unless(cfg_parse_buf(cfg, 0) == CFG_SUCCESS);
	fail_unless(cfg_parse_buf(cfg, "") == CFG_SUCCESS);

	buf = "bool = {true, true, false, wrong}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
	buf = "string = {foo, bar";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

	buf = "/* this is a comment */ bool = {true} /*// another comment */";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "/*/*/ bool = {true}//  */";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "/////// this too is a comment\nbool = {true} // another one here /* comment */";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "# this is a shell comment\nbool = {true} # another shell comment here //* comment *//";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "######## this is also a shell comment\nbool = {true} ## Use the force, Luke";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	/* Issue #98: Line of only hashes cause segfault */
	buf = "##############################################\n# some text #\n##############################################\nbool = {false}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	buf = "////////////////////////////////////////////////////////////////////////////////////////////\n// some text //\n////////////////////////////////////////////////////////////////////////////////////////////\nbool = {false}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	buf = "/******************************************************************************************/\n// some text //\n/******************************************************************************************/\nbool = {false}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

	buf = "string={/usr/local/}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(strcmp(cfg_getnstr(cfg, "string", 0), "/usr/local/") == 0);
}

static void nonexistent_option_test(void)
{
	char *buf;

	fail_unless(cfg_numopts(cfg->opts) == numopts);
	fail_unless(cfg_getopt(cfg, "nonexistent") == 0);

	buf = "section {}";
	fail_unless(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
	fail_unless(cfg_getopt(cfg, "section|subnonexistent") == 0);
}

int main(void)
{
	list_setup();

	list_string_test();
	list_integer_test();
	list_float_test();
	list_bool_test();
	list_section_test();
	parse_buf_test();
	nonexistent_option_test();

	list_teardown();

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
