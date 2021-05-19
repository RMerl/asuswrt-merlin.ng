/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file type_defs.c
 * @brief Definitions for various low-level configuration types.
 *
 * This module creates a number of var_type_def_t objects, to be used by
 * typedvar.c in manipulating variables.
 *
 * The types here are common types that can be implemented with Tor's
 * low-level functionality.  To define new types, see var_type_def_st.h.
 **/

#include "orconfig.h"
#include "lib/conf/conftypes.h"
#include "lib/conf/confdecl.h"
#include "lib/confmgt/typedvar.h"
#include "lib/confmgt/type_defs.h"
#include "lib/confmgt/unitparse.h"

#include "lib/cc/compat_compiler.h"
#include "lib/container/smartlist.h"
#include "lib/encoding/confline.h"
#include "lib/encoding/time_fmt.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/parse_int.h"
#include "lib/string/printf.h"

#include "lib/confmgt/var_type_def_st.h"

#include <stddef.h>
#include <string.h>
#include <errno.h>

//////
// CONFIG_TYPE_STRING
// CONFIG_TYPE_FILENAME
//
// These two types are the same for now, but they have different names.
//
// Warning: For this type, the default value (NULL) and "" are considered
// different values.  That is generally risky, and best avoided for other
// types in the future.
//////

static int
string_parse(void *target, const char *value, char **errmsg,
             const void *params)
{
  (void)params;
  (void)errmsg;
  char **p = (char**)target;
  *p = tor_strdup(value);
  return 0;
}

static char *
string_encode(const void *value, const void *params)
{
  (void)params;
  const char **p = (const char**)value;
  return *p ? tor_strdup(*p) : NULL;
}

static void
string_clear(void *value, const void *params)
{
  (void)params;
  char **p = (char**)value;
  tor_free(*p); // sets *p to NULL.
}

static const var_type_fns_t string_fns = {
  .parse = string_parse,
  .encode = string_encode,
  .clear = string_clear,
};

/////
// CONFIG_TYPE_INT
// CONFIG_TYPE_POSINT
//
// These types are implemented as int, possibly with a restricted range.
/////

/**
 * Parameters for parsing an integer type.
 **/
typedef struct int_type_params_t {
  int minval; /**< Lowest allowed value */
  int maxval; /**< Highest allowed value */
} int_parse_params_t;

static const int_parse_params_t INT_PARSE_UNRESTRICTED = {
  .minval = INT_MIN,
  .maxval = INT_MAX,
};

static const int_parse_params_t INT_PARSE_POSINT = {
  .minval = 0,
  .maxval = INT_MAX,
};

static int
int_parse(void *target, const char *value, char **errmsg, const void *params)
{
  const int_parse_params_t *pp;
  if (params) {
    pp = params;
  } else {
    pp = &INT_PARSE_UNRESTRICTED;
  }
  int *p = target;
  int ok=0;
  *p = (int)tor_parse_long(value, 10, pp->minval, pp->maxval, &ok, NULL);
  if (!ok) {
    tor_asprintf(errmsg, "Integer %s is malformed or out of bounds. "
                 "Allowed values are between %d and %d.",
                 value, pp->minval, pp->maxval);
    return -1;
  }
  return 0;
}

static char *
int_encode(const void *value, const void *params)
{
  (void)params;
  int v = *(int*)value;
  char *result;
  tor_asprintf(&result, "%d", v);
  return result;
}

static void
int_clear(void *value, const void *params)
{
  (void)params;
  *(int*)value = 0;
}

static bool
int_ok(const void *value, const void *params)
{
  const int_parse_params_t *pp = params;
  if (pp) {
    int v = *(int*)value;
    return pp->minval <= v && v <= pp->maxval;
  } else {
    return true;
  }
}

static const var_type_fns_t int_fns = {
  .parse = int_parse,
  .encode = int_encode,
  .clear = int_clear,
  .ok = int_ok,
};

/////
// CONFIG_TYPE_UINT64
//
// This type is an unrestricted u64.
/////

static int
uint64_parse(void *target, const char *value, char **errmsg,
             const void *params)
{
  (void)params;
  (void)errmsg;
  uint64_t *p = target;
  int ok=0;
  *p = tor_parse_uint64(value, 10, 0, UINT64_MAX, &ok, NULL);
  if (!ok) {
    tor_asprintf(errmsg, "Integer %s is malformed or out of bounds.",
                 value);
    return -1;
  }
  return 0;
}

static char *
uint64_encode(const void *value, const void *params)
{
  (void)params;
  uint64_t v = *(uint64_t*)value;
  char *result;
  tor_asprintf(&result, "%"PRIu64, v);
  return result;
}

static void
uint64_clear(void *value, const void *params)
{
  (void)params;
  *(uint64_t*)value = 0;
}

static const var_type_fns_t uint64_fns = {
  .parse = uint64_parse,
  .encode = uint64_encode,
  .clear = uint64_clear,
};

/////
// CONFIG_TYPE_INTERVAL
// CONFIG_TYPE_MSEC_INTERVAL
// CONFIG_TYPE_MEMUNIT
//
// These types are implemented using the config_parse_units() function.
// The intervals are stored as ints, whereas memory units are stored as
// uint64_ts.
/////

static int
units_parse_u64(void *target, const char *value, char **errmsg,
                const void *params)
{
  const unit_table_t *table = params;
  tor_assert(table);
  uint64_t *v = (uint64_t*)target;
  int ok=1;
  char *msg = NULL;
  *v = config_parse_units(value, table, &ok, &msg);
  if (!ok) {
    tor_asprintf(errmsg, "Provided value is malformed or out of bounds: %s",
                 msg);
    tor_free(msg);
    return -1;
  }
  if (BUG(msg)) {
    tor_free(msg);
  }
  return 0;
}

static int
units_parse_int(void *target, const char *value, char **errmsg,
               const void *params)
{
  const unit_table_t *table = params;
  tor_assert(table);
  int *v = (int*)target;
  int ok=1;
  char *msg = NULL;
  uint64_t u64 = config_parse_units(value, table, &ok, &msg);
  if (!ok) {
    tor_asprintf(errmsg, "Provided value is malformed or out of bounds: %s",
                 msg);
    tor_free(msg);
    return -1;
  }
  if (BUG(msg)) {
    tor_free(msg);
  }
  if (u64 > INT_MAX) {
    tor_asprintf(errmsg, "Provided value %s is too large", value);
    return -1;
  }
  *v = (int) u64;
  return 0;
}

static bool
units_ok_int(const void *value, const void *params)
{
  (void)params;
  int v = *(int*)value;
  return v >= 0;
}

static const var_type_fns_t memunit_fns = {
  .parse = units_parse_u64,
  .encode = uint64_encode, // doesn't use params
  .clear = uint64_clear, // doesn't use params
};

static const var_type_fns_t interval_fns = {
  .parse = units_parse_int,
  .encode = int_encode, // doesn't use params
  .clear = int_clear, // doesn't use params,
  .ok = units_ok_int // can't use int_ok, since that expects int params.
};

/////
// CONFIG_TYPE_DOUBLE
//
// This is a nice simple double.
/////

static int
double_parse(void *target, const char *value, char **errmsg,
             const void *params)
{
  (void)params;
  (void)errmsg;
  double *v = (double*)target;
  char *endptr=NULL;
  errno = 0;
  *v = strtod(value, &endptr);
  if (endptr == value || *endptr != '\0') {
    // Either there are no converted characters, or there were some characters
    // that didn't get converted.
    tor_asprintf(errmsg, "Could not convert %s to a number.", escaped(value));
    return -1;
  }
  if (errno == ERANGE) {
    // strtod will set errno to ERANGE on underflow or overflow.
    bool underflow = -.00001 < *v && *v < .00001;
    tor_asprintf(errmsg,
                 "%s is too %s to express as a floating-point number.",
                 escaped(value), underflow ? "small" : "large");
    return -1;
  }
  return 0;
}

static char *
double_encode(const void *value, const void *params)
{
  (void)params;
  double v = *(double*)value;
  char *result;
  tor_asprintf(&result, "%f", v);
  return result;
}

static void
double_clear(void *value, const void *params)
{
  (void)params;
  double *v = (double *)value;
  *v = 0.0;
}

static const var_type_fns_t double_fns = {
  .parse = double_parse,
  .encode = double_encode,
  .clear = double_clear,
};

/////
// CONFIG_TYPE_BOOL
// CONFIG_TYPE_AUTOBOOL
//
// These types are implemented as a case-insensitive string-to-integer
// mapping.
/////

typedef struct enumeration_table_t {
  const char *name;
  int value;
} enumeration_table_t;

typedef struct enumeration_params_t {
  const char *allowed_val_string;
  const enumeration_table_t *table;
} enumeration_params_t;

static int
enum_parse(void *target, const char *value, char **errmsg,
           const void *params_)
{
  const enumeration_params_t *params = params_;
  const enumeration_table_t *table = params->table;
  int *p = (int *)target;
  for (; table->name; ++table) {
    if (!strcasecmp(value, table->name)) {
      *p = table->value;
      return 0;
    }
  }
  tor_asprintf(errmsg, "Unrecognized value %s. %s",
               value, params->allowed_val_string);
  return -1;
}

static char *
enum_encode(const void *value, const void *params_)
{
  int v = *(const int*)value;
  const enumeration_params_t *params = params_;
  const enumeration_table_t *table = params->table;
  for (; table->name; ++table) {
    if (v == table->value)
      return tor_strdup(table->name);
  }
  return NULL; // error.
}

static void
enum_clear(void *value, const void *params_)
{
  int *p = (int*)value;
  const enumeration_params_t *params = params_;
  const enumeration_table_t *table = params->table;
  tor_assert(table->name);
  *p = table->value;
}

static bool
enum_ok(const void *value, const void *params_)
{
  int v = *(const int*)value;
  const enumeration_params_t *params = params_;
  const enumeration_table_t *table = params->table;
  for (; table->name; ++table) {
    if (v == table->value)
      return true;
  }
  return false;
}

static const enumeration_table_t enum_table_bool[] = {
  { "0", 0 },
  { "1", 1 },
  { NULL, 0 },
};

static const enumeration_params_t enum_params_bool = {
  "Allowed values are 0 and 1.",
  enum_table_bool
};

static const enumeration_table_t enum_table_autobool[] = {
  { "0", 0 },
  { "1", 1 },
  { "auto", -1 },
  { NULL, 0 },
};

static const enumeration_params_t enum_params_autobool = {
  "Allowed values are 0, 1, and auto.",
  enum_table_autobool
};

static const var_type_fns_t enum_fns = {
  .parse = enum_parse,
  .encode = enum_encode,
  .clear = enum_clear,
  .ok = enum_ok,
};

/////
// CONFIG_TYPE_ISOTIME
//
// This is a time_t, encoded in ISO8601 format.
/////

static int
time_parse(void *target, const char *value, char **errmsg,
           const void *params)
{
  (void) params;
  time_t *p = target;
  if (parse_iso_time(value, p) < 0) {
    tor_asprintf(errmsg, "Invalid time %s", escaped(value));
    return -1;
  }
  return 0;
}

static char *
time_encode(const void *value, const void *params)
{
  (void)params;
  time_t v = *(const time_t *)value;
  char *result = tor_malloc(ISO_TIME_LEN+1);
  format_iso_time(result, v);
  return result;
}

static void
time_clear(void *value, const void *params)
{
  (void)params;
  time_t *t = value;
  *t = 0;
}

static const var_type_fns_t time_fns = {
  .parse = time_parse,
  .encode = time_encode,
  .clear = time_clear,
};

/////
// CONFIG_TYPE_CSV
//
// This type is a comma-separated list of strings, stored in a smartlist_t.
// An empty list may be encoded either as an empty smartlist, or as NULL.
/////

static int
csv_parse(void *target, const char *value, char **errmsg,
          const void *params)
{
  (void)params;
  (void)errmsg;
  smartlist_t **sl = (smartlist_t**)target;
  *sl = smartlist_new();
  smartlist_split_string(*sl, value, ",",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  return 0;
}

static char *
csv_encode(const void *value, const void *params)
{
  (void)params;
  const smartlist_t *sl = *(const smartlist_t **)value;
  if (! sl)
    return tor_strdup("");

  return smartlist_join_strings(*(smartlist_t**)value, ",", 0, NULL);
}

static void
csv_clear(void *value, const void *params)
{
  (void)params;
  smartlist_t **sl = (smartlist_t**)value;
  if (!*sl)
    return;
  SMARTLIST_FOREACH(*sl, char *, cp, tor_free(cp));
  smartlist_free(*sl); // clears pointer.
}

static const var_type_fns_t csv_fns = {
  .parse = csv_parse,
  .encode = csv_encode,
  .clear = csv_clear,
};

/////
// CONFIG_TYPE_CSV_INTERVAL
//
// This type used to be a list of time intervals, used to determine a download
// schedule.  Now, only the first interval counts: everything after the first
// comma is discarded.
/////

static int
legacy_csv_interval_parse(void *target, const char *value, char **errmsg,
                          const void *params)
{
  (void)params;
  /* We used to have entire smartlists here.  But now that all of our
   * download schedules use exponential backoff, only the first part
   * matters. */
  const char *comma = strchr(value, ',');
  const char *val = value;
  char *tmp = NULL;
  if (comma) {
    tmp = tor_strndup(val, comma - val);
    val = tmp;
  }

  int rv = units_parse_int(target, val, errmsg, &time_units);
  tor_free(tmp);
  return rv;
}

static const var_type_fns_t legacy_csv_interval_fns = {
  .parse = legacy_csv_interval_parse,
  .encode = int_encode,
  .clear = int_clear,
};

/////
// CONFIG_TYPE_LINELIST
// CONFIG_TYPE_LINELIST_S
// CONFIG_TYPE_LINELIST_V
//
// A linelist is a raw config_line_t list.  Order is preserved.
//
// The LINELIST type is used for homogeneous lists, where all the lines
// have the same key.
//
// The LINELIST_S and LINELIST_V types are used for the case where multiple
// lines of different keys are kept in a single list, to preserve their
// relative order.  The unified list is stored as a "virtual" variable whose
// type is LINELIST_V; the individual sublists are treated as variables of
// type LINELIST_S.
//
// A linelist may be fragile or non-fragile. Assigning a line to a fragile
// linelist replaces the list with the line.  If the line has the "APPEND"
// command set on it, or if the list is non-fragile, the line is appended.
// Either way, the new list is non-fragile.
/////

static int
linelist_kv_parse(void *target, const struct config_line_t *line,
                  char **errmsg, const void *params)
{
  (void)params;
  (void)errmsg;
  config_line_t **lines = target;

  if (*lines && (*lines)->fragile) {
    if (line->command == CONFIG_LINE_APPEND) {
      (*lines)->fragile = 0;
    } else {
      config_free_lines(*lines); // sets it to NULL
    }
  }

  config_line_append(lines, line->key, line->value);
  return 0;
}

static int
linelist_kv_virt_noparse(void *target, const struct config_line_t *line,
                         char **errmsg, const void *params)
{
  (void)target;
  (void)line;
  (void)params;
  *errmsg = tor_strdup("Cannot assign directly to virtual option.");
  return -1;
}

static struct config_line_t *
linelist_kv_encode(const char *key, const void *value,
                   const void *params)
{
  (void)key;
  (void)params;
  config_line_t *lines = *(config_line_t **)value;
  return config_lines_dup(lines);
}

static struct config_line_t *
linelist_s_kv_encode(const char *key, const void *value,
                     const void *params)
{
  (void)params;
  config_line_t *lines = *(config_line_t **)value;
  return config_lines_dup_and_filter(lines, key);
}

static void
linelist_clear(void *target, const void *params)
{
  (void)params;
  config_line_t **lines = target;
  config_free_lines(*lines); // sets it to NULL
}

static bool
linelist_eq(const void *a, const void *b, const void *params)
{
  (void)params;
  const config_line_t *lines_a = *(const config_line_t **)a;
  const config_line_t *lines_b = *(const config_line_t **)b;
  return config_lines_eq(lines_a, lines_b);
}

static int
linelist_copy(void *target, const void *value, const void *params)
{
  (void)params;
  config_line_t **ptr = (config_line_t **)target;
  const config_line_t *val = *(const config_line_t **)value;
  config_free_lines(*ptr);
  *ptr = config_lines_dup(val);
  return 0;
}

static void
linelist_mark_fragile(void *target, const void *params)
{
  (void)params;
  config_line_t **ptr = (config_line_t **)target;
  if (*ptr)
    (*ptr)->fragile = 1;
}

static const var_type_fns_t linelist_fns = {
  .kv_parse = linelist_kv_parse,
  .kv_encode = linelist_kv_encode,
  .clear = linelist_clear,
  .eq = linelist_eq,
  .copy = linelist_copy,
  .mark_fragile = linelist_mark_fragile,
};

static const var_type_fns_t linelist_v_fns = {
  .kv_parse = linelist_kv_virt_noparse,
  .kv_encode = linelist_kv_encode,
  .clear = linelist_clear,
  .eq = linelist_eq,
  .copy = linelist_copy,
  .mark_fragile = linelist_mark_fragile,
};

static const var_type_fns_t linelist_s_fns = {
  .kv_parse = linelist_kv_parse,
  .kv_encode = linelist_s_kv_encode,
  .clear = linelist_clear,
  .eq = linelist_eq,
  .copy = linelist_copy,
};

/////
// CONFIG_TYPE_ROUTERSET
//
// XXXX to this module.
/////

/////
// CONFIG_TYPE_IGNORE
//
// Used to indicate an option that cannot be stored or encoded.
/////

static int
ignore_parse(void *target, const char *value, char **errmsg,
             const void *params)
{
  (void)target;
  (void)value;
  (void)errmsg;
  (void)params;
  return 0;
}

static char *
ignore_encode(const void *value, const void *params)
{
  (void)value;
  (void)params;
  return NULL;
}

static const var_type_fns_t ignore_fns = {
  .parse = ignore_parse,
  .encode = ignore_encode,
};

const var_type_def_t STRING_type_defn = {
  .name="String", .fns=&string_fns };
const var_type_def_t FILENAME_type_defn = {
  .name="Filename", .fns=&string_fns };
const var_type_def_t INT_type_defn = {
  .name="SignedInteger", .fns=&int_fns,
  .params=&INT_PARSE_UNRESTRICTED };
const var_type_def_t POSINT_type_defn = {
  .name="Integer", .fns=&int_fns,
  .params=&INT_PARSE_POSINT };
const var_type_def_t UINT64_type_defn = {
  .name="Integer", .fns=&uint64_fns, };
const var_type_def_t MEMUNIT_type_defn = {
  .name="DataSize", .fns=&memunit_fns,
  .params=&memory_units };
const var_type_def_t INTERVAL_type_defn = {
  .name="TimeInterval", .fns=&interval_fns,
  .params=&time_units };
const var_type_def_t MSEC_INTERVAL_type_defn = {
  .name="TimeMsecInterval",
  .fns=&interval_fns,
  .params=&time_msec_units };
const var_type_def_t DOUBLE_type_defn = {
  .name="Float", .fns=&double_fns, };
const var_type_def_t BOOL_type_defn = {
  .name="Boolean", .fns=&enum_fns,
  .params=&enum_params_bool };
const var_type_def_t AUTOBOOL_type_defn = {
  .name="Boolean+Auto", .fns=&enum_fns,
  .params=&enum_params_autobool };
const var_type_def_t ISOTIME_type_defn = {
  .name="Time", .fns=&time_fns, };
const var_type_def_t CSV_type_defn = {
  .name="CommaList", .fns=&csv_fns, };
const var_type_def_t CSV_INTERVAL_type_defn = {
  .name="TimeInterval",
  .fns=&legacy_csv_interval_fns, };
const var_type_def_t LINELIST_type_defn = {
  .name="LineList", .fns=&linelist_fns,
  .flags=CFLG_NOREPLACE };
/*
 * A "linelist_s" is a derived view of a linelist_v: inspecting
 * it gets part of a linelist_v, and setting it adds to the linelist_v.
 */
const var_type_def_t LINELIST_S_type_defn = {
  .name="Dependent", .fns=&linelist_s_fns,
  .flags=CFLG_NOREPLACE|
  /* The operations we disable here are
   * handled by the linelist_v. */
  CFLG_NOCOPY|CFLG_NOCMP|CFLG_NODUMP };
const var_type_def_t LINELIST_V_type_defn = {
  .name="Virtual", .fns=&linelist_v_fns,
  .flags=CFLG_NOREPLACE|CFLG_NOSET };
const var_type_def_t IGNORE_type_defn = {
  .name="Ignored", .fns=&ignore_fns,
  .flags=CFLG_NOCOPY|CFLG_NOCMP|CFLG_NODUMP|CFLG_NOSET,
};
const var_type_def_t OBSOLETE_type_defn = {
  .name="Obsolete", .fns=&ignore_fns,
  .flags=CFLG_GROUP_OBSOLETE,
};

/**
 * Table mapping conf_type_t values to var_type_def_t objects.
 **/
static const var_type_def_t *type_definitions_table[] = {
  [CONFIG_TYPE_STRING] = &STRING_type_defn,
  [CONFIG_TYPE_FILENAME] = &FILENAME_type_defn,
  [CONFIG_TYPE_INT] = &INT_type_defn,
  [CONFIG_TYPE_POSINT] = &POSINT_type_defn,
  [CONFIG_TYPE_UINT64] = &UINT64_type_defn,
  [CONFIG_TYPE_MEMUNIT] = &MEMUNIT_type_defn,
  [CONFIG_TYPE_INTERVAL] = &INTERVAL_type_defn,
  [CONFIG_TYPE_MSEC_INTERVAL] = &MSEC_INTERVAL_type_defn,
  [CONFIG_TYPE_DOUBLE] = &DOUBLE_type_defn,
  [CONFIG_TYPE_BOOL] = &BOOL_type_defn,
  [CONFIG_TYPE_AUTOBOOL] = &AUTOBOOL_type_defn,
  [CONFIG_TYPE_ISOTIME] = &ISOTIME_type_defn,
  [CONFIG_TYPE_CSV] = &CSV_type_defn,
  [CONFIG_TYPE_CSV_INTERVAL] = &CSV_INTERVAL_type_defn,
  [CONFIG_TYPE_LINELIST] = &LINELIST_type_defn,
  [CONFIG_TYPE_LINELIST_S] = &LINELIST_S_type_defn,
  [CONFIG_TYPE_LINELIST_V] = &LINELIST_V_type_defn,
  [CONFIG_TYPE_IGNORE] = &IGNORE_type_defn,
  [CONFIG_TYPE_OBSOLETE] = &OBSOLETE_type_defn,
};

/**
 * Return a pointer to the var_type_def_t object for the given
 * config_type_t value, or NULL if no such type definition exists.
 **/
const var_type_def_t *
lookup_type_def(config_type_t type)
{
  int t = type;
  tor_assert(t >= 0);
  if (t >= (int)ARRAY_LENGTH(type_definitions_table))
    return NULL;
  return type_definitions_table[t];
}
