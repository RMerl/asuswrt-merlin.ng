/*
    Copyright (c)  2000, 2003		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  LGPL v2.1 or any later

    See COPYING.LIB for the status of this software.
*/

#ifndef _CLIF_H
#define _CLIF_H


typedef struct CLIF_option_struct CLIF_option;
struct CLIF_option_struct {
	const char *short_opt;
	const char *long_opt;
	const char *arg_name;
	const char *help_string;
	int (*function) (CLIF_option *optn, char *arg);
	void *data;
	int (*function_plus) (CLIF_option *optn, char *arg);
	unsigned int flags;
};
#define CLIF_END_OPTION	    { 0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct CLIF_argument_struct CLIF_argument;
struct CLIF_argument_struct {
	const char *name;
	const char *help_string;
	int (*function) (CLIF_argument *argm, char *arg, int index);
	void *data;
	unsigned int flags;
};
#define CLIF_END_ARGUMENT   { 0, 0, 0, 0, 0 }

/*  Argument flag bits.  */
#define CLIF_MORE	(0x01)	/*  null or several  */
#define CLIF_STRICT	(0x02)	/*  arg must be present   */
#define CLIF_ACC_PREV	(0x04)  /*  arg must be accompanied with previous  */


/*  Option flag bits.  */

/*  affected only by per-option flags   */
#define CLIF_EXTRA		(0x0001)  /*  don`t show in usage line   */
#define CLIF_EXIT		(0x0002)  /*  exit after handler return   */
#define CLIF_EXCL		(0x0004)  /*  at exclusive area  */

/*  affected by per-option flags and by common `parse_flags' argument
  of CLIF_parse_cmdline(). In last case appropriate bits are translated
  for all the options.
*/
#define CLIF_MAY_JOIN_ARG	(0x0010)
#define _CLIF_STRICT_JOIN_ARG	(0x0020)
#define CLIF_JOIN_ARG		(CLIF_MAY_JOIN_ARG|_CLIF_STRICT_JOIN_ARG)
#define CLIF_MAY_NOEQUAL	(0x0040)
#define _CLIF_STRICT_NOEQUAL	(0x0080)
#define CLIF_NOEQUAL		(CLIF_MAY_NOEQUAL|_CLIF_STRICT_NOEQUAL)
#define CLIF_MAY_KEYWORD	(0x0100)
#define _CLIF_STRICT_KEYWORD	(0x0200)
#define CLIF_KEYWORD		(CLIF_MAY_KEYWORD|_CLIF_STRICT_KEYWORD)
#define CLIF_MAY_ONEDASH	(0x0400)
#define _CLIF_STRICT_ONEDASH	(0x0800)
#define CLIF_ONEDASH		(CLIF_MAY_ONEDASH|_CLIF_STRICT_ONEDASH)
#define CLIF_OPTARG		(0x1000)  /*  allow missing optarg   */
#define CLIF_ABBREV		(0x2000)  /*  allow long opt abbreviation  */
#define CLIF_SEVERAL		(0x4000)  /*  several args in one opt`s arg  */

/*  affected only by common `parse_flags' arg of CLIF_parse_cmdline() .  */
#define CLIF_HELP_EMPTY		(0x10000) /*  print help on empty cmdline  */
#define CLIF_POSIX		(0x20000) /*  follow POSIX standard  */
#define CLIF_FIRST_GROUP	(0x40000) /*  first arg - options` group   */
#define CLIF_STRICT_EXCL	(0x80000) /*  at least one exclusive  */
#define CLIF_SILENT		(0x100000)	/*  no errors on stderr   */

#define CLIF_MIN_ABBREV	2	/*  a minimal match length in abbrev  */


extern int CLIF_parse (int argc, char **argv, CLIF_option *option_list,
			    CLIF_argument *arg_list, unsigned int parse_flags);
/*  history compatibility...  */
#define CLIF_parse_cmdline(ARGC,ARGV,OPTN,ARGS,FLAGS)	\
		CLIF_parse (ARGC, ARGV, OPTN, ARGS, FLAGS)

extern void CLIF_print_options (const char *header,
					const CLIF_option *option_list);
extern void CLIF_print_arguments (const char *header,
					const CLIF_argument *argument_list);
extern void CLIF_print_usage (const char *header, const char *progname, 
					const CLIF_option *option_list,
					const CLIF_argument *argument_list);

extern int CLIF_current_help (void);

/*  Common useful option handlers.  */
extern int CLIF_version_handler (CLIF_option *optn, char *arg);
extern int CLIF_set_flag (CLIF_option *optn, char *arg);
extern int CLIF_unset_flag (CLIF_option *optn, char *arg);
extern int CLIF_set_string (CLIF_option *optn, char *arg);
extern int CLIF_set_int (CLIF_option *optn, char *arg);
extern int CLIF_set_uint (CLIF_option *optn, char *arg);
extern int CLIF_set_double (CLIF_option *optn, char *arg);
extern int CLIF_call_func (CLIF_option *optn, char *arg);

extern int CLIF_arg_string (CLIF_argument *argm, char *arg, int index);
extern int CLIF_arg_int (CLIF_argument *argm, char *arg, int index);
extern int CLIF_arg_uint (CLIF_argument *argm, char *arg, int index);
extern int CLIF_arg_double (CLIF_argument *argm, char *arg, int index);
extern int CLIF_arg_func (CLIF_argument *argm, char *arg, int index);


/*  Some useful macros.  */

#define CLIF_HELP_OPTION    \
	{ 0, "help", 0, "Read this help and exit",	\
		CLIF_call_func, CLIF_current_help, 0, CLIF_EXTRA | CLIF_EXIT }
#define CLIF_VERSION_OPTION(STR)  \
	{ "V", "version", 0, "Print version info and exit",	\
		CLIF_version_handler, STR, 0, CLIF_EXTRA | CLIF_EXIT }

#endif	/*  _CLIF_H   */
