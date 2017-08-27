/*
    Copyright (c)  2000, 2003		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  LGPL v2.1 or any later

    See COPYING.LIB for the status of this software.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "clif.h"


#if 1	/*  Bad idea, anyway...  */
#define MAX_ARGC_NUMBER	256
typedef unsigned char _CLIF_index;
#else
#define MAX_ARGC_NUMBER	(4096 / 5 + 1)	/*  POSIX ARG_MAX >= 4096 ...  */
typedef unsigned short _CLIF_index;
#endif


/*  This is needed for some print info functions.
   This is ugly for thread-safe (is it really actual on program invoking?),
   and for several CLIF_parse_cmdline invoking... But foo on this. Yeah...
*/
static struct {
	int argc;
	char **argv;
	CLIF_option *option_list;
	CLIF_argument *argument_list;
	unsigned int parse_flags;
} curr = { 0, };
	

static void err_report (const char *format, ...) {
	va_list ap;

	if (curr.parse_flags & CLIF_SILENT)
		return;

	va_start (ap, format);

	vfprintf (stderr, format, ap);

	va_end (ap);

	fprintf (stderr, "\n");

	return;
}


/*  info generation stuff...   */

#define SHORT_PLUS_MINUS	"+/-"
#define LONG_PLUS_MINUS		"++/--"
#define EXCL_DLM		" | "

static char *show_short (const CLIF_option *optn) {
	static char buf[80];
	char *p = buf;
	unsigned int flags = optn->flags | curr.parse_flags;

	if (optn->function_plus) {
	    if (!optn->function)  *p++ = '+';
	    else {
		strcpy (p, SHORT_PLUS_MINUS);
		p += sizeof (SHORT_PLUS_MINUS) - 1;
	    }
	} else
	    *p++ = '-';

	*p++ = optn->short_opt[0];

	if (optn->arg_name) {
	    char *endp = buf + sizeof (buf) - sizeof (",...]");
	    const char *s;

	    if (!(flags & _CLIF_STRICT_JOIN_ARG))  *p++ = ' ';
	    if (flags & CLIF_OPTARG)  *p++ = '[';

	    s = optn->arg_name;
	    while (*s && p < endp)  *p++ = *s++;

	    if (flags & CLIF_SEVERAL) {
		strcpy (p, ",...");
		p += sizeof (",...") - 1;	/*  last '\0' ...  */
	    }

	    if (flags & CLIF_OPTARG)  *p++ = ']';
	}

	*p = '\0';

	return buf;
}
	
static char *show_long (const CLIF_option *optn) {
	static char buf[80];
	char *p = buf;
	char *endp;
	const char *s;
	unsigned int flags = optn->flags | curr.parse_flags;


	if (!(flags & _CLIF_STRICT_KEYWORD)) {

	    if (!(flags & _CLIF_STRICT_ONEDASH)) {
		if (optn->function_plus) {
		    if (!optn->function)  {  *p++ = '+'; *p++ = '+';  }
		    else {
			strcpy (p, LONG_PLUS_MINUS);
			p += sizeof (LONG_PLUS_MINUS) - 1;
		    }
		} else {  *p++ = '-'; *p++ = '-';  }

	    } else {
		if (optn->function_plus) {
		    if (!optn->function)  *p++ = '+';
		    else {
			strcpy (p, SHORT_PLUS_MINUS);
			p += sizeof (SHORT_PLUS_MINUS) - 1;
		    }
		} else  *p++ = '-';
	    }
	}

	s = optn->long_opt;
	endp = buf + sizeof (buf) - sizeof (" [");
	while (*s && p < endp)  *p++ = *s++;

	if (optn->arg_name) {

	    if (flags & _CLIF_STRICT_NOEQUAL) {
		*p++ = ' ';
		if (flags & CLIF_OPTARG)  *p++ = '[';
	    } else {
		if (flags & CLIF_OPTARG)  *p++ = '[';
		*p++ = '=';
	    }

	    s = optn->arg_name;
	    endp = buf + sizeof (buf) - sizeof (",...]");
	    while (*s && p < endp)  *p++ = *s++;

	    if (flags & CLIF_SEVERAL) {
		strcpy (p, ",...");
		p += sizeof (",...") - 1;	/*  last '\0' ...  */
	    }

	    if (flags & CLIF_OPTARG)  *p++ = ']';
	}

	*p = '\0';

	return buf;
}

static char *show_excl (const CLIF_option *option_list, int *cnt_p) {
	static char buf[256];
	const CLIF_option *optn;
	char *p = buf;
	char *endp = buf + sizeof (buf) - sizeof (EXCL_DLM);
	int excl_cnt = 0;

	*p = '\0';
	if (cnt_p)  *cnt_p = 0;
	if (!option_list)  return buf;

	for (optn = option_list; optn->short_opt || optn->long_opt; optn++) {
	    char *s;

	    if (!(optn->flags & CLIF_EXCL))  continue;

	    if (optn->short_opt)  s = show_short (optn);
	    else  s = show_long (optn);

	    if (excl_cnt > 0) {	    /*  i.e., second etc...  */
		    strcpy (p, EXCL_DLM);
		    p += sizeof (EXCL_DLM) - 1;
	    }

	    while (*s && p < endp)  *p++ = *s++;

	    excl_cnt++;
	}

	*p = '\0';

	if (cnt_p)  *cnt_p = excl_cnt;

	return buf;
}


static int is_keyword (const CLIF_option *optn) {
	unsigned int flags = optn->flags | curr.parse_flags;

	return  (flags & _CLIF_STRICT_KEYWORD) != 0;
}


static void err_bad_opt (const char *arg, char c, int n) {
	char sym = (*arg == '+') ? '+' : '-';

	if (c)  err_report ("Bad option `%c%c' (argc %d)", sym, c, n);
	else {
	    char *p = strchr (arg, '=');
	    const char *type = (*arg == sym) ? "option" : "keyword";

	    if (p)
		err_report ("Bad %s `%s' (with arg `%s') (argc %d)",
							type, arg, p + 1, n);
	    else
		err_report ("Bad %s `%s' (argc %d)", type, arg, n);
	}
}

static void err_bad_arg (const CLIF_option *optn, char c, int n) {
	CLIF_option tmp = *optn;
	char ss[80];
	char *s;

	tmp.arg_name = NULL;

	if (c) {
	    s = show_short (&tmp);	/*  always without arg...  */
	    strncpy (ss, s, sizeof (ss));
	    s = show_short (optn);
	} else {
	    s = show_long (&tmp);	/*  always without arg...  */
	    strncpy (ss, s, sizeof (ss));
	    s = show_long (optn);
	}

	err_report ("%s `%s' (argc %d) requires an argument: `%s'",
		    (c || !is_keyword (optn)) ? "Option" : "Keyword", ss, n, s);
}
	
static void err_bad_res (const CLIF_option *optn, char c,
					const char *opt_arg, int n) {
	CLIF_option tmp = *optn;
	char *ss;
	const char *type;

	tmp.arg_name = NULL;

	if (c) {
	    ss = show_short (&tmp);
	    type = "option";
	} else {
	    ss = show_long (&tmp);
	    type = is_keyword (optn) ? "keyword" : "option";
	}

	if (optn->arg_name)
	    err_report ("Cannot handle `%s' %s with arg `%s' (argc %d)",
							ss, type, opt_arg, n);
	else
	    err_report ("Cannot handle `%s' %s (argc %d)", ss, type, n);
}

static void err_bad_excl (const CLIF_option *optn, char c, int n) {
	CLIF_option tmp = *optn;
	char *ss;
	char *excl = show_excl (curr.option_list, 0);
				/*  Note: show_(short|long)() nested!!! */

	tmp.arg_name = NULL;

	if (c)  ss = show_short (&tmp);
	else  ss = show_long (&tmp);

	err_report ("%s `%s' (argc %d): Only one of:\n    %s\n"
		    "may be specified.",
		    (c || !is_keyword (optn)) ? "Option" : "Keyword",
							    ss, n, excl);
}


static CLIF_option *find_long (char *arg, char **arg_p,
				unsigned int match, unsigned int nomatch) {
	CLIF_option *optn;
	CLIF_option *abbrev = NULL;
	char *abbrev_arg = NULL;
	int abbrev_found = 0;


	for (optn = curr.option_list;
		optn->short_opt || optn->long_opt;
		    optn++
	) {
	    char *a;
	    const char *o;
	    unsigned int flags;

	    if (!optn->long_opt)  continue;

	    flags = curr.parse_flags | optn->flags;
	    if (flags & nomatch)  continue;
	    if (match && !(flags & match))  continue;	/*  XXX: optimize it */


	    for (a = arg, o = optn->long_opt; *o && *a == *o; a++, o++) ;

	    if (*a == '\0' ||
		(*a == '=' && optn->arg_name && !(flags & _CLIF_STRICT_NOEQUAL))
	    ) {	    /*  looks like end of option...  */

		if (!*o) {	/*  explicit match found   */
		    if (*a == '=' && arg_p)  *arg_p = a + 1;
		    return optn;
		}

		if ((flags & CLIF_ABBREV) &&
		    (a - arg >= CLIF_MIN_ABBREV)
		) {
		    if (!abbrev_found) {
			abbrev_found = 1;
			abbrev = optn;
			if (*a == '=')  abbrev_arg = a + 1;
		    } else	/*  several possibility case...  */
			abbrev = NULL;
		}
	    }
	}

	if (abbrev) {	/*  implicit match found   */
	    if (abbrev_arg && arg_p)  *arg_p = abbrev_arg;
	    return abbrev;
	} else		/*  no match found   */
	    return NULL;
}

static int check_sym (const CLIF_option *optn, char sym) {

	if (sym == '+') {
	    if (!optn->function_plus)  return -1;
	}
	else if (sym == '-') {
	    if (!optn->function && optn->function_plus)
		    return -1;
	}

	return 0;
}

static int call_function (CLIF_option *optn, char *opt_arg, char sym) {
	int (*function) (CLIF_option *, char *);

	function = (sym == '+') ? optn->function_plus : optn->function;

	if (!function)  return 0;

	if (opt_arg && ((optn->flags | curr.parse_flags) & CLIF_SEVERAL)) {
	    char tmp[80];
	    char *t;
	    char *endt = tmp + sizeof (tmp);

	    while (*opt_arg) {
    
		t = tmp;
		while (t < endt && *opt_arg &&
		       *opt_arg != ' ' && *opt_arg != '\t' && *opt_arg != ','
		)  *t++ = *opt_arg++;

		if (t >= endt)  return -1;
    
		*t = '\0';
    
		if (function (optn, tmp) < 0)  return -1;
    
		while (*opt_arg == ' ' || *opt_arg == '\t' || *opt_arg == ',')
			opt_arg++;
	    }

	    return 0;
	}

	return  function (optn, opt_arg);
}


int CLIF_parse_cmdline (int argc, char *argv[],
			CLIF_option *option_list,
			CLIF_argument *argument_list,
			unsigned int parse_flags) {
	int i, j;
	CLIF_option *optn;
	CLIF_argument *argm;
	int num_args = 0;
	int num_argm = 0, strict_beg = 0, strict_end = 0;
	_CLIF_index arg_n[MAX_ARGC_NUMBER];
	unsigned int dirty_flags = 0;
	int dirty_plus = 0;
	int exclusive_cnt = 0;
	int posix = getenv ("POSIXLY_CORRECT") != NULL ||
					    (parse_flags & CLIF_POSIX);

	curr.argc = argc;
	curr.argv = argv;
	curr.option_list = option_list;
	curr.argument_list = argument_list;
	curr.parse_flags = parse_flags;

	if (argc <= 1 && (parse_flags & CLIF_HELP_EMPTY)) {
		CLIF_current_help ();
		exit (0);
	}

	/*  Scan argument_list for check and some info.  */

	if (argument_list) {
	    enum stages { STRICT_BEG, OPTIONAL, STRICT_END };
	    int stage = STRICT_BEG;

	    for (argm = argument_list; argm->name; argm++) {
		
		if (argm->flags & CLIF_STRICT) {

		    if (stage == STRICT_BEG)  strict_beg++;
		    else if (stage == OPTIONAL) {
			stage = STRICT_END;
			strict_end++;
		    }
		    else if (stage == STRICT_END)
			    strict_end++;
		} else {
		    if (stage == STRICT_BEG)  stage = OPTIONAL;
		    else if (stage == STRICT_END) {
			err_report ("Incorrect argument list set in program "
				    "source: more than one optional area.");
			return -1;
		    }
		}

		num_argm++;
	    }
	}

	/*  Scan option_list for some info.  */
	if (option_list) {

	    dirty_flags = parse_flags;

	    for (optn = option_list;
		    optn->short_opt || optn->long_opt;
			optn++
	    ) {
		dirty_flags |= optn->flags;
		if (optn->function_plus)  dirty_plus = 1;
	    }
	}

	if (dirty_flags & CLIF_EXCL)
		exclusive_cnt = 1;	/*  only one is allowed...  */


	/*  Go !   Store arguments, parse options.  */

	for (i = 1; i < argc; i++) {
	    char *arg = argv[i];
	    char *opt_arg = NULL;
	    char sym = '-';

	    if (!option_list)
		    goto  handle_arg;

	    if (*arg == '+' && dirty_plus)
		    sym = '+';

	    if (*arg != sym) {	/*  argument or keyword   */

		if (dirty_flags & CLIF_MAY_KEYWORD) {
		    optn = find_long (arg, &opt_arg, CLIF_MAY_KEYWORD, 0);
		    if (optn)  goto long_found;
		}

		if (num_args == 0 && (parse_flags & CLIF_FIRST_GROUP)) {
		    /*  ugly...  */
		    parse_flags &= ~CLIF_FIRST_GROUP;
		    dirty_flags &= ~CLIF_FIRST_GROUP;	/*  to be correct   */

		    goto  handle_short;
		}

		/*  else it is an argument   */
		goto  handle_arg;

	    }
	    else if (*++arg == sym) {	/*  `--' - long option   */
		arg++;

		if (*arg == sym ||	/*  `---' - let it be not option... */
		    (parse_flags & (_CLIF_STRICT_KEYWORD|_CLIF_STRICT_ONEDASH)) 
		) {
		    arg -= 2;
		    goto  handle_arg;	/*  not option anyway  */
		}
	
		optn = find_long (arg, &opt_arg, 0,
				_CLIF_STRICT_KEYWORD | _CLIF_STRICT_ONEDASH);
		if (optn)  goto long_found;
	
		/*  XXX: May be allow only for `--', not `++' too...  */
		if (!*arg && sym == '-') {  /*  `--' and no empty longoption */
		    option_list = NULL;	    /*  POSIX way...  */
		    continue;
		}
	
		/*  XXX: or treat as an argument sometimes???  */
		err_bad_opt (argv[i], 0, i);
		return -1;
	    }
	    else {	/*  short option, or several short options...  */

		if (dirty_flags & CLIF_MAY_ONEDASH) {
		    optn = find_long (arg, &opt_arg, CLIF_MAY_ONEDASH, 0);
		    if (optn)  goto long_found;
		}
    
		if (!*arg) {	/*  POSIX say: only "stdout specification"... */
		    arg--;
		    goto handle_arg;
		}
    
		goto  handle_short;
	    }


    long_found:	
	    if (check_sym (optn, sym) < 0) {	/*  Oops...  */
		err_bad_opt (argv[i], 0, i);
		return -1;
	    }

	    if (optn->flags & CLIF_EXCL) {
		if (!exclusive_cnt) {
		    err_bad_excl (optn, 0, i);
		    return -1;
		}
		exclusive_cnt--;
	    }
		
	    if (optn->arg_name && !opt_arg) {
		unsigned int flags = optn->flags | parse_flags;

		if (++i >= argc ||
		    !(flags & CLIF_MAY_NOEQUAL)
		) {	/*  missing opt arg   */
		    i--;

		    if (!(flags & CLIF_OPTARG)) {
			err_bad_arg (optn, 0, i);
			return -1;
		    }

		    opt_arg = NULL;
		} else
		    opt_arg = argv[i];
		   
	    }


	    if (call_function (optn, opt_arg, sym) < 0) {
		err_bad_res (optn, 0, opt_arg, i);
		return -1;
	    }

	    if (optn->flags & CLIF_EXIT)
		    exit (0);

	    continue;


    handle_arg:
	    if (argument_list) {
		if (i < MAX_ARGC_NUMBER)    /*  XXX: ugly, better report   */
			arg_n[num_args++] = i;
	    } else {
		err_report ("`%s' (argc %d): arguments are not allowed",
								 argv[i], i);
		return -1;
	    }

	    /*  POSIX say: No more options after args...  */
	    if (posix)  option_list = NULL;	/*  geniously...  */
	
	    continue;


    handle_short:

	    opt_arg = NULL;

	    do {

		for (optn = option_list;
			optn->short_opt || optn->long_opt;
			    optn++
		) {
		    if (optn->short_opt && optn->short_opt[0] == *arg)
			    break;
		}
		if (!optn->short_opt ||
		    check_sym (optn, sym) < 0
		) {
		    err_bad_opt (argv[i], *arg, i);
		    return -1;
		}

		if (optn->flags & CLIF_EXCL) {
		    if (!exclusive_cnt) {
			err_bad_excl (optn, *arg, i);
			return -1;
		    }
		    exclusive_cnt--;
		}


		if (optn->arg_name) {
		    unsigned int flags = parse_flags | optn->flags;

		    if (arg[1] == '\0') {	/*  a last one   */

			/*  POSIX say: an option with arg cannot be grouped. */
			if (posix && arg != argv[i] && arg[-1] != sym) {
				err_bad_arg (optn, *arg, i);	/*  good way? */
				return -1;
			}

			if (++i >= argc ||
			    (flags & _CLIF_STRICT_JOIN_ARG)
			) {
			    i--;

			    if (!(flags & CLIF_OPTARG)) {
				err_bad_arg (optn, *arg, i);
				return -1;
			    }

			    opt_arg = NULL;
			} else
			    opt_arg = argv[i];
		    }
		    else if ((arg == argv[i] || arg[-1] == sym) &&	
			     (flags & CLIF_MAY_JOIN_ARG)
		    ) {
			opt_arg = ++arg;
		    }
		    else {	/*  inside a group...  */
			if (!(flags & CLIF_OPTARG) ||
			    (flags & CLIF_MAY_JOIN_ARG)
			) {
			    err_bad_arg (optn, *arg, i);
			    return -1;
			}

			opt_arg = NULL;
		    }
		}

		if (call_function (optn, opt_arg, sym) < 0) {
		    err_bad_res (optn, optn->short_opt[0], opt_arg, i);
		    return -1;
		}

		if (optn->flags & CLIF_EXIT)
			exit (0);

	    } while (!opt_arg && *++arg);

	}	/*  for ( ...  )   */


	if ((parse_flags & CLIF_STRICT_EXCL) && exclusive_cnt != 0) {
		err_report ("One of these must be specified:\n    %s\n",
						    show_excl (option_list, 0));
		return -1;
	}


	/*  Now, after *ALL* options, handle arguments, if any.  */

	if (num_args < strict_beg + strict_end) {
	    /*  Missing some needed arguments.  */

	    if (num_args < strict_beg)  argm = argument_list + num_args;
	    else
		argm = argument_list +
			    ((num_args - strict_beg) + (num_argm - strict_end));

	    if (num_args == strict_beg + strict_end - 1)
		err_report ("Specify \"%s\" missing argument.", argm->name);
	    else
		err_report ("Specify \"%s\" and other missing arguments.",
								    argm->name);
	    return -1;
	}

	if (num_args > 0) {
	    _CLIF_index argm_index[MAX_ARGC_NUMBER];

	    /*  assing argm (by index) for each arg...  */
		    
	    for (i = 0, j = 0; i < strict_beg; i++, j++)
		    argm_index[i] = j;
	    for (i = num_args - strict_end, j = num_argm - strict_end;
			i < num_args; i++, j++
	    )  argm_index[i] = j;
	    for (i = strict_beg, j = strict_beg;
		    i < num_args - strict_end && j < num_argm - strict_end;
			i++
	    ) {
		argm_index[i] = j;
		if (!(argument_list[j].flags & CLIF_MORE))
			j++;
	    }

	    if (i < num_args - strict_end) {	/*  there are extra args...  */
		err_report ("Extra arg `%s' (position %d, argc %d)",
				    argv[arg_n[i]], i + 1, arg_n[i]);
		return -1;
	    }

	    if (j < num_argm - strict_end &&	
		!(argument_list[j].flags & CLIF_MORE) &&
		/*  ...i.e, there are some missing optional args...  */
		(argument_list[j].flags & CLIF_ACC_PREV)
	    ) {
		if (j == 0)
		    err_report ("Incorrect argument list set: first arg "
				"cannot be `accompanied with previous'.");
		else
		    err_report ("Arg \"%s\" must be specified because "
				"\"%s\" `%s' is used.", argument_list[j].name,
				argument_list[j - 1].name, argv[arg_n[i - 1]]);
		return -1;
	    }
		
	    if (argm_index[--i] == j &&
		    /*  above is true only after OPTIONAL area scan
		       and when `j' is stopped on CLIF_MORE  */
		++j < num_argm - strict_end
		    /*  i.e: there is a *last* one (after CLIF_MORE)
			in the OPTIONAL area  */
	    )  argm_index[i] = j;	/*  *last* is better than *more*   */
		    

	    /*  ...and work now   */

	    for (i = 0; i < num_args; i++) {
		argm = argument_list + argm_index[i];

		if (argm->function &&
		    argm->function (argm, argv[arg_n[i]], i) < 0
		) {
		    err_report ("Cannot handle \"%s\" cmdline arg `%s' "
				"on position %d (argc %d)",
				argm->name, argv[arg_n[i]], i + 1, arg_n[i]);
		    return -1;
		}
	    }

	    /*  That`s all.  */
	}

	return 0;
}


static void box_output (int start, int left, int width, const char *str,
							const char *arg_name) {
	char *p, *endp, *s;
	int l;
	char buf[1024];
	char spacer[128];	/*  assume it is enough   */

	if (left > sizeof (spacer) - 2)  left = sizeof (spacer) - 2;
	if (width > sizeof (buf) - 1)  width = sizeof (buf) - 1;

	spacer[0] = '\n';
	memset (spacer + 1, ' ', left);
	spacer[left + 1] = '\0';


	l = left - start;
	if (l > 0) {
	    memset (buf, ' ', l);
	    buf[l] = '\0';
	    fprintf (stderr, "%s", buf);
	} else 
	    fprintf (stderr, "%s", spacer);


	endp = buf + width;

	p = buf;

	while (*str) {

	    while (*str && p < endp) {
    
		if (*str == '%' && arg_name) {
		    if (str[1] == '%') {
			*p++ = '%';
			str += 2;
			continue;
		    } 
		    else if (str[1] == 's') {
			const char *a = arg_name;
    
			while (*a && p < endp)  *p++ = *a++;
			str += 2;
			continue;
		    } 
		}
    
		*p++ = *str++;
	    }
    
	    *p = '\0';
    
	    if (p < endp)  break;

    
	    while (p > buf && *p != ' ' && *p != '\t')  p--;
	    if (p <= buf)  return;	/*  foo on you   */
		    
	    *p = '\0';
	    fprintf (stderr, "%s", buf);
	    fprintf (stderr, "%s", spacer);
		
	    p++;
	    for (s = buf; *p; *s++ = *p++) ;
	    *s = '\0';
	    p = s;
	}


	fprintf (stderr, "%s", buf);
		
	return;
}


#define SHORT_LONG_DLM	"  "
#define OPT_START_DLM	"  "
#define OPT_FIELD_WIDTH	30

#define ARG_MARK_STRICT	"+     "
#define ARG_MARK_GROUP0	"  .   "
#define ARG_MARK_GROUP	"  '   "
#define ARG_MARK_OPT	"      "
#define ARG_FIELD_WIDTH	20

#define SCREEN_WIDTH	80


void CLIF_print_options (const char *header,
				const CLIF_option *option_list) {
	const CLIF_option *optn;
	char *excl;
	int excl_cnt = 0;

	/*  Print a header string, if present...  */
	if (header)  fprintf (stderr, "%s\n", header);

	if (!option_list)  return;


	for (optn = option_list; optn->short_opt || optn->long_opt; optn++) {
	    int len;

	    /*  generate and print an option usage   */

	    if (optn->short_opt) {
		if (optn->long_opt)
		    len = fprintf (stderr, OPT_START_DLM "%s"
					   SHORT_LONG_DLM "%s",
					show_short (optn), show_long (optn));
		else
		    len = fprintf (stderr, OPT_START_DLM "%s",
						    show_short (optn));
	    } else
		len = fprintf (stderr, OPT_START_DLM "%s", show_long (optn));


	    /*  print a help string, if present   */
	    
	    if (optn->help_string)
		    box_output (len, OPT_FIELD_WIDTH,
				SCREEN_WIDTH - OPT_FIELD_WIDTH,
				optn->help_string, optn->arg_name);

	    fprintf (stderr, "\n");	/*  a last one   */
	}

	excl = show_excl (option_list, &excl_cnt);
	if (excl_cnt > 0) {

	    if (excl_cnt == 1) {
		if ((curr.parse_flags & CLIF_STRICT_EXCL) &&
		    curr.option_list == option_list
		)  fprintf (stderr, "Anyway `%s' must be specified.\n", excl);
		else  /*  simple ordinary option, because excl_cnt == 1 ... */;
	    } else
	        fprintf (stderr, "Only one of these may be specified:\n"
				 "    %s\n", excl);
	}

	return;
}
		    

void CLIF_print_arguments (const char *header,
				const CLIF_argument *argument_list) {
	const CLIF_argument *argm;


	if (!argument_list)  return;

	/*  Print a header string, if present...  */
	if (header)  fprintf (stderr, "%s\n", header);


	for (argm = argument_list; argm->name; argm++) {
	    int len;

	    if (argm->flags & CLIF_STRICT)
		len = fprintf (stderr, ARG_MARK_STRICT "%s", argm->name);
	    else if (argm->flags & CLIF_MORE)
		len = fprintf (stderr, ARG_MARK_OPT "%s ...", argm->name);
	    else if (argm->flags & CLIF_ACC_PREV)
		len = fprintf (stderr, ARG_MARK_GROUP "%s", argm->name);
	    else if ((argm + 1)->name && ((argm + 1)->flags & CLIF_ACC_PREV))
		len = fprintf (stderr, ARG_MARK_GROUP0 "%s", argm->name);
	    else
		len = fprintf (stderr, ARG_MARK_OPT "%s", argm->name);

	    if (argm->help_string)
		    box_output (len, ARG_FIELD_WIDTH,
				    SCREEN_WIDTH - ARG_FIELD_WIDTH,
				    argm->help_string, argm->name);

	    fprintf (stderr, "\n");
	}

	return;
}


void CLIF_print_usage (const char *header, const char *progname, 
				const CLIF_option *option_list,
				const CLIF_argument *argument_list) {

	if (!progname && curr.argv)
		progname = curr.argv[0];

	if (!header) {
	    if (progname)
		fprintf (stderr, "Usage: %s", progname);
	    else
		fprintf (stderr, "Command line options:");
	} else {
	    if (progname)
		fprintf (stderr, "%s\n" OPT_START_DLM "%s", header, progname);
	    else
		fprintf (stderr, "%s", header);
	}


	if (option_list) {
	    const CLIF_option *optn;
	    char m_buf[256], p_buf[256], mp_buf[256];
	    char *m = m_buf, *p = p_buf, *mp = mp_buf;
	    char *end_m = m_buf + sizeof (m_buf) - 1;
	    char *end_p = p_buf + sizeof (p_buf) - 1;
	    char *end_mp = mp_buf + sizeof (mp_buf) - 1;
	    char *excl;
	    int excl_cnt = 0;


	    /*  first, show exclusive option list, if any...  */

	    excl = show_excl (option_list, &excl_cnt);
	    if (excl_cnt > 0) {
		if ((curr.parse_flags & CLIF_STRICT_EXCL) &&
		    curr.option_list == option_list
		) {
		    if (excl_cnt == 1)
			    fprintf (stderr, " %s", excl);
		    else
			fprintf (stderr, " { %s }", excl);
		} else
		    fprintf (stderr, " [ %s ]", excl);
	    }


	    /*  second, find short options without arguments...  */

	    for (optn = option_list;
		    optn->short_opt || optn->long_opt;
			optn++
	    ) {
		/*  We don`t exclude CLIF_EXTRA hear:
		   simple one char don`t eat a lot of space...
		*/

		if (!optn->short_opt ||
		    optn->arg_name ||
		    (optn->flags & CLIF_EXCL)
		)  continue;

		if (optn->function_plus) {
		    if (optn->function) {
			if (mp < end_mp)  *mp++ = optn->short_opt[0];
		    } else {
			if (p < end_p)  *p++ = optn->short_opt[0];
		    }
		} else {
		    if (m < end_m)  *m++ = optn->short_opt[0];
		}
	    }

	    if (m > (char *) m_buf) {
		*m = '\0';
		fprintf (stderr, " [ -%s ]", m_buf);
	    }
	    if (p > (char *) p_buf) {
		*p = '\0';
		fprintf (stderr, " [ +%s ]", p_buf);
	    }
	    if (mp > (char *) mp_buf) {
		*mp = '\0';
		fprintf (stderr, " [ " SHORT_PLUS_MINUS "%s ]", mp_buf);
	    }


	    /*  third, print all another...  */

	    for (optn = option_list;
		    optn->short_opt || optn->long_opt;
			optn++
	    ) {
		if (optn->flags & CLIF_EXTRA)  continue;

		if (optn->flags & CLIF_EXCL)
			continue;	/*  already handled   */

		if (optn->short_opt) {
		    if (optn->arg_name) 
			fprintf (stderr, " [ %s ]", show_short (optn));
		    else
			/*  already handled   */;
		} else
		    fprintf (stderr, " [ %s ]", show_long (optn));
	    }
	}
		        

	if (argument_list) {
	    const CLIF_argument *argm;
	    int deep = 0;

	    for (argm = argument_list; argm->name; argm++) {

		if (argm->flags & CLIF_STRICT) {
		    if (deep > 0) {
			fputc (' ', stderr);
			while (deep--)  fputc (']', stderr);
			deep = 0;
		    }
			
		    fprintf (stderr, " %s", argm->name);
		} else {
		    if (argm->flags & CLIF_MORE)
			    fprintf (stderr, " [ %s ...", argm->name);
		    else if (argm->flags & CLIF_ACC_PREV) {
			    fprintf (stderr, " %s", argm->name);
			    --deep;	/*  ugly, but easy   */
		    } else
			fprintf (stderr, " [ %s", argm->name);

		    deep++;
		}
	    }

	    if (deep > 0) {
		fputc (' ', stderr);
		while (deep--)  fputc (']', stderr);
	    }
	}


	fprintf (stderr, "\n");
}


int CLIF_current_help (void) {

	if (!curr.argc)  return -1;	/*  i.e., not inited...  */

	CLIF_print_usage ("Usage:", curr.argv[0], curr.option_list,
							curr.argument_list);

	if (curr.option_list)
		CLIF_print_options ("Options:", curr.option_list);
	
	if (curr.argument_list)
		CLIF_print_arguments ("\nArguments:", curr.argument_list);

	return 0;
}


/*  Common useful option handlers.  */

int CLIF_version_handler (CLIF_option *optn, char *arg) {

	if (!optn->data)  return -1;

	fprintf (stderr, "%s\n", ((char *) optn->data));

	return 0;	/*  be happy   */
}
	

int CLIF_set_flag (CLIF_option *optn, char *arg) {

	if (!optn->data)  return -1;

	*((int *) optn->data) = 1;

	return 0;
}


int CLIF_unset_flag (CLIF_option *optn, char *arg) {

	if (!optn->data)  return -1;

	*((int *) optn->data) = 0;

	return 0;
}


static int set_string (char **data, char *arg) {

	if (!data)  return -1;

	*data = arg;

	return 0;
}

int CLIF_set_string (CLIF_option *optn, char *arg) {

	return  set_string (optn->data, arg);
}

int CLIF_arg_string (CLIF_argument *argm, char *arg, int index) {

	return  set_string (argm->data, arg);
}


static int set_int (int *data, char *arg) {
	char *q;

	if (!data)  return -1;

	*data = (int) strtol (arg, &q, 0);

	return  (q == arg || *q) ? -1 : 0;
}

static int set_uint (unsigned int *data, char *arg) {
	char *q;

	if (!data)  return -1;

	*data = (unsigned int) strtoul (arg, &q, 0);

	return  (q == arg || *q) ? -1 : 0;
}

static int set_double (double *data, char *arg) {
	char *q;

	if (!data)  return -1;

	*data = strtod (arg, &q);

	return  (q == arg || *q) ? -1 : 0;
}


int CLIF_set_int (CLIF_option *optn, char *arg) {

	return  set_int (optn->data, arg);
}

int CLIF_set_uint (CLIF_option *optn, char *arg) {

	return  set_uint (optn->data, arg);
}

int CLIF_set_double (CLIF_option *optn, char *arg) {

	return  set_double (optn->data, arg);
}

int CLIF_arg_int (CLIF_argument *argm, char *arg, int index) {

	return  set_int (argm->data, arg);
}

int CLIF_arg_uint (CLIF_argument *argm, char *arg, int index) {

	return  set_uint (argm->data, arg);
}

int CLIF_arg_double (CLIF_argument *argm, char *arg, int index) {

	return  set_double (argm->data, arg);
}


int CLIF_call_func (CLIF_option *optn, char *arg) {

	if (!optn->data)  return -1;

	if (optn->arg_name) {
	    int (*func) (char *) = optn->data;

	    return  func (arg);
	} else {
	    int (*func) (void) = optn->data;

	    return  func ();
	}
}

int CLIF_arg_func (CLIF_argument *argm, char *arg, int index) {
	int (*func) (char *, int);

	if (!argm->data)  return -1;

	func = (int (*) (char *, int)) argm->data;

	return  func (arg, index);
}

