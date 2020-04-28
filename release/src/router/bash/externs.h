/* externs.h -- extern function declarations which do not appear in their
   own header file. */

/* Copyright (C) 1993-2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Make sure that this is included *after* config.h! */

#if !defined (_EXTERNS_H_)
#  define _EXTERNS_H_

#include "stdc.h"

/* Functions from expr.c. */
extern intmax_t evalexp __P((char *, int *));

/* Functions from print_cmd.c. */
#define FUNC_MULTILINE	0x01
#define FUNC_EXTERNAL	0x02

extern char *make_command_string __P((COMMAND *));
extern char *named_function_string __P((char *, COMMAND *, int));

extern void print_command __P((COMMAND *));
extern void print_simple_command __P((SIMPLE_COM *));
extern void print_word_list __P((WORD_LIST *, char *));

/* debugger support */
extern void print_for_command_head __P((FOR_COM *));
#if defined (SELECT_COMMAND)
extern void print_select_command_head __P((SELECT_COM *));
#endif
extern void print_case_command_head __P((CASE_COM *));
#if defined (DPAREN_ARITHMETIC)
extern void print_arith_command __P((WORD_LIST *));
#endif
#if defined (COND_COMMAND)
extern void print_cond_command __P((COND_COM *));
#endif

/* set -x support */
extern void xtrace_init __P((void));
#ifdef NEED_XTRACE_SET_DECL
extern void xtrace_set __P((int, FILE *));
#endif
extern void xtrace_fdchk __P((int));
extern void xtrace_reset __P((void));
extern char *indirection_level_string __P((void));
extern void xtrace_print_assignment __P((char *, char *, int, int));
extern void xtrace_print_word_list __P((WORD_LIST *, int));
extern void xtrace_print_for_command_head __P((FOR_COM *));
#if defined (SELECT_COMMAND)
extern void xtrace_print_select_command_head __P((SELECT_COM *));
#endif
extern void xtrace_print_case_command_head __P((CASE_COM *));
#if defined (DPAREN_ARITHMETIC)
extern void xtrace_print_arith_cmd __P((WORD_LIST *));
#endif
#if defined (COND_COMMAND)
extern void xtrace_print_cond_term __P((int, int, WORD_DESC *, char *, char *));
#endif

/* Functions from shell.c. */
extern void exit_shell __P((int)) __attribute__((__noreturn__));
extern void sh_exit __P((int)) __attribute__((__noreturn__));
extern void subshell_exit __P((int)) __attribute__((__noreturn__));
extern void disable_priv_mode __P((void));
extern void unbind_args __P((void));

#if defined (RESTRICTED_SHELL)
extern int shell_is_restricted __P((char *));
extern int maybe_make_restricted __P((char *));
#endif

extern void unset_bash_input __P((int));
extern void get_current_user_info __P((void));

/* Functions from eval.c. */
extern int reader_loop __P((void));
extern int parse_command __P((void));
extern int read_command __P((void));

/* Functions from braces.c. */
#if defined (BRACE_EXPANSION)
extern char **brace_expand __P((char *));
#endif

/* Miscellaneous functions from parse.y */
extern int yyparse __P((void));
extern int return_EOF __P((void));
extern void push_token __P((int));
extern char *xparse_dolparen __P((char *, char *, int *, int));
extern void reset_parser __P((void));
extern WORD_LIST *parse_string_to_word_list __P((char *, int, const char *));

extern int parser_in_command_position __P((void));

extern void free_pushed_string_input __P((void));

extern int parser_expanding_alias __P((void));
extern void parser_save_alias __P((void));
extern void parser_restore_alias __P((void));

extern void clear_shell_input_line __P((void));

extern char *decode_prompt_string __P((char *));

extern int get_current_prompt_level __P((void));
extern void set_current_prompt_level __P((int));

#if defined (HISTORY)
extern char *history_delimiting_chars __P((const char *));
#endif

/* Declarations for functions defined in locale.c */
extern void set_default_locale __P((void));
extern void set_default_locale_vars __P((void));
extern int set_locale_var __P((char *, char *));
extern int set_lang __P((char *, char *));
extern void set_default_lang __P((void));
extern char *get_locale_var __P((char *));
extern char *localetrans __P((char *, int, int *));
extern char *mk_msgstr __P((char *, int *));
extern char *localeexpand __P((char *, int, int, int, int *));

/* Declarations for functions defined in list.c. */
extern void list_walk __P((GENERIC_LIST *, sh_glist_func_t *));
extern void wlist_walk __P((WORD_LIST *, sh_icpfunc_t *));
extern GENERIC_LIST *list_reverse ();
extern int list_length ();
extern GENERIC_LIST *list_append ();
extern GENERIC_LIST *list_remove ();

/* Declarations for functions defined in stringlib.c */
extern int find_string_in_alist __P((char *, STRING_INT_ALIST *, int));
extern char *find_token_in_alist __P((int, STRING_INT_ALIST *, int));
extern int find_index_in_alist __P((char *, STRING_INT_ALIST *, int));

extern char *substring __P((const char *, int, int));
extern char *strsub __P((char *, char *, char *, int));
extern char *strcreplace __P((char *, int, const char *, int));
extern void strip_leading __P((char *));
extern void strip_trailing __P((char *, int, int));
extern void xbcopy __P((char *, char *, int));

/* Functions from version.c. */
extern char *shell_version_string __P((void));
extern void show_shell_version __P((int));

/* Functions from the bash library, lib/sh/libsh.a.  These should really
   go into a separate include file. */

/* declarations for functions defined in lib/sh/casemod.c */
extern char *sh_modcase __P((const char *, char *, int));

/* Defines for flags argument to sh_modcase.  These need to agree with what's
   in lib/sh/casemode.c */
#define CASE_LOWER	0x0001
#define CASE_UPPER	0x0002
#define CASE_CAPITALIZE	0x0004
#define CASE_UNCAP	0x0008
#define CASE_TOGGLE	0x0010
#define CASE_TOGGLEALL	0x0020
#define CASE_UPFIRST	0x0040
#define CASE_LOWFIRST	0x0080

#define CASE_USEWORDS	0x1000

/* declarations for functions defined in lib/sh/clktck.c */
extern long get_clk_tck __P((void));

/* declarations for functions defined in lib/sh/clock.c */
extern void clock_t_to_secs ();
extern void print_clock_t ();

/* Declarations for functions defined in lib/sh/dprintf.c */
#if !defined (HAVE_DPRINTF)
extern void dprintf __P((int, const char *, ...))  __attribute__((__format__ (printf, 2, 3)));
#endif

/* Declarations for functions defined in lib/sh/fmtulong.c */
#define FL_PREFIX     0x01    /* add 0x, 0X, or 0 prefix as appropriate */
#define FL_ADDBASE    0x02    /* add base# prefix to converted value */
#define FL_HEXUPPER   0x04    /* use uppercase when converting to hex */
#define FL_UNSIGNED   0x08    /* don't add any sign */

extern char *fmtulong __P((unsigned long int, int, char *, size_t, int));

/* Declarations for functions defined in lib/sh/fmtulong.c */
#if defined (HAVE_LONG_LONG)
extern char *fmtullong __P((unsigned long long int, int, char *, size_t, int));
#endif

/* Declarations for functions defined in lib/sh/fmtumax.c */
extern char *fmtumax __P((uintmax_t, int, char *, size_t, int));

/* Declarations for functions defined in lib/sh/fnxform.c */
extern char *fnx_fromfs __P((char *, size_t));
extern char *fnx_tofs __P((char *, size_t));

/* Declarations for functions defined in lib/sh/fpurge.c */

#if defined NEED_FPURGE_DECL
#if !HAVE_DECL_FPURGE

#if HAVE_FPURGE
#  define fpurge _bash_fpurge
#endif
extern int fpurge __P((FILE *stream));

#endif /* HAVE_DECL_FPURGE */
#endif /* NEED_FPURGE_DECL */

/* Declarations for functions defined in lib/sh/getcwd.c */
#if !defined (HAVE_GETCWD)
extern char *getcwd __P((char *, size_t));
#endif

/* Declarations for functions defined in lib/sh/input_avail.c */
extern int input_avail __P((int));

/* Declarations for functions defined in lib/sh/itos.c */
extern char *inttostr __P((intmax_t, char *, size_t));
extern char *itos __P((intmax_t));
extern char *mitos __P((intmax_t));
extern char *uinttostr __P((uintmax_t, char *, size_t));
extern char *uitos __P((uintmax_t));

/* declarations for functions defined in lib/sh/makepath.c */
#define MP_DOTILDE	0x01
#define MP_DOCWD	0x02
#define MP_RMDOT	0x04
#define MP_IGNDOT	0x08

extern char *sh_makepath __P((const char *, const char *, int));

/* declarations for functions defined in lib/sh/mbscasecmp.c */
#if !defined (HAVE_MBSCASECMP)
extern char *mbscasecmp __P((const char *, const char *));
#endif

/* declarations for functions defined in lib/sh/mbschr.c */
#if !defined (HAVE_MBSCHR)
extern char *mbschr __P((const char *, int));
#endif

/* declarations for functions defined in lib/sh/mbscmp.c */
#if !defined (HAVE_MBSCMP)
extern char *mbscmp __P((const char *, const char *));
#endif

/* declarations for functions defined in lib/sh/netconn.c */
extern int isnetconn __P((int));

/* declarations for functions defined in lib/sh/netopen.c */
extern int netopen __P((char *));

/* Declarations for  functions defined in lib/sh/oslib.c */

#if !defined (HAVE_DUP2) || defined (DUP2_BROKEN)
extern int dup2 __P((int, int));
#endif

#if !defined (HAVE_GETDTABLESIZE)
extern int getdtablesize __P((void));
#endif /* !HAVE_GETDTABLESIZE */

#if !defined (HAVE_GETHOSTNAME)
extern int gethostname __P((char *, int));
#endif /* !HAVE_GETHOSTNAME */

extern int getmaxgroups __P((void));
extern long getmaxchild __P((void));

/* declarations for functions defined in lib/sh/pathcanon.c */
#define PATH_CHECKDOTDOT	0x0001
#define PATH_CHECKEXISTS	0x0002
#define PATH_HARDPATH		0x0004
#define PATH_NOALLOC		0x0008

extern char *sh_canonpath __P((char *, int));

/* declarations for functions defined in lib/sh/pathphys.c */
extern char *sh_physpath __P((char *, int));
extern char *sh_realpath __P((const char *, char *));

/* declarations for functions defined in lib/sh/setlinebuf.c */
#ifdef NEED_SH_SETLINEBUF_DECL
extern int sh_setlinebuf __P((FILE *));
#endif

/* declarations for functions defined in lib/sh/shaccess.c */
extern int sh_eaccess __P((const char *, int));

/* declarations for functions defined in lib/sh/shmatch.c */
extern int sh_regmatch __P((const char *, const char *, int));

/* defines for flags argument to sh_regmatch. */
#define SHMAT_SUBEXP		0x001	/* save subexpressions in SH_REMATCH */
#define SHMAT_PWARN		0x002	/* print a warning message on invalid regexp */

/* declarations for functions defined in lib/sh/shmbchar.c */
extern size_t mbstrlen __P((const char *));
extern char *mbsmbchar __P((const char *));
extern int sh_mbsnlen __P((const char *, size_t, int));

/* declarations for functions defined in lib/sh/shquote.c */
extern char *sh_single_quote __P((const char *));
extern char *sh_double_quote __P((const char *));
extern char *sh_mkdoublequoted __P((const char *, int, int));
extern char *sh_un_double_quote __P((char *));
extern char *sh_backslash_quote __P((char *, const char *, int));
extern char *sh_backslash_quote_for_double_quotes __P((char *));
extern char *sh_quote_reusable __P((char *, int));
extern int sh_contains_shell_metas __P((const char *));
extern int sh_contains_quotes __P((const char *));

/* declarations for functions defined in lib/sh/spell.c */
extern int spname __P((char *, char *));
extern char *dirspell __P((char *));

/* declarations for functions defined in lib/sh/strcasecmp.c */
#if !defined (HAVE_STRCASECMP)
extern int strncasecmp __P((const char *, const char *, int));
extern int strcasecmp __P((const char *, const char *));
#endif /* HAVE_STRCASECMP */

/* declarations for functions defined in lib/sh/strcasestr.c */
#if ! HAVE_STRCASESTR
extern char *strcasestr __P((const char *, const char *));
#endif

/* declarations for functions defined in lib/sh/strchrnul.c */
#if ! HAVE_STRCHRNUL
extern char *strchrnul __P((const char *, int));
#endif

/* declarations for functions defined in lib/sh/strerror.c */
#if !defined (HAVE_STRERROR) && !defined (strerror)
extern char *strerror __P((int));
#endif

/* declarations for functions defined in lib/sh/strftime.c */
#if !defined (HAVE_STRFTIME) && defined (NEED_STRFTIME_DECL)
extern size_t strftime __P((char *, size_t, const char *, const struct tm *));
#endif

/* declarations for functions and structures defined in lib/sh/stringlist.c */

/* This is a general-purpose argv-style array struct. */
typedef struct _list_of_strings {
  char **list;
  int list_size;
  int list_len;
} STRINGLIST;

typedef int sh_strlist_map_func_t __P((char *));

extern STRINGLIST *strlist_create __P((int));
extern STRINGLIST *strlist_resize __P((STRINGLIST *, int));
extern void strlist_flush __P((STRINGLIST *));
extern void strlist_dispose __P((STRINGLIST *));
extern int strlist_remove __P((STRINGLIST *, char *));
extern STRINGLIST *strlist_copy __P((STRINGLIST *));
extern STRINGLIST *strlist_merge __P((STRINGLIST *, STRINGLIST *));
extern STRINGLIST *strlist_append __P((STRINGLIST *, STRINGLIST *));
extern STRINGLIST *strlist_prefix_suffix __P((STRINGLIST *, char *, char *));
extern void strlist_print __P((STRINGLIST *, char *));
extern void strlist_walk __P((STRINGLIST *, sh_strlist_map_func_t *));
extern void strlist_sort __P((STRINGLIST *));

/* declarations for functions defined in lib/sh/stringvec.c */

extern char **strvec_create __P((int));
extern char **strvec_resize __P((char **, int));
extern char **strvec_mcreate __P((int));
extern char **strvec_mresize __P((char **, int));
extern void strvec_flush __P((char **));
extern void strvec_dispose __P((char **));
extern int strvec_remove __P((char **, char *));
extern int strvec_len __P((char **));
extern int strvec_search __P((char **, char *));
extern char **strvec_copy __P((char **));
extern int strvec_strcmp __P((char **, char **));
extern void strvec_sort __P((char **));

extern char **strvec_from_word_list __P((WORD_LIST *, int, int, int *));
extern WORD_LIST *strvec_to_word_list __P((char **, int, int));

/* declarations for functions defined in lib/sh/strnlen.c */
#if !defined (HAVE_STRNLEN)
extern size_t strnlen __P((const char *, size_t));
#endif

/* declarations for functions defined in lib/sh/strpbrk.c */
#if !defined (HAVE_STRPBRK)
extern char *strpbrk __P((const char *, const char *));
#endif

/* declarations for functions defined in lib/sh/strtod.c */
#if !defined (HAVE_STRTOD)
extern double strtod __P((const char *, char **));
#endif

/* declarations for functions defined in lib/sh/strtol.c */
#if !HAVE_DECL_STRTOL
extern long strtol __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strtoll.c */
#if defined (HAVE_LONG_LONG) && !HAVE_DECL_STRTOLL
extern long long strtoll __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strtoul.c */
#if !HAVE_DECL_STRTOUL
extern unsigned long strtoul __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strtoull.c */
#if defined (HAVE_LONG_LONG) && !HAVE_DECL_STRTOULL
extern unsigned long long strtoull __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strimax.c */
#if !HAVE_DECL_STRTOIMAX
extern intmax_t strtoimax __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strumax.c */
#if !HAVE_DECL_STRTOUMAX
extern uintmax_t strtoumax __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strtrans.c */
extern char *ansicstr __P((char *, int, int, int *, int *));
extern char *ansic_quote __P((char *, int, int *));
extern int ansic_shouldquote __P((const char *));
extern char *ansiexpand __P((char *, int, int, int *));

/* declarations for functions defined in lib/sh/timeval.c.  No prototypes
   so we don't have to count on having a definition of struct timeval in
   scope when this file is included. */
extern void timeval_to_secs ();
extern void print_timeval ();

/* declarations for functions defined in lib/sh/tmpfile.c */
#define MT_USETMPDIR		0x0001
#define MT_READWRITE		0x0002
#define MT_USERANDOM		0x0004

extern char *sh_mktmpname __P((char *, int));
extern int sh_mktmpfd __P((char *, int, char **));
/* extern FILE *sh_mktmpfp __P((char *, int, char **)); */

/* declarations for functions defined in lib/sh/uconvert.c */
extern int uconvert __P((char *, long *, long *));

/* declarations for functions defined in lib/sh/ufuncs.c */
extern unsigned int falarm __P((unsigned int, unsigned int));
extern unsigned int fsleep __P((unsigned int, unsigned int));

/* declarations for functions defined in lib/sh/unicode.c */
extern int u32cconv __P((unsigned long, char *));
extern void u32reset __P((void));

/* declarations for functions defined in lib/sh/wcsnwidth.c */
#if defined (HANDLE_MULTIBYTE)
extern int wcsnwidth __P((const wchar_t *, size_t, int));
#endif

/* declarations for functions defined in lib/sh/winsize.c */
extern void get_new_window_size __P((int, int *, int *));

/* declarations for functions defined in lib/sh/zcatfd.c */
extern int zcatfd __P((int, int, char *));

/* declarations for functions defined in lib/sh/zgetline.c */
extern ssize_t zgetline __P((int, char **, size_t *, int, int));

/* declarations for functions defined in lib/sh/zmapfd.c */
extern int zmapfd __P((int, char **, char *));

/* declarations for functions defined in lib/sh/zread.c */
extern ssize_t zread __P((int, char *, size_t));
extern ssize_t zreadretry __P((int, char *, size_t));
extern ssize_t zreadintr __P((int, char *, size_t));
extern ssize_t zreadc __P((int, char *));
extern ssize_t zreadcintr __P((int, char *));
extern ssize_t zreadn __P((int, char *, size_t));
extern void zreset __P((void));
extern void zsyncfd __P((int));

/* declarations for functions defined in lib/sh/zwrite.c */
extern int zwrite __P((int, char *, size_t));

/* declarations for functions defined in lib/glob/gmisc.c */
extern int match_pattern_char __P((char *, char *, int));
extern int umatchlen __P((char *, size_t));

#if defined (HANDLE_MULTIBYTE)
extern int match_pattern_wchar __P((wchar_t *, wchar_t *, int));
extern int wmatchlen __P((wchar_t *, size_t));
#endif

#endif /* _EXTERNS_H_ */
