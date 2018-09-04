/*
 * Copyright (c) 2002-2017  Martin Hedenfalk <martin@bzero.se>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/** A configuration file parser library.
 * @file confuse.h
 *
 */

/**
 * \mainpage libConfuse Documentation
 *
 * \section intro
 *
 * Copyright &copy; 2002-2017 Martin Hedenfalk &lt;martin@bzero.se&gt;
 *
 * The latest versions of this manual and the libConfuse software are
 * available at http://www.nongnu.org/confuse/
 *
 *
 * <em>If you can't convince, confuse.</em>
 */

#ifndef CONFUSE_H_
#define CONFUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32) && !defined(__GNUC__)
# ifdef HAVE__FILENO
#  define fileno _fileno
# endif
# include <io.h>
# ifdef HAVE__ISATTY
#  define isatty _isatty
# endif
# ifdef BUILDING_DLL
#  define DLLIMPORT __declspec (dllexport)
# else /* ! BUILDING_DLL */
#  define DLLIMPORT __declspec (dllimport)
# endif /* BUILDING_DLL */
#else /* ! _WIN32 || __GNUC__ */
# define DLLIMPORT
#endif /* _WIN32 */

#ifndef __BORLANDC__
# define __export
#endif

/** Fundamental option types */
enum cfg_type_t {
	CFGT_NONE,
	CFGT_INT,    /**< integer */
	CFGT_FLOAT,  /**< floating point number */
	CFGT_STR,    /**< string */
	CFGT_BOOL,   /**< boolean value */
	CFGT_SEC,    /**< section */
	CFGT_FUNC,   /**< function */
	CFGT_PTR,    /**< pointer to user-defined value */
	CFGT_COMMENT /**< comment/annotation */
};
typedef enum cfg_type_t cfg_type_t;

/** Flags. */
#define CFGF_NONE 0
#define CFGF_MULTI 1       /**< option may be specified multiple times (only applies to sections) */
#define CFGF_LIST 2        /**< option is a list */
#define CFGF_NOCASE 4      /**< configuration file is case insensitive */
#define CFGF_TITLE 8       /**< option has a title (only applies to sections) */
#define CFGF_NODEFAULT 16  /**< option has no default value */
#define CFGF_NO_TITLE_DUPES 32  /**< multiple section titles must be unique
                                  (duplicates raises an error, only applies to
                                  sections) */

#define CFGF_RESET 64
#define CFGF_DEFINIT 128
#define CFGF_IGNORE_UNKNOWN 256 /**< ignore unknown options in configuration files */
#define CFGF_DEPRECATED     512  /**< option is deprecated and should be ignored. */
#define CFGF_DROP           1024 /**< option should be dropped after parsing */
#define CFGF_COMMENTS       2048 /**< Enable option annotation/comments support */

/** Return codes from cfg_parse(), cfg_parse_boolean(), and cfg_set*() functions. */
#define CFG_SUCCESS     0
#define CFG_FAIL       -1
#define CFG_FILE_ERROR -1
#define CFG_PARSE_ERROR 1

typedef union cfg_value_t cfg_value_t;
typedef union cfg_simple_t cfg_simple_t;
typedef struct cfg_opt_t cfg_opt_t;
typedef struct cfg_t cfg_t;
typedef struct cfg_defvalue_t cfg_defvalue_t;
typedef int cfg_flag_t;
typedef struct cfg_searchpath_t cfg_searchpath_t;

/** Function prototype used by CFGT_FUNC options.
 *
 * This is a callback function, registered with the CFG_FUNC
 * initializer. Each time libConfuse finds a function, the registered
 * callback function is called (parameters are passed as strings, any
 * conversion to other types should be made in the callback
 * function). libConfuse does not support any storage of the data
 * found; these are passed as parameters to the callback, and it's the
 * responsibility of the callback function to do whatever it should do
 * with the data.
 *
 * @param cfg The configuration file context.
 * @param opt The option.
 * @param argc Number of arguments passed. The callback function is
 * responsible for checking that the correct number of arguments are
 * passed.
 * @param argv Arguments as an array of character strings.
 *
 * @return On success, 0 should be returned. All other values
 * indicates an error, and the parsing is aborted. The callback
 * function should notify the error itself, for example by calling
 * cfg_error().
 *
 * @see CFG_FUNC
 */
typedef int (*cfg_func_t)(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv);

/** Function prototype used by the cfg_print_ functions.
 *
 * This callback function is used to print option values. For options
 * with a value parsing callback, this is often required, especially
 * if a string is mapped to an integer by the callback. This print
 * callback must then map the integer back to the appropriate string.
 *
 * Except for functions, the print callback function should only print
 * the value of the option, not the name and the equal sign (that is
 * handled by the cfg_opt_print function). For function options
 * however, the name and the parenthesis must be printed by this
 * function. The value to print can be accessed with the cfg_opt_get
 * functions.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @param fp File stream to print to, use stdout to print to the screen.
 *
 * @see cfg_print, cfg_set_print_func
 */
typedef void (*cfg_print_func_t)(cfg_opt_t *opt, unsigned int index, FILE *fp);

/** Value parsing callback prototype
 *
 * This is a callback function (different from the one registered with the
 * CFG_FUNC initializer) used to parse a value. This can be used to override
 * the internal parsing of a value.
 *
 * Suppose you want an integer option that only can have certain values, for
 * example 1, 2 and 3, and these should be written in the configuration file as
 * "yes", "no" and "maybe". The callback function would be called with the
 * found value ("yes", "no" or "maybe") as a string, and the result should be
 * stored in the result parameter.
 *
 * @param cfg The configuration file context.
 * @param opt The option.
 * @param value The value found in the configuration file.
 * @param result Pointer to storage for the result, cast to a void pointer.
 *
 * @return On success, 0 should be returned. All other values indicates an
 * error, and the parsing is aborted. The callback function should notify the
 * error itself, for example by calling cfg_error().
 */
typedef int (*cfg_callback_t)(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);

/** Validating callback prototype
 *
 * This callback function is called after an option has been parsed and set.
 * The function is called for both fundamental values (strings, integers etc)
 * as well as lists and sections. This can for example be used to validate that
 * all required options in a section has been set to sane values.
 *
 * @return On success, 0 should be returned. All other values indicates an
 * error, and the parsing is aborted. The callback function should notify the
 * error itself, for example by calling cfg_error().
 *
 * @see cfg_set_validate_func
 */
typedef int (*cfg_validate_callback_t)(cfg_t *cfg, cfg_opt_t *opt);

/** Validating callback2 prototype
 *
 * This callback function is called before an option is set using the
 * cfg_set*() APIs.  The function is called only for strings, integers,
 * and floats.  Compared to the regular callback function this takes a
 * value pointer argument which must be casted before use, but can also
 * be used to correct a value before it is set, e.g. when a too large
 * value is set this can be used to set the MAX.
 *
 * @return On success, 0 should be returned. All other values indicates an
 * error, and the cfg_set*() function will return without setting the value.
 *
 * @see cfg_set_validate_func2()
 */
typedef int (*cfg_validate_callback2_t)(cfg_t *cfg, cfg_opt_t *opt, void *value);

/** User-defined memory release function for CFG_PTR values
 *
 * This callback is used to free memory allocated in a value parsing callback
 * function. Especially useful for CFG_PTR options, since libConfuse will not
 * itself release such values. If the values are simply allocated with a
 * malloc(3), one can use the standard free(3) function here.
 *
 */
typedef void (*cfg_free_func_t)(void *value);

/** Boolean values. */
typedef enum { cfg_false, cfg_true } cfg_bool_t;

/** Error reporting function. */
typedef void (*cfg_errfunc_t)(cfg_t *cfg, const char *fmt, va_list ap);

/** Data structure holding information about a "section". Sections can
 * be nested. A section has a list of options (strings, numbers,
 * booleans or other sections) grouped together.
 */
struct cfg_t {
	cfg_flag_t flags;	/**< Any flags passed to cfg_init() */
	char *name;		/**< The name of this section, the root
				 * section returned from cfg_init() is
				 * always named "root" */
	char *comment;	        /**< Optional annotation/comment */
	cfg_opt_t *opts;        /**< Array of options */
	char *title;	        /**< Optional title for this section, only
				 * set if CFGF_TITLE flag is set */
	char *filename;		/**< Name of the file being parsed */
	int line;		/**< Line number in the config file */
	cfg_errfunc_t errfunc;	/**< This function (if set with
				 * cfg_set_error_function) is called for
				 * any error message. */
	cfg_searchpath_t *path;	/**< Linked list of directories to search */
};

/** Data structure holding the value of a fundamental option value.
 */
union cfg_value_t {
	long int number;	/**< integer value */
	double fpnumber;	/**< floating point value */
	cfg_bool_t boolean;	/**< boolean value */
	char *string;		/**< string value */
	cfg_t *section;		/**< section value */
	void *ptr;		/**< user-defined value */
};

/** Data structure holding the pointer to a user provided variable
 *  defined with CFG_SIMPLE_*
 */
union cfg_simple_t {
	long int *number;
	double *fpnumber;
	cfg_bool_t *boolean;
	char **string;
	void **ptr;
};

/** Data structure holding the default value given by the
 *  initialization macros.
 */
struct cfg_defvalue_t {
	long int number; 	/**< default integer value */
	double fpnumber;	/**< default floating point value */
	cfg_bool_t boolean;	/**< default boolean value */
	const char *string;	/**< default string value */
	char *parsed;		/**< default value that is parsed by
				 * libConfuse, used for lists and
				 * functions */
};

/** Data structure holding information about an option. The value(s)
 * are stored as an array of fundamental values (strings, numbers,
 * etc).
 */
struct cfg_opt_t {
	const char *name;	/**< The name of the option */
	char *comment;	        /**< Optional comment/annotation */
	cfg_type_t type;	/**< Type of option */
	unsigned int nvalues;	/**< Number of values parsed */
	cfg_value_t **values;	/**< Array of found values */
	cfg_flag_t flags;	/**< Flags */
	cfg_opt_t *subopts;	/**< Suboptions (only applies to sections) */
	cfg_defvalue_t def;	/**< Default value */
	cfg_func_t func;	/**< Function callback for CFGT_FUNC options */
	cfg_simple_t simple_value;	/**< Pointer to user-specified variable to
					 * store simple values (created with the
					 * CFG_SIMPLE_* initializers) */
	cfg_callback_t parsecb;	/**< Value parsing callback function */
	cfg_validate_callback_t  validcb;  /**< Value validating parsing callback function */
	cfg_validate_callback2_t validcb2; /**< Value validating set callback function */
	cfg_print_func_t pf;	/**< print callback function */
	cfg_free_func_t freecb;	/***< user-defined memory release function */
};

extern const char __export confuse_copyright[];
extern const char __export confuse_version[];
extern const char __export confuse_author[];

#define __CFG_STR(name, def, flags, svalue, cb) \
  {name,0,CFGT_STR,0,0,flags,0,{0,0,cfg_false,def,0},0,{.string=svalue},cb,0,0,0,0}
#define __CFG_STR_LIST(name, def, flags, svalue, cb) \
  {name,0,CFGT_STR,0,0,flags | CFGF_LIST,0,{0,0,cfg_false,0,def},0,{.string=svalue},cb,0,0,0,0}

/** Initialize a string option
 */
#define CFG_STR(name, def, flags) \
  __CFG_STR(name, def, flags, 0, 0)

/** Initialize a string list option
 */
#define CFG_STR_LIST(name, def, flags) \
  __CFG_STR_LIST(name, def, flags, 0, 0)

/** Initialize a string option with a value parsing callback
 */
#define CFG_STR_CB(name, def, flags, cb) \
  __CFG_STR(name, def, flags, 0, cb)

/** Initialize a string list option with a value parsing callback
 */
#define CFG_STR_LIST_CB(name, def, flags, cb) \
  __CFG_STR_LIST(name, def, flags, 0, cb)

/** Initialize a "simple" string option.
 *
 * "Simple" options (in lack of a better expression) does not support
 * lists of values or multiple sections. LibConfuse will store the
 * value of a simple option in the user-defined location specified by
 * the value parameter in the initializer. Simple options are not
 * stored in the cfg_t context, only a pointer. Sections can not be
 * initialized as a "simple" option.
 *
 * As of version 2.2, libConfuse can now return the values of simple
 * options with the cfg_get functions. This allows using the new
 * cfg_print function with simple options.
 *
 * libConfuse doesn't support handling default values for "simple"
 * options. They are assumed to be set by the calling application
 * before cfg_parse is called.
 *
 * @param name name of the option
 * @param svalue pointer to a character pointer (a char **). This value
 * must be initalized either to NULL or to a malloc()'ed string. You
 * can't use
 * <pre>
 * char *user = "joe";
 * ...
 * cfg_opt_t opts[] = {
 *     CFG_SIMPLE_STR("user", &user),
 * ...
 * </pre>
 * since libConfuse will try to free the static string "joe" (which is
 * an error) when a "user" option is found. Rather, use the following
 * code snippet:
 * <pre>
 * char *user = strdup("joe");
 * ...
 * cfg_opt_t opts[] = {
 *      CFG_SIMPLE_STR("user", &user),
 * ...
 * </pre>
 * Alternatively, the default value can be set after the opts struct
 * is defined, as in:
 * <pre>
 * char *user = 0;
 * ...
 * cfg_opt_t opts[] = {
 *      CFG_SIMPLE_STR("user", &user),
 * ...
 * user = strdup("joe");
 * cfg = cfg_init(opts, 0);
 * cfg_parse(cfg, filename);
 * </pre>
 *
 */
#define CFG_SIMPLE_STR(name, svalue) \
  __CFG_STR(name, 0, CFGF_NONE, svalue, 0)


#define __CFG_INT(name, def, flags, svalue, cb) \
  {name,0,CFGT_INT,0,0,flags,0,{def,0,cfg_false,0,0},0,{.number=svalue},cb,0,0,0,0}
#define __CFG_INT_LIST(name, def, flags, svalue, cb) \
  {name,0,CFGT_INT,0,0,flags | CFGF_LIST,0,{0,0,cfg_false,0,def},0,{.number=svalue},cb,0,0,0,0}

/** Initialize an integer option
 */
#define CFG_INT(name, def, flags) \
  __CFG_INT(name, def, flags, 0, 0)

/** Initialize an integer list option
 */
#define CFG_INT_LIST(name, def, flags) \
  __CFG_INT_LIST(name, def, flags, 0, 0)

/** Initialize an integer option with a value parsing callback
 */
#define CFG_INT_CB(name, def, flags, cb) \
  __CFG_INT(name, def, flags, 0, cb)

/** Initialize an integer list option with a value parsing callback
 */
#define CFG_INT_LIST_CB(name, def, flags, cb) \
  __CFG_INT_LIST(name, def, flags, 0, cb)

/** Initialize a "simple" integer option (see documentation for
 * CFG_SIMPLE_STR for more information).
 * Note that confuse uses long integers, so make sure that any pointer
 * you provide for svalue points to a long int rather than a normal int.
 * Otherwise, you will have strange problems on 64-bit architectures.
 */
#define CFG_SIMPLE_INT(name, svalue) \
  __CFG_INT(name, 0, CFGF_NONE, svalue, 0)



#define __CFG_FLOAT(name, def, flags, svalue, cb) \
  {name,0,CFGT_FLOAT,0,0,flags,0,{0,def,cfg_false,0,0},0,{.fpnumber=svalue},cb,0,0,0,0}
#define __CFG_FLOAT_LIST(name, def, flags, svalue, cb) \
  {name,0,CFGT_FLOAT,0,0,flags | CFGF_LIST,0,{0,0,cfg_false,0,def},0,{.fpnumber=svalue},cb,0,0,0,0}

/** Initialize a floating point option
 */
#define CFG_FLOAT(name, def, flags) \
  __CFG_FLOAT(name, def, flags, 0, 0)

/** Initialize a floating point list option
 */
#define CFG_FLOAT_LIST(name, def, flags) \
  __CFG_FLOAT_LIST(name, def, flags, 0, 0)

/** Initialize a floating point option with a value parsing callback
 */
#define CFG_FLOAT_CB(name, def, flags, cb) \
  __CFG_FLOAT(name, def, flags, 0, cb)

/** Initialize a floating point list option with a value parsing callback
 */
#define CFG_FLOAT_LIST_CB(name, def, flags, cb) \
  __CFG_FLOAT_LIST(name, def, flags, 0, cb)

/** Initialize a "simple" floating point option (see documentation for
 * CFG_SIMPLE_STR for more information).
 */
#define CFG_SIMPLE_FLOAT(name, svalue) \
  __CFG_FLOAT(name, 0, CFGF_NONE, svalue, 0)



#define __CFG_BOOL(name, def, flags, svalue, cb) \
  {name,0,CFGT_BOOL,0,0,flags,0,{0,0,def,0,0},0,{.boolean=svalue},cb,0,0,0,0}
#define __CFG_BOOL_LIST(name, def, flags, svalue, cb) \
  {name,0,CFGT_BOOL,0,0,flags | CFGF_LIST,0,{0,0,cfg_false,0,def},0,{.boolean=svalue},cb,0,0,0,0}

/** Initialize a boolean option
 */
#define CFG_BOOL(name, def, flags) \
  __CFG_BOOL(name, def, flags, 0, 0)

/** Initialize a boolean list option
 */
#define CFG_BOOL_LIST(name, def, flags) \
  __CFG_BOOL_LIST(name, def, flags, 0, 0)

/** Initialize a boolean option with a value parsing callback
 */
#define CFG_BOOL_CB(name, def, flags, cb) \
  __CFG_BOOL(name, def, flags, 0, cb)

/** Initialize a boolean list option with a value parsing callback
 */
#define CFG_BOOL_LIST_CB(name, def, flags, cb) \
  __CFG_BOOL_LIST(name, def, flags, 0, cb)

/** Initialize a "simple" boolean option (see documentation for
 * CFG_SIMPLE_STR for more information).
 */
#define CFG_SIMPLE_BOOL(name, svalue) \
  __CFG_BOOL(name, cfg_false, CFGF_NONE, svalue, 0)



/** Initialize a section
 *
 * @param name The name of the option
 * @param opts Array of options that are valid within this section

 * @param flags Flags, specify CFGF_MULTI if it should be possible to
 * have multiples of the same section, and CFGF_TITLE if the
 * section(s) must have a title (which can be used in the
 * cfg_gettsec() function)
 *
 */
#define CFG_SEC(name, opts, flags) \
  {name,0,CFGT_SEC,0,0,flags,opts,{0,0,cfg_false,0,0},0,{0},0,0,0,0,0}



/** Initialize a function
 * @param name The name of the option
 * @param func The callback function.
 *
 * @see cfg_func_t
 */
#define CFG_FUNC(name, func) \
  {name,0,CFGT_FUNC,0,0,CFGF_NONE,0,{0,0,cfg_false,0,0},func,{0},0,0,0,0,0}


#define __CFG_PTR(name, def, flags, svalue, parsecb, freecb) \
  {name,0,CFGT_PTR,0,0,flags,0,{0,0,cfg_false,0,def},0,{.ptr=svalue},parsecb,0,0,0,freecb}
#define __CFG_PTR_LIST(name, def, flags, svalue, parsecb, freecb) \
  {name,0,CFGT_PTR,0,0,flags | CFGF_LIST,0,{0,0,cfg_false,0,def},0,{.ptr=svalue},parsecb,0,0,0,freecb}

/** Initialize a user-defined option
 *
 * CFG_PTR options can only be used together with a value parsing callback.
 *
 * @param name The name of the option
 * @param def Default value
 * @param flags Flags
 * @param parsecb Value parsing callback
 * @param freecb Memory release function
 *
 * @see cfg_callback_t, cfg_free_func_t
 */
#define CFG_PTR_CB(name, def, flags, parsecb, freecb) \
  __CFG_PTR(name, def, flags, 0, parsecb, freecb)

/** Initialize a list of user-defined options
 */
#define CFG_PTR_LIST_CB(name, def, flags, parsecb, freecb) \
  __CFG_PTR(name, def, flags | CFGF_LIST, 0, parsecb, freecb)

/*#define CFG_SIMPLE_PTR(name, svalue, cb) \
  __CFG_PTR(name, 0, 0, svalue, cb)*/


/** Terminate list of options. This must be the last initializer in
 * the option list.
 */
#define CFG_END() \
  {0,0,CFGT_NONE,0,0,CFGF_NONE,0,{0,0,cfg_false,0,0},0,{0},0,0,0,0,0}



/** Create and initialize a cfg_t structure. This should be the first function
 * called when setting up the parsing of a configuration file. The options
 * passed in the first parameter is initialized using the CFG_* initializers.
 * The last option in the option array must be CFG_END(), unless you like
 * segmentation faults.
 *
 * The options must no longer be defined in the same scope as where the cfg_xxx
 * functions are used (since version 2.3).
 *
 * CFG_IGNORE_UNKNOWN can be specified to use the "__unknown" option
 * whenever an unknown option is parsed. Be sure to define an "__unknown"
 * option in each scope that unknown parameters are allowed.
 *
 * Call setlocale() before calling this function to localize handling of
 * types, LC_CTYPE, and messages, LC_MESSAGES, since version 2.9:
 * <pre>
 *     setlocale(LC_MESSAGES, "");
 *     setlocale(LC_CTYPE, "");
 * </pre>
 * @param opts An arrary of options
 * @param flags One or more flags (bitwise or'ed together). Currently only
 * CFGF_NOCASE and CFGF_IGNORE_UNKNOWN are available. Use 0 if no flags are
 * needed.
 *
 * @return A configuration context structure. This pointer is passed
 * to almost all other functions as the first parameter.
 */
DLLIMPORT cfg_t *__export cfg_init(cfg_opt_t *opts, cfg_flag_t flags);

/** Add a searchpath directory to the configuration context, the
 * const char* argument will be duplicated and then freed as part
 * of the usual context takedown.
 *
 * All directories added to the context in this manner will be searched
 * for the file specified in cfg_parse(), and for those included.
 * All directories added with this function will be "tilde expanded".
 * Note that the current directory is not added to the searchpath
 * by default.
 *
 * @param cfg The configuration file context as returned from cfg_init().
 * @param dir Directory to be added to the search path.
 *
 * @return On success, CFG_SUCCESS, on failure (which can only be
 * caused by a failed malloc()), CFG_PARSE_ERROR.
 */
DLLIMPORT int __export cfg_add_searchpath(cfg_t *cfg, const char *dir);

/** Search the linked-list of cfg_searchpath_t for the specified
 * file.  If not NULL, the return value is freshly allocated and
 * and should be freed by the caller.
 *
 * @param path The linked list of cfg_searchpath_t structs, each
 * containg a directory to be searched
 * @param file The file for which to search
 *
 * @return If the file is found on the searchpath then the full
 * path to the file is returned. If not found, NULL is returned.
 */
DLLIMPORT char *__export cfg_searchpath(cfg_searchpath_t *path, const char *file);

/** Parse a configuration file. Tilde expansion is performed on the
 * filename before it is opened. After a configuration file has been
 * initialized (with cfg_init()) and parsed (with cfg_parse()), the
 * values can be read with the cfg_getXXX functions.
 *
 * @param cfg The configuration file context as returned from cfg_init().
 * @param filename The name of the file to parse.
 *
 * @return On success, CFG_SUCCESS is returned. If the file couldn't
 * be opened for reading, CFG_FILE_ERROR is returned. On all other
 * errors, CFG_PARSE_ERROR is returned and cfg_error() was called with
 * a descriptive error message.
 */
DLLIMPORT int __export cfg_parse(cfg_t *cfg, const char *filename);

/** Same as cfg_parse() above, but takes an already opened file as
 * argument. Reading begins at the current position. After parsing,
 * the position is not reset. The caller is responsible for closing
 * the file.
 *
 * @param cfg The configuration file context as returned from cfg_init().
 * @param fp An open file stream.
 *
 * @see cfg_parse()
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_parse_fp(cfg_t *cfg, FILE *fp);

/** Same as cfg_parse() above, but takes a character buffer as
 * argument.
 *
 * @param cfg The configuration file context as returned from cfg_init().
 * @param buf A zero-terminated string with configuration directives.
 *
 * @see cfg_parse()
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_parse_buf(cfg_t *cfg, const char *buf);

/** Free the memory allocated for the values of a given option. Only
 * the values are freed, not the option itself (it is freed by cfg_free()).
 *
 * @see cfg_free()
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_free_value(cfg_opt_t *opt);

/** Free a cfg_t context. All memory allocated by the cfg_t context
 * structure are freed, and can't be used in any further cfg_* calls.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_free(cfg_t *cfg);

/** Install a user-defined error reporting function.
 * @return The old error reporting function is returned.
 */
DLLIMPORT cfg_errfunc_t __export cfg_set_error_function(cfg_t *cfg, cfg_errfunc_t errfunc);

/** Show a parser error. Any user-defined error reporting function is called.
 * @see cfg_set_error_function
 */
DLLIMPORT void __export cfg_error(cfg_t *cfg, const char *fmt, ...);

/** Returns the option comment
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @see cfg_getcomment
 */
DLLIMPORT char * __export cfg_opt_getcomment(cfg_opt_t *opt);

/** Returns the option comment
 *
 * This function can be used to extract option annotations from a config
 * file.  Only comments preceding the option are read by cfg_parse().
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @see cfg_setcomment
 * @return The comment for this option, or NULL if unset
 */
DLLIMPORT char * __export cfg_getcomment(cfg_t *cfg, const char *name);

/** Returns the value of an integer option, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @see cfg_getnint
 */
DLLIMPORT signed long __export cfg_opt_getnint(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_getint(), used for lists.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param index Index of the value to get. Zero based.
 * @see cfg_getint
 */
DLLIMPORT long int __export cfg_getnint(cfg_t *cfg, const char *name, unsigned int index);

/** Returns the value of an integer option. This is the same as
 * calling cfg_getnint with index 0.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested value is returned. If the option was not set
 * in the configuration file, the default value given in the
 * corresponding cfg_opt_t structure is returned. It is an error to
 * try to get an option that isn't declared.
 */
DLLIMPORT long int __export cfg_getint(cfg_t *cfg, const char *name);

/** Returns the value of a floating point option, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @see cfg_getnfloat
 */
DLLIMPORT double __export cfg_opt_getnfloat(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_getfloat(), used for lists.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param index Index of the value to get. Zero based.
 * @see cfg_getfloat
 */
DLLIMPORT double __export cfg_getnfloat(cfg_t *cfg, const char *name, unsigned int index);

/** Returns the value of a floating point option.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested value is returned. If the option was not set
 * in the configuration file, the default value given in the
 * corresponding cfg_opt_t structure is returned. It is an error to
 * try to get an option that isn't declared.
 */
DLLIMPORT double __export cfg_getfloat(cfg_t *cfg, const char *name);

/** Returns the value of a string option, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @see cfg_getnstr
 */
DLLIMPORT char *__export cfg_opt_getnstr(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_getstr(), used for lists.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param index Index of the value to get. Zero based.
 * @see cfg_getstr
 */
DLLIMPORT char *__export cfg_getnstr(cfg_t *cfg, const char *name, unsigned int index);

/** Returns the value of a string option.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested value is returned. If the option was not set
 * in the configuration file, the default value given in the
 * corresponding cfg_opt_t structure is returned. It is an error to
 * try to get an option that isn't declared.
 */
DLLIMPORT char *__export cfg_getstr(cfg_t *cfg, const char *name);

/** Returns the value of a boolean option, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @see cfg_getnbool
 */
DLLIMPORT cfg_bool_t __export cfg_opt_getnbool(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_getbool(), used for lists.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param index Index of the value to get. Zero based.
 * @see cfg_getbool
 */
DLLIMPORT cfg_bool_t __export cfg_getnbool(cfg_t *cfg, const char *name, unsigned int index);

/** Returns the value of a boolean option.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested value is returned. If the option was not set
 * in the configuration file, the default value given in the
 * corresponding cfg_opt_t structure is returned. It is an error to
 * try to get an option that isn't declared.
 */
DLLIMPORT cfg_bool_t __export cfg_getbool(cfg_t *cfg, const char *name);


DLLIMPORT void *__export cfg_opt_getnptr(cfg_opt_t *opt, unsigned int index);
DLLIMPORT void *__export cfg_getnptr(cfg_t *cfg, const char *name, unsigned int indx);

/** Returns the value of a user-defined option (void pointer).
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested value is returned. If the option was not set
 * in the configuration file, the default value given in the
 * corresponding cfg_opt_t structure is returned. It is an error to
 * try to get an option that isn't declared.
 */
DLLIMPORT void *__export cfg_getptr(cfg_t *cfg, const char *name);


/** Returns the value of a section option, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the value to get. Zero based.
 * @see cfg_getnsec
 */
DLLIMPORT cfg_t *__export cfg_opt_getnsec(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_getsec(), used for sections with the
 * CFGF_MULTI flag set.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param index Index of the section to get. Zero based.
 * @see cfg_getsec
 */
DLLIMPORT cfg_t *__export cfg_getnsec(cfg_t *cfg, const char *name, unsigned int index);

/** Returns the value of a section option, given a cfg_opt_t pointer
 * and the title.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param title The title of this section. The CFGF_TITLE flag must
 * have been set for this option.
 * @see cfg_gettsec
 */
DLLIMPORT cfg_t *__export cfg_opt_gettsec(cfg_opt_t *opt, const char *title);

/** Return a section given the title, used for section with the
 * CFGF_TITLE flag set.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param title The title of this section. The CFGF_TITLE flag must
 * have been set for this option.
 * @see cfg_getsec
 */
DLLIMPORT cfg_t *__export cfg_gettsec(cfg_t *cfg, const char *name, const char *title);

/** Returns the value of a section option. The returned value is
 * another cfg_t structure that can be used in following calls to
 * cfg_getint, cfg_getstr or other get-functions.
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @return The requested section is returned. If no section is found
 * with that name, 0 is returned. There can only be default values for
 * section without the CFGF_MULTI flag set. It is an error to try to
 * get a section that isn't declared.
 */
DLLIMPORT cfg_t *__export cfg_getsec(cfg_t *cfg, const char *name);

/** Return the number of values this option has. If no default value
 * is given for the option and no value was found in the config file,
 * 0 will be returned (ie, the option value is not set at all).
 * @param opt The option structure (eg, as returned from cfg_getopt())
 */
DLLIMPORT unsigned int __export cfg_opt_size(cfg_opt_t *opt);

/** Return the number of values this option has. If no default value
 * is given for the option and no value was found in the config file,
 * 0 will be returned (ie, the option value is not set at all).
 *
 * Note that there is no way to *not* specify a default value for integers,
 * floats and booleans. Ie, they always have default values (since 0 or NULL is
 * a valid integer/float/boolean value). Only strings and lists may have no
 * default value.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 */
DLLIMPORT unsigned int __export cfg_size(cfg_t *cfg, const char *name);

/** Return the title of a section.
 *
 * @param cfg The configuration file context.
 * @return Returns the title, or 0 if there is no title. This string
 * should not be modified.
 */
DLLIMPORT const char *__export cfg_title(cfg_t *cfg);

/** Return the name of a section.
 *
 * @param cfg The configuration file context.
 * @return Returns the title, or 0 if there is no title. This string
 * should not be modified.
 */
DLLIMPORT const char *__export cfg_name(cfg_t *cfg);

/** Return the name of an option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @return Returns the title, or 0 if there is no title. This string
 * should not be modified.
 */
DLLIMPORT const char *__export cfg_opt_name(cfg_opt_t *opt);

/** Predefined include-function. This function can be used in the
 * options passed to cfg_init() to specify a function for including
 * other configuration files in the parsing. For example:
 * CFG_FUNC("include", &cfg_include)
 */
DLLIMPORT int __export cfg_include(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv);

/** Does tilde expansion (~ -> $HOME) on the filename.
 * @return The expanded filename is returned. If a ~user was not
 * found, the original filename is returned. In any case, a
 * dynamically allocated string is returned, which should be free()'d
 * by the caller.
 */
DLLIMPORT char *__export cfg_tilde_expand(const char *filename);

/** Parse a boolean option string. Accepted "true" values are "true",
 * "on" and "yes", and accepted "false" values are "false", "off" and
 * "no".
 *
 * @return Returns 1 or 0 (true/false) if the string was parsed
 * correctly, or -1 if an error occurred.
 */
DLLIMPORT int __export cfg_parse_boolean(const char *s);

/** Return an option given it's name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 *
 * @return Returns a pointer to the option. If the option isn't declared,
 * libConfuse will print an error message and return 0.
 */
DLLIMPORT cfg_opt_t *__export cfg_getopt(cfg_t *cfg, const char *name);

/** Set an option (create an instance of an option).
 *
 * @param cfg The configuration file context.
 * @param opt The option definition.
 * @param value The initial value for the option.
 *
 * @return Returns a pointer to the value object.
 */
DLLIMPORT cfg_value_t *cfg_setopt(cfg_t *cfg, cfg_opt_t *opt, const char *value);

/** Annotate an option
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param comment The annotation
 * @see cfg_setcomment
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_setcomment(cfg_opt_t *opt, char *comment);

/** Annotate an option given its name
 *
 * All options can be annotated as long as the CFGF_COMMENTS flag is
 * given to cfg_init().
 *
 * When calling cfg_print(), annotations are saved as a C style one-liner
 * comment before each option.
 *
 * When calling cfg_parse(), only one-liner comments preceding an option
 * are read and used to annotate the option.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param comment The annotation
 *
 * @return POSIX OK(0), or non-zero on failure.  This function will fail
 * if memory for the new comment cannot be allocated.
 */
DLLIMPORT int __export cfg_setcomment(cfg_t *cfg, const char *name, char *comment);

/** Set a value of an integer option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_setnint(cfg_opt_t *opt, long int value, unsigned int index);

/** Set the value of an integer option given its name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set. If the option is a list (the CFGF_LIST flag
 * is set), only the first value (with index 0) is set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setint(cfg_t *cfg, const char *name, long int value);

/** Set a value of an integer option given its name and index.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setnint(cfg_t *cfg, const char *name, long int value, unsigned int index);

/** Set a value of a floating point option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_setnfloat(cfg_opt_t *opt, double value, unsigned int index);

/** Set the value of a floating point option given its name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set. If the option is a list (the CFGF_LIST flag
 * is set), only the first value (with index 0) is set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setfloat(cfg_t *cfg, const char *name, double value);

/** Set a value of a floating point option given its name and index.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setnfloat(cfg_t *cfg, const char *name, double value, unsigned int index);

/** Set a value of a boolean option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_setnbool(cfg_opt_t *opt, cfg_bool_t value, unsigned int index);

/** Set the value of a boolean option given its name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set. If the option is a list (the CFGF_LIST flag
 * is set), only the first value (with index 0) is set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setbool(cfg_t *cfg, const char *name, cfg_bool_t value);

/** Set a value of a boolean option given its name and index.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setnbool(cfg_t *cfg, const char *name, cfg_bool_t value, unsigned int index);

/** Set a value of a string option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param value The value to set. Memory for the string is allocated
 * and the value is copied. Any previous string value is freed.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_setnstr(cfg_opt_t *opt, const char *value, unsigned int index);

/** Set the value of a string option given its name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set. Memory for the string is allocated and the
 * value is copied. Any previous string value is freed. If the option is a list
 * (the CFGF_LIST flag is set), only the first value (with index 0) is set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setstr(cfg_t *cfg, const char *name, const char *value);

/** Set a value of a boolean option given its name and index.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param value The value to set. Memory for the string is allocated
 * and the value is copied. Any privious string value is freed.
 * @param index The index in the option value array that should be
 * modified. It is an error to set values with indices larger than 0
 * for options without the CFGF_LIST flag set.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setnstr(cfg_t *cfg, const char *name, const char *value, unsigned int index);

/** Set values for a list option. All existing values are replaced
 * with the new ones.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param nvalues Number of values to set.
 * @param ... The values to set, the type must match the type of the
 * option and the number of values must be equal to the nvalues
 * parameter.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_setlist(cfg_t *cfg, const char *name, unsigned int nvalues, ...);

DLLIMPORT int __export cfg_numopts(cfg_opt_t *opts);

/** Add values for a list option. The new values are appended to any
 * current values in the list.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param nvalues Number of values to add.
 * @param ... The values to add, the type must match the type of the
 * option and the number of values must be equal to the nvalues
 * parameter.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_addlist(cfg_t *cfg, const char *name, unsigned int nvalues, ...);

/** Set an option (create an instance of an option).
 *
 * @param cfg The configuration file context.
 * @param opt The option definition.
 * @param nvalues The number of values to set for the option.
 * @param values The value(s) for the option.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int cfg_opt_setmulti(cfg_t *cfg, cfg_opt_t *opt, unsigned int nvalues, char **values);

/** Set an option (create an instance of an option).
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param nvalues The number of values to set for the option.
 * @param values The value(s) for the option.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int cfg_setmulti(cfg_t *cfg, const char *name, unsigned int nvalues, char **values);

/** Create a new titled config section.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param title The title of this section.
 *
 * @return A pointer to the created section or if the section
 * already exists a pointer to that section is returned.
 * If the section could not be created or found, 0 is returned.
 */
DLLIMPORT cfg_t *cfg_addtsec(cfg_t *cfg, const char *name, const char *title);

/** Removes and frees a config section, given a cfg_opt_t pointer.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index Index of the section to remove. Zero based.
 * @see cfg_rmnsec
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_rmnsec(cfg_opt_t *opt, unsigned int index);

/** Indexed version of cfg_rmsec(), used for CFGF_MULTI sections.
 * @param cfg The configuration file context.
 * @param name The name of the section.
 * @param index Index of the section to remove. Zero based.
 * @see cfg_rmsec
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_rmnsec(cfg_t *cfg, const char *name, unsigned int index);

/** Removes and frees a config section. This is the same as
 * calling cfg_rmnsec with index 0.
 * @param cfg The configuration file context.
 * @param name The name of the section.
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_rmsec(cfg_t *cfg, const char *name);

/** Removes and frees a config section, given a cfg_opt_t pointer
 * and the title.
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param title The title of this section. The CFGF_TITLE flag must
 * have been set for this option.
 * @see cfg_rmtsec
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_rmtsec(cfg_opt_t *opt, const char *title);

/** Removes and frees a section given the title, used for section with the
 * CFGF_TITLE flag set.
 *
 * @param cfg The configuration file context.
 * @param name The name of the section.
 * @param title The title of this section. The CFGF_TITLE flag must
 * have been set for this option.
 * @see cfg_rmsec
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_rmtsec(cfg_t *cfg, const char *name, const char *title);

/** Default value print function.
 *
 * Print only the value of a given option. Does not handle sections or
 * functions. Use cfg_opt_print to print the whole assignment ("option
 * = value"), or cfg_print to print the whole config file.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param index The index in the option value array that should be printed
 * @param fp File stream to print to.
 *
 * @see cfg_print, cfg_opt_print
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_nprint_var(cfg_opt_t *opt, unsigned int index, FILE *fp);

/** Print an option and its value to a file.
 * Same as cfg_opt_print, but with the indentation level specified.
 * @see cfg_opt_print
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_print_indent(cfg_opt_t *opt, FILE *fp, int indent);

/** Print an option and its value to a file.
 *
 * If a print callback function is specified for the option, it is
 * used instead of cfg_opt_nprint_var.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param fp File stream to print to.
 *
 * @see cfg_print_func_t
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_opt_print(cfg_opt_t *opt, FILE *fp);

/** Print the options and values to a file.
 * Same as cfg_print, but with the indentation level specified.
 * @see cfg_print
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_print_indent(cfg_t *cfg, FILE *fp, int indent);

/** Print the options and values to a file.
 *
 * Note that options in any included file are expanded and printed
 * directly to the file. Option values given with environment
 * variables in the parsed input are also printed expanded. This means
 * that if you parse a configuration file you can't expect that the
 * output from this function is identical to the initial file.
 *
 * @param cfg The configuration file context.
 * @param fp File stream to print to, use stdout to print to the screen.
 *
 * @see cfg_print_func_t, cfg_set_print_func
 *
 * @return POSIX OK(0), or non-zero on failure.
 */
DLLIMPORT int __export cfg_print(cfg_t *cfg, FILE *fp);

/** Set a print callback function for an option.
 *
 * @param opt The option structure (eg, as returned from cfg_getopt())
 * @param pf The print function callback.
 *
 * @see cfg_print_func_t
 */
DLLIMPORT cfg_print_func_t __export cfg_opt_set_print_func(cfg_opt_t *opt, cfg_print_func_t pf);

/** Set a print callback function for an option given its name.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param pf The print callback function.
 *
 * @see cfg_print_func_t
 */
DLLIMPORT cfg_print_func_t __export cfg_set_print_func(cfg_t *cfg, const char *name, cfg_print_func_t pf);

/** Register a validating callback function for an option.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param vf The validating callback function.
 *
 * @see cfg_validate_callback_t
 */
DLLIMPORT cfg_validate_callback_t __export cfg_set_validate_func(cfg_t *cfg, const char *name, cfg_validate_callback_t vf);

/** Register a validating callback function for an option.
 *
 * This callback is called for all cfg_set*() functions, although not
 * cfg_opt_set*(), and can be used to check and modify a value/string
 * *before* it is actually set.  The regular callbacks are run after
 * the fact and are only called when parsing a buffer or file.
 *
 * @param cfg The configuration file context.
 * @param name The name of the option.
 * @param vf The validating callback function.
 *
 * @see cfg_validate_callback2_t
 */
DLLIMPORT cfg_validate_callback2_t __export cfg_set_validate_func2(cfg_t *cfg, const char *name, cfg_validate_callback2_t vf);

#ifdef __cplusplus
}
#endif
#endif /* CONFUSE_H_ */

/** @example ftpconf.c
 */

/** @example simple.c
 */

/** @example reread.c
 */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
