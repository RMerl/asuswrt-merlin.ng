/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file namemap.c
 * @brief Mappings between identifiers and 16-bit ints.
 **/

#include "orconfig.h"
#include "lib/container/smartlist.h"
#include "lib/container/namemap.h"
#include "lib/container/namemap_st.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include "ext/siphash.h"

#include <string.h>

/** Helper for namemap hashtable implementation: compare two entries. */
static inline int
mapped_name_eq(const mapped_name_t *a, const mapped_name_t *b)
{
  return !strcmp(a->name, b->name);
}

/** Helper for namemap hashtable implementation: hash an entry. */
static inline unsigned
mapped_name_hash(const mapped_name_t *a)
{
  return (unsigned) siphash24g(a->name, strlen(a->name));
}

HT_PROTOTYPE(namemap_ht, mapped_name_t, node, mapped_name_hash,
             mapped_name_eq);
HT_GENERATE2(namemap_ht, mapped_name_t, node, mapped_name_hash,
             mapped_name_eq, 0.6, tor_reallocarray_, tor_free_);

/** Set up an uninitialized <b>map</b>. */
void
namemap_init(namemap_t *map)
{
  memset(map, 0, sizeof(*map));
  HT_INIT(namemap_ht, &map->ht);
  map->names = smartlist_new();
}

/** Return the name that <b>map</b> associates with a given <b>id</b>, or
 * NULL if there is no such name. */
const char *
namemap_get_name(const namemap_t *map, unsigned id)
{
  if (map->names && id < (unsigned)smartlist_len(map->names)) {
    mapped_name_t *name = smartlist_get(map->names, (int)id);
    return name->name;
  } else {
    return NULL;
  }
}

/**
 * Return the name that <b>map</b> associates with a given <b>id</b>, or a
 * pointer to a statically allocated string describing the value of <b>id</b>
 * if no such name exists.
 **/
const char *
namemap_fmt_name(const namemap_t *map, unsigned id)
{
  static char buf[32];

  const char *name = namemap_get_name(map, id);
  if (name)
    return name;

  tor_snprintf(buf, sizeof(buf), "{%u}", id);

  return buf;
}

/**
 * Helper: As namemap_get_id(), but requires that <b>name</b> is
 * <b>namelen</b> characters long, and that <b>namelen</b> is no more than
 * MAX_NAMEMAP_NAME_LEN.
 */
static unsigned
namemap_get_id_unchecked(const namemap_t *map,
                         const char *name,
                         size_t namelen)
{
  union {
    mapped_name_t n;
    char storage[MAX_NAMEMAP_NAME_LEN + sizeof(mapped_name_t) + 1];
  } u;
  memcpy(u.n.name, name, namelen);
  u.n.name[namelen] = 0;
  const mapped_name_t *found = HT_FIND(namemap_ht, &map->ht, &u.n);
  if (found) {
    tor_assert(map->names);
    tor_assert(smartlist_get(map->names, found->intval) == found);
    return found->intval;
  }

  return NAMEMAP_ERR;
}

/**
 * Return the identifier currently associated by <b>map</b> with the name
 * <b>name</b>, or NAMEMAP_ERR if no such identifier exists.
 **/
unsigned
namemap_get_id(const namemap_t *map,
               const char *name)
{
  size_t namelen = strlen(name);
  if (namelen > MAX_NAMEMAP_NAME_LEN) {
    return NAMEMAP_ERR;
  }

  return namemap_get_id_unchecked(map, name, namelen);
}

/**
 * Return the identifier associated by <b>map</b> with the name
 * <b>name</b>, allocating a new identifier in <b>map</b> if none exists.
 *
 * Return NAMEMAP_ERR if <b>name</b> is too long, or if there are no more
 * identifiers we can allocate.
 **/
unsigned
namemap_get_or_create_id(namemap_t *map,
                         const char *name)
{
  size_t namelen = strlen(name);
  if (namelen > MAX_NAMEMAP_NAME_LEN) {
    return NAMEMAP_ERR;
  }

  if (PREDICT_UNLIKELY(map->names == NULL))
    map->names = smartlist_new();

  unsigned found = namemap_get_id_unchecked(map, name, namelen);
  if (found != NAMEMAP_ERR)
    return found;

  unsigned new_id = (unsigned)smartlist_len(map->names);
  if (new_id == NAMEMAP_ERR)
    return NAMEMAP_ERR; /* Can't allocate any more. */

  mapped_name_t *insert = tor_malloc_zero(
                       offsetof(mapped_name_t, name) + namelen + 1);
  memcpy(insert->name, name, namelen+1);
  insert->intval = new_id;

  HT_INSERT(namemap_ht, &map->ht, insert);
  smartlist_add(map->names, insert);

  return new_id;
}

/** Return the number of entries in 'names' */
size_t
namemap_get_size(const namemap_t *map)
{
  if (PREDICT_UNLIKELY(map->names == NULL))
    return 0;

  return smartlist_len(map->names);
}

/**
 * Release all storage held in <b>map</b>.
 */
void
namemap_clear(namemap_t *map)
{
  if (!map)
    return;

  HT_CLEAR(namemap_ht, &map->ht);
  if (map->names) {
    SMARTLIST_FOREACH(map->names, mapped_name_t *, n,
                      tor_free(n));
    smartlist_free(map->names);
  }
  memset(map, 0, sizeof(*map));
}
