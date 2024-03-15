/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_cache.c
 * \brief Handle hidden service descriptor caches.
 **/

/* For unit tests.*/
#define HS_CACHE_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/hs/hs_ident.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_descriptor.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/stats/rephist.h"

#include "feature/hs/hs_cache.h"

#include "feature/nodelist/networkstatus_st.h"

/* Total counter of the cache size. */
static size_t hs_cache_total_allocation = 0;

static int cached_client_descriptor_has_expired(time_t now,
           const hs_cache_client_descriptor_t *cached_desc);

/** Helper function: Return true iff the cache entry has a decrypted
 * descriptor.
 *
 * A NULL desc object in the entry means that we were not able to decrypt the
 * descriptor because we are likely lacking client authorization.  It is still
 * a valid entry but some operations can't be done without the decrypted
 * descriptor thus this function MUST be used to safe guard access to the
 * decrypted desc object. */
static inline bool
entry_has_decrypted_descriptor(const hs_cache_client_descriptor_t *entry)
{
  tor_assert(entry);
  return (entry->desc != NULL);
}

/********************** Directory HS cache ******************/

/** Directory descriptor cache. Map indexed by blinded key. */
static digest256map_t *hs_cache_v3_dir;

/** Remove a given descriptor from our cache. */
static void
remove_v3_desc_as_dir(const hs_cache_dir_descriptor_t *desc)
{
  tor_assert(desc);
  digest256map_remove(hs_cache_v3_dir, desc->key);
}

/** Store a given descriptor in our cache. */
static void
store_v3_desc_as_dir(hs_cache_dir_descriptor_t *desc)
{
  tor_assert(desc);
  digest256map_set(hs_cache_v3_dir, desc->key, desc);
}

/** Query our cache and return the entry or NULL if not found. */
static hs_cache_dir_descriptor_t *
lookup_v3_desc_as_dir(const uint8_t *key)
{
  tor_assert(key);
  return digest256map_get(hs_cache_v3_dir, key);
}

#define cache_dir_desc_free(val) \
  FREE_AND_NULL(hs_cache_dir_descriptor_t, cache_dir_desc_free_, (val))

/** Free a directory descriptor object. */
static void
cache_dir_desc_free_(hs_cache_dir_descriptor_t *desc)
{
  if (desc == NULL) {
    return;
  }
  hs_desc_plaintext_data_free(desc->plaintext_data);
  tor_free(desc->encoded_desc);
  tor_free(desc);
}

/** Helper function: Use by the free all function using the digest256map
 * interface to cache entries. */
static void
cache_dir_desc_free_void(void *ptr)
{
  cache_dir_desc_free_(ptr);
}

/** Create a new directory cache descriptor object from a encoded descriptor.
 * On success, return the heap-allocated cache object, otherwise return NULL if
 * we can't decode the descriptor. */
static hs_cache_dir_descriptor_t *
cache_dir_desc_new(const char *desc)
{
  hs_cache_dir_descriptor_t *dir_desc;

  tor_assert(desc);

  dir_desc = tor_malloc_zero(sizeof(hs_cache_dir_descriptor_t));
  dir_desc->plaintext_data =
    tor_malloc_zero(sizeof(hs_desc_plaintext_data_t));
  dir_desc->encoded_desc = tor_strdup(desc);

  if (hs_desc_decode_plaintext(desc, dir_desc->plaintext_data) < 0) {
    log_debug(LD_DIR, "Unable to decode descriptor. Rejecting.");
    goto err;
  }

  /* The blinded pubkey is the indexed key. */
  dir_desc->key = dir_desc->plaintext_data->blinded_pubkey.pubkey;
  dir_desc->created_ts = time(NULL);
  return dir_desc;

 err:
  cache_dir_desc_free(dir_desc);
  return NULL;
}

/** Return the size of a cache entry in bytes. */
static size_t
cache_get_dir_entry_size(const hs_cache_dir_descriptor_t *entry)
{
  return (sizeof(*entry) + hs_desc_plaintext_obj_size(entry->plaintext_data)
          + strlen(entry->encoded_desc));
}

/** Try to store a valid version 3 descriptor in the directory cache. Return 0
 * on success else a negative value is returned indicating that we have a
 * newer version in our cache. On error, caller is responsible to free the
 * given descriptor desc. */
static int
cache_store_v3_as_dir(hs_cache_dir_descriptor_t *desc)
{
  hs_cache_dir_descriptor_t *cache_entry;

  tor_assert(desc);

  /* Verify if we have an entry in the cache for that key and if yes, check
   * if we should replace it? */
  cache_entry = lookup_v3_desc_as_dir(desc->key);
  if (cache_entry != NULL) {
    /* Only replace descriptor if revision-counter is greater than the one
     * in our cache */
    if (cache_entry->plaintext_data->revision_counter >=
        desc->plaintext_data->revision_counter) {
      log_info(LD_REND, "Descriptor revision counter in our cache is "
               "greater or equal than the one we received (%d/%d). "
               "Rejecting!",
               (int)cache_entry->plaintext_data->revision_counter,
               (int)desc->plaintext_data->revision_counter);
      goto err;
    }
    /* We now know that the descriptor we just received is a new one so
     * remove the entry we currently have from our cache so we can then
     * store the new one. */
    remove_v3_desc_as_dir(cache_entry);
    hs_cache_decrement_allocation(cache_get_dir_entry_size(cache_entry));
    cache_dir_desc_free(cache_entry);
  }
  /* Store the descriptor we just got. We are sure here that either we
   * don't have the entry or we have a newer descriptor and the old one
   * has been removed from the cache. */
  store_v3_desc_as_dir(desc);

  /* Update our total cache size with this entry for the OOM. This uses the
   * old HS protocol cache subsystem for which we are tied with. */
  hs_cache_increment_allocation(cache_get_dir_entry_size(desc));

  /* Update HSv3 statistics */
  if (get_options()->HiddenServiceStatistics) {
    rep_hist_hsdir_stored_maybe_new_v3_onion(desc->key);
  }

  return 0;

 err:
  return -1;
}

/** Using the query which is the base64 encoded blinded key of a version 3
 * descriptor, lookup in our directory cache the entry. If found, 1 is
 * returned and desc_out is populated with a newly allocated string being the
 * encoded descriptor. If not found, 0 is returned and desc_out is untouched.
 * On error, a negative value is returned and desc_out is untouched. */
static int
cache_lookup_v3_as_dir(const char *query, const char **desc_out)
{
  int found = 0;
  ed25519_public_key_t blinded_key;
  const hs_cache_dir_descriptor_t *entry;

  tor_assert(query);

  /* Decode blinded key using the given query value. */
  if (ed25519_public_from_base64(&blinded_key, query) < 0) {
    log_info(LD_REND, "Unable to decode the v3 HSDir query %s.",
             safe_str_client(query));
    goto err;
  }

  entry = lookup_v3_desc_as_dir(blinded_key.pubkey);
  if (entry != NULL) {
    found = 1;
    if (desc_out) {
      *desc_out = entry->encoded_desc;
    }
  }

  return found;

 err:
  return -1;
}

/** Clean the v3 cache by removing any entry that has expired using the
 * <b>global_cutoff</b> value. If <b>global_cutoff</b> is 0, the cleaning
 * process will use the lifetime found in the plaintext data section. Return
 * the number of bytes cleaned. */
STATIC size_t
cache_clean_v3_as_dir(time_t now, time_t global_cutoff)
{
  size_t bytes_removed = 0;

  /* Code flow error if this ever happens. */
  tor_assert(global_cutoff >= 0);

  if (!hs_cache_v3_dir) { /* No cache to clean. Just return. */
    return 0;
  }

  DIGEST256MAP_FOREACH_MODIFY(hs_cache_v3_dir, key,
                              hs_cache_dir_descriptor_t *, entry) {
    size_t entry_size;
    time_t cutoff = global_cutoff;
    if (!cutoff) {
      /* Cutoff is the lifetime of the entry found in the descriptor. */
      cutoff = now - entry->plaintext_data->lifetime_sec;
    }

    /* If the entry has been created _after_ the cutoff, not expired so
     * continue to the next entry in our v3 cache. */
    if (entry->created_ts > cutoff) {
      continue;
    }
    /* Here, our entry has expired, remove and free. */
    MAP_DEL_CURRENT(key);
    entry_size = cache_get_dir_entry_size(entry);
    bytes_removed += entry_size;
    /* Entry is not in the cache anymore, destroy it. */
    cache_dir_desc_free(entry);
    /* Update our cache entry allocation size for the OOM. */
    hs_cache_decrement_allocation(entry_size);
    /* Logging. */
    {
      char key_b64[BASE64_DIGEST256_LEN + 1];
      digest256_to_base64(key_b64, (const char *) key);
      log_info(LD_REND, "Removing v3 descriptor '%s' from HSDir cache",
               safe_str_client(key_b64));
    }
  } DIGEST256MAP_FOREACH_END;

  return bytes_removed;
}

/** Given an encoded descriptor, store it in the directory cache depending on
 * which version it is. Return a negative value on error. On success, 0 is
 * returned. */
int
hs_cache_store_as_dir(const char *desc)
{
  hs_cache_dir_descriptor_t *dir_desc = NULL;

  tor_assert(desc);

  /* Create a new cache object. This can fail if the descriptor plaintext data
   * is unparseable which in this case a log message will be triggered. */
  dir_desc = cache_dir_desc_new(desc);
  if (dir_desc == NULL) {
    goto err;
  }

  /* Call the right function against the descriptor version. At this point,
   * we are sure that the descriptor's version is supported else the
   * decoding would have failed. */
  switch (dir_desc->plaintext_data->version) {
  case HS_VERSION_THREE:
  default:
    if (cache_store_v3_as_dir(dir_desc) < 0) {
      goto err;
    }
    break;
  }
  return 0;

 err:
  cache_dir_desc_free(dir_desc);
  return -1;
}

/** Using the query, lookup in our directory cache the entry. If found, 1 is
 * returned and desc_out is populated with a newly allocated string being
 * the encoded descriptor. If not found, 0 is returned and desc_out is
 * untouched. On error, a negative value is returned and desc_out is
 * untouched. */
int
hs_cache_lookup_as_dir(uint32_t version, const char *query,
                       const char **desc_out)
{
  int found;

  tor_assert(query);
  /* This should never be called with an unsupported version. */
  tor_assert(hs_desc_is_supported_version(version));

  switch (version) {
  case HS_VERSION_THREE:
  default:
    found = cache_lookup_v3_as_dir(query, desc_out);
    break;
  }

  return found;
}

/** Clean all directory caches using the current time now. */
void
hs_cache_clean_as_dir(time_t now)
{
  /* Now, clean the v3 cache. Set the cutoff to 0 telling the cleanup function
   * to compute the cutoff by itself using the lifetime value. */
  cache_clean_v3_as_dir(now, 0);
}

/********************** Client-side HS cache ******************/

/** Client-side HS descriptor cache. Map indexed by service identity key. */
static digest256map_t *hs_cache_v3_client;

/** Client-side introduction point state cache. Map indexed by service public
 * identity key (onion address). It contains hs_cache_client_intro_state_t
 * objects all related to a specific service. */
static digest256map_t *hs_cache_client_intro_state;

#define cache_client_desc_free(val) \
  FREE_AND_NULL(hs_cache_client_descriptor_t, cache_client_desc_free_, (val))

/** Free memory allocated by <b>desc</b>. */
static void
cache_client_desc_free_(hs_cache_client_descriptor_t *desc)
{
  if (desc == NULL) {
    return;
  }
  hs_descriptor_free(desc->desc);
  memwipe(&desc->key, 0, sizeof(desc->key));
  memwipe(desc->encoded_desc, 0, strlen(desc->encoded_desc));
  tor_free(desc->encoded_desc);
  tor_free(desc);
}

/** Helper function: Use by the free all function to clear the client cache */
static void
cache_client_desc_free_void(void *ptr)
{
  hs_cache_client_descriptor_t *desc = ptr;
  cache_client_desc_free(desc);
}

/** Return the size of a client cache entry in bytes. */
static size_t
cache_get_client_entry_size(const hs_cache_client_descriptor_t *entry)
{
  size_t size = 0;

  if (entry == NULL) {
    goto end;
  }
  size += sizeof(*entry);

  if (entry->encoded_desc) {
    size += strlen(entry->encoded_desc);
  }

  if (entry_has_decrypted_descriptor(entry)) {
    size += hs_desc_obj_size(entry->desc);
  }

 end:
  return size;
}

/** Remove a given descriptor from our cache. */
static void
remove_v3_desc_as_client(const hs_cache_client_descriptor_t *desc)
{
  tor_assert(desc);
  digest256map_remove(hs_cache_v3_client, desc->key.pubkey);
  /* Update cache size with this entry for the OOM handler. */
  hs_cache_decrement_allocation(cache_get_client_entry_size(desc));
}

/** Store a given descriptor in our cache. */
static void
store_v3_desc_as_client(hs_cache_client_descriptor_t *desc)
{
  hs_cache_client_descriptor_t *cached_desc;

  tor_assert(desc);

  /* Because the lookup function doesn't return an expired entry, it can linger
   * in the cache until we clean it up or a new descriptor is stored. So,
   * before adding, we'll make sure we are not overwriting an old descriptor
   * (which is OK in terms of semantic) but leads to memory leak. */
  cached_desc = digest256map_get(hs_cache_v3_client, desc->key.pubkey);
  if (cached_desc) {
    cache_client_desc_free(cached_desc);
  }
  digest256map_set(hs_cache_v3_client, desc->key.pubkey, desc);
  /* Update cache size with this entry for the OOM handler. */
  hs_cache_increment_allocation(cache_get_client_entry_size(desc));
}

/** Query our cache and return the entry or NULL if not found or if expired. */
STATIC hs_cache_client_descriptor_t *
lookup_v3_desc_as_client(const uint8_t *key)
{
  time_t now = approx_time();
  hs_cache_client_descriptor_t *cached_desc;

  tor_assert(key);

  /* Do the lookup */
  cached_desc = digest256map_get(hs_cache_v3_client, key);
  if (!cached_desc) {
    return NULL;
  }

  /* Don't return expired entries */
  if (cached_client_descriptor_has_expired(now, cached_desc)) {
    return NULL;
  }

  return cached_desc;
}

/** Parse the encoded descriptor in <b>desc_str</b> using
 * <b>service_identity_pk</b> to decrypt it first.
 *
 * If everything goes well, allocate and return a new
 * hs_cache_client_descriptor_t object. In case of error, return NULL. */
static hs_cache_client_descriptor_t *
cache_client_desc_new(const char *desc_str,
                      const ed25519_public_key_t *service_identity_pk,
                      hs_desc_decode_status_t *decode_status_out)
{
  hs_desc_decode_status_t ret;
  hs_descriptor_t *desc = NULL;
  hs_cache_client_descriptor_t *client_desc = NULL;

  tor_assert(desc_str);
  tor_assert(service_identity_pk);

  /* Decode the descriptor we just fetched. */
  ret = hs_client_decode_descriptor(desc_str, service_identity_pk, &desc);
  if (ret != HS_DESC_DECODE_OK &&
      ret != HS_DESC_DECODE_NEED_CLIENT_AUTH &&
      ret != HS_DESC_DECODE_BAD_CLIENT_AUTH) {
    /* In the case of a missing or bad client authorization, we'll keep the
     * descriptor in the cache because those credentials can arrive later. */
    goto end;
  }
  /* Make sure we do have a descriptor if decoding was successful. */
  if (ret == HS_DESC_DECODE_OK) {
    tor_assert(desc);
  } else {
    if (BUG(desc != NULL)) {
      /* We are not suppose to have a descriptor if the decoding code is not
       * indicating success. Just in case, bail early to recover. */
      goto end;
    }
  }

  /* All is good: make a cache object for this descriptor */
  client_desc = tor_malloc_zero(sizeof(hs_cache_client_descriptor_t));
  ed25519_pubkey_copy(&client_desc->key, service_identity_pk);
  /* Set expiration time for this cached descriptor to be the start of the next
   * time period since that's when clients need to start using the next blinded
   * pk of the service (and hence will need its next descriptor). */
  client_desc->expiration_ts = hs_get_start_time_of_next_time_period(0);
  client_desc->desc = desc;
  client_desc->encoded_desc = tor_strdup(desc_str);

 end:
  if (decode_status_out) {
    *decode_status_out = ret;
  }
  return client_desc;
}

/** Return a newly allocated and initialized hs_cache_intro_state_t object. */
static hs_cache_intro_state_t *
cache_intro_state_new(void)
{
  hs_cache_intro_state_t *state = tor_malloc_zero(sizeof(*state));
  state->created_ts = approx_time();
  return state;
}

#define cache_intro_state_free(val) \
  FREE_AND_NULL(hs_cache_intro_state_t, cache_intro_state_free_, (val))

/** Free an hs_cache_intro_state_t object. */
static void
cache_intro_state_free_(hs_cache_intro_state_t *state)
{
  tor_free(state);
}

/** Helper function: used by the free all function. */
static void
cache_intro_state_free_void(void *state)
{
  cache_intro_state_free_(state);
}

/** Return a newly allocated and initialized hs_cache_client_intro_state_t
 * object. */
static hs_cache_client_intro_state_t *
cache_client_intro_state_new(void)
{
  hs_cache_client_intro_state_t *cache = tor_malloc_zero(sizeof(*cache));
  cache->intro_points = digest256map_new();
  return cache;
}

#define cache_client_intro_state_free(val)              \
  FREE_AND_NULL(hs_cache_client_intro_state_t,          \
                cache_client_intro_state_free_, (val))

/** Free a cache_client_intro_state object. */
static void
cache_client_intro_state_free_(hs_cache_client_intro_state_t *cache)
{
  if (cache == NULL) {
    return;
  }
  digest256map_free(cache->intro_points, cache_intro_state_free_void);
  tor_free(cache);
}

/** Helper function: used by the free all function. */
static void
cache_client_intro_state_free_void(void *entry)
{
  cache_client_intro_state_free_(entry);
}

/** For the given service identity key service_pk and an introduction
 * authentication key auth_key, lookup the intro state object. Return 1 if
 * found and put it in entry if not NULL. Return 0 if not found and entry is
 * untouched. */
static int
cache_client_intro_state_lookup(const ed25519_public_key_t *service_pk,
                                const ed25519_public_key_t *auth_key,
                                hs_cache_intro_state_t **entry)
{
  hs_cache_intro_state_t *state;
  hs_cache_client_intro_state_t *cache;

  tor_assert(service_pk);
  tor_assert(auth_key);
  tor_assert_nonfatal(!ed25519_public_key_is_zero(service_pk));
  tor_assert_nonfatal(!ed25519_public_key_is_zero(auth_key));

  /* Lookup the intro state cache for this service key. */
  cache = digest256map_get(hs_cache_client_intro_state, service_pk->pubkey);
  if (cache == NULL) {
    goto not_found;
  }

  /* From the cache we just found for the service, lookup in the introduction
   * points map for the given authentication key. */
  state = digest256map_get(cache->intro_points, auth_key->pubkey);
  if (state == NULL) {
    goto not_found;
  }
  if (entry) {
    *entry = state;
  }
  return 1;
 not_found:
  return 0;
}

/** Note the given failure in state. */
static void
cache_client_intro_state_note(hs_cache_intro_state_t *state,
                              rend_intro_point_failure_t failure)
{
  tor_assert(state);
  switch (failure) {
  case INTRO_POINT_FAILURE_GENERIC:
    state->error = 1;
    break;
  case INTRO_POINT_FAILURE_TIMEOUT:
    state->timed_out = 1;
    break;
  case INTRO_POINT_FAILURE_UNREACHABLE:
    state->unreachable_count++;
    break;
  default:
    tor_assert_nonfatal_unreached();
    return;
  }
}

/** For the given service identity key service_pk and an introduction
 * authentication key auth_key, add an entry in the client intro state cache
 * If no entry exists for the service, it will create one. If state is non
 * NULL, it will point to the new intro state entry. */
static void
cache_client_intro_state_add(const ed25519_public_key_t *service_pk,
                             const ed25519_public_key_t *auth_key,
                             hs_cache_intro_state_t **state)
{
  hs_cache_intro_state_t *entry, *old_entry;
  hs_cache_client_intro_state_t *cache;

  tor_assert(service_pk);
  tor_assert(auth_key);

  /* Lookup the state cache for this service key. */
  cache = digest256map_get(hs_cache_client_intro_state, service_pk->pubkey);
  if (cache == NULL) {
    cache = cache_client_intro_state_new();
    digest256map_set(hs_cache_client_intro_state, service_pk->pubkey, cache);
  }

  entry = cache_intro_state_new();
  old_entry = digest256map_set(cache->intro_points, auth_key->pubkey, entry);
  /* This should never happened because the code flow is to lookup the entry
   * before adding it. But, just in case, non fatal assert and free it. */
  tor_assert_nonfatal(old_entry == NULL);
  tor_free(old_entry);

  if (state) {
    *state = entry;
  }
}

/** Remove every intro point state entry from cache that has been created
 * before or at the cutoff. */
static void
cache_client_intro_state_clean(time_t cutoff,
                               hs_cache_client_intro_state_t *cache)
{
  tor_assert(cache);

  DIGEST256MAP_FOREACH_MODIFY(cache->intro_points, key,
                              hs_cache_intro_state_t *, entry) {
    if (entry->created_ts <= cutoff) {
      cache_intro_state_free(entry);
      MAP_DEL_CURRENT(key);
    }
  } DIGEST256MAP_FOREACH_END;
}

/** Return true iff no intro points are in this cache. */
static int
cache_client_intro_state_is_empty(const hs_cache_client_intro_state_t *cache)
{
  return digest256map_isempty(cache->intro_points);
}

/** Check whether <b>client_desc</b> is useful for us, and store it in the
 *  client-side HS cache if so. The client_desc is freed if we already have a
 *  fresher (higher revision counter count) in the cache. */
static int
cache_store_as_client(hs_cache_client_descriptor_t *client_desc)
{
  hs_cache_client_descriptor_t *cache_entry;

  /* TODO: Heavy code duplication with cache_store_as_dir(). Consider
   * refactoring and uniting! */

  tor_assert(client_desc);

  /* Check if we already have a descriptor from this HS in cache. If we do,
   * check if this descriptor is newer than the cached one only if we have a
   * decoded descriptor. We do keep non-decoded descriptor that requires
   * client authorization. */
  cache_entry = lookup_v3_desc_as_client(client_desc->key.pubkey);
  if (cache_entry != NULL) {
    /* If the current or the new cache entry don't have a decrypted descriptor
     * (missing client authorization), we always replace the current one with
     * the new one. Reason is that we can't inspect the revision counter
     * within the plaintext data so we blindly replace. */
    if (!entry_has_decrypted_descriptor(cache_entry) ||
        !entry_has_decrypted_descriptor(client_desc)) {
      remove_v3_desc_as_client(cache_entry);
      cache_client_desc_free(cache_entry);
      goto store;
    }

    /* From this point on, we know that the decrypted descriptor is in the
     * current entry and new object thus safe to access. */

    /* If we have an entry in our cache that has a revision counter greater
     * than the one we just fetched, discard the one we fetched. */
    if (cache_entry->desc->plaintext_data.revision_counter >
        client_desc->desc->plaintext_data.revision_counter) {
      cache_client_desc_free(client_desc);
      goto done;
    }
    /* Remove old entry. Make space for the new one! */
    remove_v3_desc_as_client(cache_entry);

    /* We just removed an old descriptor and will replace it. We'll close all
     * intro circuits related to this old one so we don't have leftovers. We
     * leave the rendezvous circuits opened because they could be in use. */
    hs_client_close_intro_circuits_from_desc(cache_entry->desc);

    /* Free it. */
    cache_client_desc_free(cache_entry);
  }

 store:
  /* Store descriptor in cache */
  store_v3_desc_as_client(client_desc);

 done:
  return 0;
}

/** Return true iff the cached client descriptor at <b>cached_desc</b> has
 * expired. */
static int
cached_client_descriptor_has_expired(time_t now,
                               const hs_cache_client_descriptor_t *cached_desc)
{
  /* We use the current consensus time to see if we should expire this
   * descriptor since we use consensus time for all other parts of the protocol
   * as well (e.g. to build the blinded key and compute time periods). */
  const networkstatus_t *ns =
    networkstatus_get_reasonably_live_consensus(now,
      usable_consensus_flavor());
  /* If we don't have a recent consensus, consider this entry expired since we
   * will want to fetch a new HS desc when we get a live consensus. */
  if (!ns) {
    return 1;
  }

  if (cached_desc->expiration_ts <= ns->valid_after) {
    return 1;
  }

  return 0;
}

/** clean the client cache using now as the current time. Return the total size
 * of removed bytes from the cache. */
static size_t
cache_clean_v3_as_client(time_t now)
{
  size_t bytes_removed = 0;

  if (!hs_cache_v3_client) { /* No cache to clean. Just return. */
    return 0;
  }

  DIGEST256MAP_FOREACH_MODIFY(hs_cache_v3_client, key,
                              hs_cache_client_descriptor_t *, entry) {
    size_t entry_size;

    /* If the entry has not expired, continue to the next cached entry */
    if (!cached_client_descriptor_has_expired(now, entry)) {
      continue;
    }
    /* Here, our entry has expired, remove and free. */
    MAP_DEL_CURRENT(key);
    entry_size = cache_get_client_entry_size(entry);
    bytes_removed += entry_size;

    /* We just removed an old descriptor. We need to close all intro circuits
     * if the descriptor is decrypted so we don't have leftovers that can be
     * selected while lacking a descriptor. Circuits are selected by intro
     * authentication key thus we need the descriptor. We leave the rendezvous
     * circuits opened because they could be in use. */
    if (entry_has_decrypted_descriptor(entry)) {
      hs_client_close_intro_circuits_from_desc(entry->desc);
    }
    /* Entry is not in the cache anymore, destroy it. */
    cache_client_desc_free(entry);
    /* Update our OOM. We didn't use the remove() function because we are in
     * a loop so we have to explicitly decrement. */
    hs_cache_decrement_allocation(entry_size);
    /* Logging. */
    {
      char key_b64[BASE64_DIGEST256_LEN + 1];
      digest256_to_base64(key_b64, (const char *) key);
      log_info(LD_REND, "Removing hidden service v3 descriptor '%s' "
                        "from client cache",
               safe_str_client(key_b64));
    }
  } DIGEST256MAP_FOREACH_END;

  return bytes_removed;
}

/** Public API: Given the HS ed25519 identity public key in <b>key</b>, return
 *  its HS encoded descriptor if it's stored in our cache, or NULL if not. */
const char *
hs_cache_lookup_encoded_as_client(const ed25519_public_key_t *key)
{
  hs_cache_client_descriptor_t *cached_desc = NULL;

  tor_assert(key);

  cached_desc = lookup_v3_desc_as_client(key->pubkey);
  if (cached_desc) {
    tor_assert(cached_desc->encoded_desc);
    return cached_desc->encoded_desc;
  }

  return NULL;
}

/** Public API: Given the HS ed25519 identity public key in <b>key</b>, return
 *  its HS descriptor if it's stored in our cache, or NULL if not or if the
 *  descriptor was never decrypted. The later can happen if we are waiting for
 *  client authorization to be added. */
const hs_descriptor_t *
hs_cache_lookup_as_client(const ed25519_public_key_t *key)
{
  hs_cache_client_descriptor_t *cached_desc = NULL;

  tor_assert(key);

  cached_desc = lookup_v3_desc_as_client(key->pubkey);
  if (cached_desc && entry_has_decrypted_descriptor(cached_desc)) {
    return cached_desc->desc;
  }

  return NULL;
}

/** Public API: Given an encoded descriptor, store it in the client HS cache.
 *  Return a decode status which changes how we handle the SOCKS connection
 *  depending on its value:
 *
 *  HS_DESC_DECODE_OK: Returned on success. Descriptor was properly decoded
 *                     and is now stored.
 *
 *  HS_DESC_DECODE_NEED_CLIENT_AUTH: Client authorization is needed but the
 *                                   descriptor was still stored.
 *
 *  HS_DESC_DECODE_BAD_CLIENT_AUTH: Client authorization for this descriptor
 *                                  was not usable but the descriptor was
 *                                  still stored.
 *
 *  Any other codes means indicate where the error occurred and the descriptor
 *  was not stored. */
hs_desc_decode_status_t
hs_cache_store_as_client(const char *desc_str,
                         const ed25519_public_key_t *identity_pk)
{
  hs_desc_decode_status_t ret;
  hs_cache_client_descriptor_t *client_desc = NULL;

  tor_assert(desc_str);
  tor_assert(identity_pk);

  /* Create client cache descriptor object */
  client_desc = cache_client_desc_new(desc_str, identity_pk, &ret);
  if (!client_desc) {
    log_warn(LD_GENERAL, "HSDesc parsing failed!");
    log_debug(LD_GENERAL, "Failed to parse HSDesc: %s.", escaped(desc_str));
    goto err;
  }

  /* Push it to the cache */
  if (cache_store_as_client(client_desc) < 0) {
    ret = HS_DESC_DECODE_GENERIC_ERROR;
    goto err;
  }

  return ret;

 err:
  cache_client_desc_free(client_desc);
  return ret;
}

/** Remove and free a client cache descriptor entry for the given onion
 * service ed25519 public key. If the descriptor is decoded, the intro
 * circuits are closed if any.
 *
 * This does nothing if no descriptor exists for the given key. */
void
hs_cache_remove_as_client(const ed25519_public_key_t *key)
{
  hs_cache_client_descriptor_t *cached_desc = NULL;

  tor_assert(key);

  cached_desc = lookup_v3_desc_as_client(key->pubkey);
  if (!cached_desc) {
    return;
  }
  /* If we have a decrypted/decoded descriptor, attempt to close its
   * introduction circuit(s). We shouldn't have circuit(s) without a
   * descriptor else it will lead to a failure. */
  if (entry_has_decrypted_descriptor(cached_desc)) {
    hs_client_close_intro_circuits_from_desc(cached_desc->desc);
  }
  /* Remove and free. */
  remove_v3_desc_as_client(cached_desc);
  cache_client_desc_free(cached_desc);

  /* Logging. */
  {
    char key_b64[BASE64_DIGEST256_LEN + 1];
    digest256_to_base64(key_b64, (const char *) key);
    log_info(LD_REND, "Onion service v3 descriptor '%s' removed "
                      "from client cache",
             safe_str_client(key_b64));
  }
}

/** Clean all client caches using the current time now. */
void
hs_cache_clean_as_client(time_t now)
{
  /* Now, clean the v3 cache. Set the cutoff to 0 telling the cleanup function
   * to compute the cutoff by itself using the lifetime value. */
  cache_clean_v3_as_client(now);
}

/** Purge the client descriptor cache. */
void
hs_cache_purge_as_client(void)
{
  DIGEST256MAP_FOREACH_MODIFY(hs_cache_v3_client, key,
                              hs_cache_client_descriptor_t *, entry) {
    size_t entry_size = cache_get_client_entry_size(entry);
    MAP_DEL_CURRENT(key);
    cache_client_desc_free(entry);
    /* Update our OOM. We didn't use the remove() function because we are in
     * a loop so we have to explicitly decrement. */
    hs_cache_decrement_allocation(entry_size);
  } DIGEST256MAP_FOREACH_END;

  log_info(LD_REND, "Hidden service client descriptor cache purged.");
}

/** For a given service identity public key and an introduction authentication
 * key, note the given failure in the client intro state cache. */
void
hs_cache_client_intro_state_note(const ed25519_public_key_t *service_pk,
                                 const ed25519_public_key_t *auth_key,
                                 rend_intro_point_failure_t failure)
{
  int found;
  hs_cache_intro_state_t *entry;

  tor_assert(service_pk);
  tor_assert(auth_key);

  found = cache_client_intro_state_lookup(service_pk, auth_key, &entry);
  if (!found) {
    /* Create a new entry and add it to the cache. */
    cache_client_intro_state_add(service_pk, auth_key, &entry);
  }
  /* Note down the entry. */
  cache_client_intro_state_note(entry, failure);
}

/** For a given service identity public key and an introduction authentication
 * key, return true iff it is present in the failure cache. */
const hs_cache_intro_state_t *
hs_cache_client_intro_state_find(const ed25519_public_key_t *service_pk,
                                 const ed25519_public_key_t *auth_key)
{
  hs_cache_intro_state_t *state = NULL;
  cache_client_intro_state_lookup(service_pk, auth_key, &state);
  return state;
}

/** Cleanup the client introduction state cache. */
void
hs_cache_client_intro_state_clean(time_t now)
{
  time_t cutoff = now - HS_CACHE_CLIENT_INTRO_STATE_MAX_AGE;

  DIGEST256MAP_FOREACH_MODIFY(hs_cache_client_intro_state, key,
                              hs_cache_client_intro_state_t *, cache) {
    /* Cleanup intro points failure. */
    cache_client_intro_state_clean(cutoff, cache);

    /* Is this cache empty for this service key? If yes, remove it from the
     * cache. Else keep it. */
    if (cache_client_intro_state_is_empty(cache)) {
      cache_client_intro_state_free(cache);
      MAP_DEL_CURRENT(key);
    }
  } DIGEST256MAP_FOREACH_END;
}

/** Purge the client introduction state cache. */
void
hs_cache_client_intro_state_purge(void)
{
  DIGEST256MAP_FOREACH_MODIFY(hs_cache_client_intro_state, key,
                              hs_cache_client_intro_state_t *, cache) {
    MAP_DEL_CURRENT(key);
    cache_client_intro_state_free(cache);
  } DIGEST256MAP_FOREACH_END;

  log_info(LD_REND, "Hidden service client introduction point state "
                    "cache purged.");
}

/* This is called when new client authorization was added to the global state.
 * It attempts to decode the descriptor of the given service identity key.
 *
 * Return true if decoding was successful else false. */
bool
hs_cache_client_new_auth_parse(const ed25519_public_key_t *service_pk)
{
  bool ret = false;
  hs_cache_client_descriptor_t *cached_desc = NULL;

  tor_assert(service_pk);

  if (!hs_cache_v3_client) {
    return false;
  }

  cached_desc = lookup_v3_desc_as_client(service_pk->pubkey);
  if (cached_desc == NULL || entry_has_decrypted_descriptor(cached_desc)) {
    /* No entry for that service or the descriptor is already decoded. */
    goto end;
  }

  /* Attempt a decode. If we are successful, inform the caller. */
  if (hs_client_decode_descriptor(cached_desc->encoded_desc, service_pk,
                                  &cached_desc->desc) == HS_DESC_DECODE_OK) {
    ret = true;
  }

 end:
  return ret;
}

/**************** Generics *********************************/

/** Do a round of OOM cleanup on all directory caches. Return the amount of
 * removed bytes. It is possible that the returned value is lower than
 * min_remove_bytes if the caches get emptied out so the caller should be
 * aware of this. */
size_t
hs_cache_handle_oom(time_t now, size_t min_remove_bytes)
{
  time_t k;
  size_t bytes_removed = 0;

  /* Our OOM handler called with 0 bytes to remove is a code flow error. */
  tor_assert(min_remove_bytes != 0);

  /* The algorithm is as follow. K is the oldest expected descriptor age.
   *
   *   1) Deallocate all entries from v3 cache that are older than K hours
   *      2.1) If the amount of remove bytes has been reached, stop.
   *   2) Set K = K - 1 hour and repeat process until K is < 0.
   *
   * This ends up being O(Kn).
   */

  /* Set K to the oldest expected age in seconds which is the maximum
   * lifetime of a cache entry. */
  k = hs_cache_max_entry_lifetime();

  do {
    time_t cutoff;

    /* If K becomes negative, it means we've empty the caches so stop and
     * return what we were able to cleanup. */
    if (k < 0) {
      break;
    }
    /* Compute a cutoff value with K and the current time. */
    cutoff = now - k;

    if (bytes_removed < min_remove_bytes) {
      /* We haven't remove enough bytes so clean v3 cache. */
      bytes_removed += cache_clean_v3_as_dir(now, cutoff);
      /* Decrement K by a post period to shorten the cutoff, Two minutes
       * if we are a testing network, or one hour otherwise. */
      k -= get_options()->TestingTorNetwork ? 120 : 3600;
    }
  } while (bytes_removed < min_remove_bytes);

  return bytes_removed;
}

/** Return the maximum size of a v3 HS descriptor. */
unsigned int
hs_cache_get_max_descriptor_size(void)
{
  return (unsigned) networkstatus_get_param(NULL,
                                            "HSV3MaxDescriptorSize",
                                            HS_DESC_MAX_LEN, 1, INT32_MAX);
}

/** Initialize the hidden service cache subsystem. */
void
hs_cache_init(void)
{
  /* Calling this twice is very wrong code flow. */
  tor_assert(!hs_cache_v3_dir);
  hs_cache_v3_dir = digest256map_new();

  tor_assert(!hs_cache_v3_client);
  hs_cache_v3_client = digest256map_new();

  tor_assert(!hs_cache_client_intro_state);
  hs_cache_client_intro_state = digest256map_new();
}

/** Cleanup the hidden service cache subsystem. */
void
hs_cache_free_all(void)
{
  digest256map_free(hs_cache_v3_dir, cache_dir_desc_free_void);
  hs_cache_v3_dir = NULL;

  digest256map_free(hs_cache_v3_client, cache_client_desc_free_void);
  hs_cache_v3_client = NULL;

  digest256map_free(hs_cache_client_intro_state,
                    cache_client_intro_state_free_void);
  hs_cache_client_intro_state = NULL;
  hs_cache_total_allocation = 0;
}

/* Return total size of the cache. */
size_t
hs_cache_get_total_allocation(void)
{
  return hs_cache_total_allocation;
}

/** Decrement the total bytes attributed to the rendezvous cache by n. */
void
hs_cache_decrement_allocation(size_t n)
{
  static int have_underflowed = 0;

  if (hs_cache_total_allocation >= n) {
    hs_cache_total_allocation -= n;
  } else {
    hs_cache_total_allocation = 0;
    if (! have_underflowed) {
      have_underflowed = 1;
      log_warn(LD_BUG, "Underflow in hs_cache_decrement_allocation");
    }
  }
}

/** Increase the total bytes attributed to the rendezvous cache by n. */
void
hs_cache_increment_allocation(size_t n)
{
  static int have_overflowed = 0;
  if (hs_cache_total_allocation <= SIZE_MAX - n) {
    hs_cache_total_allocation += n;
  } else {
    hs_cache_total_allocation = SIZE_MAX;
    if (! have_overflowed) {
      have_overflowed = 1;
      log_warn(LD_BUG, "Overflow in hs_cache_increment_allocation");
    }
  }
}
