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

#include "lib/conf/conftypes.h"
#include "lib/conf/confmacros.h"
#include "lib/testsupport/testsupport.h"

/**
 * An abbreviation or alias for a configuration option.
 **/
typedef struct config_abbrev_t {
  /** The option name as abbreviated.  Not case-sensitive. */
  const char *abbreviated;
  /** The full name of the option. Not case-sensitive. */
  const char *full;
  /** True if this abbreviation should only be allowed on the command line. */
  int commandline_only;
  /** True if we should warn whenever this abbreviation is used. */
  int warn;
} config_abbrev_t;

/**
 * A note that a configuration option is deprecated, with an explanation why.
 */
typedef struct config_deprecation_t {
  /** The option that is deprecated. */
  const char *name;
  /** A user-facing string explaining why the option is deprecated. */
  const char *why_deprecated;
} config_deprecation_t;

/**
 * Handy macro for declaring "In the config file or on the command line, you
 * can abbreviate <b>tok</b>s as <b>tok</b>". Used inside an array of
 * config_abbrev_t.
 *
 * For example, to declare "NumCpu" as an abbreviation for "NumCPUs",
 * you can say PLURAL(NumCpu).
 **/
#define PLURAL(tok) { #tok, #tok "s", 0, 0 }

/**
 * Type of a callback to validate whether a given configuration is
 * well-formed and consistent.
 *
 * The configuration to validate is passed as <b>newval</b>. The previous
 * configuration, if any, is provided in <b>oldval</b>.  The
 * <b>default_val</b> argument receives a configuration object initialized
 * with default values for all its fields.  The <b>from_setconf</b> argument
 * is true iff the input comes from a SETCONF controller command.
 *
 * On success, return 0.  On failure, set *<b>msg_out</b> to a newly allocated
 * error message, and return -1.
 *
 * REFACTORING NOTE: Currently, this callback type is only used from inside
 * config_dump(); later in our refactoring, it will be cleaned up and used
 * more generally.
 */
typedef int (*validate_fn_t)(void *oldval,
                             void *newval,
                             void *default_val,
                             int from_setconf,
                             char **msg_out);

struct config_mgr_t;

/**
 * Callback to clear all non-managed fields of a configuration object.
 *
 * <b>obj</b> is the configuration object whose non-managed fields should be
 * cleared.
 *
 * (Regular fields get cleared by config_reset(), but you might have fields
 * in the object that do not correspond to configuration variables.  If those
 * fields need to be cleared or freed, this is where to do it.)
 */
typedef void (*clear_cfg_fn_t)(const struct config_mgr_t *mgr, void *obj);

/** Information on the keys, value types, key-to-struct-member mappings,
 * variable descriptions, validation functions, and abbreviations for a
 * configuration or storage format. */
typedef struct config_format_t {
  size_t size; /**< Size of the struct that everything gets parsed into. */
  struct_magic_decl_t magic; /**< Magic number info for this struct. */
  const config_abbrev_t *abbrevs; /**< List of abbreviations that we expand
                             * when parsing this format. */
  const config_deprecation_t *deprecations; /** List of deprecated options */
  const config_var_t *vars; /**< List of variables we recognize, their default
                             * values, and where we stick them in the
                             * structure. */
  validate_fn_t validate_fn; /**< Function to validate config. */
  clear_cfg_fn_t clear_fn; /**< Function to clear the configuration. */
  /** If present, extra denotes a LINELIST variable for unrecognized
   * lines.  Otherwise, unrecognized lines are an error. */
  const struct_member_t *extra;
  /** The position of a config_suite_t pointer within the toplevel object,
   * or -1 if there is no such pointer. */
  ptrdiff_t config_suite_offset;
} config_format_t;

/**
 * A collection of config_format_t objects to describe several objects
 * that are all configured with the same configuration file.
 *
 * (NOTE: for now, this only handles a single config_format_t.)
 **/
typedef struct config_mgr_t config_mgr_t;

config_mgr_t *config_mgr_new(const config_format_t *toplevel_fmt);
void config_mgr_free_(config_mgr_t *mgr);
int config_mgr_add_format(config_mgr_t *mgr,
                          const config_format_t *fmt);
void config_mgr_freeze(config_mgr_t *mgr);
#define config_mgr_free(mgr) \
  FREE_AND_NULL(config_mgr_t, config_mgr_free_, (mgr))
struct smartlist_t *config_mgr_list_vars(const config_mgr_t *mgr);
struct smartlist_t *config_mgr_list_deprecated_vars(const config_mgr_t *mgr);

/** A collection of managed configuration objects. */
typedef struct config_suite_t config_suite_t;

/**
 * Flag for config_assign: if set, then "resetting" an option changes it to
 * its default value, as specified in the config_var_t.  Otherwise,
 * "resetting" an option changes it to a type-dependent null value --
 * typically 0 or NULL.
 *
 * (An option is "reset" when it is set to an empty value, or as described in
 * CAL_CLEAR_FIRST).
 **/
#define CAL_USE_DEFAULTS      (1u<<0)
/**
 * Flag for config_assign: if set, then we reset every provided config
 * option before we set it.
 *
 * For example, if this flag is not set, then passing a multi-line option to
 * config_assign will cause any previous value to be extended. But if this
 * flag is set, then a multi-line option will replace any previous value.
 **/
#define CAL_CLEAR_FIRST       (1u<<1)
/**
 * Flag for config_assign: if set, we warn about deprecated options.
 **/
#define CAL_WARN_DEPRECATIONS (1u<<2)

void *config_new(const config_mgr_t *fmt);
void config_free_(const config_mgr_t *fmt, void *options);
#define config_free(mgr, options) do {                \
    config_free_((mgr), (options));                   \
    (options) = NULL;                                 \
  } while (0)

struct config_line_t *config_get_assigned_option(const config_mgr_t *mgr,
                                          const void *options, const char *key,
                                          int escape_val);
int config_is_same(const config_mgr_t *fmt,
                   const void *o1, const void *o2,
                   const char *name);
struct config_line_t *config_get_changes(const config_mgr_t *mgr,
                                  const void *options1, const void *options2);
void config_init(const config_mgr_t *mgr, void *options);
void *config_dup(const config_mgr_t *mgr, const void *old);
char *config_dump(const config_mgr_t *mgr, const void *default_options,
                  const void *options, int minimal,
                  int comment_defaults);
bool config_check_ok(const config_mgr_t *mgr, const void *options,
                     int severity);
int config_assign(const config_mgr_t *mgr, void *options,
                  struct config_line_t *list,
                  unsigned flags, char **msg);
const char *config_find_deprecation(const config_mgr_t *mgr,
                                    const char *key);
const char *config_find_option_name(const config_mgr_t *mgr,
                                    const char *key);
const char *config_expand_abbrev(const config_mgr_t *mgr,
                                 const char *option,
                                 int command_line, int warn_obsolete);
void warn_deprecated_option(const char *what, const char *why);

bool config_var_is_settable(const config_var_t *var);
bool config_var_is_listable(const config_var_t *var);

/* Helper macros to compare an option across two configuration objects */
#define CFG_EQ_BOOL(a,b,opt) ((a)->opt == (b)->opt)
#define CFG_EQ_INT(a,b,opt) ((a)->opt == (b)->opt)
#define CFG_EQ_STRING(a,b,opt) (!strcmp_opt((a)->opt, (b)->opt))
#define CFG_EQ_SMARTLIST(a,b,opt) smartlist_strings_eq((a)->opt, (b)->opt)
#define CFG_EQ_LINELIST(a,b,opt) config_lines_eq((a)->opt, (b)->opt)
#define CFG_EQ_ROUTERSET(a,b,opt) routerset_equal((a)->opt, (b)->opt)

#ifdef CONFPARSE_PRIVATE
STATIC void config_reset_line(const config_mgr_t *mgr, void *options,
                              const char *key, int use_defaults);
STATIC void *config_mgr_get_obj_mutable(const config_mgr_t *mgr,
                                        void *toplevel, int idx);
STATIC const void *config_mgr_get_obj(const config_mgr_t *mgr,
                                       const void *toplevel, int idx);
#endif /* defined(CONFPARSE_PRIVATE) */

#endif /* !defined(TOR_CONFPARSE_H) */
