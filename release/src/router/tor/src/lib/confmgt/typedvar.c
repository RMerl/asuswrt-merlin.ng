/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file typedvar.c
 * @brief Functions for accessing a pointer as an object of a given type.
 *
 * These functions represent a low-level API for accessing a typed variable.
 * They are used in the configuration system to examine and set fields in
 * configuration objects used by individual modules.
 *
 * Almost no code should call these directly.
 **/

#include "orconfig.h"
#include "lib/conf/conftypes.h"
#include "lib/confmgt/type_defs.h"
#include "lib/confmgt/typedvar.h"
#include "lib/encoding/confline.h"
#include "lib/log/escape.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"

#include "lib/confmgt/var_type_def_st.h"

#include <stddef.h>
#include <string.h>

/**
 * Try to parse a string in <b>value</b> that encodes an object of the type
 * defined by <b>def</b>.
 *
 * On success, adjust the lvalue pointed to by <b>target</b> to hold that
 * value, and return 0.  On failure, set *<b>errmsg</b> to a newly allocated
 * string holding an error message, and return -1.
 **/
int
typed_var_assign(void *target, const char *value, char **errmsg,
                    const var_type_def_t *def)
{
  if (BUG(!def))
    return -1; // LCOV_EXCL_LINE
  // clear old value if needed.
  typed_var_free(target, def);

  tor_assert(def->fns->parse);
  return def->fns->parse(target, value, errmsg, def->params);
}

/**
 * Try to parse a single line from the head of<b>line</b> that encodes an
 * object of the type defined in <b>def</b>. On success and failure, behave as
 * typed_var_assign().
 *
 * All types for which keys are significant should use this function.
 *
 * Note that although multiple lines may be provided in <b>line</b>,
 * only the first one is handled by this function.
 **/
int
typed_var_kvassign(void *target, const config_line_t *line,
                      char **errmsg, const var_type_def_t *def)
{
  if (BUG(!def))
    return -1; // LCOV_EXCL_LINE

  if (def->fns->kv_parse) {
    // We do _not_ free the old value here, since linelist options
    // sometimes have append semantics.
    return def->fns->kv_parse(target, line, errmsg, def->params);
  }

  int rv = typed_var_assign(target, line->value, errmsg, def);
  if (rv < 0 && *errmsg != NULL) {
    /* typed_var_assign() didn't know the line's keyword, but we do.
     * Let's add it to the error message. */
    char *oldmsg = *errmsg;
    tor_asprintf(errmsg, "Could not parse %s: %s", line->key, oldmsg);
    tor_free(oldmsg);
  }
  return rv;
}

/**
 * Release storage held by a variable in <b>target</b> of type defined by
 * <b>def</b>, and set <b>target</b> to a reasonable default.
 **/
void
typed_var_free(void *target, const var_type_def_t *def)
{
  if (BUG(!def))
    return; // LCOV_EXCL_LINE
  if (def->fns->clear) {
    def->fns->clear(target, def->params);
  }
}

/**
 * Encode a value of type <b>def</b> pointed to by <b>value</b>, and return
 * its result in a newly allocated string.  The string may need to be escaped.
 *
 * Returns NULL if this option has a NULL value, or on internal error.
 **/
char *
typed_var_encode(const void *value, const var_type_def_t *def)
{
  if (BUG(!def))
    return NULL; // LCOV_EXCL_LINE
  tor_assert(def->fns->encode);
  return def->fns->encode(value, def->params);
}

/**
 * As typed_var_encode(), but returns a newly allocated config_line_t
 * object.  The provided <b>key</b> is used as the key of the lines, unless
 * the type is one (line a linelist) that encodes its own keys.
 *
 * This function may return a list of multiple lines.
 *
 * Returns NULL if there are no lines to encode, or on internal error.
 */
config_line_t *
typed_var_kvencode(const char *key, const void *value,
                      const var_type_def_t *def)
{
  if (BUG(!def))
    return NULL; // LCOV_EXCL_LINE
  if (def->fns->kv_encode) {
    return def->fns->kv_encode(key, value, def->params);
  }
  char *encoded_value = typed_var_encode(value, def);
  if (!encoded_value)
    return NULL;

  config_line_t *result = tor_malloc_zero(sizeof(config_line_t));
  result->key = tor_strdup(key);
  result->value = encoded_value;
  return result;
}

/**
 * Set <b>dest</b> to contain the same value as <b>src</b>. Both types
 * must be as defined by <b>def</b>.
 *
 * Return 0 on success, and -1 on failure.
 **/
int
typed_var_copy(void *dest, const void *src, const var_type_def_t *def)
{
  if (BUG(!def))
    return -1; // LCOV_EXCL_LINE
  if (def->fns->copy) {
    // If we have been provided a copy function, use it.
    return def->fns->copy(dest, src, def);
  }

  // Otherwise, encode 'src' and parse the result into 'def'.
  char *enc = typed_var_encode(src, def);
  if (!enc) {
    typed_var_free(dest, def);
    return 0;
  }
  char *err = NULL;
  int rv = typed_var_assign(dest, enc, &err, def);
  if (BUG(rv < 0)) {
    // LCOV_EXCL_START
    log_warn(LD_BUG, "Encoded value %s was not parseable as a %s: %s",
             escaped(enc), def->name, err?err:"");
    // LCOV_EXCL_STOP
  }
  tor_free(err);
  tor_free(enc);
  return rv;
}

/**
 * Return true if <b>a</b> and <b>b</b> are semantically equivalent.
 * Both types must be as defined by <b>def</b>.
 **/
bool
typed_var_eq(const void *a, const void *b, const var_type_def_t *def)
{
  if (BUG(!def))
    return false; // LCOV_EXCL_LINE

  if (def->fns->eq) {
    // Use a provided eq function if we got one.
    return def->fns->eq(a, b, def->params);
  }

  // Otherwise, encode the values and compare them.
  char *enc_a = typed_var_encode(a, def);
  char *enc_b = typed_var_encode(b, def);
  bool eq = !strcmp_opt(enc_a,enc_b);
  tor_free(enc_a);
  tor_free(enc_b);
  return eq;
}

/**
 * Check whether <b>value</b> encodes a valid value according to the
 * type definition in <b>def</b>.
 */
bool
typed_var_ok(const void *value, const var_type_def_t *def)
{
  if (BUG(!def))
    return false; // LCOV_EXCL_LINE

  if (def->fns->ok)
    return def->fns->ok(value, def->params);

  return true;
}

/**
 * Mark <b>value</b> -- a variable that ordinarily would be extended by
 * assignment -- as "fragile", so that it will get replaced by the next
 * assignment instead.
 **/
void
typed_var_mark_fragile(void *value, const var_type_def_t *def)
{
  if (BUG(!def)) {
    return; // LCOV_EXCL_LINE
  }

  if (def->fns->mark_fragile)
    def->fns->mark_fragile(value, def->params);
}
