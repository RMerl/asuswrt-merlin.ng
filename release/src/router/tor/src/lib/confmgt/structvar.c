/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file structvar.c
 * @brief Functions to manipulate named and typed elements of
 *    a structure.
 *
 * These functions represent a low-level API for accessing a member of a
 * structure.  They use typedvar.c to work, and they are used in turn by the
 * configuration system to examine and set fields in configuration objects
 * used by individual modules.
 *
 * Almost no code should call these directly.
 **/

#include "orconfig.h"
#include "lib/confmgt/structvar.h"
#include "lib/cc/compat_compiler.h"
#include "lib/conf/conftypes.h"
#include "lib/confmgt/type_defs.h"
#include "lib/confmgt/typedvar.h"
#include "lib/log/util_bug.h"

#include "lib/confmgt/var_type_def_st.h"

#include <stddef.h>

/**
 * Return true iff all fields on <b>decl</b> are NULL or 0, indicating that
 * there is no object or no magic number to check.
 **/
static inline bool
magic_is_null(const struct_magic_decl_t *decl)
{
  return decl->typename == NULL &&
    decl->magic_offset == 0 &&
    decl->magic_val == 0;
}

/**
 * Set the 'magic number' on <b>object</b> to correspond to decl.
 **/
void
struct_set_magic(void *object, const struct_magic_decl_t *decl)
{
  tor_assert(decl);
  if (magic_is_null(decl))
    return;

  tor_assert(object);
  uint32_t *ptr = STRUCT_VAR_P(object, decl->magic_offset);
  *ptr = decl->magic_val;
}

/**
 * Assert that the 'magic number' on <b>object</b> to corresponds to decl.
 **/
void
struct_check_magic(const void *object, const struct_magic_decl_t *decl)
{
  tor_assert(decl);
  if (magic_is_null(decl))
    return;

  tor_assert(object);

  const uint32_t *ptr = STRUCT_VAR_P(object, decl->magic_offset);
  tor_assertf(*ptr == decl->magic_val,
              "Bad magic number on purported %s object. "
              "Expected %"PRIu32"x but got %"PRIu32"x.",
              decl->typename, decl->magic_val, *ptr);
}

/**
 * Return a mutable pointer to the member of <b>object</b> described
 * by <b>member</b>.
 **/
void *
struct_get_mptr(void *object, const struct_member_t *member)
{
  tor_assert(object);
  return STRUCT_VAR_P(object, member->offset);
}

/**
 * Return a const pointer to the member of <b>object</b> described
 * by <b>member</b>.
 **/
const void *
struct_get_ptr(const void *object, const struct_member_t *member)
{
  tor_assert(object);
  return STRUCT_VAR_P(object, member->offset);
}

/**
 * Helper: given a struct_member_t, look up the type definition for its
 * variable.
 */
static const var_type_def_t *
get_type_def(const struct_member_t *member)
{
  if (member->type_def)
    return member->type_def;

  return lookup_type_def(member->type);
}

/**
 * (As typed_var_free, but free and clear the member of <b>object</b> defined
 * by <b>member</b>.)
 **/
void
struct_var_free(void *object, const struct_member_t *member)
{
  void *p = struct_get_mptr(object, member);
  const var_type_def_t *def = get_type_def(member);

  typed_var_free(p, def);
}

/**
 * (As typed_var_copy, but copy from <b>src</b> to <b>dest</b> the member
 * defined by <b>member</b>.)
 **/
int
struct_var_copy(void *dest, const void *src, const struct_member_t *member)
{
  void *p_dest = struct_get_mptr(dest, member);
  const void *p_src = struct_get_ptr(src, member);
  const var_type_def_t *def = get_type_def(member);

  return typed_var_copy(p_dest, p_src, def);
}

/**
 * (As typed_var_eq, but compare the members of <b>a</b> and <b>b</b>
 * defined by <b>member</b>.)
 **/
bool
struct_var_eq(const void *a, const void *b, const struct_member_t *member)
{
  const void *p_a = struct_get_ptr(a, member);
  const void *p_b = struct_get_ptr(b, member);
  const var_type_def_t *def = get_type_def(member);

  return typed_var_eq(p_a, p_b, def);
}

/**
 * (As typed_var_ok, but validate the member of <b>object</b> defined by
 * <b>member</b>.)
 **/
bool
struct_var_ok(const void *object, const struct_member_t *member)
{
  const void *p = struct_get_ptr(object, member);
  const var_type_def_t *def = get_type_def(member);

  return typed_var_ok(p, def);
}

/**
 * (As typed_var_kvassign, but assign a value to the member of <b>object</b>
 * defined by <b>member</b>.)
 **/
int
struct_var_kvassign(void *object, const struct config_line_t *line,
                    char **errmsg,
                    const struct_member_t *member)
{
  void *p = struct_get_mptr(object, member);
  const var_type_def_t *def = get_type_def(member);

  return typed_var_kvassign(p, line, errmsg, def);
}

/**
 * (As typed_var_kvencode, but encode the value of the member of <b>object</b>
 * defined by <b>member</b>.)
 **/
struct config_line_t *
struct_var_kvencode(const void *object, const struct_member_t *member)
{
  const void *p = struct_get_ptr(object, member);
  const var_type_def_t *def = get_type_def(member);

  return typed_var_kvencode(member->name, p, def);
}

/**
 * Mark the field in <b>object</b> determined by <b>member</b> -- a variable
 * that ordinarily would be extended by assignment -- as "fragile", so that it
 * will get replaced by the next assignment instead.
 */
void
struct_var_mark_fragile(void *object, const struct_member_t *member)
{
  void *p = struct_get_mptr(object, member);
  const var_type_def_t *def = get_type_def(member);
  return typed_var_mark_fragile(p, def);
}

/**
 * Return the official name of this struct member.
 **/
const char *
struct_var_get_name(const struct_member_t *member)
{
  return member->name;
}

/**
 * Return the type name for this struct member.
 *
 * Do not use the output of this function to inspect a type within Tor.  It is
 * suitable for debugging, informing the controller or user of a variable's
 * type, etc.
 **/
const char *
struct_var_get_typename(const struct_member_t *member)
{
  const var_type_def_t *def = get_type_def(member);

  return def ? def->name : NULL;
}

/** Return all of the flags set for this struct member. */
uint32_t
struct_var_get_flags(const struct_member_t *member)
{
  const var_type_def_t *def = get_type_def(member);

  return def ? def->flags : 0;
}
