/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file confmgt.c
 *
 * \brief Back-end for parsing and generating key-value files, used to
 *   implement the torrc file format and the state file.
 *
 * This module is used by config.c to parse and encode torrc
 * configuration files, and by statefile.c to parse and encode the
 * $DATADIR/state file.
 *
 * To use this module, its callers provide an instance of
 * config_format_t to describe the mappings from a set of configuration
 * options to a number of fields in a C structure.  With this mapping,
 * the functions here can convert back and forth between the C structure
 * specified, and a linked list of key-value pairs.
 */

#define CONFMGT_PRIVATE
#include "orconfig.h"
#include "lib/confmgt/confmgt.h"

#include "lib/confmgt/structvar.h"
#include "lib/confmgt/unitparse.h"
#include "lib/container/bitarray.h"
#include "lib/container/smartlist.h"
#include "lib/encoding/confline.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"

#include "ext/siphash.h"

/**
 * A managed_var_t is an internal wrapper around a config_var_t in
 * a config_format_t structure.  It is used by config_mgr_t to
 * keep track of which option goes with which structure. */
typedef struct managed_var_t {
  /**
   * A pointer to the config_var_t for this option.
   */
  const config_var_t *cvar;
  /**
   * The index of the object in which this option is stored.  It is
   * IDX_TOPLEVEL to indicate that the object is the top-level object.
   **/
  int object_idx;
} managed_var_t;

static void config_reset(const config_mgr_t *fmt, void *options,
                         const managed_var_t *var, int use_defaults);
static void config_mgr_register_fmt(config_mgr_t *mgr,
                                    const config_format_t *fmt,
                                    int object_idx);

/** Release all storage held in a managed_var_t. */
static void
managed_var_free_(managed_var_t *mv)
{
  if (!mv)
    return;
  tor_free(mv);
}
#define managed_var_free(mv) \
  FREE_AND_NULL(managed_var_t, managed_var_free_, (mv))

struct config_suite_t {
  /** A list of configuration objects managed by a given configuration
   * manager. They are stored in the same order as the config_format_t
   * objects in the manager's list of subformats. */
  smartlist_t *configs;
};

/**
 * Allocate a new empty config_suite_t.
 **/
static config_suite_t *
config_suite_new(void)
{
  config_suite_t *suite = tor_malloc_zero(sizeof(config_suite_t));
  suite->configs = smartlist_new();
  return suite;
}

/** Release all storage held by a config_suite_t.  (Does not free
 * any configuration objects it holds; the caller must do that first.) */
static void
config_suite_free_(config_suite_t *suite)
{
  if (!suite)
    return;
  smartlist_free(suite->configs);
  tor_free(suite);
}

#define config_suite_free(suite) \
  FREE_AND_NULL(config_suite_t, config_suite_free_, (suite))

struct config_mgr_t {
  /** The 'top-level' configuration format.  This one is used for legacy
   * options that have not yet been assigned to different sub-modules.
   *
   * (NOTE: for now, this is the only config_format_t that a config_mgr_t
   * contains.  A subsequent commit will add more. XXXX)
   */
  const config_format_t *toplevel;
  /**
   * List of second-level configuration format objects that this manager
   * also knows about.
   */
  smartlist_t *subconfigs;
  /** A smartlist of managed_var_t objects for all configuration formats. */
  smartlist_t *all_vars;
  /** A smartlist of config_abbrev_t objects for all configuration
   * formats. These objects are used to track synonyms and abbreviations for
   * different configuration options. */
  smartlist_t *all_abbrevs;
  /** A smartlist of config_deprecation_t for all configuration formats. */
  smartlist_t *all_deprecations;
  /** True if this manager has been frozen and cannot have any more formats
   * added to it. A manager must be frozen before it can be used to construct
   * or manipulate objects. */
  bool frozen;
  /** A replacement for the magic number of the toplevel object. We override
   * that number to make it unique for this particular config_mgr_t, so that
   * an object constructed with one mgr can't be used with another, even if
   * those managers' contents are equal.
   */
  struct_magic_decl_t toplevel_magic;
};

#define IDX_TOPLEVEL (-1)

/** Create a new config_mgr_t to manage a set of configuration objects to be
 * wrapped under <b>toplevel_fmt</b>. */
config_mgr_t *
config_mgr_new(const config_format_t *toplevel_fmt)
{
  config_mgr_t *mgr = tor_malloc_zero(sizeof(config_mgr_t));
  mgr->subconfigs = smartlist_new();
  mgr->all_vars = smartlist_new();
  mgr->all_abbrevs = smartlist_new();
  mgr->all_deprecations = smartlist_new();

  config_mgr_register_fmt(mgr, toplevel_fmt, IDX_TOPLEVEL);
  mgr->toplevel = toplevel_fmt;

  return mgr;
}

/** Add a config_format_t to a manager, with a specified (unique) index. */
static void
config_mgr_register_fmt(config_mgr_t *mgr,
                        const config_format_t *fmt,
                        int object_idx)
{
  int i;

  tor_assertf(!mgr->frozen,
              "Tried to add a format to a configuration manager after "
              "it had been frozen.");

  if (object_idx != IDX_TOPLEVEL) {
    tor_assertf(! fmt->has_config_suite,
          "Tried to register a toplevel format in a non-toplevel position");
  }
  if (fmt->config_suite_offset) {
    tor_assertf(fmt->has_config_suite,
                "config_suite_offset was set, but has_config_suite was not.");
  }

  tor_assertf(fmt != mgr->toplevel &&
              ! smartlist_contains(mgr->subconfigs, fmt),
              "Tried to register an already-registered format.");

  /* register variables */
  for (i = 0; fmt->vars[i].member.name; ++i) {
    managed_var_t *mv = tor_malloc_zero(sizeof(managed_var_t));
    mv->cvar = &fmt->vars[i];
    mv->object_idx = object_idx;
    smartlist_add(mgr->all_vars, mv);
  }

  /* register abbrevs */
  if (fmt->abbrevs) {
    for (i = 0; fmt->abbrevs[i].abbreviated; ++i) {
      smartlist_add(mgr->all_abbrevs, (void*)&fmt->abbrevs[i]);
    }
  }

  /* register deprecations. */
  if (fmt->deprecations) {
    const config_deprecation_t *d;
    for (d = fmt->deprecations; d->name; ++d) {
      smartlist_add(mgr->all_deprecations, (void*)d);
    }
  }
}

/**
 * Add a new format to this configuration object.  Asserts on failure.
 *
 * Returns an internal "index" value used to identify this format within
 * all of those formats contained in <b>mgr</b>.  This index value
 * should not generally be used outside of this module.
 **/
int
config_mgr_add_format(config_mgr_t *mgr,
                      const config_format_t *fmt)
{
  tor_assert(mgr);
  int idx = smartlist_len(mgr->subconfigs);
  config_mgr_register_fmt(mgr, fmt, idx);
  smartlist_add(mgr->subconfigs, (void *)fmt);
  return idx;
}

/** Return a pointer to the config_suite_t * pointer inside a
 * configuration object; returns NULL if there is no such member. */
static inline config_suite_t **
config_mgr_get_suite_ptr(const config_mgr_t *mgr, void *toplevel)
{
  if (! mgr->toplevel->has_config_suite)
    return NULL;
  return STRUCT_VAR_P(toplevel, mgr->toplevel->config_suite_offset);
}

/**
 * Return a pointer to the configuration object within <b>toplevel</b> whose
 * index is <b>idx</b>.
 *
 * NOTE: XXXX Eventually, there will be multiple objects supported within the
 * toplevel object.  For example, the or_options_t will contain pointers
 * to configuration objects for other modules.  This function gets
 * the sub-object for a particular module.
 */
void *
config_mgr_get_obj_mutable(const config_mgr_t *mgr, void *toplevel, int idx)
{
  tor_assert(mgr);
  tor_assert(toplevel);
  if (idx == IDX_TOPLEVEL)
    return toplevel;

  tor_assertf(idx >= 0 && idx < smartlist_len(mgr->subconfigs),
              "Index %d is out of range.", idx);
  config_suite_t **suite = config_mgr_get_suite_ptr(mgr, toplevel);
  tor_assert(suite);
  tor_assert(smartlist_len(mgr->subconfigs) ==
             smartlist_len((*suite)->configs));

  return smartlist_get((*suite)->configs, idx);
}

/** As config_mgr_get_obj_mutable(), but return a const pointer. */
const void *
config_mgr_get_obj(const config_mgr_t *mgr, const void *toplevel, int idx)
{
  return config_mgr_get_obj_mutable(mgr, (void*)toplevel, idx);
}

/** Sorting helper for smartlist of managed_var_t */
static int
managed_var_cmp(const void **a, const void **b)
{
  const managed_var_t *mv1 = *(const managed_var_t**)a;
  const managed_var_t *mv2 = *(const managed_var_t**)b;

  return strcasecmp(mv1->cvar->member.name, mv2->cvar->member.name);
}

/**
 * Mark a configuration manager as "frozen", so that no more formats can be
 * added, and so that it can be used for manipulating configuration objects.
 **/
void
config_mgr_freeze(config_mgr_t *mgr)
{
  static uint64_t mgr_count = 0;

  smartlist_sort(mgr->all_vars, managed_var_cmp);
  memcpy(&mgr->toplevel_magic, &mgr->toplevel->magic,
         sizeof(struct_magic_decl_t));
  uint64_t magic_input[3] = { mgr->toplevel_magic.magic_val,
                              (uint64_t) (uintptr_t) mgr,
                              ++mgr_count };
  mgr->toplevel_magic.magic_val =
    (uint32_t)siphash24g(magic_input, sizeof(magic_input));
  mgr->frozen = true;
}

/** Release all storage held in <b>mgr</b> */
void
config_mgr_free_(config_mgr_t *mgr)
{
  if (!mgr)
    return;
  SMARTLIST_FOREACH(mgr->all_vars, managed_var_t *, mv, managed_var_free(mv));
  smartlist_free(mgr->all_vars);
  smartlist_free(mgr->all_abbrevs);
  smartlist_free(mgr->all_deprecations);
  smartlist_free(mgr->subconfigs);
  memset(mgr, 0, sizeof(*mgr));
  tor_free(mgr);
}

/** Return a new smartlist_t containing a config_var_t for every variable that
 * <b>mgr</b> knows about.  The elements of this smartlist do not need
 * to be freed; they have the same lifespan as <b>mgr</b>. */
smartlist_t *
config_mgr_list_vars(const config_mgr_t *mgr)
{
  smartlist_t *result = smartlist_new();
  tor_assert(mgr);
  SMARTLIST_FOREACH(mgr->all_vars, managed_var_t *, mv,
                    smartlist_add(result, (void*) mv->cvar));
  return result;
}

/** Return a new smartlist_t containing the names of all deprecated variables.
 * The elements of this smartlist do not need to be freed; they have the same
 * lifespan as <b>mgr</b>.
 */
smartlist_t *
config_mgr_list_deprecated_vars(const config_mgr_t *mgr)
{
  smartlist_t *result = smartlist_new();
  tor_assert(mgr);
  SMARTLIST_FOREACH(mgr->all_deprecations, config_deprecation_t *, d,
                    smartlist_add(result, (char*)d->name));
  return result;
}

/**
 * Check the magic number on <b>object</b> to make sure it's a valid toplevel
 * object, created with <b>mgr</b>.  Exit with an assertion if it isn't.
 **/
void
config_check_toplevel_magic(const config_mgr_t *mgr,
                            const void *object)
{
  struct_check_magic(object, &mgr->toplevel_magic);
}

/** Assert that the magic fields in <b>options</b> and its subsidiary
 * objects are all okay. */
static void
config_mgr_assert_magic_ok(const config_mgr_t *mgr,
                           const void *options)
{
  tor_assert(mgr);
  tor_assert(options);
  tor_assert(mgr->frozen);
  struct_check_magic(options, &mgr->toplevel_magic);

  config_suite_t **suitep = config_mgr_get_suite_ptr(mgr, (void*)options);
  if (suitep == NULL) {
    tor_assert(smartlist_len(mgr->subconfigs) == 0);
    return;
  }

  tor_assert(smartlist_len((*suitep)->configs) ==
             smartlist_len(mgr->subconfigs));
  SMARTLIST_FOREACH_BEGIN(mgr->subconfigs, const config_format_t *, fmt) {
    void *obj = smartlist_get((*suitep)->configs, fmt_sl_idx);
    tor_assert(obj);
    struct_check_magic(obj, &fmt->magic);
  } SMARTLIST_FOREACH_END(fmt);
}

/** Macro: assert that <b>cfg</b> has the right magic field for
 * <b>mgr</b>. */
#define CONFIG_CHECK(mgr, cfg) STMT_BEGIN                               \
    config_mgr_assert_magic_ok((mgr), (cfg));                           \
  STMT_END

/** Allocate an empty configuration object of a given format type. */
void *
config_new(const config_mgr_t *mgr)
{
  tor_assert(mgr->frozen);
  void *opts = tor_malloc_zero(mgr->toplevel->size);
  struct_set_magic(opts, &mgr->toplevel_magic);
  config_suite_t **suitep = config_mgr_get_suite_ptr(mgr, opts);
  if (suitep) {
    *suitep = config_suite_new();
    SMARTLIST_FOREACH_BEGIN(mgr->subconfigs, const config_format_t *, fmt) {
      void *obj = tor_malloc_zero(fmt->size);
      struct_set_magic(obj, &fmt->magic);
      smartlist_add((*suitep)->configs, obj);
    } SMARTLIST_FOREACH_END(fmt);
  }
  CONFIG_CHECK(mgr, opts);
  return opts;
}

/*
 * Functions to parse config options
 */

/** If <b>option</b> is an official abbreviation for a longer option,
 * return the longer option.  Otherwise return <b>option</b>.
 * If <b>command_line</b> is set, apply all abbreviations.  Otherwise, only
 * apply abbreviations that work for the config file and the command line.
 * If <b>warn_obsolete</b> is set, warn about deprecated names. */
const char *
config_expand_abbrev(const config_mgr_t *mgr, const char *option,
                     int command_line, int warn_obsolete)
{
  SMARTLIST_FOREACH_BEGIN(mgr->all_abbrevs, const config_abbrev_t *, abbrev) {
    /* Abbreviations are case insensitive. */
    if (!strcasecmp(option, abbrev->abbreviated) &&
        (command_line || !abbrev->commandline_only)) {
      if (warn_obsolete && abbrev->warn) {
        log_warn(LD_CONFIG,
                 "The configuration option '%s' is deprecated; "
                 "use '%s' instead.",
                 abbrev->abbreviated,
                 abbrev->full);
      }
      /* Keep going through the list in case we want to rewrite it more.
       * (We could imagine recursing here, but I don't want to get the
       * user into an infinite loop if we craft our list wrong.) */
      option = abbrev->full;
    }
  } SMARTLIST_FOREACH_END(abbrev);
  return option;
}

/** If <b>key</b> is a deprecated configuration option, return the message
 * explaining why it is deprecated (which may be an empty string). Return NULL
 * if it is not deprecated. The <b>key</b> field must be fully expanded. */
const char *
config_find_deprecation(const config_mgr_t *mgr, const char *key)
{
  if (BUG(mgr == NULL) || BUG(key == NULL))
    return NULL; // LCOV_EXCL_LINE

  SMARTLIST_FOREACH_BEGIN(mgr->all_deprecations, const config_deprecation_t *,
                          d) {
    if (!strcasecmp(d->name, key)) {
      return d->why_deprecated ? d->why_deprecated : "";
    }
  } SMARTLIST_FOREACH_END(d);
  return NULL;
}

/**
 * Find the managed_var_t object for a variable whose name is <b>name</b>
 * according to <b>mgr</b>.   Return that object, or NULL if none exists.
 *
 * If <b>allow_truncated</b> is true, then accept any variable whose
 * name begins with <b>name</b>.
 *
 * If <b>idx_out</b> is not NULL, set *<b>idx_out</b> to the position of
 * that variable within mgr-&gt;all_vars, or to -1 if the variable is
 * not found.
 */
static const managed_var_t *
config_mgr_find_var(const config_mgr_t *mgr,
                    const char *key,
                    bool allow_truncated, int *idx_out)
{
  const size_t keylen = strlen(key);
  if (idx_out)
    *idx_out = -1;

  if (!keylen)
    return NULL; /* if they say "--" on the command line, it's not an option */

  /* First, check for an exact (case-insensitive) match */
  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    if (!strcasecmp(mv->cvar->member.name, key)) {
      if (idx_out)
        *idx_out = mv_sl_idx;
      return mv;
    }
  } SMARTLIST_FOREACH_END(mv);

  if (!allow_truncated)
    return NULL;

  /* If none, check for an abbreviated match */
  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    if (!strncasecmp(key, mv->cvar->member.name, keylen)) {
      log_warn(LD_CONFIG, "The abbreviation '%s' is deprecated. "
               "Please use '%s' instead",
               key, mv->cvar->member.name);
      if (idx_out)
        *idx_out = mv_sl_idx;
      return mv;
    }
  } SMARTLIST_FOREACH_END(mv);

  /* Okay, unrecognized option */
  return NULL;
}

/**
 * If <b>key</b> is a name or an abbreviation configuration option, return
 * the corresponding canonical name for it.  Warn if the abbreviation is
 * non-standard.  Return NULL if the option does not exist.
 */
const char *
config_find_option_name(const config_mgr_t *mgr, const char *key)
{
  key = config_expand_abbrev(mgr, key, 0, 0);
  const managed_var_t *mv = config_mgr_find_var(mgr, key, true, NULL);
  if (mv)
    return mv->cvar->member.name;
  else
    return NULL;
}

/** Return the number of option entries in <b>fmt</b>. */
static int
config_count_options(const config_mgr_t *mgr)
{
  return smartlist_len(mgr->all_vars);
}

/**
 * Return true iff at least one bit from <b>flag</b> is set on <b>var</b>,
 * either in <b>var</b>'s flags, or on the flags of its type.
 **/
static bool
config_var_has_flag(const config_var_t *var, uint32_t flag)
{
  uint32_t have_flags = var->flags | struct_var_get_flags(&var->member);

  return (have_flags & flag) != 0;
}

/**
 * Return true if assigning a value to <b>var</b> replaces the previous
 * value.  Return false if assigning a value to <b>var</b> appends
 * to the previous value.
 **/
static bool
config_var_is_replaced_on_set(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NOREPLACE);
}

/**
 * Return true iff <b>var</b> may be assigned by name (e.g., via the
 * CLI, the configuration files, or the controller API).
 **/
bool
config_var_is_settable(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NOSET);
}

/**
 * Return true iff the controller is allowed to fetch the value of
 * <b>var</b>.
 **/
static bool
config_var_is_gettable(const config_var_t *var)
{
  /* Arguably, invisible or obsolete options should not be gettable.  However,
   * they have been gettable for a long time, and making them ungettable could
   * have compatibility effects.  For now, let's leave them alone.
   */

  // return ! config_var_has_flag(var, CVFLAG_OBSOLETE|CFGLAGS_INVISIBLE);
  (void)var;
  return true;
}

/**
 * Return true iff we need to check <b>var</b> for changes when we are
 * comparing config options for changes.
 *
 * A false result might mean that the variable is a derived variable, and that
 * comparing the variable it derives from compares this one too-- or it might
 * mean that there is no data to compare.
 **/
static bool
config_var_should_list_changes(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NOCMP);
}

/**
 * Return true iff we need to copy the data for <b>var</b> when we are
 * copying a config option.
 *
 * A false option might mean that the variable is a derived variable, and that
 * copying the variable it derives from copies it-- or it might mean that
 * there is no data to copy.
 **/
static bool
config_var_needs_copy(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NOCOPY);
}

/**
 * Return true iff variable <b>var</b> should appear on list of variable
 * names given to the controller or the CLI.
 *
 * (Note that this option is imperfectly obeyed. The
 * --list-torrc-options command looks at the "settable" flag, whereas
 * "GETINFO config/defaults" and "list_deprecated_*()" do not filter
 * their results. It would be good for consistency to try to converge
 * these behaviors in the future.)
 **/
bool
config_var_is_listable(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NOLIST);
}

/**
 * Return true iff variable <b>var</b> should be written out when we
 * are writing our configuration to disk, to a controller, or via the
 * --dump-config command.
 *
 * This option may be set because a variable is hidden, or because it is
 * derived from another variable which will already be written out.
 **/
static bool
config_var_is_dumpable(const config_var_t *var)
{
  return ! config_var_has_flag(var, CFLG_NODUMP);
}

/*
 * Functions to assign config options.
 */

/** <b>c</b>-\>key is known to be a real key. Update <b>options</b>
 * with <b>c</b>-\>value and return 0, or return -1 if bad value.
 *
 * Called from config_assign_line() and option_reset().
 */
static int
config_assign_value(const config_mgr_t *mgr, void *options,
                    config_line_t *c, char **msg)
{
  const managed_var_t *var;

  CONFIG_CHECK(mgr, options);

  var = config_mgr_find_var(mgr, c->key, true, NULL);
  tor_assert(var);
  tor_assert(!strcmp(c->key, var->cvar->member.name));
  void *object = config_mgr_get_obj_mutable(mgr, options, var->object_idx);

  if (config_var_has_flag(var->cvar, CFLG_WARN_OBSOLETE)) {
    log_warn(LD_GENERAL, "Skipping obsolete configuration option \"%s\".",
             var->cvar->member.name);
  } else if (config_var_has_flag(var->cvar, CFLG_WARN_DISABLED)) {
    log_warn(LD_GENERAL, "This copy of Tor was built without support for "
             "the option \"%s\". Skipping.", var->cvar->member.name);
  }

  return struct_var_kvassign(object, c, msg, &var->cvar->member);
}

/** Mark every linelist in <b>options</b> "fragile", so that fresh assignments
 * to it will replace old ones. */
static void
config_mark_lists_fragile(const config_mgr_t *mgr, void *options)
{
  tor_assert(mgr);
  tor_assert(options);

  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    void *object = config_mgr_get_obj_mutable(mgr, options, mv->object_idx);
    struct_var_mark_fragile(object, &mv->cvar->member);
  } SMARTLIST_FOREACH_END(mv);
}

/**
 * Log a warning that declaring that the option called <b>what</b>
 * is deprecated because of the reason in <b>why</b>.
 *
 * (Both arguments must be non-NULL.)
 **/
void
warn_deprecated_option(const char *what, const char *why)
{
  const char *space = (why && strlen(why)) ? " " : "";
  log_warn(LD_CONFIG, "The %s option is deprecated, and will most likely "
           "be removed in a future version of Tor.%s%s (If you think this is "
           "a mistake, please let us know!)",
           what, space, why);
}

/** If <b>c</b> is a syntactically valid configuration line, update
 * <b>options</b> with its value and return 0.  Otherwise return -1 for bad
 * key, -2 for bad value.
 *
 * If <b>clear_first</b> is set, clear the value first. Then if
 * <b>use_defaults</b> is set, set the value to the default.
 *
 * Called from config_assign().
 */
static int
config_assign_line(const config_mgr_t *mgr, void *options,
                   config_line_t *c, unsigned flags,
                   bitarray_t *options_seen, char **msg)
{
  const unsigned use_defaults = flags & CAL_USE_DEFAULTS;
  const unsigned clear_first = flags & CAL_CLEAR_FIRST;
  const unsigned warn_deprecations = flags & CAL_WARN_DEPRECATIONS;
  const managed_var_t *mvar;

  CONFIG_CHECK(mgr, options);

  int var_index = -1;
  mvar = config_mgr_find_var(mgr, c->key, true, &var_index);
  if (!mvar) {
    const config_format_t *fmt = mgr->toplevel;
    if (fmt->extra) {
      void *lvalue = STRUCT_VAR_P(options, fmt->extra->offset);
      log_info(LD_CONFIG,
               "Found unrecognized option '%s'; saving it.", c->key);
      config_line_append((config_line_t**)lvalue, c->key, c->value);
      return 0;
    } else {
      tor_asprintf(msg,
                "Unknown option '%s'.  Failing.", c->key);
      return -1;
    }
  }

  const config_var_t *cvar = mvar->cvar;
  tor_assert(cvar);

  /* Put keyword into canonical case. */
  if (strcmp(cvar->member.name, c->key)) {
    tor_free(c->key);
    c->key = tor_strdup(cvar->member.name);
  }

  const char *deprecation_msg;
  if (warn_deprecations &&
      (deprecation_msg = config_find_deprecation(mgr, cvar->member.name))) {
    warn_deprecated_option(cvar->member.name, deprecation_msg);
  }

  if (!strlen(c->value)) {
    /* reset or clear it, then return */
    if (!clear_first) {
      if (! config_var_is_replaced_on_set(cvar) &&
          c->command != CONFIG_LINE_CLEAR) {
        /* We got an empty linelist from the torrc or command line.
           As a special case, call this an error. Warn and ignore. */
        log_warn(LD_CONFIG,
                 "Linelist option '%s' has no value. Skipping.", c->key);
      } else { /* not already cleared */
        config_reset(mgr, options, mvar, use_defaults);
      }
    }
    return 0;
  } else if (c->command == CONFIG_LINE_CLEAR && !clear_first) {
    // This block is unreachable, since a CLEAR line always has an
    // empty value, and so will trigger be handled by the previous
    // "if (!strlen(c->value))" block.

    // LCOV_EXCL_START
    tor_assert_nonfatal_unreached();
    config_reset(mgr, options, mvar, use_defaults);
    // LCOV_EXCL_STOP
  }

  if (options_seen && config_var_is_replaced_on_set(cvar)) {
    /* We're tracking which options we've seen, and this option is not
     * supposed to occur more than once. */
    tor_assert(var_index >= 0);
    if (bitarray_is_set(options_seen, var_index)) {
      log_warn(LD_CONFIG, "Option '%s' used more than once; all but the last "
               "value will be ignored.", cvar->member.name);
    }
    bitarray_set(options_seen, var_index);
  }

  if (config_assign_value(mgr, options, c, msg) < 0)
    return -2;
  return 0;
}

/** Restore the option named <b>key</b> in options to its default value.
 * Called from config_assign(). */
STATIC void
config_reset_line(const config_mgr_t *mgr, void *options,
                  const char *key, int use_defaults)
{
  const managed_var_t *var;

  CONFIG_CHECK(mgr, options);

  var = config_mgr_find_var(mgr, key, true, NULL);
  if (!var)
    return; /* give error on next pass. */

  config_reset(mgr, options, var, use_defaults);
}

/** Return true iff value needs to be quoted and escaped to be used in
 * a configuration file. */
static int
config_value_needs_escape(const char *value)
{
  if (*value == '\"')
    return 1;
  while (*value) {
    switch (*value)
    {
    case '\r':
    case '\n':
    case '#':
      /* Note: quotes and backspaces need special handling when we are using
       * quotes, not otherwise, so they don't trigger escaping on their
       * own. */
      return 1;
    default:
      if (!TOR_ISPRINT(*value))
        return 1;
    }
    ++value;
  }
  return 0;
}

/** Return newly allocated line or lines corresponding to <b>key</b> in the
 * configuration <b>options</b>.  If <b>escape_val</b> is true and a
 * value needs to be quoted before it's put in a config file, quote and
 * escape that value. Return NULL if no such key exists. */
config_line_t *
config_get_assigned_option(const config_mgr_t *mgr, const void *options,
                           const char *key, int escape_val)
{
  const managed_var_t *var;
  config_line_t *result;

  tor_assert(options && key);

  CONFIG_CHECK(mgr, options);

  var = config_mgr_find_var(mgr, key, true, NULL);
  if (!var) {
    log_warn(LD_CONFIG, "Unknown option '%s'.  Failing.", key);
    return NULL;
  }
  if (! config_var_is_gettable(var->cvar)) {
    log_warn(LD_CONFIG, "Option '%s' is obsolete or unfetchable. Failing.",
             key);
    return NULL;
  }
  const void *object = config_mgr_get_obj(mgr, options, var->object_idx);

  result = struct_var_kvencode(object, &var->cvar->member);

  if (escape_val) {
    config_line_t *line;
    for (line = result; line; line = line->next) {
      if (line->value && config_value_needs_escape(line->value)) {
        char *newval = esc_for_log(line->value);
        tor_free(line->value);
        line->value = newval;
      }
    }
  }

  return result;
}
/** Iterate through the linked list of requested options <b>list</b>.
 * For each item, convert as appropriate and assign to <b>options</b>.
 * If an item is unrecognized, set *msg and return -1 immediately,
 * else return 0 for success.
 *
 * If <b>clear_first</b>, interpret config options as replacing (not
 * extending) their previous values. If <b>clear_first</b> is set,
 * then <b>use_defaults</b> to decide if you set to defaults after
 * clearing, or make the value 0 or NULL.
 *
 * Here are the use cases:
 * 1. A non-empty AllowInvalid line in your torrc. Appends to current
 *    if linelist, replaces current if csv.
 * 2. An empty AllowInvalid line in your torrc. Should clear it.
 * 3. "RESETCONF AllowInvalid" sets it to default.
 * 4. "SETCONF AllowInvalid" makes it NULL.
 * 5. "SETCONF AllowInvalid=foo" clears it and sets it to "foo".
 *
 * Use_defaults   Clear_first
 *    0                0       "append"
 *    1                0       undefined, don't use
 *    0                1       "set to null first"
 *    1                1       "set to defaults first"
 * Return 0 on success, -1 on bad key, -2 on bad value.
 *
 * As an additional special case, if a LINELIST config option has
 * no value and clear_first is 0, then warn and ignore it.
 */

/*
There are three call cases for config_assign() currently.

Case one: Torrc entry
options_init_from_torrc() calls config_assign(0, 0)
  calls config_assign_line(0, 0).
    if value is empty, calls config_reset(0) and returns.
    calls config_assign_value(), appends.

Case two: setconf
options_trial_assign() calls config_assign(0, 1)
  calls config_reset_line(0)
    calls config_reset(0)
      calls option_clear().
  calls config_assign_line(0, 1).
    if value is empty, returns.
    calls config_assign_value(), appends.

Case three: resetconf
options_trial_assign() calls config_assign(1, 1)
  calls config_reset_line(1)
    calls config_reset(1)
      calls option_clear().
      calls config_assign_value(default)
  calls config_assign_line(1, 1).
    returns.
*/
int
config_assign(const config_mgr_t *mgr, void *options, config_line_t *list,
              unsigned config_assign_flags, char **msg)
{
  config_line_t *p;
  bitarray_t *options_seen;
  const int n_options = config_count_options(mgr);
  const unsigned clear_first = config_assign_flags & CAL_CLEAR_FIRST;
  const unsigned use_defaults = config_assign_flags & CAL_USE_DEFAULTS;

  CONFIG_CHECK(mgr, options);

  /* pass 1: normalize keys */
  for (p = list; p; p = p->next) {
    const char *full = config_expand_abbrev(mgr, p->key, 0, 1);
    if (strcmp(full,p->key)) {
      tor_free(p->key);
      p->key = tor_strdup(full);
    }
  }

  /* pass 2: if we're reading from a resetting source, clear all
   * mentioned config options, and maybe set to their defaults. */
  if (clear_first) {
    for (p = list; p; p = p->next)
      config_reset_line(mgr, options, p->key, use_defaults);
  }

  options_seen = bitarray_init_zero(n_options);
  /* pass 3: assign. */
  while (list) {
    int r;
    if ((r=config_assign_line(mgr, options, list, config_assign_flags,
                              options_seen, msg))) {
      bitarray_free(options_seen);
      return r;
    }
    list = list->next;
  }
  bitarray_free(options_seen);

  /** Now we're done assigning a group of options to the configuration.
   * Subsequent group assignments should _replace_ linelists, not extend
   * them. */
  config_mark_lists_fragile(mgr, options);

  return 0;
}

/** Reset config option <b>var</b> to 0, 0.0, NULL, or the equivalent.
 * Called from config_reset() and config_free(). */
static void
config_clear(const config_mgr_t *mgr, void *options, const managed_var_t *var)
{
  void *object = config_mgr_get_obj_mutable(mgr, options, var->object_idx);
  struct_var_free(object, &var->cvar->member);
}

/** Clear the option indexed by <b>var</b> in <b>options</b>. Then if
 * <b>use_defaults</b>, set it to its default value.
 * Called by config_init() and option_reset_line() and option_assign_line(). */
static void
config_reset(const config_mgr_t *mgr, void *options,
             const managed_var_t *var, int use_defaults)
{
  config_line_t *c;
  char *msg = NULL;
  CONFIG_CHECK(mgr, options);
  config_clear(mgr, options, var); /* clear it first */

  if (!use_defaults)
    return; /* all done */

  if (var->cvar->initvalue) {
    c = tor_malloc_zero(sizeof(config_line_t));
    c->key = tor_strdup(var->cvar->member.name);
    c->value = tor_strdup(var->cvar->initvalue);
    if (config_assign_value(mgr, options, c, &msg) < 0) {
      // LCOV_EXCL_START
      log_warn(LD_BUG, "Failed to assign default: %s", msg);
      tor_free(msg); /* if this happens it's a bug */
      // LCOV_EXCL_STOP
    }
    config_free_lines(c);
  }
}

/** Release storage held by <b>options</b>. */
void
config_free_(const config_mgr_t *mgr, void *options)
{
  if (!options)
    return;

  tor_assert(mgr);

  if (mgr->toplevel->clear_fn) {
    mgr->toplevel->clear_fn(mgr, options);
  }
  config_suite_t **suitep = config_mgr_get_suite_ptr(mgr, options);
  if (suitep) {
    tor_assert(smartlist_len((*suitep)->configs) ==
               smartlist_len(mgr->subconfigs));
    SMARTLIST_FOREACH_BEGIN(mgr->subconfigs, const config_format_t *, fmt) {
      void *obj = smartlist_get((*suitep)->configs, fmt_sl_idx);
      if (fmt->clear_fn) {
        fmt->clear_fn(mgr, obj);
      }
    } SMARTLIST_FOREACH_END(fmt);
  }

  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    config_clear(mgr, options, mv);
  } SMARTLIST_FOREACH_END(mv);

  if (mgr->toplevel->extra) {
    config_line_t **linep = STRUCT_VAR_P(options,
                                         mgr->toplevel->extra->offset);
    config_free_lines(*linep);
    *linep = NULL;
  }

  if (suitep) {
    SMARTLIST_FOREACH((*suitep)->configs, void *, obj, tor_free(obj));
    config_suite_free(*suitep);
  }

  tor_free(options);
}

/** Return true iff the option <b>name</b> has the same value in <b>o1</b>
 * and <b>o2</b>.  Must not be called for LINELIST_S or OBSOLETE options.
 */
int
config_is_same(const config_mgr_t *mgr,
               const void *o1, const void *o2,
               const char *name)
{
  CONFIG_CHECK(mgr, o1);
  CONFIG_CHECK(mgr, o2);

  const managed_var_t *var = config_mgr_find_var(mgr, name, true, NULL);
  if (!var) {
    return true;
  }
  const void *obj1 = config_mgr_get_obj(mgr, o1, var->object_idx);
  const void *obj2 = config_mgr_get_obj(mgr, o2, var->object_idx);

  return struct_var_eq(obj1, obj2, &var->cvar->member);
}

/**
 * Return a list of the options which have changed between <b>options1</b> and
 * <b>options2</b>. If an option has reverted to its default value, it has a
 * value entry of NULL.
 *
 * <b>options1</b> and <b>options2</b> must be top-level configuration objects
 * of the type managed by <b>mgr</b>.
 **/
config_line_t *
config_get_changes(const config_mgr_t *mgr,
                   const void *options1, const void *options2)
{
  config_line_t *result = NULL;
  config_line_t **next = &result;
  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, managed_var_t *, mv) {
    if (! config_var_should_list_changes(mv->cvar)) {
      /* something else will check this var, or it doesn't need checking */
      continue;
    }
    const void *obj1 = config_mgr_get_obj(mgr, options1, mv->object_idx);
    const void *obj2 = config_mgr_get_obj(mgr, options2, mv->object_idx);

    if (struct_var_eq(obj1, obj2, &mv->cvar->member)) {
      continue;
    }

    const char *varname = mv->cvar->member.name;
    config_line_t *line =
      config_get_assigned_option(mgr, options2, varname, 1);

    if (line) {
      *next = line;
    } else {
      *next = tor_malloc_zero(sizeof(config_line_t));
      (*next)->key = tor_strdup(varname);
    }
    while (*next)
      next = &(*next)->next;
  } SMARTLIST_FOREACH_END(mv);

  return result;
}

/** Copy storage held by <b>old</b> into a new or_options_t and return it. */
void *
config_dup(const config_mgr_t *mgr, const void *old)
{
  void *newopts;

  newopts = config_new(mgr);
  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, managed_var_t *, mv) {
    if (! config_var_needs_copy(mv->cvar)) {
      // Something else will copy this option, or it doesn't need copying.
      continue;
    }
    const void *oldobj = config_mgr_get_obj(mgr, old, mv->object_idx);
    void *newobj = config_mgr_get_obj_mutable(mgr, newopts, mv->object_idx);
    if (struct_var_copy(newobj, oldobj, &mv->cvar->member) < 0) {
      // LCOV_EXCL_START
      log_err(LD_BUG, "Unable to copy value for %s.",
              mv->cvar->member.name);
      tor_assert_unreached();
      // LCOV_EXCL_STOP
    }
  } SMARTLIST_FOREACH_END(mv);

  return newopts;
}
/** Set all vars in the configuration object <b>options</b> to their default
 * values. */
void
config_init(const config_mgr_t *mgr, void *options)
{
  CONFIG_CHECK(mgr, options);

  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    if (!mv->cvar->initvalue)
      continue; /* defaults to NULL or 0 */
    config_reset(mgr, options, mv, 1);
  } SMARTLIST_FOREACH_END(mv);
}

/**
 * Helper for config_validate_single: see whether any immutable option
 * has changed between old_options and new_options.
 *
 * On success return 0; on failure set *msg_out to a newly allocated
 * string explaining what is wrong, and return -1.
 */
static int
config_check_immutable_flags(const config_format_t *fmt,
                             const void *old_options,
                             const void *new_options,
                             char **msg_out)
{
  tor_assert(fmt);
  tor_assert(new_options);
  if (BUG(! old_options))
    return 0;

  unsigned i;
  for (i = 0; fmt->vars[i].member.name; ++i) {
    const config_var_t *v = &fmt->vars[i];
    if (! config_var_has_flag(v, CFLG_IMMUTABLE))
      continue;

    if (! struct_var_eq(old_options, new_options, &v->member)) {
      tor_asprintf(msg_out,
                   "While Tor is running, changing %s is not allowed",
                   v->member.name);
      return -1;
    }
  }

  return 0;
}

/**
 * Normalize and validate a single object `options` within a configuration
 * suite, according to its format.  `options` may be modified as appropriate
 * in order to set ancillary data.  If `old_options` is provided, make sure
 * that the transition from `old_options` to `options` is permitted.
 *
 * On success return VSTAT_OK; on failure set *msg_out to a newly allocated
 * string explaining what is wrong, and return a different validation_status_t
 * to describe which step failed.
 **/
static validation_status_t
config_validate_single(const config_format_t *fmt,
                       const void *old_options, void *options,
                       char **msg_out)
{
  tor_assert(fmt);
  tor_assert(options);

  if (fmt->pre_normalize_fn) {
    if (fmt->pre_normalize_fn(options, msg_out) < 0) {
      return VSTAT_PRE_NORMALIZE_ERR;
    }
  }

  if (fmt->legacy_validate_fn) {
    if (fmt->legacy_validate_fn(old_options, options, msg_out) < 0) {
      return VSTAT_LEGACY_ERR;
    }
  }

  if (fmt->validate_fn) {
    if (fmt->validate_fn(options, msg_out) < 0) {
      return VSTAT_VALIDATE_ERR;
    }
  }

  if (old_options) {
    if (config_check_immutable_flags(fmt, old_options, options, msg_out) < 0) {
      return VSTAT_TRANSITION_ERR;
    }

    if (fmt->check_transition_fn) {
      if (fmt->check_transition_fn(old_options, options, msg_out) < 0) {
        return VSTAT_TRANSITION_ERR;
      }
    }
  }

  if (fmt->post_normalize_fn) {
    if (fmt->post_normalize_fn(options, msg_out) < 0) {
      return VSTAT_POST_NORMALIZE_ERR;
    }
  }

  return VSTAT_OK;
}

/**
 * Normalize and validate all the options in configuration object `options`
 * and its sub-objects. `options` may be modified as appropriate in order to
 * set ancillary data.  If `old_options` is provided, make sure that the
 * transition from `old_options` to `options` is permitted.
 *
 * On success return VSTAT_OK; on failure set *msg_out to a newly allocated
 * string explaining what is wrong, and return a different validation_status_t
 * to describe which step failed.
 **/
validation_status_t
config_validate(const config_mgr_t *mgr,
                const void *old_options, void *options,
                char **msg_out)
{
  validation_status_t rv;
  CONFIG_CHECK(mgr, options);
  if (old_options) {
    CONFIG_CHECK(mgr, old_options);
  }

  config_suite_t **suitep_new = config_mgr_get_suite_ptr(mgr, options);
  config_suite_t **suitep_old = NULL;
  if (old_options)
    suitep_old = config_mgr_get_suite_ptr(mgr, (void*) old_options);

  /* Validate the sub-objects */
  if (suitep_new) {
    SMARTLIST_FOREACH_BEGIN(mgr->subconfigs, const config_format_t *, fmt) {
      void *obj = smartlist_get((*suitep_new)->configs, fmt_sl_idx);
      const void *obj_old=NULL;
      if (suitep_old)
        obj_old = smartlist_get((*suitep_old)->configs, fmt_sl_idx);

      rv = config_validate_single(fmt, obj_old, obj, msg_out);
      if (rv < 0)
        return rv;
    } SMARTLIST_FOREACH_END(fmt);
  }

  /* Validate the top-level object. */
  rv = config_validate_single(mgr->toplevel, old_options, options, msg_out);
  if (rv < 0)
    return rv;

  return VSTAT_OK;
}

/** Allocate and return a new string holding the written-out values of the vars
 * in 'options'.  If 'minimal', do not write out any default-valued vars.
 * Else, if comment_defaults, write default values as comments.
 */
char *
config_dump(const config_mgr_t *mgr, const void *default_options,
            const void *options, int minimal,
            int comment_defaults)
{
  const config_format_t *fmt = mgr->toplevel;
  smartlist_t *elements;
  const void *defaults = default_options;
  void *defaults_tmp = NULL;
  config_line_t *line, *assigned;
  char *result;
  char *msg = NULL;

  if (defaults == NULL) {
    defaults = defaults_tmp = config_new(mgr);
    config_init(mgr, defaults_tmp);
  }

  /* XXX use a 1 here so we don't add a new log line while dumping */
  if (default_options == NULL) {
    if (config_validate(mgr, NULL, defaults_tmp, &msg) < 0) {
      // LCOV_EXCL_START
      log_err(LD_BUG, "Failed to validate default config: %s", msg);
      tor_free(msg);
      tor_assert(0);
      // LCOV_EXCL_STOP
    }
  }

  elements = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, managed_var_t *, mv) {
    int comment_option = 0;
    /* Don't save 'hidden' control variables. */
    if (! config_var_is_dumpable(mv->cvar))
      continue;
    const char *name = mv->cvar->member.name;
    if (minimal && config_is_same(mgr, options, defaults, name))
      continue;
    else if (comment_defaults &&
             config_is_same(mgr, options, defaults, name))
      comment_option = 1;

    line = assigned =
      config_get_assigned_option(mgr, options, name, 1);

    for (; line; line = line->next) {
      if (!strcmpstart(line->key, "__")) {
        /* This check detects "hidden" variables inside LINELIST_V structures.
         */
        continue;
      }
      int value_exists = line->value && *(line->value);
      smartlist_add_asprintf(elements, "%s%s%s%s\n",
                   comment_option ? "# " : "",
                   line->key, value_exists ? " " : "", line->value);
    }
    config_free_lines(assigned);
  } SMARTLIST_FOREACH_END(mv);

  if (fmt->extra) {
    line = *(config_line_t**)STRUCT_VAR_P(options, fmt->extra->offset);
    for (; line; line = line->next) {
      int value_exists = line->value && *(line->value);
      smartlist_add_asprintf(elements, "%s%s%s\n",
                             line->key, value_exists ? " " : "", line->value);
    }
  }

  result = smartlist_join_strings(elements, "", 0, NULL);
  SMARTLIST_FOREACH(elements, char *, cp, tor_free(cp));
  smartlist_free(elements);
  config_free(mgr, defaults_tmp);
  return result;
}

/**
 * Return true if every member of <b>options</b> is in-range and well-formed.
 * Return false otherwise.  Log errors at level <b>severity</b>.
 */
bool
config_check_ok(const config_mgr_t *mgr, const void *options, int severity)
{
  bool all_ok = true;

  SMARTLIST_FOREACH_BEGIN(mgr->all_vars, const managed_var_t *, mv) {
    if (!struct_var_ok(options, &mv->cvar->member)) {
      log_fn(severity, LD_BUG, "Invalid value for %s",
             mv->cvar->member.name);
      all_ok = false;
    }
  } SMARTLIST_FOREACH_END(mv);

  return all_ok;
}
