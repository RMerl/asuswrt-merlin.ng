/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file map.c
 *
 * \brief Hash-table implementations of a string-to-void* map, and of
 * a digest-to-void* map.
 **/

#include "lib/container/map.h"
#include "lib/ctime/di_ops.h"
#include "lib/defs/digest_sizes.h"
#include "lib/string/util_string.h"
#include "lib/malloc/malloc.h"

#include "lib/log/util_bug.h"

#include <stdlib.h>
#include <string.h>

#include "ext/ht.h"

/** Helper: Declare an entry type and a map type to implement a mapping using
 * ht.h.  The map type will be called <b>maptype</b>.  The key part of each
 * entry is declared using the C declaration <b>keydecl</b>.  All functions
 * and types associated with the map get prefixed with <b>prefix</b> */
#define DEFINE_MAP_STRUCTS(maptype, keydecl, prefix)      \
  typedef struct prefix ## entry_t {                      \
    HT_ENTRY(prefix ## entry_t) node;                     \
    void *val;                                            \
    keydecl;                                              \
  } prefix ## entry_t;                                    \
  struct maptype {                                        \
    HT_HEAD(prefix ## impl, prefix ## entry_t) head;      \
  }

DEFINE_MAP_STRUCTS(strmap_t, char *key, strmap_);
DEFINE_MAP_STRUCTS(digestmap_t, char key[DIGEST_LEN], digestmap_);
DEFINE_MAP_STRUCTS(digest256map_t, uint8_t key[DIGEST256_LEN], digest256map_);

/** Helper: compare strmap_entry_t objects by key value. */
static inline int
strmap_entries_eq(const strmap_entry_t *a, const strmap_entry_t *b)
{
  return !strcmp(a->key, b->key);
}

/** Helper: return a hash value for a strmap_entry_t. */
static inline unsigned int
strmap_entry_hash(const strmap_entry_t *a)
{
  return (unsigned) siphash24g(a->key, strlen(a->key));
}

/** Helper: compare digestmap_entry_t objects by key value. */
static inline int
digestmap_entries_eq(const digestmap_entry_t *a, const digestmap_entry_t *b)
{
  return tor_memeq(a->key, b->key, DIGEST_LEN);
}

/** Helper: return a hash value for a digest_map_t. */
static inline unsigned int
digestmap_entry_hash(const digestmap_entry_t *a)
{
  return (unsigned) siphash24g(a->key, DIGEST_LEN);
}

/** Helper: compare digestmap_entry_t objects by key value. */
static inline int
digest256map_entries_eq(const digest256map_entry_t *a,
                        const digest256map_entry_t *b)
{
  return tor_memeq(a->key, b->key, DIGEST256_LEN);
}

/** Helper: return a hash value for a digest_map_t. */
static inline unsigned int
digest256map_entry_hash(const digest256map_entry_t *a)
{
  return (unsigned) siphash24g(a->key, DIGEST256_LEN);
}

HT_PROTOTYPE(strmap_impl, strmap_entry_t, node, strmap_entry_hash,
             strmap_entries_eq);
HT_GENERATE2(strmap_impl, strmap_entry_t, node, strmap_entry_hash,
             strmap_entries_eq, 0.6, tor_reallocarray_, tor_free_);

HT_PROTOTYPE(digestmap_impl, digestmap_entry_t, node, digestmap_entry_hash,
             digestmap_entries_eq);
HT_GENERATE2(digestmap_impl, digestmap_entry_t, node, digestmap_entry_hash,
             digestmap_entries_eq, 0.6, tor_reallocarray_, tor_free_);

HT_PROTOTYPE(digest256map_impl, digest256map_entry_t, node,
             digest256map_entry_hash,
             digest256map_entries_eq);
HT_GENERATE2(digest256map_impl, digest256map_entry_t, node,
             digest256map_entry_hash,
             digest256map_entries_eq, 0.6, tor_reallocarray_, tor_free_);

#define strmap_entry_free(ent) \
  FREE_AND_NULL(strmap_entry_t, strmap_entry_free_, (ent))
#define digestmap_entry_free(ent) \
  FREE_AND_NULL(digestmap_entry_t, digestmap_entry_free_, (ent))
#define digest256map_entry_free(ent) \
  FREE_AND_NULL(digest256map_entry_t, digest256map_entry_free_, (ent))

static inline void
strmap_entry_free_(strmap_entry_t *ent)
{
  tor_free(ent->key);
  tor_free(ent);
}
static inline void
digestmap_entry_free_(digestmap_entry_t *ent)
{
  tor_free(ent);
}
static inline void
digest256map_entry_free_(digest256map_entry_t *ent)
{
  tor_free(ent);
}

static inline void
strmap_assign_tmp_key(strmap_entry_t *ent, const char *key)
{
  ent->key = (char*)key;
}
static inline void
digestmap_assign_tmp_key(digestmap_entry_t *ent, const char *key)
{
  memcpy(ent->key, key, DIGEST_LEN);
}
static inline void
digest256map_assign_tmp_key(digest256map_entry_t *ent, const uint8_t *key)
{
  memcpy(ent->key, key, DIGEST256_LEN);
}
static inline void
strmap_assign_key(strmap_entry_t *ent, const char *key)
{
  ent->key = tor_strdup(key);
}
static inline void
digestmap_assign_key(digestmap_entry_t *ent, const char *key)
{
  memcpy(ent->key, key, DIGEST_LEN);
}
static inline void
digest256map_assign_key(digest256map_entry_t *ent, const uint8_t *key)
{
  memcpy(ent->key, key, DIGEST256_LEN);
}

/**
 * Macro: implement all the functions for a map that are declared in
 * map.h by the DECLARE_MAP_FNS() macro.  You must additionally define a
 * prefix_entry_free_() function to free entries (and their keys), a
 * prefix_assign_tmp_key() function to temporarily set a stack-allocated
 * entry to hold a key, and a prefix_assign_key() function to set a
 * heap-allocated entry to hold a key.
 */
#define IMPLEMENT_MAP_FNS(maptype, keytype, prefix)                     \
  /** Create and return a new empty map. */                             \
  MOCK_IMPL(maptype *,                                                  \
  prefix##_new,(void))                                                  \
  {                                                                     \
    maptype *result;                                                    \
    result = tor_malloc(sizeof(maptype));                               \
    HT_INIT(prefix##_impl, &result->head);                              \
    return result;                                                      \
  }                                                                     \
                                                                        \
  /** Return the item from <b>map</b> whose key matches <b>key</b>, or  \
   * NULL if no such value exists. */                                   \
  void *                                                                \
  prefix##_get(const maptype *map, const keytype key)                   \
  {                                                                     \
    prefix ##_entry_t *resolve;                                         \
    prefix ##_entry_t search;                                           \
    tor_assert(map);                                                    \
    tor_assert(key);                                                    \
    prefix ##_assign_tmp_key(&search, key);                             \
    resolve = HT_FIND(prefix ##_impl, &map->head, &search);             \
    if (resolve) {                                                      \
      return resolve->val;                                              \
    } else {                                                            \
      return NULL;                                                      \
    }                                                                   \
  }                                                                     \
                                                                        \
  /** Add an entry to <b>map</b> mapping <b>key</b> to <b>val</b>;      \
   * return the previous value, or NULL if no such value existed. */     \
  void *                                                                \
  prefix##_set(maptype *map, const keytype key, void *val)              \
  {                                                                     \
    prefix##_entry_t search;                                            \
    void *oldval;                                                       \
    tor_assert(map);                                                    \
    tor_assert(key);                                                    \
    tor_assert(val);                                                    \
    prefix##_assign_tmp_key(&search, key);                              \
    /* We a lot of our time in this function, so the code below is */   \
    /* meant to optimize the check/alloc/set cycle by avoiding the two */\
    /* trips to the hash table that we would do in the unoptimized */   \
    /* version of this code. (Each of HT_INSERT and HT_FIND calls */     \
    /* HT_SET_HASH and HT_FIND_P.) */                                   \
    HT_FIND_OR_INSERT_(prefix##_impl, node, prefix##_entry_hash,        \
                       &(map->head),                                    \
                       prefix##_entry_t, &search, ptr,                  \
                       {                                                \
                         /* we found an entry. */                       \
                         oldval = (*ptr)->val;                          \
                         (*ptr)->val = val;                             \
                         return oldval;                                 \
                       },                                               \
                       {                                                \
                         /* We didn't find the entry. */                \
                         prefix##_entry_t *newent =                     \
                           tor_malloc_zero(sizeof(prefix##_entry_t));   \
                         prefix##_assign_key(newent, key);              \
                         newent->val = val;                             \
                         HT_FOI_INSERT_(node, &(map->head),             \
                            &search, newent, ptr);                      \
                         return NULL;                                   \
    });                                                                 \
  }                                                                     \
                                                                        \
  /** Remove the value currently associated with <b>key</b> from the map. \
   * Return the value if one was set, or NULL if there was no entry for \
   * <b>key</b>.                                                        \
   *                                                                    \
   * Note: you must free any storage associated with the returned value. \
   */                                                                   \
  void *                                                                \
  prefix##_remove(maptype *map, const keytype key)                      \
  {                                                                     \
    prefix##_entry_t *resolve;                                          \
    prefix##_entry_t search;                                            \
    void *oldval;                                                       \
    tor_assert(map);                                                    \
    tor_assert(key);                                                    \
    prefix##_assign_tmp_key(&search, key);                              \
    resolve = HT_REMOVE(prefix##_impl, &map->head, &search);            \
    if (resolve) {                                                      \
      oldval = resolve->val;                                            \
      prefix##_entry_free(resolve);                                     \
      return oldval;                                                    \
    } else {                                                            \
      return NULL;                                                      \
    }                                                                   \
  }                                                                     \
                                                                        \
  /** Return the number of elements in <b>map</b>. */                   \
  int                                                                   \
  prefix##_size(const maptype *map)                                     \
  {                                                                     \
    return HT_SIZE(&map->head);                                         \
  }                                                                     \
                                                                        \
  /** Return true iff <b>map</b> has no entries. */                     \
  int                                                                   \
  prefix##_isempty(const maptype *map)                                  \
  {                                                                     \
    return HT_EMPTY(&map->head);                                        \
  }                                                                     \
                                                                        \
  /** Assert that <b>map</b> is not corrupt. */                         \
  void                                                                  \
  prefix##_assert_ok(const maptype *map)                                \
  {                                                                     \
    tor_assert(!prefix##_impl_HT_REP_IS_BAD_(&map->head));              \
  }                                                                     \
                                                                        \
  /** Remove all entries from <b>map</b>, and deallocate storage for    \
   * those entries.  If free_val is provided, invoked it every value in \
   * <b>map</b>. */                                                     \
  MOCK_IMPL(void,                                                       \
  prefix##_free_, (maptype *map, void (*free_val)(void*)))              \
  {                                                                     \
    prefix##_entry_t **ent, **next, *this;                              \
    if (!map)                                                           \
      return;                                                           \
    for (ent = HT_START(prefix##_impl, &map->head); ent != NULL;        \
         ent = next) {                                                  \
      this = *ent;                                                      \
      next = HT_NEXT_RMV(prefix##_impl, &map->head, ent);               \
      if (free_val)                                                     \
        free_val(this->val);                                            \
      prefix##_entry_free(this);                                        \
    }                                                                   \
    tor_assert(HT_EMPTY(&map->head));                                   \
    HT_CLEAR(prefix##_impl, &map->head);                                \
    tor_free(map);                                                      \
  }                                                                     \
                                                                        \
  /** return an <b>iterator</b> pointer to the front of a map.          \
   *                                                                    \
   * Iterator example:                                                  \
   *                                                                    \
   * \code                                                              \
   * // uppercase values in "map", removing empty values.               \
   *                                                                    \
   * strmap_iter_t *iter;                                               \
   * const char *key;                                                   \
   * void *val;                                                         \
   * char *cp;                                                          \
   *                                                                    \
   * for (iter = strmap_iter_init(map); !strmap_iter_done(iter); ) {    \
   *    strmap_iter_get(iter, &key, &val);                              \
   *    cp = (char*)val;                                                \
   *    if (!*cp) {                                                     \
   *       iter = strmap_iter_next_rmv(map,iter);                       \
   *       free(val);                                                   \
   *    } else {                                                        \
   *       for (;*cp;cp++) *cp = TOR_TOUPPER(*cp);                      \
   */                                                                   \
  prefix##_iter_t *                                                     \
  prefix##_iter_init(maptype *map)                                      \
  {                                                                     \
    tor_assert(map);                                                    \
    return HT_START(prefix##_impl, &map->head);                         \
  }                                                                     \
                                                                        \
  /** Advance <b>iter</b> a single step to the next entry, and return   \
   * its new value. */                                                  \
  prefix##_iter_t *                                                     \
  prefix##_iter_next(maptype *map, prefix##_iter_t *iter)               \
  {                                                                     \
    tor_assert(map);                                                    \
    tor_assert(iter);                                                   \
    return HT_NEXT(prefix##_impl, &map->head, iter);                    \
  }                                                                     \
  /** Advance <b>iter</b> a single step to the next entry, removing the \
   * current entry, and return its new value. */                        \
  prefix##_iter_t *                                                     \
  prefix##_iter_next_rmv(maptype *map, prefix##_iter_t *iter)           \
  {                                                                     \
    prefix##_entry_t *rmv;                                              \
    tor_assert(map);                                                    \
    tor_assert(iter);                                                   \
    tor_assert(*iter);                                                  \
    rmv = *iter;                                                        \
    iter = HT_NEXT_RMV(prefix##_impl, &map->head, iter);                \
    prefix##_entry_free(rmv);                                           \
    return iter;                                                        \
  }                                                                     \
  /** Set *<b>keyp</b> and *<b>valp</b> to the current entry pointed    \
   * to by iter. */                                                     \
  void                                                                  \
  prefix##_iter_get(prefix##_iter_t *iter, const keytype *keyp,         \
                    void **valp)                                        \
  {                                                                     \
    tor_assert(iter);                                                   \
    tor_assert(*iter);                                                  \
    tor_assert(keyp);                                                   \
    tor_assert(valp);                                                   \
    *keyp = (*iter)->key;                                               \
    *valp = (*iter)->val;                                               \
  }                                                                     \
  /** Return true iff <b>iter</b> has advanced past the last entry of   \
   * <b>map</b>. */                                                     \
  int                                                                   \
  prefix##_iter_done(prefix##_iter_t *iter)                             \
  {                                                                     \
    return iter == NULL;                                                \
  }

IMPLEMENT_MAP_FNS(strmap_t, char *, strmap)
IMPLEMENT_MAP_FNS(digestmap_t, char *, digestmap)
IMPLEMENT_MAP_FNS(digest256map_t, uint8_t *, digest256map)

/** Same as strmap_set, but first converts <b>key</b> to lowercase. */
void *
strmap_set_lc(strmap_t *map, const char *key, void *val)
{
  /* We could be a little faster by using strcasecmp instead, and a separate
   * type, but I don't think it matters. */
  void *v;
  char *lc_key = tor_strdup(key);
  tor_strlower(lc_key);
  v = strmap_set(map,lc_key,val);
  tor_free(lc_key);
  return v;
}

/** Same as strmap_get, but first converts <b>key</b> to lowercase. */
void *
strmap_get_lc(const strmap_t *map, const char *key)
{
  void *v;
  char *lc_key = tor_strdup(key);
  tor_strlower(lc_key);
  v = strmap_get(map,lc_key);
  tor_free(lc_key);
  return v;
}

/** Same as strmap_remove, but first converts <b>key</b> to lowercase */
void *
strmap_remove_lc(strmap_t *map, const char *key)
{
  void *v;
  char *lc_key = tor_strdup(key);
  tor_strlower(lc_key);
  v = strmap_remove(map,lc_key);
  tor_free(lc_key);
  return v;
}
