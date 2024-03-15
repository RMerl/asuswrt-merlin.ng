/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_common.c
 * \brief Contains code shared between different HS protocol version as well
 *        as useful data structures and accessors used by other subsystems.
 **/

#define HS_COMMON_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/or/circuitbuild.h"
#include "core/or/policies.h"
#include "core/or/extendinfo.h"
#include "feature/dirauth/shared_random_state.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_dos.h"
#include "feature/hs/hs_ob.h"
#include "feature/hs/hs_ident.h"
#include "feature/hs/hs_service.h"
#include "feature/hs_common/shared_random_client.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerset.h"
#include "feature/rend/rendcommon.h"
#include "feature/relay/routermode.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/net/resolve.h"

#include "core/or/edge_connection_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/origin_circuit_st.h"
#include "feature/nodelist/routerstatus_st.h"

/* Trunnel */
#include "trunnel/ed25519_cert.h"

/** Ed25519 Basepoint value. Taken from section 5 of
 * https://tools.ietf.org/html/draft-josefsson-eddsa-ed25519-03 */
static const char *str_ed25519_basepoint =
  "(15112221349535400772501151409588531511"
  "454012693041857206046113283949847762202, "
  "463168356949264781694283940034751631413"
  "07993866256225615783033603165251855960)";

#ifdef HAVE_SYS_UN_H

/** Given <b>ports</b>, a smartlist containing hs_port_config_t,
 * add the given <b>p</b>, a AF_UNIX port to the list. Return 0 on success
 * else return -ENOSYS if AF_UNIX is not supported (see function in the
 * #else statement below). */
static int
add_unix_port(smartlist_t *ports, hs_port_config_t *p)
{
  tor_assert(ports);
  tor_assert(p);
  tor_assert(p->is_unix_addr);

  smartlist_add(ports, p);
  return 0;
}

/** Given <b>conn</b> set it to use the given port <b>p</b> values. Return 0
 * on success else return -ENOSYS if AF_UNIX is not supported (see function
 * in the #else statement below). */
static int
set_unix_port(edge_connection_t *conn, hs_port_config_t *p)
{
  tor_assert(conn);
  tor_assert(p);
  tor_assert(p->is_unix_addr);

  conn->base_.socket_family = AF_UNIX;
  tor_addr_make_unspec(&conn->base_.addr);
  conn->base_.port = 1;
  conn->base_.address = tor_strdup(p->unix_addr);
  return 0;
}

#else /* !defined(HAVE_SYS_UN_H) */

static int
set_unix_port(edge_connection_t *conn, hs_port_config_t *p)
{
  (void) conn;
  (void) p;
  return -ENOSYS;
}

static int
add_unix_port(smartlist_t *ports, hs_port_config_t *p)
{
  (void) ports;
  (void) p;
  return -ENOSYS;
}

#endif /* defined(HAVE_SYS_UN_H) */

/** Helper function: The key is a digest that we compare to a node_t object
 * current hsdir_index. */
static int
compare_digest_to_fetch_hsdir_index(const void *_key, const void **_member)
{
  const char *key = _key;
  const node_t *node = *_member;
  return tor_memcmp(key, node->hsdir_index.fetch, DIGEST256_LEN);
}

/** Helper function: The key is a digest that we compare to a node_t object
 * next hsdir_index. */
static int
compare_digest_to_store_first_hsdir_index(const void *_key,
                                          const void **_member)
{
  const char *key = _key;
  const node_t *node = *_member;
  return tor_memcmp(key, node->hsdir_index.store_first, DIGEST256_LEN);
}

/** Helper function: The key is a digest that we compare to a node_t object
 * next hsdir_index. */
static int
compare_digest_to_store_second_hsdir_index(const void *_key,
                                          const void **_member)
{
  const char *key = _key;
  const node_t *node = *_member;
  return tor_memcmp(key, node->hsdir_index.store_second, DIGEST256_LEN);
}

/** Helper function: Compare two node_t objects current hsdir_index. */
static int
compare_node_fetch_hsdir_index(const void **a, const void **b)
{
  const node_t *node1= *a;
  const node_t *node2 = *b;
  return tor_memcmp(node1->hsdir_index.fetch,
                    node2->hsdir_index.fetch,
                    DIGEST256_LEN);
}

/** Helper function: Compare two node_t objects next hsdir_index. */
static int
compare_node_store_first_hsdir_index(const void **a, const void **b)
{
  const node_t *node1= *a;
  const node_t *node2 = *b;
  return tor_memcmp(node1->hsdir_index.store_first,
                    node2->hsdir_index.store_first,
                    DIGEST256_LEN);
}

/** Helper function: Compare two node_t objects next hsdir_index. */
static int
compare_node_store_second_hsdir_index(const void **a, const void **b)
{
  const node_t *node1= *a;
  const node_t *node2 = *b;
  return tor_memcmp(node1->hsdir_index.store_second,
                    node2->hsdir_index.store_second,
                    DIGEST256_LEN);
}

/** Allocate and return a string containing the path to filename in directory.
 * This function will never return NULL. The caller must free this path. */
char *
hs_path_from_filename(const char *directory, const char *filename)
{
  char *file_path = NULL;

  tor_assert(directory);
  tor_assert(filename);

  tor_asprintf(&file_path, "%s%s%s", directory, PATH_SEPARATOR, filename);
  return file_path;
}

/** Make sure that the directory for <b>service</b> is private, using the
 * config <b>username</b>.
 *
 * If <b>create</b> is true:
 *  - if the directory exists, change permissions if needed,
 *  - if the directory does not exist, create it with the correct permissions.
 * If <b>create</b> is false:
 *  - if the directory exists, check permissions,
 *  - if the directory does not exist, check if we think we can create it.
 * Return 0 on success, -1 on failure. */
int
hs_check_service_private_dir(const char *username, const char *path,
                             unsigned int dir_group_readable,
                             unsigned int create)
{
  cpd_check_t check_opts = CPD_NONE;

  tor_assert(path);

  if (create) {
    check_opts |= CPD_CREATE;
  } else {
    check_opts |= CPD_CHECK_MODE_ONLY;
    check_opts |= CPD_CHECK;
  }
  if (dir_group_readable) {
    check_opts |= CPD_GROUP_READ;
  }
  /* Check/create directory */
  if (check_private_dir(path, check_opts, username) < 0) {
    return -1;
  }
  return 0;
}

/* Default, minimum, and maximum values for the maximum rendezvous failures
 * consensus parameter. */
#define MAX_REND_FAILURES_DEFAULT 2
#define MAX_REND_FAILURES_MIN 1
#define MAX_REND_FAILURES_MAX 10

/** How many times will a hidden service operator attempt to connect to
 * a requested rendezvous point before giving up? */
int
hs_get_service_max_rend_failures(void)
{
  return networkstatus_get_param(NULL, "hs_service_max_rdv_failures",
                                 MAX_REND_FAILURES_DEFAULT,
                                 MAX_REND_FAILURES_MIN,
                                 MAX_REND_FAILURES_MAX);
}

/** Get the default HS time period length in minutes from the consensus. */
STATIC uint64_t
get_time_period_length(void)
{
  /* If we are on a test network, make the time period smaller than normal so
     that we actually see it rotate. Specifically, make it the same length as
     an SRV protocol run. */
  if (get_options()->TestingTorNetwork) {
    unsigned run_duration = sr_state_get_protocol_run_duration();
    /* An SRV run should take more than a minute (it's 24 rounds) */
    tor_assert_nonfatal(run_duration > 60);
    /* Turn it from seconds to minutes before returning: */
    return sr_state_get_protocol_run_duration() / 60;
  }

  int32_t time_period_length = networkstatus_get_param(NULL, "hsdir_interval",
                                             HS_TIME_PERIOD_LENGTH_DEFAULT,
                                             HS_TIME_PERIOD_LENGTH_MIN,
                                             HS_TIME_PERIOD_LENGTH_MAX);
  /* Make sure it's a positive value. */
  tor_assert(time_period_length > 0);
  /* uint64_t will always be able to contain a positive int32_t */
  return (uint64_t) time_period_length;
}

/** Get the HS time period number at time <b>now</b>. If <b>now</b> is not set,
 *  we try to get the time ourselves from a live consensus. */
uint64_t
hs_get_time_period_num(time_t now)
{
  uint64_t time_period_num;
  time_t current_time;

  /* If no time is specified, set current time based on consensus time, and
   * only fall back to system time if that fails. */
  if (now != 0) {
    current_time = now;
  } else {
    networkstatus_t *ns =
      networkstatus_get_reasonably_live_consensus(approx_time(),
                                                  usable_consensus_flavor());
    current_time = ns ? ns->valid_after : approx_time();
  }

  /* Start by calculating minutes since the epoch */
  uint64_t time_period_length = get_time_period_length();
  uint64_t minutes_since_epoch = current_time / 60;

  /* Apply the rotation offset as specified by prop224 (section
   * [TIME-PERIODS]), so that new time periods synchronize nicely with SRV
   * publication */
  unsigned int time_period_rotation_offset = sr_state_get_phase_duration();
  time_period_rotation_offset /= 60; /* go from seconds to minutes */
  tor_assert(minutes_since_epoch > time_period_rotation_offset);
  minutes_since_epoch -= time_period_rotation_offset;

  /* Calculate the time period */
  time_period_num = minutes_since_epoch / time_period_length;
  return time_period_num;
}

/** Get the number of the _upcoming_ HS time period, given that the current
 *  time is <b>now</b>. If <b>now</b> is not set, we try to get the time from a
 *  live consensus. */
uint64_t
hs_get_next_time_period_num(time_t now)
{
  return hs_get_time_period_num(now) + 1;
}

/** Get the number of the _previous_ HS time period, given that the current
 * time is <b>now</b>. If <b>now</b> is not set, we try to get the time from a
 * live consensus. */
uint64_t
hs_get_previous_time_period_num(time_t now)
{
  return hs_get_time_period_num(now) - 1;
}

/** Return the start time of the upcoming time period based on <b>now</b>. If
 * <b>now</b> is not set, we try to get the time ourselves from a live
 * consensus. */
time_t
hs_get_start_time_of_next_time_period(time_t now)
{
  uint64_t time_period_length = get_time_period_length();

  /* Get start time of next time period */
  uint64_t next_time_period_num = hs_get_next_time_period_num(now);
  uint64_t start_of_next_tp_in_mins = next_time_period_num *time_period_length;

  /* Apply rotation offset as specified by prop224 section [TIME-PERIODS] */
  unsigned int time_period_rotation_offset = sr_state_get_phase_duration();
  return (time_t)(start_of_next_tp_in_mins * 60 + time_period_rotation_offset);
}

/** Using the given time period number, compute the disaster shared random
 * value and put it in srv_out. It MUST be at least DIGEST256_LEN bytes. */
static void
compute_disaster_srv(uint64_t time_period_num, uint8_t *srv_out)
{
  crypto_digest_t *digest;

  tor_assert(srv_out);

  digest = crypto_digest256_new(DIGEST_SHA3_256);

  /* Start setting up payload:
   *  H("shared-random-disaster" | INT_8(period_length) | INT_8(period_num)) */
  crypto_digest_add_bytes(digest, HS_SRV_DISASTER_PREFIX,
                          HS_SRV_DISASTER_PREFIX_LEN);

  /* Setup INT_8(period_length) | INT_8(period_num) */
  {
    uint64_t time_period_length = get_time_period_length();
    char period_stuff[sizeof(uint64_t)*2];
    size_t offset = 0;
    set_uint64(period_stuff, tor_htonll(time_period_length));
    offset += sizeof(uint64_t);
    set_uint64(period_stuff+offset, tor_htonll(time_period_num));
    offset += sizeof(uint64_t);
    tor_assert(offset == sizeof(period_stuff));

    crypto_digest_add_bytes(digest, period_stuff,  sizeof(period_stuff));
  }

  crypto_digest_get_digest(digest, (char *) srv_out, DIGEST256_LEN);
  crypto_digest_free(digest);
}

/** Due to the high cost of computing the disaster SRV and that potentially we
 *  would have to do it thousands of times in a row, we always cache the
 *  computer disaster SRV (and its corresponding time period num) in case we
 *  want to reuse it soon after. We need to cache two SRVs, one for each active
 *  time period.
 */
static uint8_t cached_disaster_srv[2][DIGEST256_LEN];
static uint64_t cached_time_period_nums[2] = {0};

/** Compute the disaster SRV value for this <b>time_period_num</b> and put it
 *  in <b>srv_out</b> (of size at least DIGEST256_LEN). First check our caches
 *  to see if we have already computed it. */
STATIC void
get_disaster_srv(uint64_t time_period_num, uint8_t *srv_out)
{
  if (time_period_num == cached_time_period_nums[0]) {
    memcpy(srv_out, cached_disaster_srv[0], DIGEST256_LEN);
    return;
  } else if (time_period_num == cached_time_period_nums[1]) {
    memcpy(srv_out, cached_disaster_srv[1], DIGEST256_LEN);
    return;
  } else {
    int replace_idx;
    // Replace the lower period number.
    if (cached_time_period_nums[0] <= cached_time_period_nums[1]) {
      replace_idx = 0;
    } else {
      replace_idx = 1;
    }
    cached_time_period_nums[replace_idx] = time_period_num;
    compute_disaster_srv(time_period_num, cached_disaster_srv[replace_idx]);
    memcpy(srv_out, cached_disaster_srv[replace_idx], DIGEST256_LEN);
    return;
  }
}

#ifdef TOR_UNIT_TESTS

/** Get the first cached disaster SRV. Only used by unittests. */
STATIC uint8_t *
get_first_cached_disaster_srv(void)
{
  return cached_disaster_srv[0];
}

/** Get the second cached disaster SRV. Only used by unittests. */
STATIC uint8_t *
get_second_cached_disaster_srv(void)
{
  return cached_disaster_srv[1];
}

#endif /* defined(TOR_UNIT_TESTS) */

/** When creating a blinded key, we need a parameter which construction is as
 * follow: H(pubkey | [secret] | ed25519-basepoint | nonce).
 *
 * The nonce has a pre-defined format which uses the time period number
 * period_num and the start of the period in second start_time_period.
 *
 * The secret of size secret_len is optional meaning that it can be NULL and
 * thus will be ignored for the param construction.
 *
 * The result is put in param_out. */
STATIC void
build_blinded_key_param(const ed25519_public_key_t *pubkey,
                        const uint8_t *secret, size_t secret_len,
                        uint64_t period_num, uint64_t period_length,
                        uint8_t *param_out)
{
  size_t offset = 0;
  const char blind_str[] = "Derive temporary signing key";
  uint8_t nonce[HS_KEYBLIND_NONCE_LEN];
  crypto_digest_t *digest;

  tor_assert(pubkey);
  tor_assert(param_out);

  /* Create the nonce N. The construction is as follow:
   *    N = "key-blind" || INT_8(period_num) || INT_8(period_length) */
  memcpy(nonce, HS_KEYBLIND_NONCE_PREFIX, HS_KEYBLIND_NONCE_PREFIX_LEN);
  offset += HS_KEYBLIND_NONCE_PREFIX_LEN;
  set_uint64(nonce + offset, tor_htonll(period_num));
  offset += sizeof(uint64_t);
  set_uint64(nonce + offset, tor_htonll(period_length));
  offset += sizeof(uint64_t);
  tor_assert(offset == HS_KEYBLIND_NONCE_LEN);

  /* Generate the parameter h and the construction is as follow:
   *    h = H(BLIND_STRING | pubkey | [secret] | ed25519-basepoint | N) */
  digest = crypto_digest256_new(DIGEST_SHA3_256);
  crypto_digest_add_bytes(digest, blind_str, sizeof(blind_str));
  crypto_digest_add_bytes(digest, (char *) pubkey, ED25519_PUBKEY_LEN);
  /* Optional secret. */
  if (secret) {
    crypto_digest_add_bytes(digest, (char *) secret, secret_len);
  }
  crypto_digest_add_bytes(digest, str_ed25519_basepoint,
                          strlen(str_ed25519_basepoint));
  crypto_digest_add_bytes(digest, (char *) nonce, sizeof(nonce));

  /* Extract digest and put it in the param. */
  crypto_digest_get_digest(digest, (char *) param_out, DIGEST256_LEN);
  crypto_digest_free(digest);

  memwipe(nonce, 0, sizeof(nonce));
}

/** Using an ed25519 public key and version to build the checksum of an
 * address. Put in checksum_out. Format is:
 *    SHA3-256(".onion checksum" || PUBKEY || VERSION)
 *
 * checksum_out must be large enough to receive 32 bytes (DIGEST256_LEN). */
static void
build_hs_checksum(const ed25519_public_key_t *key, uint8_t version,
                  uint8_t *checksum_out)
{
  size_t offset = 0;
  char data[HS_SERVICE_ADDR_CHECKSUM_INPUT_LEN];

  /* Build checksum data. */
  memcpy(data, HS_SERVICE_ADDR_CHECKSUM_PREFIX,
         HS_SERVICE_ADDR_CHECKSUM_PREFIX_LEN);
  offset += HS_SERVICE_ADDR_CHECKSUM_PREFIX_LEN;
  memcpy(data + offset, key->pubkey, ED25519_PUBKEY_LEN);
  offset += ED25519_PUBKEY_LEN;
  set_uint8(data + offset, version);
  offset += sizeof(version);
  tor_assert(offset == HS_SERVICE_ADDR_CHECKSUM_INPUT_LEN);

  /* Hash the data payload to create the checksum. */
  crypto_digest256((char *) checksum_out, data, sizeof(data),
                   DIGEST_SHA3_256);
}

/** Using an ed25519 public key, checksum and version to build the binary
 * representation of a service address. Put in addr_out. Format is:
 *    addr_out = PUBKEY || CHECKSUM || VERSION
 *
 * addr_out must be large enough to receive HS_SERVICE_ADDR_LEN bytes. */
static void
build_hs_address(const ed25519_public_key_t *key, const uint8_t *checksum,
                 uint8_t version, char *addr_out)
{
  size_t offset = 0;

  tor_assert(key);
  tor_assert(checksum);

  memcpy(addr_out, key->pubkey, ED25519_PUBKEY_LEN);
  offset += ED25519_PUBKEY_LEN;
  memcpy(addr_out + offset, checksum, HS_SERVICE_ADDR_CHECKSUM_LEN_USED);
  offset += HS_SERVICE_ADDR_CHECKSUM_LEN_USED;
  set_uint8(addr_out + offset, version);
  offset += sizeof(uint8_t);
  tor_assert(offset == HS_SERVICE_ADDR_LEN);
}

/** Helper for hs_parse_address(): Using a binary representation of a service
 * address, parse its content into the key_out, checksum_out and version_out.
 * Any out variable can be NULL in case the caller would want only one field.
 * checksum_out MUST at least be 2 bytes long. address must be at least
 * HS_SERVICE_ADDR_LEN bytes but doesn't need to be NUL terminated. */
static void
hs_parse_address_impl(const char *address, ed25519_public_key_t *key_out,
                      uint8_t *checksum_out, uint8_t *version_out)
{
  size_t offset = 0;

  tor_assert(address);

  if (key_out) {
    /* First is the key. */
    memcpy(key_out->pubkey, address, ED25519_PUBKEY_LEN);
  }
  offset += ED25519_PUBKEY_LEN;
  if (checksum_out) {
    /* Followed by a 2 bytes checksum. */
    memcpy(checksum_out, address + offset, HS_SERVICE_ADDR_CHECKSUM_LEN_USED);
  }
  offset += HS_SERVICE_ADDR_CHECKSUM_LEN_USED;
  if (version_out) {
    /* Finally, version value is 1 byte. */
    *version_out = get_uint8(address + offset);
  }
  offset += sizeof(uint8_t);
  /* Extra safety. */
  tor_assert(offset == HS_SERVICE_ADDR_LEN);
}

/** Using the given identity public key and a blinded public key, compute the
 * subcredential and put it in subcred_out.
 * This can't fail. */
void
hs_get_subcredential(const ed25519_public_key_t *identity_pk,
                     const ed25519_public_key_t *blinded_pk,
                     hs_subcredential_t *subcred_out)
{
  uint8_t credential[DIGEST256_LEN];
  crypto_digest_t *digest;

  tor_assert(identity_pk);
  tor_assert(blinded_pk);
  tor_assert(subcred_out);

  /* First, build the credential. Construction is as follow:
   *  credential = H("credential" | public-identity-key) */
  digest = crypto_digest256_new(DIGEST_SHA3_256);
  crypto_digest_add_bytes(digest, HS_CREDENTIAL_PREFIX,
                          HS_CREDENTIAL_PREFIX_LEN);
  crypto_digest_add_bytes(digest, (const char *) identity_pk->pubkey,
                          ED25519_PUBKEY_LEN);
  crypto_digest_get_digest(digest, (char *) credential, DIGEST256_LEN);
  crypto_digest_free(digest);

  /* Now, compute the subcredential. Construction is as follow:
   *  subcredential = H("subcredential" | credential | blinded-public-key). */
  digest = crypto_digest256_new(DIGEST_SHA3_256);
  crypto_digest_add_bytes(digest, HS_SUBCREDENTIAL_PREFIX,
                          HS_SUBCREDENTIAL_PREFIX_LEN);
  crypto_digest_add_bytes(digest, (const char *) credential,
                          sizeof(credential));
  crypto_digest_add_bytes(digest, (const char *) blinded_pk->pubkey,
                          ED25519_PUBKEY_LEN);
  crypto_digest_get_digest(digest, (char *) subcred_out->subcred,
                           SUBCRED_LEN);
  crypto_digest_free(digest);

  memwipe(credential, 0, sizeof(credential));
}

/** From the given list of hidden service ports, find the ones that match the
 * given edge connection conn, pick one at random and use it to set the
 * connection address. Return 0 on success or -1 if none. */
int
hs_set_conn_addr_port(const smartlist_t *ports, edge_connection_t *conn)
{
  hs_port_config_t *chosen_port;
  unsigned int warn_once = 0;
  smartlist_t *matching_ports;

  tor_assert(ports);
  tor_assert(conn);

  matching_ports = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(ports, hs_port_config_t *, p) {
    if (TO_CONN(conn)->port != p->virtual_port) {
      continue;
    }
    if (!(p->is_unix_addr)) {
      smartlist_add(matching_ports, p);
    } else {
      if (add_unix_port(matching_ports, p)) {
        if (!warn_once) {
          /* Unix port not supported so warn only once. */
          log_warn(LD_REND, "Saw AF_UNIX virtual port mapping for port %d "
                            "which is unsupported on this platform. "
                            "Ignoring it.",
                   TO_CONN(conn)->port);
        }
        warn_once++;
      }
    }
  } SMARTLIST_FOREACH_END(p);

  chosen_port = smartlist_choose(matching_ports);
  smartlist_free(matching_ports);
  if (chosen_port) {
    if (conn->hs_ident) {
      /* There is always a connection identifier at this point. Regardless of a
       * Unix or TCP port, note the virtual port. */
      conn->hs_ident->orig_virtual_port = chosen_port->virtual_port;
    }

    if (!(chosen_port->is_unix_addr)) {
      /* Get a non-AF_UNIX connection ready for connection_exit_connect() */
      tor_addr_copy(&TO_CONN(conn)->addr, &chosen_port->real_addr);
      TO_CONN(conn)->port = chosen_port->real_port;
    } else {
      if (set_unix_port(conn, chosen_port)) {
        /* Simply impossible to end up here else we were able to add a Unix
         * port without AF_UNIX support... ? */
        tor_assert(0);
      }
    }
  }
  return (chosen_port) ? 0 : -1;
}

/** Return a new hs_port_config_t with its path set to
 * <b>socket_path</b> or empty if <b>socket_path</b> is NULL */
static hs_port_config_t *
hs_port_config_new(const char *socket_path)
{
  if (!socket_path)
    return tor_malloc_zero(sizeof(hs_port_config_t) + 1);

  const size_t pathlen = strlen(socket_path) + 1;
  hs_port_config_t *conf =
    tor_malloc_zero(sizeof(hs_port_config_t) + pathlen);
  memcpy(conf->unix_addr, socket_path, pathlen);
  conf->is_unix_addr = 1;
  return conf;
}

/** Parses a virtual-port to real-port/socket mapping separated by
 * the provided separator and returns a new hs_port_config_t,
 * or NULL and an optional error string on failure.
 *
 * The format is: VirtualPort SEP (IP|RealPort|IP:RealPort|'socket':path)?
 *
 * IP defaults to 127.0.0.1; RealPort defaults to VirtualPort.
 */
hs_port_config_t *
hs_parse_port_config(const char *string, const char *sep,
                               char **err_msg_out)
{
  smartlist_t *sl;
  int virtport;
  int realport = 0;
  uint16_t p;
  tor_addr_t addr;
  hs_port_config_t *result = NULL;
  unsigned int is_unix_addr = 0;
  const char *socket_path = NULL;
  char *err_msg = NULL;
  char *addrport = NULL;

  sl = smartlist_new();
  smartlist_split_string(sl, string, sep,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 2);
  if (smartlist_len(sl) < 1 || BUG(smartlist_len(sl) > 2)) {
    err_msg = tor_strdup("Bad syntax in hidden service port configuration.");
    goto err;
  }
  virtport = (int)tor_parse_long(smartlist_get(sl,0), 10, 1, 65535, NULL,NULL);
  if (!virtport) {
    tor_asprintf(&err_msg, "Missing or invalid port %s in hidden service "
                   "port configuration", escaped(smartlist_get(sl,0)));

    goto err;
  }
  if (smartlist_len(sl) == 1) {
    /* No addr:port part; use default. */
    realport = virtport;
    tor_addr_from_ipv4h(&addr, 0x7F000001u); /* 127.0.0.1 */
  } else {
    int ret;

    const char *addrport_element = smartlist_get(sl,1);
    const char *rest = NULL;
    int is_unix;
    ret = port_cfg_line_extract_addrport(addrport_element, &addrport,
                                         &is_unix, &rest);

    if (ret < 0) {
      tor_asprintf(&err_msg, "Couldn't process address <%s> from hidden "
                   "service configuration", addrport_element);
      goto err;
    }

    if (rest && strlen(rest)) {
      err_msg = tor_strdup("HiddenServicePort parse error: invalid port "
                           "mapping");
      goto err;
    }

    if (is_unix) {
      socket_path = addrport;
      is_unix_addr = 1;
    } else if (strchr(addrport, ':') || strchr(addrport, '.')) {
      /* else try it as an IP:port pair if it has a : or . in it */
      if (tor_addr_port_lookup(addrport, &addr, &p)<0) {
        err_msg = tor_strdup("Unparseable address in hidden service port "
                             "configuration.");
        goto err;
      }
      realport = p?p:virtport;
    } else {
      /* No addr:port, no addr -- must be port. */
      realport = (int)tor_parse_long(addrport, 10, 1, 65535, NULL, NULL);
      if (!realport) {
        tor_asprintf(&err_msg, "Unparseable or out-of-range port %s in "
                     "hidden service port configuration.",
                     escaped(addrport));
        goto err;
      }
      tor_addr_from_ipv4h(&addr, 0x7F000001u); /* Default to 127.0.0.1 */
    }
  }

  /* Allow room for unix_addr */
  result = hs_port_config_new(socket_path);
  result->virtual_port = virtport;
  result->is_unix_addr = is_unix_addr;
  if (!is_unix_addr) {
    result->real_port = realport;
    tor_addr_copy(&result->real_addr, &addr);
    result->unix_addr[0] = '\0';
  }

 err:
  tor_free(addrport);
  if (err_msg_out != NULL) {
    *err_msg_out = err_msg;
  } else {
    tor_free(err_msg);
  }
  SMARTLIST_FOREACH(sl, char *, c, tor_free(c));
  smartlist_free(sl);

  return result;
}

/** Release all storage held in a hs_port_config_t. */
void
hs_port_config_free_(hs_port_config_t *p)
{
  tor_free(p);
}

/** Using a base32 representation of a service address, parse its content into
 * the key_out, checksum_out and version_out. Any out variable can be NULL in
 * case the caller would want only one field. checksum_out MUST at least be 2
 * bytes long.
 *
 * Return 0 if parsing went well; return -1 in case of error and if errmsg is
 * non NULL, a human readable string message is set. */
int
hs_parse_address_no_log(const char *address, ed25519_public_key_t *key_out,
                        uint8_t *checksum_out, uint8_t *version_out,
                        const char **errmsg)
{
  char decoded[HS_SERVICE_ADDR_LEN];

  tor_assert(address);

  if (errmsg) {
    *errmsg = NULL;
  }

  /* Obvious length check. */
  if (strlen(address) != HS_SERVICE_ADDR_LEN_BASE32) {
    if (errmsg) {
      *errmsg = "Invalid length";
    }
    goto invalid;
  }

  /* Decode address so we can extract needed fields. */
  if (base32_decode(decoded, sizeof(decoded), address, strlen(address))
      != sizeof(decoded)) {
    if (errmsg) {
      *errmsg = "Unable to base32 decode";
    }
    goto invalid;
  }

  /* Parse the decoded address into the fields we need. */
  hs_parse_address_impl(decoded, key_out, checksum_out, version_out);

  return 0;
 invalid:
  return -1;
}

/** Same has hs_parse_address_no_log() but emits a log warning on parsing
 * failure. */
int
hs_parse_address(const char *address, ed25519_public_key_t *key_out,
                 uint8_t *checksum_out, uint8_t *version_out)
{
  const char *errmsg = NULL;
  int ret = hs_parse_address_no_log(address, key_out, checksum_out,
                                    version_out, &errmsg);
  if (ret < 0) {
    log_warn(LD_REND, "Service address %s failed to be parsed: %s",
             escaped_safe_str(address), errmsg);
  }
  return ret;
}

/** Validate a given onion address. The length, the base32 decoding, and
 * checksum are validated. Return 1 if valid else 0. */
int
hs_address_is_valid(const char *address)
{
  uint8_t version;
  uint8_t checksum[HS_SERVICE_ADDR_CHECKSUM_LEN_USED];
  uint8_t target_checksum[DIGEST256_LEN];
  ed25519_public_key_t service_pubkey;

  /* Parse the decoded address into the fields we need. */
  if (hs_parse_address(address, &service_pubkey, checksum, &version) < 0) {
    goto invalid;
  }

  /* Get the checksum it's supposed to be and compare it with what we have
   * encoded in the address. */
  build_hs_checksum(&service_pubkey, version, target_checksum);
  if (tor_memcmp(checksum, target_checksum, sizeof(checksum))) {
    log_warn(LD_REND, "Service address %s invalid checksum.",
             escaped_safe_str(address));
    goto invalid;
  }

  /* Validate that this pubkey does not have a torsion component. We need to do
   * this on the prop224 client-side so that attackers can't give equivalent
   * forms of an onion address to users. */
  if (ed25519_validate_pubkey(&service_pubkey) < 0) {
    log_warn(LD_REND, "Service address %s has bad pubkey .",
             escaped_safe_str(address));
    goto invalid;
  }

  /* Valid address. */
  return 1;
 invalid:
  return 0;
}

/** Build a service address using an ed25519 public key and a given version.
 * The returned address is base32 encoded and put in addr_out. The caller MUST
 * make sure the addr_out is at least HS_SERVICE_ADDR_LEN_BASE32 + 1 long.
 *
 * Format is as follows:
 *     base32(PUBKEY || CHECKSUM || VERSION)
 *     CHECKSUM = H(".onion checksum" || PUBKEY || VERSION)
 * */
void
hs_build_address(const ed25519_public_key_t *key, uint8_t version,
                 char *addr_out)
{
  uint8_t checksum[DIGEST256_LEN];
  char address[HS_SERVICE_ADDR_LEN];

  tor_assert(key);
  tor_assert(addr_out);

  /* Get the checksum of the address. */
  build_hs_checksum(key, version, checksum);
  /* Get the binary address representation. */
  build_hs_address(key, checksum, version, address);

  /* Encode the address. addr_out will be NUL terminated after this. */
  base32_encode(addr_out, HS_SERVICE_ADDR_LEN_BASE32 + 1, address,
                sizeof(address));
  /* Validate what we just built. */
  tor_assert(hs_address_is_valid(addr_out));
}

/** From a given ed25519 public key pk and an optional secret, compute a
 * blinded public key and put it in blinded_pk_out. This is only useful to
 * the client side because the client only has access to the identity public
 * key of the service. */
void
hs_build_blinded_pubkey(const ed25519_public_key_t *pk,
                        const uint8_t *secret, size_t secret_len,
                        uint64_t time_period_num,
                        ed25519_public_key_t *blinded_pk_out)
{
  /* Our blinding key API requires a 32 bytes parameter. */
  uint8_t param[DIGEST256_LEN];

  tor_assert(pk);
  tor_assert(blinded_pk_out);
  tor_assert(!fast_mem_is_zero((char *) pk, ED25519_PUBKEY_LEN));

  build_blinded_key_param(pk, secret, secret_len,
                          time_period_num, get_time_period_length(), param);
  ed25519_public_blind(blinded_pk_out, pk, param);

  memwipe(param, 0, sizeof(param));
}

/** From a given ed25519 keypair kp and an optional secret, compute a blinded
 * keypair for the current time period and put it in blinded_kp_out. This is
 * only useful by the service side because the client doesn't have access to
 * the identity secret key. */
void
hs_build_blinded_keypair(const ed25519_keypair_t *kp,
                         const uint8_t *secret, size_t secret_len,
                         uint64_t time_period_num,
                         ed25519_keypair_t *blinded_kp_out)
{
  /* Our blinding key API requires a 32 bytes parameter. */
  uint8_t param[DIGEST256_LEN];

  tor_assert(kp);
  tor_assert(blinded_kp_out);
  /* Extra safety. A zeroed key is bad. */
  tor_assert(!fast_mem_is_zero((char *) &kp->pubkey, ED25519_PUBKEY_LEN));
  tor_assert(!fast_mem_is_zero((char *) &kp->seckey, ED25519_SECKEY_LEN));

  build_blinded_key_param(&kp->pubkey, secret, secret_len,
                          time_period_num, get_time_period_length(), param);
  ed25519_keypair_blind(blinded_kp_out, kp, param);

  memwipe(param, 0, sizeof(param));
}

/** Return true if we are currently in the time segment between a new time
 * period and a new SRV (in the real network that happens between 12:00 and
 * 00:00 UTC). Here is a diagram showing exactly when this returns true:
 *
 *    +------------------------------------------------------------------+
 *    |                                                                  |
 *    | 00:00      12:00       00:00       12:00       00:00       12:00 |
 *    | SRV#1      TP#1        SRV#2       TP#2        SRV#3       TP#3  |
 *    |                                                                  |
 *    |  $==========|-----------$===========|-----------$===========|    |
 *    |             ^^^^^^^^^^^^            ^^^^^^^^^^^^                 |
 *    |                                                                  |
 *    +------------------------------------------------------------------+
 */
MOCK_IMPL(int,
hs_in_period_between_tp_and_srv,(const networkstatus_t *consensus, time_t now))
{
  time_t valid_after;
  time_t srv_start_time, tp_start_time;

  if (!consensus) {
    consensus = networkstatus_get_reasonably_live_consensus(now,
                                                  usable_consensus_flavor());
    if (!consensus) {
      return 0;
    }
  }

  /* Get start time of next TP and of current SRV protocol run, and check if we
   * are between them. */
  valid_after = consensus->valid_after;
  srv_start_time = sr_state_get_start_time_of_current_protocol_run();
  tp_start_time = hs_get_start_time_of_next_time_period(srv_start_time);

  if (valid_after >= srv_start_time && valid_after < tp_start_time) {
    return 0;
  }

  return 1;
}

/** Return 1 if any virtual port in ports needs a circuit with good uptime.
 * Else return 0. */
int
hs_service_requires_uptime_circ(const smartlist_t *ports)
{
  tor_assert(ports);

  SMARTLIST_FOREACH_BEGIN(ports, hs_port_config_t *, p) {
    if (smartlist_contains_int_as_string(get_options()->LongLivedPorts,
                                         p->virtual_port)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(p);
  return 0;
}

/** Build hs_index which is used to find the responsible hsdirs. This index
 * value is used to select the responsible HSDir where their hsdir_index is
 * closest to this value.
 *    SHA3-256("store-at-idx" | blinded_public_key |
 *             INT_8(replicanum) | INT_8(period_length) | INT_8(period_num) )
 *
 * hs_index_out must be large enough to receive DIGEST256_LEN bytes. */
void
hs_build_hs_index(uint64_t replica, const ed25519_public_key_t *blinded_pk,
                  uint64_t period_num, uint8_t *hs_index_out)
{
  crypto_digest_t *digest;

  tor_assert(blinded_pk);
  tor_assert(hs_index_out);

  /* Build hs_index. See construction at top of function comment. */
  digest = crypto_digest256_new(DIGEST_SHA3_256);
  crypto_digest_add_bytes(digest, HS_INDEX_PREFIX, HS_INDEX_PREFIX_LEN);
  crypto_digest_add_bytes(digest, (const char *) blinded_pk->pubkey,
                          ED25519_PUBKEY_LEN);

  /* Now setup INT_8(replicanum) | INT_8(period_length) | INT_8(period_num) */
  {
    uint64_t period_length = get_time_period_length();
    char buf[sizeof(uint64_t)*3];
    size_t offset = 0;
    set_uint64(buf, tor_htonll(replica));
    offset += sizeof(uint64_t);
    set_uint64(buf+offset, tor_htonll(period_length));
    offset += sizeof(uint64_t);
    set_uint64(buf+offset, tor_htonll(period_num));
    offset += sizeof(uint64_t);
    tor_assert(offset == sizeof(buf));

    crypto_digest_add_bytes(digest, buf, sizeof(buf));
  }

  crypto_digest_get_digest(digest, (char *) hs_index_out, DIGEST256_LEN);
  crypto_digest_free(digest);
}

/** Build hsdir_index which is used to find the responsible hsdirs. This is the
 * index value that is compare to the hs_index when selecting an HSDir.
 *    SHA3-256("node-idx" | node_identity |
 *             shared_random_value | INT_8(period_length) | INT_8(period_num) )
 *
 * hsdir_index_out must be large enough to receive DIGEST256_LEN bytes. */
void
hs_build_hsdir_index(const ed25519_public_key_t *identity_pk,
                     const uint8_t *srv_value, uint64_t period_num,
                     uint8_t *hsdir_index_out)
{
  crypto_digest_t *digest;

  tor_assert(identity_pk);
  tor_assert(srv_value);
  tor_assert(hsdir_index_out);

  /* Build hsdir_index. See construction at top of function comment. */
  digest = crypto_digest256_new(DIGEST_SHA3_256);
  crypto_digest_add_bytes(digest, HSDIR_INDEX_PREFIX, HSDIR_INDEX_PREFIX_LEN);
  crypto_digest_add_bytes(digest, (const char *) identity_pk->pubkey,
                          ED25519_PUBKEY_LEN);
  crypto_digest_add_bytes(digest, (const char *) srv_value, DIGEST256_LEN);

  {
    uint64_t time_period_length = get_time_period_length();
    char period_stuff[sizeof(uint64_t)*2];
    size_t offset = 0;
    set_uint64(period_stuff, tor_htonll(period_num));
    offset += sizeof(uint64_t);
    set_uint64(period_stuff+offset, tor_htonll(time_period_length));
    offset += sizeof(uint64_t);
    tor_assert(offset == sizeof(period_stuff));

    crypto_digest_add_bytes(digest, period_stuff,  sizeof(period_stuff));
  }

  crypto_digest_get_digest(digest, (char *) hsdir_index_out, DIGEST256_LEN);
  crypto_digest_free(digest);
}

/** Return a newly allocated buffer containing the current shared random value
 * or if not present, a disaster value is computed using the given time period
 * number. If a consensus is provided in <b>ns</b>, use it to get the SRV
 * value. This function can't fail. */
uint8_t *
hs_get_current_srv(uint64_t time_period_num, const networkstatus_t *ns)
{
  uint8_t *sr_value = tor_malloc_zero(DIGEST256_LEN);
  const sr_srv_t *current_srv = sr_get_current(ns);

  if (current_srv) {
    memcpy(sr_value, current_srv->value, sizeof(current_srv->value));
  } else {
    /* Disaster mode. */
    get_disaster_srv(time_period_num, sr_value);
  }
  return sr_value;
}

/** Return a newly allocated buffer containing the previous shared random
 * value or if not present, a disaster value is computed using the given time
 * period number. This function can't fail. */
uint8_t *
hs_get_previous_srv(uint64_t time_period_num, const networkstatus_t *ns)
{
  uint8_t *sr_value = tor_malloc_zero(DIGEST256_LEN);
  const sr_srv_t *previous_srv = sr_get_previous(ns);

  if (previous_srv) {
    memcpy(sr_value, previous_srv->value, sizeof(previous_srv->value));
  } else {
    /* Disaster mode. */
    get_disaster_srv(time_period_num, sr_value);
  }
  return sr_value;
}

/** Return the number of replicas defined by a consensus parameter or the
 * default value. */
int32_t
hs_get_hsdir_n_replicas(void)
{
  /* The [1,16] range is a specification requirement. */
  return networkstatus_get_param(NULL, "hsdir_n_replicas",
                                 HS_DEFAULT_HSDIR_N_REPLICAS, 1, 16);
}

/** Return the spread fetch value defined by a consensus parameter or the
 * default value. */
int32_t
hs_get_hsdir_spread_fetch(void)
{
  /* The [1,128] range is a specification requirement. */
  return networkstatus_get_param(NULL, "hsdir_spread_fetch",
                                 HS_DEFAULT_HSDIR_SPREAD_FETCH, 1, 128);
}

/** Return the spread store value defined by a consensus parameter or the
 * default value. */
int32_t
hs_get_hsdir_spread_store(void)
{
  /* The [1,128] range is a specification requirement. */
  return networkstatus_get_param(NULL, "hsdir_spread_store",
                                 HS_DEFAULT_HSDIR_SPREAD_STORE, 1, 128);
}

/** <b>node</b> is an HSDir so make sure that we have assigned an hsdir index.
 *  Return 0 if everything is as expected, else return -1. */
static int
node_has_hsdir_index(const node_t *node)
{
  tor_assert(node_supports_v3_hsdir(node));

  /* A node can't have an HSDir index without a descriptor since we need desc
   * to get its ed25519 key.  for_direct_connect should be zero, since we
   * always use the consensus-indexed node's keys to build the hash ring, even
   * if some of the consensus-indexed nodes are also bridges. */
  if (!node_has_preferred_descriptor(node, 0)) {
    return 0;
  }

  /* At this point, since the node has a desc, this node must also have an
   * hsdir index. If not, something went wrong, so BUG out. */
  if (BUG(fast_mem_is_zero((const char*)node->hsdir_index.fetch,
                          DIGEST256_LEN))) {
    return 0;
  }
  if (BUG(fast_mem_is_zero((const char*)node->hsdir_index.store_first,
                          DIGEST256_LEN))) {
    return 0;
  }
  if (BUG(fast_mem_is_zero((const char*)node->hsdir_index.store_second,
                          DIGEST256_LEN))) {
    return 0;
  }

  return 1;
}

/** For a given blinded key and time period number, get the responsible HSDir
 * and put their routerstatus_t object in the responsible_dirs list. If
 * 'use_second_hsdir_index' is true, use the second hsdir_index of the node_t
 * is used. If 'for_fetching' is true, the spread fetch consensus parameter is
 * used else the spread store is used which is only for upload. This function
 * can't fail but it is possible that the responsible_dirs list contains fewer
 * nodes than expected.
 *
 * This function goes over the latest consensus routerstatus list and sorts it
 * by their node_t hsdir_index then does a binary search to find the closest
 * node. All of this makes it a bit CPU intensive so use it wisely. */
void
hs_get_responsible_hsdirs(const ed25519_public_key_t *blinded_pk,
                          uint64_t time_period_num, int use_second_hsdir_index,
                          int for_fetching, smartlist_t *responsible_dirs)
{
  smartlist_t *sorted_nodes;
  /* The compare function used for the smartlist bsearch. We have two
   * different depending on is_next_period. */
  int (*cmp_fct)(const void *, const void **);

  tor_assert(blinded_pk);
  tor_assert(responsible_dirs);

  sorted_nodes = smartlist_new();

  /* Make sure we actually have a live consensus */
  networkstatus_t *c =
    networkstatus_get_reasonably_live_consensus(approx_time(),
                                                usable_consensus_flavor());
  if (!c || smartlist_len(c->routerstatus_list) == 0) {
      log_warn(LD_REND, "No live consensus so we can't get the responsible "
               "hidden service directories.");
      goto done;
  }

  /* Ensure the nodelist is fresh, since it contains the HSDir indices. */
  nodelist_ensure_freshness(c);

  /* Add every node_t that support HSDir v3 for which we do have a valid
   * hsdir_index already computed for them for this consensus. */
  {
    SMARTLIST_FOREACH_BEGIN(c->routerstatus_list, const routerstatus_t *, rs) {
      /* Even though this node_t object won't be modified and should be const,
       * we can't add const object in a smartlist_t. */
      node_t *n = node_get_mutable_by_id(rs->identity_digest);
      tor_assert(n);
      if (node_supports_v3_hsdir(n) && rs->is_hs_dir) {
        if (!node_has_hsdir_index(n)) {
          log_info(LD_GENERAL, "Node %s was found without hsdir index.",
                   node_describe(n));
          continue;
        }
        smartlist_add(sorted_nodes, n);
      }
    } SMARTLIST_FOREACH_END(rs);
  }
  if (smartlist_len(sorted_nodes) == 0) {
    log_warn(LD_REND, "No nodes found to be HSDir or supporting v3.");
    goto done;
  }

  /* First thing we have to do is sort all node_t by hsdir_index. The
   * is_next_period tells us if we want the current or the next one. Set the
   * bsearch compare function also while we are at it. */
  if (for_fetching) {
    smartlist_sort(sorted_nodes, compare_node_fetch_hsdir_index);
    cmp_fct = compare_digest_to_fetch_hsdir_index;
  } else if (use_second_hsdir_index) {
    smartlist_sort(sorted_nodes, compare_node_store_second_hsdir_index);
    cmp_fct = compare_digest_to_store_second_hsdir_index;
  } else {
    smartlist_sort(sorted_nodes, compare_node_store_first_hsdir_index);
    cmp_fct = compare_digest_to_store_first_hsdir_index;
  }

  /* For all replicas, we'll select a set of HSDirs using the consensus
   * parameters and the sorted list. The replica starting at value 1 is
   * defined by the specification. */
  for (int replica = 1; replica <= hs_get_hsdir_n_replicas(); replica++) {
    int idx, start, found, n_added = 0;
    uint8_t hs_index[DIGEST256_LEN] = {0};
    /* Number of node to add to the responsible dirs list depends on if we are
     * trying to fetch or store. A client always fetches. */
    int n_to_add = (for_fetching) ? hs_get_hsdir_spread_fetch() :
                                    hs_get_hsdir_spread_store();

    /* Get the index that we should use to select the node. */
    hs_build_hs_index(replica, blinded_pk, time_period_num, hs_index);
    /* The compare function pointer has been set correctly earlier. */
    start = idx = smartlist_bsearch_idx(sorted_nodes, hs_index, cmp_fct,
                                        &found);
    /* Getting the length of the list if no member is greater than the key we
     * are looking for so start at the first element. */
    if (idx == smartlist_len(sorted_nodes)) {
      start = idx = 0;
    }
    while (n_added < n_to_add) {
      const node_t *node = smartlist_get(sorted_nodes, idx);
      /* If the node has already been selected which is possible between
       * replicas, the specification says to skip over. */
      if (!smartlist_contains(responsible_dirs, node->rs)) {
        smartlist_add(responsible_dirs, node->rs);
        ++n_added;
      }
      if (++idx == smartlist_len(sorted_nodes)) {
        /* Wrap if we've reached the end of the list. */
        idx = 0;
      }
      if (idx == start) {
        /* We've gone over the whole list, stop and avoid infinite loop. */
        break;
      }
    }
  }

 done:
  smartlist_free(sorted_nodes);
}

/*********************** HSDir request tracking ***************************/

/** Return the period for which a hidden service directory cannot be queried
 * for the same descriptor ID again, taking TestingTorNetwork into account. */
time_t
hs_hsdir_requery_period(const or_options_t *options)
{
  tor_assert(options);

  if (options->TestingTorNetwork) {
    return REND_HID_SERV_DIR_REQUERY_PERIOD_TESTING;
  } else {
    return REND_HID_SERV_DIR_REQUERY_PERIOD;
  }
}

/** Tracks requests for fetching hidden service descriptors. It's used by
 *  hidden service clients, to avoid querying HSDirs that have already failed
 *  giving back a descriptor. The same data structure is used to track v3 HS
 *  descriptor requests.
 *
 * The string map is a key/value store that contains the last request times to
 * hidden service directories for certain queries. Specifically:
 *
 *   key = base32(hsdir_identity) + base32(hs_identity)
 *   value = time_t of last request for that hs_identity to that HSDir
 *
 * where 'hsdir_identity' is the identity digest of the HSDir node, and
 * 'hs_identity' is the ed25519 blinded public key of the HS for v3. */
static strmap_t *last_hid_serv_requests_ = NULL;

/** Returns last_hid_serv_requests_, initializing it to a new strmap if
 * necessary. */
STATIC strmap_t *
get_last_hid_serv_requests(void)
{
  if (!last_hid_serv_requests_)
    last_hid_serv_requests_ = strmap_new();
  return last_hid_serv_requests_;
}

/** Look up the last request time to hidden service directory <b>hs_dir</b>
 * for descriptor request key <b>req_key_str</b> which is the blinded key for
 * v3. If <b>set</b> is non-zero, assign the current time <b>now</b> and
 * return that. Otherwise, return the most recent request time, or 0 if no
 * such request has been sent before. */
time_t
hs_lookup_last_hid_serv_request(routerstatus_t *hs_dir,
                                const char *req_key_str,
                                time_t now, int set)
{
  char hsdir_id_base32[BASE32_DIGEST_LEN + 1];
  char *hsdir_desc_comb_id = NULL;
  time_t *last_request_ptr;
  strmap_t *last_hid_serv_requests = get_last_hid_serv_requests();

  /* Create the key */
  base32_encode(hsdir_id_base32, sizeof(hsdir_id_base32),
                hs_dir->identity_digest, DIGEST_LEN);
  tor_asprintf(&hsdir_desc_comb_id, "%s%s", hsdir_id_base32, req_key_str);

  if (set) {
    time_t *oldptr;
    last_request_ptr = tor_malloc_zero(sizeof(time_t));
    *last_request_ptr = now;
    oldptr = strmap_set(last_hid_serv_requests, hsdir_desc_comb_id,
                        last_request_ptr);
    tor_free(oldptr);
  } else {
    last_request_ptr = strmap_get(last_hid_serv_requests,
                                  hsdir_desc_comb_id);
  }

  tor_free(hsdir_desc_comb_id);
  return (last_request_ptr) ? *last_request_ptr : 0;
}

/** Clean the history of request times to hidden service directories, so that
 * it does not contain requests older than REND_HID_SERV_DIR_REQUERY_PERIOD
 * seconds any more. */
void
hs_clean_last_hid_serv_requests(time_t now)
{
  strmap_iter_t *iter;
  time_t cutoff = now - hs_hsdir_requery_period(get_options());
  strmap_t *last_hid_serv_requests = get_last_hid_serv_requests();
  for (iter = strmap_iter_init(last_hid_serv_requests);
       !strmap_iter_done(iter); ) {
    const char *key;
    void *val;
    time_t *ent;
    strmap_iter_get(iter, &key, &val);
    ent = (time_t *) val;
    if (*ent < cutoff) {
      iter = strmap_iter_next_rmv(last_hid_serv_requests, iter);
      tor_free(ent);
    } else {
      iter = strmap_iter_next(last_hid_serv_requests, iter);
    }
  }
}

/** Remove all requests related to the descriptor request key string
 * <b>req_key_str</b> from the history of times of requests to hidden service
 * directories.
 *
 * This is called from purge_hid_serv_request(), which must be idempotent, so
 * any future changes to this function must leave it idempotent too. */
void
hs_purge_hid_serv_from_last_hid_serv_requests(const char *req_key_str)
{
  strmap_iter_t *iter;
  strmap_t *last_hid_serv_requests = get_last_hid_serv_requests();

  for (iter = strmap_iter_init(last_hid_serv_requests);
       !strmap_iter_done(iter); ) {
    const char *key;
    void *val;
    strmap_iter_get(iter, &key, &val);

    /* XXX: The use of REND_DESC_ID_V2_LEN_BASE32 is very wrong in terms of
     * semantic, see #23305. */

    /* This strmap contains variable-sized elements so this is a basic length
     * check on the strings we are about to compare. The key is variable sized
     * since it's composed as follows:
     *   key = base32(hsdir_identity) + base32(req_key_str)
     * where 'req_key_str' is the ed25519 blinded public key of the HS v3. */
    if (strlen(key) < REND_DESC_ID_V2_LEN_BASE32 + strlen(req_key_str)) {
      iter = strmap_iter_next(last_hid_serv_requests, iter);
      continue;
    }

    /* Check if the tracked request matches our request key */
    if (tor_memeq(key + REND_DESC_ID_V2_LEN_BASE32, req_key_str,
                  strlen(req_key_str))) {
      iter = strmap_iter_next_rmv(last_hid_serv_requests, iter);
      tor_free(val);
    } else {
      iter = strmap_iter_next(last_hid_serv_requests, iter);
    }
  }
}

/** Purge the history of request times to hidden service directories,
 * so that future lookups of an HS descriptor will not fail because we
 * accessed all of the HSDir relays responsible for the descriptor
 * recently. */
void
hs_purge_last_hid_serv_requests(void)
{
 /* Don't create the table if it doesn't exist yet (and it may very
   * well not exist if the user hasn't accessed any HSes)... */
  strmap_t *old_last_hid_serv_requests = last_hid_serv_requests_;
  /* ... and let get_last_hid_serv_requests re-create it for us if
   * necessary. */
  last_hid_serv_requests_ = NULL;

  if (old_last_hid_serv_requests != NULL) {
    log_info(LD_REND, "Purging client last-HS-desc-request-time table");
    strmap_free(old_last_hid_serv_requests, tor_free_);
  }
}

/***********************************************************************/

/** Given the list of responsible HSDirs in <b>responsible_dirs</b>, pick the
 *  one that we should use to fetch a descriptor right now. Take into account
 *  previous failed attempts at fetching this descriptor from HSDirs using the
 *  string identifier <b>req_key_str</b>. We return whether we are rate limited
 *  into *<b>is_rate_limited_out</b> if it is not NULL.
 *
 *  Steals ownership of <b>responsible_dirs</b>.
 *
 *  Return the routerstatus of the chosen HSDir if successful, otherwise return
 *  NULL if no HSDirs are worth trying right now. */
routerstatus_t *
hs_pick_hsdir(smartlist_t *responsible_dirs, const char *req_key_str,
              bool *is_rate_limited_out)
{
  smartlist_t *usable_responsible_dirs = smartlist_new();
  const or_options_t *options = get_options();
  routerstatus_t *hs_dir;
  time_t now = time(NULL);
  int excluded_some;
  bool rate_limited = false;
  int rate_limited_count = 0;
  int responsible_dirs_count = smartlist_len(responsible_dirs);

  tor_assert(req_key_str);

  /* Clean outdated request history first. */
  hs_clean_last_hid_serv_requests(now);

  /* Only select those hidden service directories to which we did not send a
   * request recently and for which we have a router descriptor here.
   *
   * Use for_direct_connect==0 even if we will be connecting to the node
   * directly, since we always use the key information in the
   * consensus-indexed node descriptors for building the index.
   **/
  SMARTLIST_FOREACH_BEGIN(responsible_dirs, routerstatus_t *, dir) {
    time_t last = hs_lookup_last_hid_serv_request(dir, req_key_str, 0, 0);
    const node_t *node = node_get_by_id(dir->identity_digest);
    if (last + hs_hsdir_requery_period(options) >= now ||
        !node || !node_has_preferred_descriptor(node, 0)) {
      SMARTLIST_DEL_CURRENT(responsible_dirs, dir);
      rate_limited_count++;
      continue;
    }
    if (!routerset_contains_node(options->ExcludeNodes, node)) {
      smartlist_add(usable_responsible_dirs, dir);
    }
  } SMARTLIST_FOREACH_END(dir);

  if (rate_limited_count > 0 || responsible_dirs_count > 0) {
    rate_limited = rate_limited_count == responsible_dirs_count;
  }

  excluded_some =
    smartlist_len(usable_responsible_dirs) < smartlist_len(responsible_dirs);

  hs_dir = smartlist_choose(usable_responsible_dirs);
  if (!hs_dir && !options->StrictNodes) {
    hs_dir = smartlist_choose(responsible_dirs);
  }

  smartlist_free(responsible_dirs);
  smartlist_free(usable_responsible_dirs);
  if (!hs_dir) {
    const char *warn_str = (rate_limited) ? "we are rate limited." :
                              "we requested them all recently without success";
    log_info(LD_REND, "Could not pick one of the responsible hidden "
                      "service directories, because %s.", warn_str);
    if (options->StrictNodes && excluded_some) {
      log_warn(LD_REND, "Could not pick a hidden service directory for the "
               "requested hidden service: they are all either down or "
               "excluded, and StrictNodes is set.");
    }
  } else {
    /* Remember that we are requesting a descriptor from this hidden service
     * directory now. */
    hs_lookup_last_hid_serv_request(hs_dir, req_key_str, now, 1);
  }

  if (is_rate_limited_out != NULL) {
    *is_rate_limited_out = rate_limited;
  }

  return hs_dir;
}

/** Given a list of link specifiers lspecs, a curve 25519 onion_key, and
 * a direct connection boolean direct_conn (true for single onion services),
 * return a newly allocated extend_info_t object.
 *
 * This function always returns an extend info with a valid IP address and
 * ORPort, or NULL. If direct_conn is false, the IP address is always IPv4.
 *
 * It performs the following checks:
 *  if there is no usable IP address, or legacy ID is missing, return NULL.
 *  if direct_conn, and we can't reach any IP address, return NULL.
 */
extend_info_t *
hs_get_extend_info_from_lspecs(const smartlist_t *lspecs,
                               const curve25519_public_key_t *onion_key,
                               int direct_conn)
{
  int have_v4 = 0, have_legacy_id = 0, have_ed25519_id = 0;
  char legacy_id[DIGEST_LEN] = {0};
  ed25519_public_key_t ed25519_pk;
  extend_info_t *info = NULL;
  tor_addr_port_t ap;

  tor_addr_make_null(&ap.addr, AF_UNSPEC);
  ap.port = 0;

  if (lspecs == NULL) {
    log_warn(LD_BUG, "Specified link specifiers is null");
    goto done;
  }

  if (onion_key == NULL) {
    log_warn(LD_BUG, "Specified onion key is null");
    goto done;
  }

  if (smartlist_len(lspecs) == 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_REND, "Empty link specifier list.");
    /* Return NULL. */
    goto done;
  }

  SMARTLIST_FOREACH_BEGIN(lspecs, const link_specifier_t *, ls) {
    switch (link_specifier_get_ls_type(ls)) {
    case LS_IPV4:
      /* Skip if we already seen a v4. If direct_conn is true, we skip this
       * block because reachable_addr_choose_from_ls() will set ap. If
       * direct_conn is false, set ap to the first IPv4 address and port in
       * the link specifiers.*/
      if (have_v4 || direct_conn) continue;
      tor_addr_from_ipv4h(&ap.addr,
                          link_specifier_get_un_ipv4_addr(ls));
      ap.port = link_specifier_get_un_ipv4_port(ls);
      have_v4 = 1;
      break;
    case LS_LEGACY_ID:
      /* Make sure we do have enough bytes for the legacy ID. */
      if (link_specifier_getlen_un_legacy_id(ls) < sizeof(legacy_id)) {
        break;
      }
      memcpy(legacy_id, link_specifier_getconstarray_un_legacy_id(ls),
             sizeof(legacy_id));
      have_legacy_id = 1;
      break;
    case LS_ED25519_ID:
      memcpy(ed25519_pk.pubkey,
             link_specifier_getconstarray_un_ed25519_id(ls),
             ED25519_PUBKEY_LEN);
      have_ed25519_id = 1;
      break;
    default:
      /* Ignore unknown. */
      break;
    }
  } SMARTLIST_FOREACH_END(ls);

  /* Choose a preferred address first, but fall back to an allowed address. */
  if (direct_conn)
    reachable_addr_choose_from_ls(lspecs, 0, &ap);

  /* Legacy ID is mandatory, and we require an IP address. */
  if (!tor_addr_port_is_valid_ap(&ap, 0)) {
    /* If we're missing the IP address, log a warning and return NULL. */
    log_info(LD_NET, "Unreachable or invalid IP address in link state");
    goto done;
  }
  if (!have_legacy_id) {
    /* If we're missing the legacy ID, log a warning and return NULL. */
    log_warn(LD_PROTOCOL, "Missing Legacy ID in link state");
    goto done;
  }

  /* We will add support for falling back to a 3-hop path in a later
   * release. */

  /* We'll validate now that the address we've picked isn't a private one. If
   * it is, are we allowed to extend to private addresses? */
  if (!extend_info_addr_is_allowed(&ap.addr)) {
    log_fn(LOG_PROTOCOL_WARN, LD_REND,
           "Requested address is private and we are not allowed to extend to "
           "it: %s:%u", safe_str(fmt_addr(&ap.addr)), ap.port);
    goto done;
  }

  /* We do have everything for which we think we can connect successfully. */
  info = extend_info_new(NULL, legacy_id,
                         (have_ed25519_id) ? &ed25519_pk : NULL, NULL,
                         onion_key, &ap.addr, ap.port, NULL, false);
 done:
  return info;
}

/***********************************************************************/

/** Initialize the entire HS subsystem. This is called in tor_init() before any
 * torrc options are loaded. Only for >= v3. */
void
hs_init(void)
{
  hs_circuitmap_init();
  hs_service_init();
  hs_cache_init();
}

/** Release and cleanup all memory of the HS subsystem (all version). This is
 * called by tor_free_all(). */
void
hs_free_all(void)
{
  hs_circuitmap_free_all();
  hs_service_free_all();
  hs_cache_free_all();
  hs_client_free_all();
  hs_ob_free_all();
}

/** For the given origin circuit circ, decrement the number of rendezvous
 * stream counter. This handles every hidden service version. */
void
hs_dec_rdv_stream_counter(origin_circuit_t *circ)
{
  tor_assert(circ);

  if (circ->hs_ident) {
    circ->hs_ident->num_rdv_streams--;
  } else {
    /* Should not be called if this circuit is not for hidden service. */
    tor_assert_nonfatal_unreached();
  }
}

/** For the given origin circuit circ, increment the number of rendezvous
 * stream counter. This handles every hidden service version. */
void
hs_inc_rdv_stream_counter(origin_circuit_t *circ)
{
  tor_assert(circ);

  if (circ->hs_ident) {
    circ->hs_ident->num_rdv_streams++;
  } else {
    /* Should not be called if this circuit is not for hidden service. */
    tor_assert_nonfatal_unreached();
  }
}

/** Return a newly allocated link specifier object that is a copy of dst. */
link_specifier_t *
link_specifier_dup(const link_specifier_t *src)
{
  link_specifier_t *dup = NULL;
  uint8_t *buf = NULL;

  if (BUG(!src)) {
    goto err;
  }

  ssize_t encoded_len_alloc = link_specifier_encoded_len(src);
  if (BUG(encoded_len_alloc < 0)) {
    goto err;
  }

  buf = tor_malloc_zero(encoded_len_alloc);
  ssize_t encoded_len_data = link_specifier_encode(buf,
                                                   encoded_len_alloc,
                                                   src);
  if (BUG(encoded_len_data < 0)) {
    goto err;
  }

  ssize_t parsed_len = link_specifier_parse(&dup, buf, encoded_len_alloc);
  if (BUG(parsed_len < 0)) {
    goto err;
  }

  goto done;

 err:
  dup = NULL;

 done:
  tor_free(buf);
  return dup;
}
