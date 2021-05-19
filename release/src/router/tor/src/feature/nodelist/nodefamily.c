/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file nodefamily.c
 * \brief Code to manipulate encoded, reference-counted node families.  We
 *  use these tricks to save space, since these families would otherwise
 *  require a large number of tiny allocations.
 **/

#include "core/or/or.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/nodefamily.h"
#include "feature/nodelist/nodefamily_st.h"
#include "feature/nodelist/nodelist.h"
#include "feature/relay/router.h"
#include "feature/nodelist/routerlist.h"

#include "ht.h"
#include "siphash.h"

#include "lib/container/smartlist.h"
#include "lib/ctime/di_ops.h"
#include "lib/defs/digest_sizes.h"
#include "lib/log/util_bug.h"

#include <stdlib.h>
#include <string.h>

/**
 * Allocate and return a blank node family with space to hold <b>n_members</b>
 * members.
 */
static nodefamily_t *
nodefamily_alloc(int n_members)
{
  size_t alloc_len = offsetof(nodefamily_t, family_members) +
    NODEFAMILY_ARRAY_SIZE(n_members);
  nodefamily_t *nf = tor_malloc_zero(alloc_len);
  nf->n_members = n_members;
  return nf;
}

/**
 * Hashtable hash implementation.
 */
static inline unsigned int
nodefamily_hash(const nodefamily_t *nf)
{
  return (unsigned) siphash24g(nf->family_members,
                               NODEFAMILY_ARRAY_SIZE(nf->n_members));
}

/**
 * Hashtable equality implementation.
 */
static inline unsigned int
nodefamily_eq(const nodefamily_t *a, const nodefamily_t *b)
{
  return (a->n_members == b->n_members) &&
    fast_memeq(a->family_members, b->family_members,
               NODEFAMILY_ARRAY_SIZE(a->n_members));
}

static HT_HEAD(nodefamily_map, nodefamily_t) the_node_families
  = HT_INITIALIZER();

HT_PROTOTYPE(nodefamily_map, nodefamily_t, ht_ent, nodefamily_hash,
             nodefamily_eq);
HT_GENERATE2(nodefamily_map, nodefamily_t, ht_ent, nodefamily_hash,
             node_family_eq, 0.6, tor_reallocarray_, tor_free_);

/**
 * Parse the family declaration in <b>s</b>, returning the canonical
 * <b>nodefamily_t</b> for its members.  Return NULL on error.
 *
 * If <b>rsa_id_self</b> is provided, it is a DIGEST_LEN-byte digest
 * for the router that declared this family: insert it into the
 * family declaration if it is not there already.
 *
 * If NF_WARN_MALFORMED is set in <b>flags</b>, warn about any
 * elements that we can't parse.  (By default, we log at info.)
 *
 * If NF_REJECT_MALFORMED is set in <b>flags</b>, treat any unparseable
 * elements as an error. (By default, we simply omit them.)
 **/
nodefamily_t *
nodefamily_parse(const char *s, const uint8_t *rsa_id_self,
                 unsigned flags)
{
  smartlist_t *sl = smartlist_new();
  smartlist_split_string(sl, s, NULL, SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  nodefamily_t *result = nodefamily_from_members(sl, rsa_id_self, flags, NULL);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);
  return result;
}

/**
 * Canonicalize the family list <b>s</b>, returning a newly allocated string.
 *
 * The canonicalization rules are fully specified in dir-spec.txt, but,
 * briefly: $hexid entries are put in caps, $hexid[=~]foo entries are
 * truncated, nicknames are put into lowercase, unrecognized entries are left
 * alone, and everything is sorted.
 **/
char *
nodefamily_canonicalize(const char *s, const uint8_t *rsa_id_self,
                        unsigned flags)
{
  smartlist_t *sl = smartlist_new();
  smartlist_t *result_members = smartlist_new();
  smartlist_split_string(sl, s, NULL, SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  nodefamily_t *nf = nodefamily_from_members(sl, rsa_id_self, flags,
                                             result_members);

  char *formatted = nodefamily_format(nf);
  smartlist_split_string(result_members, formatted, NULL,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  smartlist_sort_strings(result_members);
  char *combined = smartlist_join_strings(result_members, " ", 0, NULL);

  nodefamily_free(nf);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);
  SMARTLIST_FOREACH(result_members, char *, cp, tor_free(cp));
  smartlist_free(result_members);
  tor_free(formatted);

  return combined;
}

/**
 * qsort helper for encoded nodefamily elements.
 **/
static int
compare_members(const void *a, const void *b)
{
  return fast_memcmp(a, b, NODEFAMILY_MEMBER_LEN);
}

/**
 * Parse the member strings in <b>members</b>, returning their canonical
 * <b>nodefamily_t</b>.  Return NULL on error.
 *
 * If <b>rsa_id_self</b> is provided, it is a DIGEST_LEN-byte digest
 * for the router that declared this family: insert it into the
 * family declaration if it is not there already.
 *
 * The <b>flags</b> element is interpreted as in nodefamily_parse().
 *
 * If <b>unrecognized</b> is provided, fill it copies of any unrecognized
 * members.  (Note that malformed $hexids are not considered unrecognized.)
 **/
nodefamily_t *
nodefamily_from_members(const smartlist_t *members,
                        const uint8_t *rsa_id_self,
                        unsigned flags,
                        smartlist_t *unrecognized_out)
{
  const int n_self = rsa_id_self ? 1 : 0;
  int n_bad_elements = 0;
  int n_members = smartlist_len(members) + n_self;
  nodefamily_t *tmp = nodefamily_alloc(n_members);
  uint8_t *ptr = NODEFAMILY_MEMBER_PTR(tmp, 0);

  SMARTLIST_FOREACH_BEGIN(members, const char *, cp) {
    bool bad_element = true;
    if (is_legal_nickname(cp)) {
      ptr[0] = NODEFAMILY_BY_NICKNAME;
      tor_assert(strlen(cp) < DIGEST_LEN); // guaranteed by is_legal_nickname
      memcpy(ptr+1, cp, strlen(cp));
      tor_strlower((char*) ptr+1);
      bad_element = false;
    } else if (is_legal_hexdigest(cp)) {
      char digest_buf[DIGEST_LEN];
      char nn_buf[MAX_NICKNAME_LEN+1];
      char nn_char=0;
      if (hex_digest_nickname_decode(cp, digest_buf, &nn_char, nn_buf)==0) {
        bad_element = false;
        ptr[0] = NODEFAMILY_BY_RSA_ID;
        memcpy(ptr+1, digest_buf, DIGEST_LEN);
      }
    } else {
      if (unrecognized_out)
        smartlist_add_strdup(unrecognized_out, cp);
    }

    if (bad_element) {
      const int severity = (flags & NF_WARN_MALFORMED) ? LOG_WARN : LOG_INFO;
      log_fn(severity, LD_GENERAL,
             "Bad element %s while parsing a node family.",
             escaped(cp));
      ++n_bad_elements;
    } else {
      ptr += NODEFAMILY_MEMBER_LEN;
    }
  } SMARTLIST_FOREACH_END(cp);

  if (n_bad_elements && (flags & NF_REJECT_MALFORMED))
    goto err;

  if (rsa_id_self) {
    /* Add self. */
    ptr[0] = NODEFAMILY_BY_RSA_ID;
    memcpy(ptr+1, rsa_id_self, DIGEST_LEN);
  }

  n_members -= n_bad_elements;

  /* Sort tmp into canonical order. */
  qsort(tmp->family_members, n_members, NODEFAMILY_MEMBER_LEN,
        compare_members);

  /* Remove duplicates. */
  int i;
  for (i = 0; i < n_members-1; ++i) {
    uint8_t *thisptr = NODEFAMILY_MEMBER_PTR(tmp, i);
    uint8_t *nextptr = NODEFAMILY_MEMBER_PTR(tmp, i+1);
    if (fast_memeq(thisptr, nextptr, NODEFAMILY_MEMBER_LEN)) {
      memmove(thisptr, nextptr, (n_members-i-1)*NODEFAMILY_MEMBER_LEN);
      --n_members;
      --i;
    }
  }
  int n_members_alloc = tmp->n_members;
  tmp->n_members = n_members;

  /* See if we already allocated this family. */
  nodefamily_t *found = HT_FIND(nodefamily_map, &the_node_families, tmp);
  if (found) {
    /* If we did, great: incref it and return it. */
    ++found->refcnt;
    tor_free(tmp);
    return found;
  } else {
    /* If not, insert it into the hashtable. */
    if (n_members_alloc != n_members) {
      /* Compact the family if needed */
      nodefamily_t *tmp2 = nodefamily_alloc(n_members);
      memcpy(tmp2->family_members, tmp->family_members,
             n_members * NODEFAMILY_MEMBER_LEN);
      tor_free(tmp);
      tmp = tmp2;
    }

    tmp->refcnt = 1;
    HT_INSERT(nodefamily_map, &the_node_families, tmp);
    return tmp;
  }

 err:
  tor_free(tmp);
  return NULL;
}

/**
 * Drop our reference to <b>family</b>, freeing it if there are no more
 * references.
 */
void
nodefamily_free_(nodefamily_t *family)
{
  if (family == NULL)
    return;

  --family->refcnt;

  if (family->refcnt == 0) {
    HT_REMOVE(nodefamily_map, &the_node_families, family);
    tor_free(family);
  }
}

/**
 * Return true iff <b>family</b> contains the SHA1 RSA1024 identity
 * <b>rsa_id</b>.
 */
bool
nodefamily_contains_rsa_id(const nodefamily_t *family,
                           const uint8_t *rsa_id)
{
  if (family == NULL)
    return false;

  unsigned i;
  for (i = 0; i < family->n_members; ++i) {
    const uint8_t *ptr = NODEFAMILY_MEMBER_PTR(family, i);
    if (ptr[0] == NODEFAMILY_BY_RSA_ID &&
        fast_memeq(ptr+1, rsa_id, DIGEST_LEN)) {
      return true;
    }
  }
  return false;
}

/**
 * Return true iff <b>family</b> contains the nickname <b>name</b>.
 */
bool
nodefamily_contains_nickname(const nodefamily_t *family,
                             const char *name)
{
  if (family == NULL)
    return false;

  unsigned i;
  for (i = 0; i < family->n_members; ++i) {
    const uint8_t *ptr = NODEFAMILY_MEMBER_PTR(family, i);
    // note that the strcasecmp() is safe because there is always at least one
    // NUL in the encoded nickname, because all legal nicknames are less than
    // DIGEST_LEN bytes long.
    if (ptr[0] == NODEFAMILY_BY_NICKNAME && !strcasecmp((char*)ptr+1, name)) {
      return true;
    }
  }
  return false;
}

/**
 * Return true if <b>family</b> contains the nickname or the RSA ID for
 * <b>node</b>
 **/
bool
nodefamily_contains_node(const nodefamily_t *family,
                         const node_t *node)
{
  return
    nodefamily_contains_nickname(family, node_get_nickname(node))
    ||
    nodefamily_contains_rsa_id(family, node_get_rsa_id_digest(node));
}

/**
 * Look up every entry in <b>family</b>, and add add the corresponding
 * node_t to <b>out</b>.
 **/
void
nodefamily_add_nodes_to_smartlist(const nodefamily_t *family,
                                  smartlist_t *out)
{
  if (!family)
    return;

  unsigned i;
  for (i = 0; i < family->n_members; ++i) {
    const uint8_t *ptr = NODEFAMILY_MEMBER_PTR(family, i);
    const node_t *node = NULL;
    switch (ptr[0]) {
      case NODEFAMILY_BY_NICKNAME:
        node = node_get_by_nickname((char*)ptr+1, NNF_NO_WARN_UNNAMED);
        break;
      case NODEFAMILY_BY_RSA_ID:
        node = node_get_by_id((char*)ptr+1);
        break;
      default:
        /* LCOV_EXCL_START */
        tor_assert_nonfatal_unreached();
        break;
        /* LCOV_EXCL_STOP */
    }
    if (node)
      smartlist_add(out, (void *)node);
  }
}

/**
 * Encode <b>family</b> as a space-separated string.
 */
char *
nodefamily_format(const nodefamily_t *family)
{
  if (!family)
    return tor_strdup("");

  unsigned i;
  smartlist_t *sl = smartlist_new();
  for (i = 0; i < family->n_members; ++i) {
    const uint8_t *ptr = NODEFAMILY_MEMBER_PTR(family, i);
    switch (ptr[0]) {
      case NODEFAMILY_BY_NICKNAME:
        smartlist_add_strdup(sl, (char*)ptr+1);
        break;
      case NODEFAMILY_BY_RSA_ID: {
        char buf[HEX_DIGEST_LEN+2];
        buf[0]='$';
        base16_encode(buf+1, sizeof(buf)-1, (char*)ptr+1, DIGEST_LEN);
        tor_strupper(buf);
        smartlist_add_strdup(sl, buf);
        break;
      }
      default:
        /* LCOV_EXCL_START */
        tor_assert_nonfatal_unreached();
        break;
        /* LCOV_EXCL_STOP */
    }
  }

  char *result = smartlist_join_strings(sl, " ", 0, NULL);
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);
  return result;
}

/**
 * Free all storage held in the nodefamily map.
 **/
void
nodefamily_free_all(void)
{
  HT_CLEAR(nodefamily_map, &the_node_families);
}
