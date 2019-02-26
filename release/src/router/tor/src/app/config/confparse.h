/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file confparse.h
 *
 * \brief Header for confparse.c.
 */

#ifndef TOR_CONFPARSE_H
#define TOR_CONFPARSE_H

/** Enumeration of types which option values can take */
typedef enum config_type_t {
  CONFIG_TYPE_STRING = 0,   /**< An arbitrary string. */
  CONFIG_TYPE_FILENAME,     /**< A filename: some prefixes get expanded. */
  CONFIG_TYPE_UINT,         /**< A non-negative integer less than MAX_INT */
  CONFIG_TYPE_INT,          /**< Any integer. */
  CONFIG_TYPE_UINT64,       /**< A value in range 0..UINT64_MAX */
  CONFIG_TYPE_PORT,         /**< A port from 1...65535, 0 for "not set", or
                             * "auto".  */
  CONFIG_TYPE_INTERVAL,     /**< A number of seconds, with optional units*/
  CONFIG_TYPE_MSEC_INTERVAL,/**< A number of milliseconds, with optional
                              * units */
  CONFIG_TYPE_MEMUNIT,      /**< A number of bytes, with optional units*/
  CONFIG_TYPE_DOUBLE,       /**< A floating-point value */
  CONFIG_TYPE_BOOL,         /**< A boolean value, expressed as 0 or 1. */
  CONFIG_TYPE_AUTOBOOL,     /**< A boolean+auto value, expressed 0 for false,
                             * 1 for true, and -1 for auto  */
  CONFIG_TYPE_ISOTIME,      /**< An ISO-formatted time relative to UTC. */
  CONFIG_TYPE_CSV,          /**< A list of strings, separated by commas and
                              * optional whitespace. */
  CONFIG_TYPE_CSV_INTERVAL, /**< A list of strings, separated by commas and
                              * optional whitespace, representing intervals in
                              * seconds, with optional units.  We allow
                              * multiple values here for legacy reasons, but
                              * ignore every value after the first. */
  CONFIG_TYPE_LINELIST,     /**< Uninterpreted config lines */
  CONFIG_TYPE_LINELIST_S,   /**< Uninterpreted, context-sensitive config lines,
                             * mixed with other keywords. */
  CONFIG_TYPE_LINELIST_V,   /**< Catch-all "virtual" option to summarize
                             * context-sensitive config lines when fetching.
                             */
  CONFIG_TYPE_ROUTERSET,    /**< A list of router names, addrs, and fps,
                             * parsed into a routerset_t. */
  CONFIG_TYPE_OBSOLETE,     /**< Obsolete (ignored) option. */
} config_type_t;

#ifdef TOR_UNIT_TESTS
/**
 * Union used when building in test mode typechecking the members of a type
 * used with confparse.c.  See CONF_CHECK_VAR_TYPE for a description of how
 * it is used. */
typedef union {
  char **STRING;
  char **FILENAME;
  int *UINT; /* yes, really: Even though the confparse type is called
              * "UINT", it still uses the C int type -- it just enforces that
              * the values are in range [0,INT_MAX].
              */
  uint64_t *UINT64;
  int *INT;
  int *PORT;
  int *INTERVAL;
  int *MSEC_INTERVAL;
  uint64_t *MEMUNIT;
  double *DOUBLE;
  int *BOOL;
  int *AUTOBOOL;
  time_t *ISOTIME;
  smartlist_t **CSV;
  int *CSV_INTERVAL;
  struct config_line_t **LINELIST;
  struct config_line_t **LINELIST_S;
  struct config_line_t **LINELIST_V;
  routerset_t **ROUTERSET;
} confparse_dummy_values_t;
#endif /* defined(TOR_UNIT_TESTS) */

/** An abbreviation for a configuration option allowed on the command line. */
typedef struct config_abbrev_t {
  const char *abbreviated;
  const char *full;
  int commandline_only;
  int warn;
} config_abbrev_t;

typedef struct config_deprecation_t {
  const char *name;
  const char *why_deprecated;
} config_deprecation_t;

/* Handy macro for declaring "In the config file or on the command line,
 * you can abbreviate <b>tok</b>s as <b>tok</b>". */
#define PLURAL(tok) { #tok, #tok "s", 0, 0 }

/** A variable allowed in the configuration file or on the command line. */
typedef struct config_var_t {
  const char *name; /**< The full keyword (case insensitive). */
  config_type_t type; /**< How to interpret the type and turn it into a
                       * value. */
  off_t var_offset; /**< Offset of the corresponding member of or_options_t. */
  const char *initvalue; /**< String (or null) describing initial value. */

#ifdef TOR_UNIT_TESTS
  /** Used for compiler-magic to typecheck the corresponding field in the
   * corresponding struct. Only used in unit test mode, at compile-time. */
  confparse_dummy_values_t var_ptr_dummy;
#endif
} config_var_t;

/* Macros to define extra members inside config_var_t fields, and at the
 * end of a list of them.
 */
#ifdef TOR_UNIT_TESTS
/* This is a somewhat magic type-checking macro for users of confparse.c.
 * It initializes a union member "confparse_dummy_values_t.conftype" with
 * the address of a static member "tp_dummy.member".   This
 * will give a compiler warning unless the member field is of the correct
 * type.
 *
 * (This warning is mandatory, because a type mismatch here violates the type
 * compatibility constraint for simple assignment, and requires a diagnostic,
 * according to the C spec.)
 *
 * For example, suppose you say:
 *     "CONF_CHECK_VAR_TYPE(or_options_t, STRING, Address)".
 * Then this macro will evaluate to:
 *     { .STRING = &or_options_t_dummy.Address }
 * And since confparse_dummy_values_t.STRING has type "char **", that
 * expression will create a warning unless or_options_t.Address also
 * has type "char *".
 */
#define CONF_CHECK_VAR_TYPE(tp, conftype, member)       \
  { . conftype = &tp ## _dummy . member }
#define CONF_TEST_MEMBERS(tp, conftype, member) \
  , CONF_CHECK_VAR_TYPE(tp, conftype, member)
#define END_OF_CONFIG_VARS                                      \
  { NULL, CONFIG_TYPE_OBSOLETE, 0, NULL, { .INT=NULL } }
#define DUMMY_TYPECHECK_INSTANCE(tp)            \
  static tp tp ## _dummy
#else /* !(defined(TOR_UNIT_TESTS)) */
#define CONF_TEST_MEMBERS(tp, conftype, member)
#define END_OF_CONFIG_VARS { NULL, CONFIG_TYPE_OBSOLETE, 0, NULL }
/* Repeatedly declarable incomplete struct to absorb redundant semicolons */
#define DUMMY_TYPECHECK_INSTANCE(tp)            \
  struct tor_semicolon_eater
#endif /* defined(TOR_UNIT_TESTS) */

/** Type of a callback to validate whether a given configuration is
 * well-formed and consistent. See options_trial_assign() for documentation
 * of arguments. */
typedef int (*validate_fn_t)(void*,void*,void*,int,char**);

/** Callback to free a configuration object. */
typedef void (*free_cfg_fn_t)(void*);

/** Information on the keys, value types, key-to-struct-member mappings,
 * variable descriptions, validation functions, and abbreviations for a
 * configuration or storage format. */
typedef struct config_format_t {
  size_t size; /**< Size of the struct that everything gets parsed into. */
  uint32_t magic; /**< Required 'magic value' to make sure we have a struct
                   * of the right type. */
  off_t magic_offset; /**< Offset of the magic value within the struct. */
  config_abbrev_t *abbrevs; /**< List of abbreviations that we expand when
                             * parsing this format. */
  const config_deprecation_t *deprecations; /** List of deprecated options */
  config_var_t *vars; /**< List of variables we recognize, their default
                       * values, and where we stick them in the structure. */
  validate_fn_t validate_fn; /**< Function to validate config. */
  free_cfg_fn_t free_fn; /**< Function to free the configuration. */
  /** If present, extra is a LINELIST variable for unrecognized
   * lines.  Otherwise, unrecognized lines are an error. */
  config_var_t *extra;
} config_format_t;

/** Macro: assert that <b>cfg</b> has the right magic field for format
 * <b>fmt</b>. */
#define CONFIG_CHECK(fmt, cfg) STMT_BEGIN                               \
    tor_assert(fmt && cfg);                                             \
    tor_assert((fmt)->magic ==                                          \
               *(uint32_t*)STRUCT_VAR_P(cfg,fmt->magic_offset));        \
  STMT_END

#define CAL_USE_DEFAULTS      (1u<<0)
#define CAL_CLEAR_FIRST       (1u<<1)
#define CAL_WARN_DEPRECATIONS (1u<<2)

void *config_new(const config_format_t *fmt);
void config_free_(const config_format_t *fmt, void *options);
#define config_free(fmt, options) do {                \
    config_free_((fmt), (options));                   \
    (options) = NULL;                                 \
  } while (0)

struct config_line_t *config_get_assigned_option(const config_format_t *fmt,
                                          const void *options, const char *key,
                                          int escape_val);
int config_is_same(const config_format_t *fmt,
                   const void *o1, const void *o2,
                   const char *name);
void config_init(const config_format_t *fmt, void *options);
void *config_dup(const config_format_t *fmt, const void *old);
char *config_dump(const config_format_t *fmt, const void *default_options,
                  const void *options, int minimal,
                  int comment_defaults);
int config_assign(const config_format_t *fmt, void *options,
                  struct config_line_t *list,
                  unsigned flags, char **msg);
config_var_t *config_find_option_mutable(config_format_t *fmt,
                                         const char *key);
const char *config_find_deprecation(const config_format_t *fmt,
                                     const char *key);
const config_var_t *config_find_option(const config_format_t *fmt,
                                       const char *key);
const char *config_expand_abbrev(const config_format_t *fmt,
                                 const char *option,
                                 int command_line, int warn_obsolete);
void warn_deprecated_option(const char *what, const char *why);

/* Helper macros to compare an option across two configuration objects */
#define CFG_EQ_BOOL(a,b,opt) ((a)->opt == (b)->opt)
#define CFG_EQ_INT(a,b,opt) ((a)->opt == (b)->opt)
#define CFG_EQ_STRING(a,b,opt) (!strcmp_opt((a)->opt, (b)->opt))
#define CFG_EQ_SMARTLIST(a,b,opt) smartlist_strings_eq((a)->opt, (b)->opt)
#define CFG_EQ_LINELIST(a,b,opt) config_lines_eq((a)->opt, (b)->opt)
#define CFG_EQ_ROUTERSET(a,b,opt) routerset_equal((a)->opt, (b)->opt)

#endif /* !defined(TOR_CONFPARSE_H) */
