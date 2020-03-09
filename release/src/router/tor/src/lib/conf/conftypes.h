/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file conftypes.h
 * @brief Types used to specify configurable options.
 *
 * This header defines the types that different modules will use in order to
 * declare their configuration and state variables, and tell the configuration
 * management code about those variables.  From the individual module's point
 * of view, its configuration and state are simply data structures.
 *
 * For defining new variable types, see var_type_def_st.h.
 *
 * For the code that manipulates variables defined via this module, see
 * lib/confmgt/, especially typedvar.h and (later) structvar.h.  The
 * configuration manager is responsible for encoding, decoding, and
 * maintaining the configuration structures used by the various modules.
 *
 * STATUS NOTE: This is a work in process refactoring.  It is not yet possible
 *   for modules to define their own variables, and much of the configuration
 *   management code is still in src/app/config/.
 **/

#ifndef TOR_SRC_LIB_CONF_CONFTYPES_H
#define TOR_SRC_LIB_CONF_CONFTYPES_H

#include "lib/cc/torint.h"
#ifdef TOR_UNIT_TESTS
#include "lib/conf/conftesting.h"
#endif

#include <stddef.h>

/** Enumeration of types which option values can take */
typedef enum config_type_t {
  CONFIG_TYPE_STRING = 0,   /**< An arbitrary string. */
  CONFIG_TYPE_FILENAME,     /**< A filename: some prefixes get expanded. */
  CONFIG_TYPE_POSINT,       /**< A non-negative integer less than MAX_INT */
  CONFIG_TYPE_INT,          /**< Any integer. */
  CONFIG_TYPE_UINT64,       /**< A value in range 0..UINT64_MAX */
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
  CONFIG_TYPE_OBSOLETE,     /**< Obsolete (ignored) option. */
  /**
   * Extended type: definition appears in the <b>type_def</b> pointer
   * of the corresponding struct_member_t.
   *
   * For some types, we cannot define them as particular values of this
   * enumeration, since those types are abstractions defined at a higher level
   * than this module.  (For example, parsing a routerset_t is higher-level
   * than this module.)  To handle this, we use CONFIG_TYPE_EXTENDED for those
   * types, and give a definition for them in the struct_member_t.type_def.
   **/
  CONFIG_TYPE_EXTENDED,
} config_type_t;

/* Forward delcaration for var_type_def_t, for extended types. */
struct var_type_def_t;

/** Structure to specify a named, typed member within a structure. */
typedef struct struct_member_t {
  /** Name of the field. */
  const char *name;
  /**
   * Type of the field, according to the config_type_t enumeration.
   *
   * For any type not otherwise listed in config_type_t, this field's value
   * should be CONFIG_TYPE_EXTENDED.  When it is, the <b>type_def</b> pointer
   * must be set.
   **/
  /*
   * NOTE: In future refactoring, we might remove this field entirely, along
   * with its corresponding enumeration.  In that case, we will require that
   * type_def be set in all cases. If we do, we will also need a new mechanism
   * to enforce consistency between configuration variable types and their
   * corresponding structures, since our current design in
   * lib/conf/conftesting.h won't work any more.
   */
  config_type_t type;
  /**
   * Pointer to a type definition for the type of this field. Overrides
   * <b>type</b> if it is not NULL.  Must be set when <b>type</b> is
   * CONFIG_TYPE_EXTENDED.
   **/
  const struct var_type_def_t *type_def;
  /**
   * Offset of this field within the structure.  Compute this with
   * offsetof(structure, fieldname).
   **/
  ptrdiff_t offset;
} struct_member_t;

/**
 * Structure to describe the location and preferred value of a "magic number"
 * field within a structure.
 *
 * These 'magic numbers' are 32-bit values used to tag objects to make sure
 * that they have the correct type.
 */
typedef struct struct_magic_decl_t {
  /** The name of the structure */
  const char *typename;
  /** A value used to recognize instances of this structure. */
  uint32_t magic_val;
  /** The location within the structure at which we expect to find
   * <b>magic_val</b>. */
  ptrdiff_t magic_offset;
} struct_magic_decl_t;

/**
 * Flag to indicate that an option or type is "undumpable". An
 * undumpable option is never saved to disk.
 *
 * For historical reasons its name is usually is prefixed with __.
 **/
#define CFLG_NODUMP    (1u<<0)
/**
 * Flag to indicate that an option or type is "unlisted".
 *
 * We don't tell the controller about unlisted options when it asks for a
 * list of them.
 **/
#define CFLG_NOLIST (1u<<1)
/**
 * Flag to indicate that an option or type is "unsettable".
 *
 * An unsettable option can never be set directly by name.
 **/
#define CFLG_NOSET (1u<<2)
/**
 * Flag to indicate that an option or type does not need to be copied when
 * copying the structure that contains it.
 *
 * (Usually, if an option does not need to be copied, then either it contains
 * no data, or the data that it does contain is completely contained within
 * another option.)
 **/
#define CFLG_NOCOPY (1u<<3)
/**
 * Flag to indicate that an option or type does not need to be compared
 * when telling the controller about the differences between two
 * configurations.
 *
 * (Usually, if an option does not need to be compared, then either it
 * contains no data, or the data that it does contain is completely contained
 * within another option.)
 **/
#define CFLG_NOCMP (1u<<4)
/**
 * Flag to indicate that an option or type should not be replaced when setting
 * it.
 *
 * For most options, setting them replaces their old value.  For some options,
 * however, setting them appends to their old value.
 */
#define CFLG_NOREPLACE    (1u<<5)

/**
 * A group of flags that should be set on all obsolete options and types.
 **/
#define CFLG_GROUP_OBSOLETE \
  (CFLG_NOCOPY|CFLG_NOCMP|CFLG_NODUMP|CFLG_NOSET|CFLG_NOLIST)

/** A variable allowed in the configuration file or on the command line. */
typedef struct config_var_t {
  struct_member_t member; /** A struct member corresponding to this
                           * variable. */
  const char *initvalue; /**< String (or null) describing initial value. */
  uint32_t flags; /**< One or more flags describing special handling for this
                   * variable */
#ifdef TOR_UNIT_TESTS
  /** Used for compiler-magic to typecheck the corresponding field in the
   * corresponding struct. Only used in unit test mode, at compile-time. */
  confparse_dummy_values_t var_ptr_dummy;
#endif
} config_var_t;

#endif /* !defined(TOR_SRC_LIB_CONF_CONFTYPES_H) */
